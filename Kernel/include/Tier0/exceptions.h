#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include "types.h"
#include "interrupts.h"

void exceptions_init_simple(void);
void exceptions_page_fault_isr(T_ISR_REGISTERS_ERR Registers);
void exceptions_general_protection_isr(T_ISR_REGISTERS_ERR Registers);
void exceptions_division_by_zero_isr(T_ISR_REGISTERS Registers);
void exceptions_floating_point_isr(T_ISR_REGISTERS Registers);

#endif
