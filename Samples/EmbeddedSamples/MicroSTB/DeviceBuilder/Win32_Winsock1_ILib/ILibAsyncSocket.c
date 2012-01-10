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

#include "ILibParsers.h"
#include "ILibAsyncSocket.h"
#include "Utility.h"

#ifdef _WIN32_WCE
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000
#define sem_t HANDLE
#define sem_init(x,pshared,pvalue) *x=CreateSemaphore(NULL,pvalue,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif WIN32
#include <errno.h>
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif _POSIX
#include <errno.h>
#include <semaphore.h>
extern int errno;
#endif

#define DEBUGSTATEMENT(x)



struct AsyncSocket_SendData
{
	char* buffer;
	int bufferSize;
	int bytesSent;

	int UserFree;
	struct AsyncSocket_SendData *Next;
};

struct AsyncSocketModule
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void *Chain;

	unsigned int PendingBytesToSend;
	unsigned int TotalBytesSent;

	#ifdef _WIN32_WCE
		SOCKET internalSocket;
	#elif WIN32
		SOCKET internalSocket;
	#elif _POSIX
		int internalSocket;
	#endif

	int RemoteIPAddress;
	int LocalIPAddress;
	void(*OnData)(void* socketModule,char* buffer,int *p_beginPointer, int endPointer,void (**InterruptPtr)(void *socketModule, void *user), void **user, int *PAUSE);
	void(*OnConnect)(void* socketModule, int OK, void *user);
	void(*OnDisconnect)(void* socketModule, void *user);
	void(*OnSendOK)(void *socketModule, void *user);
	void(*OnInterrupt)(void *socketModule, void *user);

	void *user;
	int IsFree;
	int PAUSE;
	
	int FinConnect;
	int BeginPointer;
	int EndPointer;
	
	char* buffer;
	int MallocSize;
	int InitialSize;

	struct AsyncSocket_SendData *PendingSend_Head;
	struct AsyncSocket_SendData *PendingSend_Tail;
	sem_t SendLock;
};

void ILibAsyncSocket_PostSelect(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
void ILibAsyncSocket_PreSelect(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);

void ILibAsyncSocket_Destroy(void *socketModule)
{
	struct AsyncSocketModule* module = (struct AsyncSocketModule*)socketModule;
	struct AsyncSocket_SendData *temp,*current;

	if(module->internalSocket!=~0)
	{
		#ifdef _WIN32_WCE
			closesocket(module->internalSocket);
		#elif WIN32
			closesocket(module->internalSocket);
		#elif _POSIX
			close(module->internalSocket);
		#endif
	}
	
	if(module->IsFree==0)
	{
		if(module->OnInterrupt!=NULL)
		{
			module->OnInterrupt(module,module->user);
		}
	}

	if(module->buffer!=NULL)
	{
		FREE(module->buffer);
		module->buffer = NULL;
		module->MallocSize = 0;
	}
	
	temp=current=module->PendingSend_Head;
	while(current!=NULL)
	{
		temp = current->Next;
		if(current->UserFree==0)
		{
			FREE(current->buffer);
		}
		FREE(current);
		current = temp;
	}
	
	sem_destroy(&(module->SendLock));
}
void* ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, void(*OnData)(void* socketModule,char* buffer,int *p_beginPointer, int endPointer, void (**InterruptPtr)(void *socketModule, void *user),void **user, int *PAUSE), void(*OnConnect)(void* socketModule, int Connected, void *user),void(*OnDisconnect)(void* socketModule, void *user),void(*OnSendOK)(void *socketModule, void *user))
{
	struct AsyncSocketModule *RetVal = (struct AsyncSocketModule*)MALLOC(sizeof(struct AsyncSocketModule));
	memset(RetVal,0,sizeof(struct AsyncSocketModule));
	RetVal->PreSelect = &ILibAsyncSocket_PreSelect;
	RetVal->PostSelect = &ILibAsyncSocket_PostSelect;
	RetVal->Destroy = &ILibAsyncSocket_Destroy;
	
	RetVal->IsFree = 1;
	RetVal->internalSocket = -1;
	RetVal->OnData = OnData;
	RetVal->OnConnect = OnConnect;
	RetVal->OnDisconnect = OnDisconnect;
	RetVal->OnSendOK = OnSendOK;
	RetVal->buffer = (char*)MALLOC(initialBufferSize);
	RetVal->InitialSize = initialBufferSize;
	RetVal->MallocSize = initialBufferSize;

	sem_init(&(RetVal->SendLock),0,1);
	
	RetVal->Chain = Chain;
	ILibAddToChain(Chain,RetVal);

	return((void*)RetVal);
}

void ILibAsyncSocket_ClearPendingSend(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	struct AsyncSocket_SendData *data,*temp;
	
	data = module->PendingSend_Head;
	module->PendingSend_Tail = NULL;
	while(data!=NULL)
	{
		temp = data->Next;
		if(data->UserFree==0)
		{
			FREE(data->buffer);
		}
		FREE(data);
		data = temp;
	}
	module->PendingSend_Head = NULL;
}

int ILibAsyncSocket_Send(void* socketModule, char* buffer, int length, enum ILibAsyncSocket_MemoryOwnership UserFree)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	struct AsyncSocket_SendData *data = (struct AsyncSocket_SendData*)MALLOC(sizeof(struct AsyncSocket_SendData));
	int unblock=0;
	int bytesSent;

	data->buffer = buffer;
	data->bufferSize = length;
	data->bytesSent = 0;
	data->UserFree = UserFree;
	data->Next = NULL;

	sem_wait(&(module->SendLock));
	if(module->internalSocket==~0)
	{
		// Too Bad, the socket closed
		if(UserFree==0){FREE(buffer);}
		FREE(data);
		sem_post(&(module->SendLock));
		return(ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR);
	}

	module->PendingBytesToSend += length;
	if(module->PendingSend_Tail!=NULL)
	{
		module->PendingSend_Tail->Next = data;
		module->PendingSend_Tail = data;
		unblock=1;
		if(UserFree==ILibAsyncSocket_MemoryOwnership_USER)
		{
			data->buffer = (char*)MALLOC(data->bufferSize);
			memcpy(data->buffer,buffer,length);
			data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
		}
	}
	else
	{
		module->PendingSend_Tail = data;
		module->PendingSend_Head = data;
		
		bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0);
		if(bytesSent>0)
		{
			module->PendingSend_Head->bytesSent+=bytesSent;
			module->PendingBytesToSend -= bytesSent;
			module->TotalBytesSent += bytesSent;
		}
		if(bytesSent==-1)
		{
			// Send Failed
#ifdef _WIN32_WCE
			bytesSent = WSAGetLastError();
			if(bytesSent!=WSAEWOULDBLOCK)
#elif WIN32
			bytesSent = WSAGetLastError();
			if(bytesSent!=WSAEWOULDBLOCK)
#else
			if(errno!=EWOULDBLOCK)
#endif
			{
				if(UserFree==0){FREE(buffer);}
				module->PendingSend_Head = module->PendingSend_Tail = NULL;
				FREE(data);
				sem_post(&(module->SendLock));
				ILibAsyncSocket_Disconnect(socketModule);
				return(ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR);
			}
		}
		if(module->PendingSend_Head->bytesSent==module->PendingSend_Head->bufferSize)
		{
			if(UserFree==0){FREE(module->PendingSend_Head->buffer);}
			module->PendingSend_Tail = NULL;
			FREE(module->PendingSend_Head);
			module->PendingSend_Head = NULL;
		}
		else
		{
			if(UserFree==ILibAsyncSocket_MemoryOwnership_USER)
			{
				data->buffer = (char*)MALLOC(data->bufferSize);
				memcpy(data->buffer,buffer,length);
				data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
			}
			unblock = 1;
		}

	}
	sem_post(&(module->SendLock));
	if(unblock!=0) {ILibForceUnBlockChain(module->Chain);}
	return(unblock);
}
void ILibAsyncSocket_Disconnect(void* socketModule)
{
	#ifdef _WIN32_WCE
		SOCKET s;
	#elif WIN32
		SOCKET s;
	#elif _POSIX
		int s;
	#endif

	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	module->IsFree = 1;
	module->PAUSE = 1;
	s = module->internalSocket;
	module->internalSocket = ~0;
	if(s!=-1)
	{
		#ifdef _WIN32_WCE
				closesocket(s);
		#elif WIN32
				closesocket(s);
		#elif _POSIX
				close(s);
		#endif
	}
	sem_wait(&(module->SendLock));
	ILibAsyncSocket_ClearPendingSend(socketModule);
	sem_post(&(module->SendLock));
	if(module->OnDisconnect!=NULL)
	{
		module->OnDisconnect(module,module->user);
	}
}
void ILibAsyncSocket_ConnectTo(void* socketModule, int localInterface, int remoteInterface, int remotePortNumber, void (*InterruptPtr)(void *socketModule, void *user),void *user)
{
	int flags;
	struct sockaddr_in addr;
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	
	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->IsFree = 0;
	module->PAUSE = 0;
	module->user = user;
	module->OnInterrupt = InterruptPtr;
	module->buffer = (char*)realloc(module->buffer,module->InitialSize);
	module->MallocSize = module->InitialSize;
	memset((char *)&addr, 0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = remoteInterface;

	#ifdef _WIN32_WCE
		addr.sin_port = htons((unsigned short)remotePortNumber);
	#elif WIN32
		addr.sin_port = htons(remotePortNumber);
	#elif _POSIX
		addr.sin_port = htons(remotePortNumber);
	#endif
	
	if(module->internalSocket==-1)
	{
		#ifdef WINSOCK2
			ILibGetStreamSocket(localInterface,0,(HANDLE*)&(module->internalSocket));
		#else
			ILibGetStreamSocket(localInterface,0,&(module->internalSocket));
		#endif
	}
	
	module->FinConnect = 0;
	module->BeginPointer = 0;
	module->EndPointer = 0;
	
	#ifdef _WIN32_WCE
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif WIN32
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif _POSIX
		flags = fcntl(module->internalSocket,F_GETFL,0);
		fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
	#endif

	connect(module->internalSocket,(struct sockaddr*)&addr,sizeof(addr));
	ILibForceUnBlockChain(module->Chain);
}
void ILibProcessAsyncSocket(struct AsyncSocketModule *Reader)
{
	int bytesReceived;

	while(Reader->PAUSE==0 && Reader->BeginPointer!=Reader->EndPointer && Reader->BeginPointer!=0)
	{
		memcpy(Reader->buffer,Reader->buffer+Reader->BeginPointer,Reader->EndPointer-Reader->BeginPointer);
		Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
		Reader->BeginPointer = 0;
		if(Reader->OnData!=NULL)
		{
			Reader->OnData(Reader,Reader->buffer,&(Reader->BeginPointer),Reader->EndPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
		}
	}
	if(Reader->PAUSE!=0)
	{
		return;
	}

	
	/* Reading Body Only */
	if(Reader->BeginPointer == Reader->EndPointer)
	{
		Reader->BeginPointer = 0;
		Reader->EndPointer = 0;
	}
	else
	{
		if(Reader->BeginPointer!=0)
		{
			Reader->EndPointer = Reader->BeginPointer;
		}
	}
	
	bytesReceived = recv(Reader->internalSocket,Reader->buffer+Reader->EndPointer,Reader->MallocSize-Reader->EndPointer,0);
	Reader->EndPointer += bytesReceived;
	
	
	if(bytesReceived<=0)
	{
		Reader->IsFree = 1;
		sem_wait(&(Reader->SendLock));
		ILibAsyncSocket_ClearPendingSend(Reader);
		sem_post(&(Reader->SendLock));

		#ifdef _WIN32_WCE
			closesocket(Reader->internalSocket);
		#elif WIN32
			closesocket(Reader->internalSocket);
		#elif _POSIX
			close(Reader->internalSocket);
		#endif

		Reader->internalSocket = ~0;
		Reader->IsFree = 1;

		if(Reader->OnDisconnect!=NULL)
		{
			Reader->OnDisconnect(Reader,Reader->user);
		}

		if(Reader->IsFree!=0 && Reader->buffer!=NULL)
		{
			FREE(Reader->buffer);
			Reader->buffer = NULL;
			Reader->MallocSize = 0;
		}
	}
	else
	{
		if(Reader->OnData!=NULL)
		{
			Reader->OnData(Reader,Reader->buffer,&(Reader->BeginPointer),Reader->EndPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
		}
		while(Reader->PAUSE==0 && Reader->BeginPointer!=Reader->EndPointer && Reader->BeginPointer!=0)
		{
			memcpy(Reader->buffer,Reader->buffer+Reader->BeginPointer,Reader->EndPointer-Reader->BeginPointer);
			Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
			Reader->BeginPointer = 0;
			if(Reader->OnData!=NULL)
			{
				Reader->OnData(Reader,Reader->buffer,&(Reader->BeginPointer),Reader->EndPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
			}
		}
		
		if(Reader->BeginPointer==Reader->EndPointer)
		{
			Reader->BeginPointer = 0;
			Reader->EndPointer = 0;
		}
		
		if(Reader->MallocSize - Reader->EndPointer <1024)
		{
			Reader->MallocSize += 4096;
			Reader->buffer = (char*)realloc(Reader->buffer,Reader->MallocSize);
		}
	}
}

void ILibAsyncSocket_PreSelect(void* socketModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;

	if(module->internalSocket!=-1)
	{
		if(module->FinConnect==0)
		{
			/* Not Connected Yet */
			FD_SET(module->internalSocket,writeset);
			FD_SET(module->internalSocket,errorset);
		}
		else
		{
			if(module->PAUSE==0)
			{
				/* Already Connected, just needs reading */
				FD_SET(module->internalSocket,readset);
				FD_SET(module->internalSocket,errorset);
			}
		}
	}

	sem_wait(&(module->SendLock));
	if(module->PendingSend_Head!=NULL)
	{
		FD_SET(module->internalSocket,writeset);
	}
	sem_post(&(module->SendLock));
}
void ILibAsyncSocket_PostSelect(void* socketModule,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
{
	int TriggerSendOK = 0;
	struct AsyncSocket_SendData *temp;
	int bytesSent=0;
	int flags;
	struct sockaddr_in receivingAddress;
	int receivingAddressLength = sizeof(struct sockaddr_in);
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	int TRY_TO_SEND = 1;
	
	// Write Handling
	if(module->FinConnect!=0 && module->internalSocket!=~0 && FD_ISSET(module->internalSocket,writeset)!=0)
	{
		sem_wait(&(module->SendLock));
		while(TRY_TO_SEND!=0)
		{
			bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0);
			if(bytesSent>0)
			{
				module->PendingBytesToSend -= bytesSent;
				module->TotalBytesSent += bytesSent;
				module->PendingSend_Head->bytesSent+=bytesSent;
				if(module->PendingSend_Head->bytesSent==module->PendingSend_Head->bufferSize)
				{
					// Finished Sending this block
					if(module->PendingSend_Head==module->PendingSend_Tail)
					{
						module->PendingSend_Tail = NULL;
					}
					if(module->PendingSend_Head->UserFree==0)
					{
						FREE(module->PendingSend_Head->buffer);
					}
					temp = module->PendingSend_Head->Next;
					FREE(module->PendingSend_Head);
					module->PendingSend_Head = temp;
					if(module->PendingSend_Head==NULL) {TRY_TO_SEND=0;}
				}
				else
				{
					TRY_TO_SEND = 1;
				}
			}
			if(bytesSent==-1)
			{
				// Error, clean up everything
				#ifdef _WIN32_WCE
					bytesSent = WSAGetLastError();
					if(bytesSent!=WSAEWOULDBLOCK)
				#elif WIN32
					bytesSent = WSAGetLastError();
					if(bytesSent!=WSAEWOULDBLOCK)
				#else
					if(errno!=EWOULDBLOCK)
				#endif
				{
					ILibAsyncSocket_ClearPendingSend(socketModule);
					TRY_TO_SEND = 0;
				}
			}
		}
		if(module->PendingSend_Head==NULL && bytesSent!=-1) {TriggerSendOK=1;}
		sem_post(&(module->SendLock));
		if(TriggerSendOK!=0)
		{
			module->OnSendOK(module,module->user);
		}
	}

	// Connection Handling / Read Handling
	if(module->internalSocket!=~0)
	{
		if(module->FinConnect==0)
		{
			/* Not Connected Yet */
			if(FD_ISSET(module->internalSocket,writeset)!=0)
			{
				/* Connected */
				getsockname(module->internalSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);
				module->LocalIPAddress = receivingAddress.sin_addr.s_addr;
				module->FinConnect = 1;
				module->PAUSE = 0;
				
				#ifdef _WIN32_WCE
					flags = 1;
					ioctlsocket(module->internalSocket,FIONBIO,&flags);
				#elif WIN32
					flags = 1;
					ioctlsocket(module->internalSocket,FIONBIO,&flags);
				#elif _POSIX
					flags = fcntl(module->internalSocket,F_GETFL,0);
					fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
				#endif

				/* Connection Complete */
				if(module->OnConnect!=NULL)
				{
					module->OnConnect(module,-1,module->user);
				}
			}
			if(FD_ISSET(module->internalSocket,errorset)!=0)
			{
				/* Connection Failed */
				#ifdef _WIN32_WCE
					closesocket(module->internalSocket);
				#elif WIN32
					closesocket(module->internalSocket);
				#elif _POSIX
					close(module->internalSocket);
				#endif
				module->internalSocket = ~0;
				if(module->OnConnect!=NULL)
				{
					module->OnConnect(module,0,module->user);
				}
			}
		}
		else
		{
			/* Check if PeerReset */
			if(FD_ISSET(module->internalSocket,errorset)!=0)
			{
				/* Socket Closed */
				#ifdef _WIN32_WCE
					closesocket(module->internalSocket);
				#elif WIN32
					closesocket(module->internalSocket);
				#elif _POSIX
					close(module->internalSocket);
				#endif
				module->internalSocket = ~0;
				module->IsFree=1;
				module->PAUSE = 1;
				sem_wait(&(module->SendLock));
				ILibAsyncSocket_ClearPendingSend(socketModule);
				sem_post(&(module->SendLock));
				if(module->OnDisconnect!=NULL)
				{
					module->OnDisconnect(module,module->user);
				}
			}
			/* Already Connected, just needs reading */
			if(FD_ISSET(module->internalSocket,readset)!=0)
			{
				/* Data Available */
				ILibProcessAsyncSocket(module);
			}
		}
	}
}
int ILibAsyncSocket_IsFree(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	return(module->IsFree);
}
unsigned int ILibAsyncSocket_GetPendingBytesToSend(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	return(module->PendingBytesToSend);
}
unsigned int ILibAsyncSocket_GetTotalBytesSent(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
    return(module->TotalBytesSent);
}
void ILibAsyncSocket_ResetTotalBytesSent(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	module->TotalBytesSent = 0;
}

void ILibAsyncSocket_GetBuffer(void *socketModule, char **buffer, int *BeginPointer, int *EndPointer)
{
	struct AsyncSocketModule* module = (struct AsyncSocketModule*)socketModule;

	*buffer = module->buffer;
	*BeginPointer = module->BeginPointer;
	*EndPointer = module->EndPointer;
}
void ILibAsyncSocket_SetRemoteAddress(void *socketModule,int RemoteAddress)
{
	struct AsyncSocketModule* module = (struct AsyncSocketModule*)socketModule;
	module->RemoteIPAddress = RemoteAddress;
}

void ILibAsyncSocket_UseThisSocket(void *socketModule,void* UseThisSocket,void (*InterruptPtr)(void *socketModule, void *user),void *user)
{
	#ifdef _WIN32_WCE
		SOCKET TheSocket = *((SOCKET*)UseThisSocket);
	#elif WIN32
		SOCKET TheSocket = *((SOCKET*)UseThisSocket);
	#elif _POSIX
		int TheSocket = *((int*)UseThisSocket);
	#endif
	int flags;
	struct AsyncSocketModule* module = (struct AsyncSocketModule*)socketModule;
	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->internalSocket = TheSocket;
	module->IsFree = 0;
	module->OnInterrupt = InterruptPtr;
	module->user = user;
	module->FinConnect = 1;
	module->PAUSE = 0;

	module->buffer = (char*)realloc(module->buffer,module->InitialSize);
	module->MallocSize = module->InitialSize;
	module->FinConnect = 1;
	module->BeginPointer = 0;
	module->EndPointer = 0;

	#ifdef _WIN32_WCE
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif WIN32
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif _POSIX
		flags = fcntl(module->internalSocket,F_GETFL,0);
		fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
	#endif
}
int ILibAsyncSocket_GetRemoteInterface(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	return(module->RemoteIPAddress);
}
int ILibAsyncSocket_GetLocalInterface(void *socketModule)
{
	struct AsyncSocketModule *module = (struct AsyncSocketModule*)socketModule;
	struct sockaddr_in receivingAddress;
	int receivingAddressLength = sizeof(struct sockaddr_in);

	getsockname(module->internalSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);
	return(receivingAddress.sin_addr.s_addr);
}
void ILibAsyncSocket_Resume(void *socketModule)
{
	struct AsyncSocketModule *sm = (struct AsyncSocketModule*)socketModule;
	if(sm->PAUSE!=0)
	{
		sm->PAUSE=0;
		ILibForceUnBlockChain(sm->Chain);
	}
}