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

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f WmpMLps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "MainDlg.h"

extern "C"
{
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"

	// CDS Headers
	#include "MediaServerLogic.h"
	#include "MimeTypes.h"
	#include "MyString.h"
}

static HWND	hWndMainWindow = NULL;
static HANDLE UPnPThread = NULL;
unsigned long WINAPI UPnPMain(void* ptr);
void *TheChain;
void *TheStack;
void *TheMediaServerLogic;
void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;
void Sink_OnBrowse (struct MSL_BrowseArgs *browseArgs);
CMainDlg TheDlg(TRUE);

const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return l;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!bActivity && m_nLockCnt == 0) // if no activity let's really bail
        {
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
            CoSuspendClassObjects();
            if (!bActivity && m_nLockCnt == 0)
#endif
                break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

    CoInitialize(NULL);
	_Module.Init(ObjectMap, hInstance, &LIBID_ATLLib);

	// Check command line parameter for remote
    TCHAR szTokens[] = _T("-/");
    BOOL bRemote = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("Local"))==0)
        {
            bRemote = TRUE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

	::InitCommonControls();

	// spawn UPnP thread
	CreateThread(NULL, 0, &UPnPMain, NULL, 0, NULL ); 

	// Load the main dialog
	TheDlg.DoModal();

	ILibStopChain(TheChain);
    _Module.Term();
    CoUninitialize();

    return 0;
}




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
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor, NULL);
}


unsigned long WINAPI UPnPMain(void* ptr)
{
	TheChain = ILibCreateChain();

	TheStack = UpnpCreateMicroStack(TheChain, "Intel's Micro Media Server (Win32/WML)","Win32-WML-46de-a4b4-92afa44808db","0000001",1800,0);

	UpnpMonitor = ILibCreateLifeTime(TheChain);

	MSL_Callback_OnBrowse = Sink_OnBrowse;
	TheMediaServerLogic = MSL_CreateMediaServer(TheChain, TheStack, UpnpMonitor);

	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor, NULL);

	ILibStartChain(TheChain);
	return 0;
}

void Sink_OnBrowse (struct MSL_BrowseArgs *browseArgs)
{
	/*
	 *	All of the string data in browseArgs is in UTF8.
	 *	Convert to wide.
	 */
	int size;
	struct StringBrowseArgsWide *baw = (struct StringBrowseArgsWide *) malloc(sizeof(struct StringBrowseArgsWide));

	size							= ((int)strlen(browseArgs->Filter) + 1) * sizeof(unsigned short);
	baw->Filter						= (unsigned short*) malloc ( size );
	Utf8ToWide(baw->Filter, browseArgs->Filter, size);

	size							= ((int)strlen(browseArgs->ObjectID) + 1) * sizeof(unsigned short);
	baw->ObjectID					= (unsigned short*) malloc ( size );
	Utf8ToWide(baw->ObjectID, browseArgs->ObjectID, size);

	size							= ((int)strlen(browseArgs->SortCriteria) + 1) * sizeof(unsigned short);
	baw->SortCriteria				= (unsigned short*) malloc ( size );
	Utf8ToWide(baw->SortCriteria, browseArgs->SortCriteria, size);

	browseArgs->UserObject = baw;
	PostMessage(TheDlg.m_hWnd, ID_UPNP_BROWSE, NULL, (LPARAM) browseArgs);
}
