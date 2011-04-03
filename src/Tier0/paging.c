#include "Tier0/paging.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/interrupts.h"
#include "Tier0/heap.h"
#include "types.h"

T_PAGING_DIRECTORY g_kernel_page_directory __attribute__ ((aligned (4096)));
T_PAGING_TABLE g_kernel_page_tables[1024] __attribute__ ((aligned (4096)));

u8 paging_get_physical_ex(u32 Virtual, u32 *Physical, 
                          T_PAGING_DIRECTORY *Directory)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 DirectoryEntry = Directory->Entries[DirectoryIndex];

    u8 TablePresent = DirectoryEntry & 0b1;
    if (!TablePresent) 
        return 0;

    T_PAGING_TABLE *Table = Directory->Tables[DirectoryIndex];

    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    T_PAGING_PAGE Page = (Table->Pages[TableIndex]);
    if (!Page.Present)
        return 0;

    *Physical = (Page.Physical << 12);
    *Physical |= (Virtual & 0xFFF);

    return 1;
}

u8 paging_get_physical(u32 Virtual, u32 *Physical)
{
    return paging_get_physical_ex(Virtual, Physical, &g_kernel_page_directory);
}

void paging_dump_directory(void)
{
    for (u32 i = 0; i < 10; i++)
    {
        kprintf("[i] Virtual 0x%X - 0x%X, Table 0x%X.\n", i * 4096 * 1024, \
                (i + 1) * 4096 * 1024, g_kernel_page_directory.Entries[i]);
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
    u32 *DirectoryEntry = &g_kernel_page_directory.Entries[DirectoryIndex];
    *DirectoryEntry |= 0x03;

    u16 TableIndex = (Virtual >> 12) & 0x3FF;

    T_PAGING_PAGE *Page = &g_kernel_page_tables[DirectoryIndex].Pages[TableIndex];

    *((u32*)Page) = 0;
    Page->Present = 1;
    Page->RW = 1;
    Page->Physical = (Physical & 0xFFFFF000) >> 12;
    
    // Flush the TLB
    __asm__ volatile("invlpg %0" :: "m" (Virtual));
}


void paging_map_page(u32 Virtual, u32 Physical, T_PAGING_DIRECTORY *Directory, 
                     u8 User, u8 RW)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 Entry = Directory->Entries[DirectoryIndex];
    
    u8 TablePresent = (Entry & 0x01) > 0;
    
    
    if (!TablePresent)
    {
        u32 NewTablePhysical;
        T_PAGING_TABLE *NewTable = (T_PAGING_TABLE*)kmalloc_p(
            sizeof(T_PAGING_TABLE), 1, &NewTablePhysical);
        
        kmemsetw((void*)NewTable, 0, 1024);
        
        u32 *Entry = &Directory->Entries[DirectoryIndex];
        *Entry = 1;
        *Entry |= (RW << 1);
        *Entry |= (User << 2);
        *Entry |= ((u32)NewTable);
        
        Directory->Tables[DirectoryIndex] = NewTable;
    }
    
    T_PAGING_TABLE *Table = Directory->Tables[DirectoryIndex];
    
    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    
    T_PAGING_PAGE *Page = &Table->Pages[TableIndex];
    
    *((u32*)Page) = 1;
    Page->User = User;
    Page->RW = RW;
    Page->Physical = (Physical & 0xFFFFF000) >> 12;
    
    // Flush the TLB
    __asm__ volatile("invlpg %0" :: "m" (Virtual));
}

// This maps 4MB
void paging_map_kernel_table(u32 Virtual, u32 Physical)
{
    for (u16 i = 0; i < 1024; i++)
        paging_map_kernel_page(Virtual + i * 0x1000, Physical + i * 0x1000);
}

void paging_use_directory(T_PAGING_DIRECTORY *Directory)
{
    __asm volatile (  "mov %0, %%eax\n"
                    "mov %%eax, %%cr3\n"
                    "mov %%cr0, %%eax\n"
                    "orl $0x80000000, %%eax\n"
                    "mov %%eax, %%cr0\n" :: "m" (Directory->PhysicalAddress));
}

T_PAGING_DIRECTORY *paging_get_directory(void)
{
    u32 Address;
    __asm__ volatile("mov %%cr3, %%eax\n"
                     "mov %%eax, %0\n" : "=r"(Address));
    return (T_PAGING_DIRECTORY *)Address;
}

void paging_init_simple(void)
{
    // Initialize the directory
    for (u16 i  = 0; i < 1024; i++)
    {
        g_kernel_page_directory.Entries[i] = (((u32)&g_kernel_page_tables[i])
                                    + 0x40000000); 
        g_kernel_page_directory.Tables[i] = &g_kernel_page_tables[i];
    }
    
    g_kernel_page_directory.PhysicalAddress =
        (u32)g_kernel_page_directory.Entries + 0x40000000;

    // Initialize the kernel mappings (0..8MB and 3072..3080MB
    paging_map_kernel_table(0x00000000, 0x00000000);
    paging_map_kernel_table(0x00400000, 0x00400000);

    paging_map_kernel_table(0xC0000000, 0x00000000);
    paging_map_kernel_table(0xC0400000, 0x00400000);

    paging_use_directory(&g_kernel_page_directory);
}

