#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "Tier1/CInterruptDispatcher.h"
#include "Tier1/Util/CLinearList.h"
#include "Tier1/CTask.h"
#include "Tier1/CSemaphore.h"
#include "types.h"

namespace cb {
    typedef bool(*TTimerCallback)(u32);
    typedef void(*TTimerFastHook)(u32, u32, u32, u32, u32, u32, u32, u32, u32);
    typedef struct {
                TTimerCallback Callback;
                u32 Interval;
                s32 Times;
                u32 NextCall;
                u32 Extra;
                CTask *Task;
            } TCallbackInfo;
    class CTimer {
        public:
            static void Create(u32 Interval, s32 Times, TTimerCallback Callback,
                                                                 u32 Extra = 0);
            // This is like ::Create, but faster, and there can only be one, and it's fixed interval
            static void SetFastTimerHook(TTimerFastHook Hook);
            inline static u32 GetTicks(void)
            {
                if (!m_bInitialized)
                    Initialize();
                m_GetTicksSemaphore.Acquire();
                volatile u32 Ticks = m_nTicks;
                m_GetTicksSemaphore.Release();
                return Ticks;
            }
        private:
            static CLinearList<TCallbackInfo> m_Callbacks;
            volatile static u32 m_nTicks;
            static void Initialize(void);
            static void Dispatch(u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 eip);
            static bool m_bInitialized;
            static TTimerFastHook m_FastHook;
            
            static CSemaphore m_GetTicksSemaphore;

            // Called when the number of ticks is just about to overflow.
            // Reschedules all the callbacks.
            static void Reschedule(void);
    };
};

#endif
