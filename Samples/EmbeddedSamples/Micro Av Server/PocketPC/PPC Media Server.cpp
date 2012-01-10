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
#include "PPC Media Server.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <Winuser.h>

// Added by CEL
extern "C"
{
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"

	// CDS Headers
	#include "MimeTypes.h"
	#include "mystring.h"
	#include "MicroMediaServer.h"
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;				// The current instance
HWND				g_hwndCB;				// The command bar handle
int					g_DownloadStatsMapping[20];

// Added by CEL
static HWND	hWndMainWindow = NULL;
unsigned long UPnPMain(void* ptr);
void *TheChain;
void *TheStack;

struct MMSMEDIATRANSFERSTAT MmsMediaTransferStats[20];

static SHACTIVATEINFO s_sai;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);

HICON HICON_MEDIASERVER  = NULL;
HICON HICON_MEDIASERVER2 = NULL;
HICON HICON_RIGHTARROW   = NULL;
HICON HICON_LEFTARROW    = NULL;

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

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_PPCMEDIASERVER);

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
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PPCMEDIASERVER));
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
	LoadString(hInstance, IDC_PPCMEDIASERVER, szWindowClass, MAX_LOADSTRING);
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
		if (g_hwndCB) MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, FALSE);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// This method draws a single active transfer to the screen.
// pos - a number from 0 to 4, the position on the screen that
// is used to draw the information.
// index - the transfer index to draw to the screen.
void DrawTransferInfo(HDC hdc, int pos, int index)
{
	int i,j;
	unsigned short* str[100];
	RECT r;
	if (index == -1 || MmsMediaTransferStats[index].filename == NULL)
	{
		r.left = 6;
		r.right = 229;
		r.top = 98+(pos*32);
		r.bottom = (98+32)+(pos*32);
		FillRect(hdc,&r,GetSysColorBrush(COLOR_MENU));
	}
	else
	{
		SetBkColor(hdc,GetSysColor(COLOR_MENU));
		DrawIconEx(hdc,6,106+(pos*32),HICON_LEFTARROW,16,16,0,NULL,DI_IMAGE | DI_MASK);
		r.left = 24;
		r.right = 229;
		r.top = 98+(pos*32);
		r.bottom = 120+(pos*32);

		j = 0;
		for (i=strlen(MmsMediaTransferStats[index].filename);i>=0;i--)
		{
			if (MmsMediaTransferStats[index].filename[i] == '\\' || MmsMediaTransferStats[index].filename[i] == '/')
			{
				j = i+1;
				break;
			}
		}

		mbstowcs((unsigned short*)str,MmsMediaTransferStats[index].filename + j,100);
		DrawText(hdc,(unsigned short*)str,-1,&r,0);
		r.left = 24;
		r.right = 229;
		r.top = 114+(pos*32);
		r.bottom = 138+(pos*32);

		if (MmsMediaTransferStats[index].length > 0)
		{
			wsprintf((unsigned short*)str,TEXT("%d of %d (%d%%)          "),MmsMediaTransferStats[index].position,MmsMediaTransferStats[index].length,(MmsMediaTransferStats[index].position*100)/MmsMediaTransferStats[index].length);
			DrawText(hdc,(unsigned short*)str,-1,&r,0);
		}
		else
		{
			wsprintf((unsigned short*)str,TEXT("Position: %d          "),MmsMediaTransferStats[index].position);
			DrawText(hdc,(unsigned short*)str,-1,&r,0);
		}
	}
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
	HDC hdc;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	unsigned short* str[100];
	RECT rt;
	int i;

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
					SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case ID_TOOLS_EXIT:
					ILibStopChain(TheChain);
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CREATE:
			g_hwndCB = CreateRpCommandBar(hWnd); // Initialize the shell activate info structure

			// Clear up all of the active transfers.
			for (i=0;i<10;i++) {g_DownloadStatsMapping[i] = -1;}

			// Load all of the icon resources we will need to draw the UI.
			HICON_MEDIASERVER  = LoadIcon(g_hInst, (LPCTSTR)IDI_MEDIASERVER);
			HICON_MEDIASERVER2 = LoadIcon(g_hInst, (LPCTSTR)IDI_MEDIASERVER2);
			HICON_RIGHTARROW   = LoadIcon(g_hInst, (LPCTSTR)IDI_RIGHTARROW);
			HICON_LEFTARROW    = LoadIcon(g_hInst, (LPCTSTR)IDI_LEFTARROW);

			// Create the Micro Media Server thread. This thread will exist for
			// the duration of the application and will be owned by the media
			// server library.
			CreateThread(NULL, 0, &UPnPMain, 0, 0, NULL ); 

            memset (&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);

			break;
		case WM_PAINT:
		
			// Paint the complete screen here. Quite workout to get the UI looking
			// event half decent using Win32 directly.
			GetClientRect(hWndMainWindow, &rt);
			hdc = BeginPaint(hWnd, &ps);
			
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
				wsprintf((unsigned short*)str,TEXT("%d File Transfers        "),MmsCurrentTransfersCount);
				DrawText(hdc,(unsigned short*)str,-1,&r,0);
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
			wsprintf((unsigned short*)str,TEXT("%d"),MmsBrowseCount);
			r.left = 180;
			r.right = rt.right-5;
			r.top = 50;
			r.bottom = 70;
			DrawText(hdc,(unsigned short*)str,-1,&r,DT_RIGHT);
			wsprintf((unsigned short*)str,TEXT("%d"),MmsHttpRequestCount);
			r.left = 180;
			r.right = rt.right-5;
			r.top = 70;
			r.bottom = 90;
			DrawText(hdc,(unsigned short*)str,-1,&r,DT_RIGHT);

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

		case WM_CLOSE:
			ILibStopChain(TheChain);
			break;
		case WM_DESTROY:
			DestroyIcon(HICON_MEDIASERVER);
			DestroyIcon(HICON_MEDIASERVER2);
			DestroyIcon(HICON_RIGHTARROW);
			DestroyIcon(HICON_LEFTARROW);
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

// This method is used to correctly setup the g_DownloadStatsMapping table
// That table maps one of the active transfers to one of the five available
// display position on the screen. Regardless of which transfers are active
// in the Media Server's MmsMediaTransferStats table, we want to display 5
// of them on the screen starting at position 0 (top) going down.
// changeindex - If one of the active transfers had its information changes
// if was removed, this parameter will force the method to redraw it on the
// screen.
void AdjustDisplayMapping(HDC hdc,int changeindex)
{
	int i,j;
	j = 0;
	for (i=0;i<20;i++)
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
	for (i=j;i<20;i++)
	{
		if (g_DownloadStatsMapping[i] != -1)
		{
			g_DownloadStatsMapping[i] = -1;
			if (i < 5) DrawTransferInfo(hdc,i,-1);
		}
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

// Micro Media Server callback handler.
// Called when the global statictics of the Micro Media Server
// changed. For example, the number of browse requests has been
// updated. As a result, we will paint the new value on the screen.
void MmsOnStatsChangedSink()
{
	TCHAR* str[100];
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
			DrawText(hdc,TEXT("No File Transfers          "),-1,&r,0);
		}
		if (MmsCurrentTransfersCount == 1)
		{
			DrawText(hdc,TEXT("1 File Transfer          "),-1,&r,0);
		}
		if (MmsCurrentTransfersCount > 1)
		{
			wsprintf((unsigned short*)str,TEXT("%d File Transfers          "),MmsCurrentTransfersCount);
			DrawText(hdc,(unsigned short*)str,-1,&r,0);
		}

		wsprintf((unsigned short*)str,TEXT("  %d"),MmsBrowseCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 50;
		r.bottom = 70;
		DrawText(hdc,(unsigned short*)str,-1,&r,DT_RIGHT);

		wsprintf((unsigned short*)str,TEXT("  %d"),MmsHttpRequestCount);
		r.left = 180;
		r.right = rt.right-5;
		r.top = 70;
		r.bottom = 90;
		DrawText(hdc,(unsigned short*)str,-1,&r,DT_RIGHT);

		ReleaseDC(hWndMainWindow,hdc);
	}
}

// Thread creation entry point. This thread will spend all of its
// time blocked within the InitMms() call. The Micro Media Server will
// own it and use it for network handling, etc. This thread will also
// come back when an event is triggered.
unsigned long UPnPMain(void* ptr)
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
	gethostname(friendlyname+23,66);
	memcpy(friendlyname+strlen(friendlyname),")\0",2);

	TheChain = ILibCreateChain();
	TheStack = UpnpCreateMicroStack(TheChain, friendlyname, guid,"0000001",1800,0);
	InitMms(TheChain, TheStack, "\\");
	ILibStartChain(TheChain);

	StopMms();
	SendMessage(hWndMainWindow, WM_DESTROY, 0, 0);

	WSACleanup();

	return 0;
}
