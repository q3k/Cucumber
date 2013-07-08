#ifndef __CONTEXT_H__
#define __CONTEXT_H__

struct S_LOAD_CONTEXT {
    u64 KernelPhysicalStart;
    u64 KernelPhysicalEnd;
    
    u64 LoaderPhysicalStart;
    u64 LoaderPhysicalEnd;
    s8 LoaderName[80];
    
    // VGA text mode 0
    u8 VGATextModeUsed : 1;
    u32 VGACurrentLine;
    u32 VGACursorX;
    u32 VGACursorY;
    
    // Multiboot
    u8 MultibootUsed : 1;
    u64 MultibootHeader;
} __attribute__((packed));
typedef struct S_LOAD_CONTEXT T_LOAD_CONTEXT;

#endif