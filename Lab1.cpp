#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <setupapi.h>
#include <iostream>

int main() //new comment :)
{
	HDEVINFO devInfo = SetupDiGetClassDevs(nullptr, TEXT("PCI"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if(devInfo == INVALID_HANDLE_VALUE)
		return -1;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];
	DWORD a, b;
	
	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if(SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, &a, reinterpret_cast<BYTE*>(buffer), sizeof(buffer), &b);
		std::cout << std::string(buffer, buffer + a) << std::endl;
	}
	system("pause");
}
