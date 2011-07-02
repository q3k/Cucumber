#ifndef __PHYSICAL_ALLOC_H__
#define __PHYSICAL_ALLOC_H__

#include "types.h"

#define PHYSALLOC_PAGE_SIZE 4096
#define PHYSALLOC_NUM_BITMAPS ((4096 * 8 / 64) - 1)

struct S_PHYSALLOC_NODE {
	u64 Bitmaps[PHYSALLOC_NUM_BITMAPS];
	struct S_PHYSALLOC_NODE *Next;
};

typedef struct S_PHYSALLOC_NODE T_PHYSALLOC_NODE;

void physmem_init(u64 MemorySize);
void physmem_set_node_space(u64 Space);

void physmem_mark_as_used(u64 Page);
u64 physmem_allocate_page(void);
void physmem_free_page(u64 Page);

u64 physmem_page_to_physical(u64 Page);
u64 physmem_physical_to_page(u64 Physical);

#endif
