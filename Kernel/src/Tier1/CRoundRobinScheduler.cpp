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

static void IdleTaskFunction(u64 Data)
{
    // Spin forever in a halt
    for (;;) {
        asm volatile("hlt");
    }
}

void CRoundRobinScheduler::Enable(bool Enabled)
{
    m_iTaskQueuePosition = 0;
    m_PrioritizedTask = 0;
    m_InScheduler = false;

    // Hack: we want to fork an idle task, but our current task might not
    // yet have any saved registers... If so, yield now. The scheduler will
    // work fine for this one cycle without an idle task.
    T_ISR_REGISTERS Registers;
    if (!m_CurrentTask->GetUserRegisters(&Registers))
        asm volatile("int $0x99");

    // Create idle task
    m_IdleTask = m_CurrentTask->Spawn((u64)IdleTaskFunction, 0);
    m_IdleTask->SetPriority(ETP_IDLE);

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
    u64 Iterations = 0;
    CTask *NewTask = 0;
    while (NewTask == 0)
    {
        m_iTaskQueuePosition++;
        Iterations++;

        // Have we looped around the whole list? Run Idle task.
        if (Iterations > m_TaskQueue.GetSize()) {
            NewTask = m_IdleTask;
            break;
        }

        if (m_iTaskQueuePosition >= m_TaskQueue.GetSize())
            //Something happened - restart the queue
            m_iTaskQueuePosition = 0;

        CTask *Task = m_TaskQueue[m_iTaskQueuePosition];
        // Skip idle task
        if (Task->GetPriority() == ETP_IDLE)
            continue;
        // Skip disabled.
        if (Task->GetStatus() == ETS_DISABLED)
            continue;
        NewTask = m_TaskQueue[m_iTaskQueuePosition];
    }

    // Save current task details
    m_CurrentTask->SetUserRegisters(Registers);
    m_CurrentTask->PrepareReturnStack();
    // Switch to next task
    //kprintf("%i -> %i\n", m_CurrentTask->GetPID(), NewTask->GetPID());
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
