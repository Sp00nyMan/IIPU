#include "device.h"

void Device::unregister() const
{
	UnregisterDeviceNotification(notificationHandle);
}

void Device::remove(const Device& device)
{
	for (size_t i = 0; i < devices.size(); i++)
		if (device == devices[i])
		{
			devices[i].unregister();
			devices.erase(devices.begin() + i);
			break;
		}
}

void Device::register_handle(HWND hWnd)
{
	DEV_BROADCAST_HANDLE filter;

	HANDLE deviceHandle = CreateFile(devicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	filter.dbch_size = sizeof(filter);
	filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
	filter.dbch_handle = deviceHandle;
	this->notificationHandle = RegisterDeviceNotification(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	CloseHandle(deviceHandle);
}

Device::Device(PDEV_BROADCAST_DEVICEINTERFACE_A info, HWND hWnd)
{
	this->devicePath = (LPCWSTR)info->dbcc_name;
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(nullptr, nullptr);
	SetupDiOpenDeviceInterfaceW(deviceList, devicePath.c_str(), NULL, NULL);

	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	*this = Device(deviceList, deviceInfo, hWnd);
}

Device::Device(HDEVINFO deviceList, SP_DEVINFO_DATA deviceInfo, HWND hWnd)
{
	this->devInst = deviceInfo.DevInst;
	TCHAR buffer[1024];
	ZeroMemory(buffer, sizeof(buffer));
	SetupDiGetDeviceRegistryProperty(deviceList, &deviceInfo, SPDRP_DEVICEDESC, NULL, (BYTE*)buffer, 1024, NULL);
	this->name = std::wstring(buffer);
	ZeroMemory(buffer, sizeof(buffer));
	SetupDiGetDeviceRegistryProperty(deviceList, &deviceInfo, SPDRP_HARDWAREID, nullptr, (BYTE*)buffer, 1024, nullptr);
	this->HARDWARE_ID = std::wstring(buffer);
	if (!this->HARDWARE_ID.empty() && (HARDWARE_ID.find(L"PID_") != -1))
		this->pid = HARDWARE_ID.substr(HARDWARE_ID.find(L"PID_") + 4, 4);

	DWORD properties;
	SetupDiGetDeviceRegistryPropertyA(deviceList, &deviceInfo, SPDRP_CAPABILITIES, NULL, (PBYTE)&properties, sizeof(DWORD), NULL);
	this->ejectable = properties & CM_DEVCAP_REMOVABLE;

	if(hWnd != nullptr)
		this->register_handle(hWnd);
}

Device::Device(const Device& other)
{
	this->HARDWARE_ID = other.HARDWARE_ID;
	this->name = other.name;
	this->pid = other.pid;
	this->ejectable = other.ejectable;
	this->devInst = other.devInst;
	this->devicePath = other.devicePath;
	this->notificationHandle = other.notificationHandle;
}

void Device::eject() const
{
	CM_Request_Device_EjectW(this->devInst, nullptr, nullptr, NULL, NULL);
}

void Device::print() const
{
	std::wcout << name << ", ";
	std::wcout << pid;
}