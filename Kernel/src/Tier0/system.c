// Basic information gathere about system

#include "types.h"
#include "Tier0/system.h"
#include "Tier0/panic.h"
#include "Tier0/kstdio.h"
#include "Tier0/physmem.h"

T_SYSTEM_INFO g_SystemInfo;

extern u64 _end;
extern u64 _start;


#define NOTIFY_ABOUT_FEATURE(f) if (g_SystemInfo.CPUFeatures.Flags.f) \
    kprintf(" " #f);

void system_parse_cpu_features(void)
{
    g_SystemInfo.CPUFeatures.FlagsU64 = system_cpuid(1);
    
    kprintf("[i] CPU features:");
    
    NOTIFY_ABOUT_FEATURE(ACPI);
    NOTIFY_ABOUT_FEATURE(AES);
    NOTIFY_ABOUT_FEATURE(APIC);
    NOTIFY_ABOUT_FEATURE(FPU);
    NOTIFY_ABOUT_FEATURE(IA64);
    NOTIFY_ABOUT_FEATURE(MMX);
    NOTIFY_ABOUT_FEATURE(MSR);
    NOTIFY_ABOUT_FEATURE(PAE);
    NOTIFY_ABOUT_FEATURE(SSE);
    NOTIFY_ABOUT_FEATURE(SSE2);
    NOTIFY_ABOUT_FEATURE(SSE3);
    NOTIFY_ABOUT_FEATURE(SSSE3);
    NOTIFY_ABOUT_FEATURE(SSE4_1);
    NOTIFY_ABOUT_FEATURE(SSE4_2);
    
    kprintf("\n");
}

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
                
                kprintf("[i] Unavailable memory: %x - %x\n", Node->Base, Node->Base + Node->Length);

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
    T_SYSTEM_INVALID_RAM *BIOSArea = &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];
    BIOSArea->Base = 0;
    BIOSArea->Size = 1024 *1024;
    g_SystemInfo.NumInvalidAreas++;
    
    // And mark our kernel physical location as unavailable
    T_SYSTEM_INVALID_RAM *KernelArea = &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];
    KernelArea->Base = LoadContext->KernelPhysicalStart;
    KernelArea->Size = LoadContext->KernelPhysicalEnd - LoadContext->KernelPhysicalStart;
    g_SystemInfo.NumInvalidAreas++;
    
    // ...and the loader physical location.
    T_SYSTEM_INVALID_RAM *LoaderArea = &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];
    LoaderArea->Base = LoadContext->LoaderPhysicalStart;
    LoaderArea->Size = LoadContext->LoaderPhysicalEnd - LoadContext->LoaderPhysicalStart;
    g_SystemInfo.NumInvalidAreas++;
    
    // ...and the IOAPIC 
    T_SYSTEM_INVALID_RAM *IOAPICArea = &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];
    IOAPICArea->Base = 0xFEC00000;
    IOAPICArea->Size = 0xFECFFFFF - 0xFEC00000;
    g_SystemInfo.NumInvalidAreas++;
    
    /// ...and the LAPIC
    T_SYSTEM_INVALID_RAM *LAPICArea = &g_SystemInfo.InvalidMemoryAreas[g_SystemInfo.NumInvalidAreas];
    LAPICArea->Base = 0xFEE00000;
    LAPICArea->Size = 0xFEEFFFFF - 0xFEE00000;
    g_SystemInfo.NumInvalidAreas++;
    
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
        if (Area.Base >= Start && Start + Length > Area.Base + Area.Size)
            return 0;
    }
    return 1;
}

u64 system_cpuid(u32 Code)
{
    u32 Low;
    u32 High;
    __asm__ volatile ("cpuid" : "=a"(Low), "=d"(High) : "0"(Code) : "ecx", "ebx");
    
    return (u64)High | ((u64)Low << 32);
}

u8 system_msr_available(void)
{
    return CPUID_HAS(MSR);
}

u64 system_msr_get(u32 MSR)
{
    u32 Low;
    u32 High;
    __asm__ volatile("rdmsr" : "=a"(Low), "=d"(High) : "c"(MSR));
    
    return (u64)Low | ((u64)High << 32);
}

void system_msr_set(u32 MSR, u64 Data)
{
    __asm__ volatile("wrmsr" :: "a"((u32)(Data & 0xFFFFFFFF)), "d"((u32)(Data >> 32)), "c"(MSR));
}
