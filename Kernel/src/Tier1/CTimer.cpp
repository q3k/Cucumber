#include "Tier1/CTimer.h"
#include "Tier1/CScheduler.h"
using namespace cb;

extern "C" {
    #include "Tier0/apic.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/interrupts.h"
}

TTimerFastHook CTimer::m_FastHook = 0;
volatile u64 CTimer::m_nTicks = 0;
bool CTimer::m_bInitialized = false;
CLinearList<TCallbackInfo> CTimer::m_Callbacks;
CSemaphore CTimer::m_GetTicksSemaphore;

void CTimer::Initialize(void)
{
    apic_periodic(100, Dispatch);
    m_bInitialized = true;
}

void CTimer::SetFastTimerHook(TTimerFastHook Hook)
{
	__asm__ volatile("cli");

	m_FastHook = Hook;
    if (!m_bInitialized)
        Initialize();

	__asm__ volatile("sti");
}

void CTimer::Dispatch(T_ISR_REGISTERS Registers)
{
    //__asm__ volatile("cli");
    if (m_FastHook)
    	(*m_FastHook)(Registers, apic_eoi);
    DispatchCallbacks(Registers);
}

void CTimer::DispatchCallbacks(T_ISR_REGISTERS Registers)
{
    m_nTicks++;
    
    if (m_nTicks > (0xFFFFFFFFFFFFFFFF - 1000)) // 1000 ticks margin
        Reschedule();
    for (u64 i = 0; i < m_Callbacks.GetSize(); i++)
    {
        TCallbackInfo &Callback = m_Callbacks[i];
        if (Callback.Times == -1 || Callback.Times > 0)
        {
            if (m_nTicks > Callback.NextCall)
            {
                if (Callback.Times != -1)
                    Callback.Times--;
                    
                Callback.NextCall += Callback.Interval;
                
                // Call the callback
                bool Continue = (Callback.Callback)(Callback.Extra);
                if (!Continue)
                {
                    m_Callbacks.Delete(i);
                    // TODO: fix this hack
                    i--;
                }
                // We can only take care of one timer callback...
                CScheduler::PrioritizeTask(Callback.Task);
                break;
            }
        }
        else
        {
            m_Callbacks.Delete(i);
            // TODO: fix this hack
            i--;
            continue;
        }
    }
    apic_eoi();
    //__asm__ volatile("sti");

    // Tough love for the current task.
    //CScheduler::GetCurrentTask()->Yield();
}

void CTimer::Reschedule(void)
{
    u64 Amount = 0xFFFFFFFFFFFFFFFF - 1000;
    
    for (u64 i = 0; i < m_Callbacks.GetSize(); i++)
    {
        TCallbackInfo &Callback = m_Callbacks[i];
        Callback.NextCall -= Amount;
    }
    
    m_nTicks -= Amount;
}

void CTimer::Create(u64 Interval, s64 Times, TTimerCallback Callback, u64 Extra)
{
    if (!m_bInitialized)
        Initialize();
    
    TCallbackInfo NewCallback;
    
    NewCallback.Interval = Interval;
    NewCallback.Times = Times;
    NewCallback.Callback = Callback;
    // Why minus 1? "Lag" compensation.
    NewCallback.NextCall = m_nTicks + Interval - 1;
    NewCallback.Extra = Extra;
    NewCallback.Task = CScheduler::GetCurrentTask();
    
    m_Callbacks.Push(NewCallback);
}
