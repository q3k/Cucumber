#include "Tier0/panic.h"
#include "Tier0/kstdio.h"
//#include "Tier0/prng.h"
#include "preprocessor_hacks.h"

#define KPANIC_HEADER(n) KPANIC_HEADER##n

#define KPANIC_HEADER0 " *** kernel fucking panic ***"
#define KPANIC_HEADER1 " *** fucked up beyond all repair ***"
#define KPANIC_HEADER2 " *** situation normal - all fucked up ***"
#define KPANIC_HEADER3 " *** well shit. ***"
#define KPANIC_HEADER4 " *** oh noes!1!1! ***"
#define KPANIC_HEADER5 " *** crap! ***"
#define KPANIC_HEADER6 " *** flail your arms, for this is a kernel panic ***"
#define KPANIC_HEADER7 " *** oh fuck oh fuck oh fuck oh fuck oh fu...***"
#define KPANIC_HEADER8 " *** you get to keep the pieces ***"
#define KPANIC_HEADER9 " *** just give up already ***"

#define KPANIC_CASE(n) case n: \
    return KPANIC_HEADER(n);

char *kpanic_get_random_message(void)
{
    //u16 N = krand() % 10;
	u16 N = 0;
    switch (N)
    {
        PPHAX_DO10(KPANIC_CASE);
    }
    return KPANIC_HEADER0;
}

void kpanic_ex(const s8 *Error, const s8 *File, u32 Line, T_ISR_REGISTERS R)
{
    __asm__ volatile("cli");
    
    kclear();
    
    char *Message = kpanic_get_random_message();
    
    u8 Margin = (80 - kstrlen(Message)) / 2;
    
    for (u8 i = 0; i < Margin; i++)
        kprintf(" ");
    
    kprintf(Message);
    kprintf("\n");
    
    kprintf("\n");
    
    Margin = (80 - kstrlen(Error)) / 2;
    for (u8 i = 0; i < Margin; i++)
        kprintf(" ");
    kprintf("%s\n", Error);
    
    if (File != 0)
    {
        Margin = (62 - kstrlen(File)) / 2;
        for (u8 i = 0; i < Margin; i++)
            kprintf(" ");
        
        kprintf(" in file %s, line %i\n", File, Line);
    }
    else
    {
        Margin = 36;
        for (u8 i = 0; i < Margin; i++)
            kprintf(" ");
        
        kprintf("%x\n", Line);
    }
    
    // Dumping registers
    
    /*u32 ds, cr0, cr3;
    
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    __asm__ volatile("mov %%cr3, %0": "=r"(cr3));
    __asm__ volatile("mov %%ds, %0": "=r"(ds));
    
    kprintf("\n  register dump:\n");
    
    kprintf("        cr0: 0x%X cr3: 0x%x  cs: 0x%x  cs: 0x%x\n",
        cr0, cr3, R.cs, ds);
    kprintf("        eax: 0x%X ebx: 0x%x ecx: 0x%x edx: 0x%x\n",
        R.eax, R.ebx, R.ecx, R.edx);
    kprintf("        esi: 0x%X edi: 0x%x ebp: 0x%x esp: 0x%x\n",
        R.esi, R.edi, R.ebp, R.esp);*/
    
    //s32 FrameSize = R.ebp - R.esp;
    
    /*if (FrameSize > 0 && FrameSize < 0x100)
    {*/
        /*kprintf("\n  stack frame looks promising...\n");
        kprintf("  attempting stack dump:\n");
        
        u32 Number = 80;
        for (u32 *v = (u32*)R.esp; v < ((u32 *)R.esp + Number); v+=8)
        {
            kprintf("    %x %x %x %x %x %x %x %x\n",
               *v, *(v+1), *(v+2), *(v+3), *(v+4), *(v+5), *(v+6), *(v+7));
        }*/
    /*}
    else
        kprintf("\n  stack looks unusable, not dummping.\n");*/
    
    kprintf("\n  if you want to keep using the OS, please reset your PC.");
    
    for(;;){}
}

void kpanic(const s8 *Error, const s8 *File, u32 Line)
{
    __asm__ volatile("cli");
    
    T_ISR_REGISTERS R;
        
    __asm__ volatile("mov %%rax, %0": "=r"(R.rax));
    __asm__ volatile("mov %%rbx, %0": "=r"(R.rbx));
    __asm__ volatile("mov %%rcx, %0": "=r"(R.rcx));
    __asm__ volatile("mov %%rdx, %0": "=r"(R.rdx));
    
    __asm__ volatile("mov %%rsi, %0": "=r"(R.rsi));
    __asm__ volatile("mov %%rdi, %0": "=r"(R.rdi));
    __asm__ volatile("mov %%rbp, %0": "=r"(R.rbp));
    __asm__ volatile("mov %%rsp, %0": "=r"(R.rsp));
    
    __asm__ volatile("mov %%cs, %0": "=r"(R.cs));
    
    kpanic_ex(Error, File, Line, R);
}

void kassert(u8 Value, const s8* File, u32 Line)
{
    if (Value == 0)
        kpanic("assertion error", File, Line);
}

