#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <setupapi.h>
#include <iostream>

int main()
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
		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, &a, (BYTE*)buffer, sizeof(buffer), &b);
		//std::cout << "Len = " << int(b) << std::endl;
		std::cout << std::string(buffer, buffer + b) << std::endl;
	}
	system("pause");
}