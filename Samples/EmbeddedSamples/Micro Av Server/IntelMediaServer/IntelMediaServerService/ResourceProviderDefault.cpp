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
#include <windows.h>
#include <winbase.h>

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "resourceproviderdefault.h"

#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define MULTIPART_RANGE_DELIMITER "{{{{{-S3P4R470R-}}}}}"

extern "C"
{
	#include "MimeTypes.h"
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"
	#include "MyString.h"
	#include "ILibAsyncSocket.h"
}

enum TSI_Types
{
	TSI_Type_Memory,
	TSI_Type_FileHandle
};

typedef void (*DiskIO_Completion)(HANDLE fileHandle, char *buffer, int bytesRead, void *user);

class DiskIO
{
public:
	DiskIO()
	{
		DWORD tid;
		m_terminateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
		m_workerThread = CreateThread(NULL,NULL,DiskIO_RUN,this,NULL,&tid);
		m_UserItemTree = ILibInitHashTree();
	}
	void QueueReadOperation(HANDLE fileHandle, long offset, char* buffer, int bufferSize, DiskIO_Completion callback, void *user)
	{
		char key[128];
		int keySize;
		struct DataStruct *t = (struct DataStruct*)malloc(sizeof(struct DataStruct));
		t->buffer = buffer;
		t->bufferSize = bufferSize;
		t->fileHandle = fileHandle;
		t->offset = offset;
		t->user = user;
		t->CallBack = callback;
		t->parent = this;

		//ToDo: Fix, so 't' doesn't leak on Quit
		keySize = sprintf(key,"%ul",(DWORD)t);
		ILibHashTree_Lock(m_UserItemTree);
		ILibAddEntry(m_UserItemTree,key,keySize,t);
		ILibHashTree_UnLock(m_UserItemTree);
		QueueUserAPC(DiskIO_APC,m_workerThread,(ULONG_PTR)t);
	}
	virtual ~DiskIO()
	{
		void *en;
		void *data;

		SetEvent(m_terminateEvent);
		WaitForSingleObject(m_workerThread,INFINITE);
		CloseHandle(m_terminateEvent);
		CloseHandle(m_workerThread);
		
		ILibHashTree_Lock(m_UserItemTree);
		en = ILibHashTree_GetEnumerator(m_UserItemTree);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,NULL,NULL,&data);
			free(data);
		}
		ILibHashTree_UnLock(m_UserItemTree);
		ILibHashTree_DestroyEnumerator(en);
		ILibDestroyHashTree(m_UserItemTree);
	}
private:
	void *m_UserItemTree;
	
	HANDLE m_workerThread;
	HANDLE m_terminateEvent;
	static VOID CALLBACK DiskIO_APC(ULONG_PTR pdw)
	{
		struct DataStruct *ds = (struct DataStruct*)pdw;
		LPOVERLAPPED overlapped = (LPOVERLAPPED)malloc(sizeof(OVERLAPPED));
		memset(overlapped,0,sizeof(OVERLAPPED));

		overlapped->Offset = ds->offset;
		overlapped->hEvent = ds;

		ReadFileEx(ds->fileHandle,ds->buffer,ds->bufferSize,overlapped, DiskIO_ReadSink);
	}
	static DWORD WINAPI DiskIO_RUN(LPVOID lpP)
	{
		DiskIO *pDiskIO = (DiskIO*)lpP;
		while(WaitForSingleObjectEx(pDiskIO->m_terminateEvent,INFINITE,TRUE)!=WAIT_OBJECT_0);
		return(0);
	}
	static VOID CALLBACK DiskIO_ReadSink(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		char key[128];
		int keySize;

		struct DataStruct *ds = (struct DataStruct*)lpOverlapped->hEvent;
		keySize = sprintf(key,"%ul",(DWORD)ds);
		ds->CallBack(ds->fileHandle,ds->buffer,dwNumberOfBytesTransfered,ds->user);

		ILibHashTree_Lock(ds->parent->m_UserItemTree);
		ILibDeleteEntry(ds->parent->m_UserItemTree,key,keySize);
		ILibHashTree_UnLock(ds->parent->m_UserItemTree);
		free(ds);
		free(lpOverlapped);
	}
	struct DataStruct
	{
		HANDLE fileHandle;
		long offset;
		char *buffer;
		int bufferSize;
		void *user;
		DiskIO_Completion CallBack;
		DiskIO *parent;
	};
};





struct RangeRequest
{
	int StartIndex;
	int BytesSent;
	int BytesLeft;
};
class RangeRequester
{
public:
	RangeRequester(struct packetheader *hdr, int cl, const char *ct, HANDLE fileHandle)
	{
		struct packetheader_field_node *phf;
		struct parser_result *pr,*pr2,*pr3;
		struct parser_result_field *prf;
		struct RangeRequest *rr;
		int FoundRange=0;
		
		Buffer = (char*)malloc(4096);
		BufferSize = 4096;

		m_queue = ILibQueue_Create();
		m_ContentType = ct;
		m_handle = fileHandle;

		// Check If Range Request
		phf = hdr->FirstField;
		while(phf!=NULL)
		{
			if(phf->FieldLength==5 && strncasecmp(phf->Field,"RANGE",5)==0)
			{
				FoundRange=1;
				pr = ILibParseString(phf->FieldData,0,phf->FieldDataLength,"=",1);
				pr2 = ILibParseString(pr->LastResult->data,0,pr->LastResult->datalength,",",1);
				prf = pr2->FirstResult;
				RangeType = (pr2->NumResults==1?1:2);

				while(prf!=NULL)
				{
					rr = (struct RangeRequest*)malloc(sizeof(struct RangeRequest));
					rr->BytesSent = 0;
					pr3 = ILibParseString(prf->data,0,prf->datalength,"-",1);
					if(pr3->FirstResult->datalength==0)
					{
						rr->StartIndex = -1;
					}
					else
					{
						pr3->FirstResult->data[pr3->FirstResult->datalength] = 0;
						rr->StartIndex = atoi(pr3->FirstResult->data);
					}
					if(pr3->LastResult->datalength==0)
					{
						rr->BytesLeft = cl-rr->StartIndex;
					}
					else
					{
						pr3->LastResult->data[pr3->LastResult->datalength] = 0;
						if(rr->StartIndex==-1)
						{
							rr->BytesLeft = atoi(pr3->LastResult->data);
							if(rr->BytesLeft>=cl) 
							{
								rr->BytesLeft = cl;
								rr->StartIndex = 0;
							}
							else
							{
								rr->StartIndex = cl-rr->BytesLeft;
							}
						}
						else
						{
							rr->BytesLeft = atoi(pr3->LastResult->data) - rr->StartIndex;
							if(rr->BytesLeft>(cl-rr->StartIndex))
							{
								rr->BytesLeft = cl-rr->StartIndex;
							}
						}
					}
					ILibQueue_EnQueue(m_queue,rr);
					ILibDestructParserResults(pr3);
					prf = prf->NextResult;
				}
				ILibDestructParserResults(pr2);
				ILibDestructParserResults(pr);
				break;
			}
			phf=phf->NextField;
		}
		
		if(FoundRange==0)
		{
			rr = (struct RangeRequest*)malloc(sizeof(struct RangeRequest));
			RangeType = 0;
			rr->StartIndex = 0;
			rr->BytesLeft = cl;
			rr->BytesSent = 0;
			ILibQueue_EnQueue(m_queue,rr);
		}
	}
	~RangeRequester()
	{
		while(CurrentRangeComplete()==0);
		ILibQueue_Destroy(m_queue);
		free(Buffer);
	}
	struct RangeRequest *GetCurrentRange()
	{
		return((struct RangeRequest*)ILibQueue_PeekQueue(m_queue));
	}
	int CurrentRangeComplete()
	{
		void *tmp = ILibQueue_DeQueue(m_queue);
		if(tmp!=NULL) {free(tmp);}
		return(ILibQueue_PeekQueue(m_queue)==NULL?1:0);
	}
	int GetRangeType()
	{
		return(RangeType);
	}
	HANDLE GetFileHandle()
	{
		return(m_handle);
	}
	const char* GetContentType()
	{
		return(m_ContentType);
	}
	char *Buffer;
	int BufferSize;
	void *user;
private:
	void *m_queue;
	int RangeType;
	const char* m_ContentType;
	HANDLE m_handle;
};






class TransferStateInfo
{
public:
	enum TSI_Types Type;
	HANDLE FileHandle;
	const char *MimeType;							// mime type of the transfer - do not free
	char *Buffer;
	unsigned int BufferSize;

	TransferStateInfo(HANDLE fileHandle, const char *mimeType)
	{
		this->Type = TSI_Type_FileHandle;
		this->FileHandle = fileHandle;
		this->MimeType = mimeType;
		this->BufferSize = HTTP_LARGE_BUFFER_SIZE;
		this->Buffer = new char[HTTP_LARGE_BUFFER_SIZE];
	}

	TransferStateInfo(char *buffer, unsigned int buflen, const char *mimeType)
	{
		this->Type = TSI_Type_Memory;
		this->FileHandle = NULL;
		this->MimeType = mimeType;
		this->BufferSize = buflen;
		this->Buffer = buffer;
	}

	~TransferStateInfo()
	{
		delete (this->Buffer);
	}
};

int ResourceProviderDefault::rpd_PortNumber = 0;
char ResourceProviderDefault::rpd_VirDirName[64];
int ResourceProviderDefault::rpd_VirDirNameLen = 0;
MediaDatabase* ResourceProviderDefault::rpd_Database = NULL;



#define GetContentUri(title,creator,creatorLen,key,ext,virDirName,addr,port,uri,cp)	\
	if (creatorLen > 0)\
	{\
		if (ext[0] == '.')\
		{	\
			cp += sprintf(\
				uri,\
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%%20-%%20%s%s",\
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, creator, title, ext\
				);\
		}\
		else\
		{\
			cp += sprintf(\
				uri,\
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%%20-%%20%s.%s",\
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, creator, title, ext\
				);\
		}\
	}\
	else\
	{\
		if (ext[0] == '.')\
		{	\
			cp += sprintf(\
				uri, \
				"http://%d.%d.%d.%d:%d/%s/%ld/%s%s", \
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, title, ext\
				);\
		}\
		else\
		{\
			cp += sprintf(\
				uri, \
				"http://%d.%d.%d.%d:%d/%s/%ld/%s.%s", \
				(addr&0xFF), ((addr>>8)&0xFF), ((addr>>16)&0xFF), ((addr>>24)&0xFF), \
				port, virDirName, key, title, ext\
				);\
		}\
	}

#define UnescapeDbEntryFields(dbEntry,ext,title2,creator2,ext2)\
	memcpy(title2, dbEntry->Title, dbEntry->Title_length);\
	title2[dbEntry->Title_length] = '\0';\
	ILibInPlaceXmlUnEscape(title2);\
	memcpy(creator2, dbEntry->Creator, dbEntry->Creator_length);\
	creator2[dbEntry->Creator_length] = '\0';\
	ILibInPlaceXmlUnEscape(creator2);\
	strcpy(ext2, ext);\
	ILibInPlaceXmlUnEscape(ext2);\



ResourceProviderDefault::ResourceProviderDefault(MediaDatabase *database, void *webServerToken)
{
	rpd_PortNumber = ILibWebServer_GetPortNumber(webServerToken);
	this->m_Disk = new DiskIO();

	if (rpd_Database == NULL)
	{
		rpd_Database = database;
	}
	sprintf(rpd_VirDirName, "Default_%lx", this);
	rpd_VirDirNameLen = (int) strlen(rpd_VirDirName);
	ILibWebServer_RegisterVirtualDirectory(webServerToken, rpd_VirDirName, rpd_VirDirNameLen, Sink_HandleWebRequest, NULL);
	ILibWebServer_SetTag(webServerToken,this);
}

ResourceProviderDefault::~ResourceProviderDefault(void)
{
	delete this->m_Disk;
}




struct CdsMediaResource* ResourceProviderDefault::ProvideResource_HttpForItem (void* resourceProviderObject, const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen)
{
	struct CdsMediaResource *retVal = NULL, *newRes, *last;
	unsigned long time;
	long fileSize=0;
	char ext[MAX_FILE_EXT_LEN], ext2[MAX_FILE_EXT_LEN], protection[MAX_PROT_STRING_LEN];
	int resX=-1, resY=-1, duration=-1, bitrate=-1, colorDepth=-1, bitsPerSample=-1, sampleFrequency=-1, nrAudioChannels=-1;
	int i, addr, len, uriLen;
	char title[255], creator[255]; //must have same max size as dbEntry->Title and dbEntry->creator
	char *ignored = NULL;
	ResourceProviderDefault* resProvider;

	resProvider = (ResourceProviderDefault*) resourceProviderObject;

	ext[0] = '\0';
	ext2[0] = '\0';
	protection[0] = '\0';
	title[0] = '\0';
	creator[0] = '\0';

	switch(dbEntry->Class)
	{
	case MEDIA_DB_CL_IMAGE:
	case MEDIA_DB_CL_AUDIO:
	case MEDIA_DB_CL_VIDEO:
	case MEDIA_DB_CL_OTHER:
		//
		// Builds a single HTTP resource for the specified database entry
		//

		sscanf(dbEntry->Deserialization, CDS_RESFORMAT_ITEM, 
			&time, 
			&fileSize,
			ext,
			&resX,
			&resY,
			&duration,
			&bitrate,
			&colorDepth,
			&bitsPerSample,
			&sampleFrequency,
			&nrAudioChannels,
			protection
			);

		// unescape a bunch of fields needed for the resource URI
		UnescapeDbEntryFields(dbEntry, ext, title, creator, ext2);

		uriLen = HTTP_URI_BASE_LEN + SMALL_OBJID_LEN + dbEntry->Creator_length + dbEntry->Title_length + (int)strlen(ext2) + 4;

		last = NULL;
		for (i=0; i < ipAddrListLen; i++)
		{
			addr = ipAddrList[i];
			newRes = CDS_AllocateResource();

			//
			// Build the resource
			//

			// Copy the basic values across. CdsObjectToDidl() will ensure
			// that any values that are out of range are not printed.
			newRes->Bitrate = bitrate;
			newRes->BitsPerSample = bitsPerSample;
			newRes->ColorDepth = colorDepth;
			newRes->Duration = duration;
			newRes->NrAudioChannels = nrAudioChannels;
			newRes->ResolutionX = resX;
			newRes->ResolutionY = resY;
			newRes->SampleFrequency = sampleFrequency;
			newRes->Size = fileSize;

			// Deep copy the string - use malloc because CDS_DestroyObjects/CDS_DestroyResources will use free().
			// Be sure to mark the field as an allocated that should be deallocated when
			// CDS_DestroyObjects/CDS_DestroyResources is called.
			len = (int) strlen(protection) + 1;
			newRes->Protection = (char*) malloc (len);
			memcpy(newRes->Protection, protection, len);
			newRes->Allocated |= CDS_ALLOC_Protection;

			// Determine the appropriate protocolInfo string for this thing.
			// Mime types are static,s o don't bother marking them as being allocated.
			newRes->ProtocolInfo = FileExtensionToProtocolInfo(ext2, 0);

			// Build an HTTP URI for this local file using the current IP address.
			// Format is: http ://xxx.xxx.xxx.xxx.pppppppppp/[dbEntry->ID]/[rpd_VirDirName]/[Creator] - [Title].[ext]
			newRes->Value = (char*) malloc(uriLen);

			//
			// Assume that creator and title are already XML-escaped, as that is the
			// way they are supposed to be stored in the database. 
			// Also assume that file extensions are stored in their lower-case form.
			//

			// store a URI in newRes->Value
			GetContentUri(title, creator, dbEntry->Creator_length, dbEntry->Key, ext2, rpd_VirDirName, addr, rpd_PortNumber, newRes->Value, ignored);

			// ensure that last points to the tail of our resource list
			if (last == NULL)
			{
				retVal = last = newRes;
			}
			else
			{
				last = last->Next = newRes;
			}
		}
	}

	return retVal;
}

struct CdsMediaResource* ResourceProviderDefault::ProvideResource_HttpForContainer (void* resourceProviderObject, const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen)
{
	struct CdsMediaResource *retVal = NULL, *last, *newRes;
	char title[255];
	int addr, i, uriLen;
	char *ignored = NULL;
	ResourceProviderDefault* resProvider;

	resProvider = (ResourceProviderDefault*) resourceProviderObject;


	switch(dbEntry->Class)
	{
	case MEDIA_DB_CL_C_STORAGE:
	case MEDIA_DB_CL_C_PLAYLIST:
		// get an XML-unescaped representation of the title
		memcpy(title, dbEntry->Title, dbEntry->Title_length);
		title[dbEntry->Title_length] = '\0';
		ILibInPlaceXmlUnEscape(title);

		uriLen = HTTP_URI_BASE_LEN + SMALL_OBJID_LEN + dbEntry->Title_length + 8;

		last = NULL;
		for (i=0; i < ipAddrListLen; i++)
		{
			addr = ipAddrList[i];
			newRes = CDS_AllocateResource();

			//
			// Build the m3u playlist resource
			// Can also modify this code to build an ASX resource too.
			//

			// Determine the appropriate protocolInfo string for this thing.
			// Mime types are static,s o don't bother marking them as being allocated.
			newRes->ProtocolInfo = PROTINFO_PLAYLIST_M3U;

			// Build an HTTP URI for this local file using the current IP address.
			// Format is: http ://xxx.xxx.xxx.xxx.pppppppppp/[dbEntry->ID]/[rpd_VirDirName]/[Title].m3u
			newRes->Value = (char*) malloc(uriLen);

			//
			// Assume that title is already XML-escaped, as that is the
			// way it is supposed to be stored in the database.
			// Also assume that file extensions are stored in their lower-case form.
			//

			GetContentUri(title, "", 0, dbEntry->Key, EXTENSION_PLAYLIST_M3U, rpd_VirDirName, addr, rpd_PortNumber, newRes->Value, ignored);
			
			// ensure that last points to the tail of our resource list
			if (last == NULL)
			{
				retVal = last = newRes;
			}
			else
			{
				last = last->Next = newRes;
			}
		}

		break;
	}

	return retVal;
}

// OnSession
//	define through OnSession
//		OnReceive (web request)
//		OnDisconnect
//		OnSendOK

void ResourceProviderDefault::Sink_HandleDisconnect(struct ILibWebServer_Session *session)
{
	// SEND_OK and HANDLE_DISCONNECT are both called on the Microstack thread,
	// so we don't have to worry about thread-safety <wink wink>
	RangeRequester *RR = (RangeRequester*)session->User2;

	if(RR!=NULL)
	{
		CloseHandle(RR->GetFileHandle());
		delete RR;
		session->User2=NULL;
	}


	//TransferState *ts;
	//TransferStateInfo *tsi;

	//if (session->User2 != NULL)
	//{
	//	ts = (TransferState*) (session->User2);
	//	tsi = (TransferStateInfo*) ts->User;

	//	switch (tsi->Type)
	//	{
	//	case TSI_Type_Memory:
	//		// do nothing
	//		break;

	//	case TSI_Type_FileHandle:
	//		// close the file
	//		CloseHandle(tsi->FileHandle);
	//		break;
	//	}

	//	// delete any pending range sets
	//	while (ts->CurrentRangeSet != NULL)
	//	{
	//		ts->SetCurrentRangeSet();
	//	}

	//	// delete transfer info
	//	delete (tsi);

	//	// delete the transfer object
	//	delete (ts);
	//	session->User2 = NULL; 
	//}
}

void ResourceProviderDefault::Sink_ReadFileEx2(HANDLE fileHandle, char* buffer, int dwNumberOfBytesTransfered, void *user)
{
	struct ILibWebServer_Session *session;
	RangeRequester *RR;
	int z = -1;
	// get the session pointer
	session = (struct ILibWebServer_Session*) user;
	
	// get user objects from session and TransferState
	RR = (RangeRequester*)session->User2;

	if (dwNumberOfBytesTransfered > 0)
	{
		// asynchronously send the data, and configure to free buf when done

			// acknowledge that bytes have been sent
			RR->GetCurrentRange()->BytesSent+=dwNumberOfBytesTransfered;
			RR->GetCurrentRange()->BytesLeft-=dwNumberOfBytesTransfered;
			if(RR->GetCurrentRange()->BytesLeft<=0)
			{
				RR->CurrentRangeComplete();
			}
		z = ILibWebServer_StreamBody(session, RR->Buffer, dwNumberOfBytesTransfered, 1, 0);
	}
	

	if (z == 0)
	{
		// All data was sent, so we need to call SendOK again.
		Sink_HandleSendOK(session);
	}
}

void ResourceProviderDefault::Sink_HandleSendOK(struct ILibWebServer_Session *session)
{
	TransferState *ts;
	TransferStateInfo *tsi;
	char *headers;
	int headersLen;
	LPOVERLAPPED overlapped;
	int z = -1;
	RangeRequester *RR = (RangeRequester*)session->User2;

	if(RR!=NULL && RR->GetCurrentRange()!=NULL)
	{
		// File Handle
		//overlapped = (LPOVERLAPPED) malloc(sizeof(OVERLAPPED));
		//if (!overlapped)
		//{
		//	fprintf(stderr, "FATAL allocation error in Sink_HandleSendOK(). Failed LocalAlloc() for overlapped memory.\r\n");
		//	ExitProcess(1);
		//}
		//memset(overlapped, 0, sizeof(OVERLAPPED));

		// MSDN says I can use this field.
		//overlapped->hEvent = (HANDLE) session;

		// Set the starting offset on the overlapped structure.
		// Everything else can remain zero.
		//overlapped->Offset = ((DWORD)(RR->GetCurrentRange()->StartIndex)) + ((DWORD)(RR->GetCurrentRange()->BytesSent));

		// Perform an asynchronous read on the file.
		// When reading is complete, Sink_ReadFileEx will execute
		// and we'll actually send the data in that method.
		((DiskIO*)((ResourceProviderDefault*)ILibWebServer_GetTag(session->Parent))->m_Disk)->QueueReadOperation
			(
				RR->GetFileHandle(),
				((DWORD)(RR->GetCurrentRange()->StartIndex)) + ((DWORD)(RR->GetCurrentRange()->BytesSent)),
				RR->Buffer,
				RR->GetCurrentRange()->BytesLeft>RR->BufferSize?RR->BufferSize:RR->GetCurrentRange()->BytesLeft,
				ResourceProviderDefault::Sink_ReadFileEx2,
				session
				);



		//switch (tsi->Type)
		//{
		//case TSI_Type_Memory:
		//	// asynchronously send the entire range
		//	do
		//	{
		//		z = ILibWebServer_StreamBody(session, tsi->Buffer + ts->CurrentRangeSet->StartIndex, ts->CurrentRangeSet->BytesLeft, 1, 0);

		//		// mark as having sent the entire range 
		//		ts->IncrementTotalSent(ts->CurrentRangeSet->BytesLeft);

		//		// we should never send more bytes than was requested
		//		assert(ts->CurrentRangeSet->BytesLeft >= 0);

		//		if (z == 0)
		//		{
		//			ts->SetCurrentRangeSet();
		//		}

		//		if (ts->CurrentRangeSet == NULL)
		//		{
		//			break;
		//		}
		//	}
		//	while (z == 0);

		//	if (z == 0)
		//	{
		//		// if everything was sent, go ahead and indicate we're done
		//		z = ILibWebServer_StreamBody(session,"",0,1,1);
		//	}
		//	break;
	}
	else
	{
		// We are done sending data.
		if (z != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR)
		{
			z = ILibWebServer_StreamBody(session,"",0,1,1);
		}
		else
		{
			Sink_HandleDisconnect(session);
		}
	}
}

void ResourceProviderDefault::Sink_HandleWebRequest(struct ILibWebServer_Session *session, struct packetheader *packet, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user)
{
	RangeRequester *_RR;
	char *buf;
	int bufLen;

	long fileSize = 0, contentKey = 0;
	LARGE_INTEGER fileLength;
	HANDLE fileHandle;
	bool fileFound = false;
	int slashPos, gotKey = 0, dotPos;
	MediaItemData *dbEntry, *ple;
	const char *ct;
	int addr;

	unsigned long time;
	char title[255], creator[255];
	char ext[MAX_FILE_EXT_LEN], ext2[MAX_FILE_EXT_LEN], protection[MAX_PROT_STRING_LEN];
	int resX=-1, resY=-1, duration=-1, bitrate=-1, colorDepth=-1, bitsPerSample=-1, sampleFrequency=-1, nrAudioChannels=-1;

	int childCount, playlistBytesSize;
	char *playlistBytes= NULL, *pbi;

	ext[0] = '\0';
	ext2[0] = '\0';
	protection[0] = '\0';
	title[0] = '\0';
	creator[0] = '\0';



	//
	//	The rule for any HTTP URL that this module builds is that the first
	//	directory after the virtual directory name maps to a contentKey in the
	//	database. Therefore, we simply need to obtain that contentKey
	//	and query the database for that content entry.
	//
	//	If the contentKey maps to an individual file, then we simply 
	//	respond with the binary data from the local file.
	//
	//	If the contentKey maps to a playlist, then we have to check for
	//	the type of playlist by checking for the file extension. Then
	//	we have to dynamically build that playlist file and send it as
	//	a response. Because playlist files must allow RANGE requests,
	//	we need to build the playlist file before we start the response.
	//

	slashPos = IndexOf(packet->DirectiveObj+1, "/");
	if (ILibGetLong(packet->DirectiveObj+1, slashPos, &contentKey) == 0)
	{
		dbEntry = rpd_Database->QueryItem(contentKey);

		if (dbEntry != NULL)
		{
			switch (dbEntry->Class)
			{	
			case MEDIA_DB_CL_C_STORAGE:
			case MEDIA_DB_CL_C_PLAYLIST:
				ple = rpd_Database->QueryContainerItems(contentKey, MEDIA_DB_SORT_KEY);
				addr = ILibWebServer_GetLocalInterface(session);
				sscanf(dbEntry->Deserialization, CDS_RESFORMAT_CONTAINER, &childCount);
				playlistBytesSize = (540+HTTP_MAX_URI_SIZE) * childCount;
				pbi = playlistBytes = new char[playlistBytesSize];
				pbi += sprintf(pbi, "#EXTM3U\r\n");
				while (1)
				{
					if (ple->EndOfFile()) break;

					if (ple != NULL)
					{
						ple->SetStringLengths();

						sscanf(ple->Deserialization, CDS_RESFORMAT_ITEM, 
							&time, 
							&fileSize,
							ext,
							&resX,
							&resY,
							&duration,
							&bitrate,
							&colorDepth,
							&bitsPerSample,
							&sampleFrequency,
							&nrAudioChannels,
							protection
							);

						UnescapeDbEntryFields(ple, ext, title, creator, ext2);
						if (duration < 0) duration = -1;
						if (ple->Creator_length >= 0)
						{
							pbi += sprintf(pbi,"#EXTINF:%d,%s - %s\r\n", duration, creator, title);
						}
						else
						{
							pbi += sprintf(pbi,"#EXTINF:%d,%s\r\n", duration, title);
						}
						GetContentUri(title, creator, ple->Creator_length, ple->Key, ext2, rpd_VirDirName, addr, rpd_PortNumber, pbi, pbi);
						pbi += sprintf(pbi, "\r\n");

						ple->MoveNext();
					}
				}
				delete (ple);
				assert ((playlistBytes + playlistBytesSize) > pbi);
				ct = MIME_TYPE_PLAYLIST_M3U;
				fileSize = (int) (pbi - playlistBytes);
				fileFound = true;
				break;

			case MEDIA_DB_CL_IMAGE:
			case MEDIA_DB_CL_AUDIO:
			case MEDIA_DB_CL_VIDEO:
			case MEDIA_DB_CL_OTHER:
				fileHandle = CreateFile(dbEntry->Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				if (fileHandle != INVALID_HANDLE_VALUE)
				{
					//WARNING: This only works for file sizes under 2GB.
					if (GetFileSizeEx(fileHandle, &fileLength))
					{
						dotPos = LastIndexOf(dbEntry->Path, ".");
						if (dotPos >= 0)
						{
							ct = FileExtensionToMimeType(&(dbEntry->Path[dotPos+1]), 0);
						}
						else
						{
							ct = FileExtensionToMimeType("", 0);
						}
						fileSize = fileLength.LowPart;
						fileFound = true;
					}
				}
				break;
			}
			delete (dbEntry);
		}
	}

	if (fileFound)
	{
		// When this session closes (either from the server or client), this callback
		// will execute to allow the server to clean up its data.
		session->OnDisconnect = ResourceProviderDefault::Sink_HandleDisconnect;

		if (playlistBytes != NULL)
		{
			// Build an object that will keep state for this HTTP transfer
			session->User2 = NULL;

			bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nAccept-Range: bytes\r\nContent-Type: %s", ct);
			ILibWebServer_StreamHeader_Raw(session,200,"OK",buf,0);
			ILibWebServer_StreamBody(session,playlistBytes,fileSize,0,1);
		}
		else
		{
			// This session will enable simultaneous file transfers on a single thread by
			// allowing Sink_HandleSendOK to send data in chunks to the requester.
			session->OnSendOK = ResourceProviderDefault::Sink_HandleSendOK;


			// Build an object that will keep state for this HTTP transfer
			if(packet->DirectiveLength==3 && strncasecmp(packet->Directive,"GET",3)==0)
			{
				buf = (char*)malloc(1024);
				session->User2 = new RangeRequester(packet,fileSize,ct,fileHandle);
				
				switch(((RangeRequester*)session->User2)->GetRangeType())
				{
					case 0:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nAccept-Range: bytes\r\nContent-Type: %s", ct);
						break;
					case 1:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Range: bytes %d-%d/%d\r\nContent-Type: %s", _RR->GetCurrentRange()->StartIndex,_RR->GetCurrentRange()->BytesLeft,fileSize,ct);
						break;
					case 2:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Type: multipart/byteranges; boundary=%s",MULTIPART_RANGE_DELIMITER);
						break;
				}
				if(((RangeRequester*)session->User2)->GetRangeType()!=0)
				{
					ILibWebServer_StreamHeader_Raw(session,206,"Partial Content",buf,0);
				}
				else
				{
					ILibWebServer_StreamHeader_Raw(session,200,"OK",buf,0);
				}
			}
			else if(packet->DirectiveLength==4 && strncasecmp(packet->Directive,"HEAD",4)==0)
			{
				buf = (char*)malloc(1024);
				_RR = new RangeRequester(packet,fileSize,ct,fileHandle);
				switch(_RR->GetRangeType())
				{
					case 0:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nAccept-Range: bytes\r\nContent-Type: %s", ct);
						break;
					case 1:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Range: bytes %d-%d/%d\r\nContent-Type: %s", _RR->GetCurrentRange()->StartIndex,_RR->GetCurrentRange()->BytesLeft,fileSize,ct);
						break;
					case 2:
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Type: multipart/byteranges; boundary=%s",MULTIPART_RANGE_DELIMITER);
						break;
				}
				if(_RR->GetRangeType()!=0)
				{
					ILibWebServer_Send_Raw(session,"HTTP/1.1 206 Partial Content",28,1,0);
				}
				else
				{
					ILibWebServer_Send_Raw(session,"HTTP/1.1 200 OK",15,1,0);
				}
				ILibWebServer_Send_Raw(session,buf,(int)strlen(buf),0,0);
				ILibWebServer_Send_Raw(session,"\r\n\r\n",4,1,1);
				delete _RR;
				CloseHandle(fileHandle);
				return;
			}
			else
			{												 
				ILibWebServer_Send_Raw(session,"HTTP/1.1 400 Not Supported\r\nContent-Length: 0\r\n\r\n",49,1,1);
			}
		}

		// Start transfering the file
		ResourceProviderDefault::Sink_HandleSendOK(session);
	}
	else
	{
		// Send a FILE NOT FOUND message.
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found\r\nContent-Length: 0\r\n\r\n",48,1,1);
	}
}


