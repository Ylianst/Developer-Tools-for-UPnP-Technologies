// Win32_WML.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "Win32_WML_i.h"


// CWin32_WMLApp:
// See Win32_WML.cpp for the implementation of this class
//

class CWin32_WMLApp : public CWinApp
{
public:
	CWin32_WMLApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	BOOL ExitInstance(void);
};

extern CWin32_WMLApp theApp;