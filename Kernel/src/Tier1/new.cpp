#include "types.h"
extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
};

typedef long unsigned int size_t;

void *operator new(size_t size)
{
    return kmalloc(size);
}
 
void *operator new[](size_t size)
{
    return kmalloc(size);
}
 
void operator delete(void *p) throw()
{
    kfree(p);
}
 
void operator delete[](void *p) throw()
{
    kfree(p);
}
