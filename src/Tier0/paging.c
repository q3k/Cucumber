#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "types.h"

u32 g_kernel_page_directory[1024] __attribute__ ((aligned (4096)));
u32 g_low_page_table[1024] __attribute__ ((aligned (4096)));

/*u8 paging_get_physical(u32 Virtual, u32 *Physical)
{
    u16 Index = 

    u32 DirectoryEntry = g_kernel_page_directory[Index];

    u8 TablePresent = DirectoryEntry & 0b1;

    if (!TablePresent) 
        return 0;

    u32 TableAddress = 0;
    TableAddress |= (DirectoryEntry & 0xFFFFF000);

    u32 Table = *((u32 *) &TableAddress);

    u8 PagePresent = DirectoryEntry &
}*/

void paging_dump_directory(void)
{
    for (u32 i = 0; i < 10; i++)
    {
        kprintf("[i] Virtual 0x%X - 0x%X, Entry 0x%X.\n", i * 4096 * 1024, (i + 1) * 4096 * 1024, g_kernel_page_directory[i] & 0xFFFFF000);
    }
}

void paging_init_simple(void)
{
    void *RealKernelPageDir = (u8 *)g_kernel_page_directory + 0x40000000;
    void *RealLowPageTable = (u8 *)g_low_page_table + 0x40000000;

    for (u16 i = 0; i < 1024; i++)
    {
        g_low_page_table[i] = (i * 4096) | 0x03;
        g_kernel_page_directory[i] = 0;
    }

    kprintf("[i] Page Directory Physical: 0x%X, Virtual: 0x%X.\n", RealKernelPageDir, g_kernel_page_directory);
    kprintf("[i] Low Page Table Physical: 0x%X, Virtual: 0x%X.\n", RealLowPageTable, g_low_page_table);

    g_kernel_page_directory[0] = (u32)RealLowPageTable | 0x03;
    g_kernel_page_directory[768] = (u32)RealLowPageTable | 0x03;
    
    paging_dump_directory();


    __asm volatile (  "mov %0, %%eax\n"
                    "mov %%eax, %%cr3\n"
                    "mov %%cr0, %%eax\n"
                    "orl $0x80000000, %%eax\n"
                    "mov %%eax, %%cr0\n" :: "m" (RealKernelPageDir));
}
