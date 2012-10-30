#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"
#include "Tier0/system.h"
#include "types.h"

struct {
    T_PAGING_TAB_ENTRY *TempPage; // For temp page mapping.
    u64 TempPageVirtual;
} g_KernelPaging;

struct {
    u64 Start;
    u64 End;
    
    u64 Top;
} g_MiniVMM;

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

void paging_minivmm_setup(void)
{
    g_MiniVMM.Start = system_get_kernel_virtual_start() + system_get_kernel_size();
    g_MiniVMM.End = system_get_kernel_virtual_start() + 511 * 0x1000;
    g_MiniVMM.Top = g_MiniVMM.Start;
    kprintf("[i] MiniVMM: %x - %x.\n", g_MiniVMM.Start, g_MiniVMM.End);
}

u64 paging_minivmm_allocate(void)
{
    if (g_MiniVMM.Top + 0x1000 > g_MiniVMM.End)
        PANIC("MiniVMM out of memory!");
    
    u64 Result = g_MiniVMM.Top;
    g_MiniVMM.Top += 4096;
    
    return Result;
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
    __asm__ volatile("invlpg %0" :: "m"(*(u32 *)Virtual));
}
