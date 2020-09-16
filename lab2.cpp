#include <Windows.h>
#include <Setupapi.h>
#include <Ntddstor.h>
#pragma comment(lib, "setupapi.lib")

#include <iostream>
#include <regex>
#include <iomanip>
#include <stdlib.h>
//#define WIN32_LEAN_AND_MEAN


int main() {
	setlocale(LC_ALL, "Russian");
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
}
