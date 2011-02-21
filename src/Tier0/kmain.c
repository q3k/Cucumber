#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"

// Real kernel entry point, called from _start.asm
void kmain(void *mbd, u32 magic)
{
    init_simple_paging();
    gdt_create_flat();

    if (magic != 0x2BADB002)
    {
        kprintf("[e] Fatal! Boot via incompatible bootloader.\n");
        return;
    }

    char *szBootLoaderName = (char *)((u32 *)mbd)[16];

    kclear();
    kprintf("[i] Booting via %s.\n", szBootLoaderName);
    
}

