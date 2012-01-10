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

#ifndef __DVMicrostack__
#define __DVMicrostack__


#include "ILibAsyncSocket.h"

/*! \file DVMicroStack.h 
	\brief MicroStack APIs for Device Implementation
*/

/*! \defgroup MicroStack MicroStack Module
	\{
*/

struct DVDataObject;
struct packetheader;

typedef void* DVMicroStackToken;
typedef void* DVSessionToken;




/* Complex Type Parsers */


/* Complex Type Serializers */



/* DV Stack Management */
DVMicroStackToken DVCreateMicroStack(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);


void DVIPAddressListChanged(DVMicroStackToken MicroStackToken);
int DVGetLocalPortNumber(DVSessionToken token);
int   DVGetLocalInterfaceToHost(const DVSessionToken DVToken);
void* DVGetWebServerToken(const DVMicroStackToken MicroStackToken);
void DVSetTag(const DVMicroStackToken token, void *UserToken);
void *DVGetTag(const DVMicroStackToken token);
DVMicroStackToken DVGetMicroStackTokenFromSessionToken(const DVSessionToken token);
int DVGetSecurePortNumber(const DVMicroStackToken stack); // vbl added
char * DVGetCertHashFromSessionToken(const DVSessionToken token); // vbl added
char * DVGetCertNameFromSessionToken(const DVSessionToken token); // vbl added

typedef void(*DV_ActionHandler_DeviceProtection_AddIdentityList) (void* upnptoken,char* IdentityList);
typedef void(*DV_ActionHandler_DeviceProtection_AddRolesForIdentity) (void* upnptoken,char* Identity,char* RoleList);
typedef void(*DV_ActionHandler_DeviceProtection_GetACLData) (void* upnptoken);
typedef void(*DV_ActionHandler_DeviceProtection_GetAssignedRoles) (void* upnptoken);
typedef void(*DV_ActionHandler_DeviceProtection_GetRolesForAction) (void* upnptoken,char* DeviceUDN,char* ServiceId,char* ActionName);
typedef void(*DV_ActionHandler_DeviceProtection_GetSupportedProtocols) (void* upnptoken);
typedef void(*DV_ActionHandler_DeviceProtection_GetUserLoginChallenge) (void* upnptoken,char* ProtocolType,char* Name);
typedef void(*DV_ActionHandler_DeviceProtection_RemoveIdentity) (void* upnptoken,char* Identity);
typedef void(*DV_ActionHandler_DeviceProtection_RemoveRolesForIdentity) (void* upnptoken,char* Identity,char* RoleList);
typedef void(*DV_ActionHandler_DeviceProtection_SendSetupMessage) (void* upnptoken,char* ProtocolType,unsigned char* InMessage,int _InMessageLength);
typedef void(*DV_ActionHandler_DeviceProtection_SetUserLoginPassword) (void* upnptoken,char* ProtocolType,char* Name,unsigned char* Stored,int _StoredLength,unsigned char* Salt,int _SaltLength);
typedef void(*DV_ActionHandler_DeviceProtection_UserLogin) (void* upnptoken,char* ProtocolType,unsigned char* Challenge,int _ChallengeLength,unsigned char* Authenticator,int _AuthenticatorLength);
typedef void(*DV_ActionHandler_DeviceProtection_UserLogout) (void* upnptoken);
/* DV Set Function Pointers Methods */
extern void (*DVFP_PresentationPage) (void* upnptoken,struct packetheader *packet);
extern DV_ActionHandler_DeviceProtection_AddIdentityList DVFP_DeviceProtection_AddIdentityList;
extern DV_ActionHandler_DeviceProtection_AddRolesForIdentity DVFP_DeviceProtection_AddRolesForIdentity;
extern DV_ActionHandler_DeviceProtection_GetACLData DVFP_DeviceProtection_GetACLData;
extern DV_ActionHandler_DeviceProtection_GetAssignedRoles DVFP_DeviceProtection_GetAssignedRoles;
extern DV_ActionHandler_DeviceProtection_GetRolesForAction DVFP_DeviceProtection_GetRolesForAction;
extern DV_ActionHandler_DeviceProtection_GetSupportedProtocols DVFP_DeviceProtection_GetSupportedProtocols;
extern DV_ActionHandler_DeviceProtection_GetUserLoginChallenge DVFP_DeviceProtection_GetUserLoginChallenge;
extern DV_ActionHandler_DeviceProtection_RemoveIdentity DVFP_DeviceProtection_RemoveIdentity;
extern DV_ActionHandler_DeviceProtection_RemoveRolesForIdentity DVFP_DeviceProtection_RemoveRolesForIdentity;
extern DV_ActionHandler_DeviceProtection_SendSetupMessage DVFP_DeviceProtection_SendSetupMessage;
extern DV_ActionHandler_DeviceProtection_SetUserLoginPassword DVFP_DeviceProtection_SetUserLoginPassword;
extern DV_ActionHandler_DeviceProtection_UserLogin DVFP_DeviceProtection_UserLogin;
extern DV_ActionHandler_DeviceProtection_UserLogout DVFP_DeviceProtection_UserLogout;


void DVSetDisconnectFlag(DVSessionToken token,void *flag);

/* Invocation Response Methods */
void DVResponse_Error(const DVSessionToken DVToken, const int ErrorCode, const char* ErrorMsg);
void DVResponseGeneric(const DVSessionToken DVToken,const char* ServiceURI,const char* MethodName,const char* Params);
void DVResponse_DeviceProtection_AddIdentityList(const DVSessionToken DVToken, const char* IdentityListResult);
void DVResponse_DeviceProtection_AddRolesForIdentity(const DVSessionToken DVToken);
void DVResponse_DeviceProtection_GetACLData(const DVSessionToken DVToken, const char* ACL);
void DVResponse_DeviceProtection_GetAssignedRoles(const DVSessionToken DVToken, const char* RoleList);
void DVResponse_DeviceProtection_GetRolesForAction(const DVSessionToken DVToken, const char* RoleList, const char* RestrictedRoleList);
void DVResponse_DeviceProtection_GetSupportedProtocols(const DVSessionToken DVToken, const char* ProtocolList);
void DVResponse_DeviceProtection_GetUserLoginChallenge(const DVSessionToken DVToken, const unsigned char* Salt, const int _SaltLength, const unsigned char* Challenge, const int _ChallengeLength);
void DVResponse_DeviceProtection_RemoveIdentity(const DVSessionToken DVToken);
void DVResponse_DeviceProtection_RemoveRolesForIdentity(const DVSessionToken DVToken);
void DVResponse_DeviceProtection_SendSetupMessage(const DVSessionToken DVToken, const unsigned char* OutMessage, const int _OutMessageLength);
void DVResponse_DeviceProtection_SetUserLoginPassword(const DVSessionToken DVToken);
void DVResponse_DeviceProtection_UserLogin(const DVSessionToken DVToken);
void DVResponse_DeviceProtection_UserLogout(const DVSessionToken DVToken);

/* State Variable Eventing Methods */
void DVSetState_DeviceProtection_SetupReady(DVMicroStackToken microstack,int val);






/*! \} */
#endif
