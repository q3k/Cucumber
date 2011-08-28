#ifndef __MP_H__
#define __MP_H__

#include "types.h"

typedef struct {
    s8 Signature[4];
    u32 TablePhysical;
    u8 Length;
    u8 Specification;
    u8 Checksum;
} __attribute__((packed)) T_MP_POINTER;

u64 mp_find_pointer(u64 Start, u64 End);
void mp_initialize(void);

#endif
