#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#include "types.h"

namespace cb {
    class CSemaphore {
        private:
            u32 m_Available;
        public:
            CSemaphore(u32 Available = 1);
            void Acquire(void);
            void Release(void);
    };
};

#endif
