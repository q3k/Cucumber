#ifndef __CROUNDROBINSCHEDULER_H__
#define __CROUNDROBINSCHEDULER_H__

#include "Tier1/IScheduler.h"
#include "Tier1/Util/CLinearList.h"

extern "C" {
    #include "Tier0/semaphore.h"
}

namespace cb {
    class CRoundRobinScheduler : public IScheduler {
        public:
            void Enable(bool Enabled);
            void AddTask(CTask *Task);
            void NextTask(T_ISR_REGISTERS Registers, void (*EOI)(void));
            CTask *GetCurrentTask(void);
            void SetSemaphoreAvailable(CSemaphore *Semaphore);
            void PrioritizeTask(CTask *Task);
            bool InScheduler(void) {
                return m_InScheduler;
            }
        private:
            bool m_InScheduler;
            T_SEMAPHORE m_SchedulerLock;
            CTask *m_CurrentTask;
            CLinearList<CTask *> m_TaskQueue;
            u32 m_iTaskQueuePosition;

            CTask *m_PrioritizedTask;
    };
};

#endif
