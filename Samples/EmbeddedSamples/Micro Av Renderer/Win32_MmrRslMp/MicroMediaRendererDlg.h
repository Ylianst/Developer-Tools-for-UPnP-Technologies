/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


// MicroAVRendererDlg.h : header file
//

#pragma once
#include "mediaplayer1.h"

extern "C"
{
	#include "../MicroMediaRenderer.h"
}

// CMicroAVRendererDlg dialog
class CMicroAVRendererDlg : public CDialog
{
// Construction
public:
	CMicroAVRendererDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MICROAVRENDERER_DIALOG };

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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	DECLARE_EVENTSINK_MAP()
	void PositionChangeActivemoviecontrol2(double oldPosition, double newPosition);
	void StateChangeActivemoviecontrol2(long oldState, long newState);
	afx_msg LRESULT CMicroAVRendererDlg::OnMPINVOKE(WPARAM wp, LPARAM lp);
	void ErrorActivemoviecontrol2(short SCode, LPCTSTR Description, LPCTSTR Source, BOOL* CancelDisplay);
	CMediaplayer1 MediaPlayer;
	void PositionChangeMediaplayer1(double oldPosition, double newPosition);
	void PlayStateChangeMediaplayer1(long OldState, long NewState);
	void ErrorMediaplayer1();
	void OpenStateChangeMediaplayer1(long OldState, long NewState);
	afx_msg void OnFileExit();
	afx_msg void OnControlPlay();
	afx_msg void OnControlStop();
	afx_msg void OnControlPause();
public:
	void CloseDialog();

};
