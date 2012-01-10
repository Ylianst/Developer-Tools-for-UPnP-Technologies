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

struct UPnPDataObject;
struct packetheader;

/* These methods must be implemented by the user */
extern void UPnPDimmingService_GetLoadLevelStatus(void* upnptoken);
extern void UPnPDimmingService_GetMinLevel(void* upnptoken);
extern void UPnPDimmingService_SetLoadLevelTarget(void* upnptoken,unsigned char NewLoadLevelTarget);
extern void UPnPSwitchPower_GetStatus(void* upnptoken);
extern void UPnPSwitchPower_SetTarget(void* upnptoken,int newTargetValue);
extern void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet);

/* UPnP Stack Management */
void *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void *UPnP(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
void UPnPIPAddressListChanged(void *MicroStackToken);
int UPnPGetLocalPortNumber(void *token);
int   UPnPGetLocalInterfaceToHost(const void* UPnPToken);
void* UPnPGetInstance(const void* UPnPToken);

/* Invocation Response Methods */
void UPnPResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg);
void UPnPResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params);
int  UPnPPresentationResponse(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UPnPResponse_SwitchPower_GetStatus(const void* UPnPToken, const int ResultStatus);
void UPnPResponse_SwitchPower_SetTarget(const void* UPnPToken);
void UPnPResponse_DimmingService_GetLoadLevelStatus(const void* UPnPToken, const unsigned char RetLoadLevelStatus);
void UPnPResponse_DimmingService_GetMinLevel(const void* UPnPToken, const unsigned char MinLevel);
void UPnPResponse_DimmingService_SetLoadLevelTarget(const void* UPnPToken);

/* State Variable Eventing Methods */
void UPnPSetState_SwitchPower_Status(void *microstack,int val);
void UPnPSetState_DimmingService_LoadLevelStatus(void *microstack,unsigned char val);

#endif
