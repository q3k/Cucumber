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

void update_load_context(T_LOAD_CONTEXT* Context)
{
    Context->VGACurrentLine = stdio_current_line;
    Context->VGACursorX = stdio_cur_x;
    Context->VGACursorY = stdio_cur_y;
}