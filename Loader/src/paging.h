#ifndef __PAGING_H__
#define __PAGING_G__

#include "types.h"

#define GET_PML4_ENTRY(x) (((u64)x >> 39) & 0x1FF)
#define GET_PDP_ENTRY(x) (((u64)x >> 30) & 0x1FF)
#define GET_DIR_ENTRY(x) (((u64)x >> 21) & 0x1FF)
#define GET_TAB_ENTRY(x) (((u64)x >> 12) & 0x1FF)
#define GET_OFFSET(x) (x & 0xFFF)

void paging_setup(u32 AllocateFramesFrom);
void paging_map_address(u64 PhysicalStart, u64 VirtualStart, u64 Size);
void *paging_allocate(u64 VirtualStart, u64 Size);
u32 paging_get_last_frame(void);
void *paging_get_pml4(void);

#endif