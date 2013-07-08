#include "types.h"
#include "io.h"
#include "context.h"
#include "elf.h"

extern u64 omg64;
extern u64 _end;
extern u64 _start;


static inline void cpuid(u32 code, u32 *a, u32 *d) {
    __asm__ volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
}


// A simple PAE paging structure so we can jump into 64-bit.
// This will be replaced later on by the kernel code.

u64 pml4[512] __attribute__((aligned(0x1000)));

u64 page_dir_ptr_tab_low[512] __attribute__((aligned(0x1000)));
u64 page_dir_low[512] __attribute__((aligned(0x1000)));
u64 page_tab_low[512] __attribute__((aligned(0x1000)));

u64 page_dir_ptr_tab_high[512] __attribute__((aligned(0x1000)));
u64 page_dir_high[512] __attribute__((aligned(0x1000)));
u64 page_tab_high[512] __attribute__((aligned(0x1000)));

#define GET_PML4_ENTRY(x) (((u64)x >> 39) & 0x1FF)
#define GET_PDP_ENTRY(x) (((u64)x >> 30) & 0x1FF)
#define GET_DIR_ENTRY(x) (((u64)x >> 21) & 0x1FF)
#define GET_TAB_ENTRY(x) (((u64)x >> 12) & 0x1FF)
#define GET_OFFSET(x) (x & 0xFFF)

u32 create_ia32e_paging(u64 KernelPhysicalStart, u64 KernelVirtualStart, u64 KernelSize)
{
    puts("Clearing paging structures...\n");

    for (u16 i = 0; i < 512; i++)
    {
        pml4[i] = 0;
        page_dir_ptr_tab_low[i] = 0;
        page_dir_low[i] = 0;
        page_tab_low[i] = 0;
        page_dir_ptr_tab_high[i] = 0;
        page_dir_high[i] = 0;
        page_tab_high[i] = 0;
    }

    puts("Setting up identity paging for first 2MiB...\n");
    pml4[GET_PML4_ENTRY(0)] = (u32)page_dir_ptr_tab_low | 3;
    page_dir_ptr_tab_low[GET_PDP_ENTRY(0)] = (u32)page_dir_low | 3;
    page_dir_low[GET_DIR_ENTRY(0)] = (u32)page_tab_low | 3;

    u64 Address = 0;
    for (u16 i = 0; i < 512; i++)
    {
        page_tab_low[i] = Address | 3;
        Address += 0x1000;
    }

    puts("Setting up paging for the kernel...\n");
    u16 NumPages = KernelSize / 0x1000;
    puts("  (0x");
    print_hex(NumPages);
    puts(" pages)\n");

    if (NumPages > 512)
    {
        puts("Error: Kernel size > 2MiB not implemented!");
        return 1;
    }
    puts("Kernel PML4: "); print_hex(GET_PML4_ENTRY(KernelVirtualStart)); puts("\n");
    puts("Kernel  PDP: "); print_hex(GET_PDP_ENTRY(KernelVirtualStart)); puts("\n");
    puts("Kernel  DIR: "); print_hex(GET_DIR_ENTRY(KernelVirtualStart)); puts("\n");

    if (GET_PML4_ENTRY(KernelVirtualStart) != 0)
    {
        // We're NOT mapping the same PML4 entry as for identity mapping...
        puts("Different PML4...\n");
        pml4[GET_PML4_ENTRY(KernelVirtualStart)] = (u32)page_dir_ptr_tab_high | 3;
        puts("Setting pml4["); print_hex_32(GET_PML4_ENTRY(KernelVirtualStart)); puts("] to "); print_hex_32((u32)page_dir_ptr_tab_high | 3); puts(".\n");
        page_dir_ptr_tab_high[GET_PDP_ENTRY(KernelVirtualStart)] = (u32)page_dir_high | 3;
        puts("Setting pdp["); print_hex_32(GET_PDP_ENTRY(KernelVirtualStart)); puts("] to "); print_hex_32((u32)page_dir_high | 3); puts(".\n");
        page_dir_high[GET_DIR_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
        puts("Setting dir["); print_hex_32(GET_DIR_ENTRY(KernelVirtualStart)); puts("] to "); print_hex_32((u32)page_tab_high | 3); puts(".\n");
    }
    else if (GET_PDP_ENTRY(KernelVirtualStart) != 0)
    {
        // We're NOT mapping the same page directory pointer table entry as for identity paging...
        puts("Different PDPT... (");
        print_hex(GET_PDP_ENTRY(KernelVirtualStart));
        puts(")\n");
        page_dir_ptr_tab_low[GET_PDP_ENTRY(KernelVirtualStart)] = (u32)page_dir_high | 3;
        page_dir_high[GET_DIR_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
    }
    else if (GET_DIR_ENTRY(KernelVirtualStart) != 0)
    {
        // We're NOT mapping the same page directory entry as for identity paging...
        puts("Different DIR...\n");
        page_dir_low[GET_DIR_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
    }
    else
    {
        puts("Error: kernel overlaps 2MiB identity paging!\n");
        return 1;
    }

    Address = KernelPhysicalStart;
    for (u16 i = GET_TAB_ENTRY(KernelVirtualStart); i < GET_TAB_ENTRY(KernelVirtualStart) + 512; i++)
    {
        page_tab_high[i] = Address | 3;
        
        Address += 0x1000;
    }
    return 0;
}

u64 g_multiboot_header;
T_LOAD_CONTEXT g_Context;
extern u64 GDT;
extern u32 ldrEntryLow;
extern u32 ldrEntryHigh;

u32 load(void *Multiboot, unsigned int Magic)
{
    clear();
    puts("Cucumber x86-64 loader...\n");
    
    puts("GDT: \n");
    for (u32 i = 0; i < 5; i++)
    {
        puts("  ");
        print_hex(*(&GDT + i));
        puts("\n");
    }

    g_multiboot_header = (u32)Multiboot;


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

    u32 Flags = *((u32*)Multiboot);

    u8 ModulesPresent = (Flags & (1 << 3)) > 0;

    if (!ModulesPresent)
    {
        puts("Error: no 64-bit kernel loaded!\n");
        puts("  (did you forget the module line in GRUB?)\n");
        return 0;
    }

    u32 ModulesCount = *((u32*)Multiboot + 5);
    u32 ModulesAddress = *((u32*)Multiboot + 6);
    
    if (ModulesCount == 0)
    {
        puts("Error: No kernel specified! Can't boot non-existant code, sorry!\n");
        return 0;
    }

    if (ModulesCount != 1)
    {
        puts("Error: just one module is enough. Don't load a ton of them.\n");
            return 0;
    }

    puts("Kernel is @");
    print_hex(ModulesAddress);
    puts(".\n");

    struct elf_header *Header = *((struct elf_header **)ModulesAddress);
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

    puts("0x");
    print_hex(Header->NumSectionHeaderEntries);
    puts(" ELF sections.\n");

    u64 ContinuityTest = 0;
    u64 StartPhysical = 0;
    u64 StartVirtual = 0;
    u64 Size = 0;

    for (u16 i = 0; i < Header->NumSectionHeaderEntries; i++)
    {
        s8* Name = (s8*)((u32)Header + (u32)StringSection->Offset + (u32)Sections[i].Name);
        u64 PhysicalAddress = (u32)Header + Sections[i].Offset;
        u64 VirtualAddress = Sections[i].Address;

        if (VirtualAddress)
        {
            if (!StartVirtual)
                StartVirtual = VirtualAddress;

            if (!StartPhysical)
                StartPhysical = PhysicalAddress;

            puts("-> Section ");
            puts(Name);
            puts(", 0x");
            print_hex(PhysicalAddress);
            puts(" will be located at 0x");
            print_hex(VirtualAddress);
            puts(".\n");

            if (ContinuityTest && VirtualAddress != ContinuityTest)
            {
                puts("Error: kernel is not continuous!\n");
                puts("Previous section ended at 0x"); print_hex(ContinuityTest); puts("\n");
                return 0;
            }

            ContinuityTest = VirtualAddress + Sections[i].Size;
            Size += Sections[i].Size;
        }
    }

    puts("\nPaging setup:\n 0x");
    print_hex(StartVirtual);
    puts(" => 0x");
    print_hex(StartPhysical);
    puts("\n  (0x");
    print_hex(Size);
    puts(" bytes)\n");
    
    g_Context.KernelPhysicalStart = StartPhysical;
    g_Context.KernelPhysicalEnd = StartPhysical + Size;
    
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

    if (create_ia32e_paging(StartPhysical, StartVirtual, Size))
    {
        puts("Could not create paging structure, for some reason... Failing.\n");
        for (;;) {}
    }
    __asm__ volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4");
    __asm__ volatile ("movl %%eax, %%cr3" :: "a" (pml4));
    puts("CR3 is now pointing to PML4 (0x");
    print_hex((u32)pml4);
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
    update_load_context(&g_Context);

    ldrEntryLow = Header->Entry & 0xFFFFFFFF;
    ldrEntryHigh = Header->Entry >> 32;
    return 1;
}
