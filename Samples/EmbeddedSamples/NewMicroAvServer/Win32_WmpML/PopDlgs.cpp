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
#include "PopDlgs.h"
#include "CommDlg.h"

/**********************************************************
*  CAddMediaDlg: 
*    pop dialog for adding URL/path to media library
*
***********************************************************/
CAddMediaDlg::CAddMediaDlg(IWMPMediaCollection *pMC)
{
    m_spMC = pMC;
}

CAddMediaDlg::~CAddMediaDlg()
{
}

LRESULT CAddMediaDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CComBSTR            bstrURL, bstrMediaType;
    CComPtr<IWMPMedia>  spMedia;
    HRESULT             hr = E_POINTER;
    LRESULT             iRetCode;
    
    // Add the URL using mediaCollection.add()
    GetDlgItemText(IDC_URL, bstrURL.m_str);
    if(m_spMC.p != NULL)
    {
        hr = m_spMC->add(bstrURL, &spMedia);
    }

    if(SUCCEEDED(hr) && (spMedia.p != NULL))
    {
        hr = spMedia->getItemInfo(CComBSTR(_T("MediaType")), &bstrMediaType);
    }

    // Based on the media type, return different media type as return code
    // So the main dialog can update different node in the tree view
    USES_CONVERSION;

    if(SUCCEEDED(hr) && (bstrMediaType.m_str != NULL))
    {
        iRetCode = TellMediaType(OLE2T(bstrMediaType));
    }
    else
    {
        iRetCode = INVALID;
    }

    EndDialog(iRetCode);
    return 0;
}

LRESULT CAddMediaDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CAddMediaDlg::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    OPENFILENAME    ofn;
    TCHAR szFileName[MAX_PATH];

    // display open dialog
    memset(&ofn, 0, sizeof(ofn)); // initialize structure to 0/NULL
    szFileName[0] = _T('\0');

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrFilter = _T("All files\0*.*\0");
    ofn.nFilterIndex = 0; 
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = _Module.GetResourceInstance();

    if (GetOpenFileName(&ofn))
    {
        SetDlgItemText(IDC_URL, szFileName);
    }

    return 0;
}

/**********************************************************
*  CAddMediaDlg: 
*    pop dialog for deleting media from media library
*
***********************************************************/
CDelMediaDlg::CDelMediaDlg(IWMPMediaCollection *pMC)
{
    m_spMC = pMC;
}

CDelMediaDlg::~CDelMediaDlg()
{
}

// When the dialog is initialized, it lists all the media items in the combo-box
// so that the user can pick which to delete
LRESULT CDelMediaDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT             hr = E_POINTER;
    CComPtr<IWMPMedia>  spMedia;
    CComBSTR            bstrName;
    long                i, lCount;

    USES_CONVERSION;

    if(m_spMC.p != NULL)
    {
        hr = m_spMC->getAll(&m_spPlaylist);
    }

    // Populate the combo-box
    if(SUCCEEDED(hr) && (m_spPlaylist.p != NULL))
    {
        hr = m_spPlaylist->get_count(&lCount);
    }

    if(SUCCEEDED(hr))
    {
        for(i = 0; i < lCount; i++)
        {
            hr = m_spPlaylist->get_item(i, &spMedia);
            if(SUCCEEDED(hr) && (spMedia.p != NULL))
            {
                hr = spMedia->get_name(&bstrName);
                spMedia = NULL;
            }
            if(SUCCEEDED(hr) && (bstrName.m_str != NULL))
            {
                SendDlgItemMessage(IDC_MEDIALIST, LB_ADDSTRING, 0, (LPARAM)OLE2T(bstrName));
                bstrName.Empty();
            }
        }
    }
    return 0;
}

LRESULT CDelMediaDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT             hr = E_POINTER;
    CComPtr<IWMPMedia>  spMedia;
    int                 iIndex;
    CComBSTR            bstrMediaType;
    LRESULT             iRetCode;

    iIndex = SendDlgItemMessage(IDC_MEDIALIST, LB_GETCURSEL, 0, 0);
    if((iIndex >= 0) && (m_spPlaylist.p != NULL))
    {
        // Use mediaCollection.remove to delete the selected item
        hr = m_spPlaylist->get_item(iIndex, &spMedia);
        if(SUCCEEDED(hr) && (spMedia.p != NULL))
        {
            hr = m_spMC->remove(spMedia, VARIANT_FALSE);
        }

        if(SUCCEEDED(hr) && (spMedia.p != NULL))
        {
            hr = spMedia->getItemInfo(CComBSTR(_T("MediaType")), &bstrMediaType);
        }

        // Based on the media type, return different media type as return code
        // So the main dialog can update different node in the tree view
        USES_CONVERSION;

        if(SUCCEEDED(hr) && (bstrMediaType.m_str != NULL))
        {
            iRetCode = TellMediaType(OLE2T(bstrMediaType));
        }
        else
        {
            iRetCode = INVALID;
        }

    }

    EndDialog(iRetCode);
    return 0;
}

LRESULT CDelMediaDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

/**********************************************************
*  CDelPlaylistDlg: 
*    pop dialog for deleting playlist from media library
*
***********************************************************/
CDelPlaylistDlg::CDelPlaylistDlg(IWMPPlaylistCollection *pPC)
{
    m_spPC = pPC;
}

CDelPlaylistDlg::~CDelPlaylistDlg()
{
}

// When the dialog is initialized, it lists all the playlists in the combo-box
// so that the user can pick which one to delete
LRESULT CDelPlaylistDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;
    CComBSTR                bstrName;
    long                    i, lCount;

    USES_CONVERSION;

    if(m_spPC.p != NULL)
    {
        hr = m_spPC->getAll(&m_spPlaylistArray);
    }

    // Populate the combo-box
    lCount = 0;
    if(SUCCEEDED(hr) && (m_spPlaylistArray.p != NULL))
    {
        hr = m_spPlaylistArray->get_count(&lCount);
    }

    if(SUCCEEDED(hr))
    {
        for(i = 0; i < lCount; i++)
        {
            hr = m_spPlaylistArray->item(i, &spPlaylist);
            if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
            {
                hr = spPlaylist->get_name(&bstrName);
                spPlaylist = NULL;
            }
            if(SUCCEEDED(hr) && (bstrName.m_str != NULL))
            {
                SendDlgItemMessage(IDC_PLAYLISTS, LB_ADDSTRING, 0, (LPARAM)OLE2T(bstrName));
                bstrName.Empty();
            }
        }
    }
    return 0;
}

LRESULT CDelPlaylistDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT                 hr = E_POINTER;
    CComPtr<IWMPPlaylist>   spPlaylist;
    int                     iIndex;
    int                     iRetCode = INVALID;

    iIndex = SendDlgItemMessage(IDC_PLAYLISTS, LB_GETCURSEL, 0, 0);
    if((iIndex >= 0) && (m_spPlaylistArray.p != NULL))
    {
        // Use playlistCollection.remove to delete the selected playlist
        hr = m_spPlaylistArray->item(iIndex, &spPlaylist);
        if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
        {
            hr = m_spPC->remove(spPlaylist);
        }
        if(SUCCEEDED(hr))
        {
            iRetCode = PLAYLIST;
        }
        else
        {
            iRetCode = INVALID;
        }
    }
    else
    {
        iRetCode = INVALID;
    }

    EndDialog(iRetCode);
    return 0;
}

LRESULT CDelPlaylistDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

/**********************************************************
*  CImportPlaylistDlg: 
*    pop dialog for importing a playlist file into media library
*
***********************************************************/
CImportPlaylistDlg::CImportPlaylistDlg(IWMPPlayer4* pPlayer)
{
    m_spPlayer = pPlayer;
}

LRESULT CImportPlaylistDlg::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    OPENFILENAME    ofn;
    TCHAR szFileName[MAX_PATH];

    // display open dialog
    memset(&ofn, 0, sizeof(ofn)); // initialize structure to 0/NULL
    szFileName[0] = _T('\0');

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrFilter = _T("All files\0*.*\0");
    ofn.nFilterIndex = 0; 
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = _Module.GetResourceInstance();

    if (GetOpenFileName(&ofn))
    {
        SetDlgItemText(IDC_URL, szFileName);
    }

    return 0;
}

LRESULT CImportPlaylistDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CComBSTR                        bstrURL, bstrName, bstrType;
    CComPtr<IWMPPlaylistCollection> spPC;
    CComPtr<IWMPPlaylist>           spPlaylist, spPlaylistRtn;
    HRESULT                         hr = E_POINTER;
    LRESULT                         iRetCode;

    GetDlgItemText(IDC_URL, bstrURL.m_str);
    GetDlgItemText(IDC_NAME, bstrName.m_str);
    
    USES_CONVERSION;

    if(m_spPlayer.p != NULL)
    {
        hr = m_spPlayer->newPlaylist(bstrName, bstrURL, &spPlaylist);
    }

    if(SUCCEEDED(hr) && (spPlaylist.p != NULL))
    {
        // If newPlaylist succeeds, it's a static playlist
        hr = m_spPlayer->get_playlistCollection(&spPC);
        if(SUCCEEDED(hr) && (spPC.p != NULL))
        {
            hr = spPC->importPlaylist(spPlaylist, &spPlaylistRtn); 
        }
        iRetCode = SUCCEEDED(hr)? PLAYLIST : INVALID;
    }
    else
    {
        iRetCode = INVALID;
    }
    
    EndDialog(iRetCode);
    return 0;
}

LRESULT CImportPlaylistDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

/**********************************************************
*  CNewPlaylistDlg: 
*    pop dialog for creating a new playlist into media library
*
***********************************************************/
CNewPlaylistDlg::CNewPlaylistDlg(IWMPPlaylistCollection *pPC)
{
    m_spPC = pPC;
}

LRESULT CNewPlaylistDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CComBSTR                    bstrName;
    CComPtr<IWMPPlaylistArray>  spPlaylistArray;
    CComPtr<IWMPPlaylist>       spPlaylist;
    long                        lCount = 0;
    HRESULT                     hr = E_POINTER;

    GetDlgItemText(IDC_PLAYLISTNAME, bstrName.m_str);

    // Use getByName to search for duplicate playlist name
    if(m_spPC.p != NULL)
    {
        hr = m_spPC->getByName(bstrName, &spPlaylistArray);
    }

    lCount = 0;
    if(SUCCEEDED(hr) && (spPlaylistArray.p != NULL))
    {
        hr = spPlaylistArray->get_count(&lCount);
    }

    if(SUCCEEDED(hr))
    {
        if(lCount == 0)
        {
            // When the playlist is not existent, we create a new playlist
            hr = m_spPC->newPlaylist(bstrName, &spPlaylist);
            EndDialog(SUCCEEDED(hr)? PLAYLIST : INVALID);
        }
        else
        {
            // Warn user that the playlist is already there
            MessageBox(_T("This playlist exists. Please use other name"));
        }
    }

    return 0;
}

LRESULT CNewPlaylistDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

