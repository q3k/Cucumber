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
    GDT_EXECUTABLE = 1
} T_GDT_EXECUTABLE;

typedef enum E_GDT_RW {
    GDT_NOT_RW = 0,
    GDT_RW = 1
} T_GDT_RW;

struct S_GDT_SEGMENT {
    u16 LimitLow;
    u16 BaseLow;
    u8 BaseMiddle;
    u8 SegmentType     : 4;
    u8 DescriptorType  : 1;
    u8 DPL             : 2;
    u8 Present         : 1;
    u8 LimitMiddle     : 4;
    u8 SystemAvailable : 1;
    u8 LongMode        : 1;
    u8 OperationSize   : 1;
    u8 Granularity     : 1;
    u8 BaseHigh;
} __attribute__((packed));
typedef struct S_GDT_SEGMENT T_GDT_SEGMENT;

struct S_GDT_PTR {
    u16 Size;
    u64 Address;
} __attribute__((packed));
typedef struct S_GDT_PTR T_GDT_PTR;

void gdt_entry_create(u8 Index, u32 Base, u32 Limit, T_GDT_RING DPL, \
        T_GDT_EXECUTABLE Executable, T_GDT_RW ReadWrite, u8 Long);
void gdt_create_flat(void);

//From gdt.asm
void gdt_flush(void);

#endif
