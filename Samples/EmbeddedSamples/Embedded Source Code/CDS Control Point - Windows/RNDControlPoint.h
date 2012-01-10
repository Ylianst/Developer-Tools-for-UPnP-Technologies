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

#ifndef __RNDControlPoint__
#define __RNDControlPoint__

struct UPnPDevice
{
	void* CP;
	char* DeviceType;
	char* UDN;
	
	char* LocationURL;
	char* PresentationURL;
	char* FriendlyName;
	char* ManufacturerName;
	char* ManufacturerURL;
	char* ModelName;
	char* ModelDescription;
	char* ModelNumber;
	char* ModelURL;
	
	int SCPDLeft;
	int ReferenceCount;
	char* InterfaceToHost;
	int CacheTime;
	void *Tag;
	
	struct UPnPDevice *Parent;
	struct UPnPDevice *EmbeddedDevices;
	struct UPnPService *Services;
	struct UPnPDevice *Next;
};

struct UPnPService
{
	char* ServiceType;
	char* ServiceId;
	char* ControlURL;
	char* SubscriptionURL;
	char* SCPDURL;
	char* SubscriptionID;
	
	struct UPnPAction *Actions;
	struct UPnPStateVariable *Variables;
	struct UPnPDevice *Parent;
	struct UPnPService *Next;
};

struct UPnPStateVariable
{
	struct UPnPStateVariable *Next;
	struct UPnPService *Parent;
	
	char* Name;
	char **AllowedValues;
	int NumAllowedValues;
	char* Min;
	char* Max;
	char* Step;
};

struct UPnPAction
{
	char* Name;
	struct UPnPAction *Next;
};

struct UPnPAllowedValue
{
	struct UPnPAllowedValue *Next;
	
	char* Value;
};


void RNDAddRef(struct UPnPDevice *device);
void RNDRelease(struct UPnPDevice *device);

struct UPnPDevice* RNDGetDevice1(struct UPnPDevice *device,int index);
int RNDGetDeviceCount(struct UPnPDevice *device);

void PrintUPnPDevice(int indents, struct UPnPDevice *device);

void *RNDCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void RND_CP_IPAddressListChanged(void *CPToken);
struct UPnPDevice* RNDGetDevice(struct UPnPDevice *device, char* DeviceType, int number);
int RNDHasAction(struct UPnPService *s, char* action);
void RNDSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));
struct UPnPService *RNDGetService(struct UPnPDevice *device, char* ServiceName, int length);
struct UPnPService *RNDGetService_ContentDirectory(struct UPnPDevice *device);
struct UPnPService *RNDGetService_ConnectionManager(struct UPnPDevice *device);

extern void (*RNDEventCallback_ContentDirectory_TransferIDs)(struct UPnPService* Service,char* TransferIDs);
extern void (*RNDEventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* ContainerUpdateIDs);
extern void (*RNDEventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int SystemUpdateID);
extern void (*RNDEventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* SourceProtocolInfo);
extern void (*RNDEventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* SinkProtocolInfo);
extern void (*RNDEventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* CurrentConnectionIDs);

void RNDInvoke_ContentDirectory_Search(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ContainerID, char* SearchCriteria, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void RNDInvoke_ContentDirectory_StopTransferResource(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int TransferID);
void RNDInvoke_ContentDirectory_DestroyObject(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, char* ObjectID);
void RNDInvoke_ContentDirectory_UpdateObject(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, char* ObjectID, char* CurrentTagValue, char* NewTagValue);
void RNDInvoke_ContentDirectory_ExportResource(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int TransferID),void* _user, char* SourceURI, char* DestinationURI);
void RNDInvoke_ContentDirectory_GetTransferProgress(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* TransferStatus,char* TransferLength,char* TransferTotal),void* _user, unsigned int TransferID);
void RNDInvoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SearchCaps),void* _user);
void RNDInvoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int Id),void* _user);
void RNDInvoke_ContentDirectory_CreateObject(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* ObjectID,char* Result),void* _user, char* ContainerID, char* Elements);
void RNDInvoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SortCaps),void* _user);
void RNDInvoke_ContentDirectory_Browse(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void RNDInvoke_ContentDirectory_ImportResource(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int TransferID),void* _user, char* SourceURI, char* DestinationURI);
void RNDInvoke_ContentDirectory_CreateReference(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* NewID),void* _user, char* ContainerID, char* ObjectID);
void RNDInvoke_ContentDirectory_DeleteResource(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, char* ResourceURI);
void RNDInvoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status),void* _user, int ConnectionID);
void RNDInvoke_ConnectionManager_PrepareForConnection(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,int ConnectionID,int AVTransportID,int RcsID),void* _user, char* RemoteProtocolInfo, char* PeerConnectionManager, int PeerConnectionID, char* Direction);
void RNDInvoke_ConnectionManager_ConnectionComplete(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, int ConnectionID);
void RNDInvoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Source,char* Sink),void* _user);
void RNDInvoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* ConnectionIDs),void* _user);

#endif
