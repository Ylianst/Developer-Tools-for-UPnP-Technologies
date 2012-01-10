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

#include "CMicroMediaRenderer.h"

extern "C"
{
	#include "ILibParsers.h"
	#include "MediaPlayerVersions_Methods.h"
}


HBITMAP DisplayBitmap = NULL;

unsigned long WINAPI RendererThreadStart(void* ptr)
{
	char guid[20];
	char friendlyname[100];
	WSADATA wsaData;

	// Set all of the renderer callbacks
	MROnVolumeChangeRequest			= &SinkRequest_VolumeChange;
	MROnMuteChangeRequest			= &SinkRequest_MuteChange;
	MROnMediaChangeRequest			= &SinkRequest_MediaUriChange;
	MROnGetPositionRequest			= &SinkQuery_CurrentPosition;
	MROnSeekRequest					= &SinkRequest_Seek;
	MROnNextPreviousRequest			= &SinkRequest_NextPrevious;
	MROnStateChangeRequest			= &SinkRequest_StateChange;
	MROnPlayModeChangeRequest		= &SinkRequest_PlayModeChange;

	// Randomized guid generation
	srand(GetTickCount());
	for (int i=0;i<19;i++)
	{
		guid[i] = (rand() % 25) + 66;
	}
	guid[19] = 0;

	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {exit(1);}
	memcpy(friendlyname,"Intel Micro Renderer (",22);
	gethostname(friendlyname+22,70);
	memcpy(friendlyname+strlen(friendlyname),")\0",2);

	Init_TheRendererVariables(ILibCreateChain(), friendlyname, guid, "00001");

	ILibStartChain(The_RendererChain);

	The_RendererChain = NULL;

	Uninit_TheRendererVariables();
	WSACleanup();

	((CMicroMediaRenderer*)ptr)->DestroyWindow();

	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CMicroMediaRenderer

// Constructor:

CMicroMediaRenderer::CMicroMediaRenderer() :
m_szCurrentFile(NULL),
m_nFilterIndex(0),
m_dwAdviseCookie(0)
{
}

// Destructor:

CMicroMediaRenderer::~CMicroMediaRenderer()
{
	delete[] m_szCurrentFile;
	m_szCurrentFile = NULL;
}


// OnCreate: Set up the main window.

LRESULT CMicroMediaRenderer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	AtlAxWinInit();

	CComPtr<IConnectionPointContainer> spConnectionContainer;
	HRESULT		hr = S_OK;
	TCHAR*		szPlayerGUID;						// The Windows Media Player Control GUID
	RECT		rcPlayer = { 0, 254, 240, 268 };	// The initial position of the Player window
	HINSTANCE	hInst = _Module.GetModuleInstance();

	DisplayBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_DISPLAYBITMAP));
	
	// Create the host window for the Windows Media Player control.
	StringFromCLSID(__uuidof(WMP), &szPlayerGUID);
	m_wndView.Create(m_hWnd, rcPlayer, szPlayerGUID, WS_CHILD | WS_VISIBLE, NULL, IDC_PLAYER);
	CoTaskMemFree(szPlayerGUID);

	// Confirm that the host window was created.
	if (NULL == m_wndView.m_hWnd)
	{
		hr = E_FAIL;
	}

	// Retrieve a pointer to the Windows Media Player control interface.
	if (SUCCEEDED(hr))
	{
		hr = m_wndView.QueryControl(&m_spWMPPlayer);
	}

	// Start listening to events.

	if (SUCCEEDED(hr))
	{
		hr = m_spWMPPlayer->QueryInterface( IID_IConnectionPointContainer, (void**)&spConnectionContainer );
	}
	if (SUCCEEDED(hr))
	{
		hr = spConnectionContainer->FindConnectionPoint( __uuidof(_IWMPEvents), &m_spConnectionPoint );
	}
	if (SUCCEEDED(hr))
	{
		hr = m_spConnectionPoint->Advise( (IDispatch*)this, &m_dwAdviseCookie );
	}
	else
	{
		::PostQuitMessage(0);
		return 0;
	}

	Init_TheMediaPlayer(m_hWnd, &(m_spWMPPlayer.p));

	::MoveWindow(GetDlgItem(IDC_PLAYER), 0, 196, 240, 72, FALSE);

	// Create the menu bar.
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize		= sizeof(SHMENUBARINFO);
	mbi.hwndParent	= m_hWnd;
	mbi.nToolBarId	= IDM_MENU;
	mbi.hInstRes	= hInst;
	mbi.nBmpId		= 0;
	mbi.cBmpImages	= 0;
	SHCreateMenuBar(&mbi);

	CreateThread(NULL,0,&RendererThreadStart,this,0,NULL);

	return 0;
}

// OnDestroy: Clean up.

LRESULT CMicroMediaRenderer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Stop listening to events.
	if (m_spConnectionPoint)
	{
		if (0 != m_dwAdviseCookie)
			m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
		m_spConnectionPoint.Release();
	}

	DeleteObject(DisplayBitmap);
	DisplayBitmap = NULL;

	//bHandled = FALSE;
	PostQuitMessage(0);
	return 0;
}

// OnOK: Makes the program inactive and hides the program window.

LRESULT CMicroMediaRenderer::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::SendMessage(m_hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)m_hWnd);
	::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
	return 0;
}

// OnFileOpen: Displays a dialog box that lets the user choose a file to play.

LRESULT CMicroMediaRenderer::OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPENFILENAME ofn;		// Holds information used with the Open dialog box

	// Initialize the OPENFILENAME struct.
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= m_hWnd;
	ofn.lpstrFilter		= TEXT("Audio Files\0*.WMA\0Video Files\0*.WMV\0All Files\0*.*\0\0");
	ofn.nFilterIndex	= m_nFilterIndex;
	ofn.lpstrFile		= new TCHAR[ _MAX_PATH ];
	ofn.lpstrFile[0]	= TEXT('\0');
	ofn.nMaxFile		= _MAX_PATH;

	// Confirm that the allocation of ofn.lpstrFile succeeded.
	if (NULL == ofn.lpstrFile)
	{
		return 0;
	}

	// Retrieve the file selection from the Open dialog box.
	if (TRUE == GetOpenFileName(&ofn))
	{
		// Store the filter index.
		m_nFilterIndex = ofn.nFilterIndex;

		// Play the file
		if (FAILED(m_spWMPPlayer->put_FileName(ofn.lpstrFile))) return FALSE;
		if (FAILED(m_spWMPPlayer->Play())) return FALSE;
	}
	delete[] ofn.lpstrFile;
	ofn.lpstrFile = NULL;
	return 0;
}

// OnFileExit: Exits the program.

LRESULT CMicroMediaRenderer::OnFileExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ILibStopChain(The_RendererChain);
	return 0;
}


// Invoke: redirects Windows Media Player control events to the appropriate event handler.

HRESULT CMicroMediaRenderer::Invoke(	DISPID				dispIdMember,	  
									REFIID				riid,			  
									LCID				lcid,				
									WORD				wFlags,			  
									DISPPARAMS FAR		*pDispParams,  
									VARIANT FAR			*pVarResult,  
									EXCEPINFO FAR		*pExcepInfo,  
									unsigned int FAR	*puArgErr )
{
	if (!pDispParams)
	{
		return E_POINTER;
	}

	if (pDispParams->cNamedArgs != 0)
	{
		return DISP_E_NONAMEDARGS;
	}

	HRESULT hr = S_OK;

	switch (dispIdMember)
	{
		case IWMPEVENTS_DISPID_PLAYSTATECHANGE: OnPlayStateChange(pDispParams->rgvarg[0].lVal);
			break;
		default:
			hr = DISP_E_MEMBERNOTFOUND;
	}

	return( hr );
}


LRESULT CMicroMediaRenderer::OnPaint(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
	RECT rect;
	HDC memDC;
	PAINTSTRUCT paintstruct;
	HGDIOBJ gdiobj;

	HDC dc = BeginPaint(&paintstruct);
	rect.top = 0;
	rect.bottom = 196; // 30
	rect.left = 0;
	rect.right = 240;
	FillRect(dc,&rect,GetSysColorBrush(COLOR_INFOTEXT)); 
	SetBkColor(dc,0);
	SetTextColor(dc,RGB(255,255,255)); 

	rect.top = 1;
	rect.bottom = 15;
	rect.left = 2;
	rect.right = 238;
	DrawText(dc,TEXT("UPnP AV 1.0 Compatible Media Renderer"),-1,&rect,0);

	rect.top = 15;
	rect.bottom = 29;
	DrawText(dc,TEXT("Intel Microstack Technology"),-1,&rect,0);

    memDC = CreateCompatibleDC(dc);
    gdiobj = SelectObject(memDC, DisplayBitmap);
    BitBlt(dc, 30, 54, 181, 109, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, gdiobj);

	EndPaint(&paintstruct);

	return 0;
}


void CMicroMediaRenderer::OnPlayStateChange(long NewState)
{
	PM_OnStateChange(NewState);
}


LRESULT CMicroMediaRenderer::OnMPINVOKE(UINT uMsg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
	return MainSwitch(wp, lp);
}


