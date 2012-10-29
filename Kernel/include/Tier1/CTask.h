#ifndef __CTASK_H__
#define __CTASK_H__

#include "types.h"
#include "Tier1/CKernel.h"
//#include "Tier1/CPageDirectory.h"
#include "Tier1/CSemaphore.h"
#include "Tier0/semaphore.h"

/*#define TASK_MAP_CODE_START 0x10000000
#define TASK_MAP_CODE_SIZE 0x10000000

#define TASK_MAP_HEAP_START 0x20000000
#define TASK_MAP_HEAP_SIZE 0x40000000

#define TASK_MAP_STACK_START 0xA0000000
#define TASK_MAP_STACK_SIZE 0x10000000

#define TASK_MAP_KERNEL_START 0xC0000000
#define TASK_MAP_KERNEL_SIZE 0x20000000*/

extern "C" {
    u64 ctask_getrip(void);
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
        ETR_RING0,
        ETR_RING1,
        ETR_RING2,
        ETR_RING3
    };
    enum ETaskStatus {
        ETS_RUNNING,
        ETS_DISABLED,
        ETS_SLEEPING,
        ETS_WAITING_FOR_SEMAPHORE,
        ETS_WAITING_FOR_MESSAGE
    };
    class CPageDirectory;
    class CTask {
        friend class CKernel;
        friend class CScheduler;
        protected:
            void *m_Owner; //TODO: Replace me with a real type
            //CPageDirectory *m_Directory;            
            ETaskPriority m_Priority;
            ETaskRing m_Ring;
            bool m_User;
            volatile u64 m_PID;
            
            volatile u64 m_ESP, m_EIP, m_EBP;
            volatile ETaskStatus m_Status;
            volatile u64 m_StatusData;
            
            u64 m_HeapStart;
            u64 m_HeapSize;
            
            u64 m_StackStart;
            u64 m_StackSize;
            
            u64 m_ImageStart;
            u64 m_ImageSize;
            
            u64 m_KernelStart;
            u64 m_KernelSize;
            
            bool m_CreatedStack;
            
            void CreateStack(void);
            void CreateDirectory(void);
            
            void CopyKernelMemory(void);
            void CopyStack(CTask *Source);
        public:
            CTask(bool User = 0, bool Empty = false);
            ~CTask(void);
            
            //CPageDirectory *GetPageDirectory(void);
            
            // Equivalent of the POSIX fork() call.
            CTask *Fork(void);
            
            inline u64 GetPID(void) { return m_PID; }
            inline u64 GetESP(void) { return m_ESP; }
            inline u64 GetEIP(void) { return m_EIP; }
            inline u64 GetEBP(void) { return m_EBP; }
            
            /*inline u64 GetPageDirectoryPhysicalAddress(void)
            {
                return m_Directory->m_Directory->PhysicalAddress;
            }*/
            
            inline void SetESP(u64 ESP) { m_ESP = ESP; }
            inline void SetEIP(u64 EIP) { m_EIP = EIP; }
            inline void SetEBP(u64 EBP) { m_EBP = EBP; }
            
            /*inline void SetPageDirectory(CPageDirectory *Directory)
            {
                m_Directory = Directory;
            }*/
            
            void Dump(void);
            
            void Yield(void);
            void WaitForSemaphore(T_SEMAPHORE *Semaphore);
            void WaitForSemaphore(CSemaphore *Semaphore);
            
            // Makes the scheduler never give us a time slice
            void Disable(void);
            void Enable(void);
            
            // Like sleep(). Usually microseconds.
            void Sleep(u64 Ticks);
            
            // Used for waking up via a CTimer
            static bool WakeUp(u64 Extra); 
            
            inline ETaskStatus GetStatus(void)
            {
                return m_Status;
            }
            
            inline u64 GetStatusData(void)
            {
                return m_StatusData;
            }
    };
};

#endif
