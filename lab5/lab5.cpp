#include "device.h"
#include <conio.h>
#include <initguid.h>
#include <Usbiodef.h>

void printMenu() {
	std::cout << "=======================Lab 5=======================" << std::endl << std::endl;
	std::cout << ">Press q to exit" << std::endl;
	std::cout << ">Enter the number of the device to remove it" << std::endl << std::endl;
	std::cout << "==The list of connected USB devices (№/name/PID):==" << std::endl << std::endl;
}
void printDevices()
{
	for (int i = 0; i < Device::devices.size(); i++)
	{
		std::cout << i + 1 << " ";
		Device::devices[i].print();
		std::cout << std::endl;
	}
}
std::vector<Device> Device::devices;
bool exitFlag = false;

LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DEVICECHANGE)
	{
		switch (wParam)
		{
			case DBT_DEVICEARRIVAL:
			{
				Device device((PDEV_BROADCAST_DEVICEINTERFACE_A)lParam);
				if (device.getName().empty())
					break;
				Device::devices.push_back(device);

				system("cls");
				Device::devices[Device::devices.size() - 1].print();
				std::cout << " connected" << std::endl;

				printMenu();
				for (int i = 0; i < Device::devices.size(); i++)
				{
					std::cout << i + 1 << " ";
					Device::devices[i].print();
					std::cout << std::endl;
				}
				break;
			}
			case DBT_DEVICEREMOVECOMPLETE:
			{
				Device device((PDEV_BROADCAST_DEVICEINTERFACE_A)lParam);
				if (device.getName().empty())
					break;
				Device::remove(device);

				system("cls");
				device.print();
				std::cout << " disconnected" << std::endl;

				printMenu();
				for (int i = 0; i < Device::devices.size(); i++)
				{
					std::cout << i + 1 << " ";
					Device::devices[i].print();
					std::cout << std::endl;
				}
				break;
			}
			case DBT_DEVICEQUERYREMOVE:
				std::cout << "Tryed to remove" << std::endl;
				break;
			case DBT_DEVICEQUERYREMOVEFAILED:
				std::cout << "Failed to remove" << std::endl;
				break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD WINAPI initialisationThread(void*)
{
	WNDCLASSEXW wx;
	ZeroMemory(&wx, sizeof(wx));
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = (WNDPROC)WndProc;
	wx.lpszClassName = L"NONE";

	HWND hWnd = NULL;
	GUID guid = GUID_DEVINTERFACE_USB_DEVICE;
	if (RegisterClassExW(&wx))
		hWnd = CreateWindowA("NONE", "DevNotifWnd", WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, 0, NULL, GetModuleHandle(nullptr), (void*)&guid);

	DEV_BROADCAST_DEVICEINTERFACE_A filter;
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_classguid = guid;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	RegisterDeviceNotificationA(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	SP_DEVINFO_DATA devInfoData;
	const HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE) {
		std::cout << "Cannot retrieve device information set" << std::endl;
		exitFlag = true;
		return -1;
	}
		
	printMenu();

	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData) == FALSE)
			break;
		Device device = Device(deviceInfoSet, devInfoData);

		if (device.isEjectable())
		{
			Device::devices.push_back(device);
			std::cout << Device::devices.size() << " ";
			device.print();
			std::cout << std::endl;
		}
			
	}
	SetupDiDestroyDeviceInfoList(deviceInfoSet);

	MSG msg;

	while (true) 
	{
		if (exitFlag)
			break;
		if(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	HANDLE thread = CreateThread(nullptr, 0, initialisationThread, nullptr, 0, nullptr);
	if (thread == nullptr)
	{
		std::cout << "Cannot create thread." << std::endl;
		return GetLastError();
	}

	while (true)
	{
		rewind(stdin);
		char ch = _getch();

		if (exitFlag) {
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			break;
		}
		if (ch >= '1' && ch <= '9')
		{

			if ((ch - '0') <= Device::devices.size()) {
				Device device = Device::devices[ch - '0' - 1];
				if (device.isEjectable()) {
					if (!device.eject()) {
						system("cls");
						std::cout << "Cannot eject the device." << std::endl;

						printMenu();
						printDevices();
					}
				}

				else {
					system("cls");
					std::cout << "Device isn't removable." << std::endl;

					printMenu();
					printDevices();
				}
				Sleep(100);
			}
		}
		else if (ch == 'q') {
			exitFlag = true;
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			break;
		}
	}
	return 0;
}
