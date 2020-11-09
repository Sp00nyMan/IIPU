#include "device.h"

Device::Device(PDEV_BROADCAST_DEVICEINTERFACE_A info)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(nullptr, nullptr);
	SetupDiOpenDeviceInterfaceW(deviceList, (LPCWSTR)info->dbcc_name, NULL, NULL);

	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	*this = Device(deviceList, deviceInfo);

	this->deviceHandle = NULL;
	this->notificationHandle = NULL;
}

Device::Device(PDEV_BROADCAST_DEVICEINTERFACE_A info, PDEV_BROADCAST_DEVICEINTERFACE devBroadcastDeviceInterface, HWND hwnd)
{
	HDEVINFO deviceList = SetupDiCreateDeviceInfoList(nullptr, nullptr);
	SetupDiOpenDeviceInterfaceW(deviceList, (LPCWSTR)info->dbcc_name, NULL, NULL);

	SP_DEVINFO_DATA deviceInfo;
	ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
	deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);

	*this = Device(deviceList, deviceInfo);

	DEV_BROADCAST_HANDLE filter = { 0 };

	this->deviceHandle = CreateFile(devBroadcastDeviceInterface->dbcc_name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	filter.dbch_size = sizeof(filter);
	filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
	filter.dbch_handle = deviceHandle;
	this->notificationHandle = RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	CloseHandle(deviceHandle);
	devicePath = std::wstring(devBroadcastDeviceInterface->dbcc_name);
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

	this->deviceHandle = NULL;
	this->notificationHandle = NULL;
}

Device::Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData, HWND hwnd)
{
	*this = Device(deviceList, devInfoData);

	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	SP_DEVICE_INTERFACE_DETAIL_DATA* devInterfaceDetailData;
	devInterfaceData.cbSize = sizeof(devInterfaceData);
	SetupDiEnumDeviceInterfaces(deviceList, &devInfoData, &GUID_DEVINTERFACE_USB_DEVICE, 0, &devInterfaceData);

	DWORD requiredLength;
	SetupDiGetDeviceInterfaceDetail(deviceList, &devInterfaceData, NULL, 0, &requiredLength, NULL);

	devInterfaceDetailData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredLength);
	devInterfaceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	SetupDiGetDeviceInterfaceDetail(deviceList, &devInterfaceData, devInterfaceDetailData, requiredLength, NULL, &devInfoData);

	DEV_BROADCAST_HANDLE filter = { 0 };

	this->deviceHandle = CreateFile(devInterfaceDetailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	filter.dbch_size = sizeof(filter);
	filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
	filter.dbch_handle = deviceHandle;
	this->notificationHandle = RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	CloseHandle(deviceHandle);
	devicePath = std::wstring(devInterfaceDetailData->DevicePath);
}

Device::Device(const Device& other)
{
	this->HARDWARE_ID = other.HARDWARE_ID;
	this->name = other.name;
	this->pid = other.pid;
	this->ejectable = other.ejectable;
	this->devInst = other.devInst;
	this->deviceHandle = other.deviceHandle;
	this->notificationHandle = other.notificationHandle;
	this->devicePath = other.devicePath;
}

bool Device::operator ==(const Device& other) const
{
	return this->pid == other.pid;
}

bool Device::isEjectable() const { return this->ejectable; }

bool Device::eject(HWND hwnd, int i)
{
	if (CM_Request_Device_EjectW(this->devInst, nullptr, nullptr, NULL, NULL) == CR_SUCCESS)
		return true;

	DEV_BROADCAST_HANDLE filter = { 0 };

	this->deviceHandle = CreateFile(this->devicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	filter.dbch_size = sizeof(filter);
	filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
	filter.dbch_handle = deviceHandle;
	
	this->notificationHandle = RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	CloseHandle(deviceHandle);
	return false;
}

void Device::print() const
{
	std::wcout << name << ", ";
	std::wcout << pid;
}

std::wstring Device::getName() const { return name; }
