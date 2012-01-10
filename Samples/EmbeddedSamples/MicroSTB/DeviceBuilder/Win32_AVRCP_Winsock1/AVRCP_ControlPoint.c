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
#include <winsock.h>
#include <wininet.h>
#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include "ILibParsers.h"
#include "ILibSSDPClient.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "AVRCP_ControlPoint.h"
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

static const char *UPNPCP_SOAP_Header = "POST %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1300\r\nSOAPACTION: \"%s#%s\"\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n";
static const char *UPNPCP_SOAP_BodyHead = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:";
static const char *UPNPCP_SOAP_BodyTail = "></s:Body></s:Envelope>";

void AVRCP_Renew(void *state);
void AVRCP_SSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp);

struct CustomUserData
{
	int Timeout;
	char* buffer;
};
struct AVRCP_CP
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void (*DiscoverSink)(struct UPnPDevice *device);
	void (*RemoveSink)(struct UPnPDevice *device);
	
	void (*EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* value);
	void (*EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* value);
	void (*EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* value);
	void (*EventCallback_RenderingControl_LastChange)(struct UPnPService* Service,char* value);
	void (*EventCallback_AVTransport_LastChange)(struct UPnPService* Service,char* value);
	
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
void (*AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* value);
void (*AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* value);
void (*AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* value);
void (*AVRCP_EventCallback_RenderingControl_LastChange)(struct UPnPService* Service,char* value);
void (*AVRCP_EventCallback_AVTransport_LastChange)(struct UPnPService* Service,char* value);

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

void AVRCP_DestructUPnPService(struct UPnPService *service)
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
		ILibLifeTime_Remove(((struct AVRCP_CP*)service->Parent->CP)->LifeTimeMonitor,service);
		ILibDeleteEntry(((struct AVRCP_CP*)service->Parent->CP)->SIDTable,service->SubscriptionID,(int)strlen(service->SubscriptionID));
		FREE(service->SubscriptionID);
		service->SubscriptionID = NULL;
	}
	
	FREE(service);
}
void AVRCP_DestructUPnPDevice(struct UPnPDevice *device)
{
	struct UPnPDevice *d1,*d2;
	struct UPnPService *s1,*s2;
	
	d1 = device->EmbeddedDevices;
	while(d1!=NULL)
	{
		d2 = d1->Next;
		AVRCP_DestructUPnPDevice(d1);
		d1 = d2;
	}
	
	s1 = device->Services;
	while(s1!=NULL)
	{
		s2 = s1->Next;
		AVRCP_DestructUPnPService(s1);
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

void AVRCP_AddRef(struct UPnPDevice *device)
{
	struct AVRCP_CP *CP = (struct AVRCP_CP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	++d->ReferenceCount;
	sem_post(&(CP->DeviceLock));
}
void AVRCP_Release(struct UPnPDevice *device)
{
	struct AVRCP_CP *CP = (struct AVRCP_CP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	--d->ReferenceCount;
	if(d->ReferenceCount==0)
	{
		AVRCP_DestructUPnPDevice(d);
	}
	sem_post(&(CP->DeviceLock));
}
void AVRCP_DeviceDescriptionInterruptSink(void *sender, void *user1, void *user2)
{
	struct CustomUserData *cd = (struct CustomUserData*)user1;
	FREE(cd->buffer);
	FREE(user1);
}
void AVRCP_Push(struct UPnP_Stack **pp_Top, void *data)
{
	struct UPnP_Stack *frame = (struct UPnP_Stack*)MALLOC(sizeof(struct UPnP_Stack));
	frame->data = data;
	frame->next = *pp_Top;
	*pp_Top = frame;
}
void *AVRCP_Pop(struct UPnP_Stack **pp_Top)
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
void *AVRCP_Peek(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if(frame!=NULL)
	{
		RetVal = (*pp_Top)->data;
	}
	return(RetVal);
}
void AVRCP_Flush(struct UPnP_Stack **pp_Top)
{
	while(AVRCP_Pop(pp_Top)!=NULL) {}
	*pp_Top = NULL;
}

void AVRCP_AttachRootUDNToUDN(void *v_CP,char* UDN, char* RootUDN)
{
	struct UDNMapNode *node;
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
void AVRCP_AttachDeviceToUDN(void *v_CP,char* UDN, struct UPnPDevice *device)
{
	struct UDNMapNode *node;
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
void AVRCP_RemoveUDN(void *v_CP,char* UDN)
{
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
	
	if(device!=NULL) {AVRCP_Release(device);}
}
void AVRCP_MarkUDN(void *v_CP,char *UDN)
{
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
void AVRCP_AddUDN(void *v_CP,char *UDN)
{
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
struct UPnPDevice* AVRCP_GetDeviceAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	struct UPnPDevice *RetVal = NULL;
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
char* AVRCP_GetRootUDNAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	char *RetVal = NULL;
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
int AVRCP_HasUDN(void *v_CP,char *UDN)
{
	struct UDNMapNode *node;
	struct AVRCP_CP* CP = (struct AVRCP_CP*)v_CP;
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
struct packetheader *AVRCP_BuildPacket(char* IP, int Port, char* Path, char* cmd)
{
	struct packetheader *RetVal = ILibCreateEmptyPacket();
	char* HostLine = (char*)MALLOC((int)strlen(IP)+7);
	int HostLineLength = sprintf(HostLine,"%s:%d",IP,Port);
	ILibSetVersion(RetVal,"1.1",3);
	ILibSetDirective(RetVal,cmd,(int)strlen(cmd),Path,(int)strlen(Path));
	ILibAddHeaderLine(RetVal,"Host",4,HostLine,HostLineLength);
	ILibAddHeaderLine(RetVal,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1300",44);
	FREE(HostLine);
	return(RetVal);
}

void AVRCP_RemoveServiceFromDevice(struct UPnPDevice *device, struct UPnPService *service)
{
	struct UPnPService *s = device->Services;
	
	if(s==service)
	{
		device->Services = s->Next;
		AVRCP_DestructUPnPService(service);
		return;
	}
	while(s->Next!=NULL)
	{
		if(s->Next == service)
		{
			s->Next = s->Next->Next;
			AVRCP_DestructUPnPService(service);
			return;
		}
		s = s->Next;
	}
}

void AVRCP_ProcessDevice(struct UPnPDevice *device)
{
	int OK = 0;
	struct UPnPService  *s,*s2;
	struct UPnPDevice *EmbeddedDevice = device->EmbeddedDevices;
	while(EmbeddedDevice!=NULL)
	{
		AVRCP_ProcessDevice(EmbeddedDevice);
		EmbeddedDevice = EmbeddedDevice->Next;
	}
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaRenderer:1",43)==0)
	{
		s = device->Services;
		while(s!=NULL)
		{
			OK = 0;
			if(strncmp(s->ServiceType,"urn:schemas-upnp-org:service:ConnectionManager:1",48)==0)
			{
				OK = 1;
			}
			if(strncmp(s->ServiceType,"urn:schemas-upnp-org:service:RenderingControl:1",47)==0)
			{
				OK = 1;
			}
			if(strncmp(s->ServiceType,"urn:schemas-upnp-org:service:AVTransport:1",42)==0)
			{
				OK = 1;
			}
			s2 = s->Next;
			if(OK==0) {AVRCP_RemoveServiceFromDevice(device,s);}
			s = s2;
		}
	}
	
}

void AVRCP_PrintUPnPDevice(int indents, struct UPnPDevice *device)
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
		AVRCP_PrintUPnPDevice(indents+5,d);
		d = d->Next;
	}
}
struct UPnPService *AVRCP_GetService(struct UPnPDevice *device, char* ServiceName, int length)
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
struct UPnPService *AVRCP_GetService_ConnectionManager(struct UPnPDevice *device)
{
	return(AVRCP_GetService(device,"urn:schemas-upnp-org:service:ConnectionManager:1",48));
}
struct UPnPService *AVRCP_GetService_RenderingControl(struct UPnPDevice *device)
{
	return(AVRCP_GetService(device,"urn:schemas-upnp-org:service:RenderingControl:1",47));
}
struct UPnPService *AVRCP_GetService_AVTransport(struct UPnPDevice *device)
{
	return(AVRCP_GetService(device,"urn:schemas-upnp-org:service:AVTransport:1",42));
}
struct UPnPDevice *AVRCP_GetDevice2(struct UPnPDevice *device, int index, int *c_Index)
{
	struct UPnPDevice *RetVal = NULL;
	struct UPnPDevice *e_Device = NULL;
	int currentIndex = *c_Index;
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaRenderer:1",43)==0)
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
		RetVal = AVRCP_GetDevice2(e_Device,index,&currentIndex);
		if(RetVal!=NULL)
		{
			break;
		}
		e_Device = e_Device->Next;
	}
	
	*c_Index = currentIndex;
	return(RetVal);
}
struct UPnPDevice* AVRCP_GetDevice1(struct UPnPDevice *device,int index)
{
	int c_Index = -1;
	return(AVRCP_GetDevice2(device,index,&c_Index));
}
int AVRCP_GetDeviceCount(struct UPnPDevice *device)
{
	int RetVal = 0;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	
	while(e_Device!=NULL)
	{
		RetVal += AVRCP_GetDeviceCount(e_Device);
		e_Device = e_Device->Next;
	}
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaRenderer:1",43)==0)
	{
		++RetVal;
	}
	return(RetVal);
}

int AVRCP_GetErrorCode(char *buffer, int length)
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
void AVRCP_ProcessSCPD(char* buffer, int length, struct UPnPService *service)
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
void AVRCP_DeviceExpired(struct UPnPDevice *device)
{
	LVL3DEBUG(printf("Device[%s] failed to re-advertise in a timely manner\r\n",device->FriendlyName);)
	while(device->Parent!=NULL) {device = device->Parent;}
	AVRCP_SSDP_Sink(NULL, device->UDN, 0, NULL, 0,device->CP);
}
void AVRCP_FinishProcessingDevice(struct AVRCP_CP* CP, struct UPnPDevice *RootDevice)
{
	char *RootUDN = RootDevice->UDN;
	int Timeout = RootDevice->CacheTime;
	struct UPnPDevice *RetDevice;
	int i=0;
	
	RootDevice->ReferenceCount = 1;
	AVRCP_AttachDeviceToUDN(CP,RootDevice->UDN,RootDevice);
	AVRCP_MarkUDN(CP,RootDevice->UDN);
	do
	{
		RetDevice = AVRCP_GetDevice1(RootDevice,i++);
		if(RetDevice!=NULL)
		{
			AVRCP_AddUDN(CP,RetDevice->UDN);
			AVRCP_AttachRootUDNToUDN(CP,RetDevice->UDN,RootUDN);
			AVRCP_AttachDeviceToUDN(CP,RetDevice->UDN,RetDevice);
			if(CP->DiscoverSink!=NULL)
			{
				CP->DiscoverSink(RetDevice);
			}
		}
	}while(RetDevice!=NULL);
	RetDevice = AVRCP_GetDeviceAtUDN(CP,RootUDN);
	if(RetDevice!=NULL)
	{
		ILibLifeTime_Add(CP->LifeTimeMonitor,RetDevice,Timeout,&AVRCP_DeviceExpired,NULL);
		AVRCP_Release(RetDevice);
	}
}
void AVRCP_SCPD_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *dv,
void *sv,
int *PAUSE)
{
	struct UPnPDevice *device;
	struct UPnPService *service = (struct UPnPService*)sv;
	struct AVRCP_CP *CP = service->Parent->CP;
	
	if(!(header==NULL || header->StatusCode!=200) && done!=0)
	{
		AVRCP_ProcessSCPD(buffer,EndPointer, service);
		
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
				AVRCP_FinishProcessingDevice(CP,device);
			}
			else
			{
				AVRCP_DestructUPnPDevice(device);
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
			if(device->SCPDLeft==0)
			{
				AVRCP_DestructUPnPDevice(device);
			}
		}
	}
}
void AVRCP_CalculateSCPD_FetchCount(struct UPnPDevice *device)
{
	int count = 0;
	struct UPnPDevice *root;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(e_Device!=NULL)
	{
		AVRCP_CalculateSCPD_FetchCount(e_Device);
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
void AVRCP_SCPD_Fetch(struct UPnPDevice *device)
{
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	char *IP,*Path;
	unsigned short Port;
	struct packetheader *p;
	struct sockaddr_in addr;
	
	while(e_Device!=NULL)
	{
		AVRCP_SCPD_Fetch(e_Device);
		e_Device = e_Device->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		ILibParseUri(s->SCPDURL,&IP,&Port,&Path);
		DEBUGSTATEMENT(printf("SCPD: %s Port: %d Path: %s\r\n",IP,Port,Path));
		p = AVRCP_BuildPacket(IP,Port,Path,"GET");
		
		memset((char *)&addr, 0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(IP);
		addr.sin_port = htons(Port);
		
		ILibWebClient_PipelineRequest(
		((struct AVRCP_CP*)device->CP)->HTTP,
		&addr,
		p,
		&AVRCP_SCPD_Sink,
		device,
		s);
		
		FREE(IP);
		FREE(Path);
		s = s->Next;
	}
}
struct UPnPDevice* AVRCP_ProcessDeviceXML_device(struct ILibXMLNode *xml, void *v_CP,const char *BaseURL, int Timeout, int RecvAddr)
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
						tempDevice = AVRCP_ProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout, RecvAddr);
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

void AVRCP_ProcessDeviceXML(void *v_CP,char* buffer, int BufferSize, char* LocationURL, int RecvAddr, int Timeout)
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
	
	RootDevice = AVRCP_ProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout,RecvAddr);
	FREE(BaseURL);
	ILibDestructXMLNodeList(rootXML);
	
	/* Add Root Device to UDNTable */
	AVRCP_AddUDN(v_CP,RootDevice->UDN);
	
	/* Save reference to LocationURL in the RootDevice */
	RootDevice->LocationURL = (char*)MALLOC(strlen(LocationURL)+1);
	sprintf(RootDevice->LocationURL,"%s",LocationURL);
	
	/* Trim Object Structure */
	AVRCP_ProcessDevice(RootDevice);
	RootDevice->SCPDLeft = 0;
	AVRCP_CalculateSCPD_FetchCount(RootDevice);
	if(RootDevice->SCPDLeft==0)
	{
		AVRCP_FinishProcessingDevice(v_CP,RootDevice);
	}
	else
	{
		AVRCP_SCPD_Fetch(RootDevice);
	}
}

void AVRCP_HTTP_Sink_DeviceDescription(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *user,
void *cp,
int *PAUSE)
{
	struct CustomUserData *customData = (struct CustomUserData*)user;
	if(header!=NULL && done!=0)
	{
		AVRCP_ProcessDeviceXML(cp,buffer,EndPointer-(*p_BeginPointer),customData->buffer,header->ReceivingAddress,customData->Timeout);
	}
	if(done!=0)
	{
		FREE(customData->buffer);
		FREE(user);
	}
}
void AVRCP__FlushRequest(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	struct UPnPService *s = device->Services;
	char *IP;
	unsigned short Port;
	char *Path;
	
	while(ed!=NULL)
	{
		AVRCP__FlushRequest(ed);
		ed = ed->Next;
	}
	while(s!=NULL)
	{
		s = s->Next;
	}
}
void AVRCP_SSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp)
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
		DEBUGSTATEMENT(printf("MediaRenderer Hello\r\n"));
		DEBUGSTATEMENT(printf("LocationURL: %s\r\n",LocationURL));
		if(AVRCP_HasUDN(cp,LocationURL)==0)
		{
			AVRCP_AddUDN(cp,LocationURL);
			if(AVRCP_HasUDN(cp,UDN)==0)
			{
				AVRCP_AddUDN(cp,UDN);
				ILibParseUri(LocationURL,&IP,&Port,&Path);
				DEBUGSTATEMENT(printf("IP: %s Port: %d Path: %s\r\n",IP,Port,Path));
				p = AVRCP_BuildPacket(IP,Port,Path,"GET");
				
				memset((char *)&addr, 0,sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = inet_addr(IP);
				addr.sin_port = htons(Port);
				
				buffer = (char*)MALLOC((int)strlen(LocationURL)+1);
				strcpy(buffer,LocationURL);
				
				customData = (struct CustomUserData*)MALLOC(sizeof(struct CustomUserData));
				customData->Timeout = Timeout;
				customData->buffer = buffer;
				
				ILibWebClient_PipelineRequest(
				((struct AVRCP_CP*)cp)->HTTP,
				&addr,
				p,
				&AVRCP_HTTP_Sink_DeviceDescription,
				customData, 
				cp);
				
				FREE(IP);
				FREE(Path);
			}
		}
		else
		{
			// Periodic Notify Packets
			if(AVRCP_HasUDN(cp,UDN)!=0)
			{
				buffer = AVRCP_GetRootUDNAtUDN(cp,UDN);
				if(buffer!=NULL)
				{
					device = AVRCP_GetDeviceAtUDN(cp,buffer);
					if(device!=NULL)
					{
						//Extend LifetimeMonitor duration
						ILibLifeTime_Remove(((struct AVRCP_CP*)cp)->LifeTimeMonitor,device);
						ILibLifeTime_Add(((struct AVRCP_CP*)cp)->LifeTimeMonitor,device,Timeout,&AVRCP_DeviceExpired,NULL);
						AVRCP_Release(device);
					}
				}
			}
		}
	}
	else
	{
		/* Bye Bye */
		DEBUGSTATEMENT(printf("MediaRenderer ByeBye\r\n"));
		device = AVRCP_GetDeviceAtUDN(cp,UDN);
		if(device!=NULL)
		{
			ILibLifeTime_Remove(((struct AVRCP_CP*)cp)->LifeTimeMonitor,device);
			AVRCP_RemoveUDN(cp,device->LocationURL);
			do
			{
				tempDevice = AVRCP_GetDevice1(device,i++);
				if(tempDevice!=NULL)
				{
					AVRCP__FlushRequest(tempDevice);
					if(((struct AVRCP_CP*)cp)->RemoveSink!=NULL)
					{
						((struct AVRCP_CP*)cp)->RemoveSink(tempDevice);
					}
				}
			} while(tempDevice!=NULL);
			while(device->Parent!=NULL) {device=device->Parent;}
			i = device->ReferenceTiedToEvents;
			while(i!=0)
			{
				AVRCP_Release(device);
				--i;
			}
			AVRCP_Release(device);
			AVRCP_RemoveUDN(cp,UDN);
		}
	}
}
void AVRCP_ConnectionManager_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
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
								if(AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo != NULL)
								{
									AVRCP_EventCallback_ConnectionManager_SourceProtocolInfo(service,SourceProtocolInfo);
								}
							}
							if(xml->NameLength==16 && memcmp(xml->Name,"SinkProtocolInfo",16) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								SinkProtocolInfo = tempString;
								if(AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo != NULL)
								{
									AVRCP_EventCallback_ConnectionManager_SinkProtocolInfo(service,SinkProtocolInfo);
								}
							}
							if(xml->NameLength==20 && memcmp(xml->Name,"CurrentConnectionIDs",20) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								CurrentConnectionIDs = tempString;
								if(AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs != NULL)
								{
									AVRCP_EventCallback_ConnectionManager_CurrentConnectionIDs(service,CurrentConnectionIDs);
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
void AVRCP_RenderingControl_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
{
	struct ILibXMLNode *xml,*rootXML;
	char *tempString;
	int tempStringLength;
	int flg,flg2;
	
	char* LastChange = 0;
	
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
							if(xml->NameLength==10 && memcmp(xml->Name,"LastChange",10) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								LastChange = tempString;
								if(AVRCP_EventCallback_RenderingControl_LastChange != NULL)
								{
									AVRCP_EventCallback_RenderingControl_LastChange(service,LastChange);
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
void AVRCP_AVTransport_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
{
	struct ILibXMLNode *xml,*rootXML;
	char *tempString;
	int tempStringLength;
	int flg,flg2;
	
	char* LastChange = 0;
	
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
							if(xml->NameLength==10 && memcmp(xml->Name,"LastChange",10) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								LastChange = tempString;
								if(AVRCP_EventCallback_AVTransport_LastChange != NULL)
								{
									AVRCP_EventCallback_AVTransport_LastChange(service,LastChange);
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
void AVRCP_OnEventSink(
struct ILibWebServer_Session *sender,
int InterruptFlag,
struct packetheader *header,
char *buffer,
int *BeginPointer,
int BufferSize,
int done)
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
		ILibAddHeaderLine(resp,"Server",6,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1300",44);
		ILibAddHeaderLine(resp,"Content-Length",14,"0",1);
		field = header->FirstField;
		while(field!=NULL)
		{
			if(field->FieldLength==3)
			{
				if(strncasecmp(field->Field,"SID",3)==0)
				{
					sid = (char*)MALLOC(field->FieldDataLength+1);
					sprintf(sid,"%s",field->FieldData);
					value = ILibGetEntry(((struct AVRCP_CP*)sender->User)->SIDTable,field->FieldData,field->FieldDataLength);
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
			if(type_length>47 && strncmp("urn:schemas-upnp-org:service:ConnectionManager:",service->ServiceType,47)==0)
			{
				AVRCP_ConnectionManager_EventSink(buffer, BufferSize, service);
			}
			else
			if(type_length>46 && strncmp("urn:schemas-upnp-org:service:RenderingControl:",service->ServiceType,46)==0)
			{
				AVRCP_RenderingControl_EventSink(buffer, BufferSize, service);
			}
			else
			if(type_length>41 && strncmp("urn:schemas-upnp-org:service:AVTransport:",service->ServiceType,41)==0)
			{
				AVRCP_AVTransport_EventSink(buffer, BufferSize, service);
			}
		}
		ILibWebServer_Send(sender,resp);
		if(sid!=NULL){FREE(sid);}
	}
}
void AVRCP_OnSubscribeSink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *BeginPointer,
int BufferSize,
int done,
void *user,
void *vcp,
int *PAUSE)
{
	struct UPnPService *s;
	struct UPnPDevice *d;
	struct packetheader_field_node *field;
	struct parser_result *p;
	struct AVRCP_CP *cp = (struct AVRCP_CP*)vcp;
	
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
						AVRCP_AddRef(s->Parent);
						d = s->Parent;
						while(d->Parent!=NULL) {d = d->Parent;}
						++d->ReferenceTiedToEvents;
						ILibLifeTime_Add(cp->LifeTimeMonitor,s,atoi(p->LastResult->data)/2,&AVRCP_Renew,NULL);
						ILibDestructParserResults(p);
					}
					field = field->NextField;
				}
			}
		}
		AVRCP_Release(s->Parent);
	}
}

void AVRCP_Renew(void *state)
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
	ILibAddHeaderLine(p,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1300",44);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	ILibWebClient_PipelineRequest(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&destaddr,
	p,
	&AVRCP_OnSubscribeSink,
	(void*)service, service->Parent->CP);
	
	while(d->Parent!=NULL) {d = d->Parent;}
	--d->ReferenceTiedToEvents;
	FREE(IP);
	FREE(Path);
}

struct UPnPDevice* AVRCP_GetDevice(struct UPnPDevice *device, char* DeviceType, int number)
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
int AVRCP_HasAction(struct UPnPService *s, char* action)
{
	struct UPnPAction *a = s->Actions;
	
	while(a!=NULL)
	{
		if(strcmp(action,a->Name)==0) return(-1);
		a = a->Next;
	}
	return(0);
}
void AVRCP_StopCP(void *v_CP)
{
	int i;
	struct UPnPDevice *RetDevice;
	struct UDNMapNode *mn,*mn2;
	struct AVRCP_CP *CP= (struct AVRCP_CP*)v_CP;
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
					RetDevice = AVRCP_GetDevice1(mn->device,i++);
					if(RetDevice!=NULL)
					{
						CP->RemoveSink(RetDevice);
					}
				}while(RetDevice!=NULL);
			}
			AVRCP_DestructUPnPDevice(mn->device);
		}
		if(mn->RootUDN!=NULL){FREE(mn->RootUDN);}
		FREE(mn->UDN);
		FREE(mn);
		mn = mn2;
	}
	ILibDestroyHashTree(CP->SIDTable);
	FREE(CP->AddressList);
}
void AVRCP__CP_IPAddressListChanged(void *CPToken)
{
	((struct AVRCP_CP*)CPToken)->RecheckFlag = 1;
	ILibForceUnBlockChain(((struct AVRCP_CP*)CPToken)->Chain);
}
void AVRCP_CP_ProcessDeviceRemoval(struct AVRCP_CP* CP, struct UPnPDevice *device)
{
	struct UPnPDevice *temp = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(temp!=NULL)
	{
		AVRCP_CP_ProcessDeviceRemoval(CP,temp);
		temp = temp->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		ILibLifeTime_Remove(CP->LifeTimeMonitor,s);
		s = s->Next;
	}
}

void AVRCP_CP_PreSelect(void *CPToken,fd_set *readset, fd_set *writeset, fd_set *errorset, int *blocktime)
{
	struct UDNMapNode *mn,*mn2;
	struct AVRCP_CP *CP= (struct AVRCP_CP*)CPToken;
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
					AVRCP_CP_ProcessDeviceRemoval(CP,mn->device);
					CP->RemoveSink(mn->device);
					AVRCP_DestructUPnPDevice(mn->device);
					
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
void AVRCP_OnSessionSink(struct ILibWebServer_Session *session, void *User)
{
	session->OnReceive = &AVRCP_OnEventSink;
	session->User = User;
}
void *AVRCP_CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*))
{
	struct AVRCP_CP *cp = (struct AVRCP_CP*)MALLOC(sizeof(struct AVRCP_CP));
	
	cp->Destroy = &AVRCP_StopCP;
	cp->PostSelect = NULL;
	cp->PreSelect = &AVRCP_CP_PreSelect;
	cp->DiscoverSink = A;
	cp->RemoveSink = R;
	
	sem_init(&(cp->DeviceLock),0,1);
	cp->UDN_Head = NULL;
	cp->WebServer = ILibWebServer_Create(Chain,5,0,&AVRCP_OnSessionSink,cp);
	cp->SIDTable = ILibInitHashTree();
	
	cp->SSDP = ILibCreateSSDPClientModule(Chain,"urn:schemas-upnp-org:device:MediaRenderer:1", 43, &AVRCP_SSDP_Sink,cp);
	cp->HTTP = ILibCreateWebClient(5,Chain);
	ILibAddToChain(Chain,cp);
	cp->LifeTimeMonitor = ILibCreateLifeTime(Chain);
	
	cp->Chain = Chain;
	cp->RecheckFlag = 0;
	cp->AddressListLength = ILibGetLocalIPAddressList(&(cp->AddressList));
	return((void*)cp);
}
void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionInfo_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
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
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
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
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*), void* user, int ConnectionID)
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
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(179 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_ConnectionManager_GetCurrentConnectionInfo_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_ConnectionManager_GetProtocolInfo_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
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
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
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
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*), void* user)
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
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(170 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetProtocolInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_ConnectionManager_GetProtocolInfo_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionIDs_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
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
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
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
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
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
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(178 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionIDs",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_ConnectionManager_GetCurrentConnectionIDs_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_RenderingControl_SetVolume_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_RenderingControl_SetVolume(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* Channel, unsigned short DesiredVolume)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Channel)+340);
	SoapBodyTemplate = "%sSetVolume xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Channel>%s</Channel><DesiredVolume>%u</DesiredVolume></u:SetVolume%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Channel, DesiredVolume,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(164 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"SetVolume",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_RenderingControl_SetVolume_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_RenderingControl_GetMute_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	int CurrentMute = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,int))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,int))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==15 && memcmp(xml->Name,"GetMuteResponse",15)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==11 && memcmp(xml->Name,"CurrentMute",11) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					CurrentMute = 1;
					if(strncasecmp(tempBuffer,"false",5)==0 || strncmp(tempBuffer,"0",1)==0 || strncasecmp(tempBuffer,"no",2)==0)
					{
						CurrentMute = 0;
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
		((void (*)(struct UPnPService*,int,void*,int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,CurrentMute);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_RenderingControl_GetMute(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,int), void* user, unsigned int InstanceID, char* Channel)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Channel)+300);
	SoapBodyTemplate = "%sGetMute xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Channel>%s</Channel></u:GetMute%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Channel,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(162 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetMute",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_RenderingControl_GetMute_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_RenderingControl_SetMute_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_RenderingControl_SetMute(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* Channel, int DesiredMute)
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
	char* __DesiredMute;
	
	if(service==NULL)
	{
		FREE(invoke_data);
		return;
	}
	(DesiredMute!=0)?(__DesiredMute="true"):(__DesiredMute="false");
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Channel)+332);
	SoapBodyTemplate = "%sSetMute xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Channel>%s</Channel><DesiredMute>%s</DesiredMute></u:SetMute%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Channel, __DesiredMute,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(162 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"SetMute",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_RenderingControl_SetMute_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_RenderingControl_GetVolume_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	unsigned short CurrentVolume = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned short))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned short))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==17 && memcmp(xml->Name,"GetVolumeResponse",17)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==13 && memcmp(xml->Name,"CurrentVolume",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						CurrentVolume = (unsigned short) TempULong;
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
		((void (*)(struct UPnPService*,int,void*,unsigned short))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,unsigned short))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,CurrentVolume);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_RenderingControl_GetVolume(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned short), void* user, unsigned int InstanceID, char* Channel)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Channel)+304);
	SoapBodyTemplate = "%sGetVolume xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Channel>%s</Channel></u:GetVolume%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Channel,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(164 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetVolume",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_RenderingControl_GetVolume_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetCurrentTransportActions_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* Actions = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==34 && memcmp(xml->Name,"GetCurrentTransportActionsResponse",34)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==7 && memcmp(xml->Name,"Actions",7) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						Actions = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Actions);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetCurrentTransportActions(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+319);
	SoapBodyTemplate = "%sGetCurrentTransportActions xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetCurrentTransportActions%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(181 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentTransportActions",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetCurrentTransportActions_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Play_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Play(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* Speed)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Speed)+290);
	SoapBodyTemplate = "%sPlay xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Speed>%s</Speed></u:Play%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Speed,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Play",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Play_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetDeviceCapabilities_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 3;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* PlayMedia = NULL;
	char* RecMedia = NULL;
	char* RecQualityModes = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==29 && memcmp(xml->Name,"GetDeviceCapabilitiesResponse",29)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==9 && memcmp(xml->Name,"PlayMedia",9) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						PlayMedia = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"RecMedia",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						RecMedia = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==15 && memcmp(xml->Name,"RecQualityModes",15) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						RecQualityModes = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,PlayMedia,RecMedia,RecQualityModes);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetDeviceCapabilities(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*,char*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+309);
	SoapBodyTemplate = "%sGetDeviceCapabilities xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetDeviceCapabilities%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(176 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetDeviceCapabilities",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetDeviceCapabilities_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetMediaInfo_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 9;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	unsigned int NrTracks = 0;
	char* MediaDuration = NULL;
	char* CurrentURI = NULL;
	char* CurrentURIMetaData = NULL;
	char* NextURI = NULL;
	char* NextURIMetaData = NULL;
	char* PlayMedium = NULL;
	char* RecordMedium = NULL;
	char* WriteStatus = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==20 && memcmp(xml->Name,"GetMediaInfoResponse",20)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==8 && memcmp(xml->Name,"NrTracks",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						NrTracks = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==13 && memcmp(xml->Name,"MediaDuration",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						MediaDuration = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==10 && memcmp(xml->Name,"CurrentURI",10) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						CurrentURI = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==18 && memcmp(xml->Name,"CurrentURIMetaData",18) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						CurrentURIMetaData = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==7 && memcmp(xml->Name,"NextURI",7) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						NextURI = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==15 && memcmp(xml->Name,"NextURIMetaData",15) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						NextURIMetaData = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==10 && memcmp(xml->Name,"PlayMedium",10) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						PlayMedium = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==12 && memcmp(xml->Name,"RecordMedium",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						RecordMedium = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==11 && memcmp(xml->Name,"WriteStatus",11) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						WriteStatus = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetMediaInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,char*,char*,char*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+291);
	SoapBodyTemplate = "%sGetMediaInfo xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetMediaInfo%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(167 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetMediaInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetMediaInfo_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Previous_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Previous(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+283);
	SoapBodyTemplate = "%sPrevious xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:Previous%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(163 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Previous",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Previous_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Next_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Next(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+275);
	SoapBodyTemplate = "%sNext xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:Next%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Next",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Next_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetTransportSettings_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 2;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* PlayMode = NULL;
	char* RecQualityMode = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==28 && memcmp(xml->Name,"GetTransportSettingsResponse",28)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==8 && memcmp(xml->Name,"PlayMode",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						PlayMode = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==14 && memcmp(xml->Name,"RecQualityMode",14) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						RecQualityMode = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,PlayMode,RecQualityMode);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetTransportSettings(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+307);
	SoapBodyTemplate = "%sGetTransportSettings xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetTransportSettings%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(175 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetTransportSettings",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetTransportSettings_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_SetAVTransportURI_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_SetAVTransportURI(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* CurrentURI, char* CurrentURIMetaData)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(CurrentURI)+(int)strlen(CurrentURIMetaData)+367);
	SoapBodyTemplate = "%sSetAVTransportURI xmlns:u=\"%s\"><InstanceID>%u</InstanceID><CurrentURI>%s</CurrentURI><CurrentURIMetaData>%s</CurrentURIMetaData></u:SetAVTransportURI%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, CurrentURI, CurrentURIMetaData,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(172 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"SetAVTransportURI",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_SetAVTransportURI_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Pause_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Pause(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+277);
	SoapBodyTemplate = "%sPause xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:Pause%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(160 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Pause",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Pause_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetPositionInfo_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 8;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	long TempLong;
	unsigned long TempULong;
	unsigned int Track = 0;
	char* TrackDuration = NULL;
	char* TrackMetaData = NULL;
	char* TrackURI = NULL;
	char* RelTime = NULL;
	char* AbsTime = NULL;
	int RelCount = 0;
	int AbsCount = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,int,int))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,int,int))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==23 && memcmp(xml->Name,"GetPositionInfoResponse",23)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==5 && memcmp(xml->Name,"Track",5) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						Track = (unsigned int) TempULong;
					}
				}
				else 
				if(xml->NameLength==13 && memcmp(xml->Name,"TrackDuration",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TrackDuration = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==13 && memcmp(xml->Name,"TrackMetaData",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TrackMetaData = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"TrackURI",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TrackURI = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==7 && memcmp(xml->Name,"RelTime",7) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						RelTime = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==7 && memcmp(xml->Name,"AbsTime",7) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						AbsTime = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"RelCount",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						RelCount = (int) TempLong;
					}
				}
				else 
				if(xml->NameLength==8 && memcmp(xml->Name,"AbsCount",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						AbsCount = (int) TempLong;
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
		((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,int,int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,int,int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetPositionInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int,char*,char*,char*,char*,char*,int,int), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+297);
	SoapBodyTemplate = "%sGetPositionInfo xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetPositionInfo%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(170 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetPositionInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetPositionInfo_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Seek_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Seek(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* Unit, char* Target)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(Unit)+(int)strlen(Target)+305);
	SoapBodyTemplate = "%sSeek xmlns:u=\"%s\"><InstanceID>%u</InstanceID><Unit>%s</Unit><Target>%s</Target></u:Seek%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, Unit, Target,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Seek",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Seek_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_GetTransportInfo_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 3;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* CurrentTransportState = NULL;
	char* CurrentTransportStatus = NULL;
	char* CurrentSpeed = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==24 && memcmp(xml->Name,"GetTransportInfoResponse",24)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==21 && memcmp(xml->Name,"CurrentTransportState",21) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						CurrentTransportState = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==22 && memcmp(xml->Name,"CurrentTransportStatus",22) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						CurrentTransportStatus = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==12 && memcmp(xml->Name,"CurrentSpeed",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						CurrentSpeed = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
	}
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_GetTransportInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*,char*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+299);
	SoapBodyTemplate = "%sGetTransportInfo xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:GetTransportInfo%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(171 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetTransportInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_GetTransportInfo_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_SetPlayMode_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_SetPlayMode(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID, char* NewPlayMode)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(NewPlayMode)+316);
	SoapBodyTemplate = "%sSetPlayMode xmlns:u=\"%s\"><InstanceID>%u</InstanceID><NewPlayMode>%s</NewPlayMode></u:SetPlayMode%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID, NewPlayMode,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(166 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"SetPlayMode",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_SetPlayMode_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_Invoke_AVTransport_Stop_Sink(
void *WebReaderToken,
int IsInterrupt,
struct packetheader *header,
char *buffer,
int *p_BeginPointer,
int EndPointer,
int done,
void *_service,
void *state,
int *PAUSE)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		AVRCP_Release(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,AVRCP_GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			AVRCP_Release(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	AVRCP_Release(Service->Parent);
	FREE(_InvokeData);
}
void AVRCP_Invoke_AVTransport_Stop(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int InstanceID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+275);
	SoapBodyTemplate = "%sStop xmlns:u=\"%s\"><InstanceID>%u</InstanceID></u:Stop%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, InstanceID,UPNPCP_SOAP_BodyTail);
	
	AVRCP_AddRef(service->Parent);
	ILibParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Stop",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibWebClient_PipelineRequestEx(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&addr,
	headerBuffer,
	headerLength,
	0,
	buffer,
	bufferLength,
	0,
	&AVRCP_Invoke_AVTransport_Stop_Sink,
	service,
	invoke_data);
	
	FREE(IP);
	FREE(Path);
}
void AVRCP_SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK))
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
	ILibAddHeaderLine(p,"User-Agent",10,"WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1300",44);
	
	callback = (char*)MALLOC(10+(int)strlen(service->Parent->InterfaceToHost)+6+(int)strlen(Path));
	sprintf(callback,"<http://%s:%d%s>",service->Parent->InterfaceToHost,ILibWebServer_GetPortNumber(((struct AVRCP_CP*)service->Parent->CP)->WebServer),Path);
	
	ILibAddHeaderLine(p,"CALLBACK",8,callback,(int)strlen(callback));
	FREE(callback);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	AVRCP_AddRef(service->Parent);
	ILibWebClient_PipelineRequest(
	((struct AVRCP_CP*)service->Parent->CP)->HTTP,
	&destaddr,
	p,
	&AVRCP_OnSubscribeSink,
	(void*)service,
	service->Parent->CP);
	
	FREE(IP);
	FREE(Path);
}
