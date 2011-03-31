global _start
extern kmain

; Multiboot-related constants
MODULEALIGN     equ 1 << 0
MEMINFO         equ 1 << 1
FLAGS           equ MODULEALIGN | MEMINFO
MAGIC           equ 0x1BADB002
CHECKSUM        equ -(MAGIC + FLAGS)

; Other constants
STACKSIZE equ 0x10000

; #############################################################################
; ############################## text segment #################################
; #############################################################################

section .setup
align 4

global g_before_gdt

; Multiboot header
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Actual entry point
_start:
    lgdt [falsegdt]
    mov cx, 0x10
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov ss, cx
g_before_gdt:
    jmp 0x08:higherhalf

section .text
align 4

higherhalf:    
    mov esp, kstack + STACKSIZE
    
    push eax
    push ebx
    
    call kmain

    cli

; If we are here, just loop - we're dead anyway
ohshit:
    hlt
    jmp ohshit

; #############################################################################
; ############################## setup segment ################################
; #############################################################################

section .setup

falsegdt:
    dw gdt_end - gdt - 1
    dd gdt

gdt:
    dd 0, 0
    db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40
    db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40
gdt_end:

; #############################################################################
; ############################### bss segment #################################
; #############################################################################

section .bss
align 4

; here be 64k stack
kstack:
    resb STACKSIZE
