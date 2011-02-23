#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"
#include "Tier0/acpi.h"
#include "Tier0/interrupts.h"
#include "Tier0/ps2.h"
#include "Tier0/system.h"

// Just to see whether this stuff actually works
void sample_interrupt_0x2A(void)
{
    kprintf("[i] Hello from ISR for interrupt 0x2A!\n");
    return;
}

// Real kernel entry point, called from _start.asm
void kmain(void *MultibootHeader, u32 Magic)
{
    kclear();
    kprintf("                         _           \n"
            "   ___ _ _ ___ _ _ _____| |_ ___ ___ \n"
            "  |  _| | |  _| | |     | . | -_|  _|\n"
            "  |___|___|___|___|_|_|_|___|___|_|  \n\n");
    kprintf("[i] Welcome to Cucumber!\n\n");

    if (Magic != 0x2BADB002)
    {
        kprintf("[e] Fatal! Boot via incompatible bootloader.\n");
        return;
    }

    paging_init_simple();
    gdt_create_flat();
    system_parse_multiboot_header(MultibootHeader);

    kprintf("[i] Booting via %s.\n", system_get_bootloader_name());
    kprintf("[i] Memory available: %uk.\n", system_get_memory_upper());

    u32 RSDPAddress = acpi_find_rsdp();
    if (RSDPAddress == 0)
    {
        kprintf("[e] Fatal! ACPI not found.\n");
        return;
    }

    interrupts_init_simple();

    paging_allocate_page(0xDEADBEEF);
    u32 *Beef = (u32*)0xDEADBEEF;
    *Beef = 13371337;

    kprintf("Beef: %u\n", *Beef);
}
