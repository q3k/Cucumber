#include "types.h"

#ifndef __ACPI_H__
#define __ACPI_H__

#define ACPI_VERSION_UNK 0
#define ACPI_VERSION_10 1
#define ACPI_VERSION_20 2

#define ACPI_10_RSDP_SIZE 20
#define ACPI_20_RSDP_SIZE 16

struct S_ACPI_RSDP {
    s8 Magic[8];
    u8 Checksum;
    s8 OEMID[6];
    u8 Revision;
    u32 RSDTAddress;

    // Additional ACPI 2.0 fields
    u32 Length;
    u32 RSDTAddressExLow;
    u32 RSDTAddressExHigh;
    u8 ChecksumEx;
    u8 Reserved[3];
} __attribute__((packed));
typedef struct S_ACPI_RSDP T_ACPI_RSDP;

struct S_ACPI_SDT_HEADER {
    s8 Signature[4];
    u32 Length;
    u8 Revision;
    u8 Checksum;
    s8 OEMID[6];
    s8 OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
} __attribute__ ((packed));
typedef struct S_ACPI_SDT_HEADER T_ACPI_SDT_HEADER;

u32 acpi_find_rsdp(void);

#endif
