// SampleProject.h : main header file for the SAMPLEPROJECT application
//

#if !defined(AFX_SAMPLEPROJECT_H__5E391F99_D50E_412F_88CF_5FCBC18B98A0__INCLUDED_)
#define AFX_SAMPLEPROJECT_H__5E391F99_D50E_412F_88CF_5FCBC18B98A0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSampleProjectApp:
// See SampleProject.cpp for the implementation of this class
//

class CSampleProjectApp : public CWinApp
{
public:
	CSampleProjectApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSampleProjectApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSampleProjectApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAMPLEPROJECT_H__5E391F99_D50E_412F_88CF_5FCBC18B98A0__INCLUDED_)
