// A Tier0-mode keyboard driver... this is gonna get ignored when we het (some
// day) into Tier1...

#include "types.h"
#include "Tier0/ps2.h"
#include "Tier0/kstdio.h"
#include "Tier0/interrupts.h"
#include "Tier0/kbd_layout.h"

#define PS2_KBD_IRQ 
#define PS2_KBD_DATA 0x60

#define PS2_KEY_SHIFT 42
#define PS2_KEY_DEPRESSED(x) ((x >= 128) ? 1 : 0)
#define PS2_D_TO_P(x) (x - 128)
#define PS2_P_TO_D(x) (x | 128)

u8 ps2_special = 0;
u8 ps2_key_pressed = 0;
u8 ps2_key = 0;

u8 ps2_key_new = 0;

s8 ps2_getc(void)
{
    while(1)
    {
        u8 Key = ps2_wait_key();
        s8 Character = kbd_layout_translate(Key, ps2_special);
        
        if (Character != 0)
            return Character;
    }
}

u8 ps2_poll_key(void)
{
    if (ps2_key_pressed)
        return ps2_key;
    return 0;
}

u8 ps2_wait_key(void)
{
    while(1)
    {
        if (ps2_key_pressed && ps2_key_new)
            break;
    }
    ps2_key_new = 0;
    return ps2_key;
}

void _ps2_keyboard_key_pressed(u8 Code)
{
    if (Code == PS2_KEY_SHIFT)
        ps2_special |= PS2_SPECIAL_SHIFT;
    else// if (ps2_key_pressed == 0)
    {
        ps2_key_pressed = 1;
        ps2_key = Code;
        ps2_key_new = 1;
    }
}

void _ps2_keyboard_key_depressed(u8 Code)
{
    if (Code == PS2_KEY_SHIFT)
        ps2_special &= ~PS2_SPECIAL_SHIFT;
    else if (Code == ps2_key)
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
}
