#ifndef __CROUNDROBINSCHEDULER_H__
#define __CROUNDROBINSCHEDULER_H__

#include "Tier1/IScheduler.h"
#include "Tier1/Util/CLinearList.h"

namespace cb {
    class CRoundRobinScheduler : public IScheduler {
        public:
            void Enable(bool Enabled);
            void AddTask(CTask *Task);
            void NextTask(void);
            CTask *GetCurrentTask(void);
            void DispatchAvailableSemaphore(CSemaphore *Semaphore);
        private:
            CTask *m_CurrentTask;
            CLinearList<CTask *> m_TaskQueue;
            u32 m_TaskQueuePosition;
    };
};

#endif
