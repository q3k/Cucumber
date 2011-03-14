#ifndef __CLOGGER_H__
#define __CLOGGER_H__

#include "types.h"

namespace cb {
    class CHexNumber {
        public:
            CHexNumber(const u32 Data);
            u32 Get(void);
        private:
            u32 m_Data;
    };
    class CLogger {
        public:
            CLogger(void);
            CLogger &operator << (const s8 *Data);
            CLogger &operator << (const u32 Data);
            CLogger &operator << (const s32 Data);
            CLogger &operator << (CHexNumber &Data);
            void Flush(void);
        private:
            bool m_Flushed;
            void Prefix(void);
    };
};

#endif
