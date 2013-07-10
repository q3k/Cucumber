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

void paging_set_ml4(u64 ML4Physical)
{
    __asm volatile ( "mov %%rax, %%cr3\n" :: "a" (ML4Physical));
}