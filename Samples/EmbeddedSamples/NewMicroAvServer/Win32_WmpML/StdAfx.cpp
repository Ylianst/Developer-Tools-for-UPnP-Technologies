//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

/***********************************************************************
* TellMediaType
* 
* This function is used to convert media type string to enum
***********************************************************************/
MEDIATYPE TellMediaType(TCHAR *szMediaType)
{
    MEDIATYPE iRetCode = INVALID;
    if(szMediaType == NULL)
    {
        iRetCode = INVALID;
    }
    else
    {
        if(!_tcsicmp(szMediaType, _T("audio")))
        {
            iRetCode = MUSIC;
        }
        else if(!_tcsicmp(szMediaType, _T("video")))
        {
            iRetCode = VIDEO;
        }
        else if(!_tcsicmp(szMediaType, _T("")))
        {
            iRetCode = OTHER;
        }
        else if(!_tcsicmp(szMediaType, _T("playlist")))
        {
            iRetCode = PLAYLIST;
        }
        else if(!_tcsicmp(szMediaType, _T("radio")))
        {
            iRetCode = RADIO;
        }
        else
        {
            iRetCode = INVALID;
        }
    }
    
    return iRetCode;
}
