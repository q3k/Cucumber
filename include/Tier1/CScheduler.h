#ifndef __CSCHEDULER_H__
#define __CSCHEDULER_H__

#include "types.h"

#include "Tier1/CTask.h"
#include "Tier1/IScheduler.h"

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
            
            static void TimerTick(T_ISR_REGISTERS R);
        public:
            CScheduler(void);
            static void Enable(void);
            static void AddTask(CTask *Task);
            static CTask *GetCurrentTask(void);
    };
};

#endif
