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

#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#include <windows.h>
#include <stdio.h>
#include <memory.h>
#include "RNDControlPoint.h"
#include "ILibParsers.h"

void RNDResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("RND Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void RNDResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("RND Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void RNDResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("RND Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void RNDResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions)
{
	printf("RND Invoke Response: AVTransport/GetCurrentTransportActions[ErrorCode:%d](%s)\r\n",ErrorCode,Actions);
}

void RNDResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Play[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Previous[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Next[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Stop[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode)
{
	printf("RND Invoke Response: AVTransport/GetTransportSettings[ErrorCode:%d](%s,%s)\r\n",ErrorCode,PlayMode,RecQualityMode);
}

void RNDResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Seek[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/Pause[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount)
{
	printf("RND Invoke Response: AVTransport/GetPositionInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%d,%d)\r\n",ErrorCode,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
}

void RNDResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed)
{
	printf("RND Invoke Response: AVTransport/GetTransportInfo[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
}

void RNDResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/SetAVTransportURI[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes)
{
	printf("RND Invoke Response: AVTransport/GetDeviceCapabilities[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,PlayMedia,RecMedia,RecQualityModes);
}

void RNDResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: AVTransport/SetPlayMode[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus)
{
	printf("RND Invoke Response: AVTransport/GetMediaInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%s,%s,%s)\r\n",ErrorCode,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
}

void RNDResponseSink_RenderingControl_GetHorizontalKeystone(struct UPnPService* Service,int ErrorCode,void *User,short CurrentHorizontalKeystone)
{
	printf("RND Invoke Response: RenderingControl/GetHorizontalKeystone[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentHorizontalKeystone);
}

void RNDResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume)
{
	printf("RND Invoke Response: RenderingControl/GetVolume[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentVolume);
}

void RNDResponseSink_RenderingControl_SelectPreset(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SelectPreset[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetVolume[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_ListPresets(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentPresetNameList)
{
	printf("RND Invoke Response: RenderingControl/ListPresets[ErrorCode:%d](%s)\r\n",ErrorCode,CurrentPresetNameList);
}

void RNDResponseSink_RenderingControl_SetVolumeDB(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetVolumeDB[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetRedVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetRedVideoBlackLevel[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetContrast(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetContrast[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetLoudness(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetLoudness[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetBrightness(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetBrightness[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_GetLoudness(struct UPnPService* Service,int ErrorCode,void *User,int CurrentLoudness)
{
	printf("RND Invoke Response: RenderingControl/GetLoudness[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentLoudness);
}

void RNDResponseSink_RenderingControl_GetColorTemperature(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentColorTemperature)
{
	printf("RND Invoke Response: RenderingControl/GetColorTemperature[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentColorTemperature);
}

void RNDResponseSink_RenderingControl_GetSharpness(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentSharpness)
{
	printf("RND Invoke Response: RenderingControl/GetSharpness[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentSharpness);
}

void RNDResponseSink_RenderingControl_GetContrast(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentContrast)
{
	printf("RND Invoke Response: RenderingControl/GetContrast[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentContrast);
}

void RNDResponseSink_RenderingControl_GetGreenVideoGain(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentGreenVideoGain)
{
	printf("RND Invoke Response: RenderingControl/GetGreenVideoGain[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentGreenVideoGain);
}

void RNDResponseSink_RenderingControl_SetRedVideoGain(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetRedVideoGain[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetGreenVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetGreenVideoBlackLevel[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_GetVolumeDBRange(struct UPnPService* Service,int ErrorCode,void *User,short MinValue,short MaxValue)
{
	printf("RND Invoke Response: RenderingControl/GetVolumeDBRange[ErrorCode:%d](%d,%d)\r\n",ErrorCode,MinValue,MaxValue);
}

void RNDResponseSink_RenderingControl_GetRedVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentRedVideoBlackLevel)
{
	printf("RND Invoke Response: RenderingControl/GetRedVideoBlackLevel[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentRedVideoBlackLevel);
}

void RNDResponseSink_RenderingControl_GetBlueVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentBlueVideoBlackLevel)
{
	printf("RND Invoke Response: RenderingControl/GetBlueVideoBlackLevel[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentBlueVideoBlackLevel);
}

void RNDResponseSink_RenderingControl_GetBlueVideoGain(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentBlueVideoGain)
{
	printf("RND Invoke Response: RenderingControl/GetBlueVideoGain[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentBlueVideoGain);
}

void RNDResponseSink_RenderingControl_SetBlueVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetBlueVideoBlackLevel[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute)
{
	printf("RND Invoke Response: RenderingControl/GetMute[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentMute);
}

void RNDResponseSink_RenderingControl_SetBlueVideoGain(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetBlueVideoGain[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_GetVerticalKeystone(struct UPnPService* Service,int ErrorCode,void *User,short CurrentVerticalKeystone)
{
	printf("RND Invoke Response: RenderingControl/GetVerticalKeystone[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentVerticalKeystone);
}

void RNDResponseSink_RenderingControl_SetVerticalKeystone(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetVerticalKeystone[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_GetBrightness(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentBrightness)
{
	printf("RND Invoke Response: RenderingControl/GetBrightness[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentBrightness);
}

void RNDResponseSink_RenderingControl_GetVolumeDB(struct UPnPService* Service,int ErrorCode,void *User,short CurrentVolume)
{
	printf("RND Invoke Response: RenderingControl/GetVolumeDB[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentVolume);
}

void RNDResponseSink_RenderingControl_GetGreenVideoBlackLevel(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentGreenVideoBlackLevel)
{
	printf("RND Invoke Response: RenderingControl/GetGreenVideoBlackLevel[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentGreenVideoBlackLevel);
}

void RNDResponseSink_RenderingControl_GetRedVideoGain(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentRedVideoGain)
{
	printf("RND Invoke Response: RenderingControl/GetRedVideoGain[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentRedVideoGain);
}

void RNDResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetMute[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetGreenVideoGain(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetGreenVideoGain[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetSharpness(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetSharpness[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetHorizontalKeystone(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetHorizontalKeystone[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_RenderingControl_SetColorTemperature(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: RenderingControl/SetColorTemperature[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDEventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("RND Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void RNDEventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("RND Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void RNDEventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("RND Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void RNDEventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("RND Event from %s/AVTransport/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

void RNDEventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("RND Event from %s/RenderingControl/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

/* Called whenever a new device on the correct type is discovered */
void RNDDeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;
	
	printf("RND Device Added: %s\r\n", device->FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	RNDPrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			RNDSubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The RNDGetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: RNDHasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	tempService = RNDGetService_RenderingControl(device);
	RNDInvoke_RenderingControl_GetHorizontalKeystone(tempService, &RNDResponseSink_RenderingControl_GetHorizontalKeystone,NULL,250);
	RNDInvoke_RenderingControl_GetVolume(tempService, &RNDResponseSink_RenderingControl_GetVolume,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_SelectPreset(tempService, &RNDResponseSink_RenderingControl_SelectPreset,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_SetVolume(tempService, &RNDResponseSink_RenderingControl_SetVolume,NULL,250,"Sample String",250);
	RNDInvoke_RenderingControl_ListPresets(tempService, &RNDResponseSink_RenderingControl_ListPresets,NULL,250);
	RNDInvoke_RenderingControl_SetVolumeDB(tempService, &RNDResponseSink_RenderingControl_SetVolumeDB,NULL,250,"Sample String",25000);
	RNDInvoke_RenderingControl_SetRedVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_SetRedVideoBlackLevel,NULL,250,250);
	RNDInvoke_RenderingControl_SetContrast(tempService, &RNDResponseSink_RenderingControl_SetContrast,NULL,250,250);
	RNDInvoke_RenderingControl_SetLoudness(tempService, &RNDResponseSink_RenderingControl_SetLoudness,NULL,250,"Sample String",1);
	RNDInvoke_RenderingControl_SetBrightness(tempService, &RNDResponseSink_RenderingControl_SetBrightness,NULL,250,250);
	RNDInvoke_RenderingControl_GetLoudness(tempService, &RNDResponseSink_RenderingControl_GetLoudness,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_GetColorTemperature(tempService, &RNDResponseSink_RenderingControl_GetColorTemperature,NULL,250);
	RNDInvoke_RenderingControl_GetSharpness(tempService, &RNDResponseSink_RenderingControl_GetSharpness,NULL,250);
	RNDInvoke_RenderingControl_GetContrast(tempService, &RNDResponseSink_RenderingControl_GetContrast,NULL,250);
	RNDInvoke_RenderingControl_GetGreenVideoGain(tempService, &RNDResponseSink_RenderingControl_GetGreenVideoGain,NULL,250);
	RNDInvoke_RenderingControl_SetRedVideoGain(tempService, &RNDResponseSink_RenderingControl_SetRedVideoGain,NULL,250,250);
	RNDInvoke_RenderingControl_SetGreenVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_SetGreenVideoBlackLevel,NULL,250,250);
	RNDInvoke_RenderingControl_GetVolumeDBRange(tempService, &RNDResponseSink_RenderingControl_GetVolumeDBRange,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_GetRedVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_GetRedVideoBlackLevel,NULL,250);
	RNDInvoke_RenderingControl_GetBlueVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_GetBlueVideoBlackLevel,NULL,250);
	RNDInvoke_RenderingControl_GetBlueVideoGain(tempService, &RNDResponseSink_RenderingControl_GetBlueVideoGain,NULL,250);
	RNDInvoke_RenderingControl_SetBlueVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_SetBlueVideoBlackLevel,NULL,250,250);
	RNDInvoke_RenderingControl_GetMute(tempService, &RNDResponseSink_RenderingControl_GetMute,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_SetBlueVideoGain(tempService, &RNDResponseSink_RenderingControl_SetBlueVideoGain,NULL,250,250);
	RNDInvoke_RenderingControl_GetVerticalKeystone(tempService, &RNDResponseSink_RenderingControl_GetVerticalKeystone,NULL,250);
	RNDInvoke_RenderingControl_SetVerticalKeystone(tempService, &RNDResponseSink_RenderingControl_SetVerticalKeystone,NULL,250,25000);
	RNDInvoke_RenderingControl_GetBrightness(tempService, &RNDResponseSink_RenderingControl_GetBrightness,NULL,250);
	RNDInvoke_RenderingControl_GetVolumeDB(tempService, &RNDResponseSink_RenderingControl_GetVolumeDB,NULL,250,"Sample String");
	RNDInvoke_RenderingControl_GetGreenVideoBlackLevel(tempService, &RNDResponseSink_RenderingControl_GetGreenVideoBlackLevel,NULL,250);
	RNDInvoke_RenderingControl_GetRedVideoGain(tempService, &RNDResponseSink_RenderingControl_GetRedVideoGain,NULL,250);
	RNDInvoke_RenderingControl_SetMute(tempService, &RNDResponseSink_RenderingControl_SetMute,NULL,250,"Sample String",1);
	RNDInvoke_RenderingControl_SetGreenVideoGain(tempService, &RNDResponseSink_RenderingControl_SetGreenVideoGain,NULL,250,250);
	RNDInvoke_RenderingControl_SetSharpness(tempService, &RNDResponseSink_RenderingControl_SetSharpness,NULL,250,250);
	RNDInvoke_RenderingControl_SetHorizontalKeystone(tempService, &RNDResponseSink_RenderingControl_SetHorizontalKeystone,NULL,250,25000);
	RNDInvoke_RenderingControl_SetColorTemperature(tempService, &RNDResponseSink_RenderingControl_SetColorTemperature,NULL,250,250);
	
	tempService = RNDGetService_AVTransport(device);
	RNDInvoke_AVTransport_GetCurrentTransportActions(tempService, &RNDResponseSink_AVTransport_GetCurrentTransportActions,NULL,250);
	RNDInvoke_AVTransport_Play(tempService, &RNDResponseSink_AVTransport_Play,NULL,250,"Sample String");
	RNDInvoke_AVTransport_Previous(tempService, &RNDResponseSink_AVTransport_Previous,NULL,250);
	RNDInvoke_AVTransport_Next(tempService, &RNDResponseSink_AVTransport_Next,NULL,250);
	RNDInvoke_AVTransport_Stop(tempService, &RNDResponseSink_AVTransport_Stop,NULL,250);
	RNDInvoke_AVTransport_GetTransportSettings(tempService, &RNDResponseSink_AVTransport_GetTransportSettings,NULL,250);
	RNDInvoke_AVTransport_Seek(tempService, &RNDResponseSink_AVTransport_Seek,NULL,250,"Sample String","Sample String");
	RNDInvoke_AVTransport_Pause(tempService, &RNDResponseSink_AVTransport_Pause,NULL,250);
	RNDInvoke_AVTransport_GetPositionInfo(tempService, &RNDResponseSink_AVTransport_GetPositionInfo,NULL,250);
	RNDInvoke_AVTransport_GetTransportInfo(tempService, &RNDResponseSink_AVTransport_GetTransportInfo,NULL,250);
	RNDInvoke_AVTransport_SetAVTransportURI(tempService, &RNDResponseSink_AVTransport_SetAVTransportURI,NULL,250,"Sample String","Sample String");
	RNDInvoke_AVTransport_GetDeviceCapabilities(tempService, &RNDResponseSink_AVTransport_GetDeviceCapabilities,NULL,250);
	RNDInvoke_AVTransport_SetPlayMode(tempService, &RNDResponseSink_AVTransport_SetPlayMode,NULL,250,"Sample String");
	RNDInvoke_AVTransport_GetMediaInfo(tempService, &RNDResponseSink_AVTransport_GetMediaInfo,NULL,250);
	
	tempService = RNDGetService_ConnectionManager(device);
	RNDInvoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &RNDResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	RNDInvoke_ConnectionManager_GetProtocolInfo(tempService, &RNDResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	RNDInvoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &RNDResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	
}

/* Called whenever a discovered device was removed from the network */
void RNDDeviceRemoveSink(struct UPnPDevice *device)
{
	printf("RND Device Removed: %s\r\n", device->FriendlyName);
}

void *RND_CP;
void *RND_CP_chain;
/* This thread is used to monitor the user for a return keyboard input */
/* Calling RNDStopCP() will cause the Control Point thread to fall out after clean up */
DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(RND_CP_chain);
	return 0;
}

/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	/* Event callback function registration code */
	RNDEventCallback_ConnectionManager_SourceProtocolInfo=&RNDEventSink_ConnectionManager_SourceProtocolInfo;
	RNDEventCallback_ConnectionManager_SinkProtocolInfo=&RNDEventSink_ConnectionManager_SinkProtocolInfo;
	RNDEventCallback_ConnectionManager_CurrentConnectionIDs=&RNDEventSink_ConnectionManager_CurrentConnectionIDs;
	RNDEventCallback_AVTransport_LastChange=&RNDEventSink_AVTransport_LastChange;
	RNDEventCallback_RenderingControl_LastChange=&RNDEventSink_RenderingControl_LastChange;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	RND_CP_chain = ILibCreateChain();
	RND_CP = RNDCreateControlPoint(RND_CP_chain,&RNDDeviceDiscoverSink,&RNDDeviceRemoveSink);
	
	CreateThread(NULL,0,&Run,NULL,0,NULL);
	ILibStartChain(RND_CP_chain);
	return 0;
}

