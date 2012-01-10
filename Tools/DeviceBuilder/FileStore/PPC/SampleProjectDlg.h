// SampleProjectDlg.h : header file
//

#if !defined(AFX_SAMPLEPROJECTDLG_H__04151FD9_69C0_402A_B1A3_EA657F499050__INCLUDED_)
#define AFX_SAMPLEPROJECTDLG_H__04151FD9_69C0_402A_B1A3_EA657F499050__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CSampleProjectDlg dialog

class CSampleProjectDlg : public CDialog
{
// Construction
public:
	CSampleProjectDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSampleProjectDlg)
	enum { IDD = IDD_SAMPLEPROJECT_DIALOG };
	CString	m_Text;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSampleProjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	LRESULT OnUpdate(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CSampleProjectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFileQuit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAMPLEPROJECTDLG_H__04151FD9_69C0_402A_B1A3_EA657F499050__INCLUDED_)
