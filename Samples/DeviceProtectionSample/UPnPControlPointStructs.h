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

#ifndef __UPNP_CONTROLPOINT_STRUCTS__
#define __UPNP_CONTROLPOINT_STRUCTS__

#define UPNP_ERROR_SCPD_NOT_WELL_FORMED 5

struct UPnPDevice;
typedef void(*UPnPDeviceHandler)(struct UPnPDevice *device);
typedef void(*UPnPDeviceDiscoveryErrorHandler)(char *UDN, char *LocationURL, int StatusCode);

typedef enum
{
	UPnPSSDP_MSEARCH = 1,
	UPnPSSDP_NOTIFY = 2
} UPnPSSDP_MESSAGE;

struct UPnPIcon
{
	int width;
	int height;
	int colorDepth;
	char *mimeType;
	char *url;
};

struct UPnPDevice
{
	void* CP;
	char* DeviceType;
	char* UDN;
	UPnPDeviceHandler fpDestroy;
	
	char* LocationURL;
	struct sockaddr_in6 LocationAddr;
	struct sockaddr_in6 InterfaceToHostAddr;
	struct UPnPIcon *Icons;
	int IconsLength;
	char* PresentationURL;
	char* FriendlyName;
	char* ManufacturerName;
	char* ManufacturerURL;
	char* ModelName;
	char* ModelDescription;
	char* ModelNumber;
	char* ModelURL;
	
	int MaxVersion;
	int SCPDError;
	int SCPDLeft;
	int ReferenceCount;
	int ReferenceTiedToEvents;
	char* InterfaceToHost;
	int CacheTime;
	void *Tag;

	int Reserved;
	long Reserved2;
	char *Reserved3;
	int ReservedID;
	void *CustomTagTable;
	
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
	int MaxVersion;
	
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
#endif
