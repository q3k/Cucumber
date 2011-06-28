#ifndef __PHYSICAL_ALLOC_H__
#define __PHYSICAL_ALLOC_H__

#include "types.h"

void physmem_init(void);

void physmem_mark_as_used(u32 Page);
u32 physmem_allocate_page(void);
void physmem_free_page(u32 Page);

u32 physmem_page_to_physical(u32 Page);
u32 physmem_physical_to_page(u32 Physical);

#endif
