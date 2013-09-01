#include "Tier0/elf.h"
#include "Tier0/kstdio.h"

u8 elf_open(TELF *elf, void *Address, u64 Size)
{
    elf->Address = Address;
    elf->Size = Size;

    TELFHeader *Header = (TELFHeader *)Address;
    if (Header->Identification.Magic != ELF_IDENT_MAGIC)
    {
        kprintf("ELF: bad magic (0x%x)\n", Header->Identification.Magic);
        return 1;
    }
    elf->Header = Header;
    elf->Sections = (TELFSectionHeader *)((u64)Header + (u64)Header->SectionHeaderOffset);
    elf->SectionCount = Header->NumSectionHeaderEntries;

    if (Header->SectionHeaderOffset + elf->SectionCount * sizeof(TELFSectionHeader) > elf->Size)
    {
        kprintf("ELF: invalid section count/offset.\n");
        return 1;
    }
    return 0;
}

s8* elf_section_get_name(TELF *elf, u64 SectionID)
{
    u64 StringSectionID = elf->Header->SectionEntryStrings;
    TELFSectionHeader* StringSection = &elf->Sections[StringSectionID];
    TELFSectionHeader* RequestedSection = &elf->Sections[SectionID];
    return (s8*)elf->Header + StringSection->Offset + RequestedSection->Name;
}

u64 elf_section_get_physical_address(TELF *elf, u64 SectionID)
{
    TELFSectionHeader* RequestedSection = &elf->Sections[SectionID];
    return (u64)elf->Header + RequestedSection->Offset;
}

u64 elf_section_get_virtual_address(TELF *elf, u64 SectionID)
{
    TELFSectionHeader* RequestedSection = &elf->Sections[SectionID];
    return (u64)RequestedSection->Address;
}

u64 elf_section_get_size(TELF *elf, u64 SectionID)
{
    TELFSectionHeader* RequestedSection = &elf->Sections[SectionID];
    return (u64)RequestedSection->Size;
}

u64 elf_section_has_bits(TELF *elf, u64 SectionID)
{
    TELFSectionHeader* RequestedSection = &elf->Sections[SectionID];
    return !(RequestedSection->Type & SHT_NOBITS);
}