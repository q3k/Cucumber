#include "Tier0/acpi.h"
#include "Tier0/kstdio.h"

u8 g_acpi_version = 0;
u32 g_acpi_rsdt_address = 0;

u32 acpi_find_rsdp(void)
{
    s8 *szMagic = "RSD PTR ";

    // Try to find the pointer... apparently it's 16byte-aligned...
    
    kprintf("[i] Looking for the RSDP...\n");
    u32 Address = 0;
    for (u32 Search = 0x000E0000; Search <= 0x000FFFFF; Search += 4)
    {
        if (kmemcmp((u8 *)Search, (u8 *)szMagic, 8) == 0)
        {
            Address = Search;
            break;
        }
    }

    if (Address == 0)
        return 0;

    T_ACPI_RSDP *RSDP = (T_ACPI_RSDP *)Address;
    
    u8 ChecksumVerify = 0;
    for (u8 i = 0; i < ACPI_10_RSDP_SIZE; i++)
        ChecksumVerify += ((u8*)Address)[i];

    if (ChecksumVerify != 0)
    {
        kprintf("[e] ACPI checksum failed. Memory failure? Weird PC? We may never " \
                "know ...\n");
        return 0;
    }

    if (RSDP->Revision == 1)
    {
        // ACPI 2.0...
        ChecksumVerify = 0;
        for (u8 i = ACPI_10_RSDP_SIZE; i < ACPI_20_RSDP_SIZE; i++)
            ChecksumVerify += ((u8*)Address)[i];

        if (ChecksumVerify != 0)
        {
            kprintf("[e] ACPI 2.0 checksum failed. Yet 1.0 succeeded? WTF.\n");
            return 0;
        }
    }

    g_acpi_version = RSDP->Revision + 1;

    kprintf("[i] Detected ACPI version %i.0.\n", RSDP->Revision + 1);
    if (*RSDP->OEMID != 0)
        kprintf("[i] OEMID %s.\n", RSDP->OEMID);

    if (g_acpi_version == ACPI_VERSION_10)
        g_acpi_rsdt_address = Address;
    else
        g_acpi_rsdt_address = RSDP->RSDTAddressExLow;
    
    return Address;
}
