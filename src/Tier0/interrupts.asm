BITS 32
section .text

global interrupts_lidt
extern g_idt_ptr

interrupts_lidt:
    lidt [g_idt_ptr]
    ret
