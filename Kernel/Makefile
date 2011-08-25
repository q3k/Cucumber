# I hate GNU Make
#
# Seriously.
#
# Fuck M4. I do this because it's probably the most portable way, and I don't
# want to write Yet Another Compile System because fuck that shit. I could
# use cmake but I don't know whether it is flexible enough. It probably is
# but whatever. I wrote this piece of shit below, let's just keep it that
# way. There are better way to do the hthings I do below, but who gives a
# shit.

default: emulate-nohdd

SHELL:=/bin/bash
ENV:=/usr/xdev/bin
TARGET:=x86_64-pc-elf
CC:=$(ENV)/$(TARGET)-gcc
CX:=$(ENV)/$(TARGET)-g++
AS:=nasm
LD:=$(ENV)/$(TARGET)-ld

# -O2 sets -foptimize-sibling-calls which breaks code...
CFLAGS:=-m64 -mcmodel=large -Wall -Werror -nostdlib -nostartfiles -nodefaultlibs -std=c99 -g
CFLAGS+=-I ./include -Wno-packed-bitfield-compat -O2 -fno-optimize-sibling-calls


CXFLAGS:= -Wall -Werror -nostdlib -fno-builtin -nostartfiles -I ./include
CXFLAGS+= -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector 
CXFLAGS+= -Wno-packed-bitfield-compat -O3
LFLAGS:=-nostdlib -nostartfiles -nodefaultlibs

.PHONY: all clean kernel.bin emulate hdd.img

obj/src/%.nao : src/%.asm
	@echo "[i] Assembling $*.asm..."
	@$(AS) -f elf64 -o obj/src/$*.nao src/$*.asm

obj/src/%.o : src/%.c
	@echo "[i] Compiling $*.c ..."
	@if [ -e obj/src/$*.o ] ; then rm obj/src/$*.o ; fi
	@mkdir -p obj/src/$*.o
	@rmdir obj/src/$*.o
	@$(CC) $(CFLAGS) -c src/$*.c -o obj/src/$*.o
	
obj/src/%.xo : src/%.cpp
	@echo "[i] Compiling $*.cpp ..."
	@if [ -e obj/src/$*.xo ] ; then rm obj/src/$*.xo ; fi
	@mkdir -p obj/src/$*.xo
	@rmdir obj/src/$*.xo
	@$(CX) $(CXFLAGS) -c src/$*.cpp -o obj/src/$*.xo

TIER0SRC := $(shell find src/Tier0 -mindepth 1 -maxdepth 3 -name "*.c")
TIER0SRC += $(shell find src/Tier0 -mindepth 1 -maxdepth 3 -name "*.asm")
TIER0OBJ := $(patsubst %.c,%.o,$(TIER0SRC))
TIER0OBJ := $(patsubst %.asm,%.nao,$(TIER0OBJ))
TIER0 := $(foreach i, $(TIER0OBJ), obj/$(i))
Tier0: $(TIER0)

#TIER1SRC := $(shell find src/Tier1 -mindepth 1 -maxdepth 3 -name "*.cpp")
#TIER1SRC += $(shell find src/Tier1 -mindepth 1 -maxdepth 3 -name "*.asm")
#TIER1OBJ := $(patsubst %.cpp,%.xo,$(TIER1SRC))
#TIER1OBJ := $(patsubst %.asm,%.nao,$(TIER1OBJ))
#TIER1 := $(foreach i, $(TIER1OBJ), obj/$(i))
#Tier1: $(TIER1)

kernel.bin: Tier0
	@echo "[i] Linking kernel.bin..."
	@$(LD) -T src/kernel.ld -o kernel.bin $(TIER0) $(TIER1)

hdd.img: kernel.bin
	@echo "[i] Creating HDD image..."
	@if [ -e hdd_temp.img ] ; then rm hdd_temp.img ; fi
	@dd if=/dev/zero of=hdd_temp.img bs=512 count=10240 2> /dev/null
	@mkfs.vfat hdd_temp.img
	@syslinux hdd_temp.img
	@mcopy -i hdd_temp.img /usr/lib/syslinux/mboot.c32 ::mboot.c32
	@mcopy -i hdd_temp.img kernel.bin ::kernel.bin
	@mcopy -i hdd_temp.img dst/syslinux.cfg ::syslinux.cfg
	@mv hdd_temp.img hdd.img

emulate-nohdd-debug: kernel.bin
	@echo "[i] Starting GDB..."
	@gnome-terminal -x /bin/bash -c "gdb"
	@echo "[i] Starting QEmu..."
	@qemu -kernel kernel.bin -S -gdb tcp::1234

emulate-nohdd: kernel.bin
	@echo "[i] Building loader..."
	@pushd ../Loader > /dev/null && $(MAKE) -f Makefile loader.bin && popd > /dev/null
	@echo "[i] Starting QEMU..."
	@qemu-system-x86_64 -kernel ../Loader/loader.bin -initrd kernel.bin

emulate: hdd.img
	@echo "[i] Starting QEmu..."
	@qemu -hda hdd.img

clean:
	@rm -Rf obj
	@if [ -e kernel.bin ] ; then rm kernel.bin ; fi
	@if [ -e hdd_temp.img ] ; then rm hdd_temp.img ; fi
	@if [ -e hdd.img ] ; then rm hdd.img; fi
