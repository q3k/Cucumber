// Yet another shot at a frame manager... This time, instead of focusing on
// something unnecessarily complex, I'll try something simpler: an alloc-only
// frame manager. I wasn't even freeing them anyway, and I don't think I'll ever
// need that. All the hardcore memory allocation will be done on the kernel heap
// anyway.
//
// tl;dr - want to free a frame? memory leaks ahoy.
//
// I want this to be as simple as a 'top' pointer, pointing to the next address
// of the physical memory that is free. There are two things to watch out for,
// though:
//   o unusable memory regions
//   o max memory size
// We'll solve the first problem by asking system.c whether we can use a provided
// region. It used to work the previosu way around, but I'm not going to keep a
// list of unusable regions both here and in system.c...
// The second problem is solved by also keeping a max memory size, and throwing
// a kernel panic when we reach it. It could use an exception system, but since
// this code will only be called from low-level system routines, if it actually 
// happens that we run out of memory... We're probably badly screwed, anyway.

#include "Tier0/physmem.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"
#include "Tier0/paging.h"

// The amount of memory in the system, or the top usable pointer.
u64 g_MemorySize;
// The current pointer to the top of the frame stack.
u64 g_TopFrame;

void physmem_init(u64 MemorySize)
{
    g_TopFrame = 0;
    g_MemorySize = MemorySize;
}

u64 physmem_allocate_page(void)
{
    u64 NextPageStart = g_TopFrame;
    
    while (!system_memory_available(NextPageStart, PHYSMEM_PAGE_SIZE))
    {
        NextPageStart += PHYSMEM_PAGE_SIZE;
        
        if (NextPageStart > g_MemorySize)
            PANIC("Out of memory!");
    }
    
    return NextPageStart / PHYSMEM_PAGE_SIZE;
}


u64 physmem_page_to_physical(u64 Page)
{
    return Page * PHYSMEM_PAGE_SIZE;
}

u64 physmem_physical_to_page(u64 Physical)
{
    return Physical / PHYSMEM_PAGE_SIZE;
}

void physmem_read(u64 Base, u64 Size, void *Destination)
{
    u8 *DataSource = (u8 *)paging_temp_page_get_virtual();
    u64 OffsetInSource = Base & 0xFFF;
    
    u64 PreviousPageBase = Base & ~((u64)0xFFF);
    paging_temp_page_set_physical(PreviousPageBase);
    for (u64 i = 0; i < Size; i++)
    {
        u64 PageBase = (Base + i) & ~((u64)0xFFF);
        
        if (PageBase != PreviousPageBase)
            paging_temp_page_set_physical(PageBase);
        
        PreviousPageBase = PageBase;
        
        *((u8 *)Destination + i) = DataSource[OffsetInSource % 4096];
        OffsetInSource++;
    }
}
