// Kinda like a heap, but not really.
// Based on 1024 4MB superpages. You can't allocate anything lower than that,
// but who cares. The only time this is going to be called is by lower parts of
// the kernel, anyway.

#include "Tier0/physical_alloc.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

u32 g_physmem_directory[32];

void physmem_init(void)
{
    for (int i = 0; i < 32; i++)
        g_physmem_directory[i] = 0;
}

void physmem_mark_as_used(u16 SuperPage)
{
    u8 Entry = SuperPage / 32;
    u8 Bit = SuperPage - Entry * 32;
    
    g_physmem_directory[Entry] |= (1 << Bit);
}

u16 physmem_allocate_superpage(void)
{
    for (int i = 0; i < 32; i++)
    {
        u32 Entry = g_physmem_directory[i];
        if (Entry != 0xFFFFFFFF)
        {
            // Ooh, there's a superpage in this entry
            for (int j = 0; j < 32; j++)
            {
                u8 Available = (~Entry & (1 << j)) > 0;
                if (Available)
                {
                    u16 SuperPage = i * 32 + j;
                    physmem_mark_as_used(SuperPage);
                    return SuperPage;
                }
            }
        }
    }
    PANIC("Could not allocate superpage!");
    return 0;
}

void physmem_free_superpage(u16 SuperPage)
{
    u8 Entry = SuperPage / 32;
    u8 Bit = SuperPage - Entry * 32;
    
    g_physmem_directory[Entry] &= ~(1 << Bit);
}

u32 physmem_superpage_to_physical(u16 SuperPage)
{
    return SuperPage * 1024 * 1024 * 4;
}

u16 physmem_physical_to_superpage(u32 Physical)
{
    return Physical / (1024 * 1024 * 4);
}

void physmem_dump_map(void)
{
    for (int i = 0; i < 32; i++)
    {
        kprintf("%X:", i);
        for (int j = 0; j < 32; j++)
        {
            u32 SuperPage = g_physmem_directory[i];
            s8 c = (SuperPage & (1 << j)) > 0 ? 'X' : '_';
            kprintf("%c", c);
        }
        i++;
        for (int j = 0; j < 32; j++)
        {
            u32 SuperPage = g_physmem_directory[i];
            s8 c = (SuperPage & (1 << j)) > 0 ? 'X' : '_';
            kprintf("%c", c);
        }
        kprintf("\n");
    }
}
