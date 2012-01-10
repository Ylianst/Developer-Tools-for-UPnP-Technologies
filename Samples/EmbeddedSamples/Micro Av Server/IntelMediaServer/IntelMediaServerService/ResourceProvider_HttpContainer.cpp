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
#include "ResourceProvider_HttpContainer.h"
#include "ResourceProvider_Http.h"

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

ResourceProvider_HttpContainer::~ResourceProvider_HttpContainer()
{
}
ResourceProvider_HttpContainer::ResourceProvider_HttpContainer(MediaDatabase *database, void* webServerToken)
{
	this->m_Database = database;
	this->m_WebServer = webServerToken;

	// Create a virtual directory name
	sprintf(this->m_VirDirName, "HttpContainer_%lx", this);
	this->m_VirDirNameLen = (int) strlen(this->m_VirDirName);
	ILibWebServer_RegisterVirtualDirectory(webServerToken, this->m_VirDirName, this->m_VirDirNameLen, Sink_HandleWebRequest, this);
	ILibWebServer_SetTag (webServerToken, this);
}

struct CdsMediaResource* ResourceProvider_HttpContainer::GetResources(const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen)
{
	struct CdsMediaResource *retVal = NULL, *last, *newRes;
	char title[255];
	int addr, i, uriLen;
	char *ignored = NULL;

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

			GetContentUri(title, "", 0, dbEntry->Key, EXTENSION_PLAYLIST_M3U, this->m_VirDirName, addr, this->m_PortNumber, newRes->Value, ignored);
			
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

int ResourceProvider_HttpContainer::GetNumTransfers()
{
	//TODO: ResourceProvider_HttpContainer::GetNumTransfers()
	return 0;
}

void ResourceProvider_HttpContainer::DestroyTransferList(TransferProgress **tp)
{
}
TransferProgress** ResourceProvider_HttpContainer::GetTransferList(int startIndex, int maxCount, int *numReturned)
{
	//TODO: ResourceProvider_HttpContainer::GetTransfers()

	if (numReturned != NULL) *numReturned = 0;


	return NULL;
}

void ResourceProvider_HttpContainer::Sink_HandleWebRequest(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user)
{
	((ResourceProvider_HttpContainer*)user)->Sink_HandleWebRequest(session, header, bodyBuffer, beginPointer, endPointer, done);
}

void ResourceProvider_HttpContainer::Sink_HandleWebRequest (struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done)
{
	int slashPos, dotPos, fileSize;
	long contentKey;
	MediaItemData *dbEntry, *ple;
	const char *ct;
	bool fileFound = false;
	HANDLE fileHandle;
	LARGE_INTEGER fileLength;
	char *buf;
	int addr;
	int resX=-1, resY=-1, duration=-1, bitrate=-1, colorDepth=-1, bitsPerSample=-1, sampleFrequency=-1, nrAudioChannels=-1;
	char ext[MAX_FILE_EXT_LEN], ext2[MAX_FILE_EXT_LEN], protection[MAX_PROT_STRING_LEN];
	int childCount, playlistBytesSize;
	unsigned long time;
	char title[255], creator[255];
	char *playlistBytes= NULL, *pbi;

	ext[0] = '\0';
	ext2[0] = '\0';
	protection[0] = '\0';
	title[0] = '\0';
	creator[0] = '\0';

	slashPos = IndexOf(header->DirectiveObj+1, "/");
	if (ILibGetLong(header->DirectiveObj+1, slashPos, &contentKey) == 0)
	{
		dbEntry = this->m_Database->QueryItem(contentKey);

		if (dbEntry != NULL)
		{
			switch (dbEntry->Class)
			{	
			case MEDIA_DB_CL_C_STORAGE:
			case MEDIA_DB_CL_C_PLAYLIST:
				ple = this->m_Database->QueryContainerItems(contentKey, MEDIA_DB_SORT_KEY);
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
						GetContentUri(title, creator, ple->Creator_length, ple->Key, ext2, this->m_VirDirName, addr, this->m_PortNumber, pbi, pbi);
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
			}
		}
		delete (dbEntry);
	}

	if (fileFound)
	{
		//
		// TODO: Enable this to perform streaming functions in ResourceProvider_HttpContainer::Sink_HandleWebRequest 
		//

		// This session will enable simultaneous file transfers on a single thread by
		// allowing Sink_HandleSendOK to send data in chunks to the requester.
		session->OnSendOK = Sink_HandleSendOK;

		// Build an object that will keep state for this HTTP transfer
		if(header->DirectiveLength==3 && strncasecmp(header->Directive,"GET",3)==0)
		{
		}
		else if(header->DirectiveLength==4 && strncasecmp(header->Directive,"HEAD",4)==0)
		{
		}
		else
		{												 
			ILibWebServer_Send_Raw(session,"HTTP/1.1 400 Not Supported\r\nContent-Length: 0\r\n\r\n",49,1,1);
		}
	}
	else
	{
		// Send a FILE NOT FOUND message.
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found or File is Locked\r\nContent-Length: 0\r\n\r\n",68,1,1);
	}
}

void ResourceProvider_HttpContainer::Sink_HandleSendOK(struct ILibWebServer_Session *session)
{
	//TODO: Enable streaming in ResourceProvider_HttpItem::Sink_HandleSendOK
}

void ResourceProvider_HttpContainer::Sink_HandleDisconnect(struct ILibWebServer_Session *session)
{
	//TODO: ResourceProvider_HttpItem::Sink_HandleDisconnect
}
