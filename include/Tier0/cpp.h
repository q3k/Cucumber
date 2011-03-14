#ifndef __CPP_H__
#define __CPP_H__

#include "types.h"

void cpp_call_ctors(void);
void cpp_start_ckernel(void);

void __cxa_pure_virtual();
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);

#endif
