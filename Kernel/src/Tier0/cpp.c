// Basically stuff that is needed to go into C++ Land

#include "Tier0/cpp.h"
#include "Tier0/kstdio.h"

extern u64 _start_ctors;
extern u64 _end_ctors;
void CKernelStart(void);

void cpp_call_ctors(void)
{
    u32 Number = ((void *)&_end_ctors - (void *)&_start_ctors) / 8;
    kprintf("[i] Calling %i constructors before jumping to Tier1..\n", Number);
    for(u64 *C = (u64*)&_start_ctors; C < (u64*)&_end_ctors; ++C)
    {
        ((void (*) (void)) (*C)) ();
    }
}

void cpp_start_ckernel(void)
{
    CKernelStart();
}

void __cxa_pure_virtual()
{
    kprintf("[e] A pure virtual call happened. WTF?\n");
}

int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
{
    // Do nothing, for now.
    return 0;
}

void __cxa_finalize(void *f)
{
    // -- " -- " --
}
