#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#include "types.h"

extern "C" {
    #include "Tier0/atomic_operations.h"
};

namespace cb {
    class CSemaphore {
        private:
            T_ATOMIC m_Available;
        public:
            CSemaphore(u32 Available = 1);
            void Acquire(void);
            void Release(void);
    };
};

#endif
