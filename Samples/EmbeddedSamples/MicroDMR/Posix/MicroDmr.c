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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef _POSIX
#include <signal.h>
#include <time.h>
#include <unistd.h>
#endif

#include "ILibParsers.h"

#include "RemoteIOClientStack.h"

#include "MicroMediaRenderer.h"
#include "RendererStateLogic.h"
#include "Emulator_Methods.h"
#include "HttpPlaylistParser.h"

/* [CODEC]: BEGIN */
#include "CodecWrapper.h"
/* [CODEC]: END */

#include "XrtVideo.h"


void *DmrStack;
void *DmrChain;
void *DmrLTM;
void *DmrRenderer;
void *DmrRendererLogic;
void *DmrExtendedM3uProcessor;

#ifdef _POSIX
void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;
#endif

#ifdef _WINSOCK1
void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;
#endif

#ifdef _WINSOCK2
DWORD UpnpMonitorSocketReserved;
WSAOVERLAPPED UpnpMonitorSocketStateObject;
SOCKET UpnpMonitorSocket;
#endif




void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("hello");
	/* TODO: Add Web Response Code Here... */
	
	//UpnpPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}

#ifdef _POSIX
/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *
 *	This applies only if posix is used.
 */
void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(DmrStack);
		
		FREE(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
}
#endif

#ifdef _WINSOCK1
/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *
 *	This applies only if winsock1 is used.
 */
void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(DmrStack);
		
		FREE(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
}
#endif

#ifdef _WINSOCK2
/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *
 *	This applies only if winsock2 is used.
 */
void CALLBACK UpnpIPAddressMonitor
	(
	IN DWORD dwError, 
	IN DWORD cbTransferred, 
	IN LPWSAOVERLAPPED lpOverlapped, 
	IN DWORD dwFlags 
	)
{
	UpnpIPAddressListChanged(UpnpStack);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
}
#endif

#ifdef _POSIX
void BreakSink(int s)
{
	ILibStopChain(DmrChain);
}
#endif

#ifdef WIN32
DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(DmrChain);
	return 0;
}
#endif


int main(void)
{
	char udn[20];
	int i;
	char friendlyname[100];

	#ifdef WIN32
	WSADATA wsaData;
	#endif

	/* Randomized udn generation */
	#ifdef WIN32
	srand(GetTickCount());
	#endif

	#ifdef _POSIX
	srand((unsigned int)time(NULL));
	#endif

	for (i=0;i<19;i++)
	{
		udn[i] = (rand() % 25) + 66;
	}
	udn[19] = 0;

	/* generate a friendly name that has the host name in it */
	#ifdef WIN32
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {exit(1);}
	#endif
	memcpy(friendlyname,"Intel MicroDMR (",16);
	gethostname(friendlyname+16,75);
	#ifdef WIN32
	memcpy(friendlyname+strlen(friendlyname),")/Win32\0",8);
	#endif
	#ifdef _POSIX
	memcpy(friendlyname+strlen(friendlyname),")/Posix\0",8);
	#endif

	/* Setup the callbacks */
	RemoteIOConnectionChanged = &XrtVideo_ConnectionChangedSink;
	RemoteIOReset = &XrtVideo_ResetSink;
	RemoteIOCommand = &XrtVideo_CommandSink;
	
	/* Setup the Remote IO Global Values */
	RemoteIO_Application		= "XRT20:Sample Client";
	RemoteIO_MaxCommandSize		= 65000;			// Set the maximum command size, keep this at 64k
	RemoteIO_DisplayEncoding	= RemoteIO_JPEG;	// Set the image format, see (enum RemoteIOImageFormat) for complete list.
	RemoteIO_DisplayWidth		= 640;				// Set the display width
	RemoteIO_DisplayHeight		= 480;				// Set the display height
	RemoteIO_DeviceInformation	= "";				// Set propriatary information about the device

	/*
	 *	Set of commands that can be sent back to the host PC. These can be used even
	 *	if remoting is not connected. For a device with an IR remote, only SendKeyPress is used.	
	 */
	/*
	RemoteIO_SendCommand(unsigned short command, char* data, int datalength);
	RemoteIO_SendKeyPress(int key);
	RemoteIO_SendKeyUp(int key);
	RemoteIO_SendKeyDown(int key);
	RemoteIO_SendMouseUp(int X,int Y,int Button);
	RemoteIO_SendMouseDown(int X,int Y,int Button);
	RemoteIO_SendMouseMove(int X,int Y);
	*/

	/* renderer callbacks. */
	MROnVolumeChangeRequest			= &MROnVolumeChangeRequestSink;
	MROnMuteChangeRequest			= &MROnMuteChangeRequestSink;
	MROnMediaChangeRequest			= &MROnMediaChangeRequestSink;
	MROnGetPositionRequest			= &MROnGetPositionRequestSink;
	MROnSeekRequest					= &MROnSeekRequestSink;
	MROnNextPreviousRequest			= &MROnNextPreviousRequestSink;
	MROnStateChangeRequest			= &MROnStateChangeRequestSink;
	MROnPlayModeChangeRequest		= &MROnPlayModeChangeRequestSink;

	printf("Intel MicroDMR Sample Client\r\n");


	DmrChain = MR_RendererChain = ILibCreateChain();
	DmrLTM = ILibCreateLifeTime(DmrChain);

	#ifdef _DEBUG
	printf("DmrLTM=%p\r\n", DmrLTM);
	#endif

	DmrStack = MR_UpnpStack = UpnpCreateMicroStack(DmrChain,friendlyname, udn, "0000001", 20, 0);

	//ILibAddToChain(DmrChain, DmrLTM); /* seems to resolve dealyed event delivery */

    DmrExtendedM3uProcessor = MR_ExtendedM3uProcessor = CreatePlaylistParser(DmrChain, 3);
	DmrRenderer = MR_MediaRenderer = CreateMediaRenderer(DmrChain, DmrStack, DmrLTM);
	DmrRendererLogic = MR_RendererLogic = RSL_CreateRendererStateLogic
		(
		DmrChain,
		DmrRenderer,
		InstructPlaylistLogic_FindTargetUri,
		InstructCodec_SetupStream,
		InstructCodec_Play,
		InstructCodec_Stop,
		InstructCodec_Pause,
		QueryCodec_IsBusy,
		Validate_MediaUri
		);


	/* Create the RemoteIO client */
	CreateRemoteIO(DmrChain, DmrStack);

	/* initialize emulated rendering module */
	CodecWrapper_Init(MAX_STREAMS);

	/*
	 *	Set up the app to periodically monitor the available list
	 *	of IP addresses.
	 */
	#ifdef _POSIX
	UpnpMonitor = ILibCreateLifeTime(DmrChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	#endif
	#ifdef _WINSOCK1
	UpnpMonitor = ILibCreateLifeTime(DmrChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	#endif
	#ifdef _WINSOCK2
	UpnpMonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
	#endif

	/* start UPnP - blocking call*/
	#ifdef _POSIX
	signal(SIGINT,BreakSink);
	#endif

	#ifdef WIN32
	/* Setup a thread to allow user to stop renderer when user hits key */
	CreateThread(NULL,0,&Run,NULL,0,NULL);
	#endif

	ILibStartChain(DmrChain); 	

	/* be sure to free the address list */
	FREE(UpnpIPAddressList);
	
	/* clean up wsastartup */
	#ifdef WIN32
	WSACleanup();
	#endif

	return 0;
}
