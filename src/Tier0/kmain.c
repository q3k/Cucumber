#include "types.h"
#include "Tier0/kstdio.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"
#include "Tier0/acpi.h"
#include "Tier0/interrupts.h"
#include "Tier0/ps2.h"
#include "Tier0/system.h"
#include "Tier0/pic.h"
#include "Tier0/kbd_layout.h"
#include "Tier0/physical_alloc.h"
#include "Tier0/heap.h"
#include "Tier0/cpp.h"
#include "Tier0/exceptions.h"
#include "Tier0/panic.h"

#define STACK_SIZE 0x400000

void interrupts_irq_sample(void);

// Just to see whether this stuff actually works
void sample_interrupt_0x2A(void)
{
    kprintf("[i] Hello from ISR for interrupt 0x2A!\n");
    return;
}

void kmain_newstack(void);

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
    
    physmem_init();
    system_parse_multiboot_header(MultibootHeader);
    
    //Add kernel memory as reserved.
    physmem_mark_as_used(0);
    physmem_mark_as_used(1);
    
    kprintf("[i] Booting via %s.\n", system_get_bootloader_name());
    kprintf("[i] Memory available: %uk.\n", system_get_memory_upper());
    
    u32 RSDPAddress = acpi_find_rsdp();
    if (RSDPAddress == 0)
    {
        kprintf("[e] Fatal! ACPI not found.\n");
        return;
    }
    interrupts_init_simple();
    exceptions_init_simple();
    pic_init(0, 0);
    ps2_init_simple();
    kbd_layout_set_default();
    
    __asm__ volatile("sti");
    
    kprintf("[i] Hardware interrupts are now enabled.\n");
    
    heap_init_simple();
    
    // Let's create a new kernel stack... on the heap! :o
    u32 StackAddress = (u32)kmalloc_p(STACK_SIZE, 1, 0);
    ASSERT(StackAddress > 0);
    StackAddress += STACK_SIZE;
    
    kprintf("[i] New stack at %x.\n", StackAddress);
    
    // And now let's use it and forget ebp because we can.
    //__asm__ volatile("mov %0, %%esp" : : "r" (StackAddress));
    
    // This automagically creates a new usable stack
    kmain_newstack();
}

void kmain_newstack(void)
{
    kprintf("[i] Now using real stack...\n");
    
    cpp_call_ctors();
    cpp_start_ckernel();
    
    kprintf("[i] Returned from Tier1, sleeping forever.\n");
    LOOPFOREVER;
}
