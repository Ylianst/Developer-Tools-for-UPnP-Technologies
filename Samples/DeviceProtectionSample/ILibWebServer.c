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

#define HTTPVERSION "1.1"

#if defined(WIN32) && !defined(_WIN32_WCE)
#define _CRTDBG_MAP_ALLOC
#define snprintf(dst, len, frm, ...) _snprintf_s(dst, len, _TRUNCATE, frm, __VA_ARGS__)
#include <crtdbg.h>
#endif

#if defined(WINSOCK2)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#include <wininet.h>
#endif

#include "ILibParsers.h"
#include "ILibWebServer.h"
#include "ILibAsyncServerSocket.h"
#include "ILibAsyncSocket.h"
#include "ILibWebClient.h"

#ifndef MICROSTACK_NOTLS
#include "utils.h"
#endif




//#define HTTP_SESSION_IDLE_TIMEOUT 30

#ifdef ILibWebServer_SESSION_TRACKING
void ILibWebServer_SessionTrack(void *Session, char *msg)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	char tempMsg[4096];
	wchar_t t[4096];
	size_t len;
	sprintf_s(tempMsg, 4096, "Session: %p   %s\r\n", Session, msg);
	mbstowcs_s(&len, t, 4096, tempMsg, 4096);
	OutputDebugString(t);
#else
	printf("Session: %x   %s\r\n",Session,msg);
#endif
}
#define SESSION_TRACK(Session,msg) ILibWebServer_SessionTrack(Session,msg)
#else
#define SESSION_TRACK(Session,msg)
#endif
struct ILibWebServer_VirDir_Data
{
	voidfp callback;
	void *user;
};

struct ILibWebServer_StateModule
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	void *Chain;
	void *ServerSocket;
	void *LifeTime;
	void *User;
	void *Tag;

	void *VirtualDirectoryTable;

	void (*OnSession)(struct ILibWebServer_Session *SessionToken, void *User);
};

/*! \fn ILibWebServer_SetTag(ILibWebServer_ServerToken object, void *Tag)
\brief Sets the user tag associated with the server
\param object The ILibWebServer to associate the user tag with
\param Tag The user tag to associate
*/
void ILibWebServer_SetTag(ILibWebServer_ServerToken object, void *Tag)
{
	struct ILibWebServer_StateModule *s = (struct ILibWebServer_StateModule*)object;
	s->Tag = Tag;
}

/*! \fn ILibWebServer_GetTag(ILibWebServer_ServerToken object)
\brief Gets the user tag associated with the server
\param object The ILibWebServer to query
\returns The associated user tag
*/
void *ILibWebServer_GetTag(ILibWebServer_ServerToken object)
{
	struct ILibWebServer_StateModule *s = (struct ILibWebServer_StateModule*)object;
	return(s->Tag);
}

//
// Internal method dispatched by a timer to idle out a session
//
// A session can idle in two ways. 
// 1.) A TCP connection is established, but a request isn't received within an allotted time period
// 2.) A request is answered, and another request isn't received with an allotted time period
// 
void ILibWebServer_IdleSink(void *object)
{
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)object;
	if (ILibAsyncSocket_IsFree(session->Reserved2) == 0)
	{
		// This is OK, because we're on the MicroStackThread
		ILibAsyncServerSocket_Disconnect(session->Reserved1, session->Reserved2);
	}
}

//
// Chain Destroy handler
//
void ILibWebServer_Destroy(void *object)
{
	struct ILibWebServer_StateModule *s = (struct ILibWebServer_StateModule*)object;
	void *en;
	void *data;
	char *key;
	int keyLength;

	if (s->VirtualDirectoryTable != NULL)
	{
		//
		// If there are registered Virtual Directories, we need to free the resources
		// associated with them
		//
		en = ILibHashTree_GetEnumerator(s->VirtualDirectoryTable);
		while (ILibHashTree_MoveNext(en) == 0)
		{
			ILibHashTree_GetValue(en, &key, &keyLength, &data);
			free(data);
		}
		ILibHashTree_DestroyEnumerator(en);
		ILibDestroyHashTree(s->VirtualDirectoryTable);
	}
}
//
// Internal method dispatched from the underlying WebClient engine
//
// <param name="WebReaderToken">The WebClient token</param>
// <param name="InterruptFlag">Flag indicating session was interrupted</param>
// <param name="header">The HTTP header structure</param>
// <param name="bodyBuffer">buffer pointing to HTTP body</param>
// <param name="beginPointer">buffer pointer offset</param>
// <param name="endPointer">buffer length</param>
// <param name="done">Flag indicating if the entire packet has been read</param>
// <param name="user1"></param>
// <param name="user2">The ILibWebServer uses this to pass the ILibWebServer_Session object</param>
// <param name="PAUSE">Flag to pause data reads on the underlying WebClient engine</param>
void ILibWebServer_OnResponse(
	void *WebReaderToken,
	int InterruptFlag,
	struct packetheader *header,
	char *bodyBuffer,
	int *beginPointer,
	int endPointer,
	int done,
	void *user1,
	void *user2,
	int *PAUSE)
{
	struct ILibWebServer_Session *ws = (struct ILibWebServer_Session*)user2;
	struct ILibWebServer_StateModule *wsm = (struct ILibWebServer_StateModule*)ws->Parent;

	char *tmp;
	int tmpLength;
	struct parser_result *pr;
	int PreSlash = 0;

	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( user1 );

	if (header == NULL) return;

	ws->buffer = bodyBuffer + *beginPointer;
	ws->bufferLength = endPointer - *beginPointer;
	ws->done = done;

#if defined(MAX_HTTP_HEADER_SIZE) || defined(MAX_HTTP_PACKET_SIZE)
	if (ws->done == (int)ILibWebClient_DoneCode_HeaderTooBig ||
		ws->done == (int)ILibWebClient_DoneCode_BodyTooBig)
	{
		//
		// We need to return a 413 Error code for this condition, to be nice
		//
		{
			char body[255];
			int bodyLength;
			bodyLength = snprintf(body, 255, "HTTP/1.1 413 Request Too Big (MaxHeader=%d)\r\n\r\n", MAX_HTTP_HEADER_SIZE);
			ILibAsyncSocket_Send(ws->Reserved2, body, bodyLength, ILibAsyncSocket_MemoryOwnership_STATIC);
		}
	}
#endif
	//
	// Reserved4 = Request Answered Flag
	//	If this flag is set, the request was answered
	// Reserved5 = Request Made Flag
	//	If this flag is set, a request has been received
	//
	if (ws->Reserved4 != 0 || ws->Reserved5 == 0)
	{
		//
		// This session is no longer idle
		//
		ws->Reserved4 = 0;
		ws->Reserved5 = 1;
		ws->Reserved8 = 0;
		ILibLifeTime_Remove(((struct ILibWebServer_StateModule*)ws->Parent)->LifeTime, ws);
	}

	//
	// Check to make sure that the request contains a host header, as required
	// by RFC-2616, for HTTP/1.1 requests
	//
	if (done!=0 && header != NULL && header->Directive != NULL && atof(header->Version) > 1)
	{
		if (ILibGetHeaderLine(header, "host", 4) == NULL)
		{
			//
			// Host header is missing
			//
			char body[255];
			int bodyLength;
			bodyLength = snprintf(body, 255, "HTTP/1.1 400 Bad Request (Missing Host Field)\r\n\r\n");
			ILibWebServer_Send_Raw(ws, body, bodyLength, ILibAsyncSocket_MemoryOwnership_USER, 1);
			return;
		}
	}

	//
	// Check Virtual Directory
	//
	if (wsm->VirtualDirectoryTable != NULL)
	{
		//
		// Reserved7 = Virtual Directory State Object
		//
		if (ws->Reserved7 == NULL)
		{
			//
			// See if we can find the virtual directory.
			// If we do, set the State Object, so future responses don't need to 
			// do it again
			//
			pr = ILibParseString(header->DirectiveObj, 0, header->DirectiveObjLength, "/", 1);
			if (pr->FirstResult->datalength == 0)
			{
				// Does not start with '/'
				tmp = pr->FirstResult->NextResult->data;
				tmpLength = pr->FirstResult->NextResult->datalength;
				PreSlash = 1;
			}
			else
			{
				// Starts with '/'
				tmp = pr->FirstResult->data;
				tmpLength = pr->FirstResult->datalength;
			}
			ILibDestructParserResults(pr);
			//
			// Does the Virtual Directory Exist?
			//
			if (ILibHasEntry(wsm->VirtualDirectoryTable,tmp,tmpLength)!=0)
			{
				//
				// Virtual Directory is defined
				//
				header->Reserved = header->DirectiveObj;
				header->DirectiveObj = tmp + tmpLength;
				header->DirectiveObjLength -= (tmpLength + PreSlash);
				//
				// Set the StateObject, then call the handler
				//
				ws->Reserved7 = ILibGetEntry(wsm->VirtualDirectoryTable, tmp, tmpLength);
				if (ws->Reserved7 != NULL) ((ILibWebServer_VirtualDirectory)((struct ILibWebServer_VirDir_Data*)ws->Reserved7)->callback)(ws,header,bodyBuffer,beginPointer,endPointer,done,((struct ILibWebServer_VirDir_Data*)ws->Reserved7)->user);
			}
			else if (ws->OnReceive!=NULL)
			{
				//
				// If the virtual directory doesn't exist, just call the main handler
				//
				ws->OnReceive(ws, InterruptFlag, header, bodyBuffer, beginPointer, endPointer, done);
			}
		}
		else
		{
			if (ws->Reserved13==0)
			{
				//
				// The state object was already set, so we know this is the handler to use. So easy!
				//
				((ILibWebServer_VirtualDirectory)((struct ILibWebServer_VirDir_Data*)ws->Reserved7)->callback)(ws,header,bodyBuffer,beginPointer,endPointer,done,((struct ILibWebServer_VirDir_Data*)ws->Reserved7)->user);
			}
			else
			{
				ws->OnReceive(ws,InterruptFlag,header,bodyBuffer,beginPointer,endPointer,done);
			}
		}
	}
	else if (ws->OnReceive!=NULL)
	{
		//
		// Since there is no Virtual Directory lookup table, none were registered,
		// so we know we have no choice but to call the regular handler
		//
		ws->OnReceive(ws,InterruptFlag,header,bodyBuffer,beginPointer,endPointer,done);
	}


	//
	// Reserved8 = RequestAnswered method has been called
	//
	if (done!=0 && InterruptFlag==0 && header!=NULL && ws->Reserved8==0)
	{
		//
		// The request hasn't been satisfied yet, so stop reading from the socket until it is
		//
		*PAUSE=1;
	}
}

//
// Internal method dispatched from the underlying ILibAsyncServerSocket module
//
// This is dispatched when the underlying buffer has been reallocated, which may
// neccesitate extra processing
// <param name="AsyncServerSocketToken">AsyncServerSocket token</param>
// <param name="ConnectionToken">Connection token (Underlying ILibAsyncSocket)</param>
// <param name="user">The ILibWebServer_Session object</param>
// <param name="offSet">Offset to the new buffer location</param>
void ILibWebServer_OnBufferReAllocated(void *AsyncServerSocketToken, void *ConnectionToken, void *user, ptrdiff_t offSet)
{
	struct ILibWebServer_Session *ws = (struct ILibWebServer_Session*)user;

	UNREFERENCED_PARAMETER( AsyncServerSocketToken );
	UNREFERENCED_PARAMETER( ConnectionToken );

	//
	// We need to pass this down to our internal ILibWebClient for further processing
	// Reserved2 = ConnectionToken
	// Reserved3 = WebClientDataObject
	//
	ILibWebClient_OnBufferReAllocate(ws->Reserved2,ws->Reserved3,offSet);
}
#if defined(MAX_HTTP_PACKET_SIZE)
void ILibWebServer_OnBufferSizeExceeded(void *connectionToken, void *user)
{
	//
	// We need to return a 413 Error code for this condition, to be nice
	//

	char body[255];
	int bodyLength;
	bodyLength = sprintf(body,"HTTP/1.1 413 Request Too Big (MaxPacketSize=%d)\r\n\r\n",MAX_HTTP_PACKET_SIZE);
	ILibAsyncSocket_Send(connectionToken,body,bodyLength,ILibAsyncSocket_MemoryOwnership_STATIC);
}
#endif
//
// Internal method dispatched from the underlying ILibAsyncServerSocket module
//
// <param name="AsyncServerSocketModule">AsyncServerSocket token</param>
// <param name="ConnectionToken">Connection token (Underlying ILibAsyncSocket)</param>
// <param name="user">User object that can be set. (used here for ILibWebServer_Session</param>
#ifndef MICROSTACK_NOTLS
void ILibWebServer_OnConnect(void *AsyncServerSocketModule, X509 * cert, STACK_OF(X509) *certChain, void *ConnectionToken, void **user)
#else
void ILibWebServer_OnConnect(void *AsyncServerSocketModule, void *ConnectionToken, void **user)
#endif
{
	struct ILibWebServer_StateModule *wsm;
	struct ILibWebServer_Session *ws;

	//
	// Create a new ILibWebServer_Session to represent this connection
	//
	wsm = (struct ILibWebServer_StateModule*)ILibAsyncServerSocket_GetTag(AsyncServerSocketModule);
	if ((ws = (struct ILibWebServer_Session*)malloc(sizeof(struct ILibWebServer_Session))) == NULL) ILIBCRITICALEXIT(254);
	memset(ws, 0, sizeof(struct ILibWebServer_Session));
	sem_init(&(ws->Reserved11), 0, 1); // Initialize the SessionLock
	ws->Reserved12 = 1; // Reference Counter, Initial count should be 1

	//printf("#### ALLOCATED (%d) ####\r\n", ConnectionToken);

	ws->Parent = wsm;
	ws->Reserved1 = AsyncServerSocketModule;
	ws->Reserved2 = ConnectionToken;
	ws->Reserved3 = ILibCreateWebClientEx(&ILibWebServer_OnResponse, ConnectionToken, wsm, ws);
	ws->User = wsm->User;
#ifndef MICROSTACK_NOTLS
	{
	int chainlen;
    unsigned char * certbuf = NULL; // NULL => library allocates memory
    int certlen = i2d_X509(cert, &certbuf); // get DER encoding for hash
	char buf[256];

	if (certlen) {
		// Note:  commented-out code was computing SHA-1 hash from the certificate so that
		//        a RFC 4122-compliant UUID could be constructed. However, due to potential
		//        weaknesses in SHA-1, we are using a truncated SHA-256 now instead.
		//SHA_CTX ctx; // vbl added
		//unsigned char hash[SHA_DIGEST_LENGTH]; // vbl added
		//memset(hash, '\0', sizeof(hash)); // vbl added

		X509_NAME_oneline(X509_get_subject_name(cert), buf, 256);
		strncpy(ws->CertificateName, buf, sizeof(ws->CertificateName) - 1);
		ws->CertificateName[sizeof(ws->CertificateName) - 1] = '\0'; // force null-termination

		//SHA1_Init(&ctx); // vbl added
		//SHA1_Update(&ctx, certbuf, certlen);// vbl added
		//SHA1_Final(hash, &ctx); // Finish computing hash of cert

		// copy hash to first 20 bytes of ws->CertificateHash buffer
		//memset(ws->CertificateHash, '\0', sizeof(ws->CertificateHash)); // vbl added
		//memcpy(ws->CertificateHash, hash, sizeof(hash)); // vbl added

		util_sha256(certbuf, certlen, ws->CertificateHash); // use this instead of code above for SHA-256
		ws->CertificateHashPtr = & (ws->CertificateHash);

	chainlen = sk_X509_num(certChain);
	cert = sk_X509_value(certChain, 0);
	}
	// TODO:  see if I need to free memory in the X509 library now
	}
#endif
	*user = ws;
#if defined(MAX_HTTP_PACKET_SIZE)
	ILibAsyncSocket_SetMaximumBufferSize(ConnectionToken, MAX_HTTP_PACKET_SIZE, &ILibWebServer_OnBufferSizeExceeded, ws);
#endif	
	//
	// We want to know when this connection reallocates its internal buffer, because we may
	// need to fix a few things
	//
	ILibAsyncServerSocket_SetReAllocateNotificationCallback(AsyncServerSocketModule, ConnectionToken, &ILibWebServer_OnBufferReAllocated);

	//
	// Add a timed callback, because if we don't receive a request within a specified
	// amount of time, we want to close the socket, so we don't waste resources
	//
	ILibLifeTime_Add(wsm->LifeTime, ws, HTTP_SESSION_IDLE_TIMEOUT, &ILibWebServer_IdleSink, NULL);

	SESSION_TRACK(ws, "* Allocated *");
	SESSION_TRACK(ws, "AddRef");
	//
	// Inform the user that a new session was established
	//
	if (wsm->OnSession != NULL) wsm->OnSession(ws, wsm->User);
}

//
// Internal method dispatched from the underlying AsyncServerSocket engine
// 
// <param name="AsyncServerSocketModule">The ILibAsyncServerSocket token</param>
// <param name="ConnectionToken">The ILibAsyncSocket connection token</param>
// <param name="user">The ILibWebServer_Session object</param>
void ILibWebServer_OnDisconnect(void *AsyncServerSocketModule, void *ConnectionToken, void *user)
{
	struct ILibWebServer_Session *ws = (struct ILibWebServer_Session*)user;

	UNREFERENCED_PARAMETER( AsyncServerSocketModule );
	UNREFERENCED_PARAMETER( ConnectionToken );

#ifdef _DEBUG
	//printf("#### RELEASE (%d) ####\r\n", ConnectionToken);
	if (ws == NULL) { PRINTERROR(); ILIBCRITICALEXIT(253); }
#endif

	//
	// If this was a non-persistent connection, the response will queue this up to be called.
	// Some clients may close the socket anyway, before the server does, so we should remove this
	// so we don't accidently close the socket again.
	//
	ILibLifeTime_Remove(((struct ILibWebServer_StateModule*)ws->Parent)->LifeTime, ws->Reserved2);
	if (ws->Reserved10 != NULL) ws->Reserved10 = NULL;

	//
	// Reserved4 = RequestAnsweredFlag
	// Reserved5 = RequestMadeFlag
	//
	if (ws->Reserved4 != 0 || ws->Reserved5 == 0)
	{
		ILibLifeTime_Remove(((struct ILibWebServer_StateModule*)ws->Parent)->LifeTime, ws);
		ws->Reserved4 = 0;
	}

	SESSION_TRACK(ws, "OnDisconnect");
	//
	// Notify the user that this session disconnected
	//
	if (ws->OnDisconnect != NULL) ws->OnDisconnect(ws);

	ILibWebServer_Release((struct ILibWebServer_Session *)user);
}
//
// Internal method dispatched from the underlying ILibAsyncServerSocket engine
//
// <param name="AsyncServerSocketModule">The ILibAsyncServerSocket token</param>
// <param name="ConnectionToken">The ILibAsyncSocket connection token</param>
// <param name="buffer">The receive buffer</param>
// <param name="p_beginPointer">buffer offset</param>
// <param name="endPointer">buffer length</param>
// <param name="OnInterrupt">Function Pointer to handle Interrupts</param>
// <param name="user">ILibWebServer_Session object</param>
// <param name="PAUSE">Flag to pause data reads on the underlying AsyncSocket engine</param>
void ILibWebServer_OnReceive(void *AsyncServerSocketModule, void *ConnectionToken, char* buffer,int *p_beginPointer, int endPointer, void (**OnInterrupt)(void *AsyncServerSocketMoudle, void *ConnectionToken, void *user), void **user, int *PAUSE)
{
	//
	// Pipe the data down to our internal WebClient engine, which will do
	// all the HTTP processing
	//
	struct ILibWebServer_Session *ws = (struct ILibWebServer_Session*)(*user);

	UNREFERENCED_PARAMETER( AsyncServerSocketModule );
	UNREFERENCED_PARAMETER( OnInterrupt );

	ILibWebClient_OnData(ConnectionToken,buffer,p_beginPointer,endPointer,NULL,&(ws->Reserved3),PAUSE);
}

//
// Internal method dispatched from the underlying ILibAsyncServerSocket engine, signaling an interrupt
//
// <param name="AsyncServerSocketModule">The ILibAsyncServerSocket token</param>
// <param name="ConnectionToken">The ILibAsyncSocket connection token</param>
// <param name="user">The ILibWebServer_Session object</param>
void ILibWebServer_OnInterrupt(void *AsyncServerSocketModule, void *ConnectionToken, void *user)
{
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)user;
	if (session == NULL) return;
	session->SessionInterrupted = 1;

	UNREFERENCED_PARAMETER( AsyncServerSocketModule );
	UNREFERENCED_PARAMETER( ConnectionToken );

	// This is ok, because this is MicroStackThread
	ILibWebClient_DestroyWebClientDataObject(session->Reserved3);
}

//
// Internal method called when a request has been answered. Dispatched from Send routines
//
// <param name="session">The ILibWebServer_Session object</param>
// <returns>Flag indicating if the session was closed</returns>
int ILibWebServer_RequestAnswered(struct ILibWebServer_Session *session)
{
	struct packetheader *hdr = ILibWebClient_GetHeaderFromDataObject(session->Reserved3);
	struct packetheader_field_node *f;
	int PersistentConnection = 0;
	if (hdr == NULL) return 0;

	//
	// Reserved7 = Virtual Directory State Object
	//	We delete this, because the request is finished, so we don't need to direct
	//	data to this handler anymore. It needs to be recalculated next time
	//
	session->Reserved7 = NULL;

	//
	// Reserved8 = RequestAnswered method called
	//If this is set, this method was already called, so we can just exit
	//
	if (session->Reserved8 != 0) return(0);

	//
	// Set the flags, so if this re-enters, we don't process this again
	//
	session->Reserved8 = 1;
	f = hdr->FirstField;

	//
	// Reserved5 = Request Made. Since the request was answered, we can clear this.
	//
	session->Reserved5 = 0;

	//
	// Reserved6 = CloseOverrideFlag
	//	which means the session must be closed when request is complete
	//
	if (session->Reserved6==0)
	{
		//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		if (hdr->VersionLength==3 && memcmp(hdr->Version,"1.0",3)!=0)
		{
			// HTTP 1.1+ , Check for CLOSE token
			PersistentConnection = 1;
			while (f!=NULL)
			{
				if (f->FieldLength==10 && strncasecmp(f->Field,"CONNECTION",10)==0)
				{
					if (f->FieldDataLength==5 && strncasecmp(f->FieldData,"CLOSE",5)==0)
					{
						PersistentConnection = 0;
						break;
					}
				}
				f = f->NextField;
			}
		}
		//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	}

	if (PersistentConnection==0)
	{
		//
		// Ensure calling on MicroStackThread. This will just result dispatching the callback on
		// the microstack thread
		//
		ILibLifeTime_Add(((struct ILibWebServer_StateModule*)session->Parent)->LifeTime, session->Reserved2, 0, &ILibAsyncSocket_Disconnect, NULL);
	}
	else
	{
		//
		// This is a persistent connection. Set a timed callback, to idle this session if necessary
		//
		ILibLifeTime_Add(((struct ILibWebServer_StateModule*)session->Parent)->LifeTime, session, HTTP_SESSION_IDLE_TIMEOUT, &ILibWebServer_IdleSink, NULL);
		ILibWebClient_FinishedResponse_Server(session->Reserved3);
		//
		// Since we're done with this request, resume the underlying socket, so we can continue
		//
		ILibWebClient_Resume(session->Reserved3);
	}
	return(PersistentConnection==0?ILibWebServer_SEND_RESULTED_IN_DISCONNECT:0);
}

//
// Internal method dispatched from the underlying ILibAsyncServerSocket engine
//
// <param name="AsyncServerSocketModule">The ILibAsyncServerSocket token</param>
// <param name="ConnectionToken">The ILibAsyncSocket connection token</param>
// <param name="user">The ILibWebServer_Session object</param>
void ILibWebServer_OnSendOK(void *AsyncServerSocketModule, void *ConnectionToken, void *user)
{
	int flag = 0;
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)user;

	UNREFERENCED_PARAMETER( AsyncServerSocketModule );
	UNREFERENCED_PARAMETER( ConnectionToken );

	//
	// Reserved4 = RequestAnsweredFlag
	//
	if (session->Reserved4!=0)
	{
		// This is normally called when the response was sent. But since it couldn't get through
		// the first time, this method gets dispatched when it did, so now we have to call it.
		flag = ILibWebServer_RequestAnswered(session);
	}
	if (session->OnSendOK!=NULL && flag != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
	{
		// Pass this event on, if everything is ok
		session->OnSendOK(session);
	}
}

#ifndef MICROSTACK_NOTLS
void ILibWebServer_SetTLS(ILibWebServer_ServerToken object, void *ssl_ctx)
{
	struct ILibWebServer_StateModule *module = (struct ILibWebServer_StateModule*)object;
	ILibAsyncServerSocket_SetSSL_CTX(module->ServerSocket, ssl_ctx);
}
#endif

/*! \fn ILibWebServer_Create(void *Chain, int MaxConnections, int PortNumber,ILibWebServer_Session_OnSession OnSession, void *User)
\brief Constructor for ILibWebServer
\param Chain The Chain to add this module to. (Chain must <B>not</B> be running)
\param MaxConnections The maximum number of simultaneous connections
\param PortNumber The Port number to listen to (0 = Random)
\param OnSession Function Pointer to dispatch on when new Sessions are established
\param User User state object to pass to OnSession
*/
ILibWebServer_ServerToken ILibWebServer_CreateEx(void *Chain, int MaxConnections, unsigned short PortNumber, int loopbackFlag, ILibWebServer_Session_OnSession OnSession, void *User)
{
	struct ILibWebServer_StateModule *RetVal = (struct ILibWebServer_StateModule*)malloc(sizeof(struct ILibWebServer_StateModule));

	if (RetVal == NULL) { PRINTERROR(); return NULL; }
	memset(RetVal, 0, sizeof(struct ILibWebServer_StateModule));
	RetVal->Destroy = &ILibWebServer_Destroy;
	RetVal->Chain = Chain;
	RetVal->OnSession = OnSession;

	//
	// Create the underling ILibAsyncServerSocket
	//
	RetVal->ServerSocket = ILibCreateAsyncServerSocketModule(
		Chain,
		MaxConnections,
		PortNumber,
		INITIAL_BUFFER_SIZE,
		loopbackFlag, 
		&ILibWebServer_OnConnect,			// OnConnect
		&ILibWebServer_OnDisconnect,		// OnDisconnect
		&ILibWebServer_OnReceive,			// OnReceive
		&ILibWebServer_OnInterrupt,			// OnInterrupt
		&ILibWebServer_OnSendOK				// OnSendOK
		);

	if (RetVal->ServerSocket == NULL) return NULL;

	//
	// Set ourselves in the User tag of the underlying ILibAsyncServerSocket
	//
	ILibAsyncServerSocket_SetTag(RetVal->ServerSocket, RetVal);
	RetVal->LifeTime = ILibGetBaseTimer(Chain); //ILibCreateLifeTime(Chain);
	RetVal->User = User;
	ILibAddToChain(Chain, RetVal);
	return RetVal;
}

/*! \fn ILibWebServer_GetPortNumber(ILibWebServer_ServerToken WebServerToken)
\brief Returns the port number that this module is listening to
\param WebServerToken The ILibWebServer to query
\returns The listening port number
*/
unsigned short ILibWebServer_GetPortNumber(ILibWebServer_ServerToken WebServerToken)
{
	struct ILibWebServer_StateModule *WSM = (struct ILibWebServer_StateModule*) WebServerToken;
	return(ILibAsyncServerSocket_GetPortNumber(WSM->ServerSocket));
}

/*! \fn ILibWebServer_Send(struct ILibWebServer_Session *session, struct packetheader *packet)
\brief Send a response on a Session
\param session The ILibWebServer_Session to send the response on
\param packet The packet to respond with
\returns Flag indicating send status.
*/
enum ILibWebServer_Status ILibWebServer_Send(struct ILibWebServer_Session *session, struct packetheader *packet)
{
	char *buffer;
	int bufferSize;
	int RetVal = 0;

	if (session == NULL || (session != NULL && session->SessionInterrupted != 0)) 
	{
		ILibDestructPacket(packet);
		return(ILibWebServer_INVALID_SESSION);
	}
	session->Reserved4 = 1;
	bufferSize = ILibGetRawPacket(packet, &buffer);

	if ((RetVal = ILibAsyncServerSocket_Send(session->Reserved1, session->Reserved2, buffer, bufferSize, 0)) == 0)
	{
		// Completed Send
		RetVal = ILibWebServer_RequestAnswered(session);
	}
	ILibDestructPacket(packet);
	return(RetVal);
}

/*! \fn ILibWebServer_Send_Raw(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done)
\brief Send a response on a Session, directly specifying the buffers to send
\param session The ILibWebServer_Session to send the response on
\param buffer The buffer to send
\param bufferSize The length of the buffer
\param userFree The ownership flag of the buffer
\param done Flag indicating if this is everything
\returns Send Status
*/
enum ILibWebServer_Status ILibWebServer_Send_Raw(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done)
{
	int RetVal=0;
	if (session == NULL || (session != NULL && session->SessionInterrupted != 0)) 
	{
		if (userFree == ILibAsyncSocket_MemoryOwnership_CHAIN) { free(buffer); }
		return(ILibWebServer_INVALID_SESSION);
	}
	session->Reserved4 = done;

	RetVal = ILibAsyncServerSocket_Send(session->Reserved1, session->Reserved2, buffer, bufferSize, userFree);
	if (RetVal == 0 && done != 0)
	{
		// Completed Send
		RetVal = ILibWebServer_RequestAnswered(session);
	}
	return(RetVal);
}

/*! \fn ILibWebServer_StreamHeader_Raw(struct ILibWebServer_Session *session, int StatusCode,char *StatusData,char *ResponseHeaders, int ResponseHeaders_FREE)
\brief Streams the HTTP header response on a session, directly specifying the buffer
\par
\b DO \b NOT specify Content-Length or Transfer-Encoding.
\param session The ILibWebServer_Session to send the response on
\param StatusCode The HTTP status code, eg: \b 200
\param StatusData The HTTP status data, eg: \b OK
\param ResponseHeaders Additional HTTP header fields
\param ResponseHeaders_FREE Ownership flag of the addition http header fields
\returns Send Status
*/
enum ILibWebServer_Status ILibWebServer_StreamHeader_Raw(struct ILibWebServer_Session *session, int StatusCode,char *StatusData,char *ResponseHeaders, int ResponseHeaders_FREE)
{
	int len;
	char *temp;
	int RetVal;
	int tempLength;
	char *buffer;
	int bufferLength;
	struct packetheader *hdr;
	struct parser_result *pr,*pr2;
	struct parser_result_field *prf;

	if (session == NULL || (session != NULL && session->SessionInterrupted != 0)) 
	{
		if (ResponseHeaders_FREE == ILibAsyncSocket_MemoryOwnership_CHAIN) { free(ResponseHeaders); }
		return(ILibWebServer_INVALID_SESSION);
	}

	hdr = ILibWebClient_GetHeaderFromDataObject(session->Reserved3);
	if (hdr == NULL)
	{
		if (ResponseHeaders_FREE == ILibAsyncSocket_MemoryOwnership_CHAIN) { free(ResponseHeaders); }
		return(ILibWebServer_INVALID_SESSION);
	}

	//
	// Allocate the response header buffer
	// ToDo: May want to make the response version dynamic or at least #define
	//
	buffer = (char*)malloc(20 + strlen(StatusData));
	if (buffer == NULL) ILIBCRITICALEXIT(254);
	bufferLength = snprintf(buffer, 20 + strlen(StatusData), "HTTP/%s %d %s", HTTPVERSION, StatusCode, StatusData);

	//
	// Send the first portion of the headers across
	//
	RetVal = ILibWebServer_Send_Raw(session, buffer, bufferLength, 0, 0);
	if (RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
	{
		//
		// The Send went through
		//
		//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		if (!(hdr->VersionLength == 3 && memcmp(hdr->Version, "1.0", 3) == 0))
		{
			//
			// If this was not an HTTP/1.0 response, then we need to chunk
			//
			RetVal = ILibWebServer_Send_Raw(session,"\r\nTransfer-Encoding: chunked",28,1,0);
		}
		else
		{
			//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
			//
			// Since we are streaming over HTTP/1.0 , we are required to close the socket when done
			//
			session->Reserved6 = 1;
			//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		}
		//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		if (ResponseHeaders!=NULL && RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
		{
			//
			// Send the user specified headers
			//
			len = (int)strlen(ResponseHeaders);
			if (len<MAX_HEADER_LENGTH)
			{
				RetVal = ILibWebServer_Send_Raw(session,ResponseHeaders,len,ResponseHeaders_FREE,0);
			}
			else
			{
				pr = ILibParseString(ResponseHeaders,0,len,"\r\n",2);
				prf = pr->FirstResult;
				while (prf != NULL)
				{
					if (prf->datalength != 0)
					{
						pr2 = ILibParseString(prf->data, 0, prf->datalength, ":", 1);
						if (pr2->NumResults!=1)
						{
							RetVal = ILibWebServer_Send_Raw(session,"\r\n",2,ILibAsyncSocket_MemoryOwnership_STATIC,0);
							if (RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
							{
								RetVal = ILibWebServer_Send_Raw(session,pr2->FirstResult->data,pr2->FirstResult->datalength+1,ILibAsyncSocket_MemoryOwnership_USER,0);
								if (RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
								{
									tempLength = ILibFragmentText(prf->data+pr2->FirstResult->datalength+1,prf->datalength-pr2->FirstResult->datalength-1,"\r\n ",3,MAX_HEADER_LENGTH,&temp);
									RetVal = ILibWebServer_Send_Raw(session,temp,tempLength,ILibAsyncSocket_MemoryOwnership_CHAIN,0);
								}
								else
								{
									ILibDestructParserResults(pr2);
									break;
								}
							}
							else
							{
								ILibDestructParserResults(pr2);
								break;
							}
						}
						ILibDestructParserResults(pr2);
					}
					prf = prf->NextResult;
				}
				ILibDestructParserResults(pr);
			}
		}
		if (RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
		{
			//
			// Send the Header Terminator
			//
			return(ILibWebServer_Send_Raw(session, "\r\n\r\n", 4, 1, 0));
		}
		else
		{
			if (RetVal != 0 && session->Reserved10 != NULL)
			{
				session->Reserved10 = NULL;
			}
			return(RetVal);
		}
	}
	//
	// ToDo: May want to check logic if the sends didn't go through
	//
	if (RetVal!=0 && session->Reserved10 != NULL)
	{
		session->Reserved10 = NULL;
	}
	return(RetVal);
}

/*! \fn ILibWebServer_StreamHeader(struct ILibWebServer_Session *session, struct packetheader *header)
\brief Streams the HTTP header response on a session
\par
\b DO \b NOT specify Transfer-Encoding.
\param session The ILibWebServer_Session to send the response on
\param header The headers to return
\returns Send Status
*/
enum ILibWebServer_Status ILibWebServer_StreamHeader(struct ILibWebServer_Session *session, struct packetheader *header)
{
	struct packetheader *hdr;
	int bufferLength;
	char *buffer;
	int RetVal;

	if (session == NULL || (session != NULL && session->SessionInterrupted != 0)) 
	{
		ILibDestructPacket(header);
		return(ILibWebServer_INVALID_SESSION);
	}

	hdr = ILibWebClient_GetHeaderFromDataObject(session->Reserved3);
	if (hdr == NULL)
	{
		ILibDestructPacket(header);
		return(ILibWebServer_INVALID_SESSION);
	}

	//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if (!(hdr->VersionLength == 3 && memcmp(hdr->Version, "1.0", 3)==0))
	{
		//
		// If this isn't an HTTP/1.0 connection, remove content-length, and chunk the response
		//
		ILibDeleteHeaderLine(header, "Content-Length", 14);
		//
		// Add the Transfer-Encoding header
		//
		ILibAddHeaderLine(header, "Transfer-Encoding", 17, "chunked", 7);
	}
	else
	{
		//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		// Check to see if they gave us a Content-Length
		if (ILibGetHeaderLine(hdr, "Content-Length", 14)==NULL)
		{
			//
			// If it wasn't, we'll set the CloseOverrideFlag, because in order to be compliant
			// we must close the socket when done
			//
			session->Reserved6 = 1;
		}
		//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	}
	//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	//
	// Grab the bytes and send it
	//
	bufferLength = ILibGetRawPacket(header, &buffer);
	//
	// Since ILibGetRawPacket allocates memory, we give ownership to the MicroStack, and
	// let it take care of it
	//
	RetVal = ILibWebServer_Send_Raw(session, buffer, bufferLength, 0, 0);
	ILibDestructPacket(header);
	if (RetVal!=0 && session->Reserved10 != NULL)
	{
		session->Reserved10 = NULL;
	}
	return(RetVal);
}

/*! \fn ILibWebServer_StreamBody(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done)
\brief Streams the HTTP body on a session
\param session The ILibWebServer_Session to send the response on
\param buffer The buffer to send
\param bufferSize The size of the buffer
\param userFree The ownership flag of the buffer
\param done Flag indicating if this is everything
\returns Send Status
*/
enum ILibWebServer_Status ILibWebServer_StreamBody(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done)
{
	struct packetheader *hdr;
	char *hex;
	int hexLen;
	int RetVal=0;

	if (session==NULL || (session!=NULL && session->SessionInterrupted!=0)) 
	{
		if (userFree == ILibAsyncSocket_MemoryOwnership_CHAIN) { free(buffer); }
		return(ILibWebServer_INVALID_SESSION);
	}
	
	hdr = ILibWebClient_GetHeaderFromDataObject(session->Reserved3);
	if (hdr == NULL)
	{
		if (userFree == ILibAsyncSocket_MemoryOwnership_CHAIN) { free(buffer); }
		return(ILibWebServer_INVALID_SESSION);
	}

	//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if (hdr->VersionLength == 3 && memcmp(hdr->Version,"1.0",3) == 0)
	{
		//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		//
		// This is HTTP/1.0 , so we don't need to do anything special
		//
		if (bufferSize > 0)
		{
			//
			// If there is actually something to send, then send it
			//
			RetVal = ILibWebServer_Send_Raw(session,buffer,bufferSize,userFree,done);
		}
		else if (done != 0)
		{
			//
			// Nothing to send?
			//
			RetVal = ILibWebServer_RequestAnswered(session);
		}
		//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	}
	else
	{
		//
		// This is HTTP/1.1+ , so we need to chunk the body
		//
		if (bufferSize>0)
		{
			//
			// Calculate the length of the body in hex, and create the chunk header
			//
			if ((hex = (char*)malloc(16)) == NULL) ILIBCRITICALEXIT(254);
			hexLen = snprintf(hex, 16, "%X\r\n", bufferSize);
			RetVal = ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;

			//
			// Send the chunk header
			//
			if (ILibWebServer_Send_Raw(session, hex, hexLen, 0, 0)!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR)
			{
				//
				// Send the data
				//
				if (ILibWebServer_Send_Raw(session, buffer, bufferSize, userFree, 0)!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR)
				{
					//
					// The data must be terminated with a CRLF, (don't ask why, it just does)
					//
					RetVal = ILibWebServer_Send_Raw(session, "\r\n", 2, 1, 0);
				}
				else
				{
					RetVal = ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;
				}
			}
			else
			{
				RetVal = ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR;
				//
				// We didn't send the buffer yet, so we need to free it if
				// we own the memory
				//
				if (userFree == ILibAsyncSocket_MemoryOwnership_CHAIN)
				{
					free(buffer);
				}
			}
			//
			// These protections with the ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR check is
			// to prevent broken pipe errors
			//

		}
		if (done!=0 && RetVal != ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RetVal != ILibWebServer_SEND_RESULTED_IN_DISCONNECT &&
			!(hdr->DirectiveLength==4 && strncasecmp(hdr->Directive,"HEAD",4)==0))
		{
			//
			// Terminate the chunk
			//
			RetVal = ILibWebServer_Send_Raw(session,"0\r\n\r\n",5,1,1);
		}
		else if (done!=0 && RetVal >=0)
		{
			RetVal = ILibWebServer_RequestAnswered(session);
		}
	}
	//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}

	if (RetVal!=0 && session->Reserved10 != NULL)
	{
		session->Reserved10 = NULL;
	}
	return(RetVal);
}


/*! \fn ILibWebServer_GetRemoteInterface(struct ILibWebServer_Session *session)
\brief Returns the remote interface of an HTTP session
\param session The ILibWebServer_Session to query
\returns The remote interface
*/
int ILibWebServer_GetRemoteInterface(struct ILibWebServer_Session *session, struct sockaddr *remoteAddress)
{
	return ILibAsyncSocket_GetRemoteInterface(session->Reserved2, remoteAddress);
}

/*! \fn ILibWebServer_GetLocalInterface(struct ILibWebServer_Session *session)
\brief Returns the local interface of an HTTP session
\param session The ILibWebServer_Session to query
\returns The local interface
*/
int ILibWebServer_GetLocalInterface(struct ILibWebServer_Session *session, struct sockaddr *localAddress)
{
	return ILibAsyncSocket_GetLocalInterface(session->Reserved2, localAddress);
}



/*! \fn ILibWebServer_RegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength, ILibWebServer_VirtualDirectory OnVirtualDirectory, void *user)
\brief Registers a Virtual Directory with the ILibWebServer
\param WebServerToken The ILibWebServer to register with
\param vd The virtual directory path
\param vdLength The length of the path
\param OnVirtualDirectory The Virtual Directory handler
\param user User state info to pass on
\returns 0 if successful, nonzero otherwise
*/
int ILibWebServer_RegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength, ILibWebServer_VirtualDirectory OnVirtualDirectory, void *user)
{
	struct ILibWebServer_VirDir_Data *data;
	struct ILibWebServer_StateModule *s = (struct ILibWebServer_StateModule*)WebServerToken;
	if (s->VirtualDirectoryTable == NULL)
	{
		//
		// If no Virtual Directories have been registered yet, we need to initialize
		// the lookup table
		//
		s->VirtualDirectoryTable = ILibInitHashTree();
	}

	if (ILibHasEntry(s->VirtualDirectoryTable, vd, vdLength)!=0)
	{
		//
		// This Virtual Directory was already registered
		//
		return(1);
	}
	else
	{
		//
		// Add the necesary info into the lookup table
		//
		data = (struct ILibWebServer_VirDir_Data*)malloc(sizeof(struct ILibWebServer_VirDir_Data));
		if (data == NULL) ILIBCRITICALEXIT(254);
		data->callback = (voidfp)OnVirtualDirectory;
		data->user = user;
		ILibAddEntry(s->VirtualDirectoryTable, vd, vdLength,data);
	}
	return(0);
}

/*! \fn ILibWebServer_UnRegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength)
\brief UnRegisters a Virtual Directory from the ILibWebServer
\param WebServerToken The ILibWebServer to unregister from
\param vd The virtual directory path
\param vdLength The length of the path
\returns 0 if successful, nonzero otherwise
*/
int ILibWebServer_UnRegisterVirtualDirectory(ILibWebServer_ServerToken WebServerToken, char *vd, int vdLength)
{
	struct ILibWebServer_StateModule *s = (struct ILibWebServer_StateModule*)WebServerToken;
	if (ILibHasEntry(s->VirtualDirectoryTable,vd,vdLength)!=0)
	{
		//
		// The virtual directory registry was found, delete it
		//
		free(ILibGetEntry(s->VirtualDirectoryTable,vd,vdLength));
		ILibDeleteEntry(s->VirtualDirectoryTable,vd,vdLength);
		return(0);
	}
	else
	{
		//
		// Couldn't find the virtual directory registry
		//
		return(1);
	}
}

/*! \fn ILibWebServer_AddRef(struct ILibWebServer_Session *session)
\brief Reference Counter for an \a ILibWebServer_Session object
\param session The ILibWebServer_Session object
*/
void ILibWebServer_AddRef(struct ILibWebServer_Session *session)
{
	SESSION_TRACK(session,"AddRef");
	sem_wait(&(session->Reserved11));
	++session->Reserved12;
	sem_post(&(session->Reserved11));
}

/*! \fn ILibWebServer_Release(struct ILibWebServer_Session *session)
\brief Decrements reference counter for \a ILibWebServer_Session object
\par
When the counter reaches 0, the object is freed
\param session The ILibWebServer_Session object
*/
void ILibWebServer_Release(struct ILibWebServer_Session *session)
{
	int OkToFree = 0;

	SESSION_TRACK(session,"Release");
	sem_wait(&(session->Reserved11));
	if (--session->Reserved12 == 0)
	{
		//
		// There are no more outstanding references, so we can
		// free this thing
		//
		OkToFree = 1;
	}
	sem_post(&(session->Reserved11));
	if (session->SessionInterrupted == 0)
	{
		ILibLifeTime_Remove(((struct ILibWebServer_StateModule*)session->Parent)->LifeTime, session);
	}

	if (OkToFree)
	{
		if (session->SessionInterrupted == 0)
		{
			ILibWebClient_DestroyWebClientDataObject(session->Reserved3);
		}
		SESSION_TRACK(session,"** Destroyed **");
		sem_destroy(&(session->Reserved11));
		free(session);
	}
}
/*! \fn void ILibWebServer_DisconnectSession(struct ILibWebServer_Session *session)
\brief Terminates an ILibWebServer_Session.
\par
<B>Note:</B> Normally this should never be called, as the session is automatically
managed by the system, such that it can take advantage of persistent connections and such.
\param session The ILibWebServer_Session object to disconnect
*/
void ILibWebServer_DisconnectSession(struct ILibWebServer_Session *session)
{
	ILibWebClient_Disconnect(session->Reserved3);
}
/*! \fn void ILibWebServer_Pause(struct ILibWebServer_Session *session)
\brief Pauses the ILibWebServer_Session, such that reading of more data off the network is
temporarily suspended.
\par
<B>Note:</B> This method <B>MUST</B> only be called from either the \a ILibWebServer_Session_OnReceive or \a ILibWebServer_VirtualDirectory handlers.
\param session The ILibWebServer_Session object to pause.
*/
__inline void ILibWebServer_Pause(struct ILibWebServer_Session *session)
{
	ILibWebClient_Pause(session->Reserved3);
}
/*! \fn void ILibWebServer_Resume(struct ILibWebServer_Session *session)
\brief Resumes a paused ILibWebServer_Session object.
\param session The ILibWebServer_Session object to resume.
*/
__inline void ILibWebServer_Resume(struct ILibWebServer_Session *session)
{
	ILibWebClient_Resume(session->Reserved3);
}
/*! \fn void ILibWebServer_OverrideReceiveHandler(struct ILibWebServer_Session *session, ILibWebServer_Session_OnReceive OnReceive)
\brief Overrides the Receive handler, so that the passed in handler will get called whenever data is received.
\param session The ILibWebServer_Session to hijack.
\param OnReceive The handler to handle the received data
*/
__inline void ILibWebServer_OverrideReceiveHandler(struct ILibWebServer_Session *session, ILibWebServer_Session_OnReceive OnReceive)
{
	session->OnReceive = OnReceive;
	session->Reserved13 = 1;
}

/*
void *ILibWebServer_FakeConnect(void *module, void *cd)
{
struct ILibWebServer_StateModule *wsm = (struct ILibWebServer_StateModule*)module;
return ILibAsyncServerSocket_FakeConnect(wsm->ServerSocket, cd);
}
*/

// Private method used to send a file to the web session asynchronously
void ILibWebServer_StreamFileSendOK(struct ILibWebServer_Session *sender)
{
	FILE* pfile;
	size_t len;
	int status = 0;

	pfile = (FILE*)sender->User3;
	while ((len = fread(ILibScratchPad, 1, sizeof(ILibScratchPad), pfile)) != 0)
	{
		status = ILibWebServer_StreamBody(sender, ILibScratchPad, (int)len, ILibAsyncSocket_MemoryOwnership_USER, 0);
		if (status != ILibWebServer_ALL_DATA_SENT || len < sizeof(ILibScratchPad)) break;
	}

	if (len < sizeof(ILibScratchPad) || status < 0)
	{
		// Finished sending the file or got a send error, close the session
		ILibWebServer_StreamBody(sender, NULL, 0, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		fclose(pfile);
	}
}

// Streams a file to the web session asynchronously, closes the session when done.
// Caller must supply a valid file handle.
void ILibWebServer_StreamFile(struct ILibWebServer_Session *session, FILE* pfile)
{
	session->OnSendOK = ILibWebServer_StreamFileSendOK;
	session->User3 = (void*)pfile;
	ILibWebServer_StreamFileSendOK(session);
}


#ifndef MICROSTACK_NOTLS
/*
X509* ILibWebClient_SslGetCert(void* socketModule)
{
	return ILibAsyncSocket_SslGetCert(((struct ILibWebClientDataObject*)socketModule)->SOCK);
}

STACK_OF(X509)*	ILibWebClient_SslGetCerts(void* socketModule)
{
	return ILibAsyncSocket_SslGetCerts(((struct ILibWebClientDataObject*)socketModule)->SOCK);
}
*/

char* ILibWebServer_GetCertificateHash(struct ILibWebServer_Session * sessionToken)
{
	return sessionToken->CertificateHashPtr;
}

char* ILibWebServer_SetCertificateHash(struct ILibWebServer_Session * sessionToken, char* ptr)
{
	return sessionToken->CertificateHashPtr = ptr;
}

char* ILibWebServer_GetCertificateHashEx(struct ILibWebServer_Session * sessionToken)
{
	return sessionToken->CertificateHash;
}
#endif
