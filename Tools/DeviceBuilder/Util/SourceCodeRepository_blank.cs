using System;
using System.IO;
using System.Windows.Forms;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for SourceCodeRepository.
	/// </summary>
	public class SourceCodeRepository
	{
		#region EULA
		// -=S3P4R470R=- {EULA}
		// -=S3P4R470R=- {EULA}
		#endregion
		#region UPnPMicroStack.c
		// -=S3P4R470R=- {UPnPMicroStack_C}
private static string UPnPMicroStack_C = "/*\r\n* INTEL CONFIDENTIAL\r\n* Copyright (c) 2002, 2003 Intel Corporation.  All rights reserved.\r\n* \r\n* "
+"The source code contained or described herein and all documents\r\n* related to the source code (\"Mate"
+"rial\") are owned by Intel\r\n* Corporation or its suppliers or licensors.  Title to the\r\n* Material re"
+"mains with Intel Corporation or its suppliers and\r\n* licensors.  The Material contains trade secrets"
+" and proprietary\r\n* and confidential information of Intel or its suppliers and\r\n* licensors. The Mat"
+"erial is protected by worldwide copyright and\r\n* trade secret laws and treaty provisions.  No part o"
+"f the Material\r\n* may be used, copied, reproduced, modified, published, uploaded,\r\n* posted, transmi"
+"tted, distributed, or disclosed in any way without\r\n* Intel's prior express written permission.\r\n\r\n*"
+" No license under any patent, copyright, trade secret or other\r\n* intellectual property right is gra"
+"nted to or conferred upon you\r\n* by disclosure or delivery of the Materials, either expressly, by\r\n*"
+" implication, inducement, estoppel or otherwise. Any license\r\n* under such intellectual property rig"
+"hts must be express and\r\n* approved by Intel in writing.\r\n* \r\n* $Workfile: UPnPMicroStack.c\r\n* $Revi"
+"sion: #1.0.1608.24253\r\n* $Author: byroe $Date:     Thursday, "
+"May 27, 2004\r\n*\r\n*/\r\n\r\n\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n#	ifndef MICROSTACK_NO_STDAFX\r\n#"
+"		include \"stdafx.h\"\r\n#	endif\r\n#	define _CRTDBG_MAP_ALLOC\r\n#	include <winerror.h>\r\nchar* PLATFORM = "
+"\"WINDOWS\";\r\n#else\r\nchar* PLATFORM = \"POSIX\";\r\n#endif\r\n\r\n\r\n#include <stdio.h>\r\n#include <stdlib.h>\r\n#"
+"include <string.h>\r\n#include <math.h>\r\n\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n#	include <stdde"
+"f.h>\r\n#	ifdef WINSOCK2\r\n	#	include <winsock2.h>\r\n	#	include <ws2tcpip.h>\r\n#	elif WINSOCK1\r\n	#	includ"
+"e <winsock.h>\r\n	#	include <wininet.h>\r\n#	endif\r\n#	include <windows.h>\r\n#	include <winioctl.h>\r\n#	inc"
+"lude <winbase.h>\r\n#	include <crtdbg.h>\r\n#	define sem_t HANDLE\r\n#	define sem_init(x,y,z) *x=CreateSem"
+"aphore(NULL,z,FD_SETSIZE,NULL)\r\n#	define sem_destroy(x) (CloseHandle(*x)==0?1:0)\r\n#	define sem_wait("
+"x) WaitForSingleObject(*x,INFINITE)\r\n#	define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJEC"
+"T_0)?0:1)\r\n#	define sem_post(x) ReleaseSemaphore(*x,1,NULL)\r\n#	define strncasecmp(x,y,z) _strnicmp(x"
+",y,z)\r\n#else\r\n#	include <sys/types.h>\r\n#	include <sys/socket.h>\r\n#	include <netinet/in.h>\r\n#	include"
+" <arpa/inet.h>\r\n#	include <sys/time.h>\r\n#	include <netdb.h>\r\n#	include <sys/ioctl.h>\r\n#	include <net"
+"/if.h>\r\n#	include <sys/utsname.h>\r\n#	include <sys/socket.h>\r\n#	include <netinet/in.h>\r\n#	include <un"
+"istd.h>\r\n#	include <fcntl.h>\r\n#	include <errno.h>\r\n#	include <semaphore.h>\r\n#endif\r\n\r\n#include \"ILib"
+"Parsers.h\"\r\n#include \"UPnPMicroStack.h\"\r\n#include \"ILibWebServer.h\"\r\n#include \"ILibWebClient.h\"\r\n#in"
+"clude \"ILibAsyncSocket.h\"\r\n\r\n#define UPNP_HTTP_MAXSOCKETS 5\r\n#define UPNP_PORT 1900\r\n#define UPNP_GR"
+"OUP \"239.255.255.250\"\r\n#define UPnPMIN(a,b) (((a)<(b))?(a):(b))\r\n\r\n#define LVL3DEBUG(x)\r\n\r\n//{{{Func"
+"tionPointers}}}\r\n\r\n//{{{CompressedDescriptionDocs}}}\r\n\r\nstruct UPnPDataObject;\r\n\r\nstruct SubscriberI"
+"nfo\r\n{\r\n	char* SID;\r\n	int SIDLength;\r\n	int SEQ;\r\n\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	int NotLegacy;"
+"\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n	int Address;\r\n	unsigned short Port;\r\n	char* Path;\r\n	int PathLengt"
+"h;\r\n	int RefCount;\r\n	int Disposing;\r\n	\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	unsigned int Ren"
+"ewByTime;\r\n#else\r\n	struct timeval RenewByTime;\r\n#endif\r\n\r\n	struct SubscriberInfo *Next;\r\n	struct Sub"
+"scriberInfo *Previous;\r\n};\r\nstruct UPnPDataObject\r\n{\r\n	void (*PreSelect)(void* object,fd_set *readse"
+"t, fd_set *writeset, fd_set *errorset, int* blocktime);\r\n	void (*PostSelect)(void* object,int slct, "
+"fd_set *readset, fd_set *writeset, fd_set *errorset);\r\n	void (*Destroy)(void* object);\r\n	\r\n	void *Ev"
+"entClient;\r\n	void *Chain;\r\n	int UpdateFlag;\r\n	\r\n	/* Network Poll */\r\n	unsigned int NetworkPollTime;\r"
+"\n\r\n	int ForceExit;\r\n	char *UUID;\r\n	char *UDN;\r\n	char *Serial;\r\n	\r\n	void *WebServerTimer;\r\n	void *HTT"
+"PServer;\r\n	\r\n	char *DeviceDescription;\r\n	int DeviceDescriptionLength;\r\n	int InitialNotify;\r\n\r\n	//{{{"
+"StateVariables}}}\r\n\r\n	struct sockaddr_in addr;\r\n	int addrlen;\r\n\r\n	struct ip_mreq mreq;\r\n	char messag"
+"e[4096];\r\n	int *AddressList;\r\n	int AddressListLength;\r\n	\r\n	int _NumEmbeddedDevices;\r\n	int WebSocketP"
+"ortNumber;\r\n	\r\n//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	int ConfigID;\r\n	unsigned short UnicastReceiveSocket"
+"PortNumber;\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	SOCKET *UnicastReceiveSockets;\r\n#else\r\n	int"
+" *UnicastReceiveSockets;\r\n#endif\r\n//{{{END_UPnP/1.1_Specific}}}\r\n\r\n#if defined(WIN32) || defined(_WI"
+"N32_WCE)\r\n	SOCKET *NOTIFY_SEND_socks;\r\n	SOCKET NOTIFY_RECEIVE_sock;\r\n	SOCKET MSEARCH_sock;	\r\n	unsign"
+"ed int CurrentTime;\r\n	unsigned int NotifyTime;\r\n#else\r\n	int *NOTIFY_SEND_socks;\r\n	int NOTIFY_RECEIVE"
+"_sock;\r\n	int MSEARCH_sock;\r\n	struct timeval CurrentTime;\r\n	struct timeval NotifyTime;\r\n#endif\r\n\r\n	in"
+"t SID;\r\n	int NotifyCycleTime;\r\n\r\n	\r\n	sem_t EventLock;\r\n	//{{{HeadSubscriberPointers}}}\r\n};\r\n\r\nstruct"
+" MSEARCH_state\r\n{\r\n	char *ST;\r\n	int STLength;\r\n	void *upnp;\r\n	struct sockaddr_in dest_addr;\r\n};\r\nstr"
+"uct UPnPFragmentNotifyStruct\r\n{\r\n	struct UPnPDataObject *upnp;\r\n	int packetNumber;\r\n};\r\n\r\n/* Pre-dec"
+"larations */\r\nvoid UPnPFragmentedSendNotify(void *data);\r\nvoid UPnPSendNotify(const struct UPnPDataO"
+"bject *upnp);\r\nvoid UPnPSendByeBye(const struct UPnPDataObject *upnp);\r\nvoid UPnPMainInvokeSwitch();"
+"\r\nvoid UPnPSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const i"
+"nt Terminate);\r\nvoid UPnPSendData(const void* UPnPToken, const char* Data, const int DataLength, con"
+"st int Terminate);\r\nint UPnPPeriodicNotify(struct UPnPDataObject *upnp);\r\nvoid UPnPSendEvent_Body(vo"
+"id *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);\r\n\r\nvoid* UPnPGetWebServerTo"
+"ken(const void *MicroStackToken)\r\n{\r\n	return(((struct UPnPDataObject*)MicroStackToken)->HTTPServer);"
+"\r\n}\r\n//{{{BEGIN_UPnP/1.0_Specific}}}\r\n#define UPnPBuildSsdpResponsePacket(outpacket,outlength,ipaddr"
+",port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\\\r\n{\\\r\n	*outlength = sprintf(outpacket,\"HTTP"
+"/1.1 200 OK\\r\\nLOCATION: http://%d.%d.%d.%d:%d/\\r\\nEXT:\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel Mic"
+"roStack/!MICROSTACKVERSION!\\r\\nUSN: uuid:%s%s\\r\\nCACHE-CONTROL: max-age=%d\\r\\nST: %s%s\\r\\n\\r\\n\" ,(ip"
+"addr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,PLATFORM,USN,USNex,Notify"
+"Time,ST,NTex);\\\r\n}\r\n#define UPnPBuildSsdpNotifyPacket(outpacket,outlength,ipaddr,port,EmbeddedDevice"
+"Number,USN,USNex,NT,NTex,NotifyTime)\\\r\n{\\\r\n	*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nLOC"
+"ATION: http://%d.%d.%d.%d:%d/\\r\\nHOST: 239.255.255.250:1900\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel"
+" MicroStack/!MICROSTACKVERSION!\\r\\nNTS: ssdp:alive\\r\\nUSN: uuid:%s%s\\r\\nCACHE-CONTROL: max-age=%d\\r\\"
+"nNT: %s%s\\r\\n\\r\\n\",(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,PLA"
+"TFORM,USN,USNex,NotifyTime,NT,NTex);\\\r\n}\r\n//{{{END_UPnP/1.0_Specific}}}\r\n//{{{BEGIN_UPnP/1.1_Specifi"
+"c}}}\r\n#define UPnPBuildSsdpResponsePacket(outpacket,outlength,ConfigID,PortNum,bootID,maxVer,ipaddr,"
+"port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\\\r\n{\\\r\n	*outlength = sprintf(outpacket,\"HTTP/"
+"1.1 200 OK\\r\\nCONFIGID.UPNP.ORG: %d\\r\\nSEARCHPORT.UPNP.ORG: %u\\r\\nBOOTID.UPNP.ORG: %d\\r\\nMAXVERSION."
+"UPNP.ORG: %d\\r\\nLOCATION: http://%d.%d.%d.%d:%d/\\r\\nEXT:\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel Mi"
+"croStack/!MICROSTACKVERSION!\\r\\nUSN: uuid:%s%s\\r\\nCACHE-CONTROL: max-age=%d\\r\\nST: %s%s\\r\\n\\r\\n\" ,Co"
+"nfigID,PortNum,bootID,maxVer,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF"
+"),port,PLATFORM,USN,USNex,NotifyTime,ST,NTex);\\\r\n}\r\n#define UPnPBuildSsdpNotifyPacket(outpacket,outl"
+"ength,ConfigID,PortNum,bootID,maxVer,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\\"
+"\r\n{\\\r\n	*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nCONFIGID.UPNP.ORG: %d\\r\\nSEARCHPORT.UPNP"
+".ORG: %u\\r\\nBOOTID.UPNP.ORG: %d\\r\\nMAXVERSION.UPNP.ORG: %d\\r\\nLOCATION: http://%d.%d.%d.%d:%d/\\r\\nHO"
+"ST: 239.255.255.250:1900\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel MicroStack/!MICROSTACKVERSION!\\r\\n"
+"NTS: ssdp:alive\\r\\nUSN: uuid:%s%s\\r\\nCACHE-CONTROL: max-age=%d\\r\\nNT: %s%s\\r\\n\\r\\n\",ConfigID,PortNum"
+",bootID,maxVer,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,PLATFOR"
+"M,USN,USNex,NotifyTime,NT,NTex);\\\r\n}\r\nvoid UPnPFreeUnicastReceiveSockets(struct UPnPDataObject *obj)"
+"\r\n{\r\n	int i;\r\n	if(obj->UnicastReceiveSockets!=NULL)\r\n	{\r\n		for(i=0;i<obj->AddressListLength;++i)\r\n		"
+"{\r\n			if(obj->UnicastReceiveSockets[i]!=~0)\r\n			{\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n				cl"
+"osesocket(obj->UnicastReceiveSockets[i]);\r\n#else\r\n				close(obj->UnicastReceiveSockets[i]);\r\n#endif\r"
+"\n			}\r\n		}\r\n		free(obj->UnicastReceiveSockets);\r\n		obj->UnicastReceiveSockets = NULL;\r\n	}\r\n}\r\nvoid U"
+"PnPBindUnicastReceiveSockets(struct UPnPDataObject *obj)\r\n{\r\n	int OK=0,i;\r\n	struct sockaddr_in addr;"
+"	\r\n	\r\n	memset((char *)&(addr), 0, sizeof(addr));\r\n	addr.sin_family = AF_INET;\r\n	\r\n	UPnPFreeUnicastRe"
+"ceiveSockets(obj);\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	obj->UnicastReceiveSockets = (SOCKET"
+"*)malloc(obj->AddressListLength*sizeof(SOCKET));\r\n#else\r\n	obj->UnicastReceiveSockets = (int*)malloc("
+"obj->AddressListLength*sizeof(int));\r\n#endif\r\n	for(i=0;i<obj->AddressListLength;++i)\r\n	{\r\n		obj->Uni"
+"castReceiveSockets[i] = ~0;\r\n	}\r\n	\r\n	do\r\n	{\r\n		obj->UnicastReceiveSocketPortNumber = (unsigned short"
+")(1901 + ((unsigned short)rand() % 98));\r\n		for(i=0;i<obj->AddressListLength;++i)\r\n		{\r\n			addr.sin_"
+"addr.s_addr = obj->AddressList[i];\r\n			addr.sin_port = htons(obj->UnicastReceiveSocketPortNumber);\r\n"
+"			obj->UnicastReceiveSockets[i] = socket(AF_INET, SOCK_DGRAM, 0);\r\n			if(bind(obj->UnicastReceiveSo"
+"ckets[i], (struct sockaddr *) &(addr), sizeof(addr))<0)\r\n			{\r\n				UPnPFreeUnicastReceiveSockets(obj"
+");\r\n				OK=0;\r\n				break;\r\n			}\r\n			else\r\n			{\r\n				OK=1;\r\n			}\r\n		}\r\n	}while(!OK);\r\n}\r\n//{{{END_UPn"
+"P/1.1_Specific}}}\r\n\r\n//{{{BEGIN_FragmentedResponseSystem}}}\r\nvoid UPnPAsyncResponse_START(const void"
+"* UPnPToken, const char* actionName, const char* serviceUrnWithVersion)\r\n{\r\n	char* RESPONSE_HEADER ="
+" \"\\r\\nEXT:\\r\\nCONTENT-TYPE: text/xml\\r\\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1609\";\r\n	cha"
+"r* RESPONSE_BODY = \"<?xml version=\\\"1.0\\\" encoding=\\\"utf-8\\\"?>\\r\\n<s:Envelope s:encodingStyle=\\\"http"
+"://schemas.xmlsoap.org/soap/encoding/\\\" xmlns:s=\\\"http://schemas.xmlsoap.org/soap/envelope/\\\">\\r\\n<s"
+":Body>\\r\\n<u:%sResponse xmlns:u=\\\"%s\\\">\";\r\n	struct ILibWebServer_Session *session = (struct ILibWebS"
+"erver_Session*)UPnPToken;\r\n	\r\n	int headSize = (int)strlen(RESPONSE_BODY) + (int)strlen(actionName) +"
+" (int)strlen(serviceUrnWithVersion) + 1;\r\n	char* head = (char*) malloc (headSize);\r\n	\r\n	int headLeng"
+"th = sprintf(head, RESPONSE_BODY, actionName, serviceUrnWithVersion);\r\n	ILibWebServer_StreamHeader_R"
+"aw(session,200,\"OK\",RESPONSE_HEADER,1);\r\n	ILibWebServer_StreamBody(session,head,headLength,0,0);\r\n}\r"
+"\nvoid UPnPAsyncResponse_DONE(const void* UPnPToken, const char* actionName)\r\n{\r\n	char* RESPONSE_FOOT"
+"ER = \"</u:%sResponse>\\r\\n   </s:Body>\\r\\n</s:Envelope>\";\r\n	\r\n	int footSize = (int)strlen(RESPONSE_FO"
+"OTER) + (int)strlen(actionName);\r\n	char* footer = (char*) malloc(footSize);\r\n	struct ILibWebServer_S"
+"ession *session = (struct ILibWebServer_Session*)UPnPToken;\r\n	\r\n	int footLength = sprintf(footer, RE"
+"SPONSE_FOOTER, actionName);\r\n	ILibWebServer_StreamBody(session,footer,footLength,0,1);\r\n}\r\nvoid UPnP"
+"AsyncResponse_OUT(const void* UPnPToken, const char* outArgName, const char* bytes, const int byteLe"
+"ngth, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg"
+")\r\n{\r\n	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;\r\n	\r\n	if (st"
+"artArg != 0)\r\n	{\r\n		ILibWebServer_StreamBody(session,\"<\",1,1,0);\r\n		ILibWebServer_StreamBody(session"
+",(char*)outArgName,(int)strlen(outArgName),1,0);\r\n		ILibWebServer_StreamBody(session,\">\",1,1,0);\r\n	}"
+"\r\n	\r\n	if(byteLength>0)\r\n	{\r\n		ILibWebServer_StreamBody(session,(char*)bytes,byteLength,bytesMemoryOw"
+"nership,0);\r\n	}\r\n	\r\n	if (endArg != 0)\r\n	{\r\n		ILibWebServer_StreamBody(session,\"</\",2,1,0);\r\n		ILibWe"
+"bServer_StreamBody(session,(char*)outArgName,(int)strlen(outArgName),1,0);\r\n		ILibWebServer_StreamBo"
+"dy(session,\">\\r\\n\",3,1,0);\r\n	}\r\n}\r\n//{{{END_FragmentedResponseSystem}}}\r\n\r\nvoid UPnPIPAddressListCha"
+"nged(void *MicroStackToken)\r\n{\r\n	((struct UPnPDataObject*)MicroStackToken)->UpdateFlag = 1;\r\n	ILibFo"
+"rceUnBlockChain(((struct UPnPDataObject*)MicroStackToken)->Chain);\r\n}\r\nvoid UPnPInit(struct UPnPData"
+"Object *state,const int NotifyCycleSeconds,const unsigned short PortNumber)\r\n{\r\n	int ra = 1;\r\n	int i"
+";\r\n	struct sockaddr_in addr;\r\n	struct ip_mreq mreq;\r\n	unsigned char TTL = 4;\r\n	\r\n	/* Complete State "
+"Reset */\r\n	memset(state,0,sizeof(struct UPnPDataObject));\r\n	\r\n	/* Setup Notification Timer */\r\n	stat"
+"e->NotifyCycleTime = NotifyCycleSeconds;\r\n\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	state->Curre"
+"ntTime = GetTickCount() / 1000;\r\n	state->NotifyTime = state->CurrentTime  + (state->NotifyCycleTime/"
+"2);\r\n#else\r\n	gettimeofday(&(state->CurrentTime),NULL);\r\n	(state->NotifyTime).tv_sec = (state->Curren"
+"tTime).tv_sec  + (state->NotifyCycleTime/2);\r\n#endif\r\n\r\n	memset((char *)&(state->addr), 0, sizeof(st"
+"ate->addr));\r\n	state->addr.sin_family = AF_INET;\r\n	state->addr.sin_addr.s_addr = htonl(INADDR_ANY);\r"
+"\n	state->addr.sin_port = (unsigned short)htons(UPNP_PORT);\r\n	state->addrlen = sizeof(state->addr);\r\n"
+"	/* Set up socket */\r\n	state->AddressListLength = ILibGetLocalIPAddressList(&(state->AddressList));\r"
+"\n\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	state->NOTIFY_SEND_socks = (SOCKET*)malloc(sizeof(int"
+")*(state->AddressListLength));\r\n#else\r\n	state->NOTIFY_SEND_socks = (int*)malloc(sizeof(int)*(state->"
+"AddressListLength));\r\n#endif\r\n\r\n	state->NOTIFY_RECEIVE_sock = socket(AF_INET, SOCK_DGRAM, 0);\r\n	mems"
+"et((char *)&(addr), 0, sizeof(addr));\r\n	addr.sin_family = AF_INET;\r\n	addr.sin_addr.s_addr = htonl(IN"
+"ADDR_ANY);\r\n	addr.sin_port = (unsigned short)htons(UPNP_PORT);\r\n	if (setsockopt(state->NOTIFY_RECEIV"
+"E_sock, SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) < 0)\r\n	{\r\n		printf(\"Setting SockOpt SO_REUS"
+"EADDR failed\\r\\n\");\r\n		exit(1);\r\n	}\r\n	if (bind(state->NOTIFY_RECEIVE_sock, (struct sockaddr *) &(add"
+"r), sizeof(addr)) < 0)\r\n	{\r\n		printf(\"Could not bind to UPnP Listen Port\\r\\n\");\r\n		exit(1);\r\n	}\r\n	fo"
+"r(i=0;i<state->AddressListLength;++i)\r\n	{\r\n		state->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRA"
+"M, 0);\r\n		memset((char *)&(addr), 0, sizeof(addr));\r\n		addr.sin_family = AF_INET;\r\n		addr.sin_addr.s"
+"_addr = state->AddressList[i];\r\n		addr.sin_port = (unsigned short)htons(UPNP_PORT);\r\n		if (setsockop"
+"t(state->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)\r\n		{\r\n			if (s"
+"etsockopt(state->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)\r\n"
+"			{\r\n				/* Ignore this case */\r\n			}\r\n			if (bind(state->NOTIFY_SEND_socks[i], (struct sockaddr *)"
+" &(addr), sizeof(addr)) == 0)\r\n			{\r\n				mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);\r\n				mre"
+"q.imr_interface.s_addr = state->AddressList[i];\r\n				if (setsockopt(state->NOTIFY_RECEIVE_sock, IPPR"
+"OTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)\r\n				{\r\n					/* Does not matter */\r\n				}"
+"\r\n			}\r\n		}\r\n	}\r\n//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	UPnPBindUnicastReceiveSockets(state);\r\n//{{{END_U"
+"PnP/1.1_Specific}}}\r\n}\r\nvoid UPnPPostMX_Destroy(void *object)\r\n{\r\n	struct MSEARCH_state *mss = (stru"
+"ct MSEARCH_state*)object;\r\n	free(mss->ST);\r\n	free(mss);\r\n}\r\nvoid UPnPPostMX_MSEARCH(void *object)\r\n{"
+"\r\n	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;\r\n	\r\n	char *b = (char*)malloc(sizeof(ch"
+"ar)*5000);\r\n	int packetlength;\r\n	struct sockaddr_in response_addr;\r\n	int response_addrlen;\r\n#if defi"
+"ned(WIN32) || defined(_WIN32_WCE)\r\n	SOCKET *response_socket;\r\n#else\r\n	int *response_socket;\r\n#endif\r"
+"\n\r\n	int cnt;\r\n	int i;\r\n	struct sockaddr_in dest_addr = mss->dest_addr;\r\n	char *ST = mss->ST;\r\n	int S"
+"TLength = mss->STLength;\r\n	struct UPnPDataObject *upnp = (struct UPnPDataObject*)mss->upnp;\r\n	\r\n#if "
+"defined(WIN32) || defined(_WIN32_WCE)\r\n	response_socket = (SOCKET*)malloc(upnp->AddressListLength*si"
+"zeof(int));\r\n#else\r\n	response_socket = (int*)malloc(upnp->AddressListLength*sizeof(int));\r\n#endif\r\n\r"
+"\n	for(i=0;i<upnp->AddressListLength;++i)\r\n	{\r\n		response_socket[i] = socket(AF_INET, SOCK_DGRAM, 0);"
+"\r\n		if (response_socket[i]< 0)\r\n		{\r\n			printf(\"response socket\");\r\n			exit(1);\r\n		}\r\n		memset((char"
+" *)&(response_addr), 0, sizeof(response_addr));\r\n		response_addr.sin_family = AF_INET;\r\n		response_a"
+"ddr.sin_addr.s_addr = upnp->AddressList[i];\r\n		response_addr.sin_port = (unsigned short)htons(0);\r\n	"
+"	response_addrlen = sizeof(response_addr);	\r\n		if (bind(response_socket[i], (struct sockaddr *) &(re"
+"sponse_addr), sizeof(response_addr)) < 0)\r\n		{\r\n			/* Ignore if this happens */\r\n		}\r\n	}\r\n	if(STLeng"
+"th==15 && memcmp(ST,\"upnp:rootdevice\",15)==0)\r\n	{\r\n		for(i=0;i<upnp->AddressListLength;++i)\r\n		{\r\n		"
+"	//{{{BEGIN_UPnP/1.0_Specific}}}\r\n			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i"
+"],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,\"::upnp:rootdevice\",\"upnp:rootdevice\",\"\",upn"
+"p->NotifyCycleTime);\r\n			//{{{END_UPnP/1.0_Specific}}}\r\n			//{{{BEGIN_UPnP/1.1_Specific}}}\r\n			UPnPB"
+"uildSsdpResponsePacket(b,&packetlength,upnp->ConfigID,upnp->UnicastReceiveSocketPortNumber,upnp->Ini"
+"tialNotify,1,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,\"::upnp:root"
+"device\",\"upnp:rootdevice\",\"\",upnp->NotifyCycleTime);\r\n			//{{{END_UPnP/1.1_Specific}}}\r\n			cnt = sen"
+"dto(response_socket[i], b, packetlength, 0,(struct sockaddr *) &dest_addr, sizeof(dest_addr));\r\n		}\r"
+"\n	}\r\n	else if(STLength==8 && memcmp(ST,\"ssdp:all\",8)==0)\r\n	{\r\n//{{{SSDP:ALL}}}\r\n	}\r\n//{{{SSDP:OTHER}"
+"}}\r\n\r\n	for(i=0;i<upnp->AddressListLength;++i)\r\n	{\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n		clos"
+"esocket(response_socket[i]);\r\n#else\r\n		close(response_socket[i]);\r\n#endif\r\n	}\r\n	free(response_socket"
+");\r\n	free(mss->ST);\r\n	free(mss);\r\n	free(b);\r\n}\r\nvoid UPnPProcessMSEARCH(struct UPnPDataObject *upnp,"
+" struct packetheader *packet)\r\n{\r\n	char* ST = NULL;\r\n	int STLength = 0;\r\n	struct packetheader_field_"
+"node *node;\r\n	int MANOK = 0;\r\n	unsigned long MXVal;\r\n	int MXOK = 0;\r\n	int MX;\r\n	struct MSEARCH_state"
+" *mss = NULL;\r\n	\r\n	if(memcmp(packet->DirectiveObj,\"*\",1)==0)\r\n	{\r\n		if(memcmp(packet->Version,\"1.1\","
+"3)==0)\r\n		{\r\n			node = packet->FirstField;\r\n			while(node!=NULL)\r\n			{\r\n				if(strncasecmp(node->Fie"
+"ld,\"ST\",2)==0)\r\n				{\r\n					ST = (char*)malloc(1+node->FieldDataLength);\r\n					memcpy(ST,node->Field"
+"Data,node->FieldDataLength);\r\n					ST[node->FieldDataLength] = 0;\r\n					STLength = node->FieldDataLe"
+"ngth;\r\n				}\r\n				else if(strncasecmp(node->Field,\"MAN\",3)==0 && memcmp(node->FieldData,\"\\\"ssdp:disc"
+"over\\\"\",15)==0)\r\n				{\r\n					MANOK = 1;\r\n				}\r\n				else if(strncasecmp(node->Field,\"MX\",2)==0 && IL"
+"ibGetULong(node->FieldData,node->FieldDataLength,&MXVal)==0)\r\n				{\r\n					MXOK = 1;\r\n					MXVal = MX"
+"Val>10?10:MXVal;\r\n				}\r\n				node = node->NextField;\r\n			}\r\n			if(MANOK!=0 && MXOK!=0)\r\n			{\r\n				MX"
+" = (int)(0 + ((unsigned short)rand() % MXVal));\r\n				mss = (struct MSEARCH_state*)malloc(sizeof(stru"
+"ct MSEARCH_state));\r\n				mss->ST = ST;\r\n				mss->STLength = STLength;\r\n				mss->upnp = upnp;\r\n				me"
+"mset((char *)&(mss->dest_addr), 0, sizeof(mss->dest_addr));\r\n				mss->dest_addr.sin_family = AF_INET"
+";\r\n				mss->dest_addr.sin_addr = packet->Source->sin_addr;\r\n				mss->dest_addr.sin_port = packet->So"
+"urce->sin_port;\r\n				\r\n				ILibLifeTime_Add(upnp->WebServerTimer,mss,MX,&UPnPPostMX_MSEARCH,&UPnPPos"
+"tMX_Destroy);\r\n			}\r\n			else\r\n			{\r\n				free(ST);\r\n			}\r\n		}\r\n	}\r\n}\r\n//{{{DispatchMethods}}}\r\nint UP"
+"nPProcessPOST(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, "
+"int offset, int bodyBufferLength)\r\n\r\n{\r\n	struct packetheader_field_node *f = header->FirstField;\r\n	c"
+"har* HOST;\r\n	char* SOAPACTION = NULL;\r\n	int SOAPACTIONLength = 0;\r\n	struct parser_result *r,*r2;\r\n	s"
+"truct parser_result_field *prf;\r\n	\r\n	int RetVal = 0;\r\n	\r\n	while(f!=NULL)\r\n	{\r\n		if(f->FieldLength==4"
+" && strncasecmp(f->Field,\"HOST\",4)==0)\r\n		{\r\n			HOST = f->FieldData;\r\n		}\r\n		else if(f->FieldLength="
+"=10 && strncasecmp(f->Field,\"SOAPACTION\",10)==0)\r\n		{\r\n			r = ILibParseString(f->FieldData,0,f->Fiel"
+"dDataLength,\"#\",1);\r\n			SOAPACTION = r->LastResult->data;\r\n			SOAPACTIONLength = r->LastResult->data"
+"length-1;\r\n			ILibDestructParserResults(r);\r\n		}\r\n		else if(f->FieldLength==10 && strncasecmp(f->Fie"
+"ld,\"USER-AGENT\",10)==0)\r\n		{\r\n			// Check UPnP version of the Control Point which invoked us\r\n			r ="
+" ILibParseString(f->FieldData,0,f->FieldDataLength,\" \",1);\r\n			prf = r->FirstResult;\r\n			while(prf!="
+"NULL)\r\n			{\r\n				if(prf->datalength>5 && memcmp(prf->data,\"UPnP/\",5)==0)\r\n				{\r\n					r2 = ILibParse"
+"String(prf->data+5,0,prf->datalength-5,\".\",1);\r\n					r2->FirstResult->data[r2->FirstResult->dataleng"
+"th]=0;\r\n					r2->LastResult->data[r2->LastResult->datalength]=0;\r\n					if(atoi(r2->FirstResult->data"
+")==1 && atoi(r2->LastResult->data)>0)\r\n					{\r\n						session->Reserved9=1;\r\n					}\r\n					ILibDestruc"
+"tParserResults(r2);\r\n				}\r\n				prf = prf->NextResult;\r\n			}\r\n			ILibDestructParserResults(r);\r\n		}\r"
+"\n		f = f->NextField;\r\n	}\r\n\r\n//{{{DispatchControl}}}\r\n\r\n	return(RetVal);\r\n}\r\nstruct SubscriberInfo* U"
+"PnPRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers,char* SID, int SIDLength"
+")\r\n{\r\n	struct SubscriberInfo *info = *Head;\r\n	while(info!=NULL)\r\n	{\r\n		if(info->SIDLength==SIDLength"
+" && memcmp(info->SID,SID,SIDLength)==0)\r\n		{\r\n			if ( info->Previous )\r\n			info->Previous->Next = in"
+"fo->Next;\r\n			else\r\n			*Head = info->Next;\r\n			if ( info->Next )\r\n			info->Next->Previous = info->Pr"
+"evious;\r\n			break;\r\n		}\r\n		info = info->Next;\r\n		\r\n	}\r\n	if(info!=NULL)\r\n	{\r\n		info->Previous = NULL;"
+"\r\n		info->Next = NULL;\r\n		--(*TotalSubscribers);\r\n	}\r\n	return(info);\r\n}\r\n\r\n#define UPnPDestructSubsc"
+"riberInfo(info)\\\r\n{\\\r\n	free(info->Path);\\\r\n	free(info->SID);\\\r\n	free(info);\\\r\n}\r\n\r\n#define UPnPDestr"
+"uctEventObject(EvObject)\\\r\n{\\\r\n	free(EvObject->PacketBody);\\\r\n	free(EvObject);\\\r\n}\r\n\r\n#define UPnPDe"
+"structEventDataObject(EvData)\\\r\n{\\\r\n	free(EvData);\\\r\n}\r\nvoid UPnPExpireSubscriberInfo(struct UPnPDat"
+"aObject *d, struct SubscriberInfo *info)\r\n{\r\n	struct SubscriberInfo *t = info;\r\n	while(t->Previous!="
+"NULL)\r\n	{\r\n		t = t->Previous;\r\n	}\r\n	//{{{UPnPExpireSubscriberInfo1}}}\r\n\r\n	if(info->Previous!=NULL)\r\n"
+"	{\r\n		// This is not the Head\r\n		info->Previous->Next = info->Next;\r\n		if(info->Next!=NULL)\r\n		{\r\n		"
+"	info->Next->Previous = info->Previous;\r\n		}\r\n	}\r\n	else\r\n	{\r\n		// This is the Head\r\n		//{{{UPnPExpir"
+"eSubscriberInfo2}}}\r\n	}\r\n	--info->RefCount;\r\n	if(info->RefCount==0)\r\n	{\r\n		UPnPDestructSubscriberInf"
+"o(info);\r\n	}\r\n}\r\n\r\nint UPnPSubscriptionExpired(struct SubscriberInfo *info)\r\n{\r\n	int RetVal = 0;\r\n#i"
+"f defined(WIN32) || defined(_WIN32_WCE)\r\n	if(info->RenewByTime < GetTickCount()/1000) {RetVal = -1;}"
+"\r\n#else\r\n	struct timeval tv;\r\n	gettimeofday(&tv,NULL);\r\n	if((info->RenewByTime).tv_sec < tv.tv_sec) "
+"{RetVal = -1;}\r\n#endif\r\n	return(RetVal);\r\n}\r\n\r\n//{{{InitialEventBody}}}\r\n\r\nvoid UPnPProcessUNSUBSCRI"
+"BE(struct packetheader *header, struct ILibWebServer_Session *session)\r\n{\r\n	char* SID = NULL;\r\n	int "
+"SIDLength = 0;\r\n	struct SubscriberInfo *Info;\r\n	struct packetheader_field_node *f;\r\n	char* packet = "
+"(char*)malloc(sizeof(char)*50);\r\n	int packetlength;\r\n	\r\n	f = header->FirstField;\r\n	while(f!=NULL)\r\n	"
+"{\r\n		if(f->FieldLength==3)\r\n		{\r\n			if(strncasecmp(f->Field,\"SID\",3)==0)\r\n			{\r\n				SID = f->FieldDa"
+"ta;\r\n				SIDLength = f->FieldDataLength;\r\n			}\r\n		}\r\n		f = f->NextField;\r\n	}\r\n	sem_wait(&(((struct U"
+"PnPDataObject*)session->User)->EventLock));\r\n//{{{UnSubscribeDispatcher}}}\r\n	sem_post(&(((struct UPn"
+"PDataObject*)session->User)->EventLock));\r\n}\r\nvoid UPnPTryToSubscribe(char* ServiceName, long Timeou"
+"t, char* URL, int URLLength,struct ILibWebServer_Session *session)\r\n{\r\n	int *TotalSubscribers = NULL"
+";\r\n	struct SubscriberInfo **HeadPtr = NULL;\r\n	struct SubscriberInfo *NewSubscriber,*TempSubscriber;\r"
+"\n	int SIDNumber;\r\n	char *SID;\r\n	char *TempString;\r\n	int TempStringLength;\r\n	char *TempString2;\r\n	lon"
+"g TempLong;\r\n	char *packet;\r\n	int packetlength;\r\n	char* path;\r\n	\r\n	char* escapedURI;\r\n	int escapedUR"
+"ILength;\r\n	\r\n	char *packetbody = NULL;\r\n	int packetbodyLength;\r\n	\r\n	struct parser_result *p;\r\n	struc"
+"t parser_result *p2;\r\n	\r\n	struct UPnPDataObject *dataObject = (struct UPnPDataObject*)session->User;"
+"\r\n\r\n//{{{SubscribeHeadPointerInitializer}}}\r\n\r\n	if(*HeadPtr!=NULL)\r\n	{\r\n		NewSubscriber = *HeadPtr;\r"
+"\n		while(NewSubscriber!=NULL)\r\n		{\r\n			if(UPnPSubscriptionExpired(NewSubscriber)!=0)\r\n			{\r\n				Temp"
+"Subscriber = NewSubscriber->Next;\r\n				NewSubscriber = UPnPRemoveSubscriberInfo(HeadPtr,TotalSubscri"
+"bers,NewSubscriber->SID,NewSubscriber->SIDLength);\r\n				UPnPDestructSubscriberInfo(NewSubscriber);\r\n"
+"				NewSubscriber = TempSubscriber;\r\n			}\r\n			else\r\n			{\r\n				NewSubscriber = NewSubscriber->Next;\r\n"
+"			}\r\n		}\r\n	}\r\n	if(*TotalSubscribers<10)\r\n	{\r\n		NewSubscriber = (struct SubscriberInfo*)malloc(sizeo"
+"f(struct SubscriberInfo));\r\n		memset(NewSubscriber,0,sizeof(struct SubscriberInfo));\r\n		//{{{BEGIN_U"
+"PnP/1.1_Specific}}}\r\n		NewSubscriber->NotLegacy = session->Reserved9;\r\n		//{{{END_UPnP/1.1_Specific}"
+"}}\r\n		SIDNumber = ++dataObject->SID;\r\n		SID = (char*)malloc((int)strlen(dataObject->UDN)+6);\r\n		spri"
+"ntf(SID,\"%s:%d\",dataObject->UDN,SIDNumber);\r\n		p = ILibParseString(URL,0,URLLength,\"://\",3);\r\n		if(p"
+"->NumResults==1)\r\n		{\r\n			ILibWebServer_Send_Raw(session,\"HTTP/1.1 412 Precondition Failed\\r\\nConten"
+"t-Length: 0\\r\\n\\r\\n\",55,1,1);\r\n			ILibDestructParserResults(p);\r\n			return;\r\n		}\r\n		TempString = p->"
+"LastResult->data;\r\n		TempStringLength = p->LastResult->datalength;\r\n		ILibDestructParserResults(p);\r"
+"\n		p = ILibParseString(TempString,0,TempStringLength,\"/\",1);\r\n		p2 = ILibParseString(p->FirstResult-"
+">data,0,p->FirstResult->datalength,\":\",1);\r\n		TempString2 = (char*)malloc(1+sizeof(char)*p2->FirstRe"
+"sult->datalength);\r\n		memcpy(TempString2,p2->FirstResult->data,p2->FirstResult->datalength);\r\n		Temp"
+"String2[p2->FirstResult->datalength] = '\\0';\r\n		NewSubscriber->Address = inet_addr(TempString2);\r\n		"
+"if(p2->NumResults==1)\r\n		{\r\n			NewSubscriber->Port = 80;\r\n			path = (char*)malloc(1+TempStringLength"
+" - p2->FirstResult->datalength -1);\r\n			memcpy(path,TempString + p2->FirstResult->datalength,TempStr"
+"ingLength - p2->FirstResult->datalength -1);\r\n			path[TempStringLength - p2->FirstResult->datalength"
+" - 1] = '\\0';\r\n			NewSubscriber->Path = path;\r\n			NewSubscriber->PathLength = (int)strlen(path);\r\n		"
+"}\r\n		else\r\n		{\r\n			ILibGetLong(p2->LastResult->data,p2->LastResult->datalength,&TempLong);\r\n			NewSu"
+"bscriber->Port = (unsigned short)TempLong;\r\n			if(TempStringLength==p->FirstResult->datalength)\r\n			"
+"{\r\n				path = (char*)malloc(2);\r\n				memcpy(path,\"/\",1);\r\n				path[1] = '\\0';\r\n			}\r\n			else\r\n			{\r\n"
+"				path = (char*)malloc(1+TempStringLength - p->FirstResult->datalength -1);\r\n				memcpy(path,TempS"
+"tring + p->FirstResult->datalength,TempStringLength - p->FirstResult->datalength -1);\r\n				path[Temp"
+"StringLength - p->FirstResult->datalength -1] = '\\0';\r\n			}\r\n			NewSubscriber->Path = path;\r\n			NewS"
+"ubscriber->PathLength = (int)strlen(path);\r\n		}\r\n		ILibDestructParserResults(p);\r\n		ILibDestructPars"
+"erResults(p2);\r\n		free(TempString2);\r\n		\r\n		\r\n		escapedURI = (char*)malloc(ILibHTTPEscapeLength(NewS"
+"ubscriber->Path));\r\n		escapedURILength = ILibHTTPEscape(escapedURI,NewSubscriber->Path);\r\n		\r\n		free"
+"(NewSubscriber->Path);\r\n		NewSubscriber->Path = escapedURI;\r\n		NewSubscriber->PathLength = escapedUR"
+"ILength;\r\n		\r\n		\r\n		NewSubscriber->RefCount = 1;\r\n		NewSubscriber->Disposing = 0;\r\n		NewSubscriber->"
+"Previous = NULL;\r\n		NewSubscriber->SID = SID;\r\n		NewSubscriber->SIDLength = (int)strlen(SID);\r\n		New"
+"Subscriber->SEQ = 0;\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n		NewSubscriber->RenewByTime = (Get"
+"TickCount() / 1000) + Timeout;\r\n#else\r\n		gettimeofday(&(NewSubscriber->RenewByTime),NULL);\r\n		(NewSu"
+"bscriber->RenewByTime).tv_sec += (int)Timeout;\r\n#endif\r\n		NewSubscriber->Next = *HeadPtr;\r\n		if(*Hea"
+"dPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}\r\n		*HeadPtr = NewSubscriber;\r\n		++(*TotalSubscri"
+"bers);\r\n		LVL3DEBUG(printf(\"\\r\\n\\r\\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\\r\\n\",NewSubscribe"
+"r->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0"
+"xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)\r\n#if defined(WIN32) || defined("
+"_WIN32_WCE)	\r\n		LVL3DEBUG(printf(\"TIMESTAMP: %d <%d>\\r\\n\\r\\n\",(NewSubscriber->RenewByTime)-Timeout,N"
+"ewSubscriber);)\r\n#else\r\n		LVL3DEBUG(printf(\"TIMESTAMP: %d <%d>\\r\\n\\r\\n\",(NewSubscriber->RenewByTime)"
+".tv_sec-Timeout,NewSubscriber);)\r\n#endif\r\n		packet = (char*)malloc(134 + (int)strlen(SID) + 4);\r\n		p"
+"acketlength = sprintf(packet,\"HTTP/!HTTPVERSION! 200 OK\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel Mic"
+"roStack/!MICROSTACKVERSION!\\r\\nSID: %s\\r\\nTIMEOUT: Second-%ld\\r\\nContent-Length: 0\\r\\n\\r\\n\",PLATFORM"
+",SID,Timeout);\r\n		//{{{TryToSubscribe_InitialEvent}}}\r\n		if (packetbody != NULL)	    {\r\n			ILibWebSe"
+"rver_Send_Raw(session,packet,packetlength,0,1);\r\n			\r\n			UPnPSendEvent_Body(dataObject,packetbody,pa"
+"cketbodyLength,NewSubscriber);\r\n			free(packetbody);\r\n		} \r\n	}\r\n	else\r\n	{\r\n		/* Too many subscribers"
+" */\r\n		ILibWebServer_Send_Raw(session,\"HTTP/1.1 412 Too Many Subscribers\\r\\nContent-Length: 0\\r\\n\\r\\"
+"n\",56,1,1);\r\n	}\r\n}\r\nvoid UPnPSubscribeEvents(char* path,int pathlength,char* Timeout,int TimeoutLeng"
+"th,char* URL,int URLLength,struct ILibWebServer_Session* session)\r\n{\r\n	long TimeoutVal;\r\n	char* buff"
+"er = (char*)malloc(1+sizeof(char)*pathlength);\r\n	\r\n	ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);\r"
+"\n	memcpy(buffer,path,pathlength);\r\n	buffer[pathlength] = '\\0';\r\n	free(buffer);\r\n	if(TimeoutVal>7200)"
+" {TimeoutVal=7200;}\r\n	\r\n//{{{SubscribeEventsDispatcher}}}\r\n}\r\nvoid UPnPRenewEvents(char* path,int pa"
+"thlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct ILibWebServer_Session *R"
+"eaderObject)\r\n{\r\n	struct SubscriberInfo *info = NULL;\r\n	long TimeoutVal;\r\n#if !defined(WIN32) && !de"
+"fined(_WIN32_WCE)\r\n	struct timeval tv;\r\n#endif\r\n	char* packet;\r\n	int packetlength;\r\n	char* SID = (ch"
+"ar*)malloc(SIDLength+1);\r\n	memcpy(SID,_SID,SIDLength);\r\n	SID[SIDLength] ='\\0';\r\n#if defined(WIN32) |"
+"| defined(_WIN32_WCE)\r\n	LVL3DEBUG(printf(\"\\r\\n\\r\\nTIMESTAMP: %d\\r\\n\",GetTickCount()/1000);)\r\n#else\r\n"
+"	LVL3DEBUG(gettimeofday(&tv,NULL);)\r\n	LVL3DEBUG(printf(\"\\r\\n\\r\\nTIMESTAMP: %d\\r\\n\",tv.tv_sec);)\r\n#en"
+"dif\r\n	LVL3DEBUG(printf(\"SUBSCRIBER [%s] attempting to Renew Events for %s Duration [\",SID,Timeout);)"
+"\r\n	\r\n//{{{RenewHeadInitializer}}}\r\n	while(info!=NULL && strcmp(info->SID,SID)!=0)\r\n	{\r\n		info = info"
+"->Next;\r\n	}\r\n	if(info!=NULL)\r\n	{\r\n		ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);\r\n#if defined(WIN"
+"32) || defined(_WIN32_WCE)\r\n		info->RenewByTime = TimeoutVal + (GetTickCount() / 1000);\r\n#else\r\n		ge"
+"ttimeofday(&tv,NULL);\r\n		(info->RenewByTime).tv_sec = tv.tv_sec + TimeoutVal;\r\n#endif\r\n		packet = (c"
+"har*)malloc(134 + (int)strlen(SID) + 4);\r\n		packetlength = sprintf(packet,\"HTTP/!HTTPVERSION! 200 OK"
+"\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Intel MicroStack/!MICROSTACKVERSION!\\r\\nSID: %s\\r\\nTIMEOUT: Seco"
+"nd-%ld\\r\\nContent-Length: 0\\r\\n\\r\\n\",PLATFORM,SID,TimeoutVal);\r\n		ILibWebServer_Send_Raw(ReaderObjec"
+"t,packet,packetlength,0,1);\r\n		LVL3DEBUG(printf(\"OK] {%d} <%d>\\r\\n\\r\\n\",TimeoutVal,info);)\r\n	}\r\n	els"
+"e\r\n	{\r\n		LVL3DEBUG(printf(\"FAILED]\\r\\n\\r\\n\");)\r\n		ILibWebServer_Send_Raw(ReaderObject,\"HTTP/!HTTPVER"
+"SION! 412 Precondition Failed\\r\\nContent-Length: 0\\r\\n\\r\\n\",55,1,1);\r\n	}\r\n	free(SID);\r\n}\r\nvoid UPnPP"
+"rocessSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)\r\n{\r\n	char* SID ="
+" NULL;\r\n	int SIDLength = 0;\r\n	char* Timeout = NULL;\r\n	int TimeoutLength = 0;\r\n	char* URL = NULL;\r\n	i"
+"nt URLLength = 0;\r\n	struct parser_result *p;\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	struct parser_resul"
+"t *r,*r2;\r\n	struct parser_result_field *prf;\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n	struct packetheader_f"
+"ield_node *f;\r\n	\r\n	f = header->FirstField;\r\n	while(f!=NULL)\r\n	{\r\n		if(f->FieldLength==3 && strncasec"
+"mp(f->Field,\"SID\",3)==0)\r\n		{\r\n			SID = f->FieldData;\r\n			SIDLength = f->FieldDataLength;\r\n		}\r\n		el"
+"se if(f->FieldLength==8 && strncasecmp(f->Field,\"Callback\",8)==0)\r\n		{\r\n			URL = f->FieldData;\r\n			U"
+"RLLength = f->FieldDataLength;\r\n		}\r\n		else if(f->FieldLength==7 && strncasecmp(f->Field,\"Timeout\",7"
+")==0)\r\n		{\r\n			Timeout = f->FieldData;\r\n			TimeoutLength = f->FieldDataLength;\r\n		}\r\n		//{{{BEGIN_UP"
+"nP/1.1_Specific}}}\r\n		else if(f->FieldLength==10 && strncasecmp(f->Field,\"USER-AGENT\",10)==0)\r\n		{\r\n"
+"			// Check UPnP version of the Control Point which invoked us\r\n			r = ILibParseString(f->FieldData,"
+"0,f->FieldDataLength,\" \",1);\r\n			prf = r->FirstResult;\r\n			while(prf!=NULL)\r\n			{\r\n				if(prf->datal"
+"ength>5 && memcmp(prf->data,\"UPnP/\",5)==0)\r\n				{\r\n					r2 = ILibParseString(prf->data+5,0,prf->data"
+"length-5,\".\",1);\r\n					r2->FirstResult->data[r2->FirstResult->datalength]=0;\r\n					r2->LastResult->d"
+"ata[r2->LastResult->datalength]=0;\r\n					if(atoi(r2->FirstResult->data)==1 && atoi(r2->LastResult->d"
+"ata)>0)\r\n					{\r\n						session->Reserved9=1;\r\n					}\r\n					ILibDestructParserResults(r2);\r\n				}\r\n		"
+"		prf = prf->NextResult;\r\n			}\r\n			ILibDestructParserResults(r);\r\n		}\r\n		//{{{END_UPnP/1.1_Specific}"
+"}}\r\n		f = f->NextField;\r\n	}\r\n	if(Timeout==NULL)\r\n	{\r\n		Timeout = \"7200\";\r\n		TimeoutLength = 4;\r\n	}\r\n"
+"	else\r\n	{\r\n		p = ILibParseString(Timeout,0,TimeoutLength,\"-\",1);\r\n		if(p->NumResults==2)\r\n		{\r\n			Ti"
+"meout = p->LastResult->data;\r\n			TimeoutLength = p->LastResult->datalength;\r\n			if(TimeoutLength==8 "
+"&& strncasecmp(Timeout,\"INFINITE\",8)==0)\r\n			{\r\n				Timeout = \"7200\";\r\n				TimeoutLength = 4;\r\n			}\r"
+"\n		}\r\n		else\r\n		{\r\n			Timeout = \"7200\";\r\n			TimeoutLength = 4;\r\n		}\r\n		ILibDestructParserResults(p);"
+"\r\n	}\r\n	if(SID==NULL)\r\n	{\r\n		/* Subscribe */\r\n		UPnPSubscribeEvents(header->DirectiveObj,header->Dire"
+"ctiveObjLength,Timeout,TimeoutLength,URL,URLLength,session);\r\n	}\r\n	else\r\n	{\r\n		/* Renew */\r\n		UPnPRe"
+"newEvents(header->DirectiveObj,header->DirectiveObjLength,SID,SIDLength,Timeout,TimeoutLength,sessio"
+"n);\r\n	}\r\n}\r\nvoid UPnPProcessHTTPPacket(struct ILibWebServer_Session *session, struct packetheader* h"
+"eader, char *bodyBuffer, int offset, int bodyBufferLength)\r\n\r\n{\r\n	struct UPnPDataObject *dataObject "
+"= (struct UPnPDataObject*)session->User;\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	char *response"
+"Header = \"\\r\\nCONTENT-TYPE:  text/xml\\r\\nServer: WINDOWS, UPnP/!UPNPVERSION!, Intel MicroStack/!MICR"
+"OSTACKVERSION!\";\r\n#else\r\n	char *responseHeader = \"\\r\\nCONTENT-TYPE:  text/xml\\r\\nServer: POSIX, UPnP"
+"/!UPNPVERSION!, Intel MicroStack/!MICROSTACKVERSION!\";\r\n#endif\r\n	char *errorTemplate = \"HTTP/!HTTPVE"
+"RSION! %d %s\\r\\nServer: %s, UPnP/!UPNPVERSION!, Intel MicroStack/!MICROSTACKVERSION!\\r\\nContent-Leng"
+"th: 0\\r\\n\\r\\n\";\r\n	char *errorPacket;\r\n	int errorPacketLength;\r\n	char *buffer;\r\n\r\n	LVL3DEBUG(errorPac"
+"ketLength=ILibGetRawPacket(header,&errorPacket);)\r\n	LVL3DEBUG(printf(\"%s\\r\\n\",errorPacket);)\r\n	LVL3D"
+"EBUG(free(errorPacket);)			\r\n	/* Virtual Directory Support */\r\n	if(header->DirectiveObjLength>=4 && "
+"memcmp(header->DirectiveObj,\"/web\",4)==0)\r\n	{\r\n		UPnPFP_PresentationPage((void*)session,header);\r\n	}"
+"\r\n	else if(header->DirectiveLength==4 && memcmp(header->Directive,\"HEAD\",4)==0)\r\n	{\r\n		if(header->Di"
+"rectiveObjLength==1 && memcmp(header->DirectiveObj,\"/\",1)==0)\r\n		{\r\n			ILibWebServer_StreamHeader_Ra"
+"w(session,200,\"OK\",responseHeader,1);\r\n			ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_Me"
+"moryOwnership_STATIC,1);\r\n		}\r\n//{{{HeadDispatcher}}}\r\n		else\r\n		{\r\n			errorPacket = (char*)malloc(1"
+"28);\r\n			errorPacketLength = sprintf(errorPacket,errorTemplate,404,\"File Not Found\",PLATFORM);\r\n			I"
+"LibWebServer_Send_Raw(session,errorPacket,errorPacketLength,0,1);\r\n		}\r\n	}\r\n	else if(header->Directi"
+"veLength==3 && memcmp(header->Directive,\"GET\",3)==0)\r\n	{\r\n		if(header->DirectiveObjLength==1 && memc"
+"mp(header->DirectiveObj,\"/\",1)==0)\r\n		{\r\n			ILibWebServer_StreamHeader_Raw(session,200,\"OK\",response"
+"Header,1);\r\n			ILibWebServer_StreamBody(session,dataObject->DeviceDescription,dataObject->DeviceDesc"
+"riptionLength,1,1);\r\n		}\r\n//{{{GetDispatcher}}}\r\n		else\r\n		{\r\n			errorPacket = (char*)malloc(128);\r\n"
+"			errorPacketLength = sprintf(errorPacket,errorTemplate,404,\"File Not Found\",PLATFORM);\r\n			ILibWeb"
+"Server_Send_Raw(session,errorPacket,errorPacketLength,0,1);\r\n		}\r\n	}\r\n	else if(header->DirectiveLeng"
+"th==4 && memcmp(header->Directive,\"POST\",4)==0)\r\n	{\r\n		if(UPnPProcessPOST(session,header,bodyBuffer,"
+"offset,bodyBufferLength)!=0)\r\n		{\r\n			UPnPResponse_Error(session,401,\"Invalid Action\");\r\n		}\r\n	}\r\n	e"
+"lse if(header->DirectiveLength==9 && memcmp(header->Directive,\"SUBSCRIBE\",9)==0)\r\n	{\r\n		UPnPProcessS"
+"UBSCRIBE(header,session);\r\n	}\r\n	else if(header->DirectiveLength==11 && memcmp(header->Directive,\"UNS"
+"UBSCRIBE\",11)==0)\r\n	{\r\n		UPnPProcessUNSUBSCRIBE(header,session);\r\n	}\r\n	else\r\n	{\r\n		errorPacket = (ch"
+"ar*)malloc(128);\r\n		errorPacketLength = sprintf(errorPacket,errorTemplate,400,\"Bad Request\",PLATFORM"
+");\r\n		ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,1,1);\r\n	}\r\n}\r\nvoid UPnPFragmented"
+"SendNotify_Destroy(void *data);\r\nvoid UPnPMasterPreSelect(void* object,fd_set *socketset, fd_set *wr"
+"iteset, fd_set *errorset, int* blocktime)\r\n{\r\n	int i;\r\n	struct UPnPDataObject *UPnPObject = (struct "
+"UPnPDataObject*)object;\r\n	\r\n	int ra = 1;\r\n	struct sockaddr_in addr;\r\n	struct ip_mreq mreq;\r\n	unsigne"
+"d char TTL = 4;\r\n	struct UPnPFragmentNotifyStruct *f;\r\n	int timeout;\r\n	\r\n	if(UPnPObject->InitialNoti"
+"fy==0)\r\n	{\r\n		UPnPObject->InitialNotify = -1;\r\n		UPnPSendByeBye(UPnPObject);\r\n		for(i=1;i<=5;++i)\r\n	"
+"	{\r\n			f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof(struct UPnPFragmentNotifyStruct));\r\n			f-"
+">packetNumber=i;\r\n			f->upnp = UPnPObject;\r\n			timeout = (int)(0 + ((unsigned short)rand() % (500)))"
+";\r\n			do\r\n			{\r\n				f->upnp->InitialNotify = rand();\r\n			}while(f->upnp->InitialNotify==0);\r\n			ILib"
+"LifeTime_AddEx(f->upnp->WebServerTimer,f,timeout,&UPnPFragmentedSendNotify,&UPnPFragmentedSendNotify"
+"_Destroy);\r\n		}\r\n	}\r\n	if(UPnPObject->UpdateFlag!=0)\r\n	{\r\n		UPnPObject->UpdateFlag = 0;\r\n		\r\n		/* Cle"
+"ar Sockets */\r\n		//{{{BEGIN_UPnP/1.1_Specific}}}\r\n		UPnPFreeUnicastReceiveSockets(UPnPObject);\r\n		//"
+"{{{END_UPnP/1.1_Specific}}}\r\n		for(i=0;i<UPnPObject->AddressListLength;++i)\r\n		{\r\n#if defined(WIN32)"
+" || defined(_WIN32_WCE)\r\n			closesocket(UPnPObject->NOTIFY_SEND_socks[i]);\r\n#else\r\n			close(UPnPObje"
+"ct->NOTIFY_SEND_socks[i]);\r\n#endif\r\n		}\r\n		free(UPnPObject->NOTIFY_SEND_socks);\r\n		\r\n		/* Set up soc"
+"ket */\r\n		free(UPnPObject->AddressList);\r\n		UPnPObject->AddressListLength = ILibGetLocalIPAddressLis"
+"t(&(UPnPObject->AddressList));\r\n#if defined(WIN32) || defined(_WIN32_WCE)	\r\n		UPnPObject->NOTIFY_SEN"
+"D_socks = (SOCKET*)malloc(sizeof(int)*(UPnPObject->AddressListLength));\r\n#else\r\n		UPnPObject->NOTIFY"
+"_SEND_socks = (int*)malloc(sizeof(int)*(UPnPObject->AddressListLength));\r\n#endif\r\n		for(i=0;i<UPnPOb"
+"ject->AddressListLength;++i)\r\n		{\r\n			UPnPObject->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRAM,"
+" 0);\r\n			memset((char *)&(addr), 0, sizeof(addr));\r\n			addr.sin_family = AF_INET;\r\n			addr.sin_addr."
+"s_addr = UPnPObject->AddressList[i];\r\n			addr.sin_port = (unsigned short)htons(UPNP_PORT);\r\n			if (s"
+"etsockopt(UPnPObject->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)\r\n"
+"			{\r\n				if (setsockopt(UPnPObject->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL,"
+" sizeof(TTL)) < 0)\r\n				{\r\n					// Ignore the case if setting the Multicast-TTL fails\r\n				}\r\n				if"
+" (bind(UPnPObject->NOTIFY_SEND_socks[i], (struct sockaddr *) &(addr), sizeof(addr)) == 0)\r\n				{\r\n		"
+"			mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);\r\n					mreq.imr_interface.s_addr = UPnPObject->"
+"AddressList[i];\r\n					if (setsockopt(UPnPObject->NOTIFY_RECEIVE_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,"
+"(char*)&mreq, sizeof(mreq)) < 0)\r\n					{\r\n						// Does not matter if it fails, just ignore\r\n					}\r"
+"\n				}\r\n			}\r\n		}\r\n		for(i=1;i<=5;++i)\r\n		{\r\n			f = (struct UPnPFragmentNotifyStruct*)malloc(sizeof("
+"struct UPnPFragmentNotifyStruct));\r\n			f->packetNumber=i;\r\n			f->upnp = UPnPObject;\r\n			timeout = (i"
+"nt)(0 + ((unsigned short)rand() % (500)));\r\n			ILibLifeTime_AddEx(f->upnp->WebServerTimer,f,timeout,"
+"&UPnPFragmentedSendNotify,&UPnPFragmentedSendNotify_Destroy);\r\n		}\r\n	}\r\n	FD_SET(UPnPObject->NOTIFY_R"
+"ECEIVE_sock,socketset);\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	for(i=0;i<UPnPObject->AddressListLength;"
+"++i)\r\n	{\r\n		FD_SET(UPnPObject->UnicastReceiveSockets[i],socketset);\r\n	}\r\n	//{{{END_UPnP/1.1_Specific"
+"}}}\r\n}\r\n\r\nvoid UPnPMasterPostSelect(void* object,int slct, fd_set *socketset, fd_set *writeset, fd_s"
+"et *errorset)\r\n{\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	unsigned long flags=0;\r\n#endif\r\n	int c"
+"nt = 0;\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	int i;\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n	struct packethe"
+"ader *packet;\r\n	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;\r\n	\r\n	if(slct>0)\r"
+"\n	{\r\n		//{{{BEGIN_UPnP/1.1_Specific}}}\r\n		for(i=0;i<UPnPObject->AddressListLength;++i)\r\n		{\r\n			if(F"
+"D_ISSET(UPnPObject->UnicastReceiveSockets[i],socketset)!=0)\r\n			{	\r\n				cnt = recvfrom(UPnPObject->U"
+"nicastReceiveSockets[i], UPnPObject->message, sizeof(UPnPObject->message), 0,\r\n				(struct sockaddr "
+"*) &(UPnPObject->addr), &(UPnPObject->addrlen));\r\n				if (cnt < 0)\r\n				{\r\n					printf(\"recvfrom\");\r"
+"\n					exit(1);\r\n				}\r\n				else if (cnt == 0)\r\n				{\r\n					/* Socket Closed? */\r\n				}\r\n				packet ="
+" ILibParsePacketHeader(UPnPObject->message,0,cnt);\r\n				packet->Source = (struct sockaddr_in*)&(UPnP"
+"Object->addr);\r\n				packet->ReceivingAddress = 0;\r\n				if(packet->StatusCode==-1 && memcmp(packet->D"
+"irective,\"M-SEARCH\",8)==0)\r\n				{\r\n					UPnPProcessMSEARCH(UPnPObject, packet);\r\n				}\r\n				ILibDest"
+"ructPacket(packet);\r\n			}\r\n		}\r\n		//{{{END_UPnP/1.1_Specific}}}\r\n		if(FD_ISSET(UPnPObject->NOTIFY_RE"
+"CEIVE_sock,socketset)!=0)\r\n		{	\r\n			cnt = recvfrom(UPnPObject->NOTIFY_RECEIVE_sock, UPnPObject->mess"
+"age, sizeof(UPnPObject->message), 0,\r\n			(struct sockaddr *) &(UPnPObject->addr), &(UPnPObject->addr"
+"len));\r\n			if (cnt < 0)\r\n			{\r\n				printf(\"recvfrom\");\r\n				exit(1);\r\n			}\r\n			else if (cnt == 0)\r\n	"
+"		{\r\n				/* Socket Closed? */\r\n			}\r\n			packet = ILibParsePacketHeader(UPnPObject->message,0,cnt);\r\n"
+"			packet->Source = (struct sockaddr_in*)&(UPnPObject->addr);\r\n			packet->ReceivingAddress = 0;\r\n			"
+"if(packet->StatusCode==-1 && memcmp(packet->Directive,\"M-SEARCH\",8)==0)\r\n			{\r\n				UPnPProcessMSEARC"
+"H(UPnPObject, packet);\r\n			}\r\n			ILibDestructPacket(packet);\r\n		}\r\n		\r\n	}\r\n}\r\nvoid UPnPFragmentedSen"
+"dNotify_Destroy(void *data)\r\n{\r\n	free(data);\r\n}\r\nvoid UPnPFragmentedSendNotify(void *data)\r\n\r\n{\r\n	st"
+"ruct UPnPFragmentNotifyStruct *FNS = (struct UPnPFragmentNotifyStruct*)data;\r\n	int timeout;\r\n	int pa"
+"cketlength;\r\n	char* packet = (char*)malloc(5000);\r\n	int i,i2;\r\n	struct sockaddr_in addr;\r\n	int addrl"
+"en;\r\n	struct in_addr interface_addr;\r\n	\r\n	memset((char *)&addr, 0, sizeof(addr));\r\n	addr.sin_family "
+"= AF_INET;\r\n	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);\r\n	addr.sin_port = (unsigned short)htons(U"
+"PNP_PORT);\r\n	addrlen = sizeof(addr);\r\n	\r\n	memset((char *)&interface_addr, 0, sizeof(interface_addr))"
+";\r\n	\r\n	for(i=0;i<FNS->upnp->AddressListLength;++i)\r\n	{\r\n		interface_addr.s_addr = FNS->upnp->Address"
+"List[i];\r\n		if (setsockopt(FNS->upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&inte"
+"rface_addr, sizeof(interface_addr)) == 0)\r\n		{\r\n			for (i2=0;i2<2;i2++)\r\n			{\r\n				switch(FNS->packe"
+"tNumber)\r\n				{\r\n//{{{FragmentedSendNotifyCaseStatements}}}					\r\n				}\r\n			}\r\n		}\r\n	}\r\n	free(packet"
+");\r\n	timeout = (int)((FNS->upnp->NotifyCycleTime/4) + ((unsigned short)rand() % (FNS->upnp->NotifyCy"
+"cleTime/2 - FNS->upnp->NotifyCycleTime/4)));\r\n	ILibLifeTime_Add(FNS->upnp->WebServerTimer,FNS,timeou"
+"t,&UPnPFragmentedSendNotify,&UPnPFragmentedSendNotify_Destroy);\r\n}\r\nvoid UPnPSendNotify(const struct"
+" UPnPDataObject *upnp)\r\n{\r\n	int packetlength;\r\n	char* packet = (char*)malloc(5000);\r\n	int i,i2;\r\n	st"
+"ruct sockaddr_in addr;\r\n	int addrlen;\r\n	struct in_addr interface_addr;\r\n	\r\n	memset((char *)&addr, 0,"
+" sizeof(addr));\r\n	addr.sin_family = AF_INET;\r\n	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);\r\n	addr."
+"sin_port = (unsigned short)htons(UPNP_PORT);\r\n	addrlen = sizeof(addr);\r\n	\r\n	memset((char *)&interfac"
+"e_addr, 0, sizeof(interface_addr));\r\n	\r\n	for(i=0;i<upnp->AddressListLength;++i)\r\n	{\r\n		interface_add"
+"r.s_addr = upnp->AddressList[i];\r\n#if !defined(_WIN32_WCE) || (defined(_WIN32_WCE) && _WIN32_WCE>=4)"
+"\r\n		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, s"
+"izeof(interface_addr)) == 0)\r\n		{\r\n#endif\r\n			for (i2=0;i2<2;i2++)\r\n			{\r\n//{{{SendNotifyForStatemen"
+"t}}}\r\n			}\r\n#if !defined(_WIN32_WCE) || (defined(_WIN32_WCE) && _WIN32_WCE>=4)\r\n		}\r\n#endif\r\n	}\r\n	fr"
+"ee(packet);\r\n}\r\n//{{{BEGIN_UPnP/1.0_Specific}}}\r\n#define UPnPBuildSsdpByeByePacket(outpacket,outleng"
+"th,USN,USNex,NT,NTex,DeviceID)\\\r\n{\\\r\n	if(DeviceID==0)\\\r\n	{\\\r\n		*outlength = sprintf(outpacket,\"NOTIF"
+"Y * HTTP/1.1\\r\\nHOST: 239.255.255.250:1900\\r\\nNTS: ssdp:byebye\\r\\nUSN: uuid:%s%s\\r\\nNT: %s%s\\r\\nCont"
+"ent-Length: 0\\r\\n\\r\\n\",USN,USNex,NT,NTex);\\\r\n	}\\\r\n	else\\\r\n	{\\\r\n		if(memcmp(NT,\"uuid:\",5)==0)\\\r\n		{\\\r"
+"\n			*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nHOST: 239.255.255.250:1900\\r\\nNTS: ssdp:bye"
+"bye\\r\\nUSN: uuid:%s_%d%s\\r\\nNT: %s%s_%d\\r\\nContent-Length: 0\\r\\n\\r\\n\",USN,DeviceID,USNex,NT,NTex,Dev"
+"iceID);\\\r\n		}\\\r\n		else\\\r\n		{\\\r\n			*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nHOST: 239.255"
+".255.250:1900\\r\\nNTS: ssdp:byebye\\r\\nUSN: uuid:%s_%d%s\\r\\nNT: %s%s\\r\\nContent-Length: 0\\r\\n\\r\\n\",USN"
+",DeviceID,USNex,NT,NTex);\\\r\n		}\\\r\n	}\\\r\n}\r\n//{{{END_UPnP/1.0_Specific}}}\r\n//{{{BEGIN_UPnP/1.1_Specifi"
+"c}}}\r\n#define UPnPBuildSsdpByeByePacket(outpacket,outlength,ConfigID, BootID, MaxVersion, SearchPort"
+",USN,USNex,NT,NTex,DeviceID)\\\r\n{\\\r\n	if(DeviceID==0)\\\r\n	{\\\r\n		*outlength = sprintf(outpacket,\"NOTIFY "
+"* HTTP/1.1\\r\\nCONFIGID.UPNP.ORG: %d\\r\\nBOOTID.UPNP.ORG: %d\\r\\nMAXVERSION.UPNP.ORG: %d\\r\\nSEARCHPORT."
+"UPNP.ORG: %u\\r\\nHOST: 239.255.255.250:1900\\r\\nNTS: ssdp:byebye\\r\\nUSN: uuid:%s%s\\r\\nNT: %s%s\\r\\nCont"
+"ent-Length: 0\\r\\n\\r\\n\",ConfigID,BootID,MaxVersion,SearchPort,USN,USNex,NT,NTex);\\\r\n	}\\\r\n	else\\\r\n	{\\\r"
+"\n		if(memcmp(NT,\"uuid:\",5)==0)\\\r\n		{\\\r\n			*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nCONFI"
+"GID.UPNP.ORG: %d\\r\\nBOOTID.UPNP.ORG: %d\\r\\nMAXVERSION.UPNP.ORG: %d\\r\\nSEARCHPORT.UPNP.ORG: %u\\r\\nHOS"
+"T: 239.255.255.250:1900\\r\\nNTS: ssdp:byebye\\r\\nUSN: uuid:%s_%d%s\\r\\nNT: %s%s_%d\\r\\nContent-Length: 0"
+"\\r\\n\\r\\n\",ConfigID,BootID,MaxVersion,SearchPort,USN,DeviceID,USNex,NT,NTex,DeviceID);\\\r\n		}\\\r\n		else"
+"\\\r\n		{\\\r\n			*outlength = sprintf(outpacket,\"NOTIFY * HTTP/1.1\\r\\nCONFIGID.UPNP.ORG: %d\\r\\nBOOTID.UPN"
+"P.ORG: %d\\r\\nMAXVERSION.UPNP.ORG: %d\\r\\nSEARCHPORT.UPNP.ORG: %u\\r\\nHOST: 239.255.255.250:1900\\r\\nNTS"
+": ssdp:byebye\\r\\nUSN: uuid:%s_%d%s\\r\\nNT: %s%s\\r\\nContent-Length: 0\\r\\n\\r\\n\",ConfigID,BootID,MaxVers"
+"ion,SearchPort,USN,DeviceID,USNex,NT,NTex);\\\r\n		}\\\r\n	}\\\r\n}\r\n//{{{END_UPnP/1.1_Specific}}}\r\nvoid UPnP"
+"SendByeBye(const struct UPnPDataObject *upnp)\r\n{\r\n	int TempVal=0;\r\n	int packetlength;\r\n	char* packet"
+" = (char*)malloc(5000);\r\n	int i, i2;\r\n	struct sockaddr_in addr;\r\n	int addrlen;\r\n	struct in_addr inte"
+"rface_addr;\r\n	\r\n	memset((char *)&addr, 0, sizeof(addr));\r\n	addr.sin_family = AF_INET;\r\n	addr.sin_add"
+"r.s_addr = inet_addr(UPNP_GROUP);\r\n	addr.sin_port = (unsigned short)htons(UPNP_PORT);\r\n	addrlen = si"
+"zeof(addr);\r\n	\r\n	memset((char *)&interface_addr, 0, sizeof(interface_addr));\r\n	\r\n	for(i=0;i<upnp->Ad"
+"dressListLength;++i)\r\n	{\r\n		\r\n		interface_addr.s_addr = upnp->AddressList[i];\r\n#if !defined(_WIN32_W"
+"CE) || (defined(_WIN32_WCE) && _WIN32_WCE>=4)\r\n		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_"
+"IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)\r\n		{\r\n#endif		\r\n			//{{{BE"
+"GIN_UPnP/1.1_Specific}}}\r\n			if(upnp->InitialNotify!=-1)\r\n			{\r\n				TempVal = upnp->InitialNotify;  "
+"\r\n			}\r\n			//{{{END_UPnP/1.1_Specific}}}\r\n			for (i2=0;i2<2;i2++)\r\n			{\r\n//{{{SendByeByeForStatement"
+"}}}\r\n			}\r\n#if !defined(_WIN32_WCE) || (defined(_WIN32_WCE) && _WIN32_WCE>=4)\r\n		}\r\n#endif\r\n	}\r\n	fre"
+"e(packet);\r\n}\r\n\r\nvoid UPnPResponse_Error(const void* UPnPToken, const int ErrorCode, const char* Err"
+"orMsg)\r\n{\r\n	char* body;\r\n	int bodylength;\r\n	char* head;\r\n	int headlength;\r\n	body = (char*)malloc(395"
+" + (int)strlen(ErrorMsg));\r\n	bodylength = sprintf(body,\"<s:Envelope\\r\\n xmlns:s=\\\"http://schemas.xml"
+"soap.org/soap/envelope/\\\" s:encodingStyle=\\\"http://schemas.xmlsoap.org/soap/encoding/\\\"><s:Body><s:F"
+"ault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\\\"u"
+"rn:schemas-upnp-org:control-1-0\\\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></"
+"UPnPError></detail></s:Fault></s:Body></s:Envelope>\",ErrorCode,ErrorMsg);\r\n	head = (char*)malloc(59)"
+";\r\n	headlength = sprintf(head,\"HTTP/1.1 500 Internal\\r\\nContent-Length: %d\\r\\n\\r\\n\",bodylength);\r\n	I"
+"LibWebServer_Send_Raw((struct ILibWebServer_Session*)UPnPToken,head,headlength,0,0);\r\n	ILibWebServer"
+"_Send_Raw((struct ILibWebServer_Session*)UPnPToken,body,bodylength,0,1);\r\n}\r\n\r\nint UPnPGetLocalInter"
+"faceToHost(const void* UPnPToken)\r\n{\r\n	return(ILibWebServer_GetLocalInterface((struct ILibWebServer_"
+"Session*)UPnPToken));\r\n}\r\n\r\nvoid UPnPResponseGeneric(const void* UPnPToken,const char* ServiceURI,co"
+"nst char* MethodName,const char* Params)\r\n{\r\n	char* packet;\r\n	int packetlength;\r\n	struct ILibWebServ"
+"er_Session *session = (struct ILibWebServer_Session*)UPnPToken;\r\n	int RVAL=0;\r\n	\r\n	packet = (char*)m"
+"alloc(239+strlen(ServiceURI)+strlen(Params)+(strlen(MethodName)*2));\r\n	packetlength = sprintf(packet"
+",\"<?xml version=\\\"1.0\\\" encoding=\\\"utf-8\\\"?>\\r\\n<s:Envelope s:encodingStyle=\\\"http://schemas.xmlsoap"
+".org/soap/encoding/\\\" xmlns:s=\\\"http://schemas.xmlsoap.org/soap/envelope/\\\"><s:Body><u:%sResponse xm"
+"lns:u=\\\"%s\\\">%s</u:%sResponse></s:Body></s:Envelope>\",MethodName,ServiceURI,Params,MethodName);\r\n	LV"
+"L3DEBUG(printf(\"SendBody: %s\\r\\n\",packet);)\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	RVAL=ILibWe"
+"bServer_StreamHeader_Raw(session,200,\"OK\",\"\\r\\nEXT:\\r\\nCONTENT-TYPE: text/xml\\r\\nSERVER: WINDOWS, UP"
+"nP/!UPNPVERSION!, Intel MicroStack/!MICROSTACKVERSION!\",1);\r\n#else\r\n	RVAL=ILibWebServer_StreamHeader"
+"_Raw(session,200,\"OK\",\"\\r\\nEXT:\\r\\nCONTENT-TYPE: text/xml\\r\\nSERVER: POSIX, UPnP/!UPNPVERSION!, Inte"
+"l MicroStack/!MICROSTACKVERSION!\",1);\r\n#endif\r\n	if(RVAL!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR"
+" && RVAL != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)\r\n	{\r\n		RVAL=ILibWebServer_StreamBody(session,"
+"packet,packetlength,0,1);\r\n	}\r\n}\r\n\r\n//{{{InvokeResponseMethods}}}\r\n\r\nvoid UPnPSendEventSink(\r\nvoid *"
+"WebReaderToken,\r\nint IsInterrupt,\r\nstruct packetheader *header,\r\nchar *buffer,\r\nint *p_BeginPointer,"
+"\r\nint EndPointer,\r\nint done,\r\nvoid *subscriber,\r\nvoid *upnp,\r\nint *PAUSE)	\r\n{\r\n	if(done!=0 && ((stru"
+"ct SubscriberInfo*)subscriber)->Disposing==0)\r\n	{\r\n		sem_wait(&(((struct UPnPDataObject*)upnp)->Even"
+"tLock));\r\n		--((struct SubscriberInfo*)subscriber)->RefCount;\r\n		if(((struct SubscriberInfo*)subscri"
+"ber)->RefCount==0)\r\n		{\r\n			LVL3DEBUG(printf(\"\\r\\n\\r\\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSU"
+"BSCRIBE while trying to send event\\r\\n\\r\\n\",((struct SubscriberInfo*)subscriber)->SID,(((struct Subs"
+"criberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((("
+"(struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Addr"
+"ess>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)\r\n			UPnPDestructSubscriberInfo(((struct"
+" SubscriberInfo*)subscriber));\r\n		}\r\n		else if(header==NULL)\r\n		{\r\n			LVL3DEBUG(printf(\"\\r\\n\\r\\nCoul"
+"d not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\\r\\n\\r\\n\",((struct SubscriberInfo*)subscrib"
+"er)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)"
+"->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct Subscriber"
+"Info*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)\r\n			// Could not"
+" send Event, so unsubscribe the subscriber\r\n			((struct SubscriberInfo*)subscriber)->Disposing = 1;\r"
+"\n			UPnPExpireSubscriberInfo(upnp,subscriber);\r\n		}\r\n		sem_post(&(((struct UPnPDataObject*)upnp)->Ev"
+"entLock));\r\n	}\r\n}\r\nvoid UPnPSendEvent_Body(void *upnptoken,char *body,int bodylength,struct Subscrib"
+"erInfo *info)\r\n{\r\n	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;\r\n	struct s"
+"ockaddr_in dest;\r\n	int packetLength;\r\n	char *packet;\r\n	int ipaddr;\r\n	\r\n	memset(&dest,0,sizeof(dest))"
+";\r\n	dest.sin_addr.s_addr = info->Address;\r\n	dest.sin_port = htons(info->Port);\r\n	dest.sin_family = A"
+"F_INET;\r\n	ipaddr = info->Address;\r\n	\r\n	packet = (char*)malloc(info->PathLength + bodylength + 483);\r"
+"\n	packetLength = sprintf(packet,\"NOTIFY %s HTTP/!HTTPVERSION!\\r\\nSERVER: %s, UPnP/!UPNPVERSION!, Int"
+"el MicroStack/!MICROSTACKVERSION!\\r\\nHOST: %d.%d.%d.%d:%d\\r\\nContent-Type: text/xml\\r\\nNT: upnp:even"
+"t\\r\\nNTS: upnp:propchange\\r\\nSID: %s\\r\\nSEQ: %d\\r\\nContent-Length: %d\\r\\n\\r\\n<?xml version=\\\"1.0\\\" e"
+"ncoding=\\\"utf-8\\\"?><e:propertyset xmlns:e=\\\"urn:schemas-upnp-org:event-1-0\\\"><e:property><%s></e:pro"
+"perty></e:propertyset>\",info->Path,PLATFORM,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((i"
+"paddr>>24)&0xFF),info->Port,info->SID,info->SEQ,bodylength+137,body);\r\n	++info->SEQ;\r\n	\r\n	++info->Re"
+"fCount;\r\n	ILibWebClient_PipelineRequestEx(UPnPObject->EventClient,&dest,packet,packetLength,0,NULL,0"
+",0,&UPnPSendEventSink,info,upnptoken);\r\n}\r\nvoid UPnPSendEvent(void *upnptoken, char* body, const int"
+" bodylength, const char* eventname)\r\n{\r\n	struct SubscriberInfo *info = NULL;\r\n	struct UPnPDataObject"
+"* UPnPObject = (struct UPnPDataObject*)upnptoken;\r\n	struct sockaddr_in dest;\r\n	LVL3DEBUG(struct time"
+"val tv;)\r\n	\r\n	if(UPnPObject==NULL)\r\n	{\r\n		free(body);\r\n		return;\r\n	}\r\n	sem_wait(&(UPnPObject->EventL"
+"ock));\r\n//{{{SendEventHeadPointerInitializer}}}\r\n	memset(&dest,0,sizeof(dest));\r\n	while(info!=NULL)\r"
+"\n	{\r\n		if(!UPnPSubscriptionExpired(info))\r\n		{\r\n			UPnPSendEvent_Body(upnptoken,body,bodylength,info"
+");\r\n		}\r\n		else\r\n		{\r\n			//Remove Subscriber\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n			LVL3DEBU"
+"G(printf(\"\\r\\n\\r\\nTIMESTAMP: %d\\r\\n\",GetTickCount()/1000);)\r\n#else\r\n			LVL3DEBUG(gettimeofday(&tv,NU"
+"LL);)\r\n			LVL3DEBUG(printf(\"\\r\\n\\r\\nTIMESTAMP: %d\\r\\n\",tv.tv_sec);)\r\n#endif\r\n			LVL3DEBUG(printf(\"Di"
+"d not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\\r\\n\\r\\n\",((struct SubscriberInfo*)info)->SID,(((s"
+"truct SubscriberInfo*)info)->Address&0xFF),((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((s"
+"truct SubscriberInfo*)info)->Address>>16)&0xFF),((((struct SubscriberInfo*)info)->Address>>24)&0xFF)"
+",((struct SubscriberInfo*)info)->Port,info);)\r\n		}\r\n		\r\n		info = info->Next;\r\n	}\r\n	\r\n	sem_post(&(UPn"
+"PObject->EventLock));\r\n}\r\n\r\n//{{{SetStateMethods}}}\r\n\r\nvoid UPnPDestroyMicroStack(void *object)\r\n{\r\n"
+"	struct UPnPDataObject *upnp = (struct UPnPDataObject*)object;\r\n	struct SubscriberInfo  *sinfo,*sinf"
+"o2;\r\n	UPnPSendByeBye(upnp);\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	UPnPFreeUnicastReceiveSockets(upnp);"
+"\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n	sem_destroy(&(upnp->EventLock));\r\n\r\n	//{{{UPnPDestroyMicroStack_F"
+"reeEventResources}}}\r\n	\r\n	free(upnp->AddressList);\r\n	free(upnp->NOTIFY_SEND_socks);\r\n	free(upnp->UUI"
+"D);\r\n	free(upnp->Serial);\r\n	free(upnp->DeviceDescription);\r\n	\r\n	//{{{UPnPDestroyMicroStack_DestructS"
+"ubscriber}}}\r\n\r\n#if defined(WIN32) || defined(_WIN32_WCE)\r\n	WSACleanup();\r\n#endif\r\n}\r\nint UPnPGetLoc"
+"alPortNumber(void *token)\r\n{\r\n	return(((struct UPnPDataObject*)((struct ILibWebServer_Session*)token"
+")->Parent)->WebSocketPortNumber);\r\n}\r\nvoid UPnPSessionReceiveSink(\r\nstruct ILibWebServer_Session *se"
+"nder,\r\nint InterruptFlag,\r\nstruct packetheader *header,\r\nchar *bodyBuffer,\r\nint *beginPointer,\r\nint "
+"endPointer,\r\nint done)\r\n{\r\n	if(header!=NULL && done !=0 && InterruptFlag==0)\r\n	{\r\n		UPnPProcessHTTPP"
+"acket(sender,header,bodyBuffer,beginPointer==NULL?0:*beginPointer,endPointer);\r\n		if(beginPointer!=N"
+"ULL) {*beginPointer = endPointer;}\r\n	}\r\n}\r\nvoid UPnPSessionSink(struct ILibWebServer_Session *Sessio"
+"nToken, void *user)\r\n{\r\n	SessionToken->OnReceive = &UPnPSessionReceiveSink;\r\n	SessionToken->User = u"
+"ser;\r\n}\r\nvoid *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const ch"
+"ar* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)\r\n{\r\n	struct UPnPDataOb"
+"ject* RetVal = (struct UPnPDataObject*)malloc(sizeof(struct UPnPDataObject));\r\n	char* DDT;\r\n\r\n#if de"
+"fined(WIN32) || defined(_WIN32_WCE)\r\n	WORD wVersionRequested;\r\n	WSADATA wsaData;\r\n	srand((int)GetTic"
+"kCount());\r\n	#ifdef WINSOCK1\r\n		wVersionRequested = MAKEWORD( 1, 1 );	\r\n	#elif WINSOCK2\r\n		wVersionR"
+"equested = MAKEWORD( 2, 0 );\r\n	#endif\r\n	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1)"
+";}\r\n#else\r\n	struct timeval tv;\r\n	gettimeofday(&tv,NULL);\r\n	srand((int)tv.tv_sec);\r\n#endif\r\n\r\n	UPnPIn"
+"it(RetVal,NotifyCycleSeconds,PortNum);\r\n	RetVal->ForceExit = 0;\r\n	RetVal->PreSelect = &UPnPMasterPre"
+"Select;\r\n	RetVal->PostSelect = &UPnPMasterPostSelect;\r\n	RetVal->Destroy = &UPnPDestroyMicroStack;\r\n	"
+"RetVal->InitialNotify = 0;\r\n	if (UDN != NULL)\r\n	{\r\n		RetVal->UUID = (char*)malloc((int)strlen(UDN)+6"
+");\r\n		sprintf(RetVal->UUID,\"uuid:%s\",UDN);\r\n		RetVal->UDN = RetVal->UUID + 5;\r\n	}\r\n	if (SerialNumber"
+" != NULL)\r\n	{\r\n		RetVal->Serial = (char*)malloc((int)strlen(SerialNumber)+1);\r\n		strcpy(RetVal->Seri"
+"al,SerialNumber);\r\n	}\r\n	//{{{BEGIN_UPnP/1.0_Specific}}}\r\n	RetVal->DeviceDescription = (char*)malloc("
+"UPnPDeviceDescriptionTemplateLengthUX + (int)strlen(FriendlyName) + (((int)strlen(RetVal->Serial) + "
+"(int)strlen(RetVal->UUID)) * 1));\r\n	//{{{END_UPnP/1.0_Specific}}}\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r"
+"\n	RetVal->DeviceDescription = (char*)malloc(10+UPnPDeviceDescriptionTemplateLengthUX + (int)strlen(F"
+"riendlyName) + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * 1));\r\n	//{{{END_UPnP/1.1"
+"_Specific}}}\r\n	\r\n	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);\r\n	\r\n	RetVal->HTTPServer = ILib"
+"WebServer_Create(Chain,UPNP_HTTP_MAXSOCKETS,PortNum,&UPnPSessionSink,RetVal);\r\n	RetVal->WebSocketPor"
+"tNumber=(int)ILibWebServer_GetPortNumber(RetVal->HTTPServer);\r\n\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}	\r\n"
+"	RetVal->ConfigID = RetVal->WebSocketPortNumber;	\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n\r\n	ILibAddToChain"
+"(Chain,RetVal);\r\n	RetVal->EventClient = ILibCreateWebClient(5,Chain);\r\n	RetVal->Chain = Chain;\r\n	Ret"
+"Val->UpdateFlag = 0;\r\n	\r\n	DDT = ILibDecompressString((char*)UPnPDeviceDescriptionTemplate,UPnPDevice"
+"DescriptionTemplateLength,UPnPDeviceDescriptionTemplateLengthUX);\r\n	//{{{BEGIN_UPnP/1.0_Specific}}}\r"
+"\n	RetVal->DeviceDescriptionLength = sprintf(RetVal->DeviceDescription,DDT,FriendlyName,RetVal->Seria"
+"l,RetVal->UDN);\r\n	//{{{END_UPnP/1.0_Specific}}}\r\n	//{{{BEGIN_UPnP/1.1_Specific}}}\r\n	RetVal->DeviceDe"
+"scriptionLength = sprintf(RetVal->DeviceDescription,DDT,RetVal->ConfigID,FriendlyName,RetVal->Serial"
+",RetVal->UDN);\r\n	//{{{END_UPnP/1.1_Specific}}}\r\n	free(DDT);\r\n	\r\n	sem_init(&(RetVal->EventLock),0,1);"
+"\r\n	return(RetVal);\r\n}\r\n\r\n//{{{ComplexTypeCode}}}\r\n\r\n\r\n\r\n";
// -=S3P4R470R=- {UPnPMicroStack_C}
		#endregion
		#region UPnPMicroStack.h
		// -=S3P4R470R=- {UPnPMicroStack_H}
		// -=S3P4R470R=- {UPnPMicroStack_H}
		#endregion
		#region ILibParsers.c
		// -=S3P4R470R=- {ILibParsers_C}
		// -=S3P4R470R=- {ILibParsers_C}
		#endregion
		#region ILibParsers.h
		// -=S3P4R470R=- {ILibParsers_H}
		// -=S3P4R470R=- {ILibParsers_H}
		#endregion
		#region ILibAsyncSocket.c
		// -=S3P4R470R=- {ILibAsyncSocket_C}
		// -=S3P4R470R=- {ILibAsyncSocket_C}
		#endregion
		#region ILibAsyncSocket.h
		// -=S3P4R470R=- {ILibAsyncSocket_H}
		// -=S3P4R470R=- {ILibAsyncSocket_H}
		#endregion
		#region ILibAsyncServerSocket.c
		// -=S3P4R470R=- {ILibAsyncServerSocket_C}
		// -=S3P4R470R=- {ILibAsyncServerSocket_C}
		#endregion
		#region ILibAsyncServerSocket.h
		// -=S3P4R470R=- {ILibAsyncServerSocket_H}
		// -=S3P4R470R=- {ILibAsyncServerSocket_H}
		#endregion
		#region ILibWebClient.c
		// -=S3P4R470R=- {ILibWebClient_C}
		// -=S3P4R470R=- {ILibWebClient_C}
		#endregion
		#region ILibWebClient.h
		// -=S3P4R470R=- {ILibWebClient_H}
		// -=S3P4R470R=- {ILibWebClient_H}
		#endregion
		#region ILibWebServer.c
		// -=S3P4R470R=- {ILibWebServer_C}
		// -=S3P4R470R=- {ILibWebServer_C}
		#endregion
		#region ILibWebServer.h
		// -=S3P4R470R=- {ILibWebServer_H}
		// -=S3P4R470R=- {ILibWebServer_H}
		#endregion
		#region ILibSSDPClient.c
		// -=S3P4R470R=- {ILibSSDPClient_C}
		// -=S3P4R470R=- {ILibSSDPClient_C}
		#endregion
		#region ILibSSDPClient.h
		// -=S3P4R470R=- {ILibSSDPClient_H}
		// -=S3P4R470R=- {ILibSSDPClient_H}
		#endregion
		#region UPnPControlPointStructs.h
		// -=S3P4R470R=- {UPnPControlPointStructs_H}
		// -=S3P4R470R=- {UPnPControlPointStructs_H}
		#endregion


		public SourceCodeRepository()
		{
		}

		public static string GetMicroStack_C_Template(string PreFix)
		{
			return(GetEULA(PreFix+"MicroStack.c","") + RemoveOldEULA(UPnPMicroStack_C));
		}
		public static string GetMicroStack_H_Template(string PreFix)
		{
			return(GetEULA(PreFix+"MicroStack.h","") + RemoveOldEULA(UPnPMicroStack_H));
		}
		private static string GetEULA(string FileName, string Settings)
		{
			string RetVal = EULA;
			if(Settings=="") {Settings="*";}

			RetVal = RetVal.Replace("<REVISION>","#"+Application.ProductVersion);
			RetVal = RetVal.Replace("<DATE>",DateTime.Now.ToLongDateString());
			RetVal = RetVal.Replace("<FILE>",FileName);
			RetVal = RetVal.Replace("<SETTINGS>",Settings);

			return(RetVal);
		}
		private static string RemoveOldEULA(string InVal)
		{
			string RetVal = InVal;
			RetVal = RetVal.Substring(2+RetVal.IndexOf("*/"));
			return(RetVal);
		}

		public static void Generate_Parsers(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"Parsers.h","") + RemoveOldEULA(ILibParsers_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"Parsers.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"Parsers.c","") + RemoveOldEULA(ILibParsers_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"Parsers.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_AsyncSocket(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"AsyncSocket.h","") + RemoveOldEULA(ILibAsyncSocket_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncSocket.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"AsyncSocket.c","") + RemoveOldEULA(ILibAsyncSocket_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncSocket.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_AsyncServerSocket(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"AsyncServerSocket.h","") + RemoveOldEULA(ILibAsyncServerSocket_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncServerSocket.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"AsyncServerSocket.c","") + RemoveOldEULA(ILibAsyncServerSocket_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncServerSocket.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_WebClient(string PreFix, DirectoryInfo outputDir, bool Legacy)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"WebClient.h","") + RemoveOldEULA(ILibWebClient_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebClient.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"WebClient.c","") + RemoveOldEULA(ILibWebClient_C);
			lib = lib.Replace("ILib",PreFix);

			if(Legacy)
			{
				// Remove HTTP/1.1 Specific Code
				string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
				string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
				int ix = lib.IndexOf(xx);
				int iy;
				while(ix!=-1)
				{
					iy = lib.IndexOf(yy) + yy.Length;
					lib = lib.Remove(ix,iy-ix);
					ix = lib.IndexOf(xx);
				}
			}

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebClient.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_WebServer(string PreFix, DirectoryInfo outputDir, bool Legacy)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"WebServer.h","") + RemoveOldEULA(ILibWebServer_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebServer.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"WebServer.c","") + "\r\n";
			if(Legacy)
			{
				lib += "#define HTTPVERSION \"1.0\"";
			}
			else
			{
				lib += "#define HTTPVERSION \"1.1\"";
			}
			lib += RemoveOldEULA(ILibWebServer_C);
			lib = lib.Replace("ILib",PreFix);
			if(Legacy)
			{
				// Remove HTTP/1.1 Specific Code
				string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
				string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
				int ix = lib.IndexOf(xx);
				int iy;
				while(ix!=-1)
				{
					iy = lib.IndexOf(yy) + yy.Length;
					lib = lib.Remove(ix,iy-ix);
					ix = lib.IndexOf(xx);
				}
			}
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebServer.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static string RemoveAndClearTag(string BeginTag, string EndTag, string data)
		{
			// Remove HTTP/1.1 Specific Code
			int ix = data.IndexOf(BeginTag);
			int iy;
			while(ix!=-1)
			{
				iy = data.IndexOf(EndTag) + EndTag.Length;
				data = data.Remove(ix,iy-ix);
				ix = data.IndexOf(BeginTag);
			}
			return(data);
		}
		public static string RemoveTag(string BeginTag, string EndTag, string data)
		{
			data = data.Replace(BeginTag,"");
			data = data.Replace(EndTag,"");
			return(data);
		}
		public static void Generate_SSDPClient(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"SSDPClient.h","") + RemoveOldEULA(ILibSSDPClient_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"SSDPClient.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"SSDPClient.c","") + RemoveOldEULA(ILibSSDPClient_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"SSDPClient.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_UPnPControlPointStructs(string PreFix, string ReplaceText,DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA("UPnPControlPointStructs.h","") + RemoveOldEULA(UPnPControlPointStructs_H);
			lib = lib.Replace("ILib",PreFix);
			lib = lib.Replace("<REPLACE>",ReplaceText);

			writer = File.CreateText(outputDir.FullName + "\\UPnPControlPointStructs.h");
			writer.Write(lib);		
			writer.Close();
		}
	}
}
