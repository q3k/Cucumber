// Basic information gathere about system

#include "types.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/physical_alloc.h"

u32 g_system_memory_lower = 0;
u32 g_system_memory_upper = 0;
s8* g_system_bootloader = "Unknown Multiboot Bootloader";

// Just a guess...
T_SYSTEM_INVALID_RAM g_system_invalid_areas[256];
u8 g_system_num_invalid_areas;

void system_parse_multiboot_header(void *Header)
{
    u8 Flags = *((u8*)Header);

    if (Flags & 1)
    {
        g_system_memory_lower = ((u8*)Header)[4];
        g_system_memory_upper = ((u8*)Header)[8];
    }

    if ((Flags >> 9) & 1)
        g_system_bootloader = (s8*)((u32*)Header)[16];

    if ((Flags >> 6) & 1)
    {
        // Memory map from bootloader...
        u32 MapStart = ((u32*)Header)[12];
        u32 MapLength = ((u32*)Header)[11];

        g_system_num_invalid_areas = 0;
        u32 AvailableMemory = 0;

        T_SYSTEM_MLTBT_MMAP *Node = (T_SYSTEM_MLTBT_MMAP*)MapStart;

        while ((u32)Node + 4 - MapStart < MapLength)
        {
            u32 Size = Node->Size;
            if (Size == 0)
                Size = sizeof(T_SYSTEM_MLTBT_MMAP);

            if (Node->Type == 1)
                AvailableMemory += Node->LengthLow;
            else
            {
                // Not available!
                T_SYSTEM_INVALID_RAM Area = \
                     g_system_invalid_areas[g_system_num_invalid_areas];

                Area.Base = Node->BaseLow;
                Area.Size = Node->LengthLow;

                g_system_num_invalid_areas++;
                
                u32 Page = physmem_physical_to_page(Area.Base);
                for (int i = 0; i <= Area.Size / (4 * 1024); i++)
                    physmem_mark_as_used(Page + i);
            }

            Node = (T_SYSTEM_MLTBT_MMAP*)((u32)Node + Size + 4);
        }

        g_system_memory_upper = AvailableMemory / 1024;
    }
    
    // Mark first MB as used
    for (u16 i = 0; i < 1024; i++)
        physmem_mark_as_used(i);
    
    // Mark all memory > memory size as used.
    u16 StartPage = g_system_memory_upper / (1024 * 4);
    u16 NumPages = (0xFFFFFFFF / 1024 - g_system_memory_upper) / (1024 * 4);
    for (int i = 0; i < NumPages; i++)
        physmem_mark_as_used(StartPage + i);
    
    if (g_system_memory_upper % (1024 * 4) != 0)
        physmem_mark_as_used(0xFFFFFFFF / (1024 * 4));
}

u32 system_get_memory_upper(void)
{
    return g_system_memory_upper;
}

u32 system_get_memory_lower(void)
{
    return g_system_memory_lower;
}

s8 *system_get_bootloader_name(void)
{
    return g_system_bootloader;
}

u8 system_memory_available(u32 Start, u32 Length)
{
    for (u8 i = 0; i < g_system_num_invalid_areas; i++)
    {
        T_SYSTEM_INVALID_RAM Area = g_system_invalid_areas[i];

        // If the start address is somwhere in the invalid area
        if (Area.Base <= Start && Area.Base + Area.Size > Start)
            return 0;

        // If the end address is somewhere in the invalid area
        if (Area.Base <= Start + Length && Area.Base + Area.Size > Start + Length)
            return 0;

        // If the request spans accross an invalid area
        if (Area.Base >= Start && Start + Length < Area.Base + Area.Size)
            return 0;
    }
    return 1;
}

