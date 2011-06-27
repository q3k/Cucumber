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
}

__attribute__((optimize("O0"))) void CRoundRobinScheduler::NextTask(u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 eip)
{
	// A wild magic value appears!
	esp += 12;

    volatile u32 NewEBP, NewESP, NewEIP, Directory;
    
    if (m_TaskQueue.GetSize() < 1)
        PANIC("No tasks in queue!");
    
    // Fetch next task.
    CTask *NextTask;
    /*if (m_PrioritizedTask != 0)
    {
    	NextTask = m_PrioritizedTask;
    	m_PrioritizedTask = 0;
    }
    else
	{*/
		do {
			m_iTaskQueuePosition++;

			if (m_iTaskQueuePosition >= m_TaskQueue.GetSize())
				// Something happened - restart the queue
				m_iTaskQueuePosition = 0;
			NextTask = m_TaskQueue[m_iTaskQueuePosition];
		}
		while (NextTask->GetStatus() != ETS_RUNNING);
    //}
    if (m_CurrentTask->GetPID() == NextTask->GetPID())
        return;
    
    //kprintf("switching from %x %x %x (%i to %i)\n", eip, esp, ebp, m_CurrentTask->GetPID(), NextTask->GetPID());

    //kprintf("[i] %i -> %i (%x)\n", m_CurrentTask->GetPID(), NextTask->GetPID(), NextTask);
    
    // Read task details
    NewEBP = NextTask->GetEBP();
    NewESP = NextTask->GetESP();
    NewEIP = NextTask->GetEIP();
    if (!NewEIP || !NewESP || !NewEBP)
    {
        //kprintf("null %x %x %x\n", NewEIP, NewESP, NewEBP);
        return;
    }
    
    // Save current task details
    m_CurrentTask->SetEBP(ebp);
    m_CurrentTask->SetESP(esp);
    
    // Return point
    volatile u32 ReturnPoint = eip;
    
    m_CurrentTask->SetEIP(ReturnPoint);
    // Switch to next task
    m_CurrentTask = NextTask;
    
    Directory = NextTask->GetPageDirectoryPhysicalAddress();
    //kprintf("[i] I was told to jump to %x (%x %x); %x\n", NewEIP, NewESP,
    //                                                      NewEBP, Directory);
    //for(;;){}
    interrupts_irq_finish(0);
    
    __asm__ volatile("movl %1, %%esp\n"
                     "movl %2, %%ebp\n"
                     "movl %3, %%cr3\n"
                     "movl %0, %%ecx\n"
                     "jmp *%%ecx" ::
                        "r"(NewEIP),
                        "r"(NewESP),
                        "r"(NewEBP),
                        "r"(Directory));
}

void CRoundRobinScheduler::AddTask(CTask *Task)
{
    m_TaskQueue.Push(Task);
    if (m_TaskQueue.GetSize() == 1)
        m_CurrentTask = Task;
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
    u32 Physical = GetCurrentTask()->GetPageDirectory()->
                                                      Translate((u32)Semaphore);
    for (u32 i = 0; i < m_TaskQueue.GetSize(); i++)
    {
        CTask *Task = m_TaskQueue[i];
        if (Task->GetStatus() == ETS_WAITING_FOR_SEMAPHORE &&
            Task->GetStatusData() == Physical)
        {
            Task->Enable();
        }
    }
}
