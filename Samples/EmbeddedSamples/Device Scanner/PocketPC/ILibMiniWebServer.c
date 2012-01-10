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

#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#include <windows.h>
#include <math.h>
#include <winioctl.h>
#include <winbase.h>
#include <winerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <winsock.h>
#include <wininet.h>
#include <malloc.h>

#include "ILibMiniWebServer.h"
#include "ILibParsers.h"
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define DEBUGSTATEMENT(x)

struct MiniWebServerObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	
	struct ILibMWSHTTPReaderObject *Readers;
	SOCKET ListenSocket;
	int MaxConnections;
	unsigned short PortNumber;
	int Terminate;
	
	void *TimerObject;
};
struct ILibMWSHTTPReaderObject
{
	struct packetheader* PacketHeader;
	char Header[2048];
	char* Body;
	int BodySize;
	int HeaderIndex;
	int LocalIPAddress;
	
	int Body_BeginPointer;
	int Body_EndPointer;
	int Body_MallocSize;
	int Body_Read;
	
	SOCKET ClientSocket;
	int FinRead;
	struct MiniWebServerObject *Parent;
	void* user;
	void (*FunctionCallback) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user);
};
void ILibMiniWebServerProcessSocket(struct ILibMWSHTTPReaderObject *Reader)
{
	int bytesReceived;
	int i;
	struct packetheader_field_node *node;
	char* CharStar;
	
	if(Reader->BodySize==0)
	{
		/* Still Reading Headers */
		bytesReceived = recv(Reader->ClientSocket,Reader->Header+Reader->HeaderIndex,2048-Reader->HeaderIndex,0);
		if(bytesReceived==0)
		{
			if(Reader->PacketHeader!=NULL) {ILibDestructPacket(Reader->PacketHeader);}
			if(Reader->Body_MallocSize!=0) {FREE(Reader->Body);}
			Reader->Body = NULL;
			Reader->Body_MallocSize = 0;
			Reader->PacketHeader = NULL;
			closesocket(Reader->ClientSocket);
			Reader->ClientSocket = 0xFFFFFFFF;
			return;
		}
		Reader->HeaderIndex += bytesReceived;
		if(Reader->HeaderIndex>4)
		{
			/* Must have read at least 4 bytes to perform check */
			for(i=0;i<(Reader->HeaderIndex - 3);i++)
			{
				if (Reader->Header[i] == '\r' && Reader->Header[i+1] == '\n' && Reader->Header[i+2] == '\r' && Reader->Header[i+3] == '\n')
				{
					/* Finished Header */
					Reader->PacketHeader = ILibParsePacketHeader(Reader->Header,0,i+4);
					Reader->PacketHeader->ReceivingAddress = Reader->LocalIPAddress;
					Reader->BodySize = -1;
					Reader->Body_Read = 0;
					node = Reader->PacketHeader->FirstField;
					while(node!=NULL)
					{
						if(strncasecmp(node->Field,"CONTENT-LENGTH",14)==0)
						{
							CharStar = (char*)MALLOC(1+node->FieldDataLength);
							memcpy(CharStar,node->FieldData,node->FieldDataLength);
							CharStar[node->FieldDataLength] = '\0';
							Reader->BodySize = atoi(CharStar);
							FREE(CharStar);
							break;
						}
						node = node->NextField;
					}
					if(Reader->BodySize!=-1)
					{
						if(Reader->BodySize!=0)
						{
							Reader->Body = (char*)MALLOC(Reader->BodySize);
							Reader->Body_MallocSize = Reader->BodySize;
						}
						else
						{
							Reader->Body = NULL;
							Reader->Body_MallocSize = 0;
						}
					}
					else
					{
						Reader->Body = (char*)MALLOC(4096);
						Reader->Body_MallocSize = 4096;
					}
					
					if(Reader->HeaderIndex>i+4 && Reader->BodySize!=0)
					{
						/* Part of the body is in here */
						memcpy(Reader->Body,Reader->Header+i+4,Reader->HeaderIndex-(&Reader->Header[i+4]-Reader->Header));
						Reader->Body_BeginPointer = 0;
						Reader->Body_EndPointer = Reader->HeaderIndex-(int)(&Reader->Header[i+4]-Reader->Header);
						Reader->Body_Read = Reader->Body_EndPointer;
						
						if(Reader->BodySize==-1 || Reader->Body_Read>=Reader->BodySize)
						{
							DEBUGSTATEMENT(printf("Close\r\n"));
							Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,-1,Reader->user);
							
							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)
							{
								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);
								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;
								Reader->Body_BeginPointer = 0;
								Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,-1,Reader->user);
							}
							
							if(Reader->PacketHeader!=NULL) {ILibDestructPacket(Reader->PacketHeader);}
							if(Reader->Body_MallocSize!=0) {FREE(Reader->Body);}
							Reader->Body = NULL;
							Reader->Body_MallocSize = 0;
							Reader->PacketHeader = NULL;
							closesocket(Reader->ClientSocket);
							Reader->ClientSocket = 0xFFFFFFFF;
						}
						else
						{
							Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,0,Reader->user);
							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)
							{
								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);
								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;
								Reader->Body_BeginPointer = 0;
								Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user);
							}
						}
					}
					else
					{
						/* There is no body, but the packet is here */
						Reader->Body_BeginPointer = 0;
						Reader->Body_EndPointer = 0;
						
						if(Reader->BodySize<=0)
						{
							Reader->FunctionCallback(Reader,Reader->PacketHeader,NULL,&Reader->Body_BeginPointer,0,-1,Reader->user);
							if(Reader->PacketHeader!=NULL) {ILibDestructPacket(Reader->PacketHeader);}
							if(Reader->Body_MallocSize!=0) {FREE(Reader->Body);}
							Reader->Body = NULL;
							Reader->Body_MallocSize = 0;
							Reader->PacketHeader = NULL;
							closesocket(Reader->ClientSocket);
							Reader->ClientSocket = 0xFFFFFFFF;
						}
						else
						{
							Reader->FunctionCallback(Reader,Reader->PacketHeader,NULL,&Reader->Body_BeginPointer,0,0,Reader->user);
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		/* Reading Body Only */
		if(Reader->Body_BeginPointer == Reader->Body_EndPointer)
		{
			Reader->Body_BeginPointer = 0;
			Reader->Body_EndPointer = 0;
		}
		else
		{
			if(Reader->Body_BeginPointer!=0)
			{
				Reader->Body_EndPointer = Reader->Body_BeginPointer;
			}
		}
		
		
		if(Reader->Body_EndPointer == Reader->Body_MallocSize)
		{
			Reader->Body_MallocSize += 4096;
			Reader->Body = (char*)realloc(Reader->Body,Reader->Body_MallocSize);
		}
		
		bytesReceived = recv(Reader->ClientSocket,Reader->Body+Reader->Body_EndPointer,Reader->Body_MallocSize-Reader->Body_EndPointer,0);
		Reader->Body_EndPointer += bytesReceived;
		Reader->Body_Read += bytesReceived;
		
		Reader->FunctionCallback(Reader, Reader->PacketHeader, Reader->Body+Reader->Body_BeginPointer, &Reader->Body_BeginPointer, Reader->Body_EndPointer - Reader->Body_BeginPointer, 0, Reader->user);
		while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)
		{
			memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);
			Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;
			Reader->Body_BeginPointer = 0;
			Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user);				
		}
		
		if((Reader->BodySize!=-1 && Reader->Body_Read>=Reader->BodySize)||(bytesReceived==0))
		{
			if(Reader->Body_BeginPointer == Reader->Body_EndPointer)
			{
				Reader->Body_BeginPointer = 0;
				Reader->Body_EndPointer = 0;
			}
			Reader->FunctionCallback(Reader, Reader->PacketHeader, Reader->Body, &Reader->Body_BeginPointer, Reader->Body_EndPointer, -1,Reader->user);
			if(Reader->PacketHeader!=NULL) {ILibDestructPacket(Reader->PacketHeader);}
			if(Reader->Body_MallocSize!=0) {FREE(Reader->Body);}
			Reader->Body = NULL;
			Reader->Body_MallocSize = 0;
			Reader->PacketHeader = NULL;
			closesocket(Reader->ClientSocket);
			Reader->ClientSocket = 0xFFFFFFFF;
		}
		
		if(Reader->Body_BeginPointer==Reader->Body_EndPointer)
		{
			Reader->Body_BeginPointer = 0;
			Reader->Body_EndPointer = 0;
		}
	}
}
int ILibGetMiniWebServerPortNumber(void *WebServerModule)
{
	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;
	return(module->PortNumber);
}
void ILibMiniWebServerModule_Destroy(void* object)
{
	FREE(((struct MiniWebServerObject*)object)->Readers);
}
void ILibMiniWebServerModule_PreSelect(void *WebServerModule,fd_set *readset, fd_set *writeset, fd_set *errorset,int *blocktime)
{
	int i;
	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;
	int NumFree = module->MaxConnections;
	
	if(module->PortNumber==0)
	{
		module->PortNumber = ILibGetStreamSocket(htonl(INADDR_ANY),&(module->ListenSocket));
		listen(module->ListenSocket,4);
	}
	
	/* Pre Select Connected Sockets*/
	for(i=0;i<module->MaxConnections;++i)
	{
		if(module->Readers[i].ClientSocket!=0xFFFFFFFF)
		{
			/* Already Connected, just needs reading */
			FD_SET(module->Readers[i].ClientSocket,readset);
			FD_SET(module->Readers[i].ClientSocket,errorset);
			--NumFree;
		}
	}
	
	if(NumFree!=0)
	{
		/* Pre Select Listen Socket */
		FD_SET(module->ListenSocket,readset);
	}
	else
	{
		if(*blocktime>1){*blocktime=1;}
	}
}
void ILibMWS_TimerSink(void *WebServerModule)
{
	struct ILibMWSHTTPReaderObject *module = (struct ILibMWSHTTPReaderObject*)WebServerModule;
	if(module->ClientSocket!=0)
	{
		closesocket(module->ClientSocket);
		module->ClientSocket = ~0;
	}
}
void ILibMiniWebServerModule_PostSelect(void *WebServerModule, int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
{
	unsigned long flags=0;
	int i;
	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;
	struct sockaddr_in addr;
	int addrlen = sizeof(struct sockaddr_in);
	
	
	/* Select Connected Sockets*/
	for(i=0;i<module->MaxConnections;++i)
	{
		if(module->Readers[i].ClientSocket!=0xFFFFFFFF)
		{
			if(FD_ISSET(module->Readers[i].ClientSocket,errorset)!=0)
			{
				module->Readers[i].ClientSocket = 0xFFFFFFFF;
				module->Readers[i].BodySize = 0;
				//ToDo: cleanup
			}
			if(FD_ISSET(module->Readers[i].ClientSocket,readset)!=0)
			{
				ILibMiniWebServerProcessSocket(&(module->Readers[i]));
			}
			if(module->Readers[i].ClientSocket==~0 || module->Readers[i].Body!=NULL)
			{
				ILibLifeTime_Remove(module->TimerObject,&(module->Readers[i]));
			}
		}
	}
	
	/* Select Listen Socket */
	if(FD_ISSET(module->ListenSocket,readset)!=0)
	{
		for(i=0;i<module->MaxConnections;++i)
		{
			if(module->Readers[i].ClientSocket==0xFFFFFFFF)
			{
				module->Readers[i].ClientSocket = accept(module->ListenSocket,(struct sockaddr*)&addr,&addrlen);
				ioctlsocket(module->Readers[i].ClientSocket,FIONBIO,&flags);
				ILibLifeTime_Add(module->TimerObject,&(module->Readers[i]),3,&ILibMWS_TimerSink,NULL);
				module->Readers[i].HeaderIndex = 0;
				module->Readers[i].Body_BeginPointer = 0;
				module->Readers[i].Body_EndPointer = 0;
				module->Readers[i].Body_MallocSize = 0;
				module->Readers[i].Body_Read = 0;
				break;
			}
		}
	}
}
void ILibMiniWebServerCloseSession(void *ReaderModule)
{
	struct ILibMWSHTTPReaderObject *module = (struct ILibMWSHTTPReaderObject*)ReaderModule;
	SOCKET TempSocket = module->ClientSocket;
	module->ClientSocket = 0xFFFFFFFF;
	module->BodySize = 0;
	closesocket(TempSocket);
}
void ILibMiniWebServerSend(void *ReaderModule, struct packetheader *packet)
{
	struct ILibMWSHTTPReaderObject *module = (struct ILibMWSHTTPReaderObject*)ReaderModule;
	char* buffer;
	int bufferlength = ILibGetRawPacket(packet,&buffer);
	
	send(module->ClientSocket,buffer,bufferlength,0);
	
	FREE(buffer);
}
void* ILibCreateMiniWebServer(void *chain,int MaxConnections,void (*OnReceivePtr) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user),void* user)
{
	struct MiniWebServerObject *RetVal = (struct MiniWebServerObject*)MALLOC(sizeof(struct MiniWebServerObject));
	int i;
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 1, 1 );
	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}
	
	RetVal->MaxConnections = MaxConnections;
	RetVal->Readers = (struct ILibMWSHTTPReaderObject*)MALLOC(MaxConnections*sizeof(struct ILibMWSHTTPReaderObject));
	RetVal->Terminate = 0;
	RetVal->PreSelect = &ILibMiniWebServerModule_PreSelect;
	RetVal->PostSelect = &ILibMiniWebServerModule_PostSelect;
	RetVal->Destroy = &ILibMiniWebServerModule_Destroy;
	
	memset(RetVal->Readers,0,MaxConnections*sizeof(struct ILibMWSHTTPReaderObject));
	for(i=0;i<MaxConnections;++i)
	{
		RetVal->Readers[i].ClientSocket = ~0;
		RetVal->Readers[i].FunctionCallback = OnReceivePtr;
		RetVal->Readers[i].Parent = RetVal;
		RetVal->Readers[i].user = user;
	}
	
	RetVal->PortNumber = 0;
	
	RetVal->TimerObject = ILibCreateLifeTime(chain);
	ILibAddToChain(chain,RetVal);
	return((void*)RetVal);
}
