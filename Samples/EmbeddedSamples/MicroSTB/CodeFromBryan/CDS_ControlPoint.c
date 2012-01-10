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
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include "ILibParsers.h"
#include "ILibSSDPClient.h"
#include "ILibHTTPClient.h"
#include "ILibMiniWebServer.h"
#include "CDS_ControlPoint.h"
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define INVALID_DATA 0
#define DEBUGSTATEMENT(x)
#define LVL3DEBUG(x)

static const char *UPNPCP_SOAP_Header = "POST %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1283\r\nSOAPACTION: \"%s#%s\"\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n";
static const char *UPNPCP_SOAP_BodyHead = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:";
static const char *UPNPCP_SOAP_BodyTail = "></s:Body></s:Envelope>";

void CDS_Renew(void *state);
void CDS_SSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp);

struct CustomUserData
{
	int Timeout;
	char* buffer;
};
struct CDS_CP
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void (*DiscoverSink)(struct UPnPDevice *device);
	void (*RemoveSink)(struct UPnPDevice *device);
	
	void (*EventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* value);
	void (*EventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int value);
	void (*EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* value);
	void (*EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* value);
	void (*EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* value);
	
	struct UDNMapNode *UDN_Head;
	struct LifeTimeMonitorStruct *LifeTimeMonitor;
	
	void *HTTP;
	void *SSDP;
	void *WebServer;
	
	sem_t DeviceLock;
	void* SIDTable;
	
	void *Chain;
	int RecheckFlag;
	int AddressListLength;
	int *AddressList;
};
void (*CDS_EventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* value);
void (*CDS_EventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int value);
void (*CDS_EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* value);
void (*CDS_EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* value);
void (*CDS_EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* value);

struct InvokeStruct
{
	struct UPnPService *Service;
	void *CallbackPtr;
	void *User;
};
struct UPnPServiceInfo
{
	char* serviceType;
	char* SCPDURL;
	char* controlURL;
	char* eventSubURL;
	char* serviceId;
	struct UPnPServiceInfo *Next;
};
struct UPnP_Stack
{
	void *data;
	struct UPnP_Stack *next;
};

struct UDNMapNode
{
	char* UDN;
	char* RootUDN;
	long TimeStamp;
	int MARKED;
	struct UPnPDevice *device;
	struct UDNMapNode *Next;
	struct UDNMapNode *Previous;
};

void CDS_DestructUPnPService(struct UPnPService *service)
{
	struct UPnPAction *a1,*a2;
	struct UPnPStateVariable *sv1,*sv2;
	int i;
	
	a1 = service->Actions;
	while(a1!=NULL)
	{
		a2 = a1->Next;
		FREE(a1->Name);
		FREE(a1);
		a1 = a2;
	}
	
	sv1 = service->Variables;
	while(sv1!=NULL)
	{
		sv2 = sv1->Next;
		FREE(sv1->Name);
		if(sv1->Min!=NULL) {FREE(sv1->Min);}
		if(sv1->Max!=NULL) {FREE(sv1->Max);}
		if(sv1->Step!=NULL) {FREE(sv1->Step);}
		for(i=0;i<sv1->NumAllowedValues;++i)
		{
			FREE(sv1->AllowedValues[i]);
		}
		if(sv1->AllowedValues!=NULL) {FREE(sv1->AllowedValues);}
		FREE(sv1);
		sv1 = sv2;
	}
	if(service->ControlURL!=NULL) {FREE(service->ControlURL);}
	if(service->SCPDURL!=NULL) {FREE(service->SCPDURL);}
	if(service->ServiceId!=NULL) {FREE(service->ServiceId);}
	if(service->ServiceType!=NULL) {FREE(service->ServiceType);}
	if(service->SubscriptionURL!=NULL) {FREE(service->SubscriptionURL);}
	if(service->SubscriptionID!=NULL)
	{
		ILibLifeTime_Remove(((struct CDS_CP*)service->Parent->CP)->LifeTimeMonitor,service);
		ILibDeleteEntry(((struct CDS_CP*)service->Parent->CP)->SIDTable,service->SubscriptionID,(int)strlen(service->SubscriptionID));
		FREE(service->SubscriptionID);
		service->SubscriptionID = NULL;
	}
	
	FREE(service);
}
void CDS_DestructUPnPDevice(struct UPnPDevice *device)
{
	struct UPnPDevice *d1,*d2;
	struct UPnPService *s1,*s2;
	
	d1 = device->EmbeddedDevices;
	while(d1!=NULL)
	{
		d2 = d1->Next;
		CDS_DestructUPnPDevice(d1);
		d1 = d2;
	}
	
	s1 = device->Services;
	while(s1!=NULL)
	{
		s2 = s1->Next;
		CDS_DestructUPnPService(s1);
		s1 = s2;
	}
	
	LVL3DEBUG(printf("\r\n\r\nDevice Destructed\r\n");)
	if(device->PresentationURL!=NULL) {FREE(device->PresentationURL);}
	if(device->ManufacturerName!=NULL) {FREE(device->ManufacturerName);}
	if(device->ManufacturerURL!=NULL) {FREE(device->ManufacturerURL);}
	if(device->ModelName!=NULL) {FREE(device->ModelName);}
	if(device->ModelNumber!=NULL) {FREE(device->ModelNumber);}
	if(device->ModelURL!=NULL) {FREE(device->ModelURL);}
	if(device->ModelDescription!=NULL) {FREE(device->ModelDescription);}
	if(device->DeviceType!=NULL) {FREE(device->DeviceType);}
	if(device->FriendlyName!=NULL) {FREE(device->FriendlyName);}
	if(device->LocationURL!=NULL) {FREE(device->LocationURL);}
	if(device->UDN!=NULL) {FREE(device->UDN);}
	if(device->InterfaceToHost!=NULL) {FREE(device->InterfaceToHost);}
	
	FREE(device);
}

void CDS_AddRef(struct UPnPDevice *device)
{
	struct CDS_CP *CP = (struct CDS_CP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	++d->ReferenceCount;
	sem_post(&(CP->DeviceLock));
}
void CDS_Release(struct UPnPDevice *device)
{
	struct CDS_CP *CP = (struct CDS_CP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	--d->ReferenceCount;
	if(d->ReferenceCount==0)
	{
		CDS_DestructUPnPDevice(d);
	}
	sem_post(&(CP->DeviceLock));
}
void CDS_DeviceDescriptionInterruptSink(void *sender, void *user1, void *user2)
{
	struct CustomUserData *cd = (struct CustomUserData*)user1;
	FREE(cd->buffer);
	FREE(user1);
}
void CDS_Push(struct UPnP_Stack **pp_Top, void *data)
{
	struct UPnP_Stack *frame = (struct UPnP_Stack*)MALLOC(sizeof(struct UPnP_Stack));
	frame->data = data;
	frame->next = *pp_Top;
	*pp_Top = frame;
}
void *CDS_Pop(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if(frame!=NULL)
	{
		*pp_Top = frame->next;
		RetVal = frame->data;
		FREE(frame);
	}
	return(RetVal);
}
void *CDS_Peek(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if(frame!=NULL)
	{
		RetVal = (*pp_Top)->data;
	}
	return(RetVal);
}
void CDS_Flush(struct UPnP_Stack **pp_Top)
{
	while(CDS_Pop(pp_Top)!=NULL) {}
	*pp_Top = NULL;
}

void CDS_AttachRootUDNToUDN(void *v_CP,char* UDN, char* RootUDN)
{
	struct UDNMapNode *node;
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			if(node->RootUDN!=NULL)
			{
				FREE(node->RootUDN);
			}
			node->RootUDN = MALLOC(1+strlen(RootUDN));
			sprintf(node->RootUDN,"%s",RootUDN);
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
}
void CDS_AttachDeviceToUDN(void *v_CP,char* UDN, struct UPnPDevice *device)
{
	struct UDNMapNode *node;
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			node->device = device;
			//if(device->Parent==NULL) {++device->ReferenceCount;}
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
}
void CDS_RemoveUDN(void *v_CP,char* UDN)
{
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	struct UPnPDevice *device = NULL;
	struct UDNMapNode *node,*prevNode;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	prevNode = NULL;
	while (node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			device = node->device;
			node->device = NULL;
			if(node->RootUDN!=NULL)
			{
				FREE(node->RootUDN);
				node->RootUDN = NULL;
			}
			if(prevNode!=NULL)
			{
				prevNode->Next = node->Next;
				if(node->Next!=NULL)
				{
					node->Next->Previous = prevNode;
				}
			}
			else
			{
				CP->UDN_Head = node->Next;
				if(node->Next!=NULL)
				{
					node->Next->Previous = NULL;
				}
			}
			FREE(node->UDN);
			FREE(node);
			break;
		}
		prevNode = node;
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	if(device!=NULL) {CDS_Release(device);}
}
void CDS_MarkUDN(void *v_CP,char *UDN)
{
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	struct UDNMapNode *node;
	int has = 0;
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			node->MARKED=1;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
}
void CDS_AddUDN(void *v_CP,char *UDN)
{
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	struct UDNMapNode *node;
	int has = 0;
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			has = -1;
			break;
		}
		node=node->Next;
	}
	if(has==0)
	{
		node = (struct UDNMapNode*)MALLOC(sizeof(struct UDNMapNode));
		node->MARKED=0;
		node->UDN = (char*)MALLOC((int)strlen(UDN)+1);
		memcpy(node->UDN,UDN,(int)strlen(UDN));
		node->UDN[(int)strlen(UDN)] = '\0';
		node->device = NULL;
		node->RootUDN = NULL;
		node->Next = CP->UDN_Head;
		CP->UDN_Head = node;
		CP->UDN_Head->Previous = NULL;
		if(CP->UDN_Head->Next!=NULL)
		{
			CP->UDN_Head->Next->Previous = CP->UDN_Head;
		}
	}
	sem_post(&(CP->DeviceLock));
}
struct UPnPDevice* CDS_GetDeviceAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	struct UPnPDevice *RetVal = NULL;
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	if(CP->UDN_Head==NULL) {return(NULL);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			RetVal = node->device;
			if(RetVal!=NULL)
			{
				while(RetVal->Parent!=NULL) {RetVal = RetVal->Parent;}
				++RetVal->ReferenceCount;
				RetVal = node->device;
			}
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
char* CDS_GetRootUDNAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	char *RetVal = NULL;
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	if(CP->UDN_Head==NULL) {return(NULL);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			RetVal = node->RootUDN;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
int CDS_HasUDN(void *v_CP,char *UDN)
{
	struct UDNMapNode *node;
	struct CDS_CP* CP = (struct CDS_CP*)v_CP;
	int RetVal = 0;
	if(CP->UDN_Head==NULL) {return(0);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strcmp(node->UDN,UDN)==0)
		{
			RetVal = -1;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
struct packetheader *CDS_BuildPacket(char* IP, int Port, char* Path, char* cmd)
{
	struct packetheader *RetVal = ILibCreateEmptyPacket();
	char* HostLine = (char*)MALLOC((int)strlen(IP)+7);
	int HostLineLength = sprintf(HostLine,"%s:%d",IP,Port);
	ILibSetDirective(RetVal,cmd,(int)strlen(cmd),Path,(int)strlen(Path));
	ILibAddHeaderLine(RetVal,"Host",4,HostLine,HostLineLength);
	ILibAddHeaderLine(RetVal,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1283",44);
	FREE(HostLine);
	return(RetVal);
}

void CDS_RemoveServiceFromDevice(struct UPnPDevice *device, struct UPnPService *service)
{
	struct UPnPService *s = device->Services;
	
	if(s==service)
	{
		device->Services = s->Next;
		CDS_DestructUPnPService(service);
		return;
	}
	while(s->Next!=NULL)
	{
		if(s->Next == service)
		{
			s->Next = s->Next->Next;
			CDS_DestructUPnPService(service);
			return;
		}
		s = s->Next;
	}
}

void CDS_ProcessDevice(struct UPnPDevice *device)
{
	int OK = 0;
	struct UPnPService  *s,*s2;
	struct UPnPDevice *EmbeddedDevice = device->EmbeddedDevices;
	while(EmbeddedDevice!=NULL)
	{
		CDS_ProcessDevice(EmbeddedDevice);
		EmbeddedDevice = EmbeddedDevice->Next;
	}
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaServer:1",41)==0)
	{
		s = device->Services;
		while(s!=NULL)
		{
			OK = 0;
			if(strncmp(s->ServiceType,"urn:schemas-upnp-org:service:ContentDirectory:1",47)==0)
			{
				OK = 1;
			}
			if(strncmp(s->ServiceType,"urn:schemas-upnp-org:service:ConnectionManager:1",48)==0)
			{
				OK = 1;
			}
			s2 = s->Next;
			if(OK==0) {CDS_RemoveServiceFromDevice(device,s);}
			s = s2;
		}
	}
	
}

void CDS_PrintUPnPDevice(int indents, struct UPnPDevice *device)
{
	struct UPnPService *s;
	struct UPnPDevice *d;
	struct UPnPAction *a;
	int x=0;
	
	for(x=0;x<indents;++x) {printf(" ");}
	printf("Device: %s\r\n",device->DeviceType);
	
	for(x=0;x<indents;++x) {printf(" ");}
	printf("Friendly: %s\r\n",device->FriendlyName);
	
	s = device->Services;
	while(s!=NULL)
	{
		for(x=0;x<indents;++x) {printf(" ");}
		printf("   Service: %s\r\n",s->ServiceType);
		a = s->Actions;
		while(a!=NULL)
		{
			for(x=0;x<indents;++x) {printf(" ");}
			printf("      Action: %s\r\n",a->Name);
			a = a->Next;
		}
		s = s->Next;
	}
	
	d = device->EmbeddedDevices;
	while(d!=NULL)
	{
		CDS_PrintUPnPDevice(indents+5,d);
		d = d->Next;
	}
}
struct UPnPService *CDS_GetService(struct UPnPDevice *device, char* ServiceName, int length)
{
	struct UPnPService *RetService = NULL;
	struct UPnPService *s = device->Services;
	while(s!=NULL)
	{
		if((int)strlen(s->ServiceType)==length)
		{
			if(strncmp(s->ServiceType,ServiceName,length)==0)
			{
				RetService = s;
				break;
			}
		}
		s = s->Next;
	}
	
	return(RetService);
}
struct UPnPService *CDS_GetService_ContentDirectory(struct UPnPDevice *device)
{
	return(CDS_GetService(device,"urn:schemas-upnp-org:service:ContentDirectory:1",47));
}
struct UPnPService *CDS_GetService_ConnectionManager(struct UPnPDevice *device)
{
	return(CDS_GetService(device,"urn:schemas-upnp-org:service:ConnectionManager:1",48));
}
struct UPnPDevice *CDS_GetDevice2(struct UPnPDevice *device, int index, int *c_Index)
{
	struct UPnPDevice *RetVal = NULL;
	struct UPnPDevice *e_Device = NULL;
	int currentIndex = *c_Index;
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaServer:1",41)==0)
	{
		++currentIndex;
		if(currentIndex==index)
		{
			*c_Index = currentIndex;
			return(device);
		}
	}
	
	e_Device = device->EmbeddedDevices;
	while(e_Device!=NULL)
	{
		RetVal = CDS_GetDevice2(e_Device,index,&currentIndex);
		if(RetVal!=NULL)
		{
			break;
		}
		e_Device = e_Device->Next;
	}
	
	*c_Index = currentIndex;
	return(RetVal);
}
struct UPnPDevice* CDS_GetDevice1(struct UPnPDevice *device,int index)
{
	int c_Index = -1;
	return(CDS_GetDevice2(device,index,&c_Index));
}
int CDS_GetDeviceCount(struct UPnPDevice *device)
{
	int RetVal = 0;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	
	while(e_Device!=NULL)
	{
		RetVal += CDS_GetDeviceCount(e_Device);
		e_Device = e_Device->Next;
	}
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaServer:1",41)==0)
	{
		++RetVal;
	}
	return(RetVal);
}

int CDS_GetErrorCode(char *buffer, int length)
{
	int RetVal = 500;
	struct ILibXMLNode *xml,*rootXML;
	
	char *temp;
	int tempLength;
	
	rootXML = xml = ILibParseXML(buffer,0,length);
	ILibProcessXMLNodeList(xml);
	
	while(xml!=NULL)
	{
		if(xml->NameLength==8 && memcmp(xml->Name,"Envelope",8)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==4 && memcmp(xml->Name,"Body",4)==0)
				{
					xml = xml->Next;
					while(xml!=NULL)
					{
						if(xml->NameLength==5 && memcmp(xml->Name,"Fault",5)==0)
						{
							xml = xml->Next;
							while(xml!=NULL)
							{
								if(xml->NameLength==6 && memcmp(xml->Name,"detail",6)==0)
								{
									xml = xml->Next;
									while(xml!=NULL)
									{
										if(xml->NameLength==9 && memcmp(xml->Name,"UPnPError",9)==0)
										{
											xml = xml->Next;
											while(xml!=NULL)
											{
												if(xml->NameLength==9 && memcmp(xml->Name,"errorCode",9)==0)
												{
													tempLength = ILibReadInnerXML(xml,&temp);
													temp[tempLength] = 0;
													RetVal =atoi(temp);
													xml = NULL;
												}
												if(xml!=NULL) {xml = xml->Peer;}
											}
										}
										if(xml!=NULL) {xml = xml->Peer;}
									}
								}
								if(xml!=NULL) {xml = xml->Peer;}
							}
						}
						if(xml!=NULL) {xml = xml->Peer;}
					}
				}
				if(xml!=NULL) {xml = xml->Peer;}
			}
		}
		if(xml!=NULL) {xml = xml->Peer;}
	}
	ILibDestructXMLNodeList(rootXML);
	return(RetVal);
}
void CDS_ProcessSCPD(char* buffer, int length, struct UPnPService *service)
{
	struct UPnPAction *action;
	struct UPnPStateVariable *sv = NULL;
	struct UPnPAllowedValue *av = NULL;
	struct UPnPAllowedValue *avs = NULL;
	
	struct ILibXMLNode *xml,*rootXML;
	int flg2,flg3,flg4;
	
	char* tempString;
	int tempStringLength;
	
	rootXML = xml = ILibParseXML(buffer,0,length);
	ILibProcessXMLNodeList(xml);
	
	while(xml!=NULL && strncmp(xml->Name,"!",1)==0)
	{
		xml = xml->Next;
	}
	xml = xml->Next;
	while(xml!=NULL)
	{
		if(xml->NameLength==10 && memcmp(xml->Name,"actionList",10)==0)
		{
			xml = xml->Next;
			flg2 = 0;
			while(flg2==0)
			{
				if(xml->NameLength==6 && memcmp(xml->Name,"action",6)==0)
				{
					action = (struct UPnPAction*)MALLOC(sizeof(struct UPnPAction));
					action->Name = NULL;
					action->Next = service->Actions;
					service->Actions = action;
					
					xml = xml->Next;
					flg3 = 0;
					while(flg3==0)
					{
						if(xml->NameLength==4 && memcmp(xml->Name,"name",4)==0)
						{
							tempStringLength = ILibReadInnerXML(xml,&tempString);
							action->Name = (char*)MALLOC(1+tempStringLength);
							memcpy(action->Name,tempString,tempStringLength);
							action->Name[tempStringLength] = '\0';
						}
						if(xml->Peer==NULL)
						{
							flg3 = -1;
							xml = xml->Parent;
						}
						else
						{
							xml = xml->Peer;
						}
					}
				}
				if(xml->Peer==NULL)
				{
					flg2 = -1;
					xml = xml->Parent;
				}
				else
				{
					xml = xml->Peer;
				}
			}
		}
		if(xml->NameLength==17 && memcmp(xml->Name,"serviceStateTable",17)==0)
		{
			if(xml->Next->StartTag!=0)
			{
				xml = xml->Next;
				flg2 = 0;
				while(flg2==0)
				{
					if(xml->NameLength==13 && memcmp(xml->Name,"stateVariable",13)==0)
					{
						sv = (struct UPnPStateVariable*)MALLOC(sizeof(struct UPnPStateVariable));
						sv->AllowedValues = NULL;
						sv->NumAllowedValues = 0;
						sv->Max = NULL;
						sv->Min = NULL;
						sv->Step = NULL;
						sv->Name = NULL;
						sv->Next = service->Variables;
						service->Variables = sv;
						sv->Parent = service;
						
						xml = xml->Next;
						flg3 = 0;
						while(flg3==0)
						{
							if(xml->NameLength==4 && memcmp(xml->Name,"name",4)==0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								sv->Name = (char*)MALLOC(1+tempStringLength);
								memcpy(sv->Name,tempString,tempStringLength);
								sv->Name[tempStringLength] = '\0';
							}
							if(xml->NameLength==16 && memcmp(xml->Name,"allowedValueList",16)==0)
							{
								if(xml->Next->StartTag!=0)
								{
									avs = NULL;
									xml = xml->Next;
									flg4 = 0;
									while(flg4==0)
									{
										if(xml->NameLength==12 && memcmp(xml->Name,"allowedValue",12)==0)
										{
											av = (struct UPnPAllowedValue*)MALLOC(sizeof(struct UPnPAllowedValue));
											av->Next = avs;
											avs = av;
											
											tempStringLength = ILibReadInnerXML(xml,&tempString);
											av->Value = (char*)MALLOC(1+tempStringLength);
											memcpy(av->Value,tempString,tempStringLength);
											av->Value[tempStringLength] = '\0';
										}
										if(xml->Peer!=NULL)
										{
											xml = xml->Peer;
										}
										else
										{
											xml = xml->Parent;
											flg4 = -1;
										}
									}
									av = avs;
									while(av!=NULL)
									{
										++sv->NumAllowedValues;
										av = av->Next;
									}
									av = avs;
									sv->AllowedValues = (char**)MALLOC(sv->NumAllowedValues*sizeof(char*));
									for(flg4=0;flg4<sv->NumAllowedValues;++flg4)
									{
										sv->AllowedValues[flg4] = av->Value;
										av = av->Next;
									}
									av = avs;
									while(av!=NULL)
									{
										avs = av->Next;
										FREE(av);
										av = avs;
									}
								}
							}
							if(xml->NameLength==17 && memcmp(xml->Name,"allowedValueRange",17)==0)
							{
								if(xml->Next->StartTag!=0)
								{
									xml = xml->Next;
									flg4 = 0;
									while(flg4==0)
									{
										if(xml->NameLength==7)
										{
											if(memcmp(xml->Name,"minimum",7)==0)
											{
												tempStringLength = ILibReadInnerXML(xml,&tempString);
												sv->Min = (char*)MALLOC(1+tempStringLength);
												memcpy(sv->Min,tempString,tempStringLength);
												sv->Min[tempStringLength] = '\0';
											}
											else if(memcmp(xml->Name,"maximum",7)==0)
											{
												tempStringLength = ILibReadInnerXML(xml,&tempString);
												sv->Max = (char*)MALLOC(1+tempStringLength);
												memcpy(sv->Max,tempString,tempStringLength);
												sv->Max[tempStringLength] = '\0';
											}
										}
										if(xml->NameLength==4 && memcmp(xml->Name,"step",4)==0)
										{
											tempStringLength = ILibReadInnerXML(xml,&tempString);
											sv->Step = (char*)MALLOC(1+tempStringLength);
											memcpy(sv->Step,tempString,tempStringLength);
											sv->Step[tempStringLength] = '\0';
										}
										if(xml->Peer!=NULL)
										{
											xml = xml->Peer;
										}
										else
										{
											xml = xml->Parent;
											flg4 = -1;
										}
									}
								}
							}
							if(xml->Peer!=NULL)
							{
								xml = xml->Peer;
							}
							else
							{
								flg3 = -1;
								xml = xml->Parent;
							}
						}
					}
					if(xml->Peer!=NULL)
					{
						xml = xml->Peer;
					}
					else
					{
						xml = xml->Parent;
						flg2 = -1;
					}
				}
			}
		}
		xml = xml->Peer;
	}
	
	ILibDestructXMLNodeList(rootXML);
}
void CDS_DeviceExpired(struct UPnPDevice *device)
{
	LVL3DEBUG(printf("Device[%s] failed to re-advertise in a timely manner\r\n",device->FriendlyName);)
	while(device->Parent!=NULL) {device = device->Parent;}
	CDS_SSDP_Sink(NULL, device->UDN, 0, NULL, 0,device->CP);
}
void CDS_FinishProcessingDevice(struct CDS_CP* CP, struct UPnPDevice *RootDevice)
{
	char *RootUDN = RootDevice->UDN;
	int Timeout = RootDevice->CacheTime;
	struct UPnPDevice *RetDevice;
	int i=0;
	
	RootDevice->ReferenceCount = 1;
	CDS_AttachDeviceToUDN(CP,RootDevice->UDN,RootDevice);
	CDS_MarkUDN(CP,RootDevice->UDN);
	do
	{
		RetDevice = CDS_GetDevice1(RootDevice,i++);
		if(RetDevice!=NULL)
		{
			CDS_AddUDN(CP,RetDevice->UDN);
			CDS_AttachRootUDNToUDN(CP,RetDevice->UDN,RootUDN);
			CDS_AttachDeviceToUDN(CP,RetDevice->UDN,RetDevice);
			if(CP->DiscoverSink!=NULL)
			{
				CP->DiscoverSink(RetDevice);
			}
		}
	}while(RetDevice!=NULL);
	RetDevice = CDS_GetDeviceAtUDN(CP,RootUDN);
	if(RetDevice!=NULL)
	{
		ILibLifeTime_Add(CP->LifeTimeMonitor,RetDevice,Timeout,&CDS_DeviceExpired,NULL);
		CDS_Release(RetDevice);
	}
}
void CDS_SCPD_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* dv, void* sv)
{
	struct UPnPDevice *device;
	struct UPnPService *service = (struct UPnPService*)sv;
	struct CDS_CP *CP = service->Parent->CP;
	
	if(!(header==NULL || header->StatusCode!=200) && done!=0)
	{
		CDS_ProcessSCPD(buffer,EndPointer, service);
		
		device = service->Parent;
		while(device->Parent!=NULL)
		{
			device = device->Parent;
		}
		--device->SCPDLeft;
		if(device->SCPDLeft==0)
		{
			if(device->SCPDError==0)
			{
				CDS_FinishProcessingDevice(CP,device);
			}
			else
			{
				CDS_DestructUPnPDevice(device);
			}
		}
	}
	else
	{
		if(done!=0 && (header==NULL || header->StatusCode!=200))
		{
			device = service->Parent;
			while(device->Parent!=NULL)
			{
				device = device->Parent;
			}
			--device->SCPDLeft;
			if(device->SCPDError==0)
			{
				device->SCPDError=1;
				if(IsInterrupt==0)
				{
					ILibDeleteRequests(CP->HTTP,dv);
				}
			}
			if(device->SCPDLeft==0)
			{
				CDS_DestructUPnPDevice(device);
			}
		}
	}
}
void CDS_CalculateSCPD_FetchCount(struct UPnPDevice *device)
{
	int count = 0;
	struct UPnPDevice *root;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(e_Device!=NULL)
	{
		CDS_CalculateSCPD_FetchCount(e_Device);
		e_Device = e_Device->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		++count;
		s = s->Next;
	}
	
	root = device;
	while(root->Parent!=NULL)
	{
		root = root->Parent;
	}
	root->SCPDLeft += count;
}
void CDS_SCPD_Fetch(struct UPnPDevice *device)
{
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	char *IP,*Path;
	unsigned short Port;
	struct packetheader *p;
	struct sockaddr_in addr;
	
	while(e_Device!=NULL)
	{
		CDS_SCPD_Fetch(e_Device);
		e_Device = e_Device->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		ILibParseUri(s->SCPDURL,&IP,&Port,&Path);
		DEBUGSTATEMENT(printf("SCPD: %s Port: %d Path: %s\r\n",IP,Port,Path));
		p = CDS_BuildPacket(IP,Port,Path,"GET");
		
		memset((char *)&addr, 0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(IP);
		addr.sin_port = htons(Port);
		
		ILibAddRequest(((struct CDS_CP*)device->CP)->HTTP, p,&addr, &CDS_SCPD_Sink, device, s);
		
		FREE(IP);
		FREE(Path);
		s = s->Next;
	}
}
struct UPnPDevice* CDS_ProcessDeviceXML_device(struct ILibXMLNode *xml, void *v_CP,const char *BaseURL, int Timeout, int RecvAddr)
{
	struct ILibXMLNode *tempNode;
	int flg,flg2;
	char *tempString;
	int tempStringLength;
	struct parser_result *tpr;
	
	char* ServiceType = NULL;
	int ServiceTypeLength = 0;
	char* SCPDURL = NULL;
	int SCPDURLLength = 0;
	char* EventSubURL = NULL;
	int EventSubURLLength = 0;
	char* ControlURL = NULL;
	int ControlURLLength = 0;
	
	struct UPnPDevice *tempDevice;
	struct UPnPService *TempService;
	struct UPnPDevice *device = (struct UPnPDevice*)MALLOC(sizeof(struct UPnPDevice));
	memset(device,0,sizeof(struct UPnPDevice));
	device->CP = v_CP;
	device->CacheTime = Timeout;
	device->InterfaceToHost = (char*)MALLOC(16);
	sprintf(device->InterfaceToHost,"%d.%d.%d.%d",(RecvAddr&0xFF),((RecvAddr>>8)&0xFF),((RecvAddr>>16)&0xFF),((RecvAddr>>24)&0xFF));
	
	xml = xml->Next;
	while(xml!=NULL)
	{
		if(xml->NameLength==10 && memcmp(xml->Name,"deviceList",10)==0)
		{
			if(xml->Next->StartTag!=0)
			{
				xml = xml->Next;
				flg2 = 0;
				while(flg2==0)
				{
					if(xml->NameLength==6 && memcmp(xml->Name,"device",6)==0)
					{
						tempDevice = CDS_ProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout, RecvAddr);
						tempDevice->Parent = device;
						tempDevice->Next = device->EmbeddedDevices;
						device->EmbeddedDevices = tempDevice;
					}
					if(xml->Peer==NULL)
					{
						flg2 = 1;
						xml = xml->Parent;
					}
					else
					{
						xml = xml->Peer;
					}
				}
			}
		} else
		if(xml->NameLength==3 && memcmp(xml->Name,"UDN",3)==0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			if(tempStringLength>5)
			{
				if(memcmp(tempString,"uuid:",5)==0)
				{
					tempString += 5;
					tempStringLength -= 5;
				}
				device->UDN = (char*)MALLOC(tempStringLength+1);
				memcpy(device->UDN,tempString,tempStringLength);
				device->UDN[tempStringLength] = '\0';
			}
		} else
		if(xml->NameLength==10 && memcmp(xml->Name,"deviceType",10) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			
			device->DeviceType = (char*)MALLOC(tempStringLength+1);
			memcpy(device->DeviceType,tempString,tempStringLength);
			device->DeviceType[tempStringLength] = '\0';
		} else
		if(xml->NameLength==12 && memcmp(xml->Name,"friendlyName",12) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->FriendlyName = (char*)MALLOC(1+tempStringLength);
			memcpy(device->FriendlyName,tempString,tempStringLength);
			device->FriendlyName[tempStringLength] = '\0';
		} else
		if(xml->NameLength==12 && memcmp(xml->Name,"manufacturer",12) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ManufacturerName = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ManufacturerName,tempString,tempStringLength);
			device->ManufacturerName[tempStringLength] = '\0';
		} else
		if(xml->NameLength==15 && memcmp(xml->Name,"manufacturerURL",15) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ManufacturerURL = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ManufacturerURL,tempString,tempStringLength);
			device->ManufacturerURL[tempStringLength] = '\0';
		} else
		if(xml->NameLength==16 && memcmp(xml->Name,"modelDescription",16) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ModelDescription = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ModelDescription,tempString,tempStringLength);
			device->ModelDescription[tempStringLength] = '\0';
		} else
		if(xml->NameLength==9 && memcmp(xml->Name,"modelName",9) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ModelName = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ModelName,tempString,tempStringLength);
			device->ModelName[tempStringLength] = '\0';
		} else
		if(xml->NameLength==11 && memcmp(xml->Name,"modelNumber",11) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ModelNumber = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ModelNumber,tempString,tempStringLength);
			device->ModelNumber[tempStringLength] = '\0';
		} else
		if(xml->NameLength==8 && memcmp(xml->Name,"modelURL",8) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			device->ModelURL = (char*)MALLOC(1+tempStringLength);
			memcpy(device->ModelURL,tempString,tempStringLength);
			device->ModelURL[tempStringLength] = '\0';
		} else
		if(xml->NameLength==15 && memcmp(xml->Name,"presentationURL",15) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			tempString[tempStringLength] = 0;
			tpr = ILibParseString(tempString,0,tempStringLength,"://",3);
			if(tpr->NumResults==1)
			{
				/* RelativeURL */
				if(tempString[0]=='/')
				{
					device->PresentationURL = (char*)MALLOC(1+strlen(BaseURL)+tempStringLength);
					memcpy(device->PresentationURL,BaseURL,strlen(BaseURL));
					strcpy(device->PresentationURL+strlen(BaseURL),tempString+1);
				}
				else
				{
					device->PresentationURL = (char*)MALLOC(2+strlen(BaseURL)+tempStringLength);
					memcpy(device->PresentationURL,BaseURL,strlen(BaseURL));
					strcpy(device->PresentationURL+strlen(BaseURL),tempString);
				}
			}
			else
			{
				/* AbsoluteURL */
				device->PresentationURL = (char*)MALLOC(1+tempStringLength);
				memcpy(device->PresentationURL,tempString,tempStringLength);
				device->PresentationURL[tempStringLength] = '\0';
			}
			ILibDestructParserResults(tpr);
		} else
		if(xml->NameLength==11 && memcmp(xml->Name,"serviceList",11)==0)
		{
			tempNode = xml;
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==7 && memcmp(xml->Name,"service",7)==0)
				{
					ServiceType = NULL;
					ServiceTypeLength = 0;
					SCPDURL = NULL;
					SCPDURLLength = 0;
					EventSubURL = NULL;
					EventSubURLLength = 0;
					ControlURL = NULL;
					ControlURLLength = 0;
					
					xml = xml->Next;
					flg = 0;
					while(flg==0)
					{
						if(xml->NameLength==11 && memcmp(xml->Name,"serviceType",11)==0)
						{
							ServiceTypeLength = ILibReadInnerXML(xml,&ServiceType);
						} else
						if(xml->NameLength==7 && memcmp(xml->Name,"SCPDURL",7) == 0)
						{
							SCPDURLLength = ILibReadInnerXML(xml,&SCPDURL);
						} else
						if(xml->NameLength==10 && memcmp(xml->Name,"controlURL",10) == 0)
						{
							ControlURLLength = ILibReadInnerXML(xml,&ControlURL);
						} else
						if(xml->NameLength==11 && memcmp(xml->Name,"eventSubURL",11) == 0)
						{
							EventSubURLLength = ILibReadInnerXML(xml,&EventSubURL);
						}
						
						if(xml->Peer!=NULL)
						{
							xml = xml->Peer;
						}
						else
						{
							flg = 1;
							xml = xml->Parent;
						}
					}
					
					/* Finished Parsing the ServiceSection, build the Service */
					ServiceType[ServiceTypeLength] = '\0';
					SCPDURL[SCPDURLLength] = '\0';
					EventSubURL[EventSubURLLength] = '\0';
					ControlURL[ControlURLLength] = '\0';
					
					TempService = (struct UPnPService*)MALLOC(sizeof(struct UPnPService));
					TempService->SubscriptionID = NULL;
					TempService->ServiceId = NULL;
					TempService->Actions = NULL;
					TempService->Variables = NULL;
					TempService->Next = NULL;
					TempService->Parent = device;
					if(EventSubURLLength>=7 && memcmp(EventSubURL,"http://",6)==0)
					{
						/* Explicit */
						TempService->SubscriptionURL = (char*)MALLOC(EventSubURLLength+1);
						memcpy(TempService->SubscriptionURL,EventSubURL,EventSubURLLength);
						TempService->SubscriptionURL[EventSubURLLength] = '\0';
					}
					else
					{
						/* Relative */
						if(memcmp(EventSubURL,"/",1)!=0)
						{
							TempService->SubscriptionURL = (char*)MALLOC(EventSubURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->SubscriptionURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->SubscriptionURL+(int)strlen(BaseURL),EventSubURL,EventSubURLLength);
							TempService->SubscriptionURL[EventSubURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							TempService->SubscriptionURL = (char*)MALLOC(EventSubURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->SubscriptionURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->SubscriptionURL+(int)strlen(BaseURL),EventSubURL+1,EventSubURLLength-1);
							TempService->SubscriptionURL[EventSubURLLength+(int)strlen(BaseURL)-1] = '\0';
						}
					}
					if(ControlURLLength>=7 && memcmp(ControlURL,"http://",6)==0)
					{
						/* Explicit */
						TempService->ControlURL = (char*)MALLOC(ControlURLLength+1);
						memcpy(TempService->ControlURL,ControlURL,ControlURLLength);
						TempService->ControlURL[ControlURLLength] = '\0';
					}
					else
					{
						/* Relative */
						if(memcmp(ControlURL,"/",1)!=0)
						{
							TempService->ControlURL = (char*)MALLOC(ControlURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->ControlURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->ControlURL+(int)strlen(BaseURL),ControlURL,ControlURLLength);
							TempService->ControlURL[ControlURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							TempService->ControlURL = (char*)MALLOC(ControlURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->ControlURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->ControlURL+(int)strlen(BaseURL),ControlURL+1,ControlURLLength-1);
							TempService->ControlURL[ControlURLLength+(int)strlen(BaseURL)-1] = '\0';
						}
					}
					if(SCPDURLLength>=7 && memcmp(SCPDURL,"http://",6)==0)
					{
						/* Explicit */
						TempService->SCPDURL = (char*)MALLOC(SCPDURLLength+1);
						memcpy(TempService->SCPDURL,SCPDURL,SCPDURLLength);
						TempService->SCPDURL[SCPDURLLength] = '\0';
					}
					else
					{
						/* Relative */
						if(memcmp(SCPDURL,"/",1)!=0)
						{
							TempService->SCPDURL = (char*)MALLOC(SCPDURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->SCPDURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->SCPDURL+(int)strlen(BaseURL),SCPDURL,SCPDURLLength);
							TempService->SCPDURL[SCPDURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							TempService->SCPDURL = (char*)MALLOC(SCPDURLLength+(int)strlen(BaseURL)+1);
							memcpy(TempService->SCPDURL,BaseURL,(int)strlen(BaseURL));
							memcpy(TempService->SCPDURL+(int)strlen(BaseURL),SCPDURL+1,SCPDURLLength-1);
							TempService->SCPDURL[SCPDURLLength+(int)strlen(BaseURL)-1] = '\0';
						}
					}
					
					TempService->ServiceType = (char*)MALLOC(ServiceTypeLength+1);
					sprintf(TempService->ServiceType,ServiceType,ServiceTypeLength);
					TempService->Next = device->Services;
					device->Services = TempService;
					
					DEBUGSTATEMENT(printf("ServiceType: %s\r\nSCPDURL: %s\r\nEventSubURL: %s\r\nControl URL: %s\r\n",ServiceType,SCPDURL,EventSubURL,ControlURL));
				}
				xml = xml->Peer;
			}
			xml = tempNode;
		} // End of ServiceList
		xml = xml->Peer;
	} // End of While
	
	return(device);
}

void CDS_ProcessDeviceXML(void *v_CP,char* buffer, int BufferSize, char* LocationURL, int RecvAddr, int Timeout)
{
	struct UPnPDevice *RootDevice = NULL;
	
	char* IP;
	unsigned short Port;
	char* Path;
	
	char* BaseURL = NULL;
	
	struct ILibXMLNode *rootXML;
	struct ILibXMLNode *xml;
	char* tempString;
	int tempStringLength;
	
	rootXML = ILibParseXML(buffer,0,BufferSize);
	ILibProcessXMLNodeList(rootXML);
	
	xml = rootXML;
	xml = xml->Next;
	while(xml!=NULL)
	{
		if(xml->NameLength==7 && memcmp(xml->Name,"URLBase",7)==0)
		{
			tempStringLength = ILibReadInnerXML(xml,&tempString);
			if(tempString[tempStringLength-1]!='/')
			{
				BaseURL = (char*)MALLOC(2+tempStringLength);
				memcpy(BaseURL,tempString,tempStringLength);
				BaseURL[tempStringLength] = '/';
				BaseURL[tempStringLength+1] = '\0';
			}
			else
			{
				BaseURL = (char*)MALLOC(1+tempStringLength);
				memcpy(BaseURL,tempString,tempStringLength);
				BaseURL[tempStringLength] = '\0';
			}
			break;
		}
		xml = xml->Peer;
	}
	
	if(BaseURL==NULL)
	{
		ILibParseUri(LocationURL,&IP,&Port,&Path);
		BaseURL = (char*)MALLOC(18+(int)strlen(IP));
		sprintf(BaseURL,"http://%s:%d/",IP,Port);
		
		FREE(IP);
		FREE(Path);
	}
	
	DEBUGSTATEMENT(printf("BaseURL: %s\r\n",BaseURL));
	
	xml = rootXML;
	xml = xml->Next;
	while(xml->NameLength!=6 && memcmp(xml->Name,"device",6)!=0 && xml!=NULL)
	{
		xml = xml->Peer;
	}
	if(xml==NULL)
	{
		/* Error */
		ILibDestructXMLNodeList(rootXML);
		return;
	}
	
	RootDevice = CDS_ProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout,RecvAddr);
	FREE(BaseURL);
	ILibDestructXMLNodeList(rootXML);
	
	/* Add Root Device to UDNTable */
	CDS_AddUDN(v_CP,RootDevice->UDN);
	
	/* Save reference to LocationURL in the RootDevice */
	RootDevice->LocationURL = (char*)MALLOC(strlen(LocationURL)+1);
	sprintf(RootDevice->LocationURL,"%s",LocationURL);
	
	/* Trim Object Structure */
	CDS_ProcessDevice(RootDevice);
	RootDevice->SCPDLeft = 0;
	CDS_CalculateSCPD_FetchCount(RootDevice);
	if(RootDevice->SCPDLeft==0)
	{
		CDS_FinishProcessingDevice(v_CP,RootDevice);
	}
	else
	{
		CDS_SCPD_Fetch(RootDevice);
	}
}

void CDS_HTTP_Sink_DeviceDescription(
void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* cp)
{
	struct CustomUserData *customData = (struct CustomUserData*)user;
	if(header!=NULL && done!=0)
	{
		CDS_ProcessDeviceXML(cp,buffer,EndPointer-(*p_BeginPointer),customData->buffer,header->ReceivingAddress,customData->Timeout);
	}
	if(done!=0)
	{
		FREE(customData->buffer);
		FREE(user);
	}
}
void CDS__FlushRequest(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	struct UPnPService *s = device->Services;
	char *IP;
	unsigned short Port;
	char *Path;
	
	while(ed!=NULL)
	{
		CDS__FlushRequest(ed);
		ed = ed->Next;
	}
	while(s!=NULL)
	{
		ILibParseUri(s->ControlURL,&IP,&Port,&Path);
		ILibDeleteRequests(((struct CDS_CP*)device->CP)->HTTP, s);
		FREE(IP);
		FREE(Path);
		s = s->Next;
	}
}
void CDS_SSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp)
{
	struct CustomUserData *customData;
	char* buffer;
	char* IP;
	unsigned short Port;
	char* Path;
	struct packetheader *p;
	struct sockaddr_in addr;
	
	struct UPnPDevice *device,*tempDevice;
	int i=0;
	
	if(Alive!=0)
	{
		if(Timeout==0) {return;}	// ToDo: Solve this the correct way
		/* Hello */
		DEBUGSTATEMENT(printf("MediaServer Hello\r\n"));
		DEBUGSTATEMENT(printf("LocationURL: %s\r\n",LocationURL));
		if(CDS_HasUDN(cp,LocationURL)==0)
		{
			CDS_AddUDN(cp,LocationURL);
			if(CDS_HasUDN(cp,UDN)==0)
			{
				CDS_AddUDN(cp,UDN);
				ILibParseUri(LocationURL,&IP,&Port,&Path);
				DEBUGSTATEMENT(printf("IP: %s Port: %d Path: %s\r\n",IP,Port,Path));
				p = CDS_BuildPacket(IP,Port,Path,"GET");
				
				memset((char *)&addr, 0,sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = inet_addr(IP);
				addr.sin_port = htons(Port);
				
				buffer = (char*)MALLOC((int)strlen(LocationURL)+1);
				strcpy(buffer,LocationURL);
				
				customData = (struct CustomUserData*)MALLOC(sizeof(struct CustomUserData));
				customData->Timeout = Timeout;
				customData->buffer = buffer;
				
				ILibAddRequest(((struct CDS_CP*)cp)->HTTP, p,&addr, &CDS_HTTP_Sink_DeviceDescription,customData, cp);
				
				FREE(IP);
				FREE(Path);
			}
		}
		else
		{
			// Periodic Notify Packets
			if(CDS_HasUDN(cp,UDN)!=0)
			{
				buffer = CDS_GetRootUDNAtUDN(cp,UDN);
				if(buffer!=NULL)
				{
					device = CDS_GetDeviceAtUDN(cp,buffer);
					if(device!=NULL)
					{
						//Extend LifetimeMonitor duration
						ILibLifeTime_Remove(((struct CDS_CP*)cp)->LifeTimeMonitor,device);
						ILibLifeTime_Add(((struct CDS_CP*)cp)->LifeTimeMonitor,device,Timeout,&CDS_DeviceExpired,NULL);
						CDS_Release(device);
					}
				}
			}
		}
	}
	else
	{
		/* Bye Bye */
		DEBUGSTATEMENT(printf("MediaServer ByeBye\r\n"));
		device = CDS_GetDeviceAtUDN(cp,UDN);
		if(device!=NULL)
		{
			ILibLifeTime_Remove(((struct CDS_CP*)cp)->LifeTimeMonitor,device);
			CDS_RemoveUDN(cp,device->LocationURL);
			do
			{
				tempDevice = CDS_GetDevice1(device,i++);
				if(tempDevice!=NULL)
				{
					CDS__FlushRequest(tempDevice);
					if(((struct CDS_CP*)cp)->RemoveSink!=NULL)
					{
						((struct CDS_CP*)cp)->RemoveSink(tempDevice);
					}
				}
			} while(tempDevice!=NULL);
			while(device->Parent!=NULL) {device=device->Parent;}
			i = device->ReferenceTiedToEvents;
			while(i!=0)
			{
				CDS_Release(device);
				--i;
			}
			CDS_Release(device);
			CDS_RemoveUDN(cp,UDN);
		}
	}
}
void CDS_ContentDirectory_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
{
	struct ILibXMLNode *xml,*rootXML;
	char *tempString;
	int tempStringLength;
	int flg,flg2;
	
	char* ContainerUpdateIDs = 0;
	unsigned int SystemUpdateID = 0;
	unsigned long TempULong;
	
	/* Parse SOAP */
	rootXML = xml = ILibParseXML(buffer,0,bufferlength);
	ILibProcessXMLNodeList(xml);
	
	while(xml!=NULL)
	{
		if(xml->NameLength==11 && memcmp(xml->Name,"propertyset",11)==0)
		{
			if(xml->Next->StartTag!=0)
			{
				flg = 0;
				xml = xml->Next;
				while(flg==0)
				{
					if(xml->NameLength==8 && memcmp(xml->Name,"property",8)==0)
					{
						xml = xml->Next;
						flg2 = 0;
						while(flg2==0)
						{
							if(xml->NameLength==18 && memcmp(xml->Name,"ContainerUpdateIDs",18) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								ContainerUpdateIDs = tempString;
								if(CDS_EventCallback_ContentDirectory_ContainerUpdateIDs != NULL)
								{
									CDS_EventCallback_ContentDirectory_ContainerUpdateIDs(service,ContainerUpdateIDs);
								}
							}
							if(xml->NameLength==14 && memcmp(xml->Name,"SystemUpdateID",14) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								if(ILibGetULong(tempString,tempStringLength,&TempULong)==0)
								{
									SystemUpdateID = (unsigned int) TempULong;
								}
								if(CDS_EventCallback_ContentDirectory_SystemUpdateID != NULL)
								{
									CDS_EventCallback_ContentDirectory_SystemUpdateID(service,SystemUpdateID);
								}
							}
							if(xml->Peer!=NULL)
							{
								xml = xml->Peer;
							}
							else
							{
								flg2 = -1;
								xml = xml->Parent;
							}
						}
					}
					if(xml->Peer!=NULL)
					{
						xml = xml->Peer;
					}
					else
					{
						flg = -1;
						xml = xml->Parent;
					}
				}
			}
		}
		xml = xml->Peer;
	}
	
	ILibDestructXMLNodeList(rootXML);
}
void CDS_ConnectionManager_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
{
	struct ILibXMLNode *xml,*rootXML;
	char *tempString;
	int tempStringLength;
	int flg,flg2;
	
	char* SourceProtocolInfo = 0;
	char* SinkProtocolInfo = 0;
	char* CurrentConnectionIDs = 0;
	
	/* Parse SOAP */
	rootXML = xml = ILibParseXML(buffer,0,bufferlength);
	ILibProcessXMLNodeList(xml);
	
	while(xml!=NULL)
	{
		if(xml->NameLength==11 && memcmp(xml->Name,"propertyset",11)==0)
		{
			if(xml->Next->StartTag!=0)
			{
				flg = 0;
				xml = xml->Next;
				while(flg==0)
				{
					if(xml->NameLength==8 && memcmp(xml->Name,"property",8)==0)
					{
						xml = xml->Next;
						flg2 = 0;
						while(flg2==0)
						{
							if(xml->NameLength==18 && memcmp(xml->Name,"SourceProtocolInfo",18) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								SourceProtocolInfo = tempString;
								if(CDS_EventCallback_ConnectionManager_SourceProtocolInfo != NULL)
								{
									CDS_EventCallback_ConnectionManager_SourceProtocolInfo(service,SourceProtocolInfo);
								}
							}
							if(xml->NameLength==16 && memcmp(xml->Name,"SinkProtocolInfo",16) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								SinkProtocolInfo = tempString;
								if(CDS_EventCallback_ConnectionManager_SinkProtocolInfo != NULL)
								{
									CDS_EventCallback_ConnectionManager_SinkProtocolInfo(service,SinkProtocolInfo);
								}
							}
							if(xml->NameLength==20 && memcmp(xml->Name,"CurrentConnectionIDs",20) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								CurrentConnectionIDs = tempString;
								if(CDS_EventCallback_ConnectionManager_CurrentConnectionIDs != NULL)
								{
									CDS_EventCallback_ConnectionManager_CurrentConnectionIDs(service,CurrentConnectionIDs);
								}
							}
							if(xml->Peer!=NULL)
							{
								xml = xml->Peer;
							}
							else
							{
								flg2 = -1;
								xml = xml->Parent;
							}
						}
					}
					if(xml->Peer!=NULL)
					{
						xml = xml->Peer;
					}
					else
					{
						flg = -1;
						xml = xml->Parent;
					}
				}
			}
		}
		xml = xml->Peer;
	}
	
	ILibDestructXMLNodeList(rootXML);
}
void CDS_OnEventSink(void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user)
{
	int type_length;
	char* sid = NULL;
	void* value = NULL;
	struct UPnPService *service = NULL;
	struct packetheader_field_node *field = NULL;
	struct packetheader *resp;
	if(done!=0)
	{
		resp = ILibCreateEmptyPacket();
		ILibAddHeaderLine(resp,"Server",6,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1283",44);
		field = header->FirstField;
		while(field!=NULL)
		{
			if(field->FieldLength==3)
			{
				if(strncasecmp(field->Field,"SID",3)==0)
				{
					sid = (char*)MALLOC(field->FieldDataLength+1);
					sprintf(sid,"%s",field->FieldData);
					value = ILibGetEntry(((struct CDS_CP*)user)->SIDTable,field->FieldData,field->FieldDataLength);
					break;
				}
			}
			field = field->NextField;
		}
		
		if(value==NULL)
		{
			/* Not a valid SID */
			ILibSetStatusCode(resp,412,"Failed",6);
		}
		else
		{
			ILibSetStatusCode(resp,200,"OK",2);
			service = (struct UPnPService*)value;
			
			type_length = (int)strlen(service->ServiceType);
			if(type_length>46 && strncmp("urn:schemas-upnp-org:service:ContentDirectory:",service->ServiceType,46)==0)
			{
				CDS_ContentDirectory_EventSink(buffer, BufferSize, service);
			}
			else
			if(type_length>47 && strncmp("urn:schemas-upnp-org:service:ConnectionManager:",service->ServiceType,47)==0)
			{
				CDS_ConnectionManager_EventSink(buffer, BufferSize, service);
			}
		}
		ILibMiniWebServerSend(ReaderObject,resp);
		ILibDestructPacket(resp);
		ILibMiniWebServerCloseSession(ReaderObject);
		if(sid!=NULL){FREE(sid);}
	}
}
void CDS_OnSubscribeSink(void *ReaderObject, struct packetheader *header, int IsInterrupt,char* buffer, int *BeginPointer, int BufferSize, int done, void* user, void *vcp)
{
	struct UPnPService *s;
	struct UPnPDevice *d;
	struct packetheader_field_node *field;
	struct parser_result *p;
	struct CDS_CP *cp = (struct CDS_CP*)vcp;
	
	if(done!=0)
	{
		s = (struct UPnPService*)user;
		if(header!=NULL)
		{
			if(header->StatusCode==200)
			{
				/* Successful */
				field = header->FirstField;
				while(field!=NULL)
				{
					if(field->FieldLength==3 && strncasecmp(field->Field,"SID",3)==0 && s->SubscriptionID==NULL)
					{
						s->SubscriptionID = (char*)MALLOC(1+field->FieldDataLength);
						strcpy(s->SubscriptionID,field->FieldData);
						ILibAddEntry(cp->SIDTable,field->FieldData,field->FieldDataLength,s);
					} else
					if(field->FieldLength==7 && strncasecmp(field->Field,"TIMEOUT",7)==0)
					{
						p = ILibParseString(field->FieldData,0,field->FieldDataLength,"-",1);
						p->LastResult->data[p->LastResult->datalength] = '\0';
						CDS_AddRef(s->Parent);
						d = s->Parent;
						while(d->Parent!=NULL) {d = d->Parent;}
						++d->ReferenceTiedToEvents;
						ILibLifeTime_Add(cp->LifeTimeMonitor,s,atoi(p->LastResult->data)/2,&CDS_Renew,NULL);
						ILibDestructParserResults(p);
					}
					field = field->NextField;
				}
			}
		}
		CDS_Release(s->Parent);
	}
}

void CDS_Renew(void *state)
{
	struct UPnPService *service = (struct UPnPService*)state;
	struct UPnPDevice *d = service->Parent;
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	struct sockaddr_in destaddr;
	
	ILibParseUri(service->SubscriptionURL,&IP,&Port,&Path);
	p = ILibCreateEmptyPacket();
	
	ILibSetDirective(p,"SUBSCRIBE",9,Path,(int)strlen(Path));
	
	TempString = (char*)MALLOC((int)strlen(IP)+7);
	sprintf(TempString,"%s:%d",IP,Port);
	
	ILibAddHeaderLine(p,"HOST",4,TempString,(int)strlen(TempString));
	FREE(TempString);
	
	ILibAddHeaderLine(p,"SID",3,service->SubscriptionID,(int)strlen(service->SubscriptionID));
	ILibAddHeaderLine(p,"TIMEOUT",7,"Second-180",10);
	ILibAddHeaderLine(p,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1283",44);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	ILibAddRequest(((struct CDS_CP*)service->Parent->CP)->HTTP, p,&destaddr, &CDS_OnSubscribeSink,(void*)service, service->Parent->CP);
	
	while(d->Parent!=NULL) {d = d->Parent;}
	--d->ReferenceTiedToEvents;
	FREE(IP);
	FREE(Path);
}

struct UPnPDevice* CDS_GetDevice(struct UPnPDevice *device, char* DeviceType, int number)
{
	int counter = 0;
	
	device = device->EmbeddedDevices;
	while(device != NULL)
	{
		if(strlen(device->DeviceType)>=strlen(DeviceType))
		{
			if(memcmp(device->DeviceType,DeviceType,strlen(DeviceType))==0)
			{
				if(number == (++counter)) return(device);
			}
		}
		device = device->Next;
	}
	return(NULL);
}
int CDS_HasAction(struct UPnPService *s, char* action)
{
	struct UPnPAction *a = s->Actions;
	
	while(a!=NULL)
	{
		if(strcmp(action,a->Name)==0) return(-1);
		a = a->Next;
	}
	return(0);
}
void CDS_StopCP(void *v_CP)
{
	int i;
	struct UPnPDevice *RetDevice;
	struct UDNMapNode *mn,*mn2;
	struct CDS_CP *CP= (struct CDS_CP*)v_CP;
	sem_destroy(&(CP->DeviceLock));
	
	mn = CP->UDN_Head;
	while(mn!=NULL)
	{
		mn2 = mn->Next;
		if(mn->device!=NULL && mn->MARKED!=0)
		{
			i = 0;
			if(CP->RemoveSink!=NULL)
			{
				do
				{
					RetDevice = CDS_GetDevice1(mn->device,i++);
					if(RetDevice!=NULL)
					{
						CP->RemoveSink(RetDevice);
					}
				}while(RetDevice!=NULL);
			}
			CDS_DestructUPnPDevice(mn->device);
		}
		if(mn->RootUDN!=NULL){FREE(mn->RootUDN);}
		FREE(mn->UDN);
		FREE(mn);
		mn = mn2;
	}
	ILibDestroyHashTree(CP->SIDTable);
	FREE(CP->AddressList);
}
void CDS__CP_IPAddressListChanged(void *CPToken)
{
	((struct CDS_CP*)CPToken)->RecheckFlag = 1;
	ILibForceUnBlockChain(((struct CDS_CP*)CPToken)->Chain);
}
void CDS_CP_ProcessDeviceRemoval(struct CDS_CP* CP, struct UPnPDevice *device)
{
	struct UPnPDevice *temp = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(temp!=NULL)
	{
		CDS_CP_ProcessDeviceRemoval(CP,temp);
		temp = temp->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		ILibLifeTime_Remove(CP->LifeTimeMonitor,s);
		s = s->Next;
	}
}

void CDS_CP_PreSelect(void *CPToken,fd_set *readset, fd_set *writeset, fd_set *errorset, int *blocktime)
{
	struct UDNMapNode *mn,*mn2;
	struct CDS_CP *CP= (struct CDS_CP*)CPToken;
	int *IPAddressList;
	int NumAddressList;
	int i;
	int found;
	
	if(CP->RecheckFlag!=0)
	{
		CP->RecheckFlag = 0;
		
		NumAddressList = ILibGetLocalIPAddressList(&IPAddressList);
		
		mn = CP->UDN_Head;
		while(mn!=NULL)
		{
			mn2 = mn->Next;
			if(mn->device!=NULL)
			{
				found = 0;
				for(i=0;i<NumAddressList;++i)
				{
					if(IPAddressList[i]==inet_addr(mn->device->InterfaceToHost))
					{
						found = 1;
						break;
					}
				}
				if(found==0)
				{
					// Clear LifeTime for services contained
					CDS_CP_ProcessDeviceRemoval(CP,mn->device);
					CP->RemoveSink(mn->device);
					CDS_DestructUPnPDevice(mn->device);
					
					if(mn->Previous==NULL)
					{
						// This is the head
						CP->UDN_Head = mn->Next;
						if(CP->UDN_Head!=NULL)
						{
							CP->UDN_Head->Previous = NULL;
						}
					}
					else
					{
						mn->Previous->Next = mn->Next;
						if(mn->Next!=NULL)
						{
							mn->Next->Previous = mn->Previous;
						}
					}
					
					FREE(mn->UDN);
					FREE(mn);
				}
			}
			mn = mn2;
		}
		
		ILibSSDP_IPAddressListChanged(CP->SSDP);
		FREE(CP->AddressList);
		CP->AddressListLength = NumAddressList;
		CP->AddressList = IPAddressList;
	}
}
void *CDS_CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*))
{
	struct CDS_CP *cp = (struct CDS_CP*)MALLOC(sizeof(struct CDS_CP));
	
	cp->Destroy = &CDS_StopCP;
	cp->PostSelect = NULL;
	cp->PreSelect = &CDS_CP_PreSelect;
	cp->DiscoverSink = A;
	cp->RemoveSink = R;
	
	sem_init(&(cp->DeviceLock),0,1);
	cp->UDN_Head = NULL;
	cp->WebServer = ILibCreateMiniWebServer(Chain,5,&CDS_OnEventSink,cp);
	cp->SIDTable = ILibInitHashTree();
	
	cp->SSDP = ILibCreateSSDPClientModule(Chain,"urn:schemas-upnp-org:device:MediaServer:1", 41, &CDS_SSDP_Sink,cp);
	cp->HTTP = ILibCreateHTTPClientModule(Chain,5);
	ILibAddToChain(Chain,cp);
	cp->LifeTimeMonitor = ILibCreateLifeTime(Chain);
	
	cp->Chain = Chain;
	cp->RecheckFlag = 0;
	cp->AddressListLength = ILibGetLocalIPAddressList(&(cp->AddressList));
	return((void*)cp);
}
void CDS_Invoke_ContentDirectory_Browse_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 4;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	char* Result = NULL;
	unsigned int NumberReturned = 0;
	unsigned int TotalMatches = 0;
	unsigned int UpdateID = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==14 && memcmp(xml->Name,"BrowseResponse",14)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==6 && memcmp(xml->Name,"Result",6) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Result = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==14 && memcmp(xml->Name,"NumberReturned",14) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						NumberReturned = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==12 && memcmp(xml->Name,"TotalMatches",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						TotalMatches = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"UpdateID",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						UpdateID = (unsigned int) TempULong;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Result,NumberReturned,TotalMatches,UpdateID);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ContentDirectory_Browse(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int), void* user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ObjectID)+(int)strlen(BrowseFlag)+(int)strlen(Filter)+(int)strlen(SortCriteria)+420);
	SoapBodyTemplate = "%sBrowse xmlns:u=\"%s\"><ObjectID>%s</ObjectID><BrowseFlag>%s</BrowseFlag><Filter>%s</Filter><StartingIndex>%u</StartingIndex><RequestedCount>%u</RequestedCount><SortCriteria>%s</SortCriteria></u:Browse%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(161 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Browse",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ContentDirectory_Browse_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ContentDirectory_GetSortCapabilities_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* SortCaps = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==27 && memcmp(xml->Name,"GetSortCapabilitiesResponse",27)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==8 && memcmp(xml->Name,"SortCaps",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						SortCaps = tempBuffer;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,SortCaps);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+270);
	SoapBodyTemplate = "%sGetSortCapabilities xmlns:u=\"%s\"></u:GetSortCapabilities%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(174 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSortCapabilities",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ContentDirectory_GetSortCapabilities_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ContentDirectory_GetSystemUpdateID_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	unsigned int Id = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==25 && memcmp(xml->Name,"GetSystemUpdateIDResponse",25)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==2 && memcmp(xml->Name,"Id",2) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						Id = (unsigned int) TempULong;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Id);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int), void* user)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+266);
	SoapBodyTemplate = "%sGetSystemUpdateID xmlns:u=\"%s\"></u:GetSystemUpdateID%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(172 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSystemUpdateID",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ContentDirectory_GetSystemUpdateID_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ContentDirectory_Search_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 4;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	char* Result = NULL;
	unsigned int NumberReturned = 0;
	unsigned int TotalMatches = 0;
	unsigned int UpdateID = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==14 && memcmp(xml->Name,"SearchResponse",14)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==6 && memcmp(xml->Name,"Result",6) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Result = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==14 && memcmp(xml->Name,"NumberReturned",14) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						NumberReturned = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==12 && memcmp(xml->Name,"TotalMatches",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						TotalMatches = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"UpdateID",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						UpdateID = (unsigned int) TempULong;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Result,NumberReturned,TotalMatches,UpdateID);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ContentDirectory_Search(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int), void* user, char* ContainerID, char* SearchCriteria, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ContainerID)+(int)strlen(SearchCriteria)+(int)strlen(Filter)+(int)strlen(SortCriteria)+434);
	SoapBodyTemplate = "%sSearch xmlns:u=\"%s\"><ContainerID>%s</ContainerID><SearchCriteria>%s</SearchCriteria><Filter>%s</Filter><StartingIndex>%u</StartingIndex><RequestedCount>%u</RequestedCount><SortCriteria>%s</SortCriteria></u:Search%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ContainerID, SearchCriteria, Filter, StartingIndex, RequestedCount, SortCriteria,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(161 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Search",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ContentDirectory_Search_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ContentDirectory_GetSearchCapabilities_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* SearchCaps = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==29 && memcmp(xml->Name,"GetSearchCapabilitiesResponse",29)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==10 && memcmp(xml->Name,"SearchCaps",10) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						SearchCaps = tempBuffer;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,SearchCaps);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+274);
	SoapBodyTemplate = "%sGetSearchCapabilities xmlns:u=\"%s\"></u:GetSearchCapabilities%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(176 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSearchCapabilities",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ContentDirectory_GetSearchCapabilities_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ConnectionManager_GetCurrentConnectionInfo_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 7;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	long TempLong;
	int RcsID = 0;
	int AVTransportID = 0;
	char* ProtocolInfo = NULL;
	char* PeerConnectionManager = NULL;
	int PeerConnectionID = 0;
	char* Direction = NULL;
	char* Status = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==32 && memcmp(xml->Name,"GetCurrentConnectionInfoResponse",32)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==5 && memcmp(xml->Name,"RcsID",5) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						RcsID = (int) TempLong;
					}
				}
				else 
				if(xml->NameLength==13 && memcmp(xml->Name,"AVTransportID",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						AVTransportID = (int) TempLong;
					}
				}
				else 
				if(xml->NameLength==12 && memcmp(xml->Name,"ProtocolInfo",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						ProtocolInfo = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==21 && memcmp(xml->Name,"PeerConnectionManager",21) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						PeerConnectionManager = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==16 && memcmp(xml->Name,"PeerConnectionID",16) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						PeerConnectionID = (int) TempLong;
					}
				}
				else 
				if(xml->NameLength==9 && memcmp(xml->Name,"Direction",9) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Direction = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==6 && memcmp(xml->Name,"Status",6) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Status = tempBuffer;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*), void* user, int ConnectionID)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+320);
	SoapBodyTemplate = "%sGetCurrentConnectionInfo xmlns:u=\"%s\"><ConnectionID>%d</ConnectionID></u:GetCurrentConnectionInfo%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ConnectionID,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(179 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ConnectionManager_GetCurrentConnectionInfo_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ConnectionManager_GetProtocolInfo_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 2;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* Source = NULL;
	char* Sink = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==23 && memcmp(xml->Name,"GetProtocolInfoResponse",23)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==6 && memcmp(xml->Name,"Source",6) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Source = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==4 && memcmp(xml->Name,"Sink",4) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Sink = tempBuffer;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Source,Sink);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*), void* user)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+262);
	SoapBodyTemplate = "%sGetProtocolInfo xmlns:u=\"%s\"></u:GetProtocolInfo%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(170 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetProtocolInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ConnectionManager_GetProtocolInfo_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_Invoke_ConnectionManager_GetCurrentConnectionIDs_Sink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* ConnectionIDs = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		CDS_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,CDS_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			CDS_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==31 && memcmp(xml->Name,"GetCurrentConnectionIDsResponse",31)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==13 && memcmp(xml->Name,"ConnectionIDs",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						ConnectionIDs = tempBuffer;
					}
				}
				xml = xml->Peer;
			}
		}
		if(xml!=NULL) {xml = xml->Next;}
	}
	ILibDestructXMLNodeList(__xml);
	
	if(ArgLeft!=0)
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,ConnectionIDs);
	}
	CDS_Release(Service->Parent);
	FREE(_InvokeData);
}
void CDS_Invoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
{
	int headerLength;
	char *headerBuffer;
	char *SoapBodyTemplate;
	char* buffer;
	int bufferLength;
	char* IP;
	unsigned short Port;
	char* Path;
	struct sockaddr_in addr;
	struct InvokeStruct *invoke_data = (struct InvokeStruct*)MALLOC(sizeof(struct InvokeStruct));
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+278);
	SoapBodyTemplate = "%sGetCurrentConnectionIDs xmlns:u=\"%s\"></u:GetCurrentConnectionIDs%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType,UPNPCP_SOAP_BodyTail);
	
	CDS_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(178 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionIDs",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct CDS_CP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &CDS_Invoke_ConnectionManager_GetCurrentConnectionIDs_Sink,service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void CDS_SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK))
{
	char* callback;
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	struct sockaddr_in destaddr;
	
	ILibParseUri(service->SubscriptionURL,&IP,&Port,&Path);
	p = ILibCreateEmptyPacket();
	
	ILibSetDirective(p,"SUBSCRIBE",9,Path,(int)strlen(Path));
	
	TempString = (char*)MALLOC((int)strlen(IP)+7);
	sprintf(TempString,"%s:%d",IP,Port);
	
	ILibAddHeaderLine(p,"HOST",4,TempString,(int)strlen(TempString));
	FREE(TempString);
	
	ILibAddHeaderLine(p,"NT",2,"upnp:event",10);
	ILibAddHeaderLine(p,"TIMEOUT",7,"Second-180",10);
	ILibAddHeaderLine(p,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1283",44);
	
	callback = (char*)MALLOC(10+(int)strlen(service->Parent->InterfaceToHost)+6+(int)strlen(Path));
	sprintf(callback,"<http://%s:%d%s>",service->Parent->InterfaceToHost,ILibGetMiniWebServerPortNumber(((struct CDS_CP*)service->Parent->CP)->WebServer),Path);
	
	ILibAddHeaderLine(p,"CALLBACK",8,callback,(int)strlen(callback));
	FREE(callback);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	CDS_AddRef(service->Parent);
	ILibAddRequest(((struct CDS_CP*)service->Parent->CP)->HTTP, p,&destaddr, &CDS_OnSubscribeSink,(void*)service,service->Parent->CP);
	
	FREE(IP);
	FREE(Path);
}
