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
}

void CScheduler::AddTask(CTask *Task)
{
    kprintf("[i] Adding task %i (%x)\n", Task->GetPID(), Task);
    g_Scheduler.m_CurrentScheduler->AddTask(Task);
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
    // Add the Yield interrupt
    interrupts_setup_isr(CSCHEDULER_INTERRUPT_YIELD, (void*)YieldInterrupt, E_INTERRUPTS_RING0);
    // Add the Sleep interrupt
    interrupts_setup_isr(CSCHEDULER_INTERRUPT_SLEEP, (void*)SleepInterrupt, E_INTERRUPTS_RING0);
    // Add the Spawn interrupt
    interrupts_setup_isr(CSCHEDULER_INTERRUPT_SPAWN, (void*)SpawnInterrupt, E_INTERRUPTS_RING0);
    // Add the Exit interrupt
    interrupts_setup_isr(CSCHEDULER_INTERRUPT_EXIT, (void*)ExitInterrupt, E_INTERRUPTS_RING0);

    g_Scheduler.m_CurrentScheduler->Enable(true);
    CTimer::SetFastTimerHook(TimerTick);
}

static void NullEOI(void)
{

}

void CScheduler::Spawn(void(*lambda)(u64), u64 Data) {
    __asm__ __volatile__ ("mov %0, %%rax\n"
                          "mov %1, %%rbx\n"
                          "int $0x97"
                          ::"r"(lambda),"r"(Data):"rax","rbx");
}

void CScheduler::YieldInterrupt(T_ISR_REGISTERS Registers)
{
	m_NumTicks = 0;
	g_Scheduler.m_CurrentScheduler->NextTask(Registers, NullEOI);
}

void CScheduler::SleepInterrupt(T_ISR_REGISTERS Registers)
{
	m_NumTicks = 0;
	g_Scheduler.GetCurrentTask()->Sleep(Registers.rax);
	g_Scheduler.m_CurrentScheduler->NextTask(Registers, NullEOI);
}

void CScheduler::SpawnInterrupt(T_ISR_REGISTERS Registers)
{
    g_Scheduler.GetCurrentTask()->SetUserRegisters(Registers);
    g_Scheduler.GetCurrentTask()->Spawn(Registers.rax, Registers.rbx);
}

void CScheduler::ExitInterrupt(T_ISR_REGISTERS Registers)
{
    // TODO: actually remove the task
    g_Scheduler.GetCurrentTask()->m_Status = ETS_DISABLED;
	g_Scheduler.m_CurrentScheduler->NextTask(Registers, NullEOI);
}

void CScheduler::TimerTick(T_ISR_REGISTERS Registers, void (*EOI)(void))
{
	if (m_NumTicks > 20) {
		m_NumTicks = 0;
        if (!g_Scheduler.m_CurrentScheduler->InScheduler()) {
            g_Scheduler.m_CurrentScheduler->NextTask(Registers, EOI);
        }
	}
	m_NumTicks++;
}

void CScheduler::ResetTicks(void)
{
    m_NumTicks = 0;
}

void CScheduler::Sleep(u64 Ticks)
{
	__asm__ volatile ("mov %0, %%rax\n"
                      "int $0x98"
                      ::"r"(Ticks):"rax");
}

void CScheduler::Yield(void)
{
	__asm__ volatile ("int $0x99");
}

void CScheduler::DispatchAvailableSemaphore(CSemaphore *Semaphore)
{
    g_Scheduler.m_CurrentScheduler->SetSemaphoreAvailable(Semaphore);
}

