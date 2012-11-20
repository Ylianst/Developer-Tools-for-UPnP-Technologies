/*   
Copyright 2006 - 2010 Intel Corporation

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


#include "ILibAsyncSocket.h"

/*! \file UPnPMicroStack.h 
	\brief MicroStack APIs for Device Implementation
*/

/*! \defgroup MicroStack MicroStack Module
	\{
*/

struct UPnPDataObject;
struct packetheader;

typedef void* UPnPMicroStackToken;
typedef void* UPnPSessionToken;

//{{{BEGIN_MulticastEventing}}}
enum MULTICAST_EVENT_TYPE
{
	MULTICASTEVENT_TYPE_UNKNOWN =-1,
	MULTICASTEVENT_TYPE_GENERAL =0,
	MULTICASTEVENT_TYPE_INFO	=1,
	MULTICASTEVENT_TYPE_WARNING	=2,
	MULTICASTEVENT_TYPE_FAULT	=3,
	MULTICASTEVENT_TYPE_EMERGENCY=4
};
static char *MULTICAST_EVENT_TYPE_DESCRIPTION[5]={"upnp:/general","upnp:/info","upnp:/warning","upnp:/fault","upnp:/emergency"};
typedef void(*UPnPEvent_MulticastGeneric_Handler)(UPnPMicroStackToken sender, char *Origin_ServiceType, int Origin_ServiceVersion, char *Origin_ServiceID, char *Origin_DeviceUDN, enum MULTICAST_EVENT_TYPE eventType, char* VariableName, char *VariableValue);
extern UPnPEvent_MulticastGeneric_Handler UPnPOnEvent_MulticastGeneric;
//{{{BEGIN_MulticastEventing_Specific}}}typedef void(*UPnPMulticastEvent_{{{SERVICENAME}}}_{{{VARNAME}}}_Handler)(UPnPMicroStackToken sender, char *Origin_ServiceID, char *Origin_DeviceUDN, enum MULTICAST_EVENT_TYPE eventType, {{{ARGLIST}}});
extern UPnPMulticastEvent_{{{SERVICENAME}}}_{{{VARNAME}}}_Handler UPnPOnMulticastEvent_{{{SERVICENAME}}}_{{{VARNAME}}};
//{{{END_MulticastEventing_Specific}}}

;
//{{{END_MulticastEventing}}}

//{{{ComplexTypeCode}}}

/* UPnP Stack Management */
//{{{CreateMicroStackHeader}}}

void UPnPIPAddressListChanged(UPnPMicroStackToken MicroStackToken);
int UPnPGetLocalPortNumber(UPnPSessionToken token);
void* UPnPGetWebServerToken(const UPnPMicroStackToken MicroStackToken);
void UPnPSetTag(const UPnPMicroStackToken token, void *UserToken);
void *UPnPGetTag(const UPnPMicroStackToken token);
UPnPMicroStackToken UPnPGetMicroStackTokenFromSessionToken(const UPnPSessionToken token);

//{{{UPnP_Set_Function_Pointer_Methods}}}
void UPnPSetDisconnectFlag(UPnPSessionToken token,void *flag);

//{{{Invocation_Response_Methods}}}
//{{{Eventing_Methods}}}
//{{{MulticastEventing_Methods}}}
//{{{ObjectDefintions}}}
//{{{GetConfiguration}}}


/*! \} */
#endif
