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
#ifndef __CDS_ControlPoint__
#define __CDS_ControlPoint__

#include "UPnP_ControlPoint_Structs.h"


void CDS_AddRef(struct UPnPDevice *device);
void CDS_Release(struct UPnPDevice *device);

struct UPnPDevice* CDS_GetDevice1(struct UPnPDevice *device,int index);
int CDS_GetDeviceCount(struct UPnPDevice *device);
struct UPnPDevice* CDS_GetDeviceAtUDN(void *v_CP,char* UDN);

void PrintUPnPDevice(int indents, struct UPnPDevice *device);

void *CDS_CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void CDS__CP_IPAddressListChanged(void *CPToken);
struct UPnPDevice* CDS_GetDevice(struct UPnPDevice *device, char* DeviceType, int number);
int CDS_HasAction(struct UPnPService *s, char* action);
void CDS_SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));
struct UPnPService *CDS_GetService(struct UPnPDevice *device, char* ServiceName, int length);
struct UPnPService *CDS_GetService_ContentDirectory(struct UPnPDevice *device);
struct UPnPService *CDS_GetService_ConnectionManager(struct UPnPDevice *device);

extern void (*CDS_EventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* ContainerUpdateIDs);
extern void (*CDS_EventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int SystemUpdateID);
extern void (*CDS_EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* SourceProtocolInfo);
extern void (*CDS_EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* SinkProtocolInfo);
extern void (*CDS_EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* CurrentConnectionIDs);

void CDS_Invoke_ContentDirectory_Browse(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void CDS_Invoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SortCaps),void* _user);
void CDS_Invoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int Id),void* _user);
void CDS_Invoke_ContentDirectory_Search(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ContainerID, char* SearchCriteria, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void CDS_Invoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SearchCaps),void* _user);
void CDS_Invoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status),void* _user, int ConnectionID);
void CDS_Invoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Source,char* Sink),void* _user);
void CDS_Invoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* ConnectionIDs),void* _user);

#endif
