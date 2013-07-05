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
CC:=clang
CX:=clang++
AS:=nasm
LD:=ld

# -O2 sets -foptimize-sibling-calls which breaks code...
CFLAGS:= -mcmodel=large -Wall -Werror -nostdlib -ffreestanding -std=c99 -g
CFLAGS+=-I ./include -nostdinc -O2 -fno-optimize-sibling-calls -mcmodel=kernel
CFLAGS+= -mno-red-zone -ffreestanding -I ./include/Lua -I ./src/Lua -target x86_64-elf

CXFLAGS:= $(CFLAGS) -fno-rtti -fno-stack-protector -fno-exceptions -std=c++11

LFLAGS:=-nostdlib -nostartfiles -nodefaultlibs -m elf_x86_64 -nostdinc -z nodefaultlib -n

debug: CFLAGS += -DDEBUG -gdwarf-4 -fvar-tracking-assignments -O0
debug: CXFLAGS += -DDEBUG -gdwarf-4 -fvar-tracking-assignments -O0
debug: kernel.bin

.PHONY: all clean emulate hdd.img debug

obj/src/%.nao : src/%.asm
	@echo "[i] Assembling $*.asm..."
	@$(AS) -f elf64 -o obj/src/$*.nao src/$*.asm

obj/src/%.lo : src/%.lua
	@echo "[i] Packing $*.lua..."
	@contrib/luapack.py src/$*.lua obj/src/$*.lo

obj/src/%.o : src/%.c
	@echo "[i] Compiling $*.c ..."
	@if [ -e obj/src/$*.o ] ; then rm obj/src/$*.o ; fi
	@mkdir -p obj/src/$*.o
	@rmdir obj/src/$*.o
	@$(CC) $(CFLAGS) -c src/$*.c -o obj/src/$*.o

version-gen:
	@echo "[i] Generating revision file..."
	@contrib/rev-gen

include/Alentours/PCIDB.h:
	@echo "[i] Downloading pcidatabase.com header file..."
	@echo "[w] WARNING: Please check include/Alentours/PCIDB.h for possible rootkits and stuff."
	@curl http://www.pcidatabase.com/pci_c_header.php > include/Alentours/PCIDB.h
	python2 contrib/fix-pci-db.py


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

TIER1SRC := $(shell find src/Tier1 -mindepth 1 -maxdepth 3 -name "*.cpp")
TIER1SRC += $(shell find src/Tier1 -mindepth 1 -maxdepth 3 -name "*.asm")
TIER1OBJ := $(patsubst %.cpp,%.xo,$(TIER1SRC))
TIER1OBJ := $(patsubst %.asm,%.nao,$(TIER1OBJ))
TIER1 := $(foreach i, $(TIER1OBJ), obj/$(i))
Tier1: $(TIER1)

ALENTOURSSRC := $(shell find src/Alentours -mindepth 1 -maxdepth 3 -name "*.cpp")
ALENTOURSSRC += $(shell find src/Alentours -mindepth 1 -maxdepth 3 -name "*.asm")
ALENTOURSOBJ := $(patsubst %.cpp,%.xo,$(ALENTOURSSRC))
ALENTOURSOBJ := $(patsubst %.asm,%.nao,$(ALENTOURSOBJ))
ALENTOURS := $(foreach i, $(ALENTOURSOBJ), obj/$(i))
Alentours: $(ALENTOURS) include/Alentours/PCIDB.h

src/Lua/liblua.a:
	@echo "[i] Building Lua..."
	@make -C src/Lua

LUA := src/Lua/liblua.a
Lua: $(LUA)

kernel.bin: version-gen Tier0 Tier1 Lua Alentours
	@echo "[i] Linking kernel.bin..."
	@$(LD) $(LFLAGS) -T src/kernel.ld -o kernel.bin $(TIER0) $(TIER1) $(ALENTOURS) -Lsrc/Lua -llua
	@echo "[i] Creating debug symbols."
	@cp kernel.bin ksyms.elf
	#@strip --only-keep-debug ksyms.elf
	#@strip --strip-debug --strip-unneeded kernel.bin

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

emulate-nohdd-debug: debug
	@echo "[i] Starting GDB..."
	@echo -e "file kernel.bin\nsymbol-file ksyms.elf\ntarget remote localhost:1234\n" > gdbcommands
	xterm -e /bin/bash -c "sleep 1 && gdb -tui -x gdbcommands && rm gdbcommands" &
	@echo "[i] Starting QEmu..."
	@qemu-system-x86_64 --no-kvm -S -gdb tcp::1234 -d int -smp 4 -kernel ../Loader/loader.bin -initrd kernel.bin

emulate-nohdd: kernel.bin
	@echo "[i] Building loader..."
	@pushd ../Loader > /dev/null && $(MAKE) -f Makefile loader.bin && popd > /dev/null
	@echo "[i] Starting QEMU..."
	@qemu-system-x86_64 $(QEMU_FLAGS) --no-kvm -d int -smp 4 -kernel ../Loader/loader.bin -initrd kernel.bin

emulate: hdd.img
	@echo "[i] Starting QEmu..."
	@qemu -hda hdd.img

clean:
	@rm -Rf obj
	@make -C src/Lua clean
	@if [ -e kernel.bin ] ; then rm kernel.bin ; fi
	@if [ -e hdd_temp.img ] ; then rm hdd_temp.img ; fi
	@if [ -e hdd.img ] ; then rm hdd.img; fi
