#ifndef __CVECTOR_H__
#define __CVECTOR_H__

class cb {
    // An STL-like vector implementation for internal use in the kernel
    template <typename _T> class CVector {
        public:
            CVector(u32 StartElements = 32);
            ~CVector(void);
            
            // Element accessors
            _T &operator[](u32 Index);
            const _T &operator[](u32 Index) const;
            _T &Head(void);
            const _T &Head(void) const;
            _T &Tail(void);
            const _T &Tail(void) const;
            
            // Base address
            _T *BaseAddress(void);
            const _T *BaseAddress(void) const;
            
            // Size
            u32 Size(void);
            
            // Search
            u32 Find(const _T &Element) const;
            bool HasElement(const _T &Element) const;
            
            // Stack-like properties
            void Push(const _T &Element);
            void PushNew(void);
            _T &Pop(void);
            
            // Used for iteration... I could use another class unstead of u32's,
            // but who cares :V
            u32 IterationStart(void);
            u32 IterationEnd(void);
        protected:
            _T *m_Members;
            u32 m_Size;
            u32 m_MemorySize;
    };
};

#endif
