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
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "AVRCP_ControlPoint.h"
#include "ILibParsers.h"


void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void AVRCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void AVRCP_ResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetVolume[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetMute[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentMute);
}

void AVRCP_ResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetMute[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetVolume[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentVolume);
}

void AVRCP_ResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetCurrentTransportActions[ErrorCode:%d](%s)\r\n",ErrorCode,Actions);
}

void AVRCP_ResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Play[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetDeviceCapabilities[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,PlayMedia,RecMedia,RecQualityModes);
}

void AVRCP_ResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetMediaInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%s,%s,%s)\r\n",ErrorCode,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
}

void AVRCP_ResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Previous[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Next[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportSettings[ErrorCode:%d](%s,%s)\r\n",ErrorCode,PlayMode,RecQualityMode);
}

void AVRCP_ResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetAVTransportURI[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Pause[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetPositionInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%d,%d)\r\n",ErrorCode,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
}

void AVRCP_ResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Seek[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportInfo[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
}

void AVRCP_ResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetPlayMode[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Stop[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void AVRCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("AVRCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void AVRCP_EventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/RenderingControl/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

void AVRCP_EventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/AVTransport/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

/* Called whenever a new device on the correct type is discovered */
void AVRCP_DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;
	
	printf("AVRCP_ Device Added: %s\r\n", device->FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	AVRCP_PrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			AVRCP_SubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The AVRCP_GetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: AVRCP_HasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	/*
	tempService = AVRCP_GetService_AVTransport(device);
	AVRCP_Invoke_AVTransport_GetCurrentTransportActions(tempService, &AVRCP_ResponseSink_AVTransport_GetCurrentTransportActions,NULL,250);
	AVRCP_Invoke_AVTransport_Play(tempService, &AVRCP_ResponseSink_AVTransport_Play,NULL,250,"Sample String");
	AVRCP_Invoke_AVTransport_GetDeviceCapabilities(tempService, &AVRCP_ResponseSink_AVTransport_GetDeviceCapabilities,NULL,250);
	AVRCP_Invoke_AVTransport_GetMediaInfo(tempService, &AVRCP_ResponseSink_AVTransport_GetMediaInfo,NULL,250);
	AVRCP_Invoke_AVTransport_Previous(tempService, &AVRCP_ResponseSink_AVTransport_Previous,NULL,250);
	AVRCP_Invoke_AVTransport_Next(tempService, &AVRCP_ResponseSink_AVTransport_Next,NULL,250);
	AVRCP_Invoke_AVTransport_GetTransportSettings(tempService, &AVRCP_ResponseSink_AVTransport_GetTransportSettings,NULL,250);
	AVRCP_Invoke_AVTransport_SetAVTransportURI(tempService, &AVRCP_ResponseSink_AVTransport_SetAVTransportURI,NULL,250,"Sample String","Sample String");
	AVRCP_Invoke_AVTransport_Pause(tempService, &AVRCP_ResponseSink_AVTransport_Pause,NULL,250);
	AVRCP_Invoke_AVTransport_GetPositionInfo(tempService, &AVRCP_ResponseSink_AVTransport_GetPositionInfo,NULL,250);
	AVRCP_Invoke_AVTransport_Seek(tempService, &AVRCP_ResponseSink_AVTransport_Seek,NULL,250,"Sample String","Sample String");
	AVRCP_Invoke_AVTransport_GetTransportInfo(tempService, &AVRCP_ResponseSink_AVTransport_GetTransportInfo,NULL,250);
	AVRCP_Invoke_AVTransport_SetPlayMode(tempService, &AVRCP_ResponseSink_AVTransport_SetPlayMode,NULL,250,"Sample String");
	AVRCP_Invoke_AVTransport_Stop(tempService, &AVRCP_ResponseSink_AVTransport_Stop,NULL,250);
	
	tempService = AVRCP_GetService_ConnectionManager(device);
	AVRCP_Invoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	AVRCP_Invoke_ConnectionManager_GetProtocolInfo(tempService, &AVRCP_ResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	AVRCP_Invoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	
	tempService = AVRCP_GetService_RenderingControl(device);
	AVRCP_Invoke_RenderingControl_SetVolume(tempService, &AVRCP_ResponseSink_RenderingControl_SetVolume,NULL,250,"Sample String",250);
	AVRCP_Invoke_RenderingControl_GetMute(tempService, &AVRCP_ResponseSink_RenderingControl_GetMute,NULL,250,"Sample String");
	AVRCP_Invoke_RenderingControl_SetMute(tempService, &AVRCP_ResponseSink_RenderingControl_SetMute,NULL,250,"Sample String",1);
	AVRCP_Invoke_RenderingControl_GetVolume(tempService, &AVRCP_ResponseSink_RenderingControl_GetVolume,NULL,250,"Sample String");
	*/
}

/* Called whenever a discovered device was removed from the network */
void AVRCP_DeviceRemoveSink(struct UPnPDevice *device)
{
	printf("AVRCP_ Device Removed: %s\r\n", device->FriendlyName);
}

void *AVRCP__CP;
void *AVRCP__CP_chain;
void *AVRCP__CP_Monitor;
int *AVRCP__CP_IPAddressList;
int AVRCP__CP_IPAddressListLength;

void AVRCP__CP_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=AVRCP__CP_IPAddressListLength || memcmp((void*)list,(void*)AVRCP__CP_IPAddressList,sizeof(int)*length)!=0)
	{
		AVRCP__CP_IPAddressListChanged(AVRCP__CP);
		
		FREE(AVRCP__CP_IPAddressList);
		AVRCP__CP_IPAddressList = list;
		AVRCP__CP_IPAddressListLength = length;
	}
	else
	{
		FREE(list);
	}
	
	ILibLifeTime_Add(AVRCP__CP_Monitor,NULL,4,&AVRCP__CP_IPAddressMonitor,NULL);
}
/* This thread is used to monitor the user for a return keyboard input */
/* Calling AVRCP_StopCP() will cause the Control Point thread to fall out after clean up */
DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(AVRCP__CP_chain);
	return 0;
}

/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	
	/* Event callback function registration code */
	AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo=&AVRCP_EventSink_ConnectionManager_SourceProtocolInfo;
	AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo=&AVRCP_EventSink_ConnectionManager_SinkProtocolInfo;
	AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs=&AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs;
	AVRCP_EventCallback_RenderingControl_LastChange=&AVRCP_EventSink_RenderingControl_LastChange;
	AVRCP_EventCallback_AVTransport_LastChange=&AVRCP_EventSink_AVTransport_LastChange;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	AVRCP__CP_chain = ILibCreateChain();
	AVRCP__CP = AVRCP_CreateControlPoint(AVRCP__CP_chain,&AVRCP_DeviceDiscoverSink,&AVRCP_DeviceRemoveSink);
	AVRCP__CP_Monitor = ILibCreateLifeTime(AVRCP__CP_chain);
	ILibLifeTime_Add(AVRCP__CP_Monitor,NULL,4,&AVRCP__CP_IPAddressMonitor,NULL);
	
	CreateThread(NULL,0,&Run,NULL,0,&ptid);
	ILibStartChain(AVRCP__CP_chain);
	free(AVRCP__CP_IPAddressList);
	return 0;
}

