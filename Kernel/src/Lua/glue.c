// Glue code for Lua <-> Tier0

#include <stdarg.h>

#include "setjmp.h"
#include "stdio.h"
#include "Tier0/panic.h"
#include "Tier0/kstdio.h"
#include "Tier0/heap.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdlib.h"

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
    // gods help me
    u64 Result;
    __asm__ __volatile__(
            // rax in our buffer is going to be set to the magic value of 
            // 0, in order to let know code underneath that we are actually
            // in the jump target, not the setting function
            "movq $0,      0(%%rax)\n"
            "movq %%rbx,   8(%%rax)\n"
            "movq %%rcx,  16(%%rax)\n"
            "movq %%rdx,  24(%%rax)\n"
            "movq %%rsi,  32(%%rax)\n"
            "movq %%rdi,  40(%%rax)\n"
            "movq %%rsp,  48(%%rax)\n"
            "movq %%rbp,  56(%%rax)\n"
            "movq %%r8,   64(%%rax)\n"
            "movq %%r9,   72(%%rax)\n"
            "movq %%r10,  80(%%rax)\n"
            "movq %%r11,  88(%%rax)\n"
            "movq %%r12,  96(%%rax)\n"
            "movq %%r13, 104(%%rax)\n"
            "movq %%r14, 112(%%rax)\n"
            "movq %%r15, 120(%%rax)\n"

            "movq %%rax, %%rbx\n"
            "call get_rip\n"
            // two possibilities with rax here:
            //  - if not zero, then we're in setjmp for the first time
            //  - if zero, then we're in setjmp from longjmp

            "test %%rax, %%rax\n"
            "jnz setjmp_original\n"

            // still here? then we just came from longjmp(). let's return the
            // argument 'value', which we have on our stack
            "popq %%rbx\n"
            "popq %%rax\n"
            "movq %%rax, %0\n"
            "jmp setjmp_done\n"

            // helper function (get rip)
            "get_rip: movq (%%rsp), %%rax\n ret\n"

            // original setting return
            // don't forget to set rip!
            "setjmp_original:\n"
            "movq %%rax, 128(%%rbx)\n"
            "mov $0, %0\n"
            "setjmp_done:\n"
            "nop"
    :"=r"(Result):"a"(&env[0]));

    /*kprintf("SETJMP! :D\n");
    kprintf("%x %x %x %x\n", env[0].rax, env[0].rbx, env[0].rcx, env[0].rdx);
    kprintf("%x %x %x %x\n", env[0].rsi, env[0].rdi, env[0].rsp, env[0].rbp);
    kprintf("%x %x %x %x\n", env[0].r8,  env[0].r9,  env[0].r10, env[0].r11);
    kprintf("%x %x %x %x\n", env[0].r12, env[0].r13, env[0].r14, env[0].rip);
    kprintf("-------------\n");*/
    return (int)Result;
}

void longjmp(jmp_buf env, int value)
{
    __asm__ __volatile__(
            "movq  48(%%rax), %%rsp\n"
            // we can now push to the setjmp env stack

            // push 'value'
            "pushq %%rbx\n"

            // push saved rbx
            "movq   8(%%rax), %%rbx\n"
            "pushq %%rbx\n"

            "movq  16(%%rax), %%rcx\n"
            "movq  24(%%rax), %%rdx\n"
            "movq  32(%%rax), %%rsi\n"
            "movq  40(%%rax), %%rdi\n"
            "movq  56(%%rax), %%rbp\n"
            "movq  64(%%rax), %%r8\n"
            "movq  72(%%rax), %%r9\n"
            "movq  80(%%rax), %%r10\n"
            "movq  88(%%rax), %%r11\n"
            "movq  96(%%rax), %%r12\n"
            "movq 104(%%rax), %%r13\n"
            "movq 112(%%rax), %%r14\n"
            "movq 120(%%rax), %%r15\n"

            // get rip
            "movq 128(%%rax), %%rbx\n"

            // set rax to 0 so that setjmp knows we're not the original call
            "xorq %%rax, %%rax\n"

            // off we go!
            "jmp %%rbx\n"
   ::"a"(&env[0]), "b"(value));
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
        Result = rpl_vsnprintf(Buffer, 1024, format, duplicate_args);
    else if (stream == stderr)
    {
        kprintf("ERR: ");
        Result = rpl_vsnprintf(Buffer, 1024, format, duplicate_args);
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
    kprintf("\n\nabort()\n");
    for (;;) {}
    //PANIC("Lua: abort() called.");
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

