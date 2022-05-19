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

/*! \file ILibWebServer.h 
	\brief MicroStack APIs for HTTP Server functionality
*/

#ifndef __ILibWebServer__
#define __ILibWebServer__
#include "ILibParsers.h"
#include "ILibAsyncServerSocket.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \defgroup ILibWebServer ILibWebServer Module
	\{
*/

enum ILibWebServer_Status
{
	ILibWebServer_ALL_DATA_SENT						= 0,	/*!< All of the data has already been sent */
	ILibWebServer_NOT_ALL_DATA_SENT_YET				= 1,	/*!< Not all of the data could be sent, but is queued to be sent as soon as possible */
	ILibWebServer_SEND_RESULTED_IN_DISCONNECT		= -2,	/*!< A send operation resulted in the socket being closed */
	ILibWebServer_INVALID_SESSION					= -3,	/*!< The specified ILibWebServer_Session was invalid */
	ILibWebServer_TRIED_TO_SEND_ON_CLOSED_SOCKET	= -4,	/*!< A send operation was attmepted on a closed socket */
};
/*! \typedef ILibWebServer_ServerToken
	\brief The handle for an ILibWebServer module
*/
typedef void* ILibWebServer_ServerToken;

struct ILibWebServer_Session;

/*! \typedef ILibWebServer_Session_OnReceive
	\brief OnReceive Handler
	\param sender The associate \a ILibWebServer_Session object
	\param InterruptFlag boolean indicating if the underlying chain/thread is disposing
	\param header The HTTP headers that were received
	\param bodyBuffer Pointer to the HTTP body
	\param[in,out] beginPointer Starting offset of the body buffer. Advance this pointer when the data is consumed.
	\param endPointer Length of available data pointed to by \a bodyBuffer
	\param done boolean indicating if the entire packet has been received
*/
typedef void (*ILibWebServer_Session_OnReceive)(struct ILibWebServer_Session *sender, int InterruptFlag, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done);
/*! \typedef ILibWebServer_Session_OnDisconnect
	\brief Handler for when an ILibWebServer_Session object is disconnected
	\param sender The \a ILibWebServer_Session object that was disconnected
*/
typedef void (*ILibWebServer_Session_OnDisconnect)(struct ILibWebServer_Session *sender);
/*! \typedef ILibWebServer_Session_OnSendOK
	\brief Handler for when pending send operations have completed
	\par
	<B>Note:</B> This handler will only be called after all pending data from any call(s) to 
	\a ILibWebServer_Send, \a ILibWebServer_Send_Raw, \a ILibWebServer_StreamBody,
	\a ILibWebServer_StreamHeader, and/or \a ILibWebServer_StreamHeader_Raw, have completed. You will need to look at the return values
	of those methods, to determine if there is any pending data that still needs to be sent. That will determine if this handler will get called.
	\param sender The \a ILibWebServer_Session that has completed sending all of the pending data
*/
typedef	void (*ILibWebServer_Session_OnSendOK)(struct ILibWebServer_Session *sender);

/*! \struct ILibWebServer_Session
	\brief A structure representing the state of an HTTP Session
*/
struct ILibWebServer_Session
{
	/*! \var OnReceive
		\brief A Function Pointer that is triggered whenever data is received
	*/
	ILibWebServer_Session_OnReceive OnReceive;
	/*! \var OnDisconnect
		\brief A Function Pointer that is triggered when the session is disconnected
	*/
	ILibWebServer_Session_OnDisconnect OnDisconnect;
	/*! \var OnSendOK
		\brief A Function Pointer that is triggered when the send buffer is emptied
	*/
	ILibWebServer_Session_OnSendOK OnSendOK;
	void *Parent;

	/*! \var User
		\brief A reserved pointer that you can use for your own use
	*/
	void *User;
	/*! \var User2
		\brief A reserved pointer that you can use for your own use
	*/
	void *User2;
	/*! \var User3
		\brief A reserved pointer that you can use for your own use
	*/
	void *User3;
	/*! \var User4
		\brief A reserved pointer that you can use for your own use
	*/
	unsigned int User4;
	unsigned int User5;
	char* CertificateHashPtr; // Points to the certificate hash (next field) if set
	char CertificateHash[32]; // Used by the Mesh to store NodeID of this session

	void *Reserved1;	// AsyncServerSocket
	void *Reserved2;	// ConnectionToken
	void *Reserved3;	// WebClientDataObject
	void *Reserved7;	// VirtualDirectory
	int   Reserved4;	// Request Answered Flag (set by send)
	int   Reserved8;	// RequestAnswered Method Called
	int   Reserved5;	// Request Made Flag
	int   Reserved6;	// Close Override Flag
	int   Reserved9;	// Reserved for future use
	void *Reserved10;	// DisconnectFlagPointer

	sem_t Reserved11;	// Session Lock
	int   Reserved12;	// Reference Counter;
	int   Reserved13;	// Override VirDir Struct

	char *buffer;
	int bufferLength;
	int done;
	int SessionInterrupted;
};


void ILibWebServer_AddRef(struct ILibWebServer_Session *session);
void ILibWebServer_Release(struct ILibWebServer_Session *session);

/*! \typedef ILibWebServer_Session_OnSession
	\brief New Session Handler
	\param SessionToken The new Session
	\param User The \a User object specified in \a ILibWebServer_Create
*/
typedef void (*ILibWebServer_Session_OnSession)(struct ILibWebServer_Session *SessionToken, void *User);
/*! \typedef ILibWebServer_VirtualDirectory
	\brief Request Handler for a registered Virtual Directory
	\param session The session that received the request
	\param header The HTTP headers
	\param bodyBuffer Pointer to the HTTP body
	\param[in,out] beginPointer Starting index of \a bodyBuffer. Advance this pointer as data is consumed
	\param endPointer Length of available data in \a bodyBuffer
	\param done boolean indicating that the entire packet has been read
	\param user The \a user specified in \a ILibWebServer_Create
*/
typedef void (*ILibWebServer_VirtualDirectory)(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user);

#ifndef MICROSTACK_NOTLS
void ILibWebServer_SetTLS(ILibWebServer_ServerToken object, void *ssl_ctx);
#endif

void ILibWebServer_SetTag(ILibWebServer_ServerToken WebServerToken, void *Tag);
void *ILibWebServer_GetTag(ILibWebServer_ServerToken WebServerToken);

ILibWebServer_ServerToken ILibWebServer_CreateEx(void *Chain, int MaxConnections, unsigned short PortNumber, int loopbackFlag, ILibWebServer_Session_OnSession OnSession, void *User);
#define ILibWebServer_Create(Chain, MaxConnections, PortNumber, OnSession, User) ILibWebServer_CreateEx(Chain, MaxConnections, PortNumber, INADDR_ANY, OnSession, User)

int ILibWebServer_RegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength, ILibWebServer_VirtualDirectory OnVirtualDirectory, void *user);
int ILibWebServer_UnRegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength);

enum ILibWebServer_Status ILibWebServer_Send(struct ILibWebServer_Session *session, struct packetheader *packet);
enum ILibWebServer_Status ILibWebServer_Send_Raw(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done);

/*! \def ILibWebServer_Session_GetPendingBytesToSend
	\brief Returns the number of outstanding bytes to be sent
	\param session The ILibWebServer_Session object to query
*/
#define ILibWebServer_Session_GetPendingBytesToSend(session) ILibAsyncServerSocket_GetPendingBytesToSend(session->Reserved1,session->Reserved2)
/*! \def ILibWebServer_Session_GetTotalBytesSent
	\brief Returns the total number of bytes sent
	\param session The ILibWebServer_Session object to query
*/
#define ILibWebServer_Session_GetTotalBytesSent(session) ILibAsyncServerSocket_GetTotalBytesSent(session->Reserved1,session->Reserved2)
/*! \def ILibWebServer_Session_ResetTotalBytesSent
	\brief Resets the total bytes set counter
	\param session The ILibWebServer_Session object to query

*/
#define ILibWebServer_Session_ResetTotalBytesSent(session) ILibAsyncServerSocket_ResetTotalBytesSent(session->Reserved1,session->Reserved2)

unsigned short ILibWebServer_GetPortNumber(ILibWebServer_ServerToken WebServerToken);
int ILibWebServer_GetLocalInterface(struct ILibWebServer_Session *session, struct sockaddr *localAddress);
int ILibWebServer_GetRemoteInterface(struct ILibWebServer_Session *session, struct sockaddr *remoteAddress);

enum ILibWebServer_Status ILibWebServer_StreamHeader(struct ILibWebServer_Session *session, struct packetheader *header);
enum ILibWebServer_Status ILibWebServer_StreamBody(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done);

enum ILibWebServer_Status ILibWebServer_StreamHeader_Raw(struct ILibWebServer_Session *session, int StatusCode,char *StatusData,char *ResponseHeaders, int ResponseHeaders_FREE);
void ILibWebServer_DisconnectSession(struct ILibWebServer_Session *session);

void ILibWebServer_Pause(struct ILibWebServer_Session *session);
void ILibWebServer_Resume(struct ILibWebServer_Session *session);

void ILibWebServer_OverrideReceiveHandler(struct ILibWebServer_Session *session, ILibWebServer_Session_OnReceive OnReceive);

void ILibWebServer_StreamFile(struct ILibWebServer_Session *session, FILE* pfile);

#ifdef __cplusplus
}
#endif

/* \} */
#endif

