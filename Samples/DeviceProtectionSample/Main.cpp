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

#if defined(WIN32)
	#ifndef MICROSTACK_NO_STDAFX
		#include "stdafx.h"
	#endif
	#define _CRTDBG_MAP_ALLOC
	#include <TCHAR.h>
#endif

#if defined(WINSOCK2)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(WINSOCK1)
	#include <winsock.h>
	#include <wininet.h>
#endif

// Here are declarations of libraries and files with "C" linkage

extern "C" {
#include "ILibParsers.h"
#include "CPControlPoint.h"

#include "DVMicroStack.h"
#include "ILibWebServer.h"
#include "ILibAsyncSocket.h"

#if defined(WIN32)
	#include <crtdbg.h>
#endif

// #include "ErrorCodes.h"

#include "utils.h"

#include <openssl/pem.h>
//#include <openssl/err.h>
//#include <openssl/pkcs7.h>
//#include <openssl/pkcs12.h>
//#include <openssl/conf.h>
//#include <openssl/x509v3.h>
#include <openssl/engine.h>

BOOL g_USE_HTTPS = 1; // if g_USE_HTTPS == 0, then SSDP client will use LOCATION header rather than SECURELOCATION
bool g_verbose = false;
BOOL g_certToolMode = false;
}

// Here are declarations specific to C++

#include "WPSEnrollee.h"
#include "WPSRegistrar.h"
#include <algorithm>
#include <cctype>
#include "time.h"

// g_regProtocol is a singleton instance of a helper class used by both the WPSEnrollee and WPSRegistrar
// classes.  It maintains data structures and functiones related to running the WPS protocol for secure
// introduction.
CRegProtocol g_regProtocol;		

// Hard-coded PIN for testing purposes.  TODO: a real device should dynamically generate its PIN.
int g_Pin = 49226874; 

GUID DeviceGUID;
GUID CP_GUID;

WPSEnrollee * g_Enrollee = NULL;
WPSRegistrar * g_Registrar = NULL;

// The boolean flags below indicate whether the code runs as a Device, Control Point, or both
bool g_runDevice = true;
bool g_runCP = false;
bool g_runAsEnrollee = false;
bool g_runSimpleTests = false;



void RunProtectedSetup(struct UPnPService* peerService);
void CALLBACK ILib_IPAddressMonitor(IN DWORD dwError, IN DWORD cbTransferred, IN LPWSAOVERLAPPED lpOverlapped, IN DWORD dwFlags );

extern string UUIDToString( void * guid );
extern bool IsNameBasedUUID(void * guid);
extern bool format_uuid_v5(void *guid, unsigned char hash[16]);

// DPServerInfo encapsulates the DeviceProtection functionality:  data structures and functions used for
// DeviceProtection.
#include "DPServerInfo.h"

extern int TestDPServerInfo(DPServerInfo & rec); // Debug test function

extern "C" int RunDeviceProtectionTests(struct UPnPDevice *device, GUID & CPGUID, WPSRegistrar * registrar , bool runSimpleTestsOnly );

// g_devProt is an instance of the DPServerInfo class that is used by this Device.
extern DPServerInfo g_devProt("admin password");

extern "C" char g_serviceId[];
extern "C" SSL_CTX* g_client_ctx = NULL;
extern "C" SSL_CTX* g_server_ctx = NULL;

struct util_cert g_rootCert;
struct util_cert g_deviceCert;
struct util_cert g_CPCert;

char g_serviceId[128];
string g_deviceUDN;

GUID TokenToSession(DVSessionToken upnptoken)
{
	GUID localGUID;
	memset(&localGUID, 0, sizeof(localGUID));
	char * certhash = DVGetCertHashFromSessionToken(upnptoken);

	if (! certhash) {
		return localGUID;
	}
    // Now convert the cert hash into a name-based UUID
    format_uuid_v5(&localGUID, (unsigned char *) certhash); // Finished 

	Session * sess = g_devProt.GetSession(localGUID);
	if (! sess) {
		g_devProt.NewSession(localGUID);
		sess = g_devProt.GetSession(localGUID);
	}
	if (sess) {
		return sess->m_sessionId;
	}
	return localGUID;
}

static int verify_server_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
      // This code always returns 1, thus telling the OpenSSL stack to accept the certificate.
	  if (! preverify_ok) {
		  int err = X509_STORE_CTX_get_error(ctx);
          printf("verify error:num=%d:%s\n", err, X509_verify_cert_error_string(err));
	  }
      return 1;
}

static int verify_client_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
      // This code always returns 1, thus telling the OpenSSL stack to accept the certificate.
	  if (! preverify_ok) {
		  int err = X509_STORE_CTX_get_error(ctx);
          printf("verify error:num=%d:%s\n", err, X509_verify_cert_error_string(err));
	  }
	  // For debug purposes, look into the certificate to find the depth and subject name
	  char buf[256];
      X509   *err_cert = X509_STORE_CTX_get_current_cert(ctx);
      int depth = X509_STORE_CTX_get_error_depth(ctx);
      X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

      return 1;
}

extern "C" int OnSslConnection(ILibWebClient_StateObject sender, STACK_OF(X509) *certs, struct sockaddr_in6 *remoteInterface, void *user)
{
      UNREFERENCED_PARAMETER( sender );
      UNREFERENCED_PARAMETER( user );

	  printf("\n*** Control Point has established SSL connection\r\n");
      return 1; // Return 1 to accept, 0 to reject connection.
}

void *MicroStackChain;
void *CP_CP = NULL;
void *DVmicroStack;

HANDLE ILib_IPAddressMonitorTerminator;
HANDLE ILib_IPAddressMonitorThread;
DWORD ILib_MonitorSocketReserved;
WSAOVERLAPPED ILib_MonitorSocketStateObject;
SOCKET ILib_MonitorSocket;

void CPEventSink_DeviceProtection_SetupReady(struct UPnPService* Service,int SetupReady)
{
	printf("CP Event from %s/DeviceProtection/SetupReady: %d\r\n",Service->Parent->FriendlyName,SetupReady);
	if (g_Registrar->WaitingForPBCOnService(Service) && SetupReady) { 
		g_Registrar->RunWPS();
	}
}


#ifdef notdefined
This response sink code is disabled since its functionality is factored out into DPTest.cpp

void CPResponseSink_DeviceProtection_AddIdentityList(struct UPnPService* Service,int ErrorCode,void *User,char* IdentityListResult)
{
	printf("CP Invoke Response: DeviceProtection/AddIdentityList[ErrorCode:%d](%s)\r\n",ErrorCode,IdentityListResult);
}

void CPResponseSink_DeviceProtection_AddRolesForIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/AddRolesForIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_GetACLData(struct UPnPService* Service,int ErrorCode,void *User,char* ACL)
{
	printf("CP Invoke Response: DeviceProtection/GetACLData[ErrorCode:%d](%s)\r\n",ErrorCode,ACL);
}

void CPResponseSink_DeviceProtection_GetAssignedRoles(struct UPnPService* Service,int ErrorCode,void *User,char* RoleList)
{
	printf("CP Invoke Response: DeviceProtection/GetAssignedRoles[ErrorCode:%d](%s)\r\n",ErrorCode,RoleList);
}

void CPResponseSink_DeviceProtection_GetRolesForAction(struct UPnPService* Service,int ErrorCode,void *User,char* RoleList,char* RestrictedRoleList)
{
	printf("CP Invoke Response: DeviceProtection/GetRolesForAction[ErrorCode:%d](%s,%s)\r\n",ErrorCode,RoleList,RestrictedRoleList);
}

void CPResponseSink_DeviceProtection_GetSupportedProtocols(struct UPnPService* Service,int ErrorCode,void *User,char* ProtocolList)
{
	printf("CP Invoke Response: DeviceProtection/GetSupportedProtocols[ErrorCode:%d](%s)\r\n",ErrorCode,ProtocolList);
}

void CPResponseSink_DeviceProtection_RemoveIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/RemoveIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_RemoveRolesForIdentity(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/RemoveRolesForIdentity[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_SendSetupMessage(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength)
{
	printf("CP Invoke Response: DeviceProtection/SendSetupMessage[ErrorCode:%d](%s)\r\n",ErrorCode,OutMessage);
	if (ErrorCode == 0 && User) {
		WPSRegistrar * registrar = (WPSRegistrar *) User;
		registrar->SetBuffer(OutMessage, OutMessageLength);
		registrar->RunWPS();
	}
}

void CPResponseSink_DeviceProtection_SetUserLoginPassword(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/SetUserLoginPassword[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_UserLogin(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/UserLogin[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_UserLogout(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("CP Invoke Response: DeviceProtection/UserLogout[ErrorCode:%d]()\r\n",ErrorCode);
}

void CPResponseSink_DeviceProtection_GetUserLoginChallenge(struct UPnPService* Service,int ErrorCode,void *User,unsigned char* Salt,int SaltLength,unsigned char* Challenge,int ChallengeLength)
{
	printf("CP Invoke Response: DeviceProtection/GetUserLoginChallenge[ErrorCode:%d](%s,%s)\r\n",ErrorCode,Salt,Challenge);
	if (ErrorCode == 0) {

		// compute authenticator
		vector<uint8> authenticator;
		bool res = g_devProt.CP_GetAuthenticator("Administrator", "admin password", CP_GUID, DeviceGUID,
								Salt, SaltLength,Challenge,ChallengeLength, authenticator);
		if (res) {
			CPInvoke_DeviceProtection_UserLogin(Service, &CPResponseSink_DeviceProtection_UserLogin,NULL,"PKCS5",
				Challenge, ChallengeLength, & authenticator[0], (int) authenticator.size());
		}
	}
}
#endif

extern bool StringToGUID(char * uuid, GUID & peerGUID) 
{
	if (strnicmp("uuid:",uuid,5) != 0 && strlen(uuid) == 22) {
		uuid += 5; // skip over uuid: prefix
	}
	int temp[8];
	int num = sscanf(uuid,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			&peerGUID.Data1,
			&peerGUID.Data2,
			&peerGUID.Data3,
			&temp[0],
			&temp[1],
			&temp[2],
			&temp[3],
			&temp[4],
			&temp[5],
			&temp[6],
			&temp[7]);
	// For consistency with RFC 4122, keep the UUID
	// in network byte order.
	peerGUID.Data1 = htonl(peerGUID.Data1);
	peerGUID.Data2 = htons(peerGUID.Data2);
	peerGUID.Data3 = htons(peerGUID.Data3);
	if (num == 11) { // copy from the ints into the char array one by one (sscanf can't write to individual char directly)
		for (int i = 0; i < 8; i++) {
			peerGUID.Data4[i] = temp[i];
		}
	}
	return (num == 11);
}

extern "C" void CPPrintUPnPDevice(int indents, struct UPnPDevice *device);

/* Called whenever a new device on the correct type is discovered */
void CPDeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;

	printf("UPnPDevice Added: %s %s\r\n", device->FriendlyName, device->UDN);

	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	// CPPrintUPnPDevice(0,device);

	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			CPSubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}

	// run the DeviceProtection tests only if this is not our device.
	if (g_Registrar && (g_deviceUDN.length() == 0 || g_deviceUDN.substr(5,36) != device->UDN)) { 
		RunDeviceProtectionTests(device, CP_GUID, g_Registrar, g_runSimpleTests);
	}

#ifdef notdefined
Disable this code since the control point logic has been factored out into the RunDeviceProtectionTests()
function in file DPTest.cpp.

	// The following will call every method of every service in the device with sample values 
	// You can cut & paste these lines where needed. The user value is NULL, it can be freely used 
	// to pass state information. 
	// The DVGetService call can return NULL, a correct application must check this since a device 
	// can be implemented without some services. 
	
	// You can check for the existence of an action by calling: DVHasAction(serviceStruct,serviceType) 
	// where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing
	// the service urn. 

	tempService = CPGetService_DeviceProtection(device);

	string UDN("uuid:");
	UDN += device->UDN;
	CPInvoke_DeviceProtection_SendSetupMessage(tempService, &CPResponseSink_DeviceProtection_SendSetupMessage,NULL,"WPS",(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_GetSupportedProtocols(tempService, &CPResponseSink_DeviceProtection_GetSupportedProtocols,NULL);
	CPInvoke_DeviceProtection_GetAssignedRoles(tempService, &CPResponseSink_DeviceProtection_GetAssignedRoles,NULL);
	CPInvoke_DeviceProtection_GetRolesForAction(tempService, &CPResponseSink_DeviceProtection_GetRolesForAction,NULL, (char*) UDN.cstr(), tempService->ServiceId, "GetRolesForAction");
	CPInvoke_DeviceProtection_GetRolesForAction(tempService, &CPResponseSink_DeviceProtection_GetRolesForAction,NULL, (char*) UDN.cstr(), tempService->ServiceId, "SetUserLoginPassword");
	CPInvoke_DeviceProtection_GetUserLoginChallenge(tempService, &CPResponseSink_DeviceProtection_GetUserLoginChallenge,NULL,"PKCS5","foo");
	CPInvoke_DeviceProtection_UserLogin(tempService, &CPResponseSink_DeviceProtection_UserLogin,NULL,"Sample String",(unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_UserLogout(tempService, &CPResponseSink_DeviceProtection_UserLogout,NULL);
	CPInvoke_DeviceProtection_GetACLData(tempService, &CPResponseSink_DeviceProtection_GetACLData,NULL);

	CPInvoke_DeviceProtection_AddIdentityList(tempService, &CPResponseSink_DeviceProtection_AddIdentityList,NULL,IdentityList);
	CPInvoke_DeviceProtection_RemoveIdentity(tempService, &CPResponseSink_DeviceProtection_RemoveIdentity,NULL,IdTester);
	CPInvoke_DeviceProtection_SetUserLoginPassword(tempService, &CPResponseSink_DeviceProtection_SetUserLoginPassword,NULL,"PKCS5","Sample String",(unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
	CPInvoke_DeviceProtection_AddRolesForIdentity(tempService, &CPResponseSink_DeviceProtection_AddRolesForIdentity,NULL,IdTester,"Basic");
	CPInvoke_DeviceProtection_RemoveRolesForIdentity(tempService, &CPResponseSink_DeviceProtection_RemoveRolesForIdentity,NULL,IdTester,"Basic");
#endif
}

/* Called whenever a discovered device was removed from the network */
void CPDeviceRemoveSink(struct UPnPDevice *device)
{
	printf("UPnPDevice Removed: %s\r\n", device->FriendlyName);
}

// Note:  this helper function should probably be moved into the DPServerInfo class
bool CPOfSessionIsKnown(DVSessionToken upnptoken)
{
	// return true; // DEBUG FOR NOW
	char * certhash = DVGetCertHashFromSessionToken(upnptoken);
	if (!certhash) {
		return false;
	}
    // Now check that the CP is already known
	GUID peerGUID;
	memset(&peerGUID, 0, sizeof(peerGUID));
    format_uuid_v5(&peerGUID, (unsigned char *) certhash);  
	string peerId = UUIDToString((void*)& peerGUID); 
	CPIdentity * id = g_devProt.FindCPbyID(peerId);
	if (id ) { // Identity is already known
		return true;
	}
	return false;
}

// The section below contains Device code implementing the DeviceProtection service.
//

void DVDeviceProtection_AddIdentityList(DVSessionToken upnptoken,char* IdentityList)
{
	printf("Invoke: DVDeviceProtection_AddIdentityList(%s);\r\n",IdentityList);
	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "AddIdentityList");
	if (!result) {
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}	
	g_devProt.AddIdentityList(IdentityList);

	string s = g_devProt.GetIdentityList();

	DVResponse_DeviceProtection_AddIdentityList(upnptoken,s.c_str());
}

void DVDeviceProtection_AddRolesForIdentity(DVSessionToken upnptoken,char* Identity,char* RoleList)
{
	printf("Invoke: DVDeviceProtection_AddRolesForIdentity(%s,%s);\r\n",Identity,RoleList);
	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "AddRolesForIdentity");
	if (!result) {
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}

	bool res = g_devProt.AddRolesForIdentity( Identity, RoleList );
	if (res == false) {
		DVResponse_Error(upnptoken,600,"Argument value invalid");
		return;
	}
	
	DVResponse_DeviceProtection_AddRolesForIdentity(upnptoken); // success
}


void DVDeviceProtection_GetACLData(DVSessionToken upnptoken)
{
	printf("Invoke: DVDeviceProtection_GetACLData();\r\n");

	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "GetACLData");
	if (!result && !CPOfSessionIsKnown(upnptoken)) { // unknown CP
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	string s = g_devProt.GetACL();
	
	DVResponse_DeviceProtection_GetACLData(upnptoken,s.c_str());
}

void DVDeviceProtection_GetAssignedRoles(DVSessionToken upnptoken)
{
	printf("Invoke: DVDeviceProtection_GetAssignedRoles();\r\n");

	string s = g_devProt.GetRolesOfSession(TokenToSession(upnptoken));
	
	DVResponse_DeviceProtection_GetAssignedRoles(upnptoken,s.c_str());
}

void DVDeviceProtection_GetRolesForAction(DVSessionToken upnptoken,char* DeviceUDN,char* ServiceId,char* ActionName)
{
	printf("Invoke: DVDeviceProtection_GetRolesForAction(%s,%s,%s);\r\n",DeviceUDN,ServiceId,ActionName);

	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "GetRolesForAction");
	if (!result && !CPOfSessionIsKnown(upnptoken)) { // allow Public if CP is known
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	string mainRoleList = g_devProt.GetPrimaryPolicy( DeviceUDN, ServiceId, ActionName );
	string restrictedRoleList = g_devProt.GetRestrictedPolicy( DeviceUDN, ServiceId, ActionName );

	if (mainRoleList == "" && restrictedRoleList == "") { // no policy found, so assume Public
		mainRoleList = "Public";
	} 
	DVResponse_DeviceProtection_GetRolesForAction(upnptoken,mainRoleList.c_str(),restrictedRoleList.c_str());
}

void DVDeviceProtection_GetSupportedProtocols(DVSessionToken upnptoken)
{
	printf("Invoke: DVDeviceProtection_GetSupportedProtocols();\r\n");
	string s = g_devProt.GetSupportedProtocols();

	DVResponse_DeviceProtection_GetSupportedProtocols(upnptoken,s.c_str());
}

// Note:  some of the GetUserLoginChallenge logic should be moved into the DPServerInfo class.
//
void DVDeviceProtection_GetUserLoginChallenge(DVSessionToken upnptoken,char* ProtocolType,char* Name)
{
	printf("Invoke: DVDeviceProtection_GetUserLoginChallenge(%s,%s);\r\n",ProtocolType,Name);
	
	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "GetUserLoginChallenge");
	if (!result && !CPOfSessionIsKnown(upnptoken)) { // unknown CP
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	if (strcmp("PKCS5",ProtocolType) == 0) {
		// first check the Roles of the requested user and only allow logging in as a user with Admin
		// rights if the CP already has Basic or Admin rights.
		bool userIsAdmin = false;
		UserIdentity * theUser = g_devProt.FindUser(Name);
		if (!theUser) {
			DVResponse_Error(upnptoken,600,"Invalid argument");
			return;		
		}
		stringstream tempRoles(theUser->m_roles); 
		string buf; // temp variable to extract one Role at a time
		while (tempRoles >> buf) {
			if ( buf == "Admin") {
				userIsAdmin = true;		
			}
		}
		if (userIsAdmin) {
			bool CPisAdminOrBasic = false; // now check this condition...
			string s = g_devProt.GetRolesOfSession(TokenToSession(upnptoken));
			stringstream sessionRoles(s); 
			while (sessionRoles >> buf) {
				if ( buf == "Admin" || buf == "Basic") {
					CPisAdminOrBasic = true;		
				}
			}
			if (! CPisAdminOrBasic) { // CP is not authorized
				DVResponse_Error(upnptoken,606,"Action not authorized");
				return;		
			}
		}
		
		vector<uint8> salt;		
		vector<uint8> challenge;
		bool result = g_devProt.GetUserLoginChallenge(TokenToSession(upnptoken),"PKCS5", Name,salt,challenge);
		if (! result) { // Name is probably unknown
			DVResponse_Error(upnptoken,600,"Invalid argument");
			return;		
		}
		if (salt.size() != SIZE_128_BITS || challenge.size() != SIZE_128_BITS) {
			DVResponse_Error(upnptoken,600,"Internal error");
			return;
		} else {
			DVResponse_DeviceProtection_GetUserLoginChallenge(upnptoken,
				& salt[0],(int) salt.size(),& challenge[0], (int) challenge.size());
				// (unsigned char *) "Sample Binary",13,(unsigned char *) "Sample Binary",13);
		}
	} else {
		DVResponse_Error(upnptoken,600,"Invalid ProtocolType or Name");
		return;
	}
}

void DVDeviceProtection_RemoveIdentity(DVSessionToken upnptoken,char* Identity)
{
	printf("Invoke: DVDeviceProtection_RemoveIdentity(%s);\r\n",Identity);
	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "RemoveIdentity");
	if (!result) {
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}	
	result = g_devProt.RemoveIdentity(Identity);
	if (!result) {
		DVResponse_Error(upnptoken,600,"Invalid argument");
		return;	
	}
	DVResponse_DeviceProtection_RemoveIdentity(upnptoken);
}


void DVDeviceProtection_RemoveRolesForIdentity(DVSessionToken upnptoken,char* Identity,char* RoleList)
{
	printf("Invoke: DVDeviceProtection_RemoveRolesForIdentity(%s,%s);\r\n",Identity,RoleList);

	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "RemoveRolesForIdentity");
	if (!result) {
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	bool res = g_devProt.RemoveRolesForIdentity(Identity, RoleList);
	if (res == false) {
		DVResponse_Error(upnptoken,600,"Argument value invalid");
		return;
	}
	DVResponse_DeviceProtection_RemoveRolesForIdentity(upnptoken);
}

void DVDeviceProtection_SendSetupMessage(DVSessionToken upnptoken,char* ProtocolType,unsigned char* InMessage,int _InMessageLength)
{
	printf("Invoke: DVDeviceProtection_SendSetupMessage(%s,BINARY(%d));\r\n",ProtocolType,_InMessageLength);
	
	char * certhash = DVGetCertHashFromSessionToken(upnptoken);
	if (!certhash) {
		DVResponse_Error(upnptoken,606,"Not invoked over TLS");
		return;
	}
	if (strcmp(ProtocolType,"WPS") == 0) {
		if (g_Enrollee) {
			g_Enrollee->SetBuffer(upnptoken, InMessage, _InMessageLength);

			// Note:  the DVResponse message is sent by the RunWPS() method. 
			g_Enrollee->RunWPS();
		} else {
			DVResponse_Error(upnptoken,702,"Internal Error"); 
		}
	} else {
		DVResponse_Error(upnptoken,600,"Unknown ProtocolType");
	}
}


void DVDeviceProtection_SetUserLoginPassword(DVSessionToken upnptoken,char* ProtocolType,char* Name,unsigned char* Stored,int _StoredLength,unsigned char* Salt,int _SaltLength)
{
	printf("Invoke: DVDeviceProtection_SetUserLoginPassword(%s,BINARY(%d),BINARY(%d));\r\n",Name,_StoredLength,_SaltLength);

/*/ test code here
	GUID localGUID;
	char * certhash = DVGetCertHashFromSessionToken(upnptoken);
	if (certhash) {
		format_uuid_v5(&localGUID, (unsigned char *) certhash); // Convert the cert hash into a name-based UUID
	} else {
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	char * certName = DVGetCertNameFromSessionToken(upnptoken);
	int sportname = DVGetSecurePortNumber(upnptoken);
// test code here
*/

	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "SetUserLoginPassword");
	if (!result) { // not authorized based on Role of primary (unconditional) policy
		string s = g_devProt.GetUserOfSession(TokenToSession(upnptoken));
		if (s.length() == 0) {  // user name not found in session
			DVResponse_Error(upnptoken,600,"Invalid arguments");
			return;
		} 
		if (s != Name) { 
			DVResponse_Error(upnptoken,606,"Action not authorized");
			return;
		} // else, caller is currently logged in as Name, so allow changing the password.
	}

	if (strcmp(ProtocolType,"PKCS5") != 0 || _StoredLength != SIZE_128_BITS || _SaltLength != SIZE_128_BITS ) {
		DVResponse_Error(upnptoken,600,"Invalid arguments");
		return;	
	}
	result = g_devProt.SetUserLoginPassword(Name, Stored, _StoredLength, Salt, _SaltLength);
	if (!result) { // unrecognized Name
		DVResponse_Error(upnptoken,600,"Invalid arguments");
		return;	
	}
	DVResponse_DeviceProtection_SetUserLoginPassword(upnptoken);
}

void DVDeviceProtection_UserLogin(DVSessionToken upnptoken,char* ProtocolType,unsigned char* Challenge,int _ChallengeLength,unsigned char* Authenticator,int _AuthenticatorLength)
{
	printf("Invoke: DVDeviceProtection_UserLogin(%s,BINARY(%d),BINARY(%d));\r\n",ProtocolType,_ChallengeLength,_AuthenticatorLength);

	bool result = g_devProt.CheckAuthorization(TokenToSession(upnptoken), g_deviceUDN, g_serviceId, "UserLogin");
	if (!result && !CPOfSessionIsKnown(upnptoken)) {  // unknown user
		DVResponse_Error(upnptoken,606,"Action not authorized");
		return;
	}
	if (strcmp(ProtocolType,"PKCS5") != 0 || _ChallengeLength != SIZE_128_BITS ) {
		DVResponse_Error(upnptoken,600,"Invalid argument");
		return;	
	}	
	if (_AuthenticatorLength != SIZE_128_BITS ) {
		DVResponse_Error(upnptoken,701,"Invalid Authenticator");
		return;	// this could be considered as an invalid argument as well... 
	}
	// get the CP's identity, which is needed to verify the login operation
	char * certhash = DVGetCertHashFromSessionToken(upnptoken);
	GUID peerGUID;
	memset(&peerGUID, 0, sizeof(peerGUID));
    format_uuid_v5(&peerGUID, (unsigned char *) certhash);  

	result = g_devProt.UserLogin(TokenToSession(upnptoken), peerGUID, DeviceGUID, Challenge, _ChallengeLength, Authenticator, _AuthenticatorLength);
	if (!result) { // unrecognized Name
		DVResponse_Error(upnptoken,701,"Login Failure");
		return;	
	}
	// TODO: set timer here to expire login automatically according to device policy.  This may apply only to 
	// Administrator user, for example.  Call g_devProt.GetUserOfSession() to see which user is logged in.

	DVResponse_DeviceProtection_UserLogin(upnptoken);
}

void DVDeviceProtection_UserLogout(DVSessionToken upnptoken)
{
	printf("Invoke: DVDeviceProtection_UserLogout();\r\n");
	g_devProt.UserLogout(TokenToSession(upnptoken));
	DVResponse_DeviceProtection_UserLogout(upnptoken);
}

DWORD WINAPI ILib_IPAddressMonitorLoop(LPVOID args)
{
	ILib_MonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(ILib_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&ILib_MonitorSocketReserved,&ILib_MonitorSocketStateObject,&ILib_IPAddressMonitor);
	while(WaitForSingleObjectEx(ILib_IPAddressMonitorTerminator,INFINITE,TRUE)!=WAIT_OBJECT_0);
	return 0;
}

bool bTimeToExit = false;

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
      printf( "Ctrl-C event\n\n" );
	  bTimeToExit = true;
      return( TRUE );
 
    // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT: 
      printf( "Ctrl-Close event\n\n" );
	  bTimeToExit = true;
	  return( TRUE ); 
 
    // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT:  
      printf( "Ctrl-Break event\n\n" );
      return FALSE; 
 
    default: 
      return FALSE; 
  } 
} 

DWORD WINAPI Run(LPVOID args)
{
	BOOL sch = SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE );

	while (! bTimeToExit) { // vbl put this here to avoid eating up stdin characters...
		Sleep(100);
		if (g_Enrollee->PBCVerificationNeeded()) {
			g_Enrollee->DoPBCVerification();
		}
		if (g_Registrar->WaitingForPBC( ) && 
			(g_regProtocol.SecondsSinceCPPBCButtonPress() > g_regProtocol.pbc_walk_time) ) {
			g_Registrar->PBCTimeout(); // Timeout... stop waiting for Device's button press
		}
	}
	
	closesocket(ILib_MonitorSocket);
	SetEvent(ILib_IPAddressMonitorTerminator);
	WaitForSingleObject(ILib_IPAddressMonitorThread,INFINITE);
	CloseHandle(ILib_IPAddressMonitorTerminator);
	
	ILibStopChain(MicroStackChain);
	
	return 0;
}

/*
void GetPermission()
{
	bool result = g_devProt.CheckAuthorization(g_SSLSession, g_deviceUDN, g_serviceId, "AddRolesForIdentity");
	if (!result) {  // Log in as Administrator to get Admin privileges
		vector<uint8> salt;
		vector<uint8> challenge;
		result = g_devProt.GetUserLoginChallenge( g_SSLSession, "administrator", salt, challenge);
		vector<uint8> authenticator;
		result = g_devProt.CP_GetAuthenticator("Administrator", "admin password",
										 &salt[0], salt.size(), &challenge[0], challenge.size(), authenticator);
		result = g_devProt.UserLogin( g_SSLSession, &challenge[0], challenge.size(), &authenticator[0], authenticator.size());
		result = g_devProt.CheckAuthorization(g_SSLSession, g_deviceUDN, g_serviceId, "AddRolesForIdentity");
	}
}
*/



string UUIDToIdentity(void * uuid)
{
	string id = UUIDToString(uuid);
	char * vendorX = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<Identity xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \
xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">\
<CP><ID>%s</ID></CP></Identity>";
	char * buf = (char *) malloc(strlen(vendorX) + 80);
	sprintf(buf, vendorX, id.c_str());
	string ret = buf;
	free(buf);
	return ret;
}


int _tmain(int argc, _TCHAR* argv[]) 
{ 
	DWORD ptid=0;
	DWORD ptid2=0;

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	
	MicroStackChain = ILibCreateChain();

	g_runDevice = true;
	g_runCP = true;
	while (argc > 1) {
		if (strncasecmp(argv[1],"verbose",7) == 0) {
			g_verbose = true;
		}
		if (strncasecmp(argv[1],"http_only",9) == 0) {
			g_USE_HTTPS = 0;
		}
		if (strncasecmp(argv[1],"label_pin",9) == 0) {
			g_regProtocol.SetUseStaticPIN(true);
		}	
		if (strncasecmp(argv[1],"simple_test",11) == 0) {
			g_runSimpleTests = true;
		}
		if (strncasecmp(argv[1],"cert_tool_mode",14) == 0) {
			printf("Cert tool mode\n");
			g_runDevice = true;
			g_runCP = false;			
			g_certToolMode = true;
		}		
		if (strncasecmp(argv[1],"pbc",3) == 0) {
			g_Pin = 0; // use this value and UseStaticPIN to indicate PBC method
			g_regProtocol.SetUseStaticPIN(true);		
			g_regProtocol.DevPBCButtonWasPressed(); // Startup with pbc option counts as a button press.
			g_regProtocol.CPPBCButtonWasPressed(); // Also counts as a button press for CP side.
		}
		if (strncasecmp(argv[1],"dev",3) == 0) {
			printf("Device only\n");
			g_runDevice = true;
			g_runCP = false;
		} 		
		if (strncasecmp(argv[1],"cp",2) == 0) {
			printf("Control point only\n");
			g_runDevice = false;
			g_runCP = true;
		} 
		if (strncasecmp(argv[1],"both",4) == 0) {
			g_runDevice = true;
			g_runCP = true;
		} 
		argc--;
		argv++; // skip to next command line parameter
	}

	util_openssl_init();

	// TODO:  implement cert cache in files to speed startup time. 

	int l = util_mkCert(NULL, &g_rootCert, 2048, 10000, "MyLocalCA@UPnP.intel.com", CERTIFICATE_ROOT);
	if (l) {
		//util_printcert(g_rootCert);
		l = util_mkCert(&g_rootCert, &g_deviceCert, 2048, 10000, "MyDevice@UPnP.intel.com", CERTIFICATE_TLS_SERVER);
		//if (l) util_printcert(g_deviceCert);
		l = util_mkCert(&g_rootCert, &g_CPCert, 2048, 10000, "MyCP@UPnP.intel.com", CERTIFICATE_TLS_CLIENT);
		//if (l) util_printcert(g_CPCert);
	}

	if (g_USE_HTTPS) {
		// Create TLS contexts
		g_client_ctx = SSL_CTX_new(TLSv1_client_method());  // restrict to TLS v1 only
		g_server_ctx = SSL_CTX_new(TLSv1_server_method()); 

		// Server side settings

		l = SSL_CTX_use_certificate(g_server_ctx, g_deviceCert.x509);
		l = SSL_CTX_use_PrivateKey(g_server_ctx, g_deviceCert.pkey);
		l = SSL_CTX_add_extra_chain_cert(g_server_ctx, X509_dup(g_rootCert.x509));
		SSL_CTX_set_verify_depth(g_server_ctx, 2);  
		SSL_CTX_set_verify(g_server_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_server_callback); // Ask for client authentication
		// SSL_CTX_set_verify(g_server_ctx, SSL_VERIFY_PEER, verify_server_callback); // Ask for client authentication

		// Client side settings
		l = SSL_CTX_use_certificate(g_client_ctx, g_CPCert.x509);
		l = SSL_CTX_use_PrivateKey(g_client_ctx, g_CPCert.pkey);
		l = SSL_CTX_add_extra_chain_cert(g_client_ctx, X509_dup(g_rootCert.x509));
		SSL_CTX_set_verify_depth(g_client_ctx, 2);
		SSL_CTX_set_verify(g_client_ctx, SSL_VERIFY_PEER, verify_client_callback); // Ask for server authentication
	}

	memset(&CP_GUID,0,sizeof(GUID));

	// Declare and initialize structures for WPS here.  Keep them at main scope so they don't get
	// destructed prematurely before the main program exits.

	X509 * cert = g_CPCert.x509;
    unsigned char * certbuf = NULL; // NULL => library allocates memory
    int certlen = i2d_X509(cert, &certbuf); // get DER encoding for hash

	// compute hash for Registrar's ID
    unsigned char hash[SHA256_DIGEST_LENGTH];
	memset(hash, '\0', sizeof(hash));
	util_sha256((char*) certbuf, certlen, (char *) &hash);

	// Remove SHA1-based hash and use SHA-256
    //SHA_CTX ctx;
    //unsigned char hash[SHA_DIGEST_LENGTH];
	//memset(hash, '\0', sizeof(hash));
	//SHA1_Init(&ctx);
    //SHA1_Update(&ctx, certbuf, certlen);
    //SHA1_Final(hash, &ctx); // Finish computing hash of cert

    // Note that we consider the cert hash to be the “name space ID”
    // and this hash is unique, so there is no need to also include a 
    // name in the hash.

    // Now convert the cert hash into a name-based UUID
    format_uuid_v5(&CP_GUID, hash); // Finished 
	string scert = UUIDToString(&CP_GUID);
	printf("Registrar ID is %s\n",scert.c_str());

	// TODO: also free certbuf...

	// StringToGUID("2ebd9588-645d-4672-b6a6-102360b5294f",CP_GUID);
	BufferObj nullPassword;

	WPSRegistrar theRegistrar(CP_GUID, g_Pin, g_Pin ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN, nullPassword);
	g_Registrar = &theRegistrar;

	// Now do the same for the Device
	cert = g_deviceCert.x509;
    certbuf = NULL; // NULL => library allocates memory
    certlen = i2d_X509(cert, &certbuf); // get DER encoding for hash
	memset(hash, '\0', sizeof(hash));

	// Remove SHA1-based hash and use SHA-256
	//SHA1_Init(&ctx);
    //SHA1_Update(&ctx, certbuf, certlen);
    //SHA1_Final(hash, &ctx); // Finish computing hash of cert

	util_sha256((char*) certbuf, certlen, (char *) &hash); // compute the certificate's hash

    // Now convert the cert hash into a name-based UUID
    format_uuid_v5(&DeviceGUID, hash); // Finished 

	WPSEnrollee theEnrollee(DeviceGUID, g_Pin, g_Pin ? WSC_DEVICEPWDID_DEFAULT : WSC_DEVICEPWDID_PUSH_BTN, nullPassword);
	g_Enrollee = &theEnrollee;

	if (g_runCP) {
		CP_CP = CPCreateControlPoint(MicroStackChain,&CPDeviceDiscoverSink,&CPDeviceRemoveSink);
	}

	// Initialize the serviceId variable.  It is important that this value match the service ID
	// advertised in the DDD (see DVMicroStack.c) in the call to DVCreateMicroStack().  However,
	// this UPnP Stack does not include a way to programmatically determine the service ID without
	// parsing the DDD XML itself.  Therefore, the value is hard-coded here.  If the service ID used
	// to generate the stack changes, the value here must be changed as well.  Otherwise, the service ID
	// used by the Control Point when querying the Device's DeviceProtection policy will not match.
	//
	strcpy(g_serviceId,"urn:upnp-org:serviceId:DeviceProtection");

	if (g_runDevice) {

		g_deviceUDN = UUIDToString(&DeviceGUID);

		if (g_certToolMode) {
			g_deviceUDN = "c57ef22b-74f1-5f05-9754-c5ec1a543955";
			// Hard-code 60-second notify interval and arbitrary fixed port number 49159 for the sake of the Cert Tool.
			DVmicroStack = DVCreateMicroStack(MicroStackChain,"Sample Protected Device",g_deviceUDN.c_str(),"0000001",60,49159);
		} else {
			DVmicroStack = DVCreateMicroStack(MicroStackChain,"Sample Protected Device",g_deviceUDN.c_str(),"0000001",1800,0);
		}
		printf("Starting Device with UDN %s\n", g_deviceUDN.c_str());

		g_deviceUDN = "uuid:" + g_deviceUDN;

		// Now establish the access control policy for the DeviceProtection service.  Unless otherwise indicated here, it is 
		// assumed that Public is authorized.
		//
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "GetRolesForAction", "Basic Admin", "Public");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "GetUserLoginChallenge", "Basic Admin", "Public");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "UserLogin", "Basic Admin", "Public");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "GetACLData", "Basic Admin", "Public");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "AddIdentityList", "Basic Admin","");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "RemoveIdentity", "Admin","");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "SetUserLoginPassword", "Admin", "Basic");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "AddRolesForIdentity", "Admin","");
		g_devProt.SetPolicy(g_deviceUDN, g_serviceId, "RemoveRolesForIdentity", "Admin","");

		vector<uint8> storedPassword;
		vector<uint8> salt;
		bool res = g_devProt.ComputeStoredPassword( "RandomUser", "foobar", salt, true, storedPassword);
		string buf("Basic"); 
		if (res) {
			g_devProt.AddUser("RandomUser", salt, storedPassword, buf);
		}

		//TestDPServerInfo(g_devProt);

		// GetPermission();
		
		// Set the device action handler pointers
		DVFP_DeviceProtection_AddIdentityList=(DV_ActionHandler_DeviceProtection_AddIdentityList)&DVDeviceProtection_AddIdentityList;
		DVFP_DeviceProtection_AddRolesForIdentity=(DV_ActionHandler_DeviceProtection_AddRolesForIdentity)&DVDeviceProtection_AddRolesForIdentity;
		DVFP_DeviceProtection_GetACLData=(DV_ActionHandler_DeviceProtection_GetACLData)&DVDeviceProtection_GetACLData;
		DVFP_DeviceProtection_GetAssignedRoles=(DV_ActionHandler_DeviceProtection_GetAssignedRoles)&DVDeviceProtection_GetAssignedRoles;
		DVFP_DeviceProtection_GetRolesForAction=(DV_ActionHandler_DeviceProtection_GetRolesForAction)&DVDeviceProtection_GetRolesForAction;
		DVFP_DeviceProtection_GetSupportedProtocols=(DV_ActionHandler_DeviceProtection_GetSupportedProtocols)&DVDeviceProtection_GetSupportedProtocols;
		DVFP_DeviceProtection_GetUserLoginChallenge=(DV_ActionHandler_DeviceProtection_GetUserLoginChallenge)&DVDeviceProtection_GetUserLoginChallenge;
		DVFP_DeviceProtection_RemoveIdentity=(DV_ActionHandler_DeviceProtection_RemoveIdentity)&DVDeviceProtection_RemoveIdentity;
		DVFP_DeviceProtection_RemoveRolesForIdentity=(DV_ActionHandler_DeviceProtection_RemoveRolesForIdentity)&DVDeviceProtection_RemoveRolesForIdentity;
		DVFP_DeviceProtection_SendSetupMessage=(DV_ActionHandler_DeviceProtection_SendSetupMessage)&DVDeviceProtection_SendSetupMessage;
		DVFP_DeviceProtection_SetUserLoginPassword=(DV_ActionHandler_DeviceProtection_SetUserLoginPassword)&DVDeviceProtection_SetUserLoginPassword;
		DVFP_DeviceProtection_UserLogin=(DV_ActionHandler_DeviceProtection_UserLogin)&DVDeviceProtection_UserLogin;
		DVFP_DeviceProtection_UserLogout=(DV_ActionHandler_DeviceProtection_UserLogout)&DVDeviceProtection_UserLogout;
		
		/* All evented state variables MUST be initialized before DVStart is called. */
		//
		DVSetState_DeviceProtection_SetupReady(DVmicroStack,1);
		
		printf("Intel MicroStack 1.0 - Sample Protected Device,\r\n\r\n");
	}

	CreateThread(NULL,0,&Run,NULL,0,&ptid);

	if (g_runCP) {
		CPEventCallback_DeviceProtection_SetupReady=&CPEventSink_DeviceProtection_SetupReady;
	}
	
	ILib_IPAddressMonitorTerminator = CreateEvent(NULL,TRUE,FALSE,NULL);
	ILib_IPAddressMonitorThread = CreateThread(NULL,0,&ILib_IPAddressMonitorLoop,NULL,0,&ptid2);
	
	ILibStartChain(MicroStackChain);

	util_freecert(&g_deviceCert);
	util_freecert(&g_rootCert);
	util_freecert(&g_CPCert);

	void  util_openssl_uninit();
	
	return 0;
}

void CALLBACK ILib_IPAddressMonitor(
IN DWORD dwError, 
IN DWORD cbTransferred, 
IN LPWSAOVERLAPPED lpOverlapped, 
IN DWORD dwFlags 
)
{
	CP_CP_IPAddressListChanged(CP_CP);
	DVIPAddressListChanged(DVmicroStack);
	
	WSAIoctl(ILib_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&ILib_MonitorSocketReserved,&ILib_MonitorSocketStateObject,&ILib_IPAddressMonitor);
}

extern void SendWPSResponse(void * upnptoken, BufferObj & outBuf)
{
	DVResponse_DeviceProtection_SendSetupMessage(upnptoken,outBuf.GetBuf(),outBuf.Length());
}


extern void SendWPSResponseError(void * upnptoken, int code, const char * mess)
{
	DVResponse_Error(upnptoken,code,mess);
}

extern void WPS_CPWasAuthenticated(void * upnptoken, const GUID & peerGUID) 
{
	string uuid = UUIDToString((void*)& peerGUID);
	printf("successfully authenticated CP with UUID: %s\n", uuid.c_str());

	GUID localGUID;
	memset(&localGUID, 0, sizeof(localGUID));

	char * certhash = DVGetCertHashFromSessionToken(upnptoken);
	if (certhash) {
		format_uuid_v5(&localGUID, (unsigned char *) certhash); // Convert the cert hash into a name-based UUID

		// Now verify that the certificate from the TLS session matches the Registrar UUID from the WPS exchange
		if (memcmp(&peerGUID, &localGUID, sizeof(GUID)) != 0) {
			printf("!!! Error adding Control Point: Registrar UUID does not match certificate\n");
			string regid = UUIDToString((void*)& localGUID);
			printf("UUID from certificate is: %s\n", regid.c_str());
		} else {
			char * certName = DVGetCertNameFromSessionToken(upnptoken);
			if (certName) {
				certName = strchr(certName,'=');
				if (certName) {
					certName++; // point at the name after the "CN=" prefix
				}
			}
			if (! certName) {
				printf("!!! Error in certificate: No CommonName field found\n");
			} else {
				// X509   *err_cert = X509_STORE_CTX_get_current_cert(g_server_ctx);

				if (! IsNameBasedUUID((void*) &peerGUID)) {
					printf("warning:  Invalid GUID type: not name-based.\n");
				} else {
					CPIdentity * cpid = NULL;
					string id = UUIDToIdentity((void *) & peerGUID);
					bool result = g_devProt.AddRolesForIdentity( id.c_str(), "Basic" );
					if (result) { 
						cpid = g_devProt.FindCPbyID(uuid);
						if (cpid ) { // mark as directly introduced
							cpid->SetIntroduced(true);
							printf("Successfully added Basic role to existing Identity %s.\n", uuid.c_str());
						} else {
							printf("error finding identity record for %s\n", uuid);
						}
					} else {
						cpid = g_devProt.AddCP( certName, uuid.c_str(), "Basic", "" , true );
						if (cpid) {
							printf("Successfully added CP Identity %s with Basic Role: %s.\n", certName, uuid.c_str());
						}
					}
					// Make sure the CP is associated with the session
					Session * sess = g_devProt.GetSession(TokenToSession(upnptoken));
					if (sess) {
						sess->m_cp = cpid;
					}
				}
			}
		} 
	} else {
			printf("!!! Error: No TLS session found\n");
	}
	DVSetState_DeviceProtection_SetupReady(DVmicroStack,1);
}


extern void SendWPSMessage(void * registrar, void * peer, 
						   void (*CallbackPtr) (struct UPnPService* Service,int ErrorCode,void *User,unsigned char* OutMessage,int OutMessageLength),
						   BufferObj & outBuf)
{
	CPInvoke_DeviceProtection_SendSetupMessage((UPnPService*) peer, 
		CallbackPtr, registrar, "WPS", outBuf.GetBuf(), outBuf.Length());
}

extern void RegistrationReady( int value) 
{
	DVSetState_DeviceProtection_SetupReady(DVmicroStack,value);
}
