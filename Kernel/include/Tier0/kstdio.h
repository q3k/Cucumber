#ifndef __KSTDIO_H__
#define __KSTDIO_H__

#include "types.h"

#define LOOPFOREVER for(;;){}

void kstdio_init(void);
void koutb(u16 Port, u8 Data);
u8 kinb(u16 Port);
void koutl(u16 Port, u32 Data);
u32 kinl(u16 Port);
void kio_wait(void);
u32 kstrlen(const s8 *szString);
void kmove_cursor(u8 X, u8 Y);
void kputs(const s8 *szString);
void kputch(const s8 Character);
void kclear(void);
void kprint(const s8 *szString);
void kputi(s64 Number);
void kprintf(const s8 *Format, ...);
void kdump(u8 *bData, u32 Length);
void kprint_hex(u64 Number);
void kprint_hex_16(u16 Number);
void kstdio_set_globals(u8 line, u8 cur_x, u8 cur_y);
s32 kmemcmp(const u8 *MemA, const u8 *MemB, u32 Length);

#endif
