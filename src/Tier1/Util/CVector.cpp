#include "Tier1/Utils/CVector.h"
using namespace cb;

extern "C" {
    #include "Tier0/heap.h"
};

#define tmpl template <typename _T>

tmpl CVector::CVector(u32 StartElements)
{
    m_Size = m_MemorySize = StartElements;
    
    if (m_Size > 0)
        m_Members = (_T*)malloc(m_Size * sizeof(_T));
}

tmpl CVector::~CVector(void)
{
    if (m_MemorySize > 0)
        
}
