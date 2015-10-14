align 8
bits 64

section .text

global ctask_getrip
ctask_getrip:
    pop rax
    jmp rax

extern kprint_hex
extern kputch

global ctask_spawnpoint
ctask_spawnpoint:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti
    iretq
