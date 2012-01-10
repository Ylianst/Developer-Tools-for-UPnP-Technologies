// DeviceScannerPPC.h : main header file for the DEVICESCANNERPPC application
//

#if !defined(AFX_DEVICESCANNERPPC_H__E6279FD3_91F2_4810_864F_EB8C33FD3086__INCLUDED_)
#define AFX_DEVICESCANNERPPC_H__E6279FD3_91F2_4810_864F_EB8C33FD3086__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDeviceScannerPPCApp:
// See DeviceScannerPPC.cpp for the implementation of this class
//

class CDeviceScannerPPCApp : public CWinApp
{
public:
	CDeviceScannerPPCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceScannerPPCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDeviceScannerPPCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICESCANNERPPC_H__E6279FD3_91F2_4810_864F_EB8C33FD3086__INCLUDED_)
