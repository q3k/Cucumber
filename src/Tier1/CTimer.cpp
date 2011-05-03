#include "Tier1/CTimer.h"
using namespace cb;

extern "C" {
    #include "Tier0/kstdio.h"
    #include "Tier0/interrupts.h"
}

volatile u32 CTimer::m_NumTicks = 0;
bool CTimer::m_Initialized = false;
CLinearList<TCallbackInfo> CTimer::m_Callbacks;

void CTimer::Initialize(void)
{
    u32 Divider = 1193180 / 1000; // microseconds
    koutb(0x43, 0x36);
    u8 Low = (u8)(Divider & 0xFF);
    u8 High = (u8)((Divider >> 8) & 0xFF);
    koutb(0x40, Low);
    koutb(0x40, High);
    
    interrupts_setup_irq(0x00, (void*)Dispatch);
    m_Initialized = true;
}

void CTimer::Dispatch(void *Registers)
{
    __asm__ volatile("cli");
    m_NumTicks++;
    
    if (m_NumTicks > (0xFFFFFFFF - 1000)) // 1000 ticks margin
        Reschedule();
    for (u32 i = 0; i < m_Callbacks.GetSize(); i++)
    {
        TCallbackInfo &Callback = m_Callbacks[i];
        if (Callback.Times == -1 || Callback.Times > 0)
        {
            if (m_NumTicks > Callback.NextCall)
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
                    continue;
                }
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
}

void CTimer::Reschedule(void)
{
    u32 Amount = 0xFFFFFFFF - 1000;
    
    for (u32 i = 0; i < m_Callbacks.GetSize(); i++)
    {
        TCallbackInfo &Callback = m_Callbacks[i];
        Callback.NextCall -= Amount;
    }
    
    m_NumTicks -= Amount;
}

void CTimer::Create(u32 Interval, s32 Times, TTimerCallback Callback, u32 Extra)
{
    if (!m_Initialized)
        Initialize();
    
    TCallbackInfo NewCallback;
    
    NewCallback.Interval = Interval;
    NewCallback.Times = Times;
    NewCallback.Callback = Callback;
    NewCallback.NextCall = m_NumTicks + Interval;
    NewCallback.Extra = Extra;
    
    m_Callbacks.Push(NewCallback);
}
