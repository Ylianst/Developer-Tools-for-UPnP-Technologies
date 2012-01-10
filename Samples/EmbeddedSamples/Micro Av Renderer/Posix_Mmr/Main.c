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
#include <signal.h>
#include <string.h>

#include "UpnpMicroStack.h"
#include "ILibParsers.h"
#include "MicroMediaRenderer.h"

void* UpnpMicroStack;
void* UpnpMicroStackChain;

void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;

void* MR_MediaRendererState;


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

/*
 *	Method executes periodically on the UPnP microstack thread
 *	and updates the microstack with the platform's current list of
 *	active IP addresses.
 *
 *	This methodology can be optimized, but the exact nature of the
 *	optimization will be differ depending on the platform.
 */
void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(UpnpMicroStack);
		
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

/*
 *	This method executes when somebody does CTRL-BREAK.
 */
void BreakSink(int s)
{
	ILibStopChain(UpnpMicroStackChain);
}


int main(int argc, char* argv[])
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
	UpnpMicroStackChain = ILibCreateChain();
	UpnpMicroStack = UpnpCreateMicroStack(UpnpMicroStackChain,"MMR Posix-(Basic)","UDN:MMR PosixBasic","000001",1800, 0);
	MR_MediaRendererState = CreateMediaRenderer(UpnpMicroStackChain, UpnpMicroStack, ILibCreateLifeTime(UpnpMicroStackChain));

	/* Start the renderer thread chain */
	printf("Intel MicroStack 1.0 - Micro Media Renderer\r\n\r\n");

	/*
	 *	Set up the solution to periodically monitor 
	 *	the platform's current list of IP addresses.
	 *	This will allow the upnp layers to appropriately
	 *	react to IP address changes.
	 */
	UpnpMonitor = ILibCreateLifeTime(UpnpMicroStackChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);

	/* start UPnP - blocking call*/
	signal(SIGINT,BreakSink);
	ILibStartChain(UpnpMicroStackChain);

	/* be sure to free the address list */
	FREE(UpnpIPAddressList);

	return 0;
}