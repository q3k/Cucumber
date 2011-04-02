#include "types.h"

#include "Tier1/CInterruptDispatcher.h"

namespace cb {
    class CPageFaultDispatcher : public CInterruptDispatcher {
        public:
            CPageFaultDispatcher(void);
            void Dispatch(void *Registers);
    };
};
