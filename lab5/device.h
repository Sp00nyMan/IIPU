#pragma once

#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <Setupapi.h>
#include <iostream>
#include <Cfgmgr32.h>
#include <vector>
#include <string>
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
			if (device == devices[i])
			{
				devices.erase(devices.begin() + i);
				return device;
			}
	}

	Device(std::wstring name, std::wstring pid, DEVINST devInst, bool ejectable);

	Device(PDEV_BROADCAST_DEVICEINTERFACE_A info);

	Device(HDEVINFO deviceList, SP_DEVINFO_DATA devInfoData);

	Device(const Device& other);

	bool operator ==(const Device& other) const;

	bool isEjectable() const;

	bool eject() const;

	void print() const;

	std::wstring getName() const;
};
