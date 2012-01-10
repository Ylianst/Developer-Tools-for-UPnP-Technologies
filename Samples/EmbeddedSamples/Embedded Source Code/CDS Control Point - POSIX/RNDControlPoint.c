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


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "ILibParsers.h"
#include "ILibHTTPClient.h"
#include "ILibSSDPClient.h"
#include "ILibMiniWebServer.h"
#include "RNDControlPoint.h"
#include <semaphore.h>
#define INVALID_DATA 0
#define DEBUGSTATEMENT(x)
#define LVL3DEBUG(x)

const char *UPNPCP_SOAP_Header = "POST %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\nSOAPACTION: \"%s#%s\"\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n";
const char *UPNPCP_SOAP_BodyHead = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:";
const char *UPNPCP_SOAP_BodyTail = "></s:Body></s:Envelope>";

void RNDRenew(void *state);
void RNDSSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp);

struct CustomUserData
{
	int Timeout;
	char* buffer;
};
struct RNDCP
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void (*DiscoverSink)(struct UPnPDevice *device);
	void (*RemoveSink)(struct UPnPDevice *device);
	
	void (*EventCallback_ContentDirectory_TransferIDs)(struct UPnPService* Service,char* value);
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
void (*RNDEventCallback_ContentDirectory_TransferIDs)(struct UPnPService* Service,char* value);
void (*RNDEventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* value);
void (*RNDEventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int value);
void (*RNDEventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* value);
void (*RNDEventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* value);
void (*RNDEventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* value);

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
	struct UPnPDevice *device;
	struct UDNMapNode *Next;
	struct UDNMapNode *Previous;
};

void RNDDestructUPnPService(struct UPnPService *service)
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
		ILibLifeTime_Remove(((struct RNDCP*)service->Parent->CP)->LifeTimeMonitor,service);
		ILibDeleteEntry(((struct RNDCP*)service->Parent->CP)->SIDTable,service->SubscriptionID,(int)strlen(service->SubscriptionID));
		FREE(service->SubscriptionID);
		service->SubscriptionID = NULL;
	}
	
	FREE(service);
}
void RNDDestructUPnPDevice(struct UPnPDevice *device)
{
	struct UPnPDevice *d1,*d2;
	struct UPnPService *s1,*s2;
	
	d1 = device->EmbeddedDevices;
	while(d1!=NULL)
	{
		d2 = d1->Next;
		RNDDestructUPnPDevice(d1);
		d1 = d2;
	}
	
	s1 = device->Services;
	while(s1!=NULL)
	{
		s2 = s1->Next;
		RNDDestructUPnPService(s1);
		s1 = s2;
	}
	
	LVL3DEBUG(printf("\r\n\r\nDevice Destructed\r\n");)
	if(device->PresentationURL!=NULL) {FREE(device->PresentationURL);}
	if(device->ManufacturerName!=NULL) {FREE(device->ManufacturerName);}
	if(device->ManufacturerURL!=NULL) {FREE(device->ManufacturerURL);}
	if(device->ModelName!=NULL) {FREE(device->ModelName);}
	if(device->ModelNumber!=NULL) {FREE(device->ModelNumber);}
	if(device->ModelURL!=NULL) {FREE(device->ModelURL);}
	if(device->DeviceType!=NULL) {FREE(device->DeviceType);}
	if(device->FriendlyName!=NULL) {FREE(device->FriendlyName);}
	if(device->LocationURL!=NULL) {FREE(device->LocationURL);}
	if(device->UDN!=NULL) {FREE(device->UDN);}
	if(device->InterfaceToHost!=NULL) {FREE(device->InterfaceToHost);}
	
	FREE(device);
}

void RNDAddRef(struct UPnPDevice *device)
{
	struct RNDCP *CP = (struct RNDCP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	++d->ReferenceCount;
	sem_post(&(CP->DeviceLock));
}
void RNDRelease(struct UPnPDevice *device)
{
	struct RNDCP *CP = (struct RNDCP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while(d->Parent!=NULL) {d = d->Parent;}
	--d->ReferenceCount;
	if(d->ReferenceCount==0)
	{
		RNDDestructUPnPDevice(d);
	}
	sem_post(&(CP->DeviceLock));
}
void RNDPush(struct UPnP_Stack **pp_Top, void *data)
{
	struct UPnP_Stack *frame = (struct UPnP_Stack*)MALLOC(sizeof(struct UPnP_Stack));
	frame->data = data;
	frame->next = *pp_Top;
	*pp_Top = frame;
}
void *RNDPop(struct UPnP_Stack **pp_Top)
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
void *RNDPeek(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if(frame!=NULL)
	{
		RetVal = (*pp_Top)->data;
	}
	return(RetVal);
}
void RNDFlush(struct UPnP_Stack **pp_Top)
{
	while(RNDPop(pp_Top)!=NULL) {}
	*pp_Top = NULL;
}

void RNDAttachRootUDNToUDN(void *v_CP,char* UDN, char* RootUDN)
{
	struct UDNMapNode *node;
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
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
void RNDAttachDeviceToUDN(void *v_CP,char* UDN, struct UPnPDevice *device)
{
	struct UDNMapNode *node;
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
		{
			node->device = device;
			++device->ReferenceCount;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
}
void RNDRemoveUDN(void *v_CP,char* UDN)
{
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	struct UPnPDevice *device = NULL;
	struct UDNMapNode *node,*prevNode;
	if(CP->UDN_Head==NULL) {return;}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	prevNode = NULL;
	while (node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
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
	
	if(device!=NULL) {RNDRelease(device);}
}
void RNDAddUDN(void *v_CP,char *UDN)
{
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	struct UDNMapNode *node;
	int has = 0;
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
		{
			has = -1;
			break;
		}
		node=node->Next;
	}
	if(has==0)
	{
		node = (struct UDNMapNode*)MALLOC(sizeof(struct UDNMapNode));
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
struct UPnPDevice* RNDGetDeviceAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	struct UPnPDevice *RetVal = NULL;
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	if(CP->UDN_Head==NULL) {return(NULL);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
		{
			RetVal = node->device;
			if(RetVal!=NULL){++RetVal->ReferenceCount;}
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
char* RNDGetRootUDNAtUDN(void *v_CP,char* UDN)
{
	struct UDNMapNode *node;
	char *RetVal = NULL;
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	if(CP->UDN_Head==NULL) {return(NULL);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
		{
			RetVal = node->RootUDN;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
int RNDHasUDN(void *v_CP,char *UDN)
{
	struct UDNMapNode *node;
	struct RNDCP* CP = (struct RNDCP*)v_CP;
	int RetVal = 0;
	if(CP->UDN_Head==NULL) {return(0);}
	
	sem_wait(&(CP->DeviceLock));
	node = CP->UDN_Head;
	while(node!=NULL)
	{
		if(strncmp(node->UDN,UDN,(int)strlen(UDN))==0)
		{
			RetVal = -1;
			break;
		}
		node=node->Next;
	}
	sem_post(&(CP->DeviceLock));
	
	return(RetVal);
}
void RNDParseUri(char* URI, char** IP, unsigned short* Port, char** Path)
{
	struct parser_result *result,*result2,*result3;
	char *TempString,*TempString2;
	int TempStringLength,TempStringLength2;
	
	result = ILibParseString(URI, 0, (int)strlen(URI), "://", 3);
	TempString = result->LastResult->data;
	TempStringLength = result->LastResult->datalength;
	
	/* Parse Path */
	result2 = ILibParseString(TempString,0,TempStringLength,"/",1);
	TempStringLength2 = TempStringLength-result2->FirstResult->datalength;
	*Path = (char*)MALLOC(TempStringLength2+1);
	memcpy(*Path,TempString+(result2->FirstResult->datalength),TempStringLength2);
	(*Path)[TempStringLength2] = '\0';
	
	/* Parse Port Number */
	result3 = ILibParseString(result2->FirstResult->data,0,result2->FirstResult->datalength,":",1);
	if(result3->NumResults==1)
	{
		*Port = 80;
	}
	else
	{
		TempString2 = (char*)MALLOC(result3->LastResult->datalength+1);
		memcpy(TempString2,result3->LastResult->data,result3->LastResult->datalength);
		TempString2[result3->LastResult->datalength] = '\0';
		*Port = (unsigned short)atoi(TempString2);
		FREE(TempString2);
	}
	/* Parse IP Address */
	TempStringLength2 = result3->FirstResult->datalength;
	*IP = (char*)MALLOC(TempStringLength2+1);
	memcpy(*IP,result3->FirstResult->data,TempStringLength2);
	(*IP)[TempStringLength2] = '\0';
	ILibDestructParserResults(result3);
	ILibDestructParserResults(result2);
	ILibDestructParserResults(result);
}
struct packetheader *RNDBuildPacket(char* IP, int Port, char* Path, char* cmd)
{
	struct packetheader *RetVal = ILibCreateEmptyPacket();
	char* HostLine = (char*)MALLOC((int)strlen(IP)+7);
	int HostLineLength = sprintf(HostLine,"%s:%d",IP,Port);
	ILibSetDirective(RetVal,cmd,(int)strlen(cmd),Path,(int)strlen(Path));
	ILibAddHeaderLine(RetVal,"Host",4,HostLine,HostLineLength);
	ILibAddHeaderLine(RetVal,"User-Agent",10,"POSIX, UPnP/1.0, Intel MicroStack/1.0.1189",42);
	FREE(HostLine);
	return(RetVal);
}

void RNDRemoveServiceFromDevice(struct UPnPDevice *device, struct UPnPService *service)
{
	struct UPnPService *s = device->Services;
	
	if(s==service)
	{
		device->Services = s->Next;
		RNDDestructUPnPService(service);
		return;
	}
	while(s->Next!=NULL)
	{
		if(s->Next == service)
		{
			s->Next = s->Next->Next;
			RNDDestructUPnPService(service);
			return;
		}
		s = s->Next;
	}
}

void RNDProcessDevice(struct UPnPDevice *device)
{
	int OK = 0;
	struct UPnPService  *s,*s2;
	struct UPnPDevice *EmbeddedDevice = device->EmbeddedDevices;
	while(EmbeddedDevice!=NULL)
	{
		RNDProcessDevice(EmbeddedDevice);
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
			if(OK==0) {RNDRemoveServiceFromDevice(device,s);}
			s = s2;
		}
	}
	
}

void RNDPrintUPnPDevice(int indents, struct UPnPDevice *device)
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
		RNDPrintUPnPDevice(indents+5,d);
		d = d->Next;
	}
}
struct UPnPService *RNDGetService(struct UPnPDevice *device, char* ServiceName, int length)
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
struct UPnPService *RNDGetService_ContentDirectory(struct UPnPDevice *device)
{
	return(RNDGetService(device,"urn:schemas-upnp-org:service:ContentDirectory:1",47));
}
struct UPnPService *RNDGetService_ConnectionManager(struct UPnPDevice *device)
{
	return(RNDGetService(device,"urn:schemas-upnp-org:service:ConnectionManager:1",48));
}
struct UPnPDevice *RNDGetDevice2(struct UPnPDevice *device, int index, int *c_Index)
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
		RetVal = RNDGetDevice2(e_Device,index,&currentIndex);
		if(RetVal!=NULL)
		{
			break;
		}
		e_Device = e_Device->Next;
	}
	
	*c_Index = currentIndex;
	return(RetVal);
}
struct UPnPDevice* RNDGetDevice1(struct UPnPDevice *device,int index)
{
	int c_Index = -1;
	return(RNDGetDevice2(device,index,&c_Index));
}
int RNDGetDeviceCount(struct UPnPDevice *device)
{
	int RetVal = 0;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	
	while(e_Device!=NULL)
	{
		RetVal += RNDGetDeviceCount(e_Device);
		e_Device = e_Device->Next;
	}
	
	if(strncmp(device->DeviceType,"urn:schemas-upnp-org:device:MediaServer:1",41)==0)
	{
		++RetVal;
	}
	return(RetVal);
}

int RNDGetErrorCode(char *buffer, int length)
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
void RNDProcessSCPD(char* buffer, int length, struct UPnPService *service)
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
void RNDExpireRequests(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	struct UPnPService *s = device->Services;
	
	while(ed!=NULL)
	{
		RNDExpireRequests(ed);
		ed = ed->Next;
	}
	while(s!=NULL)
	{
		ILibDeleteRequests(((struct RNDCP*)s->Parent->CP)->HTTP,s);
		s = s->Next;
	}
}
void RNDDeviceExpired(struct UPnPDevice *device)
{
	printf("Device[%s] failed to re-advertise in a timely manner\r\n",device->FriendlyName);
	RNDExpireRequests(device);
	RNDSSDP_Sink(NULL, device->UDN, 0, NULL, 0,device->CP);
}
void RNDFinishProcessingDevice(struct RNDCP* CP, struct UPnPDevice *RootDevice)
{
	char *RootUDN = RootDevice->UDN;
	int Timeout = RootDevice->CacheTime;
	struct UPnPDevice *RetDevice;
	int i=0;
	
	RootDevice->ReferenceCount = 0;
	RNDAttachDeviceToUDN(CP,RootDevice->UDN,RootDevice);
	do
	{
		RetDevice = RNDGetDevice1(RootDevice,i++);
		if(RetDevice!=NULL)
		{
			RNDAddUDN(CP,RetDevice->UDN);
			RNDAttachRootUDNToUDN(CP,RetDevice->UDN,RootUDN);
			if(CP->DiscoverSink!=NULL)
			{
				CP->DiscoverSink(RetDevice);
			}
		}
	}while(RetDevice!=NULL);
	RetDevice = RNDGetDeviceAtUDN(CP,RootUDN);
	if(RetDevice!=NULL)
	{
		ILibLifeTime_Add(CP->LifeTimeMonitor,RetDevice,Timeout,&RNDDeviceExpired,NULL);
		RNDRelease(RetDevice);
	}
}
void RNDSCPD_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* dv, void* sv)
{
	struct UPnPDevice *device;
	struct UPnPService *service = (struct UPnPService*)sv;
	struct RNDCP *CP = service->Parent->CP;
	
	if(!(header==NULL || header->StatusCode!=200) && done!=0)
	{
		RNDProcessSCPD(buffer,EndPointer, service);
		
		device = service->Parent;
		while(device->Parent!=NULL)
		{
			device = device->Parent;
		}
		--device->SCPDLeft;
		if(device->SCPDLeft==0)
		{
			RNDFinishProcessingDevice(CP,device);
		}
	}
	else
	{
		if(done!=0 && (header==NULL || header->StatusCode!=200))
		{
			ILibDeleteRequests(CP->HTTP,dv);
			RNDDestructUPnPDevice(dv);
		}
	}
}
void RNDCalculateSCPD_FetchCount(struct UPnPDevice *device)
{
	int count = 0;
	struct UPnPDevice *root;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(e_Device!=NULL)
	{
		RNDCalculateSCPD_FetchCount(e_Device);
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
void RNDSCPD_Fetch(struct UPnPDevice *device)
{
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	char *IP,*Path;
	unsigned short Port;
	struct packetheader *p;
	struct sockaddr_in addr;
	
	while(e_Device!=NULL)
	{
		RNDSCPD_Fetch(e_Device);
		e_Device = e_Device->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		RNDParseUri(s->SCPDURL,&IP,&Port,&Path);
		DEBUGSTATEMENT(printf("SCPD: %s Port: %d Path: %s\r\n",IP,Port,Path));
		p = RNDBuildPacket(IP,Port,Path,"GET");
		
		memset((char *)&addr, 0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(IP);
		addr.sin_port = htons(Port);
		
		ILibAddRequest(((struct RNDCP*)device->CP)->HTTP, p,&addr, &RNDSCPD_Sink, device, s);
		
		FREE(IP);
		FREE(Path);
		s = s->Next;
	}
}
struct UPnPDevice* RNDProcessDeviceXML_device(struct ILibXMLNode *xml, void *v_CP,const char *BaseURL, int Timeout, int RecvAddr)
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
	device->CP = v_CP;
	device->CacheTime = Timeout;
	device->Tag = NULL;
	device->InterfaceToHost = (char*)MALLOC(16);
	sprintf(device->InterfaceToHost,"%d.%d.%d.%d",(RecvAddr&0xFF),((RecvAddr>>8)&0xFF),((RecvAddr>>16)&0xFF),((RecvAddr>>24)&0xFF));
	device->DeviceType = NULL;
	device->UDN = NULL;
	device->LocationURL = NULL;
	device->FriendlyName = NULL;
	device->Parent = NULL;
	device->EmbeddedDevices = NULL;
	device->Services = NULL;
	device->Next = NULL;
	device->LocationURL = NULL;
	device->PresentationURL = NULL;
	device->FriendlyName = NULL;
	device->ManufacturerName = NULL;
	device->ManufacturerURL = NULL;
	device->ModelName = NULL;
	device->ModelDescription = NULL;
	device->ModelNumber = NULL;
	device->ModelURL = NULL;
	device->SCPDLeft = 0;
	
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
						tempDevice = RNDProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout, RecvAddr);
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
			xml = xml->Next;
			tempNode = xml;
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

void RNDProcessDeviceXML(void *v_CP,char* buffer, int BufferSize, char* LocationURL, int RecvAddr, int Timeout)
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
		RNDParseUri(LocationURL,&IP,&Port,&Path);
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
	
	RootDevice = RNDProcessDeviceXML_device(xml,v_CP,BaseURL,Timeout,RecvAddr);
	FREE(BaseURL);
	ILibDestructXMLNodeList(rootXML);
	
	/* Add Root Device to UDNTable */
	RNDAddUDN(v_CP,RootDevice->UDN);
	
	/* Save reference to LocationURL in the RootDevice */
	RootDevice->LocationURL = (char*)MALLOC(strlen(LocationURL)+1);
	sprintf(RootDevice->LocationURL,"%s",LocationURL);
	
	/* Trim Object Structure */
	RNDProcessDevice(RootDevice);
	RootDevice->SCPDLeft = 0;
	RNDCalculateSCPD_FetchCount(RootDevice);
	if(RootDevice->SCPDLeft==0)
	{
		RNDFinishProcessingDevice(v_CP,RootDevice);
	}
	else
	{
		RNDSCPD_Fetch(RootDevice);
	}
}

void RNDHTTP_Sink_DeviceDescription(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* cp)
{
	struct CustomUserData *customData = (struct CustomUserData*)user;
	if(header!=NULL && done!=0)
	{
		RNDProcessDeviceXML(cp,buffer,EndPointer-(*p_BeginPointer),customData->buffer,header->ReceivingAddress,customData->Timeout);
	}
	if(done!=0)
	{
		FREE(customData->buffer);
		FREE(user);
	}
}
void RND_FlushRequest(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	struct UPnPService *s = device->Services;
	
	while(ed!=NULL)
	{
		RND_FlushRequest(ed);
		ed = ed->Next;
	}
	while(s!=NULL)
	{
		ILibDeleteRequests(((struct RNDCP*)device->CP)->HTTP, s);
		s = s->Next;
	}
}
void RNDSSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, int Timeout,void *cp)
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
		/* Hello */
		DEBUGSTATEMENT(printf("MediaServer Hello\r\n"));
		DEBUGSTATEMENT(printf("LocationURL: %s\r\n",LocationURL));
		if(RNDHasUDN(cp,LocationURL)==0)
		{
			RNDAddUDN(cp,LocationURL);
			if(RNDHasUDN(cp,UDN)==0)
			{
				RNDAddUDN(cp,UDN);
				RNDParseUri(LocationURL,&IP,&Port,&Path);
				DEBUGSTATEMENT(printf("IP: %s Port: %d Path: %s\r\n",IP,Port,Path));
				p = RNDBuildPacket(IP,Port,Path,"GET");
				
				memset((char *)&addr, 0,sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = inet_addr(IP);
				addr.sin_port = htons(Port);
				
				buffer = (char*)MALLOC((int)strlen(LocationURL)+1);
				strcpy(buffer,LocationURL);
				
				customData = (struct CustomUserData*)MALLOC(sizeof(struct CustomUserData));
				customData->Timeout = Timeout;
				customData->buffer = buffer;
				
				ILibAddRequest(((struct RNDCP*)cp)->HTTP, p,&addr, &RNDHTTP_Sink_DeviceDescription, customData, cp);
				
				FREE(IP);
				FREE(Path);
			}
		}
		else
		{
			// Periodic Notify Packets
			if(RNDHasUDN(cp,UDN)!=0)
			{
				buffer = RNDGetRootUDNAtUDN(cp,UDN);
				if(buffer!=NULL)
				{
					device = RNDGetDeviceAtUDN(cp,buffer);
					if(device!=NULL)
					{
						//Extend LifetimeMonitor duration
						ILibLifeTime_Remove(((struct RNDCP*)cp)->LifeTimeMonitor,device);
						ILibLifeTime_Add(((struct RNDCP*)cp)->LifeTimeMonitor,device,Timeout,&RNDDeviceExpired,NULL);
						RNDRelease(device);
					}
				}
			}
		}
	}
	else
	{
		/* Bye Bye */
		DEBUGSTATEMENT(printf("MediaServer ByeBye\r\n"));
		device = RNDGetDeviceAtUDN(cp,UDN);
		if(device!=NULL)
		{
			ILibLifeTime_Remove(((struct RNDCP*)cp)->LifeTimeMonitor,device);
			RNDRemoveUDN(cp,device->LocationURL);
			do
			{
				tempDevice = RNDGetDevice1(device,i++);
				if(tempDevice!=NULL)
				{
					RND_FlushRequest(tempDevice);
					if(((struct RNDCP*)cp)->RemoveSink!=NULL)
					{
						((struct RNDCP*)cp)->RemoveSink(tempDevice);
					}
				}
			} while(tempDevice!=NULL);
			RNDRelease(device);
			RNDRemoveUDN(cp,UDN);
		}
	}
}
void RNDContentDirectory_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
{
	struct ILibXMLNode *xml,*rootXML;
	char *tempString;
	int tempStringLength;
	int flg,flg2;
	
	char* TransferIDs = 0;
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
							if(xml->NameLength==11 && memcmp(xml->Name,"TransferIDs",11) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								TransferIDs = tempString;
								if(RNDEventCallback_ContentDirectory_TransferIDs != NULL)
								{
									RNDEventCallback_ContentDirectory_TransferIDs(service,TransferIDs);
								}
							}
							if(xml->NameLength==18 && memcmp(xml->Name,"ContainerUpdateIDs",18) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								ContainerUpdateIDs = tempString;
								if(RNDEventCallback_ContentDirectory_ContainerUpdateIDs != NULL)
								{
									RNDEventCallback_ContentDirectory_ContainerUpdateIDs(service,ContainerUpdateIDs);
								}
							}
							if(xml->NameLength==14 && memcmp(xml->Name,"SystemUpdateID",14) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								if(ILibGetULong(tempString,tempStringLength,&TempULong)==0)
								{
									SystemUpdateID = (unsigned int) TempULong;
								}
								if(RNDEventCallback_ContentDirectory_SystemUpdateID != NULL)
								{
									RNDEventCallback_ContentDirectory_SystemUpdateID(service,SystemUpdateID);
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
void RNDConnectionManager_EventSink(char* buffer, int bufferlength, struct UPnPService *service)
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
								if(RNDEventCallback_ConnectionManager_SourceProtocolInfo != NULL)
								{
									RNDEventCallback_ConnectionManager_SourceProtocolInfo(service,SourceProtocolInfo);
								}
							}
							if(xml->NameLength==16 && memcmp(xml->Name,"SinkProtocolInfo",16) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								SinkProtocolInfo = tempString;
								if(RNDEventCallback_ConnectionManager_SinkProtocolInfo != NULL)
								{
									RNDEventCallback_ConnectionManager_SinkProtocolInfo(service,SinkProtocolInfo);
								}
							}
							if(xml->NameLength==20 && memcmp(xml->Name,"CurrentConnectionIDs",20) == 0)
							{
								tempStringLength = ILibReadInnerXML(xml,&tempString);
								tempString[tempStringLength] = '\0';
								CurrentConnectionIDs = tempString;
								if(RNDEventCallback_ConnectionManager_CurrentConnectionIDs != NULL)
								{
									RNDEventCallback_ConnectionManager_CurrentConnectionIDs(service,CurrentConnectionIDs);
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
void RNDOnEventSink(void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user)
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
		ILibAddHeaderLine(resp,"Server",6,"POSIX, UPnP/1.0, Intel MicroStack/1.0.1189",42);
		field = header->FirstField;
		while(field!=NULL)
		{
			if(field->FieldLength==3)
			{
				if(strncasecmp(field->Field,"SID",3)==0)
				{
					sid = (char*)MALLOC(field->FieldDataLength+1);
					sprintf(sid,"%s",field->FieldData);
					value = ILibGetEntry(((struct RNDCP*)user)->SIDTable,field->FieldData,field->FieldDataLength);
					break;
				}
			}
			field = field->NextField;
		}
		
		if(value==NULL)
		{
			/* Not a valid SID */
			ILibSetStatusCode(resp,412,"Failed",6);
			if(sid!=NULL) {FREE(sid);}
		}
		else
		{
			ILibSetStatusCode(resp,200,"OK",2);
			service = (struct UPnPService*)value;
			
			type_length = (int)strlen(service->ServiceType);
			if(type_length>46 && strncmp("urn:schemas-upnp-org:service:ContentDirectory:",service->ServiceType,46)==0)
			{
				RNDContentDirectory_EventSink(buffer, BufferSize, service);
			}
			else
			if(type_length>47 && strncmp("urn:schemas-upnp-org:service:ConnectionManager:",service->ServiceType,47)==0)
			{
				RNDConnectionManager_EventSink(buffer, BufferSize, service);
			}
		}
		ILibMiniWebServerSend(ReaderObject,resp);
		ILibDestructPacket(resp);
		ILibMiniWebServerCloseSession(ReaderObject);
	}
}
void RNDOnSubscribeSink(void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user, void *vcp)
{
	struct UPnPService *s;
	struct packetheader_field_node *field;
	struct parser_result *p;
	struct RNDCP *cp = (struct RNDCP*)vcp;
	
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
					if(field->FieldLength==3 && strncasecmp(field->Field,"SID",3)==0)
					{
						s->SubscriptionID = (char*)MALLOC(1+field->FieldDataLength);
						strcpy(s->SubscriptionID,field->FieldData);
						ILibAddEntry(cp->SIDTable,field->FieldData,field->FieldDataLength,s);
					} else
					if(field->FieldLength==7 && strncasecmp(field->Field,"TIMEOUT",7)==0)
					{
						p = ILibParseString(field->FieldData,0,field->FieldDataLength,"-",1);
						p->LastResult->data[p->LastResult->datalength] = '\0';
						RNDAddRef(s->Parent);
						ILibLifeTime_Add(cp->LifeTimeMonitor,s,atoi(p->LastResult->data)/2,&RNDRenew,NULL);
						ILibDestructParserResults(p);
					}
					field = field->NextField;
				}
			}
		}
		RNDRelease(s->Parent);
	}
}

void RNDRenew(void *state)
{
	struct UPnPService *service = (struct UPnPService*)state;
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	struct sockaddr_in destaddr;
	
	RNDParseUri(service->SubscriptionURL,&IP,&Port,&Path);
	p = ILibCreateEmptyPacket();
	
	ILibSetDirective(p,"SUBSCRIBE",9,Path,(int)strlen(Path));
	
	TempString = (char*)MALLOC((int)strlen(IP)+7);
	sprintf(TempString,"%s:%d",IP,Port);
	
	ILibAddHeaderLine(p,"HOST",4,TempString,(int)strlen(TempString));
	FREE(TempString);
	
	ILibAddHeaderLine(p,"SID",3,service->SubscriptionID,(int)strlen(service->SubscriptionID));
	ILibAddHeaderLine(p,"TIMEOUT",7,"Second-180",10);
	ILibAddHeaderLine(p,"User-Agent",10,"POSIX, UPnP/1.0, Intel MicroStack/1.0.1189",42);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	ILibAddRequest(((struct RNDCP*)service->Parent->CP)->HTTP, p,&destaddr, &RNDOnSubscribeSink, (void*)service, service->Parent->CP);
	
	FREE(IP);
	FREE(Path);
}

struct UPnPDevice* RNDGetDevice(struct UPnPDevice *device, char* DeviceType, int number)
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
int RNDHasAction(struct UPnPService *s, char* action)
{
	struct UPnPAction *a = s->Actions;
	
	while(a!=NULL)
	{
		if(strcmp(action,a->Name)==0) return(-1);
		a = a->Next;
	}
	return(0);
}
void RNDStopCP(void *v_CP)
{
	int i;
	struct UPnPDevice *RetDevice;
	struct UDNMapNode *mn,*mn2;
	struct RNDCP *CP= (struct RNDCP*)v_CP;
	sem_destroy(&(CP->DeviceLock));
	
	mn = CP->UDN_Head;
	while(mn!=NULL)
	{
		mn2 = mn->Next;
		if(mn->RootUDN!=NULL){FREE(mn->RootUDN);}
		if(mn->device!=NULL)
		{
			i = 0;
			if(CP->RemoveSink!=NULL)
			{
				do
				{
					RetDevice = RNDGetDevice1(mn->device,i++);
					if(RetDevice!=NULL)
					{
						CP->RemoveSink(RetDevice);
					}
				}while(RetDevice!=NULL);
			}
			RNDDestructUPnPDevice(mn->device);
		}
		FREE(mn->UDN);
		FREE(mn);
		mn = mn2;
	}
	ILibDestroyHashTree(CP->SIDTable);
}
void RND_CP_IPAddressListChanged(void *CPToken)
{
	((struct RNDCP*)CPToken)->RecheckFlag = 1;
	ILibForceUnBlockChain(((struct RNDCP*)CPToken)->Chain);
}
void RNDCP_ProcessDeviceRemoval(struct RNDCP* CP, struct UPnPDevice *device)
{
	struct UPnPDevice *temp = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while(temp!=NULL)
	{
		RNDCP_ProcessDeviceRemoval(CP,temp);
		temp = temp->Next;
	}
	
	s = device->Services;
	while(s!=NULL)
	{
		ILibLifeTime_Remove(CP->LifeTimeMonitor,s);
		s = s->Next;
	}
}

void RNDCP_PreSelect(void *CPToken,fd_set *readset, fd_set *writeset, fd_set *errorset, int *blocktime)
{
	struct UDNMapNode *mn,*mn2;
	struct RNDCP *CP= (struct RNDCP*)CPToken;
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
			found = 0;
			for(i=0;i<NumAddressList;++i)
			{
				if(mn->device!=NULL && IPAddressList[i]==inet_addr(mn->device->InterfaceToHost))
				{
					found = 1;
					break;
				}
			}
			if(found==0)
			{
				// Clear LifeTime for services contained
				RNDCP_ProcessDeviceRemoval(CP,mn->device);
				CP->RemoveSink(mn->device);
				RNDDestructUPnPDevice(mn->device);
				
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
			mn = mn2;
		}
		
		ILibSSDP_IPAddressListChanged(CP->SSDP);
		FREE(CP->AddressList);
		CP->AddressListLength = NumAddressList;
		CP->AddressList = IPAddressList;
	}
}
void *RNDCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*))
{
	struct RNDCP *cp = (struct RNDCP*)MALLOC(sizeof(struct RNDCP));
	
	cp->Destroy = &RNDStopCP;
	cp->PostSelect = NULL;
	cp->PreSelect = &RNDCP_PreSelect;
	cp->DiscoverSink = A;
	cp->RemoveSink = R;
	
	sem_init(&(cp->DeviceLock),0,1);
	cp->UDN_Head = NULL;
	cp->WebServer = ILibCreateMiniWebServer(Chain,5,&RNDOnEventSink,cp);
	cp->SIDTable = ILibInitHashTree();
	
	ILibAddToChain(Chain,cp);
	cp->LifeTimeMonitor = ILibCreateLifeTime(Chain);
	cp->SSDP = ILibCreateSSDPClientModule(Chain,"urn:schemas-upnp-org:device:MediaServer:1", 41, &RNDSSDP_Sink,cp);
	cp->HTTP = ILibCreateHTTPClientModule(Chain,5);
	
	cp->Chain = Chain;
	cp->RecheckFlag = 0;
	cp->AddressListLength = ILibGetLocalIPAddressList(&(cp->AddressList));
	return((void*)cp);
}
void RNDInvoke_ContentDirectory_Search_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_Search(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int), void* user, char* ContainerID, char* SearchCriteria, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Search",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_Search_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_StopTransferResource_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_StopTransferResource(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, unsigned int TransferID)
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
	SoapBodyTemplate = "%sStopTransferResource xmlns:u=\"%s\"><TransferID>%u</TransferID></u:StopTransferResource%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, TransferID,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(173 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"StopTransferResource",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_StopTransferResource_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_DestroyObject_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_DestroyObject(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, char* ObjectID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ObjectID)+279);
	SoapBodyTemplate = "%sDestroyObject xmlns:u=\"%s\"><ObjectID>%s</ObjectID></u:DestroyObject%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ObjectID,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(166 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"DestroyObject",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_DestroyObject_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_UpdateObject_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_UpdateObject(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, char* ObjectID, char* CurrentTagValue, char* NewTagValue)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ObjectID)+(int)strlen(CurrentTagValue)+(int)strlen(NewTagValue)+339);
	SoapBodyTemplate = "%sUpdateObject xmlns:u=\"%s\"><ObjectID>%s</ObjectID><CurrentTagValue>%s</CurrentTagValue><NewTagValue>%s</NewTagValue></u:UpdateObject%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ObjectID, CurrentTagValue, NewTagValue,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(165 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"UpdateObject",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_UpdateObject_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_ExportResource_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	unsigned int TransferID = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==22 && memcmp(xml->Name,"ExportResourceResponse",22)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==10 && memcmp(xml->Name,"TransferID",10) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						TransferID = (unsigned int) TempULong;
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
		((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,TransferID);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_ExportResource(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int), void* user, char* SourceURI, char* DestinationURI)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(SourceURI)+(int)strlen(DestinationURI)+316);
	SoapBodyTemplate = "%sExportResource xmlns:u=\"%s\"><SourceURI>%s</SourceURI><DestinationURI>%s</DestinationURI></u:ExportResource%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, SourceURI, DestinationURI,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(167 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"ExportResource",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_ExportResource_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_GetTransferProgress_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 3;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* TransferStatus = NULL;
	char* TransferLength = NULL;
	char* TransferTotal = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==27 && memcmp(xml->Name,"GetTransferProgressResponse",27)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==14 && memcmp(xml->Name,"TransferStatus",14) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TransferStatus = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==14 && memcmp(xml->Name,"TransferLength",14) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TransferLength = tempBuffer;
					}
				}
				else 
				if(xml->NameLength==13 && memcmp(xml->Name,"TransferTotal",13) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						TransferTotal = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,TransferStatus,TransferLength,TransferTotal);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_GetTransferProgress(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*,char*), void* user, unsigned int TransferID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+305);
	SoapBodyTemplate = "%sGetTransferProgress xmlns:u=\"%s\"><TransferID>%u</TransferID></u:GetTransferProgress%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, TransferID,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(172 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetTransferProgress",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_GetTransferProgress_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_GetSearchCapabilities_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(174 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSearchCapabilities",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_GetSearchCapabilities_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_GetSystemUpdateID_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int), void* user)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(170 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSystemUpdateID",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_GetSystemUpdateID_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_CreateObject_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 2;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* ObjectID = NULL;
	char* Result = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==20 && memcmp(xml->Name,"CreateObjectResponse",20)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==8 && memcmp(xml->Name,"ObjectID",8) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						ObjectID = tempBuffer;
					}
				}
				else 
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
		((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,ObjectID,Result);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_CreateObject(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*), void* user, char* ContainerID, char* Elements)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ContainerID)+(int)strlen(Elements)+304);
	SoapBodyTemplate = "%sCreateObject xmlns:u=\"%s\"><ContainerID>%s</ContainerID><Elements>%s</Elements></u:CreateObject%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ContainerID, Elements,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(165 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"CreateObject",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_CreateObject_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_GetSortCapabilities_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(172 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetSortCapabilities",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_GetSortCapabilities_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_Browse_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_Browse(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,unsigned int,unsigned int,unsigned int), void* user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(159 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"Browse",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_Browse_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_ImportResource_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	unsigned long TempULong;
	unsigned int TransferID = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==22 && memcmp(xml->Name,"ImportResourceResponse",22)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==10 && memcmp(xml->Name,"TransferID",10) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetULong(tempBuffer,tempBufferLength,&TempULong)==0)
					{
						TransferID = (unsigned int) TempULong;
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
		((void (*)(struct UPnPService*,int,void*,unsigned int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,TransferID);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_ImportResource(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,unsigned int), void* user, char* SourceURI, char* DestinationURI)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(SourceURI)+(int)strlen(DestinationURI)+316);
	SoapBodyTemplate = "%sImportResource xmlns:u=\"%s\"><SourceURI>%s</SourceURI><DestinationURI>%s</DestinationURI></u:ImportResource%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, SourceURI, DestinationURI,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(167 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"ImportResource",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_ImportResource_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_CreateReference_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 1;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	char* NewID = NULL;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==23 && memcmp(xml->Name,"CreateReferenceResponse",23)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==5 && memcmp(xml->Name,"NewID",5) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(tempBufferLength!=0)
					{
						tempBuffer[tempBufferLength] = '\0';
						NewID = tempBuffer;
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
		((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,NewID);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_CreateReference(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user, char* ContainerID, char* ObjectID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ContainerID)+(int)strlen(ObjectID)+310);
	SoapBodyTemplate = "%sCreateReference xmlns:u=\"%s\"><ContainerID>%s</ContainerID><ObjectID>%s</ObjectID></u:CreateReference%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ContainerID, ObjectID,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(168 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"CreateReference",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_CreateReference_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ContentDirectory_DeleteResource_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ContentDirectory_DeleteResource(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, char* ResourceURI)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(ResourceURI)+287);
	SoapBodyTemplate = "%sDeleteResource xmlns:u=\"%s\"><ResourceURI>%s</ResourceURI></u:DeleteResource%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ResourceURI,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(167 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"DeleteResource",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ContentDirectory_DeleteResource_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ConnectionManager_GetCurrentConnectionInfo_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,int,int,char*,char*,int,char*,char*), void* user, int ConnectionID)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(177 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ConnectionManager_GetCurrentConnectionInfo_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ConnectionManager_PrepareForConnection_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	int ArgLeft = 3;
	struct ILibXMLNode *xml;
	struct ILibXMLNode *__xml;
	char *tempBuffer;
	int tempBufferLength;
	long TempLong;
	int ConnectionID = 0;
	int AVTransportID = 0;
	int RcsID = 0;
	
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,int,int,int))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,int,int,int))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	__xml = xml = ILibParseXML(buffer,0,EndPointer-(*p_BeginPointer));
	ILibProcessXMLNodeList(xml);
	while(xml!=NULL)
	{
		if(xml->NameLength==28 && memcmp(xml->Name,"PrepareForConnectionResponse",28)==0)
		{
			xml = xml->Next;
			while(xml!=NULL)
			{
				if(xml->NameLength==12 && memcmp(xml->Name,"ConnectionID",12) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						ConnectionID = (int) TempLong;
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
				if(xml->NameLength==5 && memcmp(xml->Name,"RcsID",5) == 0)
				{
					tempBufferLength = ILibReadInnerXML(xml,&tempBuffer);
					--ArgLeft;
					if(ILibGetLong(tempBuffer,tempBufferLength,&TempLong)==0)
					{
						RcsID = (int) TempLong;
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
		((void (*)(struct UPnPService*,int,void*,int,int,int))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User,INVALID_DATA,INVALID_DATA,INVALID_DATA);
	}
	else
	{
		((void (*)(struct UPnPService*,int,void*,int,int,int))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User,ConnectionID,AVTransportID,RcsID);
	}
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ConnectionManager_PrepareForConnection(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,int,int,int), void* user, char* RemoteProtocolInfo, char* PeerConnectionManager, int PeerConnectionID, char* Direction)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+(int)strlen(RemoteProtocolInfo)+(int)strlen(PeerConnectionManager)+(int)strlen(Direction)+431);
	SoapBodyTemplate = "%sPrepareForConnection xmlns:u=\"%s\"><RemoteProtocolInfo>%s</RemoteProtocolInfo><PeerConnectionManager>%s</PeerConnectionManager><PeerConnectionID>%d</PeerConnectionID><Direction>%s</Direction></u:PrepareForConnection%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, RemoteProtocolInfo, PeerConnectionManager, PeerConnectionID, Direction,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(173 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"PrepareForConnection",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ConnectionManager_PrepareForConnection_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ConnectionManager_ConnectionComplete_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
{
	struct UPnPService *Service = (struct UPnPService*)_service;
	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;
	if(done==0){return;}
	if(_InvokeData->CallbackPtr==NULL)
	{
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
	}
	
	((void (*)(struct UPnPService*,int,void*))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User);
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ConnectionManager_ConnectionComplete(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*), void* user, int ConnectionID)
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
	buffer = (char*)MALLOC((int)strlen(service->ServiceType)+308);
	SoapBodyTemplate = "%sConnectionComplete xmlns:u=\"%s\"><ConnectionID>%d</ConnectionID></u:ConnectionComplete%s";
	bufferLength = sprintf(buffer,SoapBodyTemplate,UPNPCP_SOAP_BodyHead,service->ServiceType, ConnectionID,UPNPCP_SOAP_BodyTail);
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(171 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"ConnectionComplete",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ConnectionManager_ConnectionComplete_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ConnectionManager_GetProtocolInfo_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*,char*), void* user)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(168 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetProtocolInfo",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ConnectionManager_GetProtocolInfo_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDInvoke_ConnectionManager_GetCurrentConnectionIDs_Sink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* _service, void *state)
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
		RNDRelease(Service->Parent);
		FREE(_InvokeData);
		return;
	}
	else
	{
		if(header==NULL)
		{
			/* Connection Failed */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
			FREE(_InvokeData);
			return;
		}
		else if(header->StatusCode!=200)
		{
			/* SOAP Fault */
			((void (*)(struct UPnPService*,int,void*,char*))_InvokeData->CallbackPtr)(Service,RNDGetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User,INVALID_DATA);
			RNDRelease(Service->Parent);
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
	RNDRelease(Service->Parent);
	FREE(_InvokeData);
}
void RNDInvoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService*,int,void*,char*), void* user)
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
	
	RNDAddRef(service->Parent);
	RNDParseUri(service->ControlURL,&IP,&Port,&Path);
	
	headerBuffer = (char*)MALLOC(176 + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType));
	headerLength = sprintf(headerBuffer,UPNPCP_SOAP_Header,Path,IP,Port,service->ServiceType,"GetCurrentConnectionIDs",bufferLength);
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(Port);
	
	invoke_data->CallbackPtr = CallbackPtr;
	invoke_data->User = user;
	ILibAddRequest_DirectEx(((struct RNDCP*)service->Parent->CP)->HTTP, headerBuffer,headerLength,buffer,bufferLength,&addr, &RNDInvoke_ConnectionManager_GetCurrentConnectionIDs_Sink, service, invoke_data);	
	
	FREE(IP);
	FREE(Path);
}
void RNDSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK))
{
	char* callback;
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	struct sockaddr_in destaddr;
	
	RNDParseUri(service->SubscriptionURL,&IP,&Port,&Path);
	p = ILibCreateEmptyPacket();
	
	ILibSetDirective(p,"SUBSCRIBE",9,Path,(int)strlen(Path));
	
	TempString = (char*)MALLOC((int)strlen(IP)+7);
	sprintf(TempString,"%s:%d",IP,Port);
	
	ILibAddHeaderLine(p,"HOST",4,TempString,(int)strlen(TempString));
	FREE(TempString);
	
	ILibAddHeaderLine(p,"NT",2,"upnp:event",10);
	ILibAddHeaderLine(p,"TIMEOUT",7,"Second-180",10);
	ILibAddHeaderLine(p,"User-Agent",10,"POSIX, UPnP/1.0, Intel MicroStack/1.0.1189",42);
	
	callback = (char*)MALLOC(10+(int)strlen(service->Parent->InterfaceToHost)+6+(int)strlen(Path));
	sprintf(callback,"<http://%s:%d%s>",service->Parent->InterfaceToHost,ILibGetMiniWebServerPortNumber(((struct RNDCP*)service->Parent->CP)->WebServer),Path);
	
	ILibAddHeaderLine(p,"CALLBACK",8,callback,(int)strlen(callback));
	FREE(callback);
	
	memset((char *)&destaddr, 0,sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = inet_addr(IP);
	destaddr.sin_port = htons(Port);
	
	RNDAddRef(service->Parent);
	ILibAddRequest(((struct RNDCP*)service->Parent->CP)->HTTP, p,&destaddr, &RNDOnSubscribeSink, (void*)service,service->Parent->CP);
	
	FREE(IP);
	FREE(Path);
}
