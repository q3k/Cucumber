#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"

// Real kernel entry point, called from _start.asm
void kmain(void *mbd, u32 magic)
{
    kclear();
    kprintf("[i] Welcome to Cucumber!\n\n");
    kprintf("[i] Magic from bootloader: 0x%x.\n", magic);

    if (magic != 0x2BADB002)
    {
        kprintf("[e] Fatal! Boot via incompatible bootloader.\n");
        return;
    }

    paging_init_simple();
    gdt_create_flat();
}

