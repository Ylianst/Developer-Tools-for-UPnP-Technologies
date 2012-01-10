// Win32_WMLDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "mediaplayer.h"

// CWin32_WMLDlg dialog
class CWin32_WMLDlg : public CDialog
{
// Construction
public:
	CWin32_WMLDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WIN32_WML_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartStop();
	// 0: Media Server stopped; 1: Media Server running; -1 Media Server in process of starting or stopping
	int m_ServerState;
	CButton m_BtnStartStop;
	void ShutdownComplete(void);
	void StartupFailed(void);
	void StartupComplete(void);
};
