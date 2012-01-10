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

#ifndef __CPControlPoint__
#define __CPControlPoint__

#include "UPnPControlPointStructs.h"

/*! \file CPControlPoint.h 
	\brief MicroStack APIs for Control Point Implementation
*/

/*! \defgroup ControlPoint Control Point Module
	\{
*/


/* Complex Type Parsers */


/* Complex Type Serializers */




/*! \defgroup CPReferenceCounter Reference Counter Methods
	\ingroup ControlPoint
	\brief Reference Counting for the UPnPDevice and UPnPService objects.
	\para
	Whenever a user application is going to keep the pointers to the UPnPDevice object that is obtained from
	the add sink (or any pointers inside them), the application <b>must</b> increment the reference counter. Failure to do so
	will lead to references to invalid pointers, when the device leaves the network.
	\{
*/
void CPAddRef(struct UPnPDevice *device);
void CPRelease(struct UPnPDevice *device);
/*! \} */   



struct UPnPDevice* CPGetDevice1(struct UPnPDevice *device,int index);
int CPGetDeviceCount(struct UPnPDevice *device);
struct UPnPDevice* CPGetDeviceEx(struct UPnPDevice *device, char* DeviceType, int start,int number);
void PrintUPnPDevice(int indents, struct UPnPDevice *device);



/*! \defgroup CPAdministration Administrative Methods
	\ingroup ControlPoint
	\brief Basic administrative functions, used to setup/configure the control point application
	\{
*/
void *CPCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void CPControlPoint_AddDiscoveryErrorHandler(void *cpToken, UPnPDeviceDiscoveryErrorHandler callback);
struct UPnPDevice* CPGetDeviceAtUDN(void *v_CP, char* UDN);
void CP_CP_IPAddressListChanged(void *CPToken);
int CPHasAction(struct UPnPService *s, char* action);
void CPUnSubscribeUPnPEvents(struct UPnPService *service);
void CPSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service, int OK));
struct UPnPService *CPGetService(struct UPnPDevice *device, char* ServiceName, int length);

void CPSetUser(void *token, void *user);
void* CPGetUser(void *token);

struct UPnPService *CPGetService_DeviceProtection(struct UPnPDevice *device);

/*! \} */


/*! \defgroup InvocationEventingMethods Invocation/Eventing Methods
	\ingroup ControlPoint
	\brief Methods used to invoke actions and receive events from a UPnPService
	\{
*/
extern void (*CPEventCallback_DeviceProtection_SetupReady)(struct UPnPService* Service,int SetupReady);

void CPInvoke_DeviceProtection_AddIdentityList(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* IdentityListResult),void* _user, char* unescaped_IdentityList);
void CPInvoke_DeviceProtection_AddRolesForIdentity(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_Identity, char* unescaped_RoleList);
void CPInvoke_DeviceProtection_GetACLData(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* ACL),void* _user);
void CPInvoke_DeviceProtection_GetAssignedRoles(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* RoleList),void* _user);
void CPInvoke_DeviceProtection_GetRolesForAction(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* RoleList,char* RestrictedRoleList),void* _user, char* unescaped_DeviceUDN, char* unescaped_ServiceId, char* unescaped_ActionName);
void CPInvoke_DeviceProtection_GetSupportedProtocols(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* ProtocolList),void* _user);
void CPInvoke_DeviceProtection_GetUserLoginChallenge(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength),void* _user, char* unescaped_ProtocolType, char* unescaped_Name);
void CPInvoke_DeviceProtection_RemoveIdentity(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_Identity);
void CPInvoke_DeviceProtection_RemoveRolesForIdentity(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_Identity, char* unescaped_RoleList);
void CPInvoke_DeviceProtection_SendSetupMessage(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,unsigned char* OutMessage,int OutMessageLength),void* _user, char* unescaped_ProtocolType, unsigned char* InMessage, int InMessageLength);
void CPInvoke_DeviceProtection_SetUserLoginPassword(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_ProtocolType, char* unescaped_Name, unsigned char* Stored, int StoredLength, unsigned char* Salt, int SaltLength);
void CPInvoke_DeviceProtection_UserLogin(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_ProtocolType, unsigned char* Challenge, int ChallengeLength, unsigned char* Authenticator, int AuthenticatorLength);
void CPInvoke_DeviceProtection_UserLogout(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user);

/*! \} */


/*! \} */
#endif
