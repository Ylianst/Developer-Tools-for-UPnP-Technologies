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

void RNDResponseSink_ContentDirectory_Search(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("RND Invoke Response: ContentDirectory/Search[ErrorCode:%d](%s,%u,%u,%u)\r\n",ErrorCode,Result,NumberReturned,TotalMatches,UpdateID);
}

void RNDResponseSink_ContentDirectory_StopTransferResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: ContentDirectory/StopTransferResource[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_ContentDirectory_DestroyObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: ContentDirectory/DestroyObject[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_ContentDirectory_UpdateObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: ContentDirectory/UpdateObject[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_ContentDirectory_ExportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("RND Invoke Response: ContentDirectory/ExportResource[ErrorCode:%d](%u)\r\n",ErrorCode,TransferID);
}

void RNDResponseSink_ContentDirectory_GetTransferProgress(struct UPnPService* Service,int ErrorCode,void *User,char* TransferStatus,char* TransferLength,char* TransferTotal)
{
	printf("RND Invoke Response: ContentDirectory/GetTransferProgress[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,TransferStatus,TransferLength,TransferTotal);
}

void RNDResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("RND Invoke Response: ContentDirectory/GetSearchCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SearchCaps);
}

void RNDResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("RND Invoke Response: ContentDirectory/GetSystemUpdateID[ErrorCode:%d](%u)\r\n",ErrorCode,Id);
}

void RNDResponseSink_ContentDirectory_CreateObject(struct UPnPService* Service,int ErrorCode,void *User,char* ObjectID,char* Result)
{
	printf("RND Invoke Response: ContentDirectory/CreateObject[ErrorCode:%d](%s,%s)\r\n",ErrorCode,ObjectID,Result);
}

void RNDResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("RND Invoke Response: ContentDirectory/GetSortCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SortCaps);
}

void RNDResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("RND Invoke Response: ContentDirectory/Browse[ErrorCode:%d](%s,%u,%u,%u)\r\n",ErrorCode,Result,NumberReturned,TotalMatches,UpdateID);
}

void RNDResponseSink_ContentDirectory_ImportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("RND Invoke Response: ContentDirectory/ImportResource[ErrorCode:%d](%u)\r\n",ErrorCode,TransferID);
}

void RNDResponseSink_ContentDirectory_CreateReference(struct UPnPService* Service,int ErrorCode,void *User,char* NewID)
{
	printf("RND Invoke Response: ContentDirectory/CreateReference[ErrorCode:%d](%s)\r\n",ErrorCode,NewID);
}

void RNDResponseSink_ContentDirectory_DeleteResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: ContentDirectory/DeleteResource[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("RND Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void RNDResponseSink_ConnectionManager_PrepareForConnection(struct UPnPService* Service,int ErrorCode,void *User,int ConnectionID,int AVTransportID,int RcsID)
{
	printf("RND Invoke Response: ConnectionManager/PrepareForConnection[ErrorCode:%d](%d,%d,%d)\r\n",ErrorCode,ConnectionID,AVTransportID,RcsID);
}

void RNDResponseSink_ConnectionManager_ConnectionComplete(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("RND Invoke Response: ConnectionManager/ConnectionComplete[ErrorCode:%d]()\r\n",ErrorCode);
}

void RNDResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("RND Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void RNDResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("RND Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void RNDEventSink_ContentDirectory_TransferIDs(struct UPnPService* Service,char* TransferIDs)
{
	printf("RND Event from %s/ContentDirectory/TransferIDs: %s\r\n",Service->Parent->FriendlyName,TransferIDs);
}

void RNDEventSink_ContentDirectory_ContainerUpdateIDs(struct UPnPService* Service,char* ContainerUpdateIDs)
{
	printf("RND Event from %s/ContentDirectory/ContainerUpdateIDs: %s\r\n",Service->Parent->FriendlyName,ContainerUpdateIDs);
}

void RNDEventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("RND Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
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
	
	tempService = RNDGetService_ConnectionManager(device);
	RNDInvoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &RNDResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	RNDInvoke_ConnectionManager_PrepareForConnection(tempService, &RNDResponseSink_ConnectionManager_PrepareForConnection,NULL,"Sample String","Sample String",25000,"Sample String");
	RNDInvoke_ConnectionManager_ConnectionComplete(tempService, &RNDResponseSink_ConnectionManager_ConnectionComplete,NULL,25000);
	RNDInvoke_ConnectionManager_GetProtocolInfo(tempService, &RNDResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	RNDInvoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &RNDResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	
	tempService = RNDGetService_ContentDirectory(device);
	RNDInvoke_ContentDirectory_Search(tempService, &RNDResponseSink_ContentDirectory_Search,NULL,"Sample String","Sample String","Sample String",250,250,"Sample String");
	RNDInvoke_ContentDirectory_StopTransferResource(tempService, &RNDResponseSink_ContentDirectory_StopTransferResource,NULL,250);
	RNDInvoke_ContentDirectory_DestroyObject(tempService, &RNDResponseSink_ContentDirectory_DestroyObject,NULL,"Sample String");
	RNDInvoke_ContentDirectory_UpdateObject(tempService, &RNDResponseSink_ContentDirectory_UpdateObject,NULL,"Sample String","Sample String","Sample String");
	RNDInvoke_ContentDirectory_ExportResource(tempService, &RNDResponseSink_ContentDirectory_ExportResource,NULL,"http://www.intel.com","http://www.intel.com");
	RNDInvoke_ContentDirectory_GetTransferProgress(tempService, &RNDResponseSink_ContentDirectory_GetTransferProgress,NULL,250);
	RNDInvoke_ContentDirectory_GetSearchCapabilities(tempService, &RNDResponseSink_ContentDirectory_GetSearchCapabilities,NULL);
	RNDInvoke_ContentDirectory_GetSystemUpdateID(tempService, &RNDResponseSink_ContentDirectory_GetSystemUpdateID,NULL);
	RNDInvoke_ContentDirectory_CreateObject(tempService, &RNDResponseSink_ContentDirectory_CreateObject,NULL,"Sample String","Sample String");
	RNDInvoke_ContentDirectory_GetSortCapabilities(tempService, &RNDResponseSink_ContentDirectory_GetSortCapabilities,NULL);
	RNDInvoke_ContentDirectory_Browse(tempService, &RNDResponseSink_ContentDirectory_Browse,NULL,"Sample String","Sample String","Sample String",250,250,"Sample String");
	RNDInvoke_ContentDirectory_ImportResource(tempService, &RNDResponseSink_ContentDirectory_ImportResource,NULL,"http://www.intel.com","http://www.intel.com");
	RNDInvoke_ContentDirectory_CreateReference(tempService, &RNDResponseSink_ContentDirectory_CreateReference,NULL,"Sample String","Sample String");
	RNDInvoke_ContentDirectory_DeleteResource(tempService, &RNDResponseSink_ContentDirectory_DeleteResource,NULL,"http://www.intel.com");
	
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
	RNDEventCallback_ContentDirectory_TransferIDs=&RNDEventSink_ContentDirectory_TransferIDs;
	RNDEventCallback_ContentDirectory_ContainerUpdateIDs=&RNDEventSink_ContentDirectory_ContainerUpdateIDs;
	RNDEventCallback_ContentDirectory_SystemUpdateID=&RNDEventSink_ContentDirectory_SystemUpdateID;
	RNDEventCallback_ConnectionManager_SourceProtocolInfo=&RNDEventSink_ConnectionManager_SourceProtocolInfo;
	RNDEventCallback_ConnectionManager_SinkProtocolInfo=&RNDEventSink_ConnectionManager_SinkProtocolInfo;
	RNDEventCallback_ConnectionManager_CurrentConnectionIDs=&RNDEventSink_ConnectionManager_CurrentConnectionIDs;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	RND_CP_chain = ILibCreateChain();
	RND_CP = RNDCreateControlPoint(RND_CP_chain,&RNDDeviceDiscoverSink,&RNDDeviceRemoveSink);
	
	CreateThread(NULL,0,&Run,NULL,0,NULL);
	ILibStartChain(RND_CP_chain);
	return 0;
}

