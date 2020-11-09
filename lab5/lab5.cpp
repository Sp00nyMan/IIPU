#include "device.h"
#include <conio.h>

void printMenu() {
	std::cout << "=======================Lab 5=======================" << std::endl << std::endl;
	std::cout << ">Press q to exit" << std::endl;
	std::cout << ">Enter the number of the device to remove it" << std::endl << std::endl;
	std::cout << "==The list of connected USB devices (№/name/PID):==" << std::endl << std::endl;
}

std::vector<Device> Device::devices;
bool exitFlag = false;
HDEVNOTIFY notificationHandle, n;
HWND g_hWnd = NULL;
HANDLE h;
char chosenDevice;


LRESULT FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DEVICECHANGE) //https://docs.microsoft.com/en-us/windows/win32/devio/wm-devicechange
	{
		switch (wParam)
		{
		case DBT_DEVICEARRIVAL:
		{
			Device device((PDEV_BROADCAST_DEVICEINTERFACE_A)lParam, (PDEV_BROADCAST_DEVICEINTERFACE)lParam, g_hWnd);
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

			Device::devices[chosenDevice - '0' - 1].close();

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
		{
			std::cout << "Trying to remove..." << std::endl;			
			break;
		}
		case DBT_DEVICEQUERYREMOVEFAILED:
		{
			std::cout << "Failed to remove." << std::endl;
			Sleep(1000);
			break;
		}
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

	
	GUID guid = GUID_DEVINTERFACE_USB_DEVICE; //https://docs.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
	if (RegisterClassExW(&wx)) //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		g_hWnd = CreateWindowA("NONE", "DevNotifWnd", WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, 0, NULL, GetModuleHandle(nullptr), (void*)&guid);

	DEV_BROADCAST_DEVICEINTERFACE_A filter; //https://docs.microsoft.com/en-us/windows/win32/api/dbt/ns-dbt-dev_broadcast_deviceinterface_a
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_classguid = guid;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	notificationHandle = RegisterDeviceNotificationA(g_hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE ); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerdevicenotificationw

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
		Device device = Device(deviceInfoSet, devInfoData, g_hWnd);

		if (device.isEjectable())
		{
			Device::devices.push_back(device);
			std::cout << Device::devices.size() << " ";
			device.print();
			std::cout << std::endl;
		}
			
	}
	SetupDiDestroyDeviceInfoList(deviceInfoSet);

	MSG msg; //https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg

	while (1) {
		if (exitFlag) {
			break;
		}
		if(PeekMessage(&msg, g_hWnd, 0, 0, PM_REMOVE)) //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
		{
			TranslateMessage(&msg); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
			DispatchMessage(&msg); //https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
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
		chosenDevice = ch;

		if (exitFlag) {
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			break;
		}
		if (ch >= '1' && ch <= '9')
		{

			if ((ch - '0') <= Device::devices.size()) {
				if (Device::devices[ch - '0' - 1].isEjectable()) {
					if (!Device::devices[ch - '0' - 1].eject(g_hWnd, ch - '0' - 1)) {
						system("cls");
						std::cout << "Cannot eject the device." << std::endl;

						printMenu();
						for (int i = 0; i < Device::devices.size(); i++)
						{
							std::cout << i + 1 << " ";
							Device::devices[i].print();
							std::cout << std::endl;
						}
					}
				}

				else {
					system("cls");
					std::cout << "Device isn't removable." << std::endl;

					printMenu();
					for (int i = 0; i < Device::devices.size(); i++)
					{
						std::cout << i + 1 << " ";
						Device::devices[i].print();
						std::cout << std::endl;
					}
				}
				Sleep(100);
			}
		}
		else if (ch == 'q') {
			exitFlag = true;
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			UnregisterDeviceNotification(notificationHandle);
			for (int i = 0; i < Device::devices.size(); i++)
			{
				Device::devices[i].close();
			}
			break;
		}
	}
	return 0;
}
