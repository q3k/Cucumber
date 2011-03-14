#ifndef __CKERNEL_H__
#define __CKERNEL_H__

#include "types.h"
#include "Tier1/CLogger.h"
#include "Tier1/Drivers/CDriverManager.h"

#define CKERNEL_MAGIC 0x8BA67FE9

namespace cb {
    class CDriverManager;
    class CKernel {
        public:
            CKernel(void);
            void Start(void);
            CLogger &Logger(void);
        private:
            u32 m_Magic;
            CLogger *m_Logger;
            CDriverManager *m_DriverManager;
    };
};

#endif
