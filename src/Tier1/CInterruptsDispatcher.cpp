#include "Tier1/CInterruptDispatcher.h"
using namespace cb;

void CInterruptDispatcher::InitializeStatic(void)
{
    if (m_InitializedStatic)
        return;
    
    m_InitializedStatic = true;
    for (u16 i = 0; i < 256; i++)
        m_Dispatchers[i] = 0;
    
    PPHAX_DO256(CID_SDID_SET);
}

CInterruptDispatcher::CInterruptDispatcher(void)
{
    if (!m_InitializedStatic)
        InitializeStatic();
    m_Interrupt = 0;
    m_Enabled = false;
}

void CInterruptDispatcher::Enable(void)
{
    if (m_Enabled)
        return;
    
    m_Enabled = true;
    
    m_Dispatchers[m_Interrupt] = this;
    
    interrupts_setup_isr(m_Interrupt, (void*)m_DispatcherFunctions[m_Interrupt],
                        E_INTERRUPTS_RING0);
}

void CInterruptDispatcher::Disable(void)
{
    if (!m_Enabled)
        return;
    
    m_Enabled = false;
    
    interrupts_delete_isr(m_Interrupt);
}

u8 CInterruptDispatcher::GetInterrupt(void)
{
    return m_Interrupt;
}

bool CInterruptDispatcher::GetEnabled(void)
{
    return m_Enabled;
}

void CInterruptDispatcher::Dispatch(void *Registers)
{
    return;
}

//All of the interrupt static implementations
PPHAX_DO256(CID_SDIS_IMP);

void *CInterruptDispatcher::m_DispatcherFunctions[256];
CInterruptDispatcher *CInterruptDispatcher::m_Dispatchers[256];
bool CInterruptDispatcher::m_InitializedStatic = false;
