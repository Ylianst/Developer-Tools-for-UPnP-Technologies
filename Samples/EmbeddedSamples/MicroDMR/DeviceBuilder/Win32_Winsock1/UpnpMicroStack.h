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
*	Target Platform = WINDOWS
*	WinSockVersion  = 1
*
*	HTTP Mode = 1.1
*	IPAddressMonitoring = YES
*
*/


struct UpnpDataObject;
struct packetheader;

/* These methods must be implemented by the user */
extern void UpnpRemoteIOClient_RemoteInput_InputKeyPress(void* upnptoken,int key);
extern void UpnpRemoteIOClient_RemoteInput_InputMouseMove(void* upnptoken,int X,int Y);
extern void UpnpRemoteIOClient_RemoteInput_GetInputSetup(void* upnptoken);
extern void UpnpRemoteIOClient_RemoteInput_InputMouseUp(void* upnptoken,int X,int Y,int Button);
extern void UpnpRemoteIOClient_RemoteInput_InputKeyUp(void* upnptoken,int key);
extern void UpnpRemoteIOClient_RemoteInput_InputKeyDown(void* upnptoken,int key);
extern void UpnpRemoteIOClient_RemoteInput_InputMouseDown(void* upnptoken,int X,int Y,int Button);
extern void UpnpRemoteIOClient_RemoteIO_ForceDisconnection(void* upnptoken);
extern void UpnpRemoteIOClient_RemoteIO_GetPeerConnection(void* upnptoken);
extern void UpnpRemoteIOClient_RemoteIO_ForceReset(void* upnptoken);
extern void UpnpRemoteIOClient_RemoteIO_SetPeerInterlock(void* upnptoken,char* PeerConnection);
extern void UpnpRemoteIOClient_RemoteIO_GetDeviceInformation(void* upnptoken);
extern void UpnpRemoteIOClient_RemoteIO_SetPeerOverride(void* upnptoken,char* PeerConnection);
extern void UpnpMediaRenderer_AVTransport_GetCurrentTransportActions(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_Play(void* upnptoken,unsigned int InstanceID,char* Speed);
extern void UpnpMediaRenderer_AVTransport_Previous(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_Next(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_Stop(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_GetTransportSettings(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_Seek(void* upnptoken,unsigned int InstanceID,char* Unit,char* Target);
extern void UpnpMediaRenderer_AVTransport_Pause(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_GetPositionInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_GetTransportInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_SetAVTransportURI(void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData);
extern void UpnpMediaRenderer_AVTransport_GetDeviceCapabilities(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_AVTransport_SetPlayMode(void* upnptoken,unsigned int InstanceID,char* NewPlayMode);
extern void UpnpMediaRenderer_AVTransport_GetMediaInfo(void* upnptoken,unsigned int InstanceID);
extern void UpnpMediaRenderer_RenderingControl_SetVolume(void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume);
extern void UpnpMediaRenderer_RenderingControl_GetMute(void* upnptoken,unsigned int InstanceID,char* Channel);
extern void UpnpMediaRenderer_RenderingControl_SetMute(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute);
extern void UpnpMediaRenderer_RenderingControl_GetVolume(void* upnptoken,unsigned int InstanceID,char* Channel);
extern void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID);
extern void UpnpMediaRenderer_ConnectionManager_GetProtocolInfo(void* upnptoken);
extern void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken);
extern void UpnpRemoteIOClient_ChannelManager_RegisterChannel(void* upnptoken,char* Name,char* PeerConnection,int Timeout);
extern void UpnpRemoteIOClient_ChannelManager_UnregisterChannel(void* upnptoken,char* PeerConnection);
extern void UpnpRemoteIOClient_ChannelManager_ClearAllChannels(void* upnptoken);
extern void UpnpRemoteIOClient_ChannelManager_GetRegisteredChannelList(void* upnptoken);
extern void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet);

/* UPnP Stack Management */
void *UpnpCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void *Upnp(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
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
void UpnpResponse_MediaRenderer_AVTransport_GetCurrentTransportActions(const void* UPnPToken, const char* Actions);
void UpnpResponse_MediaRenderer_AVTransport_Play(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_Previous(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_Next(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_Stop(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_GetTransportSettings(const void* UPnPToken, const char* PlayMode, const char* RecQualityMode);
void UpnpResponse_MediaRenderer_AVTransport_Seek(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_Pause(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_GetPositionInfo(const void* UPnPToken, const unsigned int Track, const char* TrackDuration, const char* TrackMetaData, const char* TrackURI, const char* RelTime, const char* AbsTime, const int RelCount, const int AbsCount);
void UpnpResponse_MediaRenderer_AVTransport_GetTransportInfo(const void* UPnPToken, const char* CurrentTransportState, const char* CurrentTransportStatus, const char* CurrentSpeed);
void UpnpResponse_MediaRenderer_AVTransport_SetAVTransportURI(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_GetDeviceCapabilities(const void* UPnPToken, const char* PlayMedia, const char* RecMedia, const char* RecQualityModes);
void UpnpResponse_MediaRenderer_AVTransport_SetPlayMode(const void* UPnPToken);
void UpnpResponse_MediaRenderer_AVTransport_GetMediaInfo(const void* UPnPToken, const unsigned int NrTracks, const char* MediaDuration, const char* CurrentURI, const char* CurrentURIMetaData, const char* NextURI, const char* NextURIMetaData, const char* PlayMedium, const char* RecordMedium, const char* WriteStatus);
void UpnpResponse_RemoteIOClient_RemoteIO_ForceDisconnection(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteIO_GetPeerConnection(const void* UPnPToken, const char* PeerConnection);
void UpnpResponse_RemoteIOClient_RemoteIO_ForceReset(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteIO_SetPeerInterlock(const void* UPnPToken, const char* ActivePeerConnection);
void UpnpResponse_RemoteIOClient_RemoteIO_GetDeviceInformation(const void* UPnPToken, const char* Application, const unsigned int MaxCommandSize, const int DisplayEncoding, const unsigned int DisplayWidth, const unsigned int DisplayHeight, const char* DeviceInformation);
void UpnpResponse_RemoteIOClient_RemoteIO_SetPeerOverride(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_InputKeyPress(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_InputMouseMove(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_GetInputSetup(const void* UPnPToken, const char* InputSetupIdentifier);
void UpnpResponse_RemoteIOClient_RemoteInput_InputMouseUp(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_InputKeyUp(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_InputKeyDown(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_RemoteInput_InputMouseDown(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_ChannelManager_RegisterChannel(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_ChannelManager_UnregisterChannel(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_ChannelManager_ClearAllChannels(const void* UPnPToken);
void UpnpResponse_RemoteIOClient_ChannelManager_GetRegisteredChannelList(const void* UPnPToken, const char* ChannelList);

/* State Variable Eventing Methods */
void UpnpSetState_MediaRenderer_ConnectionManager_SourceProtocolInfo(void *microstack,char* val);
void UpnpSetState_MediaRenderer_ConnectionManager_SinkProtocolInfo(void *microstack,char* val);
void UpnpSetState_MediaRenderer_ConnectionManager_CurrentConnectionIDs(void *microstack,char* val);
void UpnpSetState_MediaRenderer_RenderingControl_LastChange(void *microstack,char* val);
void UpnpSetState_MediaRenderer_AVTransport_LastChange(void *microstack,char* val);
void UpnpSetState_RemoteIOClient_RemoteIO_PeerConnection(void *microstack,char* val);
void UpnpSetState_RemoteIOClient_ChannelManager_RegisteredChannelList(void *microstack,char* val);

#endif
