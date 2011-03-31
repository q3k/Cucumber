#include "Tier1/CPageDirectory.h"
using namespace cb;

#include "Tier0/heap.h"

CPageDirectory::CPageDirectory(void)
{
    u32 PhysicalDirectory;
    m_Directory = (T_PAGING_DIRECTORY *)kmalloc_p(sizeof(T_PAGING_DIRECTORY),
        &PhysicalDirectory);
    
    m_Directory->Physical = PhysicalDirectory;
    
    
}
