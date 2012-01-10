// SampleProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SampleProject.h"
#include "SampleProjectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "winsock.h"

extern "C"
{	
	#include "ILibParsers.h"
	#include "UPnPControlPointStructs.h"
	#include "UPnPControlPoint.h"
}



void *UPnP_CP;
void *UPnP_CP_chain;

//{{{BEGIN_IPADDRESS_MONITOR}}}
void *UPnP_CP_Monitor;
int *UPnP_CP_IPAddressList;
int UPnP_CP_IPAddressListLength;
//{{{END_IPADDRESS_MONITOR}}}


DWORD WINAPI Run(LPVOID args);

CSampleProjectDlg *that;
#define WM_USER_UPDATE (WM_USER + 100)

//{{{BEGIN_IPADDRESS_MONITOR}}}
void UPnP_CP_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UPnP_CP_IPAddressListLength || memcmp((void*)list,(void*)UPnP_CP_IPAddressList,sizeof(int)*length)!=0)
	{
		UPnP_CP_IPAddressListChanged(UPnP_CP);
		
		free(UPnP_CP_IPAddressList);
		UPnP_CP_IPAddressList = list;
		UPnP_CP_IPAddressListLength = length;
	}
	else
	{
		free(list);
	}
	
	ILibLifeTime_Add(UPnP_CP_Monitor,NULL,4,&UPnP_CP_IPAddressMonitor,NULL);
}
//{{{END_IPADDRESS_MONITOR}}}



/////////////////////////////////////////////////////////////////////////////
// CSampleProjectDlg dialog

CSampleProjectDlg::CSampleProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSampleProjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSampleProjectDlg)
	m_Text = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSampleProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSampleProjectDlg)
	DDX_Text(pDX, IDC_TEXT, m_Text);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSampleProjectDlg, CDialog)
	//{{AFX_MSG_MAP(CSampleProjectDlg)
	ON_COMMAND(ID_FILE_QUIT, OnFileQuit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER_UPDATE, OnUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSampleProjectDlg message handlers

BOOL CSampleProjectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	that = this;
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// TODO: Add extra initialization here
	SHMENUBARINFO info;
	info.cbSize = sizeof(info);
	info.hwndParent = m_hWnd;
	info.dwFlags = 0;
	info.nToolBarId = IDR_MENUBAR;
	info.hInstRes = ::AfxGetInstanceHandle();
	info.nBmpId = 0;
	info.cBmpImages = 0;
	SHCreateMenuBar(&info);

	// Create the Microstack thread
	CreateThread(NULL,0,&Run,this,0,NULL);


	return TRUE;  // return TRUE  unless you set the focus to a control
}
			


void CSampleProjectDlg::OnFileQuit() 
{
	ILibStopChain(UPnP_CP_chain);
}

LRESULT CSampleProjectDlg::OnUpdate(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return(0);
}



//{{{InvokeSink}}}

//{{{EventSink}}}



/* Called whenever a new device on the correct type is discovered */
void UPnPDeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;
	CString display;
	wchar_t *FriendlyName=NULL;
	int FriendlyNameLength;
	
	FriendlyNameLength = MultiByteToWideChar(CP_UTF8,0,device->FriendlyName,-1,FriendlyName,0);
	FriendlyName = (wchar_t*)malloc(sizeof(wchar_t)*FriendlyNameLength);
	MultiByteToWideChar(CP_UTF8,0,device->FriendlyName,-1,FriendlyName,FriendlyNameLength);
	
	display.Format(_T("UPnP Device Added: %s\r\n"), FriendlyName);
	that->m_Text += display;
	that->SendMessage(WM_USER_UPDATE);

	free(FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	UPnPPrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			UPnPSubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The UPnPGetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: UPnPHasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	//{{{SampleInvoke}}}
}

/* Called whenever a discovered device was removed from the network */
void UPnPDeviceRemoveSink(struct UPnPDevice *device)
{
	CString display;
	wchar_t *FriendlyName=NULL;
	int FriendlyNameLength;
	
	FriendlyNameLength = MultiByteToWideChar(CP_UTF8,0,device->FriendlyName,-1,FriendlyName,0);
	FriendlyName = (wchar_t*)malloc(sizeof(wchar_t)*FriendlyNameLength);
	MultiByteToWideChar(CP_UTF8,0,device->FriendlyName,-1,FriendlyName,FriendlyNameLength);

	display.Format(_T("UPnP Device Removed: %s\r\n"), FriendlyName);
	that->m_Text += display;
	that->SendMessage(WM_USER_UPDATE);

	free(FriendlyName);
}


DWORD WINAPI Run(LPVOID args)
{
	DWORD ptid=0;


	UPnP_CP_chain = ILibCreateChain();
	UPnP_CP = UPnPCreateControlPoint(UPnP_CP_chain,&UPnPDeviceDiscoverSink,&UPnPDeviceRemoveSink);

	//{{{BEGIN_IPADDRESS_MONITOR}}}
	UPnP_CP_Monitor = ILibCreateLifeTime(UPnP_CP_chain);
	ILibLifeTime_Add(UPnP_CP_Monitor,NULL,4,&UPnP_CP_IPAddressMonitor,NULL);
	//{{{END_IPADDRESS_MONITOR}}}

	//{{{EventRegistrations}}}

	that->m_Text.Format(_T("Intel Control Point Microstack 1.0\r\n"));
	that->SendMessage(WM_USER_UPDATE);


	ILibStartChain(UPnP_CP_chain);

	//{{{BEGIN_IPADDRESS_MONITOR}}}
	free(UPnP_CP_IPAddressList);
	//{{{END_IPADDRESS_MONITOR}}}

	((CSampleProjectDlg*)args)->SendMessage(WM_CLOSE);

	return 0;
}



