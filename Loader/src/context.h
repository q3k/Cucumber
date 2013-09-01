#ifndef __CONTEXT_H__
#define __CONTEXT_H__

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
    u64 KernelELF;
    u64 KernelELFSize;
} __attribute__((packed));
typedef struct S_LOAD_CONTEXT T_LOAD_CONTEXT;

#endif