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

#include "stdafx.h"
#include "MainDlg.h"
#include "PopDlgs.h"
#include "RemoteHost.h"
#include <CRTDBG.H>

extern "C"
{
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"

	// CDS Headers
	#include "MediaServerLogic.h"
	#include "MimeTypes.h"
	#include "MyString.h"
	#include "CdsErrors.h"
	#include "CdsStrings.h"
	#include "CdsMediaObject.h"
	#include "CdsObjectToDidl.h"
}

extern void *TheChain;
extern void *TheStack;
extern void *TheMediaServerLogic;
extern void *UpnpMonitor;
extern int UpnpIPAddressLength;
extern int *UpnpIPAddressList;

CMainDlg *MainDlg;

/////////////////////////////////////////////////////////////////////////////
// CMainDlg

/***********************************************************************
* Constructor
***********************************************************************/
CMainDlg::CMainDlg(BOOL bIsRemote)
{
	// Initialize member variables
	m_bIsRemote = bIsRemote;
    m_hTree = NULL;
    m_hList = NULL;
    m_hMetadata = NULL;
    m_hAllMusicNode = NULL;
    m_hAllVideoNode = NULL;
    m_hMyPlaylistsNode = NULL;
    m_hAutoPlaylistwNode = NULL;

    m_bIsShowingMedia = TRUE;
    m_spSavedPlaylist = NULL;
    m_iSelectedNode = TREE_PARENT;
	MainDlg = this;
}

/***********************************************************************
* Destructor
***********************************************************************/
CMainDlg::~CMainDlg()
{
}

/***********************************************************************
* OnInitDialog
* 
* Initialize member variables of handlers
* Initialize WMP OCX
***********************************************************************/
LRESULT CMainDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Get handler of list and tree for later usage
    m_hTree = GetDlgItem(IDC_TREE);
    m_hList = GetDlgItem(IDC_LIST);
    m_hMetadata = GetDlgItem(IDC_METADATA);

    ///////////////////////////////////////////////////
    // Initialize WMP control
    //////////////////////////////////////////////////
	CComPtr<IAxWinHostWindow>	spHost;
    CComPtr<IObjectWithSite>    spHostObject;
    CComObject<CRemoteHost>     *pRemoteHost = NULL;
    RECT                        rectWMP={0,0,0,0};
    HRESULT                     hr;
    
	// Location of the WMP control
    ::GetWindowRect(GetDlgItem(IDC_RANGE), &rectWMP);
    ScreenToClient(&rectWMP);

    // Get an simple container to contain WMP OCX
	AtlAxWinInit();
    m_pView = new CAxWindow();
 	hr = m_pView? S_OK : E_OUTOFMEMORY;
   
    if(SUCCEEDED(hr))
    {
        m_pView->Create(m_hWnd, rectWMP, NULL, WS_CHILD | WS_VISIBLE);
		hr = ::IsWindow(m_pView->m_hWnd)? S_OK : E_FAIL;
	}

	if(SUCCEEDED(hr) && m_bIsRemote)
	{
        hr = m_pView->QueryHost(IID_IObjectWithSite, (void **)&spHostObject);
		hr = spHostObject.p? hr : E_FAIL;

		if(SUCCEEDED(hr))
		{
			hr = CComObject<CRemoteHost>::CreateInstance(&pRemoteHost);
			if(pRemoteHost)
			{
				pRemoteHost->AddRef();
			}
			else
			{
				hr = E_POINTER;
			}
		}

		if(SUCCEEDED(hr))
		{
			hr = spHostObject->SetSite((IWMPRemoteMediaServices *)pRemoteHost);
		}
	}

	if(SUCCEEDED(hr))
	{
		hr = m_pView->QueryHost(&spHost);
		hr = spHost.p? hr : E_FAIL;
	}

	// Create WMP control using its CLSID
	if(SUCCEEDED(hr))
	{
		hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_pView->m_hWnd, 0);
	}

	if(SUCCEEDED(hr))
	{
		hr = m_pView->QueryControl(&m_spPlayer);
		hr = m_spPlayer.p? hr : E_FAIL;
	}

    if(SUCCEEDED(hr))
    {
		// If it is remote OCX, enable Go To Media Library button
		VARIANT_BOOL	bIsRemote = VARIANT_FALSE;
		m_spPlayer->get_isRemote(&bIsRemote);
		::EnableWindow(GetDlgItem(IDC_GOTOPLAYER), bIsRemote == VARIANT_TRUE? TRUE : FALSE);

		// Now we have the WMP OCX, we get mediaCollection and playlistCollection
		// objects for later use.
        m_spPlayer->get_mediaCollection(&m_spMC);
        m_spPlayer->get_playlistCollection(&m_spPC);

		// Use tree view to show media library
        ShowMainTree();
    }

    // Release remote host object
    if(pRemoteHost)
    {
        pRemoteHost->Release();
    }

    return 1;  // Let the system set the focus
}

/***********************************************************************
* OnDestroy
* Release WMP OCX
***********************************************************************/
LRESULT CMainDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Release all WMP objects before exit
    m_spSavedPlaylist = NULL;
    m_spMC = NULL;
    m_spPC = NULL;
    m_spPlayer = NULL;

    if(m_pView != NULL)
    {
        delete m_pView;
    }

    return 0;
}

/***********************************************************************
* OnOK
* User closes the dialog
***********************************************************************/
LRESULT CMainDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnCancel
* User closes the dialog
***********************************************************************/
LRESULT CMainDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

/***********************************************************************
* OnGoToPlayer
* Switch to media player when in remote mode
***********************************************************************/
LRESULT CMainDlg::OnGoToPlayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT                         hr = E_FAIL;
    CComPtr<IWMPPlayerApplication>  spPlayerApp;
    CComPtr<IWMPPlayerServices>     spPlayerServices;  

	hr = m_spPlayer.p? S_OK : E_POINTER;

	if(SUCCEEDED(hr))
	{
        hr = m_spPlayer->QueryInterface(&spPlayerServices);
		hr = spPlayerServices.p? S_OK : E_NOINTERFACE;
	}

    if(SUCCEEDED(hr))
    {
        // Switch to media library pane
        spPlayerServices->setTaskPane(CComBSTR(_T("MediaLibrary")));
    }

	if(SUCCEEDED(hr))
	{
        hr = m_spPlayer->get_playerApplication(&spPlayerApp);
		hr = spPlayerApp.p? S_OK : E_NOINTERFACE;
	}

    if(SUCCEEDED(hr))
    {
        // Undock the player
        spPlayerApp->switchToPlayerApplication();
    }

	return 0;
}

/***********************************************************************
* OnNewPlaylist
* Called when menu Playlist > New is chosen
* It pops up a dialog using CNewPlaylistDlg
***********************************************************************/
LRESULT CMainDlg::OnNewPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CNewPlaylistDlg dlg(m_spPC);

    // When a user creates a new playlist, it returns PLAYLIST
    // then we update the relative part of the tree view
    if(dlg.DoModal() == PLAYLIST)
    {
        ShowPlaylistsTree();
    }
    return 0;
}

/***********************************************************************
* OnImportPlaylist
* Called when menu Playlist > Import is chosen
* It pops up a dialog using CImportPlaylistDlg
***********************************************************************/
LRESULT CMainDlg::OnImportPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CImportPlaylistDlg dlg(m_spPlayer);

    // When a user imports a playlist, it returns PLAYLIST
    // then we update the relative part of the tree view
    if(dlg.DoModal() == PLAYLIST)
    {
        ShowPlaylistsTree();
    }
    return 0;
}

/***********************************************************************
* OnRemovePlaylist
* Called when menu Playlist > Delete is chosen
* It pops up a dialog using CDelPlaylistDlg
***********************************************************************/
LRESULT CMainDlg::OnRemovePlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CDelPlaylistDlg dlg(m_spPC);

    // When a user delete a playlist, it returns PLAYLIST
    // then we update the relative part of the tree view
    if(dlg.DoModal() == PLAYLIST)
    {
        ShowPlaylistsTree();
    }
    return 0;
}

/***********************************************************************
* OnAddURL
* Called when menu Media > Add is chosen
* It pops up a dialog using CAddMediaDlg
***********************************************************************/
LRESULT CMainDlg::OnAddURL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CAddMediaDlg dlg(m_spMC);

    // It returns different MEDIATYPE when user adds different type
    // of URL. If the added media is playlist (ASX and WPL) then it 
    // returns PLAYLIST
    int iRetCode = dlg.DoModal();
    switch(iRetCode)
    {
    case MUSIC:
        ShowAllMusicTree();
        break;
    case VIDEO:
        ShowAllVideoTree();
        break;
    case PLAYLIST:
        ShowPlaylistsTree();
    }

    return 0;
}

/***********************************************************************
* OnRemoveMedia
* Called when menu Media > Delete is chosen
* It pops up a dialog using CDelMediaDlg
***********************************************************************/
LRESULT CMainDlg::OnRemoveMedia(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CDelMediaDlg dlg(m_spMC);

    // Return value shows what kind of media is deleted
    int iRetCode = dlg.DoModal();
    if(iRetCode == MUSIC)
    {
        ShowAllMusicTree();
    }
    else if(iRetCode == VIDEO)
    {
        ShowAllVideoTree();
    }
    return 0;
}

/***********************************************************************
* ShowMainTree
* Build the tree view of the media library
***********************************************************************/
void CMainDlg::ShowMainTree()
{
    // Now Playing node
    AddNode(_T("Now Playing"), TREE_NOWPLAYING);
    
    // All node
    AddNode(_T("All"), TREE_ALL);
    
    // All Music node
    m_hAllMusicNode = AddNode(_T("All Music"), TREE_MUSIC);
    ShowAllMusicTree();

    // All Video node
    m_hAllVideoNode = AddNode(_T("All Video"), TREE_VIDEO);
    ShowAllVideoTree();
    
    // Other Media node
    AddNode(_T("Other Media"), TREE_OTHER);

    // My Playlists and auto playlists nodes
    m_hMyPlaylistsNode = AddNode(_T("My Playlists"), TREE_MYPLAYLIST);
    m_hAutoPlaylistwNode = AddNode(_T("Auto Playlists"), TREE_AUTOPLAYLIST);
    ShowPlaylistsTree();
    
    // Radio node
    AddNode(_T("Radio"), TREE_RADIO);
}

/***********************************************************************
* AddNode
* szName: string shown on the added node
* iPara: param value for the added node
* hParent: handler to the parent node
* 
* It adds a node under hParent with given text and param
***********************************************************************/
HTREEITEM CMainDlg::AddNode(TCHAR* szName, NODENAME iPara, HTREEITEM hParent)
{
    TVINSERTSTRUCT tvins;
    
    tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvins.hParent = hParent;
    tvins.hInsertAfter = TVI_LAST;
    
    // add the node
    tvins.item.lParam = (LPARAM)iPara;
    tvins.item.pszText = szName;
    tvins.item.cchTextMax = _tcslen(szName);
    return TreeView_InsertItem(m_hTree, &tvins);
}

/***********************************************************************
* OnClickTree
* 
* It is called when a user clicks on the tree
***********************************************************************/
LRESULT CMainDlg::OnClickTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HTREEITEM hCurItem = TreeView_GetSelection(m_hTree);
    HTREEITEM hParentItem = TreeView_GetParent(m_hTree, hCurItem);
    
    // Retrieve the text and param of the selected node
    TCHAR szPlaylistName[MAX_BSTR_LONG + 1];
    TVITEM tvi;
    tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
    tvi.hItem = hCurItem;
    tvi.pszText = szPlaylistName;
    tvi.cchTextMax = MAX_BSTR_LONG;

    BOOL bVal;
    bVal = TreeView_GetItem(m_hTree, &tvi);

    // We use param of the node to tell what kind of node it is
    m_iSelectedNode = (NODENAME)(tvi.lParam);
    switch(tvi.lParam)
    {
    case TREE_ALL:                      // All node
        ShowAllMedia();
        break;
    case TREE_NOWPLAYING:               // Now playing node
        ShowNowPlaying();
        break;
    case TREE_MUSIC:                    // All music node
        ShowAllMusic();
        break;
    case TREE_MUSIC_ARTIST:             // Music > Artist node
        ShowChildNodes(hCurItem);
        break;
    case TREE_MUSIC_ARTISTITEM:         // Artis name nodes under Music > Artist node
        ShowGetByAttr(_T("Artist"), szPlaylistName, _T("Audio"));
        break;
    case TREE_MUSIC_ARTISTUNKNOWN:      // Unknonw node under Music > Artist node
        ShowGetByAttr(_T("Artist"), _T(""), _T("Audio"));
        break;
    case TREE_MUSIC_GENRE:              // Music > genre node
        ShowChildNodes(hCurItem);
        break;
    case TREE_MUSIC_GENREITEM:          // Genre value nodes under Music > Genre node
        ShowGetByAttr(_T("Genre"), szPlaylistName, _T("Audio"));
        break;
    case TREE_MUSIC_GENREUNKNOWN:       // Unknown node under Music > Genre node
        ShowGetByAttr(_T("Genre"), _T(""), _T("Audio"));
        break;
    case TREE_MUSIC_ALBUM:              // Music > album node
        ShowChildNodes(hCurItem);
        break;
    case TREE_MUSIC_ALBUMITEM:          // Album value nodes under Music > Album node
        ShowGetByAttr(_T("Album"), szPlaylistName, _T("Audio"));
        break;
    case TREE_MUSIC_ALBUMUNKNOWN:       // Unknown node under Music > Album node
        ShowGetByAttr(_T("Album"), _T(""), _T("Audio"));
        break;
    case TREE_VIDEO:                    // All video
        ShowAllVideo();
        break;
    case TREE_VIDEO_ACTOR:              // Video > Actor node
        ShowChildNodes(hCurItem);
        break;
    case TREE_VIDEO_ACTORITEM:          // Actor name nodes under Video > Actor node
        ShowGetByAttr(_T("Actor"), szPlaylistName, _T("Video"));
        break;
    case TREE_VIDEO_ACTORUNKNOWN:       // Unknown node under Video > Actor node
        ShowGetByAttr(_T("Actor"), _T(""), _T("Video"));
        break;
    case TREE_VIDEO_GENRE:              // Video > Genre node
        ShowChildNodes(hCurItem);
        break;
    case TREE_VIDEO_GENREITEM:          // Genre value nodes under Video > Genre node
        ShowGetByAttr(_T("Genre"), szPlaylistName, _T("Video"));
        break;
    case TREE_VIDEO_GENREUNKNOWN:       // Unknown node under Video > Genre node
        ShowGetByAttr(_T("Genre"), _T(""), _T("Video"));
        break;
    case TREE_OTHER:                    // Other node
        ShowOthers();
        break;
    case TREE_MYPLAYLIST:               // My Playlists node
        ShowChildNodes(hCurItem);
        break;
    case TREE_MYPLAYLISTITEM:           // Playlist nodes under My Playlists
        ShowPlaylist(szPlaylistName);
        break;
    case TREE_AUTOPLAYLIST:             // Auto Playlists node
        ShowChildNodes(hCurItem);
        break;
    case TREE_AUTOPLAYLISTITEM:         // Playlist nodes under Auto Playlists
        ShowPlaylist(szPlaylistName);
        break;
    case TREE_RADIO:                    // Radio node
        ShowAllRadio();
        break;
    default:
        break;
    }
    return 0;
}

/***********************************************************************
* ShowPlaylist
* pPlaylist: playlist object shown in the media list
* szMediaType: when it's not NULL, only media with this mediaType can be listed
* 
* ListBox has only about 64K memory to save data for strings, so this is
* not a good way to show large media library. So deal with large media
* library, use custom-drawn ListBox
***********************************************************************/
HRESULT CMainDlg::ShowPlaylist(IWMPPlaylist* pPlaylist, TCHAR *szMediaType)
{
    if(pPlaylist == NULL)
    {
        return E_POINTER;
    }

    // Reset metadata list
    ::SendMessage(m_hMetadata, LB_RESETCONTENT, 0, 0);
    // Reset media list
    ::SendMessage(m_hList, LB_RESETCONTENT, 0, 0);

    // We always save the shown playlist for higher performance
    m_spSavedPlaylist = NULL;
    m_spSavedPlaylist = pPlaylist;

    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPMedia>      spMedia;
    CComBSTR                bstrName, bstrType;
    long                    lCount, i, iListed = 0;

    USES_CONVERSION;

    // Get count
    if(pPlaylist != NULL)
    {
        hr = pPlaylist->get_count(&lCount);
    }

    // Enumerate the items
    if(SUCCEEDED(hr))
    {
        for(i = 0; i < lCount; i++)
        {
            hr = pPlaylist->get_item(i, &spMedia);

            // If the media type is restricted, then we only show those items
            // with given type
            if(szMediaType != NULL)
            {
                if(SUCCEEDED(hr) && (spMedia.p != NULL))
                {
                    hr = spMedia->getItemInfo(CComBSTR(_T("MediaType")), &bstrType);
                }

                if(SUCCEEDED(hr) && (bstrType.m_str != NULL) && (!_tcsicmp(OLE2T(bstrType), szMediaType)))
                {
                    hr = spMedia->get_name(&bstrName);
                }
            }
            else
            {
                if(SUCCEEDED(hr) && (spMedia.p != NULL))
                {
                    hr = spMedia->get_name(&bstrName);
                }
            }

            // When bstrName is not NULL, we add the name to the media list
            if(SUCCEEDED(hr) && bstrName.m_str != NULL)
            {
                iListed++;
                ::SendMessage(m_hList, LB_ADDSTRING, 0, (LPARAM)OLE2T(bstrName));
                bstrName.Empty();
            }

            // Now we can release media object
            spMedia = NULL;
        }
    }

    // Show how many items in the media list
    TCHAR szLine[20];
    _stprintf(szLine, _T("%d items"), iListed);
    SetDlgItemText(IDC_STATUS, szLine);

    m_bIsShowingMedia = TRUE;
    return hr;
}

/***********************************************************************
* ShowPlaylist
* szPlaylistName: playlist name
* 
* This function use getByName to retrieve the playlist object
***********************************************************************/
HRESULT CMainDlg::ShowPlaylist(TCHAR *szPlaylistName)
{
    HRESULT                     hr = E_POINTER;
    CComPtr<IWMPPlaylistArray>  spPlaylistArray;
    CComPtr<IWMPPlaylist>       spPlaylist;

    if(szPlaylistName != NULL)
    {
        hr = m_spPC->getByName(CComBSTR(szPlaylistName), &spPlaylistArray);
    }

    // In full application, it should consider the possibility that a My Playlist
    // and an auto playlist may have the same name, so it may need to distinguish 
    // and choose proper index of item
    // As a sample application, we use 0 to get the first item
    if(SUCCEEDED(hr) && (spPlaylistArray.p != NULL))
    {
        hr = spPlaylistArray->item(0, &spPlaylist);
    }
    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowGetByAttr
* szAttrName: attribute name
* szAttrVal: attribute value
* szMediaType: media type
* 
* This function use getByAttribute() to retrieve the playlist object 
* that fullfil the conditions. It then show the playlist in the media list
***********************************************************************/
HRESULT CMainDlg::ShowGetByAttr(TCHAR *szAttrName, TCHAR *szAttrVal, TCHAR *szMediaType)
{
    HRESULT                     hr;
    CComPtr<IWMPPlaylist>       spPlaylist;

    hr = m_spMC->getByAttribute(CComBSTR(szAttrName), CComBSTR(szAttrVal), &spPlaylist);
    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist, szMediaType);
    }

    return hr;
}

/***********************************************************************
* ShowAllMedia
* 
* This function shows all media in media list 
***********************************************************************/
HRESULT CMainDlg::ShowAllMedia()
{
    HRESULT                 hr = S_OK;
    CComPtr<IWMPPlaylist>   spPlaylist;

    hr = m_spMC->getAll(&spPlaylist);
    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowNowPlaying
* 
* This function shows all media in current playlist 
***********************************************************************/
HRESULT CMainDlg::ShowNowPlaying()
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;

    if(m_spPlayer.p != NULL)
    {
        hr = m_spPlayer->get_currentPlaylist(&spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowAllMusic
* 
* This function shows all music items in media list 
***********************************************************************/
HRESULT CMainDlg::ShowAllMusic()
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;

    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getByAttribute(CComBSTR(_T("MediaType")), CComBSTR(_T("Audio")), &spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowAllMusicTree
* 
* This function updates the All Music node and its sub-tree  
***********************************************************************/
HRESULT CMainDlg::ShowAllMusicTree()
{
    HRESULT                 hr = S_OK;
    HTREEITEM               hPreChild, hCurChild;

    // Delete all child nodes
    hPreChild = TreeView_GetChild(m_hTree, m_hAllMusicNode);
    while(hPreChild != NULL)
    {
        hCurChild = hPreChild;
        hPreChild = TreeView_GetNextSibling(m_hTree, hPreChild);
        TreeView_DeleteItem(m_hTree, hCurChild);
    }

    // Now we add artist, album and genre nodes
    hr = ShowStringCollection(
        AddNode(_T("Artist"), TREE_MUSIC_ARTIST, m_hAllMusicNode), 
        TREE_MUSIC_ARTISTITEM, _T("Artist"), _T("Audio"));
    hr = ShowStringCollection(
        AddNode(_T("Album"), TREE_MUSIC_ALBUM, m_hAllMusicNode), 
        TREE_MUSIC_ALBUMITEM, _T("Album"), _T("Audio"));
    hr = ShowStringCollection(
        AddNode(_T("Genre"), TREE_MUSIC_GENRE, m_hAllMusicNode), 
        TREE_MUSIC_GENREITEM, _T("Genre"), _T("Audio"));

    return hr;
}

/***********************************************************************
* ShowAllVideo
* 
* This function shows all video items in media list 
***********************************************************************/
HRESULT CMainDlg::ShowAllVideo()
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;

    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getByAttribute(CComBSTR(_T("MediaType")), CComBSTR(_T("Video")), &spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowAllVideoTree
* 
* This function updates the All Video node and its sub-tree  
***********************************************************************/
HRESULT CMainDlg::ShowAllVideoTree()
{
    HRESULT                 hr = S_OK;
    HTREEITEM               hPreChild, hCurChild;

    // Delete all child nodes
    hPreChild = TreeView_GetChild(m_hTree, m_hAllVideoNode);
    while(hPreChild != NULL)
    {
        hCurChild = hPreChild;
        hPreChild = TreeView_GetNextSibling(m_hTree, hPreChild);
        TreeView_DeleteItem(m_hTree, hCurChild);
    }

    // Now we add actor and genre nodes
    ShowStringCollection(
        AddNode(_T("Actor"), TREE_VIDEO_ACTOR, m_hAllVideoNode), 
        TREE_VIDEO_ACTORITEM, _T("Actor"), _T("Video"));
    ShowStringCollection(
        AddNode(_T("Genre"), TREE_VIDEO_GENRE, m_hAllVideoNode), 
        TREE_VIDEO_GENREITEM, _T("Genre"), _T("Video"));

    return hr;
}

/***********************************************************************
* ShowPlaylistsTree
* 
* This function updates the My Playlists node and Auto Playlists node
***********************************************************************/
HRESULT CMainDlg::ShowPlaylistsTree()
{
    // Remove child nodes
    HTREEITEM                   hPreChild, hCurChild;
    
    hPreChild = TreeView_GetChild(m_hTree, m_hMyPlaylistsNode);
    while(hPreChild != NULL)
    {
        hCurChild = hPreChild;
        hPreChild = TreeView_GetNextSibling(m_hTree, hPreChild);
        TreeView_DeleteItem(m_hTree, hCurChild);
    }
    hPreChild = TreeView_GetChild(m_hTree, m_hAutoPlaylistwNode);
    while(hPreChild != NULL)
    {
        hCurChild = hPreChild;
        hPreChild = TreeView_GetNextSibling(m_hTree, hPreChild);
        TreeView_DeleteItem(m_hTree, hCurChild);
    }

    // Fill playlist node
    long                        i, lCount;
    CComPtr<IWMPPlaylist>       spPlaylist;
    CComPtr<IWMPPlaylistArray>  spPlaylistArray;
    CComBSTR                    bstrName, bstrAttr, bstrType;
    BOOL                        bIsAutoPlaylist;
    HRESULT                     hr = E_POINTER;

    USES_CONVERSION;
    
    bstrAttr = _T("PlaylistType");
    
    // Get all playlists
    if(m_spPC.p != NULL)
    {
        hr = m_spPC->getAll(&spPlaylistArray);
    }

    if(SUCCEEDED(hr) && (spPlaylistArray.p != NULL))
    {
        hr = spPlaylistArray->get_count(&lCount);
    }

    if(SUCCEEDED(hr))
    {
        // Enumerate the playlist
        for(i = 0 ; i < lCount; i++)
        {
            hr = spPlaylistArray->item(i, &spPlaylist);
            if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
            {
                bIsAutoPlaylist = FALSE;

                // Use PlaylistType attribute to tell whether it's a static playlist or auto playlist
                hr = spPlaylist->getItemInfo(bstrAttr, &bstrType);
                if(SUCCEEDED(hr) && (bstrType.m_str != NULL))
                {
                    bIsAutoPlaylist = !_tcsicmp(OLE2T(bstrType), _T("auto"));
                    bstrType.Empty();
                }
                hr = spPlaylist->get_name(&bstrName);
                if(SUCCEEDED(hr) && (bstrName.m_str != NULL))
                {
                    AddNode(OLE2T(bstrName), bIsAutoPlaylist? TREE_AUTOPLAYLISTITEM: TREE_MYPLAYLISTITEM, bIsAutoPlaylist? m_hAutoPlaylistwNode: m_hMyPlaylistsNode);
                    bstrName.Empty();
                }
                spPlaylist = NULL;
            }
        }
    }
    return hr;
}

/***********************************************************************
* ShowAllRadio
* 
* This function shows all radio items in media list 
***********************************************************************/
HRESULT CMainDlg::ShowAllRadio()
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;

    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getByAttribute(CComBSTR(_T("MediaType")), CComBSTR(_T("Radio")), &spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowOthers
* 
* This function shows all other items in media list 
***********************************************************************/
HRESULT CMainDlg::ShowOthers()
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;

    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getByAttribute(CComBSTR(_T("MediaType")), CComBSTR(_T("Other")), &spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        hr = ShowPlaylist(spPlaylist);
    }

    return hr;
}

/***********************************************************************
* ShowStringCollection
* hParent: handler to the parent node
* enumNode: the type of the node. it will be used for param of the node
* szAttr: attribute name
* szMediaType: media type
* 
* This function retrieves string collection for the given attribute 
* and the media type, then build the sub-tree under the parent node 
***********************************************************************/
HRESULT CMainDlg::ShowStringCollection(HTREEITEM hParent, NODENAME enumNode, TCHAR *szAttr, TCHAR *szMediaType)
{
    HRESULT                             hr = E_POINTER;
    CComPtr<IWMPStringCollection>       spStrColl;
    CComBSTR                            bstrName;
    long                                i, lCount;

    USES_CONVERSION;

    // Retrieve string collection
    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getAttributeStringCollection(CComBSTR(szAttr), CComBSTR(szMediaType), &spStrColl);
    }

    lCount = 0;
    if(SUCCEEDED(hr) && (spStrColl.p != NULL))
    {
        hr = spStrColl->get_count(&lCount);
    }

    if(SUCCEEDED(hr))
    {
        for(i = 0; i < lCount; i++)
        {
            hr = spStrColl->item(i, &bstrName);
            if(SUCCEEDED(hr) && (bstrName.m_str != NULL))
            {
                // Add a node for the attribute value
                if(bstrName.Length() != 0)
                {
                    AddNode(OLE2T(bstrName), enumNode, hParent);
                }
                else
                {
                    // Those items with blank attribute are treated as "unknown"
                    AddNode(_T("Unknown"), (NODENAME)(enumNode + 1), hParent);
                }
                bstrName.Empty();
            }
        }
    }

    return hr;
}

/***********************************************************************
* OnSelectMedia
* 
* This function is called when a user selects an item in the media list
* It then shows metadata in the metadata list
***********************************************************************/
LRESULT CMainDlg::OnSelectMedia(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int                 iIndex;
    long                lCount, i;
    CComPtr<IWMPMedia>  spMedia;
    HRESULT             hr;
    TCHAR               szEntry[MAX_BSTR_VERY_LONG + 1];
    CComBSTR            bstrName, bstrVal;
    
    iIndex = ::SendMessage(m_hList, LB_GETCURSEL, 0, 0);
    // Only when it is showing a playlist, then the selected item is a media item
    if((iIndex >= 0) && (m_spSavedPlaylist.p != NULL))
    {
        USES_CONVERSION;

        // Reset metadata list
        ::SendMessage(m_hMetadata, LB_RESETCONTENT, 0, 0);

        // List metadata
        hr = m_spSavedPlaylist->get_item(iIndex, &spMedia);

        lCount = 0;
        if(SUCCEEDED(hr) && (spMedia.p))
        {
            hr = spMedia->get_attributeCount(&lCount);
        }
        if(SUCCEEDED(hr))
        {
            for(i = 0; i < lCount; i++)
            {
                // Use getAttributeName and getItemInfo pair to list metadata
                hr = spMedia->getAttributeName(i, &bstrName);
                if(SUCCEEDED(hr) && (bstrName.m_str != NULL))
                {
                    hr = spMedia->getItemInfo(bstrName, &bstrVal);
                }
                if(SUCCEEDED(hr) && (bstrName.m_str != NULL) && (bstrVal.m_str != NULL))
                {
                    _stprintf(szEntry, _T("%s: %s"), OLE2T(bstrName), OLE2T(bstrVal));
                    ::SendMessage(m_hMetadata, LB_ADDSTRING, 0, (LPARAM)szEntry);
                    bstrName.Empty();
                    bstrVal.Empty();
                }
            }
        }
    }

    return 0;
}

/***********************************************************************
* OnDbClickMediaList
* 
* This function is called when a user double click an item in the media list
* It then plays the item
***********************************************************************/
LRESULT CMainDlg::OnDbClickMediaList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Retrieve the selected index
    int                 iIndex;
    iIndex = ::SendMessage(m_hList, LB_GETCURSEL, 0, 0);

    // If double clicking on blank area, just return
    if(iIndex == LB_ERR)
    {
        return 0;
    }

    HRESULT                     hr = E_FAIL;
    CComPtr<IWMPMedia>          spMedia;

    // Only when it is showing a playlist, then the selected item is a media item
    if((iIndex >= 0) && (m_spSavedPlaylist.p != NULL))
    {
        hr = m_spSavedPlaylist->get_item(iIndex, &spMedia);
    }

    if(SUCCEEDED(hr) && (spMedia.p != NULL))
    {
        hr = m_spPlayer->put_currentMedia(spMedia);
    }
    return 0;
}

/***********************************************************************
* ShowChildNodes
* hParent: the parent node user clicks on
*
* This function is called when a user clicks to show playlists/stringCollections
***********************************************************************/
void CMainDlg::ShowChildNodes(HTREEITEM hParent)
{
    HTREEITEM       hChild;
    TCHAR           szPlaylistName[MAX_BSTR_LONG + 1];
    TVITEM          tvi;
    int             iChildCount = 0;

    tvi.mask = TVIF_TEXT;
    tvi.pszText = szPlaylistName;
    tvi.cchTextMax = MAX_BSTR_LONG;

    ::SendMessage(m_hList, LB_RESETCONTENT, 0, 0);

    // Enumerate the child nodes and list them in  media list
    hChild = TreeView_GetChild(m_hTree, hParent);
    while(hChild != NULL)
    {
        iChildCount++;
        tvi.hItem = hChild;
        if(TreeView_GetItem(m_hTree, &tvi))
        {
            ::SendMessage(m_hList, LB_ADDSTRING, 0, (LPARAM)szPlaylistName);
        }
        hChild = TreeView_GetNextSibling(m_hTree, hChild);
    };

    // Release m_spSavedPlaylist
    m_spSavedPlaylist = NULL;

    // Reset metadata list
    ::SendMessage(m_hMetadata, LB_RESETCONTENT, 0, 0);

    TCHAR szLine[20];
    _stprintf(szLine, _T("%d items"), iChildCount);
    SetDlgItemText(IDC_STATUS, szLine);
}



#define WML_MEDIA_TYPE			L"MediaType"
#define WML_MEDIA_TYPE_AUDIO	L"Audio"
#define WML_MEDIA_TYPE_VIDEO	L"Video"
#define WML_MEDIA_TYPE_OTHER	L"Other"
#define WML_MEDIA_TYPE_RADIO	L"Radio"

#define WML_STRING_ARTIST		L"Artist"
#define WML_STRING_ACTOR		L"Actor"
#define WML_STRING_ALBUM		L"Album"
#define WML_STRING_GENRE		L"Genre"

#define CONTAINER_ID_DELIM				L"\\"
#define CONTAINER_ID_DELIM_LEN			1

#define CONTAINER_ID_ROOT				L"0"
#define CONTAINER_ID_ROOT_LEN			1

#define CONTAINER_ID_ALL_MEDIA			L"0\\All Media"
#define CONTAINER_ID_ALL_MEDIA_LEN		11

#define CONTAINER_ID_AUDIO				L"0\\All Music"
#define CONTAINER_ID_AUDIO_LEN			11

#define CONTAINER_ID_VIDEO				L"0\\All Video"
#define CONTAINER_ID_VIDEO_LEN			11

#define CONTAINER_ID_OTHER				L"0\\Other Media"
#define CONTAINER_ID_OTHER_LEN			13

#define CONTAINER_ID_MY_PLAYLISTS		L"0\\My Playlists"
#define CONTAINER_ID_MY_PLAYLISTS_LEN	14

#define CONTAINER_ID_AUTO_PLAYLISTS		L"0\\Auto Playlists"
#define CONTAINER_ID_AUTO_PLAYLISTS_LEN	16

#define CONTAINER_ID_RADIO				L"0\\Radio"
#define CONTAINER_ID_RADIO_LEN			7

#define CONTAINER_ID_ARTIST				L"\\Artist"
#define CONTAINER_ID_ARTIST_LEN			7

#define CONTAINER_ID_ACTOR				L"\\Actor"
#define CONTAINER_ID_ACTOR_LEN			6

#define CONTAINER_ID_ALBUM				L"\\Album"
#define CONTAINER_ID_ALBUM_LEN			6

#define CONTAINER_ID_GENRE				L"\\Genre"
#define CONTAINER_ID_GENRE_LEN			6

#define CONTAINER_NAME_ALL_MEDIA		L"All Media"
#define CONTAINER_NAME_ALL_MEDIA_LEN	9

#define CONTAINER_NAME_AUDIO			L"All Music"
#define CONTAINER_NAME_AUDIO_LEN		9

#define CONTAINER_NAME_VIDEO			L"All Video"
#define CONTAINER_NAME_VIDEO_LEN		9

#define CONTAINER_NAME_OTHER			L"Other Media"
#define CONTAINER_NAME_OTHER_len		11

#define CONTAINER_NAME_MY_PLAYLISTS		L"My Playlists"
#define CONTAINER_NAME_MY_PLAYLISTS_LEN	12

#define CONTAINER_NAME_AUTO_PLAYLISTS	L"Auto Playlists"
#define CONTAINER_NAME_AUTO_PLAYLISTS_LEN	14

#define CONTAINER_NAME_RADIO			L"Radio"
#define CONTAINER_NAME_RADIO_LEN		5

#define WML_ATTRIBUTE_AUTHOR			L"Author"
#define WML_ATTRIBUTE_TITLE				L"Title"

#define STR_MEDIA_TYPE_PLAYLIST	"PlaylistType"


enum Enum_WmlFolders
{
	WmlFolders_Undefined,

	WmlFolders_Root,

	WmlFolders_AllMedia,

	WmlFolders_Audio,
	WmlFolders_Audio_AllMusic,
	WmlFolders_Audio_Artist,
	WmlFolders_Audio_Album,
	WmlFolders_Audio_Genre,

	WmlFolders_Video,
	WmlFolders_Video_AllVideo,
	WmlFolders_Video_Actor,
	WmlFolders_Video_Genre,

	WmlFolders_Other,
	WmlFolders_Other_AllOther,
	WmlFolders_Other_Genre
};

enum Enum_WmlObjTypes
{
	WmlObjType_Unknown,
	
	WmlObjType_MediaCollection,
	WmlObjType_PlaylistCollection,
	WmlObjType_PlaylistArray,
	WmlObjType_Playlist,
	WmlObjType_Media,
	WmlObjType_StringCollection,

	WmlObjType_Containers
};

enum WmlObjectFlags
{
	WmlObjFlag_IsValid			= 0x01,
	WmlObjFlag_IsContainer		= 0x02
};


struct WmlObject
{
	/* indicates the parent folder of the */
	enum Enum_WmlFolders	BaseFolder;

	/* name of the media object, relative to the base folder */
	unsigned short			*Name;

	/* if nonzero, then the object is a container */
	unsigned int			Flags;

	/*
	 *	Windows Media Library has several forms of objects for results. 
	 *	This field indicates how the object is represented through
	 *	one of the Results_xxx fields.
	 */
	enum Enum_WmlObjTypes	ResultsType;

	IWMPMediaCollection**		Results_MediaCollection;
	IWMPPlaylistCollection**	Results_PlaylistCollection;
	IWMPPlaylistArray**			Results_PlaylistArray;
	IWMPPlaylist**				Results_Playlist;
	IWMPStringCollection**		Results_StringCollection;
	IWMPMedia**					Results_Media;
	const enum Enum_WmlFolders*	Results_Containers;
};

const enum Enum_WmlFolders AudioContainers[] = 
{ 
	WmlFolders_Audio_AllMusic,
	WmlFolders_Audio_Artist,
	WmlFolders_Audio_Album,
	WmlFolders_Audio_Genre
};

const enum Enum_WmlFolders VideoContainers[] = 
{ 
	WmlFolders_Video_AllVideo,
	WmlFolders_Video_Actor,
	WmlFolders_Video_Genre
};

void DoHelper_FindMediaInPlaylist(const wchar_t *objectID, IWMPPlaylist **playlist, int *found, IWMPMedia **media)
{
	HRESULT hr;
	long pli = 0;
	long plCount;
	CComBSTR				mediaName;
	int cmp;

	/* Attempt to find a match in the playlist*/
	hr = (*playlist)->get_count(&plCount);
	if (SUCCEEDED(hr))
	{
		for (pli =0; pli < plCount; pli++)
		{
			hr = (*playlist)->get_item(pli, media);
			if (SUCCEEDED(hr))
			{
				hr = (*media)->get_name(&mediaName);
				if (SUCCEEDED(hr))
				{
					cmp = wcscmp(OLE2W(mediaName), objectID);
					if (cmp == 0)
					{
						*found = 1;
						return;
					}
				}
			}
		}
	}

	/* no match was found */
	*found = 0;
}

void DoHelper_FindStringInStringCollection(const wchar_t *objectID, const wchar_t *delim, IWMPStringCollection *stringCollection, long *index)
{
	// TODO
}


/*
 *	PRECONDITIONS:
 *					All of wmlObj fields are null or set to their default state.
 */
void DoHelper_GetAllMedia(const wchar_t *objectID, IWMPMediaCollection **mediaCollection, struct WmlObject *wmlObj)
{
	HRESULT hr;
	int found;
	
	CComPtr<IWMPMedia>		media;

	/* grab all media */
	hr = (*mediaCollection)->getAll(wmlObj->Results_Playlist);

	if (SUCCEEDED(hr))
	{
		if (wcslen(objectID) == CONTAINER_ID_ALL_MEDIA_LEN)
		{
			/*
			 *	Specified objectID maps to CONTAINER_ID_ALL_MEDIA/WmlFolders_AllMedia,
			 *	whose children are described through a single playlist
			 *	of all known items in the media collection.
			 */

			wmlObj->BaseFolder			= WmlFolders_AllMedia;
			wmlObj->Flags				|= WmlObjFlag_IsContainer;
			wmlObj->Flags				|= WmlObjFlag_IsValid;
			wmlObj->ResultsType			= WmlObjType_Playlist;
		}
		else 
		{
			/*
			 *	Specified objectID maps to a descendent of 
			 *	CONTAINER_ID_ALL_MEDIA/WmlFolders_AllMedia.
			 * 	We know that all descendents of WmlFolders_AllMedia 
			 *	are direct children of WmlFolders_AllMedia,
			 *	so attempt to find a matching media in the playlist.
			 */

			DoHelper_FindMediaInPlaylist
				(
				objectID + CONTAINER_ID_ALL_MEDIA_LEN + CONTAINER_ID_DELIM_LEN, 
				wmlObj->Results_Playlist,
				&found,
				wmlObj->Results_Media
				);

			if (found != 0) 
			{ 
				/* if a match was found, assign the results */
				wmlObj->BaseFolder			= WmlFolders_AllMedia;
				wmlObj->Flags				|= WmlObjFlag_IsValid;
				wmlObj->Name				= (unsigned short*) malloc (wcslen(objectID)+1);
				wcscpy(wmlObj->Name, objectID + CONTAINER_ID_ALL_MEDIA_LEN + CONTAINER_ID_DELIM_LEN);
				wmlObj->ResultsType			= WmlObjType_Media;
			}
		}
	}
}


struct WmlObject* DoHelper_GetWmlObject(IWMPPlayer4 *player, const wchar_t *objectID, IWMPMediaCollection **mediaCollection, IWMPPlaylistCollection **playlistCollection, IWMPPlaylistArray **playlistArray, IWMPPlaylist **playlist, IWMPStringCollection **stringCollection, IWMPMedia **media)
{
	HRESULT hr;
	struct WmlObject *wmlObj;

	/* initialize wmlObj fields */
	wmlObj = (struct WmlObject*) malloc(sizeof(struct WmlObject));
	wmlObj->BaseFolder					= WmlFolders_Undefined;
	wmlObj->Name						= NULL;
	wmlObj->Flags						= 0;
	wmlObj->ResultsType					= WmlObjType_Unknown;
	wmlObj->Results_MediaCollection		= mediaCollection;
	wmlObj->Results_PlaylistCollection	= playlistCollection;
	wmlObj->Results_PlaylistArray		= playlistArray;
	wmlObj->Results_Playlist			= playlist;
	wmlObj->Results_StringCollection	= stringCollection;
	wmlObj->Results_Media				= media;
	wmlObj->Results_Containers			= NULL;

	hr = player->get_mediaCollection(wmlObj->Results_MediaCollection);
	if (SUCCEEDED(hr))
	{
		if (wcscmp(objectID, CONTAINER_ID_ROOT) == 0)
		{
			/* root container represented through mediaCollection */
			wmlObj->BaseFolder				= WmlFolders_Root;
			wmlObj->Flags					|= WmlObjFlag_IsContainer;
			wmlObj->ResultsType				= WmlObjType_MediaCollection;
		}
		else if (wcsncmp(objectID, CONTAINER_ID_ALL_MEDIA, CONTAINER_ID_ALL_MEDIA_LEN) == 0)
		{
			DoHelper_GetAllMedia(objectID, wmlObj->Results_MediaCollection, wmlObj);
		}
		else if (wcsncmp(objectID, CONTAINER_ID_AUDIO, CONTAINER_ID_AUDIO_LEN) == 0)
		{
			//DoHelper_GetAudio(objectID, &(wmlObj->Results_MediaCollection), wmlObj);
		}
		else if (wcsncmp(objectID, CONTAINER_ID_VIDEO, wcslen(CONTAINER_ID_VIDEO)) == 0)
		{
		}
		else if (wcsncmp(objectID, CONTAINER_ID_OTHER, wcslen(CONTAINER_ID_OTHER)) == 0)
		{
		}
		else if (wcsncmp(objectID, CONTAINER_ID_MY_PLAYLISTS, wcslen(CONTAINER_ID_MY_PLAYLISTS)) == 0)
		{
		}
		else if (wcsncmp(objectID, CONTAINER_ID_AUTO_PLAYLISTS, wcslen(CONTAINER_ID_AUTO_PLAYLISTS)) == 0)
		{
		}
		else if (wcsncmp(objectID, CONTAINER_ID_RADIO, wcslen(CONTAINER_ID_RADIO)) == 0)
		{
		}
	}

	return wmlObj;
}

void DoHelper_WmlToCds_AsMedia(struct CdsMediaObject *cdsObj, struct WmlObject *wmlObj)
{
	CComBSTR bstr;
	int size;

	/* assign creator */
	(*(wmlObj->Results_Media))->getItemInfo(WML_ATTRIBUTE_AUTHOR, &bstr);
	size = (int) (bstr.Length()+1);
	cdsObj->Creator = (char*) malloc(size);
	strToUtf8(cdsObj->Creator, (const char*) OLE2W(bstr), size, 1);

	/* always restricted and never searchable */
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	/* assign ID and parentID*/
	switch(wmlObj->BaseFolder)
	{
	case WmlFolders_AllMedia:
		bstr.Empty();
		bstr.Append(CONTAINER_ID_ALL_MEDIA);
		break;
		
	default:
		fprintf(stderr, "DoHelper_WmlObjToCdsObj() has a logic error.\r\n");
		_ASSERT(1==2);
		break;
	}

	/* assign parentID */
	size = (int) (bstr.Length()+1);
	cdsObj->ParentID = (char*) malloc(size);
	strToUtf8(cdsObj->ParentID, (const char*) OLE2W(bstr), size, 1);

	/* assign ID */
	bstr.Append(CONTAINER_ID_DELIM);
	bstr.Append(wmlObj->Name);
	size = (int) (bstr.Length()+1);
	cdsObj->ID = (char*) malloc(size);
	strToUtf8(cdsObj->ID, (const char*) OLE2W(bstr), size, 1);


	/* assign media class */
	(*(wmlObj->Results_Media))->getItemInfo(WML_MEDIA_TYPE, &bstr);
	if (wcsicmp(OLE2W(bstr), WML_MEDIA_TYPE_AUDIO) == 0)
	{
		cdsObj->MediaClass = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (wcsicmp(OLE2W(bstr), WML_MEDIA_TYPE_VIDEO) == 0)
	{
		cdsObj->MediaClass = CDS_MEDIACLASS_VIDEOITEM;
	}
	else
	{
		cdsObj->MediaClass = CDS_MEDIACLASS_ITEM;
	}

	//TODO
	cdsObj->RefID = NULL;
	cdsObj->Res = NULL;

	/* assign title */
	(*(wmlObj->Results_Media))->getItemInfo(WML_ATTRIBUTE_TITLE, &bstr);
	size = (int) (bstr.Length()+1);
	cdsObj->Title = (char*) malloc(size);
	strToUtf8(cdsObj->Title, (const char*) OLE2W(bstr), size, 1);
}

void DoHelper_RespondWithChildren(struct MSL_BrowseArgs *browseArgs, struct WmlObject *wmlObj)
{
	struct CdsMediaObject cdsObj;
	long plCount;
	long pi;
	HRESULT hr;
	unsigned int filter;
	int errorOrSize;
	char *didl;

	switch (wmlObj->ResultsType)
	{
	case WmlObjType_Playlist:
		(*(wmlObj->Results_Playlist))->get_count(&plCount);
		for (pi = 0; pi < plCount; pi++)
		{
			hr = (*(wmlObj->Results_Playlist))->get_item(pi, wmlObj->Results_Media);
			if (SUCCEEDED(hr))
			{
				DoHelper_WmlToCds_AsMedia(&cdsObj, wmlObj);
				filter = CdsToDidl_GetFilterBitString(browseArgs->Filter);
				didl = CdsToDidl_GetMediaObjectDidlEscaped(&cdsObj, filter, 0, &errorOrSize);

				if (errorOrSize >= 0)
				{
					MSL_ForResponse_RespondBrowse_ResultArgument(browseArgs, didl, strlen(didl));
				}

				free(didl);
			}
		}
		break;

	default:
		fprintf(stderr, "DoHelper_RespondWithChildren() has a logic error.");
		_ASSERT(0==1);
		break;
	}
}

DWORD WINAPI Perform_UpnpBrowse(LPVOID lParam)
{
	struct MSL_BrowseArgs *browseArgs;
	struct StringBrowseArgsWide *wideArgs;
	struct WmlObject *wmlObj;
	unsigned int numberReturned=0, totalMatches=0, updateID=0;

	CoInitialize(NULL);

	CComPtr<IWMPMediaCollection>	results_MediaCollection;
	CComPtr<IWMPPlaylistCollection>	results_PlaylistCollection;
	CComPtr<IWMPPlaylistArray>		results_PlaylistArray;
	CComPtr<IWMPPlaylist>			results_Playlist;
	CComPtr<IWMPStringCollection>	results_StringCollection;
	CComPtr<IWMPMedia>				results_Media;

	browseArgs = (struct MSL_BrowseArgs*) lParam;
	wideArgs = (struct StringBrowseArgsWide*) browseArgs->UserObject;


	wmlObj = DoHelper_GetWmlObject
		(
		MainDlg->m_spPlayer,
		wideArgs->ObjectID,
		&results_MediaCollection,
		&results_PlaylistCollection,
		&results_PlaylistArray,
		&results_Playlist,
		&results_StringCollection,
		&results_Media
		);

	if (wmlObj->Flags & WmlObjFlag_IsValid)
	{
		if (browseArgs->BrowseDirectChildren == 0)
		{
			//TODO
			MSL_ForResponse_RespondError(browseArgs, CDS_EC_OBJECT_ID_NO_EXIST, CDS_EM_OBJECT_ID_NO_EXIST);
		}
		else
		{
			if (wmlObj->Flags & WmlObjFlag_IsContainer)
			{
				switch (wmlObj->ResultsType)
				{
				case WmlObjType_Playlist:
					MSL_ForResponse_RespondBrowse_StartResponse(browseArgs, 1);

					DoHelper_RespondWithChildren(browseArgs, wmlObj);

					MSL_ForResponse_RespondBrowse_FinishResponse(browseArgs, 1, numberReturned, totalMatches, updateID);
					break;

				default:
					MSL_ForResponse_RespondError(browseArgs, CDS_EC_ACTION_FAILED, CDS_EM_ACTION_FAILED);
					break;
				}
			}
			else
			{
				MSL_ForResponse_RespondError(browseArgs, CDS_EC_NO_SUCH_CONTAINER, CDS_EM_NO_SUCH_CONTAINER);
			}
		}
	}
	else
	{
		MSL_ForResponse_RespondError(browseArgs, CDS_EC_OBJECT_ID_NO_EXIST, CDS_EM_OBJECT_ID_NO_EXIST);
	}

	if (wmlObj != NULL)
	{
		/* release wmlObj resources */
		if (wmlObj->Name) { free(wmlObj->Name); }
		free(wmlObj);
	}

	free(wideArgs->Filter);
	free(wideArgs->ObjectID);
	free(wideArgs->SortCriteria);
	browseArgs->UserObject = NULL;
	MSL_DeallocateBrowseArgs(&browseArgs);

	CoUninitialize();

	return 0;
}

LRESULT CMainDlg::DoUpnpBrowse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CreateThread(NULL, NULL, Perform_UpnpBrowse, (LPVOID)lParam, NULL, NULL);
	return 0;
}

//LRESULT CMainDlg::DoUpnpBrowse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	struct MSL_BrowseArgs *browseArgs;
//	struct StringBrowseArgsWide *wideArgs;
//	struct WmlObject *wmlObj;
//	unsigned int numberReturned=0, totalMatches=0, updateID=0;
//
//
//	CComPtr<IWMPMediaCollection>	results_MediaCollection;
//	CComPtr<IWMPPlaylistCollection>	results_PlaylistCollection;
//	CComPtr<IWMPPlaylistArray>		results_PlaylistArray;
//	CComPtr<IWMPPlaylist>			results_Playlist;
//	CComPtr<IWMPStringCollection>	results_StringCollection;
//	CComPtr<IWMPMedia>				results_Media;
//
//	browseArgs = (struct MSL_BrowseArgs*) lParam;
//	wideArgs = (struct StringBrowseArgsWide*) browseArgs->UserObject;
//
//
//	wmlObj = DoHelper_GetWmlObject
//		(
//		this->m_spPlayer, 
//		wideArgs->ObjectID,
//		&results_MediaCollection,
//		&results_PlaylistCollection,
//		&results_PlaylistArray,
//		&results_Playlist,
//		&results_StringCollection,
//		&results_Media
//		);
//
//	if (wmlObj->Flags & WmlObjFlag_IsValid)
//	{
//		if (browseArgs->BrowseDirectChildren == 0)
//		{
//			//TODO
//			MSL_ForResponse_RespondError(browseArgs, CDS_EC_OBJECT_ID_NO_EXIST, CDS_EM_OBJECT_ID_NO_EXIST);
//		}
//		else
//		{
//			if (wmlObj->Flags & WmlObjFlag_IsContainer)
//			{
//				switch (wmlObj->ResultsType)
//				{
//				case WmlObjType_Playlist:
//					MSL_ForResponse_RespondBrowse_StartResponse(browseArgs, 1);
//
//					DoHelper_RespondWithChildren(browseArgs, wmlObj);
//
//					MSL_ForResponse_RespondBrowse_FinishResponse(browseArgs, 1, numberReturned, totalMatches, updateID);
//					break;
//
//				default:
//					MSL_ForResponse_RespondError(browseArgs, CDS_EC_ACTION_FAILED, CDS_EM_ACTION_FAILED);
//					break;
//				}
//			}
//			else
//			{
//				MSL_ForResponse_RespondError(browseArgs, CDS_EC_NO_SUCH_CONTAINER, CDS_EM_NO_SUCH_CONTAINER);
//			}
//		}
//	}
//	else
//	{
//		MSL_ForResponse_RespondError(browseArgs, CDS_EC_OBJECT_ID_NO_EXIST, CDS_EM_OBJECT_ID_NO_EXIST);
//	}
//
//	if (wmlObj != NULL)
//	{
//		/* release wmlObj resources */
//		if (wmlObj->Name) { free(wmlObj->Name); }
//		free(wmlObj);
//	}
//
//	free(wideArgs->Filter);
//	free(wideArgs->ObjectID);
//	free(wideArgs->SortCriteria);
//	browseArgs->UserObject = NULL;
//	MSL_DeallocateBrowseArgs(&browseArgs);
//
//	return 0;
//}
