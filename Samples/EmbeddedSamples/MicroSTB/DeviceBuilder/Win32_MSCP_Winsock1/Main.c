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
#include "MSCP_ControlPoint.h"
#include "ILibParsers.h"


void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void MSCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void MSCP_ResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP_ Invoke Response: ContentDirectory/Browse[ErrorCode:%d](%s,%u,%u,%u)\r\n",ErrorCode,Result,NumberReturned,TotalMatches,UpdateID);
}

void MSCP_ResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSortCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SortCaps);
}

void MSCP_ResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSystemUpdateID[ErrorCode:%d](%u)\r\n",ErrorCode,Id);
}

void MSCP_ResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSearchCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SearchCaps);
}

void MSCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void MSCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void MSCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("MSCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void MSCP_EventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("MSCP_ Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
}

/* Called whenever a new device on the correct type is discovered */
void MSCP_DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;
	
	printf("MSCP_ Device Added: %s\r\n", device->FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	MSCP_PrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			MSCP_SubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The MSCP_GetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: MSCP_HasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	tempService = MSCP_GetService_ContentDirectory(device);
	MSCP_Invoke_ContentDirectory_Browse(tempService, &MSCP_ResponseSink_ContentDirectory_Browse,NULL,"Sample String","Sample String","Sample String",250,250,"Sample String");
	MSCP_Invoke_ContentDirectory_GetSortCapabilities(tempService, &MSCP_ResponseSink_ContentDirectory_GetSortCapabilities,NULL);
	MSCP_Invoke_ContentDirectory_GetSystemUpdateID(tempService, &MSCP_ResponseSink_ContentDirectory_GetSystemUpdateID,NULL);
	MSCP_Invoke_ContentDirectory_GetSearchCapabilities(tempService, &MSCP_ResponseSink_ContentDirectory_GetSearchCapabilities,NULL);
	
	tempService = MSCP_GetService_ConnectionManager(device);
	MSCP_Invoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	MSCP_Invoke_ConnectionManager_GetProtocolInfo(tempService, &MSCP_ResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	MSCP_Invoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	
}

/* Called whenever a discovered device was removed from the network */
void MSCP_DeviceRemoveSink(struct UPnPDevice *device)
{
	printf("MSCP_ Device Removed: %s\r\n", device->FriendlyName);
}

void *MSCP__CP;
void *MSCP__CP_chain;
void *MSCP__CP_Monitor;
int *MSCP__CP_IPAddressList;
int MSCP__CP_IPAddressListLength;

void MSCP__CP_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=MSCP__CP_IPAddressListLength || memcmp((void*)list,(void*)MSCP__CP_IPAddressList,sizeof(int)*length)!=0)
	{
		MSCP__CP_IPAddressListChanged(MSCP__CP);
		
		FREE(MSCP__CP_IPAddressList);
		MSCP__CP_IPAddressList = list;
		MSCP__CP_IPAddressListLength = length;
	}
	else
	{
		FREE(list);
	}
	
	ILibLifeTime_Add(MSCP__CP_Monitor,NULL,4,&MSCP__CP_IPAddressMonitor,NULL);
}
/* This thread is used to monitor the user for a return keyboard input */
/* Calling MSCP_StopCP() will cause the Control Point thread to fall out after clean up */
DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(MSCP__CP_chain);
	return 0;
}

/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	
	/* Event callback function registration code */
	MSCP_EventCallback_ConnectionManager_SourceProtocolInfo=&MSCP_EventSink_ConnectionManager_SourceProtocolInfo;
	MSCP_EventCallback_ConnectionManager_SinkProtocolInfo=&MSCP_EventSink_ConnectionManager_SinkProtocolInfo;
	MSCP_EventCallback_ConnectionManager_CurrentConnectionIDs=&MSCP_EventSink_ConnectionManager_CurrentConnectionIDs;
	MSCP_EventCallback_ContentDirectory_SystemUpdateID=&MSCP_EventSink_ContentDirectory_SystemUpdateID;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	MSCP__CP_chain = ILibCreateChain();
	MSCP__CP = MSCP_CreateControlPoint(MSCP__CP_chain,&MSCP_DeviceDiscoverSink,&MSCP_DeviceRemoveSink);
	MSCP__CP_Monitor = ILibCreateLifeTime(MSCP__CP_chain);
	ILibLifeTime_Add(MSCP__CP_Monitor,NULL,4,&MSCP__CP_IPAddressMonitor,NULL);
	
	CreateThread(NULL,0,&Run,NULL,0,&ptid);
	ILibStartChain(MSCP__CP_chain);
	free(MSCP__CP_IPAddressList);
	return 0;
}

