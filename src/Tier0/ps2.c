// A Tier0-mode keyboard driver... this is gonna get ignored when we het (some
// day) into Tier1...

#include "types.h"
#include "Tier0/ps2.h"
#include "Tier0/kstdio.h"
#include "Tier0/interrupts.h"

#define PS2_KBD_IRQ 
#define PS2_KBD_DATA 0x60

void ps2_keyboard_isr(void)
{
    u8 ScanCode = kinb(PS2_KBD_DATA);

    kprintf("[i] Scancode: %u.\n", ScanCode);

    interrupts_irq_finish(0x01);
    return;
}

void ps2_init_simple(void)
{
    interrupts_setup_irq(0x01, ps2_keyboard_isr);
    interrupts_dump_idt_entry(0xF1);
}
