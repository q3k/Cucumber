#ifndef __CDRIVERMANAGER_H__
#define __CDRIVERMANAGER_H__

#include "types.h"
#include "Tier1/Drivers/IDriver.h"
#include "Tier1/CKernel.h"

namespace cb {
    class IDriver;
    class CKernel;
    
    typedef struct {
        const s8 *Name;
        IDriver *Driver;
        bool New;
        bool Loaded;
        bool Present;
    } CDriverManagerEntry;
    class CDriverManager {
        public:
            CDriverManager(u32 MaxDrivers, CKernel *Kernel);
            void AddDriver(IDriver *Driver);

            void LoadDriver(s8 *Name);
            void LoadAll(void);
            void LoadNew(void);
            
            void UnloadDriver(s8 *Name);
        private:
            CDriverManagerEntry *m_aDrivers;
            u32 m_nMaxDrivers;
            CKernel *m_Kernel;
            
            void LoadDriver(u32 Index);
    };
};

#endif
