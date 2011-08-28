#include "Tier0/mp.h"
#include "Tier0/paging.h"
#include "Tier0/physmem.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"

#define MP_TEMP_BUFFER_LENGTH 1024
u8 g_EntriesBuffer[MP_TEMP_BUFFER_LENGTH];

#define MP_MAX_CPU 32

struct {
    T_MP_CPU CPUs[MP_MAX_CPU];
    u8 NumCPUs;
} g_MP;

u64 mp_find_pointer(u64 Start, u64 End)
{
    for (u64 i = Start & ~((u64)0x10); i < End; i += 16)
    {
        u8 Pointer[16];
        physmem_read(i, 16, Pointer);
        
        if (kmemcmp(Pointer, (u8 *)"_MP_", 4) < 0)
        {
            // Found something! Let's calculate the checksum, just to be sure.
            u8 Sum = 0;
            for (u8 j = 0; j < 16; j++)
                Sum += Pointer[j];
            
            if (Sum)
            {
                kprintf("[w] Found MP pointer, but its checksum was invalid.");
                continue;
            }
            else
            {
                kprintf("[i] Found MP pointer @0x%x\n", i);
                return i;
            }
        }
    }
    
    return 0;
}

void mp_initialize(void)
{
    kprintf("[i] Looking for MP Pointer:\n");
    // According to the Intel spec, we must look for the MP Pointer in four
    // places: 
    
    // EBDA
    u32 EBDAStart;
    physmem_read(0x40e, 2, &EBDAStart);
    EBDAStart = EBDAStart << 4;
    
    kprintf("    EBDA @0x%x.\n", EBDAStart);
    
    u64 Pointer = mp_find_pointer(EBDAStart, 0x0009FFFF);
    
    // Last kilobyte of base memory
    if (!Pointer)
    {
        u32 BaseMemoryEnd;
        physmem_read(0x413, 2, &BaseMemoryEnd);
        kprintf("    Base memory end @0x%x.\n", BaseMemoryEnd - 1024);
        
        Pointer = mp_find_pointer(BaseMemoryEnd - 1024, BaseMemoryEnd);
    }
    
    // BIOS ROM
    if (!Pointer)
    {
        kprintf("    BIOS ROM @0x00E00000.\n");        
        Pointer = mp_find_pointer(0x000F0000, 0x000FFFFF);
    }
    
    // Just give up already.
    if (!Pointer)
        PANIC("No MP pointer found! Boo.");
    
    T_MP_POINTER PointerTable;
    
    physmem_read(Pointer, 16, &PointerTable);
    
    if (PointerTable.Specification != 4)
        PANIC("Unsupported MP spec!");
    
    if (PointerTable.TablePhysical)
    {
        kprintf("[i] MP Configuration Table present.\n");
        mp_parse_configuration_table(PointerTable.TablePhysical);
    }
    else
    {
        kprintf("[i] MP Configuration Type present.\n");
        // do something else... like panic.
        PANIC("MP types not implemented!");
    }
}

u8 mp_parse_cpu(u32 EntryAddress)
{
    u8 i = g_MP.NumCPUs;
    T_MP_ENTRY_CPU *CPU = (T_MP_ENTRY_CPU *)&g_EntriesBuffer[EntryAddress];
    
    if (CPU->FlagBootstrap)
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x (bootstrap)\n", i, CPU->LAPICID, CPU->CPUID);
        g_MP.CPUs[i].Bootstrap = 1;
        g_MP.CPUs[i].State = E_MP_CPU_STATE_RUNNING;
    }
    else if (!CPU->FlagAvailable)
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x, (UNAVAILABLE!!!)", i, CPU->LAPICID, CPU->CPUID);
        g_MP.CPUs[i].Bootstrap = 0;
        g_MP.CPUs[i].State = E_MP_CPU_STATE_DISABLED;
    }
    else
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x (halted)\n", i, CPU->LAPICID, CPU->CPUID);
        g_MP.CPUs[i].Bootstrap = 0;
        g_MP.CPUs[i].State = E_MP_CPU_STATE_HALTED;
    }
    
    g_MP.CPUs[i].ID = i;
    g_MP.CPUs[i].CPUID = CPU->CPUID;
    g_MP.CPUs[i].LAPICID = CPU->LAPICID;
    
    g_MP.NumCPUs++;
    
    return sizeof(T_MP_ENTRY_CPU);
}

void mp_parse_configuration_table(u32 TableAddress)
{
    T_MP_CONFIGURATION_HEADER Header;
    physmem_read(TableAddress, 44, &Header);
    
    s8 OEMName[9];
    kmemcpy(OEMName, Header.OEMName, 8);
    OEMName[8] = 0x00;
    
    s8 ProductName[13];
    kmemcpy(ProductName, Header.ProductName, 12);
    ProductName[12] = 0x00;
    
    kprintf("[i] MP OEM: %s\n", OEMName);
    kprintf("[i] MP Product: %s\n", ProductName);
    
    kprintf("[i] MP Base Configuration Table length: %i bytes.\n", Header.BaseTableLength);
    
    if (Header.BaseTableLength > MP_TEMP_BUFFER_LENGTH)
        PANIC("MP BCT too big!");
    
    physmem_read(TableAddress + 44, Header.BaseTableLength - 44, g_EntriesBuffer);
    
    u32 EntryAddress = 0;
    g_MP.NumCPUs = 0;
    while (1)
    {
        u8 EntryType = g_EntriesBuffer[EntryAddress];
        switch (EntryType)
        {
            case 0x00:
                EntryAddress += mp_parse_cpu(EntryAddress);
                break;
            default:
                for (;;) {}
        }
    }
}
