#ifndef __LOAD_CONTEXT_H__
#define __LOAD_CONTEXT_H__

// A struct passed to the kernel from a loader
//
// Helps kernel identify the execution context, etc.

#include "types.h"

struct S_LOAD_CONTEXT {
    u64 ReservedPhysicalStart;
    u64 ReservedPhysicalEnd;
    
    s8 LoaderName[80];
    
    // VGA text mode 0
    u8 VGATextModeUsed : 1;
    u32 VGACurrentLine;
    u32 VGACursorX;
    u32 VGACursorY;
    
    // Multiboot
    u8 MultibootUsed : 1;
    u64 MultibootHeader;

    // Kernel ELF
    void *KernelELF;
} __attribute__((packed));
typedef struct S_LOAD_CONTEXT T_LOAD_CONTEXT;

#endif
