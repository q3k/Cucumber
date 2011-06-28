// 8259 Programmable interrupt controller routines...
// The usual computer will switch to IOAPIC later on, once ACPI is fully
// initialized and the driver/module system is active.

// I could probably try to do without interrupts until I get in C++-land but
// you never know - also, paging without interrupts is pointless, especiallly
// if I want a good heap implementation, which is more-or-less indispensable
// if I want to ever reach Tier1 in a sane way.

// tl;dr - This code isn't going to be running all the time

#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/pic.h"

#define PIC_COMMAND_EOI 0x20

// Initialisation "words".
#define PIC_ICW1_INIT 0x10
#define PIC_ICW1_ICW4 0x01
#define PIC_ICW4_8086 0x01

void pic_init(u8 *OldMask1, u8 *OldMask2)
{
    if (OldMask1 != 0)
        *OldMask1 = kinb(PIC_1_DATA);

    if (OldMask2 != 0)
        *OldMask2 = kinb(PIC_2_DATA);

    koutb(PIC_1_ADDR, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    kio_wait();

    koutb(PIC_2_ADDR, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    kio_wait();

    // ICW2
    koutb(PIC_1_DATA, PIC_1_START);
    kio_wait();

    koutb(PIC_2_DATA, PIC_2_START);
    kio_wait();

    // ICW3
    koutb(PIC_1_DATA, 4);
    kio_wait();

    koutb(PIC_2_DATA, 2);
    kio_wait();

    // ICW4
    koutb(PIC_1_DATA, PIC_ICW4_8086);
    kio_wait();

    koutb(PIC_2_DATA, PIC_ICW4_8086);
    kio_wait();

    //We don't support ANYTHING
    koutb(PIC_1_DATA, 0xFF);
    koutb(PIC_2_DATA, 0xFF);
}

void pic_eoi(u8 IRQ)
{
    if (IRQ > 7)
        koutb(PIC_2_ADDR, PIC_COMMAND_EOI);

    koutb(PIC_1_ADDR, PIC_COMMAND_EOI);
}

void pic_unmask_irq(u8 IRQ)
{
    if (IRQ > 7)
    {
        u8 Bit = 0;
        Bit |= 1 << (IRQ - 8);

        u8 CurrentMask = kinb(PIC_2_DATA);
        u8 NewMask = ~((~CurrentMask) | Bit);

        koutb(PIC_2_DATA, NewMask);
    }
    else
    {
        u8 Bit = 0;
        Bit |= 1 << (IRQ);

        u8 CurrentMask = kinb(PIC_1_DATA);
        u8 NewMask = ~((~CurrentMask) | Bit);
        koutb(PIC_1_DATA, NewMask);
    }
}
