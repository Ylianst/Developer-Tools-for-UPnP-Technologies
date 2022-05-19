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

#ifndef __ILibWebClient__
#define __ILibWebClient__

#include "ILibParsers.h"
#include "ILibAsyncSocket.h"

#ifndef MICROSTACK_NOTLS
#include <openssl/x509v3.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! \file ILibWebClient.h 
	\brief MicroStack APIs for HTTP Client functionality
*/

#define ILibWebClientIsStatusOk(statusCode) (statusCode>=200 && statusCode<=299)

/*! \defgroup ILibWebClient ILibWebClient Module
	\{
*/

/*! \def WEBCLIENT_DESTROYED
	\brief The ILibWebClient object was destroyed
*/
#define WEBCLIENT_DESTROYED 5
/*! \def WEBCLIENT_DELETED
	\brief The ILibWebClient object was deleted with a call to ILibWebClient_DeleteRequests
*/
#define WEBCLIENT_DELETED 6

#if defined(WIN32) || defined(_WIN32_WCE)
#include <STDDEF.h>
#elif defined(_POSIX)
#if !defined(__APPLE__)
#include <malloc.h>
#endif
#endif

enum ILibWebClient_DoneCode
{
	ILibWebClient_DoneCode_NoErrors = 1,
	ILibWebClient_DoneCode_StillReading = 0,
	ILibWebClient_DoneCode_HeaderTooBig = -2,
	ILibWebClient_DoneCode_BodyTooBig = -3,
};

enum ILibWebClient_Range_Result
{
	ILibWebClient_Range_Result_OK = 0,
	ILibWebClient_Range_Result_INVALID_RANGE = 1,
	ILibWebClient_Range_Result_BAD_REQUEST = 2,
};
/*! \typedef ILibWebClient_RequestToken
	\brief The handle for a request, obtained from a call to \a ILibWebClient_PipelineRequest
*/
typedef void* ILibWebClient_RequestToken;

/*! \typedef ILibWebClient_RequestManager
	\brief An object that manages HTTP Client requests. Obtained from \a ILibCreateWebClient
*/
typedef void* ILibWebClient_RequestManager;

/*! \typedef ILibWebClient_StateObject
	\brief Handle for an HTTP Client Connection.
*/
typedef void* ILibWebClient_StateObject;

/*! \typedef ILibWebClient_OnResponse
	\brief Function Callback Pointer, dispatched to process received data
	\param WebStateObject The ILibWebClient that received a response
	\param InterruptFlag A flag indicating if this request was interrupted with a call to ILibStopChain
	\param header The packetheader object containing the HTTP headers.
	\param bodyBuffer The buffer containing the body
	\param[in,out] beginPointer The start index of the buffer referenced by \a bodyBuffer
	\param endPointer The end index of the buffer referenced by \a bodyBuffer
	\param done Flag indicating if all of the response has been received. (0 = More data to be received, NonZero = Done)
	\param user1 
	\param user2
	\param[out] PAUSE Set to nonzero value if you do not want the system to read any more data from the network
*/
typedef void(*ILibWebClient_OnResponse)(ILibWebClient_StateObject WebStateObject,int InterruptFlag,struct packetheader *header,char *bodyBuffer,int *beginPointer,int endPointer,int done,void *user1,void *user2,int *PAUSE);
/*! \typedef ILibWebClient_OnSendOK
	\brief Handler for when pending send operations have completed
	\par
	<B>Note:</B> This handler will only be called after all data from any call(s) to \a ILibWebClient_StreamRequestBody
	have completed.
	\param sender The \a ILibWebClient_StateObject whos pending sends have completed
	\param user1 User1 object that was associated with this connection
	\param user2 User2 object that was associated with this connection
*/
typedef void(*ILibWebClient_OnSendOK)(ILibWebClient_StateObject sender, void *user1, void *user2);
/*! \typedef ILibWebClient_OnDisconnect
	\brief Handler for when the session has disconnected
	\param sender The \a ILibWebClient_StateObject that has been disconnected
	\param request The ILibWebClient_RequestToken that represents the request that was in progress when the disconnect happened.
*/
typedef void(*ILibWebClient_OnDisconnect)(ILibWebClient_StateObject sender, ILibWebClient_RequestToken request);

#ifndef MICROSTACK_NOTLS
typedef int(*ILibWebClient_OnSslConnection)(ILibWebClient_StateObject sender, STACK_OF(X509) *certs, struct sockaddr_in6 *address, void *user);
#endif

//
// This is the number of seconds that a connection must be idle for, before it will
// be automatically closed. Idle means there are no pending requests
//
/*! \def HTTP_SESSION_IDLE_TIMEOUT
	\brief This is the number of seconds that a connection must be idle for, before it will be automatically closed.
	\par
	Idle means there are no pending requests
*/
#define HTTP_SESSION_IDLE_TIMEOUT 10

/*! \def HTTP_CONNECT_RETRY_COUNT
	\brief This is the number of times, an HTTP connection will be attempted, before it fails.
	\par
	This module utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will retry after 1 second, then 2, then 4, etc.
*/
#define HTTP_CONNECT_RETRY_COUNT 2

/*! \def INITIAL_BUFFER_SIZE
	\brief This initial size of the receive buffer
*/
#define INITIAL_BUFFER_SIZE 65535


ILibWebClient_RequestManager ILibCreateWebClient(int PoolSize, void *Chain);
ILibWebClient_StateObject ILibCreateWebClientEx(ILibWebClient_OnResponse OnResponse, ILibAsyncSocket_SocketModule socketModule, void *user1, void *user2);

void ILibWebClient_OnBufferReAllocate(ILibAsyncSocket_SocketModule token, void *user, ptrdiff_t offSet);
void ILibWebClient_OnData(ILibAsyncSocket_SocketModule socketModule,char* buffer,int *p_beginPointer, int endPointer,ILibAsyncSocket_OnInterrupt *InterruptPtr, void **user, int *PAUSE);
void ILibDestroyWebClient(void *object);

void ILibWebClient_DestroyWebClientDataObject(ILibWebClient_StateObject token);
struct packetheader *ILibWebClient_GetHeaderFromDataObject(ILibWebClient_StateObject token);

ILibWebClient_RequestToken ILibWebClient_PipelineRequestEx(
	ILibWebClient_RequestManager WebClient, 
	struct sockaddr *RemoteEndpoint, 
	char *headerBuffer,
	int headerBufferLength,
	int headerBuffer_FREE,
	char *bodyBuffer,
	int bodyBufferLength,
	int bodyBuffer_FREE,
	ILibWebClient_OnResponse OnResponse,
	void *user1,
	void *user2);

ILibWebClient_RequestToken ILibWebClient_PipelineRequest(
	ILibWebClient_RequestManager WebClient, 
	struct sockaddr *RemoteEndpoint,
	struct packetheader *packet,
	ILibWebClient_OnResponse OnResponse,
	void *user1,
	void *user2);

int ILibWebClient_StreamRequestBody(
									 ILibWebClient_RequestToken token, 
									 char *body,
									 int bodyLength, 
									 enum ILibAsyncSocket_MemoryOwnership MemoryOwnership,
									 int done
									 );
ILibWebClient_RequestToken ILibWebClient_PipelineStreamedRequest(
									ILibWebClient_RequestManager WebClient,
									struct sockaddr *RemoteEndpoint,
									struct packetheader *packet,
									ILibWebClient_OnResponse OnResponse,
									ILibWebClient_OnSendOK OnSendOK,
									void *user1,
									void *user2);


void ILibWebClient_FinishedResponse_Server(ILibWebClient_StateObject wcdo);
void ILibWebClient_DeleteRequests(ILibWebClient_RequestManager WebClientToken, struct sockaddr* addr);
void ILibWebClient_Resume(ILibWebClient_StateObject wcdo);
void ILibWebClient_Pause(ILibWebClient_StateObject wcdo);
void ILibWebClient_Disconnect(ILibWebClient_StateObject wcdo);
void ILibWebClient_CancelRequest(ILibWebClient_RequestToken RequestToken);
void ILibWebClient_ResetUserObjects(ILibWebClient_StateObject webstate, void *user1, void *user2);
ILibWebClient_RequestToken ILibWebClient_GetRequestToken_FromStateObject(ILibWebClient_StateObject WebStateObject);
ILibWebClient_StateObject ILibWebClient_GetStateObjectFromRequestToken(ILibWebClient_RequestToken token);

void ILibWebClient_Parse_ContentRange(char *contentRange, int *Start, int *End, int *TotalLength);
enum ILibWebClient_Range_Result ILibWebClient_Parse_Range(char *Range, long *Start, long *Length, long TotalLength);

void ILibWebClient_SetMaxConcurrentSessionsToServer(ILibWebClient_RequestManager WebClient, int maxConnections);
void ILibWebClient_SetUser(ILibWebClient_RequestManager manager, void *user);
void* ILibWebClient_GetUser(ILibWebClient_RequestManager manager);
void* ILibWebClient_GetChain(ILibWebClient_RequestManager manager);

#ifndef MICROSTACK_NOTLS
void ILibWebClient_SetTLS(ILibWebClient_RequestManager manager, void *ssl_ctx, ILibWebClient_OnSslConnection OnSslConnection);
#endif

// Added methods
int ILibWebClient_GetLocalInterface(void* socketModule, struct sockaddr *localAddress);
int ILibWebClient_GetRemoteInterface(void* socketModule, struct sockaddr *remoteAddress);

// OpenSSL supporting code
#ifndef MICROSTACK_NOTLS
X509*			ILibWebClient_SslGetCert(void* socketModule);
STACK_OF(X509)*	ILibWebClient_SslGetCerts(void* socketModule);
#endif

// Mesh specific code, used to store the NodeID in the web client
char* ILibWebClient_GetCertificateHash(void* socketModule);
char* ILibWebClient_SetCertificateHash(void* socketModule, char* ptr);
char* ILibWebClient_GetCertificateHashEx(void* socketModule);

int ILibWebClient_HasAllocatedClient(ILibWebClient_RequestManager WebClient, struct sockaddr *remoteAddress);
int ILibWebClient_GetActiveClientCount(ILibWebClient_RequestManager WebClient);

#ifdef UPNP_DEBUG
char *ILibWebClient_QueryWCDO(ILibWebClient_RequestManager wcm, char *query);
#endif

#ifdef __cplusplus
}
#endif

#endif

