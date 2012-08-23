#ifndef __TIER0_ANSI_STDIO_H__
#define __TIER0_ANSI_STDIO_H__

#include <stdarg.h>

//int printf(char *format, va_list args);
#define BUFSIZ 1024
#define FILE int
int sprintf(char *str, const char *format, ...);
int feof(FILE *stream);
unsigned int fread(void *ptr, unsigned long long int size, unsigned long long int count, FILE *stream);
int getc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *stream);
int ferror(FILE *stream);
int fclose(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int fflush(FILE *stream);
FILE *tmpfile(void);
int fscanf(FILE *stream, const char *format, ...);
int ungetc(int c, FILE *stream);
int clearerr(FILE *stream);
unsigned long long int fwrite(const void *ptr, unsigned long long int size, unsigned long long int nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int ftell(FILE *stream);
int fputs(const char *s, FILE *stream);
int printf(const char *format, ...);

#define EOF -1
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif
