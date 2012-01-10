// CWMPDVD.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPDVD

class CWMPDVD : public COleDispatchDriver
{
public:
	CWMPDVD() {}		// Calls COleDispatchDriver default constructor
	CWMPDVD(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPDVD(const CWMPDVD& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	BOOL get_isAvailable(LPCTSTR bstrItem)
	{
		BOOL result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x3e9, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms, bstrItem);
		return result;
	}
	CString get_domain()
	{
		CString result;
		InvokeHelper(0x3ea, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	void topMenu()
	{
		InvokeHelper(0x3eb, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void titleMenu()
	{
		InvokeHelper(0x3ec, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void back()
	{
		InvokeHelper(0x3ed, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void resume()
	{
		InvokeHelper(0x3ee, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}


};
