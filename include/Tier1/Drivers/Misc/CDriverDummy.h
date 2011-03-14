// A dummy driver that says hello.

#ifndef __CDRIVERDUMMY_H__
#define __CDRIVERDUMMY_H__

#include "Tier1/Drivers/IDriver.h"

namespace cb {
    class CDriverDummy : public IDriver {
        public:
            const s8 *GetName(void);
            const s8 *GetDescription(void);
            const s8 *GetAuthor(void);
            EDriverClass GetClass(void);
            EDriverLoadMethod GetLoadMethod(void);
            bool CanUnload(void);
            
            u8 Load(CKernel *Kernel);
            u8 Unload(void);
        private:
            CKernel *m_Kernel;
    };
};

#endif
