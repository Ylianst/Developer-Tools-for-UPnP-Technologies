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
#include "ILibParsers.h"
#include "UPnPMicroStack.h"

void *UPnPmicroStackChain;
void *UPnPmicroStack;
void UPnPSwitchPower_GetStatus(void* upnptoken)
{
	printf("Invoke: UPnPSwitchPower_GetStatus();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_SwitchPower_GetStatus(upnptoken,1);
}

void UPnPSwitchPower_SetTarget(void* upnptoken,int newTargetValue)
{
	printf("Invoke: UPnPSwitchPower_SetTarget(%d);\r\n",newTargetValue);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_SwitchPower_SetTarget(upnptoken);
}

void UPnPDimmingService_GetLoadLevelStatus(void* upnptoken)
{
	printf("Invoke: UPnPDimmingService_GetLoadLevelStatus();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_GetLoadLevelStatus(upnptoken,250);
}

void UPnPDimmingService_GetMinLevel(void* upnptoken)
{
	printf("Invoke: UPnPDimmingService_GetMinLevel();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_GetMinLevel(upnptoken,250);
}

void UPnPDimmingService_SetLoadLevelTarget(void* upnptoken,unsigned char NewLoadLevelTarget)
{
	printf("Invoke: UPnPDimmingService_SetLoadLevelTarget(%u);\r\n",NewLoadLevelTarget);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_SetLoadLevelTarget(upnptoken);
}

void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("UPnP Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UPnPGetLocalInterfaceToHost(upnptoken));
	
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	UPnPmicroStack = UPnPCreateMicroStack("Micro Light Bulb","f46f028c-e187-41d9-8e9e-6d68ab95da7e","0000001", 120,8080);
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_SwitchPower_Status(UPnPmicroStack,1);
	UPnPSetState_DimmingService_LoadLevelStatus(UPnPmicroStack,250);
	
	printf("Intel MicroStack 1.0 - Micro Light Bulb\r\n\r\n");
	UPnPmicroStackChain = ILibCreateChain();
	ILibAddToChain(UPnPmicroStackChain,UPnPmicroStack);
	ILibStartChain(UPnPmicroStackChain);
	
	return 0;
}

