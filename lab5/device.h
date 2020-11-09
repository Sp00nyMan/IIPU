#pragma once

#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <Setupapi.h>
#include <iostream>
#include <Cfgmgr32.h>
#include <vector>
#include <string>
#include <dbt.h>
#include <initguid.h>
#include <Usbiodef.h>

class Device
{
	std::wstring HARDWARE_ID;
	std::wstring name;
	std::wstring pid;
	bool ejectable;
	DEVINST devInst;
	HDEVNOTIFY notificationHandle;
	HANDLE deviceHandle;
	std::wstring devicePath;
public:
	static std::vector<Device> devices;
	static Device remove(const Device& device)
	{
		for (size_t i = 0; i < devices.size(); i++)
			if (device == devices[i])
			{
				devices.erase(devices.begin() + i);
				return device;
			}
	}

	Device(PDEV_BROADCAST_DEVICEINTERFACE_A info);

	Device(PDEV_BROADCAST_DEVICEINTERFACE_A info, PDEV_BROADCAST_DEVICEINTERFACE devBroadcastDeviceInterface, HWND hwnd);

	Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData);

	Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData, HWND hwnd);

	Device(const Device& other);

	bool operator ==(const Device& other) const;

	bool isEjectable() const;

	bool eject(HWND hwnd, int i);

	void print() const;

	void close() {
		UnregisterDeviceNotification(notificationHandle);
		
		notificationHandle = NULL;
		deviceHandle = NULL;
	}

	std::wstring getName() const;
};
