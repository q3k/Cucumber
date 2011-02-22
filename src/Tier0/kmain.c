#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"
#include "Tier0/acpi.h"
#include "Tier0/interrupts.h"


// Just to see whether this stuff actually works
void sample_interrupt_0x2A(void)
{
    kprintf("[i] Hello from ISR for interrupt 0x2A!\n");
    return;
}

// Real kernel entry point, called from _start.asm
void kmain(u32 Magic)
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
    
    kprintf("[i] Paging and GDT set up correctly.\n");

    u32 RSDPAddress = acpi_find_rsdp();
    if (RSDPAddress == 0)
    {
        kprintf("[e] Fatal! ACPI not found.\n");
        return;
    }

    kprintf("[i] RSDP found at 0x%X.\n", RSDPAddress);

    u32 PhysicalTest;
    u8 Result = paging_get_physical(0xC00B8000, &PhysicalTest);
    if (!Result || PhysicalTest != 0xB8000)
    {
        kprintf("[e] Paging self-test failed!\n");
        return;
    }

    interrupts_init_simple();
    interrupts_setup_isr(0x2A, sample_interrupt_0x2A, E_INTERRUPTS_RING0);

    __asm__ volatile("int $0x2a");
}
