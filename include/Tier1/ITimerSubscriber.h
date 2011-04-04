#ifndef __ITIMERSUBSRCIBER_H__
#define __ITIMERSUBSCRIBER_H__

#include "Tier1/CTimer.h"

namespace cb {
    class CTimer;
    // An interface used to subscribe to a CTimer
    class ITimerSubscriber {
        public:
            // Gets called after subscribing to a tick with TickDelta
            virtual void Tick(u32 TickDelta);
            
            // Returns wether we should be called
            virtual bool Enabled(void);
            
            // Basic functions called be derivee        
            // Unsubscribe from the current subscribed CTimer
            void Unsubscribe(void);
            
            void Subscribe(CTimer *Timer, u32 Tick);
    };
};

#endif
