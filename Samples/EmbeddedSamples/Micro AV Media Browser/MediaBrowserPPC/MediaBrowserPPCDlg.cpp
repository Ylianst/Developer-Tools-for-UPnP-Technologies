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
#include "MediaBrowserPPC.h"
#include "MediaBrowserPPCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C"
{
	#include "MSCPControlPoint.h"
	#include "ILibParsers.h"
	#include "MmsCp.h"
	#include "MyString.h"
}


#define NULL_FREE(x) free(x); x = NULL

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
CMediaBrowserPPCDlg* MainDialog = NULL;
struct UPnPDevice* devices[32];
struct UPnPDevice *MediaListCurrentDevice = NULL;
char* MediaListCurrentContainer = NULL;
struct MMSCP_ResultsList *MediaResultList = NULL;
int *IPAddressList = NULL;
int IPAddressListLen = 0;



/////////////////////////////////////////////////////////////////////////////
// CMediaBrowserPPCDlg dialog

CMediaBrowserPPCDlg::CMediaBrowserPPCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaBrowserPPCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMediaBrowserPPCDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMediaBrowserPPCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMediaBrowserPPCDlg)
	DDX_Control(pDX, IDC_MEDIASTATIC3, m_MediaText3);
	DDX_Control(pDX, IDC_MEDIASTATIC2, m_MediaText2);
	DDX_Control(pDX, IDC_MEDIASTATIC1, m_MediaText1);
	DDX_Control(pDX, IDC_COMBOMEDIAPATH, m_MediaCombo);
	DDX_Control(pDX, IDC_MEDIALIST, m_MediaList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMediaBrowserPPCDlg, CDialog)
	//{{AFX_MSG_MAP(CMediaBrowserPPCDlg)
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_MOVEBACK, OnFileMoveback)
	ON_COMMAND(ID_FILE_MOVEFORWARD, OnFileMoveforward)
	ON_COMMAND(ID_FILE_MOVETOSERVERLIST, OnFileMovetoserverlist)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_MEDIALIST, OnDblclkMedialist)
	ON_CBN_SELCHANGE(IDC_COMBOMEDIAPATH, OnSelchangeCombomediapath)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MEDIALIST, OnItemchangedMedialist)
	ON_NOTIFY(LVN_KEYDOWN, IDC_MEDIALIST, OnKeydownMedialist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMediaBrowserPPCDlg message handlers

BOOL CMediaBrowserPPCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// Extra initialization here
	CCeCommandBar *pCommandBar = (CCeCommandBar*)m_pWndEmptyCB;
	pCommandBar->InsertMenuBar(IDR_MAINMENU);

	CImageList* MediaImageList = new CImageList();
	MediaImageList->Create(16,16,0,8,8);

	CBitmap b1;
	b1.LoadBitmap(IDB_DEVICE2BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER1BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_FOLDER1BITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	b1.LoadBitmap(IDB_METHODBITMAP);
	MediaImageList->Add(&b1,RGB(0,0,0));
	b1.DeleteObject();

	m_MediaList.SetImageList(MediaImageList,LVSIL_SMALL);

	memset(devices,0,sizeof(void*)*32);

	this->m_MediaList.InsertItem(0,TEXT("Searching..."),-1);
	this->m_MediaCombo.AddString(TEXT("Servers"));
	this->m_MediaCombo.SetCurSel(0);

	MainDialog = this;
	CreateThread(NULL,0,&MediaControlPointStart,NULL,0,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CMediaBrowserPPCDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_MediaList.GetSafeHwnd() != 0)
	{
		RECT clientRect;
		this->GetClientRect(&clientRect);
		clientRect.top += 20;
		clientRect.bottom -= 42;
		m_MediaList.MoveWindow(&clientRect,false);

		clientRect.left += 2;
		clientRect.right -= 2;
		clientRect.top = clientRect.bottom + 2;
		clientRect.bottom = clientRect.top + 13;
		this->m_MediaText1.MoveWindow(&clientRect,false);

		clientRect.bottom += 13;
		clientRect.top += 13;
		this->m_MediaText2.MoveWindow(&clientRect,false);

		clientRect.bottom += 13;
		clientRect.top += 13;
		this->m_MediaText3.MoveWindow(&clientRect,false);

		this->GetClientRect(&clientRect);
		clientRect.bottom = 20;
		this->m_MediaCombo.MoveWindow(&clientRect,false);

		this->Invalidate(false);
	}
}



void DisplayMediaInfo(struct UPnPDevice *device)
{
	wchar_t str[402];
	Utf8ToWide((wchar_t*)str,device->FriendlyName,200);
	MainDialog->m_MediaText1.SetWindowText((wchar_t*)str);
	Utf8ToWide((wchar_t*)str,device->ManufacturerName,200);
	MainDialog->m_MediaText2.SetWindowText((wchar_t*)str);
	Utf8ToWide((wchar_t*)str,device->ModelDescription,200);
	MainDialog->m_MediaText3.SetWindowText((wchar_t*)str);
}

void DisplayMediaInfo(struct MMSCP_MediaObject* object)
{
	wchar_t str[2048];
	struct MMSCP_MediaResource* res;

	Utf8ToWide((wchar_t*)str,object->Title,100);
	MainDialog->m_MediaText1.SetWindowText((wchar_t*)str);

	Utf8ToWide((wchar_t*)str,object->Creator,100);
	MainDialog->m_MediaText2.SetWindowText((wchar_t*)str);

	res = MMSCP_SelectBestIpNetworkResource(object, "http-get:*:*:*", IPAddressList, IPAddressListLen);

	if (res != NULL)
	{
		Utf8ToWide((wchar_t*)str, res->Uri, 100);
		MainDialog->m_MediaText3.SetWindowText(str);
	}
	else
	{
		MainDialog->m_MediaText3.SetWindowText(TEXT(""));
	}}

void DisplayMediaInfo()
{
	MainDialog->m_MediaText1.SetWindowText(TEXT(""));
	MainDialog->m_MediaText2.SetWindowText(TEXT(""));
	MainDialog->m_MediaText3.SetWindowText(TEXT(""));
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
		retVal->Filter = CopyThisIfNotThat(filter, StarString);
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
	wchar_t str[202];

	MainDialog->m_MediaList.ShowWindow(SW_HIDE);
	if (container == NULL || MediaListCurrentDevice == NULL)
	{
		DisplayMediaInfo();
		int i,j;
		bool d = false;
		MainDialog->m_MediaList.DeleteAllItems();
		for(i=0;i<32;i++)
		{
			if (devices[i] != NULL)
			{
				Utf8ToWide((wchar_t*)str,devices[i]->FriendlyName,100);
				j = MainDialog->m_MediaList.InsertItem(0xFF,(wchar_t*)str,0);
				MainDialog->m_MediaList.SetItemData(j,(int)devices[i]);
				d = true;
			}
		}
		if (d == false) MainDialog->m_MediaList.InsertItem(0,TEXT("Searching..."),-1);
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
		MainDialog->m_MediaList.DeleteAllItems();
		MainDialog->m_MediaList.InsertItem(0,TEXT("Loading..."),-1);
		MediaListCurrentContainer = container;
	}
	MainDialog->m_MediaList.ShowWindow(SW_SHOWNA);
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
		if (MediaListCurrentDevice == device) MainDialog->OnFileMovetoserverlist();
	}

	if (IPAddressList != NULL)
	{
		free (IPAddressList);
		IPAddressList = NULL;
	}
	IPAddressListLen = ILibGetLocalIPAddressList(&IPAddressList);

	if (MediaListCurrentDevice == NULL) DisplayMediaList(NULL);
}

void Result_Browse(void *serviceObj, struct MMSCP_BrowseArgs *args, int errorCode, struct MMSCP_ResultsList *results)
{
	int j;
	MMSCP_MediaObject* object;
	MainDialog->m_MediaList.DeleteAllItems();
	wchar_t str[202];

	if (MediaResultList != NULL)
	{
		MMSCP_DestroyResultsList(MediaResultList);
		MediaResultList = NULL;
	}

	if (errorCode != 0)
	{
		MainDialog->m_MediaList.InsertItem(0,TEXT("Error"),0);
		return;
	}

	if (results == NULL || results->NumberReturned == 0)
	{
		MainDialog->m_MediaList.InsertItem(0,TEXT("Empty Container"),-1);
		return;
	}

	object = results->FirstObject;
	MediaResultList = results;
	MainDialog->m_MediaList.ShowWindow(SW_HIDE);
	while(object != NULL)
	{
		Utf8ToWide((wchar_t*)str,object->Title,100);
		if (object->MediaClass & MMSCP_CLASS_MASK_CONTAINER)
		{
			j = MainDialog->m_MediaList.InsertItem(0xFF,(wchar_t*)str,2);
			MainDialog->m_MediaList.SetItemData(j,(int)object);
		}
		else
		{
			j = MainDialog->m_MediaList.InsertItem(0xFF,(wchar_t*)str,3);
			MainDialog->m_MediaList.SetItemData(j,(int)object);
		}
		object = object->Next;
	}
	MainDialog->m_MediaList.ShowWindow(SW_SHOWNA);
}

DWORD WINAPI MediaControlPointStart(LPVOID args)
{
	chain = ILibCreateChain();
	
	ArgsBrowseMetadata = CreateBrowseArgs(ZeroString, MMSCP_BrowseFlag_Metadata, StarString, 0, 0, EmptyString);
	ArgsBrowseDirectChildren = CreateBrowseArgs(ZeroString, MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);

	MMSCP_Init(chain, &Result_Browse, &ServerAddedRemoved);
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

	while (MainDialog->m_MediaCombo.GetCount() > 1)
	{
		free((char*)MainDialog->m_MediaCombo.GetItemData(1));
		MainDialog->m_MediaCombo.DeleteString(1);
	}

	ExitFlag = true;
	PostMessage(MainDialog->GetSafeHwnd(),WM_CLOSE,0,0);

	return 0;
}

void CMediaBrowserPPCDlg::OnFileExit() 
{
	this->DestroyWindow();
}

void CMediaBrowserPPCDlg::OnFileMoveback() 
{
	int i = MainDialog->m_MediaCombo.GetCount();
	if (i == CB_ERR || i < 2) return;
	i -= 2;
	char* ID = (char*)MainDialog->m_MediaCombo.GetItemData(i);

	while (MainDialog->m_MediaCombo.GetCount() > (i+1))
	{
		free((char*)MainDialog->m_MediaCombo.GetItemData(i+1));
		MainDialog->m_MediaCombo.DeleteString(i+1);
	}

	MainDialog->m_MediaCombo.SetCurSel(i);

	if (ID == NULL)
	{
		OnFileMovetoserverlist();
	}
	else
	{
		DisplayMediaList(ID);
	}
}

void CMediaBrowserPPCDlg::OnFileMoveforward() 
{
	LRESULT result;
	OnDblclkMedialist(NULL,&result);
}

void CMediaBrowserPPCDlg::OnFileMovetoserverlist() 
{
	if (MediaResultList != NULL)
	{
		MMSCP_DestroyResultsList(MediaResultList);
		MediaResultList = NULL;
	}
	MediaListCurrentDevice = NULL;
	DisplayMediaList(NULL);
}

void CMediaBrowserPPCDlg::OnClose() 
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

void CMediaBrowserPPCDlg::OnDblclkMedialist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	struct UPnPDevice *device;
	struct UPnPService* service;
	struct MMSCP_MediaObject* object;
	wchar_t str[202];

	POSITION pos = m_MediaList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;

	if (MediaListCurrentContainer == NULL)
	{
		int index = m_MediaList.GetNextSelectedItem(pos);
		device = (struct UPnPDevice*)m_MediaList.GetItemData(index);
		if (device == NULL) return;
		service = MSCPGetService_ContentDirectory(device);
		if (service == NULL) return;
		DestroyBrowseArgs(ArgsBrowseDirectChildren);
		ArgsBrowseDirectChildren = CreateBrowseArgs("0", MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);
		MMSCP_Invoke_Browse(service, ArgsBrowseDirectChildren);
		MediaListCurrentDevice = device;
		MediaListCurrentContainer = "0";
		MainDialog->m_MediaList.DeleteAllItems();
		MainDialog->m_MediaList.InsertItem(0,TEXT("Loading..."),-1);

		Utf8ToWide((wchar_t*)str,device->FriendlyName,100);
		int i = this->m_MediaCombo.AddString((wchar_t*)str);

		char* identifier = (char*)malloc(2);
		strcpy(identifier,"0");
		this->m_MediaCombo.SetItemData(i,(DWORD_PTR)identifier);
		this->m_MediaCombo.SetCurSel(i);

		DisplayMediaInfo();
	}
	else
	{
		int index = m_MediaList.GetNextSelectedItem(pos);
		object = (struct MMSCP_MediaObject*)m_MediaList.GetItemData(index);
		if (object == NULL || (object->MediaClass & MMSCP_CLASS_MASK_CONTAINER) == 0) return;
		service = MSCPGetService_ContentDirectory(MediaListCurrentDevice);
		DestroyBrowseArgs(ArgsBrowseDirectChildren);
		ArgsBrowseDirectChildren = CreateBrowseArgs(object->ID, MMSCP_BrowseFlag_Children, FilterString, 0, MaxObjects, SortString);
		MMSCP_Invoke_Browse(service, ArgsBrowseDirectChildren);
		MediaListCurrentContainer = object->ID;
		MainDialog->m_MediaList.DeleteAllItems();
		MainDialog->m_MediaList.InsertItem(0,TEXT("Loading..."),-1);
		
		Utf8ToWide((wchar_t*)str,object->Title,100);
		int i = this->m_MediaCombo.AddString((wchar_t*)str);

		char* identifier = (char*)malloc(strlen(object->ID)+1);
		strcpy(identifier,object->ID);
		this->m_MediaCombo.SetItemData(i,(DWORD_PTR)identifier);
		this->m_MediaCombo.SetCurSel(i);

		DisplayMediaInfo();
	}
}

void CMediaBrowserPPCDlg::OnSelchangeCombomediapath() 
{
	int i = MainDialog->m_MediaCombo.GetCurSel();
	if (i == CB_ERR) return;
	char* ID = (char*)MainDialog->m_MediaCombo.GetItemData(i);

	while (MainDialog->m_MediaCombo.GetCount() > (i+1))
	{
		free((char*)MainDialog->m_MediaCombo.GetItemData(i+1));
		MainDialog->m_MediaCombo.DeleteString(i+1);
	}

	if (ID == NULL)
	{
		OnFileMovetoserverlist();
	}
	else
	{
		DisplayMediaList(ID);
	}	
}

void CMediaBrowserPPCDlg::OnItemchangedMedialist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	struct UPnPDevice *device;
	struct MMSCP_MediaObject* object;

	POSITION pos = m_MediaList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;

	if (MediaListCurrentContainer == NULL)
	{
		int index = m_MediaList.GetNextSelectedItem(pos);
		device = (struct UPnPDevice*)m_MediaList.GetItemData(index);
		if (device == NULL) return;
		DisplayMediaInfo(device);
	}
	else
	{
		int index = m_MediaList.GetNextSelectedItem(pos);
		object = (struct MMSCP_MediaObject*)m_MediaList.GetItemData(index);
		if (object == NULL) return;
		DisplayMediaInfo(object);
	}
}

void CMediaBrowserPPCDlg::OnKeydownMedialist(NMHDR* pNMHDR, LRESULT* pResult) 
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
