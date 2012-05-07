BITS 64
section .text

;global interrupts_lidt
;extern g_Interrupts

;interrupts_lidt:
;    lidt [g_Interrupts]
;    ret

;global interrupts_irq_sample
;interrupts_irq_sample:
;    mov eax, 0xB8000
;    mov byte [eax], 0x45
;    iret

