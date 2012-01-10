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
{	//{{{STANDARD_C_APP_BEGIN}}}
	#include "ILibParsers.h"
	#include "ILibWebServer.h"
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	#include "ILibParsers.h"
	//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	//{{{STANDARD_C++_APP_END}}}
	//{{{BEGIN_THREADPOOL}}}#include "ILibThreadPool.h"//{{{END_THREADPOOL}}}
	//{{{STANDARD_C_APP_BEGIN}}}
	//{{{MicroStack_Include}}}
	//{{{STANDARD_C_APP_END}}}
	//{{{BEGIN_BAREBONES}}}//{{{INCLUDES}}}//{{{END_BAREBONES}}}
}
//{{{STANDARD_C++_APP_BEGIN}}}
#include "UPnPAbstraction.h"
//{{{STANDARD_C++_APP_END}}}

//{{{STANDARD_C_APP_BEGIN}}}
void *MicroStackChain;
//{{{MICROSTACK_VARIABLE}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
CUPnP_Manager *pUPnP;
//{{{STANDARD_C++_APP_END}}}

//{{{BEGIN_THREADPOOL}}}
void *ILib_Pool;
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{GLOBALS}}}//{{{END_BAREBONES}}}

//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
void *ILib_Monitor;
int ILib_IPAddressLength;
int *ILib_IPAddressList;
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}


DWORD WINAPI Run(LPVOID args);

CSampleProjectDlg *that;
#define WM_USER_UPDATE (WM_USER + 100)


//{{{STANDARD_C++_APP_BEGIN}}}
//{{{CLASS_DEFINITIONS_DEVICE}}}
//{{{CLASS_IMPLEMENTATIONS_DEVICE}}}
//{{{STANDARD_C++_APP_END}}}


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
	this->m_Text.Format(_T("Intel MicroStack 1.0 - {{{INITSTRING}}}\r\n\r\n"));
	this->UpdateData(FALSE);
	CreateThread(NULL,0,&Run,this,0,NULL);


	return TRUE;  // return TRUE  unless you set the focus to a control
}
			


void CSampleProjectDlg::OnFileQuit() 
{
	//{{{STANDARD_C_APP_BEGIN}}}
	ILibStopChain(MicroStackChain);
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	if(pUPnP!=NULL)
	{
		pUPnP->Stop();
	}
	//{{{STANDARD_C++_APP_END}}}
}
//{{{STANDARD_C++_APP_BEGIN}}}
//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}
void {{{PREFIX}}}OnAddSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
{
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
}
void {{{PREFIX}}}OnRemoveSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
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
//{{{CP_DISCOVER/REMOVE_SINKS_END}}}
//{{{STANDARD_C++_APP_END}}}

LRESULT CSampleProjectDlg::OnUpdate(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return(0);
}

//{{{STANDARD_C_APP_BEGIN}}}
//{{{CP_EventSink}}}
//{{{CP_InvokeSink}}}

//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}
/* Called whenever a new device on the correct type is discovered */
void {{{PREFIX}}}DeviceDiscoverSink(struct UPnPDevice *device)
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
			{{{PREFIX}}}SubscribeForUPnPEvents(tempService,NULL);
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
	
	//{{{CP_SampleInvoke}}}
}

/* Called whenever a discovered device was removed from the network */
void {{{PREFIX}}}DeviceRemoveSink(struct UPnPDevice *device)
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
//{{{END_CP_DISCOVER_REMOVE_SINK}}}


//{{{DEVICE_INVOCATION_DISPATCH}}}
//{{{PresentationRequest}}}
//{{{STANDARD_C_APP_END}}}

//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
void ILib_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=ILib_IPAddressLength || memcmp((void*)list,(void*)ILib_IPAddressList,sizeof(int)*length)!=0)
	{
		//{{{STANDARD_C_APP_BEGIN}}}
		//{{{IPAddress_Changed}}}
		//{{{STANDARD_C_APP_END}}}
		//{{{STANDARD_C++_APP_BEGIN}}}
		pUPnP->IPAddressListChanged();
		//{{{STANDARD_C++_APP_END}}}

		free(ILib_IPAddressList);
		ILib_IPAddressList = list;
		ILib_IPAddressLength = length;
	}
	else
	{
		free(list);
	}
	
	
	ILibLifeTime_Add(ILib_Monitor,NULL,4,&ILib_IPAddressMonitor,NULL);
}
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}


//{{{BEGIN_THREADPOOL}}}
DWORD WINAPI ILibPoolThread(void *args)
{
	ILibThreadPool_AddThread(ILib_Pool);
	return(0);
}
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{METHODS}}}//{{{END_BAREBONES}}}

DWORD WINAPI Run(LPVOID args)
{
	DWORD ptid=0;
	//{{{BEGIN_THREADPOOL}}}int x;//{{{END_THREADPOOL}}}
	//{{{BEGIN_BAREBONES}}}//{{{MAIN_START}}}//{{{END_BAREBONES}}}

	//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP = new CUPnP_Manager();
	//{{{DERIVED_CLASS_INSERTION}}}
	//{{{CP_CONNECTING_ADD/REMOVE_SINKS}}}
	//{{{STANDARD_C++_APP_END}}}
	//{{{STANDARD_C_APP_BEGIN}}}
	MicroStackChain = ILibCreateChain();
	
	//{{{CreateControlPoint}}}
	/* TODO: Each device must have a unique device identifier (UDN) */
	//{{{CREATE_MICROSTACK}}}
	//{{{INVOCATION_FP}}}	
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	//{{{STATEVARIABLES_INITIAL_STATE}}}

	//{{{CP_EventRegistrations}}}
	//{{{STANDARD_C_APP_END}}}

	//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	//{{{STANDARD_C_APP_BEGIN}}}ILib_Monitor = ILibCreateLifeTime(MicroStackChain);//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}ILib_Monitor = ILibCreateLifeTime(pUPnP->GetChain());//{{{STANDARD_C++_APP_END}}}
	ILib_IPAddressLength = ILibGetLocalIPAddressList(&ILib_IPAddressList);
	ILibLifeTime_Add(ILib_Monitor,NULL,4,&ILib_IPAddressMonitor,NULL);
	//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}

	//{{{BEGIN_THREADPOOL}}}
	ILib_Pool = ILibThreadPool_Create();
	for(x=0;x<!NUMTHREADPOOLTHREADS!;++x)
	{
		CreateThread(NULL,0,&ILibPoolThread,NULL,0,&ptid);
	}
	//{{{END_THREADPOOL}}}
	//{{{BEGIN_BAREBONES}}}//{{{INITIALIZATIONS}}}//{{{END_BAREBONES}}}
	
	//{{{STANDARD_C_APP_BEGIN}}}
	ILibStartChain(MicroStackChain);
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->Start();
	delete pUPnP;
	pUPnP = NULL;
	//{{{STANDARD_C++_APP_END}}}


	//{{{BEGIN_THREADPOOL}}}
	if(ILib_Pool!=NULL)
	{
		CString msg,msg2;
		msg.Format(_T("Stopping Thread Pool...\r\n"));
		that->m_Text += msg;
		that->SendMessage(WM_USER_UPDATE);

		ILibThreadPool_Destroy(ILib_Pool);

		msg2.Format(_T("Thread Pool Destroyed...\r\n"));
		that->m_Text += msg2;
		that->SendMessage(WM_USER_UPDATE);
	}
	//{{{END_THREADPOOL}}}

	//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	free(ILib_IPAddressList);
	//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}

	((CSampleProjectDlg*)args)->SendMessage(WM_CLOSE);

	return 0;
}



