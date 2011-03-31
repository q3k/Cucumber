#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
#include "Tier0/paging.h"

// This is more-or less just a C++ wrapper for T_PGAE_DIRECTORY.
namespace cb {
    class CPageDirectory {
        private:
            T_PAGING_DIRECTORY *m_Directory;
        public:
            // Creates a new page directory for a kernel task
            CPageDirectory(void);
    };
};

#endif
