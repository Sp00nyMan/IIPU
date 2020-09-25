#include <Windows.h>
#include <Setupapi.h>
#pragma comment(lib, "setupapi.lib")

#include <iostream>
std::string busType[] = { "TypeUnknown",
							"Scsi",
							"Atapi",
							"Ata",
							"1394",
							"Ssa",
							"Fibre",
							"Usb",
							"RAID",
							"Scsi",
							"Sas",
							"Sata",
							"Sd",
							"Mmc",
							"Virtual",
							"FileBackedVirtual",
							"Spaces",
							"Nvme",
							"SCM",
							"Ufs",
							"Max",
							"Reserved" };

void getMemoryInfo()
{
	_ULARGE_INTEGER totalSpace;
	_ULARGE_INTEGER freeSpace;
	totalSpace.QuadPart = 0;
	freeSpace.QuadPart = 0;

	//Получаем битовую маску, представляющую имеющиеся в настоящие время дисковые накопители. 
	const unsigned long presentMask = GetLogicalDrives();

	for(char letter = 'A'; letter < 'Z'; ++letter)
	{
		if(presentMask >> letter - 'A')
		{
			std::string drive = std::string(1, letter) + ":\\";
			
			_ULARGE_INTEGER total;
			_ULARGE_INTEGER free;
			GetDiskFreeSpaceExA(drive.c_str(), 
				nullptr, &total, &free);
			total.QuadPart /= pow(1024, 2);
			free.QuadPart /= pow(1024, 2);

			//Определяем тип диска(3 - фиксированный диск) 
			if (GetDriveTypeA(drive.c_str()) == DRIVE_FIXED)
			{
				totalSpace.QuadPart += total.QuadPart;
				freeSpace.QuadPart += free.QuadPart;
			}
		}
	}
	std::cout << "Информация о пямяти [GB]: " << std::endl;
	
	std::cout << "\tВсего: " << totalSpace.QuadPart / 1024.0 << std::endl;
	std::cout << "\tСвободно: " << freeSpace.QuadPart / 1024.0 << std::endl;
	std::cout << "\tЗанято: " << (totalSpace.QuadPart - freeSpace.QuadPart) / 1024.0 << " ("<< 100 - (double)freeSpace.QuadPart / totalSpace.QuadPart * 100 << "%)" << std::endl;
}

int getDiskInfo(const char* name)
{
	STORAGE_PROPERTY_QUERY storagePropertyQuery;				//Структура с информацией об запросе 
	storagePropertyQuery.QueryType = PropertyStandardQuery;		//Запрос драйвера, чтобы он вернул дескриптор устройства. 
	storagePropertyQuery.PropertyId = StorageDeviceProperty;	//Флаг, гооврящий мы хотим получить дескриптор устройства. 
	HANDLE diskHandle = CreateFileA((std::string("\\\\.\\") + name).c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (diskHandle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = static_cast<STORAGE_DEVICE_DESCRIPTOR*>(calloc(1024, 1));
	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), deviceDescriptor, 1024, NULL, 0)) {
		return GetLastError();
	}


	char *tmp = (char*)deviceDescriptor;

	const std::string model(tmp + deviceDescriptor->ProductIdOffset);
	std::cout << "Производитель: " << model.substr(0, model.find_first_of(' ')) << std::endl;
	std::cout << "Модель: " <<  model.substr(model.find_first_of(' ') + 1) << std::endl;

	std::cout << "Серийный номер устройства: " << tmp + deviceDescriptor->SerialNumberOffset << std::endl;
	std::cout << "Версия прошивки (firmware): " << tmp + deviceDescriptor->ProductRevisionOffset << std::endl;
	std::cout << "Тип шины: " << busType[deviceDescriptor->BusType] << std::endl;

	CloseHandle(diskHandle);
}

int main() {
	setlocale(LC_ALL, "Russian");
	getDiskInfo("PhysicalDrive0");
	getMemoryInfo();
	system("pause");
	return 0;
	/*
	HANDLE devHandle = CreateFileA("\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (devHandle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	TCHAR buffer[1024];
	DWORD bytesReturned;
	memset(buffer, 0, sizeof(buffer));

	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	STORAGE_DEVICE_DESCRIPTOR* storageDeviceDescriptor;
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
	
	ZeroMemory(&storagePropertyQuery, sizeof(storagePropertyQuery));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	
	ZeroMemory(&storageDescriptorHeader, sizeof(storageDescriptorHeader));


	if (!DeviceIoControl(devHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), &storageDescriptorHeader, sizeof(storageDescriptorHeader), &bytesReturned, 0)) {
		return GetLastError();
	}

	//const DWORD outBufferSize = storageDescriptorHeader.Size;
	//BYTE* outBuffer = (BYTE*)malloc(outBufferSize);

	char outBuffer[1024];

	if (!DeviceIoControl(devHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), outBuffer, 1024, &bytesReturned, 0)) {
		return GetLastError();
	}
	storageDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)outBuffer;

	int offset = storageDeviceDescriptor->ProductIdOffset;
	std::cout << "Название устройства: ";
	while (outBuffer[offset] != '\0') {
		std::cout << outBuffer[offset];
		offset++;
	}
	std::cout << std::endl;

	offset = storageDeviceDescriptor->SerialNumberOffset;
	std::cout << "Серийный номер устройства: ";
	while (outBuffer[offset+1] != '\0') {
		std::cout << outBuffer[offset];
		offset++;
	}
	std::cout << std::endl;

	offset = storageDeviceDescriptor->ProductRevisionOffset;
	std::cout << "Версия прошивки (firmware): ";
	while (outBuffer[offset] != '\0') {
		std::cout << outBuffer[offset];
		offset++;
	}
	std::cout << std::endl;

	//STORAGE_HW_FIRMWARE_INFO_QUERY s;
	//STORAGE_HW_FIRMWARE_INFO* f;
	//STORAGE_HW_FIRMWARE_SLOT_INFO* f1;
	//ZeroMemory(&s, sizeof(s));

	//memset(outBuffer, '\0', 1024);

	//ZeroMemory(&storagePropertyQuery, sizeof(storagePropertyQuery));
	//storagePropertyQuery.PropertyId = StorageDeviceProperty;
	//storagePropertyQuery.QueryType = PropertyStandardQuery;

	//if (!DeviceIoControl(devHandle, IOCTL_STORAGE_FIRMWARE_GET_INFO, &s, sizeof(s), outBuffer, 1024, &bytesReturned, 0)) {
	//	return GetLastError();
	//}

	////f = (STORAGE_HW_FIRMWARE_INFO*)outBuffer;
	////f += sizeof(f);
	//f1 = (STORAGE_HW_FIRMWARE_SLOT_INFO*)outBuffer;

	CloseHandle(devHandle);

	system("pause");
	return 0;
	*/
}