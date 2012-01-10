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

#ifndef __AVRCP_ControlPoint__
#define __AVRCP_ControlPoint__

#include "UPnPControlPointStructs.h"

void AVRCP_AddRef(struct UPnPDevice *device);
void AVRCP_Release(struct UPnPDevice *device);

struct UPnPDevice* AVRCP_GetDevice1(struct UPnPDevice *device,int index);
int AVRCP_GetDeviceCount(struct UPnPDevice *device);
struct UPnPDevice* AVRCP_GetDeviceAtUDN(void *v_CP,char* UDN);

void PrintUPnPDevice(int indents, struct UPnPDevice *device);

void *AVRCP_CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void AVRCP__CP_IPAddressListChanged(void *CPToken);
struct UPnPDevice* AVRCP_GetDevice(struct UPnPDevice *device, char* DeviceType, int number);
int AVRCP_HasAction(struct UPnPService *s, char* action);
void AVRCP_SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));
struct UPnPService *AVRCP_GetService(struct UPnPDevice *device, char* ServiceName, int length);
struct UPnPService *AVRCP_GetService_ConnectionManager(struct UPnPDevice *device);
struct UPnPService *AVRCP_GetService_RenderingControl(struct UPnPDevice *device);
struct UPnPService *AVRCP_GetService_AVTransport(struct UPnPDevice *device);

extern void (*AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* SourceProtocolInfo);
extern void (*AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* SinkProtocolInfo);
extern void (*AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* CurrentConnectionIDs);
extern void (*AVRCP_EventCallback_RenderingControl_LastChange)(struct UPnPService* Service,char* LastChange);
extern void (*AVRCP_EventCallback_AVTransport_LastChange)(struct UPnPService* Service,char* LastChange);

void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status),void* _user, int ConnectionID);
void AVRCP_Invoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Source,char* Sink),void* _user);
void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* ConnectionIDs),void* _user);
void AVRCP_Invoke_RenderingControl_SetVolume(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* Channel, unsigned short DesiredVolume);
void AVRCP_Invoke_RenderingControl_GetMute(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,int CurrentMute),void* _user, unsigned int InstanceID, char* Channel);
void AVRCP_Invoke_RenderingControl_SetMute(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* Channel, int DesiredMute);
void AVRCP_Invoke_RenderingControl_GetVolume(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned short CurrentVolume),void* _user, unsigned int InstanceID, char* Channel);
void AVRCP_Invoke_AVTransport_GetCurrentTransportActions(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* Actions),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_Play(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* Speed);
void AVRCP_Invoke_AVTransport_GetDeviceCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* PlayMedia,char* RecMedia,char* RecQualityModes),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_GetMediaInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_Previous(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_Next(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_GetTransportSettings(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* PlayMode,char* RecQualityMode),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_SetAVTransportURI(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* CurrentURI, char* CurrentURIMetaData);
void AVRCP_Invoke_AVTransport_Pause(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_GetPositionInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_Seek(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* Unit, char* Target);
void AVRCP_Invoke_AVTransport_GetTransportInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed),void* _user, unsigned int InstanceID);
void AVRCP_Invoke_AVTransport_SetPlayMode(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID, char* NewPlayMode);
void AVRCP_Invoke_AVTransport_Stop(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user),void* _user, unsigned int InstanceID);

#endif
