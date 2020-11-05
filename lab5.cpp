#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <Setupapi.h>
#include <iostream>
#include <conio.h>
#include <Cfgmgr32.h>
#include <vector>
#include <iomanip>
#include <string>
#include <initguid.h>
#include <Usbiodef.h>
#include <dbt.h>

class Device
{
	std::wstring HARDWARE_ID;
	std::wstring name;
	std::wstring pid;
	bool ejectable;
	DEVINST devInst;
public:
	static std::vector<Device> devices;
	static Device remove(const Device& device)
	{
		for (size_t i = 0; i < devices.size(); i++)
			if(device == devices[i])
			{
				devices.erase(devices.begin() + i);
				return device;
			}
	}

	Device(std::wstring name, std::wstring pid, DEVINST devInst, bool ejectable = true): ejectable(ejectable),
	                                                                                   devInst(devInst), pid(pid),
	                                                                                   name(name)
	{
	}
	Device(PDEV_BROADCAST_DEVICEINTERFACE_A info)
	{
		HDEVINFO deviceList = SetupDiCreateDeviceInfoList(nullptr, nullptr);
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		SetupDiOpenDeviceInterfaceW(deviceList, (LPCWSTR)info->dbcc_name, NULL, &deviceInterfaceData);
		
		SP_DEVINFO_DATA deviceInfo;
		ZeroMemory(&deviceInfo, sizeof(SP_DEVINFO_DATA));
		deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		SetupDiEnumDeviceInfo(deviceList, 0, &deviceInfo);
		
		*this = Device(deviceList, deviceInfo);
	}
	Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData)
	{
		this->devInst = devInfoData.DevInst;
		TCHAR buffer[1024];
		ZeroMemory(buffer, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(deviceList, &devInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)buffer, 1024, NULL);
		this->name = std::wstring(buffer);
		ZeroMemory(buffer, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(deviceList, &devInfoData, SPDRP_HARDWAREID, nullptr, (BYTE*)buffer, 1024, nullptr);
		this->HARDWARE_ID = std::wstring(buffer);
		if(!this->HARDWARE_ID.empty())
			this->pid = HARDWARE_ID.substr(HARDWARE_ID.find(L"PID_") + 4, 4);

		DWORD properties;
		SetupDiGetDeviceRegistryPropertyA(deviceList, &devInfoData, SPDRP_CAPABILITIES, NULL, (PBYTE)&properties, sizeof(DWORD), NULL);
		this->ejectable = properties & CM_DEVCAP_REMOVABLE;
	}
	Device(const Device& other)
	{
		this->HARDWARE_ID = other.HARDWARE_ID;
		this->name = other.name;
		this->pid = other.pid;
		this->ejectable = other.ejectable;
		this->devInst = other.devInst;
	}

	bool operator ==(const Device& other) const
	{
		return this->pid == other.pid;
	}

	bool isEjectable() const { return this->ejectable; }

	void eject() const
	{
		CM_Request_Device_EjectW(this->devInst, nullptr, nullptr, NULL, NULL);
	}
	void print() const
	{
		std::wcout << name << ", ";
		std::wcout << pid;
	}

	std::wstring getName() const { return name; }
};

std::vector<Device> Device::devices;
bool exitFlag = false;

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DEVICECHANGE) //https://docs.microsoft.com/en-us/windows/win32/devio/wm-devicechange
	{
		switch (wParam)
		{
			case DBT_DEVICEARRIVAL:
				{
					Device device((PDEV_BROADCAST_DEVICEINTERFACE_A)lParam);
					if (device.getName().empty())
						break;
					Device::devices.push_back(device);
					Device::devices[Device::devices.size() - 1].print();
					std::cout << " connected" << std::endl;
				}
				break;
			case DBT_DEVICEREMOVECOMPLETE:
				{
					Device device((PDEV_BROADCAST_DEVICEINTERFACE_A)lParam);
					if (device.getName().empty())
						break;
					Device::remove(device);
					device.print();
					std::cout << " disconnected" << std::endl;;
					break;
				}
			case DBT_DEVICEQUERYREMOVE:
				std::cout << "TRYERD TO REMOVE";
				break;
			case DBT_DEVICEQUERYREMOVEFAILED:
				std::cout << "FAILED TO REMOVE";
				break;
			default:
				break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD WINAPI batteryStatusThread(void*)
{
	WNDCLASSEXW wx;
	ZeroMemory(&wx, sizeof(wx));
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = (WNDPROC)WndProc;
	wx.lpszClassName = L"NONE";

	HWND hWnd = NULL;
	GUID guid = GUID_DEVINTERFACE_USB_DEVICE; //https://docs.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
	if (RegisterClassExW(&wx)) //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		hWnd = CreateWindowA("NONE", "DevNotifWnd", WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, 0, NULL, GetModuleHandle(nullptr), (void*)&guid);

	DEV_BROADCAST_DEVICEINTERFACE_A filter; //https://docs.microsoft.com/en-us/windows/win32/api/dbt/ns-dbt-dev_broadcast_deviceinterface_a
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_classguid = guid;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	RegisterDeviceNotificationA(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerdevicenotificationw


	SP_DEVINFO_DATA devInfoData;
	const HDEVINFO deviceInterfaceSet = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInterfaceSet == INVALID_HANDLE_VALUE)
		return -1;

	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(deviceInterfaceSet, i, &devInfoData) == FALSE)
			break;
		Device device = Device(deviceInterfaceSet, devInfoData);

		if (device.isEjectable())
			Device::devices.push_back(device);
	}
	SetupDiDestroyDeviceInfoList(deviceInterfaceSet);

	MSG msg; //https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
	while (!exitFlag && GetMessageW(&msg, hWnd, 0, 0)) //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
	{
		TranslateMessage(&msg); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
		DispatchMessage(&msg); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	HANDLE thread = CreateThread(nullptr, 0, batteryStatusThread, nullptr, 0, nullptr);
	if (thread == nullptr)
	{
		std::cout << "Cannot create event." << std::endl;
		return GetLastError();
	}

	while (true)
	{
		rewind(stdin);
		char ch = _getch();
		if (ch >= '1' && ch <= '9')
		{
			Device device = Device::devices[ch - '0' - 1];
			if (device.isEjectable())
				device.eject();
			else
				std::cout << "device isn't removable" << std::endl;
			Sleep(100);
		}
		else if (ch == 'q')
			return 0;
	}
}
