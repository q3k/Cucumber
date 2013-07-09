#include "io.h"
#include "types.h"

u8 stdio_current_line = 0;
u8 stdio_cur_x = 0, stdio_cur_y = 0;

void outb(u16 Port, u8 Data)
{
    __asm__ volatile("outb %1, %0" :: "dN" (Port), "a" (Data));
}

void *memcpy(void* Destination, const void *Source, u32 Count)
{
    u8* Destination8 = (u8*)Destination;
    u8* Source8 = (u8*)Source;

    while (Count--)
    {
        *Destination8++ = *Source8++;
    }
    return Destination;
}

void *memset(void *Destination, u8 Value, u32 Count)
{
    u8 *us = (u8 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}

void *memsetw(void *Destination, u16 Value, u32 Count)
{
    u16 *us = (u16 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}


void scroll_up(void)
{
   //semaphore_acquire(&ScreenWriteLock);
   u16 Blank = 0x20 | (0x0F << 8);
   u16 Temp;

   if (stdio_cur_y >= 25)
   {
        Temp = stdio_cur_y - 25 + 1;
        memcpy((void*)0xB8000, (void*)(0xB8000 + Temp * 80 * 2), (25 - Temp) * 80 * 2);

        memsetw((void*)(0xB8000 + (25 - Temp) * 160), Blank, 160);
        stdio_cur_y = 25 - 1;
   }
   //semaphore_release(&ScreenWriteLock);
}

void move_cursor(u8 X, u8 Y)
{
    stdio_cur_x = X;
    stdio_cur_y = Y;

    //wraparound
    if (stdio_cur_x >= 80)
    {
        stdio_cur_y += stdio_cur_y / 80;
        stdio_cur_x = 0;
    }

    //wrapup
    scroll_up();

    if (Y > 24)
        Y = 24;

    u16 Position = Y * 80 + X;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(Position & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)(Position >> 8 & 0xFF));
}


void puti(s32 Number)
{
    s32 Sign, i;
    
    if ((Sign = Number) < 0)
        Number = -Number;

    u8 szString[21];

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
        putch(szString[j]);
    }
}

void putch(s8 Character)
{
    volatile u8 *VideoMemory = (u8 *)0xB8000;
    u16 Offset = (stdio_cur_y * 80 + stdio_cur_x) << 1;

    if (Character == '\n')
        move_cursor(0, stdio_cur_y + 1);
    else
    {
        VideoMemory[Offset] = Character;
        VideoMemory[Offset+1] = 0x0F;
        if (stdio_cur_x + 1 >= 80)
            move_cursor(0, stdio_cur_y + 1);
        else
            move_cursor(stdio_cur_x + 1, stdio_cur_y);
    }
}

void puts(const s8 *szString)
{
    while (*szString != 0)
    {
        putch(*szString);
        szString++;
    }
}

void clear(void)
{
    volatile u8 *VideoMemory = (u8 *)0xB8000;
    u32 Size = (80 * 25 ) << 1;
    for (u32 i = 0; i < Size; i += 2)
    {
        VideoMemory[i] = 0;
        VideoMemory[i+1] = 0xF;
    }
    move_cursor(0, 0);
}

void dump_nibble(u8 Nibble)
{
    if (Nibble < 10)
        putch(Nibble + 48);
    else
        putch(Nibble + 55);
}

void print_hex_32(u32 Number)
{
    for (s8 i = 3; i >= 0; i--)
    {
        u8 Byte = (Number >> (i << 3)) & 0xFF; //switch i bytes to the right and mask as byte
        dump_nibble((Byte >> 4)  & 0x0F); //high nibble
        dump_nibble(Byte & 0x0F); //low nibble
    }
}

void print_hex(u64 Number)
{
    for (s8 i = 7; i >= 0; i--)
    {
        u8 Byte = (Number >> (i << 3)) & 0xFF; //switch i bytes to the right and mask as byte
        dump_nibble((Byte >> 4)  & 0x0F); //high nibble
        dump_nibble(Byte & 0x0F); //low nibble
    }
}

void io_update_load_context(T_LOAD_CONTEXT* Context)
{
    Context->VGACurrentLine = stdio_current_line;
    Context->VGACursorX = stdio_cur_x;
    Context->VGACursorY = stdio_cur_y;
}

u32 strlen(const s8 *szString)
{
    const s8 *s;
    for (s = szString; *s; ++s)
    {}
    return s - szString;
}

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

void printf(const s8 *szFormat, ...)
{
    va_list ap;
    va_start(ap, szFormat);
    
    u32 Offset = 0;
    while (Offset < strlen(szFormat))
    {
        if (szFormat[Offset] == '%')
        {
            switch (szFormat[Offset + 1])
            {
                case '%':
                    putch('%');
                    break;
                case 'c':
                    putch(va_arg(ap, u32));
                    break;
                case 's':
                    puts(va_arg(ap, s8*));
                    break;
                case 'i':
                    puti(va_arg(ap, s64));
                    break;
                case 'u':
                    puti(va_arg(ap, u64));
                    break;
                case 'X':
                    {
                        u64 bData = va_arg(ap, u64);
                        print_hex(bData);
                        break;
                    }
                case 'x':
                    {
                        u32 bData = va_arg(ap, u32);
                        print_hex_32(bData);
                        break;
                    }
                default:
                    printf("printf: Unknown escape character %c!\n", szFormat[Offset + 1]);
            }
            Offset += 2;
        }
        else
        {
            putch(szFormat[Offset]);
            Offset++;
        }
    }
    
    va_end(ap);
}