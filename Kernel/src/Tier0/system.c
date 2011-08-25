// Basic information gathere about system

#include "types.h"
#include "Tier0/system.h"
#include "Tier0/panic.h"
#include "Tier0/kstdio.h"
#include "Tier0/physmem.h"

T_SYSTEM_INFO g_SystemInfo;

extern u64 _end;
extern u64 _start;

void system_parse_load_context(T_LOAD_CONTEXT *LoadContext)
{
    void *Header = (void *)LoadContext->MultibootHeader;
    u8 Flags = *((u8*)Header);

    // Lower & Upper memory limits from Multiboot header
    // The upper memory limit is not required to specify all the available RAM
    if (Flags & 1)
    {
        g_SystemInfo.MemoryLower = (u64)((u8*)Header)[4];
        g_SystemInfo.MemoryUpper = (u64)((u8*)Header)[8];
    }
    
    // Bootloader name from Multiboot header
    if ((Flags >> 9) & 1)
        g_SystemInfo.BootloaderName = (s8*)(u64)((u32*)Header)[16];
    else
        g_SystemInfo.BootloaderName = LoadContext->LoaderName;

    u64 HighestUnavailable = 0;

    // First, chew through what Multiboot gave us...
    if ((Flags >> 6) & 1)
    {
        // Memory map from bootloader...
        u64 MapStart = ((u32*)Header)[12];
        u64 MapLength = ((u32*)Header)[11];

        g_SystemInfo.NumInvalidAreas = 0;
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
                     &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];

                Area->Base = Node->Base;
                Area->Size = Node->Length;

                if (Area->Base > HighestUnavailable)
                	HighestUnavailable = Area->Base;

                g_SystemInfo.NumInvalidAreas++;
            }

            Node = (T_SYSTEM_MLTBT_MMAP*)((u64)Node + Size + 4);
        }

        g_SystemInfo.MemoryUpper = AvailableMemory / 1024;
    }
    else
        PANIC("Not implemented: Memory Map Probing");
    
    // Now, mark the BIOS area (lowest megabyte) as unavailable
    // TODO: Implement me
    
    // And mark our kernel physical location as unavailable
    // TODO: Implement me
    
    kprintf("[i] Highest unavailable address is %x.\n", HighestUnavailable);
    physmem_init(HighestUnavailable);
}

u64 system_get_memory_upper(void)
{
    return g_SystemInfo.MemoryUpper;
}

u64 system_get_memory_lower(void)
{
   return g_SystemInfo.MemoryLower;
}

s8 *system_get_bootloader_name(void)
{
    return g_SystemInfo.BootloaderName;
}

u8 system_memory_available(u64 Start, u64 Length)
{
    for (u8 i = 0; i < g_SystemInfo.NumInvalidAreas; i++)
    {
        T_SYSTEM_INVALID_RAM Area = g_SystemInfo.InvalidMemoryAreas[i];

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

