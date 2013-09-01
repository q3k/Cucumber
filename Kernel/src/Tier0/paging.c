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
} g_KernelPaging;

// allocate a page frame and make sure that it is accessible before
// our main paging is running - check whether it fits the extended
// memory area (up to 0xEFFFFF), as this is the area the bootloader
// identity mapped up to from 0x0.
void *_early_alloc(void)
{
    u64 Address = physmem_allocate_physical();
    ASSERT(Address < 0x00EFFFFF - 0x1000);
    return (void*)Address;
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
        ML4E->Physical = (u64)PDP >> 12;
    }

    T_PAGING_PDP_ENTRY *PDPE = &PDP->Entries[PDPI];
    T_PAGING_DIR *Dir = (T_PAGING_DIR *)(PDPE->Physical << 12);
    if (!PDPE->Present)
    {
        Dir = (T_PAGING_DIR *)_early_alloc();
        _zero_paging_structure(Dir);
        PDPE->Present = 1;
        PDPE->Physical = (u64)Dir >> 12;
    }

    
    T_PAGING_DIR_ENTRY *DIRE = &Dir->Entries[DIRI];
    T_PAGING_TAB *Tab = (T_PAGING_TAB *)(DIRE->Physical << 12);
    if (!DIRE->Present)
    {
        Tab = (T_PAGING_TAB *)_early_alloc();
        _zero_paging_structure(Tab);
        DIRE->Present = 1;
        DIRE->Physical = (u64)Tab >> 12;
    }

    T_PAGING_TAB_ENTRY *TABE = &Tab->Entries[TABI];
    TABE->Physical = Physical >> 12;
    TABE->Present = 1;
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
    // Identity map all the BIOS EMM (extended memory). This covers a lot of 
    // classic PC I/O mapped stuff (eg. video RAM) and probably our kernel and
    // loader artifacts.
    paging_map_area(0x0, 0x0, 0x00EFFFFF, 0);
}