#include "Tier0/gdt.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdio.h"
#include "types.h"

// Our GDT will be stored here.
T_GDT_PTR g_GDTPointer;
T_GDT_SEGMENT g_GDTEntries[5];


void gdt_entry_create(u8 Index, u32 Base, u32 Limit, T_GDT_RING DPL, \
        T_GDT_EXECUTABLE Code, T_GDT_RW ReadWrite, u8 Long)
{
    g_GDTEntries[Index].LimitLow = Limit & 0xFFFF;
    g_GDTEntries[Index].BaseLow = Base & 0xFFFF;
    g_GDTEntries[Index].BaseMiddle = (Base >> 16) & 0xFF;
    
    g_GDTEntries[Index].SegmentType  = ((u8)Code << 3);
    g_GDTEntries[Index].SegmentType |= ((u8)ReadWrite << 1);
    
    g_GDTEntries[Index].DescriptorType = 1;
    g_GDTEntries[Index].DPL = DPL;
    g_GDTEntries[Index].Present = 1;
    
    g_GDTEntries[Index].LimitMiddle = Limit >> 16;
    g_GDTEntries[Index].SystemAvailable = 0;
    g_GDTEntries[Index].LongMode = Long;
    g_GDTEntries[Index].OperationSize = !Long;
    g_GDTEntries[Index].Granularity = 1;
    g_GDTEntries[Index].BaseHigh = Base >> 24;
}

void gdt_entry_create_null(u8 Index)
{
    kmemset(&g_GDTEntries[Index], 0, 8);
}

void gdt_create_flat(void)
{
    g_GDTPointer.Size = sizeof(T_GDT_SEGMENT) * 5 - 1;
    g_GDTPointer.Address = (u64)&g_GDTEntries;

    gdt_entry_create_null(0);
    gdt_entry_create(1, 0, 0xFFFFFFFF, GDT_RING0, GDT_EXECUTABLE, GDT_RW, 0);
    gdt_entry_create(2, 0, 0xFFFFFFFF, GDT_RING0, GDT_NOT_EXECUTABLE, GDT_RW, 0);
    gdt_entry_create(3, 0, 0xFFFFFFFF, GDT_RING0, GDT_EXECUTABLE, GDT_RW, 1);
    gdt_entry_create(4, 0, 0xFFFFFFFF, GDT_RING0, GDT_NOT_EXECUTABLE, GDT_RW, 1);
    
    kprintf("[i] Setting GDT:\n");
    for (u32 i = 0; i < 5; i++)
    {
        kprintf("%x", *((u32 *)g_GDTEntries + 2 * i + 1));
        kprintf("%x\n", *((u32 *)g_GDTEntries + 2 * i));
    }

//    gdt_flush();
}

