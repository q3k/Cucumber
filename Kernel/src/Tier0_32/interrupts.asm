BITS 32
section .text

global interrupts_lidt
extern g_idt_ptr

interrupts_lidt:
    lidt [g_idt_ptr]
    ret

global interrupts_irq_sample
interrupts_irq_sample:
    mov eax, 0xB8000
    mov byte [eax], 0x45
    iret
