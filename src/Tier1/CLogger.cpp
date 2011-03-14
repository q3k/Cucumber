#include "Tier1/CLogger.h"
using namespace cb;

extern "C" {
    #include "Tier0/kstdio.h"
}

CHexNumber::CHexNumber(const u32 Data)
{
    m_Data = Data;
}

u32 CHexNumber::Get(void)
{
    return m_Data;
}

CLogger::CLogger(void)
{
    kprintf(":: CLogger Initialized.\n");
    m_Flushed = true;
}

CLogger &CLogger::operator << (const s8 *Data)
{
    Prefix();
    kprintf("%s", Data);
    return *this;
}

CLogger &CLogger::operator << (const u32 Data)
{
    Prefix();
    kprintf("%u", Data);
    return *this;
}

CLogger &CLogger::operator << (const s32 Data)
{
    Prefix();
    kprintf("%i", Data);
    return *this;
}

void CLogger::Prefix(void)
{
    if (m_Flushed)
    {
        m_Flushed = false;
        kprintf(":: ");
    }
}

void CLogger::Flush(void)
{
    kprintf("\n");
    m_Flushed = true;
}
