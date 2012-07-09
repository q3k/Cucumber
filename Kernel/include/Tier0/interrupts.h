#include "types.h"

#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

struct S_IDT_PTR {
    u16 Limit;
    u64 Base;
} __attribute__ ((packed));
typedef struct S_IDT_PTR T_IDT_PTR;

struct S_IDT_ENTRY {
    u16 OffsetLow;
    u16 Selector;
    u8 Zero1;
    u8 Type    : 4;
    u8 Zero2   : 1;
    u8 DPL     : 2;
    u8 Present : 1;

    u16 OffsetMiddle;
    u32 OffsetHigh;
    u32 Reserved;
} __attribute__ ((packed));
typedef struct S_IDT_ENTRY T_IDT_ENTRY;

enum E_INTERRUPTS_RING {
    E_INTERRUPTS_RING0 = 0,
    E_INTERRUPTS_RING1,
    E_INTERRUPTS_RING2,
    E_INTERRUPTS_RING3
};
typedef enum E_INTERRUPTS_RING T_INTERRUPTS_RING;

// This is a structure that allows easy access to a 62-byte ASM stub which
// calls a stdcall handler. Not the best way and not the shortest stub,
// but hey, it works.
struct S_ISR_STUB {
    u64 Code1;
    u64 Code2;
    u64 Code3;
    u16 Code4;
    u64 Handler;
    u64 Code5;
    u64 Code6;
    u64 Code7;
    u32 Code8;
} __attribute__ ((packed));
typedef struct S_ISR_STUB T_ISR_STUB;

typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax;
    u64 Error;
    u64 rip, cs, rflags, rsp, ss;
} __attribute__((packed)) T_ISR_REGISTERS_ERR;

typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax;
    u64 rip, cs, rflags, rsp, ss;
} __attribute ((packed)) T_ISR_REGISTERS;

u8 interrupts_init_idt(void);
void interrupts_setup_irq(u8 IRQ, void *Handler);
void interrupts_delete_isr(u8 Interrupt);
void interrupts_setup_isr_raw(u8 Interrupt, void *ASMHandler, \
                              T_INTERRUPTS_RING Ring);
void interrupts_setup_isr(u8 Interrupt, void *Handler, T_INTERRUPTS_RING Ring);
void interrupts_init_simple(void);
void interrupts_irq_finish(u8 IRQ);
void interrupts_lidt(void);
void interrupts_dump_idt_entry(u8 Interrupt);
void interrupts_dump_idt_ptr(void);

#endif

