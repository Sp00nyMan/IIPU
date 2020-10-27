#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "setupapi.lib")
#include "opencv2/opencv.hpp"

#include <Windows.h>
#include <conio.h>

#include <Setupapi.h>
#include <devguid.h>

#include <iostream>

HWND hWnd;
HHOOK g_hHook;
int photoNumber = 1;
int videoNumber = 1;
bool isCameraInUse = false;
bool hideFlag = false;

void takePhoto()
{
	isCameraInUse = true;
	cv::Mat frame;
	cv::VideoCapture vcap(0);

	if(!vcap.isOpened())
	{
		std::cerr << "Error! Unable to open camera" << std::endl;
		return;
	}

	vcap >> frame;
	if(!frame.empty())
	{
		const std::string filename("data\\Photo_" + std::to_string(photoNumber) + ".jpg");
		cv::waitKey(10);
		imwrite(filename, frame);
	}
	system("CLS");
	photoNumber++;
	std::cout << "The photo was taken" << std::endl;
	isCameraInUse = false;
	Sleep(1000);
}

void recordVideo()
{
	isCameraInUse = true;

	cv::Mat frame;
	cv::VideoCapture vcap;

	vcap.open(0);
	if(!vcap.isOpened())
	{
		std::cout << "Error! Unable to open camera" << std::endl;
		return;
	}
	const int frame_width = vcap.get(cv::CAP_PROP_FRAME_WIDTH);
	const int frame_height = vcap.get(cv::CAP_PROP_FRAME_HEIGHT);
	const std::string filename("data\\Video_" + std::to_string(videoNumber) + ".avi");

	cv::VideoWriter video(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(frame_width, frame_height));
															//motion-jpeg codec
	system("CLS");
	std::cout << "Start recording the video..." << std::endl;
	for(int i = 0; i < 70; i++)
	{
		vcap >> frame;
		cv::waitKey(33);
		video.write(frame);
	}

	videoNumber++;
	std::cout << "The video was recorded" << std::endl;
	isCameraInUse = false;
	Sleep(1000);
}

void camInfo()
{
	setlocale(LC_ALL, "Russian");
	HDEVINFO devInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_CAMERA, "USB", NULL, DIGCF_PRESENT);
	if(devInfo == INVALID_HANDLE_VALUE)
		return;

	SP_DEVINFO_DATA devInfoData;
	TCHAR buffer[1024];

	char instanceIDBuffer[1024];

	for(int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if(SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_DEVICEDESC, NULL, (BYTE*)buffer, 1024, NULL);
		std::wstring name(buffer);

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_HARDWAREID, NULL, (BYTE*)buffer, 1024, NULL);
		std::wstring ids(buffer);

		std::wstring ven(ids.substr(ids.find(L"VID_") + 4, 4));
		std::wstring dev(ids.substr(ids.find(L"PID_") + 4, 4));

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceInstanceIdA(devInfo, &devInfoData, (PSTR)instanceIDBuffer, 1024, NULL);
		std::string instanceID(instanceIDBuffer);

		if(name.substr(name.size() - 4, 4) == dev)
			name = name.substr(0, name.size() - 7);

		std::cout << "Information about camera:" << std::endl;
		std::wcout << L"Name: " << name << std::endl;
		std::wcout << L"Vendor ID: " << ven << std::endl;
		std::wcout << L"Device ID: " << dev << std::endl;
		std::cout << "Instance ID: " << instanceID << std::endl;

		SetupDiDeleteDeviceInfo(devInfo, &devInfoData);
	}
	SetupDiDestroyDeviceInfoList(devInfo);
}

void menu()
{
	std::cout << "===================Lab 4==================" << std::endl;
	camInfo();
	std::cout << "==========================================" << std::endl
		<< "Choose an option:" << std::endl
		<< "h - hide window" << std::endl
		<< "s - show window" << std::endl
		<< "p - make a photo" << std::endl
		<< "v - make a video" << std::endl
		<< "esc - exit" << std::endl;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode >= 0)
	{
		if(wParam == WM_KEYDOWN && !isCameraInUse)
		{
			KBDLLHOOKSTRUCT* pkhs = (KBDLLHOOKSTRUCT*)lParam;
			switch(pkhs->vkCode)
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			case 'H':
				ShowWindow(hWnd, SW_HIDE);
				hideFlag = true;
				break;
			case 'S':
				ShowWindow(hWnd, SW_RESTORE);
				hideFlag = false;
				break;
			case 'P':
				takePhoto();
				break;
			case 'V':
				if(hideFlag)
					break;
				recordVideo();
				break;
			}
			system("cls");
			menu();
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

int main()
{
	menu();
	hWnd = GetConsoleWindow();
	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
	MSG msg;
	GetMessage(&msg, nullptr, 0, 0);
	UnhookWindowsHookEx(g_hHook);
	return 0;
}
