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
char* UPnPPLATFORM = "WINDOWS UPnP/!UPNPVERSION! MicroStack/!MICROSTACKVERSION!";
#elif defined(__SYMBIAN32__)
char* UPnPPLATFORM = "SYMBIAN UPnP/!UPNPVERSION! MicroStack/!MICROSTACKVERSION!";
#else
char* UPnPPLATFORM = "POSIX UPnP/!UPNPVERSION! MicroStack/!MICROSTACKVERSION!";
#endif

#if defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#define snprintf(dst, len, frm, ...) _snprintf_s(dst, len, _TRUNCATE, frm, __VA_ARGS__)
#define strncpy(dst, src, len) strcpy_s(dst, len, src)
#endif

#if defined(WINSOCK2)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#include <wininet.h>
#endif
#include "ILibParsers.h"
#include "ILibSSDPClient.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"
#include "UPnPControlPoint.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
#include <crtdbg.h>
#endif

#define UPNP_PORT 1900
#define UPNP_MCASTv4_GROUP "239.255.255.250"
#define UPNP_MCASTv6_GROUP "FF05:0:0:0:0:0:0:C"
#define UPNP_MCASTv6_GROUPB "[FF05:0:0:0:0:0:0:C]"
#define UPnPMIN(a, b) (((a)<(b))?(a):(b))

#define UPnPInvocationPriorityLevel ILibAsyncSocket_QOS_CONTROL

#define INVALID_DATA 0
#define DEBUGSTATEMENT(x)
#define LVL3DEBUG(x)
#define INET_SOCKADDR_LENGTH(x) ((x == AF_INET6?sizeof(struct sockaddr_in6):sizeof(struct sockaddr_in)))

static const char *UPNPCP_SOAP_Header = "POST %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: %s\r\nSOAPACTION: \"%s#%s\"\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nContent-Length: %d\r\n\r\n";
static const char *UPNPCP_SOAP_BodyHead = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:";
static const char *UPNPCP_SOAP_BodyTail = "></s:Body></s:Envelope>";

void UPnPRenew(void *state);
void UPnPSSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, struct sockaddr* LocationAddr, int Timeout, UPnPSSDP_MESSAGE m, void *cp);

struct CustomUserData
{
	int Timeout;
	char* buffer;
	char *UDN;
	char *LocationURL;
	struct sockaddr_in6 LocationAddr;
};

struct UPnPCP
{
	void (*PreSelect)(void* object, fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object, int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void (*DiscoverSink)(struct UPnPDevice *device);
	void (*RemoveSink)(struct UPnPDevice *device);
	
	// ToDo: Check to see if this is necessary
	void (*EventCallback_DimmingService_LoadLevelStatus)(struct UPnPService* Service, unsigned char value);
	void (*EventCallback_SwitchPower_Status)(struct UPnPService* Service, int value);
	
	struct LifeTimeMonitorStruct *LifeTimeMonitor;
	
	void *HTTP;
	void *SSDP;
	void *WebServer;
	void *User;
	
	sem_t DeviceLock;
	void* SIDTable;
	void* DeviceTable_UDN;
	void* DeviceTable_Tokens;
	void* DeviceTable_URI;
	
	void *Chain;
	int RecheckFlag;

	struct sockaddr_in *IPAddressListV4;
	int IPAddressListV4Length;
	struct sockaddr_in6 *IPAddressListV6;
	int IPAddressListV6Length;

	UPnPDeviceDiscoveryErrorHandler ErrorDispatch;
};

//{{{EventCallbacks}}}

struct InvokeStruct
{
	struct UPnPService *Service;
	voidfp CallbackPtr;
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

void UPnPSetUser(void *token, void *user)
{
	((struct UPnPCP*)token)->User = user;
}

void* UPnPGetUser(void *token)
{
	return(((struct UPnPCP*)token)->User);
}

void UPnPCP_ProcessDeviceRemoval(struct UPnPCP* CP, struct UPnPDevice *device);

void UPnPDestructUPnPService(struct UPnPService *service)
{
	struct UPnPAction *a1, *a2;
	struct UPnPStateVariable *sv1, *sv2;
	int i;
	
	a1 = service->Actions;
	while (a1 != NULL)
	{
		a2 = a1->Next;
		free(a1->Name);
		free(a1);
		a1 = a2;
	}
	
	sv1 = service->Variables;
	while (sv1 != NULL)
	{
		sv2 = sv1->Next;
		free(sv1->Name);
		if (sv1->Min != NULL) {free(sv1->Min);}
		if (sv1->Max != NULL) {free(sv1->Max);}
		if (sv1->Step != NULL) {free(sv1->Step);}
		for(i=0;i<sv1->NumAllowedValues;++i) free(sv1->AllowedValues[i]);
		if (sv1->AllowedValues != NULL) {free(sv1->AllowedValues);}
		free(sv1);
		sv1 = sv2;
	}
	if (service->ControlURL != NULL) {free(service->ControlURL);}
	if (service->SCPDURL != NULL) {free(service->SCPDURL);}
	if (service->ServiceId != NULL) {free(service->ServiceId);}
	if (service->ServiceType != NULL) {free(service->ServiceType);}
	if (service->SubscriptionURL != NULL) {free(service->SubscriptionURL);}
	if (service->SubscriptionID != NULL)
	{
		ILibLifeTime_Remove(((struct UPnPCP*)service->Parent->CP)->LifeTimeMonitor, service);
		ILibDeleteEntry(((struct UPnPCP*)service->Parent->CP)->SIDTable, service->SubscriptionID, (int)strlen(service->SubscriptionID));
		free(service->SubscriptionID);
		service->SubscriptionID = NULL;
	}
	
	free(service);
}
void UPnPDestructUPnPDevice(struct UPnPDevice *device)
{
	struct UPnPDevice *d1, *d2;
	struct UPnPService *s1, *s2;
	int iconIndex;
	//{{{BEGIN_CustomTagSpecific}}}
	void *en, *en2;
	char *key;
	int keyLen;
	void *val, *ht2;
	//{{{END_CustomTagSpecific}}}
	
	d1 = device->EmbeddedDevices;
	while (d1 != NULL)
	{
		d2 = d1->Next;
		UPnPDestructUPnPDevice(d1);
		d1 = d2;
	}
	
	s1 = device->Services;
	while (s1 != NULL)
	{
		s2 = s1->Next;
		UPnPDestructUPnPService(s1);
		s1 = s2;
	}
	
	LVL3DEBUG(printf("\r\n\r\nDevice Destructed\r\n");)
	if (device->PresentationURL != NULL) {free(device->PresentationURL);}
	if (device->ManufacturerName != NULL) {free(device->ManufacturerName);}
	if (device->ManufacturerURL != NULL) {free(device->ManufacturerURL);}
	if (device->ModelName != NULL) {free(device->ModelName);}
	if (device->ModelNumber != NULL) {free(device->ModelNumber);}
	if (device->ModelURL != NULL) {free(device->ModelURL);}
	if (device->ModelDescription != NULL) {free(device->ModelDescription);}
	if (device->DeviceType != NULL) {free(device->DeviceType);}
	if (device->FriendlyName != NULL) {free(device->FriendlyName);}
	if (device->LocationURL != NULL) {free(device->LocationURL);}
	if (device->UDN != NULL) {free(device->UDN);}
	if (device->InterfaceToHost != NULL) {free(device->InterfaceToHost);}
	if (device->Reserved3 != NULL) {free(device->Reserved3);}
	if (device->IconsLength>0)
	{
		for(iconIndex=0;iconIndex<device->IconsLength;++iconIndex)
		{
			if (device->Icons[iconIndex].mimeType != NULL){free(device->Icons[iconIndex].mimeType);}
			if (device->Icons[iconIndex].url != NULL){free(device->Icons[iconIndex].url);}
		}
		free(device->Icons);
	}
	
	//{{{BEGIN_CustomTagSpecific}}}
	if (device->CustomTagTable != NULL) 
	{
		en = ILibHashTree_GetEnumerator(device->CustomTagTable);
		while (ILibHashTree_MoveNext(en) == 0)
		{
			ILibHashTree_GetValue(en, &key, &keyLen, &ht2);
			en2 = ILibHashTree_GetEnumerator(ht2);
			while (ILibHashTree_MoveNext(en2) == 0)
			{
				ILibHashTree_GetValue(en2, &key, &keyLen, &val);
				if (val != NULL)
				{
					free(val);
				}
			}
			ILibHashTree_DestroyEnumerator(en2);
			ILibDestroyHashTree(ht2);
		}
		ILibHashTree_DestroyEnumerator(en);
		ILibDestroyHashTree(device->CustomTagTable);
	}
	//{{{END_CustomTagSpecific}}}
	
	free(device);
}


/*! \fn UPnPAddRef(struct UPnPDevice *device)
\brief Increments the reference counter for a UPnP device
\param device UPnP device
*/
void UPnPAddRef(struct UPnPDevice *device)
{
	struct UPnPCP *CP = (struct UPnPCP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while (d->Parent != NULL) {d = d->Parent;}
	++d->ReferenceCount;
	sem_post(&(CP->DeviceLock));
}

void UPnPCheckfpDestroy(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	if (device->fpDestroy != NULL) {device->fpDestroy(device);}
	while (ed != NULL)
	{
		UPnPCheckfpDestroy(ed);
		ed = ed->Next;
	}
}
/*! \fn UPnPRelease(struct UPnPDevice *device)
\brief Decrements the reference counter for a UPnP device.
\para Device will be disposed when the counter becomes zero.
\param device UPnP device
*/
void UPnPRelease(struct UPnPDevice *device)
{
	struct UPnPCP *CP = (struct UPnPCP*)device->CP;
	struct UPnPDevice *d = device;
	sem_wait(&(CP->DeviceLock));
	while (d->Parent != NULL) {d = d->Parent;}
	--d->ReferenceCount;
	if (d->ReferenceCount<=0)
	{
		UPnPCheckfpDestroy(device);
		UPnPDestructUPnPDevice(d);
	}
	sem_post(&(CP->DeviceLock));
}

//void UPnPDeviceDescriptionInterruptSink(void *sender, void *user1, void *user2)
//{
	//	struct CustomUserData *cd = (struct CustomUserData*)user1;
	//	free(cd->buffer);
	//	free(user1);
//}

int UPnPIsLegacyDevice(struct packetheader *header)
{
	struct packetheader_field_node *f;
	struct parser_result_field *prf;
	struct parser_result *r, *r2;
	int Legacy = 1;
	
	// Check version of Device
	f = header->FirstField;
	while (f != NULL)
	{
		if (f->FieldLength == 6 && strncasecmp(f->Field, "SERVER", 6) == 0)
		{
			// Check UPnP version of the Control Point which invoked us
			r = ILibParseString(f->FieldData, 0, f->FieldDataLength, " ", 1);
			prf = r->FirstResult;
			while (prf != NULL)
			{
				if (prf->datalength > 5 && memcmp(prf->data, "UPnP/", 5) == 0)
				{
					r2 = ILibParseString(prf->data+5, 0, prf->datalength-5, ".", 1);
					r2->FirstResult->data[r2->FirstResult->datalength] = 0;
					r2->LastResult->data[r2->LastResult->datalength] = 0;
					if (atoi(r2->FirstResult->data) == 1 && atoi(r2->LastResult->data) > 0)
					{
						Legacy = 0;
					}
					ILibDestructParserResults(r2);
				}
				prf = prf->NextResult;
			}
			ILibDestructParserResults(r);
		}
		if (Legacy)
		{
			f = f->NextField;
		}
		else
		{
			break;
		}
	}
	return Legacy;
}
void UPnPPush(struct UPnP_Stack **pp_Top, void *data)
{
	struct UPnP_Stack *frame;
	if ((frame = (struct UPnP_Stack*)malloc(sizeof(struct UPnP_Stack))) == NULL) ILIBCRITICALEXIT(254);
	frame->data = data;
	frame->next = *pp_Top;
	*pp_Top = frame;
}

void *UPnPPop(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if (frame != NULL)
	{
		*pp_Top = frame->next;
		RetVal = frame->data;
		free(frame);
	}
	return RetVal;
}

void *UPnPPeek(struct UPnP_Stack **pp_Top)
{
	struct UPnP_Stack *frame = *pp_Top;
	void *RetVal = NULL;
	
	if (frame != NULL)
	{
		RetVal = (*pp_Top)->data;
	}
	return RetVal;
}

void UPnPFlush(struct UPnP_Stack **pp_Top)
{
	while (UPnPPop(pp_Top) != NULL) {}
	*pp_Top = NULL;
}

/*! \fn UPnPGetDeviceAtUDN(void *v_CP, char* UDN)
\brief Fetches a device with a particular UDN
\param v_CP Control Point Token
\param UDN Unique Device Name
\returns UPnP device
*/
struct UPnPDevice* UPnPGetDeviceAtUDN(void *v_CP, char* UDN)
{
	struct UPnPDevice *RetVal = NULL;
	struct UPnPCP* CP = (struct UPnPCP*)v_CP;
	
	ILibHashTree_Lock(CP->DeviceTable_UDN);
	RetVal = (struct UPnPDevice*)ILibGetEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN));
	ILibHashTree_UnLock(CP->DeviceTable_UDN);
	return(RetVal);
}
struct packetheader *UPnPBuildPacket(char* IP, int Port, char* Path, char* cmd)
{
	char* HostLine;
	int HostLineLength = (int)strlen(IP) + 7;
	struct packetheader *RetVal = ILibCreateEmptyPacket();
	if ((HostLine = (char*)malloc(HostLineLength)) == NULL) ILIBCRITICALEXIT(254);
	HostLineLength = snprintf(HostLine, HostLineLength, "%s:%d", IP, Port);
	
	ILibSetVersion(RetVal, "1.1", 3);
	ILibSetDirective(RetVal, cmd, (int)strlen(cmd), Path, (int)strlen(Path));
	ILibAddHeaderLine(RetVal, "Host", 4, HostLine, HostLineLength);
	ILibAddHeaderLine(RetVal, "User-Agent", 10, UPnPPLATFORM, (int)strlen(UPnPPLATFORM));
	free(HostLine);
	return(RetVal);
}

void UPnPRemoveServiceFromDevice(struct UPnPDevice *device, struct UPnPService *service)
{
	struct UPnPService *s = device->Services;
	
	if (s == service)
	{
		device->Services = s->Next;
		UPnPDestructUPnPService(service);
		return;
	}
	while (s->Next != NULL)
	{
		if (s->Next == service)
		{
			s->Next = s->Next->Next;
			UPnPDestructUPnPService(service);
			return;
		}
		s = s->Next;
	}
}

void UPnPProcessDevice(void *v_cp, struct UPnPDevice *device)
{
	struct UPnPCP* cp = (struct UPnPCP*)v_cp;
	struct UPnPDevice *EmbeddedDevice = device->EmbeddedDevices;
	size_t len;
	
	// Copy the LocationURL if necessary
	if (device->LocationURL == NULL && device->Parent != NULL && device->Parent->LocationURL != NULL)
	{
		len = strlen(device->Parent->LocationURL) + 1;
		if ((device->LocationURL = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		memcpy(device->LocationURL, device->Parent->LocationURL, len);
	}
	while (EmbeddedDevice != NULL)
	{
		UPnPProcessDevice(v_cp, EmbeddedDevice);
		EmbeddedDevice = EmbeddedDevice->Next;
	}
	
	// Create a table entry for each Unique Device Name, for easy mapping
	// of ssdp:byebye packets. This way any byebye packet from the device
	// heirarchy will result in the device being removed.
	
	ILibHashTree_Lock(cp->DeviceTable_UDN);
	ILibAddEntry(cp->DeviceTable_UDN, device->UDN, (int)strlen(device->UDN), device);
	ILibHashTree_UnLock(cp->DeviceTable_UDN);
}

/*! fn UPnPPrintUPnPDevice(int indents, struct UPnPDevice *device)
\brief Debug method that displays the device object structure
\param indents The number of spaces to indent each sub-entry
\param device The device to display
*/
void UPnPPrintUPnPDevice(int indents, struct UPnPDevice *device)
{
	struct UPnPService *s;
	struct UPnPDevice *d;
	struct UPnPAction *a;
	int x = 0;
	
	for(x=0; x<indents; ++x) { printf(" "); }
	printf("Device: %s\r\n", device->DeviceType);
	
	for(x=0; x<indents; ++x) { printf(" "); }
	printf("Friendly: %s\r\n", device->FriendlyName);
	
	s = device->Services;
	while (s != NULL)
	{
		for(x=0; x<indents; ++x) { printf(" "); }
		printf("   Service: %s\r\n", s->ServiceType);
		a = s->Actions;
		while (a != NULL)
		{
			for(x=0;x<indents;++x) { printf(" "); }
			printf("      Action: %s\r\n", a->Name);
			a = a->Next;
		}
		s = s->Next;
	}
	
	d = device->EmbeddedDevices;
	while (d != NULL)
	{
		UPnPPrintUPnPDevice(indents+5, d);
		d = d->Next;
	}
}

/*! \fn UPnPGetService(struct UPnPDevice *device, char* ServiceName, int length)
\brief Obtains a specific UPnP service instance of appropriate version, from within a device
\para
This method returns services who's version is greater than or equal to that specified within \a ServiceName
\param device UPnP device
\param ServiceName Service Type
\param length Length of \a ServiceName
\returns UPnP service
*/
struct UPnPService *UPnPGetService(struct UPnPDevice *device, char* ServiceName, int length)
{
	struct UPnPService *RetService = NULL;
	struct UPnPService *s = device->Services;
	struct parser_result *pr, *pr2;
	
	pr = ILibParseString(ServiceName, 0, length, ":", 1);
	while (s != NULL)
	{
		pr2 = ILibParseString(s->ServiceType, 0, (int)strlen(s->ServiceType), ":", 1);
		if (length-pr->LastResult->datalength >= (int)strlen(s->ServiceType)-pr2->LastResult->datalength && memcmp(ServiceName, s->ServiceType, length-pr->LastResult->datalength) == 0)
		{
			if (atoi(pr->LastResult->data) <= atoi(pr2->LastResult->data))
			{
				RetService = s;
				ILibDestructParserResults(pr2);
				break;
			}
		}
		ILibDestructParserResults(pr2);
		s = s->Next;
	}
	ILibDestructParserResults(pr);
	
	return RetService;
}

//{{{UPnPGetServiceMethods}}}

struct UPnPDevice *UPnPGetDevice2(struct UPnPDevice *device, int index, int *c_Index)
{
	struct UPnPDevice *RetVal = NULL;
	struct UPnPDevice *e_Device = NULL;
	int currentIndex = *c_Index;
	
	//{{{UPnPGetDevice2_if_statement}}}
	{
		++currentIndex;
		if (currentIndex == index)
		{
			*c_Index = currentIndex;
			return(device);
		}
	}
	
	e_Device = device->EmbeddedDevices;
	while (e_Device != NULL)
	{
		RetVal = UPnPGetDevice2(e_Device, index, &currentIndex);
		if (RetVal != NULL) break;
		e_Device = e_Device->Next;
	}
	
	*c_Index = currentIndex;
	return RetVal;
}

struct UPnPDevice* UPnPGetDevice1(struct UPnPDevice *device, int index)
{
	int c_Index = -1;
	return UPnPGetDevice2(device, index, &c_Index);
}

int UPnPGetDeviceCount(struct UPnPDevice *device)
{
	int RetVal = 0;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	
	while (e_Device != NULL)
	{
		RetVal += UPnPGetDeviceCount(e_Device);
		e_Device = e_Device->Next;
	}
	//{{{GetDeviceCountIfStatement}}}
	return RetVal;
}

//
// Internal method to parse the SOAP Fault
//
int UPnPGetErrorCode(char *buffer, int length)
{
	int RetVal = 500;
	struct ILibXMLNode *xml, *rootXML;
	
	char *temp;
	int tempLength;
	
	if (buffer == NULL) { return(RetVal); }
	
	rootXML = xml = ILibParseXML(buffer, 0, length);
	if (ILibProcessXMLNodeList(xml) == 0)
	{
		while (xml != NULL)
		{
			if (xml->NameLength == 8 && memcmp(xml->Name, "Envelope", 8) == 0)
			{
				xml = xml->Next;
				while (xml != NULL)
				{
					if (xml->NameLength == 4 && memcmp(xml->Name, "Body", 4) == 0)
					{
						xml = xml->Next;
						while (xml != NULL)
						{
							if (xml->NameLength == 5 && memcmp(xml->Name, "Fault", 5) == 0)
							{
								xml = xml->Next;
								while (xml != NULL)
								{
									if (xml->NameLength == 6 && memcmp(xml->Name, "detail", 6) == 0)
									{
										xml = xml->Next;
										while (xml != NULL)
										{
											if (xml->NameLength == 9 && memcmp(xml->Name, "UPnPError", 9) == 0)
											{
												xml = xml->Next;
												while (xml != NULL)
												{
													if (xml->NameLength == 9 && memcmp(xml->Name, "errorCode", 9) == 0)
													{
														tempLength = ILibReadInnerXML(xml, &temp);
														temp[tempLength] = 0;
														RetVal =atoi(temp);
														xml = NULL;
													}
													if (xml != NULL) {xml = xml->Peer;}
												}
											}
											if (xml != NULL) {xml = xml->Peer;}
										}
									}
									if (xml != NULL) {xml = xml->Peer;}
								}
							}
							if (xml != NULL) {xml = xml->Peer;}
						}
					}
					if (xml != NULL) {xml = xml->Peer;}
				}
			}
			if (xml != NULL) {xml = xml->Peer;}
		}
	}
	ILibDestructXMLNodeList(rootXML);
	return(RetVal);
}

//
// Internal method to parse the SCPD document
//
void UPnPProcessSCPD(char* buffer, int length, struct UPnPService *service)
{
	struct UPnPAction *action;
	struct UPnPStateVariable *sv = NULL;
	struct UPnPAllowedValue *av = NULL;
	struct UPnPAllowedValue *avs = NULL;
	
	struct ILibXMLNode *xml, *rootXML;
	int flg2, flg3, flg4;
	
	char* tempString;
	int tempStringLength;
	
	struct UPnPDevice *root = service->Parent;
	while (root->Parent != NULL) {root = root->Parent;}
	
	rootXML = xml = ILibParseXML(buffer, 0, length);
	if (ILibProcessXMLNodeList(xml) != 0)
	{
		// The XML Document was not well formed
		root->SCPDError=UPNP_ERROR_SCPD_NOT_WELL_FORMED;
		ILibDestructXMLNodeList(rootXML);
		return;
	}
	
	while (xml != NULL && strncmp(xml->Name, "!", 1) == 0)
	{
		xml = xml->Next;
	}
	xml = xml->Next;
	while (xml != NULL)
	{
		// Iterate all the actions in the actionList element
		if (xml->NameLength == 10 && memcmp(xml->Name, "actionList", 10) == 0)
		{
			xml = xml->Next;
			flg2 = 0;
			while (flg2 == 0)
			{
				if (xml->NameLength == 6 && memcmp(xml->Name, "action", 6) == 0)
				{
					// Action element
					if ((action = (struct UPnPAction*)malloc(sizeof(struct UPnPAction))) == NULL) ILIBCRITICALEXIT(254);
					action->Name = NULL;
					action->Next = service->Actions;
					service->Actions = action;
					
					xml = xml->Next;
					flg3 = 0;
					while (flg3 == 0)
					{
						if (xml->NameLength == 4 && memcmp(xml->Name, "name", 4) == 0)
						{
							tempStringLength = ILibReadInnerXML(xml, &tempString);
							if ((action->Name = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(action->Name, tempString, tempStringLength);
							action->Name[tempStringLength] = '\0';
						}
						if (xml->Peer == NULL)
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
				if (xml->Peer == NULL)
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
		
		// Iterate all the StateVariables in the state table
		if (xml->NameLength == 17 && memcmp(xml->Name, "serviceStateTable", 17) == 0)
		{
			if (xml->Next->StartTag != 0)
			{
				xml = xml->Next;
				flg2 = 0;
				while (flg2 == 0)
				{
					if (xml->NameLength == 13 && memcmp(xml->Name, "stateVariable", 13) == 0)
					{
						// Initialize a new state variable
						if ((sv = (struct UPnPStateVariable*)malloc(sizeof(struct UPnPStateVariable))) == NULL) ILIBCRITICALEXIT(254);
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
						while (flg3 == 0)
						{
							if (xml->NameLength == 4 && memcmp(xml->Name, "name", 4) == 0)
							{
								// Populate the name
								tempStringLength = ILibReadInnerXML(xml, &tempString);
								if ((sv->Name = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
								memcpy(sv->Name, tempString, tempStringLength);
								sv->Name[tempStringLength] = '\0';
							}
							if (xml->NameLength == 16 && memcmp(xml->Name, "allowedValueList", 16) == 0)
							{
								// This state variable defines an allowed value list
								if (xml->Next->StartTag != 0)
								{
									avs = NULL;
									xml = xml->Next;
									flg4 = 0;
									while (flg4 == 0)
									{
										// Iterate through all the allowed values, and reference them
										if (xml->NameLength == 12 && memcmp(xml->Name, "allowedValue", 12) == 0)
										{
											if ((av = (struct UPnPAllowedValue*)malloc(sizeof(struct UPnPAllowedValue))) == NULL) ILIBCRITICALEXIT(254);
											av->Next = avs;
											avs = av;
											
											tempStringLength = ILibReadInnerXML(xml, &tempString);
											if ((av->Value = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
											memcpy(av->Value, tempString, tempStringLength);
											av->Value[tempStringLength] = '\0';
										}
										if (xml->Peer != NULL)
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
									while (av != NULL)
									{
										++sv->NumAllowedValues;
										av = av->Next;
									}
									av = avs;
									if ((sv->AllowedValues = (char**)malloc(sv->NumAllowedValues*sizeof(char*))) == NULL) ILIBCRITICALEXIT(254);
									for(flg4=0;flg4<sv->NumAllowedValues;++flg4)
									{
										sv->AllowedValues[flg4] = av->Value;
										av = av->Next;
									}
									av = avs;
									while (av != NULL)
									{
										avs = av->Next;
										free(av);
										av = avs;
									}
								}
							}
							if (xml->NameLength == 17 && memcmp(xml->Name, "allowedValueRange", 17) == 0)
							{
								// The state variable defines a range
								if (xml->Next->StartTag != 0)
								{
									xml = xml->Next;
									flg4 = 0;
									while (flg4 == 0)
									{
										if (xml->NameLength == 7)
										{
											if (memcmp(xml->Name, "minimum", 7) == 0)
											{
												// Set the minimum range
												tempStringLength = ILibReadInnerXML(xml, &tempString);
												if ((sv->Min = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
												memcpy(sv->Min, tempString, tempStringLength);
												sv->Min[tempStringLength] = '\0';
											}
											else if (memcmp(xml->Name, "maximum", 7) == 0)
											{
												// Set the maximum range
												tempStringLength = ILibReadInnerXML(xml, &tempString);
												if ((sv->Max = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
												memcpy(sv->Max, tempString, tempStringLength);
												sv->Max[tempStringLength] = '\0';
											}
										}
										if (xml->NameLength == 4 && memcmp(xml->Name, "step", 4) == 0)
										{
											// Set the stepping
											tempStringLength = ILibReadInnerXML(xml, &tempString);
											if ((sv->Step = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
											memcpy(sv->Step, tempString, tempStringLength);
											sv->Step[tempStringLength] = '\0';
										}
										if (xml->Peer != NULL)
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
							if (xml->Peer != NULL)
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
					if (xml->Peer != NULL)
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


//
// Internal method called from SSDP dispatch, when the
// IP Address for a device has changed
//
void UPnPInterfaceChanged(struct UPnPDevice *device)
{
	void *cp = device->CP;
	char *UDN;
	char *LocationURL;
	int Timeout;
	size_t len;
	
	Timeout = device->CacheTime;
	len = strlen(device->UDN) + 1;
	if ((UDN = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	memcpy(UDN, device->UDN, len);
	LocationURL = device->Reserved3;
	device->Reserved3 = NULL;
	
	UPnPSSDP_Sink(NULL, device->UDN, 0, NULL, NULL, 0, UPnPSSDP_NOTIFY, device->CP);
	UPnPSSDP_Sink(NULL, UDN, -1, LocationURL, (struct sockaddr*)&(device->LocationAddr), Timeout, UPnPSSDP_NOTIFY, cp);
	
	free(UDN);
	free(LocationURL);
}

//
// Internal Timed Event Sink, that is called when a device
// has failed to refresh it's NOTIFY packets. So
// we'll assume the device is no longer available
//
void UPnPExpiredDevice(struct UPnPDevice *device)
{
	LVL3DEBUG(printf("Device[%s] failed to re-advertise in a timely manner\r\n", device->FriendlyName);)
	while (device->Parent != NULL) {device = device->Parent;}
	UPnPSSDP_Sink(NULL, device->UDN, 0, NULL, NULL, 0, UPnPSSDP_NOTIFY, device->CP);
}

//
// The discovery process for the device is complete. Just need to 
// set the reference counters, and notify the layers above us
//
void UPnPFinishProcessingDevice(struct UPnPCP* CP, struct UPnPDevice *RootDevice)
{
	int Timeout = RootDevice->CacheTime;
	struct UPnPDevice *RetDevice;
	int i = 0;
	
	RootDevice->ReferenceCount = 1;
	do
	{
		RetDevice = UPnPGetDevice1(RootDevice, i++);
		if (RetDevice != NULL)
		{
			// Set Reserved to non-zero to indicate that we are passing
			// this instance up the stack to the app layer above. Add a reference
			// to the device while we're at it.
			RetDevice->Reserved=1;
			UPnPAddRef(RetDevice);
			if (CP->DiscoverSink != NULL)
			{
				// Notify the app layer above
				CP->DiscoverSink(RetDevice);
			}
		}
	} while (RetDevice != NULL);
	//
	// Set a timed callback for the refresh cycle of the device. If the
	// device doesn't refresh by then, we'll remove this device.
	//
	ILibLifeTime_Add(CP->LifeTimeMonitor, RootDevice, Timeout, (ILibLifeTime_OnCallback)&UPnPExpiredDevice, NULL);
}

//
// Internal HTTP Sink for fetching the SCPD document
//
void UPnPSCPD_Sink(
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
	struct UPnPCP *CP = (struct UPnPCP*)service->Parent->CP;

	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( p_BeginPointer );
	UNREFERENCED_PARAMETER( dv );
	UNREFERENCED_PARAMETER( PAUSE );
	
	//
	// header == NULL if there was an error connecting to the device
	// StatusCode != 200 if there was an HTTP error in fetching the SCPD
	// done != 0 when the GET is complete
	//
	if (!(header == NULL || !ILibWebClientIsStatusOk(header->StatusCode)) && done != 0)
	{
		UPnPProcessSCPD(buffer, EndPointer, service);
		
		//
		// Fetch the root device
		//
		device = service->Parent;
		while (device->Parent != NULL)
		{
			device = device->Parent;
		}
		//
		// Decrement the counter indicating how many
		// SCPD documents are left to fetch
		//
		--device->SCPDLeft;
		
		//
		// Check to see that we have all the SCPD documents we were
		// looking for
		//
		if (device->SCPDLeft == 0)
		{
			if (device->SCPDError == 0)
			{
				//
				// No errors, complete processing
				//
				UPnPFinishProcessingDevice(CP, device);
			}
			else if (IsInterrupt == 0)
			{
				//
				// Errors occured, so free all the resources, of this 
				// stale device
				//
				UPnPCP_ProcessDeviceRemoval(CP, device);
				UPnPDestructUPnPDevice(device);
			}
		}
	}
	else
	{
		//
		// Errors happened while trying to fetch the SCPD
		//
		if (done != 0 && (header == NULL || !ILibWebClientIsStatusOk(header->StatusCode)))
		{
			//
			// Get the root device
			//
			device = service->Parent;
			while (device->Parent != NULL)
			{
				device = device->Parent;
			}
			
			//
			// Decrement the counter indicating how many
			// SCPD documents are left to fetch
			//
			--device->SCPDLeft;
			
			//
			// Set the flag indicating that an error has occured
			//
			device->SCPDError = 1;
			if (device->SCPDLeft == 0 && IsInterrupt == 0)
			{
				//
				// If all the SCPD requests have been attempted, free
				// all the resources of this stale device
				//
				UPnPCP_ProcessDeviceRemoval(CP, device);
				UPnPDestructUPnPDevice(device);
			}
		}
	}
}

//
// Internal method used to calculate how many SCPD
// documents will need to be fetched
//
void UPnPCalculateSCPD_FetchCount(struct UPnPDevice *device)
{
	int count = 0;
	struct UPnPDevice *root;
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	
	while (e_Device != NULL)
	{
		UPnPCalculateSCPD_FetchCount(e_Device);
		e_Device = e_Device->Next;
	}
	
	s = device->Services;
	while (s != NULL)
	{
		++count;
		s = s->Next;
	}
	
	root = device;
	while (root->Parent != NULL)
	{
		root = root->Parent;
	}
	root->SCPDLeft += count;
}

//
// Internal method used to actually make the HTTP
// requests to obtain all the Service Description Documents.
//
void UPnPSCPD_Fetch(struct UPnPDevice *device, struct sockaddr *LocationAddr)
{
	struct UPnPDevice *e_Device = device->EmbeddedDevices;
	struct UPnPService *s;
	char *IP, *Path;
	unsigned short Port;
	struct packetheader *p;

	while (e_Device != NULL)
	{
		//
		// We need to recursively call this on all the embedded devices, 
		// to insure all of those services are accounted for
		//
		UPnPSCPD_Fetch(e_Device, LocationAddr);
		e_Device = e_Device->Next;
	}

	//
	// Initialize address information to device
	//
	memcpy(&(device->LocationAddr), LocationAddr, INET_SOCKADDR_LENGTH(LocationAddr->sa_family));

	//
	// Iterate through all of the services contained in this device
	//
	s = device->Services;
	while (s != NULL)
	{
		//
		// Parse the SCPD URL, and then build the request packet
		//
		ILibParseUri(s->SCPDURL, &IP, &Port, &Path, NULL);
		DEBUGSTATEMENT(printf("SCPD: %s Port: %d Path: %s\r\n", IP, Port, Path));
		p = UPnPBuildPacket(IP, Port, Path, "GET");

		//
		// Actually make the HTTP Request
		//
		ILibWebClient_PipelineRequest(
			((struct UPnPCP*)device->CP)->HTTP, 
			(struct sockaddr*)&(s->Parent->LocationAddr), 
			p, 
			&UPnPSCPD_Sink, 
			device, 
			s);
		
		//
		// Free the resources from our ILibParseURI() method call
		//
		free(IP);
		free(Path);
		s = s->Next;
	}
}

//{{{BEGIN_CustomTagSpecific}}}
/*! \fn UPnPGetCustomTagFromDevice(struct UPnPDevice *d, char* FullNameSpace, char* Name)
\brief Returns the value of the specified custom XML element
\param d The UPnP device to query
\param FullNameSpace The fully resolved namespace URI of the custom element
\param Name The local name of the custom element
\returns The value of the specified XML element. NULL if it doesn't exist.
*/
char *UPnPGetCustomTagFromDevice(struct UPnPDevice *d, char* FullNameSpace, char* Name)
{
	if (ILibHasEntry(d->CustomTagTable, FullNameSpace, (int)strlen(FullNameSpace)) != 0)
	{
		return(ILibGetEntry(ILibGetEntry(d->CustomTagTable, FullNameSpace, (int)strlen(FullNameSpace)), Name, (int)strlen(Name)));
	}
	else
	{
		return(NULL);
	}
}
//{{{END_CustomTagSpecific}}}

void UPnPProcessDeviceXML_iconList(struct UPnPDevice *device, const char *BaseURL, struct ILibXMLNode *xml)
{
	struct ILibXMLNode *x2;
	int tempStringLength;
	char *tempString;
	struct parser_result *tpr;
	int numIcons = 0;
	struct UPnPIcon *Icons = NULL;
	char *iconURL=NULL;
	size_t len;
	
	//
	// Count how many icons we have
	//
	x2 = xml;
	while (x2 != NULL)
	{
		if (x2->NameLength == 4 && memcmp(x2->Name, "icon", 4) == 0)
		{
			++numIcons;
		}
		x2 = x2->Peer;
	}
	if ((Icons = (struct UPnPIcon*)malloc(numIcons*sizeof(struct UPnPIcon))) == NULL) ILIBCRITICALEXIT(254);
	memset(Icons, 0, numIcons*sizeof(struct UPnPIcon));
	device->IconsLength = numIcons;
	device->Icons = Icons;
	numIcons = 0;
	
	while (xml != NULL)
	{
		if (xml->NameLength == 4 && memcmp(xml->Name, "icon", 4) == 0)
		{
			x2 = xml->Next;
			while (x2 != NULL)
			{
				if (x2->NameLength == 5 && memcmp(x2->Name, "width", 5) == 0)
				{
					tempStringLength = ILibReadInnerXML(x2, &tempString);
					tempString[tempStringLength] = 0;
					Icons[numIcons].width = atoi(tempString);
				}
				if (x2->NameLength == 6 && memcmp(x2->Name, "height", 6) == 0)
				{
					tempStringLength = ILibReadInnerXML(x2, &tempString);
					tempString[tempStringLength] = 0;
					Icons[numIcons].height = atoi(tempString);
				}
				if (x2->NameLength == 5 && memcmp(x2->Name, "depth", 5) == 0)
				{
					tempStringLength = ILibReadInnerXML(x2, &tempString);
					tempString[tempStringLength] = 0;
					Icons[numIcons].colorDepth = atoi(tempString);
				}
				if (x2->NameLength == 8 && memcmp(x2->Name, "mimetype", 8) == 0)
				{
					tempStringLength = ILibReadInnerXML(x2, &tempString);
					Icons[numIcons].mimeType = ILibString_Copy(tempString, tempStringLength);
				}
				if (x2->NameLength == 3 && memcmp(x2->Name, "url", 3) == 0)
				{
					tempStringLength = ILibReadInnerXML(x2, &tempString);
					tempString[tempStringLength] = 0;
					tpr = ILibParseString(tempString, 0, tempStringLength, "://", 3);
					if (tpr->NumResults == 1)
					{
						// RelativeURL
						len = strlen(BaseURL);
						if (tempString[0] == '/')
						{
							if ((iconURL = (char*)malloc(1 + len + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(iconURL, BaseURL, len);
							memcpy(iconURL + len, tempString + 1, tempStringLength);
						}
						else
						{
							if ((iconURL = (char*)malloc(2 + len + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(iconURL, BaseURL, len);
							memcpy(iconURL + len, tempString, tempStringLength + 1);
						}
					}
					else
					{
						// AbsoluteURL
						if ((iconURL = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(iconURL, tempString, tempStringLength);
						iconURL[tempStringLength] = '\0';
					}
					ILibDestructParserResults(tpr);
					Icons[numIcons].url = iconURL;
				}
				x2 = x2->Peer;
			}
			++numIcons;
		}
		xml = xml->Peer;
	}
}
struct UPnPDevice* UPnPProcessDeviceXML_device(struct ILibXMLNode *xml, void *v_CP, const char *BaseURL, struct sockaddr *BaseAddr, int Timeout, struct sockaddr* RecvAddr)
{
	struct ILibXMLNode *tempNode;
	struct ILibXMLAttribute *a, *root_a;
	int flg, flg2;
	char *tempString;
	int tempStringLength;
	//{{{BEGIN_CustomTagSpecific}}}
	char *tempString2;
	//{{{END_CustomTagSpecific}}}
	struct parser_result *tpr;
	char *ServiceId = NULL;
	int ServiceIdLength = 0;
	char* ServiceType = NULL;
	int ServiceTypeLength = 0;
	char* SCPDURL = NULL;
	int SCPDURLLength = 0;
	char* EventSubURL = NULL;
	int EventSubURLLength = 0;
	char* ControlURL = NULL;
	int ControlURLLength = 0;
	int ServiceMaxVersion;
	struct UPnPDevice *tempDevice;
	struct UPnPService *TempService;
	struct UPnPDevice *device;
	size_t len;

	UNREFERENCED_PARAMETER( BaseAddr );

	if ((device = (struct UPnPDevice*)malloc(sizeof(struct UPnPDevice))) == NULL) ILIBCRITICALEXIT(254);
	memset(device, 0, sizeof(struct UPnPDevice));
	//{{{BEGIN_CustomTagSpecific}}}
	device->CustomTagTable = ILibInitHashTree();
	//{{{END_CustomTagSpecific}}}
	
	device->MaxVersion=1;
	device->CP = v_CP;
	device->CacheTime = Timeout;
	memcpy(&(device->InterfaceToHostAddr), RecvAddr, INET_SOCKADDR_LENGTH(RecvAddr->sa_family));

	// Create a human readable verion
	if ((device->InterfaceToHost = (char*)malloc(64)) == NULL) ILIBCRITICALEXIT(254);
	if (RecvAddr->sa_family == AF_INET)
	{
		// IPv4
		ILibInet_ntop(AF_INET, &(((struct sockaddr_in*)RecvAddr)->sin_addr), device->InterfaceToHost, 64);
	}
	else
	{
		// IPv6
		size_t len;
		device->InterfaceToHost[0] = '[';
		ILibInet_ntop(AF_INET6, &(((struct sockaddr_in6*)RecvAddr)->sin6_addr), device->InterfaceToHost + 1, 62);
		len = strlen(device->InterfaceToHost);
		device->InterfaceToHost[len] = ']';
		device->InterfaceToHost[len+1] = 0;
	}

	root_a = a = ILibGetXMLAttributes(xml);
	if (a != NULL)
	{
		while (a != NULL)
		{
			a->Name[a->NameLength]=0;
			if (strcasecmp(a->Name, "MaxVersion") == 0)
			{
				a->Value[a->ValueLength]=0;
				device->MaxVersion = atoi(a->Value);
				break;
			}
			a = a->Next;
		}
		ILibDestructXMLAttributeList(root_a);
	}
	
	xml = xml->Next;
	while (xml != NULL)
	{
		if (xml->NameLength == 10 && memcmp(xml->Name, "deviceList", 10) == 0)
		{
			if (xml->Next->StartTag != 0)
			{
				//
				// Iterate through all the device elements contained in the
				// deviceList element
				//
				xml = xml->Next;
				flg2 = 0;
				while (flg2 == 0)
				{
					if (xml->NameLength == 6 && memcmp(xml->Name, "device", 6) == 0)
					{
						//
						// If this device contains other devices, then we can recursively call ourselves
						//
						tempDevice = UPnPProcessDeviceXML_device(xml, v_CP, BaseURL, (struct sockaddr*)&(device->LocationAddr), Timeout, RecvAddr);
						tempDevice->Parent = device;
						tempDevice->Next = device->EmbeddedDevices;
						device->EmbeddedDevices = tempDevice;
					}
					if (xml->Peer == NULL)
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
		}
		else if (xml->NameLength == 3 && memcmp(xml->Name, "UDN", 3) == 0)
		{
			//
			// Copy the UDN out of the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if (tempStringLength>5)
			{
				if (memcmp(tempString, "uuid:", 5) == 0)
				{
					tempString += 5;
					tempStringLength -= 5;
				}
				if ((device->UDN = (char*)malloc(tempStringLength + 1)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(device->UDN, tempString, tempStringLength);
				device->UDN[tempStringLength] = '\0';
			}
		} 
		else if (xml->NameLength == 10 && memcmp(xml->Name, "deviceType", 10) == 0)
		{
			//
			// Copy the device type out of the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			
			if ((device->DeviceType = (char*)malloc(tempStringLength + 1)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->DeviceType, tempString, tempStringLength);
			device->DeviceType[tempStringLength] = '\0';
		}
		else if (xml->NameLength == 12 && memcmp(xml->Name, "friendlyName", 12) == 0)
		{
			//
			// Copy the friendly name out of the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->FriendlyName = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->FriendlyName, tempString, tempStringLength);
			device->FriendlyName[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 12 && memcmp(xml->Name, "manufacturer", 12) == 0)
		{
			//
			// Copy the Manufacturer from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ManufacturerName = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ManufacturerName, tempString, tempStringLength);
			device->ManufacturerName[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 15 && memcmp(xml->Name, "manufacturerURL", 15) == 0)
		{
			//
			// Copy the Manufacturer's URL from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ManufacturerURL = (char*)malloc(1+tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ManufacturerURL, tempString, tempStringLength);
			device->ManufacturerURL[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 16 && memcmp(xml->Name, "modelDescription", 16) == 0)
		{
			//
			// Copy the model meta data from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ModelDescription = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ModelDescription, tempString, tempStringLength);
			device->ModelDescription[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 9 && memcmp(xml->Name, "modelName", 9) == 0)
		{
			//
			// Copy the model meta data from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ModelName = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ModelName, tempString, tempStringLength);
			device->ModelName[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 11 && memcmp(xml->Name, "modelNumber", 11) == 0)
		{
			//
			// Copy the model meta data from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ModelNumber = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ModelNumber, tempString, tempStringLength);
			device->ModelNumber[tempStringLength] = '\0';
		} 
		else if (xml->NameLength == 8 && memcmp(xml->Name, "modelURL", 8) == 0)
		{
			//
			// Copy the model meta data from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if ((device->ModelURL = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(device->ModelURL, tempString, tempStringLength);
			device->ModelURL[tempStringLength] = '\0';
		}
		else if (xml->NameLength == 8 && memcmp(xml->Name, "iconList", 8) == 0)
		{
			UPnPProcessDeviceXML_iconList(device, BaseURL, xml->Next);
		} 
		else if (xml->NameLength == 15 && memcmp(xml->Name, "presentationURL", 15) == 0)
		{
			//
			// Copy the presentation URL from the description document
			//
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			tempString[tempStringLength] = 0;
			tpr = ILibParseString(tempString, 0, tempStringLength, "://", 3);
			if (tpr->NumResults == 1)
			{
				// RelativeURL
				len = strlen(BaseURL);
				if (tempString[0] == '/')
				{
					if ((device->PresentationURL = (char*)malloc(1 + len + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
					memcpy(device->PresentationURL, BaseURL, len);
					memcpy(device->PresentationURL + len, tempString + 1, tempStringLength);
				}
				else
				{
					if ((device->PresentationURL = (char*)malloc(2 + len + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
					memcpy(device->PresentationURL, BaseURL, len);
					memcpy(device->PresentationURL + len, tempString, tempStringLength + 1);
				}
			}
			else
			{
				// AbsoluteURL
				if ((device->PresentationURL = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(device->PresentationURL, tempString, tempStringLength);
				device->PresentationURL[tempStringLength] = '\0';
			}
			ILibDestructParserResults(tpr);
		} else
		//{{{CustomXMLTags}}}
		if (xml->NameLength == 11 && memcmp(xml->Name, "serviceList", 11) == 0)
		{
			// Iterate through all the services contained in the serviceList element
			tempNode = xml;
			xml = xml->Next;
			while (xml != NULL)
			{
				if (xml->NameLength == 7 && memcmp(xml->Name, "service", 7) == 0)
				{
					ServiceType = NULL;
					ServiceTypeLength = 0;
					SCPDURL = NULL;
					SCPDURLLength = 0;
					EventSubURL = NULL;
					EventSubURLLength = 0;
					ControlURL = NULL;
					ControlURLLength = 0;
					ServiceMaxVersion = 1;
					
					root_a = a = ILibGetXMLAttributes(xml);
					if (a != NULL)
					{
						while (a != NULL)
						{
							a->Name[a->NameLength]=0;
							if (strcasecmp(a->Name, "MaxVersion") == 0)
							{
								a->Value[a->ValueLength]=0;
								ServiceMaxVersion = atoi(a->Value);
								break;
							}
							a = a->Next;
						}
						ILibDestructXMLAttributeList(root_a);
					}
					
					xml = xml->Next;
					flg = 0;
					while (flg == 0)
					{
						//
						// Fetch the URIs associated with this service
						//
						if (xml->NameLength == 11 && memcmp(xml->Name, "serviceType", 11) == 0)
						{
							ServiceTypeLength = ILibReadInnerXML(xml, &ServiceType);
						}
						else if (xml->NameLength == 9 && memcmp(xml->Name, "serviceId", 9) == 0)
						{
							ServiceIdLength = ILibReadInnerXML(xml, &ServiceId);
						}
						else if (xml->NameLength == 7 && memcmp(xml->Name, "SCPDURL", 7) == 0)
						{
							SCPDURLLength = ILibReadInnerXML(xml, &SCPDURL);
						}
						else if (xml->NameLength == 10 && memcmp(xml->Name, "controlURL", 10) == 0)
						{
							ControlURLLength = ILibReadInnerXML(xml, &ControlURL);
						}
						else if (xml->NameLength == 11 && memcmp(xml->Name, "eventSubURL", 11) == 0)
						{
							EventSubURLLength = ILibReadInnerXML(xml, &EventSubURL);
						}
						
						if (xml->Peer != NULL)
						{
							xml = xml->Peer;
						}
						else
						{
							flg = 1;
							xml = xml->Parent;
						}
					}
					
					// Finished Parsing the ServiceSection, build the Service
					ServiceType[ServiceTypeLength] = '\0';
					SCPDURL[SCPDURLLength] = '\0';
					EventSubURL[EventSubURLLength] = '\0';
					ControlURL[ControlURLLength] = '\0';
					
					if ((TempService = (struct UPnPService*)malloc(sizeof(struct UPnPService))) == NULL) ILIBCRITICALEXIT(254);
					TempService->SubscriptionID = NULL;
					if ((TempService->ServiceId = (char*)malloc(ServiceIdLength + 1)) == NULL) ILIBCRITICALEXIT(254);
					TempService->ServiceId[ServiceIdLength] = 0;
					memcpy(TempService->ServiceId, ServiceId, ServiceIdLength);
					
					TempService->Actions = NULL;
					TempService->Variables = NULL;
					TempService->Next = NULL;
					TempService->Parent = device;
					TempService->MaxVersion = ServiceMaxVersion;
					if (EventSubURLLength >= 7 && memcmp(EventSubURL, "http://", 6) == 0)
					{
						// Explicit
						if ((TempService->SubscriptionURL = (char*)malloc(EventSubURLLength + 1)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(TempService->SubscriptionURL, EventSubURL, EventSubURLLength);
						TempService->SubscriptionURL[EventSubURLLength] = '\0';
					}
					else
					{
						// Relative
						if (memcmp(EventSubURL, "/", 1) != 0)
						{
							if ((TempService->SubscriptionURL = (char*)malloc(EventSubURLLength + (int)strlen(BaseURL) + 1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->SubscriptionURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->SubscriptionURL+(int)strlen(BaseURL), EventSubURL, EventSubURLLength);
							TempService->SubscriptionURL[EventSubURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							if ((TempService->SubscriptionURL = (char*)malloc(EventSubURLLength + (int)strlen(BaseURL) + 1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->SubscriptionURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->SubscriptionURL+(int)strlen(BaseURL), EventSubURL+1, EventSubURLLength-1);
							TempService->SubscriptionURL[EventSubURLLength+(int)strlen(BaseURL) - 1] = '\0';
						}
					}
					if (ControlURLLength>=7 && memcmp(ControlURL, "http://", 6) == 0)
					{
						// Explicit
						if ((TempService->ControlURL = (char*)malloc(ControlURLLength + 1)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(TempService->ControlURL, ControlURL, ControlURLLength);
						TempService->ControlURL[ControlURLLength] = '\0';
					}
					else
					{
						// Relative
						if (memcmp(ControlURL, "/", 1) != 0)
						{
							if ((TempService->ControlURL = (char*)malloc(ControlURLLength + (int)strlen(BaseURL)+1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->ControlURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->ControlURL + (int)strlen(BaseURL), ControlURL, ControlURLLength);
							TempService->ControlURL[ControlURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							if ((TempService->ControlURL = (char*)malloc(ControlURLLength + (int)strlen(BaseURL)+1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->ControlURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->ControlURL + (int)strlen(BaseURL), ControlURL + 1, ControlURLLength-1);
							TempService->ControlURL[ControlURLLength+(int)strlen(BaseURL) - 1] = '\0';
						}
					}
					if (SCPDURLLength >= 7 && memcmp(SCPDURL, "http://", 6) == 0)
					{
						// Explicit
						if ((TempService->SCPDURL = (char*)malloc(SCPDURLLength+1)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(TempService->SCPDURL, SCPDURL, SCPDURLLength);
						TempService->SCPDURL[SCPDURLLength] = '\0';
					}
					else
					{
						// Relative
						if (memcmp(SCPDURL, "/", 1) != 0)
						{
							if ((TempService->SCPDURL = (char*)malloc(SCPDURLLength + (int)strlen(BaseURL) + 1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->SCPDURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->SCPDURL + (int)strlen(BaseURL), SCPDURL, SCPDURLLength);
							TempService->SCPDURL[SCPDURLLength+(int)strlen(BaseURL)] = '\0';
						}
						else
						{
							if ((TempService->SCPDURL = (char*)malloc(SCPDURLLength + (int)strlen(BaseURL) + 1)) == NULL) ILIBCRITICALEXIT(254);
							memcpy(TempService->SCPDURL, BaseURL, (int)strlen(BaseURL));
							memcpy(TempService->SCPDURL + (int)strlen(BaseURL), SCPDURL + 1, SCPDURLLength - 1);
							TempService->SCPDURL[SCPDURLLength+(int)strlen(BaseURL) - 1] = '\0';
						}
					}
					
					if ((TempService->ServiceType = (char*)malloc(ServiceTypeLength + 1)) == NULL) ILIBCRITICALEXIT(254);
					snprintf(TempService->ServiceType, ServiceTypeLength + 1, ServiceType, ServiceTypeLength);
					TempService->Next = device->Services;
					device->Services = TempService;
					
					DEBUGSTATEMENT(printf("ServiceType: %s\r\nSCPDURL: %s\r\nEventSubURL: %s\r\nControl URL: %s\r\n", ServiceType, SCPDURL, EventSubURL, ControlURL));
				}
				xml = xml->Peer;
			}
			xml = tempNode;
		} // End of ServiceList
		xml = xml->Peer;
	} // End of While
	
	return(device);
}

//
// Internal method used to parse the Device Description XML Document
//
int UPnPProcessDeviceXML(void *v_CP, char* buffer, int BufferSize, char* LocationURL, struct sockaddr *LocationAddr, struct sockaddr *RecvAddr, int Timeout)
{
	struct UPnPDevice *RootDevice = NULL;
	char* IP;
	unsigned short Port;
	char* BaseURL = NULL;
	struct ILibXMLNode *rootXML;
	struct ILibXMLNode *xml;
	char* tempString;
	int tempStringLength;
	size_t len;
	
	//
	// Parse the XML, check that it's wellformed, and build the namespace lookup table
	//
	rootXML = ILibParseXML(buffer, 0, BufferSize);
	if (ILibProcessXMLNodeList(rootXML) != 0)
	{
		ILibDestructXMLNodeList(rootXML);
		return(1);
	}
	ILibXML_BuildNamespaceLookupTable(rootXML);
	
	//
	// We need to figure out if this particular device uses
	// relative URLs using the URLBase element.
	//
	xml = rootXML;
	xml = xml->Next;
	while (xml != NULL)
	{
		if (xml->NameLength == 7 && memcmp(xml->Name, "URLBase", 7) == 0)
		{
			tempStringLength = ILibReadInnerXML(xml, &tempString);
			if (tempString[tempStringLength-1] != '/')
			{
				if ((BaseURL = (char*)malloc(2 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(BaseURL, tempString, tempStringLength);
				BaseURL[tempStringLength] = '/';
				BaseURL[tempStringLength+1] = '\0';
			}
			else
			{
				if ((BaseURL = (char*)malloc(1 + tempStringLength)) == NULL) ILIBCRITICALEXIT(254);
				memcpy(BaseURL, tempString, tempStringLength);
				BaseURL[tempStringLength] = '\0';
			}
			break;
		}
		xml = xml->Peer;
	}
	
	//
	// If the URLBase was not specified, then we need force the
	// base url to be that of the base path that we used to fetch
	// the description document from.
	//
	
	if (BaseURL == NULL)
	{
		size_t len;
		ILibParseUri(LocationURL, &IP, &Port, NULL, NULL);
		len = 18 + strlen(IP);
		if ((BaseURL = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
		snprintf(BaseURL, len, "http://%s:%d/", IP, Port);
		free(IP);
	}
	
	DEBUGSTATEMENT(printf("BaseURL: %s\r\n", BaseURL));
	
	//
	// Now that we have the base path squared away, we can actually parse this thing!
	// Let's start by looking for the device element
	//
	xml = rootXML;
	xml = xml->Next;
	while (xml != NULL && xml->NameLength != 6 && memcmp(xml->Name, "device", 6) != 0)
	{
		xml = xml->Peer;
	}
	if (xml == NULL)
	{
		// Error
		ILibDestructXMLNodeList(rootXML);
		return(1);
	}
	
	//
	// Process the Device Element. If the device element contains other devices, 
	// it will be recursively called, so we don't need to worry
	//
	RootDevice = UPnPProcessDeviceXML_device(xml, v_CP, BaseURL, LocationAddr, Timeout, RecvAddr);
	free(BaseURL);
	ILibDestructXMLNodeList(rootXML);
	
	// Save reference to LocationURL in the RootDevice
	len = strlen(LocationURL) + 1;
	if ((RootDevice->LocationURL = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	snprintf(RootDevice->LocationURL, len, "%s", LocationURL);
	memcpy(&(RootDevice->LocationAddr), LocationAddr, INET_SOCKADDR_LENGTH(LocationAddr->sa_family));
	
	//
	// Now that we processed the device XML document, we need to fetch
	// and parse the service description documents.
	//
	UPnPProcessDevice(v_CP, RootDevice);
	RootDevice->SCPDLeft = 0;
	UPnPCalculateSCPD_FetchCount(RootDevice);
	if (RootDevice->SCPDLeft == 0)
	{
		//
		// If this device doesn't contain any services, than we don't
		// need to bother with fetching any SCPD documents
		//
		UPnPFinishProcessingDevice((struct UPnPCP*)v_CP, RootDevice);
	}
	else
	{
		//
		// Fetch the SCPD documents
		//
		UPnPSCPD_Fetch(RootDevice, LocationAddr);
	}
	return 0;
}


//
// The internal sink for obtaining the Device Description Document
//
void UPnPHTTP_Sink_DeviceDescription(
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
	struct UPnPCP* CP = (struct UPnPCP*)cp;
	
	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( IsInterrupt );
	UNREFERENCED_PARAMETER( PAUSE );

	if (done != 0)
	{
		ILibDeleteEntry(CP->DeviceTable_Tokens, customData->UDN, (int)strlen(customData->UDN));
		ILibDeleteEntry(CP->DeviceTable_URI, customData->LocationURL, (int)strlen(customData->LocationURL));
	}
	
	if (header != NULL && ILibWebClientIsStatusOk(header->StatusCode) && done != 0 && EndPointer > 0)
	{
		if (UPnPProcessDeviceXML(cp, buffer, EndPointer-(*p_BeginPointer), customData->buffer, (struct sockaddr*)&(customData->LocationAddr), (struct sockaddr*)(header->ReceivingAddress), customData->Timeout) != 0)
		{
			ILibDeleteEntry(CP->DeviceTable_UDN, customData->UDN, (int)strlen(customData->UDN));
			ILibDeleteEntry(CP->DeviceTable_URI, customData->LocationURL, (int)strlen(customData->LocationURL));
		}
	}
	else if ((header == NULL) || (header != NULL && !ILibWebClientIsStatusOk(header->StatusCode)))
	{
		if (done != 0 && CP->ErrorDispatch != NULL)
		{
			CP->ErrorDispatch(customData->UDN, customData->LocationURL, header != NULL?header->StatusCode:-1);
		}
		ILibDeleteEntry(CP->DeviceTable_UDN, customData->UDN, (int)strlen(customData->UDN));
		ILibDeleteEntry(CP->DeviceTable_URI, customData->LocationURL, (int)strlen(customData->LocationURL));
	}
	
	if (done != 0)
	{
		free(customData->buffer);
		free(customData->UDN);
		free(customData->LocationURL);
		free(user);
	}
}

void UPnP_FlushRequest(struct UPnPDevice *device)
{
	struct UPnPDevice *ed = device->EmbeddedDevices;
	struct UPnPService *s = device->Services;
	
	while (ed != NULL)
	{
		UPnP_FlushRequest(ed);
		ed = ed->Next;
	}
	while (s != NULL)
	{
		s = s->Next;
	}
}

//
// An internal method used to recursively release all the references to a device.
// While doing this, we'll also check to see if we gave any of these to the app layer
// above, and if so, trigger a removal event.
//
void UPnPCP_RecursiveReleaseAndEventDevice(struct UPnPCP* CP, struct UPnPDevice *device)
{
	struct UPnPDevice *temp = device->EmbeddedDevices;
	
	ILibDeleteEntry(CP->DeviceTable_URI, device->LocationURL, (int)strlen(device->LocationURL));
	ILibAddEntry(CP->DeviceTable_UDN, device->UDN, (int)strlen(device->UDN), NULL);
	
	while (temp != NULL)
	{
		UPnPCP_RecursiveReleaseAndEventDevice(CP, temp);
		temp = temp->Next;
	}
	
	if (device->Reserved != 0)
	{
		//
		// We gave this to the app layer above
		//
		if (CP->RemoveSink != NULL)
		{
			CP->RemoveSink(device);
		}
		UPnPRelease(device);
	}
}
void UPnPCP_ProcessDeviceRemoval(struct UPnPCP* CP, struct UPnPDevice *device)
{
	struct UPnPDevice *temp = device->EmbeddedDevices;
	struct UPnPService *s;
	struct UPnPDevice *dTemp = device;
	
	if (dTemp->Parent != NULL)
	{
		dTemp = dTemp->Parent;
	}
	ILibLifeTime_Remove(CP->LifeTimeMonitor, dTemp);
	
	while (temp != NULL)
	{
		UPnPCP_ProcessDeviceRemoval(CP, temp);
		temp = temp->Next;
	}
	
	s = device->Services;
	while (s != NULL)
	{
		// Remove all the pending requests
		ILibWebClient_DeleteRequests(((struct UPnPCP*)device->CP)->HTTP, (struct sockaddr*)&(s->Parent->LocationAddr));
		
		ILibLifeTime_Remove(CP->LifeTimeMonitor, s);
		s = s->Next;
	}
	
	if (device->Reserved != 0)
	{
		// Device was flagged, and given to the user
		if (CP->RemoveSink != NULL) CP->RemoveSink(device);
		UPnPRelease(device);
	}
	
	ILibHashTree_Lock(CP->DeviceTable_UDN);
	ILibDeleteEntry(CP->DeviceTable_UDN, device->UDN, (int)strlen(device->UDN));
	if (device->LocationURL != NULL)
	{
		ILibDeleteEntry(CP->DeviceTable_URI, device->LocationURL, (int)strlen(device->LocationURL));
	}
	ILibHashTree_UnLock(CP->DeviceTable_UDN);
}

//
// The internal sink called by our SSDP Module
//
void UPnPSSDP_Sink(void *sender, char* UDN, int Alive, char* LocationURL, struct sockaddr* LocationAddr, int Timeout, UPnPSSDP_MESSAGE m, void *cp)
{
	int i = 0;
	char* buffer;
	char* IP;
	char* Path;
	unsigned short Port;
	struct packetheader *p;
	struct UPnPDevice *device;
	struct CustomUserData *customData;
	struct UPnPCP *CP = (struct UPnPCP*)cp;
	struct timeval t;
	size_t len;
	ILibWebClient_RequestToken RT;
	void *v;

	UNREFERENCED_PARAMETER( sender );
	
	if (Alive != 0)
	{
		// Hello
		
		// A device should never advertise it's timeout value being zero. But if
		// it does, let's not waste time processing stuff
		if (Timeout == 0) {return;}
		DEBUGSTATEMENT(printf("Hello, LocationURL: %s\r\n", LocationURL));
		
		// Lock the table
		ILibHashTree_Lock(CP->DeviceTable_UDN);
		
		// We have never seen this location URL, nor have we ever seen this UDN before, 
		// so let's try and find it
		if (ILibHasEntry(CP->DeviceTable_URI, LocationURL, (int)strlen(LocationURL)) == 0 && ILibHasEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN)) == 0)
		{
			// Parse the location uri of the device description document, 
			// and build the request packet
			ILibParseUri(LocationURL, &IP, &Port, &Path, NULL);
			DEBUGSTATEMENT(printf("IP: %s Port: %d Path: %s\r\n", IP, Port, Path));
			p = UPnPBuildPacket(IP, Port, Path, "GET");

			len = strlen(LocationURL) + 1;
			if ((buffer = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(buffer, LocationURL, len);
			
			if ((customData = (struct CustomUserData*)malloc(sizeof(struct CustomUserData))) == NULL) ILIBCRITICALEXIT(254);
			customData->Timeout = Timeout;
			customData->buffer = buffer;
			memcpy(&(customData->LocationAddr), LocationAddr, INET_SOCKADDR_LENGTH(LocationAddr->sa_family));
			if (customData->LocationAddr.sin6_family == AF_INET6) customData->LocationAddr.sin6_port = htons(Port); else ((struct sockaddr_in*)&(customData->LocationAddr))->sin_port = htons(Port);

			len = strlen(LocationURL) + 1;
			if ((customData->LocationURL = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(customData->LocationURL, LocationURL, len);
			len = strlen(UDN) + 1;
			if ((customData->UDN = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
			memcpy(customData->UDN, UDN, len);
			
			// Add these items into our table, so we don't try to find it multiple times
			ILibAddEntry(CP->DeviceTable_URI, LocationURL, (int)strlen(LocationURL), customData->UDN);
			ILibAddEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN), NULL);
			
			// Make the HTTP request to fetch the Device Description Document
			RT = ILibWebClient_PipelineRequest(
				((struct UPnPCP*)cp)->HTTP,
				(struct sockaddr*)&(customData->LocationAddr),
				p,
				&UPnPHTTP_Sink_DeviceDescription,
				customData,
				cp);
			
			free(IP);
			free(Path);
			
			ILibAddEntry(CP->DeviceTable_Tokens, UDN, (int)strlen(UDN), RT);
		}
		else
		{
			// We have seen this device before, so thse packets are
			// Periodic Notify Packets
			
			// Fetch the device, this packet is advertising
			device = (struct UPnPDevice*)ILibGetEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN));
			if (device != NULL && device->ReservedID == 0 && m == UPnPSSDP_NOTIFY)
			{
				// Get the root device
				while (device->Parent != NULL)
				{
					device = device->Parent;
				}
				
				// Is this device on the same IP Address?
				if (strcmp(device->LocationURL, LocationURL) == 0)
				{
					// Extend LifetimeMonitor duration
					gettimeofday(&t, NULL);
					device->Reserved2 = t.tv_sec;
					ILibLifeTime_Remove(((struct UPnPCP*)cp)->LifeTimeMonitor, device);
					ILibLifeTime_Add(((struct UPnPCP*)cp)->LifeTimeMonitor, device, Timeout, (ILibLifeTime_OnCallback)&UPnPExpiredDevice, NULL);
				}
				else
				{
					// Same device, different Interface
					// Wait up to 7 seconds to see if the old interface is still valid.
					gettimeofday(&t, NULL);
					if (t.tv_sec-device->Reserved2>10)
					{
						device->Reserved2 = t.tv_sec;
						if (device->Reserved3 != NULL) { free(device->Reserved3); }
						len = strlen(LocationURL) + 1;
						if ((device->Reserved3 = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(device->Reserved3, LocationURL, len);

						ILibParseUri(LocationURL, NULL, &Port, NULL, NULL);
						memcpy(&(device->LocationAddr), LocationAddr, INET_SOCKADDR_LENGTH(LocationAddr->sa_family));
						if (device->LocationAddr.sin6_family == AF_INET6) device->LocationAddr.sin6_port = htons(Port); else ((struct sockaddr_in*)&(device->LocationAddr))->sin_port = htons(Port);

						ILibLifeTime_Remove(((struct UPnPCP*)cp)->LifeTimeMonitor, device);
						ILibLifeTime_Add(((struct UPnPCP*)cp)->LifeTimeMonitor, device, 7, (ILibLifeTime_OnCallback)&UPnPInterfaceChanged, NULL);
					}
				}
			}
		}
		ILibHashTree_UnLock(CP->DeviceTable_UDN);
	}
	else
	{
		// Bye Bye
		ILibHashTree_Lock(CP->DeviceTable_UDN);
		
		v = ILibGetEntry(CP->DeviceTable_Tokens, UDN, (int)strlen(UDN));
		if (v != NULL)
		{
			ILibWebClient_CancelRequest((ILibWebClient_RequestToken)v);
			ILibDeleteEntry(CP->DeviceTable_Tokens, UDN, (int)strlen(UDN));
			ILibDeleteEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN));
		}
		device = (struct UPnPDevice*)ILibGetEntry(CP->DeviceTable_UDN, UDN, (int)strlen(UDN));
		
		// Find the device that is going away
		ILibHashTree_UnLock(CP->DeviceTable_UDN);
		
		if (device != NULL)
		{
			// Get the root device
			while (device->Parent != NULL) device = device->Parent;

			// Remove the timed event, checking the refreshing of notify packets
			ILibLifeTime_Remove(((struct UPnPCP*)cp)->LifeTimeMonitor, device);
			UPnPCP_ProcessDeviceRemoval(CP, device);

			// If the app above subscribed to events, there will be extra references
			// that we can delete, otherwise, the device ain't ever going away
			i = device->ReferenceTiedToEvents;
			while (i != 0)
			{
				UPnPRelease(device);
				--i;
			}
			UPnPRelease(device);
		}
	}
}

//{{{Service_EventSink}}}

//
// Internal HTTP Sink, called when an event is delivered
//
void UPnPOnEventSink(
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
	
	//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	char *txt;
	if (header != NULL && sender->User3 == NULL && done == 0)
	{
		sender->User3 = (void*)~0;
		txt = ILibGetHeaderLine(header, "Expect", 6);
		if (txt != NULL)
		{
			if (strcasecmp(txt, "100-Continue") == 0)
			{
				// Expect Continue
				ILibWebServer_Send_Raw(sender, "HTTP/1.1 100 Continue\r\n\r\n", 25, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
			else
			{
				// Don't understand
				ILibWebServer_Send_Raw(sender, "HTTP/1.1 417 Expectation Failed\r\n\r\n", 35, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
				ILibWebServer_DisconnectSession(sender);
				return;
			}
		}
	}
	//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}

	UNREFERENCED_PARAMETER( InterruptFlag );
	UNREFERENCED_PARAMETER( BeginPointer );
	
	if (done != 0)
	{
		// We recieved the event, let's prepare the response
		resp = ILibCreateEmptyPacket();
		ILibSetVersion(resp, "1.1", 3);
		ILibAddHeaderLine(resp, "Server", 6, UPnPPLATFORM, (int)strlen(UPnPPLATFORM));
		ILibAddHeaderLine(resp, "Content-Length", 14, "0", 1);
		field = header->FirstField;
		while (field != NULL)
		{
			if (field->FieldLength == 3)
			{
				if (strncasecmp(field->Field, "SID", 3) == 0)
				{
					// We need to determine who this event is for, by looking at the subscription id
					if ((sid = (char*)malloc(field->FieldDataLength + 1)) == NULL) ILIBCRITICALEXIT(254);
					snprintf(sid, field->FieldDataLength + 1, "%s", field->FieldData);
					
					// Do we know about this SID?
					value = ILibGetEntry(((struct UPnPCP*)sender->User)->SIDTable, field->FieldData, field->FieldDataLength);
					break;
				}
			}
			field = field->NextField;
		}
		
		if (value == NULL)
		{
			// Not a valid SID
			ILibSetStatusCode(resp, 412, "Failed", 6);
		}
		else
		{
			ILibSetStatusCode(resp, 200, "OK", 2);
			service = (struct UPnPService*)value;
			
			type_length = (int)strlen(service->ServiceType);
			//{{{EventSink_IfStatement}}}
		}
		ILibWebServer_Send(sender, resp);
		if (sid != NULL){free(sid);}
	}
}

//
// Internal sink called when our attempt to unregister for events
// has gone through
//
void UPnPOnUnSubscribeSink(
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
	//struct UPnPCP *cp = (struct UPnPCP*)vcp;
	
	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( IsInterrupt );
	UNREFERENCED_PARAMETER( buffer );
	UNREFERENCED_PARAMETER( BeginPointer );
	UNREFERENCED_PARAMETER( BufferSize );
	UNREFERENCED_PARAMETER( vcp );
	UNREFERENCED_PARAMETER( PAUSE );

	if (done != 0)
	{
		s = (struct UPnPService*)user;
		if (header != NULL)
		{
			if (ILibWebClientIsStatusOk(header->StatusCode))
			{
				// Successful
			}
		}
		UPnPRelease(s->Parent);
	}
}


//
// Internal sink called when our attempt to register for events
// has gone through
//
void UPnPOnSubscribeSink(
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
	struct UPnPCP *cp = (struct UPnPCP*)vcp;
	size_t len;
	
	UNREFERENCED_PARAMETER( WebReaderToken );
	UNREFERENCED_PARAMETER( IsInterrupt );
	UNREFERENCED_PARAMETER( buffer );
	UNREFERENCED_PARAMETER( BeginPointer );
	UNREFERENCED_PARAMETER( BufferSize );
	UNREFERENCED_PARAMETER( PAUSE );

	if (done != 0)
	{
		s = (struct UPnPService*)user;
		if (header != NULL)
		{
			if (ILibWebClientIsStatusOk(header->StatusCode))
			{
				// Successful
				field = header->FirstField;
				while (field != NULL)
				{
					if (field->FieldLength == 3 && strncasecmp(field->Field, "SID", 3) == 0 && s->SubscriptionID == NULL)
					{
						//
						// Determine what subscription id was assigned to us
						//
						len = 1 + field->FieldDataLength;
						if ((s->SubscriptionID = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
						memcpy(s->SubscriptionID, field->FieldData, len);
						//
						// Make a mapping from this SID to our service, to make our lives easier
						//
						ILibAddEntry(cp->SIDTable, field->FieldData, field->FieldDataLength, s);
					}
					else if (field->FieldLength == 7 && strncasecmp(field->Field, "TIMEOUT", 7) == 0)
					{
						//
						// Determine what refresh cycle the device wants us to enforce
						//
						p = ILibParseString(field->FieldData, 0, field->FieldDataLength, "-", 1);
						p->LastResult->data[p->LastResult->datalength] = '\0';
						UPnPAddRef(s->Parent);
						d = s->Parent;
						while (d->Parent != NULL) {d = d->Parent;}
						++d->ReferenceTiedToEvents;
						ILibLifeTime_Add(cp->LifeTimeMonitor, s, atoi(p->LastResult->data)/2, (ILibLifeTime_OnCallback)&UPnPRenew, NULL);
						ILibDestructParserResults(p);
					}
					field = field->NextField;
				}
			}
		}
		UPnPRelease(s->Parent);
	}
}

//
// Internal Method used to renew our event subscription with a device
//
void UPnPRenew(void *state)
{
	struct UPnPService *service = (struct UPnPService*)state;
	struct UPnPDevice *d = service->Parent;
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	size_t len;
	
	//
	// Determine where this renewal should go
	//
	ILibParseUri(service->SubscriptionURL, &IP, &Port, &Path, NULL);
	p = ILibCreateEmptyPacket();
	ILibSetVersion(p, "1.1", 3);
	
	ILibSetDirective(p, "SUBSCRIBE", 9, Path, (int)strlen(Path));
	
	len = (int)strlen(IP)+7;
	if ((TempString = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	snprintf(TempString, len, "%s:%d", IP, Port);
	
	ILibAddHeaderLine(p, "HOST", 4, TempString, (int)strlen(TempString));
	free(TempString);
	
	ILibAddHeaderLine(p, "SID", 3, service->SubscriptionID, (int)strlen(service->SubscriptionID));
	ILibAddHeaderLine(p, "TIMEOUT", 7, "Second-180", 10);
	ILibAddHeaderLine(p, "User-Agent", 10, UPnPPLATFORM, (int)strlen(UPnPPLATFORM));
	
	//
	// Try to refresh our subscription
	//
	//ILibWebClient_SetQosForNextRequest(((struct UPnPCP*)service->Parent->CP)->HTTP, UPnPInvocationPriorityLevel);
	ILibWebClient_PipelineRequest(
		((struct UPnPCP*)service->Parent->CP)->HTTP, 
		(struct sockaddr*)&(service->Parent->LocationAddr), 
		p, 
		&UPnPOnSubscribeSink, 
		(void*)service, service->Parent->CP);
	
	while (d->Parent != NULL) {d = d->Parent;}
	--d->ReferenceTiedToEvents;
	free(IP);
	free(Path);
}

struct UPnPDevice* UPnPGetDeviceEx(struct UPnPDevice *device, char* DeviceType, int counter, int number)
{
	struct UPnPDevice *RetVal = NULL;
	struct UPnPDevice *d = device->EmbeddedDevices;
	struct parser_result *pr, *pr2;
	int DeviceTypeLength = (int)strlen(DeviceType);
	int TempLength = (int)strlen(device->DeviceType);
	
	while (d != NULL && RetVal == NULL)
	{
		RetVal = UPnPGetDeviceEx(d, DeviceType, counter, number);
		d = d->Next;
	}
	
	if (RetVal == NULL)
	{
		pr = ILibParseString(DeviceType, 0, DeviceTypeLength, ":", 1);
		pr2 = ILibParseString(device->DeviceType, 0, TempLength, ":", 1);
		
		if (DeviceTypeLength-pr->LastResult->datalength == TempLength - pr2->LastResult->datalength && atoi(pr->LastResult->data) >= atoi(pr2->LastResult->data) && memcmp(DeviceType, device->DeviceType, DeviceTypeLength-pr->LastResult->datalength) == 0)
		{
			ILibDestructParserResults(pr);
			ILibDestructParserResults(pr2);
			if (number == (++counter)) return(device);
		}
		ILibDestructParserResults(pr);
		ILibDestructParserResults(pr2);
		return(NULL);
	}
	else
	{
		return(RetVal);
	}
}

/*! \fn UPnPHasAction(struct UPnPService *s, char* action)
\brief Determines if an action exists on a service
\param s UPnP service to query
\param action action name
\returns Non-zero if it exists
*/
int UPnPHasAction(struct UPnPService *s, char* action)
{
	struct UPnPAction *a = s->Actions;
	
	while (a != NULL)
	{
		if (strcmp(action, a->Name) == 0) return(-1);
		a = a->Next;
	}
	return(0);
}

//
// Internal Trigger called when the chain is cleaning up
//
void UPnPStopCP(void *v_CP)
{
	int i;
	struct UPnPDevice *Device;
	struct UPnPCP *CP= (struct UPnPCP*)v_CP;
	void *en;
	char *key;
	int keyLength;
	void *data;
	
	
	en = ILibHashTree_GetEnumerator(CP->DeviceTable_UDN);
	while (ILibHashTree_MoveNext(en) == 0)
	{
		ILibHashTree_GetValue(en, &key, &keyLength, &data);
		if (data != NULL)
		{
			// This is a UPnPDevice
			Device = (struct UPnPDevice*)data;
			if (Device->ReservedID == 0 && Device->Parent == NULL) // This is a UPnPDevice if ReservedID == 0
			{
				// This is the Root Device (Which is only in the table once)
				UPnPCP_RecursiveReleaseAndEventDevice(CP, Device);
				i = Device->ReferenceTiedToEvents;
				while (i != 0)
				{
					UPnPRelease(Device);
					--i;
				}
				UPnPRelease(Device);
			}
		}
	}
	ILibHashTree_DestroyEnumerator(en);
	ILibDestroyHashTree(CP->SIDTable);
	ILibDestroyHashTree(CP->DeviceTable_UDN);
	ILibDestroyHashTree(CP->DeviceTable_URI);
	ILibDestroyHashTree(CP->DeviceTable_Tokens);
	
	if (CP->IPAddressListV4 != NULL) { free(CP->IPAddressListV4); CP->IPAddressListV4 = NULL; }
	if (CP->IPAddressListV6 != NULL) { free(CP->IPAddressListV6); CP->IPAddressListV6 = NULL; }
	
	sem_destroy(&(CP->DeviceLock));
}

/*! \fn UPnP_CP_IPAddressListChanged(void *CPToken)
\brief Notifies the underlying microstack that one of the ip addresses may have changed
\param CPToken Control Point Token
*/
void UPnP_CP_IPAddressListChanged(void *CPToken)
{
	if (ILibIsChainBeingDestroyed(((struct UPnPCP*)CPToken)->Chain) == 0)
	{
		((struct UPnPCP*)CPToken)->RecheckFlag = 1;
		ILibForceUnBlockChain(((struct UPnPCP*)CPToken)->Chain);
	}
}

void UPnPCP_PreSelect(void *CPToken, fd_set *readset, fd_set *writeset, fd_set *errorset, int *blocktime)
{
	struct UPnPCP *CP = (struct UPnPCP*)CPToken;
	void *en;
	
	struct UPnPDevice *device;
	char *key;
	int keyLength;
	void *data;
	void *q;
	int found;
	
	// Local Address Lists
	struct sockaddr_in *IPAddressListV4 = NULL;
	int IPAddressListV4Length;
	struct sockaddr_in6 *IPAddressListV6 = NULL;
	int IPAddressListV6Length;
	
	UNREFERENCED_PARAMETER( readset );
	UNREFERENCED_PARAMETER( writeset );
	UNREFERENCED_PARAMETER( errorset );
	UNREFERENCED_PARAMETER( blocktime );

	//
	// Do we need to recheck IP Addresses?
	//
	if (CP->RecheckFlag != 0)
	{
		CP->RecheckFlag = 0;
		
		//
		// Get the current IP Address list
		//
		IPAddressListV4Length = ILibGetLocalIPv4AddressList(&(IPAddressListV4), 1);
		IPAddressListV6Length = ILibGetLocalIPv6List(&(IPAddressListV6));

		//
		// Create a Queue, to add devices that need to be removed
		//
		q = ILibQueue_Create();
		
		//
		// Iterate through all the devices we are aware of
		//
		ILibHashTree_Lock(CP->DeviceTable_UDN);
		en = ILibHashTree_GetEnumerator(CP->DeviceTable_UDN);
		while (ILibHashTree_MoveNext(en) == 0)
		{
			ILibHashTree_GetValue(en, &key, &keyLength, &data);
			if (data != NULL)
			{
				// This is a UPnP Device
				device = (struct UPnPDevice*)data;
				if (device->ReservedID == 0 && device->Parent == NULL)
				{
					int i;
					// This is the root device, which is in the table exactly once
					
					// Iterate through all the current IP addresses, and check to 
					// see if there are any devices that aren't on one of these
					found = 0;
					if (device->InterfaceToHostAddr.sin6_family == AF_INET6)
					{
						// IPv6 Search
						for (i = 0; i < IPAddressListV6Length; ++i)
						{
							if (memcmp(&(IPAddressListV6[i]), device->InterfaceToHost, INET_SOCKADDR_LENGTH(AF_INET6)) == 0) { found = 1; break; }
						}
					}
					else
					{
						// IPv4 Search
						for (i = 0; i < IPAddressListV4Length; ++i)
						{
							if (memcmp(&(IPAddressListV4[i]), device->InterfaceToHost, INET_SOCKADDR_LENGTH(AF_INET)) == 0) { found = 1; break; }
						}
					}

					// If the device wasn't bound to any of the current IP addresses, than
					// it is no longer reachable, so we should get rid of it, and hope
					// to find it later
					if (found == 0)
					{
						// Queue Device to be removed, so we can process it outside of the lock
						ILibQueue_EnQueue(q, device);
					}
				}
			}
		}
		
		ILibHashTree_DestroyEnumerator(en);
		ILibHashTree_UnLock(CP->DeviceTable_UDN);
		
		//
		// Get rid of the devices that are no longer reachable
		//
		while (ILibQueue_PeekQueue(q) != NULL)
		{
			device = (struct UPnPDevice*)ILibQueue_DeQueue(q);
			UPnPCP_ProcessDeviceRemoval(CP, device);
			UPnPRelease(device);
		}
		ILibQueue_Destroy(q);
		ILibSSDP_IPAddressListChanged(CP->SSDP);

		if (CP->IPAddressListV4 != NULL) { free(CP->IPAddressListV4); CP->IPAddressListV4 = NULL; }
		if (CP->IPAddressListV6 != NULL) { free(CP->IPAddressListV6); CP->IPAddressListV6 = NULL; }

		CP->IPAddressListV4Length = IPAddressListV4Length;
		CP->IPAddressListV4 = IPAddressListV4;
		CP->IPAddressListV6Length = IPAddressListV6Length;
		CP->IPAddressListV6 = IPAddressListV6;
	}
}
void UPnPOnSessionSink(struct ILibWebServer_Session *session, void *User)
{
	session->OnReceive = &UPnPOnEventSink;
	session->User = User;
}


#ifdef UPNP_DEBUG

void UPnPDebug(struct ILibWebServer_Session *sender, struct UPnPDevice *dv)
{
	char tmp[25];
	
	ILibWebServer_StreamBody(sender, "<b>", 3, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, dv->FriendlyName, (int)strlen(dv->FriendlyName), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "</b>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	ILibWebServer_StreamBody(sender, "<br>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	ILibWebServer_StreamBody(sender, "  LocationURL: <A HREF=\"", 24, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, dv->LocationURL, (int)strlen(dv->LocationURL), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "\">", 2, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, dv->LocationURL, (int)strlen(dv->LocationURL), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "</A>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	ILibWebServer_StreamBody(sender, "<br>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, "  UDN: <A HREF=\"/UDN/", 21, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, dv->UDN, (int)strlen(dv->UDN), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "\">", 2, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	ILibWebServer_StreamBody(sender, dv->UDN, (int)strlen(dv->UDN), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "</A>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	
	while (dv->Parent != NULL)
	{
		dv = dv->Parent;
	}
	ILibWebServer_StreamBody(sender, "<br><i>  Reference Counter: ", 28, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	sprintf(tmp, "%d", dv->ReferenceCount);
	ILibWebServer_StreamBody(sender, tmp, (int)strlen(tmp), ILibAsyncSocket_MemoryOwnership_USER, 0);
	ILibWebServer_StreamBody(sender, "</i>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	
	
	ILibWebServer_StreamBody(sender, "<br><br>", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
}
void UPnPDebugOnQueryWCDO(struct ILibWebServer_Session *sender, char *w)
{
	struct UPnPCP *cp = (struct UPnPCP*)sender->User3;
	struct packetheader *p = ILibCreateEmptyPacket();
	char *t;
	
	ILibSetStatusCode(p, 200, "OK", 2);
	ILibSetVersion(p, "1.1", 3);
	ILibAddHeaderLine(p, "Content-Type", 12, "text/html", 9);
	ILibWebServer_StreamHeader(sender, p);
	
	ILibWebServer_StreamBody(sender, "<HTML>", 6, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	if (w != NULL)
	{
		t = ILibWebClient_QueryWCDO(cp->HTTP, w);
		ILibWebServer_StreamBody(sender, t, (int)strlen(t), ILibAsyncSocket_MemoryOwnership_CHAIN, 0);
	}
	
	ILibWebServer_StreamBody(sender, "</HTML>", 7, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
}
void UPnPDebugOnQuery(struct ILibWebServer_Session *sender, char *UDN, char *URI, char *TOK)
{
	struct UPnPCP *cp = (struct UPnPCP*)sender->User3;
	struct packetheader *p = ILibCreateEmptyPacket();
	void *en;
	char *key;
	int keyLen;
	void *data;
	int rmv = 0;
	
	char *tmp;
	
	struct UPnPDevice *dv;
	
	ILibSetStatusCode(p, 200, "OK", 2);
	ILibSetVersion(p, "1.1", 3);
	ILibAddHeaderLine(p, "Content-Type", 12, "text/html", 9);
	ILibWebServer_StreamHeader(sender, p);
	
	ILibWebServer_StreamBody(sender, "<HTML>", 6, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
	
	ILibHashTree_Lock(cp->DeviceTable_UDN);
	
	if (UDN != NULL && stricmp(UDN, "*") == 0)
	{
		// Look in the DeviceTable_UDN
		
		en = ILibHashTree_GetEnumerator(cp->DeviceTable_UDN);
		while (ILibHashTree_MoveNext(en) == 0)
		{
			ILibHashTree_GetValue(en, &key, &keyLen, &data);
			if (data != NULL)
			{
				dv = (struct UPnPDevice*)data;
				UPnPDebug(sender, dv);
			}
			else
			{
				if ((tmp = (char*)malloc(keyLen+1)) == NULL) ILIBCRITICALEXIT(254);
				tmp[keyLen]=0;
				memcpy(tmp, key, keyLen);
				ILibWebServer_StreamBody(sender, "<b>UDN: ", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				ILibWebServer_StreamBody(sender, tmp, keyLen, ILibAsyncSocket_MemoryOwnership_CHAIN, 0);
				ILibWebServer_StreamBody(sender, "</b><br><i>Not created yet</i><br>", 34, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
		}
		
		ILibHashTree_DestroyEnumerator(en);
	}
	else if (UDN != NULL)
	{
		if (memcmp(UDN, "K", 1) == 0)
		{
			rmv=1;
			UDN = UDN+1;
		}
		dv = (struct UPnPDevice*)ILibGetEntry(cp->DeviceTable_UDN, UDN, (int)strlen(UDN));
		if (ILibHasEntry(cp->DeviceTable_UDN, UDN, (int)strlen(UDN)) == 0)
		{
			ILibWebServer_StreamBody(sender, "<b>NOT FOUND</b>", 16, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
		}
		else
		{
			if (dv == NULL)
			{
				ILibWebServer_StreamBody(sender, "<b>UDN exists, but device not created yet.</b><br>", 50, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
			else
			{
				UPnPDebug(sender, dv);
			}
			if (rmv)
			{
				ILibDeleteEntry(cp->DeviceTable_UDN, UDN, (int)strlen(UDN));
				ILibWebServer_StreamBody(sender, "<i>DELETED</i>", 14, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
		}
	}
	else if (URI != NULL)
	{
		if (stricmp(URI, "*") != 0)
		{
			if (memcmp(URI, "K", 1) == 0)
			{
				rmv = 1;
				URI = URI+1;
			}
			key = (char*)ILibGetEntry(cp->DeviceTable_URI, URI, (int)strlen(URI));
			if (key == NULL)
			{
				ILibWebServer_StreamBody(sender, "<b>NOT FOUND</b>", 16, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
			else
			{
				ILibWebServer_StreamBody(sender, "<b>UDN: ", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				ILibWebServer_StreamBody(sender, key, (int)strlen(key), ILibAsyncSocket_MemoryOwnership_USER, 0);
				ILibWebServer_StreamBody(sender, "</b><br>", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				if (rmv)
				{
					ILibDeleteEntry(cp->DeviceTable_URI, URI, (int)strlen(URI));
					ILibWebServer_StreamBody(sender, "<i>DELETED</i>", 14, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				}
			}
		}
		else
		{
			en = ILibHashTree_GetEnumerator(cp->DeviceTable_URI);
			while (ILibHashTree_MoveNext(en) == 0)
			{
				ILibHashTree_GetValue(en, &key, &keyLen, &data);
				ILibWebServer_StreamBody(sender, "<b>URI: ", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				ILibWebServer_StreamBody(sender, key, (int)strlen(key), ILibAsyncSocket_MemoryOwnership_USER, 0);
				ILibWebServer_StreamBody(sender, "</b><br>", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				if (data == NULL)
				{
					ILibWebServer_StreamBody(sender, "<i>No UDN</i><br>", 17, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				}
				else
				{
					ILibWebServer_StreamBody(sender, "UDN: ", 5, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
					ILibWebServer_StreamBody(sender, (char*)data, (int)strlen((char*)data), ILibAsyncSocket_MemoryOwnership_USER, 0);
					ILibWebServer_StreamBody(sender, "<br>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				}
				ILibWebServer_StreamBody(sender, "<br>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
			ILibHashTree_DestroyEnumerator(en);
		}
	}
	else if (TOK != NULL)
	{
		if (stricmp(TOK, "*") != 0)
		{
			key = (char*)ILibGetEntry(cp->DeviceTable_Tokens, TOK, (int)strlen(TOK));
			if (key == NULL)
			{
				ILibWebServer_StreamBody(sender, "<b>NOT FOUND</b>", 16, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
			else
			{
				ILibWebServer_StreamBody(sender, "<i>Outstanding Requests</i><br>", 31, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
		}
		else
		{
			en = ILibHashTree_GetEnumerator(cp->DeviceTable_URI);
			while (ILibHashTree_MoveNext(en) == 0)
			{
				ILibHashTree_GetValue(en, &key, &keyLen, &data);
				ILibWebServer_StreamBody(sender, "<b>UDN: ", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				ILibWebServer_StreamBody(sender, key, (int)strlen(key), ILibAsyncSocket_MemoryOwnership_USER, 0);
				ILibWebServer_StreamBody(sender, "</b><br>", 8, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				if (data == NULL)
				{
					ILibWebServer_StreamBody(sender, "<i>No Tokens?</i><br>", 21, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				}
				else
				{
					ILibWebServer_StreamBody(sender, "<i>Outstanding Requests</i><br>", 31, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
				}
				ILibWebServer_StreamBody(sender, "<br>", 4, ILibAsyncSocket_MemoryOwnership_STATIC, 0);
			}
		}
	}
	
	
	ILibHashTree_UnLock(cp->DeviceTable_UDN);
	ILibWebServer_StreamBody(sender, "</HTML>", 7, ILibAsyncSocket_MemoryOwnership_STATIC, 1);
}
void UPnPOnDebugReceive(struct ILibWebServer_Session *sender, int InterruptFlag, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done)
{
	struct packetheader *r = NULL;
	if (!done){return;}
	
	header->DirectiveObj[header->DirectiveObjLength]=0;
	header->Directive[header->DirectiveLength]=0;
	
	if (stricmp(header->Directive, "GET") == 0)
	{
		if (memcmp(header->DirectiveObj, "/UDN/", 5) == 0)
		{
			UPnPDebugOnQuery(sender, header->DirectiveObj+5, NULL, NULL);
		}
		else if (memcmp(header->DirectiveObj, "/URI/", 5) == 0)
		{
			UPnPDebugOnQuery(sender, NULL, header->DirectiveObj+5, NULL);
		}
		else if (memcmp(header->DirectiveObj, "/TOK/", 5) == 0)
		{
			UPnPDebugOnQuery(sender, NULL, NULL, header->DirectiveObj+5);
		}
		else if (memcmp(header->DirectiveObj, "/WCDO/", 6) == 0)
		{
			UPnPDebugOnQueryWCDO(sender, header->DirectiveObj+6);
		}
		else
		{
			r = ILibCreateEmptyPacket();
			ILibSetStatusCode(r, 404, "Bad Request", 11);
			ILibWebServer_Send(sender, r);
		}
	}
}

void UPnPOnDebugSessionSink(struct ILibWebServer_Session *sender, void *user)
{
	sender->OnReceive = &UPnPOnDebugReceive;
	sender->User3 = user;
}
#endif
void UPnPControlPoint_AddDiscoveryErrorHandler(void *cpToken, UPnPDeviceDiscoveryErrorHandler callback)
{
	struct UPnPCP *cp = (struct UPnPCP*)cpToken;
	cp->ErrorDispatch = callback;
}
/*! \fn UPnPCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*), void(*R)(struct UPnPDevice*))
\brief Initalizes the control point
\param Chain The chain to attach this CP to
\param A AddSink Function Pointer
\param R RemoveSink Function Pointer
\returns ControlPoint Token
*/
void *UPnPCreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*), void(*R)(struct UPnPDevice*))
{
	struct UPnPCP *cp;
	if ((cp = (struct UPnPCP*)malloc(sizeof(struct UPnPCP))) == NULL) ILIBCRITICALEXIT(254);
	
	memset(cp, 0, sizeof(struct UPnPCP));
	cp->Destroy = &UPnPStopCP;
	cp->PostSelect = NULL;
	cp->PreSelect = &UPnPCP_PreSelect;
	cp->DiscoverSink = A;
	cp->RemoveSink = R;
	
	sem_init(&(cp->DeviceLock), 0, 1);
	cp->WebServer = ILibWebServer_Create(Chain, 5, 0, &UPnPOnSessionSink, cp);
	cp->SIDTable = ILibInitHashTree();
	cp->DeviceTable_UDN = ILibInitHashTree();
	cp->DeviceTable_URI = ILibInitHashTree();
	cp->DeviceTable_Tokens = ILibInitHashTree();
	#ifdef UPNP_DEBUG
	ILibWebServer_Create(Chain, 2, 7575, &UPnPOnDebugSessionSink, cp);
	#endif
	
	//{{{CreateSSDPClientModule}}}
	cp->HTTP = ILibCreateWebClient(5, Chain);
	ILibAddToChain(Chain, cp);
	cp->LifeTimeMonitor = (struct LifeTimeMonitorStruct*)ILibGetBaseTimer(Chain);

	cp->Chain = Chain;
	cp->RecheckFlag = 0;
	cp->IPAddressListV4Length = ILibGetLocalIPv4AddressList(&(cp->IPAddressListV4), 1);
	cp->IPAddressListV6Length = ILibGetLocalIPv6List(&(cp->IPAddressListV6));

	return((void*)cp);
}

//{{{Invocation_Sinks}}}
//{{{Invocation_Methods}}}

/*! \fn UPnPUnSubscribeUPnPEvents(struct UPnPService *service)
\brief Unregisters for UPnP events
\param service UPnP service to unregister from
*/
void UPnPUnSubscribeUPnPEvents(struct UPnPService *service)
{
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	char* TempString;
	size_t len;
	
	if (service->SubscriptionID == NULL) {return;}
	ILibParseUri(service->SubscriptionURL, &IP, &Port, &Path, NULL);
	p = ILibCreateEmptyPacket();
	ILibSetVersion(p, "1.1", 3);
	
	ILibSetDirective(p, "UNSUBSCRIBE", 11, Path, (int)strlen(Path));
	
	len = (int)strlen(IP) + 7;
	if ((TempString = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);
	snprintf(TempString, len, "%s:%d", IP, Port);
	
	ILibAddHeaderLine(p, "HOST", 4, TempString, (int)strlen(TempString));
	free(TempString);
	
	ILibAddHeaderLine(p, "User-Agent", 10, UPnPPLATFORM, (int)strlen(UPnPPLATFORM));
	ILibAddHeaderLine(p, "SID", 3, service->SubscriptionID, (int)strlen(service->SubscriptionID));
	
	UPnPAddRef(service->Parent);
	//ILibWebClient_SetQosForNextRequest(((struct UPnPCP*)service->Parent->CP)->HTTP, UPnPInvocationPriorityLevel);
	ILibWebClient_PipelineRequest(
	((struct UPnPCP*)service->Parent->CP)->HTTP, 
	(struct sockaddr*)&(service->Parent->LocationAddr), 
	p, 
	&UPnPOnUnSubscribeSink, 
	(void*)service, 
	service->Parent->CP);
	
	free(IP);
	free(Path);
}

/*! \fn UPnPSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service, int OK))
\brief Registers for UPnP events
\param service UPnP service to register with
\param callbackPtr Function Pointer triggered on completion
*/
void UPnPSubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service, int OK))
{
	char *IP;
	char *Path;
	unsigned short Port;
	struct packetheader *p;
	int len;

	UNREFERENCED_PARAMETER( callbackPtr );
	
	ILibParseUri(service->SubscriptionURL, &IP, &Port, &Path, NULL);
	p = ILibCreateEmptyPacket();
	ILibSetVersion(p, "1.1", 3);
	
	ILibSetDirective(p, "SUBSCRIBE", 9, Path, (int)strlen(Path));
	
	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "%s:%d", IP, Port);
	ILibAddHeaderLine(p, "HOST", 4, ILibScratchPad, len);
	ILibAddHeaderLine(p, "NT", 2, "upnp:event", 10);
	ILibAddHeaderLine(p, "TIMEOUT", 7, "Second-180", 10);
	ILibAddHeaderLine(p, "User-Agent", 10, UPnPPLATFORM, (int)strlen(UPnPPLATFORM));
	len = snprintf(ILibScratchPad, sizeof(ILibScratchPad), "<http://%s:%d%s>", service->Parent->InterfaceToHost, ILibWebServer_GetPortNumber(((struct UPnPCP*)service->Parent->CP)->WebServer), Path);
	ILibAddHeaderLine(p, "CALLBACK", 8, ILibScratchPad, len);
	
	UPnPAddRef(service->Parent);
	//ILibWebClient_SetQosForNextRequest(((struct UPnPCP*)service->Parent->CP)->HTTP, UPnPInvocationPriorityLevel);
	ILibWebClient_PipelineRequest(
	((struct UPnPCP*)service->Parent->CP)->HTTP, 
	(struct sockaddr*)&(service->Parent->LocationAddr), 
	p, 
	&UPnPOnSubscribeSink, 
	(void*)service, 
	service->Parent->CP);
	
	free(IP);
	free(Path);
}

//{{{CustomXMLTags2}}}
//{{{UPnPComplexTypes}}}


