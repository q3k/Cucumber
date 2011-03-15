#ifndef __PANIC_H__
#define __PANIC_H__

#include "types.h"

#define PANIC(m) kpanic(m, __FILE__, __LINE__)
#define ASSERT(m) kassert(m, __FILE__, __LINE__)

void kpanic(const s8 *Error, const s8 *File, u32 Line);
void kassert(u8 Value, const s8 *File, u32 Line);

#endif
