#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"
#include "load_context.h"

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


// OS-defined structures
typedef struct {
	T_PAGING_TAB_ENTRY Entries[512];
} __attribute__((packed)) T_PAGING_TAB;

typedef struct {
	T_PAGING_DIR_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_DIR;

typedef struct {
	T_PAGING_DPT_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_DPT;

typedef struct {
	T_PAGING_ML4_ENTRY Entries[512]; // For use by the CPU
} __attribute__((packed)) T_PAGING_ML4;

T_PAGING_ML4 *   paging_get_ml4(void);
void             paging_set_ml4(u64 ML4Physical);

u8               paging_get_physical(u64 Virtual, u64 *Physical);
u8               paging_get_physical_ex(u64 Virtual, u64 *Physical,T_PAGING_ML4 *ML4);

void paging_kernel_initialize(u64 KernelVirtualStart, u64 KernelPhysicalStart, u64 KernelSize);

// The temporary page is a page you can use to access some temporary physical
// location. There is only one page, 4096 bytes large. Deal with it.
void             paging_temp_page_setup(T_LOAD_CONTEXT *LoadContext);
const inline u64 paging_temp_page_get_virtual(void)
{
    return 0xFFFFFFFF80000000 + 511 * 0x1000;
}
void             paging_temp_page_set_physical(u64 Physical);

// The MiniVMM is a mini virtual memory manager that manages, similarly to the
// basic physical frame manager, allocation of _virtual_ frames, for use by
// paging functions. This should be only used by major parts of the kernel, for
// example to map some data structures to physical memory, or if physmem_read
// calls would be too slow.
void paging_minivmm_setup(u64 Start, u64 End);
u64 paging_minivmm_allocate(void);

// A simple page map call. This does no checks! Triple faults ahoy.
void paging_map_page(u64 Virtual, u64 Physical);

#endif
