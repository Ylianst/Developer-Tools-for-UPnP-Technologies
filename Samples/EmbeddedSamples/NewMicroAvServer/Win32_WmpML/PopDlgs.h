// PopDlgs.h : Declaration of the pop-up dialogs for menu items
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#ifndef __POPDLGS_H_
#define __POPDLGS_H_

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "wmp.h"
#include "wmpids.h"

/**********************************************************
*  CAddMediaDlg: 
*    pop dialog for adding URL/path to media library
*
***********************************************************/
class CAddMediaDlg : 
    public CAxDialogImpl<CAddMediaDlg>
{
public:
    CAddMediaDlg(IWMPMediaCollection *pMC);
    ~CAddMediaDlg();

    enum { IDD = IDD_ADDMEDIADLG };

BEGIN_MSG_MAP(CAddMediaDlg)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
END_MSG_MAP()

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CComPtr<IWMPMediaCollection>        m_spMC;
};


/**********************************************************
*  CAddMediaDlg: 
*    pop dialog for deleting media from media library
*    It shows all media in a list-box and use select
*    the media to be deleted
*
***********************************************************/
class CDelMediaDlg : 
    public CAxDialogImpl<CDelMediaDlg>
{
public:
    CDelMediaDlg(IWMPMediaCollection *pMC);
    ~CDelMediaDlg();

    enum { IDD = IDD_DELMEDIADLG };

BEGIN_MSG_MAP(CDelMediaDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CComPtr<IWMPMediaCollection>        m_spMC;
    CComPtr<IWMPPlaylist>               m_spPlaylist;
};


/**********************************************************
*  CDelPlaylistDlg: 
*    pop dialog for deleting playlist from media library
*    It shows all playlists in a list-box and use select
*    the media to be deleted
*
***********************************************************/
class CDelPlaylistDlg : 
    public CAxDialogImpl<CDelPlaylistDlg>
{
public:
    CDelPlaylistDlg(IWMPPlaylistCollection *pPC);
    ~CDelPlaylistDlg();

    enum { IDD = IDD_DELPLAYLISTDLG };

BEGIN_MSG_MAP(CDelPlaylistDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CComPtr<IWMPPlaylistCollection>     m_spPC;
    CComPtr<IWMPPlaylistArray>          m_spPlaylistArray;
};


/**********************************************************
*  CImportPlaylistDlg: 
*    pop dialog for importing playlist into media library
*
***********************************************************/
class CImportPlaylistDlg : 
    public CAxDialogImpl<CImportPlaylistDlg>
{
public:
    CImportPlaylistDlg(IWMPPlayer4* pPlayer);
    ~CImportPlaylistDlg(){}

    enum { IDD = IDD_IMPORTPLAYLISTDLG };

BEGIN_MSG_MAP(CImportPlaylistDlg)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
END_MSG_MAP()

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CComPtr<IWMPPlayer4>        m_spPlayer;
};


/**********************************************************
*  CNewPlaylistDlg: 
*    pop dialog for creating a new playlist into media library
*
***********************************************************/
class CNewPlaylistDlg : 
    public CAxDialogImpl<CNewPlaylistDlg>
{
public:
    CNewPlaylistDlg(IWMPPlaylistCollection *pPC);
    ~CNewPlaylistDlg(){}

    enum { IDD = IDD_NEWPLAYLISTDLG };

BEGIN_MSG_MAP(CNewPlaylistDlg)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CComPtr<IWMPPlaylistCollection>     m_spPC;
};


#endif //__POPDLGS_H_
