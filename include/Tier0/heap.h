#ifndef __HEAP_H__
#define __HEAP_H__

#include "types.h"

#define HEAP_HEADER_MAGIC 0x8A4DF92E
#define HEAP_FOOTER_MAGIC 0x9AFE352B

#define HEAP_START        0xD0000000
#define HEAP_INITIAL_SIZE 0x00004000
#define HEAP_MIN_SIZE     0x00004000

#define HEAP_INDEX_SIZE   0x00002000

typedef struct {
   u32 Magic;
   u8 Hole;
   u32 Size;
} T_HEAP_HEADER;

typedef struct {
    u32 Magic;
    T_HEAP_HEADER *Header;
} T_HEAP_FOOTER;

typedef struct {
    void **Array;
    u32 Size;
    u32 MaxSize;
} T_HEAP_INDEX;

T_HEAP_INDEX heap_index_initialize(void *Address, u32 MaxSize);
void heap_index_insert(T_HEAP_INDEX *Index, void *Value);
u8 heap_index_smaller(void *A, void *B);

typedef struct {
    T_HEAP_INDEX Index;
    u32 Start;
    u32 End;
    u32 Max;
} T_HEAP;

T_HEAP *heap_create(u32 Start, u32 End, u32 Max);
void *heap_alloc(T_HEAP *Heap, u32 Size, u8 Aligned);
void *heap_alloc_p(T_HEAP *Heap, u32 Size, u8 Aligned, u32 *Physical);
void heap_free(T_HEAP *Heap, void *Address);

void heap_init_simple(void);
void *kmalloc(u32 Size);
void *kmalloc_p(u32 Size, u8 Aligned, u32 *Physical);
void kfree(void *Data);

#endif
