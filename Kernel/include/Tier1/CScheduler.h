#ifndef __CSCHEDULER_H__
#define __CSCHEDULER_H__

#include "types.h"

#include "Tier1/CTask.h"
#include "Tier1/IScheduler.h"
#include "Tier1/CSemaphore.h"

#define CSCHEDULER_INTERRUPT_YIELD 0x99
#define CSCHEDULER_INTERRUPT_SLEEP 0x98
#define CSCHEDULER_INTERRUPT_SPAWN 0x97
#define CSCHEDULER_INTERRUPT_EXIT 0x96

extern "C" {
    #include "Tier0/interrupts.h"
}

namespace cb {
    typedef struct STaskQueueNode {
        CTask *Task;
        struct STaskQueueNode *Next;
    } TTaskQueueNode;
    class CScheduler{
        private:
            IScheduler *m_CurrentScheduler;
            
            static void YieldInterrupt(T_ISR_REGISTERS Registers);
            static void SleepInterrupt(T_ISR_REGISTERS Registers);
            static void SpawnInterrupt(T_ISR_REGISTERS Registers);
            static void ExitInterrupt(T_ISR_REGISTERS Registers);

            static void TimerTick(T_ISR_REGISTERS Registers, void (*eoi)(void));

            static u64 m_NumTicks;
        public:
            CScheduler(void);
            static void Enable(void);
            static void AddTask(CTask *Task);
            static CTask *GetCurrentTask(void);
            static void ResetTicks(void);
            static void Yield();
            static void Sleep(u64 Ticks);
            static void PrioritizeTask(CTask *Task);
            static void DispatchAvailableSemaphore(CSemaphore *Semaphore);
            static void Spawn(void(*lambda)(u64), u64 Data) {
                __asm__ __volatile__ ("mov %0, %%rax\n"
                                      "mov %1, %%rbx\n"
                                      "int $0x97"
                                      ::"r"(lambda),"r"(Data):"rax","rbx");
            }
            static void Exit(void) {
                asm volatile("int $0x96");
            }
    };
};

#endif
