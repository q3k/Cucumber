#ifndef __PIC_H__
#define __PIC_H__

#include "types.h"

#define PIC_1_ADDR 0x20
#define PIC_1_DATA 0x21
#define PIC_1_START 0xF0

#define PIC_2_ADDR 0xA0
#define PIC_2_DATA 0xA1
#define PIC_2_START 0xF8

#define PIC_IRQ_START 0xF0

void pic_init(u8 *OldMask1, u8 *OldMask2);
void pic_eoi(u8 IRQ);
void pic_unmask_irq(u8 IRQ);

#endif
