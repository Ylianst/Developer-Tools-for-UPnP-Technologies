#pragma once

#include "MediaDataBase.h"
#include "MediaItemData.h"
#include "ContentDirectoryServer.h"

class MediaServer
{
private:
	ContentDirectoryServer* m_cds;
	char* SpareBuffers[3];
	void *m_wst;

	void *ChainQuitEvent;

	HANDLE ThreadHandle;
	int	   ThreadFolderKeysAdd[30];
	int	   ThreadFolderKeysRemove[30];
	int	   ThreadFolderKeysUpdate[30];
	int    ThreadExitFlag;
	HANDLE ThreadRequestLock;

public:
	MediaServer(void);
	~MediaServer(void);

	HRESULT Init(char* ServiceExe);
	void GetVersionInfo(BSTR* info);
	int LogEvent(const OLECHAR* msg);

	void UpdateSpecialContainerCount();
	void GetSharedFolders(BSTR* Folders);
	void AddSharedFolder(BSTR Folder);
	void AddSharedFolderEx(int ParentKey,char* Folder);
	void RemoveSharedFolder(BSTR Folder);
	void RemoveAllItemsWithParent(int ParentKey);
	void AddAllItemsInPath(int pKey,char* Title,char* Path,char** TempBuffers);
	void ParseID3v1(char* file,char** SongTitle,char** AlbumName,char** ArtistName);

	static DWORD WINAPI MediaServerUpdateThread(LPVOID args);
};
