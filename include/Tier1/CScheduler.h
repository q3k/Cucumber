#ifndef __CSCHEDULER_H__
#define __CSCHEDULER_H__

#include "types.h"

#include "Tier1/CTask.h"
#include "Tier1/IScheduler.h"
#include "Tier1/CSemaphore.h"

extern "C" {
    #include "Tier0/interrupts.h"
}

namespace cb {
    typedef struct STaskQueueNode {
        CTask *Task;
        struct STaskQueueNode *Next;
    } TTaskQueueNode;
    class CScheduler {
        private:
            IScheduler *m_CurrentScheduler;
            
            static bool TimerTick(u32 Extra);
        public:
            CScheduler(void);
            static void Enable(void);
            static void AddTask(CTask *Task);
            static CTask *GetCurrentTask(void);
            static void NextTask(void);
            
            static void DispatchAvailableSemaphore(CSemaphore *Semaphore);
    };
};

#endif
