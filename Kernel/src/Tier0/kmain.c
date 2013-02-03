#include "types.h"
#include "version.h"
#include "load_context.h"
#include "Tier0/kstdio.h"
#include "Tier0/kstdlib.h"
#include "Tier0/gdt.h"
#include "Tier0/paging.h"
#include "Tier0/acpi.h"
#include "Tier0/apic.h"
#include "Tier0/smp.h"
#include "Tier0/interrupts.h"
//#include "Tier0/ps2.h"
#include "Tier0/system.h"
//#include "Tier0/pic.h"
//#include "Tier0/kbd_layout.h"
#include "Tier0/physmem.h"
#include "Tier0/heap.h"
#include "Tier0/cpp.h"
#include "Tier0/exceptions.h"
#include "Tier0/panic.h"
//#include "Tier0/prng.h"

void kmain_newstack(void);

// Real kernel entry point, called from loader
void kmain(u32 LoadContextAddress)
{
    T_LOAD_CONTEXT *LoadContext = (T_LOAD_CONTEXT*)(u64)LoadContextAddress;
    kstdio_init();
    
    if (LoadContext->VGATextModeUsed)
        kstdio_set_globals(LoadContext->VGACurrentLine, LoadContext->VGACursorX, LoadContext->VGACursorY);
    else
        kclear();
    
    kprintf("                         _           \n"
            "   ___ _ _ ___ _ _ _____| |_ ___ ___ \n"
            "  |  _| | |  _| | |     | . | -_|  _|\n"
            "  |___|___|___|___|_|_|_|___|___|_|  \n\n");
    kprintf("[i] Welcome to Cucumber (x86-64)!\n");
    kprintf("[i] %s\n\n", CUCUMBER_VERION);
    kprintf("[i] Load Context @%x     \n", LoadContext);
    if (!LoadContext->MultibootUsed)
        PANIC("No Multiboot header provided by loader!");

    kprintf("[i] Multiboot header @%x\n", LoadContext->MultibootHeader);
    
    system_parse_cpu_features();
    
    extern T_SYSTEM_INFO g_SystemInfo;
    if (!CPUID_HAS(FPU))
        PANIC("CPU doesn't have FPU!");
    if (!CPUID_HAS(MSR))
        PANIC("CPU doesn't support MSR!");
    if (!CPUID_HAS(APIC))
        PANIC("CPU doesn't support APIC!");
    
    system_parse_load_context(LoadContext); 
    kprintf("[i] Booting via %s.\n", LoadContext->LoaderName);
    kprintf("[i] Memory available: %uk.\n", system_get_memory_upper());
    kprintf("[i] Kernel physical: %x-%x.\n", system_get_kernel_physical_start(),
        system_get_kernel_physical_start() + system_get_kernel_size());
    kprintf("[i] Loader physical: %x-%x.\n", LoadContext->LoaderPhysicalStart, LoadContext->LoaderPhysicalEnd);
    kprintf("[i] Kernel virtual:  %x-%x.\n", system_get_kernel_virtual_start(),
        system_get_kernel_virtual_start() + system_get_kernel_size());

    paging_temp_page_setup();
    physmem_init();
    paging_scratch_initialize();
    // Let's create a new kernel stack
    u64 StackVirtual = (u64)paging_scratch_allocate();
    kprintf("[i] New kernel stack 0x%x\n", StackVirtual);
    
    // And now let's use it and forget ebp because we can.
    __asm__ volatile("mov %0, %%rsp" : : "r" (StackVirtual + 4096));

    // And let's create a new stack frame.
    // (and prevent gcc from inlinin the function call)
    void (*kmain_newstack_ptr)() = kmain_newstack;
    kmain_newstack_ptr();
}

void kmain_newstack(void)
{
    
    u64 RSDPAddress = acpi_find_rsdp();
    if (RSDPAddress == 0)
        PANIC("ACPI not supported! What is this, 1999?");
    
    //smp_initialize();
    interrupts_init_simple();
    exceptions_init_simple();
    apic_enable_lapic();
    heap_init_simple();
    for (;;) {}
    // enable FPU/SSE...
    __asm__ volatile(
                    "movq %cr0, %rax;"
                    "and $0xfffb, %ax;"
                    "or $0x2, %rax;"
                    "movq %rax, %cr0;"
                    "movq %cr4, %rax;"
                    "orq $0x600, %rax;"
                    "movq %rax, %cr4;");
    
    cpp_call_ctors();
    cpp_start_ckernel();
    kprintf("[i] Returned from Tier1, sleeping forever.\n");
    kprintf("[i] Free memory: %i KiB.\n", physmem_get_free());
    LOOPFOREVER;
}
