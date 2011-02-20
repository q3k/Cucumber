#include "Types.h"
#include "kstdio.h"

// Real kernel entry point, called from _start.asm
void kmain(void *mbd, u32 magic)
{
    if (magic != 0x2BADB002)
    {
        kprintf("[e] Fatal! Boot via incompatible bootloader.\n");
        return;
    }

    char *szBootLoaderName = (char *)((u32 *)mbd)[16];

    kclear();
    kprintf("[i] Booting via %s.\n", szBootLoaderName);
}
