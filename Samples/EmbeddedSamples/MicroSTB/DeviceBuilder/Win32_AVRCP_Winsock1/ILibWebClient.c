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

#ifdef _WIN32_WCE
	#define _CRTDBG_MAP_ALLOC
	#include <math.h>
	#include <winerror.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <stddef.h>
	#include <string.h>
	#include <winsock.h>
	#include <wininet.h>
	#include <windows.h>
	#include <winioctl.h>
	#include <winbase.h>
#elif WIN32
	#define _CRTDBG_MAP_ALLOC
	#include <math.h>
	#include <winerror.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <stddef.h>
	#include <string.h>
	#ifdef WINSOCK2
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#else
		#include <winsock.h>
		#include <wininet.h>
	#endif
	#include <windows.h>
	#include <winioctl.h>
	#include <winbase.h>
	#include <crtdbg.h>
#elif _POSIX
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <string.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <sys/utsname.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
#endif

#ifdef WIN32
	#define sem_t HANDLE
	#define sem_init(p_semaphore,y,initialValue) *p_semaphore=CreateSemaphore(NULL,initialValue,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#elif _WIN32_WCE
	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#else
	#include <semaphore.h>
#endif

#include <memory.h>
#include <math.h>
#include "ILibParsers.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"






#define MAX_IDLE_SESSIONS 20
#define HTTP_SESSION_IDLE_TIMEOUT 3
#define HTTP_CONNECT_RETRY_COUNT 4
#define INITIAL_BUFFER_SIZE 2048







#define PIPELINE_UNKNOWN 0
#define PIPELINE_YES 1
#define PIPELINE_NO 2

#define STARTCHUNK 0
#define ENDCHUNK 1
#define DATACHUNK 2
#define FOOTERCHUNK 3


struct ILibWebRequest
{
	char **Buffer;
	int *BufferLength;
	int *UserFree;
	int NumberOfBuffers;

	struct sockaddr_in remote;
	void *user1,*user2;
	void (*OnResponse)(
				void *WebReaderToken,
				int InterruptFlag,
				struct packetheader *header,
				char *bodyBuffer,
				int *beginPointer,
				int endPointer,
				int done,
				void *user1,
				void *user2,
				int *PAUSE);

};

struct ILibWebClientManager
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	void **socks;
	int socksLength;

	void *DataTable;
	void *idleTable;
	void *backlogQueue;

	void *timer;
	int idleCount;

	void *Chain;
	sem_t QLock;
};

struct ILibWebClient_ChunkData
{
	int Flag;
	char *buffer;
	int offset;
	int mallocSize;

	int bytesLeft;
};
struct ILibWebClientDataObject
{
	int PipelineFlag;
	int ActivityCounter;
	struct sockaddr_in remote;
	struct ILibWebClientManager *Parent;

	int FinHeader;
	int Chunked;
	int BytesLeft;
	int WaitForClose;
	int Closing;
	int Server;

	int HeaderLength;

	int ExponentialBackoff;

	struct ILibWebClient_ChunkData *chunk;
	struct packetheader *header;
	int InitialRequestAnswered;
	void* RequestQueue;
	void *SOCK;
	int LocalIP;
	int PAUSE;
};


void ILibWebClient_DestroyWebClientDataObject(void *token)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)token;
	struct ILibWebRequest *wr;
	int i;
	int zero=0;

	if(wcdo->Closing<0) 
	{
		return;
	}
	if(wcdo->SOCK!=NULL && ILibAsyncSocket_IsFree(wcdo->SOCK)==0)
	{
		wcdo->Closing = -1;
		ILibAsyncSocket_Disconnect(wcdo->SOCK);
	}
	if(wcdo->header!=NULL)
	{
		ILibDestructPacket(wcdo->header);
	}
	if(wcdo->chunk!=NULL)
	{
		if(wcdo->chunk->buffer!=NULL) {FREE(wcdo->chunk->buffer);}
		FREE(wcdo->chunk);
	}
	wr = ILibQueue_DeQueue(wcdo->RequestQueue);
	while(wr!=NULL)
	{
		for(i=0;i<wr->NumberOfBuffers;++i)
		{
			if(wr->UserFree[i]==0) {FREE(wr->Buffer[i]);}
		}
		FREE(wr->Buffer);
		FREE(wr->BufferLength);
		FREE(wr->UserFree);
		if(wcdo->Server==0 && wr->OnResponse!=NULL)
		{			
			wr->OnResponse(
					NULL,
					WEBCLIENT_DESTROYED,
					NULL,
					NULL,
					NULL,
					0,
					-1,
					wr->user1,
					wr->user2,
					&zero);		
		}
		FREE(wr);
		wr = ILibQueue_DeQueue(wcdo->RequestQueue);
	}
	ILibQueue_Destroy(wcdo->RequestQueue);
	FREE(wcdo);
}


void ILibDestroyWebClient(void *object)
{
	struct ILibWebClientManager *manager = (struct ILibWebClientManager*)object;
	void *en;
	void *wcdo;
	char *key;
	int keyLength,i;
	struct ILibWebRequest *wr;
	int zero=0;

	en = ILibHashTree_GetEnumerator(manager->DataTable);
	while(ILibHashTree_MoveNext(en)==0)
	{
		ILibHashTree_GetValue(en,&key,&keyLength,&wcdo);
		ILibWebClient_DestroyWebClientDataObject(wcdo);
	}
	ILibHashTree_DestroyEnumerator(en);
	
	wr = ILibQueue_DeQueue(manager->backlogQueue);
	while(wr!=NULL)
	{
		if(wr->OnResponse!=NULL)
		{
			wr->OnResponse(
				NULL,
				WEBCLIENT_DESTROYED,
				NULL,
				NULL,
				NULL,
				0,
				-1,
				wr->user1,
				wr->user2,
				&zero);			
		}
		for(i=0;i<wr->NumberOfBuffers;++i)
		{
			if(wr->UserFree[i]==0) {FREE(wr->Buffer[i]);}
			FREE(wr->Buffer);
			FREE(wr->BufferLength);
			FREE(wr->UserFree);
		}
		wr = ILibQueue_DeQueue(manager->backlogQueue);
	}
	ILibQueue_Destroy(manager->backlogQueue);
	ILibDestroyHashTree(manager->idleTable);
	ILibDestroyHashTree(manager->DataTable);
	sem_destroy(&(manager->QLock));
	FREE(manager->socks);
}

void ILibWebClient_TimerInterruptSink(void *object)
{
}
void ILibWebClient_TimerSink(void *object)
{
	void *enumerator;
	char IPV4Address[22];
	int IPV4AddressLength;
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)object;

	char *key;
	int keyLength;
	void *data;

	void *DisconnectSocket = NULL;

	sem_wait(&(wcdo->Parent->QLock));
	if(ILibQueue_IsEmpty(wcdo->RequestQueue)!=0)
	{
		// Still Idle
		if(wcdo->SOCK!=NULL && ILibAsyncSocket_IsFree(wcdo->SOCK)==0)
		{
			wcdo->Closing = 1;
			DisconnectSocket = wcdo->SOCK;
		}
		if(wcdo->Parent->idleCount>MAX_IDLE_SESSIONS)
		{
			--wcdo->Parent->idleCount;
			enumerator = ILibHashTree_GetEnumerator(wcdo->Parent->idleTable);
			ILibHashTree_MoveNext(enumerator);
			ILibHashTree_GetValue(enumerator,&key,&keyLength,&data);
			ILibHashTree_DestroyEnumerator(enumerator);
			ILibDeleteEntry(wcdo->Parent->idleTable,key,keyLength);
			ILibDeleteEntry(wcdo->Parent->DataTable,key,keyLength);
			ILibWebClient_DestroyWebClientDataObject(wcdo);
		}
		IPV4AddressLength = sprintf(IPV4Address,"%s:%d",
			inet_ntoa(wcdo->remote.sin_addr),
			ntohs(wcdo->remote.sin_port));
		ILibAddEntry(wcdo->Parent->idleTable,IPV4Address,IPV4AddressLength,wcdo);
		++wcdo->Parent->idleCount;
		wcdo->SOCK = NULL;
	}
	sem_post(&(wcdo->Parent->QLock));
	if(DisconnectSocket!=NULL)
	{
		ILibAsyncSocket_Disconnect(DisconnectSocket);
	}

}
void ILibWebClient_FinishedResponse_Server(void *_wcdo)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)_wcdo;
	if(wcdo->chunk!=NULL)
	{
		FREE(wcdo->chunk->buffer);
		FREE(wcdo->chunk);
		wcdo->chunk = NULL;
	}

	if(wcdo->header!=NULL)
	{
		ILibDestructPacket(wcdo->header);
	}
	wcdo->Chunked = 0;
	wcdo->header = NULL;
	wcdo->FinHeader = 0;
	wcdo->WaitForClose = 0;
	wcdo->InitialRequestAnswered = 1;
}
void ILibWebClient_FinishedResponse(void *socketModule, struct ILibWebClientDataObject *wcdo)
{
	struct ILibWebRequest *wr;
	int i;

	if(ILibAsyncSocket_IsFree(socketModule)!=0 || wcdo->Server!=0) {return;}
	if(wcdo->chunk!=NULL)
	{
		FREE(wcdo->chunk->buffer);
		FREE(wcdo->chunk);
		wcdo->chunk = NULL;
	}

	if(wcdo->header!=NULL)
	{
		ILibDestructPacket(wcdo->header);
	}
	wcdo->header = NULL;
	wcdo->FinHeader = 0;
	wcdo->WaitForClose = 0;
	wcdo->Chunked = 0;
	wcdo->InitialRequestAnswered = 1;

	sem_wait(&(wcdo->Parent->QLock));
	wr = ILibQueue_DeQueue(wcdo->RequestQueue);
	for(i=0;i<wr->NumberOfBuffers;++i)
	{
		if(wr->UserFree[i]==0) {FREE(wr->Buffer[i]);}
	}
	FREE(wr->Buffer);
	FREE(wr->BufferLength);
	FREE(wr->UserFree);
	FREE(wr);

	wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
	if(wr!=NULL)
	{
		// Send Another Request
		if(wcdo->PipelineFlag!=PIPELINE_NO)
		{
			wcdo->PipelineFlag = PIPELINE_YES;
			for(i=0;i<wr->NumberOfBuffers;++i)
			{
				ILibAsyncSocket_Send(wcdo->SOCK,wr->Buffer[i],wr->BufferLength[i],-1);
			}
		}
	}
	else
	{
		// Queue Is Empty
		ILibLifeTime_Add(wcdo->Parent->timer,wcdo,HTTP_SESSION_IDLE_TIMEOUT,&ILibWebClient_TimerSink,&ILibWebClient_TimerInterruptSink);		
	}
	sem_post(&(wcdo->Parent->QLock));
}

void ILibWebClient_ProcessChunk(struct ILibWebClientDataObject *wcdo, char *buffer, int *p_beginPointer, int endPointer)
{
	char *hex;
	int i;
	struct parser_result *pr;
	struct ILibWebRequest *wr;
	int bp;


	sem_wait(&(wcdo->Parent->QLock));
	wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
	sem_post(&(wcdo->Parent->QLock));

	if(wcdo->chunk==NULL)
	{
		wcdo->chunk = (struct ILibWebClient_ChunkData*)MALLOC(sizeof(struct ILibWebClient_ChunkData));
		memset(wcdo->chunk,0,sizeof(struct ILibWebClient_ChunkData));

		wcdo->chunk->buffer = (char*)MALLOC(INITIAL_BUFFER_SIZE);
		wcdo->chunk->mallocSize = INITIAL_BUFFER_SIZE;
	}

	switch(wcdo->chunk->Flag)
	{
		case STARTCHUNK:
			// Reading Chunk Header
			if(endPointer<3){return;}
			for(i=2;i<endPointer;++i)
			{
				if(buffer[i-2]=='\r' && buffer[i-1]=='\n')
				{
					pr = ILibParseString(buffer,0,i-2,";",1);
					pr->FirstResult->data[pr->FirstResult->datalength] = '\0';
					wcdo->chunk->bytesLeft  = (int)strtol(pr->FirstResult->data,&hex,16);
					*p_beginPointer = i;
					wcdo->chunk->Flag=wcdo->chunk->bytesLeft==0?FOOTERCHUNK:DATACHUNK;
					ILibDestructParserResults(pr);
					break;
				}
			}
			break;
		case ENDCHUNK:
			if(endPointer>=2)
			{
				*p_beginPointer = 2;
				wcdo->chunk->Flag = STARTCHUNK;
			}
			break;
		case DATACHUNK:
			if(endPointer>=wcdo->chunk->bytesLeft)
			{
				wcdo->chunk->Flag = ENDCHUNK;
				i = wcdo->chunk->bytesLeft;
			}
			else
			{
				i=endPointer;
			}

			if(wcdo->chunk->offset+endPointer>wcdo->chunk->mallocSize)
			{
				wcdo->chunk->buffer = (char*)realloc(wcdo->chunk->buffer,wcdo->chunk->mallocSize+INITIAL_BUFFER_SIZE);
			}
			memcpy(wcdo->chunk->buffer+wcdo->chunk->offset,buffer,i);
			wcdo->chunk->bytesLeft-=i;
			wcdo->chunk->offset+=i;

			bp = 0;
			if(wr->OnResponse!=NULL)
			{
				wr->OnResponse(
					wcdo->SOCK,
					0,
					wcdo->header,
					wcdo->chunk->buffer,
					&bp,
					wcdo->chunk->offset,
					0,
					wr->user1,
					wr->user2,
					&(wcdo->PAUSE));			
			}
			if(bp==i)
			{
				wcdo->chunk->offset = 0;
			}
			else if(bp!=0)
			{
				memcpy(wcdo->chunk->buffer+bp,wcdo->chunk->buffer,i-bp);
				wcdo->chunk->offset -= bp;
			}
			*p_beginPointer = i;
			break;
		case FOOTERCHUNK:
			if(endPointer>=2)
			{
				for(i=2;i<=endPointer;++i)
				{
					if(buffer[i-2]=='\r' && buffer[i-1]=='\n')
					{
						if(i==2)
						{
							// FINISHED
							if(wr->OnResponse!=NULL)
							{
								wr->OnResponse(
									wcdo->SOCK,
									0,
									wcdo->header,
									wcdo->chunk->buffer,
									p_beginPointer,
									wcdo->chunk->offset,
									-1,
									wr->user1,
									wr->user2,
									&(wcdo->PAUSE));			
							}
							if(wcdo->chunk->buffer!=NULL) {FREE(wcdo->chunk->buffer);}
							FREE(wcdo->chunk);
							wcdo->chunk = NULL;
							ILibWebClient_FinishedResponse(wcdo->SOCK,wcdo);
							*p_beginPointer = 2;
						}
						else
						{
							// Add Headers
						}
					}
				}
			}
			break;
	}
}

void ILibWebClient_OnData(void* socketModule,char* buffer,int *p_beginPointer, int endPointer,void (**InterruptPtr)(void *socketModule, void *user), void **user, int *PAUSE)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)(*user);
	struct ILibWebRequest *wr;
	struct packetheader *tph;
	struct packetheader_field_node *phfn;
	int i=0;
	int zero = 0;
	int Fini;

	if(wcdo->Server==0)
	{
		sem_wait(&(wcdo->Parent->QLock));
	}
	wr = (struct ILibWebRequest*)ILibQueue_PeekQueue(wcdo->RequestQueue);
	if(wcdo->Server==0)
	{
		sem_post(&(wcdo->Parent->QLock));
	}
	if(wr==NULL)
	{
		*p_beginPointer = endPointer;
		return;
	}
	if(wcdo->FinHeader==0)
	{
		//Still Reading Headers
		if(endPointer - (*p_beginPointer)>=4)
		{
			while(i <= (endPointer - (*p_beginPointer))-4)
			{
				if(buffer[*p_beginPointer+i]=='\r' &&
					buffer[*p_beginPointer+i+1]=='\n' &&
					buffer[*p_beginPointer+i+2]=='\r' &&
					buffer[*p_beginPointer+i+3]=='\n')
				{
					wcdo->HeaderLength = i+3;
					wcdo->WaitForClose=1;
					wcdo->BytesLeft=-1;
					wcdo->FinHeader=1;
					wcdo->header = ILibParsePacketHeader(buffer,*p_beginPointer,endPointer-(*p_beginPointer));
					if(wcdo->header!=NULL)
					{
						wcdo->header->ReceivingAddress = wcdo->LocalIP;
						//Introspect Request
						phfn = wcdo->header->FirstField;
						while(phfn!=NULL)
						{
							if(phfn->FieldLength==17 && strncasecmp(phfn->Field,"transfer-encoding",17)==0)
							{
								if(phfn->FieldDataLength==7 && strncasecmp(phfn->FieldData,"chunked",7)==0)
								{
									wcdo->WaitForClose=0;
									wcdo->Chunked = 1;
								}
							}
							if(phfn->FieldLength==14 && strncasecmp(phfn->Field,"content-length",14)==0)
							{
								wcdo->WaitForClose=0;
								phfn->FieldData[phfn->FieldDataLength] = '\0';
								wcdo->BytesLeft = atoi(phfn->FieldData);
							}
							phfn = phfn->NextField;
						}
						if(wcdo->Server!=0 && wcdo->BytesLeft==-1)
						{
							wcdo->BytesLeft=0;	// Request with no body
						}
						if(wcdo->BytesLeft==0)
						{
							// Complete Response
							if(wr->OnResponse!=NULL)
							{
								wr->OnResponse(
									socketModule,
									0,
									wcdo->header,
									NULL,
									&zero,
									0,
									-1,
									wr->user1,
									wr->user2,
									&(wcdo->PAUSE));
							}
							*p_beginPointer = *p_beginPointer + i + 4;
							ILibWebClient_FinishedResponse(socketModule,wcdo);
						}
						else
						{
							//Check to see if any of the body arrived
							if(wcdo->Chunked==0)
							{
								// Normal Encoding
								if(wcdo->BytesLeft!=-1 && (endPointer-(*p_beginPointer)) - (i+4) >= wcdo->BytesLeft)
								{
									// Read The Entire Packet
									if(wr->OnResponse!=NULL)
									{
										wr->OnResponse(
											socketModule,
											0,
											wcdo->header,
											buffer+i+4,
											&zero,
											wcdo->BytesLeft,
											-1,
											wr->user1,
											wr->user2,
											&(wcdo->PAUSE));
									}
									*p_beginPointer = *p_beginPointer + i + 4 + (zero==0?wcdo->BytesLeft:zero);
									ILibWebClient_FinishedResponse(socketModule,wcdo);
								}
								else
								{
									if(wr->OnResponse!=NULL)
									{
										wr->OnResponse(
											socketModule,
											0,
											wcdo->header,
											buffer+i+4,
											&zero,
											(endPointer - (*p_beginPointer) - (i+4)),
											0,
											wr->user1,
											wr->user2,
											&(wcdo->PAUSE));
									}
									wcdo->HeaderLength = 0;
									*p_beginPointer = i+4+zero;
									wcdo->BytesLeft -= zero;
									tph = ILibClonePacket(wcdo->header);
									ILibDestructPacket(wcdo->header);
									wcdo->header = tph;
								}
							}
							else
							{
								// Chunked
								ILibWebClient_ProcessChunk(wcdo,buffer+i+4,&zero,(endPointer - (*p_beginPointer) - (i+4)));
								*p_beginPointer = i+4+zero;
								tph = ILibClonePacket(wcdo->header);
								ILibDestructPacket(wcdo->header);
								wcdo->header = tph;
							}
						}
					}
					else
					{
						//ERROR
					}
					break;
				}

				++i;
			}
		}
	}
	else
	{
		//Just Process the Body
		if(wcdo->Chunked==0)
		{
			Fini = ((endPointer - (*p_beginPointer))>=wcdo->BytesLeft)?-1:0;
			zero = *p_beginPointer;

			// Normal
			if(wr->OnResponse!=NULL)
			{
				wr->OnResponse(
					socketModule,
					0,
					wcdo->header,
					buffer,
					&zero,
					((endPointer - (*p_beginPointer))>=wcdo->BytesLeft)?wcdo->BytesLeft:(endPointer - (*p_beginPointer)),
					Fini,
					wr->user1,
					wr->user2,
					&(wcdo->PAUSE));
			}
			if(ILibAsyncSocket_IsFree(socketModule)==0)
			{
				wcdo->BytesLeft -= *p_beginPointer;
				if(Fini!=0)
				{
					*p_beginPointer += wcdo->HeaderLength;
					*p_beginPointer = *p_beginPointer + wcdo->BytesLeft;
				}
				else
				{
					wcdo->BytesLeft -= zero;
				}
				if(Fini!=0)
				{
					ILibWebClient_FinishedResponse(socketModule,wcdo);
				}
			}
		}
		else
		{
			// Chunked
			ILibWebClient_ProcessChunk(wcdo,buffer,p_beginPointer,endPointer);
		}
	}
	if(ILibAsyncSocket_IsFree(socketModule)==0)
	{
		*PAUSE = wcdo->PAUSE;
	}
}
void ILibWebClient_RetrySink(void *object)
{
	char key[22];
	int keyLength;

	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)object;
	struct ILibWebClientManager *wcm = wcdo->Parent;
	wcdo->ExponentialBackoff = wcdo->ExponentialBackoff==0?1:wcdo->ExponentialBackoff * 2;
	sem_wait(&(wcm->QLock));
	if(wcdo->ExponentialBackoff==(int)pow((double)2,(double)HTTP_CONNECT_RETRY_COUNT))
	{
		// Retried enough times, give up
		keyLength = sprintf(key,"%s:%d",inet_ntoa(wcdo->remote.sin_addr),(int)ntohs(wcdo->remote.sin_port));
		ILibDeleteEntry(wcdo->Parent->DataTable,key,keyLength);
		ILibDeleteEntry(wcdo->Parent->idleTable,key,keyLength);
		ILibWebClient_DestroyWebClientDataObject(wcdo);
	}
	else
	{
		ILibQueue_EnQueue(wcdo->Parent->backlogQueue,wcdo);
	}
	sem_post(&(wcm->QLock));
}
void ILibWebClient_OnConnect(void* socketModule, int Connected, void *user)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	struct ILibWebRequest *r;
	int i;

	wcdo->SOCK = socketModule;
	wcdo->InitialRequestAnswered=0;

	if(Connected!=0)
	{
		//Success: Send First Request
		wcdo->LocalIP = ILibAsyncSocket_GetLocalInterface(socketModule);
		wcdo->ExponentialBackoff=1;
		sem_wait(&(wcdo->Parent->QLock));
		r = ILibQueue_PeekQueue(wcdo->RequestQueue);
		sem_post(&(wcdo->Parent->QLock));
		for(i=0;i<r->NumberOfBuffers;++i)
		{
			ILibAsyncSocket_Send(socketModule,r->Buffer[i],r->BufferLength[i],-1);
		}
	}
	else
	{
		//ToDo: Exponential Backoff / ReTry
		wcdo->Closing=2; //This is required, so we don't notify the user yet
		ILibAsyncSocket_Disconnect(socketModule);
		wcdo->Closing=0;
		wcdo->PipelineFlag = PIPELINE_UNKNOWN;
		
		ILibLifeTime_Add(wcdo->Parent->timer,wcdo,wcdo->ExponentialBackoff,&ILibWebClient_RetrySink,NULL);
	}
}
void ILibWebClient_OnDisconnect(void* socketModule, void *user)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	struct ILibWebRequest *wr;

	char *buffer;
	int BeginPointer,EndPointer;

	if(wcdo->Closing==0) 
	{
		//ToDo: This should only be set if there are pending requests
		wcdo->PipelineFlag = PIPELINE_NO;
	}
	if(wcdo->WaitForClose!=0)
	{
		// Finish this session
		ILibAsyncSocket_GetBuffer(socketModule,&buffer,&BeginPointer,&EndPointer);
		sem_wait(&(wcdo->Parent->QLock));
		wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
		sem_post(&(wcdo->Parent->QLock));
		if(wr->OnResponse!=NULL)
		{				
			wr->OnResponse(
				socketModule,
				0,
				wcdo->header,
				buffer,
				&BeginPointer,
				EndPointer,
				-1,
				wr->user1,
				wr->user2,
				&(wcdo->PAUSE));
			ILibWebClient_FinishedResponse(socketModule,wcdo);		
		}
	}
	
	if(wcdo->Closing!=0){return;}

	sem_wait(&(wcdo->Parent->QLock));
	wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
	sem_post(&(wcdo->Parent->QLock));

	if(wr!=NULL)
	{
		// Still Requests to be made
		if(wcdo->InitialRequestAnswered==0)
		{
			//Error
			wr->OnResponse(
				socketModule,
				0,
				NULL,
				NULL,
				NULL,
				0,
				-1,
				wr->user1,
				wr->user2,
				&(wcdo->PAUSE));
			ILibWebClient_FinishedResponse(socketModule,wcdo);	

			sem_wait(&(wcdo->Parent->QLock));
			wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
			sem_post(&(wcdo->Parent->QLock));
			if(wr==NULL){return;}
		}

		// Make Another Connection and Continue
		wcdo->Closing = 0;
		ILibAsyncSocket_ConnectTo(
			socketModule,
			INADDR_ANY,
			wr->remote.sin_addr.s_addr,
			(int)ntohs(wr->remote.sin_port),
			NULL,
			wcdo);
	}
}
void ILibWebClient_OnSendOK(void *socketModule, void *user)
{
}

//void ILibWebClient_PreProcess(void* WebClientModule,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
void ILibWebClient_PreProcess(void* WebClientModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClientModule;
	struct ILibWebClientDataObject *wcdo;
	int i;
	int OK=0;

	sem_wait(&(wcm->QLock));
	while(OK==0 && ILibQueue_IsEmpty(wcm->backlogQueue)==0)
	{
		OK=1;
		for(i=0;i<wcm->socksLength;++i)
		{
			if(ILibAsyncSocket_IsFree(wcm->socks[i])!=0)
			{
				OK=0;
				wcdo = ILibQueue_DeQueue(wcm->backlogQueue);
				wcdo->Closing = 0;
				ILibAsyncSocket_ConnectTo(
					wcm->socks[i], 
					INADDR_ANY, 
					wcdo->remote.sin_addr.s_addr, 
					(int)ntohs(wcdo->remote.sin_port),
					NULL,
					wcdo);
			}
			if(ILibQueue_IsEmpty(wcm->backlogQueue)!=0) {break;}
		}
	}
	sem_post(&(wcm->QLock));
}

void *ILibCreateWebClientEx(void (*OnResponse)(
								void *WebReaderToken,
								int InterruptFlag,
								struct packetheader *header,
								char *bodyBuffer,
								int *beginPointer,
								int endPointer,
								int done,
								void *user1,
								void *user2,
								int *PAUSE), void *socketModule, void *user1, void *user2)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)MALLOC(sizeof(struct ILibWebClientDataObject));
	struct ILibWebRequest *wr;
	
	memset(wcdo,0,sizeof(struct ILibWebClientDataObject));
	wcdo->Parent = NULL;
	wcdo->RequestQueue = ILibQueue_Create();
	wcdo->Server = 1;
	wcdo->SOCK = socketModule;

	wr = (struct ILibWebRequest*)MALLOC(sizeof(struct ILibWebRequest));
	memset(wr,0,sizeof(struct ILibWebRequest));
	wr->OnResponse = OnResponse;
	ILibQueue_EnQueue(wcdo->RequestQueue,wr);
	wr->user1 = user1;
	wr->user2 = user2;
	return(wcdo);
}
void *ILibCreateWebClient(int PoolSize,void *Chain)
{
	int i;
	struct ILibWebClientManager *RetVal = (struct ILibWebClientManager*)MALLOC(sizeof(struct ILibWebClientManager));
	
	memset(RetVal,0,sizeof(struct ILibWebClientManager));
	
	RetVal->Destroy = &ILibDestroyWebClient;
	RetVal->PreSelect = &ILibWebClient_PreProcess;
	//RetVal->PostSelect = &ILibWebClient_PreProcess;

	RetVal->socksLength = PoolSize;
	RetVal->socks = (void**)MALLOC(PoolSize*sizeof(void*));
	sem_init(&(RetVal->QLock),0,1);
	RetVal->Chain = Chain;

	RetVal->backlogQueue = ILibQueue_Create();
	RetVal->DataTable = ILibInitHashTree();
	RetVal->idleTable = ILibInitHashTree();

	RetVal->timer = ILibCreateLifeTime(Chain);
	ILibAddToChain(Chain,RetVal);
	for(i=0;i<PoolSize;++i)
	{
		RetVal->socks[i] = ILibCreateAsyncSocketModule(
			Chain,
			INITIAL_BUFFER_SIZE,
			&ILibWebClient_OnData,
			&ILibWebClient_OnConnect,
			&ILibWebClient_OnDisconnect,
			&ILibWebClient_OnSendOK);
	}
	return((void*)RetVal);
}

void ILibWebClient_PipelineRequest(
								void *WebClient, 
								struct sockaddr_in *RemoteEndpoint, 
								struct packetheader *packet,
								void (*OnResponse)(
													void *WebReaderToken,
													int InterruptFlag,
													struct packetheader *header,
													char *bodyBuffer,
													int *beginPointer,
													int endPointer,
													int done,
													void *user1,
													void *user2,
													int *PAUSE),
								void *user1,
								void *user2)
{
	int ForceUnBlock=0;
	char IPV4Address[22];
	int IPV4AddressLength;
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClient;
	struct ILibWebClientDataObject *wcdo;
	struct ILibWebRequest *request = (struct ILibWebRequest*)MALLOC(sizeof(struct ILibWebRequest));
	int i;

	request->NumberOfBuffers = 1;
	request->Buffer = (char**)MALLOC(1*sizeof(char*));
	request->BufferLength = (int*)MALLOC(1*sizeof(int));
	request->UserFree = (int*)MALLOC(1*sizeof(int));

	request->BufferLength[0] = ILibGetRawPacket(packet,&(request->Buffer[0]));
	request->UserFree[0] = 0;
	request->OnResponse = OnResponse;
	request->remote.sin_port = RemoteEndpoint->sin_port;
	request->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;

	request->user1 = user1;
	request->user2 = user2;

	IPV4AddressLength = sprintf(IPV4Address,"%s:%d",
		inet_ntoa(RemoteEndpoint->sin_addr),
		ntohs(RemoteEndpoint->sin_port));

	sem_wait(&(wcm->QLock));
	if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)!=0)
	{
		// Entry Found
		wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
		if(ILibQueue_IsEmpty(wcdo->RequestQueue)!=0)
		{
			// Take out of Idle State
			--wcm->idleCount;
			ILibDeleteEntry(wcm->idleTable,IPV4Address,IPV4AddressLength);
			ILibLifeTime_Remove(wcm->timer,wcdo);
			if(wcdo->SOCK==NULL || ILibAsyncSocket_IsFree(wcdo->SOCK)!=0)
			{
				ILibQueue_EnQueue(wcm->backlogQueue,wcdo);	
				ForceUnBlock=1;
			}
			else
			{
				for(i=0;i<request->NumberOfBuffers;++i)
				{
					ILibAsyncSocket_Send(wcdo->SOCK,request->Buffer[i],request->BufferLength[i],1);
				}
			}
		}
		ILibQueue_EnQueue(wcdo->RequestQueue,request);
	}
	else
	{
		// Need to queue Entry
		wcdo = (struct ILibWebClientDataObject*)MALLOC(sizeof(struct ILibWebClientDataObject));
		memset(wcdo,0,sizeof(struct ILibWebClientDataObject));
		wcdo->Parent = wcm;
		wcdo->RequestQueue = ILibQueue_Create();
		wcdo->remote.sin_port = RemoteEndpoint->sin_port;
		wcdo->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;

		ILibQueue_EnQueue(wcdo->RequestQueue,request);
		ILibAddEntry(wcm->DataTable,IPV4Address,IPV4AddressLength,wcdo);
		ILibQueue_EnQueue(wcm->backlogQueue,wcdo);		
		ForceUnBlock=1;
	}
	sem_post(&(wcm->QLock));
	if(ForceUnBlock!=0)
	{
		ILibForceUnBlockChain(wcm->Chain);
	}
	ILibDestructPacket(packet);
}
void ILibWebClient_PipelineRequestEx(
	void *WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	char *headerBuffer,
	int headerBufferLength,
	int headerBuffer_FREE,
	char *bodyBuffer,
	int bodyBufferLength,
	int bodyBuffer_FREE,
	void (*OnResponse)(
		void *WebReaderToken,
		int InterruptFlag,
		struct packetheader *header,
		char *bodyBuffer,
		int *beginPointer,
		int endPointer,
		int done,
		void *user1,
		void *user2,
		int *PAUSE),
	void *user1,
	void *user2)
{
	int ForceUnBlock=0;
	char IPV4Address[22];
	int IPV4AddressLength;
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClient;
	struct ILibWebClientDataObject *wcdo;
	struct ILibWebRequest *request = (struct ILibWebRequest*)MALLOC(sizeof(struct ILibWebRequest));
	int i;

	request->NumberOfBuffers = bodyBuffer!=NULL?2:1;
	request->Buffer = (char**)MALLOC(request->NumberOfBuffers*sizeof(char*));
	request->BufferLength = (int*)MALLOC(request->NumberOfBuffers*sizeof(int));
	request->UserFree = (int*)MALLOC(request->NumberOfBuffers*sizeof(int));

	request->Buffer[0] = headerBuffer;
	request->BufferLength[0] = headerBufferLength;
	request->UserFree[0] = headerBuffer_FREE;

	if(bodyBuffer!=NULL)
	{
		request->Buffer[1] = bodyBuffer;
		request->BufferLength[1] = bodyBufferLength;
		request->UserFree[1] = bodyBuffer_FREE;
	}

	request->OnResponse = OnResponse;
	request->remote.sin_port = RemoteEndpoint->sin_port;
	request->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;

	request->user1 = user1;
	request->user2 = user2;

	IPV4AddressLength = sprintf(IPV4Address,"%s:%d",
		inet_ntoa(RemoteEndpoint->sin_addr),
		ntohs(RemoteEndpoint->sin_port));

	sem_wait(&(wcm->QLock));
	if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)!=0)
	{
		// Entry Found
		wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
		if(ILibQueue_IsEmpty(wcdo->RequestQueue)!=0)
		{
			// Take out of Idle State
			--wcm->idleCount;
			ILibDeleteEntry(wcm->idleTable,IPV4Address,IPV4AddressLength);
			ILibLifeTime_Remove(wcm->timer,wcdo);
			if(wcdo->SOCK==NULL)
			{
				ILibQueue_EnQueue(wcm->backlogQueue,wcdo);	
				ForceUnBlock=1;
			}
			else
			{
				for(i=0;i<request->NumberOfBuffers;++i)
				{
					ILibAsyncSocket_Send(wcdo->SOCK,request->Buffer[i],request->BufferLength[i],1);
				}
			}
		}
		ILibQueue_EnQueue(wcdo->RequestQueue,request);
	}
	else
	{
		// Need to queue Entry
		wcdo = (struct ILibWebClientDataObject*)MALLOC(sizeof(struct ILibWebClientDataObject));
		memset(wcdo,0,sizeof(struct ILibWebClientDataObject));
		wcdo->Parent = wcm;
		wcdo->RequestQueue = ILibQueue_Create();
		wcdo->remote.sin_port = RemoteEndpoint->sin_port;
		wcdo->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;

		ILibQueue_EnQueue(wcdo->RequestQueue,request);
		ILibAddEntry(wcm->DataTable,IPV4Address,IPV4AddressLength,wcdo);
		ILibQueue_EnQueue(wcm->backlogQueue,wcdo);		
		ForceUnBlock=1;
	}
	sem_post(&(wcm->QLock));
	if(ForceUnBlock!=0)
	{
		ILibForceUnBlockChain(wcm->Chain);
	}
}
struct packetheader *ILibWebClient_GetHeaderFromDataObject(void *token)
{
	return(((struct ILibWebClientDataObject*)token)->header);
}
void ILibWebClient_DeleteRequests(void *WebClientToken,char *IP,int Port)
{
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClientToken;
	char IPV4Address[25];
	struct ILibWebClientDataObject *wcdo;
	int IPV4AddressLength;
	struct ILibWebRequest *wr;
	int i;
	int zero = 0;

	void *RemoveQ = ILibQueue_Create();

	IPV4AddressLength = sprintf(IPV4Address,"%s:%d",IP,Port);

	sem_wait(&(wcm->QLock));
	if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)!=0)
	{
		// Entry Found
		wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
		while(ILibQueue_IsEmpty(wcdo->RequestQueue)==0)
		{
			wr = (struct ILibWebRequest*)ILibQueue_DeQueue(wcdo->RequestQueue);
			ILibQueue_EnQueue(RemoveQ,wr);
		}
	}
	sem_post(&(wcm->QLock));
	while(ILibQueue_IsEmpty(RemoveQ)==0)
	{
		wr = (struct ILibWebRequest*)ILibQueue_DeQueue(RemoveQ);
					if(wr->OnResponse!=NULL)
		wr->OnResponse(				
			WebClientToken,
			WEBCLIENT_DELETED,
			NULL,
			NULL,
			NULL,
			0,
			-1,
			wr->user1,
			wr->user2,
			&zero);
		
		for(i=0;i<wr->NumberOfBuffers;++i)
		{
			if(wr->UserFree[i]==0) {FREE(wr->Buffer[i]);}
		}
		FREE(wr->Buffer);
		FREE(wr->BufferLength);
		FREE(wr->UserFree);
		FREE(wr);
	}
	ILibQueue_Destroy(RemoveQ);
}
void ILibWebClient_Resume(void *wcdo)
{
	struct ILibWebClientDataObject *d = (struct ILibWebClientDataObject*)wcdo;
	d->PAUSE = 0;
	ILibAsyncSocket_Resume(d->SOCK);
}

