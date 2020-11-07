#include "device.h"

Device::Device(std::wstring name, std::wstring pid, DEVINST devInst, bool ejectable = true) : ejectable(ejectable),
devInst(devInst), pid(pid),
name(name)
{
}

Device::Device(PDEV_BROADCAST_DEVICEINTERFACE_A info)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(nullptr, nullptr);
	SetupDiOpenDeviceInterfaceW(deviceList, (LPCWSTR)info->dbcc_name, NULL, NULL);

	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	*this = Device(deviceList, deviceInfo);
}

Device::Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData)
{
	this->devInst = devInfoData.DevInst;
	TCHAR buffer[1024];
	ZeroMemory(buffer, sizeof(buffer));
	SetupDiGetDeviceRegistryProperty(deviceList, &devInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)buffer, 1024, NULL);
	this->name = std::wstring(buffer);
	ZeroMemory(buffer, sizeof(buffer));
	SetupDiGetDeviceRegistryProperty(deviceList, &devInfoData, SPDRP_HARDWAREID, nullptr, (BYTE*)buffer, 1024, nullptr);
	this->HARDWARE_ID = std::wstring(buffer);
	if (!this->HARDWARE_ID.empty() && (HARDWARE_ID.find(L"PID_") != -1))
		this->pid = HARDWARE_ID.substr(HARDWARE_ID.find(L"PID_") + 4, 4);

	DWORD properties;
	SetupDiGetDeviceRegistryPropertyA(deviceList, &devInfoData, SPDRP_CAPABILITIES, NULL, (PBYTE)&properties, sizeof(DWORD), NULL);
	this->ejectable = properties & CM_DEVCAP_REMOVABLE;
}

Device::Device(const Device& other)
{
	this->HARDWARE_ID = other.HARDWARE_ID;
	this->name = other.name;
	this->pid = other.pid;
	this->ejectable = other.ejectable;
	this->devInst = other.devInst;
}

bool Device::operator ==(const Device& other) const
{
	return this->pid == other.pid;
}

bool Device::isEjectable() const { return this->ejectable; }

bool Device::eject() const
{
	if (CM_Request_Device_EjectW(this->devInst, nullptr, nullptr, NULL, NULL) == CR_SUCCESS)
		return true;
	return false;
}

void Device::print() const
{
	std::wcout << name << ", ";
	std::wcout << pid;
}

std::wstring Device::getName() const { return name; }
