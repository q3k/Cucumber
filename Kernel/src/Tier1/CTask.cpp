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
    m_UserRegistersSet = false;

    m_KernelStack = (u64)kmalloc_aligned_physical(0x1000);
    // Map LOWMEM (TODO: share this, and actually limit this to RAM size)
    m_ML4.Map(AREA_LOWMEM_START, 0x0, AREA_LOWMEM_SIZE);
    // Map the SCRATCH directory from Tier0
    m_ML4.SetDirectory(0xFFFFFFFF00000000, paging_get_scratch_directory());
    // Map the TEXT directory from Tier0
    m_ML4.SetDirectory(0xFFFFFFFF80000000, paging_get_text_directory());
    // Map the LAPIC
    m_ML4.Map(0xFEE00000, 0xFEE00000);

    // Allocate new stack
    m_UserStackSize = 1024 * 1024;
    m_UserStackStart = (u64)kmalloc_aligned_physical(m_UserStackSize);
    m_ML4.Map(AREA_STACK_START, m_UserStackStart, m_UserStackSize);
}

CTask::~CTask(void)
{
    // Deallocate everything here
}

void CTask::CopyStack(CTask *Other)
{
    CKernelML4 &OtherML4 = Other->GetML4();
    // Copy over from old stack
    for (u64 i = 0; i < (m_UserStackSize/0x1000); i++) {
        u64 Destination = m_ML4.Resolve(AREA_STACK_START + i*0x1000);
        u64 Source = OtherML4.Resolve(AREA_STACK_START + i*0x1000);
        kmemcpy((void *)Destination, (void *)Source, 0x1000);
    }
}

CTask *CTask::Spawn(u64 NewEntry, u64 Data)
{
    CKernelML4 *ML4 = new CKernelML4();
    CTask *Task = new CTask(*ML4);

    T_ISR_REGISTERS NewRegisters;
    if (!GetUserRegisters(&NewRegisters)) {
        PANIC("Tried to spawn task from parent with no user context!");
    }
    NewRegisters.rbp = AREA_STACK_START + m_UserStackSize;
    NewRegisters.rsp = AREA_STACK_START + m_UserStackSize;
    NewRegisters.rip = NewEntry;
    NewRegisters.rdi = Data;
    Task->SetUserRegisters(NewRegisters);
    Task->PrepareReturnStack();
    CScheduler::AddTask(Task);

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
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
}

void CTask::WaitForSemaphore(CSemaphore *Semaphore)
{
    m_Status = ETS_WAITING_FOR_SEMAPHORE;
    m_StatusData = m_ML4.Resolve((u64)Semaphore);
}

void CTask::Disable(void)
{
    m_Status = ETS_DISABLED;
}

void CTask::Enable(void)
{
    m_Status = ETS_RUNNING;
}

void CTask::Sleep(u64 Ticks)
{
    m_Status = ETS_DISABLED;
    CTimer::Create(Ticks, 1, WakeUp, (u64)this);
}

bool CTask::WakeUp(u64 Extra)
{
    CTask *Task = (CTask*)Extra;
    Task->Enable();    
    return true;
}

void CTask::UseStack(void (*Function) (CTask *Task))
{
    u64 Stack = AREA_STACK_START + m_UserStackSize;
    __asm__ __volatile__(
            "pushq %%rbp\n"
            "movq %%rsp, %%rax\n"

            "movq %0, %%rsp\n"
            "movq %0, %%rbp\n"
            "pushq %%rax\n"

            "pushq %2\n"
            "callq *%1\n"
            "addq $8, %%rsp\n"

            "popq %%rax\n"
            "movq %%rax, %%rsp\n"
            "popq %%rbp"
        :
        :"r"(Stack), "r"(Function), "r"(this)
        :"%rax"
    );
}
