#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "Tier1/CInterruptDispatcher.h"
#include "Tier1/Util/CLinearList.h"
#include "Tier1/CTask.h"
#include "Tier1/CSemaphore.h"
#include "types.h"

extern "C" {
    #include "Tier0/interrupts.h"
}

namespace cb {
    typedef bool(*TTimerCallback)(u64);
    typedef void(*TTimerFastHook)(T_ISR_REGISTERS, void (*)(void));
    typedef struct {
                TTimerCallback Callback;
                u64 Interval;
                s64 Times;
                u64 NextCall;
                u64 Extra;
                CTask *Task;
            } TCallbackInfo;
    class CTimer {
        public:
            static void Create(u64 Interval, s64 Times, TTimerCallback Callback,
                                                                 u64 Extra = 0);
            // This is like ::Create, but faster, and there can only be one, and it's fixed interval
            static void SetFastTimerHook(TTimerFastHook Hook);
            inline static u64 GetTicks(void)
            {
                if (!m_bInitialized)
                    Initialize();
                m_GetTicksSemaphore.Acquire();
                volatile u64 Ticks = m_nTicks;
                m_GetTicksSemaphore.Release();
                return Ticks;
            }
            static void Dispatch(T_ISR_REGISTERS Registers);
            static void DispatchCallbacks(T_ISR_REGISTERS Registers);
        private:
            static CLinearList<TCallbackInfo> m_Callbacks;
            volatile static u64 m_nTicks;
            static void Initialize(void);
            static bool m_bInitialized;
            static TTimerFastHook m_FastHook;
            
            static CSemaphore m_GetTicksSemaphore;

            // Called when the number of ticks is just about to overflow.
            // Reschedules all the callbacks.
            static void Reschedule(void);
    };
};

#endif
