#include "Tier0/paging.h"
#include "types.h"

u32 g_kernel_page_directory[1024] __attribute__ ((aligned (4096)));
u32 g_low_page_table[1024] __attribute__ ((aligned (4096)));

void init_simple_paging(void)
{
    void *RealKernelPageDir = (u8 *)g_kernel_page_directory + 0x40000000;
    void *RealLowPageTable = (u8 *)g_low_page_table + 0x40000000;

    for (u16 i = 0; i < 1024; i++)
    {
        g_kernel_page_directory[i] = (i * 4096) | 0x03;
        g_low_page_table[i] = 0;
    }

    g_kernel_page_directory[0] = (u32)RealLowPageTable | 0x03;
    g_kernel_page_directory[768] = (u32)RealLowPageTable | 0x03;

    __asm__ volatile (  "mov %0, %%eax\n"
                    "mov %%eax, %%cr3\n"
                    "mov %%cr0, %%eax\n"
                    "orl $0x80000000, %%eax\n"
                    "mov %%eax, %%cr0\n" :: "m" (RealKernelPageDir));
}
