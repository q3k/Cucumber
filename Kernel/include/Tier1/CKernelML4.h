#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdlib.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/heap.h"
};

// This is more-or less just a C++ wrapper for T_PAGING_ML4. For use for pure-kernel processes only
// It implements the following address space, virtually:
//
//  name   |       start        |        end          |              mapping
// -------- -------------------- --------------------- ------------------------------------------------------------------
// LOWMEM  - 0x0000000000000000 - 0x0000000000100000 -> lower 1MiB of physical memory, common, fits in 1/2 of a Table
// SCRATCH - 0xFFFFFFFF00000000 - 0xFFFFFFFF3FFFFFFF -> kernel scratch space & heap. 4GiB, common, fits in 1/1 of a Directory
// STACK   - 0xFFFFFFFF40000000 - 0xFFFFFFFF4FFFFFFF -> kernel stack, unique per ML4, 256MiB, fits in 1/16 of a Directory,
// TEXT    - 0xFFFFFFFF80000000 - 0xFFFFFFFF8xxxxxxx -> kernel code physical location and the temp page, common, fits in x/16 of a Directory

#define AREA_LOWMEM_START 0x0000000000000000
#define AREA_LOWMEM_SIZE (256*1024*1024)
#define AREA_SCRATCH_START 0xFFFFFFFF00000000
#define AREA_STACK_START 0xFFFFFFFF40000000

namespace cb {
    template<typename T> class CPagingStructure {
        public:
            T m_Entries[512];
            void * operator new(unsigned long Size, void * Memory) throw() {
                return Memory;
            }
            CPagingStructure(void) {
                kmemset(m_Entries, 0, sizeof(m_Entries));
            }
            T *GetEntry(u64 Index) {
                ASSERT(Index < 0x200);
                return m_Entries[Index];
            }
            bool IsEntryPresent(u64 Index) {
                ASSERT(Index < 0x200);
                return m_Entries[Index].Present;
            }
            u64 GetEntryPhysical(u64 Index) {
                ASSERT(Index < 0x200);
                return m_Entries[Index].Physical << 12;
            }
            template<typename C> void GetEntry(u64 Index, C **NewOut) {
                ASSERT(Index < 0x200);
                if (NewOut)
                    *NewOut = (C *)(m_Entries[Index].Physical << 12);
            }
            void SetEntry(u64 Index, u64 Physical) {
                ASSERT(Index < 0x200);
                m_Entries[Index].Physical = (Physical >> 12);
                m_Entries[Index].Present = 1;
                m_Entries[Index].RW = 1;
            }
            template<typename C> void NewEntry(u64 Index, C **NewOut) {
                ASSERT(Index < 0x200);
                C *New = (C *)kmalloc_aligned_physical(sizeof(C));
                new(New) C();
                m_Entries[Index].Physical = (((u64)New) >> 12);
                m_Entries[Index].Present = 1;
                m_Entries[Index].RW = 1;
                if (NewOut)
                    *NewOut = New;
            }
            template<typename C> void GetOrNewEntry(u64 Index, C **NewOut) {
                if (IsEntryPresent(Index))
                    GetEntry(Index, NewOut);
                else
                    NewEntry(Index, NewOut);
            }

    };
    class CKernelML4 {
        private:
            // The paging direcotry structure
            CPagingStructure<T_PAGING_ML4_ENTRY> *m_ML4;

            // Are we running this mapping?
            bool m_Running;

            // allocator and destructor for segments
            void *(*m_SegmentAllocator)(u64);
            void (*m_SegmentDestructor)(void *);

            // kernel stack segment physical memory
            u64 m_StackStartPhysical;
            u64 m_StackSize;

            // static pointers to common areas
            static T_PAGING_TAB *m_LOWMEM;
            static u64 m_LOWMEM_Physical;
            static T_PAGING_DIR *m_SCRATCH;
            static u64 m_SCRATCH_Physical;
            static T_PAGING_DIR *m_TEXT;
            static u64 m_TEXT_Physical;
        protected:
            template<typename P, typename C> static C* GetOrCreateSubStructure(P *Parent);
            
        public:
            // Creates a new page directory for a kernel task, with new stack and other internal stuff
            CKernelML4(bool AllocateStack = true);

            // Destroys the structures and frees the segments, if needed
            ~CKernelML4(void);

            // Maps a physical page
            void Map(u64 Virtual, u64 Physical);
            // Maps a physical area
            void Map(u64 VirtualStart, u64 Physical, u64 Size);
            // Resolves Virtual -> Physical
            u64 Resolve(u64 Virtual);

            // Gets physical start ofs tack
            u64 GetStackStartPhysical(void) { return m_StackStartPhysical; }

            // Use Mapping
            void Apply(void);

            // Use lambda in stack
            template <typename F> void UseStack(F lambda) {
                auto function = static_cast<void (*) (CKernelML4 *ML4)>(lambda);
                u64 Stack = AREA_STACK_START + 1 * 1024 * 1024;
                __asm__ __volatile__(
                        "pushq %%rbp\n"
                        "movq %%rsp, %%rax\n"

                        "movq %0, %%rsp\n"
                        "movq %0, %%rbp\n"
                        "pushq %%rax\n"

                        "pushq %2\n"
                        "callq *%1\n"
                        "addq $8, %%rsp\n"

                        "popq %%rax\n"
                        "movq %%rax, %%rsp\n"
                        "popq %%rbp"
                    :
                    :"r"(Stack), "r"(function), "r"(this)
                    :"%rax"
                );

            }

            // Debug stdout dump
            void Dump(void);

            // Get physical address of top structure
            u64 GetPhysical(void) {
                return (u64)m_ML4;
            }
    };
};

#endif
