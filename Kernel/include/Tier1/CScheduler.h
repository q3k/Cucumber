#ifndef __CSCHEDULER_H__
#define __CSCHEDULER_H__

#include "types.h"

#include "Tier1/CTask.h"
#include "Tier1/IScheduler.h"
#include "Tier1/CSemaphore.h"

#define CSCHEDULER_INTERRUPT_YIELD 0x99

extern "C" {
    #include "Tier0/interrupts.h"
}

namespace cb {
    typedef struct STaskQueueNode {
        CTask *Task;
        struct STaskQueueNode *Next;
    } TTaskQueueNode;
    class CScheduler{
        private:
            IScheduler *m_CurrentScheduler;
            
            static void Yield(T_ISR_REGISTERS Registers);
            static void TimerTick(T_ISR_REGISTERS Registers);

            static u64 m_NumTicks;
        public:
            CScheduler(void);
            static void Enable(void);
            static void AddTask(CTask *Task);
            static CTask *GetCurrentTask(void);
            static void NextTask();
            static void PrioritizeTask(CTask *Task);
            
            static void DispatchAvailableSemaphore(CSemaphore *Semaphore);
    };
};

#endif
