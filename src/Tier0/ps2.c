// A Tier0-mode keyboard driver... this is gonna get ignored when we het (some
// day) into Tier1...

#include "types.h"
#include "Tier0/ps2.h"
#include "Tier0/kstdio.h"
#include "Tier0/interrupts.h"

#define PS2_KBD_IRQ 
#define PS2_KBD_DATA 0x60

#define PS2_KEY_SHIFT 42
#define PS2_KEY_DEPRESSED(x) ((x >= 128) ? 1 : 0)
#define PS2_D_TO_P(x) (x - 128)
#define PS2_P_TO_D(x) (x | 128)

u8 ps2_shift_pressed = 0;
u8 ps2_ctrl_pressed = 0;
u8 ps2_key_pressed = 0;
u8 ps2_key = 0;

u8 ps2_key_new = 0;

u8 ps2_poll_key(void)
{
    if (ps2_key_pressed)
        return ps2_key;
    return 0;
}

u8 ps2_wait_key(void)
{
    while (!ps2_key_pressed && !ps2_key_new)
    {}
    ps2_key_new = 0;
    return ps2_key;
}

void _ps2_keyboard_key_pressed(u8 Code)
{
    if (Code == PS2_KEY_SHIFT)
        ps2_shift_pressed = 1;
    else if (ps2_key_pressed == 0)
    {
        ps2_key_pressed = 1;
        ps2_key = Code;
        ps2_key_new = 1;
    }
}

void _ps2_keyboard_key_depressed(u8 Code)
{
    if (Code == PS2_KEY_SHIFT)
        ps2_shift_pressed = 0;
    else if (ps2_key_pressed == 1 && Code == ps2_key)
        ps2_key_pressed = 0;
}

void ps2_keyboard_isr(void)
{
    u8 ScanCode = kinb(PS2_KBD_DATA);

    if (PS2_KEY_DEPRESSED(ScanCode))
        _ps2_keyboard_key_depressed(PS2_D_TO_P(ScanCode));
    else
        _ps2_keyboard_key_pressed(ScanCode);

    interrupts_irq_finish(0x01);
    return;
}

void ps2_init_simple(void)
{
    interrupts_setup_irq(0x01, ps2_keyboard_isr);
    interrupts_dump_idt_entry(0xF1);
}
