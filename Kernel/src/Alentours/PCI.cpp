#include "Alentours/PCI.h"
#include "Alentours/PCIDB.h"
extern "C" {
	#include "Tier0/kstdio.h"
}

using namespace Alentours;

u32 CPCIManager::ConfigRead(u16 Bus, u16 Device, u16 Function, u16 Offset)
{
	TPCIConfigAddress Address;
	Address.Fields.Zero = 0;
	Address.Fields.Register = Offset >> 2;
	Address.Fields.Function = Function;
	Address.Fields.Device = Device;
	Address.Fields.Bus = Bus;
	Address.Fields.Reserved = 0;
	Address.Fields.Enable = 1;

	koutl(PCI_CONFIG_ADDRESS, Address.Value);
	return kinl(PCI_CONFIG_DATA);
}

u8 CPCIManager::DevicePresent(u16 Bus, u16 Device)
{
	// try to get the VID & PID pair -  if all 1's, then no device is present
	u32 Value = CPCIManager::ConfigRead(Bus, Device, 0, 0);
	return (Value != 0xFFFFFFFF);
}

void CPCIManager::Initialize(void)
{
	for (u16 BusID = 0; BusID < 256; BusID++)
	{
		for (u8 DeviceID = 0; DeviceID < 32; DeviceID++)
	    {
	        u8 Present = DevicePresent(BusID, DeviceID);
	        if (Present)
	        {
	        	CPCIDevice Device(BusID, DeviceID);
	        	m_Devices.Push(Device);
	        }
	    }
	}
}

u8 CPCIManager::DBGetVendor(u16 VID, const s8 **VendorNameOut)
{
	u32 TableLength = PCI_VENTABLE_LEN;
	for (u32 i = 0; i < TableLength; i++)
	{
		PCI_VENTABLE *Vendor = &PciVenTable[i];
		if (Vendor->VenId == VID)
		{
			// try to use the short name, if not null
			bool Empty = true;
			const s8 *Character = Vendor->VenShort;
			while (*Character != 0)
			{
				if (*Character != 0)
				{
					Empty = false;
					break;
				}
				Character++;
			}
			if (!Empty)
				*VendorNameOut = Vendor->VenShort;
			else
				*VendorNameOut = Vendor->VenFull;
			return 1;
		}
	}
	return 0;
}

u8 CPCIManager::DBGetProduct(u16 VID, u16 PID, const s8 **ProductNameOut, const s8 **ProductDescriptionOut)
{
	u32 TableLength = PCI_DEVTABLE_LEN;
	for (u32 i = 0; i < TableLength; i++)
	{
		PCI_DEVTABLE *Product = &PciDevTable[i];
		if (Product->VenId == VID && Product->DevId == PID)
		{
			*ProductNameOut = Product->Chip;
			*ProductDescriptionOut = Product->ChipDesc;
			return 1;
		}
	}
	return 0;
}

u32 CPCIDevice::ConfigRead(u16 Function, u16 Offset)
{
	TPCIConfigAddress Address;
	Address.Fields.Zero = 0;
	Address.Fields.Register = Offset >> 2;
	Address.Fields.Function = Function;
	Address.Fields.Device = m_Device;
	Address.Fields.Bus = m_Bus;
	Address.Fields.Reserved = 0;
	Address.Fields.Enable = 1;

	koutl(PCI_CONFIG_ADDRESS, Address.Value);
	return kinl(PCI_CONFIG_DATA);
}

CPCIDevice::CPCIDevice(u16 Bus, u16 Device)
{
	m_Bus = Bus;
	m_Device = Device;

	kprintf("PCI device [%i:%i]\n", Bus, Device);

	for (u8 i = 0; i < sizeof(m_Header); i += 4)
	{
		u32 Value = ConfigRead(0, i);
		((u32 *)&m_Header)[i/4] = Value;
	}
	kprintf("    VID: %h, PID: %h\n", m_Header.VendorID, m_Header.ProductID);
	kprintf("    Type: ");
	switch (m_Header.HeaderType)
	{
		case 0x00:
			kprintf("Generic Device\n");
			break;
		case 0x01:
			kprintf("PCI-PCI Bridge\n");
			break;
		case 0x02:
			kprintf("CardBus Bridge\n");
			break;
		default:
			kprintf("Unknown.\n");
			break;
	}
	kprintf("    Info: ");
	const s8 *VendorName, *ProductName, *ProductDescription;
	if (CPCIManager::DBGetVendor(m_Header.VendorID, &VendorName))
		kprintf("%s", VendorName);
	if (CPCIManager::DBGetProduct(m_Header.VendorID, m_Header.ProductID, &ProductName, &ProductDescription))
		kprintf(" %s %s", ProductName, ProductDescription);
	kprintf("\n");
}

cb::CVector<CPCIDevice> CPCIManager::m_Devices;