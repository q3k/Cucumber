#include "Tier1/CScheduler.h"
using namespace cb;

#include "Tier1/CRoundRobinScheduler.h"
#include "Tier1/CTimer.h"

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
}

volatile CScheduler g_Scheduler;

CScheduler::CScheduler(void)
{
    m_CurrentScheduler = new CRoundRobinScheduler();
    m_CurrentScheduler->Enable(true);
}

void CScheduler::AddTask(CTask *Task)
{
    __asm__ volatile("cli");
    kprintf("[i] Adding task %i (%x)\n", Task->GetPID(), Task);
    g_Scheduler.m_CurrentScheduler->AddTask(Task);
    __asm__ volatile("sti");
}

CTask *CScheduler::GetCurrentTask(void)
{
    return g_Scheduler.m_CurrentScheduler->GetCurrentTask();
}

void CScheduler::Enable(void)
{
    // 20ms quntum
    CTimer::Create(200, -1, TimerTick);
}

__attribute__((optimize("O0"))) bool CScheduler::TimerTick(u32 Extra)
{
    g_Scheduler.m_CurrentScheduler->NextTask();
    return true;
}

void CScheduler::NextTask(void)
{
    g_Scheduler.m_CurrentScheduler->NextTask();
}

void CScheduler::DispatchAvailableSemaphore(CSemaphore *Semaphore)
{
    g_Scheduler.m_CurrentScheduler->SetSemaphoreAvailable(Semaphore);
}
