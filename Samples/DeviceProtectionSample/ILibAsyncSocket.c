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

#ifdef MEMORY_CHECK
#include <assert.h>
#define MEMCHECK(x) x
#else
#define MEMCHECK(x)
#endif

#if defined(WIN32) && !defined(_WIN32_WCE)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#if defined(WINSOCK2)
#include <winsock2.h>
#include <ws2tcpip.h>
#define MSG_NOSIGNAL 0
#elif defined(WINSOCK1)
#include <winsock.h>
#include <wininet.h>
#endif

#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#include "ILibParsers.h"
#include "ILibAsyncSocket.h"

#ifndef MICROSTACK_NOTLS
#include <openssl/ssl.h>
#endif

//#ifndef WINSOCK2
//#define SOCKET unsigned int
//#endif

#define INET_SOCKADDR_LENGTH(x) ((x==AF_INET6?sizeof(struct sockaddr_in6):sizeof(struct sockaddr_in)))

#if defined(WIN32)
#define snprintf(dst, len, frm, ...) _snprintf_s(dst, len, _TRUNCATE, frm, __VA_ARGS__)
#endif

#ifdef SEMAPHORE_TRACKING
#define SEM_TRACK(x) x
void AsyncSocket_TrackLock(const char* MethodName, int Occurance, void *data)
{
	char v[100];
	wchar_t wv[100];
	size_t l;

	snprintf(v, 100, "  LOCK[%s, %d] (%x)\r\n",MethodName,Occurance,data);
#ifdef WIN32
	mbstowcs_s(&l, wv, 100, v, 100);
	OutputDebugString(wv);
#else
	printf(v);
#endif
}
void AsyncSocket_TrackUnLock(const char* MethodName, int Occurance, void *data)
{
	char v[100];
	wchar_t wv[100];
	size_t l;

	snprintf(v, 100, "UNLOCK[%s, %d] (%x)\r\n",MethodName,Occurance,data);
#ifdef WIN32
	mbstowcs_s(&l, wv, 100, v, 100);
	OutputDebugString(wv);
#else
	printf(v);
#endif
}
#else
#define SEM_TRACK(x)
#endif

struct ILibAsyncSocket_SendData
{
	char* buffer;
	int bufferSize;
	int bytesSent;

	struct sockaddr_in6 remoteAddress;

	int UserFree;
	struct ILibAsyncSocket_SendData *Next;
};

struct ILibAsyncSocketModule
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void *Chain;

	unsigned int PendingBytesToSend;
	unsigned int TotalBytesSent;

#if defined(_WIN32_WCE) || defined(WIN32)
	SOCKET internalSocket;
#elif defined(_POSIX)
	int internalSocket;
#endif

	// The IPv4/IPv6 compliant address of the remote endpoint. We are not going to be using IPv6 all the time,
	// but we use the IPv6 structure to allocate the meximum space we need.
	struct sockaddr_in6 RemoteAddress;

	// Local interface of a given socket. This module will bind to any interface, but the actual interface used
	// is stored here.
	struct sockaddr_in6 LocalAddress;

	// Source address. Here is stored the actual source of a packet, usualy used with UDP where the source
	// of the traffic changes.
	struct sockaddr_in6 SourceAddress;

#ifdef MICROSTACK_PROXY
	// The address and port of a HTTPS proxy
	struct sockaddr_in6 ProxyAddress;
	int ProxyState;
#endif

	ILibAsyncSocket_OnData OnData;
	ILibAsyncSocket_OnConnect OnConnect;
	ILibAsyncSocket_OnDisconnect OnDisconnect;
	ILibAsyncSocket_OnSendOK OnSendOK;
	ILibAsyncSocket_OnInterrupt OnInterrupt;

	ILibAsyncSocket_OnBufferSizeExceeded OnBufferSizeExceeded;
	ILibAsyncSocket_OnBufferReAllocated OnBufferReAllocated;

	void *LifeTime;
	void *user;
	void *user2;
	int user3;
	int PAUSE;
	int FinConnect;
	int BeginPointer;
	int EndPointer;
	char* buffer;
	int MallocSize;
	int InitialSize;

	struct ILibAsyncSocket_SendData *PendingSend_Head;
	struct ILibAsyncSocket_SendData *PendingSend_Tail;
	sem_t SendLock;

	int MaxBufferSize;
	int MaxBufferSizeExceeded;
	void *MaxBufferSizeUserObject;

	// Added for TLS support
	#ifndef MICROSTACK_NOTLS
	int SSLConnect;
	SSL* ssl;
	SSL_SESSION * ssl_session;
	int  sslstate;
	BIO* sslbio;
	SSL_CTX *ssl_ctx;
	#endif
};

void ILibAsyncSocket_PostSelect(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
void ILibAsyncSocket_PreSelect(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);

//
// An internal method called by Chain as Destroy, to cleanup AsyncSocket
//
// <param name="socketModule">The AsyncSocketModule</param>
void ILibAsyncSocket_Destroy(void *socketModule)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *temp, *current;

	// Call the interrupt event if necessary
	if (!ILibAsyncSocket_IsFree(module))
	{
		if (module->OnInterrupt != NULL) module->OnInterrupt(module, module->user);
	}

	#ifndef MICROSTACK_NOTLS
	// If this is an SSL socket, free the SSL state
	if (module->ssl != NULL)
	{
		SSL_free(module->ssl); // Frees SSL session and BIO buffer at the same time
		module->ssl = NULL;
		module->sslstate = 0;
		module->sslbio = NULL;
	}
	#endif

	// Close socket if necessary
	if (module->internalSocket != ~0)
	{
#if defined(_WIN32_WCE) || defined(WIN32)
#if defined(WINSOCK2)
		shutdown(module->internalSocket, SD_BOTH);
#endif
		closesocket(module->internalSocket);
#elif defined(_POSIX)
		shutdown(module->internalSocket,SHUT_RDWR);
		close(module->internalSocket);
#endif
		module->internalSocket = (SOCKET)~0;
	}

	// Free the buffer if necessary
	if (module->buffer != NULL)
	{
		if (module->buffer != ILibScratchPad2) free(module->buffer);
		module->buffer = NULL;
		module->MallocSize = 0;
	}

	// Clear all the data that is pending to be sent
	temp = current = module->PendingSend_Head;
	while (current != NULL)
	{
		temp = current->Next;
		if (current->UserFree == 0) free(current->buffer);
		free(current);
		current = temp;
	}

	module->FinConnect = 0;
	module->user = NULL;
	#ifndef MICROSTACK_NOTLS
	module->SSLConnect = 0;
	module->sslstate = 0;
	#endif
	sem_destroy(&(module->SendLock));
}
/*! \fn ILibAsyncSocket_SetReAllocateNotificationCallback(ILibAsyncSocket_SocketModule AsyncSocketToken, ILibAsyncSocket_OnBufferReAllocated Callback)
\brief Set the callback handler for when the internal data buffer has been resized
\param AsyncSocketToken The specific connection to set the callback with
\param Callback The callback handler to set
*/
void ILibAsyncSocket_SetReAllocateNotificationCallback(ILibAsyncSocket_SocketModule AsyncSocketToken, ILibAsyncSocket_OnBufferReAllocated Callback)
{
	if (AsyncSocketToken != NULL) { ((struct ILibAsyncSocketModule*)AsyncSocketToken)->OnBufferReAllocated = Callback; }
}

/*! \fn ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, ILibAsyncSocket_OnData OnData, ILibAsyncSocket_OnConnect OnConnect, ILibAsyncSocket_OnDisconnect OnDisconnect,ILibAsyncSocket_OnSendOK OnSendOK)
\brief Creates a new AsyncSocketModule
\param Chain The chain to add this module to. (Chain must <B>not</B> be running)
\param initialBufferSize The initial size of the receive buffer
\param OnData Function Pointer that triggers when Data is received
\param OnConnect Function Pointer that triggers upon successfull connection establishment
\param OnDisconnect Function Pointer that triggers upon disconnect
\param OnSendOK Function Pointer that triggers when pending sends are complete
\returns An ILibAsyncSocket token
*/
ILibAsyncSocket_SocketModule ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, ILibAsyncSocket_OnData OnData, ILibAsyncSocket_OnConnect OnConnect, ILibAsyncSocket_OnDisconnect OnDisconnect, ILibAsyncSocket_OnSendOK OnSendOK)
{
	struct ILibAsyncSocketModule *RetVal = (struct ILibAsyncSocketModule*)malloc(sizeof(struct ILibAsyncSocketModule));
	if (RetVal == NULL) return NULL;
	memset(RetVal, 0, sizeof(struct ILibAsyncSocketModule));
	if (initialBufferSize != 0)
	{
		// Use a new buffer
		if ((RetVal->buffer = (char*)malloc(initialBufferSize)) == NULL) ILIBCRITICALEXIT(254);
	}
	else
	{
		// Use a static buffer, often used for UDP.
		initialBufferSize = sizeof(ILibScratchPad2);
		RetVal->buffer = ILibScratchPad2;
	}
	RetVal->PreSelect = &ILibAsyncSocket_PreSelect;
	RetVal->PostSelect = &ILibAsyncSocket_PostSelect;
	RetVal->Destroy = &ILibAsyncSocket_Destroy;
	RetVal->internalSocket = (SOCKET)~0;
	RetVal->OnData = OnData;
	RetVal->OnConnect = OnConnect;
	RetVal->OnDisconnect = OnDisconnect;
	RetVal->OnSendOK = OnSendOK;
	RetVal->InitialSize = initialBufferSize;
	RetVal->MallocSize = initialBufferSize;
	RetVal->LifeTime = ILibGetBaseTimer(Chain); //ILibCreateLifeTime(Chain);

	sem_init(&(RetVal->SendLock), 0, 1);

	RetVal->Chain = Chain;
	ILibAddToChain(Chain, RetVal);

	return((void*)RetVal);
}

/*! \fn ILibAsyncSocket_ClearPendingSend(ILibAsyncSocket_SocketModule socketModule)
\brief Clears all the pending data to be sent for an AsyncSocket
\param socketModule The ILibAsyncSocket to clear
*/
void ILibAsyncSocket_ClearPendingSend(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *data, *temp;

	data = module->PendingSend_Head;
	module->PendingSend_Tail = NULL;
	while (data != NULL)
	{
		temp = data->Next;
		// We only need to free this if we have ownership of this memory
		if (data->UserFree == 0) free(data->buffer);
		free(data);
		data = temp;
	}
	module->PendingSend_Head = NULL;
	module->PendingBytesToSend = 0;
}

/*! \fn ILibAsyncSocket_SendTo(ILibAsyncSocket_SocketModule socketModule, char* buffer, int length, int remoteAddress, unsigned short remotePort, enum ILibAsyncSocket_MemoryOwnership UserFree)
\brief Sends data on an AsyncSocket module to a specific destination. (Valid only for <B>UDP</B>)
\param socketModule The ILibAsyncSocket module to send data on
\param buffer The buffer to send
\param length The length of the buffer to send
\param remoteAddress The IPAddress of the destination 
\param remotePort The Port number of the destination
\param UserFree Flag indicating memory ownership. 
\returns \a ILibAsyncSocket_SendStatus indicating the send status
*/
enum ILibAsyncSocket_SendStatus ILibAsyncSocket_SendTo(ILibAsyncSocket_SocketModule socketModule, char* buffer, int length, struct sockaddr *remoteAddress, enum ILibAsyncSocket_MemoryOwnership UserFree)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *data;
	int unblock = 0;
	int bytesSent;

	// If the socket is empty, return now.
	if (socketModule == NULL) return(ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR);

	// Setup a new send data structure
	if ((data = (struct ILibAsyncSocket_SendData*)malloc(sizeof(struct ILibAsyncSocket_SendData))) == NULL) ILIBCRITICALEXIT(254);
	memset(data, 0, sizeof(struct ILibAsyncSocket_SendData));
	data->buffer = buffer;
	data->bufferSize = length;
	data->bytesSent = 0;
	data->UserFree = UserFree;
	data->Next = NULL;

	// Copy the address to the send data structure
	memset(&(data->remoteAddress), 0, sizeof(struct sockaddr_in6));
	if (remoteAddress != NULL) memcpy(&(data->remoteAddress), remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family));

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_Send", 1, module);)
	sem_wait(&(module->SendLock));

	if (module->internalSocket == ~0)
	{
		// Too Bad, the socket closed
		if (UserFree == 0) {free(buffer);}
		free(data);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send", 2, module);)
		sem_post(&(module->SendLock));
		return ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;
	}

	module->PendingBytesToSend += length;
	if (module->PendingSend_Tail != NULL || module->FinConnect == 0)
	{
		// There are still bytes that are pending to be sent, or pending connection, so we need to queue this up
		if (module->PendingSend_Tail == NULL)
		{
			module->PendingSend_Tail = data;
			module->PendingSend_Head = data;
		}
		else
		{
			module->PendingSend_Tail->Next = data;
			module->PendingSend_Tail = data;
		}
		unblock = 1;
		if (UserFree == ILibAsyncSocket_MemoryOwnership_USER)
		{
			// If we don't own this memory, we need to copy the buffer,
			// because the user may free this memory before we have a chance to send it
			if ((data->buffer = (char*)malloc(data->bufferSize)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(data->buffer, buffer, length);
			MEMCHECK(assert(length <= data->bufferSize);)
			data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
		}
	}
	else
	{
		// There is no data pending to be sent, so lets go ahead and try to send this data
		module->PendingSend_Tail = data;
		module->PendingSend_Head = data;

		#ifndef MICROSTACK_NOTLS
		if (module->ssl != NULL || remoteAddress == NULL)
		{
			if (module->ssl == NULL)
			{
				// Send on non-SSL socket, set MSG_NOSIGNAL since we don't want to get Broken Pipe signals in Linux, ignored if Windows.
				bytesSent = send(module->internalSocket, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent, MSG_NOSIGNAL);
			}
			else
			{
				// Send on SSL socket, set MSG_NOSIGNAL since we don't want to get Broken Pipe signals in Linux, ignored if Windows.
				bytesSent = SSL_write(module->ssl, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent);
			}
		}
		else
		{
			bytesSent = sendto(module->internalSocket, module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent, MSG_NOSIGNAL, (struct sockaddr*)remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family));
		}
		#else
		if (remoteAddress == NULL)
		{
			// Set MSG_NOSIGNAL since we don't want to get Broken Pipe signals in Linux, ignored if Windows.
			bytesSent = send(module->internalSocket, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent, MSG_NOSIGNAL);
		}
		else
		{
			bytesSent = sendto(module->internalSocket, module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent, MSG_NOSIGNAL, (struct sockaddr*)remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family));
		}
		#endif

		if (bytesSent > 0)
		{
			// We were able to send something, so lets increment the counters
			module->PendingSend_Head->bytesSent += bytesSent;
			module->PendingBytesToSend -= bytesSent;
			module->TotalBytesSent += bytesSent;
		}

		#ifndef MICROSTACK_NOTLS
		if (bytesSent == -1 && module->ssl != NULL)
		{
			// OpenSSL returned an error
			bytesSent = SSL_get_error(module->ssl, bytesSent);
#ifdef WIN32
			if (bytesSent != SSL_ERROR_WANT_WRITE && bytesSent != SSL_ERROR_SSL && !(bytesSent == SSL_ERROR_SYSCALL && WSAGetLastError() == WSAEWOULDBLOCK)) 
#else
			if (bytesSent != SSL_ERROR_WANT_WRITE && bytesSent != SSL_ERROR_SSL) // "bytesSent != SSL_ERROR_SSL" portion is weird, but if not present, flowcontrol fails.
#endif
			{
				//ILIBMESSAGE2("SSL WRITE ERROR1\r\n", bytesSent);	// DEBUG
				//ILIBMESSAGE2("SSL WRITE ERROR2\r\n", WSAGetLastError());

				// Most likely the socket closed while we tried to send
				if (UserFree == 0) { free(buffer); }
				module->PendingSend_Head = module->PendingSend_Tail = NULL;
				free(data);
				SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send", 3, module);)
				sem_post(&(module->SendLock));

				// Ensure Calling On_Disconnect with MicroStackThread
				ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);

				return ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;
			}
		}
		#endif

		#ifndef MICROSTACK_NOTLS
		if (bytesSent == -1 && module->ssl == NULL)
		#else
		if (bytesSent == -1)
		#endif
		{
			// Send returned an error, so lets figure out what it was, as it could be normal
#if defined(_WIN32_WCE) || defined(WIN32)
			bytesSent = WSAGetLastError();
			if (bytesSent != WSAEWOULDBLOCK)
#elif defined(_POSIX)
			if (errno != EWOULDBLOCK)
#endif
			{
				// Most likely the socket closed while we tried to send
				if (UserFree == 0) {free(buffer);}
				module->PendingSend_Head = module->PendingSend_Tail = NULL;
				free(data);
				SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send", 3, module);)
				sem_post(&(module->SendLock));

				// Ensure Calling On_Disconnect with MicroStackThread
				ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);

				return ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;
			}
		}
		if (module->PendingSend_Head->bytesSent == module->PendingSend_Head->bufferSize)
		{
			// All of the data has been sent
			if (UserFree == 0) {free(module->PendingSend_Head->buffer);}
			module->PendingSend_Tail = NULL;
			free(module->PendingSend_Head);
			module->PendingSend_Head = NULL;
		}
		else
		{
			// All of the data wasn't sent, so we need to copy the buffer
			// if we don't own the memory, because the user may free the
			// memory, before we have a chance to complete sending it.
			if (UserFree == ILibAsyncSocket_MemoryOwnership_USER)
			{
				if ((data->buffer = (char*)malloc(data->bufferSize)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(data->buffer,buffer,length);
				MEMCHECK(assert(length <= data->bufferSize);)
				data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
			}
			unblock = 1;
		}

	}
	SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send", 4, module);)
	sem_post(&(module->SendLock));
	if (unblock != 0) {ILibForceUnBlockChain(module->Chain);}
	return (enum ILibAsyncSocket_SendStatus)unblock;
}

/*! \fn ILibAsyncSocket_Disconnect(ILibAsyncSocket_SocketModule socketModule)
\brief Disconnects an ILibAsyncSocket
\param socketModule The ILibAsyncSocket to disconnect
*/
void ILibAsyncSocket_Disconnect(ILibAsyncSocket_SocketModule socketModule)
{
#if defined(_WIN32_WCE) || defined(WIN32)
	SOCKET s;
#else
	int s;
#endif
	#ifndef MICROSTACK_NOTLS
	SSL *wasssl;
	#endif

	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_Disconnect", 1, module);)
	sem_wait(&(module->SendLock));

	#ifndef MICROSTACK_NOTLS
	wasssl = module->ssl;
	if (module->ssl != NULL)
	{
		sem_post(&(module->SendLock));
		SSL_free(module->ssl); // Frees SSL session and both BIO buffers at the same time
		sem_wait(&(module->SendLock));
		module->ssl = NULL;
		module->sslstate = 0;
		module->sslbio = NULL;
	}
	#endif

	if (module->internalSocket != ~0)
	{
		// There is an associated socket that is still valid, so we need to close it
		module->PAUSE = 1;
		s = module->internalSocket;
		module->internalSocket = (SOCKET)~0;
		if (s != -1)
		{
#if defined(_WIN32_WCE) || defined(WIN32)
#if defined(WINSOCK2)
			shutdown(s, SD_BOTH);
#endif
			closesocket(s);
#elif defined(_POSIX)
			shutdown(s, SHUT_RDWR);
			close(s);
#endif
		}

		// Since the socket is closing, we need to clear the data that is pending to be sent
		ILibAsyncSocket_ClearPendingSend(socketModule);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Disconnect", 2, module);)
		sem_post(&(module->SendLock));

		#ifndef MICROSTACK_NOTLS
		if (wasssl == NULL)
		{
		#endif
			// This was a normal socket, fire the event notifying the user. Depending on connection state, we event differently
			if (module->FinConnect <= 0 && module->OnConnect != NULL) { module->OnConnect(module, 0, module->user); } // Connection Failed
			if (module->FinConnect > 0 && module->OnDisconnect != NULL) { module->OnDisconnect(module, module->user); } // Socket Disconnected
		#ifndef MICROSTACK_NOTLS
		}
		else
		{
			// This was a SSL socket, fire the event notifying the user. Depending on connection state, we event differently
			if (module->SSLConnect == 0 && module->OnConnect != NULL) { module->OnConnect(module, 0, module->user); } // Connection Failed
			if (module->SSLConnect != 0 && module->OnDisconnect != NULL) { module->OnDisconnect(module, module->user); } // Socket Disconnected
		}
		#endif
		module->FinConnect = 0;
		module->user = NULL;
		#ifndef MICROSTACK_NOTLS
		module->SSLConnect = 0;
		module->sslstate = 0;
		#endif
	}
	else
	{
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Disconnect", 3, module);)
		sem_post(&(module->SendLock));
	}
}

void ILibProcessAsyncSocket(struct ILibAsyncSocketModule *Reader, int pendingRead);
void ILibAsyncSocket_Callback(ILibAsyncSocket_SocketModule socketModule, int connectDisconnectReadWrite)
{
	if (socketModule != NULL)
	{
		struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
		if (connectDisconnectReadWrite == 0) // Connected
		{
			memset(&(module->LocalAddress), 0, sizeof(struct sockaddr_in6));

			module->FinConnect = 1;
			module->PAUSE = 0;

			#ifndef MICROSTACK_NOTLS
			if (module->ssl != NULL)
			{
				if (module->ssl_session != NULL)  // vbl added client-side session caching
				{
					printf("reusing session1 ");
					SSL_set_session(module->ssl, module->ssl_session);
					SSL_SESSION_free(module->ssl_session);
					module->ssl_session = NULL; // not sure if I should do this here...
				} else {
						printf("no prior session1 ");
				}
				// If SSL enabled, we need to complete the SSL handshake before we tell the application we are connected.
				SSL_connect(module->ssl);
			}
			else
			{
			#endif
				// No SSL, tell application we are connected.
				module->OnConnect(module, -1, module->user);			
			#ifndef MICROSTACK_NOTLS
			}
			#endif
		}
		else if (connectDisconnectReadWrite == 1) // Disconnected
			ILibAsyncSocket_Disconnect(module);
		else if (connectDisconnectReadWrite == 2) // Data read
			ILibProcessAsyncSocket(module, 1);
	}
}


/*! \fn ILibAsyncSocket_ConnectTo(ILibAsyncSocket_SocketModule socketModule, int localInterface, int remoteInterface, int remotePortNumber, ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
\brief Attempts to establish a TCP connection
\param socketModule The ILibAsyncSocket to initiate the connection
\param localInterface The interface to use to establish the connection
\param remoteInterface The remote interface to connect to
\param remotePortNumber The remote port to connect to
\param InterruptPtr Function Pointer that triggers if connection attempt is interrupted
\param user User object that will be passed to the \a OnConnect method
*/
void ILibAsyncSocket_ConnectTo(void* socketModule, struct sockaddr *localInterface, struct sockaddr *remoteAddress, ILibAsyncSocket_OnInterrupt InterruptPtr, void *user)
{
	int flags = 1;
	char *tmp;
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct sockaddr_in6 any;

	// If there is something going on and we try to connect using this socket, fail! This is not supposed to happen.
	if (module->internalSocket != -1)
	{
		PRINTERROR(); ILIBCRITICALEXIT2(253, (int)(module->internalSocket));
	}

	// Clean up
	memset(&(module->RemoteAddress), 0, sizeof(struct sockaddr_in6));
	memset(&(module->LocalAddress) , 0, sizeof(struct sockaddr_in6));
	memset(&(module->SourceAddress), 0, sizeof(struct sockaddr_in6));

	// Setup
	memcpy(&(module->RemoteAddress), remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family));
	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->PAUSE = 0;
	module->user = user;
	module->OnInterrupt = InterruptPtr;
	if ((tmp = (char*)realloc(module->buffer, module->InitialSize)) == NULL) ILIBCRITICALEXIT(254);
	module->buffer = tmp;
	module->MallocSize = module->InitialSize;

	// If localInterface is NULL, we will assume INADDRANY - IPv4/IPv6 based on remote address
	if (localInterface == NULL)
	{
		memset(&any, 0, sizeof(struct sockaddr_in6));
		any.sin6_family = remoteAddress->sa_family;
		localInterface = (struct sockaddr*)&any;
	}

	// The local port should always be zero
#ifdef _DEBUG
	if (localInterface->sa_family == AF_INET && ((struct sockaddr_in*)localInterface)->sin_port != 0) { PRINTERROR(); ILIBCRITICALEXIT(253); }
	if (localInterface->sa_family == AF_INET6 && ((struct sockaddr_in*)localInterface)->sin_port != 0) { PRINTERROR(); ILIBCRITICALEXIT(253); }
#endif

	// Allocate a new socket
	if ((module->internalSocket = ILibGetSocket(localInterface, SOCK_STREAM, IPPROTO_TCP)) == 0) {PRINTERROR(); ILIBCRITICALEXIT(253);}

	// Initialise the buffer pointers, since no data is in them yet.
	module->FinConnect = 0;
	#ifndef MICROSTACK_NOTLS
	module->SSLConnect = 0;
	module->sslstate = 0;
	#endif
	module->BeginPointer = 0;
	module->EndPointer = 0;

	// Set the socket to non-blocking mode, because we need to play nice and share the MicroStack thread
#if defined(_WIN32_WCE) || defined(WIN32)
	ioctlsocket(module->internalSocket, FIONBIO, (u_long *)(&flags));
#elif defined(_POSIX)
	flags = fcntl(module->internalSocket, F_GETFL,0);
	fcntl(module->internalSocket, F_SETFL, O_NONBLOCK | flags);
#endif

	// Turn on keep-alives for the socket
	if (setsockopt(module->internalSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&flags, sizeof(flags)) != 0) ILIBCRITICALERREXIT(253);

	// Connect the socket, and force the chain to unblock, since the select statement doesn't have us in the fdset yet.
#ifdef MICROSTACK_PROXY
	if (module->ProxyAddress.sin6_family != 0)
	{
		if (connect(module->internalSocket, (struct sockaddr*)&(module->ProxyAddress), INET_SOCKADDR_LENGTH(module->ProxyAddress.sin6_family)) != -1)
		{
			// Connect failed. Set a short time and call disconnect.
			module->FinConnect = -1;
			ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
			return;
		}
	}
	else
#endif
	if (connect(module->internalSocket, (struct sockaddr*)remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family)) != -1)
	{
		// Connect failed. Set a short time and call disconnect.
		module->FinConnect = -1;
		ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
		return;
	}

#ifdef _DEBUG
	#ifdef _POSIX
		if (errno != EINPROGRESS) // The result of the connect should always be "WOULD BLOCK" on Linux. But sometimes this fails.
		{
			// This happens when the interface is no longer available. Disconnect socket.
			module->FinConnect = -1;
			ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
			return;
		}
	#endif
	#ifdef WIN32
		if (GetLastError() != WSAEWOULDBLOCK) // The result of the connect should always be "WOULD BLOCK" on Windows.
		{
			// This happens when the interface is no longer available. Disconnect socket.
			module->FinConnect = -1;
			ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
			return;
		}
	#endif
#endif

	ILibForceUnBlockChain(module->Chain);
}

#ifdef MICROSTACK_PROXY
// Connect to a remote access using an HTTPS proxy. If "proxyAddress" is set to NULL, this call acts just to a normal connect call without a proxy.
void ILibAsyncSocket_ConnectToProxy(void* socketModule, struct sockaddr *localInterface, struct sockaddr *remoteAddress, struct sockaddr *proxyAddress, ILibAsyncSocket_OnInterrupt InterruptPtr, void *user)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	memset(&(module->ProxyAddress), 0, sizeof(struct sockaddr_in6));
	module->ProxyState = 0;
	if (proxyAddress != NULL) memcpy(&(module->ProxyAddress), proxyAddress, INET_SOCKADDR_LENGTH(proxyAddress->sa_family));
	ILibAsyncSocket_ConnectTo(socketModule, localInterface, remoteAddress, InterruptPtr, user);
}
#endif

//
// Internal method called when data is ready to be processed on an ILibAsyncSocket
//
// <param name="Reader">The ILibAsyncSocket with pending data</param>
void ILibProcessAsyncSocket(struct ILibAsyncSocketModule *Reader, int pendingRead)
{
	#ifndef MICROSTACK_NOTLS
	int ssllen;
	int sslstate;
	int sslerror;
	SSL *wasssl;
	#endif
	int iBeginPointer = 0;
	int iEndPointer = 0;
	int iPointer = 0;
	int bytesReceived = 0;
	int len;
	char *temp;

	//
	// If the thing isn't paused, and the user set the pointers such that we still have data
	// in our buffers, we need to call the user back with that data, before we attempt to read
	// more data off the network
	//
	if (!pendingRead)
	{
		if (Reader->internalSocket != ~0 && Reader->PAUSE <= 0 && Reader->BeginPointer != Reader->EndPointer)
		{
			iBeginPointer = Reader->BeginPointer;
			iEndPointer = Reader->EndPointer;
			iPointer = 0;

			while (Reader->internalSocket != ~0 && Reader->PAUSE <= 0 && Reader->BeginPointer != Reader->EndPointer && Reader->EndPointer != 0)
			{				
				Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
				Reader->BeginPointer = 0;
				if (Reader->OnData != NULL)
				{
					Reader->OnData(Reader, Reader->buffer + iBeginPointer, &(iPointer), Reader->EndPointer, &(Reader->OnInterrupt), &(Reader->user), &(Reader->PAUSE));
				}
				iBeginPointer += iPointer;
				Reader->EndPointer -= iPointer;
				if (iPointer == 0) break;
				iPointer = 0;
			}
			Reader->BeginPointer = iBeginPointer;
			Reader->EndPointer = iEndPointer;
		}
	}

	// Reading Body Only
	if (Reader->BeginPointer == Reader->EndPointer)
	{
		Reader->BeginPointer = 0;
		Reader->EndPointer = 0;
	}
	if (!pendingRead || Reader->PAUSE > 0) return;

	//
	// If we need to grow the buffer, do it now
	//
	if (bytesReceived > (Reader->MallocSize - Reader->EndPointer) || 1024 > (Reader->MallocSize - Reader->EndPointer))// the 1st portion is for ssl & cd
	{
		//
		// This memory reallocation sometimes causes Insure++
		// to incorrectly report a READ_DANGLING (usually in 
		// a call to ILibWebServer_StreamHeader_Raw.)
		// 
		// We verified that the problem is with Insure++ by
		// noting the value of 'temp' (0x008fa8e8), 
		// 'Reader->buffer' (0x00c55e80), and
		// 'MEMORYCHUNKSIZE' (0x00001800).
		//
		// When Insure++ reported the error, it (incorrectly) 
		// claimed that a pointer to memory address 0x00c55ea4
		// was invalid, while (correctly) citing the old memory
		// (0x008fa8e8-0x008fb0e7) as freed memory.
		// Normally Insure++ reports that the invalid pointer 
		// is pointing to someplace in the deallocated block,
		// but that wasn't the case.
		//
		if (Reader->MaxBufferSize == 0 || Reader->MallocSize < Reader->MaxBufferSize)
		{
			if (Reader->MaxBufferSize > 0 && (Reader->MaxBufferSize - Reader->MallocSize < MEMORYCHUNKSIZE))
			{
				Reader->MallocSize = Reader->MaxBufferSize;
			}
			else if (bytesReceived > 0)
			{
				Reader->MallocSize += bytesReceived - (Reader->MallocSize - Reader->EndPointer);
			}
			else
			{
				Reader->MallocSize += MEMORYCHUNKSIZE;
			}

			temp = Reader->buffer;
			Reader->buffer = (char*)realloc(Reader->buffer, Reader->MallocSize);
			//
			// If this realloc moved the buffer somewhere, we need to inform people of it
			//
			if (Reader->buffer != temp && Reader->OnBufferReAllocated != NULL) Reader->OnBufferReAllocated(Reader, Reader->user, Reader->buffer - temp);
		}
		else
		{
			//
			// If we grow the buffer anymore, it will exceed the maximum allowed buffer size
			//
			Reader->MaxBufferSizeExceeded = 1;
			if (Reader->OnBufferSizeExceeded != NULL) Reader->OnBufferSizeExceeded(Reader, Reader->MaxBufferSizeUserObject);
			ILibAsyncSocket_Disconnect(Reader);
			return;
		}
	}
	else if (Reader->BeginPointer != 0 && bytesReceived == 0)
	{
		//
		// We can save some cycles by moving the data back to the top
		// of the buffer, instead of just allocating more memory.
		//
		temp = Reader->buffer + Reader->BeginPointer;;
		memmove(Reader->buffer, temp, Reader->EndPointer-Reader->BeginPointer);
		Reader->EndPointer -= Reader->BeginPointer;
		Reader->BeginPointer = 0;

		//
		// Even though we didn't allocate new memory, we still moved data in the buffer, 
		// so we need to inform people of that, because it might be important
		//
		if (Reader->OnBufferReAllocated != NULL) Reader->OnBufferReAllocated(Reader, Reader->user, temp-Reader->buffer);
	}

	#ifndef MICROSTACK_NOTLS
	if (Reader->ssl != NULL)
	{
		// Read data off the SSL socket.

		// Now we will tell OpenSSL to process that data in the steam. This read may return nothing, but OpenSSL may
		// put data in the output buffer to be sent back out.
		bytesReceived = 0;
		do
		{
			// Read data from the SSL socket, this will read one SSL record at a time.
			ssllen = SSL_read(Reader->ssl, Reader->buffer+Reader->EndPointer + bytesReceived, Reader->MallocSize-Reader->EndPointer - bytesReceived);
			if (ssllen > 0) bytesReceived += ssllen;
		}
		while (ssllen > 0);

		// printf("SSL READ: LastLen = %d, Total = %d, State = %d, Error = %d\r\n", ssllen, bytesReceived, sslstate, sslerror);

		// Read the current SSL error. We do this only is no data was read, if we have any data lets process it.
		if (bytesReceived <= 0)
		{
			sslerror = SSL_get_error(Reader->ssl, ssllen);
			
			#ifdef WIN32
			if (!(sslerror == SSL_ERROR_WANT_READ || (sslerror == SSL_ERROR_SYSCALL && GetLastError() == 0)))
			{
				// There is no more data on the socket or error, shut it down.
				Reader->sslstate = 0;
				bytesReceived = -1;
			}
			#else
			if (!(sslerror == SSL_ERROR_WANT_READ || sslerror == SSL_ERROR_SYSCALL))
			{
				// There is no more data on the socket or error, shut it down.
				Reader->sslstate = 0;
				bytesReceived = -1;
			}
			#endif
		}

		sslstate = SSL_state(Reader->ssl);
		if (Reader->sslstate != 3 && sslstate == 3)
		{
			// If the SSL state changed to connected, we need to tell the application about the connection.
			Reader->sslstate = 3;
			if (Reader->SSLConnect == 0) // This is still a mistery, but if this check is not present, it's possible to signal connect more than once.
			{
				Reader->SSLConnect = 1;
				if (Reader->OnConnect != NULL) Reader->OnConnect(Reader, -1, Reader->user);
			}
		}
		if (bytesReceived == 0)
		{
			// We received no data, lets investigate why
			if (ssllen == 0 && bytesReceived == 0)
			{
				// There is no more data on the socket, shut it down.
				Reader->sslstate = 0;
				bytesReceived = -1;
			}
			else if (ssllen == -1 && sslstate == 0x2112)
			{
				// There is no more data on the socket, shut it down.
				Reader->sslstate = 0;
				bytesReceived = -1;
			}
			else return;
		}
	}
	else
	#endif
	{
		// Read data off the non-SSL, generic socket.
		// Set the receive address buffer size and read from the socket.
		len = sizeof(struct sockaddr_in6);
#if defined(WINSOCK2)
		bytesReceived = recvfrom(Reader->internalSocket, Reader->buffer+Reader->EndPointer, Reader->MallocSize-Reader->EndPointer, 0, (struct sockaddr*)&(Reader->SourceAddress), (int*)&len);
#else
		bytesReceived = recvfrom(Reader->internalSocket, Reader->buffer+Reader->EndPointer, Reader->MallocSize-Reader->EndPointer, 0, (struct sockaddr*)&(Reader->SourceAddress), (socklen_t*)&len);
#endif
	}

	sem_wait(&(Reader->SendLock));

	if (bytesReceived <= 0)
	{
		// If a UDP packet is larger than the buffer, drop it.
		#if defined(WINSOCK2)
		if (bytesReceived == SOCKET_ERROR && WSAGetLastError() == 10040) return;
		#else
		// TODO: Linux errno
		//if (bytesReceived == -1 && errno != 0) printf("ERROR: errno = %d, %s\r\n", errno, strerror(errno));
		#endif

		//
		// This means the socket was gracefully closed by the remote endpoint
		//
		SEM_TRACK(AsyncSocket_TrackLock("ILibProcessAsyncSocket", 1, Reader);)
		ILibAsyncSocket_ClearPendingSend(Reader);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibProcessAsyncSocket", 2, Reader);)

#if defined(_WIN32_WCE) || defined(WIN32)
#if defined(WINSOCK2)
		shutdown(Reader->internalSocket, SD_BOTH);
#endif
		closesocket(Reader->internalSocket);
#elif defined(_POSIX)
		shutdown(Reader->internalSocket,SHUT_RDWR);
		close(Reader->internalSocket);
#endif
		Reader->internalSocket = (SOCKET)~0;

		ILibAsyncSocket_ClearPendingSend(Reader);
		
		#ifndef MICROSTACK_NOTLS
		wasssl = Reader->ssl;
		if (Reader->ssl != NULL)
		{
			sem_post(&(Reader->SendLock));
			SSL_free(Reader->ssl); // Frees SSL session and BIO buffer at the same time
			sem_wait(&(Reader->SendLock));
			Reader->ssl = NULL;
			Reader->sslstate = 0;
			Reader->sslbio = NULL;
		}
		#endif
		sem_post(&(Reader->SendLock));

		//
		// Inform the user the socket has closed
		//
		#ifndef MICROSTACK_NOTLS
		if (wasssl != NULL)
		{
			// This was a SSL socket, fire the event notifying the user. Depending on connection state, we event differently
			if (Reader->SSLConnect == 0 && Reader->OnConnect != NULL) { Reader->OnConnect(Reader, 0, Reader->user); } // Connection Failed
			if (Reader->SSLConnect != 0 && Reader->OnDisconnect != NULL) { Reader->OnDisconnect(Reader, Reader->user); } // Socket Disconnected
		}
		else
		{
		#endif
			// This was a normal socket, fire the event notifying the user. Depending on connection state, we event differently
			if (Reader->FinConnect <= 0 && Reader->OnConnect != NULL) { Reader->OnConnect(Reader, 0, Reader->user); } // Connection Failed
			if (Reader->FinConnect > 0 && Reader->OnDisconnect != NULL) { Reader->OnDisconnect(Reader, Reader->user); } // Socket Disconnected
		#ifndef MICROSTACK_NOTLS
		}
		Reader->SSLConnect = 0;
		Reader->sslstate = 0;
		#endif
		Reader->FinConnect = 0;

		//
		// If we need to free the buffer, do so
		//
		if (Reader->buffer != NULL)
		{
			if (Reader->buffer != ILibScratchPad2) free(Reader->buffer);
			Reader->buffer = NULL;
			Reader->MallocSize = 0;
		}
	}
	else
	{
		sem_post(&(Reader->SendLock));

		//
		// Data was read, so increment our counters
		//
		Reader->EndPointer += bytesReceived;

		//
		// Tell the user we have some data
		//
		if (Reader->OnData != NULL)
		{
			iBeginPointer = Reader->BeginPointer;
			iPointer = 0;
			Reader->OnData(Reader, Reader->buffer + Reader->BeginPointer, &(iPointer), Reader->EndPointer - Reader->BeginPointer, &(Reader->OnInterrupt), &(Reader->user),&(Reader->PAUSE));
			Reader->BeginPointer += iPointer;
		}
		//
		// If the user set the pointers, and we still have data, call them back with the data
		//
		if (Reader->internalSocket != ~0 && Reader->PAUSE <= 0 && Reader->BeginPointer!=Reader->EndPointer && Reader->BeginPointer != 0)
		{
			iBeginPointer = Reader->BeginPointer;
			iEndPointer = Reader->EndPointer;
			iPointer = 0;

			while (Reader->internalSocket != ~0 && Reader->PAUSE <= 0 && Reader->BeginPointer != Reader->EndPointer && Reader->EndPointer != 0)
			{				
				Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
				Reader->BeginPointer = 0;
				if (Reader->OnData != NULL)
				{
					Reader->OnData(Reader, Reader->buffer + iBeginPointer, &(iPointer), Reader->EndPointer, &(Reader->OnInterrupt), &(Reader->user), &(Reader->PAUSE));
				}
				iBeginPointer += iPointer;
				Reader->EndPointer -= iPointer;
				if (iPointer == 0) break;
				iPointer = 0;
			}
			Reader->BeginPointer = iBeginPointer;
			Reader->EndPointer = iEndPointer;
		}

		//
		// If the user consumed all of the buffer, we can recycle it
		//
		if (Reader->BeginPointer == Reader->EndPointer)
		{
			Reader->BeginPointer = 0;
			Reader->EndPointer = 0;
		}
	}
}

/*! \fn ILibAsyncSocket_GetUser(ILibAsyncSocket_SocketModule socketModule)
\brief Returns the user object
\param socketModule The ILibAsyncSocket token to fetch the user object from
\returns The user object
*/
void *ILibAsyncSocket_GetUser(ILibAsyncSocket_SocketModule socketModule)
{
	return(socketModule == NULL?NULL:((struct ILibAsyncSocketModule*)socketModule)->user);
}

void ILibAsyncSocket_SetUser(ILibAsyncSocket_SocketModule socketModule, void* user)
{
	if (socketModule == NULL) return;
	((struct ILibAsyncSocketModule*)socketModule)->user = user;
}

void *ILibAsyncSocket_GetUser2(ILibAsyncSocket_SocketModule socketModule)
{
	return(socketModule == NULL?NULL:((struct ILibAsyncSocketModule*)socketModule)->user2);
}

void ILibAsyncSocket_SetUser2(ILibAsyncSocket_SocketModule socketModule, void* user2)
{
	if (socketModule == NULL) return;
	((struct ILibAsyncSocketModule*)socketModule)->user2 = user2;
}

int ILibAsyncSocket_GetUser3(ILibAsyncSocket_SocketModule socketModule)
{
	return(socketModule == NULL?-1:((struct ILibAsyncSocketModule*)socketModule)->user3);
}

void ILibAsyncSocket_SetUser3(ILibAsyncSocket_SocketModule socketModule, int user3)
{
	if (socketModule == NULL) return;
	((struct ILibAsyncSocketModule*)socketModule)->user3 = user3;
}

//
// Chained PreSelect handler for ILibAsyncSocket
//
// <param name="readset"></param>
// <param name="writeset"></param>
// <param name="errorset"></param>
// <param name="blocktime"></param>
void ILibAsyncSocket_PreSelect(void* socketModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	if (module->internalSocket == -1) return; // If there is not internal socket, just return now.

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PreSelect", 1, module);)
	sem_wait(&(module->SendLock));

	if (module->internalSocket != -1)
	{
		if (module->PAUSE < 0) *blocktime = 0;
		if (module->FinConnect == 0)
		{
			// Not Connected Yet
			#if defined(WIN32)
			#pragma warning( push, 3 ) // warning C4127: conditional expression is constant
			#endif
			FD_SET(module->internalSocket, writeset);
			FD_SET(module->internalSocket, errorset);
			#if defined(WIN32)
			#pragma warning( pop )
			#endif
		}
		else
		{
			if (module->PAUSE == 0) // Only if this is zero. <0 is resume, so we want to process first
			{
				// Already Connected, just needs reading
				#if defined(WIN32)
				#pragma warning( push, 3 ) // warning C4127: conditional expression is constant
				#endif
				FD_SET(module->internalSocket, readset);
				FD_SET(module->internalSocket, errorset);
				#if defined(WIN32)
				#pragma warning( pop )
				#endif
			}
		}

		if (module->PendingSend_Head != NULL)
		{
			// If there is pending data to be sent, then we need to check when the socket is writable
			#if defined(WIN32)
			#pragma warning( push, 3 ) // warning C4127: conditional expression is constant
			#endif
			FD_SET(module->internalSocket, writeset);
			#if defined(WIN32)
			#pragma warning( pop )
			#endif
		}
	}
	SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PreSelect", 2, module);)
	sem_post(&(module->SendLock));
}

//
// Chained PostSelect handler for ILibAsyncSocket
//
// <param name="socketModule"></param>
// <param name="slct"></param>
// <param name="readset"></param>
// <param name="writeset"></param>
// <param name="errorset"></param>
void ILibAsyncSocket_PostSelect(void* socketModule, int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	int TriggerSendOK = 0;
	struct ILibAsyncSocket_SendData *temp;
	int bytesSent = 0;
	int flags, len;
	int TRY_TO_SEND = 1;
	int triggerReadSet = 0;
	int triggerResume = 0;
	int triggerWriteSet = 0;
	int serr = 0, serrlen = sizeof(serr);
	#ifndef MICROSTACK_NOTLS
	SSL *wasssl;
	#endif

	UNREFERENCED_PARAMETER( slct );

	if (module->internalSocket == -1 || module->FinConnect == -1) return; // If there is not internal socket, just return now.
	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PostSelect", 1, module);)
	sem_wait(&(module->SendLock)); // Lock!

	//
	// Error Handling. If the ERROR flag is set we have a problem. If not, we must check the socket status for an error.
	// Yes, this is odd, but it's possible for a socket to report a read set and still have an error, in this past this
	// was not handled and caused a lot of problems.
	//
	if (FD_ISSET(module->internalSocket, errorset) != 0)
	{
		serr = 1;
	}
	else
	{
		// Fetch the socket error code
#if defined(WINSOCK2)
		getsockopt(module->internalSocket, SOL_SOCKET, SO_ERROR, (char*)&serr, (int*)&serrlen);
#else
		getsockopt(module->internalSocket, SOL_SOCKET, SO_ERROR, (char*)&serr, (socklen_t*)&serrlen);
#endif
	}

	#ifdef MICROSTACK_PROXY
	// Handle proxy
	if (module->FinConnect == 1 && module->ProxyState == 1 && serr == 0)
	{
		// We need to read the proxy response, all of it and not a byte more.
		if (FD_ISSET(module->internalSocket, readset) != 0)
		{
			char *ptr1, *ptr2;
			int len2, len3;
			serr = 555; // Fake proxy error
			len2 = recv(module->internalSocket, ILibScratchPad2, 1024, MSG_PEEK);
			if (len2 > 0 && len2 < 1024)
			{
				ILibScratchPad2[len2] = 0;
				ptr1 = strstr(ILibScratchPad2, "\r\n\r\n");
				ptr2 = strstr(ILibScratchPad2, " 200 ");
				if (ptr1 != NULL && ptr2 != NULL && ptr2 < ptr1)
				{
					len3 = (ptr1 + 4) - ILibScratchPad2;
					recv(module->internalSocket, ILibScratchPad2, len3, 0);
					module->FinConnect = 0; // Let pretend we never connected, this will trigger all the connection stuff.
					module->ProxyState = 2; // Move the proxy connection state forward.
					serr = 0;				// Proxy connected collectly.
				}
			}
		}
	}
	#endif

	// If there are any errors, shutdown this socket
	if (serr != 0)
	{
		// If this is an SSL socket, close down the SSL state
		#ifndef MICROSTACK_NOTLS
		if ((wasssl = module->ssl) != NULL)
		{
			sem_post(&(module->SendLock));
			SSL_free(module->ssl); // Frees SSL session and BIO buffer at the same time
			sem_wait(&(module->SendLock));
			module->ssl = NULL;
			module->sslstate = 0;
			module->sslbio = NULL;
		}
		#endif

		// Now shutdown the socket and set it to zero
		#if defined(_WIN32_WCE) || defined(WIN32)
		#if defined(WINSOCK2)
			shutdown(module->internalSocket, SD_BOTH);
		#endif
			closesocket(module->internalSocket);
		#elif defined(_POSIX)
			shutdown(module->internalSocket, SHUT_RDWR);
			close(module->internalSocket);
		#endif
		module->internalSocket = (SOCKET)~0;
		
		// Unlock before fireing the event
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 4, module);)
		sem_post(&(module->SendLock));

		#ifndef MICROSTACK_NOTLS
		if (wasssl != NULL)
		{
			// This was a SSL socket, fire the event notifying the user. Depending on connection state, we event differently
			if (module->SSLConnect == 0 && module->OnConnect != NULL) { module->OnConnect(module, 0, module->user); } // Connection Failed
			if (module->SSLConnect != 0 && module->OnDisconnect != NULL) { module->OnDisconnect(module, module->user); } // Socket Disconnected
		}
		else
		{
		#endif
			// This was a normal socket, fire the event notifying the user. Depending on connection state, we event differently
			if (module->FinConnect <= 0 && module->OnConnect != NULL) { module->OnConnect(module, 0, module->user); } // Connection Failed
			if (module->FinConnect > 0 && module->OnDisconnect != NULL) { module->OnDisconnect(module, module->user); } // Socket Disconnected
		#ifndef MICROSTACK_NOTLS
		}
		module->SSLConnect = 0;
		module->sslstate = 0;
		#endif
		module->FinConnect = 0;
	}
	else
	{
		// There are no errors, lets keep processing the socket normally
		if (module->FinConnect == 0)
		{
			// Check to see if the socket is connected
#ifdef MICROSTACK_PROXY
			if (FD_ISSET(module->internalSocket, writeset) != 0 || module->ProxyState == 2)
#else
			if (FD_ISSET(module->internalSocket, writeset) != 0)
#endif
			{
				// Connected
				len = sizeof(struct sockaddr_in6);
#if defined(WINSOCK2)
				getsockname(module->internalSocket, (struct sockaddr*)(&module->LocalAddress), (int*)&len);
#else
				getsockname(module->internalSocket, (struct sockaddr*)(&module->LocalAddress), (socklen_t*)&len);
#endif
				module->FinConnect = 1;
				module->PAUSE = 0;

				// Set the socket to non-blocking mode, so we can play nice and share the thread
				#if defined(_WIN32_WCE) || defined(WIN32)
				flags = 1;
				ioctlsocket(module->internalSocket, FIONBIO, (u_long *)(&flags));
				#elif defined(_POSIX)
				flags = fcntl(module->internalSocket, F_GETFL,0);
				fcntl(module->internalSocket, F_SETFL, O_NONBLOCK|flags);
				#endif

				// If this is a proxy connection, send the proxy connect header now.
#ifdef MICROSTACK_PROXY
				if (module->ProxyAddress.sin6_family != 0 && module->ProxyState == 0)
				{
					int len2, len3;
					ILibInet_ntop((int)(module->RemoteAddress.sin6_family), (void*)&(((struct sockaddr_in*)&(module->RemoteAddress))->sin_addr), ILibScratchPad, 4096);
					len2 = snprintf(ILibScratchPad2, 4096, "CONNECT %s:%d HTTP/1.1\r\nProxy-Connection: keep-alive\r\nHost: %s\r\n\r\n", ILibScratchPad, ntohs(module->RemoteAddress.sin6_port), ILibScratchPad);
					len3 = send(module->internalSocket, ILibScratchPad2, len2, MSG_NOSIGNAL);
					module->ProxyState = 1;
					// TODO: Set timeout. If the proxy does not respond, we need to close this connection.
					// On the other hand... This is not generally a problem, proxies will disconnect after a timeout anyway.
					
					SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 4, module);)
					sem_post(&(module->SendLock));
					return;
				}
				if (module->ProxyState == 2) module->ProxyState = 3;
#endif

				// Connection Complete
				triggerWriteSet = 1;
			}

			// Unlock before fireing the event
			SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 4, module);)
			sem_post(&(module->SendLock));

			// If we did connect, we got more things to do
			if (triggerWriteSet != 0)
			{
				#ifndef MICROSTACK_NOTLS
				if (module->ssl_ctx != NULL)
				{
					/* 	=// Make this call to setup the SSL stuff (Ylian added this to version 3769...)
					ILibAsyncSocket_SetSSLContext(module, module->ssl_ctx, 0);
					*/
					if (module->ssl_session != NULL)  // vbl added client-side session caching
					{
						printf("reusing session2 ");
						SSL_set_session(module->ssl, module->ssl_session);
						SSL_SESSION_free(module->ssl_session);
						module->ssl_session = NULL; // not sure if I should do this here...
					} else {
						printf("no prior session2 ");
					}
					// If this is an SSL socket, launch the SSL connection process
					if ((serr = SSL_connect(module->ssl)) != 1 && SSL_get_error(module->ssl, serr) != SSL_ERROR_WANT_READ)			// TODO: On Linux it's possible to get BROKEN PIPE on SSL_connect()
					{
						PRINTERROR();
						ILIBCRITICALEXIT(SSL_get_error(module->ssl, serr));
					}
				}
				else
				#endif
				{
					// If this is a normal socket, event the connection now.
					if (module->OnConnect != NULL) module->OnConnect(module, -1, module->user);
				}
			}
		}
		else
		{
			// Connected socket, we need to read data
			if (FD_ISSET(module->internalSocket, readset) != 0)
			{
				triggerReadSet = 1; // Data Available
			}
			else if (module->PAUSE < 0)
			{
				// Someone resumed a paused connection, but the FD_SET was not triggered because there is no new data on the socket.
				triggerResume = 1;
				++module->PAUSE;
			}

			// Unlock before fireing the event
			SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 4, module);)
			sem_post(&(module->SendLock));

			if (triggerReadSet != 0 || triggerResume != 0) ILibProcessAsyncSocket(module, triggerReadSet);
		}
	}
	SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 4, module);)


	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PostSelect", 1, module);)
	sem_wait(&(module->SendLock));

	// Write Handling
	if (module->FinConnect > 0 && module->internalSocket != ~0 && FD_ISSET(module->internalSocket, writeset) != 0 && module->PendingSend_Head != NULL)
	{
		//
		// Keep trying to send data, until we are told we can't
		//
		while (TRY_TO_SEND != 0)
		{
			if (module->PendingSend_Head == NULL) break;
			#ifndef MICROSTACK_NOTLS
			if (module->ssl != NULL)
			{
				// Send on SSL socket
				bytesSent = SSL_write(module->ssl, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize - module->PendingSend_Head->bytesSent);
			}
			else
			#endif
			if (module->PendingSend_Head->remoteAddress.sin6_family == 0)
			{
				bytesSent = send(module->internalSocket, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize - module->PendingSend_Head->bytesSent, MSG_NOSIGNAL);
			}
			else
			{
				bytesSent = sendto(module->internalSocket, module->PendingSend_Head->buffer + module->PendingSend_Head->bytesSent, module->PendingSend_Head->bufferSize - module->PendingSend_Head->bytesSent, MSG_NOSIGNAL, (struct sockaddr*)&module->PendingSend_Head->remoteAddress, INET_SOCKADDR_LENGTH(module->PendingSend_Head->remoteAddress.sin6_family));
			}

			if (bytesSent > 0)
			{
				module->PendingBytesToSend -= bytesSent;
				module->TotalBytesSent += bytesSent;
				module->PendingSend_Head->bytesSent += bytesSent;
				if (module->PendingSend_Head->bytesSent == module->PendingSend_Head->bufferSize)
				{
					// Finished Sending this block
					if (module->PendingSend_Head == module->PendingSend_Tail)
					{
						module->PendingSend_Tail = NULL;
					}
					if (module->PendingSend_Head->UserFree == 0)
					{
						free(module->PendingSend_Head->buffer);
					}
					temp = module->PendingSend_Head->Next;
					free(module->PendingSend_Head);
					module->PendingSend_Head = temp;
					if (module->PendingSend_Head == NULL) { TRY_TO_SEND = 0; }
				}
				else
				{
					//
					// We sent data, but not everything that needs to get sent was sent, try again
					//
					TRY_TO_SEND = 1;
				}
			}
			#ifndef MICROSTACK_NOTLS
			if (bytesSent == -1 && module->ssl == NULL)
			#else
			if (bytesSent == -1)
			#endif
			{
				// Error, clean up everything
				TRY_TO_SEND = 0;
#if defined(_WIN32_WCE) || defined(WIN32)
				if (WSAGetLastError() != WSAEWOULDBLOCK)
#elif defined(_POSIX)
				if (errno != EWOULDBLOCK)
#endif
				{
					//
					// There was an error sending
					//
					ILibAsyncSocket_ClearPendingSend(socketModule);
					ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
				}
			}
			#ifndef MICROSTACK_NOTLS
			else if (bytesSent == -1 && module->ssl != NULL)
			{
				// OpenSSL returned an error
				bytesSent = SSL_get_error(module->ssl, bytesSent);
				if (bytesSent != SSL_ERROR_WANT_WRITE)
				{
					//
					// There was an error sending
					//
					ILibAsyncSocket_ClearPendingSend(socketModule);
					ILibLifeTime_Add(module->LifeTime, socketModule, 0, &ILibAsyncSocket_Disconnect, NULL);
				}
			}
			#endif
		}
		//
		// This triggers OnSendOK, if all the pending data has been sent.
		//
		if (module->PendingSend_Head == NULL && bytesSent != -1) { TriggerSendOK = 1; }
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 2, module);)
		sem_post(&(module->SendLock));
		if (TriggerSendOK != 0) module->OnSendOK(module, module->user);
	}
	else
	{
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect", 2, module);)
		sem_post(&(module->SendLock));
	}

}

/*! \fn ILibAsyncSocket_IsFree(ILibAsyncSocket_SocketModule socketModule)
\brief Determines if an ILibAsyncSocket is in use
\param socketModule The ILibAsyncSocket to query
\returns 0 if in use, nonzero otherwise
*/
int ILibAsyncSocket_IsFree(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->internalSocket==~0?1:0);
}

int ILibAsyncSocket_IsConnected(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return module->FinConnect;
}

/*! \fn ILibAsyncSocket_GetPendingBytesToSend(ILibAsyncSocket_SocketModule socketModule)
\brief Returns the number of bytes that are pending to be sent
\param socketModule The ILibAsyncSocket to query
\returns Number of pending bytes
*/
unsigned int ILibAsyncSocket_GetPendingBytesToSend(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->PendingBytesToSend);
}

/*! \fn ILibAsyncSocket_GetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
\brief Returns the total number of bytes that have been sent, since the last reset
\param socketModule The ILibAsyncSocket to query
\returns Number of bytes sent
*/
unsigned int ILibAsyncSocket_GetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->TotalBytesSent);
}

/*! \fn ILibAsyncSocket_ResetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
\brief Resets the total bytes sent counter
\param socketModule The ILibAsyncSocket to reset
*/
void ILibAsyncSocket_ResetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	module->TotalBytesSent = 0;
}

/*! \fn ILibAsyncSocket_GetBuffer(ILibAsyncSocket_SocketModule socketModule, char **buffer, int *BeginPointer, int *EndPointer)
\brief Returns the buffer associated with an ILibAsyncSocket
\param socketModule The ILibAsyncSocket to obtain the buffer from
\param[out] buffer The buffer
\param[out] BeginPointer Stating offset of the buffer
\param[out] EndPointer Length of buffer
*/
void ILibAsyncSocket_GetBuffer(ILibAsyncSocket_SocketModule socketModule, char **buffer, int *BeginPointer, int *EndPointer)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;

	*buffer = module->buffer;
	*BeginPointer = module->BeginPointer;
	*EndPointer = module->EndPointer;
}

void ILibAsyncSocket_ModuleOnConnect(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
	if (module != NULL && module->OnConnect != NULL) module->OnConnect(module, -1, module->user);
}


#ifndef MICROSTACK_NOTLS

// these stubbed-out functions are placeholders for future work for more complete session cacheing.
int new_session_cb(SSL *ctx, SSL_SESSION *session)
{
	return 1;
}
void remove_session_cb(SSL *ctx, SSL_SESSION *session)
{
	int i = 1;
}
SSL_SESSION * get_session_cb(SSL *ctx, unsigned char *id, int len, int *ref)
{
	*ref = 0;
	return NULL;
}

// Set the SSL client context used by all connections done by this socket module. The SSL context must
// be set before using this module. If left to NULL, all connections are in the clear using TCP.
//
// This is utilized by the ILibAsyncServerSocket module
// <param name="socketModule">The ILibAsyncSocket to modify</param>
// <param name="ssl_ctx">The ssl_ctx structure</param>

void ILibAsyncSocket_SetSSLContext(ILibAsyncSocket_SocketModule socketModule, SSL_CTX *ssl_ctx, int server)
{
	if (socketModule != NULL)
	{
		struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
		if (ssl_ctx == NULL) return;

		if (module->ssl_ctx == NULL)
		{
			module->ssl_ctx = ssl_ctx;
			SSL_CTX_set_mode(ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
		}

		// If a socket is ready, setup SSL right now (otherwise, we will do this upon connection).
		if (module->internalSocket != 0 && module->internalSocket != ~0 && module->ssl == NULL)
		{
			module->ssl = SSL_new(ssl_ctx);
			module->sslstate = 0;
			module->sslbio = BIO_new_socket((int)(module->internalSocket), BIO_NOCLOSE);	// This is an odd conversion from SOCKET (possible 64bit) to 32 bit integer, but has to be done.
			SSL_set_bio(module->ssl, module->sslbio, module->sslbio);

			if (server != 0) 
			{
				SSL_set_accept_state(module->ssl); // Setup server SSL state
				// vbl: setting the session id context turns on server-side session caching
				
				printf("\nturning on server session caching\n");
				SSL_CTX_set_session_id_context(ssl_ctx,(const unsigned char *) "Device", (unsigned int) strlen("Device"));
				SSL_CTX_sess_set_new_cb(ssl_ctx, new_session_cb);
				SSL_CTX_sess_set_remove_cb(ssl_ctx, remove_session_cb);
				SSL_CTX_sess_set_get_cb(ssl_ctx, get_session_cb);
		
			} 
			else 
			{
				SSL_set_connect_state(module->ssl); // Setup client SSL state
			}
		}
	}
}
#endif

//
// Sets the remote address field
//
// This is utilized by the ILibAsyncServerSocket module
// <param name="socketModule">The ILibAsyncSocket to modify</param>
// <param name="RemoteAddress">The remote interface</param>
void ILibAsyncSocket_SetRemoteAddress(ILibAsyncSocket_SocketModule socketModule, struct sockaddr *remoteAddress)
{
	if (socketModule != NULL)
	{
		struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
		memcpy(&(module->RemoteAddress), remoteAddress, INET_SOCKADDR_LENGTH(remoteAddress->sa_family));
	}
}

/*! \fn ILibAsyncSocket_UseThisSocket(ILibAsyncSocket_SocketModule socketModule,void* UseThisSocket,ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
\brief Associates an actual socket with ILibAsyncSocket
\par
Instead of calling \a ConnectTo, you can call this method to associate with an already
connected socket.
\param socketModule The ILibAsyncSocket to associate
\param UseThisSocket The socket to associate
\param InterruptPtr Function Pointer that triggers when the TCP connection is interrupted
\param user User object to associate with this session
*/
void ILibAsyncSocket_UseThisSocket(ILibAsyncSocket_SocketModule socketModule, void* UseThisSocket, ILibAsyncSocket_OnInterrupt InterruptPtr, void *user)
{
#if defined(_WIN32_WCE) || defined(WIN32)
	SOCKET TheSocket = *((SOCKET*)UseThisSocket);
#elif defined(_POSIX)
	int TheSocket = *((int*)UseThisSocket);
#endif
	int flags;
	char *tmp;
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;

	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->internalSocket = TheSocket;
	module->OnInterrupt = InterruptPtr;
	module->user = user;
	module->FinConnect = 1;
	module->PAUSE = 0;
	#ifndef MICROSTACK_NOTLS
	module->SSLConnect = 0;
	#endif

	//
	// If the buffer is too small/big, we need to realloc it to the minimum specified size
	//
	if (module->buffer != ILibScratchPad2)
	{
		if ((tmp = (char*)realloc(module->buffer, module->InitialSize)) == NULL) ILIBCRITICALEXIT(254);
		module->buffer = tmp;
		module->MallocSize = module->InitialSize;
	}
	module->BeginPointer = 0;
	module->EndPointer = 0;

	#ifndef MICROSTACK_NOTLS
	if (module->ssl_ctx != NULL)
	{
		module->ssl = SSL_new(module->ssl_ctx);
		module->sslstate = 0;
		module->sslbio = BIO_new_socket((int)(module->internalSocket), BIO_NOCLOSE);	// This is an odd conversion from SOCKET (possible 64bit) to 32 bit integer, but has to be done.
		SSL_set_bio(module->ssl, module->sslbio, module->sslbio);
		SSL_set_accept_state(module->ssl); // Setup server SSL state
	}
	#endif

	//
	// Make sure the socket is non-blocking, so we can play nice and share the thread
	//
#if defined(_WIN32_WCE) || defined(WIN32)
	flags = 1;
	ioctlsocket(module->internalSocket, FIONBIO,(u_long *)(&flags));
#elif defined(_POSIX)
	flags = fcntl(module->internalSocket,F_GETFL,0);
	fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
#endif
}

/*! \fn ILibAsyncSocket_GetRemoteInterface(ILibAsyncSocket_SocketModule socketModule)
\brief Returns the Remote Interface of a connected session
\param socketModule The ILibAsyncSocket to query
\returns The remote interface
*/
int ILibAsyncSocket_GetRemoteInterface(ILibAsyncSocket_SocketModule socketModule, struct sockaddr *remoteAddress)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	if (module->RemoteAddress.sin6_family != 0)
	{
		memcpy(remoteAddress, &(module->RemoteAddress), INET_SOCKADDR_LENGTH(module->RemoteAddress.sin6_family));
		return INET_SOCKADDR_LENGTH(module->RemoteAddress.sin6_family);
	}
	memcpy(remoteAddress, &(module->SourceAddress), INET_SOCKADDR_LENGTH(module->SourceAddress.sin6_family));
	return INET_SOCKADDR_LENGTH(module->SourceAddress.sin6_family);
}

/*! \fn ILibAsyncSocket_GetLocalInterface(ILibAsyncSocket_SocketModule socketModule)
\brief Returns the Local Interface of a connected session, in network order
\param socketModule The ILibAsyncSocket to query
\returns The local interface
*/
int ILibAsyncSocket_GetLocalInterface(ILibAsyncSocket_SocketModule socketModule, struct sockaddr *localAddress)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	int receivingAddressLength = sizeof(struct sockaddr_in6);

	if (module->LocalAddress.sin6_family !=0)
	{
		memcpy(localAddress, &(module->LocalAddress), INET_SOCKADDR_LENGTH(module->LocalAddress.sin6_family));
		return INET_SOCKADDR_LENGTH(module->LocalAddress.sin6_family);
	}
	else
	{
#if defined(WINSOCK2)
		getsockname(module->internalSocket, localAddress, (int*)&receivingAddressLength);
#else
		getsockname(module->internalSocket, localAddress, (socklen_t*)&receivingAddressLength);
#endif
		return receivingAddressLength;
	}
}

/*! \fn ILibAsyncSocket_Resume(ILibAsyncSocket_SocketModule socketModule)
\brief Resumes a paused session
\par
Sessions can be paused, such that further data is not read from the socket until resumed
\param socketModule The ILibAsyncSocket to resume
*/
void ILibAsyncSocket_Resume(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	if (sm!=NULL)
	{
		sm->PAUSE = -1;
		ILibForceUnBlockChain(sm->Chain);
	}
}

/*! \fn ILibAsyncSocket_GetSocket(ILibAsyncSocket_SocketModule module)
\brief Obtain the underlying raw socket
\param module The ILibAsyncSocket to query
\returns The raw socket
*/
void* ILibAsyncSocket_GetSocket(ILibAsyncSocket_SocketModule module)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	return(&(sm->internalSocket));
}

void ILibAsyncSocket_SetLocalInterface(ILibAsyncSocket_SocketModule module, struct sockaddr *LocalAddress)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	memcpy(&(sm->LocalAddress), LocalAddress, INET_SOCKADDR_LENGTH(LocalAddress->sa_family));
}

void ILibAsyncSocket_SetMaximumBufferSize(ILibAsyncSocket_SocketModule module, int maxSize, ILibAsyncSocket_OnBufferSizeExceeded OnBufferSizeExceededCallback, void *user)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	sm->MaxBufferSize = maxSize;
	sm->OnBufferSizeExceeded = OnBufferSizeExceededCallback;
	sm->MaxBufferSizeUserObject = user;
}

void ILibAsyncSocket_SetSendOK(ILibAsyncSocket_SocketModule module, ILibAsyncSocket_OnSendOK OnSendOK)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	sm->OnSendOK = OnSendOK;
}

int ILibAsyncSocket_IsIPv6LinkLocal(struct sockaddr *LocalAddress)
{
	struct sockaddr_in6 *x = (struct sockaddr_in6*)LocalAddress;
#if defined(_WIN32_WCE) || defined(WIN32)
	if (LocalAddress->sa_family == AF_INET6 && x->sin6_addr.u.Byte[0] == 0xFE && x->sin6_addr.u.Byte[1] == 0x80) return 1;
#else
	if (LocalAddress->sa_family == AF_INET6 && x->sin6_addr.s6_addr[0] == 0xFE && x->sin6_addr.s6_addr[1] == 0x80) return 1;
#endif
	return 0;
}

int ILibAsyncSocket_IsModuleIPv6LinkLocal(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return ILibAsyncSocket_IsIPv6LinkLocal((struct sockaddr*)&(module->LocalAddress));
}

int ILibAsyncSocket_WasClosedBecauseBufferSizeExceeded(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	return(sm->MaxBufferSizeExceeded);
}

#ifndef MICROSTACK_NOTLS
X509 *ILibAsyncSocket_SslGetCert(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	return SSL_get_peer_certificate(sm->ssl);
}

STACK_OF(X509) *ILibAsyncSocket_SslGetCerts(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	return SSL_get_peer_cert_chain(sm->ssl);
}

// vbl added this
SSL_SESSION *ILibAsyncSocket_SslCacheSession(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	
	sm->ssl_session = SSL_get1_session(sm->ssl);

	if (sm->ssl_session == NULL) {
		printf("sess=NULL ");
	} else {
		unsigned int idlen = sm->ssl_session->session_id_length;
		if (idlen >= 16) {
			printf("sess id=%x%x%x%x%x%x%x%x\n",sm->ssl_session->session_id[0], sm->ssl_session->session_id[1],
				sm->ssl_session->session_id[2], sm->ssl_session->session_id[3],
				sm->ssl_session->session_id[4], sm->ssl_session->session_id[5],
				sm->ssl_session->session_id[6], sm->ssl_session->session_id[7]);
		}
		printf("sess ");
	}
	return sm->ssl_session;
}

#endif

