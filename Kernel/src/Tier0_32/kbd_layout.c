#include "Tier0/kbd_layout.h"
#include "Tier0/ps2.h"

u8 g_kbd_layout_default[] = {
    // 0x00
    0xFF, 0xFF,
    0xFF, 0xFF,
    '1', '!',
    '2', '@',
    '3', '#',
    '4', '$',
    '5', '%',
    '6', '^',
    '7', '&',
    '8', '*',
    '9', '(',
    '0', ')',
    '-', '_',
    '=', '+', 
    0xFF, 0xFF,
    ' ', ' ',
    
    // 0x10
    'q', 'Q',
    'w', 'W',
    'e', 'E',
    'r', 'R',
    't', 'T',
    'y', 'Y',
    'u', 'U',
    'i', 'I',
    'o', 'O',
    'p', 'P',
    '[', '{',
    ']', '}',
    '\n', '\n',
    0xFF, 0xFF,  
    'a', 'A',
    's', 'S',
    
    // 0x20
    'd', 'D',
    'f', 'F',
    'g', 'G',
    'h', 'H',
    'j', 'J',
    'k', 'K',
    'l', 'L',
    ';', ':',
    '\'', '"',
    '`', '~',
    0xFF, 0xFF,
    '\\', '|',
    'z', 'Z',
    'x', 'X',
    'c', 'C',
    'v', 'V',
    
    // 0x30
    'b', 'B',
    'n', 'N',
    'm', 'M',
    ',', '<',
    '.', '>',
    '/', '?',
    0xFF, 0xFF,
    '*', '*',
    0xFF, 0xFF,
    ' ', ' ',
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    
    // 0x40
    0xFF,  0xFF,
    0xFF,  0xFF,
    0xFF,  0xFF,
    0xFF,  0xFF,
    0xFF,  0xFF,
    0xFF,  0xFF,
    0xFF,  0xFF,
    '7', '7',
    '8', '8',
    '9', '9',
    '-', '-',
    '4', '4',
    '5', '5',
    '6', '6',
    '+', '+',
    '1', '1',
    
    // 0x50
    '2', '2',
    '3', '3',
    '0', '0',
    '.', '.',
    0xFF, 0xFF
};

T_KBD_LAYOUT g_kbd_layout_current;
u8 g_kbd_layout_current_length;

void kbd_layout_set(T_KBD_LAYOUT Layout, u8 Length)
{
    g_kbd_layout_current = Layout;
    g_kbd_layout_current_length = Length;
}

void kbd_layout_set_default(void)
{
    kbd_layout_set(g_kbd_layout_default, 0x54 * 2);
}

s8 kbd_layout_translate(u8 Key, u8 Special)
{
    if (Key > g_kbd_layout_current_length)
        return 0x00;
    
    u8 Shift = (Special & PS2_SPECIAL_SHIFT) > 0;
    s8 Mapped = g_kbd_layout_current[Key * 2 + Shift];
    
    return (Mapped == 0xFF ? 0x00 : Mapped);
}

