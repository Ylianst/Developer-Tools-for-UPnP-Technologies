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

#ifndef __UpnpMicrostack__
#define __UpnpMicrostack__


/*
*
*	Target Platform = WINDOWS / PPC2002
*	WinSockVersion  = 1
*
*	HTTP Mode = 1.1
*	IPAddressMonitoring = YES
*
*/


struct UpnpDataObject;
struct packetheader;

/* These methods must be implemented by the user */
extern void UpnpMediaRenderer_RenderingControl_SetVolume(void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume);
extern void UpnpMediaRenderer_RenderingControl_GetMute(void* upnptoken,unsigned int InstanceID,char* Channel);
extern void UpnpMediaRenderer_RenderingControl_SetMute(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute);
extern void UpnpMediaRenderer_RenderingControl_GetVolume(void* upnptoken,unsigned int InstanceID,char* Channel);
extern void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID);
extern void UpnpMediaRenderer_ConnectionManager_GetProtocolInfo(void* upnptoken);
extern void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken);
extern void UpnpMedaRenderer_AVTransport_GetCurrentTransportActions(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_Play(void* upnptoken,unsigned int InstanceID,char* Speed);
extern void UpnpMedaRenderer_AVTransport_Previous(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_Next(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_Stop(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_GetTransportSettings(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_Seek(void* upnptoken,unsigned int InstanceID,char* Unit,char* Target);
extern void UpnpMedaRenderer_AVTransport_Pause(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_GetPositionInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_GetTransportInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_SetAVTransportURI(void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData);
extern void UpnpMedaRenderer_AVTransport_GetDeviceCapabilities(void* upnptoken,unsigned int InstanceID);
extern void UpnpMedaRenderer_AVTransport_SetPlayMode(void* upnptoken,unsigned int InstanceID,char* NewPlayMode);
extern void UpnpMedaRenderer_AVTransport_GetMediaInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet);

/* UPnP Stack Management */
void *UpnpCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void UpnpIPAddressListChanged(void *MicroStackToken);
int UpnpGetLocalPortNumber(void *token);
int   UpnpGetLocalInterfaceToHost(const void* UPnPToken);
void* UpnpGetWebServerToken(const void *MicroStackToken);

/* Invocation Response Methods */
void UpnpResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg);
void UpnpResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params);
void UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* ProtocolInfo, const char* PeerConnectionManager, const int PeerConnectionID, const char* Direction, const char* Status);
void UpnpResponse_MediaRenderer_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* Source, const char* Sink);
void UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* ConnectionIDs);
void UpnpResponse_MediaRenderer_RenderingControl_SetVolume(const void* UPnPToken);
void UpnpResponse_MediaRenderer_RenderingControl_GetMute(const void* UPnPToken, const int CurrentMute);
void UpnpResponse_MediaRenderer_RenderingControl_SetMute(const void* UPnPToken);
void UpnpResponse_MediaRenderer_RenderingControl_GetVolume(const void* UPnPToken, const unsigned short CurrentVolume);
void UpnpResponse_MedaRenderer_AVTransport_GetCurrentTransportActions(const void* UPnPToken, const char* Actions);
void UpnpResponse_MedaRenderer_AVTransport_Play(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_Previous(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_Next(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_Stop(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_GetTransportSettings(const void* UPnPToken, const char* PlayMode, const char* RecQualityMode);
void UpnpResponse_MedaRenderer_AVTransport_Seek(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_Pause(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_GetPositionInfo(const void* UPnPToken, const unsigned int Track, const char* TrackDuration, const char* TrackMetaData, const char* TrackURI, const char* RelTime, const char* AbsTime, const int RelCount, const int AbsCount);
void UpnpResponse_MedaRenderer_AVTransport_GetTransportInfo(const void* UPnPToken, const char* CurrentTransportState, const char* CurrentTransportStatus, const char* CurrentSpeed);
void UpnpResponse_MedaRenderer_AVTransport_SetAVTransportURI(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_GetDeviceCapabilities(const void* UPnPToken, const char* PlayMedia, const char* RecMedia, const char* RecQualityModes);
void UpnpResponse_MedaRenderer_AVTransport_SetPlayMode(const void* UPnPToken);
void UpnpResponse_MedaRenderer_AVTransport_GetMediaInfo(const void* UPnPToken, const unsigned int NrTracks, const char* MediaDuration, const char* CurrentURI, const char* CurrentURIMetaData, const char* NextURI, const char* NextURIMetaData, const char* PlayMedium, const char* RecordMedium, const char* WriteStatus);

/* State Variable Eventing Methods */
void UpnpSetState_MediaRenderer_ConnectionManager_SourceProtocolInfo(void *microstack,char* val);
void UpnpSetState_MediaRenderer_ConnectionManager_SinkProtocolInfo(void *microstack,char* val);
void UpnpSetState_MediaRenderer_ConnectionManager_CurrentConnectionIDs(void *microstack,char* val);
void UpnpSetState_MediaRenderer_RenderingControl_LastChange(void *microstack,char* val);
void UpnpSetState_MedaRenderer_AVTransport_LastChange(void *microstack,char* val);

#endif
