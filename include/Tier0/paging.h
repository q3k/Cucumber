#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

struct S_PAGING_PAGE {
    u8 Present    :  1;
    u8 RW         :  1;
    u8 User       :  1;
    u8 Accessed   :  1;
    u8 Misc       :  8;
    u32 Physical  : 20;
} __attribute__((packed));

typedef struct S_PAGING_PAGE T_PAGING_PAGE;

typedef struct {
    T_PAGING_PAGE Pages[1024];
} T_PAGING_TABLE;

typedef struct {
    u32 Entries[1024];
    T_PAGING_TABLE *Tables[1024];
    u32 PhysicalAddress;
} T_PAGING_DIRECTORY;

void paging_init_simple(void);
u8 paging_get_physical(u32 Virtual, u32 *Physical);
u8 paging_get_physical_ex(u32 Virtual, u32 *Physical, 
                          T_PAGING_DIRECTORY *Directory);
void paging_map_kernel_page(u32 Virtual, u32 Physical);
void paging_map_kernel_table(u32 Virtual, u32 Physical);
void paging_map_page(u32 Virtual, u32 Physical, T_PAGING_DIRECTORY *Directory, 
                     u8 User, u8 RW);
void paging_use_directory(T_PAGING_DIRECTORY *Directory);

#endif
