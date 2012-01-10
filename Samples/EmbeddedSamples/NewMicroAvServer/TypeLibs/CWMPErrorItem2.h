// CWMPErrorItem2.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPErrorItem2

class CWMPErrorItem2 : public COleDispatchDriver
{
public:
	CWMPErrorItem2() {}		// Calls COleDispatchDriver default constructor
	CWMPErrorItem2(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPErrorItem2(const CWMPErrorItem2& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	long get_errorCode()
	{
		long result;
		InvokeHelper(0x385, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_errorDescription()
	{
		CString result;
		InvokeHelper(0x386, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	VARIANT get_errorContext()
	{
		VARIANT result;
		InvokeHelper(0x387, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
		return result;
	}
	long get_remedy()
	{
		long result;
		InvokeHelper(0x388, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_customUrl()
	{
		CString result;
		InvokeHelper(0x389, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	long get_condition()
	{
		long result;
		InvokeHelper(0x38a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}


};
