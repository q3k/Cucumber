#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

// Some helpful macros
#define PAGING_GET_ML4_INDEX(x) (((u64)x >> 39) & 0x1FF)
#define PAGING_GET_DPT_INDEX(x) (((u64)x >> 30) & 0x1FF)
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

struct S_PAGING_DPT_ENTRY {
	u8  Present    :  1;
	u8  RW         :  1;
	u8  User       :  1;
	u8  Accessed   :  1;
	u8  Misc       :  8;
	u64 Physical   : 40; // The physical address is limited by MAXPHYADDR
	u64 Zero       : 12;
} __attribute__((packed));
typedef struct S_PAGING_DPT_ENTRY T_PAGING_DPT_ENTRY;

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


// OS-defined structures - semi-loose.
typedef struct {
	T_PAGING_TAB_ENTRY Entries[512];
	u64 PhysicalAddress;
} T_PAGING_TAB;

typedef struct {
	T_PAGING_DIR_ENTRY Entries[512]; // For use by CPU
	T_PAGING_TAB *Children[512];     // For use by OS
	u64 PhysicalAddress;             // FOr use by OS when setting CPU
} T_PAGING_DIR;

typedef struct {
	T_PAGING_DPT_ENTRY Entries[512];
	T_PAGING_DIR *Children[512];
	u64 PhysicalAddress;
} T_PAGING_DPT;

typedef struct {
	T_PAGING_ML4_ENTRY Entries[512];
	T_PAGING_DPT *Children[512];
	u64 PhysicalAddress;
} T_PAGING_ML4;

void           paging_init_simple(u64 PhysicalVirtualOffset);
T_PAGING_ML4 * paging_get_ml4(void);
u8             paging_get_physical(u64 Virtual, u64 *Physical);
u8             paging_get_physical_ex(u64 Virtual, u64 *Physical,T_PAGING_ML4 *ML4);

/*void paging_map_kernel_page(u64 Virtual, u64 Physical);
void paging_map_kernel_table(u64 Virtual, u64 Physical);
void paging_map_page(u64 Virtual, u64 Physical, T_PAGING_DIRECTORY *Directory,
                     u8 User, u8 RW);
void paging_use_directory(T_PAGING_DIRECTORY *Directory);
T_PAGING_DIRECTORY *paging_get_directory(void);*/

#endif
