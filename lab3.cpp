#include <Windows.h>
#include <iostream>

int main()
{
	setlocale(LC_ALL, "Russian");
	_SYSTEM_POWER_STATUS status{};
	GetSystemPowerStatus(&status);
	std::cout << "����������� � ����: " << (status.ACLineStatus? "ON": "OFF") << std::endl;
	std::cout << "������� ������ �������: " << (int)status.BatteryLifePercent << "%" << std::endl;
	std::cout << "������� ����� ����������������: " << (status.SystemStatusFlag? "ON": "OFF") << std::endl;
	system("pause");
}
