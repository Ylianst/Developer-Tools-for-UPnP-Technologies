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
#include "IntelMediaServerConfig.h"
#include "MediaServer.h"

extern MediaServer* g_MediaServer;
CIntelMediaServerConfig* g_MediaServerConfig = NULL;

// CIntelMediaServerConfig

STDMETHODIMP CIntelMediaServerConfig::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IIntelMediaServerConfig
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CIntelMediaServerConfig::GetVersionInfo(BSTR* info)
{
	Fire_ServerLogEvent(L"GetVersionInfo() called");

	if (g_MediaServer != NULL)
	{
		g_MediaServer->GetVersionInfo(info);
	}

	return S_OK;
}

STDMETHODIMP CIntelMediaServerConfig::GetSharedFolders(BSTR* Folders)
{
	g_MediaServer->GetSharedFolders(Folders);
	return S_OK;
}

STDMETHODIMP CIntelMediaServerConfig::AddSharedFolder(BSTR Folder)
{
	g_MediaServer->AddSharedFolder(Folder);
	return S_OK;
}

STDMETHODIMP CIntelMediaServerConfig::RemoveSharedFolder(BSTR Folder)
{
	g_MediaServer->RemoveSharedFolder(Folder);
	return S_OK;
}
