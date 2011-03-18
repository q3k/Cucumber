// A dummy driver that says hello.

#ifndef __CDRIVERDUMMY_H__
#define __CDRIVERDUMMY_H__

#include "Tier1/Drivers/IDriver.h"

namespace cb {
    class CDriverDummy : public IDriver {
        public:
            u8 Load(CKernel *Kernel);
            u8 Unload(void);
        private:
            CKernel *m_Kernel;
    };
};

#endif
