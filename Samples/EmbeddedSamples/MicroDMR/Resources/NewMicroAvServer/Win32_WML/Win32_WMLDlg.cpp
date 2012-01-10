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

#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include "stdafx.h"
#include "Win32_WML.h"
#include "Win32_WMLDlg.h"
#include <wmp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// UPnP Includes
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

// POSIX-style synchronization
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

// UPnP Types
CLSID const CLSID_MEDIAPLAYER = { 0x6BF52A52, 0x394A, 0x11D3, { 0xB1, 0x53, 0x0, 0xC0, 0x4F, 0x79, 0xFA, 0xA6 } };

/*
 *	Encapsulates input arguments for a Browse request.
 *	All string data is in wide form.
 */
struct CdsStringBrowseArgsWide
{
	/*
	 *	ObjectID specified by the control point.
	 */
	wchar_t *ObjectID;

	/*
	 *	Metadata filter settings. Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
	 *	that describe what CDS metadata to return in the response. In this framework,
	 *	the application layer is responsible for enforcing metadata filtering.
	 */
	wchar_t *Filter;

	/*
	 *	SortCriteria strings have the following form:
	 *	[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 
	 */
	wchar_t *SortCriteria;
};

enum CdsRequestTypes
{
	CdsRequestType_Browse
};

struct CdsRequest
{
	struct CdsRequest *Next;
	enum CdsRequestTypes Type;
	int RequestedOnPort;
	int RequestedOnAddress;
	void *Args;
};

enum Enum_WmlObjTypes 
{
	WmlObjType_Undefined, 
	WmlObjType_Media,
	WmlObjType_Playlist,
	WmlObjType_StringCollection,
	WmlObjType_FilteredPlaylist,
	WmlObjType_PlaylistArray,

	WmlObjType_WmlObjArray
};

enum Enum_WmlFolders;

struct WML_Object
{
	/* use new and delete */
	enum Enum_WmlObjTypes			Type;

	/* used when Type==WmlObjType_Media */
	CComPtr <IWMPMedia>				Media;

	/* used when Type==WmlObjType_Playlist or WmlObjType_FilteredPlaylist */
	CComPtr <IWMPPlaylist>			Playlist;

	/* used when Type==WmlObjType_StringCollection */
	CComPtr <IWMPStringCollection>	StringCollection;


	/*
	 *	Next and WmlObjArray are used in a mutually exclusive manner. 
	 */

	/* Used whenever we have a dynamic number of WML_Objects */
	//struct WML_Object*				Next;

	/*
	 *	Used only for standard containers that are predefined.
	 *	Used if and only if Type==WmlObjType_WmlObjArray.
	 */
	struct WML_Object**				WmlObjArray;
	int								WmlObjArrayLen;
	enum Enum_WmlFolders			ObjId;


	/*
	 *	used when Type==WmlObjType_FilteredPlaylist 
	 */

	/*	Attribute to filter for. Assign WML_ATTRIBUTE_xxx string */
	wchar_t							*Attribute;

	/*	The value that the attribute should be for the playlist element */
	wchar_t							*AttributeValue;

	/*
	 *	used when Type==WmlObjType_PlaylistArray
	 */
	CComPtr<IWMPPlaylistArray>		PlaylistArray;
	int								PlaylistArrayIsAutoType;
};

struct WML_CachePath
{
	/* use new and delete */
	CStringW Name;
	struct WML_CachePath *Next;
};

struct WML_ObjId
{
	enum Enum_WmlFolders BaseFolder;
	wchar_t *SubFolder;
	wchar_t *Source;

	// contains the portion of "Source" that can be
	// found through the SourcePrefixIndex.
	wchar_t *SourcePrefix;
	int		 SourcePrefixIndex;

	// do not deallocate: points to a wchar_t in Source.
	// Position is for the file's 'name' specified after the prefix.
	wchar_t	*_SourcePostfix;

	// Source can point to a file like an M3U playlist.
	// EmbeddedSource and similar variables point to
	// an individual track within the playlist.
	wchar_t *EmbeddedSource;
	wchar_t	*EmbeddedSourcePrefix;
	int		 EmbeddedSourcePrefixIndex;
	int		 EmbeddedTrackIndex;
	
	// do not deallocate: points to a wchar_t in Source
	// Position is for the file's 'name' specified after the prefix.
	wchar_t	*_EmbeddedSourcePostfix;

};

// UPnP Methods
void Upnp_IPAddressMonitor(void *data);
unsigned long WINAPI Upnp_Main(void* ptr);
void Upnp_OnBrowseSearch (struct MSL_CdsQuery *browseArgs);

// Windows Media Library MediaServer methods

void WML_CdsRequestsClear();
void WML_DeallocateBrowseRequest(struct CdsRequest *request);
struct CdsRequest* WML_CdsRequestsEnqueue(struct CdsRequest *req);
struct CdsRequest* WML_CdsRequestsDequeue();

void WML_CachePathAdd(CStringW dirName);
void WML_CachePathClear();
int WML_CachePathGetByIndex(int index, CStringW *outVal);
int WML_CachePathGetByValue(CStringW val);
void WML_GetCachePathAndFile(/*IN*/CStringW *uri, /*INOUT*/CStringW *cachePath, /*INOUT*/ CStringW *file);

void WML_DestroyWmlObjId(struct WML_ObjId* objId);

struct WML_ObjId* WML_GetWmlObjId(wchar_t *objectId);
void WML_GetParentObjId(struct WML_ObjId* objId, /*INOUT*/struct WML_ObjId *parentId);

int WML_GetWmlObject(/*IN*/struct WML_ObjId* objId, /*INOUT*/struct WML_Object *retVal);
struct WML_Object* WML_CreateWmlObject();
void WML_DestroyWmlObjectList(struct WML_Object*);

void WML_PerformBrowse(struct MSL_CdsQuery *browseArgs, struct CdsStringBrowseArgsWide* wideArgs, int requestedOnAddress, int requestedOnPort);
void WML_PerformBrowse_DidlResponse(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port);

void WML_DoBrowseDirectChildren(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID);
void WML_DoBrowseMetadata(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *updateId);

void WML_RespondWithSetFromStringCollection(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, IWMPStringCollection *strColl, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID, 	char *utf8_ParentID, int utf8_ParentIDLen);
void WML_RespondWithSetFromPlaylist(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, IWMPPlaylist *playlist, wchar_t *attribFilter, wchar_t *attribFilterValue, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID, char *utf8_ParentID, int utf8_ParentIDLen);

void WML_RespondWithContainerFromPlaylist(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, char *utf8Creator, unsigned int mediaClass, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObj);
void WML_RespondWithContainerFromPlaylistArray(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObj);
void WML_RespondWithContainerFromStringCollection(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, IWMPStringCollection *strCol);
void WML_RespondWithContainerFromWmlObjArray(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object **wmlObjArray, int wmlObjArrayLen, char *utf8_ParentID, int utf8_ParentIDLen);
void WML_RespondWithContainerFromWmlObjectList(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObjList);
void WML_RespondWithItemFromMedia(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, IWMPMedia *media);

void WML_RespondWithError(struct MSL_CdsQuery *browseArgs, enum Enum_CdsErrors error);

void WML_SetCdsObjFromPlaylist(/*IN*/char *utf8_ParentID, int utf8_ParentIDLen, /*IN*/ char *utf8Title, int utf8TitleLen, /*IN*/char *utf8Creator, int utf8CreatorLen, unsigned int mediaClass, int *addr, int addrLen, int port, /*IN*/IWMPPlaylist *playlist, /*IN*/wchar_t *attribFilter, /*IN*/wchar_t *attribFilterValue, /*IN*/unsigned int filter, /*INOUT*/struct CdsMediaObject *cdsObj);
void WML_SetCdsObjFromMedia(/*IN*/char *utf8_ParentID, int utf8_ParentIDLen, int appendColon, /*IN*/int embeddedTrackNumber, char *utf8_RefIdParentID, int utf8_RefIdParentIDLen, int *addr, int addrLen, int port, /*IN*/IWMPMedia *media, /*IN*/unsigned int filter, /*INOUT*/struct CdsMediaObject *cdsObj);

char* WML_WmlObjIdToUtf8(struct WML_ObjId* objId);
void WML_WmlObjToUtf8Title(/*INOUT*/char *dest, /*IN*/int destSize, /*IN*/struct WML_ObjId *objId);
int WML_WmlObjToUtf8TitleLen(/*IN*/struct WML_ObjId *objId);

// Container hierarchy for our Media Server

#define WML_NUM_CONTAINERS 16 // number of entries in Enum_WmlFolders-1.
#define WML_HOST_NAME_SIZE 20
#define WML_BASE_CONTAINER_TITLE_LEN 50
int WML_ContainerTitleLen[WML_NUM_CONTAINERS];

enum Enum_WmlFolders
{
	WmlFolders_Root = 0,
	WmlFolders_AllMedia,

	WmlFolders_Audio,
	WmlFolders_Audio_AllAudio,
	WmlFolders_Audio_ByAlbum,
	WmlFolders_Audio_ByArtist,
	WmlFolders_Audio_ByGenre,

	WmlFolders_Video,
	WmlFolders_Video_AllVideo,
	WmlFolders_Video_ByActor,
	WmlFolders_Video_ByGenre,

	WmlFolders_AutoPlaylists,
	WmlFolders_MyPlaylists,

	WmlFolders_Other,
	WmlFolders_Other_AllOther,
	WmlFolders_Other_ByGenre,

	//WmlFolders_Radio,

	// reserved - must be last - not a valid container
	WmlFolders_Undefined
};

enum Enum_WmlFolders WML_StandardParents[WML_NUM_CONTAINERS] = 
{
	WmlFolders_Undefined,
	WmlFolders_Root,

	WmlFolders_Root,
	WmlFolders_Audio,
	WmlFolders_Audio,
	WmlFolders_Audio,
	WmlFolders_Audio,

	WmlFolders_Root,
	WmlFolders_Video,
	WmlFolders_Video,
	WmlFolders_Video,

	WmlFolders_Root,
	WmlFolders_Root,

	WmlFolders_Root,
	WmlFolders_Other,
	WmlFolders_Other

	//WmlFolders_Root
};

struct WML_Object WML_StandardContainers[WML_NUM_CONTAINERS];

#define WML_RootChildren_Len 6
enum Enum_WmlFolders WML_RootChildren[WML_RootChildren_Len] = 
{
	WmlFolders_AllMedia,
	WmlFolders_Audio,
	WmlFolders_Video,
	WmlFolders_AutoPlaylists,
	WmlFolders_MyPlaylists,
	WmlFolders_Other
	//WmlFolders_Radio
};
struct WML_Object* WML_RootChildrenObjects[WML_RootChildren_Len];

#define WML_AudioChildren_Len 4
enum Enum_WmlFolders WML_AudioChildren[WML_AudioChildren_Len] = 
{
	WmlFolders_Audio_AllAudio,
	WmlFolders_Audio_ByAlbum,
	WmlFolders_Audio_ByArtist,
	WmlFolders_Audio_ByGenre
};
struct WML_Object* WML_AudioChildrenObjects[WML_AudioChildren_Len];

#define WML_VideoChildren_Len 3
enum Enum_WmlFolders WML_VideoChildren[WML_VideoChildren_Len] = 
{
	WmlFolders_Video_AllVideo,
	WmlFolders_Video_ByActor,
	WmlFolders_Video_ByGenre
};
struct WML_Object* WML_VideoChildrenObjects[WML_VideoChildren_Len];

#define WML_OtherChildren_Len 2
enum Enum_WmlFolders WML_OtherChildren[WML_OtherChildren_Len] = 
{
	WmlFolders_Other_AllOther,
	WmlFolders_Other_ByGenre
};
struct WML_Object* WML_OtherChildrenObjects[WML_OtherChildren_Len];

char WML_ContainerID[WML_NUM_CONTAINERS+1][WML_BASE_CONTAINER_TITLE_LEN];
int WML_ContainerIDLen[WML_NUM_CONTAINERS+1];

char WML_ContainerTitle[WML_NUM_CONTAINERS+1][WML_BASE_CONTAINER_TITLE_LEN] = 
{
	"Windows Media Library", 
	"All Media",

	"Audio",
	"All Audio",
	"Audio By Albums",
	"Audio By Artist",
	"Audio By Genre",

	"Video",
	"All Video",
	"Videos By Actor",
	"Videos By Genre",

	"Auto Playlists",
	"My Playlists",
	
	"Other Media",
	"All Other Media",
	"Other Media By Genre",

	//"Radio",

	// reserved - must be last - not a valid container
	""
};

#define WML_ATTRIBUTE_ALBUMTITLE		L"WM/AlbumTitle"
#define WML_ATTRIBUTE_AUTHOR			L"Author"
#define	WML_ATTRIBUTE_BITRATE			L"Bitrate"
#define WML_ATTRIBUTE_DURATION			L"Duration"
#define WML_ATTRIBUTE_FILESIZE			L"FileSize"
#define WML_ATTRIBUTE_FILETYPE			L"FileType"
#define WML_ATTRIBUTE_GENRE				L"WM/Genre"
#define WML_ATTRIBUTE_MEDIATYPE			L"MediaType"
#define WML_ATTRIBUTE_MEDIACLASS2		L"WM/MediaClassSecondaryID"
#define WML_ATTRIBUTE_PLAYLISTTYPE		L"PlaylistType"
#define WML_ATTRIBUTE_SOURCEURL			L"SourceURL"
#define WML_ATTRIBUTE_TITLE				L"Title"

enum Enum_WmlAttributes
{
	WML_Attrib_AlbumTitle,
	WML_Attrib_Author,
	WML_Attrib_Bitrate,
	WML_Attrib_Duration,
	WML_Attrib_FileSize,
	WML_Attrib_FileType,
	WML_Attrib_Genre,
	WML_Attrib_MediaType,
	WML_Attrib_MediaClass2,
	Wml_Attrib_PlaylistType,
	Wml_Attrib_SourceURL,
	WML_Attrib_Title
};

wchar_t *WML_Attributes[] = 
{
	WML_ATTRIBUTE_ALBUMTITLE,
	WML_ATTRIBUTE_AUTHOR,
	WML_ATTRIBUTE_BITRATE,
	WML_ATTRIBUTE_DURATION,
	WML_ATTRIBUTE_FILESIZE,
	WML_ATTRIBUTE_FILETYPE,	
	WML_ATTRIBUTE_GENRE,
	WML_ATTRIBUTE_MEDIATYPE,
	WML_ATTRIBUTE_MEDIACLASS2,
	WML_ATTRIBUTE_PLAYLISTTYPE,
	WML_ATTRIBUTE_SOURCEURL,
	WML_ATTRIBUTE_TITLE
};

#define WML_MEDIA_TYPE_AUDIO	L"Audio"
#define WML_MEDIA_TYPE_VIDEO	L"Video"
#define WML_MEDIA_TYPE_OTHER	L"Other"
#define WML_MEDIA_TYPE_RADIO	L"Radio"

#define WML_MEDIA_CLASS2_STATIC_PLAYLIST	L"D0E20D5C-CAD6-4F66-9FA1-6018830F1DCC"
#define WML_MEDIA_CLASS2_AUTO_PLAYLIST		L"EB0BAFB6-3C4F-4C31-AA39-95C7B8D7831D"

#define WML_MEDIA_PLAYLIST_TYPE_AUTO		L"Auto"

#define WML_CREATOR_WML			"Windows Media Library"
#define WML_CREATOR_USER		"User/Local File"
#define WML_CREATOR_INTERNET	"Internet Source"

enum Enum_WmlMediaTypes
{
	Audio,
	Video,
	Other,
	Radio
};

wchar_t *WML_MediaTypes[] = 
{
	WML_MEDIA_TYPE_AUDIO,
	WML_MEDIA_TYPE_VIDEO,
	WML_MEDIA_TYPE_OTHER,
	WML_MEDIA_TYPE_RADIO
};

// UPnP and Media Server Variables

void	*The_Chain;
void	*The_Stack;
void	*The_MediaServerLogic;
void	*The_Monitor;
int		The_IPAddressLength;
int		*The_IPAddressList;
sem_t	The_IPAddressListLock;

HANDLE	The_BrowseSearchStartEvent;
int		The_BrowseSearchThreadStarted;
int		The_ProcessRequestsContinue;
sem_t	The_ProcessRequestLock;
sem_t	The_CdsRequestsLock;
sem_t	The_DirNamesLock;

struct CdsRequest *The_CdsRequestHead = NULL, *The_CdsRequestTail = NULL;
struct WML_CachePath *The_DirNameHead = NULL;
int The_DirNameNum = 0;

CComPtr<IWMPPlayer4>			The_MP;
CComPtr<IWMPMediaCollection>	The_MC;
CComPtr<IWMPPlaylistCollection>	The_PL;

char							*The_AllMediaObjId;
int								The_AllMediaObjIdLen;

void WML_CdsRequestsClear()
{
	struct CdsRequest* req;

	sem_wait(&The_CdsRequestsLock);

	req = The_CdsRequestHead;
	while (req != NULL)
	{
		switch (req->Type)
		{
		case CdsRequestType_Browse:
			WML_DeallocateBrowseRequest(req);
			break;
		}
		
		req = req->Next;
	}

	sem_post(&The_CdsRequestsLock);
}

struct CdsRequest* WML_CdsRequestsEnqueue(struct CdsRequest *req)
{
	struct CdsRequest* retVal = NULL;
	sem_wait(&The_CdsRequestsLock);

	retVal = The_CdsRequestHead;
	if (The_CdsRequestTail == NULL)
	{
		The_CdsRequestHead = The_CdsRequestTail = req;
	}
	else
	{
		The_CdsRequestTail = The_CdsRequestTail->Next = req;
	}
	req->Next = NULL;

	sem_post(&The_CdsRequestsLock);
	return retVal;
}

struct CdsRequest* WML_CdsRequestsDequeue()
{
	struct CdsRequest* retVal = NULL;
	sem_wait(&The_CdsRequestsLock);

	retVal = The_CdsRequestHead;
	if (retVal != NULL)
	{
		The_CdsRequestHead = The_CdsRequestHead->Next;
		if (The_CdsRequestHead == NULL)
		{
			The_CdsRequestTail = NULL; 
		}
	}

	sem_post(&The_CdsRequestsLock);
	return retVal;
}

void WML_CachePathAdd(CStringW dirName)
{
	struct WML_CachePath *temp = NULL;

	sem_wait(&The_DirNamesLock);
	if (The_DirNameHead == NULL)
	{
		The_DirNameHead = new WML_CachePath();
		The_DirNameHead->Name.Append(dirName);
		The_DirNameHead->Next = NULL;
		The_DirNameNum++;
	}
	else
	{
		temp = The_DirNameHead;
		while (temp != NULL)
		{
			if (temp->Name.Compare(dirName) == 0)
			{
				break;
			}
			else if (temp->Next == NULL)
			{
				temp->Next = new WML_CachePath();
				temp = temp->Next;
				temp->Name.Append(dirName);
				temp->Next = NULL;
				The_DirNameNum++;
			}
			temp = temp->Next;
		}
	}
	sem_post(&The_DirNamesLock);
}

void WML_CachePathClear()
{
	struct WML_CachePath *temp = NULL, *next = NULL;
	
	sem_wait(&The_DirNamesLock);
	temp = The_DirNameHead;
	The_DirNameHead = NULL;
	while (temp != NULL)
	{
		next = temp->Next;
		delete (temp);
		The_DirNameNum--;
		temp = next;
	}
	sem_post(&The_DirNamesLock);
}

int WML_CachePathGetByIndex(int index, CStringW *outVal)
{
	struct WML_CachePath *temp = NULL;
	int i = -1;
	int retVal = 0;

	sem_wait(&The_DirNamesLock);
	outVal->SetString(L"");
	temp = The_DirNameHead;
	while (temp != NULL)
	{
		i++;
		if (i == index)
		{
			outVal->SetString(temp->Name);
			retVal = 1;
			break;
		}
		temp = temp->Next;
	}
	sem_post(&The_DirNamesLock);

	return retVal;
}

int WML_CachePathGetByValue(CStringW val)
{
	struct WML_CachePath *temp = NULL;
	int retVal = -1;
	int i = 0;

	sem_wait(&The_DirNamesLock);
	temp = The_DirNameHead;
	while (temp != NULL)
	{
		if (val.Compare(temp->Name) == 0)
		{
			retVal = i;
			break;
		}
		temp = temp->Next;
		i++;
	}
	sem_post(&The_DirNamesLock);

	return retVal;
}

void WML_GetCachePathAndFile(/*IN*/CStringW *uri, /*INOUT*/CStringW *cachePath, /*INOUT*/ CStringW *file)
{
	int slashPos;

	if (cachePath != NULL)
	{
		cachePath->SetString(L"");
		slashPos = uri->ReverseFind('\\');
		if (slashPos > 0)
		{
			cachePath->Append(uri->GetBuffer(), slashPos+1);
		}
		else
		{
			slashPos = uri->ReverseFind('/');
			if (slashPos > 0)
			{
				cachePath->Append(uri->GetBuffer(), slashPos+1);
			}
		}
	}

	if ((file != NULL) & (slashPos > 0))
	{
		file->SetString(L"");
		file->Append(uri->GetBuffer()+slashPos+1);
	}
}

void WML_DeallocateBrowseRequest(struct CdsRequest *request)
{
	struct MSL_CdsQuery *browseArgs;
	struct CdsStringBrowseArgsWide *baw;

	browseArgs = (struct MSL_CdsQuery*) request->Args;
	baw = (struct CdsStringBrowseArgsWide*) browseArgs->UserObject;

	free(baw->Filter);
	free(baw->ObjectID);
	free(baw->SortCriteria);
	free(baw);
	MSL_DeallocateCdsQuery(browseArgs);
	free(request);
}

void Upnp_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	sem_wait(&The_IPAddressListLock);
	length = ILibGetLocalIPAddressList(&list);
	if(length!=The_IPAddressLength || memcmp((void*)list,(void*)The_IPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(The_Stack);
		
		FREE(The_IPAddressList);
		The_IPAddressList = list;
		The_IPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	sem_post(&The_IPAddressListLock);
	
	ILibLifeTime_Add(The_Monitor,NULL,4,&Upnp_IPAddressMonitor, NULL);
}

unsigned long WINAPI Upnp_BrowseSearchLoop(void* dlgctrl)
{
	HRESULT hr;
	struct CdsRequest *request;
	struct MSL_CdsQuery *browseArgs;
	struct CdsStringBrowseArgsWide *baw;
	int i;
	char hostname[WML_HOST_NAME_SIZE];
	char containerTitle[WML_BASE_CONTAINER_TITLE_LEN];
	//wchar_t wcontainerTitle[WML_BASE_CONTAINER_TITLE_LEN];
	WSADATA wsaData;
	//int wlen;
	int len;
	CComPtr<IWMPPlaylist> allMedia;
	CComPtr<IWMPMedia> media;
	long count;
	CComBSTR bstr;
	CStringW cachePath, temp, file;
	int errorFlag = 0;

	CoInitialize(NULL);

	hr = The_MP.CoCreateInstance(CLSID_MEDIAPLAYER);

	if ((!FAILED(hr)) && (The_MP.p != NULL))
	{
		hr = The_MP->get_mediaCollection(&The_MC);
		hr = The_MP->get_playlistCollection(&The_PL);

		if ((!FAILED(hr)) && (The_MC.p != NULL) && (The_PL.p != NULL))
		{
			/* grab the host name of the computer and apply to root container title */
			if (WSAStartup(MAKEWORD(1,1), &wsaData) == 0)
			{
				memset(hostname, 0, WML_HOST_NAME_SIZE);
				gethostname(hostname, WML_HOST_NAME_SIZE);
				hostname[WML_HOST_NAME_SIZE-1] = '\0';
				memset(containerTitle, 0, WML_BASE_CONTAINER_TITLE_LEN);
				len = sprintf(containerTitle, "Windows Media Library (%s)", hostname);
				memset(WML_ContainerTitle[WmlFolders_Root], 0, WML_BASE_CONTAINER_TITLE_LEN);
				memcpy(WML_ContainerTitle[WmlFolders_Root], containerTitle, len);
				/*
				wlen = (int) mbstowcs(wcontainerTitle, containerTitle, WML_BASE_CONTAINER_TITLE_LEN);
				memset(WML_ContainerTitle[1], 0, WML_BASE_CONTAINER_TITLE_LEN);
				memcpy(WML_ContainerTitle[1], wcontainerTitle, wlen*2);
				*/

				/* populate the length arrays */
				for (i=0; i < WML_NUM_CONTAINERS; i++)
				{
					WML_ContainerTitleLen[i] = (int) strlen(WML_ContainerTitle[i]);
					WML_ContainerIDLen[i] = sprintf(WML_ContainerID[i], "%d", i);
					
					//WML_ContainerIDLen[i] = (int) wcslen(WML_ContainerID[i]);
				}
				i = WML_NUM_CONTAINERS;
				WML_ContainerIDLen[i] = sprintf(WML_ContainerID[i], "%d", -1);

				/* get all media items */
				hr = The_MC->getAll(&allMedia);
				if (!(FAILED(hr)))
				{
					/* iterate through all items and build a list of all paths (excluding filename/query) */
					if (!FAILED(allMedia->get_count(&count)))
					{
						for (i=0; i < count; i++)
						{
							media = NULL;
							if (!FAILED(allMedia->get_item(i, &media)))
							{
								if (!FAILED(media->get_sourceURL(&bstr)))
								{
									temp.SetString(L"");
									temp.Append(bstr);
									WML_GetCachePathAndFile(&temp, &cachePath, &file);
									if (cachePath.GetLength() > 0)
									{
										WML_CachePathAdd(cachePath);
									}
								}
								else
								{
									errorFlag = 1;
								}
							}
							else
							{
								errorFlag = 1;
							}
						}

						if (errorFlag == 0)
						{
							/* grab the object ID for the AllMedia folder */
							The_AllMediaObjId = (char*) malloc(5);
							memset(The_AllMediaObjId, 0, 5);
							The_AllMediaObjIdLen = sprintf(The_AllMediaObjId, "%d", (int)WmlFolders_AllMedia);

							/* build a skeletal container hierarchy that will exist regardless of what media's in WML */

							for (i=0; i < WML_RootChildren_Len; i++)
							{
								WML_RootChildrenObjects[i] = &WML_StandardContainers[WML_RootChildren[i]];
							}
							for (i=0; i < WML_AudioChildren_Len; i++)
							{
								WML_AudioChildrenObjects[i] = &WML_StandardContainers[WML_AudioChildren[i]];
							}
							for (i=0; i < WML_VideoChildren_Len; i++)
							{
								WML_VideoChildrenObjects[i] = &WML_StandardContainers[WML_VideoChildren[i]];
							}
							for (i=0; i < WML_OtherChildren_Len ; i++)
							{
								WML_OtherChildrenObjects[i] = &WML_StandardContainers[WML_OtherChildren[i]];
							}

							for (i=0; i < WML_NUM_CONTAINERS; i++)
							{
								//WML_StandardContainers[i].Next = NULL;
								WML_StandardContainers[i].ObjId = (Enum_WmlFolders) i;

								switch (i)
								{
								case WmlFolders_Root:
									WML_StandardContainers[i].Type = WmlObjType_WmlObjArray;
									WML_StandardContainers[i].WmlObjArray = WML_RootChildrenObjects;
									WML_StandardContainers[i].WmlObjArrayLen = WML_RootChildren_Len;
									break;
								
								case WmlFolders_AllMedia:
									WML_StandardContainers[i].Type = WmlObjType_Playlist;
									WML_StandardContainers[i].Playlist = NULL;
									if (FAILED(The_MC->getAll(&(WML_StandardContainers[i].Playlist))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Audio:
									WML_StandardContainers[i].Type = WmlObjType_WmlObjArray;
									WML_StandardContainers[i].WmlObjArray = WML_AudioChildrenObjects;
									WML_StandardContainers[i].WmlObjArrayLen = WML_AudioChildren_Len;
									break;

								case WmlFolders_Audio_AllAudio:
									WML_StandardContainers[i].Type = WmlObjType_Playlist;
									WML_StandardContainers[i].Playlist = NULL;
									if (FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_MEDIATYPE, WML_MEDIA_TYPE_AUDIO, &(WML_StandardContainers[i].Playlist))))
									{
										errorFlag = 1;
									}
									break;
								
								case WmlFolders_Audio_ByAlbum:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_ALBUMTITLE, WML_MEDIA_TYPE_AUDIO, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;
								
								case WmlFolders_Audio_ByArtist:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_AUTHOR, WML_MEDIA_TYPE_AUDIO, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Audio_ByGenre:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_GENRE, WML_MEDIA_TYPE_AUDIO, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Video:
									WML_StandardContainers[i].Type = WmlObjType_WmlObjArray;
									WML_StandardContainers[i].WmlObjArray = WML_VideoChildrenObjects;
									WML_StandardContainers[i].WmlObjArrayLen = WML_VideoChildren_Len;
									break;

								case WmlFolders_Video_AllVideo:
									WML_StandardContainers[i].Type = WmlObjType_Playlist;
									WML_StandardContainers[i].Playlist = NULL;
									if (FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_MEDIATYPE, WML_MEDIA_TYPE_VIDEO, &(WML_StandardContainers[i].Playlist))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Video_ByActor:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_AUTHOR, WML_MEDIA_TYPE_VIDEO, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Video_ByGenre:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_GENRE, WML_MEDIA_TYPE_VIDEO, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_AutoPlaylists:
									WML_StandardContainers[i].Type = WmlObjType_PlaylistArray;
									WML_StandardContainers[i].PlaylistArrayIsAutoType = 1;
									WML_StandardContainers[i].PlaylistArray = NULL;
									if (FAILED(The_PL->getAll(&(WML_StandardContainers[i].PlaylistArray))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_MyPlaylists:
									WML_StandardContainers[i].Type = WmlObjType_PlaylistArray;
									WML_StandardContainers[i].PlaylistArrayIsAutoType = 0;
									WML_StandardContainers[i].PlaylistArray = NULL;
									if (FAILED(The_PL->getAll(&(WML_StandardContainers[i].PlaylistArray))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Other:
									WML_StandardContainers[i].Type = WmlObjType_WmlObjArray;
									WML_StandardContainers[i].WmlObjArray = WML_OtherChildrenObjects;
									WML_StandardContainers[i].WmlObjArrayLen = WML_OtherChildren_Len;
									break;

								case WmlFolders_Other_AllOther:
									WML_StandardContainers[i].Type = WmlObjType_Playlist;
									WML_StandardContainers[i].Playlist = NULL;
									if (FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_MEDIATYPE, WML_MEDIA_TYPE_OTHER, &(WML_StandardContainers[i].Playlist))))
									{
										errorFlag = 1;
									}
									break;

								case WmlFolders_Other_ByGenre:
									WML_StandardContainers[i].Type = WmlObjType_StringCollection;
									WML_StandardContainers[i].StringCollection = NULL;
									if (FAILED(The_MC->getAttributeStringCollection(WML_ATTRIBUTE_GENRE, WML_MEDIA_TYPE_OTHER, &(WML_StandardContainers[i].StringCollection))))
									{
										errorFlag = 1;
									}
									break;

								//case WmlFolders_Radio:
								//	WML_StandardContainers[i].Type = WmlObjType_Playlist;
								//	WML_StandardContainers[i].Playlist = NULL;
								//	if (FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_MEDIATYPE, WML_MEDIA_TYPE_RADIO, &(WML_StandardContainers[i].Playlist))))
								//	{
								//		errorFlag = 1;
								//	}
								//	break;
								}
							}

							if (errorFlag == 0)
							{
								/* signal that the MediaServer is actually ready to use now */
								The_BrowseSearchThreadStarted = 1;
								SetEvent(The_BrowseSearchStartEvent);

								while (The_ProcessRequestsContinue != 0)
								{
									request = WML_CdsRequestsDequeue();
									if (request != NULL)
									{
										switch (request->Type)
										{
										case CdsRequestType_Browse:
											browseArgs = (struct MSL_CdsQuery*) request->Args;
											baw = (struct CdsStringBrowseArgsWide*) browseArgs->UserObject;
											WML_PerformBrowse(browseArgs, baw, request->RequestedOnAddress, request->RequestedOnPort);
											break;

										default:
											ASSERT(0);
											break;
										}

										WML_DeallocateBrowseRequest(request);
									}
									else
									{
										sem_wait(&The_ProcessRequestLock);
									}
								}
								free(The_AllMediaObjId);
							}
							else
							{
								SetEvent(The_BrowseSearchStartEvent);
							}
						}
						else
						{
							SetEvent(The_BrowseSearchStartEvent);
						}

						WML_CachePathClear();
					}
					else
					{
						SetEvent(The_BrowseSearchStartEvent);
					}
				}
				else
				{
					SetEvent(The_BrowseSearchStartEvent);
				}

				WSACleanup();
			}
			else
			{
				SetEvent(The_BrowseSearchStartEvent);
			}
		}
		else
		{
			SetEvent(The_BrowseSearchStartEvent);
		}

		The_PL = NULL;
		The_MC = NULL;
	}
	else
	{
		SetEvent(The_BrowseSearchStartEvent);
	}
	The_MP = NULL;

	CoUninitialize();
	return 0;
}

void WML_DestroyWmlObjId(struct WML_ObjId* objId)
{
	if (objId->SubFolder != NULL) { free (objId->SubFolder); }
	if (objId->Source != NULL) { free (objId->Source); }
	if (objId->SourcePrefix != NULL) { free (objId->SourcePrefix); }
	free(objId);
}

#define TS_LEN 20
void WML_SetCdsObjFromPlaylist(/*IN*/char *utf8_ParentID, int utf8_ParentIDLen, /*IN*/ char *utf8Title, int utf8TitleLen, /*IN*/char *utf8Creator, int utf8CreatorLen, unsigned int mediaClass, int *addr, int addrLen, int port, /*IN*/IWMPPlaylist *playlist, /*IN*/wchar_t *attribFilter, /*IN*/wchar_t *attribFilterValue, /*IN*/unsigned int filter, /*INOUT*/struct CdsMediaObject *cdsObj)
{
	long longVal;
	int i;
	CComPtr <IWMPMedia> media;
	CComBSTR bstr;

	// Always restricted... and not searchable for now
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	// ID
	i = utf8_ParentIDLen+utf8TitleLen+2;
	cdsObj->ID = (char*) malloc(i);
	memcpy(cdsObj->ID, utf8_ParentID, utf8_ParentIDLen);
	memcpy(cdsObj->ID+utf8_ParentIDLen, ":", 1);
	memcpy(cdsObj->ID+utf8_ParentIDLen+1, utf8Title, utf8TitleLen);
	cdsObj->ID[utf8_ParentIDLen+1+utf8TitleLen] = '\0';

	// parentID
	cdsObj->ParentID = (char*) malloc(utf8_ParentIDLen+1);
	memcpy(cdsObj->ParentID, utf8_ParentID, utf8_ParentIDLen);
	cdsObj->ParentID[utf8_ParentIDLen] = '\0';

	// media class
	if (mediaClass & CDS_CLASS_MASK_CONTAINER)
	{
		cdsObj->MediaClass = mediaClass;
	}
	else
	{
		cdsObj->MediaClass = CDS_CLASS_MASK_CONTAINER;
	}

	// creator
	if (utf8Creator != NULL)
	{
		cdsObj->Creator = (char*)malloc(utf8CreatorLen+1);
		memcpy(cdsObj->Creator, utf8Creator, utf8CreatorLen);
		cdsObj->Creator[utf8CreatorLen] =  '\0';
	}
	else
	{
		playlist->getItemInfo(WML_ATTRIBUTE_AUTHOR, &bstr);
		i = bstr.Length();
		cdsObj->Creator = (char*)malloc((i*2)+1);
		strToUtf8(cdsObj->Creator, (char*)OLE2W(bstr), (i*2)+1, 1, NULL);
	}

	// title
	cdsObj->Title = (char*)malloc(utf8TitleLen+1);
	memcpy(cdsObj->Title, utf8Title, utf8TitleLen);
	cdsObj->Title[utf8TitleLen] = '\0';

	//TODO: WML_SetCdsObjFromPlaylist() - do resources

	// childCount
	playlist->get_count(&longVal);
	if (attribFilter == NULL)
	{
		cdsObj->ChildCount = (int) longVal;
	}
	else
	{
		cdsObj->ChildCount = 0;
		for (i=0; i < longVal; i++)
		{
			media = NULL;
			playlist->get_item(i, &media);
			media->getItemInfo(attribFilter, &bstr);
			if (wcsicmp(attribFilterValue, OLE2W(bstr))==0)
			{
				cdsObj->ChildCount++;
			}
		}
	}
}


void WML_SetCdsObjFromMedia(/*IN*/char *utf8_ParentID, int utf8_ParentIDLen, int appendColon, /*IN*/int embeddedTrackNumber, char *utf8_RefIdParentID, int utf8_RefIdParentIDLen, int *addr, int addrLen, int port, /*IN*/IWMPMedia *media, /*IN*/unsigned int filter, /*INOUT*/struct CdsMediaObject *cdsObj)
{
	CComBSTR bstr, fileType;
	int size, pi;
	CStringW sourceUrl, cachePath, file, refId;
	int cachePathIndex;
	int i;
	struct CdsMediaResource *res, **lastRes;
	wchar_t *uri, *uriFile;
	char *protInfo, *uriEscape;
	int uriEscapeLen, isLocalFile;

	double dbl;
	long lng1, lng2;
	int resX, resY, resDuration, resBitrate, resColorDepth;
	long resSize;
	char str[TS_LEN];
	CComPtr <IWMPPlaylistArray> pla;
	CComPtr <IWMPPlaylist> pl;

	/*
	 *	Windows Media Library objects are always restricted
	 *	....and never searchable for now
	 */
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	/* assign object id */
	media->get_sourceURL(&bstr);
	sourceUrl.Append(bstr);
	WML_GetCachePathAndFile(&sourceUrl, &cachePath, &file);
	cachePathIndex = WML_CachePathGetByValue(cachePath);
	if (cachePathIndex >= 0)
	{
		/* copy parent ID */
		cdsObj->ParentID = (char*) malloc (utf8_ParentIDLen+1);
		memcpy(cdsObj->ParentID, utf8_ParentID, utf8_ParentIDLen+1);

		/* assign object ID */
		size = ((int) file.GetLength()) + utf8_ParentIDLen + 30;
		cdsObj->ID = (char*) malloc (size);
		if (embeddedTrackNumber >= 0)
		{
			pi = sprintf(cdsObj->ID, "%s?!?%d?@?", cdsObj->ParentID, cachePathIndex);
			pi += strToUtf8(cdsObj->ID+pi, (const char*) file.GetBuffer(), size-pi, 1, NULL);
			pi += sprintf(cdsObj->ID+pi, "?#?");
			pi += sprintf(cdsObj->ID+pi, "%d", embeddedTrackNumber);
		}
		else
		{
			if (appendColon)
			{
				pi = sprintf(cdsObj->ID, "%s::%d:", cdsObj->ParentID, cachePathIndex);
			}
			else
			{
				pi = sprintf(cdsObj->ID, "%s:%d:", cdsObj->ParentID, cachePathIndex);
			}
			strToUtf8(cdsObj->ID+pi, (const char*) file.GetBuffer(), size-pi, 1, NULL);
		}

		/* assign title */
		media->getItemInfo(WML_ATTRIBUTE_TITLE, &bstr);
		size = (int) (bstr.Length()+1);
		cdsObj->Title = (char*) malloc(size);
		strToUtf8(cdsObj->Title, (const char*) OLE2W(bstr), size, 1, NULL);

		/* assign media class */
		media->getItemInfo(WML_ATTRIBUTE_MEDIATYPE, &bstr);
		media->getItemInfo(WML_ATTRIBUTE_FILETYPE, &fileType);

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
			cdsObj->MediaClass = FileExtensionToClassCode((char*)OLE2W(fileType), 1);
		}

		if (cdsObj->MediaClass & CDS_CLASS_MASK_CONTAINER)
		{
			ASSERT(cdsObj->MediaClass & CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER);
			
			//
			//	TODO: Improve the performance of this code by building a table. 
			//	We may not be able to fix the lack of correlation between
			//	IWMPPlaylist and IWMPMedia objects, but we can at least
			//	improve the search time for results. 

			The_PL->getAll(&pla);
			pla->get_count(&lng1);
			for (i=0; i < lng1; i++)
			{
				// This code encounters an error case when the WML's list
				// of playlists has duplicate names. WML unfortunatley has
				// no way of correlating an IWMPMedia with an IWMPPlaylist.
				// If microsoft applied the SourceURL attribute to the
				// IWMPPlaylist object, then we'd be in great shape.

				pl = NULL;
				pla->item(i, &pl);
				pl->get_name(&bstr);
				if (wcsnicmp(OLE2W(bstr), file.GetBuffer(), wcslen(OLE2W(bstr))) == 0)
				{
					pl->get_count(&lng2);
					cdsObj->ChildCount = (int) lng2;
					break;
				}
			}

			ASSERT(cdsObj->ChildCount >= 0);
		}
		else
		{
			/* assign ref ID */
			if (utf8_RefIdParentID != NULL)
			{
				size = ((int) file.GetLength()) + utf8_RefIdParentIDLen + 20;
				cdsObj->RefID = (char*) malloc(size);
				pi = sprintf(cdsObj->RefID, "%s::%d:", utf8_RefIdParentID, cachePathIndex);
				strToUtf8(cdsObj->RefID+pi, (const char*) file.GetBuffer(), size-pi, 1, NULL);
			}
		}

		/* assign creator */
		if (filter & CdsFilter_Creator)
		{
			media->getItemInfo(WML_ATTRIBUTE_AUTHOR, &bstr);
			size = (int) (bstr.Length()+1);
			if (size > 1)
			{
				cdsObj->Creator = (char*) malloc(size);
				strToUtf8(cdsObj->Creator, (const char*) OLE2W(bstr), size, 1, NULL);
			}
		}

		/* create resources */
		if (filter & CdsFilter_ResAllAttribs)
		{
			/* lastRes is where we can assign the new resource */
			lastRes = &(cdsObj->Res);

			/* get the source URI and get its URI-escaped forms*/
			uri = (wchar_t*) OLE2W(sourceUrl.GetBuffer());
			uriFile = (wchar_t*) OLE2W(file.GetBuffer());

			if (
				((uri[0] == '\\') && (uri[1] == '\\')) ||
				((uri[1] == ':') && (uri[2] == '\\'))
				)
			{
				isLocalFile = 1;

				if (file.Find(L"%") < 0)
				{
					uriEscapeLen = strUtf8Len((char*)uriFile,1,1);
					uriEscape = (char*) malloc(uriEscapeLen+1);
					strToEscapedUri(uriEscape, (char*)uriFile, uriEscapeLen+1, 1, NULL);
				}
				else
				{
					uriEscapeLen = file.GetLength();
					uriEscape = (char*) malloc(uriEscapeLen+1);
					strToUtf8(uriEscape, (char*)uriFile, uriEscapeLen+1, 1, NULL);
				}
			}
			else
			{
				isLocalFile = 0;

				if (sourceUrl.Find(L"%") < 0)
				{
					uriEscapeLen = strUtf8Len((char*)uri,1,1);
					uriEscape = (char*) malloc(uriEscapeLen+1);
					strToEscapedUri(uriEscape, (char*)uri, uriEscapeLen+1, 1, NULL);
				}
				else
				{
					uriEscapeLen = sourceUrl.GetLength();
					uriEscape = (char*) malloc(uriEscapeLen+1);
					strToUtf8(uriEscape, (char*)uri, uriEscapeLen+1, 1, NULL);
				}
			}

			/* get protocolInfo - returned value is static */
			protInfo = FileExtensionToProtocolInfo((char*)(OLE2W(fileType)), 1);

			/* obtain information for populating resources - always initialize to zero */
			resSize = resX = resY = resDuration = resBitrate = resColorDepth = 0;

			/* resolution not supported by Windows Media Library */
			if (filter & CdsFilter_Resolution)
			{
				media->get_imageSourceHeight(&lng1);
				media->get_imageSourceWidth(&lng2);
				if ((lng1 > 0) && (lng2 > 0))
				{
					resY = (int) lng1;
					resX = (int) lng2;
				}
			}

			if (filter & CdsFilter_Duration)
			{
				media->get_duration(&dbl);
				if (dbl > 0)
				{
					resDuration = (int) dbl;
				}
			}

			if (filter & CdsFilter_Bitrate)
			{
				media->getItemInfo(WML_ATTRIBUTE_BITRATE, &bstr);
				lng1 = bstr.Length();
				if (lng1 > 0)
				{
					lng2 = 0;
					strToUtf8(str, (char*)OLE2W(bstr), TS_LEN, 1, NULL);
					ILibGetLong(str, TS_LEN, &lng2);
					if (lng2 > 0)
					{
						// CDS actually says bitrate is to be in bytes/second
						resBitrate = lng2 / 8;
					}
				}
			}

			if (filter & CdsFilter_Size)
			{
				media->getItemInfo(WML_ATTRIBUTE_FILESIZE, &bstr);
				lng1 = bstr.Length();
				if (lng1 > 0)
				{
					lng2 = 0;
					strToUtf8(str, (char*)OLE2W(bstr), TS_LEN, 1, NULL);
					ILibGetLong(str, TS_LEN, &lng2);
					if (lng2 > 0)
					{
						resSize = lng2;
					}
				}
			}

			if (isLocalFile != 0)
			{
				/* the source file is local */

				for (i=0; i < addrLen; i++)
				{
					/* res->ProtocolInfoAllocated will be zero */
					res = CDS_AllocateResource();

					/* assign static protocolInfo - do not mark the field as allocated */
					res->ProtocolInfo = protInfo;

					/* populate value */
					size = 40 + uriEscapeLen;
					res->Value = (char*) malloc (size);
					pi = sprintf(res->Value, "http://%d.%d.%d.%d:%d/%d/", (addr[i]&0xFF), ((addr[i]>>8)&0xFF), ((addr[i]>>16)&0xFF), ((addr[i]>>24)&0xFF), port, cachePathIndex);
					memcpy(res->Value+pi, uriEscape, uriEscapeLen+1);

					res->Bitrate = resBitrate;
					res->ColorDepth = resColorDepth;
					res->Duration = resDuration;
					res->ResolutionX = resX;
					res->ResolutionY = resY;
					res->Size = resSize;

					*lastRes = res;
					lastRes = &(res->Next);
				}
			}
			else
			{
				/* res->ProtocolInfoAllocated will be zero */
				res = CDS_AllocateResource();
				
				/* assign static protocolInfo - do not mark the field as allocated */
				res->ProtocolInfo = protInfo;
				
				/* copy URI value */
				size = 2 + uriEscapeLen;
				res->Value = (char*) malloc (size);
				memcpy(res->Value, uriEscape, uriEscapeLen+1);

				*lastRes = res;
			}

			free(uriEscape);
		}
	}
}

struct WML_ObjId* WML_GetWmlObjId(wchar_t *objectId)
{
	// Each objectID in this CDS has the following format.
	//	[BaseFolder]:[SubFolder]:[UriFolder]:[UriFile]?!?[SubUriFolder]?@?[SubUriFile]?#?[TrackIndex]
	//
	//	[BaseFolder]		One of the hierarchical folders that can be abstracted 
	//						from WML's flat list of media.
	//
	//	[SubFolder]			String. Some media may be stored in a dynamic container, such as
	//						for a particular artist. This value may be an empty string.
	//
	//	[UriFolder]			Integer index into the SourceUriFolders array.
	//
	//	[UriFile]			String representation of the file that comes after UriFolder.
	//						There exists a translation between a media's sourceURL and UriFolder+UriFile.
	//
	//	[SubUriFolder]		UriFolder+UriFile can collectively represent a file like an M3U playlist.
	//	[SubUriFile]		SubUriFolder+SubUriFile can collectively represent a file/track within the playlist.
	//	[TrackIndex]		

	struct WML_ObjId* retVal = NULL;
	CStringW id, bf, sf, ufolder, ufile, efolder, efile, etrackindex;
	int pc1, pc2, pc3, pq1, pq2, pq3;
	long lBaseFolder = -1, lUriFolder = -1, leUriFolder = -1, leTrackIndex = -1;
	enum Enum_WmlFolders basefolder;
	int len, len1, len2;
	int tmp, error = 0;
	CStringW str;
	char numString[256];
	
	id.SetString(objectId);
	pc1 = id.Find(L":");
	if (pc1 > 0)
	{
		bf.SetString(objectId, pc1);
		pc2 = id.Find(L":", pc1+1); 

		if (pc2 > 0) 
		{
			sf.SetString(objectId+pc1+1, pc2-pc1-1);
			pc3 = id.Find(L":", pc2+1); 

			if (pc3 > 0) 
			{ 
				pq1 = id.Find(L"?!?", pc3+1);

				if (pq1 > 0)
				{
					ufolder.SetString(objectId+pc2+1, pc3-pc2-1);
					ufile.SetString(objectId+pc3+1, pq1-pc3-1);

					pq2 = id.Find(L"?@?", pq1+3);

					if (pq2 > 0)
					{
						pq3 = id.Find(L"?#?", pq2+3);
						if (pq3 > 0)
						{
							efolder.SetString(objectId+pq1+3, pq2-pq1-3);
							efile.SetString(objectId+pq2+3, pq3-pq2-3);
						}
					}
					else
					{
						pq3 = id.Find(L"?#?", pq1+3);
						if (pq3 > 0)
						{
							efolder.SetString(L"");
							efile.SetString(objectId+pq1+3, pq3-pq1-3);
						}
					}

					if (pq3 > 0)
					{
						etrackindex.SetString(objectId+pq3+3);
					}
					else
					{
						error = 1;
					}
				}
				else
				{
					ufolder.SetString(objectId+pc2+1, pc3-pc2-1);
					ufile.SetString(objectId+pc3+1);
				}
			}
			else
			{
				ufolder.SetString(objectId+pc2+1);
			}
		}
		else
		{
			sf.SetString(objectId+pc1+1);
		}
	}
	else
	{
		bf.SetString(objectId);
	}

	if (error == 0)
	{
		len = bf.GetLength();
		for (len1=0; len1 < len; len1++)
		{
			numString[len1] = (char) (objectId[len1] & 0x00FF);
		}
		numString[len] = '\0';
		ILibGetLong(numString, len, &lBaseFolder);
		basefolder = (enum Enum_WmlFolders) (lBaseFolder);

		if ((basefolder >= 0) && (basefolder < WML_NUM_CONTAINERS))
		{
			retVal = (struct WML_ObjId*) malloc (sizeof (struct WML_ObjId));
			memset(retVal, 0, sizeof(struct WML_ObjId));
			retVal->EmbeddedTrackIndex = -1;

			retVal->BaseFolder = basefolder;
			
			// parse sub folder

			len = sf.GetLength();
			if (len > 0)
			{
				retVal->SubFolder = (wchar_t*) malloc((len+1)*2);
				sf.CopyChars(retVal->SubFolder, sf.GetBuffer(), len);
				retVal->SubFolder[len] = '\0';
			}

			// get URI string index

			len = ufolder.GetLength();
			if (len > 0)
			{
				for (len1=0; len1 < len; len1++)
				{
					numString[len1] = (char) (objectId[pc2+1+len1] & 0x00FF);
				}
				numString[len] = '\0';

				// parse uri/dirname string index 
				lUriFolder = -1;
				ILibGetLong(numString, len, &lUriFolder);

				if ((lBaseFolder >= 0) && (lBaseFolder < WML_NUM_CONTAINERS))
				{
					tmp = WML_CachePathGetByIndex((int)lUriFolder, &str);
					if (tmp != 0)
					{
						len1 = str.GetLength(); 
						len2 = ufile.GetLength();
						len = len1 + len2 + 1;

						retVal->Source = (wchar_t*) malloc((len+1)*2);
						retVal->SourcePrefix = (wchar_t*) malloc((len1+1)*2);
						
						swprintf(retVal->Source, L"%s%s", str.GetBuffer(), ufile.GetBuffer());
						swprintf(retVal->SourcePrefix, L"%s", str.GetBuffer());

						retVal->SourcePrefixIndex = (int) lUriFolder;
						retVal->_SourcePostfix = retVal->Source+len1;

						// parse out efolder/efile
						len = efolder.GetLength();
						if (len > 0)
						{
							for (len1=0; len1 < len; len1++)
							{
								numString[len1] = (char) (objectId[pq1+3+len1] & 0x00FF);
							}
							numString[len] = '\0';

							ILibGetLong(numString, len, &leUriFolder);
							tmp = WML_CachePathGetByIndex((int)leUriFolder, &str);
							if (tmp != 0)
							{
								len1 = str.GetLength(); 
								len2 = efile.GetLength();
								len = len1 + len2 + 1;

								retVal->EmbeddedSource = (wchar_t*) malloc((len+1)*2);
								retVal->EmbeddedSourcePrefix = (wchar_t*) malloc((len1+1)*2);
								
								swprintf(retVal->EmbeddedSource, L"%s%s", str.GetBuffer(), efile.GetBuffer());
								swprintf(retVal->EmbeddedSourcePrefix, L"%s", str.GetBuffer());

								retVal->EmbeddedSourcePrefixIndex = (int) leUriFolder;
								retVal->_EmbeddedSourcePostfix = retVal->EmbeddedSource+len1;
							}
							else
							{
								error = 1;
							}
						}
						else
						{
							len = efile.GetLength();
							if (len > 0)
							{
								retVal->EmbeddedSource = (wchar_t*) malloc((len+1)*2);
								swprintf(retVal->EmbeddedSource, L"%s", efile.GetBuffer());
							}
						}

						if (retVal->EmbeddedSource != NULL)
						{
							len = etrackindex.GetLength();
							if (len > 0)
							{
								for (len1=0; len1 < len; len1++)
								{
									numString[len1] = (char) (objectId[pq3+3+len1] & 0x00FF);
								}
								numString[len] = '\0';

								ILibGetLong(numString, len, &leTrackIndex);
								if (leTrackIndex > 0)
								{
									retVal->EmbeddedTrackIndex = (int)leTrackIndex;
								}
								else
								{
									error = 1;
								}
							}
							else
							{
								error = 1;
							}
						}
					}
					else
					{
						error = 1;
					}
				}
				else
				{
					error = 1;
				}
			}
			else
			{
				len = ufile.GetLength();
				if (len > 0)
				{
					retVal->Source = (wchar_t*) malloc((len+1)*2);
					swprintf(retVal->Source, L"%s", ufile.GetBuffer());
				}
			}
		}
	}

	if (error)
	{
		if (retVal != NULL) { WML_DestroyWmlObjId(retVal); retVal = NULL; }
	}

	return retVal;
}

void WML_GetParentObjId(struct WML_ObjId* objId, /*INOUT*/struct WML_ObjId *parentId)
{
	struct WML_ObjId gpid;

	parentId->EmbeddedSource = parentId->_EmbeddedSourcePostfix = parentId->EmbeddedSourcePrefix = NULL;
	parentId->Source = parentId->SubFolder = parentId->_SourcePostfix = parentId->SourcePrefix = NULL;

	if (objId->SubFolder != NULL)
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				//	If a SubFolder, Source, and EmbeddedSource
				//	are all defined, then the objId maps to
				//	an individual track within a specific playlist
				//	that can be found by parsing the the 
				//	BaseFolder+SubFolder+Source route. In this case,
				//	the parentId is simply the objId's fields
				//	without the embedded fields. We simply copy-by-value
				//	the fields over from objId and ensure the approrpiate
				//	'Embedded' fields are NULL.

				*parentId = *objId;
				parentId->EmbeddedSource = parentId->_EmbeddedSourcePostfix = parentId->EmbeddedSourcePrefix = NULL;
			}
			else
			{
				//	If only the SubFolder and Source are specified, then
				//	objId maps to an individual item that can be
				//	found in a BaseFolder+SubFolder+Source mapping.
				//	In this case, the parentId is simply the BaseFolder
				//	and SubFolder fields.
				//
				//	In the end, the parentId should map to a dynamic container,
				//	which have parents like:
				//		WmlFolders_Audio_ByAlbum
				//		WmlFolders_Audio_ByArtist
				//		WmlFolders_Audio_ByGenre

				parentId->BaseFolder = objId->BaseFolder;
				parentId->SubFolder = objId->SubFolder;

				#ifdef _DEBUG
				WML_GetParentObjId(parentId, &gpid);
				
				ASSERT(
					(gpid.SubFolder == NULL) &&
					(gpid.Source == NULL) &&
					(gpid.EmbeddedSource == NULL)
					);

				ASSERT(
					(gpid.BaseFolder == WmlFolders_Audio_ByAlbum) ||
					(gpid.BaseFolder == WmlFolders_Audio_ByArtist) ||
					(gpid.BaseFolder == WmlFolders_Video_ByActor) ||
					(gpid.BaseFolder == WmlFolders_Video_ByGenre) ||
					(gpid.BaseFolder == WmlFolders_Other_ByGenre) 
					);
				#endif
			}
		}
		else
		{
			//	If SubFolder is specified and Source is empty,
			//	then, the objId maps to a dynamic container found as a child
			//	of a standard container. Examples include the 
			//	children of 
			//		WmlFolders_Audio_AllAudio,
			//		WmlFolders_Audio_ByAlbum,
			//		WmlFolders_Audio_ByArtist,
			//		WmlFolders_Audio_ByGenre.
			//	In this case, all we need to do is use 
			//	objId->BaseFolder as the value for the parent.

			parentId->BaseFolder = objId->BaseFolder;

			ASSERT(
				(parentId->BaseFolder == WmlFolders_Audio_AllAudio) ||
				(parentId->BaseFolder == WmlFolders_Audio_ByAlbum) ||
				(parentId->BaseFolder == WmlFolders_Audio_ByArtist) ||
				(parentId->BaseFolder == WmlFolders_Audio_ByGenre) ||
				(parentId->BaseFolder == WmlFolders_Video_AllVideo) ||
				(parentId->BaseFolder == WmlFolders_Video_ByActor) ||
				(parentId->BaseFolder == WmlFolders_Video_ByGenre) ||
				(parentId->BaseFolder == WmlFolders_Other_ByGenre)
				);
		}
	}
	else
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				//	If only the Source and EmbeddedSource
				//	are defined, then the objId maps to
				//	an individual track within a specific playlist
				//	that can be found as a child of a BaseFolder. 
				//	In this case, the parentId is simply the objId's fields
				//	without the embedded fields. We simply copy-by-value
				//	the fields over from objId and ensure the approrpiate
				//	'Embedded' fields are NULL.

				*parentId = *objId;
				parentId->EmbeddedSource = parentId->_EmbeddedSourcePostfix = parentId->EmbeddedSourcePrefix = NULL;
			}
			else
			{
				//	If the SubFolder and EmbeddedSource are not specified, 
				//	but the Source is specified, then tne objId maps to 
				//	an individual object that is a child of a standard 
				//	container. This means we simply need to use the 
				//	objId->BaseFolder to figure out who the parent happens 
				//	to be. Only certain standard containers have children,
				//	as noted by the ASSERT statements below.
				
				parentId->BaseFolder = objId->BaseFolder;

				ASSERT(
					(parentId->BaseFolder == WmlFolders_AllMedia) ||
					(parentId->BaseFolder == WmlFolders_Audio_AllAudio) ||
					(parentId->BaseFolder == WmlFolders_Video_AllVideo) ||
					(parentId->BaseFolder == WmlFolders_Other_AllOther) || 
					(parentId->BaseFolder == WmlFolders_MyPlaylists) ||
					(parentId->BaseFolder == WmlFolders_AutoPlaylists)
					);
			}
		}
		else
		{
			//	If neither Source nor SubFolder are specified,
			//	then the objId maps to a standard container.
			//	Standard containers have standard containers
			//	for parents, so we'll simply use the array
			//	that maps standard containers to their parents.

			parentId->BaseFolder = WML_StandardParents[objId->BaseFolder];
		}
	}
}

int WML_GetWmlObjFromPlaylist(/*IN*/struct WML_ObjId* objId, /*IN*/struct WML_Object *bc, /*INOUT*/struct WML_Object *wmlObj)
{
	int successCall = 0;
	//	The wmlObj->Playlist object represents the 
	//	collection of media to search.

	if (objId->SubFolder != NULL)
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				//	Defined: SubFolder, Source, and EmbeddedSource
				//	The wmlObj represents a track referenced within
				//	a playlist file.
				//	
				//	objId->Source will identify the desired
				//	playlist file. objId->EmbeddedSource will
				//	identify the desired track in the playlist file.
			}
			else
			{
				//	Defined: SubFolder, Source
				//	The wmlObj represents media referenced within
				//	a dynamic container.
			}
		}
		else
		{
			//	Defined: SubFolder
			//	The wmlObj represents a dynamic container, which
			//	is always a child to a standard container.
		}
	}
	else
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
			}
			else
			{
			}
		}
		else
		{
			if (objId->EmbeddedSource != NULL)
			{
			}
			else
			{
			}
		}
	}

	return successCall;
}

int WML_GetWmlObject(/*IN*/struct WML_ObjId* objId, /*INOUT*/struct WML_Object *wmlObj)
{
	int successCall = 0, i, j;
	long longVal, longVal2;
	struct WML_Object *bc;
	IWMPPlaylist *pl;
	CComBSTR bstr, bstr2, bstr3;
	int cmp, playlistOK;
	wchar_t *attribValue;

	wmlObj->Type = WmlObjType_Undefined;
	//wmlObj->Next = NULL;

	if (
		(objId->SubFolder != NULL) ||
		(objId->Source != NULL) ||
		(objId->EmbeddedSource != NULL)
		)
	{
		bc = &(WML_StandardContainers[objId->BaseFolder]);

		switch (bc->Type)
		{
		case WmlObjType_Playlist:
		case WmlObjType_FilteredPlaylist:
			// These containers have no sub folders.

			ASSERT(
				(objId->BaseFolder==WmlFolders_AllMedia) ||
				(objId->BaseFolder==WmlFolders_Audio_AllAudio) ||
				(objId->BaseFolder==WmlFolders_Video_AllVideo) ||
				(objId->BaseFolder==WmlFolders_Other_AllOther)
				//(objId->BaseFolder==WmlFolders_Radio)
				);

			if (objId->SubFolder == NULL)
			{
				if (objId->Source != NULL)
				{

					// objId maps to an IWMPMedia.
					// Iterate through the wmlObj->Playlist
					// and see if there's an appropriate match.
					pl = bc->Playlist.p;
					if (!FAILED(pl->get_count(&longVal)))
					{
						for (i=0; i < longVal; i++)
						{
							if ((bc->Type == WmlObjType_Playlist) || (!FAILED(wmlObj->Media->getItemInfo(bc->Attribute, &(bstr)))))
							{
								if ((bc->Type == WmlObjType_Playlist) || (wcsicmp(bc->AttributeValue, OLE2W(bstr)) == 0))
								{
									if (!FAILED(pl->get_item(i, &(wmlObj->Media))))
									{
										if (!FAILED(wmlObj->Media->get_sourceURL(&bstr)))
										{
											if (wcsicmp(objId->Source, OLE2W(bstr)) == 0)
											{
												if (objId->EmbeddedSource == NULL)
												{
													// The media does not represent an entry
													// in a playlist.
													if (bc->Type == WmlObjType_FilteredPlaylist)
													{
														if (!FAILED(wmlObj->Media->getItemInfo(bc->Attribute, &(bstr2))))
														{
															if (wcsicmp(bc->AttributeValue, OLE2W(bstr2)) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
															else
															{
																wmlObj->Media = NULL;
															}
														}
														else
														{
															wmlObj->Media = NULL;
														}
													}
													else
													{
														wmlObj->Media->getItemInfo(WML_ATTRIBUTE_MEDIATYPE, &bstr2);
														if (
															(wcsicmp(OLE2W(bstr2), WML_MEDIA_TYPE_AUDIO) == 0) ||
															(wcsicmp(OLE2W(bstr2), WML_MEDIA_TYPE_VIDEO) == 0) ||
															(wcsicmp(OLE2W(bstr2), WML_MEDIA_TYPE_RADIO) == 0)
															)
														{
															// This media is an individual content
															wmlObj->Type = WmlObjType_Media;
															successCall = 1;
														}
														else
														{
															// This media is a playlist
															if (!FAILED(The_PL->getAll(&(wmlObj->PlaylistArray))))
															{
																if (!FAILED(wmlObj->PlaylistArray->get_count(&longVal2)))
																{
																	for (j=0; j < longVal2; j++)
																	{
																		if (!FAILED(wmlObj->PlaylistArray->item(j, &(wmlObj->Playlist))))
																		{
																			if (!FAILED(wmlObj->Playlist->get_name(&bstr2)))
																			{
																				if (wcsnicmp(objId->_SourcePostfix, OLE2W(bstr2), bstr2.Length()) == 0)
																				{
																					// We're still going to mark this as a WmlObjType_Media
																					wmlObj->Type = WmlObjType_Media;
																					successCall = 1;
																					break;
																				}
																			}
																		}
																		
																		if (successCall == 0)
																		{
																			wmlObj->Playlist = NULL;
																		}
																	}
																}
															}
														}
														break;
													}
												}
												else
												{
													// The media represents an entry
													// in a playlist. So, find the appropriate
													// IWMPPlaylist that represents this playlist
													// and check to see if the info matches up.

													wmlObj->Media = NULL;
													if (!FAILED(The_PL->getAll(&(wmlObj->PlaylistArray))))
													{
														if (!FAILED(wmlObj->PlaylistArray->get_count(&longVal2)))
														{
															for (j=0; j < longVal2; j++)
															{
																if (!FAILED(wmlObj->PlaylistArray->item(j, &(wmlObj->Playlist))))
																{
																	if (!FAILED(wmlObj->Playlist->get_name(&bstr2)))
																	{
																		if (wcsnicmp(objId->_SourcePostfix, OLE2W(bstr2), bstr2.Length()) == 0)
																		{
																			//TODO: Comparing the filename is not sufficient.
																			// This code is buggy because if windows library
																			// has multiple playlists with the same name, then
																			// we have no way of mapping the IWMPPlaylist object
																			// to an IWMPMedia object. Until this is fixed,
																			// User will have to ensure that none of the 
																			// playlists have any name collisions.

																			if (!FAILED(wmlObj->Playlist->get_item(objId->EmbeddedTrackIndex, &(wmlObj->Media))))
																			{
																				if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
																				{
																					if (wcsicmp(objId->EmbeddedSource, OLE2W(bstr3))==0)
																					{
																						wmlObj->Type = WmlObjType_Media;
																						successCall = 1;
																						break;
																					}
																					else
																					{
																						wmlObj->Media = NULL;
																					}
																				}
																				else
																				{
																					wmlObj->Media = NULL;
																				}
																			}
																			else
																			{
																				wmlObj->Media = NULL;
																			}
																		}
																		else
																		{
																			wmlObj->Playlist = NULL;
																		}
																	}
																	else
																	{
																		wmlObj->Playlist = NULL;
																	}
																}
																else
																{
																	wmlObj->Playlist = NULL;
																}
															}
														}
													}
													wmlObj->PlaylistArray = NULL;
												}
												break;
											}
											else
											{
												wmlObj->Media = NULL;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			break;

		case WmlObjType_PlaylistArray:
			// Get a playlist or an individual track within a playlist.
			// Note that neither WmlFolders_AutoPlaylists nor WmlFolders_MyPlaylists
			// are subdivided into dynamic containers.

			ASSERT(
				(objId->BaseFolder==WmlFolders_AutoPlaylists) ||
				(objId->BaseFolder==WmlFolders_MyPlaylists)
				);

			if (objId->SubFolder == NULL)
			{
				if (objId->Source != NULL)
				{
					if (!FAILED(bc->PlaylistArray->get_count(&longVal)))
					{
						for (i=0; i < longVal; i++)
						{
							wmlObj->Playlist = NULL;
							playlistOK = 0;
							if (!FAILED(bc->PlaylistArray->item(i, &(wmlObj->Playlist))))
							{
								if (!FAILED(wmlObj->Playlist->getItemInfo(WML_ATTRIBUTE_PLAYLISTTYPE, &bstr)))
								{
									//
									//	This code has a bug because there's no way for us to correlate
									//

									cmp = wcsicmp(WML_MEDIA_PLAYLIST_TYPE_AUTO, OLE2W(bstr));
									if (
										(objId->BaseFolder == WmlFolders_AutoPlaylists) &&
										(cmp ==0 )
										)
									{
										if (!FAILED(wmlObj->Playlist->get_name(&bstr2)))
										{
											if (wcsicmp(objId->_SourcePostfix, OLE2W(bstr2)) == 0)
											{
												playlistOK = 1;
											}
										}
									}
									else if (
										(objId->BaseFolder == WmlFolders_MyPlaylists) &&
										(cmp != 0)
										)
									{
										if (!FAILED(wmlObj->Playlist->get_name(&bstr2)))
										{
											if (wcsicmp(objId->_SourcePostfix, OLE2W(bstr2)) == 0)
											{
												playlistOK = 1;
											}
										}
									}
								}
							}

							if (playlistOK != 0)
							{
								if (objId->EmbeddedSource == NULL)
								{
									wmlObj->Type = WmlObjType_Playlist;
									successCall = 1;
									break;
								}
								else
								{
									if (!FAILED(wmlObj->Playlist->get_item(objId->EmbeddedTrackIndex, &(wmlObj->Media))))
									{
										if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
										{
											if (wcsicmp(objId->EmbeddedSource, OLE2W(bstr3))==0)
											{
												wmlObj->Type = WmlObjType_Media;
												successCall = 1;
												break;
											}
											else
											{
												wmlObj->Media = NULL;
											}
										}
										else
										{
											wmlObj->Media = NULL;
										}
									}
									else
									{
										wmlObj->Media = NULL;
									}
								}
							}
						}
					}
				}
				else
				{
					// must specify a playlist source
				}
			}
			else
			{
				// we have no dynamic container organization...
			}

			break;

		case WmlObjType_StringCollection:
			ASSERT(
				(objId->BaseFolder==WmlFolders_Audio_ByAlbum) ||
				(objId->BaseFolder==WmlFolders_Audio_ByArtist) ||
				(objId->BaseFolder==WmlFolders_Audio_ByGenre) ||
				(objId->BaseFolder==WmlFolders_Video_ByActor) ||
				(objId->BaseFolder==WmlFolders_Video_ByGenre)
				);

			if (
				(objId->SubFolder != NULL) &&
				(objId->EmbeddedSource == NULL)
				)
			{
				cmp = wcsicmp(objId->SubFolder, L"Unknown");
				if (cmp == 0)
				{
					attribValue = L"";
				}
				else
				{
					attribValue = objId->SubFolder;
				}

				if (!FAILED(bc->StringCollection->get_count(&longVal)))
				{
					for (i=0; i < longVal; i++)
					{
						if (!FAILED(bc->StringCollection->item(i, &bstr)))
						{
							if (
								(wcsicmp(objId->SubFolder, OLE2W(bstr))==0) ||
								(cmp == 0)
								)
							{
								// object is an IWMPPlaylist representing all
								// media with attributes that conforms to the 
								// intent of the dynamic container/subfolder's

								if (objId->BaseFolder == WmlFolders_Audio_ByAlbum)
								{
									wmlObj->Playlist = NULL;
									if (!FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_ALBUMTITLE, attribValue, &(wmlObj->Playlist))))
									{
										if (objId->Source == NULL)
										{
											wmlObj->Type = WmlObjType_FilteredPlaylist;
											wmlObj->Attribute = WML_ATTRIBUTE_MEDIATYPE;
											wmlObj->AttributeValue = WML_MEDIA_TYPE_AUDIO;
											successCall = 1;
										}
										else
										{
											if (!FAILED(wmlObj->Playlist->get_count(&longVal2)))
											{
												for (j=0; j < longVal2; j++)
												{
													wmlObj->Media = NULL;
													if (!FAILED(wmlObj->Playlist->get_item(j, &(wmlObj->Media))))
													{
														if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
														{
															if (wcscmp(OLE2W(bstr3), objId->Source) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
														}
													}
												}
											}
											else
											{
												wmlObj->Playlist = NULL;
											}
										}
										break;
									}
								}
								else if (objId->BaseFolder == WmlFolders_Audio_ByArtist)
								{
									wmlObj->Playlist = NULL;
									if (!FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_AUTHOR, attribValue, &(wmlObj->Playlist))))
									{
										if (objId->Source == NULL)
										{
											wmlObj->Type = WmlObjType_FilteredPlaylist;
											wmlObj->Attribute = WML_ATTRIBUTE_MEDIATYPE;
											wmlObj->AttributeValue = WML_MEDIA_TYPE_AUDIO;
											successCall = 1;
										}
										else
										{
											if (!FAILED(wmlObj->Playlist->get_count(&longVal2)))
											{
												for (j=0; j < longVal2; j++)
												{
													wmlObj->Media = NULL;
													if (!FAILED(wmlObj->Playlist->get_item(j, &(wmlObj->Media))))
													{
														if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
														{
															if (wcscmp(OLE2W(bstr3), objId->Source) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
														}
													}
												}
											}
											else
											{
												wmlObj->Playlist = NULL;
											}
										}
										break;
									}
								}
								else if (objId->BaseFolder == WmlFolders_Audio_ByGenre)
								{
									wmlObj->Playlist = NULL;
									if (!FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_GENRE, attribValue, &(wmlObj->Playlist))))
									{
										if (objId->Source == NULL)
										{
											wmlObj->Type = WmlObjType_FilteredPlaylist;
											wmlObj->Attribute = WML_ATTRIBUTE_MEDIATYPE;
											wmlObj->AttributeValue = WML_MEDIA_TYPE_AUDIO;
											successCall = 1;
										}
										else
										{
											if (!FAILED(wmlObj->Playlist->get_count(&longVal2)))
											{
												for (j=0; j < longVal2; j++)
												{
													wmlObj->Media = NULL;
													if (!FAILED(wmlObj->Playlist->get_item(j, &(wmlObj->Media))))
													{
														if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
														{
															if (wcscmp(OLE2W(bstr3), objId->Source) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
														}
													}
												}
											}
											else
											{
												wmlObj->Playlist = NULL;
											}
										}
										break;
									}
								}
								else if (objId->BaseFolder == WmlFolders_Video_ByActor)
								{
									wmlObj->Playlist = NULL;
									if (!FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_AUTHOR, attribValue, &(wmlObj->Playlist))))
									{
										if (objId->Source == NULL)
										{
											wmlObj->Type = WmlObjType_FilteredPlaylist;
											wmlObj->Attribute = WML_ATTRIBUTE_MEDIATYPE;
											wmlObj->AttributeValue = WML_MEDIA_TYPE_VIDEO;
											successCall = 1;
										}
										else
										{
											if (!FAILED(wmlObj->Playlist->get_count(&longVal2)))
											{
												for (j=0; j < longVal2; j++)
												{
													wmlObj->Media = NULL;
													if (!FAILED(wmlObj->Playlist->get_item(j, &(wmlObj->Media))))
													{
														if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
														{
															if (wcscmp(OLE2W(bstr3), objId->Source) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
														}
													}
												}
											}
											else
											{
												wmlObj->Playlist = NULL;
											}
										}
										break;
									}
								}
								else if (objId->BaseFolder == WmlFolders_Video_ByGenre)
								{
									wmlObj->Playlist = NULL;
									if (!FAILED(The_MC->getByAttribute(WML_ATTRIBUTE_GENRE, attribValue, &(wmlObj->Playlist))))
									{
										if (objId->Source == NULL)
										{
											wmlObj->Type = WmlObjType_FilteredPlaylist;
											wmlObj->Attribute = WML_ATTRIBUTE_MEDIATYPE;
											wmlObj->AttributeValue = WML_MEDIA_TYPE_VIDEO;
											successCall = 1;
										}
										else
										{
											if (!FAILED(wmlObj->Playlist->get_count(&longVal2)))
											{
												for (j=0; j < longVal2; j++)
												{
													wmlObj->Media = NULL;
													if (!FAILED(wmlObj->Playlist->get_item(j, &(wmlObj->Media))))
													{
														if (!FAILED(wmlObj->Media->get_sourceURL(&bstr3)))
														{
															if (wcscmp(OLE2W(bstr3), objId->Source) == 0)
															{
																wmlObj->Type = WmlObjType_Media;
																successCall = 1;
																break;
															}
														}
													}
												}
											}
											else
											{
												wmlObj->Playlist = NULL;
											}
										}
										break;
									}
								}
								else
								{
									ASSERT(0);
								}
							}
						}
					}
				}

			}

			break;

		case WmlObjType_WmlObjArray:
			// No objects can be found here.
			break;

		default:
			//	Logic error in code
			ASSERT(0);
			break;
		}
	}
	else
	{
		*wmlObj = WML_StandardContainers[objId->BaseFolder];
		successCall = 1;
	}

	return successCall;
}

struct WML_Object* WML_CreateWmlObject()
{
	struct WML_Object* wmlObj;
	
	wmlObj = new WML_Object();
	//wmlObj->Next = NULL;
	wmlObj->WmlObjArray = NULL;
	wmlObj->WmlObjArrayLen = 0;
	wmlObj->Type = WmlObjType_Undefined;

	return wmlObj;
}

void WML_DestroyWmlObjectList(struct WML_Object* wmlObj)
{
	struct WML_Object* current, *next;

	current = wmlObj;
	while (current != NULL)
	{
		//next = current->Next;
		delete (current);
		//current = next;
		current = NULL;
	}
}

void WML_PerformBrowse(struct MSL_CdsQuery *browseArgs, struct CdsStringBrowseArgsWide* wideArgs, int requestedOnAddress, int requestedOnPort)
{

	struct WML_Object *wmlObj = NULL;
	struct WML_ObjId* objId = NULL;
	enum Enum_CdsErrors error = CdsError_None;
	//struct WML_Object wmlObj;
	int *addresses;
	int numAddresses;
	int i, swapValue;
	struct WML_Object staticObj;
	
	objId = WML_GetWmlObjId(wideArgs->ObjectID);

	if (objId != NULL)
	{
		// Obtain all of the local IP addresses
		sem_wait(&The_IPAddressListLock);
		numAddresses = The_IPAddressLength;
		addresses = (int*) malloc(sizeof(int) * numAddresses);
		memcpy(addresses, The_IPAddressList, The_IPAddressLength*sizeof(int));
		sem_post(&The_IPAddressListLock);

		// Ensure the order of addresses is correct
		if (addresses[0] != requestedOnAddress)
		{
			swapValue = addresses[0];
			addresses[0] = requestedOnAddress;
			for (i=1; i < numAddresses; i++)
			{
				if (addresses[i] == requestedOnAddress)
				{
					addresses[i] = swapValue;
					break;
				}
			}
		}

		wmlObj = &staticObj;
		if (WML_GetWmlObject(objId, wmlObj) != 0)
		{
		}
		else
		{
			wmlObj = NULL;
		}

		// we're doing a BrowseMetadata or BrowseDirectChildren on a standard container
		if (wmlObj != NULL)
		{
			if (wmlObj->Type != WmlObjType_Undefined)
			{
				WML_PerformBrowse_DidlResponse(browseArgs, objId, wmlObj, addresses, numAddresses, requestedOnPort);
			}
			else
			{
				error = CdsError_NoSuchObject;
			}
		}
		else
		{
			error = CdsError_NoSuchObject;
		}

		free(addresses);
		if (objId->SubFolder != NULL) free(objId->SubFolder);
		if (objId->Source != NULL) free(objId->Source);
		if (objId->EmbeddedSource != NULL) free(objId->EmbeddedSource);
		if (objId->SourcePrefix != NULL) free(objId->SourcePrefix);
		if (objId->EmbeddedSourcePrefix != NULL) free(objId->EmbeddedSourcePrefix);
		free(objId);
	}
	else
	{
		error = CdsError_NoSuchObject;
	}

	if (error != CdsError_None)
	{
		WML_RespondWithError(browseArgs, error);
	}
}

void WML_DoBrowseMetadata(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *updateId)
{
	char *utf8_ParentId = NULL;
	int utf8_ParentIdLen;
	unsigned int filter;
	struct WML_ObjId parentId;

	WML_GetParentObjId(objId, &parentId);
	utf8_ParentId = WML_WmlObjIdToUtf8(&parentId);
	utf8_ParentIdLen = (int) strlen(utf8_ParentId);
	filter = CdsToDidl_GetFilterBitString(browseArgs->Filter);

	if (0)//if (wmlObj->Next != NULL)
	{
		//	We have a list of WML_Objects and we need to respond
		//	with the DIDL-Lite for the container object that
		//	would be the parent for these WML_Objects.
		//	
		//	This code should never execute. It has been deprecated.
		ASSERT(0);
		
		//ASSERT((objId->Source == NULL) && (objId->SubFolder == NULL));
		//ASSERT(
		//	(objId->BaseFolder == WmlFolders_Root) ||
		//	(objId->BaseFolder == WmlFolders_Audio) ||
		//	(objId->BaseFolder == WmlFolders_Video) ||
		//	(objId->BaseFolder == WmlFolders_Other)
		//	);

		//WML_RespondWithContainerFromWmlObjectList(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, filter, addresses, addressesLen, port, wmlObj);
	}
	else
	{
		switch (wmlObj->Type)
		{
		case WmlObjType_Media:
			//	Respond with a single DIDL-Lite CDS object for the 
			//	windows media object represented in wmlObj->Media.
			//	It should be noted that if wmlObj->Media represents a playlist
			//	it will be represented through a <container> element.

			WML_RespondWithItemFromMedia(browseArgs, &parentId, utf8_ParentId, utf8_ParentIdLen, filter, addresses, addressesLen, port, wmlObj->Media.p);
			break;

		case WmlObjType_Playlist:
			//	Respond with a DIDL-Lite <container> element that represents
			//	the information about the playlist represented in wmlObj->Playlist.

			//if (objId->BaseFolder == WmlFolders_Audio_ByAlbum)
			//{
			//	WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_MUSICALBUM, filter, addresses, addressesLen, port, wmlObj);
			//}
			//else if (
			//	(objId->BaseFolder == WmlFolders_Audio_ByArtist) ||
			//	(objId->BaseFolder == WmlFolders_Video_ByActor)
			//	)
			//{
			//	WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_PERSON, filter, addresses, addressesLen, port, wmlObj);
			//}
			//else if (
			//	(objId->BaseFolder == WmlFolders_Audio_ByGenre) ||
			//	(objId->BaseFolder == WmlFolders_Video_ByGenre)
			//	)
			//{
			//	WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_GENRE, filter, addresses, addressesLen, port, wmlObj);
			//}
			//else
			//{
			//	WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, 0, filter, addresses, addressesLen, port, wmlObj);
			//}
			WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_ALBUM, filter, addresses, addressesLen, port, wmlObj);
			break;

		case WmlObjType_FilteredPlaylist:
			//	Respond with a DIDL-Lite <container> element that represents
			//	the information about the playlist represented in wmlObj->Playlist.
			//	Be sure that the contents of the playlist are filtered appropriately
			//	given the wmlObj->SubFolder field.

			ASSERT(
				(objId->SubFolder != NULL) &&
				(parentId.SubFolder == NULL)
				);

			if (objId->BaseFolder == WmlFolders_Audio_ByAlbum)
			{
				WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_MUSICALBUM, filter, addresses, addressesLen, port, wmlObj);
			}
			else if (
				(objId->BaseFolder == WmlFolders_Audio_ByArtist) ||
				(objId->BaseFolder == WmlFolders_Video_ByActor)
				)
			{
				WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_PERSON, filter, addresses, addressesLen, port, wmlObj);
			}
			else if (
				(objId->BaseFolder == WmlFolders_Audio_ByGenre) ||
				(objId->BaseFolder == WmlFolders_Video_ByGenre)
				)
			{
				WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, CDS_MEDIACLASS_GENRE, filter, addresses, addressesLen, port, wmlObj);
			}
			else
			{
				WML_RespondWithContainerFromPlaylist(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, NULL, 0, filter, addresses, addressesLen, port, wmlObj);
			}
			break;

		case WmlObjType_StringCollection:
			//	Respond with a DIDL-Lite <container> element that is parent
			//	to a variety of SubFolder-specified containers.
			//	This code should only execute for a container object that does
			//	not have a subfolder specified. Example containers include:
			//		WmlFolders_Audio_ByAlbum,
			//		WmlFolders_Audio_ByArtist,
			//		WmlFolders_Audio_ByGenre

			ASSERT(
				(objId->SubFolder == NULL) &&
				(objId->Source == NULL)
				);

			WML_RespondWithContainerFromStringCollection(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, filter, addresses, addressesLen, port, wmlObj->StringCollection.p);
			break;

		case WmlObjType_PlaylistArray:
			//	Respond with a DIDL-Lite <container> element that represents
			//	the information about the container that owns these playlists.

			WML_RespondWithContainerFromPlaylistArray(browseArgs, objId, &parentId, utf8_ParentId, utf8_ParentIdLen, filter, addresses, addressesLen, port, wmlObj);
			break;

		case WmlObjType_WmlObjArray:
			//	We have a list of WML_Objects and we need to respond
			//	with the DIDL-Lite for the container object that
			//	would be the parent for these WML_Objects.
			//
			//	Only special folders standard containers should
			//	execute with this code. They are listed in the
			//	ASSERT statements below.

			ASSERT((objId->Source == NULL) && (objId->SubFolder == NULL));
			ASSERT(
				(objId->BaseFolder == WmlFolders_Root) ||
				(objId->BaseFolder == WmlFolders_Audio) ||
				(objId->BaseFolder == WmlFolders_Video) ||
				(objId->BaseFolder == WmlFolders_Other)
				);

			WML_RespondWithContainerFromWmlObjArray(browseArgs, objId, &parentId, filter, addresses, addressesLen, port, wmlObj->WmlObjArray, wmlObj->WmlObjArrayLen, utf8_ParentId, utf8_ParentIdLen);
			break;

		default:
			// Logic error in the code
			ASSERT(0);
			break;
		}
	}

	free (utf8_ParentId);
}

void WML_RespondWithSetFromStringCollection(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, IWMPStringCollection *strColl, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID, 	char *utf8_ParentID, int utf8_ParentIDLen)
{
	HRESULT hr;
	long childCount;
	struct CdsMediaObject *cdsObj;
	CComBSTR bstr, bstr2;
	long pi;
	char *didl;
	unsigned int filter, mediaClass;
	int errorOrSize, startIndex, endIndex;
	char *appendId;
	int appendIdLen;
	//CComPtr <IWMPPlaylist> container;
	struct WML_Object childWml;
	struct WML_ObjId childObjId;
	CComPtr <IWMPMedia> media;
	
	char title[255];
	int titleLen;

	char creator[255];
	int creatorLen;

	hr = strColl->get_count(&childCount);

	startIndex = 0;
	endIndex = (int)childCount;

	if (browseArgs->StartingIndex > 0)
	{
		startIndex = browseArgs->StartingIndex;
	}

	if (browseArgs->RequestedCount > 0)
	{
		endIndex = startIndex + browseArgs->RequestedCount - 1;
	}

	if (endIndex > childCount-1)
	{
		endIndex = childCount-1;
	}

	if (SUCCEEDED(hr))
	{
		*totalMatches = abs(childCount);
		filter = CdsToDidl_GetFilterBitString(browseArgs->Filter);

		for (pi = startIndex; pi <= endIndex; pi++)
		{
			hr = strColl->item(pi, &bstr);

			if (SUCCEEDED(hr))
			{
				if (bstr.Length() == 0)
				{
					bstr.Append(L"Unknown");
				}

				childObjId = *parentID;
				childObjId.SubFolder = OLE2W(bstr);

				switch(parentID->BaseFolder)
				{
				case WmlFolders_Audio_ByAlbum:
					mediaClass = CDS_MEDIACLASS_ALBUM;
					titleLen = strToUtf8(title, (char*)childObjId.SubFolder, 255, 1, NULL);

					//TODO - get creator of album
					creatorLen = sprintf(creator, WML_CREATOR_USER);
					break;

				case WmlFolders_Audio_ByArtist:
				case WmlFolders_Video_ByActor:
					mediaClass = CDS_MEDIACLASS_PERSON;
					titleLen = strToUtf8(title, (char*)childObjId.SubFolder, 255, 1, NULL);
					creatorLen = strToUtf8(creator, (char*)childObjId.SubFolder, 255, 1, NULL);
					break;

				case WmlFolders_Audio_ByGenre:
				case WmlFolders_Video_ByGenre:
					mediaClass = CDS_MEDIACLASS_CONTAINER;
					titleLen = strToUtf8(title, (char*)childObjId.SubFolder, 255, 1, NULL);
					creatorLen = sprintf(creator, WML_CREATOR_USER);
					break;

				default:
					mediaClass = CDS_MEDIACLASS_CONTAINER;
					creatorLen = sprintf(creator, WML_CREATOR_USER);
				}

				ASSERT(childObjId.Source == NULL);
				ASSERT(childObjId.EmbeddedSource == NULL);

				WML_GetWmlObject(&childObjId, &childWml);

				cdsObj = CDS_AllocateObject();


				appendIdLen = bstr.Length()*2+1;
				appendId = (char*) malloc(appendIdLen);
				appendIdLen = strToUtf8(appendId, (char*)childObjId.SubFolder, appendIdLen, 1, NULL);

				WML_SetCdsObjFromPlaylist(utf8_ParentID, utf8_ParentIDLen, appendId, appendIdLen, creator, creatorLen, mediaClass, addresses, addressesLen, port, childWml.Playlist.p, childWml.Attribute, childWml.AttributeValue, filter, cdsObj);
				didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);

				free (appendId);
				if (errorOrSize >= 0)
				{
					(*numberReturned)++;
					MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
				}

				/* deallocate the DIDL-Lite string */
				if (didl!=NULL) { free(didl); }

				/*
				 *	Deallocate everything associated with cdsObj.
				 */
				CDS_DestroyObjects(cdsObj);
			}
			else
			{
				fprintf(stderr, "WML_PerformBrowse_DidlResponse() failed a WML call.");
			}
		}
	}
	else
	{
		fprintf(stderr, "WML_PerformBrowse_DidlResponse() failed a WML call.");
	}
}

void WML_RespondWithContainerFromPlaylist(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, char *utf8Creator, unsigned int mediaClass, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObj)
{
	struct CdsMediaObject *cdsObj;
	long numObjects;
	IWMPPlaylist *pl;
	int i, size;
	CComPtr <IWMPMedia> media;
	CComBSTR bstr;
	int errorOrSize;
	char *didl;
	CComBSTR creator;

	pl = wmlObj->Playlist.p;

	//	This responds with a <container> object to represent the wmlObj->Playlist field. 
	//	The method appropriately excludes results that do not match the values at
	//	wml->Attribute and wmlObj->AttributeValue.

	cdsObj = CDS_AllocateObject();
	
	//
	//	Restricted and not searchable
	//
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	//
	//	Assign ID
	//
	cdsObj->ID = WML_WmlObjIdToUtf8(objId);

	//
	//	Assign ParentID - be sure to set to NULL before destroying
	//
	cdsObj->ParentID = utf8_ParentID;

	//
	//	Always a container
	//
	if (mediaClass & CDS_MEDIACLASS_CONTAINER)
	{
		cdsObj->MediaClass = mediaClass;
	}
	else
	{
		cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;
	}


	//
	//	Assign Title
	//
	size = WML_WmlObjToUtf8TitleLen(objId) + 1;
	cdsObj->Title = (char*) malloc(size);
	WML_WmlObjToUtf8Title(cdsObj->Title, size, objId);

	//
	//	Determine the creator - be sure to set to NULL
	//
	pl->getItemInfo(WML_ATTRIBUTE_MEDIACLASS2, &bstr);
	if (utf8Creator != NULL)
	{
		cdsObj->Creator = utf8Creator;
	}
	else if (wcsicmp(OLE2W(bstr), WML_MEDIA_CLASS2_AUTO_PLAYLIST) == 0)
	{
		//	Windows Media Library created this playlist
		cdsObj->Creator = WML_CREATOR_WML;
	}
	else
	{
		// User created this playlist
		cdsObj->Creator = WML_CREATOR_USER;
	}
	
	//
	//	Determine the childCount value;
	//
	pl->get_count(&numObjects);
	if (wmlObj->Type == WmlObjType_FilteredPlaylist)
	{
		// This is only for filteredPlaylists.

		ASSERT(wmlObj->AttributeValue != NULL);
		ASSERT(wmlObj->Attribute != NULL);

		//	Count the number of items in the playlist
		//	that match the criteria specified in
		//	wmlObj->Attribute and wmlObj->AttributeValue.

		cdsObj->ChildCount = 0;
		for (i=0; i < numObjects; i++)
		{
			pl->get_item(i, &media);

			media->getItemInfo(wmlObj->Attribute, &bstr);
			if (wcsicmp(OLE2W(bstr), wmlObj->AttributeValue) == 0)
			{
				cdsObj->ChildCount++;
			}

			media = NULL;
		}
	}
	else
	{
		// this is for normal Playlists
		cdsObj->ChildCount = (int) numObjects;
	}

	didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);
	if (errorOrSize >= 0)
	{
		MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
	}
	free(didl);

	cdsObj->Creator = cdsObj->ParentID = NULL;
	CDS_DestroyObjects(cdsObj);
}

void WML_RespondWithContainerFromPlaylistArray(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObj)
{
	long numCount;
	int i;
	IWMPPlaylistArray *pa;
	CComPtr <IWMPPlaylist> pl;
	CComBSTR bstr;
	
	struct CdsMediaObject *cdsObj;
	int size, errorOrSize;
	char *didl;

	pa = wmlObj->PlaylistArray.p;
	pa->get_count(&numCount);

	//	This responds with a <container> object to represent the wmlObj->PlaylistArray field. 
	//	The type of playlist to find is determined by wmlObj->PlaylistArrayIsAutoType

	cdsObj = CDS_AllocateObject();
	
	//
	//	Restricted and not searchable
	//
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	//
	//	Assign ID
	//
	cdsObj->ID = WML_WmlObjIdToUtf8(objId);

	//
	//	Assign ParentID - be sure to set to NULL before destroying
	//
	cdsObj->ParentID = utf8_ParentID;

	//
	//	Always a container
	//
	cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;

	//
	//	Assign Title
	//
	size = WML_WmlObjToUtf8TitleLen(objId) + 1;
	cdsObj->Title = (char*) malloc(size);
	WML_WmlObjToUtf8Title(cdsObj->Title, size, objId);

	//
	//	Determine the creator - be sure to NULL field
	//
	if (wmlObj->PlaylistArrayIsAutoType != 0)
	{
		cdsObj->Creator = WML_CREATOR_WML;
	}
	else
	{
		cdsObj->Creator = WML_CREATOR_USER;
	}
	
	//
	//	Determine the childCount value;
	//
	cdsObj->ChildCount = 0;
	for (i=0; i < numCount; i++)
	{
		pa->item(i, &pl);

		pl->getItemInfo(WML_ATTRIBUTE_PLAYLISTTYPE, &bstr);

		if (wcsicmp(OLE2W(bstr), WML_MEDIA_PLAYLIST_TYPE_AUTO) == 0)
		{
			if (wmlObj->PlaylistArrayIsAutoType != 0)
			{
				cdsObj->ChildCount++;
			}
		}
		else
		{
			if (wmlObj->PlaylistArrayIsAutoType == 0)
			{
				cdsObj->ChildCount++;
			}
		}

		pl = NULL;
	}


	didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);
	if (errorOrSize >= 0)
	{
		MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
	}
	free(didl);

	cdsObj->Creator = cdsObj->ParentID = NULL;
	CDS_DestroyObjects(cdsObj);
}

void WML_RespondWithContainerFromStringCollection(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, IWMPStringCollection *strCol)
{
	struct CdsMediaObject *cdsObj;
	long numObjects;
	int size, errorOrSize;
	CComBSTR bstr;
	char *didl;

	//	This responds with a <container> object to represent the wmlObj->StringCollection field. 

	cdsObj = CDS_AllocateObject();
	
	//
	//	Restricted and not searchable
	//
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

	//
	//	Assign ID
	//
	cdsObj->ID = WML_WmlObjIdToUtf8(objId);

	//
	//	Assign ParentID - be sure to set to NULL before destroying
	//
	cdsObj->ParentID = utf8_ParentID;

	//
	//	Always a container
	//
	cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;

	//
	//	Assign Title
	//
	size = WML_WmlObjToUtf8TitleLen(objId) + 1;
	cdsObj->Title = (char*) malloc(size);
	WML_WmlObjToUtf8Title(cdsObj->Title, size, objId);

	//
	//	Determine the creator - be sure to NULL the field
	//
	cdsObj->Creator = WML_CREATOR_WML;
	
	//
	//	Determine the childCount value;
	//
	strCol->get_count(&numObjects);
	cdsObj->ChildCount = (int) numObjects;

	didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);
	if (errorOrSize >= 0)
	{
		MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
	}
	free(didl);

	cdsObj->Creator = cdsObj->ParentID = NULL;
	CDS_DestroyObjects(cdsObj);
}

void WML_RespondWithContainerFromWmlObjArray(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object **wmlObjArray, int wmlObjArrayLen, char *utf8_ParentID, int utf8_ParentIDLen)
{
	struct CdsMediaObject *cdsObj;
	char *didl, *utf8_objId;
	int errorOrSize;
	int utf8_objIdLen;

	ASSERT(objId->Source == NULL);
	ASSERT(objId->SubFolder == NULL);

	utf8_objId = WML_WmlObjIdToUtf8(objId);
	utf8_objIdLen = (int) strlen(utf8_objId);

	cdsObj = CDS_AllocateObject();

	/* optimize by not doing memcopies */
	cdsObj->RefID = NULL;
	cdsObj->ID = WML_ContainerID[objId->BaseFolder];
	cdsObj->ParentID = WML_ContainerID[parentId->BaseFolder];
	cdsObj->Title = WML_ContainerTitle[objId->BaseFolder];
	cdsObj->Creator = "Windows Media Library";
	cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;
	cdsObj->ChildCount = wmlObjArrayLen;
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;
	cdsObj->Next = NULL;
	cdsObj->RefID = NULL;
	cdsObj->Res = NULL;

	didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);

	if (errorOrSize >= 0)
	{
		MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
	}

	/* deallocate the DIDL-Lite string */
	if (didl != NULL) { free(didl); }

	/*
	 *	Deallocate everything associated with cdsObj.
	 *	Ensure that we simply set fields that are pointing to static/global allocations to null.
	 */
	cdsObj->ID = cdsObj->Title = cdsObj->Creator = cdsObj->ParentID = NULL;
	CDS_DestroyObjects(cdsObj);

	free(utf8_objId);
}

void WML_RespondWithContainerFromWmlObjectList(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_ObjId* parentId, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, struct WML_Object *wmlObjList)
{
	struct CdsMediaObject *cdsObj;

	//TODO(IGNORE): WML_RespondWithContainerFromWmlObjectList
	unsigned int numberReturned=0, totalMatches=0, updateID=0;

	cdsObj = CDS_AllocateObject();

	// Always restricted... and never searchable for now
	cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;


	cdsObj->ID = NULL;
	cdsObj->MediaClass = 0;
	cdsObj->ParentID = NULL;
	cdsObj->Title = NULL;

	CDS_DestroyObjects(cdsObj);
}

void WML_RespondWithItemFromMedia(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, char *utf8_ParentID, int utf8_ParentIDLen, unsigned int filter, int *addresses, int addressesLen, int port, IWMPMedia *media)
{
	struct CdsMediaObject *cdsObj;
	char *didl;
	int errorOrSize;

	cdsObj = CDS_AllocateObject();

	if ((parentID->BaseFolder == WmlFolders_AllMedia) && (parentID->SubFolder == NULL) && (parentID->Source == NULL) && (parentID->EmbeddedSource == NULL))
	{
		WML_SetCdsObjFromMedia(utf8_ParentID, utf8_ParentIDLen, 1, -1, NULL, 0, addresses, addressesLen, port, media, filter, cdsObj);
	}
	else
	{
		WML_SetCdsObjFromMedia(utf8_ParentID, utf8_ParentIDLen, 0, parentID->EmbeddedTrackIndex, The_AllMediaObjId, The_AllMediaObjIdLen, addresses, addressesLen, port, media, filter, cdsObj);
	}
	didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, filter, 0, &errorOrSize);

	if (errorOrSize >= 0)
	{
		MSL_ForQueryResponse_ResultArgument(browseArgs, didl, (int)strlen(didl));
	}
	else
	{
		ASSERT(0);
	}

	/* deallocate the DIDL-Lite string */
	if (didl!=NULL) { free(didl); }

	/*
		*	Deallocate everything associated with cdsObj.
		*/
	CDS_DestroyObjects(cdsObj);
}

void WML_RespondWithSetFromPlaylist(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* parentID, IWMPPlaylist *playlist, wchar_t *attribFilter, wchar_t *attribFilterValue, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID, char *utf8_ParentID, int utf8_ParentIDLen)
{
	CComPtr <IWMPMedia> media;
	long childCount;
	HRESULT hr;
	unsigned int filter;
	int pi = 0;
	CComBSTR bstr;
	int ok = 1, startIndex, endIndex;

	hr = playlist->get_count(&childCount);

	startIndex = 0;
	endIndex = (int)childCount;

	if (browseArgs->StartingIndex > 0)
	{
		startIndex = browseArgs->StartingIndex;
	}

	if (browseArgs->RequestedCount > 0)
	{
		endIndex = startIndex + browseArgs->RequestedCount - 1;
	}

	if (endIndex > childCount-1)
	{
		endIndex = childCount-1;
	}

	
	if (SUCCEEDED(hr))
	{
		filter = CdsToDidl_GetFilterBitString(browseArgs->Filter);
		
		for (pi = startIndex; pi <= endIndex; pi++)
		{
			media = NULL;
			hr = playlist->get_item(pi, &media);
			if (SUCCEEDED(hr))
			{
				if (attribFilter != NULL)
				{
					media->getItemInfo(attribFilter, &bstr);
					if (wcsicmp(OLE2W(bstr), attribFilterValue) == 0)
					{
						ok = 1;
					}
					else
					{
						ok = 0;
					}
				}

				if (ok)
				{
					// TODO - support RANGE/incremental responses
					WML_RespondWithItemFromMedia(browseArgs, parentID, utf8_ParentID, utf8_ParentIDLen, filter, addresses, addressesLen, port, media.p);
					(*totalMatches)++;
					(*numberReturned)++;
				}
			}
			else
			{
				fprintf(stderr, "WML_PerformBrowse_DidlResponse() failed a WML call.");
			}
		}
	}
	else
	{
		fprintf(stderr, "WML_PerformBrowse_DidlResponse() failed a WML call.");
	}
}

void WML_DoBrowseDirectChildren(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port, /*INOUT*/ unsigned int *numberReturned, unsigned int *totalMatches, unsigned int *updateID)
{
	// TODO: WML_DoBrowseDirectChildren
	char *utf8_ParentId = NULL;
	int utf8_ParentIdLen;
	unsigned int filter;
	struct WML_ObjId parentId;
	struct WML_ObjId childObjId; 
	unsigned int childUpdateId;
	int i, startIndex, endIndex;

	WML_GetParentObjId(objId, &parentId);
	filter = CdsToDidl_GetFilterBitString(browseArgs->Filter);

	if (0)//if (wmlObj->Next != NULL)
	{
		// We have a linked list of WML_Objects
		// and we want to serialize all of them to
		// DIDL as a flat list of children.
		ASSERT(0);
	}
	else
	{
		switch (wmlObj->Type)
		{
		case WmlObjType_Playlist:
		case WmlObjType_FilteredPlaylist:
			// We have a playlist object to represent all of the children.
			// Each element in the playlist represents an element in the result list.
			// Result elements can include <item> and <container> elements.
			// <container> elements will always be of type object.container.playlistContainer

			//if (objId->BaseFolder == WmlFolders_AllMedia)
			//{
			//	objId->SubFolder = L"";
			//}
			utf8_ParentId = WML_WmlObjIdToUtf8(objId);
			utf8_ParentIdLen = (int) strlen(utf8_ParentId);
			//if (objId->BaseFolder == WmlFolders_AllMedia)
			//{
			//	objId->SubFolder = NULL;
			//}

			if (wmlObj->Type == WmlObjType_FilteredPlaylist)
			{
				WML_RespondWithSetFromPlaylist(browseArgs, objId, wmlObj->Playlist.p, wmlObj->Attribute, wmlObj->AttributeValue, addresses, addressesLen, port, numberReturned, totalMatches, updateID, utf8_ParentId, utf8_ParentIdLen);
			}
			else
			{
				WML_RespondWithSetFromPlaylist(browseArgs, objId, wmlObj->Playlist.p, NULL, NULL, addresses, addressesLen, port, numberReturned, totalMatches, updateID, utf8_ParentId, utf8_ParentIdLen);
			}
			break;

		case WmlObjType_StringCollection:
			// We have a collection of strings to represent child objects.
			// This code should execute for containers like 
			//		WmlFolders_Audio_ByAlbum
			//		WmlFolders_Audio_ByArtist
			//		WmlFolders_Audio_ByGenre
			// The result DIDL-Lite will consiste of a list of <container>
			// elements for the various albums, artists, genres, or applicable grouping.

			utf8_ParentId = WML_WmlObjIdToUtf8(objId);
			utf8_ParentIdLen = (int) strlen(utf8_ParentId);
			WML_RespondWithSetFromStringCollection(browseArgs, objId, wmlObj->StringCollection.p, addresses, addressesLen, port, numberReturned, totalMatches, updateID, utf8_ParentId, utf8_ParentIdLen);
			break;

		case WmlObjType_PlaylistArray:
			break;

		case WmlObjType_WmlObjArray:
			ASSERT(objId->Source == NULL);
			ASSERT(objId->EmbeddedSource == NULL);
			childObjId = *objId;
			
			startIndex = 0;
			endIndex = wmlObj->WmlObjArrayLen-1;

			if (browseArgs->StartingIndex > 0)
			{
				startIndex = browseArgs->StartingIndex;
			}

			if (browseArgs->RequestedCount > 0)
			{
				endIndex = startIndex + browseArgs->RequestedCount - 1;
			}

			if (endIndex > wmlObj->WmlObjArrayLen-1)
			{
				endIndex = wmlObj->WmlObjArrayLen-1;
			}

			for (i=startIndex; i <= endIndex; i++)
			{
				childObjId.BaseFolder = wmlObj->WmlObjArray[i]->ObjId;
				WML_DoBrowseMetadata(browseArgs, &childObjId, wmlObj->WmlObjArray[i], addresses, addressesLen, port, &childUpdateId);
				(*numberReturned)++;
			}
			*totalMatches = wmlObj->WmlObjArrayLen;
			break;
		}
	}

	free (utf8_ParentId);
}

void WML_PerformBrowse_DidlResponse(struct MSL_CdsQuery *browseArgs, struct WML_ObjId* objId, struct WML_Object *wmlObj, int *addresses, int addressesLen, int port)
{
	unsigned int numberReturned=0, totalMatches=0, updateID=0;

	MSL_ForQueryResponse_Start(browseArgs, 1);

	if (browseArgs->QueryType == MS_Query_BrowseDirectChildren)
	{
		// print children of wmlObj
		WML_DoBrowseDirectChildren(browseArgs, objId, wmlObj, addresses, addressesLen, port, &numberReturned, &totalMatches, &updateID);
	}
	else
	{
		// print metadata for the wmlObj
		WML_DoBrowseMetadata(browseArgs, objId, wmlObj, addresses, addressesLen, port, &updateID);
		numberReturned = totalMatches = 1;
	}

	MSL_ForQueryResponse_FinishResponse(browseArgs, 1, numberReturned, totalMatches, updateID);
}

void WML_RespondWithError(struct MSL_CdsQuery *browseArgs, enum Enum_CdsErrors error)
{
	switch (error)
	{
	case CdsError_ActionFailed:
	case CdsError_NoSuchObject:
	case CdsError_NoSuchContainer:
		MSL_ForResponse_RespondError(browseArgs, CDS_ErrorCodes[error], CDS_ErrorStrings[error]);
		break;
	
	default:
		fprintf(stderr, "ERROR: WML_RespondWithError() error=%d\r\n", error);
		break;
	};
}

char* WML_WmlObjIdToUtf8(struct WML_ObjId* objId)
{
	int objIdSize = 50;
	char *retVal = NULL;
	int pi = 0;

	//(BaseFolder in int form):(SubFolder in string form):(UriFolder in int form):(UriFile in string form)

	if (objId->SubFolder != NULL) { objIdSize += (int)wcslen(objId->SubFolder); }
	if (objId->_SourcePostfix != NULL) { objIdSize += (int)wcslen(objId->_SourcePostfix); }
	if (objId->_EmbeddedSourcePostfix != NULL) { objIdSize += (int)wcslen(objId->_EmbeddedSourcePostfix); }
	
	retVal = (char*) malloc(objIdSize);

	if (objId->SubFolder != NULL)
	{
		pi += sprintf(retVal, "%d:", (int)objId->BaseFolder);

		if (objId->Source != NULL)
		{
			pi += strToUtf8(retVal+pi, (char*)objId->SubFolder, objIdSize-pi, 1, NULL);
			if (objId->_SourcePostfix != NULL)
			{
				pi += sprintf(retVal+pi, "%d:", (int)objId->SourcePrefixIndex);
				pi += strToUtf8(retVal+pi, (char*)objId->_SourcePostfix, objIdSize-pi, 1, NULL);
			}
			else
			{
				pi += strToUtf8(retVal+pi, (char*)objId->_SourcePostfix, objIdSize-pi, 1, NULL);
			}

			if (objId->EmbeddedSource != NULL)
			{
				ASSERT(
					(objId->EmbeddedSourcePrefix != NULL) &&
					(objId->EmbeddedTrackIndex >= 0) &&
					(objId->EmbeddedSourcePrefixIndex >= 0)
					);
				//?!?[SubUriFolder]?@?[SubUriFile]?#?[TrackIndex]
				pi += sprintf(retVal+pi, "?!?%d?@?", objId->EmbeddedSourcePrefixIndex);
				pi += strToUtf8(retVal+pi, (char*)objId->_EmbeddedSourcePostfix, objIdSize-pi, 1, NULL);
				pi += sprintf(retVal+pi, "?#?%d", objId->EmbeddedTrackIndex);
			}
		}
		else if (wcscmp(objId->SubFolder, L"")==0)
		{
		}
		else
		{
			pi += strToUtf8(retVal+pi, (char*)objId->SubFolder, objIdSize-pi, 1, NULL);
		}
	}
	else
	{
		if (objId->_SourcePostfix != NULL)
		{
			pi += sprintf(retVal, "%d::", (int)objId->BaseFolder);
			pi += sprintf(retVal+pi, "%d:", (int)objId->SourcePrefixIndex);
			pi += strToUtf8(retVal+pi, (char*)objId->_SourcePostfix, objIdSize-pi, 1, NULL);
		}
		else
		{
			pi += sprintf(retVal, "%d", (int)objId->BaseFolder);
		}
	}

	return retVal;
}

int WML_WmlObjToUtf8TitleLen(/*IN*/struct WML_ObjId *objId)
{
	if (objId->SubFolder != NULL)
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				return strUtf8Len((char*)objId->_EmbeddedSourcePostfix, 1, 0);
			}
			else
			{
				return strUtf8Len((char*)objId->_SourcePostfix, 1, 0);
			}
		}
		else
		{
			return strUtf8Len((char*)objId->SubFolder, 1, 0);
		}
	}
	else
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				return strUtf8Len((char*)objId->_EmbeddedSourcePostfix, 1, 0);
			}
			else
			{
				return strUtf8Len((char*)objId->_SourcePostfix, 1, 0);
			}
		}
		else
		{
			return (int)strlen(WML_ContainerTitle[objId->BaseFolder]);
		}
	}
}

void WML_WmlObjToUtf8Title(/*INOUT*/char *dest, /*IN*/int destSize, /*IN*/struct WML_ObjId *objId)
{
	if (objId->SubFolder != NULL)
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				strToUtf8(dest, (char*)objId->_EmbeddedSourcePostfix, destSize, 1, NULL);
			}
			else
			{
				strToUtf8(dest, (char*)objId->_SourcePostfix, destSize, 1, NULL);
			}
		}
		else
		{
			strToUtf8(dest, (char*)objId->SubFolder, destSize, 1, NULL);
		}
	}
	else
	{
		if (objId->Source != NULL)
		{
			if (objId->EmbeddedSource != NULL)
			{
				strToUtf8(dest, (char*)objId->_EmbeddedSourcePostfix, destSize, 1, NULL);
			}
			else
			{
				strToUtf8(dest, (char*)objId->_SourcePostfix, destSize, 1, NULL);
			}
		}
		else
		{
			sprintf(dest, "%s", WML_ContainerTitle[(int)objId->BaseFolder]);
		}
	}
}

unsigned long WINAPI Upnp_Main(void* dlgctrl)
{
	CWin32_WMLDlg *dlg = (CWin32_WMLDlg*) dlgctrl;

	The_BrowseSearchThreadStarted = 0;
	The_ProcessRequestsContinue = 1;
	sem_init(&The_IPAddressListLock, 0, 1);
	sem_init(&The_ProcessRequestLock, 0, 1);
	sem_init(&The_CdsRequestsLock, 0, 1);
	sem_init(&The_DirNamesLock, 0, 1);
	The_BrowseSearchStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CreateThread(NULL, 0, &Upnp_BrowseSearchLoop, dlgctrl, 0, NULL);
	WaitForSingleObject(The_BrowseSearchStartEvent, INFINITE);
	ResetEvent(The_BrowseSearchStartEvent);

	if (The_BrowseSearchThreadStarted != 0)
	{
		// prep the CDS request processing data structures
		The_CdsRequestHead = The_CdsRequestTail = NULL;

		// set up the UPnP stack
		The_Chain = ILibCreateChain();
		The_Stack = UpnpCreateMicroStack(The_Chain, "Intel's Micro Media Server (Win32/WML)","Win32-WML-46de-a4b4-92afa44808db","0000001",1800,0);
		The_Monitor = ILibCreateLifeTime(The_Chain);

		MSL_Callback_OnQuery = Upnp_OnBrowseSearch;
		The_MediaServerLogic = MSL_CreateMediaServer(The_Chain, The_Stack, The_Monitor);

		The_IPAddressLength = ILibGetLocalIPAddressList(&The_IPAddressList);
		ILibLifeTime_Add(The_Monitor,NULL,4,&Upnp_IPAddressMonitor, NULL);

		dlg->StartupComplete();

		ILibStartChain(The_Chain);

		// flush the request queue
		WML_CdsRequestsClear();
	}
	else
	{
		dlg->StartupFailed();
	}

	// upon return from ILibStartChain, the media server is gone
	CloseHandle(The_BrowseSearchStartEvent); The_BrowseSearchStartEvent = NULL;
	sem_destroy(&The_DirNamesLock);
	sem_destroy(&The_ProcessRequestLock);
	sem_destroy(&The_CdsRequestsLock);
	sem_destroy(&The_IPAddressListLock);
	free(The_IPAddressList);
	The_Chain = The_Stack = The_Monitor = The_MediaServerLogic = The_IPAddressList = NULL;
	MSL_Callback_OnQuery = NULL;
	dlg->ShutdownComplete();

	return 0;
}

void Upnp_OnBrowseSearch (struct MSL_CdsQuery *browseArgs)
{
	/*
	 *	All of the string data in browseArgs is in UTF8.
	 *	Convert to wide.
	 */
	struct CdsRequest *req, *front;
	int size;
	struct CdsStringBrowseArgsWide *baw = (struct CdsStringBrowseArgsWide *) malloc(sizeof(struct CdsStringBrowseArgsWide));
	char unescaped[1024];

	size							= ((int)strlen(browseArgs->Filter) + 1) * sizeof(wchar_t);
	baw->Filter						= (wchar_t*) malloc ( size );
	Utf8ToWide(baw->Filter, browseArgs->Filter, size);

	ILibInPlaceXmlUnEscape(browseArgs->ObjectID);
	size							= ((int)strlen(browseArgs->ObjectID) + 1) * sizeof(wchar_t);
	baw->ObjectID					= (wchar_t*) malloc ( size );
	Utf8ToWide(baw->ObjectID, browseArgs->ObjectID, size);

	size							= ((int)strlen(browseArgs->SortCriteria) + 1) * sizeof(wchar_t);
	baw->SortCriteria				= (wchar_t*) malloc ( size );
	Utf8ToWide(baw->SortCriteria, browseArgs->SortCriteria, size);

	browseArgs->UserObject = baw;

	// Add the request so it can be processed on a different thread.
	// Unblock the other thread if appropriate.

	req = (struct CdsRequest*) malloc(sizeof(struct CdsRequest));
	req->Args = browseArgs;
	req->Type = CdsRequestType_Browse;
	req->RequestedOnAddress = UpnpGetLocalInterfaceToHost(browseArgs->UpnpToken);
	req->RequestedOnPort = UpnpGetLocalPortNumber(browseArgs->UpnpToken);
		
	front = WML_CdsRequestsEnqueue(req);
	if (front == NULL)
	{
		sem_post(&The_ProcessRequestLock);
	}
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWin32_WMLDlg dialog



CWin32_WMLDlg::CWin32_WMLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWin32_WMLDlg::IDD, pParent)
	, m_ServerState(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWin32_WMLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START_STOP, m_BtnStartStop);
	//DDX_Control(pDX, IDC_MEDIAPLAYER, m_wmp);
}

BEGIN_MESSAGE_MAP(CWin32_WMLDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START_STOP, OnBnClickedStartStop)
END_MESSAGE_MAP()


// CWin32_WMLDlg message handlers

BOOL CWin32_WMLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWin32_WMLDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CWin32_WMLDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();

	if (this->m_ServerState == 1)
	{
		this->OnBnClickedStartStop();
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWin32_WMLDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWin32_WMLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWin32_WMLDlg::OnBnClickedStartStop()
{
	if (this->m_ServerState == 0)
	{
		// Only execute code if the media server is actually stopped
		this->m_ServerState = 1;
		this->m_BtnStartStop.SetWindowText("Starting MediaServer");
		CreateThread(NULL, 0, &Upnp_Main, this, 0, NULL ); 
	}
	else if (this->m_ServerState == 2)
	{
		// Instruct the processing thread to stop
		The_ProcessRequestsContinue = 0;
		sem_post(&The_ProcessRequestLock);

		this->m_ServerState = -1;
		this->m_BtnStartStop.SetWindowText("MediaServer shutting down");
		ILibStopChain(The_Chain);
	}
}

void CWin32_WMLDlg::ShutdownComplete(void)
{
	if (this->m_ServerState == -1)
	{
		this->m_ServerState = 0;
		this->m_BtnStartStop.SetWindowText("Click to start MediaServer");
	}
}

void CWin32_WMLDlg::StartupFailed(void)
{
	if (this->m_ServerState == 1)
	{
		this->m_ServerState = -1;
		this->m_BtnStartStop.SetWindowText("MediaServer failed to start up");
	}
}

void CWin32_WMLDlg::StartupComplete(void)
{
	if (this->m_ServerState == 1)
	{
		this->m_ServerState = 2;
		this->m_BtnStartStop.SetWindowText("Click to stop MediaServer");
	}
}
