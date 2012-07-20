#include "Tier0/exceptions.h"
#include "Tier0/interrupts.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

#define PAGEFAULT_TEXT "Page Fault (_____)."
#define RERR_TO_R(RERR, R) R.rax = RERR.rax; \
                           R.rbx = RERR.rbx; \
                           R.rcx = RERR.rcx; \
                           R.rdx = RERR.rdx; \
                           R.rdi = RERR.rdi; \
                           R.rsi = RERR.rip; \
                           R.rsp = RERR.rsp; \
                           R.rbp = RERR.rbp; \
                           R.cs  = RERR.cs; \
                           R.rip = RERR.rip; \
                           R.rflags = RERR.rflags; \
                           R.ss = RERR.ss; \
                           R.r8 = RERR.r8; \
                           R.r9 = RERR.r9; \
                           R.r10 = RERR.r10; \
                           R.r11 = RERR.r11; \
                           R.r12 = RERR.r12; \
                           R.r13 = RERR.r13; \
                           R.r14 = RERR.r14; \
                           R.r15 = RERR.r15;

void exceptions_init_simple(void)
{
    interrupts_setup_isr(0x00, (void*)exceptions_division_by_zero_isr,
                        E_INTERRUPTS_RING0);
    interrupts_setup_isr(0x0D, (void*)exceptions_general_protection_isr,
                        E_INTERRUPTS_RING0);
    interrupts_setup_isr(0x0E, (void*)exceptions_page_fault_isr,
                        E_INTERRUPTS_RING0);
    interrupts_setup_isr(0x10, (void*)exceptions_floating_point_isr,
                        E_INTERRUPTS_RING0);
}

void exceptions_floating_point_isr(T_ISR_REGISTERS Registers)
{
    PANIC("Floating point exception. wat.");
}

void exceptions_general_protection_isr(T_ISR_REGISTERS_ERR Registers)
{
    T_ISR_REGISTERS NoErr;
    RERR_TO_R(Registers, NoErr);
    PANIC_EX("General protection fault in kernel task.", NoErr);
}

void exceptions_division_by_zero_isr(T_ISR_REGISTERS Registers)
{
    PANIC_EX("Divison by Zero.", Registers);
}

void exceptions_page_fault_isr(T_ISR_REGISTERS_ERR Registers)
{
    u64 FaultAddress;
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
    PANIC_EX_HEX(Error, R, FaultAddress);
}
