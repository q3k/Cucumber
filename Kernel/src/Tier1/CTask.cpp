#include "Tier1/CTask.h"
using namespace cb;

extern "C" {
//    #include "Tier0/physical_alloc.h"
    #include "Tier0/paging.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/kstdlib.h"
    #include "Tier0/system.h"
};

#include "Tier1/CScheduler.h"
#include "Tier1/CTimer.h"

///extern CPageDirectory *g_KernelPageDirectory;
u32 CTaskNextPID = 0;

CTask::CTask(CKernelML4 &ML4) : m_ML4(ML4)
{
    m_Status = ETS_RUNNING;
    m_PID = CTaskNextPID++;
    m_Priority = ETP_NORMAL;
    m_Owner = 0;
    m_Ring = ETR_RING0;
}

CTask::~CTask(void)
{
}

void CTask::CopyStack(CTask *Other)
{
    CKernelML4 &OtherML4 = Other->GetML4();
    // Allocate new stack
    u64 StackSize = 1024*1024;
    u64 StackStart = (u64)kmalloc_aligned_physical(StackSize);
    m_ML4.Map(AREA_STACK_START, StackStart, StackSize);
    // Copy over from old stack
    for (u64 i = 0; i < (StackSize/0x1000); i++) {
        u64 Destination = m_ML4.Resolve(AREA_STACK_START + i*0x1000);
        u64 Source = OtherML4.Resolve(AREA_STACK_START + i*0x1000);
        kmemcpy((void *)Destination, (void *)Source, 0x1000);
    }
}

CTask *CTask::Fork(void) __attribute__ ((optnone))
{
    __asm__ volatile("cli");
    volatile u64 RSP, RBP;
 
    CTask *ParentTask = CScheduler::GetCurrentTask();
    CKernelML4 *ML4 = new CKernelML4();
    CTask *NewTask = new CTask(*ML4);
 
    __asm__ volatile("mov %%rsp, %0" : "=r"(RSP));
    __asm__ volatile("mov %%rbp, %0" : "=r"(RBP));
 
    NewTask->CopyStack(this);
    volatile u64 ForkPoint = ctask_getrip();
 
    if (CScheduler::GetCurrentTask() == ParentTask)
    {
        NewTask->m_RSP = RSP;
        NewTask->m_RBP = RBP;
        NewTask->m_RIP = ForkPoint;
        kprintf("[i] Forked: PID %i, RSP %x, RIP %x...\n", NewTask->m_PID,
                RSP, ForkPoint);
        CScheduler::AddTask(NewTask);
     
        __asm__ volatile("sti");
     
        return (CTask *)ParentTask;
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
    kprintf("d:%x s:%x b:%x i:%x\n",m_ML4.GetPhysical(),
                                      m_RSP, m_RBP, m_RIP);
}

void CTask::Yield(void)
{
    CScheduler::NextTask();
}

void CTask::WaitForSemaphore(T_SEMAPHORE *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
    __asm__ volatile ("sti");
 
    Yield();
}

void CTask::WaitForSemaphore(CSemaphore *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
    __asm__ volatile ("sti");
    Yield();
}

void CTask::Disable(void)
{
    __asm__ volatile ("cli");
    m_Status = ETS_DISABLED;
    __asm__ volatile ("sti");
    Yield();
}

void CTask::Enable(void)
{
    __asm__ volatile ("cli");
    m_Status = ETS_RUNNING;
    __asm__ volatile ("sti");
}

void CTask::Sleep(u64 Ticks)
{
    __asm__ volatile ("cli");
    m_Status = ETS_DISABLED;
    CTimer::Create(Ticks, 1, WakeUp, (u64)this);
    __asm__ volatile ("sti");
    while (m_Status == ETS_DISABLED)
    {
        Yield();
    }
}

bool CTask::WakeUp(u64 Extra)
{
    CTask *Task = (CTask*)Extra;
    Task->Enable();    
    return true;
}
