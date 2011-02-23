#include "Tier0/acpi.h"
#include "Tier0/kstdio.h"

u8 g_acpi_version = 0;
u32 g_acpi_rsdt_address = 0;

u32 acpi_find_rsdp(void)
{
    s8 *szMagic = "RSD PTR ";

    // Try to find the pointer... apparently it's 16byte-aligned...
    
    u32 Address = 0;
    for (u32 Search = 0x000E0000; Search <= 0x000FFFFF; Search += 4)
    {
        if (kmemcmp((u8 *)Search, (u8 *)szMagic, 8) == 0)
        {
            u8 ChecksumVerify = 0;
            for (u8 i = 0; i < ACPI_10_RSDP_SIZE; i++)
                ChecksumVerify += ((u8*)Search)[i];
            
            if (ChecksumVerify == 0)
            {            
                if (((T_ACPI_RSDP *)Search)->Revision == 1)
                {
                    ChecksumVerify = 0;
                    for (u8 i = ACPI_10_RSDP_SIZE; i < ACPI_20_RSDP_SIZE; i++)
                        ChecksumVerify += ((u8*)Search)[i];
                    
                    if (ChecksumVerify == 0)
                    {
                        Address = Search;
                        break;
                    }
                }
                else
                {
                    Address = Search;
                    break;
                }
            }
        }
    }

    if (Address == 0)
        return 0;
    
    T_ACPI_RSDP *RSDP = (T_ACPI_RSDP *)Address;
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
