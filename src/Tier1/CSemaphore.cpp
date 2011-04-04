#include "Tier1/CSemaphore.h"
using namespace cb;

CSemaphore::CSemaphore(u32 Available)
{
    m_Available = Available;
}

void CSemaphore::Acquire(void)
{
    // Just spinlock...
    while (1)
    {
        if (m_Available > 0)
        {
            m_Available--;
            break;
        }
    }
}

void CSemaphore::Release(void)
{
    m_Available++;
}
