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

#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#include <windows.h>
#include <math.h>
#include <winioctl.h>
#include <winbase.h>
#include <winerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <winsock.h>
#include <wininet.h>
#include <malloc.h>

#include "ILibSSDPClient.h"
#include "ILibParsers.h"

#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define DEBUGSTATEMENT(x)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
struct SSDPClientModule
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void (*FunctionCallback)(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout, void *user);
	char* DeviceURN;
	int DeviceURNLength;
	
	int *IPAddress;
	int NumIPAddress;
	
	SOCKET SSDPListenSocket;
	SOCKET MSEARCH_Response_Socket;
	int Terminate;
	void *Reserved;
};


void ILibReadSSDP(SOCKET ReadSocket, struct SSDPClientModule *module)
{
	int bytesRead = 0;
	char* buffer = (char*)malloc(4096);
	struct sockaddr_in addr;
	int addrlen = sizeof(struct sockaddr_in);
	struct packetheader *packet;
	struct packetheader_field_node *node;
	struct parser_result* pnode;
	
	char* Location = NULL;
	char* UDN = NULL;
	int Timeout = 0;
	int Alive = 0;
	int OK;
	int rt;
	
	bytesRead = recvfrom(ReadSocket, buffer, 4096, 0, (struct sockaddr *) &addr, &addrlen);
	if(bytesRead<=0) 
	{
		FREE(buffer);
		return;
	}
	packet = ILibParsePacketHeader(buffer,0,bytesRead);
	
	if(packet->Directive==NULL)
	{
		/* M-SEARCH Response */
		if(packet->StatusCode==200)
		{
			node = packet->FirstField;
			while(node!=NULL)
			{
				if(strncasecmp(node->Field,"LOCATION",8)==0)
				{
					Location = (char*)MALLOC(node->FieldDataLength+1);
					memcpy(Location,node->FieldData,node->FieldDataLength);
					Location[node->FieldDataLength] = '\0';
				}
				if(strncasecmp(node->Field,"CACHE-CONTROL",13)==0)
				{
					pnode = ILibParseString(node->FieldData, 0, node->FieldDataLength, "=", 1);
					pnode->LastResult->data[pnode->LastResult->datalength] = '\0';
					Timeout = atoi(pnode->LastResult->data);
					ILibDestructParserResults(pnode);
				}
				if(strncasecmp(node->Field,"USN",3)==0)
				{
					pnode = ILibParseString(node->FieldData, 0, node->FieldDataLength, "::", 2);
					pnode->FirstResult->data[pnode->FirstResult->datalength] = '\0';
					UDN = pnode->FirstResult->data+5;
					ILibDestructParserResults(pnode);
				}
				node = node->NextField;
			}
			if(module->FunctionCallback!=NULL)
			{
				module->FunctionCallback(module,UDN,-1,Location,Timeout,module->Reserved);
			}
			
		}
	}
	else
	{
		/* Notify Packet */
		if(strncasecmp(packet->Directive,"NOTIFY",6)==0)
		{
			OK = 0;
			rt = 0;
			node = packet->FirstField;
			while(node!=NULL)
			{
				node->Field[node->FieldLength] = '\0';
				if(strncasecmp(node->Field,"NT",2)==0 && node->FieldLength==2)
				{
					node->FieldData[node->FieldDataLength] = '\0';
					if(strncasecmp(node->FieldData,module->DeviceURN,module->DeviceURNLength)==0)
					{
						OK = -1;
					}
					else if(strncasecmp(node->FieldData,"upnp:rootdevice",15)==0)
					{
						rt = -1;
					}
					else
					{
						break;
					}
				}
				if(strncasecmp(node->Field,"NTS",3)==0)
				{
					if(strncasecmp(node->FieldData,"ssdp:alive",10)==0)
					{
						Alive = -1;
						rt = 0;
					}
					else
					{
						Alive = 0;
						OK = 0;
					}
				}
				if(strncasecmp(node->Field,"USN",3)==0)
				{
					pnode = ILibParseString(node->FieldData, 0, node->FieldDataLength, "::", 2);
					pnode->FirstResult->data[pnode->FirstResult->datalength] = '\0';
					UDN = pnode->FirstResult->data+5;
					ILibDestructParserResults(pnode);
				}
				if(strncasecmp(node->Field,"LOCATION",8)==0)
				{
					Location = (char*)MALLOC(node->FieldDataLength+1);
					memcpy(Location,node->FieldData,node->FieldDataLength);
					Location[node->FieldDataLength] = '\0';
				}
				if(strncasecmp(node->Field,"CACHE-CONTROL",13)==0)
				{
					pnode = ILibParseString(node->FieldData, 0, node->FieldDataLength, "=", 1);
					pnode->LastResult->data[pnode->LastResult->datalength] = '\0';
					Timeout = atoi(pnode->LastResult->data);
					ILibDestructParserResults(pnode);
				}
				node = node->NextField;
			}
			if(OK!=0 || rt!=0)
			{
				if(module->FunctionCallback!=NULL)
				{
					module->FunctionCallback(module,UDN,Alive,Location,Timeout,module->Reserved);
				}
			}
		}
	}
	if(Location!=NULL) {FREE(Location);}
	ILibDestructPacket(packet);
	FREE(buffer);
}

void ILibSSDPClientModule_PreSelect(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct SSDPClientModule *module = (struct SSDPClientModule*)object;
	FD_SET(module->SSDPListenSocket,readset);
	FD_SET(module->MSEARCH_Response_Socket, readset);
}
void ILibSSDPClientModule_PostSelect(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
{
	struct SSDPClientModule *module = (struct SSDPClientModule*)object;
	if(slct>0)
	{
		if(FD_ISSET(module->SSDPListenSocket,readset)!=0)
		{
			ILibReadSSDP(module->SSDPListenSocket,module);
		}
		if(FD_ISSET(module->MSEARCH_Response_Socket,readset)!=0)
		{
			ILibReadSSDP(module->MSEARCH_Response_Socket,module);
		}
	}
}

void ILibSSDPClientModule_Destroy(void *object)
{
	struct SSDPClientModule *s = (struct SSDPClientModule*)object;
	FREE(s->DeviceURN);
	if(s->IPAddress!=NULL)
	{
		FREE(s->IPAddress);
	}
}
void ILibSSDP_IPAddressListChanged(void *SSDPToken)
{
	struct SSDPClientModule *RetVal = (struct SSDPClientModule*)SSDPToken;
	int i;
	struct sockaddr_in dest_addr;
	
	struct ip_mreq mreq;
	char* buffer;
	int bufferlength;
	struct in_addr interface_addr;
	
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	dest_addr.sin_port = htons(UPNP_PORT);
	
	for(i=0;i<RetVal->NumIPAddress;++i)
	{
		mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
		mreq.imr_interface.s_addr = RetVal->IPAddress[i];
		if (setsockopt(RetVal->SSDPListenSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
		{
		}
		
	}
	
	buffer = (char*)MALLOC(105+RetVal->DeviceURNLength);
	bufferlength = sprintf(buffer,"M-SEARCH * HTTP/1.1\r\nMX: 3\r\nST: %s\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\n\r\n",RetVal->DeviceURN);
	
	FREE(RetVal->IPAddress);
	RetVal->NumIPAddress = ILibGetLocalIPAddressList(&(RetVal->IPAddress));
	
	for(i=0;i<RetVal->NumIPAddress;++i)
	{
		interface_addr.s_addr = RetVal->IPAddress[i];
		if (setsockopt(RetVal->MSEARCH_Response_Socket, IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			sendto(RetVal->MSEARCH_Response_Socket, buffer, bufferlength, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	
	FREE(buffer);
}
void* ILibCreateSSDPClientModule(void *chain, char* DeviceURN, int DeviceURNLength, void (*CallbackPtr)(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout, void *user),void *user)
{
	int i;
	struct sockaddr_in addr;
	struct sockaddr_in dest_addr;
	struct SSDPClientModule *RetVal = (struct SSDPClientModule*)MALLOC(sizeof(struct SSDPClientModule));
	int ra = 1;
	struct ip_mreq mreq;
	char* buffer;
	int bufferlength;
	char* _DeviceURN;
	struct in_addr interface_addr;
	unsigned char TTL = 4;
	
	memset((char *)&addr, 0, sizeof(addr));
	memset((char *)&interface_addr, 0, sizeof(interface_addr));
	memset((char *)&(addr), 0, sizeof(dest_addr));
	
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	dest_addr.sin_port = htons(UPNP_PORT);
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(UPNP_PORT);	
	
	RetVal->Destroy = &ILibSSDPClientModule_Destroy;
	RetVal->PreSelect = &ILibSSDPClientModule_PreSelect;
	RetVal->PostSelect = &ILibSSDPClientModule_PostSelect;
	
	RetVal->Reserved = user;
	RetVal->Terminate = 0;
	RetVal->FunctionCallback = CallbackPtr;
	RetVal->DeviceURN = (char*)MALLOC(DeviceURNLength+1);
	memcpy(RetVal->DeviceURN,DeviceURN,DeviceURNLength);
	RetVal->DeviceURN[DeviceURNLength] = '\0';
	RetVal->DeviceURNLength = DeviceURNLength;
	
	RetVal->NumIPAddress = ILibGetLocalIPAddressList(&(RetVal->IPAddress));
	RetVal->SSDPListenSocket = socket(AF_INET, SOCK_DGRAM, 0);
	ILibGetDGramSocket(htonl(INADDR_ANY), &(RetVal->MSEARCH_Response_Socket));
	
	if (setsockopt(RetVal->MSEARCH_Response_Socket, IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)
	{
		/* Ignore this case */
	}
	if (setsockopt(RetVal->SSDPListenSocket, SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) < 0)
	{
		DEBUGSTATEMENT(printf("Setting SockOpt SO_REUSEADDR failed\r\n"));
		exit(1);
	}
	if (bind(RetVal->SSDPListenSocket, (struct sockaddr *) &(addr), sizeof(addr)) < 0)
	{
		printf("SSDPListenSocket bind");
		exit(1);
	}
	
	for(i=0;i<RetVal->NumIPAddress;++i)
	{
		mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
		mreq.imr_interface.s_addr = RetVal->IPAddress[i];
		if (setsockopt(RetVal->SSDPListenSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
		{
			printf("SSDPListenSocket setsockopt mreq");
			exit(1);
		}
		
	}
	
	ILibAddToChain(chain,RetVal);
	_DeviceURN = (char*)MALLOC(DeviceURNLength+1);
	memcpy(_DeviceURN,DeviceURN,DeviceURNLength);
	_DeviceURN[DeviceURNLength] = '\0';
	buffer = (char*)MALLOC(105+DeviceURNLength);
	bufferlength = sprintf(buffer,"M-SEARCH * HTTP/1.1\r\nMX: 3\r\nST: %s\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\n\r\n",_DeviceURN);
	
	for(i=0;i<RetVal->NumIPAddress;++i)
	{
		interface_addr.s_addr = RetVal->IPAddress[i];
		if (setsockopt(RetVal->MSEARCH_Response_Socket, IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			sendto(RetVal->MSEARCH_Response_Socket, buffer, bufferlength, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	
	FREE(_DeviceURN);
	FREE(buffer);
	return(RetVal);
}
