#ifndef __TIER0_ANSI_STDLIB_H__
#define __TIER0_ANSI_STDLIB_H__

void *malloc(unsigned long long int size);
void free(void *);
void *realloc(void *ptr, unsigned long long int size);
int abs(int X);
void abort(void);
long int strtol(const char *nptr, char **endptr, int base);
void exit(int status);

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#endif
