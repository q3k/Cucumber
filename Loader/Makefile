default: emulate-nohdd

SHELL:=/usr/bin/env bash
CC:=clang
AS:=nasm
LD:=ld.lld

# -O2 sets -foptimize-sibling-calls which breaks code...
CFLAGS:=-Wall -nostdlib -std=c99 -g -ffreestanding
CFLAGS+=-I ./include -O1 -fno-optimize-sibling-calls
CFLAGS+= -fno-builtin -nostdinc -target i586-elf -nostdlib
CFLAGS+= -fno-stack-protector

LFLAGS:=-nostdlib -m elf_i386

.PHONY: all clean loader.bin emulate hdd.img

obj/src/%.nao : src/%.asm
	@echo "[i] Assembling $*.asm..."
	@$(AS) -f elf -o obj/src/$*.nao src/$*.asm

obj/src/%.o : src/%.c
	@echo "[i] Compiling $*.c ..."
	@if [ -e obj/src/$*.o ] ; then rm obj/src/$*.o ; fi
	@mkdir -p obj/src/$*.o
	@rmdir obj/src/$*.o
	@$(CC) $(CFLAGS) -c src/$*.c -o obj/src/$*.o

SRC := $(shell find src -mindepth 1 -maxdepth 3 -name "*.c")
SRC += $(shell find src -mindepth 1 -maxdepth 3 -name "*.asm")
OBJ := $(patsubst %.c,%.o,$(SRC))
OBJ := $(patsubst %.asm,%.nao,$(OBJ))
OBJ_ALL := $(foreach i, $(OBJ), obj/$(i))
Loader: $(OBJ_ALL)

loader.bin: Loader
	@echo "[i] Linking loader.bin..."
	@$(LD) $(LFLAGS) -T src/loader.ld -o loader.bin $(OBJ_ALL)

emulate-nohdd-debug: loader.bin
	@echo "[i] Starting GDB..."
	@gnome-terminal -x /bin/bash -c "gdb"
	@echo "[i] Starting QEmu..."
	@qemu-system-x86_64 -kernel loader.bin -S -gdb tcp::1234

emulate-nohdd: loader.bin
	@echo "[i] Starting QEMU..."
	@qemu-system-x86_64 -kernel loader.bin

clean:
	@rm -Rf obj
	@if [ -e loader.bin ] ; then rm loader.bin ; fi
	@if [ -e hdd_temp.img ] ; then rm hdd_temp.img ; fi
	@if [ -e hdd.img ] ; then rm hdd.img; fi
