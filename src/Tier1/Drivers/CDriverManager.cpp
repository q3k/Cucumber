#include "Tier1/Drivers/CDriverManager.h"
using namespace cb;

CDriverManager::CDriverManager(u32 MaxDrivers, CKernel *Kernel)
{
    m_Kernel = Kernel;

    m_aDrivers = new CDriverManagerEntry[MaxDrivers];
    m_nMaxDrivers = MaxDrivers;
    for (u32 i = 0; i < m_nMaxDrivers; i++)
        m_aDrivers[i].Present = false;
}

void CDriverManager::AddDriver(IDriver *Driver)
{
    u32 Index = 0;
    while (m_aDrivers[Index].Present)
        Index++;
    
    m_aDrivers[Index].Present = true;
    m_aDrivers[Index].New = true;
    m_aDrivers[Index].Loaded = false;
    m_aDrivers[Index].Driver = Driver;
}

void CDriverManager::LoadNew(void)
{
    for (u32 i = 0; i < m_nMaxDrivers; i++)
    {
        if (m_aDrivers[i].Present && m_aDrivers[i].New)
        {
            m_aDrivers[i].New = false;
            LoadDriver(i);
        }
    }
}

void CDriverManager::LoadDriver(u32 Index)
{
    IDriver *Driver = m_aDrivers[Index].Driver;
        
    u8 Result = Driver->Load(m_Kernel);
    if (Result == 0)
    {
        m_aDrivers[Index].Loaded = true;
        m_Kernel->Logger() << "Loaded driver " << Driver->m_Name << ".";
        m_Kernel->Logger().Flush();
    }
    else
    {
        m_Kernel->Logger() << "Could not load driver.";
        m_Kernel->Logger().Flush();
    }
}
