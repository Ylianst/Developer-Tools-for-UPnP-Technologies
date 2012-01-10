// DeviceScannerPPCDlg.h : header file
//

#if !defined(AFX_DEVICESCANNERPPCDLG_H__609A817F_9DF8_4416_827B_A925126DCCCD__INCLUDED_)
#define AFX_DEVICESCANNERPPCDLG_H__609A817F_9DF8_4416_827B_A925126DCCCD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDeviceScannerPPCDlg dialog

class CDeviceScannerPPCDlg : public CDialog
{
// Construction
public:
	CDeviceScannerPPCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviceScannerPPCDlg)
	enum { IDD = IDD_DEVICESCANNERPPC_DIALOG };
	CTreeCtrl	m_DeviceTree;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceScannerPPCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDeviceScannerPPCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFileExit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICESCANNERPPCDLG_H__609A817F_9DF8_4416_827B_A925126DCCCD__INCLUDED_)
