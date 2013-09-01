#ifndef __PHYSMEM_H__
#define __PHYSMEM_H__

#include "types.h"

#define PHYSMEM_PAGE_SIZE 4096

// Page frame allocation
void physmem_init(void);
// This retrns a frame number, not an address!
u64 physmem_allocate_page(void);
// But this returns an address:
u64 physmem_allocate_physical(void);

u64 physmem_page_to_physical(u64 Page);
u64 physmem_physical_to_page(u64 Physical);

// Read physical data
void physmem_read(u64 Base, u64 Size, void *Destination);

// Return how much memory is free (including not reserved by system)
u64 physmem_get_free(void);

#endif
