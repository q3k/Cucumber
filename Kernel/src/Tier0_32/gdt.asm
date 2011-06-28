BITS 32
section .text

global gdt_flush
extern g_gdt_ptr

gdt_flush:
    lgdt [g_gdt_ptr]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:finish

finish:
    ret
