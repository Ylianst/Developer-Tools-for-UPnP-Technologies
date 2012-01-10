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
#include "UPnP Light.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <winsock.h>

extern "C"
{
	#include "ILibParsers.h"
	#include "UPnPMicroStack.h"
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;				// The current instance
HWND				g_hwndCB;				// The command bar handle

static SHACTIVATEINFO s_sai;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);


static bool SwitchPower = false;
static int  DimmingService = 100;
static HWND	hWndMainWindow = NULL;
static HANDLE UPnPThread = NULL;

void *UPnPmicroStackChain;
void *UPnPmicroStack;

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_UPNPLIGHT);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UPNPLIGHT));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd = NULL;
	TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	g_hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_UPNPLIGHT, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

	//If it is already running, then focus on the window
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		// set focus to foremost child window
		// The "| 0x01" is used to bring any owned windows to the foreground and
		// activate them.
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	} 

	MyRegisterClass(hInstance, szWindowClass);
	
	RECT	rect;
	GetClientRect(hWnd, &rect);
	
	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{	
		return FALSE;
	}

	hWndMainWindow = hWnd;

	//When the main window is created using CW_USEDEFAULT the height of the menubar (if one
	// is created is not taken into account). So we resize the window after creating it
	// if a menubar is present
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		rc.bottom -= MENU_HEIGHT;
		if (g_hwndCB)
			MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, FALSE);
	}


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

unsigned long UPnPMain(void* ptr);

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
	HDC hdc;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	wchar_t* str;
	wchar_t* s;
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
				case IDM_HELP_ABOUT:
					DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				    break;
				case IDOK:
				case IDM_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case ID_FILE_TOGGLESWITCH:
					SwitchPower = !SwitchPower;
					InvalidateRect(hWndMainWindow,NULL,false);
					UPnPSetState_SwitchPower_Status(UPnPmicroStack,SwitchPower);
					break;
				case ID_FILE_DIMMERUP:
					DimmingService += 20;
					if (DimmingService > 100) DimmingService = 100;
					InvalidateRect(hWndMainWindow,NULL,false);
					UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
					break;
				case ID_FILE_DIMMERDOWN:
					DimmingService -= 20;
					if (DimmingService < 0) DimmingService = 0;
					InvalidateRect(hWndMainWindow,NULL,false);
					UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CLOSE:
			ILibStopChain(UPnPmicroStackChain);
			break;
		case WM_CREATE:
			g_hwndCB = CreateRpCommandBar(hWnd);
            // Initialize the shell activate info structure
            memset (&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
			UPnPThread = CreateThread(NULL, 0, &UPnPMain, 0, 0, NULL ); 
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

			if (SwitchPower == true) s = TEXT("ON"); else s = TEXT("OFF");
			str = (unsigned short*)malloc(400);
			swprintf(str,TEXT("Power %s, Dimmer %d"),s,DimmingService);
			DrawText(hdc, str, -1, &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			free(str);

			EndPaint(hWnd, &ps);
			break; 
		case WM_DESTROY:
			CommandBar_Destroy(g_hwndCB);
			PostQuitMessage(0);
			break;
		case WM_ACTIVATE:
            // Notify shell of our activate message
			SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
     		break;
		case WM_SETTINGCHANGE:
			SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
     		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

HWND CreateRpCommandBar(HWND hwnd)
{
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hwnd;
	mbi.nToolBarId = IDM_MENU;
	mbi.hInstRes   = g_hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;

	if (!SHCreateMenuBar(&mbi)) 
		return NULL;

	return mbi.hwndMB;
}

// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	SHINITDLGINFO shidi;

	switch (message)
	{
		case WM_INITDIALOG:
			// Create a Done button and size it.  
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);
			return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
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
	InvalidateRect(hWndMainWindow,NULL,false);
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
	InvalidateRect(hWndMainWindow,NULL,false);
	UPnPResponse_DimmingService_SetLoadLevelTarget(upnptoken);
	UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
}

void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 404 File not found\r\n\r\n" , 31 , 1);
}

unsigned long UPnPMain(void* ptr)
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
	UPnPmicroStack = UPnPCreateMicroStack(UPnPmicroStackChain,friendlyname,guid,"0000001",120,8085);

	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_SwitchPower_Status(UPnPmicroStack,SwitchPower);
	UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,DimmingService);
	
	ILibStartChain(UPnPmicroStackChain);

	WSACleanup();
	SendMessage(hWndMainWindow, WM_DESTROY, 0, 0);

	UPnPThread = 0;
	return 0;
}

