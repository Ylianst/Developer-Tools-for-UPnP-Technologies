#ifndef CONTROL_POINTS_MANAGER_H
#define CONTROL_POINTS_MANAGER_H

#include "UPnPControlPointStructs.h"

/*
void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status);
void AVRCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink);
void AVRCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs);
void AVRCP_ResponseSink_RenderingControl_SetVolume(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_RenderingControl_GetMute(struct UPnPService* Service,int ErrorCode,void *User,int CurrentMute);
void AVRCP_ResponseSink_RenderingControl_SetMute(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_RenderingControl_GetVolume(struct UPnPService* Service,int ErrorCode,void *User,unsigned short CurrentVolume);
void AVRCP_ResponseSink_AVTransport_GetCurrentTransportActions(struct UPnPService* Service,int ErrorCode,void *User,char* Actions);
void AVRCP_ResponseSink_AVTransport_Play(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_Previous(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_Next(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_Stop(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_GetTransportSettings(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMode,char* RecQualityMode);
void AVRCP_ResponseSink_AVTransport_Seek(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_Pause(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_GetPositionInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Track,char* TrackDuration,char* TrackMetaData,char* TrackURI,char* RelTime,char* AbsTime,int RelCount,int AbsCount);
void AVRCP_ResponseSink_AVTransport_GetTransportInfo(struct UPnPService* Service,int ErrorCode,void *User,char* CurrentTransportState,char* CurrentTransportStatus,char* CurrentSpeed);
void AVRCP_ResponseSink_AVTransport_SetAVTransportURI(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_GetDeviceCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* PlayMedia,char* RecMedia,char* RecQualityModes);
void AVRCP_ResponseSink_AVTransport_SetPlayMode(struct UPnPService* Service,int ErrorCode,void *User);
void AVRCP_ResponseSink_AVTransport_GetMediaInfo(struct UPnPService* Service,int ErrorCode,void *User,unsigned int NrTracks,char* MediaDuration,char* CurrentURI,char* CurrentURIMetaData,char* NextURI,char* NextURIMetaData,char* PlayMedium,char* RecordMedium,char* WriteStatus);
void AVRCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo);
void AVRCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo);
void AVRCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs);
void AVRCP_EventSink_RenderingControl_LastChange(struct UPnPService* Service,char* LastChange);
void AVRCP_EventSink_AVTransport_LastChange(struct UPnPService* Service,char* LastChange);
void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status);
void MSCP_ResponseSink_ConnectionManager_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink);
void MSCP_ResponseSink_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs);
void MSCP_ResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID);
void MSCP_ResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps);
void MSCP_ResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id);
void MSCP_ResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps);
void MSCP_EventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo);
void MSCP_EventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo);
void MSCP_EventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs);
void MSCP_EventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID);
*/

void CP_Manager_Init();


#endif