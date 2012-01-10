// CWMPCdromCollection.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPCdromCollection

class CWMPCdromCollection : public CWnd
{
protected:
	DECLARE_DYNCREATE(CWMPCdromCollection)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x6BF52A52, 0x394A, 0x11D3, { 0xB1, 0x53, 0x0, 0xC0, 0x4F, 0x79, 0xFA, 0xA6 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
				UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE,
				BSTR bstrLicKey = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// Attributes
public:

// Operations
public:

	long get_count()
	{
		long result;
		InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	LPDISPATCH Item(long lIndex)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x12e, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, lIndex);
		return result;
	}
	LPDISPATCH getByDriveSpecifier(LPCTSTR bstrDriveSpecifier)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x12f, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, bstrDriveSpecifier);
		return result;
	}


};
