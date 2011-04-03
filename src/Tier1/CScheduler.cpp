#include "Tier1/CScheduler.h"
using namespace cb;

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
}

CScheduler g_Scheduler;

CScheduler::CScheduler(void)
{
    m_CurrentTask = 0;
    m_TaskQueueStart = 0;
    m_TaskQueueCurrent = 0;
}

void CScheduler::AddTask(CTask *Task)
{
    __asm__ volatile("cli");
    TTaskQueueNode *Current = g_Scheduler.m_TaskQueueStart;
    
    if (Current == 0)
    {
        g_Scheduler.m_TaskQueueCurrent = (TTaskQueueNode*)
                                        kmalloc(sizeof(TTaskQueueNode));
        g_Scheduler.m_TaskQueueCurrent->Task = Task;
        g_Scheduler.m_TaskQueueCurrent->Next = 0;
        g_Scheduler.m_TaskQueueStart = g_Scheduler.m_TaskQueueCurrent;
        g_Scheduler.m_CurrentTask = Task;
        return;
    }
        
    while (Current->Next != 0)
        Current = Current->Next;
    
    // We are now at the last node.
    Current->Next = (TTaskQueueNode*)kmalloc(sizeof(TTaskQueueNode));
    Current->Next->Task = Task;
    Current->Next->Next = 0;
    __asm__ volatile("sti");
}

void CScheduler::NextTask(void)
{   
    u32 NewEBP, NewESP, NewEIP, EBP, ESP, Directory;
    TTaskQueueNode *Next;
    
    if (g_Scheduler.m_TaskQueueStart == 0)
        PANIC("No tasks in queue!");
    
    if (g_Scheduler.m_TaskQueueCurrent == 0)
        PANIC("Current task is null!");
    
    // Return point
    volatile u32 ReturnPoint = ctask_geteip();
    
    if (ReturnPoint == 0xFEEDFACE)
    {
        //We are in the next task already
        __asm__ volatile("mov %%esp, %0" : "=r"(ESP));
        __asm__ volatile("mov %%ebp, %0" : "=r"(EBP));
        return;
    }
    
    // Fetch next task.
    if (g_Scheduler.m_TaskQueueCurrent->Next == 0)
        Next = g_Scheduler.m_TaskQueueStart;
    else
        Next = g_Scheduler.m_TaskQueueCurrent->Next;
    
    // Save current task details
    __asm__ volatile("mov %%esp, %0" : "=r"(ESP));
    __asm__ volatile("mov %%ebp, %0" : "=r"(EBP));
    g_Scheduler.m_CurrentTask->m_EBP = EBP;
    g_Scheduler.m_CurrentTask->m_ESP = ESP;
    g_Scheduler.m_CurrentTask->m_EIP = ReturnPoint;
    
    
    
    // Read task details    
    NewEBP = Next->Task->m_EBP;
    NewESP = Next->Task->m_ESP;
    NewEIP = Next->Task->m_EIP;
    
    if (!NewEIP || !NewESP || !NewEBP)
    {
        kprintf("[i] no wai\n");
        return;
    }
    
    // Switch to next task
    g_Scheduler.m_TaskQueueCurrent = Next;
    g_Scheduler.m_CurrentTask = Next->Task;
    
    Directory = Next->Task->m_Directory->m_Directory->PhysicalAddress;
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

CTask *CScheduler::GetCurrentTask(void)
{
    return g_Scheduler.m_CurrentTask;
}

void CScheduler::Enable(void)
{
    u32 Divisor = 0xFFFF;
    koutb(0x43, 0x36);
    u8 Low = (u8)(Divisor & 0xFF);
    u8 High = (u8)((Divisor >> 8) & 0xFF);
    koutb(0x40, Low);
    koutb(0x40, High);

    interrupts_setup_irq(0x00, (void*)CScheduler::TimerTick);
}

void CScheduler::TimerTick(T_ISR_REGISTERS R)
{
    NextTask();
}
