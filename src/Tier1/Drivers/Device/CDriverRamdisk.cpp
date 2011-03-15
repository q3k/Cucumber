#include "Tier1/Drivers/Device/CDriverRamdisk.h"
using namespace cb;

const s8 *CDriverRamdisk::GetDescription(void)
{ return "Ramdisk Driver"; }
const s8 *CDriverRamdisk::GetName(void)
{ return "org.q3k.drivers.ramdisk"; }
const s8 *CDriverRamdisk::GetAuthor(void)
{ return "Sergiusz Bazanski"; }
EDriverClass CDriverRamdisk::GetClass(void)
{ return EDC_DEVICE; }
EDriverLoadMethod CDriverRamdisk::GetLoadMethod(void)
{ return EDLM_ALWAYS; }
bool CDriverRamdisk::CanUnload(void)
{ return false; }

u8 CDriverRamdisk::Load(CKernel *Kernel)
{
    m_Kernel = Kernel;    
    return 0;
}

u8 CDriverRamdisk::Unload(void)
{
    return 0;
}

u32 CDriverRamdisk::GetSize(void)
{
    return 2;
}

const u8 *CDriverRamdisk::Read(u32 Offset, u32 Length)
{
    return (u8*)"lo";
}

void CDriverRamdisk::Write(u32 Offset, u32 Length, const u8 *Data)
{
    return;
}

IDeviceOperations CDriverRamdisk::GetSupportedOperations(void)
{
    return (IDeviceOperations)(IDO_GET_SIZE | IDO_READ);
}
