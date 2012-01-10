// MediaBrowserPPCDlg.h : header file
//

#if !defined(AFX_MEDIABROWSERPPCDLG_H__99C2B1E9_82B0_4489_B89F_66174B4C66AC__INCLUDED_)
#define AFX_MEDIABROWSERPPCDLG_H__99C2B1E9_82B0_4489_B89F_66174B4C66AC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CMediaBrowserPPCDlg dialog

class CMediaBrowserPPCDlg : public CDialog
{
// Construction
public:
	CMediaBrowserPPCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMediaBrowserPPCDlg)
	enum { IDD = IDD_MEDIABROWSERPPC_DIALOG };
	CStatic	m_MediaText3;
	CStatic	m_MediaText2;
	CStatic	m_MediaText1;
	CComboBox	m_MediaCombo;
	CListCtrl	m_MediaList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMediaBrowserPPCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMediaBrowserPPCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileExit();
	afx_msg void OnFileMoveback();
	afx_msg void OnFileMoveforward();
	afx_msg void OnFileMovetoserverlist();
	afx_msg void OnClose();
	afx_msg void OnDblclkMedialist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombomediapath();
	afx_msg void OnItemchangedMedialist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownMedialist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDIABROWSERPPCDLG_H__99C2B1E9_82B0_4489_B89F_66174B4C66AC__INCLUDED_)
