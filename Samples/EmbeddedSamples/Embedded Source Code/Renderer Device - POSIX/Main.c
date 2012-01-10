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

#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "ILibParsers.h"
#include "UPnPMicroStack.h"

void *UPnPmicroStackChain;
void *UPnPmicroStack;

void *UPnPMonitor;
int UPnPIPAddressLength;
int *UPnPIPAddressList;

void UPnPRenderingControl_GetHorizontalKeystone(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetHorizontalKeystone(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetHorizontalKeystone(upnptoken,25000);
}

void UPnPRenderingControl_GetVolume(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UPnPRenderingControl_GetVolume(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetVolume(upnptoken,250);
}

void UPnPRenderingControl_SelectPreset(void* upnptoken,unsigned int InstanceID,char* PresetName)
{
	printf("Invoke: UPnPRenderingControl_SelectPreset(%u,%s);\r\n",InstanceID,PresetName);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SelectPreset(upnptoken);
}

void UPnPRenderingControl_SetVolume(void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume)
{
	printf("Invoke: UPnPRenderingControl_SetVolume(%u,%s,%u);\r\n",InstanceID,Channel,DesiredVolume);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetVolume(upnptoken);
}

void UPnPRenderingControl_ListPresets(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_ListPresets(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_ListPresets(upnptoken,"Sample String");
}

void UPnPRenderingControl_SetVolumeDB(void* upnptoken,unsigned int InstanceID,char* Channel,short DesiredVolume)
{
	printf("Invoke: UPnPRenderingControl_SetVolumeDB(%u,%s,%d);\r\n",InstanceID,Channel,DesiredVolume);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetVolumeDB(upnptoken);
}

void UPnPRenderingControl_SetRedVideoBlackLevel(void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoBlackLevel)
{
	printf("Invoke: UPnPRenderingControl_SetRedVideoBlackLevel(%u,%u);\r\n",InstanceID,DesiredRedVideoBlackLevel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetRedVideoBlackLevel(upnptoken);
}

void UPnPRenderingControl_SetContrast(void* upnptoken,unsigned int InstanceID,unsigned short DesiredContrast)
{
	printf("Invoke: UPnPRenderingControl_SetContrast(%u,%u);\r\n",InstanceID,DesiredContrast);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetContrast(upnptoken);
}

void UPnPRenderingControl_SetLoudness(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredLoudness)
{
	printf("Invoke: UPnPRenderingControl_SetLoudness(%u,%s,%d);\r\n",InstanceID,Channel,DesiredLoudness);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetLoudness(upnptoken);
}

void UPnPRenderingControl_SetBrightness(void* upnptoken,unsigned int InstanceID,unsigned short DesiredBrightness)
{
	printf("Invoke: UPnPRenderingControl_SetBrightness(%u,%u);\r\n",InstanceID,DesiredBrightness);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetBrightness(upnptoken);
}

void UPnPRenderingControl_GetLoudness(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UPnPRenderingControl_GetLoudness(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetLoudness(upnptoken,1);
}

void UPnPRenderingControl_GetColorTemperature(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetColorTemperature(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetColorTemperature(upnptoken,250);
}

void UPnPRenderingControl_GetSharpness(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetSharpness(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetSharpness(upnptoken,250);
}

void UPnPRenderingControl_GetContrast(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetContrast(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetContrast(upnptoken,250);
}

void UPnPRenderingControl_GetGreenVideoGain(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetGreenVideoGain(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetGreenVideoGain(upnptoken,250);
}

void UPnPRenderingControl_SetRedVideoGain(void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoGain)
{
	printf("Invoke: UPnPRenderingControl_SetRedVideoGain(%u,%u);\r\n",InstanceID,DesiredRedVideoGain);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetRedVideoGain(upnptoken);
}

void UPnPRenderingControl_SetGreenVideoBlackLevel(void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoBlackLevel)
{
	printf("Invoke: UPnPRenderingControl_SetGreenVideoBlackLevel(%u,%u);\r\n",InstanceID,DesiredGreenVideoBlackLevel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetGreenVideoBlackLevel(upnptoken);
}

void UPnPRenderingControl_GetVolumeDBRange(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UPnPRenderingControl_GetVolumeDBRange(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetVolumeDBRange(upnptoken,25000,25000);
}

void UPnPRenderingControl_GetRedVideoBlackLevel(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetRedVideoBlackLevel(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetRedVideoBlackLevel(upnptoken,250);
}

void UPnPRenderingControl_GetBlueVideoBlackLevel(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetBlueVideoBlackLevel(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetBlueVideoBlackLevel(upnptoken,250);
}

void UPnPRenderingControl_GetBlueVideoGain(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetBlueVideoGain(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetBlueVideoGain(upnptoken,250);
}

void UPnPRenderingControl_SetBlueVideoBlackLevel(void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoBlackLevel)
{
	printf("Invoke: UPnPRenderingControl_SetBlueVideoBlackLevel(%u,%u);\r\n",InstanceID,DesiredBlueVideoBlackLevel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetBlueVideoBlackLevel(upnptoken);
}

void UPnPRenderingControl_GetMute(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UPnPRenderingControl_GetMute(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetMute(upnptoken,1);
}

void UPnPRenderingControl_SetBlueVideoGain(void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoGain)
{
	printf("Invoke: UPnPRenderingControl_SetBlueVideoGain(%u,%u);\r\n",InstanceID,DesiredBlueVideoGain);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetBlueVideoGain(upnptoken);
}

void UPnPRenderingControl_GetVerticalKeystone(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetVerticalKeystone(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetVerticalKeystone(upnptoken,25000);
}

void UPnPRenderingControl_SetVerticalKeystone(void* upnptoken,unsigned int InstanceID,short DesiredVerticalKeystone)
{
	printf("Invoke: UPnPRenderingControl_SetVerticalKeystone(%u,%d);\r\n",InstanceID,DesiredVerticalKeystone);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetVerticalKeystone(upnptoken);
}

void UPnPRenderingControl_GetBrightness(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetBrightness(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetBrightness(upnptoken,250);
}

void UPnPRenderingControl_GetVolumeDB(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UPnPRenderingControl_GetVolumeDB(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetVolumeDB(upnptoken,25000);
}

void UPnPRenderingControl_GetGreenVideoBlackLevel(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetGreenVideoBlackLevel(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetGreenVideoBlackLevel(upnptoken,250);
}

void UPnPRenderingControl_GetRedVideoGain(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPRenderingControl_GetRedVideoGain(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_GetRedVideoGain(upnptoken,250);
}

void UPnPRenderingControl_SetMute(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute)
{
	printf("Invoke: UPnPRenderingControl_SetMute(%u,%s,%d);\r\n",InstanceID,Channel,DesiredMute);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetMute(upnptoken);
}

void UPnPRenderingControl_SetGreenVideoGain(void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoGain)
{
	printf("Invoke: UPnPRenderingControl_SetGreenVideoGain(%u,%u);\r\n",InstanceID,DesiredGreenVideoGain);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetGreenVideoGain(upnptoken);
}

void UPnPRenderingControl_SetSharpness(void* upnptoken,unsigned int InstanceID,unsigned short DesiredSharpness)
{
	printf("Invoke: UPnPRenderingControl_SetSharpness(%u,%u);\r\n",InstanceID,DesiredSharpness);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetSharpness(upnptoken);
}

void UPnPRenderingControl_SetHorizontalKeystone(void* upnptoken,unsigned int InstanceID,short DesiredHorizontalKeystone)
{
	printf("Invoke: UPnPRenderingControl_SetHorizontalKeystone(%u,%d);\r\n",InstanceID,DesiredHorizontalKeystone);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetHorizontalKeystone(upnptoken);
}

void UPnPRenderingControl_SetColorTemperature(void* upnptoken,unsigned int InstanceID,unsigned short DesiredColorTemperature)
{
	printf("Invoke: UPnPRenderingControl_SetColorTemperature(%u,%u);\r\n",InstanceID,DesiredColorTemperature);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_RenderingControl_SetColorTemperature(upnptoken);
}

void UPnPAVTransport_GetCurrentTransportActions(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetCurrentTransportActions(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetCurrentTransportActions(upnptoken,"Sample String");
}

void UPnPAVTransport_Play(void* upnptoken,unsigned int InstanceID,char* Speed)
{
	printf("Invoke: UPnPAVTransport_Play(%u,%s);\r\n",InstanceID,Speed);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Play(upnptoken);
}

void UPnPAVTransport_Previous(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_Previous(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Previous(upnptoken);
}

void UPnPAVTransport_Next(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_Next(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Next(upnptoken);
}

void UPnPAVTransport_Stop(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_Stop(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Stop(upnptoken);
}

void UPnPAVTransport_GetTransportSettings(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetTransportSettings(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetTransportSettings(upnptoken,"Sample String","Sample String");
}

void UPnPAVTransport_Seek(void* upnptoken,unsigned int InstanceID,char* Unit,char* Target)
{
	printf("Invoke: UPnPAVTransport_Seek(%u,%s,%s);\r\n",InstanceID,Unit,Target);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Seek(upnptoken);
}

void UPnPAVTransport_Pause(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_Pause(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_Pause(upnptoken);
}

void UPnPAVTransport_GetPositionInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetPositionInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetPositionInfo(upnptoken,250,"Sample String","Sample String","Sample String","Sample String","Sample String",25000,25000);
}

void UPnPAVTransport_GetTransportInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetTransportInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetTransportInfo(upnptoken,"Sample String","Sample String","Sample String");
}

void UPnPAVTransport_SetAVTransportURI(void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData)
{
	printf("Invoke: UPnPAVTransport_SetAVTransportURI(%u,%s,%s);\r\n",InstanceID,CurrentURI,CurrentURIMetaData);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_SetAVTransportURI(upnptoken);
}

void UPnPAVTransport_GetDeviceCapabilities(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetDeviceCapabilities(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetDeviceCapabilities(upnptoken,"Sample String","Sample String","Sample String");
}

void UPnPAVTransport_SetPlayMode(void* upnptoken,unsigned int InstanceID,char* NewPlayMode)
{
	printf("Invoke: UPnPAVTransport_SetPlayMode(%u,%s);\r\n",InstanceID,NewPlayMode);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_SetPlayMode(upnptoken);
}

void UPnPAVTransport_GetMediaInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UPnPAVTransport_GetMediaInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_AVTransport_GetMediaInfo(upnptoken,250,"Sample String","Sample String","Sample String","Sample String","Sample String","Sample String","Sample String","Sample String");
}

void UPnPConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	printf("Invoke: UPnPConnectionManager_GetCurrentConnectionInfo(%d);\r\n",ConnectionID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetCurrentConnectionInfo(upnptoken,25000,25000,"Sample String","Sample String",25000,"Sample String","Sample String");
}

void UPnPConnectionManager_GetProtocolInfo(void* upnptoken)
{
	printf("Invoke: UPnPConnectionManager_GetProtocolInfo();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetProtocolInfo(upnptoken,"Sample String","Sample String");
}

void UPnPConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	printf("Invoke: UPnPConnectionManager_GetCurrentConnectionIDs();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"Sample String");
}

void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("UPnP Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UPnPGetLocalInterfaceToHost(upnptoken));
	
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}

void UPnPIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UPnPIPAddressLength || memcmp((void*)list,(void*)UPnPIPAddressList,sizeof(int)*length)!=0)
	{
		UPnPIPAddressListChanged(UPnPmicroStack);
		
		FREE(UPnPIPAddressList);
		UPnPIPAddressList = list;
		UPnPIPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UPnPMonitor,NULL,4,&UPnPIPAddressMonitor,NULL);
}
void BreakSink(int s)
{
	ILibStopChain(UPnPmicroStackChain);
}
int main(void)
{
	UPnPmicroStackChain = ILibCreateChain();
	
	/* TODO: Each device must have a unique device identifier (UDN) */
	UPnPmicroStack = UPnPCreateMicroStack(UPnPmicroStackChain,"Intel AV Renderer","fada97a9-5917-4c8f-a2a9-147b44e84953","0000001",1800,0);
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_RenderingControl_LastChange(UPnPmicroStack,"Sample String");
	UPnPSetState_AVTransport_LastChange(UPnPmicroStack,"Sample String");
	UPnPSetState_ConnectionManager_SourceProtocolInfo(UPnPmicroStack,"Sample String");
	UPnPSetState_ConnectionManager_SinkProtocolInfo(UPnPmicroStack,"Sample String");
	UPnPSetState_ConnectionManager_CurrentConnectionIDs(UPnPmicroStack,"Sample String");
	
	printf("Intel MicroStack 1.0 - Intel AV Renderer\r\n\r\n");
	
	UPnPMonitor = ILibCreateLifeTime(UPnPmicroStackChain);
	UPnPIPAddressLength = ILibGetLocalIPAddressList(&UPnPIPAddressList);
	ILibLifeTime_Add(UPnPMonitor,NULL,4,&UPnPIPAddressMonitor,NULL);
	
	signal(SIGINT,BreakSink);
	ILibStartChain(UPnPmicroStackChain);
	
	return 0;
}

