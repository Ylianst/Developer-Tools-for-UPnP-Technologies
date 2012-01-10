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
#include "resource.h"
#include "IntelMediaServerService.h"
#include "MediaServer.h"
#include <stdio.h>

MediaServer *g_MediaServer = NULL;

class CIntelMediaServerServiceModule : public CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >
{
public :
	DECLARE_LIBID(LIBID_IntelMediaServerServiceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_INTELMEDIASERVERSERVICE, "{91556716-97AA-4787-8EE7-D8D648886E24}")

	static DWORD WINAPI MediaServerInitThread(LPVOID args)
	{
		Sleep(50);
		CoInitializeEx(0,COINIT_MULTITHREADED);
		g_MediaServer = new MediaServer();

		// Fetch path of this service
		TCHAR szFilePath[MAX_PATH];
		DWORD dwFLen = ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
		if( dwFLen == 0 || dwFLen == MAX_PATH ) return E_FAIL;

		g_MediaServer->Init(szFilePath);

		return 0;
	}

	HRESULT Run(int nShowCmd = SW_HIDE)
	{
		CreateThread(NULL,0,&MediaServerInitThread,this,0,NULL);

		HRESULT r = CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >::Run(nShowCmd);

		if (g_MediaServer != NULL)
		{
			delete g_MediaServer;
			g_MediaServer = NULL;
		}

		return r;
	}

	void OnStop() throw()
	{
		if (g_MediaServer != NULL)
		{
			delete g_MediaServer;
			g_MediaServer = NULL;
		}
		CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >::OnStop();
	}

	void OnPause() throw()
	{
		if (g_MediaServer != NULL)
		{
			delete g_MediaServer;
			g_MediaServer = NULL;
		}
		CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >::OnPause();
	}

	void OnContinue() throw()
	{
		if (g_MediaServer == NULL)
		{
			// Fetch path of this service
			TCHAR szFilePath[MAX_PATH];
			DWORD dwFLen = ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
			if( dwFLen == 0 || dwFLen == MAX_PATH ) return;

			g_MediaServer = new MediaServer();
			HRESULT r = g_MediaServer->Init(szFilePath);
		}
		CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >::OnContinue();
	}

	void OnShutdown() throw()
	{
		if (g_MediaServer != NULL)
		{
			delete g_MediaServer;
			g_MediaServer = NULL;
		}

		CAtlServiceModuleT< CIntelMediaServerServiceModule, IDS_SERVICENAME >::OnShutdown();
	}

};

CIntelMediaServerServiceModule _AtlModule;


//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    return _AtlModule.WinMain(nShowCmd);
}

