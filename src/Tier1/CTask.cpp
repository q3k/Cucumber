#include "Tier1/CTask.h"
using namespace cb;

extern "C" {
    #include "Tier0/physical_alloc.h"
    #include "Tier0/paging.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
};

#include "Tier1/CScheduler.h"
#include "Tier1/CTimer.h"

extern CPageDirectory *g_KernelPageDirectory;
u32 CTaskNextPID = 0;

CTask::CTask(bool User, bool Empty)
{
    m_User = User;
    if (!Empty)
    {
        CreateDirectory();
        CreateStack();
        CopyKernelMemory();
        
        m_CreatedStack = true;
    }
    else
        m_CreatedStack = false;
    
    m_Status = ETS_RUNNING;
    m_PID = CTaskNextPID++;
    m_Priority = ETP_NORMAL;
    m_Owner = 0;
    m_Ring = (User ? ETR_RING3 : ETR_RING0);
}

CTask::~CTask(void)
{
    if (m_CreatedStack)
        physmem_free_page(m_StackStart / (4 * 1024));
}

void CTask::CreateStack(void)
{
    m_StackStart = physmem_allocate_page() * 4 * 1024;
    m_StackSize = TASK_MAP_STACK_SIZE;
    
    m_Directory->MapTable(TASK_MAP_STACK_START, m_StackStart, 1, 1);
}

void CTask::CreateDirectory(void)
{
    m_Directory = new CPageDirectory();
}

void CTask::CopyKernelMemory(void)
{
    //Copy all the kernel tables...    
    m_KernelStart = 0xC0000000;
    m_KernelSize = 0x20000000;
    
    u32 NumTables = m_KernelSize / (4 * 1024 * 1024);
    
    for (u32 i = 0; i < NumTables; i++)
        m_Directory->LinkTable(m_KernelStart + i * 4 * 1024 * 1024,
                               g_KernelPageDirectory);
    
    m_Directory->LinkTable(0x00000000, g_KernelPageDirectory);
    m_Directory->LinkTable(0x00400000, g_KernelPageDirectory);
    //m_Directory->LinkTable(0x00800000, g_KernelPageDirectory);
    //m_Directory->LinkTable(0x01000000, g_KernelPageDirectory);
}

void CTask::CopyStack(CTask *Source)
{
    CPageDirectory *SourcePD = Source->GetPageDirectory();
    SourcePD->CopyPage(TASK_MAP_STACK_START, m_Directory, m_User, 1);
}

CPageDirectory *CTask::GetPageDirectory(void)
{
    return m_Directory;
}

CTask *CTask::Fork(void)
{
    __asm__ volatile("cli");
    volatile u32 ESP, EBP;
    
    CTask *ParentTask = CScheduler::GetCurrentTask();
    CTask *NewTask = new CTask(m_User);
    NewTask->m_Owner = m_Owner;
    NewTask->m_Ring = m_Ring;
    if (m_User)
    {
        //TODO: Write code for userland
        PANIC("Cannot fork usermode program!");
    }
    
    __asm__ volatile("mov %%esp, %0" : "=r"(ESP));
    __asm__ volatile("mov %%ebp, %0" : "=r"(EBP));
    
    NewTask->CopyStack(this);
    volatile u32 ForkPoint = ctask_geteip();
    
    
    if (CScheduler::GetCurrentTask() == ParentTask)
    {
        NewTask->m_ESP = ESP;
        NewTask->m_EBP = EBP;
        NewTask->m_EIP = ForkPoint;
        kprintf("[i] Forked: TID %i, ESP %x, EBP %x, EIP %x...\n", NewTask->m_PID,
                ESP, EBP, ForkPoint);
        CScheduler::AddTask(NewTask);
        
        __asm__ volatile("sti");
        
        return ParentTask;
    }
    else
    {
        __asm__ volatile("sti");
        //for(;;){} 
        return NewTask;
    }
}

void CTask::Dump(void)
{
    kprintf("d:%x s:%x b:%x i:%x\n",m_Directory->m_Directory->PhysicalAddress,
                                      m_ESP, m_EBP, m_EIP);
}

void CTask::Yield(void)
{
    CScheduler::NextTask();
}

void CTask::WaitForSemaphore(T_SEMAPHORE *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_Directory->Translate((u32)Semaphore);
    
    Yield();
    __asm__ volatile ("sti");
}

void CTask::WaitForSemaphore(CSemaphore *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_Directory->Translate((u32)Semaphore);
    
    Yield();
    __asm__ volatile ("sti");
}

void CTask::Disable(void)
{
    __asm__ volatile ("cli");
    m_Status = ETS_DISABLED;
    Yield();
    __asm__ volatile ("sti");
}

void CTask::Enable(void)
{
    __asm__ volatile ("cli");
    m_Status = ETS_RUNNING;
    __asm__ volatile ("sti");
}

void CTask::Sleep(u32 Ticks)
{
    __asm__ volatile ("cli");
    m_Status = ETS_DISABLED;
    CTimer::Create(Ticks, 1, WakeUp, (u32)this);
    Yield();
    __asm__ volatile ("sti");
}

bool CTask::WakeUp(u32 Extra)
{
    CTask *Task = (CTask*)Extra;
    Task->Enable();    
    return true;
}
