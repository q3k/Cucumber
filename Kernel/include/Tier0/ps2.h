#ifndef __PS2_H__
#define __PS2_H__

#define PS2_SPECIAL_CTRL 1
#define PS2_SPECIAL_ALT 2
#define PS2_SPECIAL_SHIFT 4
#define PS2_SPECIAL_META 8
#define PS2_SPECIAL_HYPER 16

s8 ps2_getc(void);
u8 ps2_wait_key(void);
u8 ps2_poll_key(void);
void ps2_keyboard_isr(void);
void ps2_init_simple(void);

#endif
