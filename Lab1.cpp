#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <setupapi.h>
#include <iostream>
#include <regex>
#include <iomanip>

int main()
{
	setlocale(LC_ALL, "Russian");
	std::cout << std::left << std::setw(120) << "Название" << std::left << std::setw(12) << "Vendor ID" << std::left << std::setw(12) << "Device ID" << std::endl;
	HDEVINFO devInfo = SetupDiGetClassDevs(nullptr, TEXT("PCI"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (devInfo == INVALID_HANDLE_VALUE)
		return -1;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];
	DWORD a, b;
	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;
		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, &a, (BYTE*)buffer, 1024, &b);
		std::wstring name(buffer);
		//int p = name.find(L" - ");
		//if(p != std::string::npos)
		//	name = name.substr(0, p);

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_HARDWAREID, &a, (BYTE*)buffer, 1024, &b);
		std::wstring ids(buffer);

		std::wstring ven(ids.substr(ids.find(L"VEN_") + 4, 4));
		std::wstring dev(ids.substr(ids.find(L"DEV_") + 4, 4));

		//std::wcout << name.substr(name.size() - 4, 4) << '\t';
		if (name.substr(name.size() - 4, 4) == dev)
			name = name.substr(0, name.size() - 7);
		std::wcout << std::left << std::setw(120) << name << "\t";
		std::wcout << std::left << std::setw(12) << ven <<  std::left << std::setw(12) << dev << std::endl;
	}
	system("pause");
}
