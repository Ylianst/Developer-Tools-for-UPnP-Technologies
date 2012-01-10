/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <malloc.h>
#include <windows.h>
#include <memory.h>
#include "Light Bulb.h"
#define MAX_LOADSTRING 100

extern "C"
{
	#include "ILibParsers.h"
	#include "UPnPMicroStack.h"
}

static bool SwitchPower = false;
static int  DimmingService = 100;
static HWND	hWndMainWindow = NULL;
DWORD WINAPI UPnPMain(LPVOID ptr);

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

void *UPnPmicroStackChain;
void *UPnPmicroStack;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LIGHTBULB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LIGHTBULB);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_LIGHTBULB);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_LIGHTBULB;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDC_LIGHTBULB);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   hWnd = CreateWindow(szWindowClass, szTitle, WS_SYSMENU | WS_BORDER,
      CW_USEDEFAULT, CW_USEDEFAULT, 240, 320, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hWndMainWindow = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	LPSTR s;
	LPSTR str;
	HBRUSH brush;
	COLORREF color;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			PostMessage(hWnd,WM_CLOSE,0,0);
			break;
		case ID_FILE_TOGGLESWITCH:
			SwitchPower = !SwitchPower;
			InvalidateRect(hWndMainWindow,NULL,true);
			UPnPSetState_SwitchPower_Status(UPnPmicroStack,SwitchPower);
			break;
		case ID_FILE_DIMMERUP:
			DimmingService += 20;
			if (DimmingService > 100) DimmingService = 100;
			InvalidateRect(hWndMainWindow,NULL,true);
			UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
			break;
		case ID_FILE_DIMMERDOWN:
			DimmingService -= 20;
			if (DimmingService < 0) DimmingService = 0;
			InvalidateRect(hWndMainWindow,NULL,true);
			UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		RECT rt;

		hdc = BeginPaint(hWnd, &ps);

		color = RGB(128,128,128);
		if (SwitchPower == true)
		{
			color = RGB(128+DimmingService,128+DimmingService,128-DimmingService);
		}

		GetClientRect(hWnd, &rt);
		brush = CreateSolidBrush(color);
		SetBkColor(hdc,color);
		FillRect(hdc,&rt,brush);
		DeleteObject(brush);

		if (SwitchPower == true) s = "ON"; else s = "OFF";
		str = (char*)malloc(200);
		sprintf(str,"Power %s, Dimmer %d",s,DimmingService);
		DrawText(hdc, str, (int)strlen(str), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		free(str);

		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
		CreateThread(NULL, NULL, &UPnPMain, hWnd, 0, NULL);
		break;
	case WM_CLOSE:
		ILibStopChain(UPnPmicroStackChain);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		//IDC_ABOUTEDITBOX

		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void UPnPSwitchPower_GetStatus(void* upnptoken)
{
	UPnPResponse_SwitchPower_GetStatus(upnptoken,SwitchPower);
}

void UPnPSwitchPower_SetTarget(void* upnptoken,int newTargetValue)
{
	SwitchPower = (newTargetValue != 0);
	InvalidateRect(hWndMainWindow,NULL,true);
	UPnPResponse_SwitchPower_SetTarget(upnptoken);
	UPnPSetState_SwitchPower_Status(UPnPmicroStack,SwitchPower);
}

void UPnPDimmingService_GetLoadLevelStatus(void* upnptoken)
{
	UPnPResponse_DimmingService_GetLoadLevelStatus(upnptoken,DimmingService);
}

void UPnPDimmingService_GetMinLevel(void* upnptoken)
{
	UPnPResponse_DimmingService_GetMinLevel(upnptoken,0);
}

void UPnPDimmingService_SetLoadLevelTarget(void* upnptoken,unsigned char NewLoadLevelTarget)
{
	DimmingService = NewLoadLevelTarget;
	InvalidateRect(hWndMainWindow,NULL,true);
	UPnPResponse_DimmingService_SetLoadLevelTarget(upnptoken);
	UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
}

void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 404 File not found\r\n\r\n" , 31 , 1);
}

DWORD WINAPI UPnPMain(LPVOID ptr)
{
	int i;
	char guid[20];
	char friendlyname[100];
	WSADATA wsaData;

	srand((int)GetTickCount());
	for (i=0;i<19;i++)
	{
		guid[i] = (rand() % 25) + 66;
	}
	guid[19] = 0;

	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {exit(1);}
	memcpy(friendlyname,"Intel MicroLight (",18);
	gethostname(friendlyname+18,70);
	memcpy(friendlyname+strlen(friendlyname),")\0",2);

	UPnPmicroStackChain = ILibCreateChain();
	UPnPmicroStack = UPnPCreateMicroStack(UPnPmicroStackChain,friendlyname,guid,"0000001",120,8056);

	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_SwitchPower_Status(UPnPmicroStack,SwitchPower);
	UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
	
	ILibStartChain(UPnPmicroStackChain);

	WSACleanup();
	SendMessage((HWND)ptr, WM_DESTROY, 0, 0);

	return 0;
}
