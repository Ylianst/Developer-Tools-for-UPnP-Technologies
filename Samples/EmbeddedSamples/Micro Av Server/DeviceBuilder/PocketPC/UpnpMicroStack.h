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
extern void UpnpConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID);
extern void UpnpConnectionManager_GetProtocolInfo(void* upnptoken);
extern void UpnpConnectionManager_GetCurrentConnectionIDs(void* upnptoken);
extern void UpnpContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria);
extern void UpnpContentDirectory_GetSortCapabilities(void* upnptoken);
extern void UpnpContentDirectory_GetSystemUpdateID(void* upnptoken);
extern void UpnpContentDirectory_GetSearchCapabilities(void* upnptoken);
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
void UpnpAsyncResponse_START(const void* UPnPToken, const char* actionName, const char* serviceUrnWithVersion);
void UpnpAsyncResponse_DONE(const void* UPnPToken, const char* actionName);
void UpnpAsyncResponse_OUT(const void* UPnPToken, const char* outArgName, const char* bytes, const int byteLength, const int startArg, const int endArg);
void UpnpResponse_ContentDirectory_Browse(const void* UPnPToken, const char* Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID);
void UpnpResponse_ContentDirectory_GetSortCapabilities(const void* UPnPToken, const char* SortCaps);
void UpnpResponse_ContentDirectory_GetSystemUpdateID(const void* UPnPToken, const unsigned int Id);
void UpnpResponse_ContentDirectory_GetSearchCapabilities(const void* UPnPToken, const char* SearchCaps);
void UpnpResponse_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* ProtocolInfo, const char* PeerConnectionManager, const int PeerConnectionID, const char* Direction, const char* Status);
void UpnpResponse_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* Source, const char* Sink);
void UpnpResponse_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* ConnectionIDs);

/* State Variable Eventing Methods */
void UpnpSetState_ContentDirectory_SystemUpdateID(void *microstack,unsigned int val);
void UpnpSetState_ConnectionManager_SourceProtocolInfo(void *microstack,char* val);
void UpnpSetState_ConnectionManager_SinkProtocolInfo(void *microstack,char* val);
void UpnpSetState_ConnectionManager_CurrentConnectionIDs(void *microstack,char* val);

#endif
