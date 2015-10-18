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
    
    m_Logger = new CLogger();
    Alentours::CPCIManager::Initialize();
    CKernelML4 *ML4 = new CKernelML4();
    CTask *KernelTask = new CTask(*ML4);
    kprintf("[i] Kernel task has PID %i.\n", KernelTask->GetPID());
    CScheduler::AddTask(KernelTask);
        
    ML4->Apply();
    ML4->UseStack([](CKernelML4 *ML4) {
        kprintf("[i] Switched to Tier1 stack\n");
        u64 RSP;
        __asm__ __volatile__("movq %%rsp, %0" :"=r"(RSP));
        kprintf("[i] RSP is %X\n", RSP);

        CScheduler::Enable();
        // After enabling, only CScheduler::* calls are allowed for API
        kprintf("[i] Enabled scheduler.\n");
        g_Kernel.SpawnThreads();
        for(;;) { asm volatile("hlt"); }
    });
}

void CKernel::SpawnThreads(void)
{
    CScheduler::Spawn([](u64 foo) {
        for (;;) {
            kprintf("child 1!\n");
            CScheduler::Sleep(100);
        }
        CScheduler::Exit();
    }, 0);
    CScheduler::Spawn([](u64 foo) {
        for (;;) {
            kprintf("child 2!\n");
            CScheduler::Sleep(100);
        }
        CScheduler::Exit();
    }, 0);
}
