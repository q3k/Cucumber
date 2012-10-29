#include "Tier0/interrupts.h"
#include "Tier0/paging.h"
#include "Tier0/kstdio.h"
#include "Tier0/pic.h"
#include "preprocessor_hacks.h"

struct {
    // IDT
    T_IDT_PTR IDTPointer;
    T_IDT_ENTRY IDTEntries[256];
    T_ISR_STUB ISRStubs[256];

} __attribute__((packed)) g_Interrupts;

void interrupts_lidt(void)
{
    __asm__ __volatile__("movq %0, %%rax; lidt (%%rax)" : : "p"(&g_Interrupts.IDTPointer));
}

u8 interrupts_init_idt(void)
{
    g_Interrupts.IDTPointer.Limit = 256 * 16;
    g_Interrupts.IDTPointer.Base = (u64)g_Interrupts.IDTEntries;
    kprintf("[i] Setting up IDT at 0x%x.\n", g_Interrupts.IDTEntries);
    kprintf("[i] IDT Entry size %i bytes.\n", sizeof(T_IDT_ENTRY));

    // Null those entries!
    for (u16 i = 0; i < 256; i++)
    {
        // Maybe I should access the struct's members...
        // Or i can just cast that to to u32's and null them.
        // This will set the Present flag to 0 either way
        *((u64 *)(&g_Interrupts.IDTEntries[i])) = 0;
        *(((u64 *)(&g_Interrupts.IDTEntries[i]) + 1)) = 0;
    }

    return 1; 
}
void interrupts_create_stub(T_ISR_STUB *Destination, u64 Address)
{
    Destination->Code1 = 0x57565552515350fa;        
    Destination->Code2 = 0x5341524151415041;
    Destination->Code3 = 0x5741564155415441;
    Destination->Code4 = 0xb848;
    Destination->Handler = Address;     // Address
    Destination->Code5 = 0x5d415e415f41d0ff;
    Destination->Code6 = 0x59415a415b415c41;
    Destination->Code7 = 0x5b595a5d5e5f5841;
    Destination->Code8 = 0xcf48fb58;
}
/*
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
}*/

void interrupts_dump_idt_entry(u8 Interrupt)
{
    kprintf("[i] IDT Entry for interrupt %x, %x:\n", Interrupt, &g_Interrupts.IDTEntries[Interrupt]);
    T_IDT_ENTRY Entry = g_Interrupts.IDTEntries[Interrupt];
    kprintf("   Offset: 0x%x.\n", (u64 )((u64)Entry.OffsetHigh << 32) + ((u64)Entry.OffsetMiddle << 16) + (u64)Entry.OffsetLow);
    kprintf("   Selector: %u.\n", Entry.Selector);
    kprintf("   P: %u.\n", Entry.Present);
    kprintf("   DPL: %u.\n", Entry.DPL);
    kprintf("   S: %u.\n", Entry.Selector);
    kprintf("   Gate type: %u.\n", Entry.Type);
}

void interrupts_setup_isr_raw(u8 Interrupt, void *ASMHandler, \
                              T_INTERRUPTS_RING Ring)
{
    u64 uASMHandler = (u64)ASMHandler;
    g_Interrupts.IDTEntries[Interrupt].OffsetLow = uASMHandler & 0xFFFF;
    g_Interrupts.IDTEntries[Interrupt].OffsetMiddle = (uASMHandler >> 16) & 0xFFFF;
    g_Interrupts.IDTEntries[Interrupt].OffsetHigh = uASMHandler >> 32;

    g_Interrupts.IDTEntries[Interrupt].Selector = 0x18;
    g_Interrupts.IDTEntries[Interrupt].Zero1 = 0;
    g_Interrupts.IDTEntries[Interrupt].Zero2 = 0;
    g_Interrupts.IDTEntries[Interrupt].DPL = Ring;
    g_Interrupts.IDTEntries[Interrupt].Present = 1;
    g_Interrupts.IDTEntries[Interrupt].Type = 0b1111;
}

void interrupts_setup_isr(u8 Interrupt, void *Handler, \
                          T_INTERRUPTS_RING Ring)
{
    interrupts_create_stub(&g_Interrupts.ISRStubs[Interrupt], (u64)Handler);
    
    u64 ASMHandler = (u64)&g_Interrupts.ISRStubs[Interrupt];
    interrupts_setup_isr_raw(Interrupt, (void*)ASMHandler, Ring);
}

/*void interrupts_delete_isr(u8 Interrupt)
{
    *((u32*)&g_idt_entries[Interrupt]) = 0;
}
*/

void interrupts_remap_pic(void)
{
    // remap the PIC IRQ's to 0x20-0x27 and 0x28-0x2F

    // save masks
    u8 MaskMaster = kinb(0x21);
    u8 MaskSlave = kinb(0xA1);

    koutb(0x20, 0x11); // start initialization sequence
    koutb(0xA0, 0x11); // -   "   -

    koutb(0x21, 0x20); // master from 0x20
    koutb(0xA1, 0x28); // slave from 0x28
    koutb(0x21, 0x04); // slave PIC at IRQ2
    koutb(0xA1, 0x02); // cascade
    koutb(0x21, 0x01); // 8086 mode
    koutb(0xA1, 0x01); // 8086 mode

    // restore masks
    koutb(0x21, MaskMaster);
    koutb(0xA1, MaskSlave);
}

#define GENERIC_IRQ_DEF(i) void interrupts_irq##i##_isr(void) { kprintf("[w] OOPS: Unhandled IRQ"#i"\n"); }
#define GENERIC_IRQ_BIND(i) interrupts_setup_isr(0x20 + i, (void *)interrupts_irq##i##_isr, E_INTERRUPTS_RING0)
PPHAX_DO16(GENERIC_IRQ_DEF);

void interrupts_init_simple(void)
{
    interrupts_init_idt();
    interrupts_lidt();
    interrupts_remap_pic();

    // generic IRQ handlers
    PPHAX_DO16(GENERIC_IRQ_BIND);

    // enable hardware interrupts
    __asm__ __volatile__("sti;");
    kprintf("[i] Hardware interrupts enabled.\n");
}
/*
void interrupts_irq_finish(u8 IRQ)
{
    if (g_interrupts_chip == E_INTERRUPTS_CHIP_PIC)
        pic_eoi(IRQ);
}
*/
