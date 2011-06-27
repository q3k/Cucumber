#ifndef __ISCHEDULER_H__
#define __ISCHEDULER_H__

#include "Tier1/CTask.h"
#include "Tier1/CSemaphore.h"

namespace cb {
    class IScheduler {
        public:
            virtual void Enable(bool Enabled) = 0;
            virtual void AddTask(CTask *Task) = 0;
            virtual void NextTask(u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 eip) = 0;
            virtual CTask *GetCurrentTask(void) = 0;
            virtual void SetSemaphoreAvailable(CSemaphore *Semaphore) = 0;
            virtual void PrioritizeTask(CTask *Task) = 0;
    };
};

#endif
