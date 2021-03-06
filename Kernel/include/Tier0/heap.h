#ifndef __HEAP_H__
#define __HEAP_H__

#include "types.h"

#define HEAP_HEADER_MAGIC 0x8A4DF92ED6EAB782
#define HEAP_FOOTER_MAGIC 0x9AFE352B73FE48AB

#define HEAP_INITIAL_SIZE 0x01000000
#define HEAP_MIN_SIZE     0x01000000

#define HEAP_INDEX_SIZE   0x00002000

#define HEAP_START 0xFFFFFFFF00000000

typedef struct {
   u64 Magic;
   u8 Hole;
   u64 Size;
} T_HEAP_HEADER;

typedef struct {
    u64 Magic;
    T_HEAP_HEADER *Header;
} T_HEAP_FOOTER;

typedef struct {
    // wat
    void **Array;
    u64 Size;
    u64 MaxSize;
} T_HEAP_INDEX;

T_HEAP_INDEX heap_index_initialize(void *Address, u32 MaxSize);
void heap_index_insert(T_HEAP_INDEX *Index, void *Value);
u8 heap_index_smaller(void *A, void *B);

typedef struct {
    T_HEAP_INDEX Index;
    u64 Start;
    u64 End;
    u64 Max;
} T_HEAP;

T_HEAP *heap_create(u64 Size);
void *heap_alloc(T_HEAP *Heap, u64 Size, u8 Aligned);
void *heap_alloc_p(T_HEAP *Heap, u64 Size, u8 Aligned, u64 *Physical);
void heap_free(T_HEAP *Heap, void *Address);

void heap_init_simple(void);
// Allocate, return virtual address
void *kmalloc(u64 Size);
// Allocate aligned to 0x1000, return virtual address
void *kmalloc_aligned(u64 Size);
// Allocate aligned to 0x1000, return physical address
void *kmalloc_aligned_physical(u64 Size);
void *kmalloc_p(u64 Size, u8 Aligned, u64 *Physical);
void kfree(void *Data);

#endif
