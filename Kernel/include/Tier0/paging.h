#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"
#include "load_context.h"

// Some helpful macros
#define PAGING_GET_PML4_INDEX(x) (((u64)x >> 39) & 0x1FF)
#define PAGING_GET_PDP_INDEX(x) (((u64)x >> 30) & 0x1FF)
#define PAGING_GET_DIR_INDEX(x) (((u64)x >> 21) & 0x1FF)
#define PAGING_GET_TAB_INDEX(x) (((u64)x >> 12) & 0x1FF)
#define PAGING_GET_PAGE_OFFSET(x) (x & 0xFFF)

// Intel-defined entries - strict!
struct S_PAGING_TAB_ENTRY {
    u8  Present    :  1;
    u8  RW         :  1;
    u8  User       :  1;
    u8  Accessed   :  1;
    u8  Misc       :  8;
    u64 Physical   : 40; // The physical address is limited by MAXPHYADDR
    u16 Zero       : 12;
} __attribute__((packed));
typedef struct S_PAGING_TAB_ENTRY T_PAGING_TAB_ENTRY;

struct S_PAGING_DIR_ENTRY {
	u8  Present    :  1;
	u8  RW         :  1;
	u8  User       :  1;
	u8  Accessed   :  1;
	u8  Misc       :  8;
	u64 Physical   : 40; // The physical address is limited by MAXPHYADDR
	u64 Zero       : 12;
} __attribute__((packed));
typedef struct S_PAGING_DIR_ENTRY T_PAGING_DIR_ENTRY;

struct S_PAGING_PDP_ENTRY {
	u8  Present    :  1;
	u8  RW         :  1;
	u8  User       :  1;
	u8  Accessed   :  1;
	u8  Misc       :  8;
	u64 Physical   : 40; // The physical address is limited by MAXPHYADDR
	u64 Zero       : 12;
} __attribute__((packed));
typedef struct S_PAGING_PDP_ENTRY T_PAGING_PDP_ENTRY;

struct S_PAGING_ML4_ENTRY {
	u8  Present    :  1;
	u8  RW         :  1;
	u8  User       :  1;
	u8  Accessed   :  1;
	u8  Misc       :  8;
	u64 Physical   : 40; // The physical address is limited by MAXPHYADDR
	u64 Zero       : 12;
} __attribute__((packed));
typedef struct S_PAGING_ML4_ENTRY T_PAGING_ML4_ENTRY;


// OS-defined structures
typedef struct {
	T_PAGING_TAB_ENTRY Entries[512];
} __attribute__((packed)) T_PAGING_TAB;

typedef struct {
	T_PAGING_DIR_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_DIR;

typedef struct {
	T_PAGING_PDP_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_PDP;

typedef struct {
	T_PAGING_ML4_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_ML4;

// Generic funcitons
T_PAGING_ML4 *   paging_get_ml4(void);
void             paging_set_ml4(u64 ML4Physical);

// Management of kernel paging structures
void paging_kernel_init(void);
// Map a 4k page from Physical to Virtual. AccessBits is undefined right now.
void paging_map_page(u64 Virtual, u64 Physical, void *AccessBits);
// Map an arbitrary Size range from Physical to Virtual. Must still be kinda aligned.
void paging_map_area(u64 PhysicalStart, u64 VirtualStart, u64 Size, void *AccessBits);
u8 _paging_resolve(u64 Virtual, u64 *PhysicalOut);
// Get directory of SCRATCH memory region, for use by Tier1 allocators
u64 paging_get_scratch_directory(void);
// Get directory of TEXT memory region, for use by Tier1 allocators
u64 paging_get_text_directory(void);
#endif
