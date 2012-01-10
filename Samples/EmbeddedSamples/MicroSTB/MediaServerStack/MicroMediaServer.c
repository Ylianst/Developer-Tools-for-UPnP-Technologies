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


#define MULTIPART_RANGE_DELIMITER "{{{{{-S3P4R470R-}}}}}"
#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <string.h>
#include "UpnpMicroStack.h"
#include "ILibParsers.h"
#include "MicroMediaServer.h"
#include "MyString.h"
#include "PortingFunctions.h"
#include "MimeTypes.h"

#include "CdsMediaObject.h"
#include "CdsMediaClass.h"
#include "CdsObjectToDidl.h"

#include "ILibWebServer.h"

#include "MicroSTB.h"
#include "Utility.h"

#ifdef UNDER_CE
#define strnicmp _strnicmp
#define assert ASSERT
#endif

#ifdef WIN32
#ifndef UNDER_CE
#include "assert.h"
#endif
#endif

#ifdef _POSIX
#include "assert.h"
#define strnicmp strncasecmp
#include <semaphore.h>
#endif

// POSIX-style synchronization
#ifndef _POSIX
	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif


/************************************************************************************/
/* START SECTION - Configuration info for the media server. */

/* This value should be one of the xxx_DIR_DELIMITER_STR values defined in the FileSystem configuration section. */
char* DIRDELIMITER;

/* The value of the shared root path. String value and its corresponding length variable initialized through SetRootPath(). */
char* ROOTPATH;
int ROOTPATHLENGTH;

/* MMS Stats */
void (*MmsOnStatsChanged) (void) = NULL;
void (*MmsOnTransfersChanged) (int) = NULL;
int MmsBrowseCount = 0;
int MmsHttpRequestCount = 0;
int MmsCurrentTransfersCount = 0;
struct MMSMEDIATRANSFERSTAT MmsMediaTransferStats[DOWNLOAD_STATS_ARRAY_SIZE];

void *MMS_Chain;
void *MMS_MicroStack;

//sem_t UpnpIPAddressListLock;
//int *UpnpIPAddressList;
//int UpnpIPAddressListLength;
/* END SECTION - Internal state variables and configuration for the media server. */
/************************************************************************************/


/************************************************************************************/
/* START SECTION - Stuff specific to FileSystem stuff. */

/* FileName To Didl struct */
struct FNTD
{
	char*	DirDelimiter;	/* Delimiter used for directory names */
	char*	Root;			/* Root path. */
	int		RootLength;		/* Length of Root */

    char*	Filter;			/* Comma separated list of tags to include. Use the * char to indicate all fields. NULL indicates minimum.*/
	unsigned int SI;		/* Starting index */
	unsigned int RC;		/* Requested count */
	unsigned int CI;		/* Current index - used internally. */
	unsigned int NR;		/* OUT: Number returned in DIDL response. used internally. */
	unsigned int TM;		/* OUT: Total number of matches. used internally */
	unsigned long UpdateID;	/* OUT: UpdateID. used internally. */

	FILE*	File;			/* Print DIDL here if non-NULL. Works in conjuction with Socket. */
	char*	String;			/* strcat DIDL here if non-NULL */
	int		FileSize;		/* length of the file in bytes - used for res@size */

	const char*	ArgName;		/* Needed if UpnpToken != NULL */
	void*	UpnpToken;		/* Print DIDL here if non-NULL. Works in conjunction with File. */

	char* BaseUri;			/* http://[ip address]:[port]/dir */

	int *AddressList;
	int AddressListLen;
	int Port;
};

/* CHAR and STRING defines for Win32 directory delimiter */
#define WIN32_DIR_DELIMITER_CHR '\\'
#define WIN32_DIR_DELIMITER_STR "\\"

/* CHAR and STRING defines for UNIX directory delimiter */
#define UNIX_DIR_DELIMITER_CHR '/'
#define UNIX_DIR_DELIMITER_STR "/"

#define MAX_PATH_LENGTH 4096

/* #define stuff used for ProcessDir */
#define RECURSE_NEVER			0	/* Never recurse */
#define RECURSE_WHEN_FOUND		1	/* Recurse directories immediately when entry found. */
#define RECURSE_AFTER_PROCESS	2	/* NOT IMPLEMENTED: Recurse subdirs after done processing entries in the directory. */
#define RECURSE_BEFORE_PROCESS	3	/* NOT IMPLEMENTED: Recurse subdirs before processing any entries in the directory.*/
#define PROCESS_WHEN_FOUND	0	/* Process the entry immediately. */
#define PROCESS_DIRS_FIRST	1	/* Process directory entries first. */
#define PROCESS_FILE_FIRST	2	/* Process file entries first. */

/* Returns file extension for ASCII-encoded paths. */
char* GetFileExtension(char* pathName, int returnCopy)
{
	int len;
	int i;

	len = (int) strlen(pathName);

	for (i = len-1; i >= 0; i--)
	{
		if (
			(WIN32_DIR_DELIMITER_CHR == pathName[i]) ||
			(UNIX_DIR_DELIMITER_CHR == pathName[i])
			)
		{
			return NULL;
		}

		if ('.' == pathName[i])
		{
			if (returnCopy == 0)
			{
				return pathName+i;
			}
			else
			{
				char* deepCopy = (char*) MALLOC(len+1);
				strcpy(deepCopy, pathName);
				return deepCopy;
			}
		}
	}

	return NULL;
}

int GetLastIndexOfParentPath(char* pathName, char* dirDelimiter, int includeDelimiter)
{
	int dLen;
	int pLen;
	int i, j, ij, len;
	int foundDelim = 0;

	dLen = (int) strlen(dirDelimiter);
	pLen = (int) strlen(pathName);

	for (i = pLen; i >= 0; i--)
	{
		for (j = 0; j < dLen; j++)
		{
			ij = i+j;
			if (i+j < pLen)
			{
				if (dirDelimiter[j] != pathName[ij])
				{
					/* if a delimiter char doesn't match, then go to pathName[i-1] */
					break;
				}
				if (j == dLen-1)
				{
					/* make sure we didn't find a delimiter that ends pathName */
					if (i+dLen < pLen)
					{
						foundDelim = 1;
					}
				}
			}
		}

		if (0 != foundDelim)
		{
			if (0 != includeDelimiter)
			{
				len = i+dLen;
			}
			else
			{
				len = i;
			}

			return len-1;
		}
	}

	return -1;
}

/* Returns parent path of ASCII encoded path */
char* GetParentPath(char* pathName, char* dirDelimiter, int includeDelimiter)
{
	char* substring;
	int pos;

	pos = GetLastIndexOfParentPath(pathName, dirDelimiter, includeDelimiter) + 1;
	if (pos >= 0)
	{
		substring = (char*) MALLOC(pos+1);
		strncpy(substring, pathName, pos);
		substring[pos] = '\0';
		return substring;
	}
	return NULL;
}

/* Returns filename for ascii-encoded path */
char* GetFileName(char* pathName, char* dirDelimiter, int returnExtension)
{
	int pos = GetLastIndexOfParentPath(pathName, dirDelimiter, 1);
	int pLen = (int) strlen(pathName);
	int len = pLen - pos;
	int dotPos;
	char* name;
	int i,j;
	int nlen;

	if (returnExtension == 0)
	{
		dotPos = LastIndexOf(pathName, ".");
		if ((dotPos >= 0) && (dotPos >= pos))
		{
			len = len - (pLen-dotPos);
		}
	}

	name = (char*)MALLOC(len+1);
	j=pos+1;
	nlen = len - 1;
	for (i=0; i < len; i++)
	{
		name[i] = pathName[j];
		j++;
	}
	name[nlen] = '\0';

	if (EndsWith(name, dirDelimiter, 0))
	{
		name[nlen-(int) strlen(dirDelimiter)] = '\0';
	}

	return name;
}

void ProcessDir(char* dir, int processWhen, int recurseWhen, void (*callForEachFile)(char*, void*), void* arg)
{
	char path[MAX_PATH_LENGTH];
	char filename[MAX_PATH_LENGTH];
	void* dirObj;
	int ewDD;
	struct FNTD* fntd = (struct FNTD*) arg;
	int nextFile = 0;
	int fileSize = 0;

	if (callForEachFile == NULL) return;

	dirObj = PCGetDirFirstFile(dir,filename,MAX_PATH_LENGTH,&fileSize);

	if (dirObj == NULL)
	{
		fprintf(stderr, "ProcessDir: can't open %s\n", dir);
		return;
	}

	ewDD = EndsWith(dir, DIRDELIMITER, 0);

	do
	{
		if (ProceedWithDirEntry(dir, filename, MAX_PATH_LENGTH) != 0)
		{

			/* path is acceptable length */

			if (ewDD != 0)
			{
				sprintf(path, "%s%s", dir, filename);
			}
			else
			{
				sprintf(path, "%s%s%s", dir, DIRDELIMITER, filename);
			}

			fntd->FileSize = fileSize;
			(callForEachFile)(path, fntd);
		}

		nextFile = PCGetDirNextFile(dirObj,dir,filename,MAX_PATH_LENGTH, &fileSize);
	}
	while (nextFile != 0);

	PCCloseDir(dirObj);
}

/* END SECTION - Stuff specific to FileSystem stuff. */
/************************************************************************************/


/************************************************************************************/
/* START SECTION - Virtual Directory stuff*/

struct RangeRequest
{
	int StartIndex;
	int BytesLeft;
	int MultiRange;
};

struct WebRequest
{
	void* UpnpToken;
	char* UnescapedDirectiveObj;
	int UnescapedDirectiveObjLen;
	
	char *ansiPath;
	char *fullPath;

	char* Root;
	int RootLen;
	
	char* DirDelimiter;
	int DirDelimLen;
	
	char* VirtualDir;
	int VirDirLen;

	FILE *f;
	char *buffer;
	int totalBytes;
	int sentBytes;
	int tsi;
	struct packetheader *p;
	void *Range;
	char *ct;
	int cl;
};

char MungeHexDigit(char* one_hexdigit)
{
	char r = -1;
	char c = *one_hexdigit;
	if (c >= '0' && c <= '9')
	{
		r = c - '0';
	}
	else if (c >= 'A' && c <= 'F')
	{
		r = c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'F')
	{
		r = c - 'a' + 10;
	}

	return r;
}

char GetCharFromHex(char* two_hexdigits)
{
	char c1 = MungeHexDigit(two_hexdigits);
	char c2 = MungeHexDigit(two_hexdigits+1);

	char result = 0;
	if (c1 != -1 && c2 != -1)
	{
		result = (c1 << 4) + c2;
	}
	return result;
}

#define SENDSIZE 32768
char *MMS_STRING_ROOT = "Root";
char *MMS_STRING_UNKNOWN = "Unknown";
char *MMS_STRING_RESULT = "Result";

void HandleDisconnect(struct ILibWebServer_Session *session)
{
	struct WebRequest *wr;
	if(session->User2!=NULL)
	{
		wr = (struct WebRequest*)session->User2;
		memset(&(MmsMediaTransferStats[wr->tsi]),0,sizeof(struct MMSMEDIATRANSFERSTAT));
		if (MmsOnTransfersChanged != NULL) 
		{
			MmsOnTransfersChanged(wr->tsi);
		}
		FREE(wr->ansiPath);
		FREE(wr->fullPath);
		FREE(wr->buffer);
		FREE(wr->UnescapedDirectiveObj);
		ILibDestructPacket(wr->p);
		FREE(wr);
		session->User2 = NULL;
	}
}

void HandleSendOK(struct ILibWebServer_Session *session)
{
	int z,numRead;
	struct WebRequest *wr = (struct WebRequest*)session->User2;
	struct RangeRequest *rr = NULL;
	char *buf;

	if(wr->f!=NULL)
	{
		if(wr->Range!=NULL)
		{
			rr = (struct RangeRequest*)ILibQueue_PeekQueue(wr->Range);
			if(rr!=NULL)
			{
				PCFileSeek(wr->f,(long)rr->StartIndex,SEEK_SET);
			}
		}
		do
		{
			z=1;
			numRead = (int)PCFileRead(wr->buffer, 1, SENDSIZE, wr->f);
			if(rr!=NULL && numRead>0 && numRead>rr->BytesLeft)
			{
				numRead = rr->BytesLeft;
			}
			if(numRead>0 && (rr==NULL||((rr!=NULL)&&( rr->BytesLeft>0))))
			{
				wr->sentBytes+=numRead;
				if(rr!=NULL) {rr->BytesLeft-=numRead;}
				MmsMediaTransferStats[wr->tsi].position = wr->sentBytes;
				if (MmsOnTransfersChanged != NULL) 
				{
					MmsOnTransfersChanged(wr->tsi);
				}
				z = ILibWebServer_StreamBody(session,wr->buffer,numRead,1,0);
			}
			else
			{
				memset(&(MmsMediaTransferStats[wr->tsi]),0,sizeof(struct MMSMEDIATRANSFERSTAT));
				if (MmsOnTransfersChanged != NULL) 
				{
					MmsOnTransfersChanged(wr->tsi);
				}

				if(rr!=NULL)
				{
					rr = ILibQueue_DeQueue(wr->Range);
					FREE(rr);
					rr = ILibQueue_PeekQueue(wr->Range);
					if(rr==NULL)
					{
						ILibQueue_Destroy(wr->Range);
						wr->Range = NULL;
					}
					else
					{
						z=0;
						PCFileSeek(wr->f,(long)rr->StartIndex,SEEK_SET);
						if(rr->MultiRange!=0)
						{
							buf = (char*)MALLOC(1024);
							sprintf(buf,"%s\r\nContent-Type: %s\r\nContent-Range: bytes %d-%d/%d\r\n\r\n",MULTIPART_RANGE_DELIMITER,wr->ct,rr->StartIndex,rr->BytesLeft,wr->cl);
							ILibWebServer_StreamBody(session,buf,(int)strlen(buf),0,0);
						}
						continue;
					}
				}
				// Done Reading
				session->OnSendOK = NULL;
				session->User2 = NULL;

				ILibWebServer_StreamBody(session,wr->buffer,0,1,1);
				PCFileClose(wr->f);
				wr->f = NULL;
				FREE(wr->ansiPath);
				FREE(wr->fullPath);
				ILibDestructPacket(wr->p);
				FREE(wr->buffer);
				FREE(wr->UnescapedDirectiveObj);
				FREE(wr);
			}
		}while(z==0);
	}
}

void* HandleWebRequest(void* webRequest)
{
	struct WebRequest* wr = (struct WebRequest*) webRequest;
	int ewSlash;
	char* lastChar;
	char* dj;
	char* di;
	int copied, slashCount, si;
	int fpBufLen;
	char *fp, *ud;
	int k;
	char* ext;
	char* ct;
	int cl = 0;
	char *buf;
	int bufLen;
	int dirEntryType = 0;
	void* f;
	int totalSent;
	int sendStatus;
	int numRead;
	int fpLen,fpSize;
	int transferStatIndex = -1;
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)wr->UpnpToken;
	int z;
	struct packetheader_field_node *phf;
	struct parser_result *pr,*pr2,*pr3;
	struct parser_result_field *prf;
	struct RangeRequest *rr;

	session->OnDisconnect = &HandleDisconnect;

	wr->buffer = NULL;
	MmsCurrentTransfersCount++;
	if (MmsOnStatsChanged != NULL) MmsOnStatsChanged();

	/* remove trailing slash from directive, if present */
#ifdef _DEBUG
	printf("\r\nDirective1='%s'", wr->p->DirectiveObj);
#endif
	ewSlash = EndsWith(wr->p->DirectiveObj, "/", 0);
	if (ewSlash != 0)
	{
		wr->p->DirectiveObj[wr->p->DirectiveObjLength-1] = '\0';
		wr->p->DirectiveObjLength = wr->p->DirectiveObjLength - 1;
	}
#ifdef _DEBUG
	printf("\r\nDirective2='%s'", wr->p->DirectiveObj);
#endif

	/* convert from escaped HTTP directive to unescaped directive */
	wr->UnescapedDirectiveObj = (char*) MALLOC (wr->p->DirectiveObjLength+1);
	wr->UnescapedDirectiveObjLen = wr->p->DirectiveObjLength;
	lastChar = wr->p->DirectiveObj + wr->p->DirectiveObjLength;
	dj = wr->p->DirectiveObj;
	di = wr->UnescapedDirectiveObj;

	while (dj < lastChar)
	{
		copied = 0;
		if (*dj == '%')
		{
			char r = GetCharFromHex(dj+1);

			if (r != 0)
			{
				*di = r;
				dj = dj + 2;
				copied = 1;
			}
		}

		if (copied == 0)
		{
			*di = *dj;
		}
		dj = dj + 1;
		di = di + 1;
	}
	*di = '\0';
	wr->UnescapedDirectiveObjLen = (int) (di - wr->UnescapedDirectiveObj);
	printf("\r\nUnescapedDirective='%s' %d=%d\r\n", wr->UnescapedDirectiveObj, wr->UnescapedDirectiveObjLen, (int) strlen(wr->UnescapedDirectiveObj));


	/* determine the full local path where the directive should map to */
	slashCount = 0;

	for (si=0; si <wr->UnescapedDirectiveObjLen; si++)
	{
		if (wr->UnescapedDirectiveObj[si] == '/')
		{
			slashCount = slashCount + 1;
		}
	}
	fpBufLen = wr->RootLen +wr->UnescapedDirectiveObjLen + (slashCount*wr->DirDelimLen) + 1;
	wr->fullPath = (char*) MALLOC (fpBufLen);
	sprintf(wr->fullPath, "%s", wr->Root);

	fp = wr->fullPath + wr->RootLen;
	ud = wr->UnescapedDirectiveObj + wr->VirDirLen + 2;


	while (*ud != '\0')
	{
		if (*ud == '/')
		{
			for (k=0; k < wr->DirDelimLen; k++)
			{
				*fp = wr->DirDelimiter[k];
			}
			fp = fp + k;
		}
		else
		{
			*fp = *ud;
			fp = fp + 1;
		}

		ud = ud + 1;
	}
	*fp = '\0';

	fpLen = (int) strlen(wr->fullPath);
	printf("Requesting='%s' strlen='%d'\r\n", wr->fullPath, fpLen);

	fpSize = fpLen+1;
	wr->ansiPath = (char*) MALLOC(fpSize);

	/* TODO: Simply copy wr->fullPath to wr->ansiPath if PortingFunctions
	 *		 has been modified to support unicode file paths.
	 */
	Utf8ToAnsi(wr->ansiPath, wr->fullPath, fpSize);

	/* determine if the path is a file or a directory, or nonexistent */

	dirEntryType = PCGetFileDirType(wr->fullPath);

	if (dirEntryType > 0)
	{
		if (dirEntryType == 1)
		{
			/* is a file */
			printf("\r\nFound='%s' strlen='%d'\r\n", wr->fullPath, (int) strlen(wr->ansiPath));
			/* otherwise, just send the file */

			ext = (char*) GetFileExtension(wr->ansiPath, 0);
			ct = (char*) FileExtensionToMimeType(ext, 0);

			if (*ct == '\0') { ct = "application/octet-stream"; }

			f = PCFileOpen(wr->ansiPath, "rb");

			if (f != NULL)
			{
				if (transferStatIndex != -1)
				{
					#ifdef _WIN32_WCE
					PCFileSeek(f,0,SEEK_END);
					MmsMediaTransferStats[transferStatIndex].length = PCFileTell(f);
					PCFileSeek(f,0,SEEK_SET);
					#endif

					#ifdef WIN32
					PCFileSeek(f,0,SEEK_END);
					MmsMediaTransferStats[transferStatIndex].length = PCFileTell(f);
					PCFileSeek(f,0,SEEK_SET);
					#endif
				}
			}

			totalSent =0;
			sendStatus = 0;


			if (f != NULL)
			{
				for (k=0;k<20;k++)
				{
					if (MmsMediaTransferStats[k].filename == NULL)
					{
						transferStatIndex = k;
						MmsMediaTransferStats[transferStatIndex].filename = wr->fullPath;
						MmsMediaTransferStats[transferStatIndex].download = 1;
						MmsMediaTransferStats[transferStatIndex].length = 0;
						MmsMediaTransferStats[transferStatIndex].position = 0;
						if (MmsOnTransfersChanged != NULL) MmsOnTransfersChanged(transferStatIndex);
						break;
					}
				}

				cl = PCGetFileSize(wr->fullPath);
				buf = (char*)MALLOC(2048);
				
				wr->tsi = transferStatIndex;
				wr->totalBytes = cl;
				wr->sentBytes=0;
				wr->f = f;
				session->OnSendOK = &HandleSendOK;
				session->User2 = wr;
				
				// Check If Range Request
				phf = wr->p->FirstField;
				while(phf!=NULL)
				{
					if(phf->FieldLength==5 && strncasecmp(phf->Field,"RANGE",5)==0)
					{
						wr->Range = ILibQueue_Create();
						pr = ILibParseString(phf->FieldData,0,phf->FieldDataLength,"=",1);
						pr2 = ILibParseString(pr->LastResult->data,0,pr->LastResult->datalength,",",1);
						prf = pr2->FirstResult;
						while(prf!=NULL)
						{
							rr = (struct RangeRequest*)MALLOC(sizeof(struct RangeRequest));
							rr->MultiRange=(pr2->NumResults==1?0:1);
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
							ILibQueue_EnQueue(wr->Range,rr);
							ILibDestructParserResults(pr3);
							prf = prf->NextResult;
						}
						ILibDestructParserResults(pr2);
						ILibDestructParserResults(pr);
						break;
					}
					phf=phf->NextField;
				}
				rr=NULL;
				if(wr->Range!=NULL)
				{
					rr = (struct RangeRequest*)ILibQueue_PeekQueue(wr->Range);
					if(rr->MultiRange==0)
					{
						// Single Range Request
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Range: bytes %d-%d/%d\r\nContent-Type: %s", rr->StartIndex,rr->BytesLeft,cl,ct);
					}
					else
					{
						// MultiPart Range Request
						wr->ct = ct;
						wr->cl = cl;
						bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nContent-Type: multipart/byteranges; boundary=%s",MULTIPART_RANGE_DELIMITER);
					}
				}
				else
				{
					rr=NULL;
					bufLen = sprintf(buf, "\r\nServer: Intel CEL / MicroMediaServer\r\nAccept-Range: bytes\r\nContent-Type: %s", ct);
				}
				if(wr->p->DirectiveLength==4 && strncasecmp(wr->p->Directive,"HEAD",4)==0)
				{
					PCFileClose(wr->f);
					wr->f = NULL;
					if(wr->Range!=NULL)
					{
						ILibWebServer_Send_Raw(session,"HTTP/1.1 206 Partial Content",28,1,0);
					}
					else
					{
						ILibWebServer_Send_Raw(session,"HTTP/1.1 200 OK",15,1,0);
					}
					ILibWebServer_Send_Raw(session,buf,(int)strlen(buf),0,0);
					ILibWebServer_Send_Raw(session,"\r\n\r\n",4,1,1);
					return(NULL);
				}

				if(wr->Range!=NULL)
				{
					ILibWebServer_StreamHeader_Raw(wr->UpnpToken,206,"Partial Content",buf,0);
					if(rr->MultiRange!=0)
					{
						buf = (char*)MALLOC(1024);
						bufLen = sprintf(buf,"%s\r\nContent-Type: %s\r\nContent-Range: bytes %d-%d/%d\r\n\r\n",MULTIPART_RANGE_DELIMITER,wr->ct,rr->StartIndex,rr->BytesLeft,wr->cl);
						ILibWebServer_StreamBody(session,buf,(int)strlen(buf),0,0);
					}
				}
				else
				{
					ILibWebServer_StreamHeader_Raw(wr->UpnpToken,200,"OK",buf,0);
				}
				wr->buffer = (char*)MALLOC(SENDSIZE);
				
				if(rr!=NULL)
				{
					PCFileSeek(wr->f,(long)rr->StartIndex,SEEK_SET);
				}

				do
				{
					z=1;
					numRead = (int)PCFileRead(wr->buffer, 1, SENDSIZE, wr->f);
					if(rr!=NULL && numRead>0 && numRead>rr->BytesLeft)
					{
						numRead = rr->BytesLeft;
					}
					if(numRead>0 && (rr==NULL||((rr!=NULL)&&( rr->BytesLeft>0))))
					{
						wr->sentBytes+=numRead;
						if(rr!=NULL) {rr->BytesLeft-=numRead;}
						MmsMediaTransferStats[transferStatIndex].position = wr->sentBytes;
						if (MmsOnTransfersChanged != NULL) 
						{
							MmsOnTransfersChanged(transferStatIndex);
						}
						z = ILibWebServer_StreamBody(session,wr->buffer,numRead,1,0);
					}
					else
					{
						// Done Reading
						if(rr!=NULL)
						{
							rr = ILibQueue_DeQueue(wr->Range);
							FREE(rr);
							rr = ILibQueue_PeekQueue(wr->Range);
							if(rr==NULL)
							{
								ILibQueue_Destroy(wr->Range);
								wr->Range = NULL;
							}
							else
							{
								z=0;
								PCFileSeek(wr->f,(long)rr->StartIndex,SEEK_SET);
								if(rr->MultiRange!=0)
								{
									buf = (char*)MALLOC(1024);
									bufLen = sprintf(buf,"%s\r\nContent-Type: %s\r\nContent-Range: bytes %d-%d/%d\r\n\r\n",MULTIPART_RANGE_DELIMITER,wr->ct,rr->StartIndex,rr->BytesLeft,wr->cl);
									ILibWebServer_StreamBody(session,buf,(int)strlen(buf),0,0);
								}
								continue;
							}
						}

						memset(&(MmsMediaTransferStats[wr->tsi]),0,sizeof(struct MMSMEDIATRANSFERSTAT));
						if (MmsOnTransfersChanged != NULL) 
						{
							MmsOnTransfersChanged(wr->tsi);
						}

						session->OnSendOK = NULL;
						session->User2 = NULL;
						ILibWebServer_StreamBody(session,wr->buffer,0,1,1);
						f = wr->f;
						wr->f = NULL;
						PCFileClose(f);
						FREE(wr->ansiPath);
						FREE(wr->fullPath);
						FREE(wr->buffer);
						FREE(wr->UnescapedDirectiveObj);
						ILibDestructPacket(wr->p);
						FREE(wr);
						wr = NULL;
					}
				}while(z==0);
			}
			else
			{
				ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found or File is Locked\r\nContent-Length: 0\r\n\r\n",66,1,1);
			}
		}
		else
		{
			ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found or File is Locked\r\nContent-Length: 0\r\n\r\n",66,1,1);
		}
	}
	else
	{
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found or File is Locked\r\nContent-Length: 0\r\n\r\n",66,1,1);
	}


	if (wr != NULL)
	{
		/* if we don't actually transfer stuff, we still need to deallocate some memory */
		FREE(wr->ansiPath);
		FREE(wr->fullPath);
		FREE(wr->UnescapedDirectiveObj);
		ILibDestructPacket(wr->p);
		FREE(wr);
		wr = NULL;
	}

	
	return NULL;
}

/* END SECTION - Virtual Directory stuff */
/************************************************************************************/


/************************************************************************************/
/* START SECTION - Stuff specific to CDS */

/* BrowseFlags */
#define BROWSEMETADATA "BrowseMetadata"
#define BROWSEDIRECTCHILDREN "BrowseDirectChildren"

/* DIDL formating */
#define DIDL_HEADER "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp\">"
#define DIDL_FOOTER "\r\n</DIDL-Lite>"

#define DIDL_HEADER_ESCAPED "&lt;DIDL-Lite xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite&quot; xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp&quot;&gt;"
#define DIDL_FOOTER_ESCAPED "\r\n&lt;/DIDL-Lite&gt;"
#define DIDL_HEADER_ESCAPED_LEN 195
#define DIDL_FOOTER_ESCAPED_LEN 20

/* MAX length for a DIDL entry should be 1024 */
#define DIDL_ENTRY_LEN 1024

struct BrowseRequest
{
	void* UpnpToken;	/* no copy */
	char* ObjectID;		/* deep copied */
	char* BrowseFlag;	/* deep copied */
	char* Filter;		/* deep copied */
	unsigned int StartingIndex;
	unsigned int RequestedCount;
	char* SortCriteria;	/* deep copied */
	char* BaseUri;		/* must MMS_FREE */
	int * AddressList;  /* deep copied */
	int AddressListLen;
	int Port;
};

void InitFntd(struct FNTD* fntd)
{
	/*fntd->Extensions = NULL;*/
	/*fntd->MimeTypes = NULL;*/
	fntd->Root = NULL;
	fntd->RootLength = -1;
	fntd->DirDelimiter = NULL;

	fntd->Filter = NULL;
	fntd->SI = 0;
	fntd->RC = 0;
	fntd->CI = 0;
	fntd->NR = 0;
	fntd->TM = 0;
	fntd->UpdateID = 0;

	fntd->File = NULL;
	fntd->String = NULL;
	fntd->UpnpToken = NULL;
	fntd->ArgName = NULL;

	fntd->BaseUri = NULL;
}



void DirectoryEntryToDidl(char* pathName, struct FNTD* fntd)
{
	char* ext = NULL;
	unsigned int mediaClass = 0;
	char* mime = NULL;
	char *entry = NULL;
	char* parentDir = NULL;
	char* parentID = NULL;
	int fnLen;
	char* title = NULL;
	int ddLen;
	int idLen;
	int pidLen;
	char* id = NULL;
	int j;

	char *cpId, *cpParentId, *cpPath;
	int cpIdLen, cpParentIdLen, cpPathLen;

	int ewDD;
	char* baseUri = NULL;
	int fdType = 0xFFFFFFFF;
	char* pchar;
	int ri;
	int entryLen;

	unsigned int filterMask = 0;
	struct CdsMediaObject *cdsObj = NULL;
	struct CdsMediaResource **cdsRes = NULL;

	if (pathName == NULL) return;
	if (fntd == NULL) return;

	/* obtain a bit array describing what metadata to include in the response */
	filterMask = CdsToDidl_GetFilterBitString(fntd->Filter);

	baseUri = fntd->BaseUri;
	fdType = PCGetFileDirType(pathName);

	if (fdType == 2)
	{
		mediaClass = CDS_MEDIACLASS_CONTAINER;
		mime = "";
		ext = "";
	}
	else
	{
		ext = GetFileExtension(pathName, 0);
		if (ext != NULL)
		{
			mime = (char*) FileExtensionToMimeType(ext, 0);
		}
	}

	if ((fntd->CI >= fntd->SI) && ((fntd->NR < fntd->RC) || (fntd->RC == 0)))
	{
		entry = NULL;

		/* get parentID and parent directory */
		parentDir = NULL;
		parentID = NULL;

		if (strcmp(pathName, fntd->Root) == 0)
		{
			parentID = (char*) MALLOC(3);
			strcpy(parentID, "-1");
		}
		else
		{
			parentDir = GetParentPath(pathName, fntd->DirDelimiter, 1);

			if (strcmp(parentDir, fntd->Root) == 0)
			{
				parentID = (char*) MALLOC(2);
				strcpy(parentID, "0");
			}
			else
			{
				parentID = (char*) MALLOC((int) strlen(parentDir) + 3);
				sprintf(parentID, "0%s%s", fntd->DirDelimiter, parentDir+fntd->RootLength);
				pidLen = (int) strlen(parentID);
				parentID [pidLen-1] = '\0';
				pidLen = pidLen - 1;
			}
		}

		/* get title*/
		fnLen = (int) strlen(pathName);
		title = GetFileName(pathName, fntd->DirDelimiter, 0);

		if (title[0] == '\0')
		{
			FREE(title);
			if (strnicmp(pathName, fntd->Root, (int) strlen(pathName)) == 0)
			{
				title = MMS_STRING_ROOT;
			}
			else
			{
				title = MMS_STRING_UNKNOWN;
			}
		}

		/* get object id: full path, where root=0 */
		ddLen = (int) strlen(fntd->DirDelimiter);
		idLen = fnLen - fntd->RootLength + 1 + ddLen;
		id = (char*) MALLOC (idLen+1);
		j = fntd->RootLength;
		pchar = pathName+j;

		if (pathName[j] == '\0')
		{
			sprintf(id, "0");
		}
		else
		{
			sprintf(id, "0%s%s", fntd->DirDelimiter, pchar);
		}

		/* determine if directory or file */
		if ((id != NULL) && (parentID != NULL) && (title != NULL))
		{

			/* get a copy of the id - ensure that it does not end with a directory delimiter */
			cpIdLen = idLen;
			cpId = (char*) MALLOC(cpIdLen+1);
			memcpy(cpId, id, cpIdLen);
			cpId[cpIdLen] = '\0';
			ewDD = EndsWith(cpId, DIRDELIMITER, 0);
			if (ewDD != 0)
			{
				cpIdLen--;
				cpId[cpIdLen] = '\0';
			}

			/* get parent ID - ensure that it does not end with a directory delimiter */
			cpParentIdLen = (int) strlen(parentID);
			cpParentId = (char*) MALLOC(cpParentIdLen+1);
			memcpy(cpParentId, parentID, cpParentIdLen);
			cpParentId[cpParentIdLen] = '\0';
			ewDD = EndsWith(cpParentId, DIRDELIMITER, 0);
			if (ewDD != 0)
			{
				cpParentIdLen--;
				cpParentId[cpParentIdLen] = '\0';
			}

			/* get a copy of the path - ensure directory delimiter is forward slash */
			cpPathLen = (int) strlen(pathName+ROOTPATHLENGTH);
			cpPath = (char*) MALLOC(cpPathLen+1);
			memcpy(cpPath, pathName+ROOTPATHLENGTH, cpPathLen);
			cpPath[cpPathLen] = '\0';
			if (strcmp(UNIX_DIR_DELIMITER_STR, DIRDELIMITER) != 0)
			{
				int x;
				for (x=0; x < cpPathLen; x++)
				{
					if (cpPath[x] == WIN32_DIR_DELIMITER_CHR)
					{
						cpPath[x] = UNIX_DIR_DELIMITER_CHR;
					}
				}
			}

			/*
			 *	determine updateID using date of last modify 
			 */
			fntd->UpdateID = PCGetContainerUpdateID(pathName);

			if (PCGetFileDirType(pathName) == 2)
			{
				if (title != NULL)
				{
					//sprintf(entry, DIDL_CONTAINER_ALL, escapedID, escapedParentID, escapedTitle);
					cdsObj = CDS_AllocateObject();
					cdsObj->Title = title;
					cdsObj->ID = cpId;
					cdsObj->ParentID = cpParentId;
					cdsObj->ChildCount = PCGetGetDirEntryCount(pathName, DIRDELIMITER);
					cdsObj->MediaClass = mediaClass;

					entry = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, 0, filterMask, 0, &entryLen);
					ext = "NOT-NULL";
				}
			}
			else
			{
				if (
					(ext != NULL) &&
					(mime != NULL) &&
					(cpPath != NULL)
					)
				{
					cdsObj = CDS_AllocateObject();
					cdsObj->Title = title;
					cdsObj->ID = cpId;
					cdsObj->ParentID = cpParentId;
					cdsObj->MediaClass = FileExtensionToClassCode(ext, 0);

					cdsRes = &(cdsObj->Res);
					for (ri = 0; ri < fntd->AddressListLen; ri++)
					{
						(*cdsRes) = CDS_AllocateResource();
						(*cdsRes)->Size = fntd->FileSize;
						(*cdsRes)->ProtocolInfo = FileExtensionToProtocolInfo(ext, 0);
						
						(*cdsRes)->Value = (char*) MALLOC(40+cpPathLen);
						sprintf((*cdsRes)->Value, "http://%d.%d.%d.%d:%d/web/%s", (fntd->AddressList[ri]&0xFF), ((fntd->AddressList[ri]>>8)&0xFF), ((fntd->AddressList[ri]>>16)&0xFF), ((fntd->AddressList[ri]>>24)&0xFF), fntd->Port, cpPath);

						cdsRes = &((*cdsRes)->Next);
					}

					entry = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, 0, filterMask, 0, &entryLen);
				}

			}

			FREE(cpId);
			FREE(cpParentId);
			FREE(cpPath);

			if (cdsObj != NULL)
			{
				cdsObj->Creator = NULL;
				cdsObj->ID = NULL;
				cdsObj->ParentID = NULL;
				cdsObj->RefID = NULL;
				cdsObj->Title = NULL;
				CDS_DestroyObjects(cdsObj);
			}
		}

		/* print the entry to the file, socket, and string */
		if (entry != NULL)
		{
			if (fntd->File != NULL)
			{
				fprintf(fntd->File, "%s", entry);
			}

			if (fntd->String != NULL)
			{
				/* BUGBUG: For some reason the POSIX code only works consistently if I stick this printf here. */
				printf(" ");
				strcat(fntd->String, entry);
			}

			if (fntd->UpnpToken != NULL)
			{
				UpnpAsyncResponse_OUT(fntd->UpnpToken, fntd->ArgName, entry, entryLen, 0, 0);
			}

			fntd->NR = fntd->NR + 1;
		}

		/* Clean up */
		if (parentID != NULL) FREE(parentID);
		if (parentDir != NULL) FREE(parentDir);
		if (id != NULL) FREE(id);
		if ((title != MMS_STRING_UNKNOWN) && (title != MMS_STRING_ROOT) && (title != NULL)) FREE(title);
	}

	if (entry != NULL)
	{
		/* intentionally call FREE (not MMS_FREE) because we use CdsToDidl_GetMediaObjectDidlEscaped() */
		FREE(entry);
	}

	/* increment total matches */
	if ((ext != NULL) /*&& (upnpClass != NULL)*/ && (mime != NULL))
	{
		fntd->TM = fntd->TM + 1;
		fntd->CI = fntd->CI + 1;
	}
}

void DirectoryEntryToDidl2(char* pathName, void* fntd)
{
	DirectoryEntryToDidl(pathName, (struct FNTD*)fntd);
}

void BrowseDirectChildren(void* upnpToken, char* dirPath, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* baseUri, int *addressList, int addressListLen, int port)
{
	struct FNTD fntd;
	char numResult[30];

	InitFntd(&fntd);

	fntd.AddressList = addressList;
	fntd.AddressListLen = addressListLen;
	fntd.Port = port;

	fntd.DirDelimiter = DIRDELIMITER;
	fntd.Root = ROOTPATH;
	fntd.RootLength = ROOTPATHLENGTH;
	
	fntd.BaseUri = baseUri;

	fntd.Filter = Filter;
	fntd.SI = StartingIndex;
	fntd.RC = RequestedCount;

	/*fntd.File = stdout;*/
	fntd.UpnpToken = upnpToken;
	fntd.ArgName = MMS_STRING_RESULT;
	
	UpnpAsyncResponse_START(upnpToken, "Browse", "urn:schemas-upnp-org:service:ContentDirectory:1");
	
	UpnpAsyncResponse_OUT(upnpToken, MMS_STRING_RESULT, DIDL_HEADER_ESCAPED, DIDL_HEADER_ESCAPED_LEN,1, 0);
	ProcessDir(dirPath, RECURSE_NEVER, PROCESS_WHEN_FOUND, DirectoryEntryToDidl2, &fntd);
	UpnpAsyncResponse_OUT(upnpToken, MMS_STRING_RESULT, DIDL_FOOTER_ESCAPED, DIDL_FOOTER_ESCAPED_LEN,0, 1);

	sprintf(numResult, "%d", fntd.NR);
	UpnpAsyncResponse_OUT(upnpToken, "NumberReturned", numResult, (int) strlen(numResult), 1,1);

	sprintf(numResult, "%d", fntd.TM);
	UpnpAsyncResponse_OUT(upnpToken, "TotalMatches", numResult, (int) strlen(numResult), 1,1);

	fntd.UpdateID = 0;
	sprintf(numResult, "%u", fntd.UpdateID);
	UpnpAsyncResponse_OUT(upnpToken, "UpdateID", numResult, (int) strlen(numResult), 1,1);

	UpnpAsyncResponse_DONE(upnpToken, "Browse");
}

void BrowseMetadata(void* upnpToken, char* pathName, char* Filter, char* baseUri, int *addressList, int addressListLen, int port)
{
	struct FNTD fntd;
	struct FNTD fntd2;
	char numResult[30];
	int filesize;

	/*
	if (EndsWith(pathName, DIRDELIMITER))
	{
		pathName[(int) strlen(pathName) - (int) strlen(DIRDELIMITER)] = '\0';
	}
	*/

	filesize = PCGetFileSize(pathName);

	InitFntd(&fntd);

	fntd.AddressList = addressList;
	fntd.AddressListLen = addressListLen;
	fntd.Port = port;

	fntd.DirDelimiter = DIRDELIMITER;
	fntd.Root = ROOTPATH;
	fntd.RootLength = ROOTPATHLENGTH;
	
	fntd.BaseUri = baseUri;

	fntd.Filter = Filter;
	fntd.SI = 0;
	fntd.RC = 0;

	/* fntd.File = stdout; */
	fntd.UpnpToken = upnpToken;
	fntd.ArgName = MMS_STRING_RESULT;

	fntd.FileSize = filesize;

	/* get container updateID */
	InitFntd(&fntd2);
	fntd2.DirDelimiter = DIRDELIMITER;
	fntd2.Root = ROOTPATH;
	fntd2.RootLength = ROOTPATHLENGTH;
	fntd2.Filter = "";
	fntd2.UpdateID = 0;

	UpnpAsyncResponse_START(upnpToken, "Browse", "urn:schemas-upnp-org:service:ContentDirectory:1");
	
	UpnpAsyncResponse_OUT(upnpToken, MMS_STRING_RESULT , DIDL_HEADER_ESCAPED, DIDL_HEADER_ESCAPED_LEN,1, 0);
	DirectoryEntryToDidl (pathName, &fntd);
	UpnpAsyncResponse_OUT(upnpToken, MMS_STRING_RESULT , DIDL_FOOTER_ESCAPED, DIDL_FOOTER_ESCAPED_LEN,0, 1);

	UpnpAsyncResponse_OUT(upnpToken, "NumberReturned", "1", 1, 1, 1);
	UpnpAsyncResponse_OUT(upnpToken, "TotalMatches", "1", 1, 1, 1);

	sprintf(numResult, "%u", fntd2.UpdateID);
	UpnpAsyncResponse_OUT(upnpToken, "UpdateID", numResult, (int) strlen(numResult), 1, 1);

	UpnpAsyncResponse_DONE(upnpToken, "Browse");
}

void CdsBrowse(void* upnpToken, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria, char* baseUri, int *addresses, int addressesLen, int port)
{
	int objIdLen;
	char* filepath;
	char* errorMsg;

	if (ObjectID[0] != '0')
	{
		/* error */
		fprintf(stderr, "\r\nERROR: CdsBrowse() - ObjectID not found.");
		UpnpResponse_Error(upnpToken,701,"ObjectID does not exist.");
		return;
	}

	if (SortCriteria != NULL)
	{
		if (SortCriteria[0] != '\0')
		{
			/* error */
			fprintf(stderr, "\r\nERROR: CdsBrowse() - SortCriteria not allowed.");
			UpnpResponse_Error(upnpToken,709,"MediaServer does not support sorting.");
			return;
		}
	}

	/* translate the ObjectID into a usable path */
	objIdLen = ILibInPlaceXmlUnEscape(ObjectID);
	filepath = (char*)MALLOC(ROOTPATHLENGTH + objIdLen + 10);

	if (objIdLen > 2) 
	{
		if (BrowseFlag[6] == 'M')
		{
			sprintf(filepath,"%s%s",ROOTPATH,ObjectID+2);
		}
		else
		{
			sprintf(filepath,"%s%s%s",ROOTPATH,ObjectID+2,DIRDELIMITER);
		}
	} 
	else 
	{
		sprintf(filepath,"%s",ROOTPATH);
	}

	printf("CdsBrowse(): filepath=%s\r\n", filepath);

	switch (PCGetFileDirType(filepath))
	{
		case 0:	/* Does not exist */
			errorMsg = (char*) MALLOC((int) strlen(filepath)+100);
			sprintf(errorMsg, "GetFileAttributes(%s) failed", filepath);
			UpnpResponse_Error(upnpToken,800,errorMsg);
			FREE(errorMsg);
			break;
		case 1: /* Is a File */
			if (strcmp(BrowseFlag, BROWSEMETADATA) == 0)
			{
				BrowseMetadata(upnpToken, filepath, Filter, baseUri, addresses, addressesLen, port);
			}
			else if (strcmp(BrowseFlag, BROWSEDIRECTCHILDREN) == 0)
			{
				/* error */
				fprintf(stderr, "\r\nERROR: CdsBrowse() - BrowseFlag not allowed on an item.");
				UpnpResponse_Error(upnpToken,710,"ObjectID is not a container.");
			}
			break;
		case 2: /* Is a Directory */
			if (strcmp(BrowseFlag, BROWSEMETADATA) == 0)
			{
				BrowseMetadata(upnpToken, filepath, Filter, baseUri, addresses, addressesLen, port);
			}
			else
			{
				BrowseDirectChildren(upnpToken, filepath, Filter, StartingIndex, RequestedCount, baseUri, addresses, addressesLen, port);
			}
			break;
	}

	FREE(filepath);
}


void* DoBrowse(void* browseRequest)
{
	struct BrowseRequest* br = (struct BrowseRequest*) browseRequest;

	printf("DoBrowse()\r\n");

	LockBrowse();

	CdsBrowse(br->UpnpToken, br->ObjectID, br->BrowseFlag, br->Filter, br->StartingIndex, br->RequestedCount, br->SortCriteria, br->BaseUri, br->AddressList, br->AddressListLen, br->Port);

	UnlockBrowse();

	FREE (br->ObjectID);
	FREE (br->BrowseFlag);
	if (br->Filter != NULL)
	{
		FREE (br->Filter);
	}
	if (br->SortCriteria != NULL)
	{
		FREE (br->SortCriteria);
	}
	FREE (br->BaseUri);
	FREE (br->AddressList);
	FREE (br);

#ifdef SPAWN_BROWSE_THREAD
	EndThisThread();
#endif

	return NULL;
}
/* END SECTION - Stuff specific to CDS */
/************************************************************************************/


/************************************************************************************/
/* START SECTION - Dispatch sinks generated in original main.c */

void UpnpMediaServer_ContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	int ipaddr;
	struct BrowseRequest* br;
	int filterLen, sortCriteriaLen;
	int portNum;
	int *addresses;
	int numAddresses,i,swapValue;

	// Update the statictics
	MmsBrowseCount++;
	if (MmsOnStatsChanged != NULL) MmsOnStatsChanged();

	printf("UPnP Invoke: UpnpMediaServer_ContentDirectory_Browse('%s','%s','%s',%d,%d,'%s');\r\n",ObjectID,BrowseFlag,Filter,StartingIndex,RequestedCount,SortCriteria);


	if (ObjectID == NULL)
	{
		UpnpResponse_Error(upnptoken,701,"ObjectID does not exist.");
		return;
	}

	if (BrowseFlag == NULL)
	{
		UpnpResponse_Error(upnptoken,402,"Invalid value for BrowseFlag. Value was null.");
		return;
	}

	/*spawn thread and respond*/
	br = (struct BrowseRequest*) MALLOC(sizeof (struct BrowseRequest));
	br->UpnpToken = upnptoken;

	br->ObjectID = (char*) MALLOC ((int) strlen(ObjectID)+1);
	strcpy(br->ObjectID, ObjectID);

	br->BrowseFlag = (char*) MALLOC ((int) strlen(BrowseFlag)+1);
	strcpy(br->BrowseFlag, BrowseFlag);
  
	if (Filter == NULL)
	{
		br->Filter = NULL;
	}
	else
	{
		filterLen = (int) strlen(Filter);
		br->Filter = (char*) MALLOC (filterLen+1);
		strcpy(br->Filter, Filter);
	}

	br->StartingIndex = StartingIndex;
	br->RequestedCount = RequestedCount;
	
	if (SortCriteria == NULL)
	{
		br->SortCriteria = NULL;
	}
	else
	{
		sortCriteriaLen = (int) strlen(SortCriteria);
		br->SortCriteria = (char*) MALLOC (sortCriteriaLen+1);
		strcpy(br->SortCriteria, SortCriteria);
	}

	ipaddr = ILibWebServer_GetLocalInterface(upnptoken);
	//ipaddr =  UpnpGetLocalInterfaceToHost(((struct ILibWebServer_Session*)upnptoken)->Parent);
	br->BaseUri = (char*) MALLOC(1025);
	portNum = (int)ILibWebServer_GetPortNumber(((struct ILibWebServer_Session*)upnptoken)->Parent);
	//portNum = UpnpGetLocalPortNumber (UpnpGetInstance(br->UpnpToken));
	sprintf(br->BaseUri, "http://%d.%d.%d.%d:%d/web", (ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF), portNum);

	// Obtain all of the local IP addresses
	sem_wait(&UpnpIPAddressListLock);
	numAddresses = UpnpIPAddressListLength;
	addresses = (int*) MALLOC(sizeof(int) * numAddresses);

	memcpy(addresses, UpnpIPAddressList, UpnpIPAddressListLength*sizeof(int));
	sem_post(&UpnpIPAddressListLock);

	// Ensure the order of addresses is correct
	if (addresses[0] != ipaddr)
	{
		swapValue = addresses[0];
		addresses[0] = ipaddr;
		for (i=1; i < numAddresses; i++)
		{
			if (addresses[i] == ipaddr)
			{
				addresses[i] = swapValue;
				break;
			}
		}
	}
	br->AddressList = addresses;
	br->AddressListLen = numAddresses;
	br->Port = portNum;


#ifdef SPAWN_BROWSE_THREAD
	SpawnNormalThread(&DoBrowse, (void*) br);
#else
	DoBrowse((void*) br);
#endif
	
}

void UpnpMediaServer_ContentDirectory_GetSortCapabilities(void* upnptoken)
{
  printf("UPnP Invoke: UpnpMediaServer_ContentDirectory_GetSortCapabilities();\r\n");

  /* TODO: Place Action Code Here... */

  /* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UpnpResponse_MediaServer_ContentDirectory_GetSortCapabilities(upnptoken,"");
}

void UpnpMediaServer_ContentDirectory_GetSystemUpdateID(void* upnptoken)
{
  unsigned int sid;

  printf("UPnP Invoke: UpnpMediaServer_ContentDirectory_GetSystemUpdateID();\r\n");

  /* TODO: Place Action Code Here... */

  /* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
  sid = PCGetSystemUpdateID();
  UpnpResponse_MediaServer_ContentDirectory_GetSystemUpdateID(upnptoken,sid);
}

void UpnpMediaServer_ContentDirectory_GetSearchCapabilities(void* upnptoken)
{
  printf("UPnP Invoke: UpnpMediaServer_ContentDirectory_GetSearchCapabilities();\r\n");

  /* TODO: Place Action Code Here... */

  /* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UpnpResponse_MediaServer_ContentDirectory_GetSearchCapabilities(upnptoken,"");
}

void UpnpMediaServer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
  printf("UPnP Invoke: UpnpMediaServer_ConnectionManager_GetCurrentConnectionInfo(%u);\r\n",ConnectionID);

  /* TODO: Place Action Code Here... */

  UpnpResponse_Error(upnptoken,706,"Specified connection does not exist.");
}

void UpnpMediaServer_ConnectionManager_GetProtocolInfo(void* upnptoken)
{
  printf("UPnP Invoke: UpnpMediaServer_ConnectionManager_GetProtocolInfo();\r\n");

  /* TODO: Place Action Code Here... */

  /* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UpnpResponse_MediaServer_ConnectionManager_GetProtocolInfo(upnptoken,"http-get:*:*:*","");
}

void UpnpMediaServer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
  printf("UPnP Invoke: UpnpMediaServer_ConnectionManager_GetCurrentConnectionIDs();\r\n");

  /* TODO: Place Action Code Here... */

  /* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UpnpResponse_MediaServer_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"");
}

#ifndef EXCLUDE_MMS_PRESENTATION_REQUEST
void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	struct WebRequest* webRequest = (struct WebRequest*) MALLOC(sizeof(struct WebRequest));
	memset(webRequest,0,sizeof(struct WebRequest));

	webRequest->UpnpToken = upnptoken;
	webRequest->p = ILibClonePacket(packet);

	MmsHttpRequestCount++;
	if (MmsOnStatsChanged != NULL) MmsOnStatsChanged();

	webRequest->Root = ROOTPATH;
	webRequest->RootLen = ROOTPATHLENGTH;

	webRequest->DirDelimiter = DIRDELIMITER;
	webRequest->DirDelimLen = (int) strlen(DIRDELIMITER);

	webRequest->VirtualDir = "web";
	webRequest->VirDirLen = (int) strlen(webRequest->VirtualDir);
	HandleWebRequest((void*)webRequest);
	//SpawnNormalThread(&HandleWebRequest, (void*) webRequest);
}
#endif
/* END SECTION - Dispatch sinks generated in original main.c */
/************************************************************************************/


/************************************************************************************/
/* START SECTION - MicroMediaServer interface implementations */

void InitMms(void* chain, void *stack, char *sharedRootPath)
{
	char* rootPath;
	int i, rootLen, ew;

	MMS_Chain = chain;
	MMS_MicroStack = stack;

	memset(MmsMediaTransferStats,0,sizeof(struct MMSMEDIATRANSFERSTAT)*20);

#ifdef SPAWN_BROWSE_THREAD
	InitBrowseLock();
#endif

//	sem_init(&UpnpIPAddressListLock, 0, 1);

	/* set up the root path settings */
	rootPath = sharedRootPath;
	rootLen = (int) strlen(rootPath);
	DIRDELIMITER = NULL;
	for (i = 0; i < rootLen; i++)
	{
		if (WIN32_DIR_DELIMITER_CHR == rootPath[i])
		{
			DIRDELIMITER = WIN32_DIR_DELIMITER_STR;
		}
		else if (UNIX_DIR_DELIMITER_CHR == rootPath[i])
		{
			DIRDELIMITER = UNIX_DIR_DELIMITER_STR;
		}
	}
	if (DIRDELIMITER == NULL)
	{
		DIRDELIMITER = UNIX_DIR_DELIMITER_STR;
	}
	ew = EndsWith(rootPath, DIRDELIMITER, 0);
	if (ew == 0)
	{
		ROOTPATH = (char*) MALLOC (rootLen+1+(int) strlen(DIRDELIMITER));
		sprintf(ROOTPATH, "%s%s", rootPath, DIRDELIMITER);
	}
	else
	{
		ROOTPATH = (char*) MALLOC (rootLen+1);
		strcpy(ROOTPATH, rootPath);
	}
	ROOTPATHLENGTH = (int) strlen(ROOTPATH);

	/* Taken from code-generated main.c - does state variable initialization */

	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UpnpSetState_MediaServer_ConnectionManager_SourceProtocolInfo(MMS_MicroStack, "http-get:*:audio/mpeg:*,http-get:*:audio/x-ms-wma:*");
	UpnpSetState_MediaServer_ConnectionManager_SinkProtocolInfo(MMS_MicroStack, "");
	UpnpSetState_MediaServer_ConnectionManager_CurrentConnectionIDs(MMS_MicroStack, "");
	UpnpSetState_MediaServer_ContentDirectory_SystemUpdateID(MMS_MicroStack, 0);

	printf("Media Server Initialized\n");

#ifdef SPAWN_BROWSE_THREAD
	DestroyBrowseLock();
#endif

}

void StopMms()
{
//	MMS_MicroStack = NULL;
//	MMS_Chain = NULL;
	FREE (ROOTPATH);
//	sem_destroy(&UpnpIPAddressListLock);
}

void UpdateIPAddresses(int *addresses, int addressesLen)
{
	sem_wait(&UpnpIPAddressListLock);
	UpnpIPAddressList = addresses;
	UpnpIPAddressListLength = addressesLen;
	sem_post(&UpnpIPAddressListLock);
}

/* END SECTION - MicroMediaServer interface implementations */
/************************************************************************************/
