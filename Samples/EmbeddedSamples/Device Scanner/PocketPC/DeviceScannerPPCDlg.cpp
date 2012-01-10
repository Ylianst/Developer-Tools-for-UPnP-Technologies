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
#include "DeviceScannerPPC.h"
#include "DeviceScannerPPCDlg.h"

extern "C"
{
	#include "ILibParsers.h"
	#include "UPnPControlPoint.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HWND	hWndMainWindow = NULL;
void TestCallback(struct UPnPDevice *device);
CDeviceScannerPPCDlg* MainDialog = NULL;
HTREEITEM MainRootNode = NULL;
void *UPnP_CP;
void *UPnP_CP_chain;

DWORD WINAPI UPnPStackThreadRun(LPVOID args);
void UPnPDeviceDiscoverSink(struct UPnPDevice *device);
void UPnPDeviceRemoveSink(struct UPnPDevice *device);


/////////////////////////////////////////////////////////////////////////////
// CDeviceScannerPPCDlg dialog

CDeviceScannerPPCDlg::CDeviceScannerPPCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceScannerPPCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeviceScannerPPCDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeviceScannerPPCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceScannerPPCDlg)
	DDX_Control(pDX, IDC_DEVICETREE, m_DeviceTree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDeviceScannerPPCDlg, CDialog)
	//{{AFX_MSG_MAP(CDeviceScannerPPCDlg)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeviceScannerPPCDlg message handlers

BOOL CDeviceScannerPPCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// Add extra initialization here
	MainDialog = this;
	CCeCommandBar *pCommandBar = (CCeCommandBar*)m_pWndEmptyCB;
	pCommandBar->InsertMenuBar(IDR_MAINMENU);
	
	this->MoveWindow(0,0,240,320,false);

	CImageList* TreeImageList = new CImageList();
	TreeImageList->Create(16,16,1,1,1);

	CBitmap b1;
	CBitmap b2;
	CBitmap b3;
	CBitmap b4;
	CBitmap b5;
	CBitmap b6;
	CBitmap b7;
	CBitmap b8;

	b1.LoadBitmap(IDB_COMPUTERBITMAP);
	b2.LoadBitmap(IDB_DEVICEBITMAP);
	b3.LoadBitmap(IDB_SERVICE1BITMAP);
	b4.LoadBitmap(IDB_SERVICE2BITMAP);
	b5.LoadBitmap(IDB_FOLDER1BITMAP);
	b6.LoadBitmap(IDB_FOLDER2BITMAP);
	b7.LoadBitmap(IDB_METHODBITMAP);
	b8.LoadBitmap(IDB_VARIABLEBITMAP);

	TreeImageList->Add(&b1,RGB(0,0,0));
	TreeImageList->Add(&b2,RGB(0,0,0));
	TreeImageList->Add(&b3,RGB(0,0,0));
	TreeImageList->Add(&b4,RGB(0,0,0));
	TreeImageList->Add(&b5,RGB(0,0,0));
	TreeImageList->Add(&b6,RGB(0,0,0));
	TreeImageList->Add(&b7,RGB(0,0,0));
	TreeImageList->Add(&b8,RGB(0,0,0));
	
	b1.DeleteObject();
	b2.DeleteObject();
	b3.DeleteObject();
	b4.DeleteObject();
	b5.DeleteObject();
	b6.DeleteObject();
	b7.DeleteObject();
	b8.DeleteObject();

	m_DeviceTree.SetImageList(TreeImageList,TVSIL_NORMAL);
	MainRootNode = m_DeviceTree.InsertItem(TEXT("UPnP Devices"),0,0,TVI_ROOT,TVI_LAST);
	//m_DeviceTree.InsertItem(TEXT("Device"),1,1,TreeRootNode,TVI_LAST);

	UPnP_CP_chain = ILibCreateChain();
	UPnP_CP = UPnPCreateControlPoint(UPnP_CP_chain,&UPnPDeviceDiscoverSink,&UPnPDeviceRemoveSink);
	CreateThread(NULL,0,&UPnPStackThreadRun,NULL,0,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

DWORD WINAPI UPnPStackThreadRun(LPVOID args)
{
	ILibStartChain(UPnP_CP_chain);
	SendMessage(hWndMainWindow, WM_DESTROY, 0, 0);
	return 0;
}

void CDeviceScannerPPCDlg::OnFileExit()
{
	this->DestroyWindow();
}

void CDeviceScannerPPCDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_DeviceTree.GetSafeHwnd() != 0)
	{
		//RECT clientRect;
		//this->GetClientRect(&clientRect);
		//m_DeviceTree.MoveWindow(&clientRect,true);
		m_DeviceTree.MoveWindow(0,27,240,267,true);
	}
}

void TreeAddDevice(HTREEITEM root,struct UPnPDevice *device)
{
	if (MainDialog == NULL || MainDialog->m_DeviceTree.GetSafeHwnd() == NULL) return;
	
	unsigned short* str = (unsigned short*)malloc(512);
	
	//Invoke_SwitchPower_SetTarget(GetService_SwitchPower(device), NULL, NULL, 1);

	size_t s = mbstowcs((unsigned short*)str,device->FriendlyName,strlen(device->FriendlyName)+1);
	HTREEITEM devicenode = MainDialog->m_DeviceTree.InsertItem((unsigned short*)str,1,1,root,TVI_LAST);
	device->Tag = devicenode;

	UPnPDevice* embeddeddevice = device->EmbeddedDevices;
	while (embeddeddevice != NULL)
	{
		TreeAddDevice(devicenode,embeddeddevice);
		embeddeddevice = embeddeddevice->Next;
	}

	UPnPService* service = device->Services;
	while (service != NULL)
	{
		char* servicename = service->ServiceType;
		if (strcmp(service->ServiceType,"urn:schemas-upnp-org:service:") != 0)
		{
			servicename = service->ServiceType + 29;
		}
		mbstowcs((unsigned short*)str,servicename,strlen(servicename)+1);
		HTREEITEM servicenode = MainDialog->m_DeviceTree.InsertItem((unsigned short*)str,2,2,devicenode,TVI_LAST);
		service = service->Next;
	}

	free(str);
}

void TreeRemoveDevice(HTREEITEM root,struct UPnPDevice *device)
{
	if (MainDialog == NULL || MainDialog->m_DeviceTree.GetSafeHwnd() == NULL) return;

	if (device->Tag == NULL) return;
	MainDialog->m_DeviceTree.DeleteItem((HTREEITEM)device->Tag);
}

void UPnPDeviceDiscoverSink(struct UPnPDevice *device)
{
	TreeAddDevice(MainRootNode,device);
}

void UPnPDeviceRemoveSink(struct UPnPDevice *device)
{
	TreeRemoveDevice(MainRootNode,device);
}
