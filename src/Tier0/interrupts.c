#include "Tier0/interrupts.h"
#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/pic.h"

T_IDT_PTR g_idt_ptr;
T_IDT_ENTRY g_idt_entries[256];
T_ISR_STUB g_isr_stubs[256];

T_INTERRUPTS_CHIP g_interrupts_chip = E_INTERRUPTS_CHIP_UNK;

void interrupts_set_chip(T_INTERRUPTS_CHIP Chip)
{
    g_interrupts_chip = Chip;

    if (Chip == E_INTERRUPTS_CHIP_UNK)
        kprintf("[i] Interrupts: Turning off.\n");
    else if (Chip == E_INTERRUPTS_CHIP_PIC)
        kprintf("[i] Interrupts: Switching to 8259 based interrupts.\n");
    else if (Chip == E_INTERRUPTS_CHIP_APIC)
        kprintf("[i] Interrupts: Switching to intel I/O APIC based "
                "interrupts.\n");
}

u8 interrupts_init_idt(void)
{
    g_idt_ptr.Limit = 256 * 8;

     u32 Physical = 0;
     u8 Result = paging_get_physical((u32)g_idt_entries, &Physical);
     if (!Result)
         return 0;

     kprintf("[i] Setting up IDT at 0x%x (0x%x Virtual).\n", Physical, g_idt_entries);
    
    g_idt_ptr.Base = Physical;

    // Null those entries!
    for (u16 i = 0; i < 256; i++)
    {
        // Maybe I should access the struct's members...
        // Or i can just cast that to to u32's and null them.
        // This will set the Present flag to 0 either way
        *((u32 *)(&g_idt_entries[i])) = 0;
        *(((u32 *)(&g_idt_entries[i]) + 1)) = 0;
    }

    return 1; 
}

// This creates an ASM stub for 

// This creates a 12-byte ASM stub for a handler
void interrupts_create_stub(T_ISR_STUB *Destination, u32 Address)
{
    // The ASM code is as follows:
    //   cli
    //   pushad
    //   mov eax, Handler
    //   call eax
    //   popad
    //   sti
    //   iret
    Destination->Code1 = 0x60FA;        // pishad, cli
    Destination->Code2 = 0xB8;          // mov eax,
    Destination->Handler = Address;     // Address
    Destination->Code3 = 0xFB61D0FF;    // sti, popad, call eax
    Destination->Code4 = 0xCF;          // iret
}

void interrupts_setup_irq(u8 IRQ, void *Handler)
{
    if (g_interrupts_chip != E_INTERRUPTS_CHIP_PIC)
    {
        kprintf("[e] Sorry, but I only do PIC-based interrupts for now :(.\n");
        return;
    }
    
    u8 Interrupt = IRQ + PIC_IRQ_START;
    kprintf("[i] New handler for IRQ %i (Interrupt %i).\n", IRQ, Interrupt);
    interrupts_setup_isr(Interrupt, Handler, E_INTERRUPTS_RING0);

    // We also need to set the IRQ mask
    pic_unmask_irq(IRQ);
}

void interrupts_dump_idt_ptr(void)
{
    kprintf("[i] IDT Pointer structure:\n");
    kprintf("   Base:  0x%x.\n", g_idt_ptr.Base);
    kprintf("   Limit: 0x%x.\n", g_idt_ptr.Limit);
}

void interrupts_dump_idt_entry(u8 Interrupt)
{
    u32 *dwEntry = (u32 *)&g_idt_entries[Interrupt];
    kprintf("[i] IDT Entry for interrupt %x:\n", Interrupt);
    kprintf("   DW Low : 0x%x.\n", *dwEntry);
    kprintf("   DW High: 0x%x.\n", *(dwEntry + 1));
    
    T_IDT_ENTRY Entry = g_idt_entries[Interrupt];
    kprintf("   Offset: 0x%x.\n", (Entry.OffsetHigh << 16) + Entry.OffsetLow);
    kprintf("   Selector: %u.\n", Entry.Selector);
    kprintf("   Zero: %u.\n", Entry.Zero);
    kprintf("   P: %u.\n", (Entry.Type >> 7) & 0b1);
    kprintf("   DPL: %u.\n", (Entry.Type >> 5) & 0b11);
    kprintf("   S: %u.\n", (Entry.Type >> 4) & 0b1);
    kprintf("   Gate type: %u.\n", Entry.Type & 0b1111);
}

void interrupts_setup_isr_raw(u8 Interrupt, void *ASMHandler, \
                              T_INTERRUPTS_RING Ring)
{
    u32 uASMHandler = (u32)ASMHandler;
    g_idt_entries[Interrupt].OffsetLow = uASMHandler & 0xFFFF;
    g_idt_entries[Interrupt].OffsetHigh = (uASMHandler >> 16) & 0xFFFF;
    g_idt_entries[Interrupt].Selector = 0x08;
    g_idt_entries[Interrupt].Zero = 0;
    
    u8 Type = 0;
    Type |= (1 << 7);
    Type |= (Ring << 5);
    Type |= (0 << 4);
    Type |= 0xE;
    g_idt_entries[Interrupt].Type = Type;
}

void interrupts_setup_isr(u8 Interrupt, void *Handler, \
                          T_INTERRUPTS_RING Ring)
{
    interrupts_create_stub(&g_isr_stubs[Interrupt], (u32)Handler);
    
    u32 ASMHandler = (u32)&g_isr_stubs[Interrupt];
    interrupts_setup_isr_raw(Interrupt, (void*)ASMHandler, Ring);
}

void interrupts_delete_isr(u8 Interrupt)
{
    *((u32*)&g_idt_entries[Interrupt]) = 0;
}

void interrupts_init_simple(void)
{
    interrupts_set_chip(E_INTERRUPTS_CHIP_PIC);
    interrupts_init_idt();
    interrupts_lidt();
}

void interrupts_irq_finish(u8 IRQ)
{
    if (g_interrupts_chip == E_INTERRUPTS_CHIP_PIC)
        pic_eoi(IRQ);
}
