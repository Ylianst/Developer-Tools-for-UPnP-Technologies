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


#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
	#include <crtdbg.h>
#endif
#include <string.h>

/*
Include the correct files for the OS. In some cases, the compiler will still try
to find all header files even if only a few are used, causing warnings.
*/

#ifdef _WIN32_WCE
	//#error "PocketPC compilation detected"
	#include "ILibParsers.h"
	#include "ILibAsyncSocket.h"
	#include "UpnpMicroStack.h"

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif WIN32
	//#error "Windows compilation detected"
	#include "ILibParsers.h"
	#include "ILibAsyncSocket.h"
	#include "UpnpMicroStack.h"

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif _POSIX
	#include <stdlib.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <time.h>

	#include "ILibParsers.h"
	#include "ILibAsyncSocket.h"
	#include "UpnpMicroStack.h"

#endif

#include "RemoteIOClientStack.h"

#ifdef _DEBUG
	#define DEBUGONLY(x) x
#endif

#ifndef _DEBUG
	#define DEBUGONLY(x) 
#endif


// This is the main state structure for the Remote I/O Client Stack
struct RIODataObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	void *WorkerChain;
#ifdef _WIN32_WCE
	HANDLE WorkerThread;
#elif WIN32
	HANDLE WorkerThread;
#elif _POSIX
	pthread_t WorkerThread;
#endif

	void*  RIOmicroStack;
	void*  Session;
	int    SessionPort;
	struct RemoteIOChannel* ChannelList;
	char*  PeerConnection;
	int    JumboCommandSize;
	void*  RIOLifeTime;
	int    EventModerationSet;
};

// Chained list structure for Remote I/O Channels
struct RemoteIOChannel
{
	char* name;
	char* uri;

	#ifdef _WIN32_WCE
		int expiration;
	#elif WIN32
		int expiration;
	#elif _POSIX
		struct timeval expiration;
	#endif

	struct RemoteIOChannel* next;
};

struct RIODataObject* RIO = NULL;
static sem_t RemoteIOLock;
static int RemoteIO_RefCounter = 0;

void (*RemoteIOConnectionChanged) (char* PeerConnection) = NULL;
void (*RemoteIOReset) () = NULL;
void (*RemoteIOCommand) (unsigned short command, char* data, int datalength) = NULL;

char*			RemoteIO_FriendlyName = "";
char*			RemoteIO_UniqueIdentifier = "";
char*			RemoteIO_SerialNumber = "";
int				RemoteIO_NotifyPeriod = 0;
unsigned short	RemoteIO_UPnPPort = 0;
char*			RemoteIO_Application = "";
unsigned int	RemoteIO_MaxCommandSize = 0;
int				RemoteIO_DisplayEncoding = 2;
unsigned int	RemoteIO_DisplayWidth = 0;
unsigned int	RemoteIO_DisplayHeight = 0;
char*			RemoteIO_DeviceInformation = "";

// Parse a URI string and returns the IP, port and Path portions of the URI
void ParseUri(char* URI, char** IP, int* Port, char** Path)
{
	struct parser_result *result,*result2,*result3;
	char *TempString,*TempString2;
	int TempStringLength,TempStringLength2;
	
	result = ILibParseString(URI, 0, (int)strlen(URI), "://", 3);
	TempString = result->LastResult->data;
	TempStringLength = result->LastResult->datalength;
	
	/* Parse Path */
	result2 = ILibParseString(TempString,0,TempStringLength,"/",1);
	TempStringLength2 = TempStringLength-result2->FirstResult->datalength;
	*Path = (char*)MALLOC(TempStringLength2+1);
	memcpy(*Path,TempString+(result2->FirstResult->datalength),TempStringLength2);
	(*Path)[TempStringLength2] = '\0';
	
	/* Parse Port Number */
	result3 = ILibParseString(result2->FirstResult->data,0,result2->FirstResult->datalength,":",1);
	if(result3->NumResults==1)
	{
		*Port = 80;
	}
	else
	{
		TempString2 = (char*)MALLOC(result3->LastResult->datalength+1);
		memcpy(TempString2,result3->LastResult->data,result3->LastResult->datalength);
		TempString2[result3->LastResult->datalength] = '\0';
		*Port = atoi(TempString2);
		FREE(TempString2);
	}
	/* Parse IP Address */
	TempStringLength2 = result3->FirstResult->datalength;
	*IP = (char*)MALLOC(TempStringLength2+1);
	memcpy(*IP,result3->FirstResult->data,TempStringLength2);
	(*IP)[TempStringLength2] = '\0';
	ILibDestructParserResults(result3);
	ILibDestructParserResults(result2);
	ILibDestructParserResults(result);
}

// Called by the UPnP Remote I/O Microstack
// Implements the ForceDisconnect call, lets a CP disconnect this RIO client.
// If this RIO client is not connected, this call has no effect.
void UpnpRemoteIOClient_RemoteIO_ForceDisconnection(void* upnptoken)
{
	sem_wait(&RemoteIOLock);
	if (RIO->PeerConnection != NULL)
	{
		// Disconnect the socket
		ILibAsyncSocket_Disconnect(RIO->Session);

		// Set new connection state
		free(RIO->PeerConnection);
		RIO->PeerConnection = NULL;

		// Disconnect the session
		if (RIO->Session != NULL)
		{
			// Disconnect RemoteIOSession
			ILibAsyncSocket_Disconnect(RIO->Session);
		}
		// Event the new connection
		UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,"");
		UpnpResponse_RemoteIOClient_RemoteIO_ForceDisconnection(upnptoken);
		sem_post(&RemoteIOLock);

		// Event the user
		if (RemoteIOConnectionChanged != NULL) RemoteIOConnectionChanged(RIO->PeerConnection);
	}
	else
	{
		UpnpResponse_RemoteIOClient_RemoteIO_ForceDisconnection(upnptoken);
		sem_post(&RemoteIOLock);
	}
}

// Called by the UPnP Remote I/O Microstack
// Implements the GetPeerConnection call. Allows a CP to get the URI to
// which this RIO client is currently connected to.
void UpnpRemoteIOClient_RemoteIO_GetPeerConnection(void* upnptoken)
{
	sem_wait(&RemoteIOLock);
	if (RIO->PeerConnection == NULL)
	{
		UpnpResponse_RemoteIOClient_RemoteIO_GetPeerConnection(upnptoken,"");
	}
	else
	{
		UpnpResponse_RemoteIOClient_RemoteIO_GetPeerConnection(upnptoken,RIO->PeerConnection);
	}
	sem_post(&RemoteIOLock);
}

// Called by the UPnP Remote I/O Microstack
// Implements the ForceReset call, lets a CP force reset this RIO client.
// Used mostly for debugging
void UpnpRemoteIOClient_RemoteIO_ForceReset(void* upnptoken)
{
	UpnpResponse_RemoteIOClient_RemoteIO_ForceReset(upnptoken);

	// Event the user
	if (RemoteIOReset != NULL) RemoteIOReset();
}

// Called by the UPnP Remote I/O Microstack
// Implements the ForceDisconnect call, lets a CP connect this RIO client
// to a URI if, and only if, this RIO client is not currently connected.
void UpnpRemoteIOClient_RemoteIO_SetPeerInterlock(void* upnptoken,char* PeerConnection)
{
	struct parser_result* ParsedAddress = NULL;
	char* RemoteIOSessionPath = NULL;
	char* RemoteIOSessionAddress = NULL;
	int address = 0;

	if (PeerConnection == NULL || (int)strlen(PeerConnection) < 7)
	{
		UpnpResponse_Error(upnptoken,700,"Invalid PeerConnection");
		return;
	}

	sem_wait(&RemoteIOLock);
	if (RIO->PeerConnection == NULL)
	{
		RIO->PeerConnection = (char*)malloc((int)strlen(PeerConnection) + 1);
		strcpy(RIO->PeerConnection,PeerConnection);

		// Event the new connection
		UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,RIO->PeerConnection);

		// Connect session
		ParseUri(RIO->PeerConnection,&RemoteIOSessionAddress,&RIO->SessionPort,&RemoteIOSessionPath);
		free(RemoteIOSessionPath);
		
		ParsedAddress = ILibParseString(RemoteIOSessionAddress,0,(int)strlen(RemoteIOSessionAddress),".",1);
		
		address  = atoi(ParsedAddress->FirstResult->data);
		address += atoi(ParsedAddress->FirstResult->NextResult->data) << 8;
		address += atoi(ParsedAddress->FirstResult->NextResult->NextResult->data) << 16;
		address += atoi(ParsedAddress->FirstResult->NextResult->NextResult->NextResult->data) << 24;

		ILibAsyncSocket_ConnectTo(RIO->Session,0,address,RIO->SessionPort,NULL,NULL);
		ILibDestructParserResults(ParsedAddress);
		free(RemoteIOSessionAddress);
		UpnpResponse_RemoteIOClient_RemoteIO_SetPeerInterlock(upnptoken,RIO->PeerConnection);
		sem_post(&RemoteIOLock);

		// Event the user
		//if (RemoteIOConnectionChanged != NULL) RemoteIOConnectionChanged(RIO->PeerConnection);
	}
	else
	{
		UpnpResponse_RemoteIOClient_RemoteIO_SetPeerInterlock(upnptoken,RIO->PeerConnection);
		sem_post(&RemoteIOLock);
	}
}

// Called by the UPnP Remote I/O Microstack
// Implements the GetDeviceInformation call, lets a CP get information about this RIO client.
void UpnpRemoteIOClient_RemoteIO_GetDeviceInformation(void* upnptoken)
{
	UpnpResponse_RemoteIOClient_RemoteIO_GetDeviceInformation(upnptoken,RemoteIO_Application,RemoteIO_MaxCommandSize,RemoteIO_DisplayEncoding,RemoteIO_DisplayWidth,RemoteIO_DisplayHeight,RemoteIO_DeviceInformation);
}

// Called by the UPnP Remote I/O Microstack
// Implements the SetPeerOverride call, lets a CP connect this RIO client to
// a new URI. If this RIO client is currently connected, it will disconnect and
// switch to the new URI.
void UpnpRemoteIOClient_RemoteIO_SetPeerOverride(void* upnptoken,char* PeerConnection)
{
	struct parser_result* ParsedAddress = NULL;
	char* RemoteIOSessionPath = NULL;
	char* RemoteIOSessionAddress = NULL;
	int address = 0;

	
	if (upnptoken && (PeerConnection == NULL || (int)strlen(PeerConnection) < 7))
	{
		UpnpResponse_Error(upnptoken,700,"Invalid PeerConnection");
		return;
	}

	sem_wait(&RemoteIOLock);
	if (RIO->PeerConnection == NULL || strcmp(RIO->PeerConnection,PeerConnection) != 0)
	{
		if (RIO->PeerConnection != NULL)
		{
			// Disconnect the socket
			ILibAsyncSocket_Disconnect(RIO->Session);
			free(RIO->PeerConnection);
		}

		// Set the new session URI
		RIO->PeerConnection = (char*)malloc((int)strlen(PeerConnection) + 1);
		strcpy(RIO->PeerConnection,PeerConnection);

		// Event the new connection
		UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,RIO->PeerConnection);

		// Connect session
		ParseUri(RIO->PeerConnection,&RemoteIOSessionAddress,&RIO->SessionPort,&RemoteIOSessionPath);
		free(RemoteIOSessionPath);

		ParsedAddress = ILibParseString(RemoteIOSessionAddress,0,(int)strlen(RemoteIOSessionAddress),".",1);

		address  = atoi(ParsedAddress->FirstResult->data);
		address += atoi(ParsedAddress->FirstResult->NextResult->data) << 8;
		address += atoi(ParsedAddress->FirstResult->NextResult->NextResult->data) << 16;
		address += atoi(ParsedAddress->FirstResult->NextResult->NextResult->NextResult->data) << 24;

		ILibAsyncSocket_ConnectTo(RIO->Session,0,address,RIO->SessionPort,NULL,NULL);
		ILibDestructParserResults(ParsedAddress);
		free(RemoteIOSessionAddress);
		sem_post(&RemoteIOLock);

		// Event the user
		if (RemoteIOConnectionChanged != NULL) RemoteIOConnectionChanged(RIO->PeerConnection);
	}
	else 
	{
		sem_post(&RemoteIOLock);
	}
	
	if(upnptoken) {UpnpResponse_RemoteIOClient_RemoteIO_SetPeerOverride(upnptoken);}
}

// Called by the UPnP Remote I/O Microstack
// Implements the GetInputSetup call, lets a CP get information about Remote Input
// for this device. This information is rarly used, but good for future proofing.
void UpnpRemoteIOClient_RemoteInput_GetInputSetup(void* upnptoken)
{
	UpnpResponse_RemoteIOClient_RemoteInput_GetInputSetup(upnptoken,"intel.desktop");
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputKeyPress(void* upnptoken,int key)
{
	RemoteIO_SendCommand(RIO_KEY_PRESS,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
	UpnpResponse_RemoteIOClient_RemoteInput_InputKeyPress(upnptoken);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputKeyUp(void* upnptoken,int key)
{
	RemoteIO_SendCommand(RIO_KEY_UP,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
	UpnpResponse_RemoteIOClient_RemoteInput_InputKeyUp(upnptoken);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputKeyDown(void* upnptoken,int key)
{
	RemoteIO_SendCommand(RIO_KEY_DOWN,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
	UpnpResponse_RemoteIOClient_RemoteInput_InputKeyDown(upnptoken);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputMouseUp(void* upnptoken,int X,int Y,int Button)
{
	RemoteIO_SendMouseUp(X,Y,Button);
	UpnpResponse_RemoteIOClient_RemoteInput_InputMouseUp(upnptoken);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputMouseDown(void* upnptoken,int X,int Y,int Button)
{
	RemoteIO_SendMouseDown(X,Y,Button);
	UpnpResponse_RemoteIOClient_RemoteInput_InputMouseDown(upnptoken);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void UpnpRemoteIOClient_RemoteInput_InputMouseMove(void* upnptoken,int X,int Y)
{
	RemoteIO_SendMouseMove(X,Y);
	UpnpResponse_RemoteIOClient_RemoteInput_InputMouseMove(upnptoken);
}

// PRIVATE - Creates a list of all channels creating a char* that can be
// send to the CP as a result of and actions or as an event.
// Make sure to call free() on the return value when done, also, make sure
// to hold the Remote I/O lock before calling the method (to lock the channel list)
char* UpnpRemoteIOClient_ChannelManager_CreateChannelList()
{
	// Scan the channel list and compute the total size
	int totalsize = 1;
	char* channellist;
	char* channellistindex;
	int templength;
	struct RemoteIOChannel* channelindex = RIO->ChannelList;
	while (channelindex != NULL)
	{
		totalsize += (int)strlen(channelindex->name);
		totalsize += (int)strlen(channelindex->uri);
		totalsize += 4;
		channelindex = channelindex->next;
	}

	if (totalsize == 0)
	{
		// Creat an empty channel list
		channellist = (char*)malloc(1);
		channellist[0] = 0;
		return channellist;
	}
	else
	{
		channellist = (char*)malloc(totalsize);
		channellistindex = channellist;
		channelindex = RIO->ChannelList;
		while (channelindex != NULL)
		{
			// Copy name
			templength = (int)strlen(channelindex->name);
			memcpy(channellistindex,channelindex->name,templength);
			channellistindex[templength  ] = '\r';
			channellistindex[templength+1] = '\n';
			channellistindex += (templength + 2);

			// Copy URI
			templength = (int)strlen(channelindex->uri);
			memcpy(channellistindex,channelindex->uri,templength);
			channellistindex[templength  ] = '\r';
			channellistindex[templength+1] = '\n';
			channellistindex += (templength + 2);

			channelindex = channelindex->next;
		}
		channellistindex[0] = 0;;
		return channellist;
	}
}

// PRIVATE - Called by the lifetime monitor when a channel expires. This call will
// remove the channel by calling the UpnpRemoteIOClient_ChannelManager_UnregisterChannel method.
void RemoteIO_ChannelExpireSink(void *data)
{
	struct RemoteIOChannel* channel = (struct RemoteIOChannel*)data;
	char* channeluri = MALLOC(strlen(channel->uri)+1);
	strcpy(channeluri,channel->uri);
	UpnpRemoteIOClient_ChannelManager_UnregisterChannel(NULL,channeluri);
	free(channeluri);
}

// PRIVATE - Called internaly to event the current list of channels. This event is
// moderated, so this call will be invoked by the lifetime monitor once the moderation
// time expired.
void RemoteIO_EventChannelList(void *data)
{
	char* channellist;

	// Event available channels
	sem_wait(&RemoteIOLock);
	channellist = UpnpRemoteIOClient_ChannelManager_CreateChannelList();
	UpnpSetState_RemoteIOClient_ChannelManager_RegisteredChannelList(RIO->RIOmicroStack,channellist);
	free(channellist);
	RIO->EventModerationSet = 0;
	sem_post(&RemoteIOLock);
}

// Called by the UPnP Remote I/O Microstack
// Implements the RegisterChannel call, lets the CP register a new RIO channels for a
// certain amont of time. The CP must re-register the channel from time-to-time to
// prevent the channel from expiring.
void UpnpRemoteIOClient_ChannelManager_RegisterChannel(void* upnptoken,char* Name,char* PeerConnection,int Timeout)
{
	// Scan the channel list for an existing channel
	struct RemoteIOChannel* channelindex = RIO->ChannelList;
	struct RemoteIOChannel* newchannel;

	printf("RegisterChannel[%s] (%d): %s\r\n",PeerConnection,Timeout,Name);

	if (PeerConnection == NULL)
	{
		if (upnptoken != NULL) UpnpResponse_Error(upnptoken,800,"Invalid PeerConnection URI");
		return;
	}

	sem_wait(&RemoteIOLock);

	while (channelindex != NULL)
	{
		// Look for a match
		if (strcmp(channelindex->uri,PeerConnection) == 0) break;
		channelindex = channelindex->next;
	}

	if (channelindex != NULL)
	{
		// Update the expiration time
		ILibLifeTime_Remove(RIO->RIOLifeTime,channelindex);
		#ifdef _WIN32_WCE
			channelindex->expiration = (GetTickCount() / 1000) + Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,channelindex,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#elif WIN32
			channelindex->expiration = (GetTickCount() / 1000) + Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,channelindex,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#elif _POSIX
			gettimeofday(&(channelindex->expiration),NULL);
			(channelindex->expiration).tv_sec += (int)Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,channelindex,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#endif
	}
	else
	{
		// Add a new channel to the channel list
		newchannel = (struct RemoteIOChannel*)malloc(sizeof(struct RemoteIOChannel));
		newchannel->name = (char*)malloc(strlen(Name)+1);
		strcpy(newchannel->name,Name);
		newchannel->uri = (char*)malloc(strlen(PeerConnection)+1);
		strcpy(newchannel->uri,PeerConnection);
		#ifdef _WIN32_WCE
			newchannel->expiration = (GetTickCount() / 1000) + Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,newchannel,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#elif WIN32
			newchannel->expiration = (GetTickCount() / 1000) + Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,newchannel,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#elif _POSIX
			gettimeofday(&(newchannel->expiration),NULL);
			(newchannel->expiration).tv_sec += (int)Timeout;
			ILibLifeTime_Add(RIO->RIOLifeTime,newchannel,Timeout,&RemoteIO_ChannelExpireSink, NULL);
		#endif
		newchannel->next = RIO->ChannelList;
		RIO->ChannelList = newchannel;

		// Set the channels to be evented
		if (RIO->EventModerationSet == 0)
		{
			ILibLifeTime_Add(RIO->RIOLifeTime,NULL,2,&RemoteIO_EventChannelList, NULL);
			RIO->EventModerationSet = 1;
		}
	}

	UpnpResponse_RemoteIOClient_ChannelManager_RegisterChannel(upnptoken);

	sem_post(&RemoteIOLock);
}

// Called by the UPnP Remote I/O Microstack
// Implements the RegisterChannel call, lets the CP un-register a RIO channel.
void UpnpRemoteIOClient_ChannelManager_UnregisterChannel(void* upnptoken,char* PeerConnection)
{
	// Scan the channel list for an existing channel
	struct RemoteIOChannel* channelprevious = NULL;
	struct RemoteIOChannel* channelindex = RIO->ChannelList;

	printf("UnRegisterChannel: %s\r\n",PeerConnection);

	if (PeerConnection == NULL)
	{
		if (upnptoken != NULL) UpnpResponse_Error(upnptoken,800,"Invalid PeerConnection URI");
		return;
	}

	sem_wait(&RemoteIOLock);
	while (channelindex != NULL)
	{
		// Look for a match
		if (strcmp(channelindex->uri,PeerConnection) == 0) break;
		channelprevious = channelindex;
		channelindex = channelindex->next;
	}

	// Delete the channel from the list, and free the channel struct
	if (channelindex != NULL)
	{
		ILibLifeTime_Remove(RIO->RIOLifeTime,channelindex);

		if (channelprevious == NULL)
		{
			RIO->ChannelList = channelindex->next;
		}
		else
		{
			channelprevious->next = channelindex->next;
		}
		free(channelindex->name);
		free(channelindex->uri);
		free(channelindex);

		// Set the channels to be evented
		if (RIO->EventModerationSet == 0)
		{
			ILibLifeTime_Add(RIO->RIOLifeTime,NULL,2,&RemoteIO_EventChannelList, NULL);
			RIO->EventModerationSet = 1;
		}
	}
	
	if (upnptoken != NULL) UpnpResponse_RemoteIOClient_ChannelManager_UnregisterChannel(upnptoken);

	sem_post(&RemoteIOLock);
}

// Called by the UPnP Remote I/O Microstack
// Implements the RegisterChannel call, lets the CP clear all registered RIO channels.
// Usualy, RIO Servers should check to see if their own channels are in the registration
// list. If not, they re-register. In practice, this call will cause all servers to immidiatly
// re-register all channels.
void UpnpRemoteIOClient_ChannelManager_ClearAllChannels(void* upnptoken)
{
	// Scan the channel list and delete
	struct RemoteIOChannel* channelnext;
	struct RemoteIOChannel* channelindex = RIO->ChannelList;
	RIO->ChannelList = NULL;
	
	sem_wait(&RemoteIOLock);

	// No channels to clean, respond and exit
	if (channelindex == NULL)
	{
		UpnpResponse_RemoteIOClient_ChannelManager_ClearAllChannels(upnptoken);
		sem_post(&RemoteIOLock);
		return;
	}

	// Clear all of the channels one by one.
	while (channelindex != NULL)
	{
		channelnext = channelindex->next;
		free(channelindex->name);
		free(channelindex->uri);
		free(channelindex);
		channelindex = channelnext;
	}

	if(upnptoken)
	{
		UpnpResponse_RemoteIOClient_ChannelManager_ClearAllChannels(upnptoken);

		// Set the channels to be evented
		if (RIO->EventModerationSet == 0)
		{
			ILibLifeTime_Add(RIO->RIOLifeTime,NULL,2,&RemoteIO_EventChannelList, NULL);
			RIO->EventModerationSet = 1;
		}
	}

	sem_post(&RemoteIOLock);
}

// Called by the UPnP Remote I/O Microstack
// Implements the GetRegisteredChannelList call, lets the CP obtain the complete
// list of all registered channels.
void UpnpRemoteIOClient_ChannelManager_GetRegisteredChannelList(void* upnptoken)
{
	char* channellist;
	sem_wait(&RemoteIOLock);
	channellist = UpnpRemoteIOClient_ChannelManager_CreateChannelList();
	UpnpResponse_RemoteIOClient_ChannelManager_GetRegisteredChannelList(upnptoken,channellist);
	free(channellist);
	sem_post(&RemoteIOLock);
}

#ifndef EXCLUDE_RIO_PRESENTATION_REQUEST 
void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	breakme
	//UpnpPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}
#endif

// Normal coding rules are suspended in the function for the benefit of speed.
// This method supports Jumbo commands, that feature can be removed to optimize on sub-64k devices.
// This method supports the obsolete BigImage command - Jumbo should always be used
void OnRemoteIODataSink(void* socketModule,char* buf,int *p_beginPointer, int endPointer, void (**InterruptPtr)(void *socketModule, void *user),void **user, int *PAUSE)
{
	int datalength = 0;
	unsigned char* buffer = buf;
	int command;

	ProcessNextCommand:
	if (endPointer < 4) return;

	if (RIO->JumboCommandSize == 0)
	{
		datalength = buffer[0] | (buffer[1] << 8);
	}
	else
	{
		datalength = RIO->JumboCommandSize;
	}

	command = (unsigned short)(buffer[2] | (buffer[3] << 8));	

	if (command == RIO_XWPC_BIGIMAGE)
	{
		if (endPointer < 8) return;
		datalength = 8 + (buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24));
	}

	if (endPointer >= datalength)
	{
		if (command == RIO_JUMBO)
		{
			RIO->JumboCommandSize = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
		}
		else
		{
			// The user is assumer to have the RemoteIOCommand pointer set, no check.
			RemoteIOCommand((unsigned short)command,(char*)(buffer+4),datalength-4);
			RIO->JumboCommandSize = 0;
		}
		*p_beginPointer += datalength;
		buffer += datalength;
		endPointer -= datalength;
		goto ProcessNextCommand;
	}
}

// PRIVATE - Called by the AsyncSocket module when the XRT connection is established. Once connected,
// the stack immidiatly sends the XRT REQUEST command to get things started.
void OnRemoteIOConnectSink(void* socketModule, int Connected, void *user)
{
	if (Connected != 0)
	{
		// Send REQUEST command
		RemoteIO_SendCommand(RIO_REQUEST,RIO->PeerConnection,(int)strlen(RIO->PeerConnection),ILibAsyncSocket_MemoryOwnership_USER);
	}
	else
	{
		sem_wait(&RemoteIOLock);

		// Set new connection state
		if (RIO->PeerConnection != NULL) free(RIO->PeerConnection);
		RIO->PeerConnection = NULL;

		// Event the new connection
		UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,"");

		sem_post(&RemoteIOLock);

		// Event the user
		if (RemoteIOConnectionChanged != NULL) RemoteIOConnectionChanged(RIO->PeerConnection);
	}
}

// PRIVATE - Called by the AsyncSocket module when the XRT socket as disconnected.
// This will cause the RIO Client to return to un-connected state.
void OnRemoteIODisconnectSink(void* socketModule, void *user)
{
	// Set new connection state
	free(RIO->PeerConnection);
	RIO->PeerConnection = NULL;

	// Event the user
	if (RemoteIOConnectionChanged != NULL) RemoteIOConnectionChanged(RIO->PeerConnection);

	// Event the new connection
	UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,"");
}

// PRIVATE - Called by the chaining system to clean up. Usualy called before the
// application is terminated.
void RemoteIODestroyChain(void* object)
{
	struct RIODataObject* remoteio = (struct RIODataObject*)object;
	ILibStopChain(remoteio->WorkerChain);

#ifdef _WIN32_WCE
	WaitForSingleObject(RIO->WorkerThread,INFINITE);
#elif WIN32
	WaitForSingleObject(RIO->WorkerThread,INFINITE);
#elif _POSIX
	pthread_join(RIO->WorkerThread,NULL);
#endif

	UpnpRemoteIOClient_ChannelManager_ClearAllChannels(NULL);	
	if(RIO->PeerConnection!=NULL) {free(RIO->PeerConnection);}

	RemoteIO_RefCounter--;
	if (RemoteIO_RefCounter == 0) sem_destroy(&RemoteIOLock);	
}

// PRIVATE - Entry point for the XRT thread. In the Remote I/O stack, the XRT
// commands are handled by a different thread than the UPnP Microstack thread.
// Both modules could have been chained on the same thread, but it is expected that
// XRT image decoding will take a bit of time and may lock up the UPnP processing.
// So it is best to keep them seperate.
#ifdef _WIN32_WCE
DWORD WINAPI RemoteIOSessionThreadEntry(void* param)
#elif WIN32
DWORD WINAPI RemoteIOSessionThreadEntry(void* param)
#elif _POSIX
void* RemoteIOSessionThreadEntry(void* param)
#endif
{
	ILibStartChain(RIO->WorkerChain);
	return(0);
}

// PUBLIC - Creates a Remote I/O Client Device Stack. This stack does not include the UPnP stack
// one must be build into the project with the "Upnp" prefix using Intel Device Builder. All
// callbacks must be externs. Remote I/O can be a root or embedded device.
void* CreateRemoteIO(void* Chain, void* UpnpStack)
{
	if (RemoteIO_RefCounter == 0) sem_init(&RemoteIOLock,0,1);
	RemoteIO_RefCounter++;

	RIO = (struct RIODataObject*)malloc(sizeof(struct RIODataObject));
	memset(RIO,0,sizeof(struct RIODataObject));

	// Start the new Remote IO session
	RIO->Destroy = &RemoteIODestroyChain;
	RIO->WorkerChain = ILibCreateChain();
	RIO->Session = ILibCreateAsyncSocketModule(
		RIO->WorkerChain,
		RemoteIO_MaxCommandSize+4,
		&OnRemoteIODataSink,
		&OnRemoteIOConnectSink,
		&OnRemoteIODisconnectSink,
		NULL);

	#ifdef _WIN32_WCE
		RIO->WorkerThread = (void*)CreateThread(NULL,0,RemoteIOSessionThreadEntry,NULL,0,NULL);
	#elif WIN32
		RIO->WorkerThread = (void*)CreateThread(NULL,0,RemoteIOSessionThreadEntry,NULL,0,NULL);
	#elif _POSIX
		pthread_create(&(RIO->WorkerThread),NULL,RemoteIOSessionThreadEntry,NULL);
		pthread_detach(RIO->WorkerThread);
	#endif
	
	RIO->RIOmicroStack = UpnpStack;

	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(RIO->RIOmicroStack,"");
	UpnpSetState_RemoteIOClient_ChannelManager_RegisteredChannelList(RIO->RIOmicroStack,"");

	RIO->RIOLifeTime = ILibCreateLifeTime(Chain);
	//ILibAddToChain(Chain,RIO->RIOLifeTime);
	ILibAddToChain(Chain,RIO);

	return RIO;
}

// PRIVATE - User internaly to build and send XRT commands. If XRT is not connected,
// the command is ignored.
void RemoteIO_SendCommand(unsigned short command, char* data, int datalength, int userfree)
{
	char header[4];
	header[0] = (datalength+4) & 0xFF;
	header[1] = (datalength+4) >> 8;
	header[2] = command & 0xFF;
	header[3] = command >> 8;

	if (RIO != NULL)
	{
		if (datalength > 0xFFFB) RemoteIO_SendCommand(RIO_JUMBO,(char*)&datalength,4,ILibAsyncSocket_MemoryOwnership_USER);
		ILibAsyncSocket_Send(RIO->Session,header,4,ILibAsyncSocket_MemoryOwnership_USER);
		if (data != NULL) ILibAsyncSocket_Send(RIO->Session,data,datalength,userfree);
	}
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendKeyPress(int key)
{
	RemoteIO_SendCommand(RIO_KEY_PRESS,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendKeyUp(int key)
{
	RemoteIO_SendCommand(RIO_KEY_UP,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendKeyDown(int key)
{
	RemoteIO_SendCommand(RIO_KEY_DOWN,(char*)&key,4,ILibAsyncSocket_MemoryOwnership_USER);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendMouseUp(int X,int Y,int Button)
{
	char data[12];
	memcpy(data+0,(char*)&X,4);
	memcpy(data+4,(char*)&Y,4);
	memcpy(data+8,(char*)&Button,4);
	RemoteIO_SendCommand(RIO_MOUSE_UP,(char*)&data,12,ILibAsyncSocket_MemoryOwnership_USER);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendMouseDown(int X,int Y,int Button)
{
	char data[12];
	memcpy(data+0,(char*)&X,4);
	memcpy(data+4,(char*)&Y,4);
	memcpy(data+8,(char*)&Button,4);
	RemoteIO_SendCommand(RIO_MOUSE_DOWN,(char*)&data,12,ILibAsyncSocket_MemoryOwnership_USER);
}

// Called by the UPnP Remote I/O Microstack
// Implements the InputKeyPress call, lets a CP inject user input into this RIO client.
void RemoteIO_SendMouseMove(int X,int Y)
{
	char data[12];
	memcpy(data+0,(char*)&X,4);
	memcpy(data+4,(char*)&Y,4);
	memset(data+8,0,4);
	RemoteIO_SendCommand(RIO_MOUSE_MOVE,(char*)&data,12,ILibAsyncSocket_MemoryOwnership_USER);
}

// PUBLIC - Used by the programmer to lock the RIO Stack state. This is useful
// for example to get the list of available channels without having the list while
// looking at each node one-by-one.
// Must never be called twice in a row, will lock up.
void RemoteIO_Lock()
{
	sem_wait(&RemoteIOLock);
}

// PUBLIC - Must be called each time RemoteIO_Lock is called.
void RemoteIO_UnLock()
{
	sem_post(&RemoteIOLock);
}
