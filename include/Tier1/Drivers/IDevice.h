#ifndef __IDEVICE_H__
#define __IDEVICE_H__

#include "types.h"

namespace cb {
    enum IDeviceOperations {
        IDO_GET_SIZE = 1,
        IDO_READ = 2,
        IDO_WRITE = 4
    };
    class IDevice {
        public:
            virtual u32 GetSize(void) = 0;
            virtual const u8 *Read(u32 Offset, u32 Length) = 0;
            virtual void Write(u32 Offset, u32 Length, const u8 *Data) = 0;
            
            IDeviceOperations m_DeviceOperations;
    };
};

#endif
