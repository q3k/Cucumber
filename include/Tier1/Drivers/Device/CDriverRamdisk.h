#ifndef __CDRIVERRAMDISK_H__
#define __CDRIVERRAMDISK_H__

#include "Tier1/Drivers/IDriver.h"
#include "Tier1/Drivers/IDevice.h"
#include "Tier1/CKernel.h"

namespace cb {
    class CDriverRamdisk : public IDriver, IDevice {
        public:
            u8 Load(CKernel *Kernel);
            u8 Unload(void);
            
            // Device interace
            u32 GetSize(void);
            const u8 *Read(u32 Offset, u32 Length);
            void Write(u32 Offset, u32 Length, const u8 *Data);
        private:
            CKernel *m_Kernel;
    };
};

#endif
