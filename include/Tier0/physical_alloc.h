#ifndef __PHYSICAL_ALLOC_H__
#define __PHYSICAL_ALLOC_H__

#include "types.h"

void physmem_init(void);

void physmem_mark_as_used(u16 SuperPage);
u16 physmem_allocate_superpage(void);
void physmem_free_superpage(u16 SuperPage);

u32 physmem_superpage_to_physical(u16 SuperPage);
u16 physmem_physical_to_superpage(u32 Physical);

void physmem_dump_map(void);

#endif
