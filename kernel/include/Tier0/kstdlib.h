#ifndef __KSTDLIB_H__
#define __KSTDLIB_H__

#include "types.h"

void *kmemcpy(void* Destination, const void *Source, u64 Count);
void *kmemset(void *Destination, u8 Value, u64 Count);
void *kmemsetw(void *Destination, u16 Value, u64 Count);

#endif
