#include "Tier1/CTask.h"
#include "Tier1/CTimer.h"
#include "Tier1/CScheduler.h"
#include "Tier1/CRoundRobinScheduler.h"
using namespace cb;

extern "C" {
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/heap.h"
}

void CRoundRobinScheduler::Enable(bool Enabled)
{
    m_iTaskQueuePosition = 0;
    m_PrioritizedTask = 0;
    m_InScheduler = false;

    semaphore_init(&m_SchedulerLock);
}

void CRoundRobinScheduler::NextTask(T_ISR_REGISTERS Registers, void (*EOI)(void))
{
    m_InScheduler = true;
    volatile u64 ML4;
    
    if (m_TaskQueue.GetSize() < 1)
        PANIC("No tasks in queue!");

    EOI();
    // Fetch next task.
    CTask *NewTask = 0;

    __asm__ volatile("sti");
    while (NewTask == 0)
    {
        m_iTaskQueuePosition++;
	    if (m_iTaskQueuePosition >= m_TaskQueue.GetSize())
	        //Something happened - restart the queue
            m_iTaskQueuePosition = 0;

        // skip disabled
        if (m_TaskQueue[m_iTaskQueuePosition]->GetStatus() == ETS_DISABLED)
            continue;

        NewTask = m_TaskQueue[m_iTaskQueuePosition];
    }
    __asm__ volatile("cli");

    // Save current task details
    m_CurrentTask->SetUserRegisters(Registers);
    m_CurrentTask->PrepareReturnStack();
    // Switch to next task
    m_CurrentTask = NewTask;
    
    ML4 = m_CurrentTask->GetML4().GetPhysical();
    u64 RIP, RSP, RBP;
    m_CurrentTask->GetKernelRegisters(&RIP, &RSP, &RBP);

    m_InScheduler = false;
    CScheduler::ResetTicks();
    EOI();

    __asm__ volatile("movq %1, %%rsp\n"
                     "movq %2, %%rbp\n"
                     "movq %3, %%cr3\n"
                     "movq %0, %%rcx\n"
                     "sti\n"
                     "jmp *%%rcx" ::
                        "r"(RIP),
                        "r"(RSP),
                        "r"(RBP),
                        "r"(ML4));
}

void CRoundRobinScheduler::AddTask(CTask *Task)
{
    semaphore_acquire(&m_SchedulerLock);
    m_TaskQueue.Push(Task);
    if (m_TaskQueue.GetSize() == 1)
        m_CurrentTask = Task;
    semaphore_release(&m_SchedulerLock);
}

CTask *CRoundRobinScheduler::GetCurrentTask(void)
{
    return m_CurrentTask;
}

void CRoundRobinScheduler::PrioritizeTask(CTask *Task)
{
	m_PrioritizedTask = Task;
}

void CRoundRobinScheduler::SetSemaphoreAvailable(CSemaphore *Semaphore)
{
    u64 Physical = GetCurrentTask()->GetML4().Resolve((u64)Semaphore);
    for (u64 i = 0; i < m_TaskQueue.GetSize(); i++)
    {
        CTask *Task = m_TaskQueue[i];
        if (Task->GetStatus() == ETS_WAITING_FOR_SEMAPHORE &&
            Task->GetStatusData() == Physical)
        {
            Task->Enable();
        }
    }
}
