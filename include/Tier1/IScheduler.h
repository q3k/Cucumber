#ifndef __ISCHEDULER_H__
#define __ISCHEDULER_H__

#include "Tier1/CTask.h"

namespace cb {
    class IScheduler {
        public:
            virtual void Enable(bool Enabled) = 0;
            virtual void AddTask(CTask *Task) = 0;
            virtual void NextTask(void) = 0;
            virtual CTask *GetCurrentTask(void) = 0;
    };
};

#endif
