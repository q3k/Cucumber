#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/semaphore.h"
#include <stdarg.h>

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

u8 g_kstdio_current_line = 0;
u8 g_kstdio_cur_x = 0, g_kstdio_cur_y = 0;

T_SEMAPHORE ScreenWriteLock;

void kstdio_init(void)
{
    semaphore_init(&ScreenWriteLock);
}

void koutb(u16 Port, u8 Data)
{
    __asm__ volatile("outb %1, %0" :: "dN" (Port), "a" (Data));
}

u8 kinb(u16 Port)
{
    u8 Return;
    __asm__ volatile("inb %1, %0" :"=a"(Return):"Nd"(Port));
    return Return;
}

void kio_wait(void)
{
    __asm__ volatile("jmp 1f;1:jmp 1f;1:");
}

void kputi(s32 Number)
{
    s32 Sign, i;
    
    if ((Sign = Number) < 0)
        Number = -Number;

    u8 szString[11];

    i = 0;
    do {
        szString[i++] = Number % 10 + '0';
    } while (( Number /= 10) > 0);

    if (Sign < 0)
        szString[i] = '-';
    else
        i--;
    
    for (s32 j = i; j >= 0; j--)
    {
        kputch(szString[j]);
    }
}

void kprintf(const s8 *szFormat, ...)
{
    //semaphore_acquire(&ScreenWriteLock);
    va_list ap;
    va_start(ap, szFormat);
    
    u32 Offset = 0;
    while (Offset < kstrlen(szFormat))
    {
        if (szFormat[Offset] == '%')
        {
            switch (szFormat[Offset + 1])
            {
                case '%':
                    kputch('%');
                    break;
                case 'c':
                    kputch(va_arg(ap, u32));
                    break;
                case 's':
                    kputs(va_arg(ap, s8*));
                    break;
                case 'i':
                    kputi(va_arg(ap, s32));
                    break;
                case 'u':
                    kputi(va_arg(ap, u32));
                    break;
                case 'm': //dump
                    ; u8 *szString = va_arg(ap, u8*); //stupid gcc bug
                    kdump(szString, kstrlen((s8 *)szString));
                    break;
                case 'X':
                case 'x':
                    ; u32 bData = va_arg(ap, u32);
                    kprint_hex(bData);
                    break;
                default:
                    kprintf("printf: Unknown escape character %c!\n", szFormat[Offset + 1]);
            }
            Offset += 2;
        }
        else
        {
            kputch(szFormat[Offset]);
            Offset++;
        }
    }
    
    va_end(ap);
    //semaphore_release(&ScreenWriteLock);
}

void kscroll_up(void)
{
   //semaphore_acquire(&ScreenWriteLock);
   u16 Blank = 0x20 | (0x0F << 8);
   u16 Temp;

   if (g_kstdio_cur_y >= 25)
   {
        Temp = g_kstdio_cur_y - 25 + 1;
        kmemcpy((void*)0xC00B8000, (void*)(0xC00B8000 + Temp * 80 * 2), (25 - Temp) * 80 * 2);

        kmemsetw((void*)(0xC00B8000 + (25 - Temp) * 160), Blank, 160);
        g_kstdio_cur_y = 25 - 1;
   }
   //semaphore_release(&ScreenWriteLock);
}

u32 kstrlen(const s8 *szString)
{
    const s8 *s;
    for (s = szString; *s; ++s)
    {}
    return s - szString;
}

void kmove_cursor(u8 X, u8 Y)
{
    g_kstdio_cur_x = X;
    g_kstdio_cur_y = Y;

    //wraparound
    if (g_kstdio_cur_x >= 80)
    {
        g_kstdio_cur_y += g_kstdio_cur_y / 80;
        g_kstdio_cur_x = 0;
    }

    //wrapup
    kscroll_up();

    if (Y > 24)
        Y = 24;

    u16 Position = Y * 80 + X;
    
    koutb(0x3D4, 0x0F);
    koutb(0x3D5, (u8)(Position & 0xFF));
    
    koutb(0x3D4, 0x0E);
    koutb(0x3D5, (u8)(Position >> 8 & 0xFF));
}

void kdump_nibble(u8 Nibble)
{
    if (Nibble < 10)
        kputch(Nibble + 48);
    else
        kputch(Nibble + 55);
}

void kprint_hex(u32 Number)
{
    for (s8 i = 3; i >= 0; i--)
    {
        u8 Byte = (Number >> (i << 3)) & 0xFF; //switch i bytes to the right and mask as byte
        kdump_nibble((Byte >> 4)  & 0x0F); //high nibble
        kdump_nibble(Byte & 0x0F); //low nibble
    }
}

void kdump(u8 *bData, u32 Length)
{
    for (u32 i = 0; i < Length; i++)
    {
        u8 Low = bData[i] & 0x0F;
        u8 High = (bData[i] >> 4) & 0x0F;

        kdump_nibble(High);
        kdump_nibble(Low);
    }
}

void kputch(s8 Character)
{
    semaphore_acquire(&ScreenWriteLock);
    volatile u8 *VideoMemory = (u8 *)0xC00B8000;
    u16 Offset = (g_kstdio_cur_y * 80 + g_kstdio_cur_x) << 1;

    if (Character == '\n')
        kmove_cursor(0, g_kstdio_cur_y + 1);
    else
    {
        VideoMemory[Offset] = Character;
        VideoMemory[Offset+1] = 0x0F;
        if (g_kstdio_cur_x + 1 >= 80)
            kmove_cursor(0, g_kstdio_cur_y + 1);
        else
            kmove_cursor(g_kstdio_cur_x + 1, g_kstdio_cur_y);
    }
    semaphore_release(&ScreenWriteLock);
}

void kputs(const s8 *szString)
{
    while (*szString != 0)
    {
        kputch(*szString);
        szString++;
    }
}

void kprint(const s8 *szString)
{
    kputs(szString);
}

void kclear(void)
{
    volatile u8 *VideoMemory = (u8 *)0xC00B8000;
    u32 Size = (80 * 25 ) << 1;
    for (u32 i = 0; i < Size; i += 2)
    {
        VideoMemory[i] = 0;
        VideoMemory[i+1] = 0xF;
    }
    kmove_cursor(0, 0);
}

s32 kmemcmp(u8 *MemA, u8 *MemB, u32 Length)
{
    u32 Result = -1;
    for (u32 Search = 0; Search < Length; Search++)
    {
        if (MemA[Search] != MemB[Search])
        {
            Result = Search;
            break;
        }
    }
    return Result;
}
