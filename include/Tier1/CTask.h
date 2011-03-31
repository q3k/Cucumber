#ifndef __CTASK_H__
#define __CTASK_H__

#include "types.h"
#include "Tier0/paging.h"

namespace cb {
    enum ETaskPriority {
        ETP_STALL,
        ETP_LOW,
        ETP_NORMAL,
        ETP_HIGH,
        ETP_REALTIME
    };
    enum ETaskRing {
        ETP_RING0,
        ETP_RING1,
        ETP_RING2,
        ETP_RING3
    };
    class CTask {
        private:
            void *m_Owner; //TODO: Replace me with a real type
            T_PAGING_DIRECTORY *m_Directory;            
            ETaskPriority m_Priority;
            ETaskRing m_Ring;
            
            u32 HeapStart;
            u32 HeapSize;
            u32 StackStart;
            u32 StackSize;
            u32 ImageStart;
            u32 ImageSize;
            u32 KernelStart;
            u32 KernelSize;
        
        public:
            // Default constructor... creates a new kernel task.
            CTask(void);
            
            // Equivalent of the POSIX fork() call.
            u8 Fork(void);
            
    };
};

#endif
