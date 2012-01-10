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

#include "resource.h" 
#include <commdlg.h>
#include "PlayerOCX.h"
#include "events_dispid.h"

#define IDC_PLAYER			1000
#define IDB_PLAY			1001
#define IDB_PAUSE			1002
#define	IDB_STOP			1003
#define IDB_FASTREVERSE		1004
#define IDB_FASTFORWARD		1005
#define MAX_PLAYER_HEIGHT	198
#define STATUS_BAR_HEIGHT	14
#define WM_MPINVOKE			(WM_USER + 1)

/////////////////////////////////////////////////////////////////////////////
// CMicroMediaRenderer

class CMicroMediaRenderer : 
	public CWindowImpl<CMicroMediaRenderer, CWindow, CWinTraits<WS_VISIBLE, NULL> >,
	public CComObjectRootEx<CComSingleThreadModel>,
	public _IWMPEvents
{
	CAxWindow					m_wndView;						// The window that hosts the Windows Media Player control
	CComPtr<IWMP>				m_spWMPPlayer;					// A pointer to the Windows Media Player control interface
	CComPtr<IConnectionPoint>	m_spConnectionPoint;			// The connection point through which events are handled
	DWORD						m_dwAdviseCookie;				// A cookie used by the connection point to cease event notifications
	DWORD						m_nFilterIndex;					// An index to the current file name filter
	TCHAR*						m_szCurrentFile;				// The name of the current file (without path information)

public:

	BEGIN_COM_MAP(CMicroMediaRenderer)
		COM_INTERFACE_ENTRY_IID(__uuidof(_IWMPEvents), _IWMPEvents)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	DECLARE_WND_CLASS(NULL)

	BEGIN_MSG_MAP(CMicroMediaRenderer)

		MESSAGE_HANDLER(	WM_CREATE,			OnCreate)
		MESSAGE_HANDLER(	WM_PAINT,			OnPaint)
		MESSAGE_HANDLER(	WM_DESTROY,			OnDestroy)
		MESSAGE_HANDLER(	WM_MPINVOKE,		OnMPINVOKE)

		COMMAND_ID_HANDLER( IDOK,				OnOK)
		COMMAND_ID_HANDLER( ID_FILE_OPEN,		OnFileOpen)
		COMMAND_ID_HANDLER( ID_FILE_EXIT,		OnFileExit)

	END_MSG_MAP()

	
	// CMediaBookmarker methods:

	CMicroMediaRenderer();
	~CMicroMediaRenderer();

	// Window methods:
	
	LRESULT OnCreate(		UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(		UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	// User-interface methods:

	LRESULT OnOK(			WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileOpen(		WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileExit(		WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	// IDispatch methods:

	STDMETHOD(GetIDsOfNames)(	REFIID				riid, 
								OLECHAR FAR *FAR	*rgszNames,
								unsigned int		cNames, 
								LCID				lcid, 
								DISPID FAR			*rgDispId )
																{ return( E_NOTIMPL ); };

	STDMETHOD(GetTypeInfo)(		unsigned int		iTInfo, 
								LCID				lcid, 
								ITypeInfo FAR *FAR	*ppTInfo )
																{ return( E_NOTIMPL ); };

	STDMETHOD(GetTypeInfoCount)(unsigned int FAR	*pctinfo )
																{ return( E_NOTIMPL ); };

	STDMETHOD(Invoke)(			DISPID				dispIdMember,	  
								REFIID				riid,			  
								LCID				lcid,				
								WORD				wFlags,			  
								DISPPARAMS FAR		*pDispParams,  
								VARIANT FAR			*pVarResult,  
								EXCEPINFO FAR		*pExcepInfo,  
								unsigned int FAR	*puArgErr );


	// Windows Media Player Control event methods:

	void OnPlayStateChange(long NewState);

	void SetMediaPlayerVolume();

	LRESULT OnMPINVOKE(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

typedef CComObject<CMicroMediaRenderer> CComMicroMediaRenderer;
