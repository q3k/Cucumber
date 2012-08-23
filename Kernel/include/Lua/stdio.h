#ifndef __TIER0_ANSI_STDIO_H__
#define __TIER0_ANSI_STDIO_H__

#include <stdarg.h>
#include "Tier0/kstdlib.h"

//int printf(char *format, va_list args);
#define BUFSIZ 1024
#define FILE int
int sprintf(char *str, const char *format, ...);

#endif
