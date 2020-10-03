#include <Windows.h>
#include <iostream>

int main()
{
	setlocale(LC_ALL, "Russian");
	_SYSTEM_POWER_STATUS status{};
	GetSystemPowerStatus(&status);
	std::cout << "Подключение к сети: " << (status.ACLineStatus? "ON": "OFF") << std::endl;
	std::cout << "Уровень заряда батареи: " << (int)status.BatteryLifePercent << "%" << std::endl;
	std::cout << "Текущий режим энергосбережения: " << (status.SystemStatusFlag? "ON": "OFF") << std::endl;
	system("pause");
}
