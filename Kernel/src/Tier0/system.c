// Basic information gathere about system

#include "types.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/physical_alloc.h"

u64 g_system_memory_lower = 0;
u64 g_system_memory_upper = 0;
s8* g_system_bootloader = "Unknown Multiboot Bootloader";

// Just a guess...
T_SYSTEM_INVALID_RAM g_system_invalid_areas[256];
u8 g_system_num_invalid_areas;
extern u64 _end;

void system_parse_multiboot_header(void *Header)
{
    u8 Flags = *((u8*)Header);

    if (Flags & 1)
    {
        g_system_memory_lower = (u64)((u8*)Header)[4];
        g_system_memory_upper = (u64)((u8*)Header)[8];
    }

    if ((Flags >> 9) & 1)
        g_system_bootloader = (s8*)(u64)((u32*)Header)[16];

    u64 HighestUnavailable = 0;

    if ((Flags >> 6) & 1)
    {
        // Memory map from bootloader...
        u64 MapStart = ((u32*)Header)[12];
        u64 MapLength = ((u32*)Header)[11];

        g_system_num_invalid_areas = 0;
        u64 AvailableMemory = 0;

        T_SYSTEM_MLTBT_MMAP *Node = (T_SYSTEM_MLTBT_MMAP*)MapStart;

        while ((u64)Node + 4 - MapStart < MapLength)
        {
            u32 Size = Node->Size;
            if (Size == 0)
                Size = sizeof(T_SYSTEM_MLTBT_MMAP);

            if (Node->Type == 1)
                AvailableMemory += Node->Length;
            else
            {
                // Not available!
                T_SYSTEM_INVALID_RAM *Area = \
                     &g_system_invalid_areas[g_system_num_invalid_areas];

                Area->Base = Node->Base;
                Area->Size = Node->Length;

                if (Area->Base > HighestUnavailable)
                	HighestUnavailable = Area->Base;

                g_system_num_invalid_areas++;
            }

            Node = (T_SYSTEM_MLTBT_MMAP*)((u64)Node + Size + 4);
        }

        g_system_memory_upper = AvailableMemory / 1024;
    }
    
    kprintf("[i] Highest unavailable address is %x.\n", HighestUnavailable);
    physmem_init(HighestUnavailable);


    for (u8 i = 0; i < g_system_num_invalid_areas; i++)
    {
    	T_SYSTEM_INVALID_RAM *Area = &g_system_invalid_areas[i];
    	if (Area->Base < HighestUnavailable)
    	{
    		u64 Page = physmem_physical_to_page(Area->Base);
    		kprintf("[i] %x - %x unavailable\n", Area->Base, Area->Base + Area->Size);
    		for (int j = 0; j <= Area->Size / PHYSALLOC_PAGE_SIZE; j++)
				physmem_mark_as_used(Page + j);
    	}
    }

    // Mark BIOS (1MB) as used
    //u64 BIOSSize = 1024*1024;
    //for (u16 i = 0; i <  BIOSSize / PHYSALLOC_PAGE_SIZE; i++)
    //        physmem_mark_as_used(i);

    // Mark kernel memory as used
    //u64 KernelSize = ((u64)&_end) - 0xFF000000;
    //for (u16 i = 0; i <  KernelSize / PHYSALLOC_PAGE_SIZE; i++)
    //    physmem_mark_as_used(i);

    // Set the new node space for physmem
    u64 Page = physmem_allocate_page();
    u64 Memory = physmem_page_to_physical(Page);

    // TODO: Fix relying on identity paging for low memory addresses
    physmem_set_node_space(Memory);
}

u64 system_get_memory_upper(void)
{
    return g_system_memory_upper;
}

u64 system_get_memory_lower(void)
{
    return g_system_memory_lower;
}

s8 *system_get_bootloader_name(void)
{
    return g_system_bootloader;
}

u8 system_memory_available(u64 Start, u64 Length)
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

