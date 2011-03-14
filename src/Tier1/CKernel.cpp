#include "Tier1/CKernel.h"
#include "Tier1/Drivers/Misc/CDriverDummy.h"
using namespace cb;

CKernel g_Kernel;

extern "C" {
    #include "Tier0/kstdio.h"
    
    void CKernelStart(void)
    {
        g_Kernel.Start();
    }
}

CKernel::CKernel(void)
{
    m_Magic = CKERNEL_MAGIC;
}

CLogger &CKernel::Logger(void)
{
    return *m_Logger;
}

void CKernel::Start(void)
{
    kprintf("[i] Hello from C++ land!\n");
    
    if (m_Magic != CKERNEL_MAGIC)
    {
        kprintf("[e] Error! My constructor wasn't called properly.\n");
        return;
    }
    
    m_Logger = new CLogger();
    m_DriverManager = new CDriverManager(64, this);
    
    IDriver *Dummy = new CDriverDummy();
    m_DriverManager->AddDriver(Dummy);
    
    m_DriverManager->LoadNew();
    
    
    for (;;) {}
}

