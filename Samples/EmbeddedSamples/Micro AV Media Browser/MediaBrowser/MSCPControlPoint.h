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

#ifndef __MSCPControlPoint__
#define __MSCPControlPoint__

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


void MSCPAddRef(struct UPnPDevice *device);
void MSCPRelease(struct UPnPDevice *device);

struct UPnPDevice* MSCPGetDevice1(struct UPnPDevice *device,int index);
int MSCPGetDeviceCount(struct UPnPDevice *device);

void PrintUPnPDevice(int indents, struct UPnPDevice *device);

void *MSCPCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void MSCP_CP_IPAddressListChanged(void *CPToken);
struct UPnPDevice* MSCPGetDevice(struct UPnPDevice *device, char* DeviceType, int number);
int MSCPHasAction(struct UPnPService *s, char* action);
void MSCPSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));
struct UPnPService *MSCPGetService(struct UPnPDevice *device, char* ServiceName, int length);
struct UPnPService *MSCPGetService_ContentDirectory(struct UPnPDevice *device);

extern void (*MSCPEventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* ContainerUpdateIDs);
extern void (*MSCPEventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int SystemUpdateID);
extern void (*MSCPEventCallback_ContentDirectory_TransferIDs)(struct UPnPService* Service,char* TransferIDs);

void MSCPInvoke_ContentDirectory_Browse(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void MSCPInvoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SortCaps),void* _user);
void MSCPInvoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int Id),void* _user);
void MSCPInvoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* SearchCaps),void* _user);

#endif
