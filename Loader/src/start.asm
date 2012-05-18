extern load
extern puts
extern print_hex
extern g_Context;
global _loader
global omg64
global GDT
global ldrEntryLow
global ldrEntryHigh

; Multiboot-related constants
MODULEALIGN     equ 1 << 0
MEMINFO         equ 1 << 1
FLAGS           equ MODULEALIGN | MEMINFO
MAGIC           equ 0x1BADB002
CHECKSUM        equ -(MAGIC + FLAGS)

; Other constants
STACKSIZE equ 0x1000

; #############################################################################
; ############################## text segment #################################
; #############################################################################

section .text
align 4

; Multiboot header
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; GDTR
GDTR:
	dw 5*8-1
	dd GDT
	dd 0

; GDT
GDT:
	dw 0,0,0,0           								; null desciptor 		(0x00)
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x00 	; 32-bit code segment	(0x08)
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x00	; 32-bit data segment 	(0x10)
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 10101111b, 0x00 	; 64-bit code segment	(0x18)
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 10101111b, 0x00 	; 64-bit data segment	(0x20)

ldrEntryLow:
    dd 0
ldrEntryHigh:
    dd 0

str_back_in_asm:
	db "Back in assembler!", 0

; Actual entry point
_loader:
	mov esp, stack+STACKSIZE
	push eax
	push ebx

	lgdt [GDTR]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:_loader_gdt

hang:
    jmp hang

_loader_gdt:
	call load

    test eax, eax
    jz hang

    mov eax, [ldrEntryHigh]
    push eax
    mov eax, [ldrEntryLow]
    push eax

    call print_hex

	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov edi, g_Context
	; 64-bit, here we come!
    jmp 0x18:jmptohigh    

jmptohigh: ; pop rax, call rax
    db 0x58
    db 0xff
    db 0xd0

; #############################################################################
; ############################### bss segment #################################
; #############################################################################

section .bss
align 4

; here be 4k stack
stack:
    resb STACKSIZE
