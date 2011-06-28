#include "types.h"
#include "Tier0/kstdio.h"
/*#include "Tier0/gdt.h"
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
#include "Tier0/prng.h"*/

// Real kernel entry point, called from loader
void kmain(u32 current_line, u32 cursor_x, u32 cursor_y)
{
    kstdio_init();
    kstdio_set_globals(current_line, cursor_x, cursor_y);
    //kclear();
    kprintf("\n                         _           \n"
            "   ___ _ _ ___ _ _ _____| |_ ___ ___ \n"
            "  |  _| | |  _| | |     | . | -_|  _|\n"
            "  |___|___|___|___|_|_|_|___|___|_|  \n\n");
    kprintf("[i] Welcome to Cucumber (x86-64)!\n");
    //kprintf("%x %x %x\n", current_line, cursor_x, cursor_y);

    for (;;) {}

    /*if (Magic != 0x2BADB002)
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
    
    kprintf("[i] Initializing PRNG...\n");
    u16 RLow, RHigh;
    __asm__ __volatile__ ("rdtsc" : "=a" (RLow), "=d" (RHigh));
    u32 R = (RHigh << 16) | RLow;
    kprintf("[i] %i\n", R);
    kseed(R);
    for (u32 Rl = 0; Rl < R; Rl++)
    {
        krand();
    }
    
    // Let's create a new kernel stack
    u32 StackPhysical = physmem_allocate_page() * 1024 * 4;
    paging_map_kernel_page(0xA0000000, StackPhysical);
    
    // And now let's use it and forget ebp because we can.
    __asm__ volatile("mov %0, %%esp" : : "r" (0xA0000000 + 4095));
    
    // This automagically creates a new usable stack frame
    kmain_newstack();*/
}

/*void kmain_newstack(void)
{
    kprintf("[i] Now using real stack...\n");
    cpp_call_ctors();
    cpp_start_ckernel();
    kprintf("[i] Returned from Tier1, sleeping forever.\n");
    LOOPFOREVER;
}*/
