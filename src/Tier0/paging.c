#include "Tier0/paging.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "types.h"

// 10 megabytes is safe guess, I guess.
#define PAGING_FREEFORALL_START 0x00F00000

u32 g_kernel_page_directory[1024] __attribute__ ((aligned (4096)));
u32 g_kernel_page_tables[1024][1024] __attribute__ ((aligned (4096)));

u32 g_paging_current_offset = PAGING_FREEFORALL_START;

u8 paging_get_physical(u32 Virtual, u32 *Physical)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 DirectoryEntry = g_kernel_page_directory[DirectoryIndex];

    u8 TablePresent = DirectoryEntry & 0b1;
    if (!TablePresent) 
        return 0;

    u32 TableAddress = 0;
    TableAddress |= (DirectoryEntry & 0xFFFFF000);
    u32 *Table = (u32 *)TableAddress;


    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    u32 TableEntry = Table[TableIndex];

    u8 PagePresent = TableEntry &0b1;
    if (!PagePresent)
        return 0;

    *Physical = 0;
    *Physical |= (TableEntry & 0xFFFFF000);
    *Physical |= (Virtual & 0xFFF);

    return 1;
}

void paging_dump_directory(void)
{
    for (u32 i = 0; i < 10; i++)
    {
        kprintf("[i] Virtual 0x%X - 0x%X, Table 0x%X.\n", i * 4096 * 1024, \
                (i + 1) * 4096 * 1024, g_kernel_page_directory[i]);
    }
}

// Hey, Serge, or whoever will read this.
//
// Do NOT modify me to be used in user processes. I know it may be tempting to
// do so, but don't. I'm dead serious.
//
// This is strictly (!) kernel-only. This assumes that the tables are already
// created. If we were to create an empty set of tables, it would mean wasting
// 1MB of memory for each process - and that's a Bad Thing. However, we can
// permit ourselves to do this for the kernel. Heck, it's necessary - we need
// solid paging to make a solid heap which will enable us to create dynamic
// page tables for user processes. Woo.

// This maps 4KB
void paging_map_kernel_page(u32 Virtual, u32 Physical)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;

    // Set directory entry to available
    u32 *DirectoryEntry = &g_kernel_page_directory[DirectoryIndex];
    *DirectoryEntry |= 0x03;

    u16 TableIndex = (Virtual >> 12) & 0x3FF;

    u32 *TableEntry = &g_kernel_page_tables[DirectoryIndex][TableIndex];


    *TableEntry = 0;
    // Set to present and writable
    *TableEntry |= 0x3;
    // Set to point to the physical address.
    *TableEntry |= (Physical & 0xFFFFF000);
}


// This maps 4MB
void paging_map_kernel_table(u32 Virtual, u32 Physical)
{
    for (u16 i = 0; i < 1024; i++)
        paging_map_kernel_page(Virtual + i * 0x1000, Physical + i * 0x1000);
}

void paging_init_simple(void)
{
    // Initialize the directory
    for (u16 i  = 0; i < 1024; i++)
        g_kernel_page_directory[i] = (((u32)g_kernel_page_tables[i]) \
                                    + 0x40000000); 

    // Initialize the kernel mappings (0..8MB and 3072..3080MB
    paging_map_kernel_table(0x00000000, 0x00000000);
    paging_map_kernel_table(0x00400000, 0x00400000);

    paging_map_kernel_table(0xC0000000, 0x00000000);
    paging_map_kernel_table(0xC0400000, 0x00400000);

    void *PhysicalDirectory = (u8 *)g_kernel_page_directory + \
                                        0x40000000;
    __asm volatile (  "mov %0, %%eax\n"
                    "mov %%eax, %%cr3\n"
                    "mov %%cr0, %%eax\n"
                    "orl $0x80000000, %%eax\n"
                    "mov %%eax, %%cr0\n" :: "m" (PhysicalDirectory));
}

// This allocates a 4kb page for whatever reason
void paging_allocate_page(u32 Virtual)
{
    u32 MaximumAddress = system_get_memory_upper();
    while (!system_memory_available(g_paging_current_offset, 0x1000))
    {
        g_paging_current_offset += 0x1000;
        if (g_paging_current_offset > MaximumAddress)
        {
            kprintf("[e] Fatal error: out of memory!\n");
            for (;;) {}
        }
    }

    paging_map_kernel_page(Virtual, g_paging_current_offset);
    g_paging_current_offset += 0x1000;
}
