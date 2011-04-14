#ifndef __CTASK_H__
#define __CTASK_H__

#include "types.h"
#include "Tier1/CPageDirectory.h"
#include "Tier1/CKernel.h"

// Task memory map...
//              _______________________
//  0x00000000 |                       | This is left unused because of possible
//             |    thar be dragons    | segfaults due to dereferencing of null
//             |_______________________| pointers and other crap like that
//  0x10000000 |                       |
//             |     process code      | The process image lies here, unless
//             |_______________________| we're in the kernel, then it's unused
//  0x20000000 |                       |
//             |     process heap      |
//             |_______________________|
//  0x30000000 |                       |
//             |     process heap      | The process heap is here. Managed via
//             |_______________________| the tier0 heap mechanism
//  0x40000000 |                       |
//             |     process heap      |
//             |_______________________|
//  0x50000000 |                       |
//             |     process heap      |
//             |_______________________|
//  0x60000000 |                       |
//             |     process data      |
//             |_______________________|  .data, .rodata and the like
//  0x70000000 |                       |
//             |     process data      |
//             |_______________________|
//  0x80000000 |                       |
//             |                       |
//             |_______________________|
//  0x90000000 |                       |
//             |                       |
//             |_______________________|
//  0xA0000000 |                       | Process stack. Starts at 4kb, then
//             |     process stack     | grows if needed
//             |_______________________| 
//  0xB0000000 |                       |
//             |                       |
//             |_______________________|
//  0xC0000000 |                       |
//             |      kernel code      | 
//             |_______________________| We need this in every process.
//  0xD0000000 |                       |
//             |      kernel heap      |
//             |_______________________|
//  0xE0000000 |                       |
//             |          IPC          | 
//             |_______________________| For IPC (message passing)
//  0xF0000000 |                       |
//             |          IPC          |
//             |_______________________|
//  0xFFFFFFFF

#define TASK_MAP_CODE_START 0x10000000
#define TASK_MAP_CODE_SIZE 0x10000000

#define TASK_MAP_HEAP_START 0x20000000
#define TASK_MAP_HEAP_SIZE 0x40000000

#define TASK_MAP_STACK_START 0xA0000000
#define TASK_MAP_STACK_SIZE 0x10000000

#define TASK_MAP_KERNEL_START 0xC0000000
#define TASK_MAP_KERNEL_SIZE 0x20000000

extern "C" {
    u32 ctask_geteip(void);
}

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
    class CPageDirectory;
    class CTask {
        friend class CKernel;
        friend class CScheduler;
        protected:
            void *m_Owner; //TODO: Replace me with a real type
            CPageDirectory *m_Directory;            
            ETaskPriority m_Priority;
            ETaskRing m_Ring;
            bool m_User;
            u32 m_PID;
            
            u32 m_ESP, m_EIP, m_EBP;
            
            u32 m_HeapStart;
            u32 m_HeapSize;
            
            u32 m_StackStart;
            u32 m_StackSize;
            
            u32 m_ImageStart;
            u32 m_ImageSize;
            
            u32 m_KernelStart;
            u32 m_KernelSize;
            
            bool m_CreatedStack;
            
            void CreateStack(void);
            void CreateDirectory(void);
            
            void CopyKernelMemory(void);
            void CopyStack(CTask *Source);
        public:
            CTask(bool User = 0, bool Empty = false);
            ~CTask(void);
            
            CPageDirectory *GetPageDirectory(void);
            
            // Equivalent of the POSIX fork() call.
            CTask *Fork(void);
            
            inline u32 GetPID(void) { return m_PID; }
            inline u32 GetESP(void) { return m_ESP; }
            inline u32 GetEIP(void) { return m_EIP; }
            inline u32 GetEBP(void) { return m_EBP; }
            
            inline u32 GetPageDirectoryPhysicalAddress(void)
            {
                //return m_Directory->m_Directory->PhysicalAddress;
                return 0;
            }
            
            inline void SetESP(u32 ESP) { m_ESP = ESP; }
            inline void SetEIP(u32 EIP) { m_EIP = EIP; }
            inline void SetEBP(u32 EBP) { m_EBP = EBP; }
            
            inline void SetPageDirectory(CPageDirectory *Directory)
            {
                m_Directory = Directory;
            }
            
            void Dump(void);
            
    };
};

#endif
