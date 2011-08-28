#ifndef __SMP_H__
#define __SMP_H__

#include "types.h"

// OS structures
typedef enum {
    E_SMP_CPU_STATE_DISABLED,
    E_SMP_CPU_STATE_HALTED,
    E_SMP_CPU_STATE_IDLE,
    E_SMP_CPU_STATE_RUNNING
} E_SMP_CPU_STATE;

typedef struct {
    u8 ID;
    u32 CPUID;
    
    u8 LAPICID;
    E_SMP_CPU_STATE State;
    
    u8 Bootstrap : 1;
} T_SMP_CPU;

// BIOS-provided Structures
typedef struct {
    s8 Signature[4];
    u32 TablePhysical;
    u8 Length;
    u8 Specification;
    u8 Checksum;
} __attribute__((packed)) T_SMP_POINTER;

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
} __attribute__((packed)) T_SMP_CONFIGURATION_HEADER;

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
} __attribute__((packed)) T_SMP_ENTRY_CPU;

u64 smp_find_pointer(u64 Start, u64 End);
void smp_initialize(void);
void smp_parse_configuration_table(u32 TableAddress);

#endif
