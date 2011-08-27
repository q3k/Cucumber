#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"
#include "types.h"

// The basic structures for the first kernel thread...
// Here's an overview of how it will look like:
//
// ML4 -> DPT -> - DIR0 -> 512x Tab -> 512x512x Page
//               - DIR1 -> 512x Tab -> 512x512x Page
//               - DIR2 -> 512x Tab -> 512x512x Page
//               - DIR3 -> 512x Tab -> 512x512x Page
// This lets us have more-or-less dynamic paging of the whole first 32-bit
// memory space... Since we can't have true dynamic allocation of paging
// structures (as he wave no kernel heap, as we have no paging), we'll have to
// settle for this. The size of this whole mess should be around 8mib or so.
struct {
    T_PAGING_ML4 *g_KernelML4; // This is allocated semi-dynamically
} __attribute__((packed)) g_KernelPaging;

T_PAGING_ML4 *paging_get_ml4(void)
{
	u64 Address;
	__asm__ volatile("mov %%cr3, %0\n" : "=r"(Address));
	return (T_PAGING_ML4*)Address;
}

T_PAGING_ML4 *paging_get_kernel_ml4(void)
{
    //return &g_KernelPaging.ML4;
    return 0;
}

u8 paging_get_physical_ex(u64 Virtual, u64 *Physical, T_PAGING_ML4 *ML4)
{
    /*u16 ML4Index = PAGING_GET_ML4_INDEX(Virtual);
    u16 DPTIndex = PAGING_GET_DPT_INDEX(Virtual);
    u16 DirIndex = PAGING_GET_DIR_INDEX(Virtual);
    u16 TabIndex = PAGING_GET_TAB_INDEX(Virtual);
    

    if (!ML4->Entries[ML4Index].Present)
    	return 1;
    T_PAGING_DPT *DPT = ML4->Children[ML4Index];

    if (!DPT->Entries[DPTIndex].Present)
    	return 1;
    T_PAGING_DIR *Dir = DPT->Children[DPTIndex];

    if (!Dir->Entries[DirIndex].Present)
    	return 1;
    T_PAGING_TAB *Tab = Dir->Children[DirIndex];

    if (!Tab->Entries[TabIndex].Present)
    	return 1;

    (*Physical) = (Tab->Entries[TabIndex].Physical << 12) + PAGING_GET_PAGE_OFFSET(Virtual);*/

    return 0;
}

u8 paging_get_physical(u64 Virtual, u64 *Physical)
{
	T_PAGING_ML4 *ml4 = paging_get_ml4();
    return paging_get_physical_ex(Virtual, Physical, ml4);
}

// This initializes the paging structure for the first kernel thread
void paging_init_simple(u64 KernelPhysicalStart, u64 KernelPhysicalSize)
{
    
}

void paging_use_ml4(T_PAGING_ML4 *ML4)
{
    //__asm volatile ( "mov %%rax, %%cr3\n" :: "a" (ML4->PhysicalAddress));
}

/*void paging_dump_directory(void)
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
}*/

