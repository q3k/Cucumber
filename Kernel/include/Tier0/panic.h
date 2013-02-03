#ifndef __PANIC_H__
#define __PANIC_H__

#include "types.h"
#include "Tier0/interrupts.h"

#define PANIC(m) kpanic(m, __FILE__, __LINE__)
#define PANIC_EX(m, R) kpanic_ex(m, __FILE__, __LINE__, R)
#define PANIC_EX_HEX(m, R, h) kpanic_ex(m, 0, h, R)
#define ASSERT(m) kassert(m, __FILE__, __LINE__)

void kpanic_ex(const s8 *Error, const s8 *File, u64 Line, T_ISR_REGISTERS R);
void kpanic(const s8 *Error, const s8 *File, u32 Line);
void kassert(u8 Value, const s8 *File, u32 Line);

#endif
