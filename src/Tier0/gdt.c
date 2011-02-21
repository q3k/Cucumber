#include "Tier0/gdt.h"
#include "Tier0/kstdlib.h"
#include "types.h"

// Our GDT will be stored here.
T_GDT_PTR g_gdt_ptr;
T_GDT_ENTRY g_gdt_entries[3];


void gdt_entry_create(u8 Index, u32 Base, u32 Limit, T_GDT_RING Ring, \
        T_GDT_EXECUTABLE Executable, T_GDT_RW ReadWrite)
{
    g_gdt_entries[Index].BaseLow = (Base & 0xFFFF);
    g_gdt_entries[Index].BaseMiddle = ((Base >> 16) & 0xFF);
    g_gdt_entries[Index].BaseHigh = ((Base >> 24) & 0xFF);

    g_gdt_entries[Index].LimitLow = (Limit & 0xFFFF);
    g_gdt_entries[Index].Granularity = ((Limit >> 16) & 0x0F);

    // Let's set the granularity and size nibble to 0xC0
    g_gdt_entries[Index].Granularity |= 0xC0;
    
    g_gdt_entries[Index].Access = 0b10010000;
    g_gdt_entries[Index].Access |= ((((u8)ReadWrite)  << 1) & 0b00000010);
    g_gdt_entries[Index].Access |= ((((u8)Executable) << 3) & 0b00001000);
    g_gdt_entries[Index].Access |= ((((u8)Ring)       << 5) & 0b01100000);
}

void gdt_entry_create_null(u8 Index)
{
    kmemsetw(&g_gdt_entries[Index], 0, 4);
}

void gdt_create_flat(void)
{
    g_gdt_ptr.Size = sizeof(T_GDT_ENTRY) * 6 - 1;
    g_gdt_ptr.Address = (u32)&g_gdt_entries;

    gdt_entry_create_null(0);
    gdt_entry_create(1, 0, 0xFFFFFFFF, GDT_RING0, GDT_EXECUTABLE, GDT_RW);
    gdt_entry_create(2, 0, 0xFFFFFFFF, GDT_RING0, GDT_NOT_EXECUTABLE, GDT_RW);

    gdt_flush();
}

