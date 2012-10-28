alentours.pci.Device = {}

alentours.pci.buses = {}
alentours.pci.devices = {}

function alentours.pci.Initialize()
	local Devices = alentours.pci.__GetAllDevices()
	for Location, DeviceRaw in pairs(Devices) do
		local BusID = Location.Bus
		local DeviceID = Location.Device

		if alentours.pci.buses[BusID] == nil then
			 alentours.pci.buses[BusID] = {}
		end

		local Device = {}
		Device.__raw = DeviceRaw
		setmetatable(Device, alentours.pci.Device)

		alentours.pci.buses[BusID][DeviceID] = Device
	end
end

function alentours.pci.Device:GetVendor()
	return self.__raw.VID, self.__raw.VendorString
end

function alentours.pci.Device:GetProduct()
	return self.__raw.PID, self.__raw.ProductName, self.__raw.ProductDescrption
end

function alentours.pci.Device:WriteControl32(Offset, Data)

end