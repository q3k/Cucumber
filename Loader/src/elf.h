#ifndef __ELF_H__
#define __ELF_H__

struct elf_ident {
    u32 Magic; // \x7FELF
    u8 Class;
    u8 Data;
    u8 Version;
    u8 Padding[9];
};

struct elf_header {
    struct elf_ident Identification;
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
};

#define SHT_PROGBITS 1
#define SHT_NOBITS 8

struct elf_section_header {
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
};

#endif