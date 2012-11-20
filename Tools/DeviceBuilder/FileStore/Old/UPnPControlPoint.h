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

#ifndef __UPnPControlPoint__
#define __UPnPControlPoint__

#include "UPnPControlPointStructs.h"

/*! \file UPnPControlPoint.h 
	\brief MicroStack APIs for Control Point Implementation
*/

/*! \defgroup ControlPoint Control Point Module
	\{
*/

//{{{UPnPComplexTypes}}}


/*! \defgroup CPReferenceCounter Reference Counter Methods
	\ingroup ControlPoint
	\brief Reference Counting for the UPnPDevice and UPnPService objects.
	\para
	Whenever a user application is going to keep the pointers to the UPnPDevice object that is obtained from
	the add sink (or any pointers inside them), the application <b>must</b> increment the reference counter. Failure to do so
	will lead to references to invalid pointers, when the device leaves the network.
	\{
*/
void UPnPAddRef(struct UPnPDevice *device);
void UPnPRelease(struct UPnPDevice *device);
/*! \} */   



struct UPnPDevice* UPnPGetDevice1(struct UPnPDevice *device,int index);
int UPnPGetDeviceCount(struct UPnPDevice *device);
struct UPnPDevice* UPnPGetDeviceEx(struct UPnPDevice *device, char* DeviceType, int start,int number);
void PrintUPnPDevice(int indents, struct UPnPDevice *device);

//{{{BEGIN_CustomTagSpecific}}}
/*! \defgroup CustomXMLTags Custom XML Tags
	\ingroup ControlPoint
	\brief Methods used to obtain metadata information from a specific UPnPDevice object.
	\{
*/
char *UPnPGetCustomTagFromDevice(struct UPnPDevice *d, char* FullNameSpace, char* Name);
//{{{CustomXMLTags}}}
/*! \} */
//{{{END_CustomTagSpecific}}}


/*! \defgroup CPAdministration Administrative Methods
	\ingroup ControlPoint
	\brief Basic administrative functions, used to setup/configure the control point application
	\{
*/
void *UPnPCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void UPnPControlPoint_AddDiscoveryErrorHandler(void *cpToken, UPnPDeviceDiscoveryErrorHandler callback);
struct UPnPDevice* UPnPGetDeviceAtUDN(void *v_CP, char* UDN);
void UPnP_CP_IPAddressListChanged(void *CPToken);
int UPnPHasAction(struct UPnPService *s, char* action);
void UPnPUnSubscribeUPnPEvents(struct UPnPService *service);
void UPnPSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service, int OK));
struct UPnPService *UPnPGetService(struct UPnPDevice *device, char* ServiceName, int length);

void UPnPSetUser(void *token, void *user);
void* UPnPGetUser(void *token);

//{{{UPnPGetService}}}
/*! \} */


/*! \defgroup InvocationEventingMethods Invocation/Eventing Methods
	\ingroup ControlPoint
	\brief Methods used to invoke actions and receive events from a UPnPService
	\{
*/
//{{{ExternEventCallbacks}}}
//{{{UPnPInvoke_Methods}}}
/*! \} */


/*! \} */
#endif
