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
char* DVPLATFORM = "WINDOWS";
#elif defined(__SYMBIAN32__)
char* DVPLATFORM = "SYMBIAN";
#else
char* DVPLATFORM = "POSIX";
#endif

#if defined(WIN32)
#define snprintf sprintf_s
#define _CRTDBG_MAP_ALLOC
#endif

#if defined(WINSOCK2)
#	include <winsock2.h>
#	include <ws2tcpip.h>
#elif defined(WINSOCK1)
#	include <winsock.h>
#	include <wininet.h>
#endif

#include "ILibParsers.h"
#include "DVMicroStack.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"
#include "ILibAsyncUDPSocket.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
#include <crtdbg.h>
#endif

#define UPNP_SSDP_TTL 4
#define UPNP_HTTP_MAXSOCKETS 5
#define UPNP_MAX_SSDP_HEADER_SIZE 4096
#define UPNP_PORT 1900
#define UPNP_MCASTv4_GROUP "239.255.255.250"
#define UPNP_MCASTv6_GROUP "FF05:0:0:0:0:0:0:C" // Site local
#define UPNP_MCASTv6_GROUPB "[FF05:0:0:0:0:0:0:C]"
#define UPNP_MCASTv6_LINK_GROUP "FF02:0:0:0:0:0:0:C" // Link local
#define UPNP_MCASTv6_LINK_GROUPB "[FF02:0:0:0:0:0:0:C]"
#define DV_MAX_SUBSCRIPTION_TIMEOUT 7200
#define DVMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)
#define INET_SOCKADDR_LENGTH(x) ((x==AF_INET6?sizeof(struct sockaddr_in6):sizeof(struct sockaddr_in)))

#if defined(WIN32)
#pragma warning( push, 3 ) // warning C4310: cast truncates constant value
#endif

extern BOOL g_certToolMode;	// vbl added

//{{{ObjectDefintions}}}
DVMicroStackToken DVCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);
/* UPnP Set Function Pointers Methods */
void (*DVFP_PresentationPage) (void* upnptoken,struct packetheader *packet);
/*! \var DVFP_DeviceProtection_AddIdentityList
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> AddIdentityList
*/
DV_ActionHandler_DeviceProtection_AddIdentityList DVFP_DeviceProtection_AddIdentityList;
/*! \var DVFP_DeviceProtection_AddRolesForIdentity
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> AddRolesForIdentity
*/
DV_ActionHandler_DeviceProtection_AddRolesForIdentity DVFP_DeviceProtection_AddRolesForIdentity;
/*! \var DVFP_DeviceProtection_GetACLData
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetACLData
*/
DV_ActionHandler_DeviceProtection_GetACLData DVFP_DeviceProtection_GetACLData;
/*! \var DVFP_DeviceProtection_GetAssignedRoles
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetAssignedRoles
*/
DV_ActionHandler_DeviceProtection_GetAssignedRoles DVFP_DeviceProtection_GetAssignedRoles;
/*! \var DVFP_DeviceProtection_GetRolesForAction
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetRolesForAction
*/
DV_ActionHandler_DeviceProtection_GetRolesForAction DVFP_DeviceProtection_GetRolesForAction;
/*! \var DVFP_DeviceProtection_GetSupportedProtocols
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetSupportedProtocols
*/
DV_ActionHandler_DeviceProtection_GetSupportedProtocols DVFP_DeviceProtection_GetSupportedProtocols;
/*! \var DVFP_DeviceProtection_GetUserLoginChallenge
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetUserLoginChallenge
*/
DV_ActionHandler_DeviceProtection_GetUserLoginChallenge DVFP_DeviceProtection_GetUserLoginChallenge;
/*! \var DVFP_DeviceProtection_RemoveIdentity
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> RemoveIdentity
*/
DV_ActionHandler_DeviceProtection_RemoveIdentity DVFP_DeviceProtection_RemoveIdentity;
/*! \var DVFP_DeviceProtection_RemoveRolesForIdentity
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> RemoveRolesForIdentity
*/
DV_ActionHandler_DeviceProtection_RemoveRolesForIdentity DVFP_DeviceProtection_RemoveRolesForIdentity;
/*! \var DVFP_DeviceProtection_SendSetupMessage
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> SendSetupMessage
*/
DV_ActionHandler_DeviceProtection_SendSetupMessage DVFP_DeviceProtection_SendSetupMessage;
/*! \var DVFP_DeviceProtection_SetUserLoginPassword
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> SetUserLoginPassword
*/
DV_ActionHandler_DeviceProtection_SetUserLoginPassword DVFP_DeviceProtection_SetUserLoginPassword;
/*! \var DVFP_DeviceProtection_UserLogin
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> UserLogin
*/
DV_ActionHandler_DeviceProtection_UserLogin DVFP_DeviceProtection_UserLogin;
/*! \var DVFP_DeviceProtection_UserLogout
\brief Dispatch Pointer for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> UserLogout
*/
DV_ActionHandler_DeviceProtection_UserLogout DVFP_DeviceProtection_UserLogout;


const int DVDeviceDescriptionTemplateLengthUX = 853;
const int DVDeviceDescriptionTemplateLength = 481;
const char DVDeviceDescriptionTemplate[481]={
	0x5A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22
	,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C
	,0x72,0x6F,0x6F,0x74,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65
	,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x64,0x65,0x76,0x69,0x63,0x65,0x2D
	,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0xC6,0x14,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F
	,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02
	,0x02,0x3C,0x2F,0x8D,0x0B,0x00,0x06,0x12,0x00,0x08,0x02,0x05,0x54,0x79,0x70,0x65,0x3E,0x1B,0x1C,0x0A
	,0x3A,0x42,0x61,0x73,0x69,0x63,0x3A,0x31,0x3C,0x2F,0x0B,0x0C,0x12,0x3C,0x66,0x72,0x69,0x65,0x6E,0x64
	,0x6C,0x79,0x4E,0x61,0x6D,0x65,0x3E,0x25,0x73,0x3C,0x2F,0x4D,0x04,0x21,0x3C,0x6D,0x61,0x6E,0x75,0x66
	,0x61,0x63,0x74,0x75,0x72,0x65,0x72,0x3E,0x49,0x6E,0x74,0x65,0x6C,0x20,0x43,0x6F,0x72,0x70,0x6F,0x72
	,0x61,0x74,0x69,0x6F,0x6E,0x3C,0x2F,0x0D,0x08,0x00,0x8D,0x0B,0x10,0x55,0x52,0x4C,0x3E,0x68,0x74,0x74
	,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x69,0x04,0x0F,0x06,0x2E,0x63,0x6F,0x6D,0x3C,0x2F,0x90,0x09
	,0x0D,0x3C,0x6D,0x6F,0x64,0x65,0x6C,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0xC4,0x15,0x0C,0x3E,0x50,0x72
	,0x6F,0x74,0x65,0x63,0x74,0x65,0x64,0x20,0x44,0xC5,0x4A,0x02,0x3C,0x2F,0xD1,0x08,0x00,0x46,0x0D,0x00
	,0x85,0x2D,0x00,0x97,0x0B,0x00,0x47,0x30,0x00,0xC5,0x09,0x0A,0x75,0x6D,0x62,0x65,0x72,0x3E,0x58,0x31
	,0x3C,0x2F,0x0C,0x04,0x06,0x3C,0x73,0x65,0x72,0x69,0x61,0x88,0x07,0x00,0x44,0x3F,0x00,0x4D,0x04,0x0A
	,0x3C,0x55,0x44,0x4E,0x3E,0x75,0x75,0x69,0x64,0x3A,0x04,0x46,0x03,0x55,0x44,0x4E,0x45,0x0C,0x00,0xC4
	,0x6E,0x04,0x4C,0x69,0x73,0x74,0x49,0x03,0x00,0x89,0x05,0x00,0x5A,0x60,0x00,0xC7,0x0D,0x01,0x3A,0x86
	,0x33,0x00,0x87,0x37,0x05,0x69,0x6F,0x6E,0x3A,0x31,0xC5,0x1C,0x00,0x8A,0x63,0x00,0x07,0x18,0x02,0x49
	,0x64,0x45,0x72,0x00,0x10,0x10,0x02,0x49,0x64,0x91,0x10,0x02,0x3C,0x2F,0xCA,0x0C,0x05,0x3C,0x53,0x43
	,0x50,0x44,0xC4,0x5D,0x00,0xD0,0x19,0x0B,0x2F,0x73,0x63,0x70,0x64,0x2E,0x78,0x6D,0x6C,0x3C,0x2F,0xC8
	,0x08,0x08,0x3C,0x63,0x6F,0x6E,0x74,0x72,0x6F,0x6C,0xD5,0x0B,0x00,0x07,0x07,0x02,0x3C,0x2F,0x4B,0x09
	,0x09,0x3C,0x65,0x76,0x65,0x6E,0x74,0x53,0x75,0x62,0x55,0x18,0x00,0x45,0x07,0x02,0x3C,0x2F,0x0C,0x09
	,0x00,0x89,0x36,0x03,0x3E,0x3C,0x2F,0xCD,0x4D,0x01,0x2F,0xC8,0xAD,0x01,0x2F,0x44,0xCA,0x01,0x3E,0x00
	,0x00};
/* DeviceProtection */
const int DVDeviceProtectionDescriptionLengthUX = 5829;
const int DVDeviceProtectionDescriptionLength = 1115;
const char DVDeviceProtectionDescription[1115] = {
	0x50,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x3B,0x20,0x63,0x68,0x61,0x72,0x73,0x65,0x74,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x0D
	,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C,0x20,0x55,0x50
	,0x6E,0x44,0x13,0x0D,0x30,0x2C,0x20,0x4D,0x69,0x63,0x72,0x6F,0x53,0x74,0x61,0x63,0x6B,0x04,0x04,0x3B
	,0x2E,0x33,0x39,0x35,0x31,0x0D,0x0A,0x43,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74
	,0x68,0x3A,0x20,0x35,0x36,0x39,0x37,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72
	,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x88
	,0x1B,0x37,0x3F,0x3E,0x3C,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E
	,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B
	,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72
	,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69
	,0x73,0x74,0x08,0x03,0x12,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x41,0x64,0x64,0x49,0x64,0x65,0x6E,0x74
	,0x69,0x74,0x79,0x84,0x07,0x02,0x3C,0x2F,0x85,0x05,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65,0x6E,0x74
	,0x87,0x0C,0x00,0x87,0x03,0x00,0x07,0x0D,0x00,0x54,0x0C,0x04,0x64,0x69,0x72,0x65,0x06,0x16,0x04,0x69
	,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56
	,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0x04,0x64,0x01,0x5F,0x8E,0x1F
	,0x00,0x95,0x0B,0x02,0x3C,0x2F,0x4A,0x20,0x00,0xDB,0x22,0x05,0x52,0x65,0x73,0x75,0x6C,0x53,0x24,0x03
	,0x6F,0x75,0x74,0xBF,0x24,0x00,0x9D,0x24,0x01,0x2F,0x0E,0x4B,0x04,0x2F,0x61,0x63,0x74,0xCB,0x5C,0x00
	,0xCA,0x59,0x08,0x52,0x6F,0x6C,0x65,0x73,0x46,0x6F,0x72,0xC8,0x5B,0x00,0xED,0x5A,0x00,0xFF,0x59,0x00
	,0xCA,0x59,0x00,0xF2,0x58,0x00,0x04,0x2E,0x00,0xBF,0x7A,0x00,0x86,0x7A,0x06,0x53,0x74,0x72,0x69,0x6E
	,0x67,0xBF,0x54,0x00,0x49,0xAE,0x0A,0x47,0x65,0x74,0x41,0x43,0x4C,0x44,0x61,0x74,0x61,0x25,0xAD,0x03
	,0x41,0x43,0x4C,0xBF,0x86,0x06,0x50,0x45,0x5F,0x41,0x43,0x4C,0xFF,0x2F,0x00,0xCD,0x2F,0x06,0x73,0x73
	,0x69,0x67,0x6E,0x65,0x46,0x86,0x00,0x65,0xDE,0x00,0xDA,0x62,0x03,0x6F,0x75,0x74,0x3F,0x63,0x00,0x3F
	,0x63,0x00,0x88,0xB7,0x01,0x41,0xC5,0xFD,0x00,0x25,0xB7,0x09,0x44,0x65,0x76,0x69,0x63,0x65,0x55,0x44
	,0x4E,0xBF,0x96,0x00,0xAB,0x96,0x00,0x8F,0xD7,0x03,0x53,0x65,0x72,0xC4,0x20,0x02,0x49,0x64,0xBF,0x20
	,0x00,0xBA,0x20,0x00,0xC6,0x4B,0x04,0x4E,0x61,0x6D,0x65,0x7F,0x41,0x00,0x7A,0x41,0x00,0x7F,0x95,0x00
	,0xBF,0x20,0x00,0xC5,0xB5,0x09,0x65,0x73,0x74,0x72,0x69,0x63,0x74,0x65,0x64,0x7F,0xB8,0x00,0x7F,0xB8
	,0x00,0x9D,0xEB,0x11,0x53,0x75,0x70,0x70,0x6F,0x72,0x74,0x65,0x64,0x50,0x72,0x6F,0x74,0x6F,0x63,0x6F
	,0x6C,0xE6,0xEC,0x00,0x88,0x0B,0x00,0xFB,0xED,0x00,0x94,0x1E,0x00,0x3F,0xEE,0x00,0x0A,0xEE,0x12,0x55
	,0x73,0x65,0x72,0x4C,0x6F,0x67,0x69,0x6E,0x43,0x68,0x61,0x6C,0x6C,0x65,0x6E,0x67,0x65,0xED,0x35,0x03
	,0x54,0x79,0x70,0xBF,0xAE,0x00,0xFB,0xEF,0x00,0xFF,0xCD,0x00,0xBE,0xEE,0x03,0x53,0x61,0x6C,0xFF,0xCC
	,0x00,0x44,0xED,0x06,0x42,0x61,0x73,0x65,0x36,0x34,0x72,0xED,0x00,0x91,0x6B,0x00,0xFF,0x20,0x01,0x34
	,0xBF,0xCA,0x00,0x89,0xCA,0x0E,0x52,0x65,0x6D,0x6F,0x76,0x65,0x49,0x64,0x65,0x6E,0x74,0x69,0x74,0x79
	,0xE5,0xC8,0x00,0x50,0x0B,0x00,0x39,0x92,0x00,0x8A,0x1D,0x00,0x3F,0x33,0x00,0x0D,0x33,0x08,0x52,0x6F
	,0x6C,0x65,0x73,0x46,0x6F,0x72,0x3F,0x35,0x00,0x3F,0x35,0x00,0xB2,0xC7,0x00,0x04,0x2E,0x03,0x4C,0x69
	,0x73,0x53,0xA9,0x00,0xFF,0xE7,0x00,0x7E,0x88,0x10,0x53,0x65,0x6E,0x64,0x53,0x65,0x74,0x75,0x70,0x4D
	,0x65,0x73,0x73,0x61,0x67,0x65,0xE5,0x88,0x0B,0x50,0x72,0x6F,0x74,0x6F,0x63,0x6F,0x6C,0x54,0x79,0x70
	,0xBF,0xFC,0x00,0xBB,0xFC,0x02,0x49,0x6E,0xCF,0x2C,0x00,0x79,0xAA,0x00,0xB8,0xFD,0x03,0x4F,0x75,0x74
	,0xD9,0x20,0x00,0xFF,0xFD,0x00,0xBF,0x75,0x14,0x53,0x65,0x74,0x55,0x73,0x65,0x72,0x4C,0x6F,0x67,0x69
	,0x6E,0x50,0x61,0x73,0x73,0x77,0x6F,0x72,0x64,0xBF,0x76,0x00,0xBF,0x76,0x00,0xEC,0xCA,0x03,0x4E,0x61
	,0x6D,0xFF,0x95,0x00,0xFB,0x95,0x06,0x53,0x74,0x6F,0x72,0x65,0x64,0x3F,0x95,0x00,0x3A,0x95,0x04,0x53
	,0x61,0x6C,0x74,0x7F,0xB4,0x00,0x7F,0x93,0x00,0x51,0x93,0x00,0x89,0x92,0x00,0xBF,0x90,0x00,0xBF,0x90
	,0x00,0xAC,0xE6,0x08,0x43,0x68,0x61,0x6C,0x6C,0x65,0x6E,0x67,0x3F,0xB1,0x00,0x3B,0x72,0x0D,0x41,0x75
	,0x74,0x68,0x65,0x6E,0x74,0x69,0x63,0x61,0x74,0x6F,0x72,0x7F,0x74,0x00,0x7F,0x74,0x00,0x58,0x74,0x02
	,0x6F,0x75,0x89,0x9A,0x00,0x49,0x7E,0x00,0x87,0x80,0x00,0xC6,0x83,0x07,0x73,0x65,0x72,0x76,0x69,0x63
	,0x65,0x05,0xF8,0x01,0x54,0x46,0xED,0x01,0x73,0x0C,0xFB,0x10,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65
	,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0xC8,0x3E,0x00,0x4A,0xE1,0x03,0x41,0x43,0x4C,0x49,0xF2,0x03
	,0x61,0x74,0x61,0x04,0x85,0x02,0x3E,0x73,0x47,0xE6,0x00,0x49,0x04,0x03,0x3C,0x2F,0x73,0x4E,0xE7,0x00
	,0x6F,0x19,0x00,0xC8,0xDA,0x00,0x0F,0x1A,0x05,0x62,0x69,0x6E,0x2E,0x62,0x87,0xE1,0x00,0x3E,0x1B,0x09
	,0x53,0x75,0x70,0x70,0x6F,0x72,0x74,0x65,0x64,0x08,0xB7,0x01,0x73,0x7F,0x35,0x00,0xA3,0x4E,0x02,0x49
	,0x64,0x84,0x8F,0x02,0x74,0x79,0xBF,0x1A,0x00,0xAB,0x1A,0x03,0x4C,0x69,0x73,0x09,0x84,0x00,0x7F,0x6B
	,0x00,0x9B,0x84,0x00,0x88,0xF4,0x00,0x7F,0x85,0x00,0x8C,0x9E,0x03,0x79,0x65,0x73,0x89,0x6A,0x08,0x65
	,0x74,0x75,0x70,0x52,0x65,0x61,0x64,0x12,0x4E,0x07,0x62,0x6F,0x6F,0x6C,0x65,0x61,0x6E,0x1C,0x9E,0x01
	,0x2F,0x53,0xBC,0x03,0x2F,0x73,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00,0x00};

#if defined(WIN32)
#pragma warning( pop )
#endif

struct DVDataObject;

// It should not be necessary to expose/modify any of these structures. They are used by the internal stack
struct SubscriberInfo
{
	char* SID;		// Subscription ID
	int SIDLength;
	int SEQ;
	
	int Address;
	unsigned short Port;
	char* Path;
	int PathLength;
	int RefCount;
	int Disposing;
	
	struct timeval RenewByTime;
	
	struct SubscriberInfo *Next;
	struct SubscriberInfo *Previous;
};

struct DVDataObject
{
	// Absolutely DO NOT put anything above these 3 function pointers
	ILibChain_PreSelect PreSelect;
	ILibChain_PostSelect PostSelect;
	ILibChain_Destroy Destroy;
	
	void *EventClient;
	void *Chain;
	int UpdateFlag;
	
	// Network Poll
	unsigned int NetworkPollTime;
	
	int ForceExit;
	char *UUID;
	char *UDN;
	char *Serial;
	void *User;
	void *User2;
	
	void *WebServerTimer;
	void *HTTPServer;
	void *HTTPSServer; // vbl added
	char *DeviceDescription;
	int DeviceDescriptionLength;	int InitialNotify;
	char* DeviceProtection_SetupReady;
	
	struct sockaddr_in addr;
	int addrlen;
	
	// Current local interfaces
	struct sockaddr_in* AddressListV4;
	int AddressListV4Length;
	struct sockaddr_in6* AddressListV6;
	int AddressListV6Length;
	
	// Multicast Addresses
	struct sockaddr_in MulticastAddrV4;
	struct sockaddr_in6 MulticastAddrV6SL;
	struct sockaddr_in6 MulticastAddrV6LL;
	
	int _NumEmbeddedDevices;
	int WebSocketPortNumber;
	int WebSocketSPortNumber; // vbl added
	
	void **NOTIFY_RECEIVE_socks;
	void **NOTIFY_SEND_socks;
	void **NOTIFY_RECEIVE_socks6;
	void **NOTIFY_SEND_socks6;
	struct timeval CurrentTime;
	struct timeval NotifyTime;
	
	int SID;
	int NotifyCycleTime;
	sem_t EventLock;
	struct SubscriberInfo *HeadSubscriberPtr_DeviceProtection;
	int NumberOfSubscribers_DeviceProtection;
	
};

struct MSEARCH_state
{
	char *ST;
	int STLength;
	void *upnp;
	struct sockaddr_in6 dest_addr;
	struct sockaddr_in6 localIPAddress;
	void *Chain;
	void *SubChain;
};
struct DVFragmentNotifyStruct
{
	struct DVDataObject *upnp;
	int packetNumber;
};

/* Pre-declarations */
void DVFragmentedSendNotify(void *data);
void DVSendNotify(const struct DVDataObject *upnp);
void DVSendByeBye(const struct DVDataObject *upnp);
void DVMainInvokeSwitch();
void DVSendDataXmlEscaped(const void* DVToken, const char* Data, const int DataLength, const int Terminate);
void DVSendData(const void* DVToken, const char* Data, const int DataLength, const int Terminate);
int DVPeriodicNotify(struct DVDataObject *upnp);
void DVSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);
void DVProcessMSEARCH(struct DVDataObject *upnp, struct packetheader *packet);
struct in_addr DV_inaddr;


/*! \fn DVGetWebServerToken(const DVMicroStackToken MicroStackToken)
\brief Converts a MicroStackToken to a WebServerToken
\par
\a MicroStackToken is the void* returned from a call to DVCreateMicroStack. The returned token, is the server token
not the session token.
\param MicroStackToken MicroStack Token
\returns WebServer Token
*/
void* DVGetWebServerToken(const DVMicroStackToken MicroStackToken)
{
	return(((struct DVDataObject*)MicroStackToken)->HTTPServer);
}
int DVBuildSendSsdpResponsePacket(void* module, const struct DVDataObject *upnp, struct sockaddr* local, struct sockaddr* target, int EmbeddedDeviceNumber, char* USNex, char* ST, char* NTex)
{
	int len;
	UNREFERENCED_PARAMETER( EmbeddedDeviceNumber );
	
	if (local->sa_family == AF_INET)
	{
		// IPv4 address format
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in*)local)->sin_addr), ILibScratchPad2, sizeof(ILibScratchPad2));
	}
	else if (local->sa_family == AF_INET6)
	{
		// IPv6 address format
		size_t len2;
		ILibScratchPad2[0] = '[';
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in6*)local)->sin6_addr), ILibScratchPad2 + 1, sizeof(ILibScratchPad2) - 2);
		len2 = strlen(ILibScratchPad2);
		ILibScratchPad2[len2] = ']';
		ILibScratchPad2[len2 + 1] = 0;
	}
	
	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nSECURELOCATION.UPNP.ORG: https://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/1.0, MicroStack/1.0.3769\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n", 
		ILibScratchPad2, upnp->WebSocketPortNumber, ILibScratchPad2, upnp->WebSocketSPortNumber, DVPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, ST, NTex);
	if (g_certToolMode) {	
		Sleep(40); // vbl added to reduce burstiness
	}
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}



int DVBuildSendSsdpNotifyPacket(void* module, const struct DVDataObject *upnp, struct sockaddr* local, int EmbeddedDeviceNumber, char* USNex, char* NT, char* NTex)
{
	int len;
	struct sockaddr* multicast = NULL;
	char* mcaststr = NULL;
	UNREFERENCED_PARAMETER( EmbeddedDeviceNumber );
	
	if (local->sa_family == AF_INET)
	{
		// IPv4 address format
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in*)local)->sin_addr), ILibScratchPad2, sizeof(ILibScratchPad2));
		multicast = (struct sockaddr*)&(upnp->MulticastAddrV4);
		mcaststr = UPNP_MCASTv4_GROUP;
	}
	else if (local->sa_family == AF_INET6)
	{
		// IPv6 address format
		size_t len;
		ILibScratchPad2[0] = '[';
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in6*)local)->sin6_addr), ILibScratchPad2 + 1, sizeof(ILibScratchPad2) - 2);
		len = strlen(ILibScratchPad2);
		ILibScratchPad2[len] = ']';
		ILibScratchPad2[len + 1] = 0;
		if (ILibAsyncSocket_IsIPv6LinkLocal(local))
		{
			multicast = (struct sockaddr*)&(upnp->MulticastAddrV6LL);
			mcaststr = UPNP_MCASTv6_LINK_GROUPB;
		}
		else
		{
			multicast = (struct sockaddr*)&(upnp->MulticastAddrV6SL);
			mcaststr = UPNP_MCASTv6_GROUPB;
		}
	}
	else return 0;
	
	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nSECURELOCATION.UPNP.ORG: https://%s:%d/\r\nEXT:\r\nHOST: %s:1900\r\nSERVER: %s, UPnP/1.0, MicroStack/1.0.3769\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", 
		ILibScratchPad2, upnp->WebSocketPortNumber, ILibScratchPad2, upnp->WebSocketSPortNumber, mcaststr, DVPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, NT, NTex);
	
	if (g_certToolMode) {	
		Sleep(40); // vbl added to reduce burstiness
	}
	return ILibAsyncUDPSocket_SendTo(module, multicast, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
void DVSetDisconnectFlag(DVSessionToken token,void *flag)
{
	((struct ILibWebServer_Session*)token)->Reserved10=flag;
}

/*! \fn DVIPAddressListChanged(DVMicroStackToken MicroStackToken)
\brief Tell the underlying MicroStack that an IPAddress may have changed
\param MicroStackToken Microstack
*/
void DVIPAddressListChanged(DVMicroStackToken MicroStackToken)
{
	((struct DVDataObject*)MicroStackToken)->UpdateFlag = 1;
	ILibForceUnBlockChain(((struct DVDataObject*)MicroStackToken)->Chain);
}

void DVSSDPSink(ILibAsyncUDPSocket_SocketModule socketModule, char* buffer, int bufferLength, struct sockaddr_in6 *remoteInterface, void *user, void *user2, int *PAUSE)
{
	struct packetheader *packet;
	UNREFERENCED_PARAMETER( user2 );
	UNREFERENCED_PARAMETER( PAUSE );
	
	packet = ILibParsePacketHeader(buffer, 0, bufferLength);
	if (packet != NULL)
	{
		// Fill in the source and local interface addresses
		memcpy(&(packet->Source), remoteInterface, INET_SOCKADDR_LENGTH(remoteInterface->sin6_family));
		ILibAsyncUDPSocket_GetLocalInterface(socketModule, (struct sockaddr*)&(packet->ReceivingAddress));
		
		if (packet->StatusCode == -1 && memcmp(packet->Directive, "M-SEARCH", 8) == 0 && ((struct sockaddr_in6*)(packet->ReceivingAddress))->sin6_family != 0)
		{
			// Process the search request with our Multicast M-SEARCH Handler
			DVProcessMSEARCH(user, packet);
		}
		ILibDestructPacket(packet);
	}
}
//
//	Internal underlying Initialization, that shouldn't be called explicitely
// 
// <param name="state">State object</param>
// <param name="NotifyCycleSeconds">Cycle duration</param>
// <param name="PortNumber">Port Number</param>
void DVInit(struct DVDataObject *state, void *chain, const int NotifyCycleSeconds, const unsigned short PortNumber)
{
	int i;
	struct sockaddr_in any4;
	struct sockaddr_in6 any6;
	UNREFERENCED_PARAMETER( PortNumber );
	
	// Setup ANY addresses
	memset(&any4, 0, sizeof(struct sockaddr_in));
	any4.sin_family = AF_INET;
	any4.sin_port = htons(UPNP_PORT);
	memset(&any6, 0, sizeof(struct sockaddr_in6));
	any6.sin6_family = AF_INET6;
	any6.sin6_port = htons(UPNP_PORT);
	
	state->Chain = chain;
	
	// Setup notification timer
	state->NotifyCycleTime = NotifyCycleSeconds;
	gettimeofday(&(state->CurrentTime), NULL);
	(state->NotifyTime).tv_sec = (state->CurrentTime).tv_sec  + (state->NotifyCycleTime / 2);
	
	// Fetch the list of local IPv4 interfaces
	state->AddressListV4Length = ILibGetLocalIPv4AddressList(&(state->AddressListV4), 1);
	
	// Setup the IPv4 sockets
	if ((state->NOTIFY_SEND_socks = (void**)malloc(sizeof(void*)*(state->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);
	if ((state->NOTIFY_RECEIVE_socks = (void**)malloc(sizeof(void*)*(state->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);
	
	// Setup multicast IPv4 address
	memset(&state->MulticastAddrV4, 0, sizeof(struct sockaddr_in));
	state->MulticastAddrV4.sin_family = AF_INET;
	state->MulticastAddrV4.sin_port = htons(UPNP_PORT);
	ILibInet_pton(AF_INET, UPNP_MCASTv4_GROUP, &(state->MulticastAddrV4.sin_addr));
	
	// Test IPv6 support
	if (ILibDetectIPv6Support())
	{
		// Fetch the list of local IPv6 interfaces
		state->AddressListV6Length = ILibGetLocalIPv6List(&(state->AddressListV6));
		
		// Setup the IPv6 sockets
		if ((state->NOTIFY_SEND_socks6 = (void**)malloc(sizeof(void*)*(state->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
		if ((state->NOTIFY_RECEIVE_socks6 = (void**)malloc(sizeof(void*)*(state->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
		
		// Setup multicast IPv6 address (Site Local)
		memset(&state->MulticastAddrV6SL, 0, sizeof(struct sockaddr_in6));
		state->MulticastAddrV6SL.sin6_family = AF_INET6;
		state->MulticastAddrV6SL.sin6_port = htons(UPNP_PORT);
		ILibInet_pton(AF_INET6, UPNP_MCASTv6_GROUP, &(state->MulticastAddrV6SL.sin6_addr));
		
		// Setup multicast IPv6 address (Link Local)
		memset(&state->MulticastAddrV6LL, 0, sizeof(struct sockaddr_in6));
		state->MulticastAddrV6LL.sin6_family = AF_INET6;
		state->MulticastAddrV6LL.sin6_port = htons(UPNP_PORT);
		ILibInet_pton(AF_INET6, UPNP_MCASTv6_LINK_GROUP, &(state->MulticastAddrV6LL.sin6_addr));
	}
	
	// Iterate through all the current IPv4 addresses
	for (i = 0; i < state->AddressListV4Length; ++i)
	{
		(state->AddressListV4[i]).sin_port = 0; // Bind to ANY port for outbound packets
		state->NOTIFY_SEND_socks[i] = ILibAsyncUDPSocket_CreateEx(
		chain,
		UPNP_MAX_SSDP_HEADER_SIZE,
		(struct sockaddr*)&(state->AddressListV4[i]),
		ILibAsyncUDPSocket_Reuse_SHARED,
		NULL,
		NULL,
		state);
		
		ILibAsyncUDPSocket_SetMulticastTTL(state->NOTIFY_SEND_socks[i], UPNP_SSDP_TTL);
		ILibAsyncUDPSocket_SetMulticastLoopback(state->NOTIFY_SEND_socks[i], 1);
		
		(state->AddressListV4[i]).sin_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
		state->NOTIFY_RECEIVE_socks[i] = ILibAsyncUDPSocket_CreateEx(
		state->Chain,
		UPNP_MAX_SSDP_HEADER_SIZE,
		(struct sockaddr*)&any4,
		ILibAsyncUDPSocket_Reuse_SHARED,
		&DVSSDPSink,
		NULL,
		state);
		
		ILibAsyncUDPSocket_JoinMulticastGroupV4(state->NOTIFY_RECEIVE_socks[i], (struct sockaddr_in*)&(state->MulticastAddrV4), (struct sockaddr*)&(state->AddressListV4[i]));
		ILibAsyncUDPSocket_SetLocalInterface(state->NOTIFY_RECEIVE_socks[i], (struct sockaddr*)&(state->AddressListV4[i]));
		ILibAsyncUDPSocket_SetMulticastLoopback(state->NOTIFY_RECEIVE_socks[i], 1);
	}
	
	if (state->AddressListV6Length > 0)
	{
		// Iterate through all the current IPv6 interfaces
		for (i = 0; i < state->AddressListV6Length; ++i)
		{
			(state->AddressListV6[i]).sin6_port = 0;
			state->NOTIFY_SEND_socks6[i] = ILibAsyncUDPSocket_CreateEx(
			chain,
			UPNP_MAX_SSDP_HEADER_SIZE,
			(struct sockaddr*)&(state->AddressListV6[i]),
			ILibAsyncUDPSocket_Reuse_SHARED,
			NULL,
			NULL,
			state);
			
			ILibAsyncUDPSocket_SetMulticastTTL(state->NOTIFY_SEND_socks6[i], UPNP_SSDP_TTL);
			ILibAsyncUDPSocket_SetMulticastLoopback(state->NOTIFY_SEND_socks6[i], 1);
			
			(state->AddressListV6[i]).sin6_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
			state->NOTIFY_RECEIVE_socks6[i] = ILibAsyncUDPSocket_CreateEx(
			state->Chain,
			UPNP_MAX_SSDP_HEADER_SIZE,
			(struct sockaddr*)&any6,
			ILibAsyncUDPSocket_Reuse_SHARED,
			&DVSSDPSink,
			NULL,
			state);
			
			if (ILibAsyncSocket_IsIPv6LinkLocal((struct sockaddr*)&(state->AddressListV6[i])))
			{
				ILibAsyncUDPSocket_JoinMulticastGroupV6(state->NOTIFY_RECEIVE_socks6[i], &(state->MulticastAddrV6LL), state->AddressListV6[i].sin6_scope_id);
			}
			else
			{
				ILibAsyncUDPSocket_JoinMulticastGroupV6(state->NOTIFY_RECEIVE_socks6[i], &(state->MulticastAddrV6SL), state->AddressListV6[i].sin6_scope_id);
			}
			ILibAsyncUDPSocket_SetMulticastLoopback(state->NOTIFY_RECEIVE_socks6[i], 1);
			ILibAsyncUDPSocket_SetLocalInterface(state->NOTIFY_RECEIVE_socks6[i], (struct sockaddr*)&(state->AddressListV6[i]));
		}
	}
}

void DVPostMX_Destroy(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	free(mss->ST);
	free(mss);
}
void DVOnPostMX_MSEARCH_SendOK(ILibAsyncUDPSocket_SocketModule socketModule, void *user1, void *user2)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)user1;
	UNREFERENCED_PARAMETER( socketModule );
	UNREFERENCED_PARAMETER( user2 );
	
	ILibChain_SafeRemove_SubChain(mss->Chain, mss->SubChain);
	free(mss->ST);
	free(mss);
}

void DVPostMX_MSEARCH(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	void *response_socket;
	void *subChain;
	char *ST = mss->ST;
	int STLength = mss->STLength;
	struct DVDataObject *upnp = (struct DVDataObject*)mss->upnp;
	int rcode = 0;
	
	subChain = ILibCreateChain();
	
	response_socket = ILibAsyncUDPSocket_CreateEx(
	subChain,
	UPNP_MAX_SSDP_HEADER_SIZE,
	(struct sockaddr*)&(mss->localIPAddress),
	ILibAsyncUDPSocket_Reuse_SHARED,
	NULL,
	DVOnPostMX_MSEARCH_SendOK,
	mss);
	
	ILibChain_SafeAdd_SubChain(mss->Chain, subChain);
	mss->SubChain = subChain;
	
	// Search for root device
	if (STLength == 15 && memcmp(ST, "upnp:rootdevice", 15) == 0)
	{
		DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
	}
	// Search for everything
	else if (STLength == 8 && memcmp(ST, "ssdp:all", 8) == 0)
	{
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "", upnp->UUID, "");
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "");
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "");
		
	}
	else if (STLength == (int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)
	{
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0,"",upnp->UUID,"");
	}
	else if (STLength >= 34 && memcmp(ST,"urn:schemas-upnp-org:device:Basic:",34)==0 && atoi(ST+34)<=1)
	{
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::urn:schemas-upnp-org:device:Basic:1", ST, "");
	}
	else if (STLength >= 46 && memcmp(ST,"urn:schemas-upnp-org:service:DeviceProtection:",46)==0 && atoi(ST+46)<=1)
	{
		rcode = DVBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", ST, "");
	}
	
	
	if (rcode == 0)
	{
		ILibChain_SafeRemove_SubChain(mss->Chain, subChain);
		free(mss->ST);
		free(mss);
	}
}

void DVProcessMSEARCH(struct DVDataObject *upnp, struct packetheader *packet)
{
	char* ST = NULL;
	unsigned long MXVal = 0;
	int STLength = 0, MANOK = 0, MXOK = 0, MX;
	struct packetheader_field_node *node;
	struct MSEARCH_state *mss = NULL;
	
	if (memcmp(packet->DirectiveObj, "*", 1)==0)
	{
		if (memcmp(packet->Version, "1.1", 3)==0)
		{
			node = packet->FirstField;
			while(node!=NULL)
			{
				if (node->FieldLength==2 && strncasecmp(node->Field,"ST",2)==0)
				{
					// This is what is being searched for
					if ((ST = (char*)malloc(1 + node->FieldDataLength)) == NULL) ILIBCRITICALEXIT(254);
					memcpy(ST, node->FieldData, node->FieldDataLength);
					ST[node->FieldDataLength] = 0;
					STLength = node->FieldDataLength;
				}
				else if (node->FieldLength == 3 && strncasecmp(node->Field, "MAN", 3) == 0 && memcmp(node->FieldData, "\"ssdp:discover\"", 15) == 0)
				{
					// This is a required header field
					MANOK = 1;
				}
				else if (node->FieldLength == 2 && strncasecmp(node->Field, "MX", 2) == 0 && ILibGetULong(node->FieldData, node->FieldDataLength, &MXVal) == 0)
				{
					// If the timeout value specified is greater than 10 seconds, just force it down to 10 seconds
					MXOK = 1;
					MXVal = MXVal>10?10:MXVal;
				}
				node = node->NextField;
			}
			if (MANOK != 0 && MXOK != 0)
			{
				if (MXVal == 0)
				{
					MX = 0;
				}
				else
				{
					// The timeout value should be a random number between 0 and the specified value
					MX = (int)(0 + ((unsigned short)rand() % MXVal));
				}
				if ((mss = (struct MSEARCH_state*)malloc(sizeof(struct MSEARCH_state))) == NULL) ILIBCRITICALEXIT(254);
				mss->ST = ST;
				mss->STLength = STLength;
				mss->upnp = upnp;
				memcpy(&(mss->dest_addr), &(packet->Source), sizeof(struct sockaddr_in6));
				memcpy(&(mss->localIPAddress), &(packet->ReceivingAddress), sizeof(struct sockaddr_in6));
				mss->Chain = upnp->Chain;
				
				// Register for a timed callback, so we can respond later
				ILibLifeTime_Add(upnp->WebServerTimer, mss, MX, &DVPostMX_MSEARCH, &DVPostMX_Destroy);
			}
			else
			{
				free(ST);
			}
		}
	}
}

void DVDispatch_DeviceProtection_AddIdentityList(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_IdentityList = NULL;
	int p_IdentityListLength = 0;
	char* _IdentityList = "";
	int _IdentityListLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 15 && memcmp(xnode->Name, "AddIdentityList",15) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 12 && memcmp(xnode->Name, "IdentityList",12)==0)
								{
									p_IdentityListLength = ILibReadInnerXML(xnode, &p_IdentityList);
									p_IdentityList[p_IdentityListLength]=0;
									OK |= 1;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 1)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_IdentityListLength = ILibInPlaceXmlUnEscape(p_IdentityList);
	_IdentityList = p_IdentityList;
	if (DVFP_DeviceProtection_AddIdentityList == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_AddIdentityList((void*)ReaderObject,_IdentityList);
}

void DVDispatch_DeviceProtection_AddRolesForIdentity(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_Identity = NULL;
	int p_IdentityLength = 0;
	char* _Identity = "";
	int _IdentityLength;
	char *p_RoleList = NULL;
	int p_RoleListLength = 0;
	char* _RoleList = "";
	int _RoleListLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 19 && memcmp(xnode->Name, "AddRolesForIdentity",19) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 8 && memcmp(xnode->Name, "Identity",8)==0)
								{
									p_IdentityLength = ILibReadInnerXML(xnode, &p_Identity);
									p_Identity[p_IdentityLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 8 && memcmp(xnode->Name, "RoleList",8)==0)
								{
									p_RoleListLength = ILibReadInnerXML(xnode, &p_RoleList);
									p_RoleList[p_RoleListLength]=0;
									OK |= 2;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 3)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_IdentityLength = ILibInPlaceXmlUnEscape(p_Identity);
	_Identity = p_Identity;
	_RoleListLength = ILibInPlaceXmlUnEscape(p_RoleList);
	_RoleList = p_RoleList;
	if (DVFP_DeviceProtection_AddRolesForIdentity == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_AddRolesForIdentity((void*)ReaderObject,_Identity,_RoleList);
}

#define DVDispatch_DeviceProtection_GetACLData(buffer,offset,bufferLength, session)\
{\
	if (DVFP_DeviceProtection_GetACLData == NULL)\
	DVResponse_Error(session,501,"No Function Handler");\
	else\
	DVFP_DeviceProtection_GetACLData((void*)session);\
}

#define DVDispatch_DeviceProtection_GetAssignedRoles(buffer,offset,bufferLength, session)\
{\
	if (DVFP_DeviceProtection_GetAssignedRoles == NULL)\
	DVResponse_Error(session,501,"No Function Handler");\
	else\
	DVFP_DeviceProtection_GetAssignedRoles((void*)session);\
}

void DVDispatch_DeviceProtection_GetRolesForAction(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_DeviceUDN = NULL;
	int p_DeviceUDNLength = 0;
	char* _DeviceUDN = "";
	int _DeviceUDNLength;
	char *p_ServiceId = NULL;
	int p_ServiceIdLength = 0;
	char* _ServiceId = "";
	int _ServiceIdLength;
	char *p_ActionName = NULL;
	int p_ActionNameLength = 0;
	char* _ActionName = "";
	int _ActionNameLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 17 && memcmp(xnode->Name, "GetRolesForAction",17) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 9 && memcmp(xnode->Name, "DeviceUDN",9)==0)
								{
									p_DeviceUDNLength = ILibReadInnerXML(xnode, &p_DeviceUDN);
									p_DeviceUDN[p_DeviceUDNLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 9 && memcmp(xnode->Name, "ServiceId",9)==0)
								{
									p_ServiceIdLength = ILibReadInnerXML(xnode, &p_ServiceId);
									p_ServiceId[p_ServiceIdLength]=0;
									OK |= 2;
								}
								else if (xnode->NameLength == 10 && memcmp(xnode->Name, "ActionName",10)==0)
								{
									p_ActionNameLength = ILibReadInnerXML(xnode, &p_ActionName);
									p_ActionName[p_ActionNameLength]=0;
									OK |= 4;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 7)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_DeviceUDNLength = ILibInPlaceXmlUnEscape(p_DeviceUDN);
	_DeviceUDN = p_DeviceUDN;
	_ServiceIdLength = ILibInPlaceXmlUnEscape(p_ServiceId);
	_ServiceId = p_ServiceId;
	_ActionNameLength = ILibInPlaceXmlUnEscape(p_ActionName);
	_ActionName = p_ActionName;
	if (DVFP_DeviceProtection_GetRolesForAction == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_GetRolesForAction((void*)ReaderObject,_DeviceUDN,_ServiceId,_ActionName);
}

#define DVDispatch_DeviceProtection_GetSupportedProtocols(buffer,offset,bufferLength, session)\
{\
	if (DVFP_DeviceProtection_GetSupportedProtocols == NULL)\
	DVResponse_Error(session,501,"No Function Handler");\
	else\
	DVFP_DeviceProtection_GetSupportedProtocols((void*)session);\
}

void DVDispatch_DeviceProtection_GetUserLoginChallenge(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ProtocolType = NULL;
	int p_ProtocolTypeLength = 0;
	char* _ProtocolType = "";
	int _ProtocolTypeLength;
	char *p_Name = NULL;
	int p_NameLength = 0;
	char* _Name = "";
	int _NameLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 21 && memcmp(xnode->Name, "GetUserLoginChallenge",21) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 12 && memcmp(xnode->Name, "ProtocolType",12)==0)
								{
									p_ProtocolTypeLength = ILibReadInnerXML(xnode, &p_ProtocolType);
									p_ProtocolType[p_ProtocolTypeLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 4 && memcmp(xnode->Name, "Name",4)==0)
								{
									p_NameLength = ILibReadInnerXML(xnode, &p_Name);
									p_Name[p_NameLength]=0;
									OK |= 2;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 3)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_ProtocolTypeLength = ILibInPlaceXmlUnEscape(p_ProtocolType);
	_ProtocolType = p_ProtocolType;
	_NameLength = ILibInPlaceXmlUnEscape(p_Name);
	_Name = p_Name;
	if (DVFP_DeviceProtection_GetUserLoginChallenge == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_GetUserLoginChallenge((void*)ReaderObject,_ProtocolType,_Name);
}

void DVDispatch_DeviceProtection_RemoveIdentity(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_Identity = NULL;
	int p_IdentityLength = 0;
	char* _Identity = "";
	int _IdentityLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 14 && memcmp(xnode->Name, "RemoveIdentity",14) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 8 && memcmp(xnode->Name, "Identity",8)==0)
								{
									p_IdentityLength = ILibReadInnerXML(xnode, &p_Identity);
									p_Identity[p_IdentityLength]=0;
									OK |= 1;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 1)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_IdentityLength = ILibInPlaceXmlUnEscape(p_Identity);
	_Identity = p_Identity;
	if (DVFP_DeviceProtection_RemoveIdentity == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_RemoveIdentity((void*)ReaderObject,_Identity);
}

void DVDispatch_DeviceProtection_RemoveRolesForIdentity(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_Identity = NULL;
	int p_IdentityLength = 0;
	char* _Identity = "";
	int _IdentityLength;
	char *p_RoleList = NULL;
	int p_RoleListLength = 0;
	char* _RoleList = "";
	int _RoleListLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 22 && memcmp(xnode->Name, "RemoveRolesForIdentity",22) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 8 && memcmp(xnode->Name, "Identity",8)==0)
								{
									p_IdentityLength = ILibReadInnerXML(xnode, &p_Identity);
									p_Identity[p_IdentityLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 8 && memcmp(xnode->Name, "RoleList",8)==0)
								{
									p_RoleListLength = ILibReadInnerXML(xnode, &p_RoleList);
									p_RoleList[p_RoleListLength]=0;
									OK |= 2;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 3)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_IdentityLength = ILibInPlaceXmlUnEscape(p_Identity);
	_Identity = p_Identity;
	_RoleListLength = ILibInPlaceXmlUnEscape(p_RoleList);
	_RoleList = p_RoleList;
	if (DVFP_DeviceProtection_RemoveRolesForIdentity == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_RemoveRolesForIdentity((void*)ReaderObject,_Identity,_RoleList);
}

void DVDispatch_DeviceProtection_SendSetupMessage(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ProtocolType = NULL;
	int p_ProtocolTypeLength = 0;
	char* _ProtocolType = "";
	int _ProtocolTypeLength;
	char *p_InMessage = NULL;
	int p_InMessageLength = 0;
	unsigned char* _InMessage = NULL;
	int _InMessageLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 16 && memcmp(xnode->Name, "SendSetupMessage",16) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 12 && memcmp(xnode->Name, "ProtocolType",12)==0)
								{
									p_ProtocolTypeLength = ILibReadInnerXML(xnode, &p_ProtocolType);
									p_ProtocolType[p_ProtocolTypeLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 9 && memcmp(xnode->Name, "InMessage",9)==0)
								{
									p_InMessageLength = ILibReadInnerXML(xnode, &p_InMessage);
									OK |= 2;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 3)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_ProtocolTypeLength = ILibInPlaceXmlUnEscape(p_ProtocolType);
	_ProtocolType = p_ProtocolType;
	_InMessageLength = ILibBase64Decode(p_InMessage,p_InMessageLength,&_InMessage);
	if (DVFP_DeviceProtection_SendSetupMessage == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_SendSetupMessage((void*)ReaderObject,_ProtocolType,_InMessage,_InMessageLength);
	free(_InMessage);
}

void DVDispatch_DeviceProtection_SetUserLoginPassword(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ProtocolType = NULL;
	int p_ProtocolTypeLength = 0;
	char* _ProtocolType = "";
	int _ProtocolTypeLength;
	char *p_Name = NULL;
	int p_NameLength = 0;
	char* _Name = "";
	int _NameLength;
	char *p_Stored = NULL;
	int p_StoredLength = 0;
	unsigned char* _Stored = NULL;
	int _StoredLength;
	char *p_Salt = NULL;
	int p_SaltLength = 0;
	unsigned char* _Salt = NULL;
	int _SaltLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 20 && memcmp(xnode->Name, "SetUserLoginPassword",20) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 12 && memcmp(xnode->Name, "ProtocolType",12)==0)
								{
									p_ProtocolTypeLength = ILibReadInnerXML(xnode, &p_ProtocolType);
									p_ProtocolType[p_ProtocolTypeLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 4 && memcmp(xnode->Name, "Name",4)==0)
								{
									p_NameLength = ILibReadInnerXML(xnode, &p_Name);
									p_Name[p_NameLength]=0;
									OK |= 2;
								}
								else if (xnode->NameLength == 6 && memcmp(xnode->Name, "Stored",6)==0)
								{
									p_StoredLength = ILibReadInnerXML(xnode, &p_Stored);
									OK |= 4;
								}
								else if (xnode->NameLength == 4 && memcmp(xnode->Name, "Salt",4)==0)
								{
									p_SaltLength = ILibReadInnerXML(xnode, &p_Salt);
									OK |= 8;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 15)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_ProtocolTypeLength = ILibInPlaceXmlUnEscape(p_ProtocolType);
	_ProtocolType = p_ProtocolType;
	_NameLength = ILibInPlaceXmlUnEscape(p_Name);
	_Name = p_Name;
	_StoredLength = ILibBase64Decode(p_Stored,p_StoredLength,&_Stored);
	_SaltLength = ILibBase64Decode(p_Salt,p_SaltLength,&_Salt);
	if (DVFP_DeviceProtection_SetUserLoginPassword == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_SetUserLoginPassword((void*)ReaderObject,_ProtocolType,_Name,_Stored,_StoredLength,_Salt,_SaltLength);
	free(_Stored);
	free(_Salt);
}

void DVDispatch_DeviceProtection_UserLogin(char *buffer, int offset, int bufferLength, struct ILibWebServer_Session *ReaderObject)
{
	int OK = 0;
	char *p_ProtocolType = NULL;
	int p_ProtocolTypeLength = 0;
	char* _ProtocolType = "";
	int _ProtocolTypeLength;
	char *p_Challenge = NULL;
	int p_ChallengeLength = 0;
	unsigned char* _Challenge = NULL;
	int _ChallengeLength;
	char *p_Authenticator = NULL;
	int p_AuthenticatorLength = 0;
	unsigned char* _Authenticator = NULL;
	int _AuthenticatorLength;
	struct ILibXMLNode *xnode = ILibParseXML(buffer, offset, bufferLength);
	struct ILibXMLNode *root = xnode;
	if (ILibProcessXMLNodeList(root)!=0)
	{
		/* The XML is not well formed! */
		ILibDestructXMLNodeList(root);
		DVResponse_Error(ReaderObject, 501, "Invalid XML");
		return;
	}
	while(xnode != NULL)
	{
		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, "Envelope", 8)==0)
		{
			// Envelope
			xnode = xnode->Next;
			while(xnode != NULL)
			{
				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, "Body", 4) == 0)
				{
					// Body
					xnode = xnode->Next;
					while(xnode != NULL)
					{
						if (xnode->StartTag != 0 && xnode->NameLength == 9 && memcmp(xnode->Name, "UserLogin",9) == 0)
						{
							// Inside the interesting part of the SOAP
							xnode = xnode->Next;
							while(xnode != NULL)
							{
								if (xnode->NameLength == 12 && memcmp(xnode->Name, "ProtocolType",12)==0)
								{
									p_ProtocolTypeLength = ILibReadInnerXML(xnode, &p_ProtocolType);
									p_ProtocolType[p_ProtocolTypeLength]=0;
									OK |= 1;
								}
								else if (xnode->NameLength == 9 && memcmp(xnode->Name, "Challenge",9)==0)
								{
									p_ChallengeLength = ILibReadInnerXML(xnode, &p_Challenge);
									OK |= 2;
								}
								else if (xnode->NameLength == 13 && memcmp(xnode->Name, "Authenticator",13)==0)
								{
									p_AuthenticatorLength = ILibReadInnerXML(xnode, &p_Authenticator);
									OK |= 4;
								}
								if (xnode->Peer == NULL)
								{
									xnode = xnode->Parent;
									break;
								}
								else
								{
									xnode = xnode->Peer;
								}
							}
						}
						if (xnode != NULL)
						{
							if (xnode->Peer == NULL)
							{
								xnode = xnode->Parent;
								break;
							}
							else
							{
								xnode = xnode->Peer;
							}
						}
					}
				}
				if (xnode != NULL)
				{
					if (xnode->Peer == NULL)
					{
						xnode = xnode->Parent;
						break;
					}
					else
					{
						xnode = xnode->Peer;
					}
				}
			}
		}
		if (xnode != NULL){xnode = xnode->Peer;}
	}
	ILibDestructXMLNodeList(root);
	if (OK != 7)
	{
		DVResponse_Error(ReaderObject, 402, "Illegal value");
		return;
	}
	
	/* Type Checking */
	_ProtocolTypeLength = ILibInPlaceXmlUnEscape(p_ProtocolType);
	_ProtocolType = p_ProtocolType;
	_ChallengeLength = ILibBase64Decode(p_Challenge,p_ChallengeLength,&_Challenge);
	_AuthenticatorLength = ILibBase64Decode(p_Authenticator,p_AuthenticatorLength,&_Authenticator);
	if (DVFP_DeviceProtection_UserLogin == NULL)
	DVResponse_Error(ReaderObject,501,"No Function Handler");
	else
	DVFP_DeviceProtection_UserLogin((void*)ReaderObject,_ProtocolType,_Challenge,_ChallengeLength,_Authenticator,_AuthenticatorLength);
	free(_Challenge);
	free(_Authenticator);
}

#define DVDispatch_DeviceProtection_UserLogout(buffer,offset,bufferLength, session)\
{\
	if (DVFP_DeviceProtection_UserLogout == NULL)\
	DVResponse_Error(session,501,"No Function Handler");\
	else\
	DVFP_DeviceProtection_UserLogout((void*)session);\
}



int DVProcessPOST(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
{
	struct packetheader_field_node *f = header->FirstField;
	char* HOST;
	char* SOAPACTION = NULL;
	int SOAPACTIONLength = 0;
	struct parser_result *r, *r2;
	struct parser_result_field *prf;
	int RetVal = 0;
	
	// Iterate through all the HTTP Headers
	while(f!=NULL)
	{
		if (f->FieldLength == 4 && strncasecmp(f->Field, "HOST", 4) == 0)
		{
			HOST = f->FieldData;
		}
		else if (f->FieldLength == 10 && strncasecmp(f->Field, "SOAPACTION", 10) == 0)
		{
			r = ILibParseString(f->FieldData, 0, f->FieldDataLength, "#", 1);
			SOAPACTION = r->LastResult->data;
			SOAPACTIONLength = r->LastResult->datalength - 1;
			ILibDestructParserResults(r);
		}
		else if (f->FieldLength == 10 && strncasecmp(f->Field, "USER-AGENT", 10) == 0)
		{
			// Check UPnP version of the Control Point which invoked us
			r = ILibParseString(f->FieldData, 0, f->FieldDataLength, " ", 1);
			prf = r->FirstResult;
			while(prf!=NULL)
			{
				if (prf->datalength>5 && memcmp(prf->data, "UPnP/", 5)==0)
				{
					r2 = ILibParseString(prf->data + 5, 0, prf->datalength - 5, ".", 1);
					r2->FirstResult->data[r2->FirstResult->datalength] = 0;
					r2->LastResult->data[r2->LastResult->datalength] = 0;
					if (atoi(r2->FirstResult->data) == 1 && atoi(r2->LastResult->data) > 0)
					{
						session->Reserved9 = 1;
					}
					ILibDestructParserResults(r2);
				}
				prf = prf->NextResult;
			}
			ILibDestructParserResults(r);
		}
		f = f->NextField;
	}
	if (header->DirectiveObjLength==25 && memcmp((header->DirectiveObj)+1,"DeviceProtection/control",24)==0)
	{
		if (SOAPACTIONLength==15 && memcmp(SOAPACTION,"AddIdentityList",15)==0)
		{
			DVDispatch_DeviceProtection_AddIdentityList(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==19 && memcmp(SOAPACTION,"AddRolesForIdentity",19)==0)
		{
			DVDispatch_DeviceProtection_AddRolesForIdentity(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==10 && memcmp(SOAPACTION,"GetACLData",10)==0)
		{
			DVDispatch_DeviceProtection_GetACLData(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==16 && memcmp(SOAPACTION,"GetAssignedRoles",16)==0)
		{
			DVDispatch_DeviceProtection_GetAssignedRoles(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==17 && memcmp(SOAPACTION,"GetRolesForAction",17)==0)
		{
			DVDispatch_DeviceProtection_GetRolesForAction(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetSupportedProtocols",21)==0)
		{
			DVDispatch_DeviceProtection_GetSupportedProtocols(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetUserLoginChallenge",21)==0)
		{
			DVDispatch_DeviceProtection_GetUserLoginChallenge(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==14 && memcmp(SOAPACTION,"RemoveIdentity",14)==0)
		{
			DVDispatch_DeviceProtection_RemoveIdentity(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==22 && memcmp(SOAPACTION,"RemoveRolesForIdentity",22)==0)
		{
			DVDispatch_DeviceProtection_RemoveRolesForIdentity(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==16 && memcmp(SOAPACTION,"SendSetupMessage",16)==0)
		{
			DVDispatch_DeviceProtection_SendSetupMessage(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==20 && memcmp(SOAPACTION,"SetUserLoginPassword",20)==0)
		{
			DVDispatch_DeviceProtection_SetUserLoginPassword(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==9 && memcmp(SOAPACTION,"UserLogin",9)==0)
		{
			DVDispatch_DeviceProtection_UserLogin(bodyBuffer, offset, bodyBufferLength, session);
		}
		else if (SOAPACTIONLength==10 && memcmp(SOAPACTION,"UserLogout",10)==0)
		{
			DVDispatch_DeviceProtection_UserLogout(bodyBuffer, offset, bodyBufferLength, session);
		}
		else
		{
			RetVal=1;
		}
	}
	else
	{
		RetVal=1;
	}
	
	return(RetVal);
}
struct SubscriberInfo* DVRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers, char* SID, int SIDLength)
{
	struct SubscriberInfo *info = *Head;
	while(info != NULL)
	{
		if (info->SIDLength == SIDLength && memcmp(info->SID, SID, SIDLength) == 0)
		{
			if (info->Previous) info->Previous->Next = info->Next; else *Head = info->Next;
			if (info->Next) info->Next->Previous = info->Previous;
			break;
		}
		info = info->Next;
	}
	if (info != NULL)
	{
		info->Previous = NULL;
		info->Next = NULL;
		--(*TotalSubscribers);
	}
	return(info);
}

#define DVDestructSubscriberInfo(info)\
{\
	free(info->Path);\
	free(info->SID);\
	free(info);\
}

#define DVDestructEventObject(EvObject)\
{\
	free(EvObject->PacketBody);\
	free(EvObject);\
}

#define DVDestructEventDataObject(EvData)\
{\
	free(EvData);\
}

void DVExpireSubscriberInfo(struct DVDataObject *d, struct SubscriberInfo *info)
{
	struct SubscriberInfo *t = info;
	while(t->Previous != NULL) { t = t->Previous; }
	if (d->HeadSubscriberPtr_DeviceProtection==t)
	{
		--(d->NumberOfSubscribers_DeviceProtection);
	}
	
	
	if (info->Previous != NULL)
	{
		// This is not the Head
		info->Previous->Next = info->Next;
		if (info->Next != NULL) { info->Next->Previous = info->Previous; }
	}
	else
	{
		// This is the Head
		if (d->HeadSubscriberPtr_DeviceProtection==info)
		{
			d->HeadSubscriberPtr_DeviceProtection = info->Next;
			if (info->Next!=NULL)
			{
				info->Next->Previous = NULL;
			}
		}
		else 
		{
			// Error
			return;
		}
		
	}
	--info->RefCount;
	if (info->RefCount == 0) { DVDestructSubscriberInfo(info); }
}

int DVSubscriptionExpired(struct SubscriberInfo *info)
{
	int RetVal = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ((info->RenewByTime).tv_sec < tv.tv_sec) { RetVal = -1; }
	return(RetVal);
}

void DVGetInitialEventBody_DeviceProtection(struct DVDataObject *DVObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(25+(int)strlen(DVObject->DeviceProtection_SetupReady));
	if ((*body = (char*)malloc(sizeof(char) * TempLength)) == NULL) ILIBCRITICALEXIT(254);
	*bodylength = snprintf(*body, sizeof(char) * TempLength, "SetupReady>%s</SetupReady",DVObject->DeviceProtection_SetupReady);
}


void DVProcessUNSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
	char* SID = NULL;
	int SIDLength = 0;
	struct SubscriberInfo *Info;
	struct packetheader_field_node *f;
	char* packet;
	int packetlength;
	if ((packet = (char*)malloc(50)) == NULL) ILIBCRITICALEXIT(254);
	
	// Iterate through all the HTTP headers
	f = header->FirstField;
	while(f != NULL)
	{
		if (f->FieldLength == 3)
		{
			if (strncasecmp(f->Field, "SID", 3) == 0)
			{
				// Get the Subscription ID
				SID = f->FieldData;
				SIDLength = f->FieldDataLength;
			}
		}
		f = f->NextField;
	}
	sem_wait(&(((struct DVDataObject*)session->User)->EventLock));
	if (header->DirectiveObjLength==23 && memcmp(header->DirectiveObj + 1,"DeviceProtection/event",22)==0)
	{
		Info = DVRemoveSubscriberInfo(&(((struct DVDataObject*)session->User)->HeadSubscriberPtr_DeviceProtection),&(((struct DVDataObject*)session->User)->NumberOfSubscribers_DeviceProtection),SID,SIDLength);
		if (Info != NULL)
		{
			--Info->RefCount;
			if (Info->RefCount == 0)
			{
				DVDestructSubscriberInfo(Info);
			}
			packetlength = snprintf(packet, 50, "HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n", 200, "OK");
			ILibWebServer_Send_Raw(session, packet, packetlength, 0, 1);
		}
		else
		{
			packetlength = snprintf(packet, 50, "HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n", 412, "Invalid SID");
			ILibWebServer_Send_Raw(session, packet, packetlength, 0, 1);
		}
	}
	
	sem_post(&(((struct DVDataObject*)session->User)->EventLock));
}

void DVTryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength, struct ILibWebServer_Session *session)
{
	int *TotalSubscribers = NULL;
	struct SubscriberInfo **HeadPtr = NULL;
	struct SubscriberInfo *NewSubscriber, *TempSubscriber;
	int SIDNumber, rnumber;
	char *SID;
	char *TempString;
	int TempStringLength;
	char *TempString2;
	long TempLong;
	char *packet;
	int packetlength;
	char* path;
	size_t len;
	
	char* escapedURI;
	int escapedURILength;
	
	char *packetbody = NULL;
	int packetbodyLength = 0;
	
	struct parser_result *p;
	struct parser_result *p2;
	
	struct DVDataObject *dataObject = (struct DVDataObject*)session->User;
	
	if (strncmp(ServiceName,"DeviceProtection",16)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_DeviceProtection);
		HeadPtr = &(dataObject->HeadSubscriberPtr_DeviceProtection);
	}
	
	
	if (*HeadPtr!=NULL)
	{
		NewSubscriber = *HeadPtr;
		while(NewSubscriber != NULL)
		{
			if (DVSubscriptionExpired(NewSubscriber) != 0)
			{
				TempSubscriber = NewSubscriber->Next;
				NewSubscriber = DVRemoveSubscriberInfo(HeadPtr, TotalSubscribers, NewSubscriber->SID, NewSubscriber->SIDLength);
				DVDestructSubscriberInfo(NewSubscriber);
				NewSubscriber = TempSubscriber;
			}
			else
			{
				NewSubscriber = NewSubscriber->Next;
			}
		}
	}
	
	// The Maximum number of subscribers can be bounded
	if (*TotalSubscribers < 10)
	{
		if ((NewSubscriber = (struct SubscriberInfo*)malloc(sizeof(struct SubscriberInfo))) == NULL) ILIBCRITICALEXIT(254);
		memset(NewSubscriber, 0, sizeof(struct SubscriberInfo));
		
		// The SID must be globally unique, so lets generate it using a bunch of random hex characters
		if ((SID = (char*)malloc(43)) == NULL) ILIBCRITICALEXIT(254);
		memset(SID, 0, 38);
		snprintf(SID, 43, "uuid:");
		for(SIDNumber = 5; SIDNumber <= 12; ++SIDNumber)
		{
			rnumber = rand() % 16;
			snprintf(SID + SIDNumber, 43 - SIDNumber, "%x", rnumber);
		}
		snprintf(SID + SIDNumber, 43 - SIDNumber, "-");
		for(SIDNumber = 14; SIDNumber <= 17; ++SIDNumber)
		{
			rnumber = rand()%16;
			snprintf(SID + SIDNumber, 43 - SIDNumber, "%x", rnumber);
		}
		snprintf(SID + SIDNumber, 43 - SIDNumber, "-");
		for(SIDNumber = 19; SIDNumber <= 22; ++SIDNumber)
		{
			rnumber = rand()%16;
			snprintf(SID + SIDNumber, 43 - SIDNumber, "%x", rnumber);
		}
		snprintf(SID + SIDNumber, 43 - SIDNumber, "-");
		for(SIDNumber = 24; SIDNumber <= 27; ++SIDNumber)
		{
			rnumber = rand()%16;
			snprintf(SID + SIDNumber, 43 - SIDNumber, "%x", rnumber);
		}
		snprintf(SID + SIDNumber, 43 - SIDNumber, "-");
		for(SIDNumber = 29; SIDNumber <= 40; ++SIDNumber)
		{
			rnumber = rand()%16;
			snprintf(SID + SIDNumber, 43 - SIDNumber, "%x", rnumber);
		}
		
		p = ILibParseString(URL, 0, URLLength, "://", 3);
		if (p->NumResults == 1)
		{
			ILibWebServer_Send_Raw(session, "HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n", 55, 1, 1);
			ILibDestructParserResults(p);
			return;
		}
		TempString = p->LastResult->data;
		TempStringLength = p->LastResult->datalength;
		ILibDestructParserResults(p);
		p = ILibParseString(TempString, 0, TempStringLength,"/", 1);
		p2 = ILibParseString(p->FirstResult->data, 0, p->FirstResult->datalength, ":", 1);
		if ((TempString2 = (char*)malloc(1 + sizeof(char) * p2->FirstResult->datalength)) == NULL) ILIBCRITICALEXIT(254);
		memcpy(TempString2, p2->FirstResult->data, p2->FirstResult->datalength);
		TempString2[p2->FirstResult->datalength] = '\0';
		NewSubscriber->Address = inet_addr(TempString2);
		if (p2->NumResults == 1)
		{
			NewSubscriber->Port = 80;
			if ((path = (char*)malloc(1+TempStringLength - p2->FirstResult->datalength -1)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(path,TempString + p2->FirstResult->datalength,TempStringLength - p2->FirstResult->datalength -1);
			path[TempStringLength - p2->FirstResult->datalength - 1] = '\0';
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		else
		{
			ILibGetLong(p2->LastResult->data,p2->LastResult->datalength,&TempLong);
			NewSubscriber->Port = (unsigned short)TempLong;
			if (TempStringLength == p->FirstResult->datalength)
			{
				if ((path = (char*)malloc(2)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(path, "/", 1);
				path[1] = '\0';
			}
			else
			{
				if ((path = (char*)malloc(1 + TempStringLength - p->FirstResult->datalength - 1)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(path,TempString + p->FirstResult->datalength,TempStringLength - p->FirstResult->datalength - 1);
				path[TempStringLength - p->FirstResult->datalength - 1] = '\0';
			}
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		ILibDestructParserResults(p);
		ILibDestructParserResults(p2);
		free(TempString2);
		
		if ((escapedURI = (char*)malloc(ILibHTTPEscapeLength(NewSubscriber->Path))) == NULL) ILIBCRITICALEXIT(254);
		escapedURILength = ILibHTTPEscape(escapedURI, NewSubscriber->Path);
		
		free(NewSubscriber->Path);
		NewSubscriber->Path = escapedURI;
		NewSubscriber->PathLength = escapedURILength;
		
		NewSubscriber->RefCount = 1;
		NewSubscriber->Disposing = 0;
		NewSubscriber->Previous = NULL;
		NewSubscriber->SID = SID;
		NewSubscriber->SIDLength = (int)strlen(SID);
		NewSubscriber->SEQ = 0;
		
		// Determine what the subscription renewal cycle is
		gettimeofday(&(NewSubscriber->RenewByTime),NULL);
		(NewSubscriber->RenewByTime).tv_sec += (int)Timeout;
		
		NewSubscriber->Next = *HeadPtr;
		if (*HeadPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}
		*HeadPtr = NewSubscriber;
		++(*TotalSubscribers);
		LVL3DEBUG(printf("\r\n\r\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\r\n",NewSubscriber->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)
		LVL3DEBUG(printf("TIMESTAMP: %d <%d>\r\n\r\n",(NewSubscriber->RenewByTime).tv_sec-Timeout,NewSubscriber);)
		
		len = 134 + (int)strlen(SID) + (int)strlen(DVPLATFORM) + 4;
		if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		packetlength = snprintf(packet, len, "HTTP/1.1 200 OK\r\nSERVER: %s, UPnP/1.0, MicroStack/1.0.3951\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",DVPLATFORM,SID,Timeout);
		if (strcmp(ServiceName,"DeviceProtection")==0)
		{
			DVGetInitialEventBody_DeviceProtection(dataObject,&packetbody,&packetbodyLength);
		}
		
		if (packetbody != NULL)
		{
			ILibWebServer_Send_Raw(session, packet, packetlength, 0, 1);
			DVSendEvent_Body(dataObject, packetbody, packetbodyLength, NewSubscriber);
			free(packetbody);
		} 
	}
	else
	{
		// Too many subscribers
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Too Many Subscribers\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}
}

void DVSubscribeEvents(char* path, int pathlength, char* Timeout, int TimeoutLength, char* URL, int URLLength, struct ILibWebServer_Session* session)
{
	long TimeoutVal;
	char* buffer;
	if ((buffer = (char*)malloc(1 + sizeof(char)*pathlength)) == NULL) ILIBCRITICALEXIT(254);
	
	ILibGetLong(Timeout, TimeoutLength, &TimeoutVal);
	memcpy(buffer, path, pathlength);
	buffer[pathlength] = '\0';
	free(buffer);
	if (TimeoutVal>DV_MAX_SUBSCRIPTION_TIMEOUT) { TimeoutVal = DV_MAX_SUBSCRIPTION_TIMEOUT; }
	if (pathlength==23 && memcmp(path+1,"DeviceProtection/event",22)==0)
	{
		DVTryToSubscribe("DeviceProtection",TimeoutVal,URL,URLLength,session);
	}
	else
	{
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Invalid Service Name\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}
	
}

void DVRenewEvents(char* path, int pathlength, char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct ILibWebServer_Session *ReaderObject)
{
	struct SubscriberInfo *info = NULL;
	long TimeoutVal;
	struct timeval tv;
	char* packet;
	int packetlength;
	char* SID;
	size_t len;
	
	if ((SID = (char*)malloc(SIDLength + 1)) == NULL) ILIBCRITICALEXIT(254);
	memcpy(SID, _SID, SIDLength);
	SID[SIDLength] = '\0';
	
	LVL3DEBUG(gettimeofday(&tv, NULL);)
	LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n", tv.tv_sec);)
	LVL3DEBUG(printf("SUBSCRIBER [%s] attempting to Renew Events for %s Duration [", SID, Timeout);)
	if (pathlength==23 && memcmp(path+1,"DeviceProtection/event",22)==0)
	{
		info = ((struct DVDataObject*)ReaderObject->User)->HeadSubscriberPtr_DeviceProtection;
	}
	
	
	// Find this SID in the subscriber list, and recalculate the expiration timeout
	while(info != NULL && strcmp(info->SID,SID) != 0)
	{
		info = info->Next;
	}
	if (info != NULL)
	{
		ILibGetLong(Timeout, TimeoutLength, &TimeoutVal);
		if (TimeoutVal>DV_MAX_SUBSCRIPTION_TIMEOUT) {TimeoutVal = DV_MAX_SUBSCRIPTION_TIMEOUT;}
		
		gettimeofday(&tv,NULL);
		(info->RenewByTime).tv_sec = tv.tv_sec + TimeoutVal;
		
		len = 134 + strlen(SID) + 4;
		if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		packetlength = snprintf(packet, len, "HTTP/1.1 200 OK\r\nSERVER: %s, UPnP/1.0, MicroStack/1.0.3951\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",DVPLATFORM,SID,TimeoutVal);
		ILibWebServer_Send_Raw(ReaderObject, packet, packetlength, 0, 1);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n", TimeoutVal, info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		ILibWebServer_Send_Raw(ReaderObject, "HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n", 55, 1, 1);
	}
	free(SID);
}

void DVProcessSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
	char* SID = NULL;
	int SIDLength = 0;
	char* Timeout = NULL;
	int TimeoutLength = 0;
	char* URL = NULL;
	int URLLength = 0;
	struct parser_result *p;
	struct packetheader_field_node *f;
	
	// Iterate through all the HTTP Headers
	f = header->FirstField;
	while(f!=NULL)
	{
		if (f->FieldLength==3 && strncasecmp(f->Field, "SID", 3) == 0)
		{
			// Get the Subscription ID
			SID = f->FieldData;
			SIDLength = f->FieldDataLength;
		}
		else if (f->FieldLength == 8 && strncasecmp(f->Field, "Callback", 8) == 0)
		{
			// Get the Callback URL
			URL = f->FieldData;
			URLLength = f->FieldDataLength;
		}
		else if (f->FieldLength == 7 && strncasecmp(f->Field, "Timeout", 7) == 0)
		{
			// Get the requested timeout value
			Timeout = f->FieldData;
			TimeoutLength = f->FieldDataLength;
		}
		f = f->NextField;
	}
	if (Timeout == NULL)
	{
		// It a timeout wasn't specified, force it to a specific value
		Timeout = "7200";
		TimeoutLength = 4;
	}
	else
	{
		p = ILibParseString(Timeout, 0, TimeoutLength, "-", 1);
		if (p->NumResults == 2)
		{
			Timeout = p->LastResult->data;
			TimeoutLength = p->LastResult->datalength;
			if (TimeoutLength == 8 && strncasecmp(Timeout, "INFINITE", 8) == 0)
			{
				// Infinite timeouts will cause problems, so we don't allow it
				Timeout = "7200";
				TimeoutLength = 4;
			}
		}
		else
		{
			Timeout = "7200";
			TimeoutLength = 4;
		}
		ILibDestructParserResults(p);
	}
	if (SID == NULL)
	{
		// If not SID was specified, this is a subscription request
		// Subscribe
		DVSubscribeEvents(header->DirectiveObj, header->DirectiveObjLength, Timeout, TimeoutLength, URL, URLLength, session);
	}
	else
	{
		// If a SID was specified, it is a renewal request for an existing subscription
		// Renew
		DVRenewEvents(header->DirectiveObj, header->DirectiveObjLength, SID, SIDLength, Timeout, TimeoutLength, session);
	}
}
void DVProcessHTTPPacket(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
{
	struct DVDataObject *dataObject = (struct DVDataObject*)session->User;
	#if defined(WIN32) || defined(_WIN32_WCE)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: WINDOWS, UPnP/1.0, MicroStack/1.0.3951";
	#elif defined(__SYMBIAN32__)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: SYMBIAN, UPnP/1.0, MicroStack/1.0.3951";
	#else
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: POSIX, UPnP/1.0, MicroStack/1.0.3951";
	#endif
	char *errorTemplate = "HTTP/1.1 %d %s\r\nServer: %s, UPnP/1.0, MicroStack/1.0.3951\r\nContent-Length: 0\r\n\r\n";
	char *errorPacket;
	int errorPacketLength;
	char *buffer;
	
	LVL3DEBUG(errorPacketLength = ILibGetRawPacket(header, &errorPacket);)
	LVL3DEBUG(printf("%s\r\n",errorPacket);)
	LVL3DEBUG(free(errorPacket);)			
	
	
	if (header->DirectiveLength == 4 && memcmp(header->Directive,"HEAD", 4) == 0)
	{
		if (header->DirectiveObjLength == 1 && memcmp(header->DirectiveObj, "/", 1) == 0)
		{
			// A HEAD request for the device description document.
			// We stream the document back, so we don't return content length or anything because the actual response won't have it either
			ILibWebServer_StreamHeader_Raw(session, 200, "OK", responseHeader, 1);
			ILibWebServer_StreamBody(session, NULL, 0, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"DeviceProtection/scpd.xml",25)==0)
		{
			ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
			ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
		}
		
		else
		{
			// A HEAD request for something we don't have
			if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
			errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 404, "File Not Found", DVPLATFORM);
			ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, 0, 1);
		}
	}
	else if (header->DirectiveLength == 3 && memcmp(header->Directive, "GET", 3) == 0)
	{
		if (header->DirectiveObjLength == 1 && memcmp(header->DirectiveObj,"/", 1) == 0)
		{
			// A GET Request for the device description document, so lets stream it back to the client
			ILibWebServer_StreamHeader_Raw(session, 200, "OK", responseHeader, 1);
			ILibWebServer_StreamBody(session, dataObject->DeviceDescription, dataObject->DeviceDescriptionLength, 1, 1);
		}
		else if (header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"DeviceProtection/scpd.xml",25)==0)
		{
			buffer = ILibDecompressString((unsigned char*)DVDeviceProtectionDescription,DVDeviceProtectionDescriptionLength,DVDeviceProtectionDescriptionLengthUX);
			ILibWebServer_Send_Raw(session,buffer,DVDeviceProtectionDescriptionLengthUX,0,1);
		}
		
		else
		{
			// A GET Request for something we don't have
			if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
			errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 404, "File Not Found", DVPLATFORM);
			ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, 0, 1);
		}
	}
	else if (header->DirectiveLength == 4 && memcmp(header->Directive,"POST",4) == 0)
	{
		// Defer Control to the POST Handler
		if (DVProcessPOST(session, header, bodyBuffer, offset, bodyBufferLength) != 0)
		{
			// A POST for an action that doesn't exist
			DVResponse_Error(session, 401, "Invalid Action");
		}
	}
	else if (header->DirectiveLength == 9 && memcmp(header->Directive, "SUBSCRIBE" ,9) == 0)
	{
		// Subscription Handler
		DVProcessSUBSCRIBE(header,session);
	}
	else if (header->DirectiveLength == 11 && memcmp(header->Directive, "UNSUBSCRIBE", 11) == 0)
	{
		// UnSubscribe Handler
		DVProcessUNSUBSCRIBE(header,session);
	}	else
	{
		// The client tried something we didn't expect/support
		if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
		errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 400, "Bad Request", DVPLATFORM);
		ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, ILibAsyncSocket_MemoryOwnership_CHAIN, 1);
	}
}
void DVFragmentedSendNotify_Destroy(void *data);
void DVMasterPreSelect(void* object, void *socketset, void *writeset, void *errorset, int* blocktime)
{
	int i;
	struct DVDataObject *DVObject = (struct DVDataObject*)object;	
	struct DVFragmentNotifyStruct *f;
	int timeout;
	UNREFERENCED_PARAMETER( socketset );
	UNREFERENCED_PARAMETER( writeset );
	UNREFERENCED_PARAMETER( errorset );
	UNREFERENCED_PARAMETER( blocktime );
	
	if (DVObject->InitialNotify == 0)
	{
		// The initial "HELLO" packets were not sent yet, so lets send them
		DVObject->InitialNotify = -1;
		
		// In case we were interrupted, we need to flush out the caches of
		// all the control points by sending a "byebye" first, to insure
		// control points don't ignore our "hello" packets thinking they are just
		// periodic re-advertisements.
		//* vbl disable this feature for Cert tool */ DVSendByeBye(DVObject);
		
		// PacketNumber 0 is the controller, for the rest of the packets. Send
		// one of these to send out an advertisement "group"
		if ((f = (struct DVFragmentNotifyStruct*)malloc(sizeof(struct DVFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
		f->packetNumber = 0;
		f->upnp = DVObject;
		
		// We need to inject some delay in these packets to space them out,
		// otherwise we could overflow the inbound buffer of the recipient, causing them
		// to lose packets. And UPnP/1.0 control points are not as robust as UPnP/1.1 control points,
		// so they need all the help they can get ;)
		timeout = (int)(0 + ((unsigned short)rand() % (500)));
		do { f->upnp->InitialNotify = rand(); } while (f->upnp->InitialNotify == 0);
		
		// Register for the timed callback, to actually send the packet
		ILibLifeTime_AddEx(f->upnp->WebServerTimer, f, timeout, &DVFragmentedSendNotify, &DVFragmentedSendNotify_Destroy);
	}
	
	if (DVObject->UpdateFlag != 0)
	{
		// Somebody told us that we should recheck our IP Address table, as one of them may have changed
		DVObject->UpdateFlag = 0;
		
		// Clear Sockets
		// Iterate through all the currently bound IPv4 addresses and release the sockets
		if (DVObject->AddressListV4 != NULL)
		{
			for (i = 0; i < DVObject->AddressListV4Length; ++i) ILibChain_SafeRemove(DVObject->Chain, DVObject->NOTIFY_SEND_socks[i]);
			free(DVObject->NOTIFY_SEND_socks);
			for (i = 0; i < DVObject->AddressListV4Length; ++i) ILibChain_SafeRemove(DVObject->Chain, DVObject->NOTIFY_RECEIVE_socks[i]);
			free(DVObject->NOTIFY_RECEIVE_socks);
			free(DVObject->AddressListV4);
		}
		if (DVObject->AddressListV6 != NULL)
		{
			for (i = 0; i < DVObject->AddressListV6Length; ++i) ILibChain_SafeRemove(DVObject->Chain, DVObject->NOTIFY_SEND_socks6[i]);
			free(DVObject->NOTIFY_SEND_socks6);
			for (i = 0; i < DVObject->AddressListV6Length; ++i) ILibChain_SafeRemove(DVObject->Chain, DVObject->NOTIFY_RECEIVE_socks6[i]);
			free(DVObject->NOTIFY_RECEIVE_socks6);
			free(DVObject->AddressListV6);
		}
		
		// Fetch a current list of ip addresses
		DVObject->AddressListV4Length = ILibGetLocalIPv4AddressList(&(DVObject->AddressListV4), 1);
		
		// Re-Initialize our SEND socket
		if ((DVObject->NOTIFY_SEND_socks = (void**)malloc(sizeof(void*)*(DVObject->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);
		if ((DVObject->NOTIFY_RECEIVE_socks = (void**)malloc(sizeof(void*)*(DVObject->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);
		
		// Test IPv6 support
		if (ILibDetectIPv6Support())
		{
			// Fetch the list of local IPv6 interfaces
			DVObject->AddressListV6Length = ILibGetLocalIPv6List(&(DVObject->AddressListV6));
			
			// Setup the IPv6 sockets
			if ((DVObject->NOTIFY_SEND_socks6 = (void**)malloc(sizeof(void*)*(DVObject->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
			if ((DVObject->NOTIFY_RECEIVE_socks6 = (void**)malloc(sizeof(void*)*(DVObject->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
		}
		
		// Iterate through all the current IP Addresses
		for (i = 0; i < DVObject->AddressListV4Length; ++i)
		{
			(DVObject->AddressListV4[i]).sin_port = 0; // Bind to ANY port for outbound packets
			DVObject->NOTIFY_SEND_socks[i] = ILibAsyncUDPSocket_CreateEx(
			DVObject->Chain,
			UPNP_MAX_SSDP_HEADER_SIZE,
			(struct sockaddr*)&(DVObject->AddressListV4[i]),
			ILibAsyncUDPSocket_Reuse_SHARED,
			NULL,
			NULL,
			DVObject);
			
			ILibAsyncUDPSocket_SetMulticastTTL(DVObject->NOTIFY_SEND_socks[i], UPNP_SSDP_TTL);
			ILibAsyncUDPSocket_SetMulticastLoopback(DVObject->NOTIFY_SEND_socks[i], 1);
			
			(DVObject->AddressListV4[i]).sin_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
			DVObject->NOTIFY_RECEIVE_socks[i] = ILibAsyncUDPSocket_CreateEx(
			DVObject->Chain,
			UPNP_MAX_SSDP_HEADER_SIZE,
			(struct sockaddr*)&(DVObject->AddressListV4[i]),
			ILibAsyncUDPSocket_Reuse_SHARED,
			&DVSSDPSink,
			NULL,
			DVObject);
			
			ILibAsyncUDPSocket_JoinMulticastGroupV4(DVObject->NOTIFY_RECEIVE_socks[i], (struct sockaddr_in*)&(DVObject->MulticastAddrV4), (struct sockaddr*)&(DVObject->AddressListV4[i]));
			ILibAsyncUDPSocket_SetMulticastLoopback(DVObject->NOTIFY_RECEIVE_socks[i], 1);
		}
		
		if (DVObject->AddressListV6Length > 0)
		{
			// Iterate through all the current IPv6 interfaces
			for (i = 0; i < DVObject->AddressListV6Length; ++i)
			{
				(DVObject->AddressListV6[i]).sin6_port = 0;
				DVObject->NOTIFY_SEND_socks6[i] = ILibAsyncUDPSocket_CreateEx(
				DVObject->Chain,
				UPNP_MAX_SSDP_HEADER_SIZE,
				(struct sockaddr*)&(DVObject->AddressListV6[i]),
				ILibAsyncUDPSocket_Reuse_SHARED,
				NULL,
				NULL,
				DVObject);
				
				ILibAsyncUDPSocket_SetMulticastTTL(DVObject->NOTIFY_SEND_socks6[i], UPNP_SSDP_TTL);
				ILibAsyncUDPSocket_SetMulticastLoopback(DVObject->NOTIFY_SEND_socks6[i], 1);
				
				(DVObject->AddressListV6[i]).sin6_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
				DVObject->NOTIFY_RECEIVE_socks6[i] = ILibAsyncUDPSocket_CreateEx(
				DVObject->Chain,
				UPNP_MAX_SSDP_HEADER_SIZE,
				(struct sockaddr*)&(DVObject->AddressListV6[i]),
				ILibAsyncUDPSocket_Reuse_SHARED,
				&DVSSDPSink,
				NULL,
				DVObject);
				
				if (ILibAsyncSocket_IsIPv6LinkLocal((struct sockaddr*)&(DVObject->AddressListV6[i])))
				{
					ILibAsyncUDPSocket_JoinMulticastGroupV6(DVObject->NOTIFY_RECEIVE_socks6[i], &(DVObject->MulticastAddrV6LL), DVObject->AddressListV6[i].sin6_scope_id);
				}
				else
				{
					ILibAsyncUDPSocket_JoinMulticastGroupV6(DVObject->NOTIFY_RECEIVE_socks6[i], &(DVObject->MulticastAddrV6SL), DVObject->AddressListV6[i].sin6_scope_id);
				}
				ILibAsyncUDPSocket_SetMulticastLoopback(DVObject->NOTIFY_RECEIVE_socks6[i], 1);
			}
		}
		
		// Iterate through all the packet types, and re-broadcast
		for (i = 1; i <= 4; ++i)
		{
			if ((f = (struct DVFragmentNotifyStruct*)malloc(sizeof(struct DVFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i;
			f->upnp = DVObject;
			
			// Inject some random delay, to spread these packets out, to help prevent the inbound buffer of the recipient from overflowing, causing dropped packets.
			timeout = (int)(0 + ((unsigned short)rand() % (500)));
			ILibLifeTime_AddEx(f->upnp->WebServerTimer, f, timeout, &DVFragmentedSendNotify, &DVFragmentedSendNotify_Destroy);
		}
	}
}

void DVFragmentedSendNotify_Destroy(void *data)
{
	free(data);
}

void DVFragmentedSendNotify(void *data)
{
	int i,i2;
	int subsetRange;
	int timeout, timeout2;
	struct DVFragmentNotifyStruct *f;
	struct DVFragmentNotifyStruct *FNS = (struct DVFragmentNotifyStruct*)data;
	
	if (FNS->packetNumber == 0)
	{				
		subsetRange = 5000 / 5; // Make sure all our packets will get out within 5 seconds
		
		// Send the first "group"
		for (i2 = 0; i2 <= 4; ++i2)
		{
			if ((f = (struct DVFragmentNotifyStruct*)malloc(sizeof(struct DVFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i2 + 1;
			f->upnp = FNS->upnp;
			timeout2 = (rand() % subsetRange);
			ILibLifeTime_AddEx(FNS->upnp->WebServerTimer, f, timeout2, &DVFragmentedSendNotify, &DVFragmentedSendNotify_Destroy);
		}
		
		// Now Repeat this "group" after 7 seconds, to insure there is no overlap
		for (i2 = 0; i2 <= 4; ++i2)
		{
			if ((f = (struct DVFragmentNotifyStruct*)malloc(sizeof(struct DVFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i2 + 1;
			f->upnp = FNS->upnp;
			timeout2 = 7000 + (rand() % subsetRange);
			ILibLifeTime_AddEx(FNS->upnp->WebServerTimer, f, timeout2, &DVFragmentedSendNotify, &DVFragmentedSendNotify_Destroy);
		}
		
		// Calculate the next transmission window and spread the packets
		timeout = (int)((FNS->upnp->NotifyCycleTime / 4) + ((unsigned short)rand() % (FNS->upnp->NotifyCycleTime / 2 - FNS->upnp->NotifyCycleTime / 4)));
		ILibLifeTime_Add(FNS->upnp->WebServerTimer, FNS, timeout, &DVFragmentedSendNotify, &DVFragmentedSendNotify_Destroy);
	}
	
	for (i = 0; i < FNS->upnp->AddressListV4Length; ++i)
	{
		switch(FNS->packetNumber)
		{
			case 1:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
			break;
			case 2:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), 0, "", "uuid:", FNS->upnp->UDN);
			break;
			case 3:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), 0, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "");
			break;
			case 4:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "");
			break;
			
		}
	}
	
	for (i = 0; i < FNS->upnp->AddressListV6Length; ++i)
	{
		switch(FNS->packetNumber)
		{
			case 1:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
			break;
			case 2:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), 0, "", "uuid:", FNS->upnp->UDN);
			break;
			case 3:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), 0, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "");
			break;
			case 4:
			DVBuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "");
			break;
			
		}
	}
	
	if (FNS->packetNumber != 0) free(FNS);
}


void DVSendNotify(const struct DVDataObject *upnp)
{
	int i, i2;
	for (i=0;i<upnp->AddressListV4Length;++i)
	{
		for (i2=0; i2<2; i2++)
		{
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), 0, "", "uuid:", upnp->UDN);
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), 0, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "");
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "");
			
		}
	}
	for (i=0;i<upnp->AddressListV6Length;++i)
	{
		for (i2=0; i2<2; i2++)
		{
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), 0, "", "uuid:", upnp->UDN);
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), 0, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "");
			DVBuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), 0, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "");
			
		}
	}
}

int DVBuildSendSsdpByeByePacket(void* module, const struct DVDataObject *upnp, struct sockaddr* target, char* mcastgroup, char* USNex, char* NT, char* NTex, int DeviceID)
{
	int len;
	
	if (DeviceID == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n", mcastgroup, upnp->UDN, USNex, NT, NTex);
	}
	else
	{
		if (memcmp(NT, "uuid:", 5) == 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s_%d\r\nContent-Length: 0\r\n\r\n", mcastgroup, upnp->UDN, DeviceID, USNex, NT, NTex, DeviceID);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n", mcastgroup, upnp->UDN, DeviceID, USNex, NT, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}

void DVSendByeBye(const struct DVDataObject *upnp)
{
	int i, i2;
	struct sockaddr* t1;
	char* t2;
	
	for (i=0; i<upnp->AddressListV4Length; ++i)
	{	
		for (i2=0; i2<2; i2++)
		{
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, "::upnp:rootdevice", "upnp:rootdevice", "", 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, "", "uuid:", upnp->UDN, 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "", 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "", 0);
			
		}
	}
	
	for (i=0; i<upnp->AddressListV6Length; ++i)
	{	
		if (ILibAsyncSocket_IsIPv6LinkLocal((struct sockaddr*)&(upnp->AddressListV6[i])))
		{
			t1 = (struct sockaddr*)&(upnp->MulticastAddrV6LL);
			t2 = UPNP_MCASTv6_LINK_GROUPB;
		}
		else
		{
			t1 = (struct sockaddr*)&(upnp->MulticastAddrV6SL);
			t2 = UPNP_MCASTv6_GROUPB;
		}
		
		for (i2=0; i2<2; i2++)
		{
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, "::upnp:rootdevice", "upnp:rootdevice", "", 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, "", "uuid:", upnp->UDN, 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, "::urn:schemas-upnp-org:device:Basic:1", "urn:schemas-upnp-org:device:Basic:1", "", 0);
			DVBuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, "::urn:schemas-upnp-org:service:DeviceProtection:1", "urn:schemas-upnp-org:service:DeviceProtection:1", "", 0);
			
		}
	}
}

/*! \fn DVResponse_Error(const DVSessionToken DVToken, const int ErrorCode, const char* ErrorMsg)
\brief Responds to the client invocation with a SOAP Fault
\param DVToken UPnP token
\param ErrorCode Fault Code
\param ErrorMsg Error Detail
*/
void DVResponse_Error(const DVSessionToken DVToken, const int ErrorCode, const char* ErrorMsg)
{
	char* body;
	int bodylength;
	char* head;
	int headlength;
	int len = 395 + (int)strlen(ErrorMsg);
	
	if ((body = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	bodylength = snprintf(body, len, "<s:Envelope\r\n xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></UPnPError></detail></s:Fault></s:Body></s:Envelope>",ErrorCode,ErrorMsg);
	if ((head = (char*)malloc(59)) == NULL) ILIBCRITICALEXIT(254);
	headlength = snprintf(head, 59, "HTTP/1.1 500 Internal\r\nContent-Length: %d\r\n\r\n",bodylength);
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)DVToken, head, headlength, 0, 0);
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)DVToken, body, bodylength, 0, 1);
}

/*! \fn DVGetLocalInterfaceToHost(const DVSessionToken DVToken)
\brief When a UPnP request is dispatched, this method determines which ip address actually received this request
\param DVToken UPnP token
\returns IP Address
*/
/*
int DVGetLocalInterfaceToHost(const DVSessionToken DVToken)
{
	return(ILibWebServer_GetLocalInterface((struct ILibWebServer_Session*)DVToken));
}
*/

void DVResponseGeneric(const DVMicroStackToken DVToken, const char* ServiceURI, const char* MethodName, const char* Params)
{
	int RVAL = 0;
	char* packet;
	int packetlength;
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)DVToken;
	size_t len = 239 + strlen(ServiceURI) + strlen(Params) + (strlen(MethodName) * 2);
	
	if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	packetlength = snprintf(packet, len, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
	LVL3DEBUG(printf("SendBody: %s\r\n", packet);)
	#if defined(WIN32) || defined(_WIN32_WCE)
	RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: WINDOWS, UPnP/1.0, MicroStack/1.0.3951", 1);
	#elif defined(__SYMBIAN32__)
	RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: SYMBIAN, UPnP/1.0, MicroStack/1.0.3951", 1);
	#else
	RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: POSIX, UPnP/1.0, MicroStack/1.0.3951", 1);
	#endif
	if (RVAL!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RVAL != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
	{
		RVAL=ILibWebServer_StreamBody(session, packet, packetlength, 0, 1);
	}
}

/*! \fn DVResponse_DeviceProtection_AddIdentityList(const DVSessionToken DVToken, const char* unescaped_IdentityListResult)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> AddIdentityList
\param DVToken MicroStack token
\param unescaped_IdentityListResult Value of argument IdentityListResult \b     Note: Automatically Escaped
*/
void DVResponse_DeviceProtection_AddIdentityList(const DVSessionToken DVToken, const char* unescaped_IdentityListResult)
{
	char* body;
	char *IdentityListResult = (char*)malloc(1+ILibXmlEscapeLength(unescaped_IdentityListResult));
	
	ILibXmlEscape(IdentityListResult, unescaped_IdentityListResult);
	if ((body = (char*)malloc(42+strlen(IdentityListResult))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 42+strlen(IdentityListResult), "<IdentityListResult>%s</IdentityListResult>", IdentityListResult);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "AddIdentityList", body);
	free(body);
	free(IdentityListResult);
}

/*! \fn DVResponse_DeviceProtection_AddRolesForIdentity(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> AddRolesForIdentity
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_AddRolesForIdentity(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","AddRolesForIdentity","");
}

/*! \fn DVResponse_DeviceProtection_GetACLData(const DVSessionToken DVToken, const char* unescaped_ACL)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetACLData
\param DVToken MicroStack token
\param unescaped_ACL Value of argument ACL \b     Note: Automatically Escaped
*/
void DVResponse_DeviceProtection_GetACLData(const DVSessionToken DVToken, const char* unescaped_ACL)
{
	char* body;
	char *ACL = (char*)malloc(1+ILibXmlEscapeLength(unescaped_ACL));
	
	ILibXmlEscape(ACL, unescaped_ACL);
	if ((body = (char*)malloc(12+strlen(ACL))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 12+strlen(ACL), "<ACL>%s</ACL>", ACL);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "GetACLData", body);
	free(body);
	free(ACL);
}

/*! \fn DVResponse_DeviceProtection_GetAssignedRoles(const DVSessionToken DVToken, const char* unescaped_RoleList)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetAssignedRoles
\param DVToken MicroStack token
\param unescaped_RoleList Value of argument RoleList \b     Note: Automatically Escaped
*/
void DVResponse_DeviceProtection_GetAssignedRoles(const DVSessionToken DVToken, const char* unescaped_RoleList)
{
	char* body;
	char *RoleList = (char*)malloc(1+ILibXmlEscapeLength(unescaped_RoleList));
	
	ILibXmlEscape(RoleList, unescaped_RoleList);
	if ((body = (char*)malloc(22+strlen(RoleList))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 22+strlen(RoleList), "<RoleList>%s</RoleList>", RoleList);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "GetAssignedRoles", body);
	free(body);
	free(RoleList);
}

/*! \fn DVResponse_DeviceProtection_GetRolesForAction(const DVSessionToken DVToken, const char* unescaped_RoleList, const char* unescaped_RestrictedRoleList)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetRolesForAction
\param DVToken MicroStack token
\param unescaped_RoleList Value of argument RoleList \b     Note: Automatically Escaped
\param unescaped_RestrictedRoleList Value of argument RestrictedRoleList \b     Note: Automatically Escaped
*/
void DVResponse_DeviceProtection_GetRolesForAction(const DVSessionToken DVToken, const char* unescaped_RoleList, const char* unescaped_RestrictedRoleList)
{
	char* body;
	char *RoleList = (char*)malloc(1+ILibXmlEscapeLength(unescaped_RoleList));
	char *RestrictedRoleList = (char*)malloc(1+ILibXmlEscapeLength(unescaped_RestrictedRoleList));
	
	ILibXmlEscape(RoleList, unescaped_RoleList);
	ILibXmlEscape(RestrictedRoleList, unescaped_RestrictedRoleList);
	if ((body = (char*)malloc(63+strlen(RoleList)+strlen(RestrictedRoleList))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 63+strlen(RoleList)+strlen(RestrictedRoleList), "<RoleList>%s</RoleList><RestrictedRoleList>%s</RestrictedRoleList>", RoleList, RestrictedRoleList);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "GetRolesForAction", body);
	free(body);
	free(RoleList);
	free(RestrictedRoleList);
}

/*! \fn DVResponse_DeviceProtection_GetSupportedProtocols(const DVSessionToken DVToken, const char* unescaped_ProtocolList)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetSupportedProtocols
\param DVToken MicroStack token
\param unescaped_ProtocolList Value of argument ProtocolList \b     Note: Automatically Escaped
*/
void DVResponse_DeviceProtection_GetSupportedProtocols(const DVSessionToken DVToken, const char* unescaped_ProtocolList)
{
	char* body;
	char *ProtocolList = (char*)malloc(1+ILibXmlEscapeLength(unescaped_ProtocolList));
	
	ILibXmlEscape(ProtocolList, unescaped_ProtocolList);
	if ((body = (char*)malloc(30+strlen(ProtocolList))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 30+strlen(ProtocolList), "<ProtocolList>%s</ProtocolList>", ProtocolList);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "GetSupportedProtocols", body);
	free(body);
	free(ProtocolList);
}

/*! \fn DVResponse_DeviceProtection_GetUserLoginChallenge(const DVSessionToken DVToken, const unsigned char* Salt, const int _SaltLength, const unsigned char* Challenge, const int _ChallengeLength)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> GetUserLoginChallenge
\param DVToken MicroStack token
\param Salt Value of argument Salt
\param SaltLength Length of \a Salt
\param Challenge Value of argument Challenge
\param ChallengeLength Length of \a Challenge
*/
void DVResponse_DeviceProtection_GetUserLoginChallenge(const DVSessionToken DVToken, const unsigned char* Salt, const int _SaltLength, const unsigned char* Challenge, const int _ChallengeLength)
{
	char* body;
	
	unsigned char* Salt_Base64;
	unsigned char* Challenge_Base64;
	ILibBase64Encode((unsigned char*)Salt, _SaltLength, &Salt_Base64);
	ILibBase64Encode((unsigned char*)Challenge, _ChallengeLength, &Challenge_Base64);
	if ((body = (char*)malloc(37+strlen(Salt_Base64)+strlen(Challenge_Base64))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 37+strlen(Salt_Base64)+strlen(Challenge_Base64), "<Salt>%s</Salt><Challenge>%s</Challenge>", Salt_Base64, Challenge_Base64);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "GetUserLoginChallenge", body);
	free(body);
	free(Salt_Base64);
	free(Challenge_Base64);
}

/*! \fn DVResponse_DeviceProtection_RemoveIdentity(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> RemoveIdentity
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_RemoveIdentity(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","RemoveIdentity","");
}

/*! \fn DVResponse_DeviceProtection_RemoveRolesForIdentity(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> RemoveRolesForIdentity
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_RemoveRolesForIdentity(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","RemoveRolesForIdentity","");
}

/*! \fn DVResponse_DeviceProtection_SendSetupMessage(const DVSessionToken DVToken, const unsigned char* OutMessage, const int _OutMessageLength)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> SendSetupMessage
\param DVToken MicroStack token
\param OutMessage Value of argument OutMessage
\param OutMessageLength Length of \a OutMessage
*/
void DVResponse_DeviceProtection_SendSetupMessage(const DVSessionToken DVToken, const unsigned char* OutMessage, const int _OutMessageLength)
{
	char* body;
	
	unsigned char* OutMessage_Base64;
	ILibBase64Encode((unsigned char*)OutMessage, _OutMessageLength, &OutMessage_Base64);
	if ((body = (char*)malloc(26+strlen(OutMessage_Base64))) == NULL) ILIBCRITICALEXIT(254);
	snprintf(body, 26+strlen(OutMessage_Base64), "<OutMessage>%s</OutMessage>", OutMessage_Base64);
	DVResponseGeneric(DVToken, "urn:schemas-upnp-org:service:DeviceProtection:1", "SendSetupMessage", body);
	free(body);
	free(OutMessage_Base64);
}

/*! \fn DVResponse_DeviceProtection_SetUserLoginPassword(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> SetUserLoginPassword
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_SetUserLoginPassword(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","SetUserLoginPassword","");
}

/*! \fn DVResponse_DeviceProtection_UserLogin(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> UserLogin
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_UserLogin(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","UserLogin","");
}

/*! \fn DVResponse_DeviceProtection_UserLogout(const DVSessionToken DVToken)
\brief Response Method for DeviceProtection >> urn:schemas-upnp-org:service:DeviceProtection:1 >> UserLogout
\param DVToken MicroStack token
*/
void DVResponse_DeviceProtection_UserLogout(const DVSessionToken DVToken)
{
	DVResponseGeneric(DVToken,"urn:schemas-upnp-org:service:DeviceProtection:1","UserLogout","");
}


void DVSendEventSink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *subscriber,
void *upnp,
int *PAUSE)	
{
	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( IsInterrupt );
	UNREFERENCED_PARAMETER( buffer );
	UNREFERENCED_PARAMETER( p_BeginPointer );
	UNREFERENCED_PARAMETER( EndPointer );
	UNREFERENCED_PARAMETER( PAUSE );
	
	if (done != 0 && ((struct SubscriberInfo*)subscriber)->Disposing == 0)
	{
		sem_wait(&(((struct DVDataObject*)upnp)->EventLock));
		--((struct SubscriberInfo*)subscriber)->RefCount;
		if (((struct SubscriberInfo*)subscriber)->RefCount == 0)
		{
			LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n", ((struct SubscriberInfo*)subscriber)->SID, (((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF), ((struct SubscriberInfo*)subscriber)->Port);)
			DVDestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
		}
		else if (header == NULL)
		{
			LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n", ((struct SubscriberInfo*)subscriber)->SID, (((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF), ((struct SubscriberInfo*)subscriber)->Port);)
			// Could not send Event, so unsubscribe the subscriber
			((struct SubscriberInfo*)subscriber)->Disposing = 1;
			DVExpireSubscriberInfo(upnp, subscriber);
		}
		sem_post(&(((struct DVDataObject*)upnp)->EventLock));
	}
}

void DVSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info)
{
	struct DVDataObject* DVObject = (struct DVDataObject*)upnptoken;
	struct sockaddr_in dest;
	int packetLength;
	char *packet;
	int ipaddr;
	int len;
	
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = info->Address;
	dest.sin_port = htons(info->Port);
	dest.sin_family = AF_INET;
	ipaddr = info->Address;
	
	len = info->PathLength + bodylength + 483;
	if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	packetLength = snprintf(packet, len, "NOTIFY %s HTTP/1.1\r\nSERVER: %s, UPnP/1.0, MicroStack/1.0.3951\r\nHOST: %s:%d\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,DVPLATFORM,inet_ntoa(dest.sin_addr),info->Port,info->SID,info->SEQ,bodylength+137,body);
	++info->SEQ;
	
	++info->RefCount;
	ILibWebClient_PipelineRequestEx(DVObject->EventClient, (struct sockaddr*)(&dest), packet, packetLength, 0, NULL, 0, 0, &DVSendEventSink, info, upnptoken);
}

void DVSendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
	struct SubscriberInfo *info = NULL;
	struct DVDataObject* DVObject = (struct DVDataObject*)upnptoken;
	struct sockaddr_in dest;
	LVL3DEBUG(struct timeval tv;)
	
	if (DVObject == NULL)
	{
		free(body);
		return;
	}
	sem_wait(&(DVObject->EventLock));
	if (strncmp(eventname,"DeviceProtection",16)==0)
	{
		info = DVObject->HeadSubscriberPtr_DeviceProtection;
	}
	
	memset(&dest, 0, sizeof(dest));
	while(info != NULL)
	{
		if (!DVSubscriptionExpired(info))
		{
			DVSendEvent_Body(upnptoken, body, bodylength, info);
		}
		else
		{
			// Remove Subscriber
			LVL3DEBUG(gettimeofday(&tv, NULL);)
			LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n", tv.tv_sec);)
			LVL3DEBUG(printf("Did not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\r\n\r\n", ((struct SubscriberInfo*)info)->SID, (((struct SubscriberInfo*)info)->Address&0xFF), ((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((struct SubscriberInfo*)info)->Address>>16)&0xFF), ((((struct SubscriberInfo*)info)->Address>>24)&0xFF), ((struct SubscriberInfo*)info)->Port, info);)
		}
		info = info->Next;
	}
	sem_post(&(DVObject->EventLock));
}
/*! \fn DVSetState_DeviceProtection_SetupReady(DVMicroStackToken upnptoken, int val)
\brief Sets the state of SetupReady << urn:schemas-upnp-org:service:DeviceProtection:1 << DeviceProtection \par
\b Note: Must be called at least once prior to start
\param upnptoken The MicroStack token
\param val The new value of the state variable
*/
void DVSetState_DeviceProtection_SetupReady(DVMicroStackToken upnptoken, int val)
{
	struct DVDataObject *DVObject = (struct DVDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	if (val != 0) valstr = "true"; else valstr = "false";
	DVObject->DeviceProtection_SetupReady = valstr;
	bodylength = 30 + (int)strlen(valstr);
	if ((body = (char*)malloc(bodylength)) == NULL) ILIBCRITICALEXIT(254);
	bodylength = snprintf(body, bodylength, "%s>%s</%s", "SetupReady", valstr, "SetupReady");
	DVSendEvent(upnptoken, body, bodylength, "DeviceProtection");
	free(body);
}


void DVDestroyMicroStack(void *object)
{
	struct DVDataObject *upnp = (struct DVDataObject*)object;
	struct SubscriberInfo  *sinfo,*sinfo2;	DVSendByeBye(upnp);
	sem_destroy(&(upnp->EventLock));
	
	if (upnp->AddressListV4 != NULL) free(upnp->AddressListV4);
	if (upnp->AddressListV6 != NULL) free(upnp->AddressListV6);
	if (upnp->NOTIFY_SEND_socks != NULL) free(upnp->NOTIFY_SEND_socks);
	if (upnp->NOTIFY_RECEIVE_socks != NULL) free(upnp->NOTIFY_RECEIVE_socks);
	if (upnp->NOTIFY_SEND_socks6 != NULL) free(upnp->NOTIFY_SEND_socks6);
	if (upnp->NOTIFY_RECEIVE_socks6 != NULL) free(upnp->NOTIFY_RECEIVE_socks6);
	free(upnp->UUID);
	free(upnp->Serial);
	free(upnp->DeviceDescription);
	sinfo = upnp->HeadSubscriberPtr_DeviceProtection;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		DVDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	
}

int DVGetLocalPortNumber(DVSessionToken token)
{
	return(ILibWebServer_GetPortNumber(((struct ILibWebServer_Session*)token)->Parent));
}

void DVSessionReceiveSink(
struct ILibWebServer_Session *sender,
int InterruptFlag,
struct packetheader *header,
char *bodyBuffer,
int *beginPointer,
int endPointer,
int done)
{
	char *txt;
	if (header != NULL && sender->User3 == NULL && done == 0)
	{
		sender->User3 = (void*)~0;
		txt = ILibGetHeaderLine(header,"Expect",6);
		if (txt!=NULL)
		{
			if (strcasecmp(txt,"100-Continue")==0)
			{
				// Expect Continue
				ILibWebServer_Send_Raw(sender,"HTTP/1.1 100 Continue\r\n\r\n",25,ILibAsyncSocket_MemoryOwnership_STATIC,0);
			}
			else
			{
				// Don't understand
				ILibWebServer_Send_Raw(sender,"HTTP/1.1 417 Expectation Failed\r\n\r\n",35,ILibAsyncSocket_MemoryOwnership_STATIC,1);
				ILibWebServer_DisconnectSession(sender);
				return;
			}
		}
	}
	if (header != NULL && done !=0 && InterruptFlag == 0)
	{
		DVProcessHTTPPacket(sender, header, bodyBuffer, beginPointer == NULL?0:*beginPointer, endPointer);
		if (beginPointer!=NULL) {*beginPointer = endPointer;}
	}
}
void DVSessionSink(struct ILibWebServer_Session *SessionToken, void *user)
{
	SessionToken->OnReceive = &DVSessionReceiveSink;
	SessionToken->User = user;
}

void DVS_SessionSink(struct ILibWebServer_Session *SessionToken, void *user)
{
	SessionToken->OnReceive = &DVSessionReceiveSink;
	SessionToken->User = user;
	printf("secure session ");
	// maybe keep track of the port number here?  That way, later on the device dispatching
	// code can check whether the HTTP request has come in on a secure port.

	// localPort = DVGetLocalPortNumber(SessionToken);

	// TODO: get the session identifier, since this is a TLS connection
}

void DVSetTag(const DVMicroStackToken token, void *UserToken)
{
	((struct DVDataObject*)token)->User = UserToken;
}

void *DVGetTag(const DVMicroStackToken token)
{
	return(((struct DVDataObject*)token)->User);
}

DVMicroStackToken DVGetMicroStackTokenFromSessionToken(const DVSessionToken token)
{
	return(((struct ILibWebServer_Session*)token)->User);
}

char * DVGetCertHashFromSessionToken(const DVSessionToken token)
{
	struct ILibWebServer_Session *ws;

	ws = ((struct ILibWebServer_Session*)token); // try this first...

	if (ws) {
		return ws->CertificateHashPtr;
	}
	return NULL;
}

char * DVGetCertNameFromSessionToken(const DVSessionToken token)
{
	struct ILibWebServer_Session *ws;

	ws = ((struct ILibWebServer_Session*)token); // try this first...

	if (ws) {
		return ws->CertificateName;
	}
	return NULL;
}

int DVGetSecurePortNumber(const DVMicroStackToken stack) 
{
	struct DVDataObject* obj = (struct DVDataObject*) stack;
	return obj->WebSocketSPortNumber;
}

extern SSL_CTX* g_server_ctx;

DVMicroStackToken DVCreateMicroStack(void *Chain, const char* FriendlyName,const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)

{
	struct DVDataObject* RetVal;
	char* DDT;	struct timeval tv;
	size_t len;
	if ((RetVal = (struct DVDataObject*)malloc(sizeof(struct DVDataObject))) == NULL) ILIBCRITICALEXIT(254);
	
	gettimeofday(&tv,NULL);
	srand((int)tv.tv_sec);
	
	// Complete State Reset
	memset(RetVal, 0, sizeof(struct DVDataObject));
	
	RetVal->ForceExit = 0;
	RetVal->PreSelect = &DVMasterPreSelect;
	RetVal->PostSelect = NULL;
	RetVal->Destroy = &DVDestroyMicroStack;
	RetVal->InitialNotify = 0;
	if (UDN != NULL)
	{
		len = (int)strlen(UDN) + 6;
		if ((RetVal->UUID = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		snprintf(RetVal->UUID, len, "uuid:%s", UDN);
		RetVal->UDN = RetVal->UUID + 5;
	}
	if (SerialNumber != NULL)
	{
		len = strlen(SerialNumber) + 1;
		if (len > 20) len = 20;
		if ((RetVal->Serial = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		memcpy(RetVal->Serial, SerialNumber, len);
		RetVal->Serial[len - 1] = 0;
	}
	
	len = 10 + DVDeviceDescriptionTemplateLengthUX+ (int)strlen(FriendlyName)  + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * 1);
	if ((RetVal->DeviceDescription = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	
	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
	RetVal->HTTPServer = ILibWebServer_Create(Chain, UPNP_HTTP_MAXSOCKETS, PortNum, &DVSessionSink, RetVal);
	RetVal->WebSocketPortNumber = (int)ILibWebServer_GetPortNumber(RetVal->HTTPServer);

	// vbl added:  create a HTTP server instance for the https: socket, the 0 parameter means get random port #
	RetVal->HTTPSServer = ILibWebServer_Create(Chain,UPNP_HTTP_MAXSOCKETS,0,&DVS_SessionSink,RetVal);
	RetVal->WebSocketSPortNumber=(int)ILibWebServer_GetPortNumber(RetVal->HTTPSServer);
#ifndef HTTPONLY
		ILibWebServer_SetTLS(RetVal->HTTPSServer, g_server_ctx);	// set the server side TLS context
#endif
	
	ILibAddToChain(Chain, RetVal); 
	DVInit(RetVal ,Chain, NotifyCycleSeconds, RetVal->WebSocketSPortNumber); // connects multicast listener 

	RetVal->EventClient = ILibCreateWebClient(5, Chain);
	RetVal->UpdateFlag = 0;
	
	DDT = ILibDecompressString((unsigned char*)DVDeviceDescriptionTemplate, DVDeviceDescriptionTemplateLength, DVDeviceDescriptionTemplateLengthUX);
	RetVal->DeviceDescriptionLength = snprintf(RetVal->DeviceDescription, len, DDT, FriendlyName, RetVal->Serial, RetVal->UDN);
	
	free(DDT);
	
	sem_init(&(RetVal->EventLock), 0, 1);
	return(RetVal);
}





