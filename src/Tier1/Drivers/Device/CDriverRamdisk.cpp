#include "Tier1/Drivers/Device/CDriverRamdisk.h"
using namespace cb;

u8 CDriverRamdisk::Load(CKernel *Kernel)
{
    m_Name = "org.q3k.drivers.ramdisk";
    m_Description = "Ramdisk Driver";
    m_Author = "Sergiusz Bazanski";
    m_Class = EDC_DEVICE;
    m_LoadMethod = EDLM_ALWAYS;
    m_Unloadable = false;
    
    m_DeviceOperations = (IDeviceOperations)(IDO_GET_SIZE | IDO_READ);
    
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
