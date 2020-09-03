#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <setupapi.h>
#include <iostream>
//#include <regex>
#include <iomanip>
const char name_len = 100;
int main()
{
	setlocale(LC_ALL, "Russian");
	std::cout << std::left << std::setw(name_len) << "Название" << std::left << std::setw(12) << "Vendor ID" << std::left << std::setw(12) << "Device ID" << std::endl;
	
	HDEVINFO devInfo = SetupDiGetClassDevs(nullptr, TEXT("PCI"), nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (devInfo == INVALID_HANDLE_VALUE)
		return -1;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];

	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;
		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, nullptr, (BYTE*)buffer, 1024, nullptr);
		std::wstring name(buffer);

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_HARDWAREID, nullptr, (BYTE*)buffer, 1024, nullptr);
		std::wstring ids(buffer);

		std::wstring ven(ids.substr(ids.find(L"VEN_") + 4, 4));
		std::wstring dev(ids.substr(ids.find(L"DEV_") + 4, 4));

		if (name.substr(name.size() - 4, 4) == dev)
			name = name.substr(0, name.size() - 7);
		std::wcout << std::left << std::setw(name_len) << name << "\t";
		std::wcout << std::left << std::setw(12) << ven <<  std::left << std::setw(12) << dev << std::endl;
		SetupDiDeleteDeviceInfo(devInfo, &devInfoData);
	}
	SetupDiDestroyDeviceInfoList(devInfo);
	system("pause");
}