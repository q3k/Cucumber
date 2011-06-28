#ifndef __PRNG_H__
#define __PRNG_H__

#include "types.h"

#define PRNG_C 12345
#define PRNG_A 1103515245
#define PRNG_M 0xFFFFFFFF

void kseed(u32 Seed);
u32 krand(void);

#endif
