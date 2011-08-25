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
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

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
    PANIC("Not implemented!");
    return 0;
}


u64 physmem_page_to_physical(u64 Page)
{
    PANIC("Not implemented!");
    return 0;
}

u64 physmem_physical_to_page(u64 Physical)
{
    PANIC("Not implemented!");
    return 0;
}
