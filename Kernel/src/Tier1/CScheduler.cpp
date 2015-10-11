#include "Tier1/CScheduler.h"
using namespace cb;

#include "Tier1/CRoundRobinScheduler.h"
#include "Tier1/CTimer.h"
#include "Tier1/CTask.h"

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
	#include "Tier0/interrupts.h"
}

volatile CScheduler g_Scheduler;
u64 CScheduler::m_NumTicks = 0;

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

void CScheduler::PrioritizeTask(CTask *Task)
{
	g_Scheduler.m_CurrentScheduler->PrioritizeTask(Task);
}

void CScheduler::Enable(void)
{
    CTimer::SetFastTimerHook(TimerTick);

    // Add the Yield interrupt
    interrupts_setup_isr(CSCHEDULER_INTERRUPT_YIELD, (void*)Yield, E_INTERRUPTS_RING0);
}

void CScheduler::Yield(T_ISR_REGISTERS Registers)
{
	__asm__ volatile("cli");
	m_NumTicks = 0;
	g_Scheduler.m_CurrentScheduler->NextTask(Registers);
	__asm__ volatile("sti");
}

void CScheduler::TimerTick(T_ISR_REGISTERS Registers)
{
	if (m_NumTicks > 20)
	{
		m_NumTicks = 0;
		g_Scheduler.m_CurrentScheduler->NextTask(Registers);
	}
	m_NumTicks++;
}

void CScheduler::NextTask(void)
{
	//__asm__ volatile("pusha");
	__asm__ volatile ("int $0x99");
	//__asm__ volatile("popa");
}

void CScheduler::DispatchAvailableSemaphore(CSemaphore *Semaphore)
{
    g_Scheduler.m_CurrentScheduler->SetSemaphoreAvailable(Semaphore);
}

