#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"
#include "Tier0/system.h"
#include "Tier0/physmem.h"
#include "Tier0/elf.h"
#include "types.h"

T_PAGING_ML4 *paging_get_ml4(void)
{
	u64 Address;
	__asm__ volatile("mov %%cr3, %0\n" : "=r"(Address));
	return (T_PAGING_ML4*)Address;
}

void paging_set_ml4(u64 ML4Physical)
{
    __asm volatile ( "mov %%rax, %%cr3\n" :: "a" (ML4Physical));
}

void _zero_paging_structure(void *Structure)
{
    for (unsigned i = 0; i < 512; i++)
        ((u64 *)Structure)[i] = 0;
}


struct {
    T_PAGING_ML4 *ML4;
    u8 FullIdentityMapping;
} g_KernelPaging;

// allocate a page frame and make sure that it is accessible before
// our main paging is running - check whether it fits the extended
// memory area (up to 0xEFFFFF), as this is the area the bootloader
// identity mapped up to from 0x0.
void *_early_alloc(void)
{
    u64 Address = physmem_allocate_physical();
    if (!g_KernelPaging.FullIdentityMapping)
        ASSERT(Address < 0x00EFFFFF - 0x1000);
    return (void*)Address;
}

u8 _paging_resolve(u64 Virtual, u64 *PhysicalOut)
{
    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);
    u64 DIRI = PAGING_GET_DIR_INDEX(Virtual);
    u64 TABI = PAGING_GET_TAB_INDEX(Virtual);

    T_PAGING_ML4_ENTRY *ML4E = &g_KernelPaging.ML4->Entries[PML4I];
    if (!ML4E->Present)
        return -1;
    T_PAGING_PDP *PDP = (T_PAGING_PDP *)(ML4E->Physical << 12);
    T_PAGING_PDP_ENTRY *PDPE = &PDP->Entries[PDPI];
    if (!PDPE->Present)
        return -2;
    T_PAGING_DIR *Dir = (T_PAGING_DIR *)(PDPE->Physical << 12);
    T_PAGING_DIR_ENTRY *DIRE = &Dir->Entries[DIRI];
    if (!DIRE->Present)
        return -3;
    T_PAGING_TAB *Tab = (T_PAGING_TAB *)(DIRE->Physical << 12);
    T_PAGING_TAB_ENTRY *TABE = &Tab->Entries[TABI];
    if (!TABE->Present)
        return -4;

    if (PhysicalOut)
        *PhysicalOut = TABE->Physical << 12;

    return 0;
}

u64 paging_get_text_directory(void)
{
    u64 Virtual = 0xFFFFFFFF80000000;
    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);

    T_PAGING_ML4_ENTRY *ML4E = &g_KernelPaging.ML4->Entries[PML4I];
    if (!ML4E->Present)
        PANIC("Cannot get text ML4E!");
    T_PAGING_PDP *PDP = (T_PAGING_PDP *)(ML4E->Physical << 12);
    T_PAGING_PDP_ENTRY *PDPE = &PDP->Entries[PDPI];
    if (!PDPE->Present)
        PANIC("Cannot get text PDPE!");
    return PDPE->Physical << 12;
}

u64 paging_get_scratch_directory(void)
{
    u64 Virtual = 0xFFFFFFFF00000000;
    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);

    T_PAGING_ML4_ENTRY *ML4E = &g_KernelPaging.ML4->Entries[PML4I];
    if (!ML4E->Present)
        PANIC("Cannot get scratch ML4E!");
    T_PAGING_PDP *PDP = (T_PAGING_PDP *)(ML4E->Physical << 12);
    T_PAGING_PDP_ENTRY *PDPE = &PDP->Entries[PDPI];
    if (!PDPE->Present)
        PANIC("Cannot get scratch PDPE!");
    return PDPE->Physical << 12;
}

// AccessBits is reserved for future use
void paging_map_page(u64 Virtual, u64 Physical, void *AccessBits)
{
    if (Virtual % 0x1000 || Physical % 0x1000)
        PANIC("BUG: Requsted allocation of unaligned address.\n");

    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);
    u64 DIRI = PAGING_GET_DIR_INDEX(Virtual);
    u64 TABI = PAGING_GET_TAB_INDEX(Virtual);

    T_PAGING_ML4_ENTRY *ML4E = &g_KernelPaging.ML4->Entries[PML4I];
    T_PAGING_PDP *PDP = (T_PAGING_PDP *)(ML4E->Physical << 12);
    if (!ML4E->Present)
    {
        PDP = (T_PAGING_PDP *)_early_alloc();
        _zero_paging_structure(PDP);
        ML4E->Present = 1;
        ML4E->RW = 1;
        ML4E->Physical = (u64)PDP >> 12;
    }

    T_PAGING_PDP_ENTRY *PDPE = &PDP->Entries[PDPI];
    T_PAGING_DIR *Dir = (T_PAGING_DIR *)(PDPE->Physical << 12);
    if (!PDPE->Present)
    {
        Dir = (T_PAGING_DIR *)_early_alloc();
        _zero_paging_structure(Dir);
        PDPE->Present = 1;
        PDPE->RW = 1;
        PDPE->Physical = (u64)Dir >> 12;
    }

    
    T_PAGING_DIR_ENTRY *DIRE = &Dir->Entries[DIRI];
    T_PAGING_TAB *Tab = (T_PAGING_TAB *)(DIRE->Physical << 12);
    if (!DIRE->Present)
    {
        Tab = (T_PAGING_TAB *)_early_alloc();
        _zero_paging_structure(Tab);
        DIRE->Present = 1;
        DIRE->RW = 1;
        DIRE->Physical = (u64)Tab >> 12;
    }

    T_PAGING_TAB_ENTRY *TABE = &Tab->Entries[TABI];
    TABE->Physical = Physical >> 12;
    TABE->Present = 1;
    TABE->RW = 1;
}

void paging_map_area(u64 PhysicalStart, u64 VirtualStart, u64 Size, void *AccessBits)
{
    if (VirtualStart % 0x1000 || PhysicalStart % 0x1000)
        PANIC("BUG: Requsted allocation of unaligned address.\n");

    u64 AlignedSize = Size;
    if (AlignedSize % 0x1000)
        AlignedSize = (AlignedSize + 0x1000) & 0xFFFFF000;
    for (u64 i = 0; i < AlignedSize; i += 0x1000)
        paging_map_page(VirtualStart + i, PhysicalStart + i, AccessBits);
}

void paging_kernel_init(void)
{
    g_KernelPaging.ML4 = _early_alloc();
    g_KernelPaging.FullIdentityMapping = 0;
    kprintf("[i] Setting up new paging structures...\n");
    // Identity map a whole bunch of memory - TODO, make this map all of physmem..?
    paging_map_area(0x0, 0x0, 0x0FFFFFFF, 0);

    // Copy all the necessary ELF sections of our kernel image
    // However, we need to copy them from the currently running kernel,
    // as we have some important .data and .bss contents we need to keep.

    // This results in the fact, that we need to do it in two passes:
    // The first uses all the necessary system functions to allocate
    // the memory for the new kernel memory. After this pass ends, we must
    // ensure that there will be no .data or .bss modifications.
    // The second pass copies the data.

    // First pass:
    kprintf("[i] Setting up paging for kernel ELF:\n");
    TELF *Elf = system_get_kernel_elf();
    for (u64 Section = 0; Section < Elf->SectionCount; Section++)
    {
        u64 Virtual = elf_section_get_virtual_address(Elf, Section);

        // non-loadable sections have a zero VA
        if (Virtual)
        {
            u64 Size = elf_section_get_size(Elf, Section);
            s8 *Name = elf_section_get_name(Elf, Section);
            kprintf(" - '%s': %x (%i)\n", Name, Virtual, Size);
            ASSERT(!(Virtual % 0x1000));

            u64 NumPages = Size / 0x1000;
            if (Size % 0x1000)
                NumPages++;

            for (u64 i = 0; i < NumPages; i++)
            {
                // Allocate the memory...
                u64 TargetPhysical = physmem_allocate_physical();
                // ..and map the page.
                paging_map_page(Virtual + (i * 0x1000), TargetPhysical, 0);
            }
            
        }
    }

    // Second pass:
    for (u64 Section = 0; Section < Elf->SectionCount; Section++)
    {
        u64 Virtual = elf_section_get_virtual_address(Elf, Section);
        if (Virtual)
        {
            u64 Size = elf_section_get_size(Elf, Section);
            u64 NumPages = Size / 0x1000;
            if (Size % 0x1000)
                NumPages++;

            for (u64 i = 0; i < NumPages; i++)
            {
                // Get the physical address of the page we copy data to.
                u64 TargetPhysical;
                u64 Source = Virtual + (i * 0x1000);
                ASSERT(_paging_resolve(Source, &TargetPhysical) == 0);

                // This ensures that the last page, which might not be
                // fully occupied, is copied properly
                u64 BytesToCopy = (Size - (i * 0x1000));
                if (BytesToCopy > 0x1000)
                    BytesToCopy = 0x1000;

                // Copy the data.
                kmemcpy((u8 *)TargetPhysical, (u8 *)Source, BytesToCopy);

                // Stupid check...
                ASSERT(((u64 *)Source)[0] == ((u64 *)TargetPhysical)[0]);
            }
        }
    }

    // Immediately apply new paging structures.
    __asm__ volatile ("movq %%rax, %%cr3" :: "a" (g_KernelPaging.ML4));
    g_KernelPaging.FullIdentityMapping = 1;

    // TODO: release loader-provided paging in order to conserver memory.
}
