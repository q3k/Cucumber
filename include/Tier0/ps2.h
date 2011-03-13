#ifndef __PS2_H__
#define __PS2_H__

u8 ps2_wait_key(void);
u8 ps2_poll_key(void);
void ps2_keyboard_isr(void);
void ps2_init_simple(void);

#endif
