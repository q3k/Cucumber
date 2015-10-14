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
    u64 ctask_spawnpoint(void);
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
            u64 m_KernelStack;

            // Registers from user program (iret et al)
            T_ISR_REGISTERS m_UserRegisters;
            // Registers of the kernel stack/code
            u64 m_KernelRIP; u64 m_KernelRSP; u64 m_KernelRBP;

            volatile ETaskStatus m_Status;
            volatile u64 m_StatusData;

        public:
            void PrepareReturnStack(void);
            //CTask(bool User = 0, bool Empty = false);
            CTask(CKernelML4 &ML4);
            ~CTask(void);
            
            CKernelML4 &GetML4(void) volatile {
                return m_ML4;
            }
            // Clone into new task
            CTask *Spawn(u64 NewEntry, u64 Data);
                        
            inline u64 GetPID(void) { return m_PID; }
            inline const T_ISR_REGISTERS *GetUserRegisters(void) { return &m_UserRegisters; }
            inline void GetUserRegisters(T_ISR_REGISTERS *Out) { *Out = m_UserRegisters; }
            inline void SetUserRegisters(T_ISR_REGISTERS Registers) { m_UserRegisters = Registers; }
            inline void GetKernelRegisters(u64 *RIP, u64 *RSP, u64 *RBP) {
                *RIP = m_KernelRIP;
                *RSP = m_KernelRSP;
                *RBP = m_KernelRBP;
            }
            inline void SetKernelRegisters(u64 RIP, u64 RSP, u64 RBP) {
                m_KernelRIP = RIP;
                m_KernelRSP = RSP;
                m_KernelRBP = RBP;
            }
           
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
