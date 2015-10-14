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

    m_KernelStack = (u64)kmalloc_aligned_physical(0x1000);
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

CTask *CTask::Spawn(u64 NewEntry, u64 Data)
{
    __asm__ volatile("cli");
 
    CKernelML4 *ML4 = new CKernelML4();
    CTask *Task = new CTask(*ML4);

    // TODO: unhardcode this
    T_ISR_REGISTERS NewRegisters;
    // Copy from previous task, just to be sure everything is sane
    GetUserRegisters(&NewRegisters);
    NewRegisters.rbp = AREA_STACK_START + 1 * 1024 * 1024;
    NewRegisters.rsp = AREA_STACK_START + 1 * 1024 * 1024;
    NewRegisters.rip = NewEntry;
    NewRegisters.rdi = Data;
    Task->SetUserRegisters(NewRegisters);
    Task->PrepareReturnStack();

    CScheduler::AddTask(Task);

    __asm__ __volatile__("sti");
    return Task;
}

void CTask::PrepareReturnStack(void)
{
    kmemcpy((void *)m_KernelStack, &m_UserRegisters, sizeof(m_UserRegisters));
    m_KernelRIP = (u64)ctask_spawnpoint;
    m_KernelRSP = m_KernelStack;
    m_KernelRBP = m_KernelStack;
}

void CTask::Dump(void)
{
    kprintf("d:%x s:%x b:%x i:%x\n",m_ML4.GetPhysical(),
        m_UserRegisters.rsp, m_UserRegisters.rbp, m_UserRegisters.rip);
}

void CTask::WaitForSemaphore(T_SEMAPHORE *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
    __asm__ volatile ("sti");
}

void CTask::WaitForSemaphore(CSemaphore *Semaphore)
{
    __asm__ volatile ("cli");
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
    __asm__ volatile ("sti");
}

void CTask::Disable(void)
{
    __asm__ volatile ("cli");
    m_Status = ETS_DISABLED;
    __asm__ volatile ("sti");
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
}

bool CTask::WakeUp(u64 Extra)
{
    CTask *Task = (CTask*)Extra;
    Task->Enable();    
    return true;
}
