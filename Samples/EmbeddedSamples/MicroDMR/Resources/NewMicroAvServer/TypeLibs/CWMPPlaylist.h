// CWMPPlaylist.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPPlaylist

class CWMPPlaylist : public COleDispatchDriver
{
public:
	CWMPPlaylist() {}		// Calls COleDispatchDriver default constructor
	CWMPPlaylist(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPPlaylist(const CWMPPlaylist& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	long get_count()
	{
		long result;
		InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_name()
	{
		CString result;
		InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	void put_name(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xca, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_attributeCount()
	{
		long result;
		InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_attributeName(long lIndex)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xd3, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, parms, lIndex);
		return result;
	}
	LPDISPATCH get_Item(long lIndex)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xd4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms, lIndex);
		return result;
	}
	CString getItemInfo(LPCTSTR bstrName)
	{
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xcb, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, bstrName);
		return result;
	}
	void setItemInfo(LPCTSTR bstrName, LPCTSTR bstrValue)
	{
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0xcc, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrName, bstrValue);
	}
	BOOL get_isIdentical(LPDISPATCH pIWMPPlaylist)
	{
		BOOL result;
		static BYTE parms[] = VTS_DISPATCH ;
		InvokeHelper(0xd5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms, pIWMPPlaylist);
		return result;
	}
	void clear()
	{
		InvokeHelper(0xcd, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void insertItem(long lIndex, LPDISPATCH pIWMPMedia)
	{
		static BYTE parms[] = VTS_I4 VTS_DISPATCH ;
		InvokeHelper(0xce, DISPATCH_METHOD, VT_EMPTY, NULL, parms, lIndex, pIWMPMedia);
	}
	void appendItem(LPDISPATCH pIWMPMedia)
	{
		static BYTE parms[] = VTS_DISPATCH ;
		InvokeHelper(0xcf, DISPATCH_METHOD, VT_EMPTY, NULL, parms, pIWMPMedia);
	}
	void removeItem(LPDISPATCH pIWMPMedia)
	{
		static BYTE parms[] = VTS_DISPATCH ;
		InvokeHelper(0xd0, DISPATCH_METHOD, VT_EMPTY, NULL, parms, pIWMPMedia);
	}
	void moveItem(long lIndexOld, long lIndexNew)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0xd1, DISPATCH_METHOD, VT_EMPTY, NULL, parms, lIndexOld, lIndexNew);
	}


};
