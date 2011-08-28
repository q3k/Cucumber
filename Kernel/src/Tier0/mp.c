#include "Tier0/mp.h"
#include "Tier0/paging.h"
#include "Tier0/physmem.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

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
        // do something.
    }
    else
    {
        kprintf("[i] MP Configuration Type present.\n");
        // do something else
    }
}
