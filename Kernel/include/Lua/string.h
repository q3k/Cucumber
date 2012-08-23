#ifndef __TIER0_ANSI_STRING_H__
#define __TIER0_ANSI_STRING_H__

#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"

const char *strchr (const char *str, int character);
inline int strcmp(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);

const char *strchr (const char *str, int character)
{
    while (*str != 0)
    {
        if (*str == character)
            return str;
        str++;
    }
    return 0;
}
inline int strcmp(const char *s1, const char *s2)
{
    while((*s1 && *s2) && (*s1++ == *s2++));
    return *(--s1) - *(--s2);
}

char *strpbrk(const char *s1, const char *s2)
{
    while(*s1)
        if(strchr(s2, *s1++))
            return (char*)--s1;
    return 0;
}

#define strlen(A) kstrlen(A)
#define memcpy(A, B, C) kmemcpy(A, B, C)
#define memcmp(A, B, C) kmemcmp((const u8*)A, (const u8*)B, C)
#endif
