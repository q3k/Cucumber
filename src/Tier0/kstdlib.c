#include "Types.h"
#include "kstdlib.h"

void *kmemcpy(void* Destination, const void *Source, u32 Count)
{
    u8* Destination8 = (u8*)Destination;
    u8* Source8 = (u8*)Source;

    while (Count--)
    {
        *Destination8++ = *Source8++;
    }
    return Destination;
}

void *kmemset(void *Destination, u8 Value, u32 Count)
{
    u8 *us = (u8 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}

void *kmemsetw(void *Destination, u16 Value, u32 Count)
{
    u16 *us = (u16 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}
