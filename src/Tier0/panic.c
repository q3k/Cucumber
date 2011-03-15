#include "Tier0/panic.h"
#include "Tier0/kstdio.h"

#define KPANIC_HEADER "**** KERNEL PANIC ****"

void kpanic(const s8 *Error, const s8 *File, u32 Line)
{
    __asm__ volatile("cli");
    
    kclear();
    
    u8 Margin = (80 - kstrlen(KPANIC_HEADER)) / 2;
    
    for (u8 i = 0; i < Margin; i++)
        kprintf(" ");
    
    kprintf(KPANIC_HEADER);
    kprintf("\n");
    
    kprintf("\n");
    
    Margin = (80 - kstrlen(Error)) / 2;
    for (u8 i = 0; i < Margin; i++)
        kprintf(" ");
    kprintf("%s\n", Error);
    
    Margin = (62 - kstrlen(File)) / 2;
    for (u8 i = 0; i < Margin; i++)
        kprintf(" ");
    
    kprintf(" in file %s, line %i\n", File, Line);
    
    // Dumping registers
    
    kprintf("\n  register dump:\n");
    
    u32 cr0, cr3, eax, ebx, ecx, edx, esi, edi, ebp, esp, cs, ds;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    __asm__ volatile("mov %%cr3, %0": "=r"(cr3));
    
    __asm__ volatile("mov %%eax, %0": "=r"(eax));
    __asm__ volatile("mov %%ebx, %0": "=r"(ebx));
    __asm__ volatile("mov %%ecx, %0": "=r"(ecx));
    __asm__ volatile("mov %%edx, %0": "=r"(edx));
    
    __asm__ volatile("mov %%esi, %0": "=r"(esi));
    __asm__ volatile("mov %%edi, %0": "=r"(edi));
    __asm__ volatile("mov %%ebp, %0": "=r"(ebp));
    __asm__ volatile("mov %%esp, %0": "=r"(esp));
    
    __asm__ volatile("mov %%cs, %0": "=r"(cs));
    __asm__ volatile("mov %%ds, %0": "=r"(ds));
    
    kprintf("        cr0: 0x%X cr3: 0x%x  cs: 0x%x  ds: 0x%x\n",
        cr0, cr3, cs, ds);
    kprintf("        eax: 0x%X ebx: 0x%x ecx: 0x%x edx: 0x%x\n",
        eax, ebx, ecx, edx);
    kprintf("        esi: 0x%X edi: 0x%x ebp: 0x%x esp: 0x%x\n",
        esi, edi, ebp, esp);
    
    s32 FrameSize = ebp - esp;
    
    if (FrameSize > 0 && FrameSize < 0x100)
    {
        kprintf("\n  stack frame looks promising...\n");
        kprintf("  attempting stack dump:\n");
        
        u32 Number = 80;
        for (u32 *v = (u32*)esp; v < ((u32 *)esp + Number); v+=8)
        {
            kprintf("    %x %x %x %x %x %x %x %x\n",
               *v, *(v+1), *(v+2), *(v+3), *(v+4), *(v+5), *(v+6), *(v+7));
        }
    }
    else
        kprintf("\n  stack looks unusable, not dummping.\n");
    
    kprintf("\n  if you want to keep using the OS, please reset your PC.");
    
    for(;;){}
}

void kassert(u8 Value, const s8* File, u32 Line)
{
    if (Value == 0)
        kpanic("assertion error", File, Line);
}

