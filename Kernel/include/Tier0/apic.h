#ifndef __APIC_H__
#define __APIC_H__

#include "types.h"
#include "interrupts.h"

// To write to the IMCR (interrupt mode configuration register), you need two
// IO ports: APIC_IMCR_CTRL, and APIC_IMCR_DATA. First you output APIC_IMCR_SEL
// to _CTRL, and then you can write to _DATA. The possible values are:
//  o 0x00 - Default, bypass the APIC
//  o 0x01 - Pass NMI and 8259A interrupts through the APIC

#define APIC_IMCR_CTRL 0x22
#define APIC_IMCR_DATA 0x23

#define APIC_IMCR_SEL 0x70

void apic_enable_lapic(void);
void apic_eoi(void);
void apic_periodic(u64 Frequency, void (*Callback)(T_ISR_REGISTERS));

#define APIC_ID 0x0020
#define APIC_Version 0x0030
#define APIC_TPR 0x0080
#define APIC_APR 0x0090
#define APIC_PPR 0x00A0
#define APIC_EOI 0x00B0
#define APIC_LDR 0x00D0
#define APIC_DFR 0x00E0
#define APIC_SVR 0x00F0
#define APIC_ISR 0x0100
#define APIC_TMR 0x0180
#define APIC_IRR 0x0200
#define APIC_ESR 0x0280
#define APIC_ICR1 0x0300
#define APIC_ICR2 0x0310
#define APIC_LVTTimer 0x0320
#define APIC_LVTThermal 0x0330
#define APIC_LVTPerformanceCounter 0x0340
#define APIC_LVTLINT0 0x0350
#define APIC_LVTLINT1 0x0360
#define APIC_LVTError 0x0370
#define APIC_TimerICR 0x0380
#define APIC_TimerCCR 0x0390
#define APIC_TimerDCR 0x03E0

// BY THE POWER OF MACROS
#define APIC_ISR_GET(a, bit) ((*((u32*)(a + APIC_ISR + ((bit / 32) * 16))) >> (bit % 32)) & 1)
#define APIC_TMR_GET(a, bit) ((*((u32*)(a + APIC_TMR + ((bit / 32) * 16))) >> (bit % 32)) & 1)
#define APIC_IRR_GET(a, bit) ((*((u32*)(a + APIC_IRR + ((bit / 32) * 16))) >> (bit % 32)) & 1)

#endif
