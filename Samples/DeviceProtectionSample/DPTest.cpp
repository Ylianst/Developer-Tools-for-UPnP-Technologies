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

#if defined(WIN32) || defined(_WIN32_WCE)
#	ifndef MICROSTACK_NO_STDAFX
#		include "stdafx.h"
#	endif
char* CPPLATFORM = "WINDOWS UPnP/1.0 MicroStack/1.0.3769";
#elif defined(__SYMBIAN32__)
char* CPPLATFORM = "SYMBIAN UPnP/1.0 MicroStack/1.0.3769";
#else
char* CPPLATFORM = "POSIX UPnP/1.0 MicroStack/1.0.3769";
#endif

#if defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#define snprintf _snprintf
#endif

#if defined(WINSOCK2)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#include <wininet.h>
#endif

extern "C" {
#include "CPControlPoint.h"
}

#if defined(WIN32) && !defined(_WIN32_WCE)
#include <crtdbg.h>
#endif

#include "WPSRegistrar.h"

#include "DPServerInfo.h"
extern DPServerInfo g_devProt;

bool g_RunAllTests = true;

extern bool StringToGUID(char * uuid, GUID & theGUID);
extern bool IsNameBasedUUID(void * guid);
extern string UUIDToString(void * guid);

// For convenience sake (and to avoid adding parameters to the various function calls), declare some global pointers
// here for the UPnP Device, the DeviceProtection service, and the ControlPoint's GUID.  Using these globals means that 
// this test code is not reentrant.  It will only work correctly if it is called synchronously by a single thread.  If
// more than one thread calls into this code at a time, then only the last caller's pointers will be used.  So, if 
// multiple Devices are discovered, they should be processed one at a time.
//
GUID * theCPGUID = NULL;
struct UPnPService *theService;
struct UPnPDevice *theDevice;

// Most of these response sink functions simply print out a result value and return.  However, in certain cases a
// response sink function serves as a synchronization point to defer processing of a particular sequence of actions
// until that response sink is called.  Otherwise, the control thread might send requests that will be rejected if
// operations such as WPS introduction or UserLogin() have not yet completed.
//


void TestEventSink_DeviceProtection_SetupReady(struct UPnPService* Service,int SetupReady)
{
	printf("CP Event from %s/DeviceProtection/SetupReady: %d\r\n",Service->Parent->FriendlyName,SetupReady);
}


void TestResponseSink_DeviceProtection_AddIdentityList(struct UPnPService* Service,int ErrorCode,void *User,char* IdentityListResult)
{
	printf("CP Invoke Response: DeviceProtection/AddIdentityList[ErrorCode:%d](%s)\r\n",ErrorCode,IdentityListResult);
}

void TestResponseSink_DeviceProtection_AddRolesForIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/AddRolesForIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void TestResponseSink_DeviceProtection_GetACLData(struct UPnPService* Service,int ErrorCode,void *User,char* ACL)
{
	printf("CP Invoke Response: DeviceProtection/GetACLData[ErrorCode:%d](%s)\r\n",ErrorCode,ACL);
}

void TestResponseSink_DeviceProtection_GetAssignedRoles(struct UPnPService* Service,int ErrorCode,void *User,char* RoleList)
{
	printf("CP Invoke Response: DeviceProtection/GetAssignedRoles[ErrorCode:%d](%s)\r\n",ErrorCode,RoleList);
}

void TestResponseSink_DeviceProtection_GetRolesForAction(struct UPnPService* Service,int ErrorCode,void *User,char* RoleList,char* RestrictedRoleList)
{
	printf("CP Invoke Response: DeviceProtection/GetRolesForAction[ErrorCode:%d](%s,%s)\r\n",ErrorCode,RoleList,RestrictedRoleList);
}

void TestResponseSink_DeviceProtection_GetSupportedProtocols(struct UPnPService* Service,int ErrorCode,void *User,char* ProtocolList)
{
	printf("CP Invoke Response: DeviceProtection/GetSupportedProtocols[ErrorCode:%d](%s)\r\n",ErrorCode,ProtocolList);
}


void TestResponseSink_DeviceProtection_RemoveIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/RemoveIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void TestResponseSink_DeviceProtection_RemoveRolesForIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/RemoveRolesForIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void TestResponseSink_DeviceProtection_SendSetupMessage(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength)
{
	printf("CP Invoke Response: DeviceProtection/SendSetupMessage[ErrorCode:%d](%s)\r\n",ErrorCode,OutMessage);

	if (ErrorCode == 0 && User) {
		WPSRegistrar * registrar = (WPSRegistrar *) User;
		registrar->SetBuffer(OutMessage, OutMessageLength);
		registrar->RunWPS();
	} 
}

extern int g_Pin;

void ErrorResponseSink_DeviceProtection_SendSetupMessage(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength)
{
	printf("CP Invoke Response: DeviceProtection/SendSetupMessage[ErrorCode:%d](%s)\r\n",ErrorCode,OutMessage);

	if (ErrorCode != 0 && User) { // sanity check

		WPSRegistrar * registrar = (WPSRegistrar *) User;
		// Re-initialize the Registrar and try again with the correct PIN
		registrar->StartWPS(Service, g_Pin, g_Pin ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN, &TestResponseSink_DeviceProtection_SendSetupMessage);
		registrar->RunWPS();
	} else 	if (ErrorCode == 0 && User) {
		WPSRegistrar * registrar = (WPSRegistrar *) User;
		registrar->SetBuffer(OutMessage, OutMessageLength);
		registrar->RunWPS();
	} 
}

void TestResponseSink_DeviceProtection_SetUserLoginPassword(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/SetUserLoginPassword[ErrorCode:%d]()\r\n",ErrorCode);
}

int ContinueDeviceProtectionTests(void);
int ContinueDeviceProtectionTests2(void);

void TestResponseSink_DeviceProtection_UserLogin(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/UserLogin[ErrorCode:%d]()\r\n",ErrorCode);
	if (ErrorCode == 0) {
		ContinueDeviceProtectionTests2();
	}
}

void TestResponseSink_DeviceProtection_UserLogout(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/UserLogout[ErrorCode:%d]()\r\n",ErrorCode);
}

void TestResponseSink_DeviceProtection_GetUserLoginChallenge(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength)
{
	printf("CP Invoke Response: DeviceProtection/GetUserLoginChallenge[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Salt,Challenge);
	struct UPnPDevice *device = Service->Parent;

 	if (ErrorCode == 0 && device) {

		// first convert device->UDN into GUID
		GUID deviceGUID;
		if (StringToGUID(device->UDN, deviceGUID)) {
			// compute authenticator
			vector<uint8> authenticator;
			bool res = g_devProt.CP_GetAuthenticator("Administrator", "admin password", * theCPGUID, deviceGUID,
									Salt, SaltLength,Challenge,ChallengeLength, authenticator);
			if (res) {
				CPInvoke_DeviceProtection_UserLogin(Service, &TestResponseSink_DeviceProtection_UserLogin,NULL,"PKCS5",
					Challenge, ChallengeLength, & authenticator[0], (int) authenticator.size());
			}
		} // if UDN contains a valid GUID
	} 
}

void Error1ResponseSink_DeviceProtection_GetUserLoginChallenge(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength)
{
	printf("CP Invoke Response: DeviceProtection/GetUserLoginChallenge[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Salt,Challenge);
	struct UPnPDevice *device = Service->Parent;

 	if (ErrorCode == 0 && device) {

		// first convert device->UDN into GUID
		GUID deviceGUID;
		if (StringToGUID(device->UDN, deviceGUID)) {
			// compute authenticator
			vector<uint8> authenticator;
			bool res = g_devProt.CP_GetAuthenticator("Administrator", "foobar", * theCPGUID, deviceGUID,
									Salt, SaltLength,Challenge,ChallengeLength, authenticator);
			if (res) { // try UserLogin() with an unknown user name "foo"
				CPInvoke_DeviceProtection_UserLogin(Service, &TestResponseSink_DeviceProtection_UserLogin,NULL,"foo",
					Challenge, ChallengeLength, & authenticator[0], (int) authenticator.size());
			}
		} // if UDN contains a valid GUID
	} 
}

void Error2ResponseSink_DeviceProtection_GetUserLoginChallenge(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength)
{
	printf("CP Invoke Response: DeviceProtection/GetUserLoginChallenge[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Salt,Challenge);
 	if (ErrorCode == 0) {
		CPInvoke_DeviceProtection_UserLogin(Service, &TestResponseSink_DeviceProtection_UserLogin,NULL,"PKCS5",
			Challenge, ChallengeLength, (unsigned char *) "foo", 3); // invalid authenticator
	} 
}
void Error3ResponseSink_DeviceProtection_GetUserLoginChallenge(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength)
{
	printf("CP Invoke Response: DeviceProtection/GetUserLoginChallenge[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Salt,Challenge);

 	if (ErrorCode == 0) {
		CPInvoke_DeviceProtection_UserLogin(Service, &TestResponseSink_DeviceProtection_UserLogin,NULL,"PKCS5",
			(unsigned char *) "foo", 3, (unsigned char *) "bar", 3); // invalid challenge and authenticator
	} 
}

	char * IdentityList = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<Identities xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">\
<User><Name>Tester</Name></User>\
</Identities>";

	char * IdTester = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<Identity xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">\
<User><Name>Tester</Name></User></Identity>";

	char * vendorX = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<Identity xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">\
<CP><ID>e593d8e6-6b8b-49d9-845a-21828db570e9</ID></CP></Identity>";

int ContinueDeviceProtectionTests(void)
{
	// At this point, the CP has been introduced to the Device, and its Role is set to "Public Basic" (note that it is
	// also permitted for the Role to be just "Basic").
	CPInvoke_DeviceProtection_GetAssignedRoles(theService, &TestResponseSink_DeviceProtection_GetAssignedRoles,NULL);

	CPInvoke_DeviceProtection_GetACLData(theService, &TestResponseSink_DeviceProtection_GetACLData,NULL);
	CPInvoke_DeviceProtection_AddIdentityList(theService, &TestResponseSink_DeviceProtection_AddIdentityList,NULL,IdentityList);
	CPInvoke_DeviceProtection_AddRolesForIdentity(theService, &TestResponseSink_DeviceProtection_AddRolesForIdentity,NULL,IdTester,"Basic");
	
	string UDN("uuid:");
	UDN += theDevice->UDN;
	CPInvoke_DeviceProtection_GetRolesForAction(theService, &TestResponseSink_DeviceProtection_GetRolesForAction,NULL, (char*) UDN.c_str(), theService->ServiceId, "SetUserLoginPassword");
	
	// Now test a few error conditions around the UserLogin functionality.  Some of these error conditions are coded into the
	// callback function of the response sink.
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &TestResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"foo","Administrator");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &TestResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","foo");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &Error1ResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","Administrator");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &Error2ResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","Administrator");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &Error3ResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","Administrator");

	// now try to log in successfully as Administrator
	//
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &TestResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","Administrator");

	return 1;
}

int ContinueDeviceProtectionTests2(void)
{
	// At this point, the CP should have successfully logged in.  Let's check if we now have the Admin Role.
	// If so, then we will be allowed to invoke all of the following protected actions.
	//
	CPInvoke_DeviceProtection_GetAssignedRoles(theService, &TestResponseSink_DeviceProtection_GetAssignedRoles,NULL);

	string UDN("uuid:");
	UDN += theDevice->UDN;
	CPInvoke_DeviceProtection_GetRolesForAction(theService, &TestResponseSink_DeviceProtection_GetRolesForAction,NULL, (char*) UDN.c_str(), theService->ServiceId, "SetUserLoginPassword");
	CPInvoke_DeviceProtection_GetACLData(theService, &TestResponseSink_DeviceProtection_GetACLData,NULL);

	CPInvoke_DeviceProtection_AddIdentityList(theService, &TestResponseSink_DeviceProtection_AddIdentityList,NULL,IdentityList);
	CPInvoke_DeviceProtection_SetUserLoginPassword(theService, &TestResponseSink_DeviceProtection_SetUserLoginPassword,NULL,"PKCS5","Tester",(unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_AddRolesForIdentity(theService, &TestResponseSink_DeviceProtection_AddRolesForIdentity,NULL,IdTester,"Admin");
	CPInvoke_DeviceProtection_RemoveRolesForIdentity(theService, &TestResponseSink_DeviceProtection_RemoveRolesForIdentity,NULL,IdTester,"Basic");
	CPInvoke_DeviceProtection_GetACLData(theService, &TestResponseSink_DeviceProtection_GetACLData,NULL);
	CPInvoke_DeviceProtection_RemoveIdentity(theService, &TestResponseSink_DeviceProtection_RemoveIdentity,NULL,IdTester);
	CPInvoke_DeviceProtection_GetACLData(theService, &TestResponseSink_DeviceProtection_GetACLData,NULL);
	CPInvoke_DeviceProtection_UserLogout(theService, &TestResponseSink_DeviceProtection_UserLogout,NULL);
	CPInvoke_DeviceProtection_GetAssignedRoles(theService, &TestResponseSink_DeviceProtection_GetAssignedRoles,NULL);

	return 1;
}


extern void WPSDeviceWasAuthenticated(const GUID & peerGUID) 
{
	string uuid = UUIDToString((void*)& peerGUID);
	printf("successfully authenticated Device with UUID: %s\n", uuid.c_str());
	if (! IsNameBasedUUID((void*) &peerGUID)) {
		printf("warning:  This GUID is not of type name-based.\n");
	}
	ContinueDeviceProtectionTests();
}

extern "C" int RunDeviceProtectionTests(struct UPnPDevice *device, GUID & CPGUID, WPSRegistrar * registrar, bool runSimpleTestsOnly )
{
	if (! device || ! registrar) {
		return 0;
	}
	g_RunAllTests = ! runSimpleTestsOnly;
	theService = CPGetService_DeviceProtection(device);
	if (! theService) {
		return 0;
	}
	theDevice = device;
	theCPGUID = & CPGUID;

	// First try to invoke all of the actions without being introduced to the Device.  Only the first three
	// actions and UserLogout() should be permitted at this point.
	// 
	// This same code can be invoked over HTTP, if the CP is invoked with TLS turned off.
	//

	if (g_RunAllTests) {
		CPInvoke_DeviceProtection_SendSetupMessage(theService, &TestResponseSink_DeviceProtection_SendSetupMessage,NULL,"WPS",(unsigned char *) "",0);
	}
	CPInvoke_DeviceProtection_GetSupportedProtocols(theService, &TestResponseSink_DeviceProtection_GetSupportedProtocols,NULL);
	CPInvoke_DeviceProtection_GetAssignedRoles(theService, &TestResponseSink_DeviceProtection_GetAssignedRoles,NULL);
	
	string UDN("uuid:");
	UDN += theDevice->UDN;
	CPInvoke_DeviceProtection_GetRolesForAction(theService, &TestResponseSink_DeviceProtection_GetRolesForAction,NULL, (char*) UDN.c_str(), theService->ServiceId, "SetUserLoginPassword");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(theService, &TestResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","foo");
	CPInvoke_DeviceProtection_UserLogin(theService, &TestResponseSink_DeviceProtection_UserLogin,NULL,"Sample String",(unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_UserLogout(theService, &TestResponseSink_DeviceProtection_UserLogout,NULL);
	CPInvoke_DeviceProtection_GetACLData(theService, &TestResponseSink_DeviceProtection_GetACLData,NULL);

	CPInvoke_DeviceProtection_AddIdentityList(theService, &TestResponseSink_DeviceProtection_AddIdentityList,NULL,IdentityList);
	CPInvoke_DeviceProtection_RemoveIdentity(theService, &TestResponseSink_DeviceProtection_RemoveIdentity,NULL,IdTester);
	CPInvoke_DeviceProtection_SetUserLoginPassword(theService, &TestResponseSink_DeviceProtection_SetUserLoginPassword,NULL,"PKCS5","Sample String",(unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_AddRolesForIdentity(theService, &TestResponseSink_DeviceProtection_AddRolesForIdentity,NULL,IdTester,"Basic");
	CPInvoke_DeviceProtection_RemoveRolesForIdentity(theService, &TestResponseSink_DeviceProtection_RemoveRolesForIdentity,NULL,IdTester,"Basic");

	if (g_RunAllTests) {
		// Try invoking SendSetupMessage() with an invalid protocol name
		CPInvoke_DeviceProtection_SendSetupMessage(theService, 
			&TestResponseSink_DeviceProtection_SendSetupMessage, registrar, "wps", (unsigned char *) "", 0);
	}

		// Now complete a WPS introduction to get the authorizations needed to finish the rest of the tests.
		//
		// The way this works is as follows: 
		// 1. Call StartWPS() to initialize, then call RunWPS() to initiate the WPS exchange over SendSetupMessage()
		//    first using an incorrect PIN (0) and then a correct PIN. Different callback functions are used for each
		//    of these cases so the logic can be distinguished.  Otherwise, there can be a race condition where 
		//    responses from the run of the protocol with the invalid PIN overlap with the subsequent run that uses the
		//    valid PIN. This can happen because the Device is not explicitly enforcing the restriction that only one
		//    instance of the protocol is allowed to run at a time.  Instead, what happens is the two instances
		//    collide with each other and neither completes successfully.  This should really be fixed. (TODO)
		// 2. Once the WPS exchange completes successfully, WPSDeviceWasAuthenticated() is called by
		//    WPSRegistrar::ProcessDone().  
		// 3. WPSDeviceWasAuthenticated() calls ContinueDeviceProtectionTests() to continue tests that require
		//    the CP to be authenticated.
	 
		// First, deliberately try to use an invalid PIN number.  After the error is detected, change back to the 
		// correct one and start over.

		//registrar->StartWPS(theService, 11111111 /* invalid PIN for WSC_DEVICEPWDID_DEFAULT */, WSC_DEVICEPWDID_DEFAULT, 
		//	                &ErrorResponseSink_DeviceProtection_SendSetupMessage);

	registrar->StartWPS(theService, g_Pin, g_Pin ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN, &TestResponseSink_DeviceProtection_SendSetupMessage);
	registrar->RunWPS();

	return 1;
}

