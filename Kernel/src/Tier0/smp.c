#include "Tier0/smp.h"
#include "Tier0/paging.h"
#include "Tier0/physmem.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/panic.h"

#define SMP_TESMP_BUFFER_LENGTH 1024
u8 g_EntriesBuffer[SMP_TESMP_BUFFER_LENGTH];

#define SMP_MAX_CPU 32
#define SMP_MAX_IOAPIC 9

struct {
    T_SMP_CPU CPUs[SMP_MAX_CPU];
    u8 NumCPUs;
    
    T_SMP_IOAPIC IOAPICs[SMP_MAX_IOAPIC];
    u8 NumIOAPICs;
} g_SMP;

u64 smp_find_pointer(u64 Start, u64 End)
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
                kprintf("[w] Found SMP pointer, but its checksum was invalid.");
                continue;
            }
            else
            {
                kprintf("[i] Found SMP pointer @0x%x\n", i);
                return i;
            }
        }
    }
    
    return 0;
}

void smp_initialize(void)
{
    kprintf("[i] Looking for SMP Pointer:\n");
    // According to the Intel spec, we must look for the SMP Pointer in four
    // places: 
    
    // EBDA
    u32 EBDAStart;
    physmem_read(0x40e, 2, &EBDAStart);
    EBDAStart = EBDAStart << 4;
    
    kprintf("    EBDA @0x%x.\n", EBDAStart);
    
    u64 Pointer = smp_find_pointer(EBDAStart, 0x0009FFFF);
    
    // Last kilobyte of base memory
    if (!Pointer)
    {
        u32 BaseMemoryEnd;
        physmem_read(0x413, 2, &BaseMemoryEnd);
        kprintf("    Base memory end @0x%x.\n", BaseMemoryEnd - 1024);
        
        Pointer = smp_find_pointer(BaseMemoryEnd - 1024, BaseMemoryEnd);
    }
    
    // BIOS ROM
    if (!Pointer)
    {
        kprintf("    BIOS ROM @0x00E00000.\n");        
        Pointer = smp_find_pointer(0x000F0000, 0x000FFFFF);
    }
    
    // Just give up already.
    if (!Pointer)
        PANIC("No SMP pointer found! Boo.");
    
    T_SMP_POINTER PointerTable;
    
    physmem_read(Pointer, 16, &PointerTable);
    
    if (PointerTable.Specification != 4)
        PANIC("Unsupported SMP spec!");
    
    if (PointerTable.TablePhysical)
    {
        kprintf("[i] SMP Configuration Table present.\n");
        smp_parse_configuration_table(PointerTable.TablePhysical);
    }
    else
    {
        kprintf("[i] SMP Configuration Type present.\n");
        // do something else... like panic.
        PANIC("SMP types not ismplemented!");
    }
}

u8 smp_parse_io_interrupt(u32 EntryAddress)
{
    //T_SMP_ENTRY_IO_INTERRUPT *Interrupt = (T_SMP_ENTRY_IO_INTERRUPT *)&g_EntriesBuffer[EntryAddress];
    
    //kprintf("    IO Interrupt %i %i %i %i %i %i %i\n", Interrupt->InterruptType, Interrupt->Polarity, Interrupt->TriggerMode, Interrupt->SourceBusID, Interrupt->SourceBusIRQ, Interrupt->DestinationIOAPICID, Interrupt->DestinationIOAPICINTIN);
    
    return 8;
}

u8 smp_parse_local_interrupt(u32 EntryAddress)
{
    //T_SMP_ENTRY_LOCAL_INTERRUPT *Interrupt = (T_SMP_ENTRY_LOCAL_INTERRUPT *)&g_EntriesBuffer[EntryAddress];
    
    //kprintf("    L  Interrupt %i %i %i %i %i %i %i\n", Interrupt->InterruptType, Interrupt->Polarity, Interrupt->TriggerMode, Interrupt->SourceBusID, Interrupt->SourceBusIRQ, Interrupt->DestinationLAPICID, Interrupt->DestinationLAPICLINTIN);
    
    return 8;
}

u8 smp_parse_ioapic(u32 EntryAddress)
{
    T_SMP_ENTRY_IOAPIC *IOAPIC  = (T_SMP_ENTRY_IOAPIC *)&g_EntriesBuffer[EntryAddress];
    
    if (IOAPIC->Available)
    {
        g_SMP.IOAPICs[g_SMP.NumIOAPICs].ID = IOAPIC->ID;
        g_SMP.IOAPICs[g_SMP.NumIOAPICs].Address = IOAPIC->Address;
        
        kprintf("    IOAPIC ID %i, @0x%x\n", IOAPIC->ID, IOAPIC->Address);
    }
    else
        kprintf("    IOAPIC ID %i, unavailable!!!", IOAPIC->ID);
    
    return 8;
}

u8 smp_parse_bus(u32 EntryAddress)
{
    T_SMP_ENTRY_BUS *Bus = (T_SMP_ENTRY_BUS *)&g_EntriesBuffer[EntryAddress];
    
    s8 Name[7];
    kmemcpy(Name, Bus->BusType, 6);
    Name[6] = 0x00;
    
    kprintf("    Bus #%i, %s\n", Bus->BusID, Name);
    
    return 8;
}

u8 smp_parse_cpu(u32 EntryAddress)
{
    u8 i = g_SMP.NumCPUs;
    T_SMP_ENTRY_CPU *CPU = (T_SMP_ENTRY_CPU *)&g_EntriesBuffer[EntryAddress];
    
    if (CPU->FlagBootstrap)
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x (bootstrap)\n", i, CPU->LAPICID, CPU->CPUID);
        g_SMP.CPUs[i].Bootstrap = 1;
        g_SMP.CPUs[i].State = E_SMP_CPU_STATE_RUNNING;
    }
    else if (!CPU->FlagAvailable)
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x, (unavailable", i, CPU->LAPICID, CPU->CPUID);
        g_SMP.CPUs[i].Bootstrap = 0;
        g_SMP.CPUs[i].State = E_SMP_CPU_STATE_DISABLED;
    }
    else
    {
        kprintf("    CPU #%i, LAPIC sig %x, cpuid %x (halted)\n", i, CPU->LAPICID, CPU->CPUID);
        g_SMP.CPUs[i].Bootstrap = 0;
        g_SMP.CPUs[i].State = E_SMP_CPU_STATE_HALTED;
    }
    
    g_SMP.CPUs[i].ID = i;
    g_SMP.CPUs[i].CPUID = CPU->CPUID;
    g_SMP.CPUs[i].LAPICID = CPU->LAPICID;
    
    g_SMP.NumCPUs++;
    
    return sizeof(T_SMP_ENTRY_CPU);
}

void smp_parse_configuration_table(u32 TableAddress)
{
    T_SMP_CONFIGURATION_HEADER Header;
    physmem_read(TableAddress, 44, &Header);
    
    s8 OEMName[9];
    kmemcpy(OEMName, Header.OEMName, 8);
    OEMName[8] = 0x00;
    
    s8 ProductName[13];
    kmemcpy(ProductName, Header.ProductName, 12);
    ProductName[12] = 0x00;
    
    kprintf("[i] SMP OEM: %s\n", OEMName);
    kprintf("[i] SMP Product: %s\n", ProductName);
    
    kprintf("[i] SMP Base Configuration Table length: %i bytes.\n", Header.BaseTableLength);
    
    if (Header.BaseTableLength > SMP_TESMP_BUFFER_LENGTH)
        PANIC("SMP BCT too big!");
    
    physmem_read(TableAddress + 44, Header.BaseTableLength - 44, g_EntriesBuffer);
    
    u32 EntryAddress = 0;
    g_SMP.NumCPUs = 0;
    g_SMP.NumIOAPICs = 0;
    
    while (EntryAddress < Header.BaseTableLength - sizeof(T_SMP_CONFIGURATION_HEADER))
    {
        u8 EntryType = g_EntriesBuffer[EntryAddress];
        switch (EntryType)
        {
            case 0x00:
                EntryAddress += smp_parse_cpu(EntryAddress);
                break;
            case 0x01:
                EntryAddress += smp_parse_bus(EntryAddress);
                break;
            case 0x02:
                EntryAddress += smp_parse_ioapic(EntryAddress);
                break;
            case 0x03:
                EntryAddress += smp_parse_io_interrupt(EntryAddress);
                break;
            case 0x04:
                EntryAddress += smp_parse_local_interrupt(EntryAddress);
                break;
        }
    }
}
