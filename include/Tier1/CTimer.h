#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "Tier1/CInterruptDispatcher.h"
#include "Tier1/Util/CLinearList.h"
#include "types.h"

namespace cb {
    typedef bool(*TTimerCallback)(u32);
    typedef struct {
                TTimerCallback Callback;
                u32 Interval;
                s32 Times;
                u32 NextCall;
                u32 Extra;
            } TCallbackInfo;
    class CTimer {
        public:
            static void Create(u32 Interval, s32 Times, TTimerCallback Callback,
                                                                 u32 Extra = 0);
            inline static u32 GetTicks(void)
            {
                if (!m_Initialized)
                    Initialize();
                return m_NumTicks;
            }
        private:
            static CLinearList<TCallbackInfo> m_Callbacks;
            volatile static u32 m_NumTicks;
            static void Initialize(void);
            static void Dispatch(void *Registers);
            static bool m_Initialized;
            
            // Called when the number of ticks is just about to overflow.
            // Reschedules all the callbacks.
            static void Reschedule(void);
    };
};

#endif
