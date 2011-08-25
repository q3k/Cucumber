#ifndef __PHYSICAL_ALLOC_H__
#define __PHYSICAL_ALLOC_H__

#include "types.h"

#define PHYSALLOC_PAGE_SIZE 4096

void physmem_init(u64 MemorySize);
u64 physmem_allocate_page(void);

u64 physmem_page_to_physical(u64 Page);
u64 physmem_physical_to_page(u64 Physical);

#endif
