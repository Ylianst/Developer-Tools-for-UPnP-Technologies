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
#include "UPnPMicroStack.h"

void UPnPSwitchPower_GetStatus(void* upnptoken)
{
	printf("UPnP Invoke: SwitchPower_GetStatus();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_SwitchPower_GetStatus(upnptoken,1);
}

void UPnPSwitchPower_SetTarget(void* upnptoken,int newTargetValue)
{
	printf("UPnP Invoke: SwitchPower_SetTarget(%d);\r\n",newTargetValue);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_SwitchPower_SetTarget(upnptoken);
}

void UPnPDimmingService_GetLoadLevelStatus(void* upnptoken)
{
	printf("UPnP Invoke: DimmingService_GetLoadLevelStatus();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_GetLoadLevelStatus(upnptoken,250);
}

void UPnPDimmingService_GetMinLevel(void* upnptoken)
{
	printf("UPnP Invoke: DimmingService_GetMinLevel();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_GetMinLevel(upnptoken,250);
}

void UPnPDimmingService_SetLoadLevelTarget(void* upnptoken,unsigned char NewLoadLevelTarget)
{
	printf("UPnP Invoke: DimmingService_SetLoadLevelTarget(%u);\r\n",NewLoadLevelTarget);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_DimmingService_SetLoadLevelTarget(upnptoken);
}

void UPnPPresentationRequest(void* upnptoken, char* directive)
{
	printf("Presentation Request: %s\r\n", directive);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UPnPGetLocalInterfaceToHost(upnptoken));
	
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	UPnPFP_PresentationPage=&UPnPPresentationRequest;
	UPnPFP_DimmingService_GetLoadLevelStatus=&UPnPDimmingService_GetLoadLevelStatus;
	UPnPFP_DimmingService_GetMinLevel=&UPnPDimmingService_GetMinLevel;
	UPnPFP_DimmingService_SetLoadLevelTarget=&UPnPDimmingService_SetLoadLevelTarget;
	UPnPFP_SwitchPower_GetStatus=&UPnPSwitchPower_GetStatus;
	UPnPFP_SwitchPower_SetTarget=&UPnPSwitchPower_SetTarget;
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_SwitchPower_Status(1);
	UPnPSetState_DimmingService_LoadLevelStatus(250);
	
	printf("Intel's UPnP MicroStack 1.0\r\nConnected & Extended PC Lab (CEL)\r\n\r\n");
	UPnPStart("Micro Light Bulb","438b856b-98ac-473c-9651-1f1645bd332f","0000001", 120,8080);
	
	return 0;
}

