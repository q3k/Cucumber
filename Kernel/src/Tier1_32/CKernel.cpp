#include "Tier1/CKernel.h"
#include "Tier1/CPageFaultDispatcher.h"
#include "Tier1/CPageDirectory.h"
#include "Tier1/CTask.h"
#include "Tier1/CScheduler.h"
#include "Tier1/Util/CVector.h"
#include "Tier1/Util/CLinearList.h"
#include "Tier1/CTimer.h"

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
    kprintf("[i] Hello from C++ land!\n");
       
    if (m_dwMagic != CKERNEL_MAGIC)
    {
        kprintf("[e] Error! My constructor wasn't called properly.\n");
        return;
    }
    
    m_Logger = new CLogger();

    CTask *KernelTask = CreateKernelTask();
    kprintf("[i] Kernel task has TID %i.\n", KernelTask->GetPID());
    CScheduler::AddTask(KernelTask);
    CScheduler::Enable();
    
    CTask *ParentTask = CScheduler::GetCurrentTask();    
    CTask *NewTask = ParentTask->Fork();
    CTimer::GetTicks();
    if (NewTask == ParentTask)
    {
    
        for (;;) {
            for (volatile u32 i = 0; i < 14000; i++)
            {
                for (volatile u32 j = 0; j < 650; j++){}
            }
            kprintf("[i] Hello! I'm the parent process %i.\n", CTimer::GetTicks());
        }
    }
    else
    {
        for (;;) {
            //CScheduler::GetCurrentTask()->Sleep(1000);
        	for (volatile u32 i = 0; i < 14000; i++)
        	            {
        	                for (volatile u32 j = 0; j < 650; j++){}
        	            }
            kprintf("[i] Hello! I'm the child process %i.\n", CTimer::GetTicks());
        }
    }
}

extern T_PAGING_DIRECTORY g_kernel_page_directory;
extern CPageDirectory *g_KernelPageDirectory;
CTask *CKernel::CreateKernelTask(void)
{
    // Create the directory wrapper
    CPageDirectory *Directory = new CPageDirectory(true);
    // Every table already exists.
    for (u8 i = 0; i < 32; i++)
        Directory->m_OwnerBitmap[i] = 1;
    Directory->m_Directory = &g_kernel_page_directory;
    g_KernelPageDirectory = Directory;
    
    // Create the task wrapper
    CTask *Task = new CTask(false, true);
    Task->m_Directory = Directory;
    
    // Just say a stack is here, as it already is -- see kmain.c
    Task->m_StackStart = Directory->Translate(TASK_MAP_STACK_START);
    Task->m_StackSize = TASK_MAP_STACK_SIZE;
    
    // Same goes for kernel memory
    Task->m_KernelStart = 0xC0000000;
    Task->m_KernelSize = 0x20000000;
    
    return Task;
}
