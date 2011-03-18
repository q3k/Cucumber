#ifndef __IDRIVER_H__
#define __IDRIVER_H__

#include "types.h"
#include "Tier1/CKernel.h"

namespace cb {
    class CKernel;
    
    enum EDriverClass {
        EDC_NONE,
        EDC_FILESYSTEM,
        EDC_DEVICE,
        BDC_BLOCKDEVICE
    };
    enum EDriverLoadMethod {
        EDLM_ALWAYS = 1,
        EDLM_PCIPRESENT = 2,
        EDLM_USBPRESENT = 4
    };
    class IDriver {
        public:virtual u8 Load(CKernel *Kernel) = 0;
            virtual u8 Unload(void) = 0;
            
            const s8 *m_Name;
            const s8 *m_Description;
            const s8 *m_Author;
            
            EDriverClass m_Class;
            EDriverLoadMethod m_LoadMethod;
            
            bool m_Unloadable;
    };
};

#endif
