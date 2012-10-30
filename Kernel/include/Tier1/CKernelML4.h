#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
};

// This is more-or less just a C++ wrapper for T_PAGING_ML4. For use for pure-kernel processes only
// It implements the following address space, virtually:
//
//  name   |       start        |        end          |              mapping
// -------- -------------------- --------------------- ------------------------------------------------------------------
// LOWMEM  - 0x0000000000000000 - 0x0000000000100000 -> lower 1MiB of physical memory, common, fits in 1/2 of a Table
// SCRATCH - 0xFFFFFFFF00000000 - 0xFFFFFFFF3FFFFFFF -> kernel scratch space & heap. 4GiB, common, fits in 1/1 of a Directory
// STACK   - 0xFFFFFFFF40000000 - 0xFFFFFFFF4FFFFFFF -> kernel stack, unique per ML4, 256MiB, fits in 1/16 of a Directory,
// TEXT    - 0xFFFFFFFF80000000 - 0xFFFFFFFF8xxxxxxx -> kernel code physical location, common, fits in x/16 of a Directory

namespace cb {
    class CKernelML4 {
        private:
            // The paging direcotry structure
            T_PAGING_ML4 *m_Directory;
            bool m_ShouldFreeML4;

            // allocator and destructor for segments
            void *(*m_SegmentAllocator)(u64);
            void (*m_SegmentDestructor)(void *);

            // kernel stack segment physical memory
            u64 m_StackStartPhysical;
            u64 m_StackSize;

            // static pointers to common areas
            static T_PAGING_TAB *m_LOWMEM;
            static T_PAGING_DIR *m_SCRATCH;
            static T_PAGING_DIR *m_TEXT;
        public:
            // Creates a new page directory for a kernel task, with new stack and other internal stuff
            // Allocator is a function pointer to an allocator to use, Destructor is the same but for
            // deallocation
            CKernelML4(void *(*Allocator)(u64), void (*Destructor)(void *));

            // Destroys the structures and frees the segments, if needed
            ~CKernelML4(void);
    };
};

#endif
