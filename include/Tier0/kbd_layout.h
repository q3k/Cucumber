#ifndef __KBD_LAYOUT_H__
#define __KBD_LAYOUT_H__

#include "types.h"

enum E_KBD_SPECIAL {
    E_KBD_ESC,
    E_KBD_ENTER,
    E_KBD_LCTRL,
    E_KBD_RCTRL,
    E_KBD_LSHIFT,
    E_KBD_RSHIFT,
    E_KBD_LALT,
    E_KBD_RALT,
} typedef T_KBD_KEY;

typedef u8 * T_KBD_LAYOUT;

void kbd_layout_set(T_KBD_LAYOUT Layout, u8 Length);
void kbd_layout_set_default(void);
s8 kbd_layout_translate(u8 Key, u8 Special);

#endif
