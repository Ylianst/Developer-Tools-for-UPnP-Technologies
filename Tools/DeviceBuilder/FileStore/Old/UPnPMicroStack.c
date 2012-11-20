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

#if defined(WIN32) || defined(_WIN32_WCE)
#	ifndef MICROSTACK_NO_STDAFX
#		include "stdafx.h"
#	endif
char* UPnPPLATFORM = "WINDOWS";
#elif defined(__SYMBIAN32__)
char* UPnPPLATFORM = "SYMBIAN";
#else
char* UPnPPLATFORM = "POSIX";
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
#include "UPnPMicroStack.h"
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
//{{{BEGIN_MulticastEventing}}}
#define UPNP_MULTICASTEVENT_PORT 1800
//{{{END_MulticastEventing}}}
#define UPNP_MCASTv4_GROUP "239.255.255.250"
#define UPNP_MCASTv6_GROUP "FF05:0:0:0:0:0:0:C" // Site local
#define UPNP_MCASTv6_GROUPB "[FF05:0:0:0:0:0:0:C]"
#define UPNP_MCASTv6_LINK_GROUP "FF02:0:0:0:0:0:0:C" // Link local
#define UPNP_MCASTv6_LINK_GROUPB "[FF02:0:0:0:0:0:0:C]"
#define UPnP_MAX_SUBSCRIPTION_TIMEOUT {{{UPnP_MAX_SUBSCRIPTION_TIMEOUT}}}
#define UPnPMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)
#define INET_SOCKADDR_LENGTH(x) ((x==AF_INET6?sizeof(struct sockaddr_in6):sizeof(struct sockaddr_in)))

#if defined(WIN32)
#pragma warning( push, 3 ) // warning C4310: cast truncates constant value
#endif
//{{{ObjectDefintions}}}
//{{{FunctionPointers}}}
//{{{Device_Default_Model_BEGIN}}}
//{{{CompressedDescriptionDocs}}}
//{{{Device_Default_Model_END}}}
#if defined(WIN32)
#pragma warning( pop )
#endif

//{{{DeviceIcon_Begin}}}
const char UPnPDeviceIcon_LGPNG[{{{IconLength_LGPNG}}}]={{{ICON_LGPNG}}};
const char UPnPDeviceIcon_LGJPG[{{{IconLength_LGJPG}}}]={{{ICON_LGJPG}}};
const char UPnPDeviceIcon_SMPNG[{{{IconLength_SMPNG}}}]={{{ICON_SMPNG}}};
const char UPnPDeviceIcon_SMJPG[{{{IconLength_SMJPG}}}]={{{ICON_SMJPG}}};
//{{{DeviceIcon_End}}}
struct UPnPDataObject;

// It should not be necessary to expose/modify any of these structures. They are used by the internal stack
struct SubscriberInfo
{
	char* SID;		// Subscription ID
	int SIDLength;
	int SEQ;

	//{{{BEGIN_UPnP/1.1_Specific}}}
	int NotLegacy;
	//{{{END_UPnP/1.1_Specific}}}
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

struct UPnPDataObject
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
	//{{{Device_Default_Model_BEGIN}}}	char *DeviceDescription;
	int DeviceDescriptionLength;//{{{Device_Default_Model_END}}}
	int InitialNotify;
	//{{{StateVariables}}}
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

	void **NOTIFY_RECEIVE_socks;
	void **NOTIFY_SEND_socks;
	void **NOTIFY_RECEIVE_socks6;
	void **NOTIFY_SEND_socks6;
	//{{{BEGIN_UPnP/1.1_Specific}}}
	int ConfigID;
	unsigned short UnicastReceiveSocketPortNumber;
	void **UnicastReceiveSockets;
	//{{{END_UPnP/1.1_Specific}}}
	struct timeval CurrentTime;
	struct timeval NotifyTime;

	int SID;
	int NotifyCycleTime;
	//{{{BEGIN_MulticastEventing}}}
	void *MulticastEventListener;
	//{{{END_MulticastEventing}}}
	sem_t EventLock;
	//{{{HeadSubscriberPointers}}}
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
struct UPnPFragmentNotifyStruct
{
	struct UPnPDataObject *upnp;
	int packetNumber;
};

/* Pre-declarations */
//{{{Device_Object_Model_BEGIN}}}
void UPnPStreamDescriptionDocument(struct ILibWebServer_Session *session);
//{{{Device_Object_Model_END}}}
void UPnPFragmentedSendNotify(void *data);
void UPnPSendNotify(const struct UPnPDataObject *upnp);
void UPnPSendByeBye(const struct UPnPDataObject *upnp);
void UPnPMainInvokeSwitch();
void UPnPSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UPnPSendData(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
int UPnPPeriodicNotify(struct UPnPDataObject *upnp);
void UPnPSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);
void UPnPProcessMSEARCH(struct UPnPDataObject *upnp, struct packetheader *packet);
struct in_addr UPnP_inaddr;

/*! \fn UPnPGetWebServerToken(const UPnPMicroStackToken MicroStackToken)
\brief Converts a MicroStackToken to a WebServerToken
\par
\a MicroStackToken is the void* returned from a call to UPnPCreateMicroStack. The returned token, is the server token
not the session token.
\param MicroStackToken MicroStack Token
\returns WebServer Token
*/
void* UPnPGetWebServerToken(const UPnPMicroStackToken MicroStackToken)
{
	return(((struct UPnPDataObject*)MicroStackToken)->HTTPServer);
}
//{{{BEGIN_MulticastEventing}}}
#define UPnPMulticastPacketTemplate "NOTIFY * HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nUSN: uuid:%s::%s\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSEQ: %d\r\nLVL: %s\r\nBOOTID.UPNP.ORG: %d\r\n\r\n<?xml version=\"1.0\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s>%s</%s></e:property></e:propertyset>"
UPnPEvent_MulticastGeneric_Handler UPnPOnEvent_MulticastGeneric = NULL;
//{{{END_MulticastEventing}}}
//{{{BEGIN_UPnP/1.0_Specific}}}
//{{{BEGIN_EmbeddedDevices=0}}}
int UPnPBuildSendSsdpResponsePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, struct sockaddr* target, int EmbeddedDeviceNumber, char* USNex, char* ST, char* NTex)
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

	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n", ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, ST, NTex);
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}

int UPnPBuildSendSsdpNotifyPacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, int EmbeddedDeviceNumber, char* USNex, char* NT, char* NTex)
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

	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nHOST: %s:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", ILibScratchPad2, upnp->WebSocketPortNumber, mcaststr, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, NT, NTex);
	return ILibAsyncUDPSocket_SendTo(module, multicast, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
//{{{END_EmbeddedDevices=0}}}
//{{{BEGIN_EmbeddedDevices>0}}}
int UPnPBuildSendSsdpResponsePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, struct sockaddr* target, int EmbeddedDeviceNumber, char* USNex, char* ST, char* NTex)
{
	int len;

	if (local->sa_family == AF_INET)
	{
		// IPv4 address format
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in*)local)->sin_addr), ILibScratchPad2, sizeof(ILibScratchPad2));
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
	}

	if (EmbeddedDeviceNumber == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n",ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, ST, NTex);
	}
	else
	{
		if (strcmp(ST, "ssdp:all" )== 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nST: uuid:%s_%d%s\r\n\r\n" ,ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, upnp->UDN, EmbeddedDeviceNumber, NTex);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, ST, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}

int UPnPBuildSendSsdpNotifyPacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, int EmbeddedDeviceNumber, char* USNex, char* NT, char* NTex)
{
	int len;
	struct sockaddr* multicast;
	char* mcaststr;

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
		// multicast = (struct sockaddr*)&(upnp->MulticastAddrV6); // TODO: Old code line. To be removed.
		if (ILibAsyncSocket_IsIPv6LinkLocal(local)) { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6LL); } else { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6SL); }
		mcaststr = UPNP_MCASTv6_GROUPB;
	}

	if (EmbeddedDeviceNumber == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, NT, NTex);
	}
	else
	{
		if (memcmp(NT, "uuid:", 5) == 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s_%d\r\n\r\n", ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, NT, NTex, EmbeddedDeviceNumber);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, NT, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, multicast, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
//{{{END_EmbeddedDevices>0}}}
//{{{END_UPnP/1.0_Specific}}}
//{{{BEGIN_UPnP/1.1_Specific}}}
//{{{BEGIN_EmbeddedDevices=0}}}
int UPnPBuildSendSsdpResponsePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, struct sockaddr* target, int EmbeddedDeviceNumber, char* USNex, char* ST, char* NTex)
{
	int len;

	if (local->sa_family == AF_INET)
	{
		// IPv4 address format
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in*)local)->sin_addr), ILibScratchPad2, sizeof(ILibScratchPad2));
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
	}

	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, ST, NTex);
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}

int UPnPBuildSendSsdpNotifyPacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, int EmbeddedDeviceNumber, char* USNex, char* NT, char* NTex)
{
	int len;
	struct sockaddr* multicast;
	char* mcaststr;

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
		// multicast = (struct sockaddr*)&(upnp->MulticastAddrV6); // TODO: Old code line. To be removed.
		if (ILibAsyncSocket_IsIPv6LinkLocal(local)) { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6LL); } else { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6SL); }
		mcaststr = UPNP_MCASTv6_GROUPB;
	}

	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nHOST: %s:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, mcaststr, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, NT, NTex);
	return ILibAsyncUDPSocket_SendTo(module, multicast, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
//{{{END_EmbeddedDevices=0}}}
//{{{BEGIN_EmbeddedDevices>0}}}
int UPnPBuildSendSsdpResponsePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, struct sockaddr* target, int EmbeddedDeviceNumber, char* USNex, char* ST, char* NTex)
{
	int len;

	if (local->sa_family == AF_INET)
	{
		// IPv4 address format
		ILibInet_ntop(local->sa_family, &(((struct sockaddr_in*)local)->sin_addr), ILibScratchPad2, sizeof(ILibScratchPad2));
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
	}

	if (EmbeddedDeviceNumber == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, ST, NTex);
	}
	else
	{
		if (strcmp(ST, "ssdp:all" )== 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nST: uuid:%s_%d%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, USN, EmbeddedDeviceNumber, NTex);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "HTTP/1.1 200 OK\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nEXT:\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, ST, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}

int UPnPBuildSendSsdpNotifyPacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* local, int EmbeddedDeviceNumber, char* USNex, char* NT, char* NTex)
{
	int len;
	struct sockaddr* multicast;
	char* mcaststr;

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
		// multicast = (struct sockaddr*)&(upnp->MulticastAddrV6); // TODO: Old code line. To be removed.
		if (ILibAsyncSocket_IsIPv6LinkLocal(local)) { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6LL); } else { multicast = (struct sockaddr*)&(upnp->MulticastAddrV6SL); }
		mcaststr = UPNP_MCASTv6_GROUPB;
	}

	if (EmbeddedDeviceNumber == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, USNex, upnp->NotifyCycleTime, NT, NTex);
	}
	else
	{
		if (memcmp(NT, "uuid:", 5) == 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s_%d\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, NT, NTex, EmbeddedDeviceNumber);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nLOCATION: http://%s:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nNTS: ssdp:alive\r\nUSN: uuid:%s_%d%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, ILibScratchPad2, upnp->WebSocketPortNumber, UPnPPLATFORM, upnp->UDN, EmbeddedDeviceNumber, USNex, upnp->NotifyCycleTime, NT, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, multicast, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
//{{{END_EmbeddedDevices>0}}}
void UPnPUnicastSSDPSink(ILibAsyncUDPSocket_SocketModule socketModule,char* buffer, int bufferLength, int remoteInterface, unsigned short remotePort, void *user, void *user2, int *PAUSE)
{
	struct packetheader *packet;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)user;

	packet = ILibParsePacketHeader(buffer, 0, bufferLength);
	if (packet != NULL)
	{
		// Fill in the source and local interface addresses
		memcpy(&(packet->Source), &(UPnPObject->addr), INET_SOCKADDR_LENGTH(UPnPObject->addr.sin_family));
		ILibAsyncUDPSocket_GetLocalInterface(socketModule, (struct sockaddr*)&(packet->ReceivingAddress));

		if (packet->StatusCode == -1 && memcmp(packet->Directive, "M-SEARCH", 8) == 0)
		{
			// Process the search request with our regular Multicast M-SEARCH Handler
			UPnPProcessMSEARCH((struct UPnPDataObject*)UPnPObject, packet);
		}
		ILibDestructPacket(packet);
	}
}

void UPnPFreeUnicastReceiveSockets(struct UPnPDataObject *obj)
{
	int i;
	if (obj->UnicastReceiveSockets != NULL)
	{
		for(i = 0; i < obj->AddressListV4Length; ++i) if (obj->UnicastReceiveSockets[i] != NULL) ILibChain_SafeRemove(obj->Chain, obj->UnicastReceiveSockets[i]);
		free(obj->UnicastReceiveSockets);
		obj->UnicastReceiveSockets = NULL;
	}
}

void UPnPBindUnicastReceiveSockets(struct UPnPDataObject *obj)
{
	int i, OK = 0;
	UPnPFreeUnicastReceiveSockets(obj);
	if ((obj->UnicastReceiveSockets = (void**)malloc(obj->AddressListV4Length * sizeof(void*))) == NULL) ILIBCRITICALEXIT(254);

	do
	{
		obj->UnicastReceiveSocketPortNumber = (unsigned short)(1901 + ((unsigned short)rand() % 98));
		for(i = 0; i < obj->AddressListV4Length; ++i)
		{
			((struct sockaddr_in*)&(obj->AddressListV4[i]))->sin_port = obj->UnicastReceiveSocketPortNumber;

			obj->UnicastReceiveSockets[i] = ILibAsyncUDPSocket_CreateEx(
				obj->Chain,
				UPNP_MAX_SSDP_HEADER_SIZE,
				(struct sockaddr*)&(obj->AddressListV4[i]),
				ILibAsyncUDPSocket_Reuse_SHARED,
				&UPnPUnicastSSDPSink,
				NULL,
				obj);
			if (obj->UnicastReceiveSockets[i] == NULL)
			{
				UPnPFreeUnicastReceiveSockets(obj);
				OK = 0;
				break;
			}
			else
			{
				OK = 1;
			}
		}
	} while(!OK);
}
//{{{END_UPnP/1.1_Specific}}}
void UPnPSetDisconnectFlag(UPnPSessionToken token,void *flag)
{
	((struct ILibWebServer_Session*)token)->Reserved10=flag;
}
//{{{BEGIN_FragmentedResponseSystem}}}
/*! \defgroup FragmentResponse Fragmented Response System
\ingroup MicroStack
\brief Methods used by application to response to invocations in a fragmented manner.
\par
Typically an application will use one of the \a UPnPResponse_ methods to resond to an invocation
request. However, that requires that all of the arguments are known at the time that method is called.
There are times when the application may not be able to do that, such as when querying a back-end
server.<br><br>
In this case, the application can utilise the Fragmented Response System. The application would need
to call \a UPnPAsyncResponse_START exactly once to initialise the response. Then the application would
repeatedly call \a UPnPAsyncResponse_OUT for each argument in the response. Then finally, a call to
\a UPnPAsyncResponse_DONE to complete
\{
*/
/*! \fn UPnPAsyncResponse_START(const UPnPSessionToken UPnPToken, const char* actionName, const char* serviceUrnWithVersion)
\brief Fragmented Response Initializer
\param UPnPToken The UPnP token received in the invocation request
\param actionName The name of the method this response is for
\param serviceUrnWithVersion The full service type URN, this response is for.
\returns Send Status
*/
int UPnPAsyncResponse_START(const UPnPSessionToken UPnPToken, const char* actionName, const char* serviceUrnWithVersion)
{
	int RetVal = 0;
#if defined(WIN32) || defined(_WIN32_WCE)
	char* RESPONSE_HEADER = "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: WINDOWS, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#elif defined(__SYMBIAN32__)
	char* RESPONSE_HEADER = "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: SYMBIAN, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#else
	char* RESPONSE_HEADER = "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: POSIX, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#endif
	char* RESPONSE_BODY = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n<s:Body>\r\n<u:%sResponse xmlns:u=\"%s\">";
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;
	int headSize;
	char* head; 
	int headLength;

	if (session == NULL){return(1);}

	headSize = (int)strlen(RESPONSE_BODY) + (int)strlen(actionName) + (int)strlen(serviceUrnWithVersion) + 1;
	if ((head = (char*)malloc(headSize)) == NULL) ILIBCRITICALEXIT(254);
	headLength = snprintf(head, headSize, RESPONSE_BODY, actionName, serviceUrnWithVersion);

	RetVal = ILibWebServer_StreamHeader_Raw(session, 200, "OK", RESPONSE_HEADER, 1);
	if (RetVal >= 0)
	{
		RetVal = ILibWebServer_StreamBody(session, head, headLength, 0, 0);
	}
	else
	{
		free(head);
	}
	return RetVal;
}
/*! \fn UPnPAsyncResponse_DONE(const UPnPSessionToken UPnPToken, const char* actionName)
\brief Fragmented Response Finalizer
\param UPnPToken The UPnP token received in the invocation request
\param actionName The name of the method this response is for
\returns Send Status
*/
int UPnPAsyncResponse_DONE(const UPnPSessionToken UPnPToken, const char* actionName)
{
	char* RESPONSE_FOOTER = "</u:%sResponse>\r\n   </s:Body>\r\n</s:Envelope>";

	int footSize = (int)strlen(RESPONSE_FOOTER) + (int)strlen(actionName);
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;
	char* footer;
	int footLength;

	if ((footer = (char*)malloc(footSize)) == NULL) ILIBCRITICALEXIT(254);
	footLength = snprintf(footer, footSize, RESPONSE_FOOTER, actionName);

	return(ILibWebServer_StreamBody(session, footer, footLength, 0, 1));
}
/*! \fn UPnPAsyncResponse_OUT(const UPnPSessionToken UPnPToken, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg)
\brief Fragmented Response Data
\param UPnPToken The UPnP token received in the invocation request
\param outArgName Variable Name
\param bytes Variable Data \b Note: For string types, this MUST be escaped.
\param byteLength Length of \a bytes
\param bytesMemoryOwnership Memory Ownership flag for \a bytes
\param startArg Boolean. 1 to start response, 0 to continue response
\param endArg Boolean. 1 to finish response, 0 to continue response
\returns Send Status
*/
int UPnPAsyncResponse_OUT(const UPnPSessionToken UPnPToken, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg)
{
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;
	int RetVal = 0;

	if (startArg != 0)
	{
		RetVal = ILibWebServer_StreamBody(session, "<", 1, 1, 0);
		if (RetVal >= 0) RetVal = ILibWebServer_StreamBody(session, (char*)outArgName, (int)strlen(outArgName), 1, 0);
		if (RetVal >= 0) RetVal = ILibWebServer_StreamBody(session, ">", 1, 1, 0);
	}

	if (byteLength>0 && RetVal >= 0)
	{
		RetVal = ILibWebServer_StreamBody(session, (char*)bytes, byteLength, bytesMemoryOwnership, 0);
	}

	if (endArg != 0 && RetVal >= 0)
	{
		RetVal = ILibWebServer_StreamBody(session, "</", 2, 1, 0);
		if (RetVal >= 0) RetVal = ILibWebServer_StreamBody(session, (char*)outArgName, (int)strlen(outArgName), 1, 0);
		if (RetVal >= 0) RetVal = ILibWebServer_StreamBody(session, ">\r\n", 3, 1, 0);
	}
	return RetVal;
}
/*! \} */
//{{{END_FragmentedResponseSystem}}}

/*! \fn UPnPIPAddressListChanged(UPnPMicroStackToken MicroStackToken)
\brief Tell the underlying MicroStack that an IPAddress may have changed
\param MicroStackToken Microstack
*/
void UPnPIPAddressListChanged(UPnPMicroStackToken MicroStackToken)
{
	((struct UPnPDataObject*)MicroStackToken)->UpdateFlag = 1;
	ILibForceUnBlockChain(((struct UPnPDataObject*)MicroStackToken)->Chain);
}

void UPnPSSDPSink(ILibAsyncUDPSocket_SocketModule socketModule, char* buffer, int bufferLength, struct sockaddr_in6 *remoteInterface, void *user, void *user2, int *PAUSE)
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
			UPnPProcessMSEARCH(user, packet);
		}
		ILibDestructPacket(packet);
	}
}
//{{{BEGIN_MulticastEventing}}}
void OnUPnPMulticastEvent(ILibAsyncUDPSocket_SocketModule socketModule, char* buffer, int bufferLength, int remoteInterface, unsigned short remotePort, void *user, void *user2, int *PAUSE)
{
	int i;
	char *LVL;
	struct packetheader *p;
	char *body = NULL;
	int bodyLength = 0;
	enum MULTICAST_EVENT_TYPE eventType;
	struct ILibXMLNode *root,*current;
	void *pStack;
	char *ServiceType = NULL;
	int ServiceVersion = -1;
	char *DeviceUDN = NULL;
	char *ServiceID = NULL;
	char *VariableValue;
	int VariableValueLength;
	{{{VARDEFS}}}

	p = ILibParsePacketHeader(buffer, 0, bufferLength);
	if (p != NULL)
	{
		if (p->DirectiveLength == 6 && 
			strncasecmp(p->Directive, "NOTIFY", 6) == 0 &&
			ILibGetHeaderLine(p, "NT", 2) != NULL &&
			strcmp(ILibGetHeaderLine(p, "NT", 2), "upnp:event") == 0
			)
		{
			body = ILibGetHeaderLine(p, "USN" ,3);
			if (body != NULL)
			{
				i = ILibString_IndexOf(body, (int)strlen(body), "::", 2);
				DeviceUDN = body + 5; //uuid:
				body[i] = 0;
				ServiceType = body + i + 2; // ::
				i = ILibString_LastIndexOf(ServiceType, (int)strlen(ServiceType), ":", 1);
				ServiceType[i] = 0;
				ServiceVersion = atoi(ServiceType + i + 1);
			}

			body = p->Body;
			bodyLength = p->BodyLength;
			if (body == NULL)
			{
				i = ILibString_IndexOf(buffer, bufferLength, "\0\n\r\n", 4);
				if (i > 0)
				{
					body = buffer + i + 4;
					bodyLength = bufferLength - i - 4;
				}
			}
			if (body != NULL)
			{
				LVL = ILibGetHeaderLine(p, "LVL", 3);
				eventType = (enum MULTICAST_EVENT_TYPE)ILibFindEntryInTable(LVL, MULTICAST_EVENT_TYPE_DESCRIPTION);

				//
				// Parse the  Body
				//
				root=current=ILibParseXML(body, 0, bodyLength);
				if (ILibProcessXMLNodeList(current) == 0)
				{
					ILibXML_BuildNamespaceLookupTable(root);
					ILibCreateStack(&pStack);
					while(current != NULL)
					{
						current->Name[current->NameLength] = 0;
						if (ILibXML_LookupNamespace(current, current->NSTag, current->NSLength) != NULL &&
							strcmp(ILibXML_LookupNamespace(current, current->NSTag, current->NSLength), "urn:schemas-upnp-org:event-1-0") == 0 &&
							strcasecmp(current->Name, "propertyset") == 0)
						{
							ILibPushStack(&pStack,current);
							current=current->Next;
							while(current != NULL)
							{
								current->Name[current->NameLength] = 0;
								if ( ILibXML_LookupNamespace(current, current->NSTag, current->NSLength) != NULL &&
									strcmp(ILibXML_LookupNamespace(current, current->NSTag, current->NSLength),"urn:schemas-upnp-org:event-1-0") == 0 &&
									strcasecmp(current->Name, "property") == 0)
								{
									ILibPushStack(&pStack, current);
									current = current->Next;
									while(current != NULL)
									{
										// Check the variables
										current->Name[current->NameLength] = 0;
										VariableValueLength = ILibReadInnerXML(current, &VariableValue);
										VariableValue[VariableValueLength] = 0;
										//{{{BEGIN_CHECK_MULTICASTVARIABLE}}}
										if (strcmp(current->Name,"{{{VARNAME}}}") == 0)
										{
											if ((int)strlen(ServiceType) > {{{SERVICETYPELENGTH}}} && strncmp(ServiceType, "{{{SERVICETYPE}}}", {{{SERVICETYPELENGTH}}})==0)
											{
												if (UPnPOnMulticastEvent_{{{SERVICENAME}}}_{{{VARNAME}}}!=NULL)
												{
													{{{VARSERIALIZE}}}
													UPnPOnMulticastEvent_{{{SERVICENAME}}}_{{{VARNAME}}}(
														user,
														ServiceID,
														DeviceUDN,
														eventType,
													{{{VARDISPATCH}}}
													);
												}
											}
										}
										//{{{END_CHECK_MULTICASTVARIABLE}}}
										if (UPnPOnEvent_MulticastGeneric != NULL)
										{
											UPnPOnEvent_MulticastGeneric(
												user,
												ServiceType, 
												ServiceVersion, 
												ServiceID,
												DeviceUDN, 
												eventType,
												current->Name, 
												VariableValue
												);
										}

										current = current->Peer;
									}
									current = (struct ILibXMLNode*)ILibPopStack(&pStack);
								}
								current = current->Peer;
							}
							current = (struct ILibXMLNode*)ILibPopStack(&pStack);
						}
						current = current->Peer;
					}
				}
				ILibDestructXMLNodeList(root);
			}
		}
		ILibDestructPacket(p);
	}
}
//{{{END_MulticastEventing}}}
//
//	Internal underlying Initialization, that shouldn't be called explicitely
// 
// <param name="state">State object</param>
// <param name="NotifyCycleSeconds">Cycle duration</param>
// <param name="PortNumber">Port Number</param>
void UPnPInit(struct UPnPDataObject *state, void *chain, const int NotifyCycleSeconds, const unsigned short PortNumber)
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
			&UPnPSSDPSink,
			NULL,
			state);

		ILibAsyncUDPSocket_JoinMulticastGroupV4(state->NOTIFY_RECEIVE_socks[i], (struct sockaddr_in*)&(state->MulticastAddrV4), (struct sockaddr*)&(state->AddressListV4[i]));
		ILibAsyncUDPSocket_SetLocalInterface(state->NOTIFY_RECEIVE_socks[i], (struct sockaddr*)&(state->AddressListV4[i]));
		ILibAsyncUDPSocket_SetMulticastLoopback(state->NOTIFY_RECEIVE_socks[i], 1);
		//{{{BEGIN_MulticastEventing}}}
		/*
		state->MulticastEventListener = ILibAsyncUDPSocket_Create(state->Chain, UPNP_MAX_SSDP_HEADER_SIZE, 0, UPNP_MULTICASTEVENT_PORT, ILibAsyncUDPSocket_Reuse_SHARED, &OnUPnPMulticastEvent, NULL, state);
		ILibAsyncUDPSocket_JoinMulticastGroup(
			state->MulticastEventListener,
			state->AddressListV4[i],
			inet_addr(UPNP_MCASTv4_GROUP));
		*/
		//{{{END_MulticastEventing}}}
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
				&UPnPSSDPSink,
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
			//{{{BEGIN_MulticastEventing}}}
			/*
			state->MulticastEventListener = ILibAsyncUDPSocket_Create(state->Chain, UPNP_MAX_SSDP_HEADER_SIZE, 0, UPNP_MULTICASTEVENT_PORT, ILibAsyncUDPSocket_Reuse_SHARED, &OnUPnPMulticastEvent, NULL, state);
			ILibAsyncUDPSocket_JoinMulticastGroup(
				state->MulticastEventListener,
				state->AddressListV6[i],
				inet_addr(UPNP_MCASTv6_GROUP));
			*/
			//{{{END_MulticastEventing}}}
		}
	}
	//{{{BEGIN_UPnP/1.1_Specific}}}
	UPnPBindUnicastReceiveSockets(state);
	//{{{END_UPnP/1.1_Specific}}}
}

void UPnPPostMX_Destroy(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	free(mss->ST);
	free(mss);
}
//{{{BEGIN_VERSION>1}}}
int UPnPFixVersion(char *packet, char *ST, int version)
{
	int packetLen = (int)strlen(packet);
	int i = ILibString_IndexOf(packet, packetLen, ST, (int)strlen(ST));
	int i2 = 1 + ILibString_IndexOf(ST, (int)strlen(ST), ":",1);
	int i3 = ILibString_IndexOf(packet + i + i2, packetLen - (i + i2), "\r\n", 2);
	char val[32];
	int valLen;

	valLen = snprintf(val, 32, "%d", version);
	if (i3 < valLen)
	{
		memmove(packet+i+i2+valLen,packet+i+i2+i3,1+(packetLen-(i+i2+i3)));
		packetLen += (valLen - i3);
	}
	memcpy(packet + i + i2, val, valLen);	
	return(packetLen);
}
//{{{END_VERSION>1}}}
void UPnPOnPostMX_MSEARCH_SendOK(ILibAsyncUDPSocket_SocketModule socketModule, void *user1, void *user2)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)user1;
	UNREFERENCED_PARAMETER( socketModule );
	UNREFERENCED_PARAMETER( user2 );

	ILibChain_SafeRemove_SubChain(mss->Chain, mss->SubChain);
	free(mss->ST);
	free(mss);
}

void UPnPPostMX_MSEARCH(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	void *response_socket;
	void *subChain;
	char *ST = mss->ST;
	int STLength = mss->STLength;
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)mss->upnp;
	int rcode = 0;

	subChain = ILibCreateChain();

	response_socket = ILibAsyncUDPSocket_CreateEx(
		subChain,
		UPNP_MAX_SSDP_HEADER_SIZE,
		(struct sockaddr*)&(mss->localIPAddress),
		ILibAsyncUDPSocket_Reuse_SHARED,
		NULL,
		UPnPOnPostMX_MSEARCH_SendOK,
		mss);

	ILibChain_SafeAdd_SubChain(mss->Chain, subChain);
	mss->SubChain = subChain;

	// Search for root device
	if (STLength == 15 && memcmp(ST, "upnp:rootdevice", 15) == 0)
	{
		//{{{BEGIN_UPnP/1.0_Specific}}}
		UPnPBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
		//{{{END_UPnP/1.0_Specific}}}
		//{{{BEGIN_UPnP/1.1_Specific}}}
		UPnPBuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, "::upnp:rootdevice", "upnp:rootdevice", "");
		//{{{END_UPnP/1.1_Specific}}}
	}
	// Search for everything
	else if (STLength == 8 && memcmp(ST, "ssdp:all", 8) == 0)
	{
		//{{{SSDP:ALL}}}
	}
	//{{{SSDP:OTHER}}}

	if (rcode == 0)
	{
		ILibChain_SafeRemove_SubChain(mss->Chain, subChain);
		free(mss->ST);
		free(mss);
	}
}

void UPnPProcessMSEARCH(struct UPnPDataObject *upnp, struct packetheader *packet)
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
				ILibLifeTime_Add(upnp->WebServerTimer, mss, MX, &UPnPPostMX_MSEARCH, &UPnPPostMX_Destroy);
			}
			else
			{
				free(ST);
			}
		}
	}
}

//{{{DispatchMethods}}}

int UPnPProcessPOST(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
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
	//{{{DispatchControl}}}
	return(RetVal);
}
//{{{BEGIN_EVENTPROCESSING}}}
struct SubscriberInfo* UPnPRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers, char* SID, int SIDLength)
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

#define UPnPDestructSubscriberInfo(info)\
{\
	free(info->Path);\
	free(info->SID);\
	free(info);\
}

#define UPnPDestructEventObject(EvObject)\
{\
	free(EvObject->PacketBody);\
	free(EvObject);\
}

#define UPnPDestructEventDataObject(EvData)\
{\
	free(EvData);\
}

void UPnPExpireSubscriberInfo(struct UPnPDataObject *d, struct SubscriberInfo *info)
{
	struct SubscriberInfo *t = info;
	while(t->Previous != NULL) { t = t->Previous; }
	//{{{UPnPExpireSubscriberInfo1}}}

	if (info->Previous != NULL)
	{
		// This is not the Head
		info->Previous->Next = info->Next;
		if (info->Next != NULL) { info->Next->Previous = info->Previous; }
	}
	else
	{
		// This is the Head
		//{{{UPnPExpireSubscriberInfo2}}}
	}
	--info->RefCount;
	if (info->RefCount == 0) { UPnPDestructSubscriberInfo(info); }
}

int UPnPSubscriptionExpired(struct SubscriberInfo *info)
{
	int RetVal = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ((info->RenewByTime).tv_sec < tv.tv_sec) { RetVal = -1; }
	return(RetVal);
}

//{{{InitialEventBody}}}

void UPnPProcessUNSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
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
	sem_wait(&(((struct UPnPDataObject*)session->User)->EventLock));
	//{{{UnSubscribeDispatcher}}}
	sem_post(&(((struct UPnPDataObject*)session->User)->EventLock));
}

void UPnPTryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength, struct ILibWebServer_Session *session)
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

	struct UPnPDataObject *dataObject = (struct UPnPDataObject*)session->User;

	//{{{SubscribeHeadPointerInitializer}}}

	if (*HeadPtr!=NULL)
	{
		NewSubscriber = *HeadPtr;
		while(NewSubscriber != NULL)
		{
			if (UPnPSubscriptionExpired(NewSubscriber) != 0)
			{
				TempSubscriber = NewSubscriber->Next;
				NewSubscriber = UPnPRemoveSubscriberInfo(HeadPtr, TotalSubscribers, NewSubscriber->SID, NewSubscriber->SIDLength);
				UPnPDestructSubscriberInfo(NewSubscriber);
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
		//{{{BEGIN_UPnP/1.1_Specific}}}
		NewSubscriber->NotLegacy = session->Reserved9;
		//{{{END_UPnP/1.1_Specific}}}

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

		len = 134 + (int)strlen(SID) + (int)strlen(UPnPPLATFORM) + 4;
		if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		packetlength = snprintf(packet, len, "HTTP/!HTTPVERSION! 200 OK\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",UPnPPLATFORM,SID,Timeout);
		//{{{TryToSubscribe_InitialEvent}}}
		if (packetbody != NULL)
		{
			ILibWebServer_Send_Raw(session, packet, packetlength, 0, 1);
			UPnPSendEvent_Body(dataObject, packetbody, packetbodyLength, NewSubscriber);
			free(packetbody);
		} 
	}
	else
	{
		// Too many subscribers
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Too Many Subscribers\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}
}

void UPnPSubscribeEvents(char* path, int pathlength, char* Timeout, int TimeoutLength, char* URL, int URLLength, struct ILibWebServer_Session* session)
{
	long TimeoutVal;
	char* buffer;
	if ((buffer = (char*)malloc(1 + sizeof(char)*pathlength)) == NULL) ILIBCRITICALEXIT(254);

	ILibGetLong(Timeout, TimeoutLength, &TimeoutVal);
	memcpy(buffer, path, pathlength);
	buffer[pathlength] = '\0';
	free(buffer);
	if (TimeoutVal>UPnP_MAX_SUBSCRIPTION_TIMEOUT) { TimeoutVal = UPnP_MAX_SUBSCRIPTION_TIMEOUT; }
	//{{{SubscribeEventsDispatcher}}}
}

void UPnPRenewEvents(char* path, int pathlength, char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct ILibWebServer_Session *ReaderObject)
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
	//{{{RenewHeadInitializer}}}

	// Find this SID in the subscriber list, and recalculate the expiration timeout
	while(info != NULL && strcmp(info->SID,SID) != 0)
	{
		info = info->Next;
	}
	if (info != NULL)
	{
		ILibGetLong(Timeout, TimeoutLength, &TimeoutVal);
		if (TimeoutVal>UPnP_MAX_SUBSCRIPTION_TIMEOUT) {TimeoutVal = UPnP_MAX_SUBSCRIPTION_TIMEOUT;}

		gettimeofday(&tv,NULL);
		(info->RenewByTime).tv_sec = tv.tv_sec + TimeoutVal;

		len = 134 + strlen(SID) + 4;
		if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		packetlength = snprintf(packet, len, "HTTP/!HTTPVERSION! 200 OK\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",UPnPPLATFORM,SID,TimeoutVal);
		ILibWebServer_Send_Raw(ReaderObject, packet, packetlength, 0, 1);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n", TimeoutVal, info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		ILibWebServer_Send_Raw(ReaderObject, "HTTP/!HTTPVERSION! 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n", 55, 1, 1);
	}
	free(SID);
}

void UPnPProcessSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
	char* SID = NULL;
	int SIDLength = 0;
	char* Timeout = NULL;
	int TimeoutLength = 0;
	char* URL = NULL;
	int URLLength = 0;
	struct parser_result *p;
	//{{{BEGIN_UPnP/1.1_Specific}}}
	struct parser_result *r,*r2;
	struct parser_result_field *prf;
	//{{{END_UPnP/1.1_Specific}}}
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
		//{{{BEGIN_UPnP/1.1_Specific}}}
		else if (f->FieldLength == 10 && strncasecmp(f->Field, "USER-AGENT", 10) == 0)
		{
			// Check UPnP version of the Control Point which invoked us
			r = ILibParseString(f->FieldData,0,f->FieldDataLength," ",1);
			prf = r->FirstResult;
			while(prf!=NULL)
			{
				if (prf->datalength>5 && memcmp(prf->data, "UPnP/", 5) == 0)
				{
					r2 = ILibParseString(prf->data + 5, 0, prf->datalength - 5, ".", 1);
					r2->FirstResult->data[r2->FirstResult->datalength] = 0;
					r2->LastResult->data[r2->LastResult->datalength] = 0;
					if (atoi(r2->FirstResult->data) == 1 && atoi(r2->LastResult->data) > 0) session->Reserved9 = 1;
					ILibDestructParserResults(r2);
				}
				prf = prf->NextResult;
			}
			ILibDestructParserResults(r);
		}
		//{{{END_UPnP/1.1_Specific}}}
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
		UPnPSubscribeEvents(header->DirectiveObj, header->DirectiveObjLength, Timeout, TimeoutLength, URL, URLLength, session);
	}
	else
	{
		// If a SID was specified, it is a renewal request for an existing subscription
		// Renew
		UPnPRenewEvents(header->DirectiveObj, header->DirectiveObjLength, SID, SIDLength, Timeout, TimeoutLength, session);
	}
}
//{{{END_EVENTPROCESSING}}}
//{{{Device_Object_Model_BEGIN}}}
void UPnPStreamDescriptionDocument_SCPD(struct ILibWebServer_Session *session, int StartActionList, char *buffer, int offset, int length, int DoneActionList, int Done)
{
	if (StartActionList)
	{
		ILibWebServer_StreamBody(session, "<?xml version=\"1.0\" encoding=\"utf-8\" ?><scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"><specVersion><major>1</major><minor>0</minor></specVersion><actionList>", 157, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	}
	if (buffer != NULL)
	{
		ILibWebServer_StreamBody(session, buffer + offset, length, ILibAsyncSocket_MemoryOwnership_USER, 0);
	}
	if (DoneActionList)
	{
		ILibWebServer_StreamBody(session, "</actionList><serviceStateTable>", 32, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	}
	if (Done)
	{
		ILibWebServer_StreamBody(session, "</serviceStateTable></scpd>", 27, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
	}
}
//{{{Device_Object_Model_END}}}
void UPnPProcessHTTPPacket(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)
{
	//{{{Device_Default_Model_BEGIN}}}
	struct UPnPDataObject *dataObject = (struct UPnPDataObject*)session->User;
	//{{{Device_Default_Model_END}}}
	//{{{Device_Object_Model_BEGIN}}}
	//{{{HASALLOWEDVALUES_BEGIN}}}int i;
	//{{{HASALLOWEDVALUES_END}}}
	//{{{Device_Object_Model_END}}}
#if defined(WIN32) || defined(_WIN32_WCE)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: WINDOWS, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#elif defined(__SYMBIAN32__)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: SYMBIAN, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#else
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: POSIX, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#endif
	char *errorTemplate = "HTTP/!HTTPVERSION! %d %s\r\nServer: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nContent-Length: 0\r\n\r\n";
	char *errorPacket;
	int errorPacketLength;
	char *buffer;

	LVL3DEBUG(errorPacketLength = ILibGetRawPacket(header, &errorPacket);)
	LVL3DEBUG(printf("%s\r\n",errorPacket);)
	LVL3DEBUG(free(errorPacket);)			

	//{{{PRESENTATIONPAGE}}}
	if (header->DirectiveLength == 4 && memcmp(header->Directive,"HEAD", 4) == 0)
	{
		if (header->DirectiveObjLength == 1 && memcmp(header->DirectiveObj, "/", 1) == 0)
		{
			// A HEAD request for the device description document.
			// We stream the document back, so we don't return content length or anything because the actual response won't have it either
			ILibWebServer_StreamHeader_Raw(session, 200, "OK", responseHeader, 1);
			ILibWebServer_StreamBody(session, NULL, 0, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		//{{{DeviceIcon_Begin}}}
		else if (header->DirectiveObjLength == 9 && memcmp(header->DirectiveObj, "/icon.png", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_SMPNG, {{{IconLength_HEAD_SMPNG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength == 9 && memcmp(header->DirectiveObj, "/icon.jpg", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_SMJPG, {{{IconLength_HEAD_SMJPG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength==10 && memcmp(header->DirectiveObj, "/icon2.png", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_LGPNG, {{{IconLength_HEAD_LGPNG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength==10 && memcmp(header->DirectiveObj, "/icon2.jpg", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_LGJPG, {{{IconLength_HEAD_LGJPG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		//{{{DeviceIcon_End}}}
		//{{{HeadDispatcher}}}
		else
		{
			// A HEAD request for something we don't have
			if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
			errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 404, "File Not Found", UPnPPLATFORM);
			ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, 0, 1);
		}
	}
	else if (header->DirectiveLength == 3 && memcmp(header->Directive, "GET", 3) == 0)
	{
		if (header->DirectiveObjLength == 1 && memcmp(header->DirectiveObj,"/", 1) == 0)
		{
			// A GET Request for the device description document, so lets stream it back to the client
			//{{{Device_Object_Model_BEGIN}}}
			UPnPStreamDescriptionDocument(session);
			//{{{Device_Object_Model_END}}}
			//{{{Device_Default_Model_BEGIN}}}
			ILibWebServer_StreamHeader_Raw(session, 200, "OK", responseHeader, 1);
			ILibWebServer_StreamBody(session, dataObject->DeviceDescription, dataObject->DeviceDescriptionLength, 1, 1);
			//{{{Device_Default_Model_END}}}
		}
		//{{{DeviceIcon_Begin}}}
		else if (header->DirectiveObjLength == 9 && memcmp(header->DirectiveObj, "/icon.png", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_SMPNG, {{{IconLength_SMPNG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength == 10 && memcmp(header->DirectiveObj, "/icon2.png", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_LGPNG, {{{IconLength_LGPNG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength == 9 && memcmp(header->DirectiveObj, "/icon.jpg", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_SMJPG, {{{IconLength_SMJPG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		else if (header->DirectiveObjLength == 10 && memcmp(header->DirectiveObj, "/icon2.jpg", 1) == 0)
		{
			ILibWebServer_Send_Raw(session, (char*)UPnPDeviceIcon_LGJPG, {{{IconLength_LGJPG}}}, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
		}
		//{{{DeviceIcon_End}}}
		//{{{GetDispatcher}}}
		else
		{
			// A GET Request for something we don't have
			if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
			errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 404, "File Not Found", UPnPPLATFORM);
			ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, 0, 1);
		}
	}
	else if (header->DirectiveLength == 4 && memcmp(header->Directive,"POST",4) == 0)
	{
		// Defer Control to the POST Handler
		if (UPnPProcessPOST(session, header, bodyBuffer, offset, bodyBufferLength) != 0)
		{
			// A POST for an action that doesn't exist
			UPnPResponse_Error(session, 401, "Invalid Action");
		}
	}
	//{{{BEGIN_EVENTPROCESSING}}}
	else if (header->DirectiveLength == 9 && memcmp(header->Directive, "SUBSCRIBE" ,9) == 0)
	{
		// Subscription Handler
		UPnPProcessSUBSCRIBE(header,session);
	}
	else if (header->DirectiveLength == 11 && memcmp(header->Directive, "UNSUBSCRIBE", 11) == 0)
	{
		// UnSubscribe Handler
		UPnPProcessUNSUBSCRIBE(header,session);
	}//{{{END_EVENTPROCESSING}}}
	else
	{
		// The client tried something we didn't expect/support
		if ((errorPacket = (char*)malloc(128)) == NULL) ILIBCRITICALEXIT(254);
		errorPacketLength = snprintf(errorPacket, 128, errorTemplate, 400, "Bad Request", UPnPPLATFORM);
		ILibWebServer_Send_Raw(session, errorPacket, errorPacketLength, ILibAsyncSocket_MemoryOwnership_CHAIN, 1);
	}
}
void UPnPFragmentedSendNotify_Destroy(void *data);
void UPnPMasterPreSelect(void* object, void *socketset, void *writeset, void *errorset, int* blocktime)
{
	int i;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;	
	struct UPnPFragmentNotifyStruct *f;
	int timeout;
	UNREFERENCED_PARAMETER( socketset );
	UNREFERENCED_PARAMETER( writeset );
	UNREFERENCED_PARAMETER( errorset );
	UNREFERENCED_PARAMETER( blocktime );

	if (UPnPObject->InitialNotify == 0)
	{
		// The initial "HELLO" packets were not sent yet, so lets send them
		UPnPObject->InitialNotify = -1;

		// In case we were interrupted, we need to flush out the caches of
		// all the control points by sending a "byebye" first, to insure
		// control points don't ignore our "hello" packets thinking they are just
		// periodic re-advertisements.
		UPnPSendByeBye(UPnPObject);

		// PacketNumber 0 is the controller, for the rest of the packets. Send
		// one of these to send out an advertisement "group"
		if ((f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof(struct UPnPFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
		f->packetNumber = 0;
		f->upnp = UPnPObject;

		// We need to inject some delay in these packets to space them out,
		// otherwise we could overflow the inbound buffer of the recipient, causing them
		// to lose packets. And upnp/1.0 control points are not as robust as upnp/1.1 control points,
		// so they need all the help they can get ;)
		timeout = (int)(0 + ((unsigned short)rand() % (500)));
		do { f->upnp->InitialNotify = rand(); } while (f->upnp->InitialNotify == 0);

		// Register for the timed callback, to actually send the packet
		ILibLifeTime_AddEx(f->upnp->WebServerTimer, f, timeout, &UPnPFragmentedSendNotify, &UPnPFragmentedSendNotify_Destroy);
	}

	if (UPnPObject->UpdateFlag != 0)
	{
		// Somebody told us that we should recheck our IP Address table, as one of them may have changed
		UPnPObject->UpdateFlag = 0;

		//{{{BEGIN_UPnP/1.1_Specific}}}
		// Release all of our upnp/1.1 unicast sockets. We'll re-initialise them when we iterate through all the current IP Addresses
		UPnPFreeUnicastReceiveSockets(UPnPObject);
		//{{{END_UPnP/1.1_Specific}}}
		//{{{BEGIN_MulticastEventing}}}
		ILibChain_SafeRemove(UPnPObject->Chain, UPnPObject->MulticastEventListener);
		//{{{END_MulticastEventing}}}
		// Clear Sockets
		// Iterate through all the currently bound IPv4 addresses and release the sockets
		if (UPnPObject->AddressListV4 != NULL)
		{
			for (i = 0; i < UPnPObject->AddressListV4Length; ++i) ILibChain_SafeRemove(UPnPObject->Chain, UPnPObject->NOTIFY_SEND_socks[i]);
			free(UPnPObject->NOTIFY_SEND_socks);
			for (i = 0; i < UPnPObject->AddressListV4Length; ++i) ILibChain_SafeRemove(UPnPObject->Chain, UPnPObject->NOTIFY_RECEIVE_socks[i]);
			free(UPnPObject->NOTIFY_RECEIVE_socks);
			free(UPnPObject->AddressListV4);
		}
		if (UPnPObject->AddressListV6 != NULL)
		{
			for (i = 0; i < UPnPObject->AddressListV6Length; ++i) ILibChain_SafeRemove(UPnPObject->Chain, UPnPObject->NOTIFY_SEND_socks6[i]);
			free(UPnPObject->NOTIFY_SEND_socks6);
			for (i = 0; i < UPnPObject->AddressListV6Length; ++i) ILibChain_SafeRemove(UPnPObject->Chain, UPnPObject->NOTIFY_RECEIVE_socks6[i]);
			free(UPnPObject->NOTIFY_RECEIVE_socks6);
			free(UPnPObject->AddressListV6);
		}

		// Fetch a current list of ip addresses
		UPnPObject->AddressListV4Length = ILibGetLocalIPv4AddressList(&(UPnPObject->AddressListV4), 1);

		// Re-Initialize our SEND socket
		if ((UPnPObject->NOTIFY_SEND_socks = (void**)malloc(sizeof(void*)*(UPnPObject->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);
		if ((UPnPObject->NOTIFY_RECEIVE_socks = (void**)malloc(sizeof(void*)*(UPnPObject->AddressListV4Length))) == NULL) ILIBCRITICALEXIT(254);

		// Test IPv6 support
		if (ILibDetectIPv6Support())
		{
			// Fetch the list of local IPv6 interfaces
			UPnPObject->AddressListV6Length = ILibGetLocalIPv6List(&(UPnPObject->AddressListV6));

			// Setup the IPv6 sockets
			if ((UPnPObject->NOTIFY_SEND_socks6 = (void**)malloc(sizeof(void*)*(UPnPObject->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
			if ((UPnPObject->NOTIFY_RECEIVE_socks6 = (void**)malloc(sizeof(void*)*(UPnPObject->AddressListV6Length))) == NULL) ILIBCRITICALEXIT(254);
		}

		// Iterate through all the current IP Addresses
		for (i = 0; i < UPnPObject->AddressListV4Length; ++i)
		{
			(UPnPObject->AddressListV4[i]).sin_port = 0; // Bind to ANY port for outbound packets
			UPnPObject->NOTIFY_SEND_socks[i] = ILibAsyncUDPSocket_CreateEx(
				UPnPObject->Chain,
				UPNP_MAX_SSDP_HEADER_SIZE,
				(struct sockaddr*)&(UPnPObject->AddressListV4[i]),
				ILibAsyncUDPSocket_Reuse_SHARED,
				NULL,
				NULL,
				UPnPObject);

			ILibAsyncUDPSocket_SetMulticastTTL(UPnPObject->NOTIFY_SEND_socks[i], UPNP_SSDP_TTL);
			ILibAsyncUDPSocket_SetMulticastLoopback(UPnPObject->NOTIFY_SEND_socks[i], 1);

			(UPnPObject->AddressListV4[i]).sin_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
			UPnPObject->NOTIFY_RECEIVE_socks[i] = ILibAsyncUDPSocket_CreateEx(
				UPnPObject->Chain,
				UPNP_MAX_SSDP_HEADER_SIZE,
				(struct sockaddr*)&(UPnPObject->AddressListV4[i]),
				ILibAsyncUDPSocket_Reuse_SHARED,
				&UPnPSSDPSink,
				NULL,
				UPnPObject);

			ILibAsyncUDPSocket_JoinMulticastGroupV4(UPnPObject->NOTIFY_RECEIVE_socks[i], (struct sockaddr_in*)&(UPnPObject->MulticastAddrV4), (struct sockaddr*)&(UPnPObject->AddressListV4[i]));
			ILibAsyncUDPSocket_SetMulticastLoopback(UPnPObject->NOTIFY_RECEIVE_socks[i], 1);
			//{{{BEGIN_MulticastEventing}}}
			/*
			UPnPObject->MulticastEventListener = ILibAsyncUDPSocket_Create(UPnPObject->Chain, UPNP_MAX_SSDP_HEADER_SIZE, 0, UPNP_MULTICASTEVENT_PORT, ILibAsyncUDPSocket_Reuse_SHARED, &OnUPnPMulticastEvent, NULL, UPnPObject);
			ILibAsyncUDPSocket_JoinMulticastGroup(UPnPObject->MulticastEventListener, UPnPObject->AddressList[i], inet_addr(UPNP_MCASTv4_GROUP));
			*/
			//{{{END_MulticastEventing}}}
		}
		//{{{BEGIN_UPnP/1.1_Specific}}}
		// Re-initialise our UPnP/1.1 unicast sockets
		UPnPBindUnicastReceiveSockets(UPnPObject);
		//{{{END_UPnP/1.1_Specific}}}

		if (UPnPObject->AddressListV6Length > 0)
		{
			// Iterate through all the current IPv6 interfaces
			for (i = 0; i < UPnPObject->AddressListV6Length; ++i)
			{
				(UPnPObject->AddressListV6[i]).sin6_port = 0;
				UPnPObject->NOTIFY_SEND_socks6[i] = ILibAsyncUDPSocket_CreateEx(
					UPnPObject->Chain,
					UPNP_MAX_SSDP_HEADER_SIZE,
					(struct sockaddr*)&(UPnPObject->AddressListV6[i]),
					ILibAsyncUDPSocket_Reuse_SHARED,
					NULL,
					NULL,
					UPnPObject);

				ILibAsyncUDPSocket_SetMulticastTTL(UPnPObject->NOTIFY_SEND_socks6[i], UPNP_SSDP_TTL);
				ILibAsyncUDPSocket_SetMulticastLoopback(UPnPObject->NOTIFY_SEND_socks6[i], 1);

				(UPnPObject->AddressListV6[i]).sin6_port = htons(UPNP_PORT); // Bind to UPnP port for inbound packets
				UPnPObject->NOTIFY_RECEIVE_socks6[i] = ILibAsyncUDPSocket_CreateEx(
					UPnPObject->Chain,
					UPNP_MAX_SSDP_HEADER_SIZE,
					(struct sockaddr*)&(UPnPObject->AddressListV6[i]),
					ILibAsyncUDPSocket_Reuse_SHARED,
					&UPnPSSDPSink,
					NULL,
					UPnPObject);

				if (ILibAsyncSocket_IsIPv6LinkLocal((struct sockaddr*)&(UPnPObject->AddressListV6[i])))
				{
					ILibAsyncUDPSocket_JoinMulticastGroupV6(UPnPObject->NOTIFY_RECEIVE_socks6[i], &(UPnPObject->MulticastAddrV6LL), UPnPObject->AddressListV6[i].sin6_scope_id);
				}
				else
				{
					ILibAsyncUDPSocket_JoinMulticastGroupV6(UPnPObject->NOTIFY_RECEIVE_socks6[i], &(UPnPObject->MulticastAddrV6SL), UPnPObject->AddressListV6[i].sin6_scope_id);
				}
				ILibAsyncUDPSocket_SetMulticastLoopback(UPnPObject->NOTIFY_RECEIVE_socks6[i], 1);
				//{{{BEGIN_MulticastEventing}}}
				/*
				UPnPObject->MulticastEventListener = ILibAsyncUDPSocket_Create(UPnPObject->Chain, UPNP_MAX_SSDP_HEADER_SIZE, 0, UPNP_MULTICASTEVENT_PORT, ILibAsyncUDPSocket_Reuse_SHARED, &OnUPnPMulticastEvent, NULL, UPnPObject);
				ILibAsyncUDPSocket_JoinMulticastGroup(UPnPObject->MulticastEventListener, UPnPObject->AddressList[i], inet_addr(UPNP_MCASTv6_GROUP));
				*/
				//{{{END_MulticastEventing}}}
			}
		}

		// Iterate through all the packet types, and re-broadcast
		for (i = 1; i <= 4; ++i)
		{
			if ((f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof(struct UPnPFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i;
			f->upnp = UPnPObject;

			// Inject some random delay, to spread these packets out, to help prevent the inbound buffer of the recipient from overflowing, causing dropped packets.
			timeout = (int)(0 + ((unsigned short)rand() % (500)));
			ILibLifeTime_AddEx(f->upnp->WebServerTimer, f, timeout, &UPnPFragmentedSendNotify, &UPnPFragmentedSendNotify_Destroy);
		}
	}
}

void UPnPFragmentedSendNotify_Destroy(void *data)
{
	free(data);
}

void UPnPFragmentedSendNotify(void *data)
{
	int i,i2;
	int subsetRange;
	int timeout, timeout2;
	struct UPnPFragmentNotifyStruct *f;
	struct UPnPFragmentNotifyStruct *FNS = (struct UPnPFragmentNotifyStruct*)data;

	if (FNS->packetNumber == 0)
	{				
		subsetRange = 5000 / 5; // Make sure all our packets will get out within 5 seconds

		// Send the first "group"
		for (i2 = 0; i2 <= 4; ++i2)
		{
			if ((f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof(struct UPnPFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i2 + 1;
			f->upnp = FNS->upnp;
			timeout2 = (rand() % subsetRange);
			ILibLifeTime_AddEx(FNS->upnp->WebServerTimer, f, timeout2, &UPnPFragmentedSendNotify, &UPnPFragmentedSendNotify_Destroy);
		}

		// Now Repeat this "group" after 7 seconds, to insure there is no overlap
		for (i2 = 0; i2 <= 4; ++i2)
		{
			if ((f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof(struct UPnPFragmentNotifyStruct))) == NULL) ILIBCRITICALEXIT(254);
			f->packetNumber = i2 + 1;
			f->upnp = FNS->upnp;
			timeout2 = 7000 + (rand() % subsetRange);
			ILibLifeTime_AddEx(FNS->upnp->WebServerTimer, f, timeout2, &UPnPFragmentedSendNotify, &UPnPFragmentedSendNotify_Destroy);
		}

		// Calculate the next transmission window and spread the packets
		timeout = (int)((FNS->upnp->NotifyCycleTime / 4) + ((unsigned short)rand() % (FNS->upnp->NotifyCycleTime / 2 - FNS->upnp->NotifyCycleTime / 4)));
		ILibLifeTime_Add(FNS->upnp->WebServerTimer, FNS, timeout, &UPnPFragmentedSendNotify, &UPnPFragmentedSendNotify_Destroy);
	}

	for (i = 0; i < FNS->upnp->AddressListV4Length; ++i)
	{
		switch(FNS->packetNumber)
		{
			//{{{FragmentedSendNotifyCaseStatements}}}
		}
	}

	for (i = 0; i < FNS->upnp->AddressListV6Length; ++i)
	{
		switch(FNS->packetNumber)
		{
			//{{{FragmentedSendNotifyV6CaseStatements}}}
		}
	}

	if (FNS->packetNumber != 0) free(FNS);
}


void UPnPSendNotify(const struct UPnPDataObject *upnp)
{
	int i, i2;
	for (i=0;i<upnp->AddressListV4Length;++i)
	{
		for (i2=0; i2<2; i2++)
		{
			//{{{SendNotifyForStatement}}}
		}
	}
	for (i=0;i<upnp->AddressListV6Length;++i)
	{
		for (i2=0; i2<2; i2++)
		{
			//{{{SendNotifyV6ForStatement}}}
		}
	}
}

//{{{BEGIN_UPnP/1.0_Specific}}}
int UPnPBuildSendSsdpByeByePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* target, char* mcastgroup, char* USNex, char* NT, char* NTex, int DeviceID)
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
//{{{END_UPnP/1.0_Specific}}}
//{{{BEGIN_UPnP/1.1_Specific}}}
int UPnPBuildSendSsdpByeByePacket(void* module, const struct UPnPDataObject *upnp, struct sockaddr* target, char* mcastgroup, char* USNex, char* NT, char* NTex, int DeviceID)
{
	int len;

	if (DeviceID == 0)
	{
		len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, mcastgroup, upnp->UDN, USNex, NT, NTex);
	}
	else
	{
		if (memcmp(NT, "uuid:", 5) == 0)
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s_%d\r\nContent-Length: 0\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, mcastgroup, upnp->UDN, DeviceID, USNex, NT, NTex, DeviceID);
		}
		else
		{
			len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "NOTIFY * HTTP/1.1\r\nCONFIGID.UPNP.ORG: %d\r\nSEARCHPORT.UPNP.ORG: %u\r\nBOOTID.UPNP.ORG: %d\r\nMAXVERSION.UPNP.ORG: %d\r\nHOST: %s:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s_%d%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n", upnp->ConfigID, upnp->UnicastReceiveSocketPortNumber, upnp->InitialNotify, 1, mcastgroup, upnp->UDN, DeviceID, USNex, NT, NTex);
		}
	}
	return ILibAsyncUDPSocket_SendTo(module, target, ILibScratchPad, len, ILibAsyncSocket_MemoryOwnership_USER);
}
//{{{END_UPnP/1.1_Specific}}}

void UPnPSendByeBye(const struct UPnPDataObject *upnp)
{
	int i, i2;
	struct sockaddr* t1;
	char* t2;
	//{{{BEGIN_UPnP/1.1_Specific}}}
	int TempVal = 0;
	//{{{END_UPnP/1.1_Specific}}}

	for (i=0; i<upnp->AddressListV4Length; ++i)
	{	
		//{{{BEGIN_UPnP/1.1_Specific}}}
		if (upnp->InitialNotify != -1) TempVal = upnp->InitialNotify;  
		//{{{END_UPnP/1.1_Specific}}}
		for (i2=0; i2<2; i2++)
		{
			//{{{SendByeByeForStatement}}}
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
			//{{{SendByeByeV6ForStatement}}}
		}
	}
}

/*! \fn UPnPResponse_Error(const UPnPSessionToken UPnPToken, const int ErrorCode, const char* ErrorMsg)
\brief Responds to the client invocation with a SOAP Fault
\param UPnPToken UPnP token
\param ErrorCode Fault Code
\param ErrorMsg Error Detail
*/
void UPnPResponse_Error(const UPnPSessionToken UPnPToken, const int ErrorCode, const char* ErrorMsg)
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
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)UPnPToken, head, headlength, 0, 0);
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)UPnPToken, body, bodylength, 0, 1);
}

void UPnPResponseGeneric(const UPnPMicroStackToken UPnPToken, const char* ServiceURI, const char* MethodName, const char* Params)
{
	int RVAL = 0;
	char* packet;
	int packetlength;
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;
	size_t len = 239 + strlen(ServiceURI) + strlen(Params) + (strlen(MethodName) * 2);

	if ((packet = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	packetlength = snprintf(packet, len, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
	LVL3DEBUG(printf("SendBody: %s\r\n", packet);)
#if defined(WIN32) || defined(_WIN32_WCE)
		RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: WINDOWS, UPnP/1.0, MicroStack/!MICROSTACKVERSION!", 1);
#elif defined(__SYMBIAN32__)
		RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: SYMBIAN, UPnP/1.0, MicroStack/!MICROSTACKVERSION!", 1);
#else
		RVAL=ILibWebServer_StreamHeader_Raw(session, 200, "OK", "\r\nEXT:\r\nCONTENT-TYPE: text/xml; charset=\"utf-8\"\r\nSERVER: POSIX, UPnP/1.0, MicroStack/!MICROSTACKVERSION!", 1);
#endif
	if (RVAL!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RVAL != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
	{
		RVAL=ILibWebServer_StreamBody(session, packet, packetlength, 0, 1);
	}
}

//{{{InvokeResponseMethods}}}
//{{{BEGIN_EVENTPROCESSING}}}
void UPnPSendEventSink(
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
		sem_wait(&(((struct UPnPDataObject*)upnp)->EventLock));
		--((struct SubscriberInfo*)subscriber)->RefCount;
		if (((struct SubscriberInfo*)subscriber)->RefCount == 0)
		{
			LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n", ((struct SubscriberInfo*)subscriber)->SID, (((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF), ((struct SubscriberInfo*)subscriber)->Port);)
			UPnPDestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
		}
		else if (header == NULL)
		{
			LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n", ((struct SubscriberInfo*)subscriber)->SID, (((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF), ((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF), ((struct SubscriberInfo*)subscriber)->Port);)
			// Could not send Event, so unsubscribe the subscriber
			((struct SubscriberInfo*)subscriber)->Disposing = 1;
			UPnPExpireSubscriberInfo(upnp, subscriber);
		}
		sem_post(&(((struct UPnPDataObject*)upnp)->EventLock));
	}
}

void UPnPSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info)
{
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
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
	packetLength = snprintf(packet, len, "NOTIFY %s HTTP/!HTTPVERSION!\r\nSERVER: %s, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!\r\nHOST: %s:%d\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,UPnPPLATFORM,inet_ntoa(dest.sin_addr),info->Port,info->SID,info->SEQ,bodylength+137,body);
	++info->SEQ;

	++info->RefCount;
	ILibWebClient_PipelineRequestEx(UPnPObject->EventClient, (struct sockaddr*)(&dest), packet, packetLength, 0, NULL, 0, 0, &UPnPSendEventSink, info, upnptoken);
}

void UPnPSendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
	struct SubscriberInfo *info = NULL;
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
	struct sockaddr_in dest;
	LVL3DEBUG(struct timeval tv;)

	if (UPnPObject == NULL)
	{
		free(body);
		return;
	}
	sem_wait(&(UPnPObject->EventLock));
	//{{{SendEventHeadPointerInitializer}}}
	memset(&dest, 0, sizeof(dest));
	while(info != NULL)
	{
		if (!UPnPSubscriptionExpired(info))
		{
			UPnPSendEvent_Body(upnptoken, body, bodylength, info);
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
	sem_post(&(UPnPObject->EventLock));
}
//{{{END_EVENTPROCESSING}}}
//{{{SetStateMethods}}}
void UPnPDestroyMicroStack(void *object)
{
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)object;
	//{{{BEGIN_EVENTPROCESSING}}}struct SubscriberInfo  *sinfo,*sinfo2;//{{{END_EVENTPROCESSING}}}
	UPnPSendByeBye(upnp);
	//{{{BEGIN_UPnP/1.1_Specific}}}
	UPnPFreeUnicastReceiveSockets(upnp);
	//{{{END_UPnP/1.1_Specific}}}
	sem_destroy(&(upnp->EventLock));
	//{{{UPnPDestroyMicroStack_FreeEventResources}}}
	if (upnp->AddressListV4 != NULL) free(upnp->AddressListV4);
	if (upnp->AddressListV6 != NULL) free(upnp->AddressListV6);
	if (upnp->NOTIFY_SEND_socks != NULL) free(upnp->NOTIFY_SEND_socks);
	if (upnp->NOTIFY_RECEIVE_socks != NULL) free(upnp->NOTIFY_RECEIVE_socks);
	if (upnp->NOTIFY_SEND_socks6 != NULL) free(upnp->NOTIFY_SEND_socks6);
	if (upnp->NOTIFY_RECEIVE_socks6 != NULL) free(upnp->NOTIFY_RECEIVE_socks6);
	free(upnp->UUID);
	free(upnp->Serial);
	//{{{Device_Default_Model_BEGIN}}}free(upnp->DeviceDescription);
	//{{{Device_Default_Model_END}}}
	//{{{BEGIN_EVENTPROCESSING}}}
	//{{{UPnPDestroyMicroStack_DestructSubscriber}}}
	//{{{END_EVENTPROCESSING}}}
}

int UPnPGetLocalPortNumber(UPnPSessionToken token)
{
	return(ILibWebServer_GetPortNumber(((struct ILibWebServer_Session*)token)->Parent));
}

void UPnPSessionReceiveSink(
struct ILibWebServer_Session *sender,
	int InterruptFlag,
struct packetheader *header,
	char *bodyBuffer,
	int *beginPointer,
	int endPointer,
	int done)
{
	//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
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
	//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	if (header != NULL && done !=0 && InterruptFlag == 0)
	{
		UPnPProcessHTTPPacket(sender, header, bodyBuffer, beginPointer == NULL?0:*beginPointer, endPointer);
		if (beginPointer!=NULL) {*beginPointer = endPointer;}
	}
}
void UPnPSessionSink(struct ILibWebServer_Session *SessionToken, void *user)
{
	SessionToken->OnReceive = &UPnPSessionReceiveSink;
	SessionToken->User = user;
}

void UPnPSetTag(const UPnPMicroStackToken token, void *UserToken)
{
	((struct UPnPDataObject*)token)->User = UserToken;
}

void *UPnPGetTag(const UPnPMicroStackToken token)
{
	return(((struct UPnPDataObject*)token)->User);
}

UPnPMicroStackToken UPnPGetMicroStackTokenFromSessionToken(const UPnPSessionToken token)
{
	return(((struct ILibWebServer_Session*)token)->User);
}

//{{{CreateMicroStackDefinition}}}
{
	struct UPnPDataObject* RetVal;
	//{{{Device_Default_Model_BEGIN}}}char* DDT;//{{{Device_Default_Model_END}}}
	struct timeval tv;
	size_t len;
	if ((RetVal = (struct UPnPDataObject*)malloc(sizeof(struct UPnPDataObject))) == NULL) ILIBCRITICALEXIT(254);

	gettimeofday(&tv,NULL);
	srand((int)tv.tv_sec);
	//{{{ObjectModel_MetaData}}}
	// Complete State Reset
	memset(RetVal, 0, sizeof(struct UPnPDataObject));

	RetVal->ForceExit = 0;
	RetVal->PreSelect = &UPnPMasterPreSelect;
	RetVal->PostSelect = NULL;
	RetVal->Destroy = &UPnPDestroyMicroStack;
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

	//{{{Device_Default_Model_BEGIN}}}
	//{{{DeviceDescriptionMalloc}}}
	//{{{Device_Default_Model_END}}}
	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
	RetVal->HTTPServer = ILibWebServer_Create(Chain, UPNP_HTTP_MAXSOCKETS, PortNum, &UPnPSessionSink, RetVal);
	RetVal->WebSocketPortNumber = (int)ILibWebServer_GetPortNumber(RetVal->HTTPServer);

	//{{{BEGIN_UPnP/1.1_Specific}}}	
	RetVal->ConfigID = RetVal->WebSocketPortNumber;	
	//{{{END_UPnP/1.1_Specific}}}

	ILibAddToChain(Chain, RetVal);
	UPnPInit(RetVal ,Chain, NotifyCycleSeconds, PortNum);

	RetVal->EventClient = ILibCreateWebClient(5, Chain);
	RetVal->UpdateFlag = 0;

	//{{{Device_Default_Model_BEGIN}}}
	DDT = ILibDecompressString((unsigned char*)UPnPDeviceDescriptionTemplate, UPnPDeviceDescriptionTemplateLength, UPnPDeviceDescriptionTemplateLengthUX);
	//{{{CreateMicroStack_sprintf}}}
	free(DDT);
	//{{{Device_Default_Model_END}}}

	sem_init(&(RetVal->EventLock), 0, 1);
	//{{{Device_Object_Model_BEGIN}}}UPnPGetConfiguration()->MicrostackToken=RetVal;//{{{Device_Object_Model_END}}}
	return(RetVal);
}
//{{{ComplexTypeCode}}}

//{{{Device_Object_Model_BEGIN}}}
void UPnPStreamDescriptionDocument(struct ILibWebServer_Session *session)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: WINDOWS, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#elif defined(__SYMBIAN32__)
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: SYMBIAN, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#else
	char *responseHeader = "\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: POSIX, UPnP/!UPNPVERSION!, MicroStack/!MICROSTACKVERSION!";
#endif

	char *tempString;
	int tempStringLength;
	char *xString,*xString2;

	// Device
	ILibWebServer_StreamHeader_Raw(session, 200, "OK", responseHeader, 1);

	//{{{DEVICE_BEGIN}}}
	xString2 = ILibDecompressString((unsigned char*){{{DEVICE}}}Reserved,{{{DEVICE}}}ReservedXL,{{{DEVICE}}}ReservedUXL);
	xString = ILibString_Replace(xString2, (int)strlen(xString2), "http://255.255.255.255:255/", 27, "%s", 2);
	free(xString2);
	tempStringLength = (int)(strlen(xString) + strlen({{{DEVICE}}}Manufacturer) + strlen({{{DEVICE}}}ManufacturerURL) + strlen({{{DEVICE}}}ModelDescription) + strlen({{{DEVICE}}}ModelName)+strlen({{{DEVICE}}}ModelNumber)+strlen({{{DEVICE}}}ModelURL) + strlen({{{DEVICE}}}ProductCode) + strlen({{{DEVICE}}}FriendlyName) + strlen({{{DEVICE2}}}UDN) + strlen({{{DEVICE2}}}Serial));
	if ((tempString = (char*)malloc(tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
	tempStringLength = snprintf(tempString, tempStringLength, xString, 
	{{{DEVICE}}}FriendlyName,
	{{{DEVICE}}}Manufacturer,
	{{{DEVICE}}}ManufacturerURL,
	{{{DEVICE}}}ModelDescription,
	{{{DEVICE}}}ModelName,
	{{{DEVICE}}}ModelNumber,
	{{{DEVICE}}}ModelURL,
	{{{DEVICE2}}}Serial,
	{{{DEVICE2}}}UDN);
	free(xString);
	ILibWebServer_StreamBody(session ,tempString, tempStringLength - {{{DEVICE_SUBTRACTION}}}, ILibAsyncSocket_MemoryOwnership_CHAIN, 0);

	// Embedded Services
	ILibWebServer_StreamBody(session, "<serviceList>", 13, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	//{{{SERVICE_BEGIN}}}
	if ({{{SERVICE}}}!=NULL)
	{
		xString = ILibDecompressString((unsigned char*){{{SERVICE}}}->Reserved, {{{SERVICE}}}->ReservedXL, {{{SERVICE}}}->ReservedUXL);
		ILibWebServer_StreamBody(session, xString, {{{SERVICE}}}->ReservedUXL, ILibAsyncSocket_MemoryOwnership_CHAIN, 0);
	}
	//{{{SERVICE_END}}}
	ILibWebServer_StreamBody(session,"</serviceList>", 14, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	//{{{BEGIN_HASEMBEDDEDDEVICES}}}ILibWebServer_StreamBody(session,"<deviceList>",12,ILibAsyncSocket_MemoryOwnership_STATIC,0);
	//{{{EMBEDDED_DEVICES}}}
	ILibWebServer_StreamBody(session,"</deviceList>", 13, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	//{{{END_HASEMBEDDEDDEVICES}}}
	ILibWebServer_StreamBody(session,"</device>", 9, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	//{{{DEVICE_END}}}

	ILibWebServer_StreamBody(session,"</root>", 7, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
}
//{{{GetConfiguration}}}
//{{{Device_Object_Model_END}}}

