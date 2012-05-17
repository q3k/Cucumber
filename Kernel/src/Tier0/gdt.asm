BITS 64
section .text

;global gdt_flush
;extern g_GDTPointer

;gdt_flush:
;    lgdt [g_GDTPointer]
;    mov ax, 0x20
;    mov ds, ax
;    mov es, ax
;    mov fs, ax
;    mov gs, ax
;    mov ss, ax
;
;    ret
