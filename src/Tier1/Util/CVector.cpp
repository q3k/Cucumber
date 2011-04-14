/*#include "Tier1/Util/CVector.h"
using namespace cb;

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdlib.h"
};

template <class _T> CVector<_T>::CVector(u32 StartElements)
{
    m_Size = m_MemorySize = StartElements;
    
    if (m_Size > 0)
        m_Members = (_T*)kmalloc(m_Size * sizeof(_T));
}

template <class _T> CVector<_T>::~CVector(void)
{
    if (m_MemorySize > 0)
        kfree(m_Members);
}

template <class _T> _T &CVector<_T>::operator[](u32 Index)
{
    ASSERT(Index < m_Size);
    return m_Members[Index];
}

template <class _T> const _T &CVector<_T>::operator[](u32 Index) const
{
    ASSERT(Index < m_Size);
    return m_Members[Index];
}

template <class _T> _T &CVector<_T>::Head(void)
{
    ASSERT(m_Size > 0);
    return m_Members[Size() - 1];
}

template <class _T> const _T &CVector<_T>::Head(void) const
{
    ASSERT(m_Size > 0);
    return m_Members[Size() - 1];
}

template <class _T> _T &CVector<_T>::Tail(void)
{
    ASSERT(m_Size > 0);
    return m_Members[0];
}

template <class _T> const _T &CVector<_T>::Tail(void) const
{
    ASSERT(m_Size > 0);
    return m_Members[0];
}

template <class _T> _T *CVector<_T>::BaseAddress(void)
{
    return m_Members;
}

template <class _T> const _T *CVector<_T>::BaseAddress(void) const
{
    return m_Members;
}

template <class _T> u32 CVector<_T>::Size(void)
{
    return m_Size;
}

template <class _T> s32 CVector<_T>::Find(const _T &Element) const
{
    for (u32 i = 0; i < Size(); i++)
        if (m_Members[i] == Element)
            return i;
    return -1;
}

template <class _T> bool CVector<_T>::HasElement(const _T &Element) const
{
    for (u32 i = 0; i < Size(); i++)
        if (m_Members[i] == Element)
            return true;
    return false;
}

template <class _T> _T &CVector<_T>::Pop(void)
{
    ASSERT(m_Size > 0);
    
}

template <class _T> void CVector<_T>::MoarMemoryPlox(void)
{
    u32 NewSize = m_MemorySize * 2;
    
    _T *NewData = (_T*)kmalloc(NewSize * sizeof(_T));
    
    // Copy the data over
    kmemcpy((void*)NewData, (void*)m_Members, m_MemorySize * sizeof(_T));
    
    kfree(m_Members);
    m_Members = NewData;
    
    m_MemorySize = NewSize;
}

template <class _T> void CVector<_T>::Push(const _T &Element)
{
    if (m_Size + 1 > m_MemorySize)
        MoarMemoryPlox();
    
#ifdef CVECTOR_MEMCPY_ELEMENTS
    kmemcpy((void*)(m_Members + m_Size), (void*)&Element, sizeof(_T));
#else
    m_Members[m_Size] = Element;
#endif
    m_Size++;
}

template <class _T> void CVector<_T>::PushNew(void)
{
    if (m_Size + 1 > m_MemorySize)
        MoarMemoryPlox();
    
    (m_Members + m_Size) = new _T();
    
    m_Size++;
}*/
