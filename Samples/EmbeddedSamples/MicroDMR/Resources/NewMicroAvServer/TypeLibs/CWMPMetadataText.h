// CWMPMetadataText.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPMetadataText

class CWMPMetadataText : public COleDispatchDriver
{
public:
	CWMPMetadataText() {}		// Calls COleDispatchDriver default constructor
	CWMPMetadataText(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPMetadataText(const CWMPMetadataText& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	CString get_Description()
	{
		CString result;
		InvokeHelper(0x420, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	CString get_text()
	{
		CString result;
		InvokeHelper(0x41f, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}


};
