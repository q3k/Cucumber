#ifndef __ISCHEDULER_H__
#define __ISCHEDULER_H__

#include "Tier1/CTask.h"
#include "Tier1/CSemaphore.h"

extern "C" {
    #include "Tier0/interrupts.h"
}

namespace cb {
    class IScheduler {
        public:
            virtual void Enable(bool Enabled) = 0;
            virtual void AddTask(CTask *Task) = 0;
            virtual void NextTask(T_ISR_REGISTERS Registers) = 0;
            virtual CTask *GetCurrentTask(void) = 0;
            virtual void SetSemaphoreAvailable(CSemaphore *Semaphore) = 0;
            virtual void PrioritizeTask(CTask *Task) = 0;
    };
};

#endif
