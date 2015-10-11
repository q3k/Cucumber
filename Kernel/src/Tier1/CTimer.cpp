#include "Tier1/CTimer.h"
#include "Tier1/CScheduler.h"
using namespace cb;

extern "C" {
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
    u32 Divider = 1193180 / 1000; // microseconds
    koutb(0x43, 0x36);
    u8 Low = (u8)(Divider & 0xFF);
    u8 High = (u8)((Divider >> 8) & 0xFF);
    koutb(0x40, Low);
    koutb(0x40, High);

    interrupts_setup_irq(0x00, (void*)Dispatch);
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
    __asm__ volatile("cli");

    if (m_FastHook)
    	(*m_FastHook)(Registers);

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
    interrupts_irq_finish(0);
    __asm__ volatile("sti");

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
