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


// MicroAVRendererDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MicroMediaRendererApp.h"
#include "MicroMediaRendererDlg.h"
#include <errno.h>
#include <winsock.h>

extern "C"
{
	#include "ILibParsers.h"
	#include "MediaPlayerVersions_Methods.h"
	#include "UpnpMicroStack.h"
}

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
		UpnpIPAddressListChanged(The_UpnpStack);
		
		FREE(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
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

unsigned long WINAPI RendererThreadStart(void* ptr)
{
	char guid[20];
	char friendlyname[100];
	WSADATA wsaData;

	CoInitialize(NULL);

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
#ifdef STATIC_UDN
	guid[0]  = 'M';
	guid[1]  = 'I';
	guid[2]  = 'C';
	guid[3]  = 'R';
	guid[4]  = 'O';
	guid[5]  = 'R';
	guid[6]  = 'E';
	guid[7]  = 'N';
	guid[8]  = 'D';
	guid[9]  = 'E';
	guid[10] = 'R';
	guid[11] = 'E';
	guid[12] = 'R';
	guid[13] = '1';
	guid[14] = '2';
	guid[15] = '3';
	guid[16] = '4';
	guid[17] = '6';
	guid[18] = '7';
#endif

	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {exit(1);}
	memcpy(friendlyname,"Intel Micro AV Renderer (",25);
	gethostname(friendlyname+25,66);
	memcpy(friendlyname+strlen(friendlyname),")/Win32\0",8);

	// Create and start the UPnP AV Renderer
	CMicroAVRendererDlg* dialog = (CMicroAVRendererDlg*)ptr;

	Init_TheRendererVariables(ILibCreateChain(), friendlyname, guid, "00001");

	Sleep(2000);

	/*
	 *	Set up the app to periodically monitor the available list
	 *	of IP addresses.
	 */
	#ifdef _WINSOCK1
	UpnpMonitor = ILibCreateLifeTime(The_RendererChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	#endif
	#ifdef _WINSOCK2
	UpnpMonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
	#endif

	ILibStartChain(The_RendererChain);

	The_RendererChain = NULL;
	
	#ifdef _WINSOCK1
	free(UpnpIPAddressList);
	#endif

	Uninit_TheRendererVariables();
	WSACleanup();

	//((CMicroAVRendererDlg*)ptr)->DestroyWindow();

	CoUninitialize();

	PostMessage(dialog->GetSafeHwnd(), WM_CLOSE, NULL, NULL);

	return 0;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMicroAVRendererDlg dialog

CMicroAVRendererDlg::CMicroAVRendererDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMicroAVRendererDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_RENDERER);
}

void CMicroAVRendererDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIAPLAYER1, MediaPlayer);
}

BEGIN_MESSAGE_MAP(CMicroAVRendererDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE( WM_MPINVOKE , OnMPINVOKE )
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_CONTROL_PLAY, OnControlPlay)
	ON_COMMAND(ID_CONTROL_STOP, OnControlStop)
	ON_COMMAND(ID_CONTROL_PAUSE, OnControlPause)
END_MESSAGE_MAP()


// CMicroAVRendererDlg message handlers

BOOL CMicroAVRendererDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Extra initialization here
	Init_TheMediaPlayer(this->GetSafeHwnd(), &MediaPlayer);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMicroAVRendererDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMicroAVRendererDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMicroAVRendererDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMicroAVRendererDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	if (this->IsWindowVisible())
	{
		CRect rect;
		GetClientRect(&rect);
		MediaPlayer.MoveWindow(&rect,false);
	}
}

int CMicroAVRendererDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateThread(NULL,0,&RendererThreadStart,this,0,NULL);

	return 0;
}

void CMicroAVRendererDlg::OnClose()
{
	if (The_RendererChain != NULL)
	{
		ILibStopChain(The_RendererChain);
	}
	else
	{
		CDialog::OnClose();
	}
}

BEGIN_EVENTSINK_MAP(CMicroAVRendererDlg, CDialog)
	ON_EVENT(CMicroAVRendererDlg, IDC_MEDIAPLAYER1, 2, PositionChangeMediaplayer1, VTS_R8 VTS_R8)
	ON_EVENT(CMicroAVRendererDlg, IDC_MEDIAPLAYER1, 3012, PlayStateChangeMediaplayer1, VTS_I4 VTS_I4)
	ON_EVENT(CMicroAVRendererDlg, IDC_MEDIAPLAYER1, 3010, ErrorMediaplayer1, VTS_NONE)
	ON_EVENT(CMicroAVRendererDlg, IDC_MEDIAPLAYER1, 3011, PlayStateChangeMediaplayer1, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CMicroAVRendererDlg::OnFileExit()
{
	this->DestroyWindow();
}


void CMicroAVRendererDlg::OnControlPlay()
{
	PM_Play();
}

void CMicroAVRendererDlg::OnControlStop()
{
	PM_Stop();
}

void CMicroAVRendererDlg::OnControlPause()
{
	PM_Pause();
}

LRESULT CMicroAVRendererDlg::OnMPINVOKE(WPARAM wp, LPARAM lp)
{
	return MainSwitch(wp, lp);
}

void CMicroAVRendererDlg::ErrorMediaplayer1()
{
	HandleMediaPlayer_OnError();
}

void CMicroAVRendererDlg::PlayStateChangeMediaplayer1(long OldState, long NewState)
{
	PM_OnStateChange(NewState);
}

void CMicroAVRendererDlg::PositionChangeMediaplayer1(double oldPosition, double newPosition)
{
	//MRSetMediaPosition(1,1,(int)(newPosition/100));
	/* position is not evented, so no need to do anything here */
}

