#ifndef __ELF_H__
#define __ELF_H__

#include "types.h"

#define ELF_IDENT_MAGIC 0x464c457f // \x7FELF
typedef struct elf_ident {
    u32 Magic; // \x7FELF
    u8 Class;
    u8 Data;
    u8 Version;
    u8 Padding[9];
} __attribute__((packed)) _elf_ident;

typedef struct {
    _elf_ident Identification;
    u16 Type;
    u16 Machine;
    u32 Version;
    u64 Entry;
    u64 ProgramHeaderOffset;
    u64 SectionHeaderOffset;
    u32 Flags;
    u16 HeaderSize;
    u16 ProgramHeaderEntrySize;
    u16 NumProgramHeaderEntries;
    u16 SectionHeaderEntrySize;
    u16 NumSectionHeaderEntries;
    u16 SectionEntryStrings;
} __attribute__((packed)) TELFHeader;

#define SHT_PROGBITS 1
#define SHT_NOBITS 8

typedef struct {
    u32 Name;
    u32 Type;
    u64 Flags;
    u64 Address;
    u64 Offset;
    u64 Size;
    u32 Link;
    u32 Info;
    u64 Alignment;
    u64 FixedSize;
} __attribute__((packed)) TELFSectionHeader;

typedef struct {
    void *Address;
    u64 Size;
    TELFHeader *Header;

    u64 SectionCount;
    TELFSectionHeader *Sections;
} TELF;

u8 elf_open(TELF *elf, void *Address, u64 Size);
s8* elf_section_get_name(TELF *elf, u64 Section);
u64 elf_section_get_physical_address(TELF *elf, u64 Section);
u64 elf_section_get_virtual_address(TELF *elf, u64 Section);
u64 elf_section_get_size(TELF *elf, u64 Section);
// Returns whether this section has actual bits to load (is data or text)
u64 elf_section_has_bits(TELF *elf, u64 SectionID);

#endif