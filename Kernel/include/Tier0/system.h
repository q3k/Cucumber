#ifndef __SYSTEM_H__
#define __SYSTEM_H__

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

void system_parse_multiboot_header(void *Header);
u64 system_get_memory_upper(void);
u64 system_get_memory_lower(void);
s8 *system_get_bootloader_name(void);
u8 system_memory_available(u64 Start, u64 Length);

#endif
