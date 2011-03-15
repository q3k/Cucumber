#ifndef __CDRIVERRAMDISK_H__
#define __CDRIVERRAMDISK_H__

#include "Tier1/Drivers/IDriver.h"
#include "Tier1/Drivers/IDevice.h"
#include "Tier1/CKernel.h"

namespace cb {
    class CDriverRamdisk : public IDriver, IDevice {
        public:
            // Driver interface
            const s8 *GetName(void);
            const s8 *GetDescription(void);
            const s8 *GetAuthor(void);
            EDriverClass GetClass(void);
            EDriverLoadMethod GetLoadMethod(void);
            bool CanUnload(void);
            
            u8 Load(CKernel *Kernel);
            u8 Unload(void);
            
            // Device interace
            u32 GetSize(void);
            const u8 *Read(u32 Offset, u32 Length);
            void Write(u32 Offset, u32 Length, const u8 *Data);
            IDeviceOperations GetSupportedOperations(void);
        private:
            CKernel *m_Kernel;
    };
};

#endif
