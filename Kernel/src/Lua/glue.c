// Glue code for Lua <-> Tier0

#include <stdarg.h>

#include "setjmp.h"
#include "Tier0/panic.h"
#include "Tier0/kstdio.h"

int errno;

// math.h implementation
//double floor(double X)
//{
//    return (double)((int) X);
//}

//double pow(double B, double E)
//{
//    PANIC("pow() stub.");
//    return 0;
//}

//double ldexp(double A, int B)
//{
//    PANIC("ldexp() stub.");
//    return 0;
//}

// setjmp.h implementation
int setjmp(jmp_buf env)
{
    PANIC("setjmp() stub.");
    return 0;
}

void longjmp(jmp_buf env, int value)
{
    PANIC("longjmp() stub.");
}

// stdio.h implementation
//int printf(char *format, va_list args)
//{
//    kprintf("lua :");
//
//    va_list duplicate_args;
//    va_copy(duplicate_args, args);
//    int result = kprintf(format, duplicate_args);
//    va_end(duplicate_args);
//
//    return result;
//}

// stdlib.h implementation
void *malloc(unsigned long long int i)
{
    PANIC("malloc() stub.");
    return 0;
}

int abs(int X)
{
    return (X > 0) ? X : (X * -1);
}

void abort(void)
{
    PANIC("abort() stub.");
}

// string.h implementation
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
int strcmp(const char *s1, const char *s2)
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


