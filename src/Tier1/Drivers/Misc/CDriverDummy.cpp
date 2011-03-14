#include "Tier1/Drivers/Misc/CDriverDummy.h"
using namespace cb;

const s8 *CDriverDummy::GetDescription(void)
{
    return "Dummy Driver";
}

const s8 *CDriverDummy::GetName(void)
{
    return "org.q3k.drivers.dummy";
}

const s8 *CDriverDummy::GetAuthor(void)
{
    return "Sergiusz Bazanski";
}

EDriverClass CDriverDummy::GetClass(void)
{
    return EDC_NONE;
}

EDriverLoadMethod CDriverDummy::GetLoadMethod(void)
{
    return EDLM_ALWAYS;
}

bool CDriverDummy::CanUnload(void)
{
    return true;
}

u8 CDriverDummy::Load(CKernel *Kernel)
{
    m_Kernel = Kernel;
    Kernel->Logger() << "Dummy driver loaded!";
    Kernel->Logger().Flush();
    
    return 0;
}

u8 CDriverDummy::Unload(void)
{
    m_Kernel->Logger() << "Dummy driver unloaded!";
    m_Kernel->Logger().Flush();
    return 0;
}
