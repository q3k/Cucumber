#ifndef __ATOMIC_OPERATIONS_H__
#define __ATOMIC_OPERATIONS_H__

#include "types.h"

typedef struct {
    u32 Value;
} T_ATOMIC;

#define atomic_init(n) { (n) }
#define atomic_read(v) ((v)->Value)
#define atomic_set(v, n) (((v)->Value) = n)

static inline void atomic_add(T_ATOMIC *Atom, u32 Value)
{
    __asm__ volatile("lock addl %1, %0" : "+m"(Atom->Value) : "ir"(Value));
}

static inline void atomic_sub(T_ATOMIC *Atom, u32 Value)
{
    __asm__ volatile("lock subl %1, %0" : "+m"(Atom->Value) : "ir"(Value));
}

static inline u8 atomic_sub_and_test(T_ATOMIC *Atom, u32 Value)
{
    u8 C;
    __asm__ volatile("lock subl %2, %0\n"
                     "sete %1\n"
                     : "+m"(Atom->Value), "=qm"(C)
                     : "ir" (Value) : "memory");
   return C;
}

static inline void atomic_inc(T_ATOMIC *Atom)
{
    __asm__ volatile("lock incl %0" : "+m"(Atom->Value));
}

static inline void atomic_dec(T_ATOMIC *Atom)
{
    __asm__ volatile("lock decl %0" : "+m"(Atom->Value));
}

static inline u8 atomic_dec_and_test(T_ATOMIC *Atom)
{
    u8 C;
    __asm__ volatile("lock decl %0\n"
                     "sete %1" : "+m"(Atom->Value), "=qm"(C) :: "memory");
    return C;
}

static inline u8 atomic_inc_and_test(T_ATOMIC *Atom)
{
    u8 C;
    __asm__ volatile("lock incl %0\n"
                     "sete %1" : "+m"(Atom->Value), "=qm"(C) :: "memory");
    return C;
}

#endif
