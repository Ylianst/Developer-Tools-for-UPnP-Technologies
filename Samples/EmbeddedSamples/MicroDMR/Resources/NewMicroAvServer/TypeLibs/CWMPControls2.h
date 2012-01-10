// CWMPControls2.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWMPControls2

class CWMPControls2 : public COleDispatchDriver
{
public:
	CWMPControls2() {}		// Calls COleDispatchDriver default constructor
	CWMPControls2(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPControls2(const CWMPControls2& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:

	BOOL get_isAvailable(LPCTSTR bstrItem)
	{
		BOOL result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x3e, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms, bstrItem);
		return result;
	}
	void play()
	{
		InvokeHelper(0x33, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void stop()
	{
		InvokeHelper(0x34, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void pause()
	{
		InvokeHelper(0x35, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void fastForward()
	{
		InvokeHelper(0x36, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void fastReverse()
	{
		InvokeHelper(0x37, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	double get_currentPosition()
	{
		double result;
		InvokeHelper(0x38, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
		return result;
	}
	void put_currentPosition(double newValue)
	{
		static BYTE parms[] = VTS_R8 ;
		InvokeHelper(0x38, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	CString get_currentPositionString()
	{
		CString result;
		InvokeHelper(0x39, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	void next()
	{
		InvokeHelper(0x3a, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void previous()
	{
		InvokeHelper(0x3b, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	LPDISPATCH get_currentItem()
	{
		LPDISPATCH result;
		InvokeHelper(0x3c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
		return result;
	}
	void put_currentItem(LPDISPATCH newValue)
	{
		static BYTE parms[] = VTS_DISPATCH ;
		InvokeHelper(0x3c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_currentMarker()
	{
		long result;
		InvokeHelper(0x3d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_currentMarker(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x3d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	void playItem(LPDISPATCH pIWMPMedia)
	{
		static BYTE parms[] = VTS_DISPATCH ;
		InvokeHelper(0x3f, DISPATCH_METHOD, VT_EMPTY, NULL, parms, pIWMPMedia);
	}
	void step(long lStep)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x40, DISPATCH_METHOD, VT_EMPTY, NULL, parms, lStep);
	}


};
