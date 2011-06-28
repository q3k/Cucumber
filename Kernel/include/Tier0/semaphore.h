#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "types.h"
#include "Tier0/atomic_operations.h"

typedef T_ATOMIC T_SEMAPHORE;

static inline void semaphore_init(T_SEMAPHORE *Semaphore)
{
    atomic_set(Semaphore, 1);
}

static inline void semaphore_init_with_number(T_SEMAPHORE *Semaphore, u32 N)
{
    atomic_set(Semaphore, N);
}

static inline void semaphore_acquire(T_SEMAPHORE *Semaphore)
{
    // Spinlock again
    while(1)
    {
        if (atomic_read(Semaphore) > 0)
        {
            atomic_dec(Semaphore);
            break;
        }
    }
}

static inline void semaphore_release(T_SEMAPHORE *Semaphore)
{
    atomic_inc(Semaphore);
}

#endif
