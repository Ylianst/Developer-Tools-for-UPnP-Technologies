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
#include <windows.h>
#include <stdio.h>
#include <memory.h>
#include "MSCPControlPoint.h"
#include "ILibParsers.h"

void MSCPResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("MSCP Invoke Response: ConnectionManager/GetCurrentConnectionInfo(%d,%d,%s,%s,%d,%s,%s)\r\n",RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void MSCPResponseSink_ConnectionManager_ConnectionComplete(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ConnectionManager/ConnectionComplete()\r\n");
}

void MSCPResponseSink_ConnectionManager_PrepareForConnection(struct UPnPService* Service,int ErrorCode,void *User,int ConnectionID,int AVTransportID,int RcsID)
{
	printf("MSCP Invoke Response: ConnectionManager/PrepareForConnection(%d,%d,%d)\r\n",ConnectionID,AVTransportID,RcsID);
}

void MSCPResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("MSCP Invoke Response: ConnectionManager/GetProtocolInfo(%s,%s)\r\n",Source,Sink);
}

void MSCPResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("MSCP Invoke Response: ConnectionManager/GetCurrentConnectionIDs(%s)\r\n",ConnectionIDs);
}

void MSCPResponseSink_ContentDirectory_Search(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP Invoke Response: ContentDirectory/Search(%s,%u,%u,%u)\r\n",Result,NumberReturned,TotalMatches,UpdateID);
}

void MSCPResponseSink_ContentDirectory_StopTransferResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/StopTransferResource()\r\n");
}

void MSCPResponseSink_ContentDirectory_DestroyObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/DestroyObject()\r\n");
}

void MSCPResponseSink_ContentDirectory_DeleteResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/DeleteResource()\r\n");
}

void MSCPResponseSink_ContentDirectory_UpdateObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/UpdateObject()\r\n");
}

void MSCPResponseSink_ContentDirectory_ExportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("MSCP Invoke Response: ContentDirectory/ExportResource(%u)\r\n",TransferID);
}

void MSCPResponseSink_ContentDirectory_GetTransferProgress(struct UPnPService* Service,int ErrorCode,void *User,char* TransferStatus,char* TransferLength,char* TransferTotal)
{
	printf("MSCP Invoke Response: ContentDirectory/GetTransferProgress(%s,%s,%s)\r\n",TransferStatus,TransferLength,TransferTotal);
}

void MSCPResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSearchCapabilities(%s)\r\n",SearchCaps);
}

void MSCPResponseSink_ContentDirectory_CreateObject(struct UPnPService* Service,int ErrorCode,void *User,char* ObjectID,char* Result)
{
	printf("MSCP Invoke Response: ContentDirectory/CreateObject(%s,%s)\r\n",ObjectID,Result);
}

void MSCPResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSortCapabilities(%s)\r\n",SortCaps);
}

void MSCPResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP Invoke Response: ContentDirectory/Browse(%s,%u,%u,%u)\r\n",Result,NumberReturned,TotalMatches,UpdateID);
}

void MSCPResponseSink_ContentDirectory_ImportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("MSCP Invoke Response: ContentDirectory/ImportResource(%u)\r\n",TransferID);
}

void MSCPResponseSink_ContentDirectory_CreateReference(struct UPnPService* Service,int ErrorCode,void *User,char* NewID)
{
	printf("MSCP Invoke Response: ContentDirectory/CreateReference(%s)\r\n",NewID);
}

void MSCPResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSystemUpdateID(%u)\r\n",Id);
}

void MSCPEventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("MSCP Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void MSCPEventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("MSCP Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void MSCPEventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("MSCP Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void MSCPEventSink_ContentDirectory_TransferIDs(struct UPnPService* Service,char* TransferIDs)
{
	printf("MSCP Event from %s/ContentDirectory/TransferIDs: %s\r\n",Service->Parent->FriendlyName,TransferIDs);
}

void MSCPEventSink_ContentDirectory_ContainerUpdateIDs(struct UPnPService* Service,char* ContainerUpdateIDs)
{
	printf("MSCP Event from %s/ContentDirectory/ContainerUpdateIDs: %s\r\n",Service->Parent->FriendlyName,ContainerUpdateIDs);
}

void MSCPEventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("MSCP Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
}

/* Called whenever a new device on the correct type is discovered */
void MSCPDeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;
	
	printf("MSCP Device Added: %s\r\n", device->FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	MSCPPrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			MSCPSubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The MSCPGetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: MSCPHasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	tempService = MSCPGetService_ConnectionManager(device);
	MSCPInvoke_ConnectionManager_GetCurrentConnectionInfo(tempService, &MSCPResponseSink_ConnectionManager_GetCurrentConnectionInfo,NULL,25000);
	MSCPInvoke_ConnectionManager_ConnectionComplete(tempService, &MSCPResponseSink_ConnectionManager_ConnectionComplete,NULL,25000);
	MSCPInvoke_ConnectionManager_PrepareForConnection(tempService, &MSCPResponseSink_ConnectionManager_PrepareForConnection,NULL,"Sample String","Sample String",25000,"Sample String");
	MSCPInvoke_ConnectionManager_GetProtocolInfo(tempService, &MSCPResponseSink_ConnectionManager_GetProtocolInfo,NULL);
	MSCPInvoke_ConnectionManager_GetCurrentConnectionIDs(tempService, &MSCPResponseSink_ConnectionManager_GetCurrentConnectionIDs,NULL);
	
	tempService = MSCPGetService_ContentDirectory(device);
	MSCPInvoke_ContentDirectory_Search(tempService, &MSCPResponseSink_ContentDirectory_Search,NULL,"Sample String","Sample String","Sample String",250,250,"Sample String");
	MSCPInvoke_ContentDirectory_StopTransferResource(tempService, &MSCPResponseSink_ContentDirectory_StopTransferResource,NULL,250);
	MSCPInvoke_ContentDirectory_DestroyObject(tempService, &MSCPResponseSink_ContentDirectory_DestroyObject,NULL,"Sample String");
	MSCPInvoke_ContentDirectory_DeleteResource(tempService, &MSCPResponseSink_ContentDirectory_DeleteResource,NULL,"http://www.intel.com");
	MSCPInvoke_ContentDirectory_UpdateObject(tempService, &MSCPResponseSink_ContentDirectory_UpdateObject,NULL,"Sample String","Sample String","Sample String");
	MSCPInvoke_ContentDirectory_ExportResource(tempService, &MSCPResponseSink_ContentDirectory_ExportResource,NULL,"http://www.intel.com","http://www.intel.com");
	MSCPInvoke_ContentDirectory_GetTransferProgress(tempService, &MSCPResponseSink_ContentDirectory_GetTransferProgress,NULL,250);
	MSCPInvoke_ContentDirectory_GetSearchCapabilities(tempService, &MSCPResponseSink_ContentDirectory_GetSearchCapabilities,NULL);
	MSCPInvoke_ContentDirectory_CreateObject(tempService, &MSCPResponseSink_ContentDirectory_CreateObject,NULL,"Sample String","Sample String");
	MSCPInvoke_ContentDirectory_GetSortCapabilities(tempService, &MSCPResponseSink_ContentDirectory_GetSortCapabilities,NULL);
	MSCPInvoke_ContentDirectory_Browse(tempService, &MSCPResponseSink_ContentDirectory_Browse,NULL,"Sample String","Sample String","Sample String",250,250,"Sample String");
	MSCPInvoke_ContentDirectory_ImportResource(tempService, &MSCPResponseSink_ContentDirectory_ImportResource,NULL,"http://www.intel.com","http://www.intel.com");
	MSCPInvoke_ContentDirectory_CreateReference(tempService, &MSCPResponseSink_ContentDirectory_CreateReference,NULL,"Sample String","Sample String");
	MSCPInvoke_ContentDirectory_GetSystemUpdateID(tempService, &MSCPResponseSink_ContentDirectory_GetSystemUpdateID,NULL);
	
}

/* Called whenever a discovered device was removed from the network */
void MSCPDeviceRemoveSink(struct UPnPDevice *device)
{
	printf("MSCP Device Removed: %s\r\n", device->FriendlyName);
}

void *MSCP_CP;
void *MSCP_CP_chain;
/* This thread is used to monitor the user for a return keyboard input */
/* Calling MSCPStopCP() will cause the Control Point thread to fall out after clean up */
DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(MSCP_CP_chain);
	return 0;
}

/* Main entry point to the sample application */
int _tmain(int argc, _TCHAR* argv[])
{
	/* Event callback function registration code */
	MSCPEventCallback_ConnectionManager_SourceProtocolInfo=&MSCPEventSink_ConnectionManager_SourceProtocolInfo;
	MSCPEventCallback_ConnectionManager_SinkProtocolInfo=&MSCPEventSink_ConnectionManager_SinkProtocolInfo;
	MSCPEventCallback_ConnectionManager_CurrentConnectionIDs=&MSCPEventSink_ConnectionManager_CurrentConnectionIDs;
	MSCPEventCallback_ContentDirectory_TransferIDs=&MSCPEventSink_ContentDirectory_TransferIDs;
	MSCPEventCallback_ContentDirectory_ContainerUpdateIDs=&MSCPEventSink_ContentDirectory_ContainerUpdateIDs;
	MSCPEventCallback_ContentDirectory_SystemUpdateID=&MSCPEventSink_ContentDirectory_SystemUpdateID;
	
	printf("Intel Control Point Microstack 1.0\r\n");
	
	
	MSCP_CP_chain = ILibCreateChain();
	MSCP_CP = MSCPCreateControlPoint(MSCP_CP_chain,&MSCPDeviceDiscoverSink,&MSCPDeviceRemoveSink);
	
	CreateThread(NULL,0,&Run,NULL,0,NULL);
	ILibStartChain(MSCP_CP_chain);
	return 0;
}

