#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"
#include "types.h"

struct {
    T_PAGING_TAB_ENTRY *TempPage; // For temp page mapping.
    u64 TempPageVirtual;
} __attribute__((packed)) g_KernelPaging;

T_PAGING_ML4 *paging_get_ml4(void)
{
	u64 Address;
	__asm__ volatile("mov %%cr3, %0\n" : "=r"(Address));
	return (T_PAGING_ML4*)Address;
}

void paging_temp_page_setup(T_LOAD_CONTEXT *LoadContext)
{
    // Try using page 511 (last) from kernel table
    u64 PageVirtual = 0xFF000000 + 511 * 4096;
    u64 MaxMapped = 4096 * 512; // first 2Mib by loader
    
    u64 KernelSize = LoadContext->KernelPhysicalEnd - LoadContext->KernelPhysicalStart;
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
    __asm__ volatile("invlpg %0" :: "m"(*(u32 *)g_KernelPaging.TempPageVirtual));
}

u8 paging_get_physical_ex(u64 Virtual, u64 *Physical, T_PAGING_ML4 *ML4)
{
    PANIC("not implemented!");
    return 0;
}

u8 paging_get_physical(u64 Virtual, u64 *Physical)
{
	T_PAGING_ML4 *ml4 = paging_get_ml4();
    return paging_get_physical_ex(Virtual, Physical, ml4);
}

void paging_set_ml4(u64 ML4Physical)
{
    __asm volatile ( "mov %%rax, %%cr3\n" :: "a" (ML4Physical));
}
