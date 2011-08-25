extern load
extern puts
extern stdio_current_line
extern stdio_cur_x
extern stdio_cur_y
extern g_multiboot_header;
global _loader

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

_loader_gdt:
	call load

	mov ebx, eax

	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Did we get an entry point address?
	test ebx, ebx
	jz hang

	movzx edi, byte [stdio_current_line]
	movzx esi, byte [stdio_cur_x]
	movzx edx, byte [stdio_cur_y]
	mov ecx, dword [g_multiboot_header]
	
	; 64-bit, here we come!
	call 0x18:0xFF001000

hang:
   hlt
   jmp   hang

; #############################################################################
; ############################### bss segment #################################
; #############################################################################

section .bss
align 4

; here be 4k stack
stack:
    resb STACKSIZE
