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

#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "ILibParsers.h"
#include "UPnPMicroStack.h"

void *UPnPmicroStackChain;
void *UPnPmicroStack;

void *UPnPMonitor;
int UPnPIPAddressLength;
int *UPnPIPAddressList;

void UPnPConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	printf("Invoke: UPnPConnectionManager_GetCurrentConnectionInfo(%d);\r\n",ConnectionID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetCurrentConnectionInfo(upnptoken,25000,25000,"Sample String","Sample String",25000,"Sample String","Sample String");
}

void UPnPConnectionManager_PrepareForConnection(void* upnptoken,char* RemoteProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction)
{
	printf("Invoke: UPnPConnectionManager_PrepareForConnection(%s,%s,%d,%s);\r\n",RemoteProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_PrepareForConnection(upnptoken,25000,25000,25000);
}

void UPnPConnectionManager_ConnectionComplete(void* upnptoken,int ConnectionID)
{
	printf("Invoke: UPnPConnectionManager_ConnectionComplete(%d);\r\n",ConnectionID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_ConnectionComplete(upnptoken);
}

void UPnPConnectionManager_GetProtocolInfo(void* upnptoken)
{
	printf("Invoke: UPnPConnectionManager_GetProtocolInfo();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetProtocolInfo(upnptoken,"Sample String","Sample String");
}

void UPnPConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	printf("Invoke: UPnPConnectionManager_GetCurrentConnectionIDs();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"Sample String");
}

void UPnPContentDirectory_Search(void* upnptoken,char* ContainerID,char* SearchCriteria,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	printf("Invoke: UPnPContentDirectory_Search(%s,%s,%s,%u,%u,%s);\r\n",ContainerID,SearchCriteria,Filter,StartingIndex,RequestedCount,SortCriteria);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_Search(upnptoken,"Sample String",250,250,250);
}

void UPnPContentDirectory_StopTransferResource(void* upnptoken,unsigned int TransferID)
{
	printf("Invoke: UPnPContentDirectory_StopTransferResource(%u);\r\n",TransferID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_StopTransferResource(upnptoken);
}

void UPnPContentDirectory_DestroyObject(void* upnptoken,char* ObjectID)
{
	printf("Invoke: UPnPContentDirectory_DestroyObject(%s);\r\n",ObjectID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_DestroyObject(upnptoken);
}

void UPnPContentDirectory_UpdateObject(void* upnptoken,char* ObjectID,char* CurrentTagValue,char* NewTagValue)
{
	printf("Invoke: UPnPContentDirectory_UpdateObject(%s,%s,%s);\r\n",ObjectID,CurrentTagValue,NewTagValue);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_UpdateObject(upnptoken);
}

void UPnPContentDirectory_ExportResource(void* upnptoken,char* SourceURI,char* DestinationURI)
{
	printf("Invoke: UPnPContentDirectory_ExportResource(%s,%s);\r\n",SourceURI,DestinationURI);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_ExportResource(upnptoken,250);
}

void UPnPContentDirectory_GetTransferProgress(void* upnptoken,unsigned int TransferID)
{
	printf("Invoke: UPnPContentDirectory_GetTransferProgress(%u);\r\n",TransferID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_GetTransferProgress(upnptoken,"Sample String","Sample String","Sample String");
}

void UPnPContentDirectory_GetSearchCapabilities(void* upnptoken)
{
	printf("Invoke: UPnPContentDirectory_GetSearchCapabilities();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_GetSearchCapabilities(upnptoken,"Sample String");
}

void UPnPContentDirectory_GetSystemUpdateID(void* upnptoken)
{
	printf("Invoke: UPnPContentDirectory_GetSystemUpdateID();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_GetSystemUpdateID(upnptoken,250);
}

void UPnPContentDirectory_CreateObject(void* upnptoken,char* ContainerID,char* Elements)
{
	printf("Invoke: UPnPContentDirectory_CreateObject(%s,%s);\r\n",ContainerID,Elements);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_CreateObject(upnptoken,"Sample String","Sample String");
}

void UPnPContentDirectory_GetSortCapabilities(void* upnptoken)
{
	printf("Invoke: UPnPContentDirectory_GetSortCapabilities();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_GetSortCapabilities(upnptoken,"Sample String");
}

void UPnPContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	printf("Invoke: UPnPContentDirectory_Browse(%s,%s,%s,%u,%u,%s);\r\n",ObjectID,BrowseFlag,Filter,StartingIndex,RequestedCount,SortCriteria);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	/* UPnPResponse_ContentDirectory_Browse(upnptoken,"Sample String",250,250,250); */
	
	/* Fragmented response system, action result is constructed and sent on-the-fly. */
	UPnPAsyncResponse_START(upnptoken, "Browse", "urn:schemas-upnp-org:service:ContentDirectory:1");
	UPnPAsyncResponse_OUT(upnptoken, "Result", "", 0, 1, 1);
	UPnPAsyncResponse_OUT(upnptoken, "NumberReturned", "", 0, 1, 1);
	UPnPAsyncResponse_OUT(upnptoken, "TotalMatches", "", 0, 1, 1);
	UPnPAsyncResponse_OUT(upnptoken, "UpdateID", "", 0, 1, 1);
	UPnPAsyncResponse_DONE(upnptoken, "Browse");
}

void UPnPContentDirectory_ImportResource(void* upnptoken,char* SourceURI,char* DestinationURI)
{
	printf("Invoke: UPnPContentDirectory_ImportResource(%s,%s);\r\n",SourceURI,DestinationURI);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_ImportResource(upnptoken,250);
}

void UPnPContentDirectory_CreateReference(void* upnptoken,char* ContainerID,char* ObjectID)
{
	printf("Invoke: UPnPContentDirectory_CreateReference(%s,%s);\r\n",ContainerID,ObjectID);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_CreateReference(upnptoken,"Sample String");
}

void UPnPContentDirectory_DeleteResource(void* upnptoken,char* ResourceURI)
{
	printf("Invoke: UPnPContentDirectory_DeleteResource(%s);\r\n",ResourceURI);
	
	/* TODO: Place Action Code Here... */
	
	/* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UPnPResponse_ContentDirectory_DeleteResource(upnptoken);
}

void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("UPnP Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UPnPGetLocalInterfaceToHost(upnptoken));
	
	UPnPPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}

void UPnPIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UPnPIPAddressLength || memcmp((void*)list,(void*)UPnPIPAddressList,sizeof(int)*length)!=0)
	{
		UPnPIPAddressListChanged(UPnPmicroStack);
		
		FREE(UPnPIPAddressList);
		UPnPIPAddressList = list;
		UPnPIPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UPnPMonitor,NULL,4,&UPnPIPAddressMonitor,NULL);
}
void BreakSink(int s)
{
	ILibStopChain(UPnPmicroStackChain);
}
int main(void)
{
	UPnPmicroStackChain = ILibCreateChain();
	
	/* TODO: Each device must have a unique device identifier (UDN) */
	UPnPmicroStack = UPnPCreateMicroStack(UPnPmicroStackChain,"Intel Media Server","35bb3970-f938-4391-b962-c52d566032ea","0000001",1800,0);
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UPnPSetState_ConnectionManager_SourceProtocolInfo(UPnPmicroStack,"Sample String");
	UPnPSetState_ConnectionManager_SinkProtocolInfo(UPnPmicroStack,"Sample String");
	UPnPSetState_ConnectionManager_CurrentConnectionIDs(UPnPmicroStack,"Sample String");
	UPnPSetState_ContentDirectory_TransferIDs(UPnPmicroStack,"Sample String");
	UPnPSetState_ContentDirectory_ContainerUpdateIDs(UPnPmicroStack,"Sample String");
	UPnPSetState_ContentDirectory_SystemUpdateID(UPnPmicroStack,250);
	
	printf("Intel MicroStack 1.0 - Intel Media Server\r\n\r\n");
	
	UPnPMonitor = ILibCreateLifeTime(UPnPmicroStackChain);
	UPnPIPAddressLength = ILibGetLocalIPAddressList(&UPnPIPAddressList);
	ILibLifeTime_Add(UPnPMonitor,NULL,4,&UPnPIPAddressMonitor,NULL);
	
	signal(SIGINT,BreakSink);
	ILibStartChain(UPnPmicroStackChain);
	
	return 0;
}

