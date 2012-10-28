#ifndef __TIER0_ANSI_SETJMP_H__
#define __TIER0_ANSI_SETJMP_H__

#include "Tier0/panic.h"
#include "Tier0/interrupts.h"

struct __jmp_buf {
    u64 rax; //   0
    u64 rbx; //   8
    u64 rcx; //  16
    u64 rdx; //  24
    u64 rsi; //  32
    u64 rdi; //  40
    u64 rsp; //  48
    u64 rbp; //  56
    u64 r8;  //  64
    u64 r9;  //  72
    u64 r10; //  80
    u64 r11; //  88
    u64 r12; //  96
    u64 r13; // 104
    u64 r14; // 112
    u64 r15; // 120
    u64 rip; // 128
} __attribute__((packed));

// POSIX sez:
//  jmp_buf must be an array type so that poor, poor programmers don't have
//  to use pointers, yet we can still return-by-argument. whaddafuck.
typedef struct __jmp_buf jmp_buf[1];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int value);

#endif
