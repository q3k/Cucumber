#include "Tier1/CKernel.h"
#include "Tier1/CKernelML4.h"
#include "Tier1/CTask.h"
#include "Tier1/CScheduler.h"
#include "Tier1/Util/CVector.h"
#include "Tier1/Util/CLinearList.h"
#include "Tier1/CTimer.h"
#include "Alentours/PCI.h"

using namespace cb;

CKernel g_Kernel;

extern "C" {
    #include "Tier0/kstdio.h"
    #include "Tier0/panic.h"
    #include "Tier0/heap.h"
    
    void CKernelStart(void)
    {
        g_Kernel.Start();
    }
}

CKernel::CKernel(void)
{
    m_dwMagic = CKERNEL_MAGIC;
}

CLogger &CKernel::Logger(void)
{
    return *m_Logger;
}

void CKernel::Start(void)
{
    if (m_dwMagic != CKERNEL_MAGIC)
    {
        kprintf("[e] Error! My constructor wasn't called properly.\n");
        return;
    }
    kprintf("wat\n");
    
    m_Logger = new CLogger();
    Alentours::CPCIManager::Initialize();

    CKernelML4 *ML4 = new CKernelML4();
    CTask *KernelTask = new CTask(*ML4);
    CScheduler::AddTask(KernelTask);
    kprintf("[i] Kernel task has PID %i.\n", KernelTask->GetPID());
        
    ML4->Apply();
    KernelTask->UseStack([](CTask *Task) {
        kprintf("[i] Switched to Tier1 stack\n");

        // After enabling, only CScheduler::* calls are allowed for API
        CScheduler::Enable();
        kprintf("[i] Enabled scheduler.\n");

        g_Kernel.SpawnThreads();
        CScheduler::Exit();
    });
}

void CKernel::SpawnThreads(void)
{
    for (u64 i = 0; i < 10; i++) {
        CScheduler::Spawn([](u64 ID) {
            for (;;) {
                kprintf("%i", ID);
                CScheduler::Sleep(10*ID);
            }
            CScheduler::Exit();
        }, i);
    }
}
