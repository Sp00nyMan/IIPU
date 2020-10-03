#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <powrprof.h>

bool exitFlag = 0;

DWORD WINAPI batteryStatusThread(PVOID param) {
	_SYSTEM_POWER_STATUS status{};
	int batteryLifeTime = 0;
	int hours = 0, minutes = 0, seconds = 0;
	while (1) {
		system("cls");
		if (exitFlag) {
			return 0;
		}
		GetSystemPowerStatus(&status);
		std::cout << "=============================Lab 3==" << std::endl;
		std::cout << "Choose an option" << std::endl;
		std::cout << "s - sleep" << std::endl;
		std::cout << "g - gibernation" << std::endl;
		std::cout << "q - quit" << std::endl;
		std::cout << "====================================" << std::endl;
		std::cout << "Battery info: " << std::endl;
		std::cout << "AC power status: " << (status.ACLineStatus ? "ON" : "OFF") << std::endl;
		std::cout << "Battery charge level: " << (int)status.BatteryLifePercent << "%" << std::endl;
		std::cout << "Battery saver mode: " << (status.SystemStatusFlag ? "ON" : "OFF") << std::endl;
		if (status.ACLineStatus) {
			std::cout << "Battery life time: AC power ON" << std::endl;
		}
		else {
			batteryLifeTime = status.BatteryLifeTime;
			std::cout << "Battery life time: " << std::endl;
			hours = batteryLifeTime / 3600;
			std::cout << "\t - Hours: " << hours << std::endl;
			minutes = (batteryLifeTime - hours * 3600) / 60;
			std::cout << "\t - Minutes: " << minutes << std::endl;
			seconds = (batteryLifeTime - hours * 3600 - minutes * 60);
			std::cout << "\t - Seconds: " << seconds << std::endl;
		}
		Sleep(100);
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	HANDLE thread;
	thread = CreateThread(NULL, 0, batteryStatusThread, NULL, 0, NULL);
	if (thread == NULL) {
		std::cout << "Cannot create event." << std::endl;
		return GetLastError();
	}

	while (1) {
		rewind(stdin);
		if (_getch() == 's') {
			SetSuspendState(FALSE, TRUE, FALSE);
		}
		if (_getch() == 'g') {
			SetSuspendState(TRUE, FALSE, FALSE);
		}
		if (_getch() == 'q') {
			exitFlag = 1;
			break;
		}
	}
	WaitForSingleObject(thread, INFINITE);
	system("pause");
	return 0;
}
