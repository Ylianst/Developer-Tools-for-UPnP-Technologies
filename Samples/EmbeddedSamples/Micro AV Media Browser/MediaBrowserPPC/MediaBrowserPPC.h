// MediaBrowserPPC.h : main header file for the MEDIABROWSERPPC application
//

#if !defined(AFX_MEDIABROWSERPPC_H__4D04C7CB_5674_4DA5_B147_A27D7F47CEBE__INCLUDED_)
#define AFX_MEDIABROWSERPPC_H__4D04C7CB_5674_4DA5_B147_A27D7F47CEBE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMediaBrowserPPCApp:
// See MediaBrowserPPC.cpp for the implementation of this class
//

class CMediaBrowserPPCApp : public CWinApp
{
public:
	CMediaBrowserPPCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMediaBrowserPPCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMediaBrowserPPCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDIABROWSERPPC_H__4D04C7CB_5674_4DA5_B147_A27D7F47CEBE__INCLUDED_)
