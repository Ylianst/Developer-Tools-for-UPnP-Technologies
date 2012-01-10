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

#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "mediaserver.h"
#include "IntelMediaServerConfig.h"

#include "ResourceProvider.h"
#include "ResourceProvider_HttpItem.h"
#include "ResourceProvider_HttpContainer.h"

extern "C"
{
	#include "PortingFunctions.h"
	#include "CdsMediaClass.h"
	#include "MimeTypes.h"
}

#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

extern CIntelMediaServerConfig* g_MediaServerConfig;
MediaDatabase* m_database = NULL;

MediaServer::MediaServer()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	m_cds = NULL;
	ThreadHandle = 0;
	ThreadExitFlag = 0;

	memset(ThreadFolderKeysRemove,0,sizeof(ThreadFolderKeysRemove));
	memset(ThreadFolderKeysAdd,0,sizeof(ThreadFolderKeysAdd));
	memset(ThreadFolderKeysUpdate,0,sizeof(ThreadFolderKeysUpdate));
}

HRESULT MediaServer::Init(char* ServiceExe)
{
	// Get the path from the EXE (Destructive)
	for (int i=(int)strlen(ServiceExe);i>0;i--)
	{
		if (ServiceExe[i] == '\\')
		{
			ServiceExe[i+1] = 0;
			break;
		}
	}

	// Setup the database
	m_database = new MediaDatabase();
	bool success = m_database->Open(ServiceExe);
	if (success == false)
	{
		delete m_database;
		m_database = NULL;
		OutputDebugString("INTEL MEDIA SERVER: Failed to open database\r\n");
		return E_FAIL;
	}

	this->ChainQuitEvent = (void*)CreateSemaphore(NULL,0,FD_SETSIZE,NULL);

	// create the CDS
	this->m_cds = new ContentDirectoryServer(m_database,&(this->ChainQuitEvent));

	// get the webserver token
	this->m_wst = this->m_cds->GetWebServerToken();

	// register a method for getting resources of an item
	this->m_cds->ResourceProviderRegister(new ResourceProvider_HttpItem(m_database, this->m_wst));

	// register a method for getting playlist resources of a container
	this->m_cds->ResourceProviderRegister(new ResourceProvider_HttpContainer(m_database, this->m_wst));

	OutputDebugString("INTEL MEDIA SERVER SUCCESFUL START\r\n");

	// Allocate Spare Buffers
	SpareBuffers[0] = (char*)malloc(2048);
	SpareBuffers[1] = (char*)malloc(2048);
	SpareBuffers[2] = (char*)malloc(40);

	// Start Media Server Update Thread
	ThreadRequestLock = CreateEvent(NULL,false,false,NULL);
	ThreadHandle = CreateThread(NULL,0,&MediaServerUpdateThread,this,0,NULL);

	/*
	// Example of how to make a Media Server Database Query
	//MediaItemData* item = m_database->QueryAllItems(MEDIA_DB_SORT_NONE,MEDIA_DB_PL_YES);
	//MediaItemData* item = m_database->QuerySpecialContainerItems(MEDIA_DB_SC_GENRE, "Speed", MEDIA_DB_CL_AUDIO, MEDIA_DB_SORT_TITLE | MEDIA_DB_SORT_DESCENDING, MEDIA_DB_PL_ALL);
	MediaItemData* item = m_database->QueryContainerItems(2, MEDIA_DB_SORT_TITLE);
	if (item != NULL)
	{
		// At least one item in the query response
		while(1)
		{
			// Do something with the item here
			if (item->EndOfFile() == 0) break;
			item->MoveNext();
		}
		delete item; // Clean-up query response
	}

	// Example of how to make a Media Server Special Container Database Query
	SpecialContainerData* sitem = m_database->QuerySpecialContainer(MEDIA_DB_SC_GENRE,MEDIA_DB_CL_C_PLAYLIST);
	if (sitem != NULL)
	{
		// At least one item in the query response
		while(1)
		{
			// Do something with the container item here
			if (sitem->EndOfFile() != 0) break;
			sitem->MoveNext();
		}
		delete sitem; // Clean-up query response
	}
	*/

	return S_OK;
}

MediaServer::~MediaServer(void)
{
	ThreadExitFlag = 1;
	SetEvent(ThreadRequestLock);

	OutputDebugString("INTEL MEDIA SERVER STOP\r\n");

	delete m_cds;
	m_cds = NULL;

	WaitForSingleObject((HANDLE)this->ChainQuitEvent,INFINITE);
	CloseHandle((HANDLE)this->ChainQuitEvent);

	m_database->Close();
	delete m_database;

	CloseHandle(ThreadRequestLock);

	// Free spare buffers
	free(SpareBuffers[0]);
	free(SpareBuffers[1]);
	free(SpareBuffers[2]);
}

void MediaServer::GetVersionInfo(BSTR* info)
{
	*info = SysAllocString(L"Intel Media Server v0.1");
}

int MediaServer::LogEvent(const OLECHAR* msg)
{
	if (g_MediaServerConfig != NULL)
	{
		return g_MediaServerConfig->Fire_ServerLogEvent(SysAllocString(msg));
	}
	return 0;
}

void MediaServer::GetSharedFolders(BSTR* Folders)
{
	char* FoldersStr = NULL;
	int   FoldersStrLen = 0;
	int   t = 0;
	OLECHAR* wFoldersStr;

	// TODO: Low priority - Optimize this fonction.
	if (m_database == NULL) {*Folders = NULL;return;}
	MediaItemData* items = m_database->QueryContainerItems(13,MEDIA_DB_SORT_TITLE);
	if (items == NULL) {*Folders = NULL;return;}

	do
	{
		if (FoldersStr == NULL)
		{
			FoldersStr = (char*)malloc(items->Path_length + 2);
			memcpy(FoldersStr,items->Path,items->Path_length);
			FoldersStr[items->Path_length] = ';';
			FoldersStr[items->Path_length+1] = 0;
			FoldersStrLen = items->Path_length+1;
		}
		else
		{
			FoldersStr = (char*)realloc(FoldersStr,(FoldersStrLen + items->Path_length) + 2);
			memcpy(FoldersStr+FoldersStrLen,items->Path,items->Path_length);
			t = FoldersStrLen + items->Path_length;
			FoldersStr[t] = ';';
			t++;
			FoldersStr[t] = 0;
			FoldersStrLen += (items->Path_length + 1);
		}
		items->MoveNext();
	}
	while (items->EndOfFile() == false);

	wFoldersStr = (OLECHAR*)malloc((FoldersStrLen+2)*2);
	mbstowcs(wFoldersStr,FoldersStr,FoldersStrLen+1);
	free(FoldersStr);

	*Folders = SysAllocString(wFoldersStr);
	free(wFoldersStr);
	
	delete items;
}

DWORD WINAPI MediaServer::MediaServerUpdateThread(LPVOID args)
{
	MediaServer* mediaserver = (MediaServer*)args;

	mediaserver->UpdateSpecialContainerCount();
	while (mediaserver->ThreadExitFlag == 0)
	{
		WaitForSingleObject(mediaserver->ThreadRequestLock,INFINITE);
		if (mediaserver->ThreadExitFlag != 0) break;

		for (int i=0;i<30;i++)
		{
			if (mediaserver->ThreadFolderKeysAdd[i] != 0)
			{
				// Mass Media Addition
				int key = mediaserver->ThreadFolderKeysAdd[i];
				MediaItemData* item = m_database->QueryItem(key);
				mediaserver->AddAllItemsInPath(key,NULL,item->Path,mediaserver->SpareBuffers);
				delete item;
				mediaserver->ThreadFolderKeysAdd[i] = 0;
				if (mediaserver->ThreadExitFlag != 0) break;
			}
			if (mediaserver->ThreadFolderKeysRemove[i] != 0)
			{
				// Mass Media Removal
				int key = mediaserver->ThreadFolderKeysRemove[i];
				mediaserver->RemoveAllItemsWithParent(key);
				m_database->DeleteItem(key);
				mediaserver->ThreadFolderKeysRemove[i] = 0;
				if (mediaserver->ThreadExitFlag != 0) break;
			}
			if (mediaserver->ThreadFolderKeysUpdate[i] != 0)
			{
				// Mass Media Update
				int key = mediaserver->ThreadFolderKeysUpdate[i];

				// TODO: Update

				mediaserver->ThreadFolderKeysUpdate[i] = 0;
				if (mediaserver->ThreadExitFlag != 0) break;
			}
		}
		if (mediaserver->ThreadExitFlag != 0) break;
		mediaserver->UpdateSpecialContainerCount();
	}

	return 0;
}

void MediaServer::AddSharedFolder(BSTR Folder)
{
	char mbFolder[2048];
	char* title;
	int pos = 0;
	wcstombs(mbFolder,Folder,2048);
	int len = (int)strlen(mbFolder);

	// Check for existance of this container
	if (m_database->QueryContainerExist(13,mbFolder) == true) return;

	// Adjust the folder title
	for (int i=0;i<(len-1);i++)
	{
		if (mbFolder[i] == '\\') pos = i;
	}

	if (pos == 0)
	{
		title = mbFolder;
	}
	else
	{
		title = mbFolder + pos + 1;
	}

	int ParentKey = m_database->AddNewItem(13,title,"","","",MEDIA_DB_CL_C_STORAGE,false,"0",mbFolder,0);

	// Add to queue of thread work
	for (int i=0;i<30;i++)
	{
		if (ThreadFolderKeysAdd[i] == 0)
		{
			ThreadFolderKeysAdd[i] = ParentKey;
			SetEvent(ThreadRequestLock);
			break;
		}
	}
}

void MediaServer::RemoveSharedFolder(BSTR Folder)
{
	char mbFolder[2048];
	wcstombs(mbFolder,Folder,2048);

	if (m_database == NULL) {return;}
	MediaItemData* items = m_database->QueryContainerItems(13,MEDIA_DB_SORT_TITLE);
	if (items == NULL) {return;}

	int key = 0;
	do
	{
		if (strcmp(mbFolder,items->Path) == 0)
		{
			key = items->Key;
			break;
		}
		items->MoveNext();
	}
	while (items->EndOfFile() == false);

	if (key == 0) return;

	// Add to queue of thread work
	for (int i=0;i<30;i++)
	{
		if (ThreadFolderKeysRemove[i] == 0)
		{
			ThreadFolderKeysRemove[i] = key;
			m_database->UpdateItemParent(key,0);
			SetEvent(ThreadRequestLock);
			break;
		}
	}
}

void MediaServer::RemoveAllItemsWithParent(int ParentKey)
{
	// Search for embedded containers & delete (recursive)
	MediaItemData* item = m_database->QueryEmbeddedContainers(ParentKey);
	if (item != NULL)
	{
		// At least one item in the query response
		while(1)
		{
			RemoveAllItemsWithParent(item->Key);
			if (item->EndOfFile() != 0) break;
			item->MoveNext();
		}
		delete item; // Clean-up query response
	}

	m_database->DeleteItemsWithParent(ParentKey);
}

void MediaServer::AddAllItemsInPath(int pKey,char* Title,char* Path,char** TempBuffers)
{
	void* token;
	char* CreatorPtr;
	char* TitlePtr;
	int FileCount = 0;
	char* Path2;
	int ParentKey;
	char* SongTitleData = NULL;
	char* AlbumNameData = NULL;
	char* ArtistNameData = NULL;

	// DESER INFO
	unsigned long time=0;	// [time (since epoch); must be valid]
	int Filesize=0;		// [fileSize; negative means unknown value] 
	char *ExtPtr="";		// [file extension; must be non-NULL and present, max length of MAX_FILE_EXT_LEN] 
	int resX=-1;			// [resolution-X; negative if unknown] 
	int resY=-1;			// [resolution-y; negative if unknown] 
	int duration=-1;		// [duration in seconds; negative if unknown]
	int bitrate=-1;			// [bitrate; negative if unknown or if VBR-encoded] 
	int colorDepth=-1;		// [colorDepth; negative if unknown] 
	int bitsPerSample=-1;	// [bitsPerSample; negative if unknown] 
	int sampleFreq=-1;		// [sampleFrequency; negative if unknown] 
	int nrAudioChannels=-1;	// [number of audio channels; negative if unknown] 
	char *protection="";	// [protection string; blank if unkonown, max length of MAX_PROT_STRING_LEN] 


	if (Title != NULL)
	{
		ParentKey = m_database->AddNewItem(pKey,Title,"","","",MEDIA_DB_CL_C_STORAGE,false,"0",Path,0);
	}
	else
	{
		ParentKey = pKey;
	}

	Path2 = (char*)malloc(strlen(Path) + 1);
	strcpy(Path2,Path);

	token = PCGetDirFirstFile(Path, TempBuffers[0], MAX_PATH, &Filesize);
	if (token != NULL)
	{
		do
		{
			// Form the complete path
			if (strlen(Path) < 4)
			{
				sprintf(TempBuffers[1],"%s%s",Path2,TempBuffers[0]);
			}
			else
			{
				sprintf(TempBuffers[1],"%s\\%s",Path2,TempBuffers[0]);
			}

			int ItemClass = MEDIA_DB_CL_OTHER;
			int type = PCGetFileDirType(TempBuffers[1]);
			if (type == 1)
			{
				// Detach the file extention
				ExtPtr = NULL;
				for (int i = (int)strlen(TempBuffers[0]);i>0;i--)
				{
					if (TempBuffers[0][i] == '.') break;
				}
				if (i > 0)
				{
					ExtPtr = TempBuffers[0] + i;
					ExtPtr[0] = 0;
					ExtPtr++;

					// Find the right media class
					unsigned int classcode = FileExtensionToClassCode(ExtPtr,0);
					switch (classcode)
					{
						case CDS_MEDIACLASS_AUDIOITEM: {ItemClass = MEDIA_DB_CL_AUDIO;break;}
						case CDS_MEDIACLASS_VIDEOITEM: {ItemClass = MEDIA_DB_CL_VIDEO;break;}
						case CDS_MEDIACLASS_IMAGEITEM: {ItemClass = MEDIA_DB_CL_IMAGE;break;}
					}
				}
				else
				{
					ExtPtr = "<>";
				}

				// Look for ID3v1
				if (ItemClass == MEDIA_DB_CL_AUDIO && stricmp(ExtPtr,"MP3") == 0)
				{
					ParseID3v1(TempBuffers[1],&SongTitleData,&AlbumNameData,&ArtistNameData);
				}

				// Look for a creator in the filename
				int j = (int)strlen(TempBuffers[0]);
				for (i=0;i<j;i++)
				{
					if (TempBuffers[0][i] == ' ' && TempBuffers[0][i+1] == '-' && TempBuffers[0][i+2] == ' ') break;
				}
				if (i != j)
				{
					CreatorPtr = TempBuffers[0];
					TempBuffers[0][i] = 0;
					TitlePtr = TempBuffers[0] + i + 3;
				}
				else
				{
					CreatorPtr = "";
					TitlePtr = TempBuffers[0];
				}

				// Build De-SER field
				//sprintf(TempBuffers[2],"%s;%d",ExtPtr,Filesize);
				sprintf(TempBuffers[2], CDS_RESFORMAT_ITEM,
					time,			// [time (since epoch); must be valid]
					Filesize,		// [fileSize; negative means unknown value] 
					ExtPtr,			// [file extension; must be non-NULL and present, max length of MAX_FILE_EXT_LEN] 
					resX,			// [resolution-X; negative if unknown] 
					resY,			// [resolution-y; negative if unknown] 
					duration,		// [duration in seconds; negative if unknown]
					bitrate,		// [bitrate; negative if unknown or if VBR-encoded] 
					colorDepth,		// [colorDepth; negative if unknown] 
					bitsPerSample,	// [bitsPerSample; negative if unknown] 
					sampleFreq,		// [sampleFrequency; negative if unknown] 
					nrAudioChannels,// [number of audio channels; negative if unknown] 
					protection		// [protection string; blank if unkonown, max length of MAX_PROT_STRING_LEN] 
					);

				// Add to database
				if (SongTitleData == NULL)
				{
					// Use Filename information
					m_database->AddNewItem(ParentKey,TitlePtr,CreatorPtr,"","",ItemClass,false,TempBuffers[2],TempBuffers[1],0);
				}
				else
				{
					// Use File Metadata Information
					if (ArtistNameData == NULL) ArtistNameData = "";
					if (AlbumNameData == NULL) AlbumNameData = "";
					m_database->AddNewItem(ParentKey,SongTitleData,ArtistNameData,"",AlbumNameData,ItemClass,false,TempBuffers[2],TempBuffers[1],0);
					if (ArtistNameData != "") free(ArtistNameData);
					if (AlbumNameData != "") free(AlbumNameData);
					free(SongTitleData);
					SongTitleData = NULL;
					AlbumNameData = NULL;
					ArtistNameData = NULL;
				}
				FileCount++;
			}
			else
			if (type == 2)
			{
				// Directory Add
				if ((strcmp(TempBuffers[0],".") != 0) && (strcmp(TempBuffers[0],"..") != 0))
				{
					AddAllItemsInPath(ParentKey,TempBuffers[0],TempBuffers[1],TempBuffers);
				}
			}
		}
		while (PCGetDirNextFile(token, NULL, TempBuffers[0], MAX_PATH, &Filesize) != 0);
		free(Path2);
		PCCloseDir(token);

		m_database->UpdateItemDeser(ParentKey,FileCount,0);
	}
}

void MediaServer::UpdateSpecialContainerCount()
{
	// Get Total Counts
	int AudioCount;
	int PictureCount;
	int VideoCount;
	int OtherCount;
	m_database->GetClassCount(&AudioCount,&PictureCount,&VideoCount,&OtherCount);

	// Update All Items
	m_database->UpdateItemDeser(1,AudioCount+PictureCount+VideoCount+OtherCount,0);

	// Update All Music
	m_database->UpdateItemDeser(6,AudioCount,0);

	// Update All Pictures
	m_database->UpdateItemDeser(9,PictureCount,0);

	// Update All Videos
	m_database->UpdateItemDeser(11,VideoCount,0);

	// Update Music->Artist
	m_database->UpdateItemDeser(7,m_database->GetArtistCount(MEDIA_DB_CL_AUDIO),0);

	// Update Music->Album
	m_database->UpdateItemDeser(5,m_database->GetAlbumCount(MEDIA_DB_CL_AUDIO),0);

	// Update Music->Genre
	m_database->UpdateItemDeser(8,m_database->GetGenreCount(MEDIA_DB_CL_AUDIO),0);

	// Update Video->Artist
	m_database->UpdateItemDeser(10,m_database->GetArtistCount(MEDIA_DB_CL_VIDEO),0);

	// Update Video->Genre
	m_database->UpdateItemDeser(12,m_database->GetGenreCount(MEDIA_DB_CL_VIDEO),0);

	// Update User Files
	m_database->UpdateItemDeser(13,m_database->GetContainerCount(13),0);
}

void MediaServer::ParseID3v1(char* file,char** SongTitle,char** AlbumName,char** ArtistName)
{
	char buf[128];
	*SongTitle = NULL;
	*AlbumName = NULL;
	*ArtistName = NULL;

	FILE* f = fopen(file,"r");
	if (f == NULL) return;
	if (fseek(f,0,SEEK_END) != 0 || ftell(f) < 128)
	{
		fclose(f);
		return;
	}

	fseek(f,-128,SEEK_END);
	if (fread(buf,1,128,f) != 128)
	{
		fclose(f);
		return;
	}
	fclose(f);

	if (buf[0] != 'T' || buf[1] != 'A' || buf[2] != 'G') return;

	for (int i=29;i>0;i--)
	{
		if (buf[i+3] != ' ') break;
		buf[i+3] = 0;
	}

	for (int i=29;i>0;i--)
	{
		if (buf[i+33] != ' ') break;
		buf[i+33] = 0;
	}

	for (int i=29;i>0;i--)
	{
		if (buf[i+63] != ' ') break;
		buf[i+63] = 0;
	}

	*SongTitle = (char*)malloc(31);
	memcpy(*SongTitle,buf+3,30);
	(*SongTitle)[30] = 0;
	*ArtistName = (char*)malloc(31);
	memcpy(*ArtistName,buf+33,30);
	(*ArtistName)[30] = 0;
	*AlbumName = (char*)malloc(31);
	memcpy(*AlbumName,buf+63,30);
	(*AlbumName)[30] = 0;
}
