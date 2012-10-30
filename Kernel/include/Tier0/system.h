#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "load_context.h"

#define SYSTEM_KERNEL_VIRTUAL 0xFFFFFFFF80000000

struct S_SYSTEM_MLTBT_MMAP {
    u32 Size;
    u64 Base;
    u64 Length;
    u32 Type;
} __attribute__ ((packed));
typedef struct S_SYSTEM_MLTBT_MMAP T_SYSTEM_MLTBT_MMAP;

struct S_SYSTEM_INVALID_RAM {
    u64 Base;
    u64 Size;
} __attribute__ ((packed));
typedef struct S_SYSTEM_INVALID_RAM T_SYSTEM_INVALID_RAM;

void system_parse_cpu_features(void);
void system_parse_load_context(T_LOAD_CONTEXT *LoadContext);
u64 system_get_memory_upper(void);
u64 system_get_memory_lower(void);
s8 *system_get_bootloader_name(void);
u8 system_memory_available(u64 Start, u64 Length);

// CPUID stuff
typedef union {
    struct {
        u8 FPU     : 1;
        u8 VME     : 1;
        u8 DE      : 1;
        u8 PSE     : 1;
        u8 TSC     : 1;
        u8 MSR     : 1;
        u8 PAE     : 1;
        u8 MCE     : 1;
        u8 CX8     : 1;
        u8 APIC    : 1;
        u8 Res4    : 1;
        u8 SEP     : 1;
        u8 MTRR    : 1;
        u8 PGE1    : 1;
        u8 MCA     : 1;
        u8 CMOV    : 1;
        u8 PAT     : 1;
        u8 PSE36   : 1;
        u8 PSN     : 1;
        u8 CLF     : 1;
        u8 Res5    : 1;
        u8 DTES    : 1;
        u8 ACPI    : 1;
        u8 MMX     : 1;
        u8 FXSR    : 1;
        u8 SSE     : 1;
        u8 SSE2    : 1;
        u8 SS      : 1;
        u8 HTT     : 1;
        u8 TM1     : 1;
        u8 IA64    : 1;
        u8 PBE     : 1;
        u8 SSE3    : 1;
        u8 PCLMUL  : 1;
        u8 DTES64  : 1;
        u8 MONITOR : 1;
        u8 DS_CPL  : 1;
        u8 VMX     : 1;
        u8 SMX     : 1;
        u8 EST     : 1;
        u8 TM2     : 1;
        u8 SSSE3   : 1;
        u8 CID     : 1;
        u8 Res1    : 1;
        u8 FMA     : 1;
        u8 CX16    : 1;
        u8 ETPRD   : 1;
        u8 PDCM    : 1;
        u8 Res2    : 2;
        u8 DCA     : 1;
        u8 SSE4_1  : 1;
        u8 SSE4_2  : 1;
        u8 x2APIC  : 1;
        u8 MOVBE   : 1;
        u8 POPCNT  : 1;
        u8 Res3    : 1;
        u8 AES     : 1;
        u8 XSAVE   : 1;
        u8 OSXSAVE : 1;
        u8 AVX     : 1;
    } __attribute__((packed)) Flags;
    u64 FlagsU64;
} T_CPUID_FEATURES;

#define CPUID_HAS(f) (g_SystemInfo.CPUFeatures.Flags.f ? 1 : 0)

typedef struct {
    u64 MemoryLower;
    u64 MemoryUpper;
    s8* BootloaderName;
    
    T_CPUID_FEATURES CPUFeatures;
    
    // Just a guess...
    T_SYSTEM_INVALID_RAM InvalidMemoryAreas[256];
    u8 NumInvalidAreas;

    // kernel code size and location
    u64 KernelSize;
    u64 KernelPhysicalStart;
    u64 KernelVirtualStart;
} T_SYSTEM_INFO;

u64 system_cpuid(u32 Code);

// MSR stuff
u8 system_msr_available(void);
u64 system_msr_get(u32 MSR);
void system_msr_set(u32 MSR, u64 Data);

// kernel load address, size and mapping
u64 system_get_kernel_size(void);
u64 system_get_kernel_physical_start(void);
u64 system_get_kernel_virtual_start(void);

#endif
