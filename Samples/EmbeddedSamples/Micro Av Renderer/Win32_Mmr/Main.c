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

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ILibParsers.h"
#include "MicroMediaRenderer.h"
#include "UpnpMicroStack.h"

void* MR_RendererChain;
void* UpnpStack;
void* MR_MediaRenderer;

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

void MROnVolumeChangeRequestSink(enum MR_Enum_AudioChannels Channel,unsigned short Value)
{
	/* TODO: Handle volume change */
}

void MROnMuteChangeRequestSink(enum MR_Enum_AudioChannels Channel,int Value)
{
	/* TODO: Handle mute change */
}

void MROnMediaChangeRequestSink(const char* MediaUri)
{
	/* TODO: Handle media URI change */
}

void MROnGetPositionRequestSink(int* seconds, int* absSeconds, int* count, int* absCount)
{
	/* TODO: Handle position state request */
}

void MROnSeekRequestSink(enum MR_Enum_SeekModes seekMode, int seekPosition)
{
	/* TODO: Handle seek command*/
}

void MROnNextPreviousRequestSink(int trackDelta)
{
	/* TODO: Handle volume next/previous command*/
}

void MROnStateChangeRequestSink(enum MR_Enum_States state)
{
	/* TODO: Handle play/stop/pause/etc request */
}

void MROnPlayModeChangeRequestSink(enum MR_Enum_PlayModes playmode)
{
	/* TODO: Handle play mode change */
}

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
		UpnpIPAddressListChanged(UpnpStack);
		
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

DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(MR_RendererChain);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*
	 *	Set all of the renderer callbacks.
	 *	If the UPnP device has multiple renderers, it will need
	 *	to map function pointer callbacks for each renderer device.
	 */
	MROnVolumeChangeRequest			= &MROnVolumeChangeRequestSink;
	MROnMuteChangeRequest			= &MROnMuteChangeRequestSink;
	MROnMediaChangeRequest			= &MROnMediaChangeRequestSink;
	MROnGetPositionRequest			= &MROnGetPositionRequestSink;
	MROnSeekRequest					= &MROnSeekRequestSink;
	MROnNextPreviousRequest			= &MROnNextPreviousRequestSink;
	MROnStateChangeRequest			= &MROnStateChangeRequestSink;
	MROnPlayModeChangeRequest		= &MROnPlayModeChangeRequestSink;

	/* TODO: Each device must have a unique device identifier (UDN) - The UDN should be generated dynamically*/
	MR_RendererChain = ILibCreateChain();
	UpnpStack = UpnpCreateMicroStack(MR_RendererChain,"MMR Win32-Basic","UDN:MMR Win32-Basic","000001",1800,0);
	MR_MediaRenderer = CreateMediaRenderer(MR_RendererChain, UpnpStack, ILibCreateLifeTime(MR_RendererChain));

	/*
	 *	Set up the app to periodically monitor the available list
	 *	of IP addresses.
	 */
	#ifdef _WINSOCK1
	UpnpMonitor = ILibCreateLifeTime(MR_RendererChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	#endif
	#ifdef _WINSOCK2
	UpnpMonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
	#endif

	/* Setup a thread to allow user to stop renderer when user hits key */
	CreateThread(NULL,0,&Run,NULL,0,NULL);

	/* Start the renderer thread chain */
	printf("Intel MicroStack 1.0 - Micro Media Renderer\r\n\r\n");
	ILibStartChain(MR_RendererChain);
	
	/* be sure to free the address list */
	FREE(UpnpIPAddressList);

	return 0;
}