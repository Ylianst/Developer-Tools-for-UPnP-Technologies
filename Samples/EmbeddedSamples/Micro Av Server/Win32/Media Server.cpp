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
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <windows.h>
#include "Media Server.h"
#include <string.h>

#define MAX_LOADSTRING 100


extern "C"
{
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"

	// CDS Headers
	#include "MicroMediaServer.h"
	#include "MimeTypes.h"
	#include "mystring.h"
}

static HWND	hWndMainWindow = NULL;
static HANDLE UPnPThread = NULL;
unsigned long WINAPI UPnPMain(void* ptr);
void *TheChain;
void *TheStack;

#ifdef _WINSOCK1
void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;
#endif

#ifdef _WINSOCK2
DWORD UpnpMonitorSocketReserved;
WSAOVERLAPPED UpnpMonitorSocketStateObject;
SOCKET UpnpMonitorSocket;
#endif


#define MAX_SHOWN_TRANSFERS	5
HINSTANCE g_hInst;								// current instance
int   g_DownloadStatsMapping[DOWNLOAD_STATS_ARRAY_SIZE];

// Handle to all of the icon ressources.
HICON HICON_MEDIASERVER  = NULL;
HICON HICON_MEDIASERVER2 = NULL;
HICON HICON_RIGHTARROW   = NULL;
HICON HICON_LEFTARROW    = NULL;

void DrawTransferInfo(HDC hdc,int pos, int index);
void AdjustDisplayMapping(HDC hdc,int changeindex);

//DWORD (WINAPI *PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

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

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MEDIASERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MEDIASERVER);

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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_MEDIASERVER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_MEDIASERVER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_MEDIASERVER);

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

   g_hInst = hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   hWnd = CreateWindow(szWindowClass, szTitle, WS_BORDER | WS_SYSMENU,
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

void ShadowBox(HDC hdc,const RECT *rect)
{
	HPEN p1 = CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
	HPEN p2 = CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DLIGHT));
	SelectObject(hdc,p2);
	MoveToEx(hdc,rect->left,rect->bottom,NULL);
	LineTo(hdc,rect->left,rect->top);
	LineTo(hdc,rect->right,rect->top);
	SelectObject(hdc,p1);
	LineTo(hdc,rect->right,rect->bottom);
	LineTo(hdc,rect->left,rect->bottom);
	DeleteObject(p1);
	DeleteObject(p2);
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
	int wmId, wmEvent,i;
	PAINTSTRUCT ps;
	HDC hdc;
	char* str[100];
	RECT rt;

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
			ILibStopChain(TheChain);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:

		// Paint the complete screen here. Quite workout to get the UI looking
		// event half decent using Win32 directly.
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMainWindow, &rt);

		// Paint the top portion of the screen
		RECT r;
		r.left = 1;
		r.right = rt.right-1;
		r.top = 0;
		r.bottom = 40;
		FillRect(hdc,&r,GetSysColorBrush(COLOR_SCROLLBAR));
		DrawEdge(hdc,&r,EDGE_RAISED,BF_RECT);
		DrawIcon(hdc,8,5,HICON_MEDIASERVER2);
		SetBkColor(hdc,GetSysColor(COLOR_SCROLLBAR));

		// Paint the title
		r.left = 50;
		r.right = rt.right-1;
		r.top = 4;
		r.bottom = 50;
		DrawText(hdc,TEXT("Intel Micro Media Server"),-1,&r,0);

		// Paint the transfer count stat label & value
		r.left = 50;
		r.right = rt.right-1;
		r.top = 20;
		r.bottom = 50;
		if (MmsCurrentTransfersCount == 0)
		{
			DrawText(hdc,TEXT("No File Transfers          "),-1,&r,0);
		}
		if (MmsCurrentTransfersCount == 1)
		{
			DrawText(hdc,TEXT("1 File Transfer          "),-1,&r,0);
		}
		if (MmsCurrentTransfersCount > 1)
		{
			sprintf((char*)str,"%d File Transfers        ",MmsCurrentTransfersCount);
			DrawText(hdc,(LPCSTR)str,-1,&r,0);
		}

		// Paint the main portion of the screen
		r.left = 1;
		r.right = rt.right-1;
		r.top = 42;
		r.bottom = 267;
		FillRect(hdc,&r,GetSysColorBrush(COLOR_SCROLLBAR));

		// Paint global media server stats labels
		r.left = 8;
		r.right = 150;
		r.top = 50;
		r.bottom = 70;
		DrawText(hdc,TEXT("Browse Requests"),-1,&r,0);
		r.left = 8;
		r.right = 150;
		r.top = 70;
		r.bottom = 90;
		DrawText(hdc,TEXT("HTTP Requests"),-1,&r,0);

		// Paint global media server stats values
		sprintf((char*)str,"%d",MmsBrowseCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 50;
		r.bottom = 70;
		DrawText(hdc,(LPCSTR)str,-1,&r,DT_RIGHT);
		sprintf((char*)str,"%d",MmsHttpRequestCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 70;
		r.bottom = 90;
		DrawText(hdc,(LPCSTR)str,-1,&r,DT_RIGHT);

		// Paint the transfer window edge
		r.left = 2;
		r.right = rt.right-1;
		r.top = 94;
		r.bottom = 264;
		DrawEdge(hdc,&r,EDGE_SUNKEN,BF_RECT);

		// Paint the white transfer window
		r.left = 4;
		r.right = rt.right-5;
		r.top = 96;
		r.bottom = 262;
		FillRect(hdc,&r,GetSysColorBrush(COLOR_MENU));

		// Draw all of the active transfers on the screen (up to 5)
		for (i=0;i<5;i++)
		{
			DrawTransferInfo(hdc,i,g_DownloadStatsMapping[i]);
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
		// Clear up all of the active transfers.
		for (i=0;i<DOWNLOAD_STATS_ARRAY_SIZE;i++) {g_DownloadStatsMapping[i] = -1;}

		// Load all of the icon resources we will need to draw the UI.
		HICON_MEDIASERVER  = LoadIcon(g_hInst, (LPCTSTR)IDI_MEDIASERVER);
		HICON_MEDIASERVER2 = LoadIcon(g_hInst, (LPCTSTR)IDI_MEDIASERVER2);
		HICON_RIGHTARROW   = LoadIcon(g_hInst, (LPCTSTR)IDI_RIGHTARROW);
		HICON_LEFTARROW    = LoadIcon(g_hInst, (LPCTSTR)IDI_LEFTARROW);

		CreateThread(NULL, 0, &UPnPMain, hWnd, 0, NULL ); 
		break;
	case WM_CLOSE:
		ILibStopChain(TheChain);
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

// This method draws a single active transfer to the screen.
// pos - a number from 0 to 4, the position on the screen that
// is used to draw the information.
// index - the transfer index to draw to the screen.
void DrawTransferInfo(HDC hdc,int pos, int index)
{
	int i,j;
	char* str[100];
	RECT r;
	HBRUSH lightgraybrush;

	if (index == -1 || MmsMediaTransferStats[index].filename == NULL)
	{
		r.left = 6;
		r.right = 229;
		r.top = 98+(pos*32);
		r.bottom = (98+32)+(pos*32);
		lightgraybrush = CreateSolidBrush(0x00FFFFFF);
		FillRect(hdc,&r,lightgraybrush);
		DeleteObject(lightgraybrush);
	}
	else
	{
		SetBkColor(hdc,0x00FFFFFF);
		DrawIconEx(hdc,6,106+(pos*32),HICON_LEFTARROW,16,16,0,NULL,DI_IMAGE | DI_MASK);
		r.left = 24;
		r.right = 229;
		r.top = 98+(pos*32);
		r.bottom = 120+(pos*32);

		j = 0;
		for (i=(int)strlen(MmsMediaTransferStats[index].filename);i>=0;i--)
		{
			if (MmsMediaTransferStats[index].filename[i] == '\\' || MmsMediaTransferStats[index].filename[i] == '/')
			{
				j = i+1;
				break;
			}
		}

		DrawText(hdc,MmsMediaTransferStats[index].filename + j,-1,&r,0);
		r.left = 24;
		r.right = 229;
		r.top = 114+(pos*32);
		r.bottom = 138+(pos*32);

		if (MmsMediaTransferStats[index].length > 0)
		{
			sprintf((char*)str,"%d of %d (%d%%)          ",MmsMediaTransferStats[index].position,MmsMediaTransferStats[index].length,(MmsMediaTransferStats[index].position*100)/MmsMediaTransferStats[index].length);
			DrawText(hdc,(LPCSTR)str,-1,&r,0);
		}
		else
		{
			sprintf((char*)str,"Position: %d          ",MmsMediaTransferStats[index].position);
			DrawText(hdc,(LPCSTR)str,-1,&r,0);
		}
	}
}

void AdjustDisplayMapping(HDC hdc,int changeindex)
{
	int i,j;
	j = 0;
	for (i=0;i<DOWNLOAD_STATS_ARRAY_SIZE;i++)
	{
		if (MmsMediaTransferStats[i].filename != NULL)
		{
			if (g_DownloadStatsMapping[j] != i || i == changeindex)
			{
				if (g_DownloadStatsMapping[j] != i) {DrawTransferInfo(hdc,j,-1);}
				g_DownloadStatsMapping[j] = i;
				if (j < 5) {DrawTransferInfo(hdc,j,i);}
			}
			j++;
		}
	}
	for (i=j;i<DOWNLOAD_STATS_ARRAY_SIZE;i++)
	{
		if (g_DownloadStatsMapping[i] != -1)
		{
			g_DownloadStatsMapping[i] = -1;
			if (i < MAX_SHOWN_TRANSFERS) DrawTransferInfo(hdc,i,-1);
		}
	}
}

void MmsOnStatsChangedSink()
{
	char* str[100];
	HDC hdc;
	RECT r;

	hdc = GetDC(hWndMainWindow);
	if (hdc != NULL)
	{
		RECT rt;
		GetClientRect(hWndMainWindow, &rt);
		SetBkColor(hdc,GetSysColor(COLOR_SCROLLBAR));

		r.left = 50;
		r.right = rt.right-1;
		r.top = 20;
		r.bottom = 50;
		if (MmsCurrentTransfersCount == 0)
		{
			DrawText(hdc,"No File Transfers",-1,&r,0);
		} else
		if (MmsCurrentTransfersCount == 1)
		{
			DrawText(hdc,"1 File Transfer",-1,&r,0);
		} else
		if (MmsCurrentTransfersCount > 1)
		{
			sprintf((char*)str,"%d File Transfers",MmsCurrentTransfersCount);
			DrawText(hdc,(LPCSTR)str,-1,&r,0);
		}

		sprintf((char*)str,"%d",MmsBrowseCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 50;
		r.bottom = 70;
		DrawText(hdc,(LPCSTR)str,-1,&r,DT_RIGHT);

		sprintf((char*)str,"%d",MmsHttpRequestCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 70;
		r.bottom = 90;
		DrawText(hdc,(LPCSTR)str,-1,&r,DT_RIGHT);

		ReleaseDC(hWndMainWindow,hdc);
	}
}

// Micro Media Server callback handler.
// Called when an active transfer data has changed: New transfer,
// position changed or transfer over.
void MmsMediaTransferStatsSink(int index)
{
	HDC hdc;
	hdc = GetDC(hWndMainWindow);
	
	// We simple call the display method and force him to repaint this
	// transfer. If the transfer is over, the method will also fix up
	// the UI, bringing other transfer up by one on the screen if needed.
	AdjustDisplayMapping(hdc,index);
	ReleaseDC(hWndMainWindow,hdc);
}

#ifdef _WINSOCK1
/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *
 *	This applies only if winsock1 is used.
 */
void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(TheStack);
		
		FREE(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
		UpdateIPAddresses(UpnpIPAddressList, UpnpIPAddressLength);
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
}
#endif

#ifdef _WINSOCK2
/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *
 *	This applies only if winsock2 is used.
 */
void CALLBACK UpnpIPAddressMonitor
	(
	IN DWORD dwError, 
	IN DWORD cbTransferred, 
	IN LPWSAOVERLAPPED lpOverlapped, 
	IN DWORD dwFlags 
	)
{
	UpnpIPAddressListChanged(UpnpStack);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
}
#endif

unsigned long WINAPI UPnPMain(void* ptr)
{
	int i;
	char guid[20];
	char friendlyname[100];
	WSADATA wsaData;

	MmsOnStatsChanged = &MmsOnStatsChangedSink;
	MmsOnTransfersChanged = &MmsMediaTransferStatsSink;

	srand((int)GetTickCount());
	for (i=0;i<19;i++)
	{
		guid[i] = (rand() % 25) + 66;
	}
	guid[19] = 0;

	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {exit(1);}
	memcpy(friendlyname,"Intel Micro AV Server (",23);
	gethostname(friendlyname+23,68);
	memcpy(friendlyname+strlen(friendlyname),")/Win32\0",8);

	TheChain = ILibCreateChain();
	TheStack = UpnpCreateMicroStack(TheChain, friendlyname, guid, "0000001", 1800, 0);
	InitMms(TheChain, TheStack, ".\\");

	/*
	 *	Set up the app to periodically monitor the available list
	 *	of IP addresses.
	 */
	#ifdef _WINSOCK1
	UpnpMonitor = ILibCreateLifeTime(TheChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	#endif
	#ifdef _WINSOCK2
	UpnpMonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
	#endif
	UpdateIPAddresses(UpnpIPAddressList, UpnpIPAddressLength);

	ILibStartChain(TheChain);
	FREE(UpnpIPAddressList);
	StopMms();
	WSACleanup();

	SendMessage(hWndMainWindow, WM_DESTROY, 0, 0);
	return 0;
}
