#include <Windows.h>
#include <Setupapi.h>
#include <ntddscsi.h>
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

	const unsigned long presentMask = GetLogicalDrives();

	for (char letter = 'A'; letter < 'Z'; ++letter)
	{
		if (presentMask >> letter - 'A')
		{
			std::string drive = std::string(1, letter) + ":\\";

			_ULARGE_INTEGER total;
			_ULARGE_INTEGER free;
			GetDiskFreeSpaceExA(drive.c_str(),
				nullptr, &total, &free);
			total.QuadPart /= pow(1024, 2);
			free.QuadPart /= pow(1024, 2);

			if (GetDriveTypeA(drive.c_str()) == DRIVE_FIXED)
			{
				totalSpace.QuadPart += total.QuadPart;
				freeSpace.QuadPart += free.QuadPart;
			}
		}
	}
	std::cout << "Сведения о памяти [GB]: " << std::endl;

	std::cout << "\tВсего: " << totalSpace.QuadPart / 1024.0 << std::endl;
	std::cout << "\tСвободно: " << freeSpace.QuadPart / 1024.0 << std::endl;
	std::cout << "\tЗанято: " << (totalSpace.QuadPart - freeSpace.QuadPart) / 1024.0 << " (" << 100 - (double)freeSpace.QuadPart / totalSpace.QuadPart * 100 << "%)" << std::endl;
}

void getAtaPioDmaSupportStandarts(HANDLE diskHandle)
{
	std::cout << "Список поддерживаемых режимов: " << std::endl;
	UCHAR identifyDataBuffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };

	ATA_PASS_THROUGH_EX& PTE = *(ATA_PASS_THROUGH_EX*)identifyDataBuffer;	
	PTE.Length = sizeof(PTE);
	PTE.TimeOutValue = 10;									
	PTE.DataTransferLength = 512;							
	PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);		
	PTE.AtaFlags = ATA_FLAGS_DATA_IN;						

	if (!DeviceIoControl(diskHandle,
		IOCTL_ATA_PASS_THROUGH,							
		&PTE,
		sizeof(identifyDataBuffer),
		&PTE,
		sizeof(identifyDataBuffer),
		NULL,
		NULL))
	{
		if (GetLastError() == 5)
			std::cout << "Доступ запрещен" << std::endl;
		return;
	}

	WORD* data = (WORD*)(identifyDataBuffer + sizeof(ATA_PASS_THROUGH_EX));
	int bitArray[16];

	unsigned short dmaSupportedBytes = data[63];
	for (int i = 15; i >= 0; --i)
	{
		bitArray[i] = dmaSupportedBytes & (int)pow(2, 15) ? 1 : 0;
		dmaSupportedBytes = dmaSupportedBytes << 1;
	}

	std::cout << "Поддержка DMA: ";
	std::cout << "Multiword DMA: ";
	for (int i = 0; i < 3; i++)
		if (bitArray[i]) {
			std::cout << "DMA" << i << " и ниже";
			break;
		}
	std::cout << std::endl;

	dmaSupportedBytes = data[88];
	for (int i = 15; i >= 0; --i)
	{
		bitArray[i] = dmaSupportedBytes & (int)pow(2, 15) ? 1 : 0;
		dmaSupportedBytes = dmaSupportedBytes << 1;
	}

	std::cout << "Ultra DMA: ";
	for (int i = 0; i < 7; i++)
		if (bitArray[i]) {
			std::cout << "DMA" << i << " и ниже";
			break;
		}
	std::cout << std::endl;


	unsigned short pioSupportedBytes = data[64];

	for (int i = 15; i >= 0; --i)
	{
		bitArray[i] = pioSupportedBytes & (int)pow(2, 15) ? 1 : 0;
		pioSupportedBytes = pioSupportedBytes << 1;
	}

	std::cout << "Поддержка PIO: ";
	for (int i = 0; i < 8; i++)
		if (bitArray[i])
			std::cout << "PIO" << i << ", ";
	std::cout << "\b\b" << "  " << std::endl;
}

int getDiskInfo(const char* name)
{
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	storagePropertyQuery.QueryType = PropertyStandardQuery; 
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	HANDLE diskHandle = CreateFileA((std::string("\\\\.\\") + name).c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (diskHandle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = static_cast<STORAGE_DEVICE_DESCRIPTOR*>(calloc(1024, 1));
	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), deviceDescriptor, 1024, NULL, 0)) {
		return GetLastError();
	}


	char* tmp = (char*)deviceDescriptor;

	const std::string model(tmp + deviceDescriptor->ProductIdOffset);
	std::cout << "Производитель: " << model.substr(0, model.find_first_of(' ')) << std::endl;
	std::cout << "Модель: " << model.substr(model.find_first_of(' ') + 1) << std::endl;

	std::cout << "Серийный номер устройства: " << tmp + deviceDescriptor->SerialNumberOffset << std::endl;
	std::cout << "Версия прошивки (firmware): " << tmp + deviceDescriptor->ProductRevisionOffset << std::endl;
	std::cout << "Тип шины: " << busType[deviceDescriptor->BusType] << std::endl;

	getAtaPioDmaSupportStandarts(diskHandle);

	CloseHandle(diskHandle);
	return 0;
}

int main() {
	setlocale(LC_ALL, "Russian");
	getDiskInfo("PhysicalDrive0");
	getMemoryInfo();
	system("pause");
	return 0;
}
