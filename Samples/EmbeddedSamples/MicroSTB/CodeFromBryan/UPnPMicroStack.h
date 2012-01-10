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

#ifndef __UPnPMicrostack__
#define __UPnPMicrostack__


/*
*
*	Target Platform = WINDOWS
*	WinSockVersion  = 2
*
*	HTTP Mode = 1.0
*	IPAddressMonitoring = YES
*
*/


struct UPnPDataObject;
struct packetheader;

/* UPnP Stack Management */
void *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void *UPnP(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void UPnPIPAddressListChanged(void *MicroStackToken);
int UPnPGetLocalPortNumber(void *token);
int   UPnPGetLocalInterfaceToHost(const void* UPnPToken);
void* UPnPGetInstance(const void* UPnPToken);

/* UPnP Set Function Pointers Methods */
extern void (*UPnPFP_PresentationPage) (void* upnptoken,struct packetheader *packet);
extern void (*UPnPFP_ConnectionManager_GetCurrentConnectionInfo) (void* upnptoken,int ConnectionID);
extern void (*UPnPFP_ConnectionManager_PrepareForConnection) (void* upnptoken,char* RemoteProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction);
extern void (*UPnPFP_ConnectionManager_ConnectionComplete) (void* upnptoken,int ConnectionID);
extern void (*UPnPFP_ConnectionManager_GetProtocolInfo) (void* upnptoken);
extern void (*UPnPFP_ConnectionManager_GetCurrentConnectionIDs) (void* upnptoken);
extern void (*UPnPFP_AVTransport_GetCurrentTransportActions) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_Play) (void* upnptoken,unsigned int InstanceID,char* Speed);
extern void (*UPnPFP_AVTransport_Previous) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_Next) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_Stop) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_GetTransportSettings) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_Seek) (void* upnptoken,unsigned int InstanceID,char* Unit,char* Target);
extern void (*UPnPFP_AVTransport_Pause) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_GetPositionInfo) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_GetTransportInfo) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_SetAVTransportURI) (void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData);
extern void (*UPnPFP_AVTransport_GetDeviceCapabilities) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_AVTransport_SetPlayMode) (void* upnptoken,unsigned int InstanceID,char* NewPlayMode);
extern void (*UPnPFP_AVTransport_GetMediaInfo) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetHorizontalKeystone) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetVolume) (void* upnptoken,unsigned int InstanceID,char* Channel);
extern void (*UPnPFP_RenderingControl_SelectPreset) (void* upnptoken,unsigned int InstanceID,char* PresetName);
extern void (*UPnPFP_RenderingControl_SetVolume) (void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume);
extern void (*UPnPFP_RenderingControl_ListPresets) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_SetVolumeDB) (void* upnptoken,unsigned int InstanceID,char* Channel,short DesiredVolume);
extern void (*UPnPFP_RenderingControl_SetRedVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoBlackLevel);
extern void (*UPnPFP_RenderingControl_SetContrast) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredContrast);
extern void (*UPnPFP_RenderingControl_SetLoudness) (void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredLoudness);
extern void (*UPnPFP_RenderingControl_SetBrightness) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBrightness);
extern void (*UPnPFP_RenderingControl_GetLoudness) (void* upnptoken,unsigned int InstanceID,char* Channel);
extern void (*UPnPFP_RenderingControl_GetColorTemperature) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetSharpness) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetContrast) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetGreenVideoGain) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_SetRedVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoGain);
extern void (*UPnPFP_RenderingControl_SetGreenVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoBlackLevel);
extern void (*UPnPFP_RenderingControl_GetVolumeDBRange) (void* upnptoken,unsigned int InstanceID,char* Channel);
extern void (*UPnPFP_RenderingControl_GetRedVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetBlueVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetBlueVideoGain) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_SetBlueVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoBlackLevel);
extern void (*UPnPFP_RenderingControl_GetMute) (void* upnptoken,unsigned int InstanceID,char* Channel);
extern void (*UPnPFP_RenderingControl_SetBlueVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoGain);
extern void (*UPnPFP_RenderingControl_GetVerticalKeystone) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_SetVerticalKeystone) (void* upnptoken,unsigned int InstanceID,short DesiredVerticalKeystone);
extern void (*UPnPFP_RenderingControl_GetBrightness) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetVolumeDB) (void* upnptoken,unsigned int InstanceID,char* Channel);
extern void (*UPnPFP_RenderingControl_GetGreenVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_GetRedVideoGain) (void* upnptoken,unsigned int InstanceID);
extern void (*UPnPFP_RenderingControl_SetMute) (void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute);
extern void (*UPnPFP_RenderingControl_SetGreenVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoGain);
extern void (*UPnPFP_RenderingControl_SetSharpness) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredSharpness);
extern void (*UPnPFP_RenderingControl_SetHorizontalKeystone) (void* upnptoken,unsigned int InstanceID,short DesiredHorizontalKeystone);
extern void (*UPnPFP_RenderingControl_SetColorTemperature) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredColorTemperature);

/* Invocation Response Methods */
void UPnPResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg);
void UPnPResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params);
int  UPnPPresentationResponse(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UPnPResponse_RenderingControl_GetHorizontalKeystone(const void* UPnPToken, const short CurrentHorizontalKeystone);
void UPnPResponse_RenderingControl_GetVolume(const void* UPnPToken, const unsigned short CurrentVolume);
void UPnPResponse_RenderingControl_SelectPreset(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetVolume(const void* UPnPToken);
void UPnPResponse_RenderingControl_ListPresets(const void* UPnPToken, const char* CurrentPresetNameList);
void UPnPResponse_RenderingControl_SetVolumeDB(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetRedVideoBlackLevel(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetContrast(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetLoudness(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetBrightness(const void* UPnPToken);
void UPnPResponse_RenderingControl_GetLoudness(const void* UPnPToken, const int CurrentLoudness);
void UPnPResponse_RenderingControl_GetColorTemperature(const void* UPnPToken, const unsigned short CurrentColorTemperature);
void UPnPResponse_RenderingControl_GetSharpness(const void* UPnPToken, const unsigned short CurrentSharpness);
void UPnPResponse_RenderingControl_GetContrast(const void* UPnPToken, const unsigned short CurrentContrast);
void UPnPResponse_RenderingControl_GetGreenVideoGain(const void* UPnPToken, const unsigned short CurrentGreenVideoGain);
void UPnPResponse_RenderingControl_SetRedVideoGain(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetGreenVideoBlackLevel(const void* UPnPToken);
void UPnPResponse_RenderingControl_GetVolumeDBRange(const void* UPnPToken, const short MinValue, const short MaxValue);
void UPnPResponse_RenderingControl_GetRedVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentRedVideoBlackLevel);
void UPnPResponse_RenderingControl_GetBlueVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentBlueVideoBlackLevel);
void UPnPResponse_RenderingControl_GetBlueVideoGain(const void* UPnPToken, const unsigned short CurrentBlueVideoGain);
void UPnPResponse_RenderingControl_SetBlueVideoBlackLevel(const void* UPnPToken);
void UPnPResponse_RenderingControl_GetMute(const void* UPnPToken, const int CurrentMute);
void UPnPResponse_RenderingControl_SetBlueVideoGain(const void* UPnPToken);
void UPnPResponse_RenderingControl_GetVerticalKeystone(const void* UPnPToken, const short CurrentVerticalKeystone);
void UPnPResponse_RenderingControl_SetVerticalKeystone(const void* UPnPToken);
void UPnPResponse_RenderingControl_GetBrightness(const void* UPnPToken, const unsigned short CurrentBrightness);
void UPnPResponse_RenderingControl_GetVolumeDB(const void* UPnPToken, const short CurrentVolume);
void UPnPResponse_RenderingControl_GetGreenVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentGreenVideoBlackLevel);
void UPnPResponse_RenderingControl_GetRedVideoGain(const void* UPnPToken, const unsigned short CurrentRedVideoGain);
void UPnPResponse_RenderingControl_SetMute(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetGreenVideoGain(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetSharpness(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetHorizontalKeystone(const void* UPnPToken);
void UPnPResponse_RenderingControl_SetColorTemperature(const void* UPnPToken);
void UPnPResponse_AVTransport_GetCurrentTransportActions(const void* UPnPToken, const char* Actions);
void UPnPResponse_AVTransport_Play(const void* UPnPToken);
void UPnPResponse_AVTransport_Previous(const void* UPnPToken);
void UPnPResponse_AVTransport_Next(const void* UPnPToken);
void UPnPResponse_AVTransport_Stop(const void* UPnPToken);
void UPnPResponse_AVTransport_GetTransportSettings(const void* UPnPToken, const char* PlayMode, const char* RecQualityMode);
void UPnPResponse_AVTransport_Seek(const void* UPnPToken);
void UPnPResponse_AVTransport_Pause(const void* UPnPToken);
void UPnPResponse_AVTransport_GetPositionInfo(const void* UPnPToken, const unsigned int Track, const char* TrackDuration, const char* TrackMetaData, const char* TrackURI, const char* RelTime, const char* AbsTime, const int RelCount, const int AbsCount);
void UPnPResponse_AVTransport_GetTransportInfo(const void* UPnPToken, const char* CurrentTransportState, const char* CurrentTransportStatus, const char* CurrentSpeed);
void UPnPResponse_AVTransport_SetAVTransportURI(const void* UPnPToken);
void UPnPResponse_AVTransport_GetDeviceCapabilities(const void* UPnPToken, const char* PlayMedia, const char* RecMedia, const char* RecQualityModes);
void UPnPResponse_AVTransport_SetPlayMode(const void* UPnPToken);
void UPnPResponse_AVTransport_GetMediaInfo(const void* UPnPToken, const unsigned int NrTracks, const char* MediaDuration, const char* CurrentURI, const char* CurrentURIMetaData, const char* NextURI, const char* NextURIMetaData, const char* PlayMedium, const char* RecordMedium, const char* WriteStatus);
void UPnPResponse_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* ProtocolInfo, const char* PeerConnectionManager, const int PeerConnectionID, const char* Direction, const char* Status);
void UPnPResponse_ConnectionManager_PrepareForConnection(const void* UPnPToken, const int ConnectionID, const int AVTransportID, const int RcsID);
void UPnPResponse_ConnectionManager_ConnectionComplete(const void* UPnPToken);
void UPnPResponse_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* Source, const char* Sink);
void UPnPResponse_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* ConnectionIDs);

/* State Variable Eventing Methods */
void UPnPSetState_RenderingControl_LastChange(void *microstack,char* val);
void UPnPSetState_AVTransport_LastChange(void *microstack,char* val);
void UPnPSetState_ConnectionManager_SourceProtocolInfo(void *microstack,char* val);
void UPnPSetState_ConnectionManager_SinkProtocolInfo(void *microstack,char* val);
void UPnPSetState_ConnectionManager_CurrentConnectionIDs(void *microstack,char* val);

#endif
