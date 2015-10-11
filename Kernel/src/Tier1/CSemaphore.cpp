#include "Tier1/CSemaphore.h"
#include "Tier1/CScheduler.h"
using namespace cb;

CSemaphore::CSemaphore(u64 Available)
{
    atomic_set(&m_Available, Available);
}

void CSemaphore::Acquire(void)
{
    __asm__ volatile("cli");
    if (atomic_read(&m_Available) > 0)
    {
        atomic_dec(&m_Available);
        return;
    }
    else
    {
        // Tell the scheduler we are waiting for the semaphore
        //CScheduler::GetCurrentTask()->WaitForSemaphore(this);
        
        // If we are here, try to acquire the semaphore again.
        Acquire();
    }
    __asm__ volatile("sti");
}

void CSemaphore::Release(void)
{
    __asm__ volatile("cli");
    atomic_inc(&m_Available);
    
    // If the resource is now free, tell the scheduler
    if (atomic_read(&m_Available) > 0)
        CScheduler::DispatchAvailableSemaphore(this);
    __asm__ volatile("sti");
}
