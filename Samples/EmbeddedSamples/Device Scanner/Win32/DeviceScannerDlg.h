// DeviceScannerDlg.h : header file
//

#pragma once
#include "afxcmn.h"


// CDeviceScannerDlg dialog
class CDeviceScannerDlg : public CDialog
{
// Construction
public:
	CDeviceScannerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DEVICESCANNER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileExit();
	CTreeCtrl DeviceTree;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
};
