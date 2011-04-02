#include "types.h"

#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

struct S_IDT_PTR {
    u16 Limit;
    u32 Base;
} __attribute__ ((packed));
typedef struct S_IDT_PTR T_IDT_PTR;

struct S_IDT_ENTRY {
    u16 OffsetLow;
    u16 Selector;
    u8 Zero;
    u8 Type;
    u16 OffsetHigh;
} __attribute__ ((packed));
typedef struct S_IDT_ENTRY T_IDT_ENTRY;

enum E_INTERRUPTS_RING {
    E_INTERRUPTS_RING0 = 0,
    E_INTERRUPTS_RING1,
    E_INTERRUPTS_RING2,
    E_INTERRUPTS_RING3
};
typedef enum E_INTERRUPTS_RING T_INTERRUPTS_RING;

enum E_INTERRUPTS_CHIP {
    E_INTERRUPTS_CHIP_UNK,
    E_INTERRUPTS_CHIP_PIC,
    E_INTERRUPTS_CHIP_APIC
};
typedef enum E_INTERRUPTS_CHIP T_INTERRUPTS_CHIP;

// This is a structure that allows easy access to a 12-byte ASM stub which
// calls a stdcall handler. Not the best way and not the shortest stub,
// but hey, it works.
struct S_ISR_STUB {
    u16 Code1;
    u8 Code2;
    u32 Handler;
    u32 Code3;
    u8 Code4;
} __attribute__ ((packed));
typedef struct S_ISR_STUB T_ISR_STUB;

typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 Error;
    u32 eip, cs, eflags, useresp, ss;
} T_ISR_REGISTERS_ERR;

typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip, cs, eflags, useresp, ss;
} T_ISR_REGISTERS;

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

