#include "Tier1/CPageFaultDispatcher.h"
using namespace cb;

extern "C" {
    #include "Tier0/panic.h"
};

CPageFaultDispatcher::CPageFaultDispatcher(void)
{
    m_Interrupt = 0x0E;
    Enable();
}

void CPageFaultDispatcher::Dispatch(void *Registers)
{
    PANIC("PAGE FAULT LAWL");
}
