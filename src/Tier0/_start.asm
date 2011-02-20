global _start
extern kmain

; Multiboot-related constants
MODULEALIGN     equ 1 << 0
MEMINFO         equ 1 << 1
FLAGS           equ MODULEALIGN | MEMINFO
MAGIC           equ 0x1BADB002
CHECKSUM        equ -(MAGIC + FLAGS)

; Other constants
STACKSIZE equ 0x4000

; ##############################################################################
; ############################# text segment ###################################
; ##############################################################################

section .text
align 4

; Multiboot header
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Actual entry point
_start:
    mov esp, kstack + STACKSIZE
    push eax
    push ebx
    
    call kmain

    cli

; If we are here, just loop - we're dead anyway
ohshit:
    hlt
    jmp ohshit


; ##############################################################################
; ################################ bss segment #################################
; ##############################################################################

section .bss
align 4

; here be 16k stack
kstack:
    resb STACKSIZE
