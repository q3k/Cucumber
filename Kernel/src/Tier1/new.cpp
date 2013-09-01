#include "types.h"
extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
};

typedef long unsigned int size_t;

void *operator new(size_t size)
{
    PANIC("nonew4u");
    return kmalloc(size);
}
 
void *operator new[](size_t size)
{
    PANIC("nonew4u");
    return kmalloc(size);
}
 
void operator delete(void *p)
{
    PANIC("nodel4u");
    kfree(p);
}
 
void operator delete[](void *p)
{
    PANIC("nodel4u");
    kfree(p);
}
