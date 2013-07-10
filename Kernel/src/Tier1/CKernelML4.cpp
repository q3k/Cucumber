#include "Tier1/CKernelML4.h"
using namespace cb;

extern "C"
{
    #include "Tier0/paging.h"
    #include "Tier0/heap.h"
    #include "Tier0/system.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
}

#define ASSERT_ALIGNED(m) ASSERT(!(m & 0xFFF))
#define POPULATE_PAGING_ENTRY(Entry, address) do { Entry.Present = 0;\
    Entry.RW = 0; \
    Entry.User = 0; \
    Entry.Misc = 0; \
    Entry.Zero = 0; \
    Entry.Physical = address >> 12; } while(0)

void CKernelML4::PopulateCommonPointers(void)
{
    /* bullshit */
}