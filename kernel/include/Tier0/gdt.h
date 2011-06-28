#ifndef __GDT_H__
#define __GDT_H__

#include "types.h"

typedef enum E_GDT_RING {
    GDT_RING0 = 0,
    GDT_RING1,
    GDT_RING2,
    GDT_RING3 
} T_GDT_RING;

typedef enum E_GDT_EXECUTABLE {
    GDT_NOT_EXECUTABLE = 0,
    GDT_EXECUTABLE
} T_GDT_EXECUTABLE;

typedef enum E_GDT_RW {
    GDT_NOT_RW = 0,
    GDT_RW
} T_GDT_RW;

struct S_GDT_ENTRY {
    u16 LimitLow;
    u16 BaseLow;
    u8 BaseMiddle;
    u8 Access;
    u8 Granularity;
    u8 BaseHigh;
} __attribute__((packed));
typedef struct S_GDT_ENTRY T_GDT_ENTRY;

struct S_GDT_PTR {
    u16 Size;
    u32 Address;
} __attribute__((packed));
typedef struct S_GDT_PTR T_GDT_PTR;

void gdt_entry_create(u8 Index, u32 Base, u32 Limit, T_GDT_RING Ring, \
        T_GDT_EXECUTABLE Executable, T_GDT_RW ReadWrite);
void gdt_create_flat(void);

//From gdt.asm
void gdt_flush(void);

#endif
