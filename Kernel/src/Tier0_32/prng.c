#include "Tier0/prng.h"
#include "Tier0/kstdio.h"

u32 g_prng_previous = 13371337;

void kseed(u32 Seed)
{
    g_prng_previous = Seed;
}

u32 krand(void)
{
    g_prng_previous = (PRNG_A * g_prng_previous + PRNG_C) % PRNG_M;
    return g_prng_previous;
}
