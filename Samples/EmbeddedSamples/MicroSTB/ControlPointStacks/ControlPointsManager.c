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

#include "ControlPointsManager.h"

#include <stdio.h>

void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void AVRCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("AVRCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void AVRCP_ResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetVolume[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetMute[ErrorCode:%d](%d)\r\n",ErrorCode,CurrentMute);
}

void AVRCP_ResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: RenderingControl/SetMute[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume)
{
	printf("AVRCP_ Invoke Response: RenderingControl/GetVolume[ErrorCode:%d](%u)\r\n",ErrorCode,CurrentVolume);
}

void AVRCP_ResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetCurrentTransportActions[ErrorCode:%d](%s)\r\n",ErrorCode,Actions);
}

void AVRCP_ResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Play[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Previous[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Next[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Stop[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportSettings[ErrorCode:%d](%s,%s)\r\n",ErrorCode,PlayMode,RecQualityMode);
}

void AVRCP_ResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Seek[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/Pause[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetPositionInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%d,%d)\r\n",ErrorCode,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
}

void AVRCP_ResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetTransportInfo[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
}

void AVRCP_ResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetAVTransportURI[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetDeviceCapabilities[ErrorCode:%d](%s,%s,%s)\r\n",ErrorCode,PlayMedia,RecMedia,RecQualityModes);
}

void AVRCP_ResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("AVRCP_ Invoke Response: AVTransport/SetPlayMode[ErrorCode:%d]()\r\n",ErrorCode);
}

void AVRCP_ResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus)
{
	printf("AVRCP_ Invoke Response: AVTransport/GetMediaInfo[ErrorCode:%d](%u,%s,%s,%s,%s,%s,%s,%s,%s)\r\n",ErrorCode,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
}

void AVRCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void AVRCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("AVRCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("AVRCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void AVRCP_EventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/RenderingControl/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

void AVRCP_EventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange)
{
	printf("AVRCP_ Event from %s/AVTransport/LastChange: %s\r\n",Service->Parent->FriendlyName,LastChange);
}

void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionInfo[ErrorCode:%d](%d,%d,%s,%s,%d,%s,%s)\r\n",ErrorCode,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
}

void MSCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetProtocolInfo[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Source,Sink);
}

void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("MSCP_ Invoke Response: ConnectionManager/GetCurrentConnectionIDs[ErrorCode:%d](%s)\r\n",ErrorCode,ConnectionIDs);
}

void MSCP_ResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP_ Invoke Response: ContentDirectory/Browse[ErrorCode:%d](%s,%u,%u,%u)\r\n",ErrorCode,Result,NumberReturned,TotalMatches,UpdateID);
}

void MSCP_ResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSortCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SortCaps);
}

void MSCP_ResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSystemUpdateID[ErrorCode:%d](%u)\r\n",ErrorCode,Id);
}

void MSCP_ResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("MSCP_ Invoke Response: ContentDirectory/GetSearchCapabilities[ErrorCode:%d](%s)\r\n",ErrorCode,SearchCaps);
}

void MSCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
}

void MSCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("MSCP_ Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
}

void MSCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("MSCP_ Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
}

void MSCP_EventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("MSCP_ Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
}