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
#include "ResourceProvider_HttpItem.h"
#include "ResourceProvider_Http.h"
#include "ResourceProvider.h"

#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define MULTIPART_RANGE_DELIMITER "{{{{{-S3P4R470R-}}}}}"

extern "C"
{
	#include "MimeTypes.h"
	#include "UpnpMicroStack.h"
	#include "ILibParsers.h"
	#include "MyString.h"
	#include "ILibAsyncSocket.h"
	#include "ILibAsyncServerSocket.h"
	#include "ILibWebServer.h"
}



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
		this->FileLength = cl;

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
	int FileLength;
private:
	void *m_queue;
	int RangeType;
	const char* m_ContentType;
	HANDLE m_handle;
};





class HttpItem_TransferProgress : TransferProgress
{
public:
	HttpItem_TransferProgress(char *SessionKey, int SessionKeyLength, void *TransferTable)
	{
		TransferProgress();
		key = (char*)malloc(SessionKeyLength);
		memcpy(key,SessionKey,SessionKeyLength);
		m_TransferTable = TransferTable;
	}

	virtual long GetSentBytes()
	{
		long RetVal = -1;
		ILibWebServer_Session *session;

		ILibHashTree_Lock(m_TransferTable);
		session = (ILibWebServer_Session*)ILibGetEntry(m_TransferTable,key,keyLength);
		if(session!=NULL)
		{
			RetVal = ILibWebServer_Session_GetTotalBytesSent(session);
		}
		ILibHashTree_UnLock(m_TransferTable);
		return(RetVal);
	}
	virtual long GetTotalBytes()
	{
		long RetVal = -1;
		ILibWebServer_Session *session;

		ILibHashTree_Lock(m_TransferTable);
		session = (ILibWebServer_Session*)ILibGetEntry(m_TransferTable,key,keyLength);
		if(session!=NULL)
		{
			RetVal = (long)((RangeRequester*)session->User2)->FileLength;
		}
		ILibHashTree_UnLock(m_TransferTable);
		return(RetVal);
	}
	virtual ~HttpItem_TransferProgress()
	{
		free(key);
	}
protected:
	char *key;
	int keyLength;
	void *m_TransferTable;
};






ResourceProvider_HttpItem::ResourceProvider_HttpItem(MediaDatabase *database, void* webServerToken)
{
	this->m_Database = database;
	this->m_WebServer = webServerToken;
	this->m_Disk = new DiskIO();
	this->m_TransferTable = ILibInitHashTree();
	this->m_PortNumber = ILibWebServer_GetPortNumber(webServerToken);
	
	// Create a virtual directory name
	sprintf(this->m_VirDirName, "HttpItems_%lx", this);
	this->m_VirDirNameLen = (int) strlen(this->m_VirDirName);
	ILibWebServer_RegisterVirtualDirectory(webServerToken, this->m_VirDirName, this->m_VirDirNameLen, Sink_HandleWebRequest, this);
	ILibWebServer_SetTag (webServerToken, this);
}
ResourceProvider_HttpItem::~ResourceProvider_HttpItem()
{
	delete this->m_Disk;
	ILibDestroyHashTree(this->m_TransferTable);
}

struct CdsMediaResource* ResourceProvider_HttpItem::GetResources(const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen)
{
	struct CdsMediaResource *retVal = NULL, *newRes, *last;
	unsigned long time;
	long fileSize=0;
	char ext[MAX_FILE_EXT_LEN], ext2[MAX_FILE_EXT_LEN], protection[MAX_PROT_STRING_LEN];
	int resX=-1, resY=-1, duration=-1, bitrate=-1, colorDepth=-1, bitsPerSample=-1, sampleFrequency=-1, nrAudioChannels=-1;
	int i, addr, len, uriLen;
	char title[255], creator[255]; //must have same max size as dbEntry->Title and dbEntry->creator
	char *ignored = NULL;

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
			GetContentUri(title, creator, dbEntry->Creator_length, dbEntry->Key, ext2, this->m_VirDirName, addr, this->m_PortNumber, newRes->Value, ignored);

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

int ResourceProvider_HttpItem::GetNumTransfers()
{
	int RetVal = 0;
	void *en;

	ILibHashTree_Lock(this->m_TransferTable);
	en = ILibHashTree_GetEnumerator(this->m_TransferTable);
	while(ILibHashTree_MoveNext(en)==0)
	{
		++RetVal;
	}
	ILibHashTree_DestroyEnumerator(en);
	ILibHashTree_UnLock(this->m_TransferTable);
	return(RetVal);
}

void ResourceProvider_HttpItem::DestroyTransferList(TransferProgress **tp)
{
}
TransferProgress** ResourceProvider_HttpItem::GetTransferList(int startIndex, int maxCount, int *numReturned)
{
	TransferProgress **RetVal;
	void *en;
	char *k;
	int kl;
	int idx=0;
	int i=-1;
	if(numReturned==NULL)
	{
		return(NULL);
	}
	
	*numReturned = 0;
	RetVal = (TransferProgress**)malloc(maxCount*sizeof(HttpItem_TransferProgress*));
	ILibHashTree_Lock(this->m_TransferTable);
	en = ILibHashTree_GetEnumerator(this->m_TransferTable);
	while(ILibHashTree_MoveNext(en)==0 && *numReturned<maxCount)
	{
		if(idx>=startIndex)
		{
			++i;
			ILibHashTree_GetValue(m_TransferTable,&k,&kl,NULL);
			RetVal[i] = (TransferProgress*)new HttpItem_TransferProgress(k,kl,m_TransferTable);
		}
		++idx;
	}

	ILibHashTree_DestroyEnumerator(en);
	ILibHashTree_UnLock(this->m_TransferTable);
	
	*numReturned = i+1;
	if(*numReturned==0)
	{
		free(RetVal);
		RetVal = NULL;
	}
	return(RetVal);
}

void ResourceProvider_HttpItem::Sink_HandleWebRequest(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user)
{
	((ResourceProvider_HttpItem*)user)->Sink_HandleWebRequest(session, header, bodyBuffer, beginPointer, endPointer, done);
}

void ResourceProvider_HttpItem::Sink_HandleWebRequest (struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done)
{
	int slashPos, dotPos, fileSize;
	long contentKey;
	MediaItemData *dbEntry;
	const char *ct;
	bool fileFound = false;
	HANDLE fileHandle;
	LARGE_INTEGER fileLength;
	char buf[1024];
	int bufLen;
	RangeRequester *_RR;
	char key[64];
	int keyLength;

	slashPos = IndexOf(header->DirectiveObj+1, "/");
	if (ILibGetLong(header->DirectiveObj+1, slashPos, &contentKey) == 0)
	{
		dbEntry = this->m_Database->QueryItem(contentKey);

		if (dbEntry != NULL)
		{
			switch (dbEntry->Class)
			{	
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
		}
		delete (dbEntry);
	}

	if (fileFound)
	{
		//
		// TODO: Enable this to perform streaming functions in ResourceProvider_HttpItem::Sink_HandleWebRequest 
		//

		// This session will enable simultaneous file transfers on a single thread by
		// allowing Sink_HandleSendOK to send data in chunks to the requester.
		session->OnSendOK = Sink_HandleSendOK;
		session->OnDisconnect = ResourceProvider_HttpItem::Sink_HandleDisconnect;
		
		// Build an object that will keep state for this HTTP transfer
		if(header->DirectiveLength==3 && strncasecmp(header->Directive,"GET",3)==0)
		{
			keyLength = sprintf(key,"%ul",(DWORD)session);
			ILibHashTree_Lock(this->m_TransferTable);
			ILibAddEntry(this->m_TransferTable,key,keyLength,session);
			ILibHashTree_UnLock(this->m_TransferTable);

			ILibWebServer_Session_ResetTotalBytesSent(session);
			_RR = new RangeRequester(header,fileSize,ct,fileHandle);
			session->User2 = _RR;
			((RangeRequester*)session->User2)->user = this;
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
				ILibWebServer_StreamHeader_Raw(session,206,"Partial Content",buf,ILibAsyncSocket_MemoryOwnership_USER);
			}
			else
			{
				ILibWebServer_StreamHeader_Raw(session,200,"OK",buf,ILibAsyncSocket_MemoryOwnership_USER);
			}
			
			// Start transfering the file
			ResourceProvider_HttpItem::Sink_HandleSendOK(session);
		}
		else if(header->DirectiveLength==4 && strncasecmp(header->Directive,"HEAD",4)==0)
		{
			_RR = new RangeRequester(header,fileSize,ct,fileHandle);
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
				ILibWebServer_Send_Raw(session,"HTTP/1.1 206 Partial Content",28,ILibAsyncSocket_MemoryOwnership_STATIC,0);
			}
			else
			{
				ILibWebServer_Send_Raw(session,"HTTP/1.1 200 OK",15,ILibAsyncSocket_MemoryOwnership_STATIC,0);
			}
			ILibWebServer_Send_Raw(session,buf,bufLen,ILibAsyncSocket_MemoryOwnership_USER,0);
			ILibWebServer_Send_Raw(session,"\r\n\r\n",4,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			delete _RR;
			CloseHandle(fileHandle);
			return;
		}
		else
		{												 
			ILibWebServer_Send_Raw(session,"HTTP/1.1 400 Not Supported\r\nContent-Length: 0\r\n\r\n",49,ILibAsyncSocket_MemoryOwnership_STATIC,1);
		}
	}
	else
	{
		// Send a FILE NOT FOUND message.
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found or File is Locked\r\nContent-Length: 0\r\n\r\n",68,ILibAsyncSocket_MemoryOwnership_STATIC,1);
	}
}

void ResourceProvider_HttpItem::Sink_HandleSendOK(struct ILibWebServer_Session *session)
{
	char *headers;
	int headersLen;
	int z = -1;
	RangeRequester *RR = (RangeRequester*)session->User2;
	DiskIO* DIO = ((ResourceProvider_HttpItem*)RR->user)->m_Disk;

	if(RR!=NULL && RR->GetCurrentRange()!=NULL)
	{
		DIO->QueueReadOperation
			(
				RR->GetFileHandle(),
				((DWORD)(RR->GetCurrentRange()->StartIndex)) + ((DWORD)(RR->GetCurrentRange()->BytesSent)),
				RR->Buffer,
				RR->GetCurrentRange()->BytesLeft>RR->BufferSize?RR->BufferSize:RR->GetCurrentRange()->BytesLeft,
				ResourceProvider_HttpItem::Sink_ReadFileEx,
				session
				);

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
void ResourceProvider_HttpItem::Sink_ReadFileEx(HANDLE fileHandle, char* buffer, int dwNumberOfBytesTransfered, void *user)
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

void ResourceProvider_HttpItem::Sink_HandleDisconnect(struct ILibWebServer_Session *session)
{
	RangeRequester *RR = (RangeRequester*)session->User2;
	ResourceProvider_HttpItem *RP = (ResourceProvider_HttpItem*)RR->user;
	char key[64];
	int keyLength;

	keyLength = sprintf(key,"%ul",(DWORD)session);
	ILibHashTree_Lock(RP->m_TransferTable);
	ILibDeleteEntry(RP->m_TransferTable,key,keyLength);
	ILibHashTree_UnLock(RP->m_TransferTable);

	if(RR!=NULL)
	{
		CloseHandle(RR->GetFileHandle());
		delete RR;
		session->User2=NULL;
	}
}
