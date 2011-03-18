#include "Tier1/Drivers/Misc/CDriverDummy.h"
using namespace cb;

u8 CDriverDummy::Load(CKernel *Kernel)
{
    m_Name = "org.q3k.drivers.dummy";
    m_Description = "Dummy Driver";
    m_Author = "Sergiusz Bazanski";
    m_Class = EDC_NONE;
    m_LoadMethod = EDLM_ALWAYS;
    m_Unloadable = true;
    
    m_Kernel = Kernel;
    
    return 0;
}

u8 CDriverDummy::Unload(void)
{
    m_Kernel->Logger() << "Dummy driver unloaded!";
    m_Kernel->Logger().Flush();
    return 0;
}
