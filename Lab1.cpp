#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <setupapi.h>
#include <iostream>
#include <string.h>

int main()
{
	HDEVINFO devInfo = SetupDiGetClassDevs(nullptr, TEXT("PCI"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (devInfo == INVALID_HANDLE_VALUE)
		return -1;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];
	DWORD a, b;
	char *vendorID, *deviceID;

	for (int i = 0; ; i++)
	{
		memset(&devInfoData, 0, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;
		memset(buffer, 0, sizeof(buffer));
		//SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, &a, reinterpret_cast<BYTE*>(buffer), sizeof(buffer), &b);
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, &a, (BYTE*)buffer, sizeof(buffer), &b);
		SetupDiDeleteDeviceInfo(devInfo, &devInfoData);
		vendorID = strstr((char*)buffer, "Vid_") + 4;
		//deviceID = strstr((char*)buffer, "Pid_") + 4;
		//std::cout << devInfoData.ClassGuid << std::endl;
		std::cout << (int)b << std::endl;
		int j = 0;
		while (j!=1024) {
			std::cout << (char)buffer[j];
			j++;
		}
		
		//std::cout << vendorID << std::endl;
		//std::cout << std::string(buffer, buffer + a) << std::endl;
	}
	system("pause");
}
