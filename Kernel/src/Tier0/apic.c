#include "Tier0/apic.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/paging.h"

struct {
    T_APIC_LAPIC *APIC;
} g_APIC;

void apic_enable_lapic(void)
{
    u64 APICMSR = system_msr_get(0x1B);
    
    if (APICMSR & (1 << 11))
        kprintf("[i] APIC enable flag set in APIC MSR.\n");
    else
    {
        kprintf("[i] APIC enable flag not set in APIC MSR. Trying to set it...\n");
        APICMSR |= (1 << 22);
        system_msr_set(0x1B, APICMSR);
    }
    
    u64 Virtual = paging_minivmm_allocate();
    kprintf("[i] LAPIC will be @0x%x.\n", Virtual);
    paging_map_page(Virtual, 0xFEE00000);
    
    g_APIC.APIC = (T_APIC_LAPIC *)Virtual;
}
