#include "Tier1/CSemaphore.h"
using namespace cb;

CSemaphore::CSemaphore(u32 Available)
{
    atomic_set(&m_Available, Available);
}

void CSemaphore::Acquire(void)
{
    // Just spinlock...
    while (1)
    {
        if (atomic_read(&m_Available) > 0)
        {
            atomic_dec(&m_Available);
            break;
        }
    }
}

void CSemaphore::Release(void)
{
    atomic_inc(&m_Available);
}
