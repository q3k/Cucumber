#include "Tier0/exceptions.h"
#include "Tier0/interrupts.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

#define PAGEFAULT_TEXT "Page Fault (_____)."
#define RERR_TO_R(RERR, R) R.eax = RERR.eax; \
                           R.ebx = RERR.ebx; \
                           R.ecx = RERR.ecx; \
                           R.edx = RERR.edx; \
                           R.edi = RERR.edi; \
                           R.esi = RERR.eip; \
                           R.esp = RERR.esp; \
                           R.ebp = RERR.ebx; \
                           R.cs  = RERR.cs;

void exceptions_init_simple(void)
{
    interrupts_setup_isr(0x00, (void*)exceptions_division_by_zero_isr,
                        E_INTERRUPTS_RING0);
    interrupts_setup_isr(0x0E, (void*)exceptions_page_fault_isr,
                        E_INTERRUPTS_RING0);
}

void exceptions_division_by_zero_isr(T_ISR_REGISTERS Registers)
{
    PANIC_EX("Divison by Zero.", Registers);
}

void exceptions_page_fault_isr(T_ISR_REGISTERS_ERR Registers)
{
    u32 FaultAddress;
    __asm__ volatile("mov %%cr2, %0" : "=r" (FaultAddress));
    
    u8 Present = !(Registers.Error & 0x01);
    u8 Write = Registers.Error & 0x02;
    u8 User = Registers.Error & 0x04;
    u8 Reserved = Registers.Error & 0x08;
    u8 Execute = Registers.Error &0x10;
    
    s8 *Error = PAGEFAULT_TEXT;
    u8 o = 12;
    if (Present)
        Error[o] = 'p';
    if (Write)
        Error[o + 1] = 'w';
    if (User)
        Error[o + 2] = 'u';
    if (Reserved)
        Error[o + 3] = 'r';
    if (Execute)
        Error[o + 4] = 'x';
    
    T_ISR_REGISTERS R;
    RERR_TO_R(Registers, R); 
    for(;;){}       
    //PANIC_EX_HEX(Error, R, FaultAddress);
}
