// Glue code for Lua <-> Tier0

#include <stdarg.h>

#include "setjmp.h"
#include "stdio.h"
#include "Tier0/panic.h"
#include "Tier0/kstdio.h"
#include "Tier0/heap.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdio.h"

int errno;
#define NULL 0

// file descriptors for stdio...
FILE _stdin = 0;
FILE _stdout = 1;
FILE _stderr = 2;
FILE *stdin = &_stdin;
FILE *stdout = &_stderr;
FILE *stderr = &_stderr;

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
    //PANIC("setjmp() stub.");
    return 0;
}

void longjmp(jmp_buf env, int value)
{
    PANIC("longjmp() stub.");
}

// stdio.h implementation
// from snprintf.c
int rpl_snprintf(char *str, unsigned long int size, const char *format, ...);
int fprintf(FILE *stream, const char *format, ...)
{
    va_list duplicate_args;
    va_list args;
    va_start(args, format);
    va_copy(duplicate_args, args);

    char Buffer[1024];
    int Result = -1;
    if (stream == stdout)
        Result = rpl_snprintf(Buffer, 1024, format, duplicate_args);
    else if (stream == stderr)
    {
        kprintf("ERR: ");
        Result = rpl_snprintf(Buffer, 1024, format, duplicate_args);
    }

    if (Result >= 0)
        kprintf("%s", Buffer);

    va_end(duplicate_args);
    va_end(args);
    return Result;
}

int fflush(FILE *stream)
{
    return 0;
}

int getc(FILE *stream)
{
    PANIC("Lua: getc() stub.");
    return 0;
}

int feof(FILE *stream)
{
#warning feof() stub!
    return 0;
}

int ferror(FILE *stream)
{
#warning ferror() stub!
    return 0;
}


unsigned int fread(void *ptr, unsigned long long int size, unsigned long long int count, FILE *stream)
{
    PANIC("Lua: fread() stub.");
    return 0;
}

FILE *fopen(const char *path, const char *mode)
{
#warning fopen() stub!
    return NULL;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
#warning freopen() stub!
    return NULL;
}


int fclose(FILE *fp)
{
#warning fclose() stub!
    return 0;
}

// stdlib.h implementation
void *malloc(unsigned long long int i)
{
    return kmalloc(i);
}

int abs(int X)
{
    return (X > 0) ? X : (X * -1);
}

void abort(void)
{
    PANIC("Lua: abort() called.");
    __builtin_unreachable();
}

void *realloc(void *ptr, unsigned long int size)
{
    if (size == 0 && ptr != 0)
    {
        kfree(ptr);
        return NULL;
    }
    if (ptr == NULL)
        return kmalloc(size);

    void *Data = kmalloc(size);
    kmemcpy(Data, ptr, size);
    kfree(ptr);

    return Data;
}

void free(void *ptr)
{
    kfree(ptr);
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

int luai_numpow(void *L, int a, int b)
{
    int Result = 1;
    for (int i = 0; i < b; i++)
    {
        Result *= a;
    }
    return Result;
}

long int strtol (const char *str, char **endptr, int base)
{
    const char *beg = str;
    // find end of string
    while (*str >= '0' && *str <= '9')
        str++;
    str--;
    if (endptr != 0)
        *endptr = (char *)str; // shut up gcc

    long int value = 0;
    while (str >= beg)
    {
        char v = *str - '0';
        value += v;
        value *= 10;

        str--;
    }
    return value;
}

char *strerror(int errnum)
{
    return "Terrible error.";
}

char *strstr(const char *haystack, const char *needle)
{
    unsigned int HaystackLength = kstrlen(haystack);
    unsigned int NeedleLength = kstrlen(needle);

    if (NeedleLength > HaystackLength)
        return NULL;

    for (unsigned int i = 0; i <= HaystackLength - NeedleLength; i++)
    {
        char Okay = 1;
        for (unsigned int j = 0; j < NeedleLength; j++)
        {
            if (*(haystack + i + j) != *(needle + j))
            {
                Okay = 0;
                break;
            }
        }
        if (Okay)
            return (char *)haystack + i;
    }
    return NULL;
}

