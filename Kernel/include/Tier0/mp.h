#ifndef __MP_H__
#define __MP_H__

#include "types.h"

typedef struct {
    s8 Signature[4];
    u32 TablePhysical;
    u8 Length;
    u8 Specification;
    u8 Checksum;
} __attribute__((packed)) T_MP_POINTER;

typedef struct {
    u8 Signature[4];
    u16 BaseTableLength;
    u8 Specification;
    u8 Checksum;
    u8 OEMName[8];
    u8 ProductName[12];
    u32 OEMTablePointer;
    u16 OEMTableSize;
    u16 NumEntries;
    u32 LAPICAddress;
    u16 ExtendedTableLength;
    u8 ExtendedTableChecksum;
    u8 Reserved;
} __attribute__((packed)) T_MP_CONFIGURATION_HEADER;

typedef struct {
    u8 EntryType;
    u8 LAPICID;
    u8 LAPICVersion;
    u8 FlagAvailable : 1;
    u8 FlagBootstrap : 1;
    u8 FlagReserved  : 6;
    u32 Signature;
    u32 CPUID;
    u32 Reserved1;
    u32 Reserved2;
} __attribute__((packed)) T_MP_ENTRY_CPU;

u64 mp_find_pointer(u64 Start, u64 End);
void mp_initialize(void);
void mp_parse_configuration_table(u32 TableAddress);

#endif
