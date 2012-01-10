// MainDlg.h : Declaration of the CMainDlg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#ifndef __MAINDLG_H_
#define __MAINDLG_H_

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "wmp.h"
#include "wmpids.h"

enum NODENAME
{
    TREE_PARENT = -1,
    TREE_ALL,
    TREE_NOWPLAYING,
    TREE_MUSIC,
    TREE_MUSIC_ARTIST,
    TREE_MUSIC_ARTISTITEM,
    TREE_MUSIC_ARTISTUNKNOWN,
    TREE_MUSIC_GENRE,
    TREE_MUSIC_GENREITEM,
    TREE_MUSIC_GENREUNKNOWN,
    TREE_MUSIC_ALBUM,
    TREE_MUSIC_ALBUMITEM,
    TREE_MUSIC_ALBUMUNKNOWN,
    TREE_VIDEO,
    TREE_VIDEO_ACTOR,
    TREE_VIDEO_ACTORITEM,
    TREE_VIDEO_ACTORUNKNOWN,
    TREE_VIDEO_GENRE,
    TREE_VIDEO_GENREITEM,
    TREE_VIDEO_GENREUNKNOWN,
    TREE_OTHER,
    TREE_MYPLAYLIST,
    TREE_MYPLAYLISTITEM,
    TREE_AUTOPLAYLIST,
    TREE_AUTOPLAYLISTITEM,
    TREE_RADIO
};

/////////////////////////////////////////////////////////////////////////////
// CMainDlg
class CMainDlg : 
    public CAxDialogImpl<CMainDlg>
{
public:
    CMainDlg(BOOL bIsRemote);
    ~CMainDlg();

    enum { IDD = IDD_MAINDLG };

BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_GOTOPLAYER, OnGoToPlayer)
    COMMAND_ID_HANDLER(ID_PLAYLISTS_NEWPLAYLIST, OnNewPlaylist)
    COMMAND_ID_HANDLER(ID_PLAYLISTS_IMPORTPLAYLIST, OnImportPlaylist)
    COMMAND_ID_HANDLER(ID_PLAYLISTS_REMOVE, OnRemovePlaylist)
    COMMAND_ID_HANDLER(ID_MEDIA_ADDURLOR, OnAddURL)
    COMMAND_ID_HANDLER(ID_MEDIA_REMOVE, OnRemoveMedia)
	NOTIFY_HANDLER(IDC_BASTREE, TVN_SELCHANGED, OnClickTree)
    COMMAND_HANDLER(IDC_NAMELIST, LBN_DBLCLK, OnDbClickMediaList)
    COMMAND_HANDLER(IDC_NAMELIST, LBN_SELCHANGE, OnSelectMedia)

	MESSAGE_HANDLER(ID_UPNP_BROWSE, DoUpnpBrowse)

END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnGoToPlayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnImportPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemovePlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddURL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemoveMedia(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDbClickMediaList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelectMedia(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT DoUpnpBrowse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    // Utility functions
    void        ShowMainTree();
    HTREEITEM   AddNode(TCHAR* szName, NODENAME iPara, HTREEITEM hParent = TVI_ROOT);
    HRESULT     ShowNowPlaying();
    HRESULT     ShowAllMedia();
    HRESULT     ShowAllMusic();
    HRESULT     ShowAllMusicTree();
    HRESULT     ShowAllVideo();
    HRESULT     ShowAllVideoTree();
    HRESULT     ShowPlaylistsTree();
    HRESULT     ShowOthers();
    HRESULT     ShowStringCollection(HTREEITEM hParent, NODENAME enumNode, TCHAR *szAttr, TCHAR *szMediaType);
    HRESULT     ShowPlaylist(IWMPPlaylist* pPlaylist, TCHAR *szMediaType = NULL);
    HRESULT     ShowPlaylist(TCHAR *szPlaylistName);
    HRESULT     ShowGetByAttr(TCHAR *szAttrName, TCHAR *szAttrVal, TCHAR *szMediaType);
    HRESULT     ShowAllRadio();
    void        ShowChildNodes(HTREEITEM hParent);

public:
    CComPtr<IWMPPlayer4>            m_spPlayer;             // Player

private:
    /************* Embedded WMP objects ********************/
	BOOL							m_bIsRemote;
    CAxWindow                       *m_pView;               // IE control to hold WMP OCX
    CComPtr<IWMPMediaCollection>    m_spMC;                 // MediaCollection object
    CComPtr<IWMPPlaylistCollection> m_spPC;                 // PlaylistCollection object
    CComPtr<IWMPPlaylist>           m_spSavedPlaylist;      // Saved playlist object for media list
    
    /************* Saved handlers for fast access ********************/
    HWND                            m_hTree;                // Tree view
    HWND                            m_hList;                // Media list
    HWND                            m_hMetadata;            // Metadata list
    HTREEITEM                       m_hAllMusicNode;        // All music node
    HTREEITEM                       m_hAllVideoNode;        // All video node
    HTREEITEM                       m_hMyPlaylistsNode;     // My playlists
    HTREEITEM                       m_hAutoPlaylistwNode;   // Auto playlist

    NODENAME                        m_iSelectedNode;        // The node that is selected    
    BOOL                            m_bIsShowingMedia;      // Whether items in media list is real media names or just list of nodes
};


/*
 *	Encapsulates input arguments for a Browse request.
 *	All string data is in wide form.
 */
struct StringBrowseArgsWide
{
	/*
	 *	ObjectID specified by the control point.
	 */
	unsigned short *ObjectID;

	/*
	 *	Metadata filter settings. Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
	 *	that describe what CDS metadata to return in the response. In this framework,
	 *	the application layer is responsible for enforcing metadata filtering.
	 */
	unsigned short *Filter;

	/*
	 *	SortCriteria strings have the following form:
	 *	[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 
	 */
	unsigned short *SortCriteria;
};

#endif //__MAINDLG_H_
