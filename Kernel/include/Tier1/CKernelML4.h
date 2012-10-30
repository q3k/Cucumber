#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
};

// This is more-or less just a C++ wrapper for T_PAGING_ML4. For use for pure-kernel processes only
namespace cb {
    class CKernelML4 {
        private:
            // The paging direcotry structure
            T_PAGING_ML4 *m_Directory;

            // allocator and destructor for segments
            void *(*m_SegmentAllocator)(u64);
            void (*m_SegmentDestructor)(void *);
        public:
            // Creates a new page directory for a kernel task, with new stack and other private segments
            // Allocator is a function pointer to an allocator to use, Destructor is the same but for
            // deallocation
            CKernelML4(void *(*Allocator)(u64), void (*Destructor)(void *));
            // Creates a new page directory for a kernel task, with existing stack
            // This constructor also makes the destructor not free anything.
            CKernelML4(void *StackStart, u64 StackSize);

            // Destroys the structures and frees the segments, if needed
            ~CKernelML4(void);
    };
};

#endif
