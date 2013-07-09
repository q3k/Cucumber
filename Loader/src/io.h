#ifndef __IO_H__
#define __IO_H__

#include "types.h"
#include "context.h"

void outb(u16 Port, u8 Data);
void *memcpy(void* Destination, const void *Source, u32 Count);
void *memset(void *Destination, u8 Value, u32 Count);
void *memsetw(void *Destination, u16 Value, u32 Count);
void scroll_up(void);
void move_cursor(u8 X, u8 Y);
void putch(s8 Character);
void puts(const s8 *szString);
void clear(void);
void dump_nibble(u8 Nibble);
void print_hex_32(u32 Number);
void print_hex(u64 Number);
void io_update_load_context(T_LOAD_CONTEXT *Context);
void printf(const s8 *szFormat, ...);

#endif