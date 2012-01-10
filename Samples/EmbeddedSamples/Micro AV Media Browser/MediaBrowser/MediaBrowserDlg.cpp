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
#include "MediaBrowser.h"
#include "MediaBrowserDlg.h"

extern "C"
{
	#include "MSCPControlPoint.h"
	#include "ILibParsers.h"
	#include "MmsCp.h"
	#include "MyString.h"
}


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NULL_FREE(x) free(x); x = NULL

#define MAX_STR_LEN 1024

DWORD WINAPI MediaControlPointStart(LPVOID args);
void DisplayDeviceList();

struct MMSCP_BrowseArgs *ArgsBrowseMetadata, *ArgsBrowseDirectChildren;
void* chain;
char *ZeroString = "0";
char *StarString = "*";
char *EmptyString = "";
const int MaxObjects = 100;
char *FilterString = "dc:title,dc:creator,upnp:class,res";
char *SortString = "";//+dc:creator,+dc:title";
bool ExitFlag = false;
CMediaBrowserDlg* MainDialog = NULL;
struct UPnPDevice* devices[32];
struct UPnPDevice *MediaListCurrentDevice = NULL;
char* MediaListCurrentContainer = NULL;
struct MMSCP_ResultsList *MediaResultList = NULL;

int *IPAddressList = NULL;
int IPAddressListLen = 0;

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
public:
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


// CMediaBrowserDlg dialog



CMediaBrowserDlg::CMediaBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaBrowserDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
}

void CMediaBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIALIST, MediaList);
	DDX_Control(pDX, IDC_COMBOMEDIAPATH, MediaCombo);
	DDX_Control(pDX, IDC_MEDIATEXT1, MediaText1);
	DDX_Control(pDX, IDC_MEDIATEXT2, MediaText2);
	DDX_Control(pDX, IDC_MEDIATEXT3, MediaText3);
}

BEGIN_MESSAGE_MAP(CMediaBrowserDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_MEDIALIST, OnNMDblclkMedialist)
	ON_CBN_SELCHANGE(IDC_COMBOMEDIAPATH, OnCbnSelchangeCombomediapath)
	ON_COMMAND(ID_FILE_SERVERLIST, OnFileServerlist)
	ON_COMMAND(ID_FILE_MOVEBACK, OnFileMoveback)
	ON_COMMAND(ID_FILE_MOVEFORWARD, OnFileMoveforward)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MEDIALIST, OnLvnItemchangedMedialist)
	ON_WM_KEYDOWN()
	ON_NOTIFY(LVN_KEYDOWN, IDC_MEDIALIST, OnLvnKeydownMedialist)
END_MESSAGE_MAP()


// CMediaBrowserDlg message handlers

BOOL CMediaBrowserDlg::OnInitDialog()
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
	CImageList* MediaImageList = new CImageList();
	MediaImageList->Create(16,16,ILC_COLOR16,8,8);

	CBitmap b1;
	b1.LoadBitmap(IDB_DEVICE2BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER1BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER2BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_METHODBITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	MediaList.SetImageList(MediaImageList,LVSIL_SMALL);

	memset(devices,0,sizeof(void*)*32);

	this->MediaList.InsertItem(0,"Searching...",-1);
	this->MediaCombo.AddString("Servers");
	this->MediaCombo.SetCurSel(0);

	MainDialog = this;
	CreateThread(NULL,0,&MediaControlPointStart,NULL,0,NULL);

	this->MoveWindow(100,100,240,320,true);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMediaBrowserDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMediaBrowserDlg::OnPaint() 
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
HCURSOR CMediaBrowserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void DisplayMediaInfo(struct UPnPDevice *device)
{
	char str[MAX_STR_LEN];
	Utf8ToAnsi(str, device->FriendlyName, MAX_STR_LEN);
	MainDialog->MediaText1.SetWindowText(str);
	Utf8ToAnsi(str, device->ManufacturerName, MAX_STR_LEN);
	MainDialog->MediaText2.SetWindowText(str);
	Utf8ToAnsi(str, device->ModelDescription, MAX_STR_LEN);
	MainDialog->MediaText3.SetWindowText(str);
}

void DisplayMediaInfo(struct MMSCP_MediaObject* object)
{
	char str[MAX_STR_LEN];
	struct MMSCP_MediaResource* res;
	
	Utf8ToAnsi(str, object->Title, MAX_STR_LEN);
	MainDialog->MediaText1.SetWindowText(str);

	Utf8ToAnsi(str, object->Creator, MAX_STR_LEN);
	MainDialog->MediaText2.SetWindowText(str);

	res = MMSCP_SelectBestIpNetworkResource(object, "http-get:*:*:*", IPAddressList, IPAddressListLen);

	if (res != NULL)
	{
		sprintf(str, "%s", res->Uri);
		MainDialog->MediaText3.SetWindowText(str);
	}
	else
	{
		MainDialog->MediaText3.SetWindowText("");
	}
}

void DisplayMediaInfo()
{
	MainDialog->MediaText1.SetWindowText("");
	MainDialog->MediaText2.SetWindowText("");
	MainDialog->MediaText3.SetWindowText("");
}

char* CopyThisIfNotThat(char *copyThis, const char *ifNotThat)
{
	int len;
	char *retVal = NULL;

	if (copyThis != NULL)
	{
		if (copyThis != ifNotThat)
		{
			if (ifNotThat != NULL)
			{
				len = (int) strlen(copyThis);
				retVal = (char*) malloc(len+1);
				strcpy(retVal, copyThis);
			}
		}
		else
		{
			retVal = copyThis;
		}
	}

	return retVal;
}

struct MMSCP_BrowseArgs* CreateBrowseArgs(char *objectID, enum MMSCP_Enum_BrowseFlag browseFlag, char *filter, unsigned int startingIndex, unsigned int requestedCount, char *sortCriteria)
{
	struct MMSCP_BrowseArgs* retVal;

	retVal = (struct MMSCP_BrowseArgs*) malloc (sizeof(struct MMSCP_BrowseArgs));
	memset(retVal, 0, sizeof(struct MMSCP_BrowseArgs));

	retVal->ObjectID = CopyThisIfNotThat(objectID, ZeroString);
	retVal->BrowseFlag = browseFlag;
	if (browseFlag == MMSCP_BrowseFlag_Metadata)
	{
		retVal->Filter = CopyThisIfNotThat(filter, FilterString);
		retVal->SortCriteria = CopyThisIfNotThat(sortCriteria, EmptyString);
	}
	else
	{
		retVal->Filter = CopyThisIfNotThat(filter, FilterString);
		retVal->SortCriteria = CopyThisIfNotThat(sortCriteria, SortString);
	}
	retVal->StartingIndex = 0;
	retVal->RequestedCount = MaxObjects;

	return retVal;
}

void DestroyBrowseArgs(struct MMSCP_BrowseArgs* args)
{
	if (args->ObjectID != ZeroString) {	NULL_FREE(args->ObjectID); }
	if ((args->Filter != FilterString) && (args->Filter != StarString)) {NULL_FREE(args->Filter);}
	if ((args->SortCriteria != EmptyString) && (args->SortCriteria != SortString)) {NULL_FREE(args->SortCriteria);}
	memset(args, 0, sizeof(struct MMSCP_BrowseArgs));
	NULL_FREE(args);
}

void DisplayMediaList(char* container)
{
	char str[MAX_STR_LEN];

	if (container == NULL || MediaListCurrentDevice == NULL)
	{
		DisplayMediaInfo();
		int i,j;
		bool d = false;
		MainDialog->MediaList.LockWindowUpdate();
		MainDialog->MediaList.DeleteAllItems();
		for(i=0;i<32;i++)
		{
			if (devices[i] != NULL)
			{
				Utf8ToAnsi(str, devices[i]->FriendlyName, MAX_STR_LEN);
				j = MainDialog->MediaList.InsertItem(0xFF,str,0);
				MainDialog->MediaList.SetItemData(j,(DWORD_PTR)devices[i]);
				d = true;
			}
		}
		MainDialog->MediaList.UnlockWindowUpdate();
		if (d == false) MainDialog->MediaList.InsertItem(0,TEXT("Searching..."),-1);
		MediaListCurrentContainer = NULL;
	}
	else
	{
		DisplayMediaInfo();
		struct UPnPService* service;
		service = MSCPGetService_ContentDirectory(MediaListCurrentDevice);
		if (service == NULL) return;
		DestroyBrowseArgs(ArgsBrowseDirectChildren);
		ArgsBrowseDirectChildren = CreateBrowseArgs(container, MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);
		MMSCP_Invoke_Browse(service, ArgsBrowseDirectChildren);
		MainDialog->MediaList.DeleteAllItems();
		MainDialog->MediaList.InsertItem(0,"Loading...",-1);
		MediaListCurrentContainer = container;
	}
}

void ServerAddedRemoved(struct UPnPDevice *device, int added)
{
	int i;
	if (added != 0)
	{
		for(i=0;i<32;i++)
		{
			if (devices[i] == NULL) {devices[i] = device;break;}
		}
	}
	else
	{
		for(i=0;i<32;i++)
		{
			if (devices[i] == device) {devices[i] = NULL;break;}
		}
		if (MediaListCurrentDevice == device) MainDialog->OnFileServerlist();
	}
	if (MediaListCurrentDevice == NULL) DisplayMediaList(NULL);

	if (IPAddressList != NULL)
	{
		free (IPAddressList);
		IPAddressList = NULL;
	}
	IPAddressListLen = ILibGetLocalIPAddressList(&IPAddressList);
}

void Result_Browse(void *serviceObj, struct MMSCP_BrowseArgs *args, int errorCode, struct MMSCP_ResultsList *results)
{
	int j;
	MMSCP_MediaObject* object;
	char str[MAX_STR_LEN];

	MainDialog->MediaList.DeleteAllItems();

	if (MediaResultList != NULL)
	{
		MMSCP_DestroyResultsList(MediaResultList);
		MediaResultList = NULL;
	}

	if (errorCode != 0)
	{
		MainDialog->MediaList.InsertItem(0,TEXT("Error"),0);
		return;
	}

	if (results == NULL || results->NumberReturned == 0)
	{
		MainDialog->MediaList.InsertItem(0,TEXT("Empty Container"),-1);
		return;
	}

	object = results->FirstObject;
	MediaResultList = results;
	MainDialog->MediaList.LockWindowUpdate();
	while(object != NULL)
	{
		Utf8ToAnsi(str, object->Title, MAX_STR_LEN);

		if (object->MediaClass & MMSCP_CLASS_MASK_CONTAINER)
		{
			j = MainDialog->MediaList.InsertItem(0xFF,str,2);
			MainDialog->MediaList.SetItemData(j,(DWORD_PTR)object);
		}
		else
		{
			j = MainDialog->MediaList.InsertItem(0xFF,str,3);
			MainDialog->MediaList.SetItemData(j,(DWORD_PTR)object);
		}

		object = object->Next;
	}
	MainDialog->MediaList.UnlockWindowUpdate();
}

DWORD WINAPI MediaControlPointStart(LPVOID args)
{
	chain = ILibCreateChain();
	
	ArgsBrowseMetadata = CreateBrowseArgs(ZeroString, MMSCP_BrowseFlag_Metadata, StarString, 0, 0, EmptyString);
	ArgsBrowseDirectChildren = CreateBrowseArgs(ZeroString, MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);

	MMSCP_Init(chain, Result_Browse, ServerAddedRemoved);
	ILibStartChain(chain);

	if (IPAddressList != NULL)
	{
		free (IPAddressList);
		IPAddressList = NULL;
	}

	if (MediaResultList != NULL)
	{
		MMSCP_DestroyResultsList(MediaResultList);
		MediaResultList = NULL;
	}

	DestroyBrowseArgs(ArgsBrowseMetadata);
	DestroyBrowseArgs(ArgsBrowseDirectChildren);

	while (MainDialog->MediaCombo.GetCount() > 1)
	{
		free((char*)MainDialog->MediaCombo.GetItemData(1));
		MainDialog->MediaCombo.DeleteString(1);
	}

	ExitFlag = true;
	PostMessage(MainDialog->GetSafeHwnd(),WM_CLOSE,0,0);

	return 0;
}

void CMediaBrowserDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (MediaList.GetSafeHwnd() != 0)
	{
		RECT clientRect;
		this->GetClientRect(&clientRect);
		clientRect.top += 20;
		clientRect.bottom -= 42;
		MediaList.MoveWindow(&clientRect,false);

		clientRect.left += 2;
		clientRect.right -= 2;
		clientRect.top = clientRect.bottom + 2;
		clientRect.bottom = clientRect.top + 13;
		this->MediaText1.MoveWindow(&clientRect,false);

		clientRect.bottom += 13;
		clientRect.top += 13;
		this->MediaText2.MoveWindow(&clientRect,false);

		clientRect.bottom += 13;
		clientRect.top += 13;
		this->MediaText3.MoveWindow(&clientRect,false);

		this->GetClientRect(&clientRect);
		clientRect.bottom = 20;
		this->MediaCombo.MoveWindow(&clientRect,false);

		this->Invalidate(false);
	}
}

void CMediaBrowserDlg::OnClose()
{
	if (ExitFlag == true)
	{
		CDialog::OnClose();
	}
	else
	{
		ILibStopChain(chain);
	}
}

void CMediaBrowserDlg::OnNMDblclkMedialist(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	struct UPnPDevice *device;
	struct UPnPService* service;
	struct MMSCP_MediaObject* object;
	char str[MAX_STR_LEN];

	POSITION pos = MediaList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;

	if (MediaListCurrentContainer == NULL)
	{
		int index = MediaList.GetNextSelectedItem(pos);
		device = (struct UPnPDevice*)MediaList.GetItemData(index);
		if (device == NULL) return;
		service = MSCPGetService_ContentDirectory(device);
		if (service == NULL) return;
		DestroyBrowseArgs(ArgsBrowseDirectChildren);
		ArgsBrowseDirectChildren = CreateBrowseArgs("0", MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);
		MMSCP_Invoke_Browse(service, ArgsBrowseDirectChildren);
		MediaListCurrentDevice = device;
		MediaListCurrentContainer = "0";
		MainDialog->MediaList.DeleteAllItems();
		MainDialog->MediaList.InsertItem(0,"Loading...",-1);

		Utf8ToAnsi(str, device->FriendlyName, MAX_STR_LEN);
		int i = this->MediaCombo.AddString(str);

		char* identifier = (char*)malloc(2);
		strcpy(identifier,"0");
		this->MediaCombo.SetItemData(i,(DWORD_PTR)identifier);
		this->MediaCombo.SetCurSel(i);

		DisplayMediaInfo();
	}
	else
	{
		int index = MediaList.GetNextSelectedItem(pos);
		object = (struct MMSCP_MediaObject*)MediaList.GetItemData(index);
		if (object == NULL || (object->MediaClass & MMSCP_CLASS_MASK_CONTAINER) == 0) return;
		service = MSCPGetService_ContentDirectory(MediaListCurrentDevice);
		DestroyBrowseArgs(ArgsBrowseDirectChildren);
		ArgsBrowseDirectChildren = CreateBrowseArgs(object->ID, MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);
		MMSCP_Invoke_Browse(service, ArgsBrowseDirectChildren);
		MediaListCurrentContainer = object->ID;
		MainDialog->MediaList.DeleteAllItems();
		MainDialog->MediaList.InsertItem(0,"Loading...",-1);

		Utf8ToAnsi(str, object->Title, MAX_STR_LEN);
		int i = this->MediaCombo.AddString(str);
		
		char* identifier = (char*)malloc(strlen(object->ID)+1);
		strcpy(identifier,object->ID);
		this->MediaCombo.SetItemData(i,(DWORD_PTR)identifier);
		this->MediaCombo.SetCurSel(i);

		DisplayMediaInfo();
	}
}

void CMediaBrowserDlg::OnCbnSelchangeCombomediapath()
{
	int i = MainDialog->MediaCombo.GetCurSel();
	if (i == CB_ERR) return;
	char* ID = (char*)MainDialog->MediaCombo.GetItemData(i);

	while (MainDialog->MediaCombo.GetCount() > (i+1))
	{
		free((char*)MainDialog->MediaCombo.GetItemData(i+1));
		MainDialog->MediaCombo.DeleteString(i+1);
	}

	if (ID == NULL)
	{
		OnFileServerlist();
	}
	else
	{
		DisplayMediaList(ID);
	}
}

void CMediaBrowserDlg::OnFileServerlist()
{
	if (MediaResultList != NULL)
	{
		MMSCP_DestroyResultsList(MediaResultList);
		MediaResultList = NULL;
	}
	MediaListCurrentDevice = NULL;
	DisplayMediaList(NULL);
}

void CMediaBrowserDlg::OnFileMoveback()
{
	int i = MainDialog->MediaCombo.GetCount();
	if (i == CB_ERR || i < 2) return;
	i -= 2;
	char* ID = (char*)MainDialog->MediaCombo.GetItemData(i);

	while (MainDialog->MediaCombo.GetCount() > (i+1))
	{
		free((char*)MainDialog->MediaCombo.GetItemData(i+1));
		MainDialog->MediaCombo.DeleteString(i+1);
	}
	MainDialog->MediaCombo.SetCurSel(i);

	if (ID == NULL)
	{
		OnFileServerlist();
	}
	else
	{
		DisplayMediaList(ID);
	}
}

void CMediaBrowserDlg::OnFileMoveforward()
{
	LRESULT result;
	OnNMDblclkMedialist(NULL,&result);
}

void CMediaBrowserDlg::OnFileExit()
{
	this->DestroyWindow();
}

void CMediaBrowserDlg::OnLvnItemchangedMedialist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	struct UPnPDevice *device;
	struct MMSCP_MediaObject* object;

	POSITION pos = MediaList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;

	if (MediaListCurrentContainer == NULL)
	{
		int index = MediaList.GetNextSelectedItem(pos);
		device = (struct UPnPDevice*)MediaList.GetItemData(index);
		if (device == NULL) return;
		DisplayMediaInfo(device);
	}
	else
	{
		int index = MediaList.GetNextSelectedItem(pos);
		object = (struct MMSCP_MediaObject*)MediaList.GetItemData(index);
		if (object == NULL) return;
		DisplayMediaInfo(object);
	}
}

void CMediaBrowserDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMediaBrowserDlg::OnLvnKeydownMedialist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;

	WORD key = pLVKeyDow->wVKey;
	if (key == 37)
	{
		OnFileMoveback();
	}
	else
	if (key == 39)
	{
		OnFileMoveforward();
	}
}
