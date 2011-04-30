#include "Tier1/CScheduler.h"
using namespace cb;

#include "Tier1/CRoundRobinScheduler.h"

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
    u32 Divisor = 0xFFFFFFFF;
    koutb(0x43, 0x36);
    u8 Low = (u8)(Divisor & 0xFF);
    u8 High = (u8)((Divisor >> 8) & 0xFF);
    koutb(0x40, Low);
    koutb(0x40, High);

    interrupts_setup_irq(0x00, (void*)CScheduler::TimerTick);
}

__attribute__((optimize("O0"))) void CScheduler::TimerTick(T_ISR_REGISTERS R)
{
    __asm__ volatile("cli");
    g_Scheduler.m_CurrentScheduler->NextTask();
    interrupts_irq_finish(0);
    __asm__ volatile("sti");
}
