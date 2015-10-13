#ifndef __CKERNEL_H__
#define __CKERNEL_H__

#include "types.h"
#include "Tier1/CLogger.h"
//#include "Tier1/CTask.h"

#define CKERNEL_MAGIC 0x8BA67FE9

namespace cb {
    class CTask;
    class CKernel {
        public:
            CKernel(void);
            void Start(void);
            CLogger &Logger(void);
            
        private:
            u32 m_dwMagic;
            CLogger *m_Logger;

            void SpawnThreads(void);
            
//            CTask *CreateKernelTask(void);
    };
};

#endif
