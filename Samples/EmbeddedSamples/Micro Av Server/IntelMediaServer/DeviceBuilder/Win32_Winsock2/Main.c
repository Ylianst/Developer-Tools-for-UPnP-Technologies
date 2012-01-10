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
#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "ILibParsers.h"
#include "UpnpMicroStack.h"
#include "ILibWebServer.h"
#include <winsock2.h>

void *UpnpmicroStackChain;
void *UpnpmicroStack;

DWORD UpnpMonitorSocketReserved;
WSAOVERLAPPED UpnpMonitorSocketStateObject;
SOCKET UpnpMonitorSocket;

void UpnpConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	printf("Invoke: UpnpConnectionManager_GetCurrentConnectionInfo(%d);\r\n",ConnectionID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ConnectionManager_GetCurrentConnectionInfo(upnptoken,25000,25000,"Sample String","Sample String",25000,"Sample String","Sample String");
}

void UpnpConnectionManager_GetProtocolInfo(void* upnptoken)
{
	printf("Invoke: UpnpConnectionManager_GetProtocolInfo();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ConnectionManager_GetProtocolInfo(upnptoken,"Sample String","Sample String");
}

void UpnpConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	printf("Invoke: UpnpConnectionManager_GetCurrentConnectionIDs();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"Sample String");
}

void UpnpContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	printf("Invoke: UpnpContentDirectory_Browse(%s,%s,%s,%u,%u,%s);\r\n",ObjectID,BrowseFlag,Filter,StartingIndex,RequestedCount,SortCriteria);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	/* UpnpResponse_ContentDirectory_Browse(upnptoken,"Sample String",250,250,250); */
	
	/* Fragmented response system, action result is constructed and sent on-the-fly. */
	UpnpAsyncResponse_START(upnptoken, "Browse", "urn:schemas-upnp-org:service:ContentDirectory:1");
	UpnpAsyncResponse_OUT(upnptoken, "Result", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "NumberReturned", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "TotalMatches", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "UpdateID", "", 0, 1, 1);
	UpnpAsyncResponse_DONE(upnptoken, "Browse");
}

void UpnpContentDirectory_GetSortCapabilities(void* upnptoken)
{
	printf("Invoke: UpnpContentDirectory_GetSortCapabilities();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ContentDirectory_GetSortCapabilities(upnptoken,"Sample String");
}

void UpnpContentDirectory_GetSystemUpdateID(void* upnptoken)
{
	printf("Invoke: UpnpContentDirectory_GetSystemUpdateID();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ContentDirectory_GetSystemUpdateID(upnptoken,250);
}

void UpnpContentDirectory_Search(void* upnptoken,char* ContainerID,char* SearchCriteria,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	printf("Invoke: UpnpContentDirectory_Search(%s,%s,%s,%u,%u,%s);\r\n",ContainerID,SearchCriteria,Filter,StartingIndex,RequestedCount,SortCriteria);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	/* UpnpResponse_ContentDirectory_Search(upnptoken,"Sample String",250,250,250); */
	
	/* Fragmented response system, action result is constructed and sent on-the-fly. */
	UpnpAsyncResponse_START(upnptoken, "Search", "urn:schemas-upnp-org:service:ContentDirectory:1");
	UpnpAsyncResponse_OUT(upnptoken, "Result", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "NumberReturned", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "TotalMatches", "", 0, 1, 1);
	UpnpAsyncResponse_OUT(upnptoken, "UpdateID", "", 0, 1, 1);
	UpnpAsyncResponse_DONE(upnptoken, "Search");
}

void UpnpContentDirectory_GetSearchCapabilities(void* upnptoken)
{
	printf("Invoke: UpnpContentDirectory_GetSearchCapabilities();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_ContentDirectory_GetSearchCapabilities(upnptoken,"Sample String");
}

void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("Upnp Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UpnpGetLocalInterfaceToHost(upnptoken));
	
	ILibWebServer_Send_Raw(upnptoken, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n" , 38 , 1, 1);
}

DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(UpnpmicroStackChain);
	return 0;
}

void CALLBACK UpnpIPAddressMonitor(
IN DWORD dwError, 
IN DWORD cbTransferred, 
IN LPWSAOVERLAPPED lpOverlapped, 
IN DWORD dwFlags 
)
{
	UpnpIPAddressListChanged(UpnpmicroStack);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
}
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	UpnpmicroStackChain = ILibCreateChain();
	
	/* TODO: Each device must have a unique device identifier (UDN) */
	UpnpmicroStack = UpnpCreateMicroStack(UpnpmicroStackChain,"Intel's Micro Media Server","8a4a1919-1d14-4453-9d71-cc1b9933522f","0000001",1800,0);
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UpnpSetState_ConnectionManager_SourceProtocolInfo(UpnpmicroStack,"Sample String");
	UpnpSetState_ConnectionManager_SinkProtocolInfo(UpnpmicroStack,"Sample String");
	UpnpSetState_ConnectionManager_CurrentConnectionIDs(UpnpmicroStack,"Sample String");
	UpnpSetState_ContentDirectory_ContainerUpdateIDs(UpnpmicroStack,"Sample String");
	UpnpSetState_ContentDirectory_SystemUpdateID(UpnpmicroStack,250);
	
	printf("Intel MicroStack 1.0 - Intel's Micro Media Server\r\n\r\n");
	CreateThread(NULL,0,&Run,NULL,0,&ptid);
	
	UpnpMonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(UpnpMonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UpnpMonitorSocketReserved,&UpnpMonitorSocketStateObject,&UpnpIPAddressMonitor);
	
	ILibStartChain(UpnpmicroStackChain);
	
	closesocket(UpnpMonitorSocket);
	return 0;
}

