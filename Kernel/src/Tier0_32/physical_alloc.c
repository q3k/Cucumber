// Kinda like a heap, but not really.
// Based on 1024 4MB superpages. You can't allocate anything lower than that,
// but who cares. The only time this is going to be called is by lower parts of
// the kernel, anyway.

#include "Tier0/physical_alloc.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

u32 g_physmem_directory[32 * 1024];

void physmem_init(void)
{
    for (int i = 0; i < 32; i++)
        g_physmem_directory[i] = 0;
}

void physmem_mark_as_used(u32 Page)
{
    u32 Entry = Page / 32;
    u8 Bit = Page % 32;
    
    g_physmem_directory[Entry] |= (1 << Bit);
}

u32 physmem_allocate_page(void)
{
    for (u32 i = 0; i < 32 * 1024; i++)
    {
        u32 Entry = g_physmem_directory[i];
        if (Entry != 0xFFFFFFFF)
        {
            // Ooh, there's a page in this entry
            for (int j = 0; j < 32; j++)
            {
                u8 Available = (~Entry & (1 << j)) > 0;
                if (Available)
                {
                    u32 Page = i * 32 + j;
                    physmem_mark_as_used(Page);
                    return Page;
                }
            }
        }
    }
    PANIC("Could not allocate page!");
    return 0;
}

void physmem_free_page(u32 Page)
{
    u8 Entry = Page / 32;
    u8 Bit = Page % 32;
    
    g_physmem_directory[Entry] &= ~(1 << Bit);
}

u32 physmem_page_to_physical(u32 Page)
{
    return Page * 1024 * 4;
}

u32 physmem_physical_to_page(u32 Physical)
{
    return Physical / (1024 * 4);
}
