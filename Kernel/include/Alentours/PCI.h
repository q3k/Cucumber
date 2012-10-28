#ifndef __PCI_H__
#define __PCI_H__

// ports for PCI config on IA-32(e)
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#include "types.h"
#include "Tier1/Util/CVector.h"

namespace Alentours
{
	// structure representing the configuration register internals
	typedef struct
	{
		u8 Zero     : 2;
		u8 Register : 6;
		u8 Function : 3;
		u8 Device   : 5;
		u8 Bus      : 8;
		u8 Reserved : 7;
		u8 Enable   : 1;
	} __attribute__((packed)) TPCIConfigAddressFields;
	typedef union
	{
		TPCIConfigAddressFields Fields;
		u32 Value;
		
	} TPCIConfigAddress;
	typedef struct
	{
		u16 VendorID;
		u16 ProductID;
		u16 Command;
		u16 Code;
		u8 Revision;
		u8 ProgInterface;
		u8 Subclass;
		u8 Class;
		u8 CacheLineSize;
		u8 LatencyTimer;
		u8 HeaderType : 7;
		u8 MultiFunction : 1;
		u8 BIST;
	} __attribute__((packed)) TPCIHeaderCommon;
	typedef union
	{
		u32 Value;
		struct
		{
			u8 Zero : 1;
			u8 Type : 2;
			u8 Prefetchable : 1;
			u32 Address : 28;
		} __attribute__((packed)) MemoryBAR;
		struct
		{
			u8 One : 1;
			u8 Reserved : 1;
			u32 Address : 30;
		} __attribute__((packed)) IOBAR;
	} TPCIBAR;

	typedef struct
	{
		TPCIBAR BAR[6];
		u32 CardbusCISPointer;
		u16 SubsystemVID;
		u16 SubsystemID;
		u32 ExpansionROMAddress;
		u8 Capabilities;
		u32 Reserved0 : 24;
		u32 Reserved1;
		u8 InterruptLine;
		u8 InterruptPIN;
		u8 MinGrant;
		u8 MaxLatency;
	} __attribute__((packed)) TPCIHeaderDevice;

	class CPCIDevice
	{
	private:
		u16 m_Bus, m_Device;
		TPCIHeaderCommon m_Header;
		TPCIHeaderDevice *m_DeviceHeader;
	public:
		CPCIDevice(u16 Bus, u16 Device);
		u32 ConfigRead(u16 Function, u16 Offset);
		void ConfigWrite(u16 Function, u16 Offset, u32 Data);

	};

	class CPCIManager
	{
	private:
		static cb::CVector<CPCIDevice> m_Devices;
		static u8 DevicePresent(u16 Bus, u16 Device);
		static u32 ConfigRead(u16 Bus, u16 Device, u16 Function, u16 Offset);
	public:
		static void Initialize(void);

		static u32 GetDeviceCount(void);
		static CPCIDevice *GetDeviceByIndex(void);
		static CPCIDevice *GetDeviceByAddress(u16 Bus, u16 Device);
		static void GetDeviceByIDPair(cb::CVector<CPCIDevice> &Devices, u16 Vendor, u16 Device);
		static void GetAllDevices(cb::CVector<CPCIDevice> &Devices);

		static u8 DBGetVendor(u16 VID, const s8 **VendorNameOut);
		static u8 DBGetProduct(u16 VID, u16 PID, const s8 **ProductNameOut, const s8 **ProductDescriptionOut);
	};
};

#endif