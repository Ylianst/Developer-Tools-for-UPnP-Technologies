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

// Changed to UPNP_ControlPoint.c
//
// In method "Run", replace
//   SSDP = CreateSSDPClientModule("urn:schemas-upnp-org:device:BinaryLight:1", 41, &SSDP_Sink);
// with
//   SSDP = CreateSSDPClientModule("upnp:rootdevice", 15, &SSDP_Sink);
//
// In method "SCPD_Sink":
//
//			CP->DiscoverSink(device);
			/*
			do
			{
				RetDevice = UPnPGetDevice1(device,i++);
				if(RetDevice!=NULL)
				{
					UPnPAddUDN(CP,RetDevice->UDN);
					UPnPAttachRootUDNToUDN(CP,RetDevice->UDN,RootUDN);
					if(CP->DiscoverSink!=NULL)
					{
						CP->DiscoverSink(RetDevice);
					}
				}
			}while(RetDevice!=NULL);
			*/
//


#include "stdafx.h"
#include "DeviceScanner.h"
#include "DeviceScannerDlg.h"

extern "C"
{
	#include "ILibParsers.h"
	#include "UPnPControlPoint.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static HWND	hWndMainWindow = NULL;
void TestCallback(struct UPnPDevice *device);
CDeviceScannerDlg* MainDialog = NULL;
HTREEITEM MainRootNode = NULL;
void *UPnP_CP;
void *UPnP_CP_chain;

DWORD WINAPI UPnPStackThreadRun(LPVOID args);
void UPnPDeviceDiscoverSink(struct UPnPDevice *device);
void UPnPDeviceRemoveSink(struct UPnPDevice *device);

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


// CDeviceScannerDlg dialog



CDeviceScannerDlg::CDeviceScannerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceScannerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SCANNERICON);
}

void CDeviceScannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICETREE, DeviceTree);
}

BEGIN_MESSAGE_MAP(CDeviceScannerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDeviceScannerDlg message handlers

BOOL CDeviceScannerDlg::OnInitDialog()
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

	// Add extra initialization here
	MainDialog = this;
	hWndMainWindow = this->GetSafeHwnd();

	CImageList* TreeImageList = new CImageList();
	TreeImageList->Create(16,16,ILC_COLOR16,8,8);

	CBitmap b1;
	b1.LoadBitmap(IDB_COMPUTERBITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_DEVICEBITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_SERVICE1BITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_SERVICE2BITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER1BITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER2BITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_METHODBITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_VARIABLEBITMAP);
	TreeImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	DeviceTree.SetImageList(TreeImageList,TVSIL_NORMAL);
	MainRootNode = DeviceTree.InsertItem("UPnP Devices",0,0,TVI_ROOT,TVI_LAST);

	UPnP_CP_chain = ILibCreateChain();
	UPnP_CP = UPnPCreateControlPoint(UPnP_CP_chain,&UPnPDeviceDiscoverSink,&UPnPDeviceRemoveSink);
	CreateThread(NULL,0,&UPnPStackThreadRun,NULL,0,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

DWORD WINAPI UPnPStackThreadRun(LPVOID args)
{
	ILibStartChain(UPnP_CP_chain);
	UPnP_CP_chain = NULL;
	MainDialog->SendMessage(WM_CLOSE,0,0);
	return 0;
}

void CDeviceScannerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDeviceScannerDlg::OnPaint() 
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
HCURSOR CDeviceScannerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDeviceScannerDlg::OnFileExit()
{
	this->DestroyWindow();
}

void CDeviceScannerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (DeviceTree.GetSafeHwnd() != 0)
	{
		RECT clientRect;
		this->GetClientRect(&clientRect);
		DeviceTree.MoveWindow(&clientRect,true);
	}
}

void TreeAddDevice(HTREEITEM root,struct UPnPDevice *device)
{
	//Invoke_SwitchPower_SetTarget(GetService_SwitchPower(device), NULL, NULL, 1);
	HTREEITEM devicenode = MainDialog->DeviceTree.InsertItem(device->FriendlyName,1,1,root,TVI_LAST);	
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
		HTREEITEM servicenode = MainDialog->DeviceTree.InsertItem(service->ServiceType,2,2,devicenode,TVI_LAST);
		service = service->Next;
	}
}

void TreeRemoveDevice(HTREEITEM root,struct UPnPDevice *device)
{
	if (MainDialog == NULL || MainDialog->DeviceTree.GetSafeHwnd() == NULL) return;

	if (device->Tag == NULL) return;
	MainDialog->DeviceTree.DeleteItem((HTREEITEM)device->Tag);
}

void UPnPDeviceDiscoverSink(struct UPnPDevice *device)
{
	TreeAddDevice(MainRootNode,device);
}

void UPnPDeviceRemoveSink(struct UPnPDevice *device)
{
	TreeRemoveDevice(MainRootNode,device);
}

void CDeviceScannerDlg::OnClose()
{
	if (UPnP_CP_chain != NULL)
	{
		ILibStopChain(UPnP_CP_chain);
	}
	else
	{
		CDialog::OnClose();
	}
}
