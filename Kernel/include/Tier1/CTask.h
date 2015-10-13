#ifndef __CTASK_H__
#define __CTASK_H__

#include "types.h"
#include "Tier1/CKernel.h"
#include "Tier1/CKernelML4.h"
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
            CKernelML4 &m_ML4;
            ETaskPriority m_Priority;
            ETaskRing m_Ring;
            volatile u64 m_PID;
            
            volatile u64 m_RSP, m_RIP, m_RBP;
            volatile ETaskStatus m_Status;
            volatile u64 m_StatusData;
        public:
            //CTask(bool User = 0, bool Empty = false);
            CTask(CKernelML4 &ML4);
            ~CTask(void);
            
            CKernelML4 &GetML4(void) volatile {
                return m_ML4;
            }
            //CPageDirectory *GetPageDirectory(void);
            
            // Equivalent of the POSIX fork() call.
            CTask *Fork(void);
            // Fork and run lambda
            template <typename F> CTask *Fork(F lambda) {
                CTask *NewTask = Fork();
                if (NewTask == this)
                    return NewTask;
                else
                {
                    lambda();
                    m_Status = ETS_DISABLED;
                    for (;;) {Yield();}
                    // Should never reach
                    return NewTask;
                }
            }
            
            inline u64 GetPID(void) { return m_PID; }
            inline u64 GetRSP(void) { return m_RSP; }
            inline u64 GetRIP(void) { return m_RIP; }
            inline u64 GetRBP(void) { return m_RBP; }
            
            /*inline u64 GetPageDirectoryPhysicalAddress(void)
            {
                return m_Directory->m_Directory->PhysicalAddress;
            }*/
            
            inline void SetRSP(u64 RSP) { m_RSP = RSP; }
            inline void SetRIP(u64 RIP) { m_RIP = RIP; }
            inline void SetRBP(u64 RBP) { m_RBP = RBP; }
            
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

            // Copy a stack from another Task
            void CopyStack(CTask *Other);
            
            inline ETaskStatus GetStatus(void) {
                return m_Status;
            }
            
            inline u64 GetStatusData(void) {
                return m_StatusData;
            }
    };
};

#endif
