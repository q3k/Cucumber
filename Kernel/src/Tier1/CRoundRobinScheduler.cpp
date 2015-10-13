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

    semaphore_init(&m_SchedulerLock);
}

void CRoundRobinScheduler::NextTask(T_ISR_REGISTERS Registers)
{
    volatile u64 NewRBP, NewRSP, NewRIP, ML4;
    
    if (m_TaskQueue.GetSize() < 1)
        PANIC("No tasks in queue!");
    
    // Fetch next task.
    m_iTaskQueuePosition++;
	if (m_iTaskQueuePosition >= m_TaskQueue.GetSize())
	    //Something happened - restart the queue
        m_iTaskQueuePosition = 0;
    CTask *NextTask = m_TaskQueue[m_iTaskQueuePosition];

    if (m_CurrentTask->GetPID() == NextTask->GetPID())
    {
        interrupts_irq_finish(0);
        return;
    }
    if (NextTask->GetStatus() == ETS_DISABLED)
    {
        interrupts_irq_finish(0);
        return;
    }
    
    //kprintf("switching from %X %X %X (%i to %i)\n", Registers.rip, Registers.rsp,
    //        Registers.rbp, m_CurrentTask->GetPID(), NextTask->GetPID());

    //kprintf("[i] %i -> %i (%x)\n", m_CurrentTask->GetPID(), NextTask->GetPID(), NextTask);
    
    // Read task details
    NewRBP = NextTask->GetRBP();
    NewRSP = NextTask->GetRSP();
    NewRIP = NextTask->GetRIP();
    if (!NewRIP || !NewRSP || !NewRBP)
    {
        //kprintf("null %X %X %X\n", NewRIP, NewRSP, NewRBP);
        interrupts_irq_finish(0);
        return;
    }
    
    // Save current task details
    m_CurrentTask->SetRBP(Registers.rbp);
    m_CurrentTask->SetRSP(Registers.rsp);
    
    // Return point
    volatile u64 ReturnPoint = Registers.rip;
    m_CurrentTask->SetRIP(ReturnPoint);

    // Switch to next task
    m_CurrentTask = NextTask;
    
    ML4 = NextTask->GetML4().GetPhysical();
    //kprintf("[i] I was told to jump to %X (%X %X); %X\n", NewRIP, NewRSP,
    //                                                      NewRBP, ML4);
    interrupts_irq_finish(0);
    
    __asm__ volatile("movq %1, %%rsp\n"
                     "movq %2, %%rbp\n"
                     "movq %3, %%cr3\n"
                     "movq %0, %%rcx\n"
                     "sti\n"
                     "jmp *%%rcx" ::
                        "r"(NewRIP),
                        "r"(NewRSP),
                        "r"(NewRBP),
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
