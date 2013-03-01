#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"
#include "Tier0/system.h"
#include "Tier0/physmem.h"
#include "types.h"

struct {
    T_PAGING_TAB_ENTRY *TempPage; // For temp page mapping.
    u64 TempPageVirtual;
} g_KernelPaging;

T_PAGING_ML4 *paging_get_ml4(void)
{
	u64 Address;
	__asm__ volatile("mov %%cr3, %0\n" : "=r"(Address));
	return (T_PAGING_ML4*)Address;
}

void paging_temp_page_setup(void)
{
    u64 KernelVirtualStart = system_get_kernel_virtual_start();
    u64 KernelSize = system_get_kernel_size();

    // Try using page 511 (last) from kernel table
    u64 PageVirtual = KernelVirtualStart + 511 * 4096;
    u64 MaxMapped = 4096 * 512; // first 2Mib by loader
    
    if (KernelSize >= MaxMapped)
        PANIC("Cannot set up temp page, kernel > 2Mib!");
    
    T_PAGING_ML4 *ML4 = paging_get_ml4();
    kprintf("[i] Loader-provided ML4 @0x%x\n", ML4);
    
    if ((u64)ML4 > MaxMapped)
        PANIC("Cannot set up temp page, ML4 not accessible!");
    
    u64 aDPT = ML4->Entries[PAGING_GET_ML4_INDEX(PageVirtual)].Physical << 12;
    
    if (aDPT > MaxMapped)
        PANIC("Cannot set up temp page, DPT not accessible!");
    T_PAGING_DPT *DPT = (T_PAGING_DPT *)aDPT;
    
    u64 aDir = DPT->Entries[PAGING_GET_DPT_INDEX(PageVirtual)].Physical << 12;
    
    if (aDir > MaxMapped)
        PANIC("Cannot set up temp page, Dir not accessible!");
    T_PAGING_DIR *Dir = (T_PAGING_DIR *)aDir;
    
    u64 aTab = Dir->Entries[PAGING_GET_DIR_INDEX(PageVirtual)].Physical << 12;
    
    if (aTab > MaxMapped)
        PANIC("Cannot set up temp page, Tab not accessible!");
    T_PAGING_TAB *Tab = (T_PAGING_TAB *)aTab;
    
    g_KernelPaging.TempPage = &Tab->Entries[511];
    
    kprintf("[i] Using paging table entry @0x%x as temporary page.\n", g_KernelPaging.TempPage);
    g_KernelPaging.TempPageVirtual = PageVirtual;
}

void paging_temp_page_set_physical(u64 Physical)
{
    if ((Physical & 0xFFF) != 0)
        PANIC("Tried to set temp page physical to unaligned address!");
    
    // TODO: check if smaller than maxphyaddr 

    g_KernelPaging.TempPage->Physical = Physical >> 12;
    __asm__ volatile("invlpg %0" :: "m"(*(u64 *)g_KernelPaging.TempPageVirtual));
}

inline volatile const u64 paging_temp_page_get_virtual(void)
{
    return 0xFFFFFFFF80000000 + 511 * 0x1000;
}

/*u8 paging_get_physical_ex(u64 Virtual, u64 *Physical, T_PAGING_ML4 *ML4)
{
    if (Virtual < g_KernelPaging.KernelVirtualStart || Virtual > g_KernelPaging.KernelVirtualStart + g_KernelPaging.KernelSize)
    {
        PANIC("not implemented");
        return 0;
    }

    *Physical = Virtual - g_KernelPaging.KernelVirtualStart + g_KernelPaging.KernelPhysicalStart;
    return 1;
}

u8 paging_get_physical(u64 Virtual, u64 *Physical)
{
	T_PAGING_ML4 *ml4 = paging_get_ml4();
    return paging_get_physical_ex(Virtual, Physical, ml4);
}*/

void paging_set_ml4(u64 ML4Physical)
{
    __asm volatile ( "mov %%rax, %%cr3\n" :: "a" (ML4Physical));
}

struct {
    u32 UnmanagedSize;
    u8 HeapSetUp;
    u64 DirectoryPhysical;
} g_PagingScratch;

void paging_scratch_initialize(void)
{
    // let's first allocate a physical frame for the DIR
    u64 DirPhysical = physmem_allocate_page() * 4096;
    // map it to our trusty temp page
    paging_temp_page_set_physical(DirPhysical);
    T_PAGING_DIR *Directory = (T_PAGING_DIR *)paging_temp_page_get_virtual();
    // zero the entries
    for (u16 i = 0; i < 512; i++)
        Directory->Entries[i].Present = 0;

    // attach the scratch to the DPT. we can do this without using a temp page,
    // as the boot paging structures lie in the 2mib identity paged zone
    u16 ML4Entry = PAGING_GET_ML4_INDEX(0xFFFFFFFF00000000);
    T_PAGING_ML4 *ML4 = paging_get_ml4();

    ASSERT(ML4->Entries[ML4Entry].Present);
    u64 aDPT = ML4->Entries[ML4Entry].Physical << 12;
    kprintf("[i] DPT Physical 0x%x, ML4 index %i.\n", aDPT, ML4Entry);

    kprintf("[i] Scratch DIR Physical 0x%x\n", DirPhysical);
    T_PAGING_DPT *DPT = (T_PAGING_DPT *)aDPT;
    u16 DPTEntry = PAGING_GET_DPT_INDEX(0xFFFFFFFF00000000);

    ASSERT(!DPT->Entries[DPTEntry].Present);
    DPT->Entries[DPTEntry].Present = 1;
    DPT->Entries[DPTEntry].RW = 1;
    DPT->Entries[DPTEntry].Physical = DirPhysical >> 12;

    g_PagingScratch.UnmanagedSize = 0;
    g_PagingScratch.HeapSetUp = 0;
    g_PagingScratch.DirectoryPhysical = DirPhysical;
}

void *paging_scratch_map(u64 Physical)
{
    if (g_PagingScratch.HeapSetUp)
        PANIC("Trying to allocate unmanaged scratch after heap exists, abort!");

    u64 Virtual = 0xFFFFFFFF00000000 + g_PagingScratch.UnmanagedSize;
    
    u16 DirEntry = PAGING_GET_DIR_INDEX(Virtual);

    paging_temp_page_set_physical(g_PagingScratch.DirectoryPhysical);
    T_PAGING_DIR *Directory = (T_PAGING_DIR *)paging_temp_page_get_virtual();

    // create table if necessary
    u64 TablePhysical;
    if (!Directory->Entries[DirEntry].Present)
    {
        // create a new page table
        TablePhysical = physmem_allocate_page() * 4096;
        paging_temp_page_set_physical(TablePhysical);
        T_PAGING_TAB *Table = (T_PAGING_TAB*)paging_temp_page_get_virtual();
        // zero the table
        for (u16 i  = 0; i < 512; i++)
            Table->Entries[i].Present = 0;

        // set the directory to point where it should
        paging_temp_page_set_physical(g_PagingScratch.DirectoryPhysical);
        Directory->Entries[DirEntry].Present = 1;
        Directory->Entries[DirEntry].RW = 1;
        Directory->Entries[DirEntry].Physical = TablePhysical >> 12;
    }
    else
        TablePhysical = Directory->Entries[DirEntry].Physical << 12;

    // set the table entry to point to our new page frame
    paging_temp_page_set_physical(TablePhysical);
    T_PAGING_TAB *Table = (T_PAGING_TAB*)paging_temp_page_get_virtual();
    u16 TabEntry = PAGING_GET_TAB_INDEX(Virtual);
    Table->Entries[TabEntry].Present = 1;
    Table->Entries[TabEntry].RW = 1;
    Table->Entries[TabEntry].Physical = Physical >> 12;

    g_PagingScratch.UnmanagedSize += 4096;
    __asm__ __volatile__("invlpg %0" :: "m"(Virtual));
    return (void *)Virtual;
}

void *paging_scratch_allocate(void)
{
    u64 Physical = physmem_allocate_page() * 4096;
    return paging_scratch_map(Physical);
}

void paging_map_page(u64 Virtual, u64 Physical)
{
    T_PAGING_ML4 *ML4 = paging_get_ml4();
    u64 aDPT = ML4->Entries[PAGING_GET_ML4_INDEX(Virtual)].Physical << 12;
    T_PAGING_DPT *DPT = (T_PAGING_DPT *)aDPT;
    u64 aDir = DPT->Entries[PAGING_GET_DPT_INDEX(Virtual)].Physical << 12;
    T_PAGING_DIR *Dir = (T_PAGING_DIR *)aDir;
    u64 aTab = Dir->Entries[PAGING_GET_DIR_INDEX(Virtual)].Physical << 12;
    T_PAGING_TAB *Tab = (T_PAGING_TAB *)aTab;
    
    Tab->Entries[PAGING_GET_TAB_INDEX(Virtual)].Physical = Physical >> 12;    
    __asm__ volatile("invlpg %0" :: "m"(Virtual));
}

u64 paging_scratch_get_physical(void* Virtual)
{
    u16 DirEntry = PAGING_GET_DIR_INDEX(Virtual);
    paging_temp_page_set_physical(g_PagingScratch.DirectoryPhysical);
    T_PAGING_DIR *Directory = (T_PAGING_DIR *)paging_temp_page_get_virtual();

    if (!Directory->Entries[DirEntry].Present)
        PANIC("Address not in directory!");

    u64 TablePhysical = Directory->Entries[DirEntry].Physical << 12;
    paging_temp_page_set_physical(TablePhysical);
    T_PAGING_TAB *Table = (T_PAGING_TAB*)paging_temp_page_get_virtual();
    u16 TabEntry = PAGING_GET_TAB_INDEX(Virtual);

    if (!Table->Entries[TabEntry].Present)
        PANIC("Address not in table!");

    return Table->Entries[TabEntry].Physical << 12;
}