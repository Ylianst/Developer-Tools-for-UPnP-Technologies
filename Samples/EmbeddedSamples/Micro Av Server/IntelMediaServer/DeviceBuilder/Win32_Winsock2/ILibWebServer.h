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

#ifndef __ILibWebServer__
#define __ILibWebServer__

#define ILibWebServer_SEND_RESULTED_IN_DISCONNECT -2

struct ILibWebServer_Session;
typedef void (*ILibWebServer_Session_OnReceive)\
				(struct ILibWebServer_Session *sender,\
					int InterruptFlag,\
					struct packetheader *header,\
					char *bodyBuffer,\
					int *beginPointer,\
					int endPointer,\
					int done);

struct ILibWebServer_Session
{
	ILibWebServer_Session_OnReceive OnReceive;
	void (*OnDisconnect)(struct ILibWebServer_Session *sender);
	void (*OnSendOK)(struct ILibWebServer_Session *sender);
	void *Parent;
	void *User;
	void *User2;

	void *Reserved1;	// AsyncServerSocket
	void *Reserved2;	// ConnectionToken
	void *Reserved3;	// WebClientDataObject
	void *Reserved7;	// VirtualDirectory
	int Reserved4;	// Request Answered Flag (set by send)
	int Reserved8;	// RequestAnswered Method Called
	int Reserved5;	// Request Made Flag
	int Reserved6;	// Close Override Flag
};

typedef void (*ILibWebServer_Session_OnSession)(struct ILibWebServer_Session *SessionToken, void *User);
typedef void (*ILibWebServer_VirtualDirectory)(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user);

void ILibWebServer_SetTag(void *WebServerToken, void *Tag);
void *ILibWebServer_GetTag(void *WebServerToken);

void *ILibWebServer_Create(void *Chain, int MaxConnections, int PortNumber,ILibWebServer_Session_OnSession OnSession, void *User);
int ILibWebServer_RegisterVirtualDirectory(void *WebServerToken, char *vd, int vdLength, ILibWebServer_VirtualDirectory OnVirtualDirectory, void *user);
int ILibWebServer_UnRegisterVirtualDirectory(void *WebServerToken, char *vd, int vdLength);

int ILibWebServer_Send(struct ILibWebServer_Session *session, struct packetheader *packet);
int ILibWebServer_Send_Raw(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done);

#define ILibWebServer_Session_GetPendingBytesToSend(session) ILibAsyncServerSocket_GetPendingBytesToSend(session->Reserved1,session->Reserved2)
#define ILibWebServer_Session_GetTotalBytesSent(session) ILibAsyncServerSocket_GetTotalBytesSent(session->Reserved1,session->Reserved2)
#define ILibWebServer_Session_ResetTotalBytesSent(session) ILibAsyncServerSocket_ResetTotalBytesSent(session->Reserved1,session->Reserved2)

unsigned short ILibWebServer_GetPortNumber(void *WebServerToken);
int ILibWebServer_GetLocalInterface(struct ILibWebServer_Session *session);

int ILibWebServer_StreamHeader(struct ILibWebServer_Session *session, struct packetheader *header);
int ILibWebServer_StreamBody(struct ILibWebServer_Session *session, char *buffer, int bufferSize, int userFree, int done);

int ILibWebServer_StreamHeader_Raw(struct ILibWebServer_Session *session, int StatusCode,char *StatusData,char *ResponseHeaders, int ResponseHeaders_FREE);

#endif
