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

/* UPnP MicroStack, Main Module */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "UpnpMicroStack.h"
#include "ILibParsers.h"
#include "MicroMediaServer.h"
#include <stdlib.h>
#include <unistd.h>

void *TheChain;
void *TheStack;

void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;

/*
 *	Method gets periodically executed on the microstack
 *	thread to update the list of known IP addresses.
 *	This allows the upnp layer to adjust to changes
 *	in the IP address list for the platform.
 *	This applies only if winsock1 is used.
 */
void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(TheStack);
		
		free(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
		UpdateIPAddresses(UpnpIPAddressList, UpnpIPAddressLength);
	}
	else
	{
		free(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
}

void BreakSink(int s)
{
	ILibStopChain(TheChain);
}

int main(int argv, char** argc)
{
	char udn[20];
	char friendlyname[100];
	int i;

	/* Randomized udn generation */
	srand((unsigned int)time(NULL));
	for (i=0;i<19;i++)
	{
		udn[i] = (rand() % 25) + 66;
	}
	udn[19] = 0;

	/* get friendly name with hostname */
	memcpy(friendlyname,"Intel Micro AV Server (",23);
	gethostname(friendlyname+23,68);
	memcpy(friendlyname+strlen(friendlyname),")/Posix\0",8);

	/* command line arg processing */
	TheChain = ILibCreateChain();

	TheStack = UpnpCreateMicroStack(TheChain, friendlyname, udn,"0000001",1800,0);

	if (argv != 2)
	{
		printf("\r\n\r\nUSAGE: You can specify a path name when running this application.\r\n\r\n");
		InitMms(TheChain, TheStack, "./");
	}
	else
	{
		InitMms(TheChain, TheStack, argc[1]);
	}

	/*
	 *	Set up the app to periodically monitor the available list
	 *	of IP addresses.
	 */
	UpnpMonitor = ILibCreateLifeTime(TheChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	UpdateIPAddresses(UpnpIPAddressList, UpnpIPAddressLength);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);

	/* start UPnP - blocking call*/
	signal(SIGINT,BreakSink);
	ILibStartChain(TheChain);

	return 0;
}
