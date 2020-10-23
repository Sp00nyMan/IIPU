#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

bool exitFlag = false;

DWORD WINAPI batteryStatusThread(void*) {
	_SYSTEM_POWER_STATUS status;
	while (true) {
		system("cls");
		if (exitFlag) {
			return 0;
		}
		GetSystemPowerStatus(&status);
		std::cout << "================LAB 3===============" << std::endl;
		std::cout << "Choose an option" << std::endl;
		std::cout << "s - Sleep" << std::endl;
		std::cout << "h - Hibernation" << std::endl;
		std::cout << "q - Quit" << std::endl;
		std::cout << "====================================" << std::endl;
		std::cout << "Battery info: " << std::endl;
		std::cout << "AC power status: " << (status.ACLineStatus ? "ON" : "OFF") << std::endl;
		std::cout << "Battery charge level: " << (int)status.BatteryLifePercent << "%" << std::endl;
		std::cout << "Battery saver mode: " << (status.SystemStatusFlag ? "ON" : "OFF") << std::endl;
		if (status.ACLineStatus)
			std::cout << "Battery life time: AC power ON" << std::endl;
		else 
		{
			int batteryLifeTime = status.BatteryLifeTime;
			std::cout << "Battery life time: ";
			if(batteryLifeTime == -1)
				std::cout << "calculating..." << std::endl;
			else
			{
				int hours = batteryLifeTime / 3600;
				int minutes = (batteryLifeTime - hours * 3600) / 60;
				int seconds = (batteryLifeTime - hours * 3600 - minutes * 60);
				std::cout << "\t - Hours: " << hours << std::endl;
				std::cout << "\t - Minutes: " << minutes << std::endl;
				std::cout << "\t - Seconds: " << seconds << std::endl;
			}
		}
		Sleep(100);
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	HANDLE thread = CreateThread(nullptr, 0, batteryStatusThread, nullptr, 0, nullptr);
	if (!thread) {
		std::cout << "Failed creating thread." << std::endl;
		return GetLastError();
	}

	while (!exitFlag) {
		rewind(stdin);
		if(_getch() == 's')
			SetSuspendState(FALSE, TRUE, FALSE);
		if(_getch() == 'h')
			SetSuspendState(TRUE, FALSE, FALSE);
		if(_getch() == 'q')
			exitFlag = true;
	}
	WaitForSingleObject(thread, INFINITE);
}
