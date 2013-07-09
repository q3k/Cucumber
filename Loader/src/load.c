#include "types.h"
#include "io.h"
#include "context.h"
#include "elf.h"
#include "paging.h"
#include "multiboot.h"

extern u64 omg64;
extern u64 _end;
extern u64 _start;


static inline void cpuid(u32 code, u32 *a, u32 *d) {
    __asm__ volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
}

TMULTIBOOT_INFO *g_Multiboot;
T_LOAD_CONTEXT g_Context;
extern u64 GDT;
extern u32 ldrEntryLow;
extern u32 ldrEntryHigh;

u32 load(void *Multiboot, unsigned int Magic)
{
    clear();
    puts("Cucumber x86-64 loader...\n");

    g_Multiboot = Multiboot;
    if (Magic != 0x2BADB002)
    {
        puts("Error: not booted via Multiboot!\n");
        return 0;
    }

    u32 CPUID_A, CPUID_D;
    cpuid(0x80000001, &CPUID_A, &CPUID_D);
    u8 SupportFor64 = (CPUID_D & (1 << 29)) > 0;
    if (!SupportFor64)
    {
        puts("Error: You CPU does not support long mode!\n");
        return 0;
    }

    if (!(g_Multiboot->Flags & MULTIBOOT_INFO_MODS))
    {
        puts("Error: no 64-bit kernel loaded!\n");
        puts("  (did you forget the module line in GRUB?)\n");
        return 0;
    }

    if (!g_Multiboot->ModulesCount)
    {
        puts("Error: No kernel specified! Can't boot non-existant code, sorry!\n");
        return 0;
    }
    if (g_Multiboot->ModulesCount != 1)
    {
        puts("Error: just one module is enough. Don't load a ton of them.\n");
            return 0;
    }

    u32 KernelStart = g_Multiboot->Modules[0].ModuleStart;
    u32 KernelEnd = g_Multiboot->Modules[0].ModuleEnd;
    puts("Kernel is ");
    print_hex(KernelStart);
    puts("-");
    print_hex(KernelEnd);
    puts(".\n");

    struct elf_header *Header = (struct elf_header *)KernelStart;
    if (Header->Identification.Magic != 0x464C457F)
    {
        puts("Error: Module is not an ELF file!\n");
        return 0;
    }

    if (Header->Identification.Class != 2)
    {
        puts("Error: Module is not a 64-bit ELF file!\n");
        return 0;
    }

    if (!Header->Entry)
    {
        puts("Error: Kernel does not have entry point!\n");
        return 0;
    }

    puts("Entry point @");
    print_hex(Header->Entry);
    puts(".\n");

    if (Header->SectionHeaderEntrySize != sizeof(struct elf_section_header))
    {
        puts("Error: Weird section header entry size!\n");
        return 0;
    }

    struct elf_section_header *Sections = (struct elf_section_header *)((u32)Header + (u32)Header->SectionHeaderOffset);
    struct elf_section_header *StringSection = &Sections[Header->SectionEntryStrings];

    //u32 *Strings = (u32*)(ModulesAddress + (u32)StringSection->Offset);

    puts("Kernel ELF has 0x");
    print_hex(Header->NumSectionHeaderEntries);
    puts(" sections.\n");

    // Loop through ELF sections to find physical space occupied by them
    u64 StartPhysical = 0;
    u64 EndPhysical = 0;

    for (u16 i = 0; i < Header->NumSectionHeaderEntries; i++)
    {
        u64 PhysicalAddress = (u32)Header + Sections[i].Offset;
        u64 VirtualAddress = Sections[i].Address;

        if (VirtualAddress)
        {
            if (!StartPhysical)
                StartPhysical = PhysicalAddress;

            u64 EndAddress = StartPhysical + Sections[i].Size;
            if (EndAddress > EndPhysical)
                EndPhysical = EndAddress;
        }
    }

    u32 FreeSpaceStart = (u32)&_end;
    if (EndPhysical > FreeSpaceStart)
        FreeSpaceStart = EndPhysical;
    if (KernelEnd > FreeSpaceStart)
        FreeSpaceStart = KernelEnd;
    if (FreeSpaceStart % 0x1000)
        FreeSpaceStart = (FreeSpaceStart + 0x1000) & 0xFFFFF000;

    g_Context.KernelPhysicalStart = FreeSpaceStart;
    
    u32 KernelApproximateSize = FreeSpaceStart - StartPhysical;
    if (FreeSpaceStart + KernelApproximateSize > 0x00EFFFFF)
    {
        puts("Kernel will probably not fit in extended memory. Failing.\n");
        for(;;) {}
    }
    puts("Paging frames will be allocated from 0x"); print_hex(FreeSpaceStart); puts("\n");
    paging_setup(FreeSpaceStart);
    // allocate idnetity mapping of low & extended memory (up to 0x00EFFFFF)
    paging_map_address(0, 0, 0x00EFFFFF);
    // map the kernel sections
    for (u16 i = 0; i < Header->NumSectionHeaderEntries; i++)
    {
        u64 PhysicalAddress = (u32)Header + Sections[i].Offset;
        u64 VirtualAddress = Sections[i].Address;
        u64 Size = Sections[i].Size;
        s8* Name = (s8*)((u32)Header + (u32)StringSection->Offset + (u32)Sections[i].Name);

        if (VirtualAddress)
        {
            // allocate space for that section...
            u64 SizeAligned = Size;
            if (SizeAligned % 0x1000)
                SizeAligned = (SizeAligned + 0x1000) & 0xFFFFF000;
            u8 *Destination = paging_allocate(VirtualAddress, SizeAligned);

            puts(" -> "); puts(Name); puts(" at "); print_hex(VirtualAddress);
            if (!(Sections[i].Type & SHT_NOBITS))
            {
                // not .bss - copy data
                puts(" (copy from "); print_hex(PhysicalAddress); puts(")\n");
                u8 *Source = (u8 *)((u32)PhysicalAddress);
                for (u32 j = 0; j < Size; j++)
                    Destination[j] = Source[j];
            }
            else
            {
                for (u32 j = 0; j < Size; j++)
                    Destination[j] = 0;
                puts(" (zeroed out)\n");
            }
        }
    }
    g_Context.KernelPhysicalEnd = paging_get_last_frame();
    
    s8 *LoaderName = "Cucumber x86-64 loader";
    u8 i = 0;
    while (*LoaderName)
    {
        g_Context.LoaderName[i] = *LoaderName;
        i++;
        LoaderName++;
    }
    g_Context.VGATextModeUsed = 1;
    g_Context.MultibootUsed = 1;
    g_Context.MultibootHeader = (u32)Multiboot;
    g_Context.LoaderPhysicalStart = (u32)&_start;
    g_Context.LoaderPhysicalEnd = (u32)&_end;
    
    puts("Load context 0x");
    print_hex((u32)&g_Context);
    puts(".\n");

    __asm__ volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4");
    __asm__ volatile ("movl %%eax, %%cr3" :: "a" (paging_get_pml4()));
    puts("CR3 is now pointing to PML4 (0x");
    print_hex((u32)paging_get_pml4());
    puts(")\n");

    puts("Here it goes, enabling long mode...\n");

    __asm__ volatile(    "movl $0xc0000080, %%ecx;\n"
                        "rdmsr;\n"
                        "orl $0x100, %%eax;\n"
                        "wrmsr;\n"
                        "movl %%cr0, %%ebx;\n"
                        "bts $31, %%ebx;\n"
                        "movl %%ebx, %%cr0;":::"eax","ebx","ecx");

    puts("Now in 32-bit compability mode, jumping to the kernel...\n");
    io_update_load_context(&g_Context);
    // for (;;) {}
    ldrEntryLow = Header->Entry & 0xFFFFFFFF;
    ldrEntryHigh = Header->Entry >> 32;
    return 1;
}
