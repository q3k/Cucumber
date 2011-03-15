#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

void paging_init_simple(void);
u8 paging_get_physical(u32 Virtual, u32 *Physical);
void paging_map_kernel_page(u32 Virtual, u32 Physical);
void paging_map_kernel_table(u32 Virtual, u32 Physical);

#endif
