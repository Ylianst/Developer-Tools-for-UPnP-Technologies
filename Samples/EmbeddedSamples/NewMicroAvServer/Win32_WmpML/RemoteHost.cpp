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
#include "RemoteHost.h"
#include "resource.h"
#include "stdio.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteHost

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRemoteHost::CRemoteHost()
{

}

CRemoteHost::~CRemoteHost()
{

}

//***************************************************************************
// QueryService()
// API from IServiceProvider
//***************************************************************************
HRESULT CRemoteHost::QueryService(REFGUID guidService, REFIID riid, void ** ppv)
{
    return ppv? QueryInterface(riid, ppv) : E_POINTER;
}


//***************************************************************************
// GetServiceType()
// Always return Remote so that the player OCX runs at remote state
//***************************************************************************
HRESULT CRemoteHost::GetServiceType(BSTR * pbstrType)
{
    HRESULT hr = E_POINTER;
    if(pbstrType)
    {
        *pbstrType = ::SysAllocString(L"Remote");
        hr = *pbstrType? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetApplicationName()
// Return the application name. It will be shown in player's menu View >
// Switch to applications
//***************************************************************************
HRESULT CRemoteHost::GetApplicationName(BSTR * pbstrName)
{
    HRESULT     hr = E_POINTER;
    if(pbstrName)
    {
        CComBSTR    bstrAppName = _T("");
        bstrAppName.LoadString(IDS_PROJNAME);
        *pbstrName = bstrAppName.Detach();
        hr = *pbstrName? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetScriptableObject()
// There is no scriptable object in this application
//***************************************************************************
HRESULT CRemoteHost::GetScriptableObject(BSTR * pbstrName, IDispatch ** ppDispatch)
{
    if(pbstrName)
    {
        *pbstrName = NULL;
    }
    if(ppDispatch)
    {
        *ppDispatch = NULL;
    }
    return E_NOTIMPL;
}

//***************************************************************************
// GetCustomUIMode()
// When UI mode of the player OCX is set to custom, this function is called
// to give the skin file path that will be loaded to the player OCX.
// 
//***************************************************************************
HRESULT CRemoteHost::GetCustomUIMode(BSTR * pbstrFile)
{
    return E_NOTIMPL;
}
