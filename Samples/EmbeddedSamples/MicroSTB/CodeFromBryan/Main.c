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
#include "AVR_ControlPoint.h"
#include "CDS_ControlPoint.h"
#include "ILibParsers.h"
#include <winsock2.h>

void *AVR_Table;
void *CDS_Table;

void AVR_ResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: RenderingControl/SetVolume[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute)
{
	printf("AVR_ Invoke Response: RenderingControl/GetMute[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentMute);
}

void AVR_ResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: RenderingControl/SetMute[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume)
{
	printf("AVR_ Invoke Response: RenderingControl/GetVolume[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentVolume);
}

void AVR_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("AVR_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void AVR_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("AVR_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void AVR_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("AVR_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void AVR_ResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions)
{
	printf("AVR_ Invoke Response: AVTransport/GetCurrentTransportActions[ErrorCode:%d](%s)\r\n",ErrorCode,Actions);
}

void AVR_ResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Play[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Previous[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Next[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Stop[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode)
{
	printf("AVR_ Invoke Response: AVTransport/GetTransportSettings[ErrorCode:%d](%s,%s)\r\n",ErrorCode,PlayMode,RecQualityMode);
}

void AVR_ResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Seek[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/Pause[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount)
{
	printf("AVR_ Invoke Response: AVTransport/GetPositionInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%d,%d)\r\n",ErrorCode,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
}

void AVR_ResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed)
{
	printf("AVR_ Invoke Response: AVTransport/GetTransportInfo[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
}

void AVR_ResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/SetAVTransportURI[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes)
{
	printf("AVR_ Invoke Response: AVTransport/GetDeviceCapabilities[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,PlayMedia,RecMedia,RecQualityModes);
}

void AVR_ResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVR_ Invoke Response: AVTransport/SetPlayMode[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVR_ResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus)
{
	printf("AVR_ Invoke Response: AVTransport/GetMediaInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%s,%s,%s)\r\n",ErrorCode,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
}

void AVR_EventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVR_ Event from %s/RenderingControl/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

void AVR_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("AVR_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void AVR_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("AVR_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void AVR_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("AVR_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void AVR_EventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVR_ Event from %s/AVTransport/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}
void CDS_DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
//	struct UPnPService *tempService;
	
	printf("CDS_ Device Added: %s\r\n", device->FriendlyName);
	ILibHashTree_Lock(CDS_Table);
	ILibAddEntry(CDS_Table,device->UDN,strlen(device->UDN),device);
	ILibHashTree_UnLock(CDS_Table);
}
/* Called whenever a new device on the correct type is discovered */
void AVR_DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
//	struct UPnPService *tempService;
	
	printf("AVR_ Device Added: %s\r\n", device->FriendlyName);
	ILibHashTree_Lock(AVR_Table);
	ILibAddEntry(AVR_Table,device->UDN,strlen(device->UDN),device);
	ILibHashTree_UnLock(AVR_Table);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	AVR_PrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	//while(tempDevice!=NULL)
	//{
	//	tempService = tempDevice->Services;
	//	while(tempService!=NULL)
	//	{
	//		AVR_SubscribeForUPnPEvents(tempService,NULL);
	//		tempService = tempService->Next;
	//	}
	//	tempDevice = tempDevice->Next;
	//}
	//	
	//tempService = AVR_GetService_ConnectionManager(device);
	//AVR_Invoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &AVR_ResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	//AVR_Invoke_ConnectionManager_GetProtocolInfo(tempService, &AVR_ResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	//AVR_Invoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &AVR_ResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	//
	//tempService = AVR_GetService_RenderingControl(device);
	//AVR_Invoke_RenderingControl_SetVolume(tempService, &AVR_ResponseSink_RenderingControl_SetVolume,NULL,250,"Sample String",250);
	//AVR_Invoke_RenderingControl_GetMute(tempService, &AVR_ResponseSink_RenderingControl_GetMute,NULL,250,"Sample String");
	//AVR_Invoke_RenderingControl_SetMute(tempService, &AVR_ResponseSink_RenderingControl_SetMute,NULL,250,"Sample String",1);
	//AVR_Invoke_RenderingControl_GetVolume(tempService, &AVR_ResponseSink_RenderingControl_GetVolume,NULL,250,"Sample String");
	//
	//tempService = AVR_GetService_AVTransport(device);
	//AVR_Invoke_AVTransport_GetCurrentTransportActions(tempService, &AVR_ResponseSink_AVTransport_GetCurrentTransportActions,NULL,250);
	//AVR_Invoke_AVTransport_Play(tempService, &AVR_ResponseSink_AVTransport_Play,NULL,250,"Sample String");
	//AVR_Invoke_AVTransport_Previous(tempService, &AVR_ResponseSink_AVTransport_Previous,NULL,250);
	//AVR_Invoke_AVTransport_Next(tempService, &AVR_ResponseSink_AVTransport_Next,NULL,250);
	//AVR_Invoke_AVTransport_Stop(tempService, &AVR_ResponseSink_AVTransport_Stop,NULL,250);
	//AVR_Invoke_AVTransport_GetTransportSettings(tempService, &AVR_ResponseSink_AVTransport_GetTransportSettings,NULL,250);
	//AVR_Invoke_AVTransport_Seek(tempService, &AVR_ResponseSink_AVTransport_Seek,NULL,250,"Sample String","Sample String");
	//AVR_Invoke_AVTransport_Pause(tempService, &AVR_ResponseSink_AVTransport_Pause,NULL,250);
	//AVR_Invoke_AVTransport_GetPositionInfo(tempService, &AVR_ResponseSink_AVTransport_GetPositionInfo,NULL,250);
	//AVR_Invoke_AVTransport_GetTransportInfo(tempService, &AVR_ResponseSink_AVTransport_GetTransportInfo,NULL,250);
	//AVR_Invoke_AVTransport_SetAVTransportURI(tempService, &AVR_ResponseSink_AVTransport_SetAVTransportURI,NULL,250,"Sample String","Sample String");
	//AVR_Invoke_AVTransport_GetDeviceCapabilities(tempService, &AVR_ResponseSink_AVTransport_GetDeviceCapabilities,NULL,250);
	//AVR_Invoke_AVTransport_SetPlayMode(tempService, &AVR_ResponseSink_AVTransport_SetPlayMode,NULL,250,"Sample String");
	//AVR_Invoke_AVTransport_GetMediaInfo(tempService, &AVR_ResponseSink_AVTransport_GetMediaInfo,NULL,250);
	
}

/* Called whenever a discovered device was removed from the network */
void AVR_DeviceRemoveSink(struct UPnPDevice *device)
{
	printf("AVR_ Device Removed: %s\r\n", device->FriendlyName);
	ILibHashTree_Lock(AVR_Table);
	ILibDeleteEntry(AVR_Table,device->UDN,strlen(device->UDN));
	ILibHashTree_UnLock(AVR_Table);
}

void CDS_DeviceRemoveSink(struct UPnPDevice *device)
{
	printf("CDS_ Device Removed: %s\r\n", device->FriendlyName);
	ILibHashTree_Lock(CDS_Table);
	ILibDeleteEntry(CDS_Table,device->UDN,strlen(device->UDN));
	ILibHashTree_UnLock(CDS_Table);
}

void *AVR__CP;
void *CDS__CP;
void *AVR__CP_chain;
DWORD AVR_MonitorSocketReserved;
WSAOVERLAPPED AVR_MonitorSocketStateObject;
SOCKET AVR_MonitorSocket;

void CALLBACK AVR_IPAddressMonitor(
IN DWORD dwError,
IN DWORD cbTransferred,
IN LPWSAOVERLAPPED lpOverlapped,
IN DWORD dwFlags
)
{
	AVR__CP_IPAddressListChanged(AVR__CP);
	WSAIoctl(AVR_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&AVR_MonitorSocketReserved,&AVR_MonitorSocketStateObject,&AVR_IPAddressMonitor);
}
/* This thread is used to monitor the user for a return keyboard input */
/* Calling AVR_StopCP() will cause the Control Point thread to fall out after clean up */
DWORD WINAPI Run(LPVOID args)
{
	char buffer[10];
	char *currChar;
	buffer[0] = '\0';

	while(stricmp(buffer, "quit") && stricmp(buffer, "quit\r\n") && stricmp(buffer, "quit\n"))
	{
		fgets(buffer, sizeof(buffer), stdin);
		currChar = buffer;
		while(*currChar != '\n' && *currChar != '\r' && *currChar != '\0') currChar++;
		*currChar = '\0';

		printf("you typed: %s\r\n", buffer);
	}
	ILibStopChain(AVR__CP_chain);
	return 0;
}

/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	
	/* Event callback function registration code */
	AVR_EventCallback_RenderingControl_LastChange=&AVR_EventSink_RenderingControl_LastChange;
	AVR_EventCallback_ConnectionManager_SourceProtocolInfo=&AVR_EventSink_ConnectionManager_SourceProtocolInfo;
	AVR_EventCallback_ConnectionManager_SinkProtocolInfo=&AVR_EventSink_ConnectionManager_SinkProtocolInfo;
	AVR_EventCallback_ConnectionManager_CurrentConnectionIDs=&AVR_EventSink_ConnectionManager_CurrentConnectionIDs;
	AVR_EventCallback_AVTransport_LastChange=&AVR_EventSink_AVTransport_LastChange;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	AVR__CP_chain = ILibCreateChain();
	AVR__CP = AVR_CreateControlPoint(AVR__CP_chain,&AVR_DeviceDiscoverSink,&AVR_DeviceRemoveSink);
	CDS__CP = CDS_CreateControlPoint(AVR__CP_chain,&CDS_DeviceDiscoverSink,&CDS_DeviceRemoveSink);
	AVR_MonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(AVR_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&AVR_MonitorSocketReserved,&AVR_MonitorSocketStateObject,&AVR_IPAddressMonitor);
	

	AVR_Table = ILibInitHashTree();
	CDS_Table = ILibInitHashTree();

	CreateThread(NULL,0,&Run,NULL,0,&ptid);
	ILibStartChain(AVR__CP_chain);
	closesocket(AVR_MonitorSocket);

	ILibDestroyHashTree(AVR_Table);
	ILibDestroyHashTree(CDS_Table);

	return 0;
}

