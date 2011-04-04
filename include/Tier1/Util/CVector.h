#ifndef __CVECTOR_H__
#define __CVECTOR_H__

class cb {
    // An STL-like vector implementation for internal use in the kernel
    template <typename _T> class CVector {
        public:
            CVector(u32 StartElements = 32);
            void Push(_T Element);
            void Pop(_T Element);
            u32 IterationStart(void);
            u32 IterationEnd(void);
        private:
            _T *m_Members;
    };
};

#endif
