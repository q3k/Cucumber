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

typedef struct {
    u8 ID;
    u32 Address;
} T_SMP_IOAPIC;

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

typedef struct {
    u8 EntryType;
    u8 BusID;
    u8 BusType[6];
} __attribute__((packed)) T_SMP_ENTRY_BUS;

typedef struct {
    u8 EntryType;
    u8 ID;
    u8 Version;
    u8 Available : 1;
    u8 Reserved  : 7;
    u32 Address;
} __attribute__((packed)) T_SMP_ENTRY_IOAPIC;

typedef enum {
    E_SMP_INTERRUPT_TYPE_INT = 0,
    E_SMP_INTERRUPT_TYPE_NMI = 1,
    E_SMP_INTERRUPT_TYPE_SMI = 2,
    E_SMP_INTERRUPT_TYPE_ExtINT = 3
} E_SMP_INTERRUPT_TYPE;

typedef enum {
    E_SMP_POLARITY_BUS = 0,
    E_SMP_POLARITY_HIGH = 1,
    E_SMP_POLARITY_RESERVED = 2,
    E_SMP_POLARITY_LOW = 3
} E_SMP_POLARITY;

typedef enum {
    E_SMP_TRIGGER_MODE_BUS = 0,
    E_SMP_TRIGGER_MODE_EDGE = 1,
    E_SMP_TRIGGER_MODE_RESERVED = 2,
    E_SMP_TRIGGER_MODE_LEVEL = 3
} E_SMP_TRIGGER_MODE;

typedef struct {
    u8 EntryType;
    E_SMP_INTERRUPT_TYPE InterruptType : 8;
    E_SMP_POLARITY Polarity : 2;
    E_SMP_TRIGGER_MODE TriggerMode : 2;
    u16 Reserved : 12;
    u8 SourceBusID;
    u8 SourceBusIRQ;
    u8 DestinationIOAPICID;
    u8 DestinationIOAPICINTIN;
}  __attribute__((packed)) T_SMP_ENTRY_IO_INTERRUPT;

typedef struct {
    u8 EntryType;
    E_SMP_INTERRUPT_TYPE InterruptType : 8; //hack?
    E_SMP_POLARITY Polarity : 2;
    E_SMP_TRIGGER_MODE TriggerMode : 2;
    u16 Reserved : 12;
    u8 SourceBusID;
    u8 SourceBusIRQ;
    u8 DestinationLAPICID;
    u8 DestinationLAPICLINTIN;
} __attribute__((packed)) T_SMP_ENTRY_LOCAL_INTERRUPT;

u64 smp_find_pointer(u64 Start, u64 End);
void smp_initialize(void);
void smp_parse_configuration_table(u32 TableAddress);

#endif
