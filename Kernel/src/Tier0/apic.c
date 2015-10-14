#include "Tier0/apic.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/paging.h"
#include "Tier0/interrupts.h"
#include "Tier0/panic.h"

#define APIC_SET32(field, value) do { *((volatile u32 *)(g_APIC.LAPIC + field)) = (value);} while(0)
#define APIC_GET32(field) (*((volatile u32*)(g_APIC.LAPIC + field)))
#define APIC_SET32A(field, value) do { APIC_SET32(field, value); ASSERT(APIC_GET32(field) == (value));} while(0)

#define APIC_CALIBRATION_SAMPLES 8
#define APIC_TIMER_INTERRUPT 200
struct {
    volatile void *LAPIC;
    volatile u32 CalibrationCounter;
    volatile u32 CalibrationSamples[APIC_CALIBRATION_SAMPLES];
    u32 TicksPerSecond;
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
    apic_eoi();
}

void apic_calibration_interrupt(T_ISR_REGISTERS Registers)
{
    u32 Sample = APIC_GET32(APIC_TimerCCR);
    if (g_APIC.CalibrationCounter >= APIC_CALIBRATION_SAMPLES)
    {
        koutb(0x20, 0x20);
        return;
    }
    g_APIC.CalibrationSamples[g_APIC.CalibrationCounter] = Sample;
    g_APIC.CalibrationCounter++;

    // is the APIC timer 0? well shit.
    if (Sample == 0)
        PANIC("APIC Calibration look reached null timer value. Too many saples?");

    koutb(0x20, 0x20);
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
    
    paging_map_page(0xFEE00000, 0xFEE00000, 0);
    g_APIC.LAPIC = (void*)0xFEE00000;
    kprintf("[i] LAPIC will be @0x%x.\n", g_APIC.LAPIC);

    // prepare interrupts ..
    interrupts_setup_isr(39, (void *)apic_spurious_interrupt, E_INTERRUPTS_RING0);
    interrupts_setup_isr(APIC_TIMER_INTERRUPT, (void *)apic_timer_interrupt, E_INTERRUPTS_RING0);

    // reset APIC to a kinda known state
    APIC_SET32(APIC_DFR, 0xFFFFFFFF);
    APIC_SET32(APIC_LDR, (APIC_GET32(APIC_LDR)&0x00FFFFFF)|1);
    APIC_SET32A(APIC_LVTTimer, 0x10000); // mask bit
    APIC_SET32A(APIC_LVTPerformanceCounter, 4 << 8); // NMI bit
    APIC_SET32A(APIC_LVTLINT0, 0x10000); // mask vit
    APIC_SET32A(APIC_LVTLINT1, 0x10000); // mask bit
    APIC_SET32A(APIC_TPR, 0); // task priority = 0 (accept all)

    APIC_SET32A(APIC_SVR, 39 | 0x100); // spurious interrupt + sw enable bit
    APIC_SET32A(APIC_LVTTimer, 0x1000); // mask bit
    APIC_SET32A(APIC_TimerDCR, 0x03); // timer divider = bus speed / 16

    kprintf("[i] LAPIC ready to calibrate timer.\n");

    // calibration time! let's use the PIT
    interrupts_setup_isr(32, (void *)apic_calibration_interrupt, E_INTERRUPTS_RING0);
    // let's unmask IRQ0 only
    koutb(0x21, 0xFE);
    koutb(0xA1, 0xFF);

    // let's make LINT0 trigger an extINT (yay PIC-time)
    APIC_SET32(APIC_LVTLINT0, 0x8700);

    // let's start the APIC timer and zero the calibration counter
    APIC_SET32(APIC_TimerICR, 0xFFFFFFFF);
    g_APIC.CalibrationCounter = 0;

    // now let's program the PIT to run a channel0 (IRQ0) timer with a 1000Hz loop
    koutb(0x43, 0x36);
    koutb(0x40, 0x9C);
    koutb(0x40, 0x2E);

    // the timer is now running!
    while (g_APIC.CalibrationCounter < APIC_CALIBRATION_SAMPLES) {};

    // enough samples, let's clean up
    kprintf("[i] Calibration loop finished.\n");
    koutb(0x21, 0xFF);
    koutb(0x21, 0xFF);
    APIC_SET32(APIC_LVTLINT0, 0x1000); // mask bit
    APIC_SET32(APIC_LVTTimer, 0x10000); // mask bit
    interrupts_setup_isr(32, (void *)apic_timer_interrupt, E_INTERRUPTS_RING0);

    u64 Average = 0;
    for (u32 i = 0; i < APIC_CALIBRATION_SAMPLES -1; i++)
        Average += (g_APIC.CalibrationSamples[i] - g_APIC.CalibrationSamples[i + 1]);

    Average /= (APIC_CALIBRATION_SAMPLES - 1);
    g_APIC.TicksPerSecond = (Average * 100);
    kprintf("[i] APIC is running at %i ticks per second.\n", g_APIC.TicksPerSecond);
}

void apic_periodic(u64 Frequency, void (*Callback)(T_ISR_REGISTERS))
{
    u64 Counter = g_APIC.TicksPerSecond / Frequency;
    if (Counter > 0xFFFFFFFF) {
        PANIC("Can't fit frequency in counter range!");
    }
    // Set divider to 1/1th of bus speed
    //APIC_SET32A(APIC_TimerDCR, 0b1011);
    
    // Setup callback ISR
    interrupts_setup_isr(APIC_TIMER_INTERRUPT, Callback, E_INTERRUPTS_RING0);
    // Enable periodic timer in LVT on ISR 0x98
    APIC_SET32A(APIC_LVTTimer, 0x20000 | APIC_TIMER_INTERRUPT);
    // Set initial counter to counter (timer should now be running)
    APIC_SET32A(APIC_TimerICR, Counter);
}

#undef APIC_SET
