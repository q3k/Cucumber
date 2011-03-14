#ifndef __IDRIVER_H__
#define __IDRIVER_H__

#include "types.h"
#include "Tier1/CKernel.h"

namespace cb {
    class CKernel;
    
    enum EDriverClass {
        EDC_NONE,
        EDC_FILESYSTEM,
        BDC_BLOCKDEVICE
    };
    enum EDriverLoadMethod {
        EDLM_ALWAYS = 1,
        EDLM_PCIPRESENT = 2,
        EDLM_USBPRESENT = 4
    };
    class IDriver {
        public:
            virtual const s8 *GetName(void) = 0;
            virtual const s8 *GetDescription(void) = 0;
            virtual const s8 *GetAuthor(void) = 0;
            virtual EDriverClass GetClass(void) = 0;
            virtual EDriverLoadMethod GetLoadMethod(void) = 0;
            virtual bool CanUnload(void) = 0;
            
            virtual u8 Load(CKernel *Kernel) = 0;
            virtual u8 Unload(void) = 0;
    };
};

#endif
