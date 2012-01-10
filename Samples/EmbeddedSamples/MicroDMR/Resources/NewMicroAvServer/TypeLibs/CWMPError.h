// CWMPError.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPError

class CWMPError : public COleDispatchDriver
{
public:
	CWMPError() {}		// Calls COleDispatchDriver default constructor
	CWMPError(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPError(const CWMPError& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	void clearErrorQueue()
	{
		InvokeHelper(0x353, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	long get_errorCount()
	{
		long result;
		InvokeHelper(0x354, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	LPDISPATCH get_Item(long dwIndex)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x355, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms, dwIndex);
		return result;
	}
	void webHelp()
	{
		InvokeHelper(0x356, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}


};
