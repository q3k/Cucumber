#include "Tier0/apic.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/paging.h"
#include "Tier0/interrupts.h"
#include "Tier0/panic.h"

#define APIC_SET32(field, value) do { *((u32 *)(g_APIC.LAPIC + field)) = (value);} while(0)
#define APIC_GET32(field) (*((u32*)(g_APIC.LAPIC + field)))
#define APIC_SET32A(field, value) do { APIC_SET32(field, value); ASSERT(APIC_GET32(field) == (value));} while(0)

struct {
    void *LAPIC;
} g_APIC;

void apic_eoi(void)
{
    APIC_SET32(APIC_EOI, 0xFFFFFFFF);
}

void apic_spurious_interrupt(T_ISR_REGISTERS Registers)
{

}

void apic_timer_interrupt(T_ISR_REGISTERS Registers)
{
    kprintf("ohai\n");
}


void apic_enable_lapic(void)
{
    u64 APICMSR = system_msr_get(0x1B);
    
    if (APICMSR & (1 << 11))
        kprintf("[i] APIC enable flag set in APIC MSR.\n");
    else
    {
        kprintf("[i] APIC enable flag not set in APIC MSR. Trying to set it...\n");
        APICMSR |= (1 << 11);
        system_msr_set(0x1B, APICMSR);
    }
    
    u64 Virtual = paging_minivmm_allocate();
    kprintf("[i] LAPIC will be @0x%x.\n", Virtual);
    paging_map_page(Virtual, 0xFEE00000);

    g_APIC.LAPIC = (void *)Virtual;

    // reset APIC to a kinda known state
    APIC_SET32A(APIC_DFR, 0xFFFFFFFF);
    APIC_SET32(APIC_LDR, (APIC_GET32(APIC_LDR)&0x00FFFFFF)|1);
    APIC_SET32A(APIC_LVTTimer, 0x10000); // mask bit
    APIC_SET32A(APIC_LVTPerformanceCounter, 4 << 8); // NMI bit
    APIC_SET32A(APIC_LVTLINT0, 0x10000); // mask bit
    APIC_SET32A(APIC_LVTLINT1, 0x10000); // mask bit
    APIC_SET32A(APIC_TPR, 0); // task priority = 0 (accept all)

    // prepare interrupts ..
    interrupts_setup_isr(39, (void *)apic_spurious_interrupt, E_INTERRUPTS_RING0);
    interrupts_setup_isr(32, (void *)apic_timer_interrupt, E_INTERRUPTS_RING0);

    APIC_SET32A(APIC_SVR, 39 | 0x100); // spurious interrupt + sw enable bit
    APIC_SET32A(APIC_LVTTimer, 32); // apic timer interrupt
    APIC_SET32A(APIC_TimerDCR, 0x03); // timer divider = bus speed / 16

    kprintf("[i] LAPIC ready to calibrate timer.\n");

    // calibration time! let's use the PIT
    koutb(0x61, kinb(0x61) | 0xFD); // disable speaker
    koutb(0x43, 0xB2); // one shot, channel 2

    // to setup a 100Hz loop, wee need a divider of...
    // 1193180 / 100 Hz = 11931 = 0x2e9b
    koutb(0x42, 0x9b); // write low byte
    kinb(0x60); // wait a bit
    koutb(0x42, 0x2e); // write high bte

    // now we are ready to fire the PIT timer
    kprintf("[i] Firing PIT for calibration...\n");
    u8 Temporary = kinb(0x61) & 0xFE;
    koutb(0x61, Temporary); // gate low
    koutb(0x61, Temporary | 1); // gate high
    // don't forget to actually fire the APIC timer at the same time...
    APIC_SET32(APIC_TimerICR, 0xFFFFFFFF);
    // for(;;)
    // {
    //     kprintf("APIC: %x\n", APIC_GET32(APIC_TimerICR));
    // }

    // wait for the PIT counter to reach zero...
    while (!(kinb(0x61) & 0x020));
    // disable APIC timer
    APIC_SET32(APIC_LVTTimer, 0x10000); // mask bit
    kprintf("[i] Timer reached zero, APIC Timer @%x.\n", APIC_GET32(APIC_TimerICR));
}

#undef APIC_SET