#ifndef __KSTDLIB_H__
#define __KSTDLIB_H__

#include "types.h"

void *kmemcpy(void* Destination, const void *Source, u32 Count);
void *kmemset(void *Destination, u8 Value, u32 Count);
void *kmemsetw(void *Destination, u16 Value, u32 Count);

#endif
