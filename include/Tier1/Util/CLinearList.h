#ifndef __CLINEAR_LIST_H__
#define __CLINEAR_LIST_H__

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/panic.h"
};

namespace cb {
    template <class _T> class CLinearList {
        public:
            CLinearList(void);
            
            // Stack-like access
            void Push(const _T &Element);
            _T &Pop(void);
            
            // Indexing operator
            _T &operator[](u32 Index);
            const _T &operator[](u32 Index) const;
            
            void Insert(const _T &Element, u32 Index = 0);
            void Delete(u32 Index);
            
            u32 GetSize(void) const;
            u32 GetSize(void);
        private:
            struct SLinearListNode {
                _T Data;
                struct SLinearListNode *Next;
            };
            typedef struct SLinearListNode TLinearListNode;
            TLinearListNode *m_Data;
            bool m_SizeCacheValid;
            u32 m_SizeCache;
    };
    
    template <class _T> CLinearList<_T>::CLinearList(void)
    {
        m_Data = 0;
        m_SizeCacheValid = true;
        m_SizeCache = 0;
    }
    
    template <class _T> void CLinearList<_T>::Push(const _T &Element)
    {
        if (m_Data == 0)
        {
            m_Data = (TLinearListNode *)kmalloc(sizeof(TLinearListNode));
            m_Data->Data = Element;
            m_Data->Next = 0;
            m_SizeCacheValid = false;
            return;
        }
        
        TLinearListNode *LastNode = m_Data;
        while (LastNode->Next != 0)
            LastNode = LastNode->Next;
        
        TLinearListNode *NewNode = 
                           (TLinearListNode *)kmalloc(sizeof(TLinearListNode));
        NewNode->Data = Element;
        NewNode->Next = 0;
        LastNode->Next = NewNode;
        
        m_SizeCacheValid = false;
        
        return;
    }
    
    template <class _T> _T &CLinearList<_T>::Pop(void)
    {
        ASSERT(m_Data != 0);
        if (m_Data->Next == 0)
        {
            _T &Element = m_Data->Data;
            kfree((void*)m_Data);
            m_Data = 0;
            m_SizeCacheValid = false;
            return Element;
        }
        
        TLinearListNode *LastNode = m_Data;
        while (LastNode->Next->Next != 0)
            LastNode = LastNode ->Next;
        
        _T &Element = LastNode->Next->Data;
        kfree((void*)LastNode->Next);
        LastNode->Next = 0;
        
        m_SizeCacheValid = false;
        
        return Element;
    }
    
    template <class _T> _T &CLinearList<_T>::operator[](u32 Index)
    {
        ASSERT(m_Data != 0);
        TLinearListNode *Node = m_Data;
        for (u32 i = 0; i < Index; i++)
        {
            Node = Node->Next;
            ASSERT(Node != 0);
        }
        
        return Node->Data;
    }
    
    template <class _T> const _T &CLinearList<_T>::operator[](u32 Index) const
    {
        ASSERT(m_Data != 0);
        TLinearListNode *Node = m_Data;
        for (u32 i = 0; i < Index; i++)
        {
            Node = Node->Next;
            ASSERT(Node);
        }
        
        return Node->Data;
    }
    
    template <class _T> void CLinearList<_T>::Insert(
                                                   const _T &Element, u32 Index)
    {
        if (m_Data == 0 && Index == 0)
        {
            Push(Element);
            m_SizeCacheValid = false;
            return;
        }
        
        ASSERT(m_Data);
        
        TLinearListNode *NodeBefore;
        for (u32 i = 0; i < Index - 1; i++)
        {
            NodeBefore = NodeBefore->Next;
            ASSERT(NodeBefore);
        }
        // Last node?
        if (NodeBefore->Next == 0)
        {
            Push(Element);
            return;
        }
        
        TLinearListNode *NodeAfter = NodeBefore->Next;
        
        TLinearListNode *NewNode = (TLinearListNode*)
                                               kmalloc(sizeof(TLinearListNode));
        NewNode->Data = Element;
        NewNode->Next = NodeAfter;
        NodeBefore->Next = NewNode;
        
        m_SizeCacheValid = false;
    }
    
    template <class _T> void CLinearList<_T>::Delete(u32 Index)
    {
        if (m_Data == 0 && Index == 0)
        {
            Pop();
            m_SizeCacheValid = false;
            return;
        }
        ASSERT(m_Data);
        
        TLinearListNode *NodeBefore = m_Data;
        for (u32 i = 0; i < Index - 1; i++)
        {
            NodeBefore = NodeBefore->Next;
            ASSERT(NodeBefore);
        }
        
        TLinearListNode *NodeToBeDeleted = NodeBefore->Next;
        ASSERT(NodeToBeDeleted);
        TLinearListNode *NodeAfter = NodeToBeDeleted->Next;
        kfree((void*)NodeToBeDeleted);
        NodeBefore->Next = NodeAfter;
        
        m_SizeCacheValid = false;
    }
    
    template <class _T> u32 CLinearList<_T>::GetSize(void) const
    {
        if (m_SizeCacheValid)
            return m_SizeCache;
        
        TLinearListNode *Node = m_Data;
        u32 Size = 1;
        while (Node->Next != 0)
        {
            Node = Node->Next;
            Size++;
        }
        return m_SizeCache;
    }
    
    template <class _T> u32 CLinearList<_T>::GetSize(void)
    {
        if (m_SizeCacheValid)
            return m_SizeCache;
        
        if (m_Data == 0)
        {
            m_SizeCache = 0;
            m_SizeCacheValid = true;
            return 0;
        }
        
        u32 Size = 1;
        TLinearListNode *Node = m_Data;
        while (Node->Next != 0)
        {
            Node = Node->Next;
            Size++;
        }
        
        m_SizeCache = Size;
        m_SizeCacheValid = true;
        return m_SizeCache;
    }
};

#endif
