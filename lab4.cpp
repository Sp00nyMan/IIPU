#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <devguid.h>
#include <SetupAPI.h>
#include <conio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#pragma comment (lib, "Setupapi.lib")
HHOOK hHook(NULL);

HWND FRAME_WINDOW;
bool RECORDING = false;
bool EXIT = false;
bool Hide = false;
bool CAPTURE = false;
int COUNT = 0;

void showInstructions();
void makePhoto();
void makeVideo();
void printCameraInfo();
LRESULT CALLBACK myLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


int main()
{
	cv::VideoCapture vcap(0);

	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, myLowLevelKeyboardProc, NULL, 0);

	if(!vcap.isOpened())
	{
		std::cout << "Error opening video stream or file" << std::endl;
		return -1;
	}

	int frameWidth = vcap.get(cv::CAP_PROP_FRAME_WIDTH);
	int frameHeight = vcap.get(cv::CAP_PROP_FRAME_HEIGHT);

	char filename[30];
	printCameraInfo();
	showInstructions();

	vcap.~VideoCapture();
	for(int i = 0;; i++)
	{
		if(i == 0)
			FRAME_WINDOW = GetForegroundWindow();

		cv::waitKey(33);
		if(EXIT)
			break;
		if(RECORDING && !Hide)
		{
			makeVideo();
			RECORDING = false;
		}
		if(RECORDING && Hide)
		{
			RECORDING = false;
		}

		if(CAPTURE)
			makePhoto();
	}
	return 0;
}

void makePhoto()
{
	static int number = 0;
	number++;

	cv::VideoCapture cap(0);
	system("CLS");

	if(cap.isOpened())
	{
		cv::Mat img;
		cap >> img;
		if(!img.empty())
		{
			char fileName[1000];
			sprintf_s(fileName, "data\\photo %d.jpg", number);
			imwrite(fileName, img);
		}
	}

	system("CLS");
	std::cout << "Photo was made." << std::endl << std::endl;
	CAPTURE = false;
	showInstructions();

	cap.~VideoCapture();
}

void makeVideo()
{
	static int number = 0;
	number++;
	long videoLenght;
	std::cout << "Video lenght in seconds: ";
	std::cin >> videoLenght;
	long long curFrame = 0;
	auto prevClock = clock();

	char fileName[1000];
	sprintf_s(fileName, "data\\video %d.avi", number);

	cv::VideoCapture cap(0);

	system("CLS");

	double dWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	double dHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	cv::VideoWriter video(fileName, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(dWidth, dHeight));
	while(curFrame < videoLenght * 10)
	{
		while(clock() - prevClock < 100);
		prevClock = clock();
		curFrame++;

		cv::Mat frame;
		cap >> frame;
		if(frame.empty())
			break;
		video.write(frame);
	}

	cap.release();
	video.release();

	system("CLS");
	std::cout << "Video was made." << std::endl << std::endl;
	cap.~VideoCapture();
	showInstructions();
}

LRESULT CALLBACK myLowLevelKeyboardProc(const int nCode, const WPARAM wParam, const LPARAM lParam)
{
	switch(wParam)
	{
		case WM_KEYDOWN:
			KBDLLHOOKSTRUCT* k = (KBDLLHOOKSTRUCT*)lParam;
			if(k->vkCode == VK_F1)
			{
				ShowWindow(FindWindowA("ConsoleWindowClass", NULL), SW_HIDE);
				ShowWindow(FRAME_WINDOW, SW_HIDE);
				Hide = true;
			}
			if(k->vkCode == VK_F2)
			{
				ShowWindow(FindWindowA("ConsoleWindowClass", NULL), SW_SHOW);
				ShowWindow(FRAME_WINDOW, SW_SHOW);
				Hide = false;
			}
			if(k->vkCode == VK_F3)
				RECORDING = true;
			if(k->vkCode == VK_ESCAPE)
			{
				if(COUNT == 1)
					RECORDING = false;
				else
					EXIT = true;
			}
			if(k->vkCode == VK_F4)
				CAPTURE = true;
			break;
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void showInstructions()
{
	std::cout << "Menu"<< std::endl;

	std::cout << "F1 to hide program;"			<< std::endl << 
				 "F2 to show program;"			<< std::endl << 
				 "F3 for starting recording;"	<< std::endl <<
				 "F4 for taking a photo;"		<< std::endl <<
				 "Esc to exit"					<< std::endl;
}

void printCameraInfo()
{
	SP_DEVINFO_DATA DeviceInfoData = {0};
	HDEVINFO DeviceInfoSet = SetupDiGetClassDevsA(&GUID_DEVCLASS_CAMERA, "USB", NULL, DIGCF_PRESENT);
	if(DeviceInfoSet == INVALID_HANDLE_VALUE)
		exit(1);
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	SetupDiEnumDeviceInfo(DeviceInfoSet, 0, &DeviceInfoData);
	char deviceName[256];
	SetupDiGetDeviceRegistryPropertyA(DeviceInfoSet, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)deviceName,
									  sizeof(deviceName), 0);
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	std::cout << "Name: " << deviceName << std::endl;
}
