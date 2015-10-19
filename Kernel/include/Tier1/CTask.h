#ifndef __CTASK_H__
#define __CTASK_H__

#include "types.h"
#include "Tier1/CKernel.h"
#include "Tier1/CKernelML4.h"
#include "Tier1/CSemaphore.h"
#include "Tier0/semaphore.h"

extern "C" {
    u64 ctask_getrip(void);
    u64 ctask_spawnpoint(void);
}

namespace cb {
    enum ETaskPriority {
        ETP_IDLE,
        ETP_NORMAL,
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
            ETaskPriority m_Priority;
            ETaskRing m_Ring;
            volatile u64 m_PID;

            /// Context switching data
            // Registers from user program (iret et al)
            T_ISR_REGISTERS m_UserRegisters;
            bool m_UserRegistersSet;
            // Registers of the kernel stack/code
            u64 m_KernelRIP; u64 m_KernelRSP; u64 m_KernelRBP;
            // Physical address of the kernel stack to use when in interrupt
            u64 m_KernelStack;

            /// Memory data
            // Paging structure
            CKernelML4 &m_ML4;
            // User STACK segment
            u64 m_UserStackStart;
            // User STACK size
            u64 m_UserStackSize;

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
            inline bool GetUserRegisters(T_ISR_REGISTERS *Out)
            {
                if (!m_UserRegistersSet)
                    return false;
                *Out = m_UserRegisters;
                return true;
            }
            inline void SetUserRegisters(T_ISR_REGISTERS Registers)
            {
                m_UserRegistersSet = true;
                m_UserRegisters = Registers;
            }
            inline void GetKernelRegisters(u64 *RIP, u64 *RSP, u64 *RBP)
            {
                *RIP = m_KernelRIP;
                *RSP = m_KernelRSP;
                *RBP = m_KernelRBP;
            }
            inline void SetKernelRegisters(u64 RIP, u64 RSP, u64 RBP)
            {
                m_KernelRIP = RIP;
                m_KernelRSP = RSP;
                m_KernelRBP = RBP;
            }
            inline void SetPriority(ETaskPriority NewPriority)
            {
                m_Priority = NewPriority;
            }
            inline ETaskPriority GetPriority(void)
            {
                return m_Priority;
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
            // Run function in task stack
            void UseStack(void (*Function) (CTask *Task));
            
            inline ETaskStatus GetStatus(void) {
                return m_Status;
            }
            
            inline u64 GetStatusData(void) {
                return m_StatusData;
            }
    };
};

#endif
