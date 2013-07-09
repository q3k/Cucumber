#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include "types.h"

typedef struct
{
    u32 ModuleStart;
    u32 ModuleEnd;
    s8 *String;
    u32 Reserved;
} __attribute__((packed)) TMULTIBOOT_MODULE;

typedef struct {
    u32 Flags;
    u32 MemLower;
    u32 MemUpper;
    u32 BootDevice;
    u32 CommandLines;
    u32 ModulesCount;
    TMULTIBOOT_MODULE *Modules;
} __attribute__((packed)) TMULTIBOOT_INFO;

#define MULTIBOOT_INFO_MEMORY                   0x00000001
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
#define MULTIBOOT_INFO_MODS                     0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200
#define MULTIBOOT_INFO_APM_TABLE                0x00000400
#define MULTIBOOT_INFO_VIDEO_INFO               0x00000800

#endif