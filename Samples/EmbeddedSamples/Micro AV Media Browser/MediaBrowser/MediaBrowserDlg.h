// MediaBrowserDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CMediaBrowserDlg dialog
class CMediaBrowserDlg : public CDialog
{
// Construction
public:
	CMediaBrowserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MEDIABROWSER_DIALOG };

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
	CListCtrl MediaList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnNMDblclkMedialist(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox MediaCombo;
	afx_msg void OnCbnSelchangeCombomediapath();
	afx_msg void OnFileServerlist();
	afx_msg void OnFileMoveback();
	afx_msg void OnFileMoveforward();
	afx_msg void OnFileExit();
	CStatic MediaText1;
	CStatic MediaText2;
	CStatic MediaText3;
	afx_msg void OnLvnItemchangedMedialist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLvnKeydownMedialist(NMHDR *pNMHDR, LRESULT *pResult);
};
