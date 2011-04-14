#include "Tier1/CRoundRobinScheduler.h"
using namespace cb;

extern "C" {
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/heap.h"
}

void CRoundRobinScheduler::Enable(bool Enabled)
{
    m_TaskQueuePosition = 0;
}

__attribute__((optimize("O0"))) void CRoundRobinScheduler::NextTask(void)
{
    __asm__ volatile("cli");
    volatile u32 NewEBP, NewESP, NewEIP, EBP, ESP, Directory;
    
    if (m_TaskQueue.GetSize() == 0)
        PANIC("No tasks in queue!");
    
    // Fetch next task.
    m_TaskQueuePosition++;
    if (m_TaskQueuePosition >= m_TaskQueue.GetSize())
        // Something happened - restart the queue
        m_TaskQueuePosition = 0;
    CTask *NextTask = m_TaskQueue[m_TaskQueuePosition];
    
    // Read task details    
    NewEBP = NextTask->GetEBP();
    NewESP = NextTask->GetESP();
    NewEIP = NextTask->GetEIP();
    
    if (!NewEIP || !NewESP || !NewEBP)
    {
        kprintf("[i] no wai\n");
        return;
    }
    
    // Save current task details
    __asm__ volatile("mov %%esp, %0" : "=r"(ESP));
    __asm__ volatile("mov %%ebp, %0" : "=r"(EBP));
    m_CurrentTask->SetEBP(EBP);
    m_CurrentTask->SetESP(ESP);
    
    // Return point
    volatile u32 ReturnPoint = ctask_geteip();
    //kprintf("return point %x\n", ReturnPoint);
    
    if (ReturnPoint == 0xFEEDFACE)
    {
        //We are in the next task already
        return;
    }
    
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
                     "movl $0xFEEDFACE, %%eax\n"
                     "jmp *%%ecx" ::
                        "r"(NewEIP),
                        "r"(NewESP),
                        "r"(NewEBP),
                        "r"(Directory));
}

void CRoundRobinScheduler::AddTask(CTask *Task)
{
    __asm__ volatile("cli");
    m_TaskQueue.Push(Task);
    __asm__ volatile("sti");
}

CTask *CRoundRobinScheduler::GetCurrentTask(void)
{
    return m_CurrentTask;
}
