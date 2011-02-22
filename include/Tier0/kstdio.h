#ifndef __KSTDIO_H__
#define __KSTDIO_H__

#include "types.h"

void koutb(u16 Port, u8 Data);
u8 kinb(u16 Port);
void kio_wait(void);
u32 kstrlen(s8 *szString);
void kmove_cursor(u8 X, u8 Y);
void kputs(s8 *szString);
void kputch(s8 Character);
void kclear(void);
void kprint(s8 *szString);
void kputi(s32 Number);
void kprintf(s8 *Format, ...);
void kdump(u8 *bData, u32 Length);
void kprint_hex(u32 Number);
s32 kmemcmp(u8 *MemA, u8 *MemB, u32 Length);

#endif
