#ifndef __TIER0_ANSI_SETJMP_H__
#define __TIER0_ANSI_SETJMP_H__

#include "Tier0/panic.h"

typedef u32 jmp_buf;

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int value);

#endif
