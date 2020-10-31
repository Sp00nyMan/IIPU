#pragma comment(lib, "setupapi.lib")
#include <Windows.h>
#include <Setupapi.h>
#include <iostream>
#include <conio.h>
#include <Cfgmgr32.h>

using namespace std;

bool exitFlag = 0;

DWORD WINAPI batteryStatusThread(PVOID param) {
	HDEVINFO devInfo;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];
	char userInput = 0;

	setlocale(LC_ALL, "Russian");

	while (1) {
		system("cls");
		if (exitFlag) {
			return 0;
		}
		cout << "=====================Lab 5====================" << endl;
		cout << "Press q to exit" << endl << endl;
		cout << "==The list of connected USB devices===========" << endl << endl;
		devInfo = SetupDiGetClassDevsA(NULL, "USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
		if (devInfo == INVALID_HANDLE_VALUE)
			return -1;

		for (int i = 0; ; i++)
		{
			devInfoData.cbSize = sizeof(devInfoData);
			if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
				break;
			memset(buffer, 0, sizeof(buffer));
			SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)buffer, 1024, NULL);
			std::wstring name(buffer);

			memset(buffer, 0, sizeof(buffer));
			SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_REMOVAL_POLICY, NULL, (BYTE*)buffer, 1024, NULL);
			DWORD* type;
			type = (DWORD*)buffer;
			if (*type == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL || *type == CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL) {
				std::wcout << name << std::endl;
			}

			//SetupDiDeleteDeviceInfo(devInfo, &devInfoData); // OTHERWISE NOT ALL USB DEVICES ARE BEING SHOWN
		}
		SetupDiDestroyDeviceInfoList(devInfo);
		Sleep(100);
	}
}

int main()
{
	HANDLE thread;

	HDEVINFO devInfo;
	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];
	char userInput = 0;

	setlocale(LC_ALL, "Russian");

	thread = CreateThread(NULL, 0, batteryStatusThread, NULL, 0, NULL);
	if (thread == NULL) {
		std::cout << "Cannot create event." << std::endl;
		return GetLastError();
	}

	while (1) {
		rewind(stdin);

		if (_getch() == 'q') {
			exitFlag = 1;
			break;
		}
	}
	WaitForSingleObject(thread, INFINITE);
	system("pause");
	return 0;
}
