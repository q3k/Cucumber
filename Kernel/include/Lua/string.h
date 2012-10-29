#ifndef __TIER0_ANSI_STRING_H__
#define __TIER0_ANSI_STRING_H__

#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"

const char *strchr (const char *str, int character);
int strcmp(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strerror(int errnum);
char *strstr(const char *haystack, const char *needle);
unsigned long long int strspn(const char *s, const char *accept);
void *memchr(const void *s, int c, unsigned long long int n);
char *strcpy(char *dest, const char *src);
int memcmp(const void *s1, const void *s2, unsigned long int n);

#define strcoll(A, B) strcmp(A, B)
#define strlen(A) kstrlen(A)
#define memcpy(A, B, C) kmemcpy(A, B, C)
#endif
