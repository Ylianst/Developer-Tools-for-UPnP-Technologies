// IntelMediaServerConfig.h : Declaration of the CIntelMediaServerConfig

#pragma once
#include "resource.h"       // main symbols

#include "IntelMediaServerService.h"
#include "_IIntelMediaServerConfigEvents_CP.h"

class ATL_NO_VTABLE CIntelMediaServerConfig;
extern CIntelMediaServerConfig* g_MediaServerConfig;

// CIntelMediaServerConfig

class ATL_NO_VTABLE CIntelMediaServerConfig : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CIntelMediaServerConfig, &CLSID_IntelMediaServerConfig>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CIntelMediaServerConfig>,
	public CProxy_IIntelMediaServerConfigEvents<CIntelMediaServerConfig>, 
	public IDispatchImpl<IIntelMediaServerConfig, &IID_IIntelMediaServerConfig, &LIBID_IntelMediaServerServiceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CIntelMediaServerConfig()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_INTELMEDIASERVERCONFIG)

DECLARE_NOT_AGGREGATABLE(CIntelMediaServerConfig)

BEGIN_COM_MAP(CIntelMediaServerConfig)
	COM_INTERFACE_ENTRY(IIntelMediaServerConfig)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CIntelMediaServerConfig)
	CONNECTION_POINT_ENTRY(__uuidof(_IIntelMediaServerConfigEvents))
END_CONNECTION_POINT_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		g_MediaServerConfig = this;
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (g_MediaServerConfig == this) g_MediaServerConfig = NULL;
	}

public:

	STDMETHOD(GetVersionInfo)(BSTR* info);
	STDMETHOD(GetSharedFolders)(BSTR* Folders);
	STDMETHOD(AddSharedFolder)(BSTR Folder);
	STDMETHOD(RemoveSharedFolder)(BSTR Folder);
};

OBJECT_ENTRY_AUTO(__uuidof(IntelMediaServerConfig), CIntelMediaServerConfig)
