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
	std::wstring devicePath;
	std::wstring pid;
	bool ejectable;
	DEVINST devInst;

	HDEVNOTIFY notificationHandle;
	void register_handle(HWND hWnd);
public:
	static std::vector<Device> devices;

	static void remove(const Device& device);

	Device(PDEV_BROADCAST_DEVICEINTERFACE info, HWND hWnd = nullptr);

	Device(HDEVINFO deviceList, SP_DEVINFO_DATA deviceInfo, HWND hWnd);

	Device(const Device& other);

	bool operator ==(const Device& other) const { return this->pid == other.pid; }

	bool isEjectable() const { return this->ejectable; }

	void eject() const;

	void print() const;

	std::wstring getName() const { return name; };

	void unregister() const;
};
