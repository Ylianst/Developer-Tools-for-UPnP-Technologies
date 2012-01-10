#pragma once

#include <stdlib.h>
#include "ContentDirectoryServer.h"
#include "TransferState.h"
#include "MediaDatabase.h"
#include "MediaItemData.h"
extern "C"
{
	#include "CdsMediaObject.h"
	#include "UpnpMicroStack.h"
	#include "ILibWebServer.h"
}

#define HTTP_MAX_URI_SIZE 1024
#define HTTP_URI_BASE_LEN 128

#define HTTP_SMALL_BUFFER_SIZE 1024
#define HTTP_LARGE_BUFFER_SIZE 4096

class DiskIO;

// Singleton
class ResourceProviderDefault
{
private:
	static int rpd_PortNumber;
	static char rpd_VirDirName[64];
	static int rpd_VirDirNameLen;
	static MediaDatabase *rpd_Database;
	DiskIO *m_Disk;

public:
	ResourceProviderDefault(MediaDatabase *database, void* webServerToken);
	~ResourceProviderDefault();

	// Returns a single HTTP resource if dbEntry is an item
	static struct CdsMediaResource* ProvideResource_HttpForItem (void* resourceProviderObject, const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen);

	// Returns one or more HTTP playlist resources if dbEntry is a playlist or storage container
	static struct CdsMediaResource* ProvideResource_HttpForContainer (void* resourceProviderObject, const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen);

	static VOID CALLBACK Sink_ReadFileEx(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

	static void Sink_ReadFileEx2(HANDLE fileHandle, char* buffer, int dwNumberOfBytesTransfered, void *user);
	static void Sink_HandleSendOK(struct ILibWebServer_Session *session);
	static void Sink_HandleDisconnect(struct ILibWebServer_Session *session);
	static void Sink_HandleWebRequest(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user);
};
