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
#include "ILibParsers.h"
#include "UPnPMicroStack.h"
#include "ILibHTTPClient.h"

#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define UPnPMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)

const int UPnPDeviceDescriptionTemplateLengthUX = 1165;
const int UPnPDeviceDescriptionTemplateLength = 597;
const char UPnPDeviceDescriptionTemplate[597]={
	0x87,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D
	,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D
	,0x38,0x22,0x3F,0x3E,0x3C,0x72,0x6F,0x6F,0x74,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E
	,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x64,0x65
	,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0xC6,0x14,0x0B,0x3E
	,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E
	,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x00,0x06,0x12,0x00,0x08,0x02,0x05,0x54,0x79,0x70
	,0x65,0x3E,0x1B,0x1C,0x10,0x3A,0x42,0x69,0x6E,0x61,0x72,0x79,0x4C,0x69,0x67,0x68,0x74,0x3A,0x31,0x3C
	,0x2F,0x8B,0x0D,0x12,0x3C,0x66,0x72,0x69,0x65,0x6E,0x64,0x6C,0x79,0x4E,0x61,0x6D,0x65,0x3E,0x25,0x73
	,0x3C,0x2F,0x4D,0x04,0x21,0x3C,0x6D,0x61,0x6E,0x75,0x66,0x61,0x63,0x74,0x75,0x72,0x65,0x72,0x3E,0x49
	,0x6E,0x74,0x65,0x6C,0x20,0x43,0x6F,0x72,0x70,0x6F,0x72,0x61,0x74,0x69,0x6F,0x6E,0x3C,0x2F,0x0D,0x08
	,0x00,0x8D,0x0B,0x10,0x55,0x52,0x4C,0x3E,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x69
	,0x04,0x0F,0x06,0x2E,0x63,0x6F,0x6D,0x3C,0x2F,0x90,0x09,0x0D,0x3C,0x6D,0x6F,0x64,0x65,0x6C,0x44,0x65
	,0x73,0x63,0x72,0x69,0x70,0xC4,0x15,0x00,0x48,0x1A,0x20,0x45,0x4C,0x20,0x53,0x74,0x61,0x63,0x6B,0x20
	,0x42,0x75,0x69,0x6C,0x64,0x65,0x72,0x20,0x47,0x65,0x6E,0x65,0x72,0x61,0x74,0x65,0x64,0x20,0x4D,0x69
	,0x63,0x72,0x6F,0x86,0x07,0x02,0x3C,0x2F,0x11,0x10,0x00,0x86,0x14,0x00,0xC4,0x34,0x02,0x20,0x2F,0x48
	,0x03,0x05,0x75,0x6D,0x62,0x65,0x72,0xC4,0x03,0x05,0x73,0x65,0x72,0x69,0x61,0x07,0x04,0x00,0x85,0x3C
	,0x00,0x4D,0x04,0x0A,0x3C,0x55,0x44,0x4E,0x3E,0x75,0x75,0x69,0x64,0x3A,0x44,0x43,0x03,0x55,0x44,0x4E
	,0x45,0x0C,0x00,0x84,0x6D,0x04,0x4C,0x69,0x73,0x74,0x49,0x03,0x00,0x89,0x05,0x00,0x1A,0x5F,0x00,0xC7
	,0x0D,0x0E,0x3A,0x53,0x77,0x69,0x74,0x63,0x68,0x50,0x6F,0x77,0x65,0x72,0x3A,0x31,0x85,0x1B,0x00,0x8A
	,0x5F,0x00,0xC7,0x16,0x02,0x49,0x64,0xC5,0x6F,0x00,0xD0,0x0E,0x02,0x49,0x64,0x4C,0x0F,0x04,0x2E,0x30
	,0x30,0x30,0x0A,0x10,0x08,0x49,0x64,0x3E,0x3C,0x53,0x43,0x50,0x44,0xC4,0x59,0x00,0x8B,0x18,0x0B,0x2F
	,0x73,0x63,0x70,0x64,0x2E,0x78,0x6D,0x6C,0x3C,0x2F,0x88,0x07,0x08,0x3C,0x63,0x6F,0x6E,0x74,0x72,0x6F
	,0x6C,0x90,0x0A,0x00,0xC7,0x05,0x02,0x3C,0x2F,0x0B,0x08,0x09,0x3C,0x65,0x76,0x65,0x6E,0x74,0x53,0x75
	,0x62,0xD0,0x15,0x00,0x05,0x06,0x02,0x3C,0x2F,0xCC,0x07,0x02,0x3C,0x2F,0x10,0x43,0x00,0x6B,0x45,0x08
	,0x44,0x69,0x6D,0x6D,0x69,0x6E,0x67,0x53,0x07,0x49,0x00,0x31,0x46,0x00,0x0E,0x10,0x00,0xDA,0x46,0x00
	,0x0E,0x1A,0x00,0x9F,0x47,0x00,0x4F,0x0B,0x00,0x61,0x48,0x00,0x4F,0x17,0x00,0x1E,0x49,0x01,0x2F,0xCD
	,0x91,0x01,0x2F,0x88,0xF0,0x03,0x2F,0x72,0x6F,0x00,0x00,0x03,0x6F,0x74,0x3E,0x00,0x00};
/* SwitchPower */
const int UPnPSwitchPowerDescriptionLengthUX = 832;
const int UPnPSwitchPowerDescriptionLength = 422;
const char UPnPSwitchPowerDescription[422] = {
	0x88,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D
	,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D
	,0x38,0x22,0x3F,0x3E,0x3C,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E
	,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B
	,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72
	,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69
	,0x73,0x74,0x08,0x03,0x12,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x53,0x74,0x61,0x74,0x75
	,0x73,0x3C,0x2F,0x05,0x04,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65,0x6E,0x74,0x07,0x0B,0x00,0x87,0x03
	,0x00,0x87,0x0B,0x05,0x52,0x65,0x73,0x75,0x6C,0x4F,0x0C,0x04,0x64,0x69,0x72,0x65,0x86,0x14,0x05,0x6F
	,0x75,0x74,0x3C,0x2F,0xCA,0x03,0x08,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x04,0x18,0x0A,0x65,0x56
	,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x88,0x1B,0x00,0x55,0x07,0x02,0x3C,0x2F,0x4A,0x1C,0x01,0x2F
	,0x8E,0x22,0x04,0x2F,0x61,0x63,0x74,0xCB,0x32,0x00,0xC7,0x2F,0x09,0x53,0x65,0x74,0x54,0x61,0x72,0x67
	,0x65,0x74,0xE5,0x2F,0x03,0x6E,0x65,0x77,0x86,0x0B,0x05,0x56,0x61,0x6C,0x75,0x65,0x52,0x30,0x02,0x69
	,0x6E,0x22,0x30,0x00,0xC8,0x1B,0x00,0x39,0x30,0x01,0x2F,0x0C,0x63,0x00,0x47,0x78,0x00,0xC5,0x48,0x01
	,0x54,0xC6,0x40,0x01,0x73,0xCC,0x4B,0x11,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D
	,0x22,0x79,0x65,0x73,0x22,0x08,0x3E,0x00,0xCE,0x60,0x11,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x62
	,0x6F,0x6F,0x6C,0x65,0x61,0x6E,0x3C,0x2F,0x89,0x04,0x03,0x3C,0x2F,0x73,0x8E,0x58,0x00,0xDA,0x17,0x02
	,0x6E,0x6F,0x88,0x17,0x00,0xCE,0x54,0x00,0xAC,0x17,0x01,0x2F,0x53,0x34,0x01,0x2F,0xC4,0xB9,0x01,0x3E
	,0x00,0x00};
/* DimmingService */
const int UPnPDimmingServiceDescriptionLengthUX = 1333;
const int UPnPDimmingServiceDescriptionLength = 507;
const char UPnPDimmingServiceDescription[507] = {
	0x88,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D
	,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D
	,0x38,0x22,0x3F,0x3E,0x3C,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E
	,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B
	,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72
	,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69
	,0x73,0x74,0x08,0x03,0x1B,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x4C,0x6F,0x61,0x64,0x4C
	,0x65,0x76,0x65,0x6C,0x53,0x74,0x61,0x74,0x75,0x73,0x3C,0x2F,0x45,0x06,0x09,0x3C,0x61,0x72,0x67,0x75
	,0x6D,0x65,0x6E,0x74,0x47,0x0D,0x00,0x87,0x03,0x00,0xC7,0x0D,0x01,0x52,0xD9,0x0D,0x04,0x64,0x69,0x72
	,0x65,0x46,0x18,0x05,0x6F,0x75,0x74,0x3C,0x2F,0xCA,0x03,0x08,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64
	,0x84,0x19,0x0A,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x51,0x1F,0x00,0x95,0x09,0x02,0x3C
	,0x2F,0x0A,0x20,0x01,0x2F,0x4E,0x26,0x04,0x2F,0x61,0x63,0x74,0xCB,0x38,0x00,0xCA,0x35,0x03,0x4D,0x69
	,0x6E,0x85,0x35,0x00,0x25,0x34,0x00,0x50,0x0B,0x00,0xAF,0x31,0x00,0x0A,0x1B,0x00,0xFF,0x2F,0x00,0x87
	,0x65,0x01,0x53,0x8B,0x65,0x06,0x54,0x61,0x72,0x67,0x65,0x74,0xA5,0x65,0x03,0x4E,0x65,0x77,0xD7,0x0D
	,0x00,0x8A,0x65,0x02,0x69,0x6E,0x6B,0x65,0x00,0x08,0x1F,0x00,0x79,0x65,0x01,0x2F,0x4C,0x9E,0x00,0x87
	,0xB3,0x00,0x45,0x80,0x01,0x54,0x06,0x76,0x01,0x73,0x4C,0x83,0x11,0x20,0x73,0x65,0x6E,0x64,0x45,0x76
	,0x65,0x6E,0x74,0x73,0x3D,0x22,0x79,0x65,0x73,0x22,0x07,0xA9,0x00,0x98,0x9A,0x0D,0x61,0x74,0x61,0x54
	,0x79,0x70,0x65,0x3E,0x75,0x69,0x31,0x3C,0x2F,0x89,0x03,0x12,0x3C,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64
	,0x56,0x61,0x6C,0x75,0x65,0x52,0x61,0x6E,0x67,0x65,0x45,0xC7,0x04,0x69,0x6D,0x75,0x6D,0xC7,0xC7,0x00
	,0xC5,0x02,0x04,0x3C,0x6D,0x61,0x78,0x05,0x05,0x05,0x31,0x30,0x30,0x3C,0x2F,0x48,0x03,0x02,0x3C,0x2F
	,0x93,0x0F,0x02,0x2F,0x73,0x4E,0xA3,0x00,0x5A,0x2D,0x03,0x6E,0x6F,0x22,0x58,0x94,0x00,0x57,0x2B,0x00
	,0x34,0x17,0x00,0x18,0x79,0x00,0x3F,0x44,0x00,0x39,0x44,0x01,0x2F,0x53,0x76,0x03,0x2F,0x73,0x63,0x00
	,0x00,0x03,0x70,0x64,0x3E,0x00,0x00};

struct UPnPDataObject;

struct HTTPReaderObject
{
	char Header[4000];
	char* Body;
	struct packetheader *ParsedHeader;
	int BodySize;
	int HeaderIndex;
	int BodyIndex;
	SOCKET ClientSocket;
	int FinRead;
	struct UPnPDataObject *Parent;
};
struct SubscriberInfo
{
	char* SID;
	int SIDLength;
	int SEQ;
	
	int Address;
	unsigned short Port;
	char* Path;
	int PathLength;
	int RefCount;
	int Disposing;
	
	unsigned int RenewByTime;
	struct SubscriberInfo *Next;
	struct SubscriberInfo *Previous;
};
struct UPnPDataObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	
	void *EventClient;
	void *Chain;
	int UpdateFlag;
	
	/* Network Poll */
	unsigned int NetworkPollTime;
	
	int ForceExit;
	char *UUID;
	char *UDN;
	char *Serial;
	
	void *WebServerTimer;
	
	char *DeviceDescription;
	int DeviceDescriptionLength;
	int InitialNotify;
	char* DimmingService_LoadLevelStatus;
	char* SwitchPower_Status;
	struct sockaddr_in addr;
	int addrlen;
	SOCKET MSEARCH_sock;
	struct ip_mreq mreq;
	char message[4096];
	int *AddressList;
	int AddressListLength;
	
	int _NumEmbeddedDevices;
	SOCKET WebSocket;
	int WebSocketPortNumber;
	struct HTTPReaderObject ReaderObjects[5];
	SOCKET *NOTIFY_SEND_socks;
	SOCKET NOTIFY_RECEIVE_sock;
	
	int SID;
	
	unsigned int CurrentTime;
	int NotifyCycleTime;
	unsigned int NotifyTime;
	
	sem_t EventLock;
	struct SubscriberInfo *HeadSubscriberPtr_DimmingService;
	int NumberOfSubscribers_DimmingService;
	struct SubscriberInfo *HeadSubscriberPtr_SwitchPower;
	int NumberOfSubscribers_SwitchPower;
};

struct MSEARCH_state
{
	char *ST;
	int STLength;
	void *upnp;
	struct sockaddr_in dest_addr;
};

/* Pre-declarations */
void UPnPSendNotify(const struct UPnPDataObject *upnp);
void UPnPSendByeBye();
void UPnPMainInvokeSwitch();
void UPnPSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UPnPSendData(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
int UPnPPeriodicNotify(struct UPnPDataObject *upnp);
void UPnPSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);

char* UPnPDecompressString(unsigned char* CurrentCompressed, const int bufferLength, const int DecompressedLength)
{
	unsigned char *RetVal = (char*)MALLOC(DecompressedLength+1);
	unsigned char *CurrentUnCompressed = RetVal;
	unsigned char *EndPtr = RetVal + DecompressedLength;
	int offset,length;
	
	do
	{
		/* UnCompressed Data Block */
		memcpy(CurrentUnCompressed,CurrentCompressed+1,(int)*CurrentCompressed);
		CurrentUnCompressed += (int)*CurrentCompressed;
		CurrentCompressed += 1+((int)*CurrentCompressed);
		
		/* CompressedBlock */
		length = (*((unsigned short*)(CurrentCompressed)))&((unsigned short)63);
		offset = (*((unsigned short*)(CurrentCompressed)))>>6;
		memcpy(CurrentUnCompressed,CurrentUnCompressed-offset,length);
		CurrentCompressed += 2;
		CurrentUnCompressed += length;
	} while(CurrentUnCompressed < EndPtr);
	RetVal[DecompressedLength] = 0;
	return(RetVal);
}
void* UPnPGetInstance(const void* UPnPToken)
{
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	return (void*)(ReaderObject->Parent);
}

#define UPnPBuildSsdpResponsePacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"HTTP/1.1 200 OK\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nEXT:\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1181\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,ST,NTex);\
}

#define UPnPBuildSsdpNotifyPacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1181\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n",(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,NT,NTex);\
}

void UPnPIPAddressListChanged(void *MicroStackToken)
{
	((struct UPnPDataObject*)MicroStackToken)->UpdateFlag = 1;
	ILibForceUnBlockChain(((struct UPnPDataObject*)MicroStackToken)->Chain);
}
void UPnPInit(struct UPnPDataObject *state,const int NotifyCycleSeconds,const unsigned short PortNumber)
{
	int ra = 1;
	int i,flags;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	/* Complete State Reset */
	memset(state,0,sizeof(struct UPnPDataObject));
	
	/* Setup Notification Timer */
	state->NotifyCycleTime = NotifyCycleSeconds;
	state->CurrentTime = GetTickCount() / 1000;
	state->NotifyTime = state->CurrentTime  + (state->NotifyCycleTime/2);
	
	/* Initialize Client Sockets */
	for(i=0;i<5;++i)
	{
		memset(&(state->ReaderObjects[i]),0,sizeof(state->ReaderObjects[i]));
	}
	/* Setup WebSocket */
	if(PortNumber!=0)
	{
		memset((char *)&(addr), 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = (unsigned short)htons(PortNumber);
		state->WebSocket = socket(AF_INET, SOCK_STREAM, 0);
		flags = 1;
		ioctlsocket(state->WebSocket,FIONBIO,&flags);
		if (setsockopt(state->WebSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&ra, sizeof(ra)) < 0)
		{
			printf("Setting SockOpt SO_REUSEADDR failed (HTTP)");
			exit(1);
		}
		if (bind(state->WebSocket, (struct sockaddr *) &(addr), sizeof(addr)) < 0)
		{
			printf("WebSocket bind");
			exit(1);
		}
		state->WebSocketPortNumber = PortNumber;
	}
	else
	{
		state->WebSocketPortNumber = ILibGetStreamSocket(htonl(INADDR_ANY),&(state->WebSocket));
		flags = 1;
		ioctlsocket(state->WebSocket,FIONBIO,&flags);
	}
	if (listen(state->WebSocket,5)!=0)
	{
		printf("WebSocket listen");
		exit(1);
	}
	memset((char *)&(state->addr), 0, sizeof(state->addr));
	state->addr.sin_family = AF_INET;
	state->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	state->addr.sin_port = (unsigned short)htons(UPNP_PORT);
	state->addrlen = sizeof(state->addr);
	/* Set up socket */
	state->AddressListLength = ILibGetLocalIPAddressList(&(state->AddressList));
	state->NOTIFY_SEND_socks = (SOCKET*)MALLOC(sizeof(int)*(state->AddressListLength));
	state->NOTIFY_RECEIVE_sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset((char *)&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	if (setsockopt(state->NOTIFY_RECEIVE_sock, SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) < 0)
	{
		printf("Setting SockOpt SO_REUSEADDR failed\r\n");
		exit(1);
	}
	if (bind(state->NOTIFY_RECEIVE_sock, (struct sockaddr *) &(addr), sizeof(addr)) < 0)
	{
		printf("Could not bind to UPnP Listen Port\r\n");
		exit(1);
	}
	for(i=0;i<state->AddressListLength;++i)
	{
		state->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRAM, 0);
		memset((char *)&(addr), 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = state->AddressList[i];
		addr.sin_port = (unsigned short)htons(UPNP_PORT);
		if (setsockopt(state->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)
		{
			if (setsockopt(state->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)
			{
				/* Ignore this case */
			}
			if (bind(state->NOTIFY_SEND_socks[i], (struct sockaddr *) &(addr), sizeof(addr)) == 0)
			{
				mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
				mreq.imr_interface.s_addr = state->AddressList[i];
				if (setsockopt(state->NOTIFY_RECEIVE_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
				{
					/* Does not matter */
				}
			}
		}
	}
}
void UPnPPostMX_Destroy(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	FREE(mss->ST);
	FREE(mss);
}
void UPnPPostMX_MSEARCH(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	
	char *b = (char*)MALLOC(sizeof(char)*5000);
	int packetlength;
	struct sockaddr_in response_addr;
	int response_addrlen;
	SOCKET *response_socket;
	int cnt;
	int i;
	struct sockaddr_in dest_addr = mss->dest_addr;
	char *ST = mss->ST;
	int STLength = mss->STLength;
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)mss->upnp;
	
	response_socket = (SOCKET*)MALLOC(upnp->AddressListLength*sizeof(int));
	for(i=0;i<upnp->AddressListLength;++i)
	{
		response_socket[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (response_socket[i]< 0)
		{
			printf("response socket");
			exit(1);
		}
		memset((char *)&(response_addr), 0, sizeof(response_addr));
		response_addr.sin_family = AF_INET;
		response_addr.sin_addr.s_addr = upnp->AddressList[i];
		response_addr.sin_port = (unsigned short)htons(0);
		response_addrlen = sizeof(response_addr);	
		if (bind(response_socket[i], (struct sockaddr *) &(response_addr), sizeof(response_addr)) < 0)
		{
			/* Ignore if this happens */
		}
	}
	if(STLength==15 && memcmp(ST,"upnp:rootdevice",15)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	else if(STLength==8 && memcmp(ST,"ssdp:all",8)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:BinaryLight:1","urn:schemas-upnp-org:device:BinaryLight:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:SwitchPower:1","urn:schemas-upnp-org:service:SwitchPower:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:DimmingService:1","urn:schemas-upnp-org:service:DimmingService:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength==(int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=41 && memcmp(ST,"urn:schemas-upnp-org:device:BinaryLight:1",41)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:BinaryLight:1","urn:schemas-upnp-org:device:BinaryLight:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=42 && memcmp(ST,"urn:schemas-upnp-org:service:SwitchPower:1",42)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:SwitchPower:1","urn:schemas-upnp-org:service:SwitchPower:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=45 && memcmp(ST,"urn:schemas-upnp-org:service:DimmingService:1",45)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:DimmingService:1","urn:schemas-upnp-org:service:DimmingService:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	for(i=0;i<upnp->AddressListLength;++i)
	{
		closesocket(response_socket[i]);
	}
	FREE(response_socket);
	FREE(mss->ST);
	FREE(mss);
}
void UPnPProcessMSEARCH(struct UPnPDataObject *upnp, struct packetheader *packet)
{
	char* ST = NULL;
	int STLength = 0;
	struct packetheader_field_node *node;
	int MANOK = 0;
	unsigned long MXVal;
	int MXOK = 0;
	int MX;
	struct MSEARCH_state *mss = NULL;
	
	if(memcmp(packet->DirectiveObj,"*",1)==0)
	{
		if(memcmp(packet->Version,"1.1",3)==0)
		{
			node = packet->FirstField;
			while(node!=NULL)
			{
				if(_strnicmp(node->Field,"ST",2)==0)
				{
					ST = (char*)MALLOC(1+node->FieldDataLength);
					memcpy(ST,node->FieldData,node->FieldDataLength);
					ST[node->FieldDataLength] = 0;
					STLength = node->FieldDataLength;
				}
				else if(_strnicmp(node->Field,"MAN",3)==0 && memcmp(node->FieldData,"\"ssdp:discover\"",15)==0)
				{
					MANOK = 1;
				}
				else if(_strnicmp(node->Field,"MX",2)==0 && ILibGetULong(node->FieldData,node->FieldDataLength,&MXVal)==0)
				{
					MXOK = 1;
					MXVal = MXVal>10?10:MXVal;
				}
				node = node->NextField;
			}
			if(MANOK!=0 && MXOK!=0)
			{
				MX = (int)(0 + ((unsigned short)rand() % MXVal));
				mss = (struct MSEARCH_state*)MALLOC(sizeof(struct MSEARCH_state));
				mss->ST = ST;
				mss->STLength = STLength;
				mss->upnp = upnp;
				memset((char *)&(mss->dest_addr), 0, sizeof(mss->dest_addr));
				mss->dest_addr.sin_family = AF_INET;
				mss->dest_addr.sin_addr = packet->Source->sin_addr;
				mss->dest_addr.sin_port = packet->Source->sin_port;
				
				ILibLifeTime_Add(upnp->WebServerTimer,mss,MX,&UPnPPostMX_MSEARCH,&UPnPPostMX_Destroy);
			}
			else
			{
				FREE(ST);
			}
		}
	}
}
#define UPnPDispatch_DimmingService_GetLoadLevelStatus(xml, ReaderObject)\
{\
	UPnPDimmingService_GetLoadLevelStatus((void*)ReaderObject);\
}

#define UPnPDispatch_DimmingService_GetMinLevel(xml, ReaderObject)\
{\
	UPnPDimmingService_GetMinLevel((void*)ReaderObject);\
}

void UPnPDispatch_DimmingService_SetLoadLevelTarget(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_NewLoadLevelTarget = NULL;
	int p_NewLoadLevelTargetLength = 0;
	unsigned char _NewLoadLevelTarget = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==18 && memcmp(VarName,"NewLoadLevelTarget",18) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_NewLoadLevelTarget = temp3->LastResult->data;
					p_NewLoadLevelTargetLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_NewLoadLevelTarget,p_NewLoadLevelTargetLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Illegal value");
			return;
		}
		_NewLoadLevelTarget = (unsigned char)TempULong;
	}
	UPnPDimmingService_SetLoadLevelTarget((void*)ReaderObject,_NewLoadLevelTarget);
}

#define UPnPDispatch_SwitchPower_GetStatus(xml, ReaderObject)\
{\
	UPnPSwitchPower_GetStatus((void*)ReaderObject);\
}

void UPnPDispatch_SwitchPower_SetTarget(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	int OK = 0;
	char *p_newTargetValue = NULL;
	int p_newTargetValueLength = 0;
	int _newTargetValue = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==14 && memcmp(VarName,"newTargetValue",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_newTargetValue = temp3->LastResult->data;
					p_newTargetValueLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK=0;
	if(p_newTargetValueLength==4)
	{
		if(_strnicmp(p_newTargetValue,"true",4)==0)
		{
			OK = 1;
			_newTargetValue = 1;
		}
	}
	if(p_newTargetValueLength==5)
	{
		if(_strnicmp(p_newTargetValue,"false",5)==0)
		{
			OK = 1;
			_newTargetValue = 0;
		}
	}
	if(p_newTargetValueLength==1)
	{
		if(memcmp(p_newTargetValue,"0",1)==0)
		{
			OK = 1;
			_newTargetValue = 0;
		}
		if(memcmp(p_newTargetValue,"1",1)==0)
		{
			OK = 1;
			_newTargetValue = 1;
		}
	}
	if(OK==0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UPnPSwitchPower_SetTarget((void*)ReaderObject,_newTargetValue);
}

void UPnPProcessPOST(struct packetheader* header, struct HTTPReaderObject *ReaderObject)
{
	struct packetheader_field_node *f = header->FirstField;
	char* HOST;
	char* SOAPACTION = NULL;
	int SOAPACTIONLength = 0;
	struct parser_result *r;
	struct parser_result *xml;
	
	xml = ILibParseString(header->Body,0,header->BodyLength,"<",1);
	while(f!=NULL)
	{
		if(f->FieldLength==4 && _strnicmp(f->Field,"HOST",4)==0)
		{
			HOST = f->FieldData;
		}
		else if(f->FieldLength==10 && _strnicmp(f->Field,"SOAPACTION",10)==0)
		{
			r = ILibParseString(f->FieldData,0,f->FieldDataLength,"#",1);
			SOAPACTION = r->LastResult->data;
			SOAPACTIONLength = r->LastResult->datalength-1;
			ILibDestructParserResults(r);
		}
		f = f->NextField;
	}
	if(header->DirectiveObjLength==20 && memcmp((header->DirectiveObj)+1,"SwitchPower/control",19)==0)
	{
		if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"GetStatus",9)==0)
		{
			UPnPDispatch_SwitchPower_GetStatus(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"SetTarget",9)==0)
		{
			UPnPDispatch_SwitchPower_SetTarget(xml, ReaderObject);
		}
	}
	else if(header->DirectiveObjLength==23 && memcmp((header->DirectiveObj)+1,"DimmingService/control",22)==0)
	{
		if(SOAPACTIONLength==18 && memcmp(SOAPACTION,"GetLoadLevelStatus",18)==0)
		{
			UPnPDispatch_DimmingService_GetLoadLevelStatus(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"GetMinLevel",11)==0)
		{
			UPnPDispatch_DimmingService_GetMinLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==18 && memcmp(SOAPACTION,"SetLoadLevelTarget",18)==0)
		{
			UPnPDispatch_DimmingService_SetLoadLevelTarget(xml, ReaderObject);
		}
	}
	ILibDestructParserResults(xml);
}
struct SubscriberInfo* UPnPRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers,char* SID, int SIDLength)
{
	struct SubscriberInfo *info = *Head;
	struct SubscriberInfo **ptr = Head;
	while(info!=NULL)
	{
		if(info->SIDLength==SIDLength && memcmp(info->SID,SID,SIDLength)==0)
		{
			*ptr = info->Next;
			if(info->Next!=NULL) 
			{
				(*ptr)->Previous = info->Previous;
				if((*ptr)->Previous!=NULL) 
				{
					(*ptr)->Previous->Next = info->Next;
					if((*ptr)->Previous->Next!=NULL)
					{
						(*ptr)->Previous->Next->Previous = (*ptr)->Previous;
					}
				}
			}
			break;
		}
		ptr = &(info->Next);
		info = info->Next;
	}
	if(info!=NULL)
	{
		info->Previous = NULL;
		info->Next = NULL;
		--(*TotalSubscribers);
	}
	return(info);
}

#define UPnPDestructSubscriberInfo(info)\
{\
	FREE(info->Path);\
	FREE(info->SID);\
	FREE(info);\
}

#define UPnPDestructEventObject(EvObject)\
{\
	FREE(EvObject->PacketBody);\
	FREE(EvObject);\
}

#define UPnPDestructEventDataObject(EvData)\
{\
	FREE(EvData);\
}
void UPnPExpireSubscriberInfo(struct UPnPDataObject *d, struct SubscriberInfo *info)
{
	struct SubscriberInfo *t = info;
	while(t->Previous!=NULL)
	{
		t = t->Previous;
	}
	if(d->HeadSubscriberPtr_DimmingService==t)
	{
		--(d->NumberOfSubscribers_DimmingService);
	}
	else if(d->HeadSubscriberPtr_SwitchPower==t)
	{
		--(d->NumberOfSubscribers_SwitchPower);
	}
	if(info->Previous!=NULL)
	{
		// This is not the Head
		info->Previous->Next = info->Next;
		if(info->Next!=NULL)
		{
			info->Previous->Next->Previous = info->Previous;
		}
	}
	else
	{
		// This is the Head
		if(d->HeadSubscriberPtr_DimmingService==info)
		{
			d->HeadSubscriberPtr_DimmingService = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_DimmingService;
			}
		}
		else if(d->HeadSubscriberPtr_SwitchPower==info)
		{
			d->HeadSubscriberPtr_SwitchPower = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_SwitchPower;
			}
		}
		else
		{
			// Error
			return;
		}
	}
	ILibDeleteRequests(d->EventClient,info);
	--info->RefCount;
	if(info->RefCount==0)
	{
		UPnPDestructSubscriberInfo(info);
	}
}

int UPnPSubscriptionExpired(struct SubscriberInfo *info)
{
	int RetVal = 0;
	if(info->RenewByTime < GetTickCount()/1000) {RetVal = -1;}
	return(RetVal);
}
void UPnPGetInitialEventBody_DimmingService(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(35+(int)strlen(UPnPObject->DimmingService_LoadLevelStatus));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"LoadLevelStatus>%s</LoadLevelStatus",UPnPObject->DimmingService_LoadLevelStatus);
}
void UPnPGetInitialEventBody_SwitchPower(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(17+(int)strlen(UPnPObject->SwitchPower_Status));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"Status>%s</Status",UPnPObject->SwitchPower_Status);
}
void UPnPProcessUNSUBSCRIBE(struct packetheader *header, struct HTTPReaderObject *ReaderObject)
{
	char* SID = NULL;
	int SIDLength = 0;
	struct SubscriberInfo *Info;
	struct packetheader_field_node *f;
	char* packet = (char*)MALLOC(sizeof(char)*40);
	int packetlength;
	
	f = header->FirstField;
	while(f!=NULL)
	{
		if(f->FieldLength==3)
		{
			if(_strnicmp(f->Field,"SID",3)==0)
			{
				SID = f->FieldData;
				SIDLength = f->FieldDataLength;
			}
		}
		f = f->NextField;
	}
	sem_wait(&(ReaderObject->Parent->EventLock));
	if(header->DirectiveObjLength==21 && memcmp(header->DirectiveObj + 1,"DimmingService/event",20)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_DimmingService),&(ReaderObject->Parent->NumberOfSubscribers_DimmingService),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	else if(header->DirectiveObjLength==18 && memcmp(header->DirectiveObj + 1,"SwitchPower/event",17)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_SwitchPower),&(ReaderObject->Parent->NumberOfSubscribers_SwitchPower),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	sem_post(&(ReaderObject->Parent->EventLock));
	FREE(packet);
}
void UPnPTryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength,struct HTTPReaderObject *ReaderObject)
{
	int *TotalSubscribers = NULL;
	struct SubscriberInfo **HeadPtr = NULL;
	struct SubscriberInfo *NewSubscriber,*TempSubscriber;
	int SIDNumber;
	char *SID;
	char *TempString;
	int TempStringLength;
	char *TempString2;
	long TempLong;
	char *packet;
	int packetlength;
	char* path;
	
	char *packetbody = NULL;
	int packetbodyLength;
	
	struct parser_result *p;
	struct parser_result *p2;
	
	if(strncmp(ServiceName,"DimmingService",14)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_DimmingService);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_DimmingService);
	}
	if(strncmp(ServiceName,"SwitchPower",11)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_SwitchPower);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_SwitchPower);
	}
	if(*HeadPtr!=NULL)
	{
		NewSubscriber = *HeadPtr;
		while(NewSubscriber!=NULL)
		{
			if(UPnPSubscriptionExpired(NewSubscriber)!=0)
			{
				TempSubscriber = NewSubscriber->Next;
				NewSubscriber = UPnPRemoveSubscriberInfo(HeadPtr,TotalSubscribers,NewSubscriber->SID,NewSubscriber->SIDLength);
				UPnPDestructSubscriberInfo(NewSubscriber);
				NewSubscriber = TempSubscriber;
			}
			else
			{
				NewSubscriber = NewSubscriber->Next;
			}
		}
	}
	if(*TotalSubscribers<10)
	{
		NewSubscriber = (struct SubscriberInfo*)MALLOC(sizeof(struct SubscriberInfo));
		SIDNumber = ++ReaderObject->Parent->SID;
		SID = (char*)MALLOC(10 + 6);
		sprintf(SID,"uuid:%d",SIDNumber);
		p = ILibParseString(URL,0,URLLength,"://",3);
		if(p->NumResults==1)
		{
			send(ReaderObject->ClientSocket,"HTTP/1.1 412 Precondition Failed\r\n\r\n",36,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			ILibDestructParserResults(p);
			return;
		}
		TempString = p->LastResult->data;
		TempStringLength = p->LastResult->datalength;
		ILibDestructParserResults(p);
		p = ILibParseString(TempString,0,TempStringLength,"/",1);
		p2 = ILibParseString(p->FirstResult->data,0,p->FirstResult->datalength,":",1);
		TempString2 = (char*)MALLOC(1+sizeof(char)*p2->FirstResult->datalength);
		memcpy(TempString2,p2->FirstResult->data,p2->FirstResult->datalength);
		TempString2[p2->FirstResult->datalength] = '\0';
		NewSubscriber->Address = inet_addr(TempString2);
		if(p2->NumResults==1)
		{
			NewSubscriber->Port = 80;
			path = (char*)MALLOC(1+TempStringLength - p2->FirstResult->datalength -1);
			memcpy(path,TempString + p2->FirstResult->datalength,TempStringLength - p2->FirstResult->datalength -1);
			path[TempStringLength - p2->FirstResult->datalength - 1] = '\0';
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		else
		{
			ILibGetLong(p2->LastResult->data,p2->LastResult->datalength,&TempLong);
			NewSubscriber->Port = (unsigned short)TempLong;
			if(TempStringLength==p->FirstResult->datalength)
			{
				path = (char*)MALLOC(2);
				memcpy(path,"/",1);
				path[1] = '\0';
			}
			else
			{
				path = (char*)MALLOC(1+TempStringLength - p->FirstResult->datalength -1);
				memcpy(path,TempString + p->FirstResult->datalength,TempStringLength - p->FirstResult->datalength -1);
				path[TempStringLength - p->FirstResult->datalength -1] = '\0';
			}
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		ILibDestructParserResults(p);
		ILibDestructParserResults(p2);
		FREE(TempString2);
		NewSubscriber->RefCount = 1;
		NewSubscriber->Disposing = 0;
		NewSubscriber->Previous = NULL;
		NewSubscriber->SID = SID;
		NewSubscriber->SIDLength = (int)strlen(SID);
		NewSubscriber->SEQ = 0;
		NewSubscriber->RenewByTime = (GetTickCount() / 1000) + Timeout;
		NewSubscriber->Next = *HeadPtr;
		if(*HeadPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}
		*HeadPtr = NewSubscriber;
		++(*TotalSubscribers);
		LVL3DEBUG(printf("\r\n\r\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\r\n",NewSubscriber->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)
		LVL3DEBUG(printf("TIMESTAMP: %d <%d>\r\n\r\n",(NewSubscriber->RenewByTime)-Timeout,NewSubscriber);)
		packet = (char*)MALLOC(134 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1181\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,Timeout);
		if(strcmp(ServiceName,"DimmingService")==0)
		{
			UPnPGetInitialEventBody_DimmingService(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"SwitchPower")==0)
		{
			UPnPGetInitialEventBody_SwitchPower(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		if (packetbody != NULL)	    {
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			FREE(packet);
			
			UPnPSendEvent_Body(ReaderObject->Parent,packetbody,packetbodyLength,NewSubscriber);
			FREE(packetbody);
		} 
	}
	else
	{
		/* Too many subscribers */
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Too Many Subscribers\r\n\r\n",37,0);
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}
void UPnPSubscribeEvents(char* path,int pathlength,char* Timeout,int TimeoutLength,char* URL,int URLLength,struct HTTPReaderObject* ReaderObject)
{
	long TimeoutVal;
	char* buffer = (char*)MALLOC(1+sizeof(char)*pathlength);
	
	ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
	memcpy(buffer,path,pathlength);
	buffer[pathlength] = '\0';
	FREE(buffer);
	if(TimeoutVal>7200) {TimeoutVal=7200;}
	
	if(pathlength==18 && memcmp(path+1,"SwitchPower/event",17)==0)
	{
		UPnPTryToSubscribe("SwitchPower",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else if(pathlength==21 && memcmp(path+1,"DimmingService/event",20)==0)
	{
		UPnPTryToSubscribe("DimmingService",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else
	{
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Invalid Service Name\r\n\r\n",37,0);
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}
void UPnPRenewEvents(char* path,int pathlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct HTTPReaderObject *ReaderObject)
{
	struct SubscriberInfo *info = NULL;
	long TimeoutVal;
	char* packet;
	int packetlength;
	char* SID = (char*)MALLOC(SIDLength+1);
	memcpy(SID,_SID,SIDLength);
	SID[SIDLength] ='\0';
	LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",GetTickCount()/1000);)
	LVL3DEBUG(printf("SUBSCRIBER [%s] attempting to Renew Events for %s Duration [",SID,Timeout);)
	if(pathlength==21 && memcmp(path+1,"DimmingService/event",20)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_DimmingService;
	}
	else if(pathlength==18 && memcmp(path+1,"SwitchPower/event",17)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_SwitchPower;
	}
	while(info!=NULL && strcmp(info->SID,SID)!=0)
	{
		info = info->Next;
	}
	if(info!=NULL)
	{
		ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
		info->RenewByTime = TimeoutVal + (GetTickCount() / 1000);
		packet = (char*)MALLOC(113 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1181\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\n\r\n",SID,TimeoutVal);
		send(ReaderObject->ClientSocket,packet,packetlength,0);
		FREE(packet);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n",TimeoutVal,info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Precondition Failed\r\n\r\n",36,0);
	}
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(SID);
}
void UPnPProcessSUBSCRIBE(struct packetheader *header, struct HTTPReaderObject *ReaderObject)
{
	char* SID = NULL;
	int SIDLength = 0;
	char* Timeout = NULL;
	int TimeoutLength = 0;
	char* URL = NULL;
	int URLLength = 0;
	struct parser_result *p;
	struct packetheader_field_node *f;
	
	f = header->FirstField;
	while(f!=NULL)
	{
		if(f->FieldLength==3 && _strnicmp(f->Field,"SID",3)==0)
		{
			SID = f->FieldData;
			SIDLength = f->FieldDataLength;
		}
		else if(f->FieldLength==8 && _strnicmp(f->Field,"Callback",8)==0)
		{
			URL = f->FieldData;
			URLLength = f->FieldDataLength;
		}
		else if(f->FieldLength==7 && _strnicmp(f->Field,"Timeout",7)==0)
		{
			Timeout = f->FieldData;
			TimeoutLength = f->FieldDataLength;
		}
		f = f->NextField;
	}
	if(Timeout==NULL)
	{
		Timeout = "7200";
		TimeoutLength = 4;
	}
	else
	{
		p = ILibParseString(Timeout,0,TimeoutLength,"-",1);
		if(p->NumResults==2)
		{
			Timeout = p->LastResult->data;
			TimeoutLength = p->LastResult->datalength;
			if(TimeoutLength==8 && _strnicmp(Timeout,"INFINITE",8)==0)
			{
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
	if(SID==NULL)
	{
		/* Subscribe */
		UPnPSubscribeEvents(header->DirectiveObj,header->DirectiveObjLength,Timeout,TimeoutLength,URL,URLLength,ReaderObject);
	}
	else
	{
		/* Renew */
		UPnPRenewEvents(header->DirectiveObj,header->DirectiveObjLength,SID,SIDLength,Timeout,TimeoutLength,ReaderObject);
	}
}
void UPnPProcessHTTPPacket(struct packetheader* header, struct HTTPReaderObject *ReaderObject)
{
	char *buffer;
	/* Virtual Directory Support */
	if(header->DirectiveObjLength>=4 && memcmp(header->DirectiveObj,"/web",4)==0)
	{
		UPnPPresentationRequest((void*)ReaderObject,header);
	}
	else if(header->DirectiveLength==3 && memcmp(header->Directive,"GET",3)==0)
	{
		if(header->DirectiveObjLength==1 && memcmp(header->DirectiveObj,"/",1)==0)
		{
			send(ReaderObject->ClientSocket,ReaderObject->Parent->DeviceDescription,ReaderObject->Parent->DeviceDescriptionLength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			return;
		}
		else if(header->DirectiveObjLength==21 && memcmp((header->DirectiveObj)+1,"SwitchPower/scpd.xml",20)==0)
		{
			buffer = UPnPDecompressString((char*)UPnPSwitchPowerDescription,UPnPSwitchPowerDescriptionLength,UPnPSwitchPowerDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPSwitchPowerDescriptionLengthUX, 0);
			FREE(buffer);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else if(header->DirectiveObjLength==24 && memcmp((header->DirectiveObj)+1,"DimmingService/scpd.xml",23)==0)
		{
			buffer = UPnPDecompressString((char*)UPnPDimmingServiceDescription,UPnPDimmingServiceDescriptionLength,UPnPDimmingServiceDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPDimmingServiceDescriptionLengthUX, 0);
			FREE(buffer);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
	}
	else if(header->DirectiveLength==4 && memcmp(header->Directive,"POST",4)==0)
	{
		UPnPProcessPOST(header,ReaderObject);
	}
	else if(header->DirectiveLength==9 && memcmp(header->Directive,"SUBSCRIBE",9)==0)
	{
		UPnPProcessSUBSCRIBE(header,ReaderObject);
	}
	else if(header->DirectiveLength==11 && memcmp(header->Directive,"UNSUBSCRIBE",11)==0)
	{
		UPnPProcessUNSUBSCRIBE(header,ReaderObject);
	}
}
void UPnPProcessHTTPSocket(struct HTTPReaderObject *ReaderObject)
{
	int bytesReceived = 0;
	int ContentLength = 0;
	struct packetheader_field_node *field;
	int headsize = 0;
	int x;
	
	if(ReaderObject->Body == NULL)
	{
		/* Still Reading Headers */
		bytesReceived = recv(ReaderObject->ClientSocket,ReaderObject->Header+ReaderObject->HeaderIndex,4000-ReaderObject->HeaderIndex,0);
		if(bytesReceived!=0 && bytesReceived!=0xFFFFFFFF)
		{
			/* Received Data
			*/
			ReaderObject->HeaderIndex += bytesReceived;
			if(ReaderObject->HeaderIndex >= 4)
			{
				/* Must have read at least 4 bytes to have a header */
				
				headsize = 0;
				for(x=0;x<(ReaderObject->HeaderIndex - 3);x++)
				{
					//printf("CMP: %x\r\n",*((int*)(ReaderObject->Header + x)));
					//if (*((int*)((ReaderObject->Header) + x)) == 0x0A0D0A0D)
					if (ReaderObject->Header[x] == '\r' && ReaderObject->Header[x+1] == '\n' && ReaderObject->Header[x+2] == '\r' && ReaderObject->Header[x+3] == '\n')
					{
						headsize = x + 4;
						break;
					}
				}
				
				if(headsize != 0)
				{
					/* Complete reading header */
					ReaderObject->ParsedHeader = ILibParsePacketHeader(ReaderObject->Header,0,headsize);
					field = ReaderObject->ParsedHeader->FirstField;
					while(field!=NULL)
					{
						if(field->FieldLength>=14)
						{
							if(_strnicmp(field->Field,"content-length",14)==0)
							{
								ContentLength = atoi(field->FieldData);
								break;
							}
						}
						field = field->NextField;
					}
					if(ContentLength==0)
					{
						/* No Body */
						ReaderObject->FinRead = 1;
						UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
					}
					else
					{
						/* There is a Body */
						
						/* Check to see if over reading has occured */
						if (headsize < ReaderObject->HeaderIndex)
						{
							if(ReaderObject->HeaderIndex - headsize >= ContentLength)
							{
								ReaderObject->FinRead=1;
								ReaderObject->ParsedHeader->Body = ReaderObject->Header + headsize;
								ReaderObject->ParsedHeader->BodyLength = ContentLength;
								UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
							}
							else
							{
								ReaderObject->Body = (char*)MALLOC(sizeof(char)*ContentLength);
								ReaderObject->BodySize = ContentLength;
								
								memcpy(ReaderObject->Body,ReaderObject->Header + headsize,UPnPMIN(ReaderObject->HeaderIndex - headsize,ContentLength));
								ReaderObject->BodyIndex = ReaderObject->HeaderIndex - headsize;
							}
						}
						else
						{
							ReaderObject->Body = (char*)MALLOC(sizeof(char)*ContentLength);
							ReaderObject->BodySize = ContentLength;
						}
					}
					//ILibDestructPacket(header);
				}
			}
		}
		else
		if(bytesReceived==0)
		{
			/* Socket Closed */
			ReaderObject->ClientSocket = 0;
		}
	}
	else
	{
		/* Reading Body */
		bytesReceived = recv(ReaderObject->ClientSocket,
		ReaderObject->Body+ReaderObject->BodyIndex,
		ReaderObject->BodySize-ReaderObject->BodyIndex,
		0);
		if(bytesReceived!=0)
		{
			/* Received Data */
			ReaderObject->BodyIndex += bytesReceived;
			if(ReaderObject->BodyIndex==ReaderObject->BodySize)
			{
				ReaderObject->FinRead=1;
				//header = ILibParsePacketHeader(ReaderObject->Header,0,ReaderObject->HeaderIndex);
				ReaderObject->ParsedHeader->Body = ReaderObject->Body;
				ReaderObject->ParsedHeader->BodyLength = ReaderObject->BodySize;
				UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
				//ILibDestructPacket(header);
			}
		}
		else
		{
			/* Socket Closed/Error */
			ReaderObject->ClientSocket = 0;
		}
	}
}
void UPnPMasterPreSelect(void* object,fd_set *socketset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	int i;
	int NumFree = 5;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;
	int notifytime;
	
	int ra = 1;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	if(UPnPObject->InitialNotify==0)
	{
		UPnPObject->InitialNotify = -1;
		UPnPSendByeBye(UPnPObject);
		UPnPSendNotify(UPnPObject);
	}
	if(UPnPObject->UpdateFlag!=0)
	{
		UPnPObject->UpdateFlag = 0;
		
		/* Clear Sockets */
		for(i=0;i<UPnPObject->AddressListLength;++i)
		{
			closesocket(UPnPObject->NOTIFY_SEND_socks[i]);
		}
		FREE(UPnPObject->NOTIFY_SEND_socks);
		
		/* Set up socket */
		FREE(UPnPObject->AddressList);
		UPnPObject->AddressListLength = ILibGetLocalIPAddressList(&(UPnPObject->AddressList));
		UPnPObject->NOTIFY_SEND_socks = (SOCKET*)MALLOC(sizeof(int)*(UPnPObject->AddressListLength));
		
		for(i=0;i<UPnPObject->AddressListLength;++i)
		{
			UPnPObject->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRAM, 0);
			memset((char *)&(addr), 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = UPnPObject->AddressList[i];
			addr.sin_port = (unsigned short)htons(UPNP_PORT);
			if (setsockopt(UPnPObject->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)
			{
				if (setsockopt(UPnPObject->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)
				{
					// Ignore the case if setting the Multicast-TTL fails
				}
				if (bind(UPnPObject->NOTIFY_SEND_socks[i], (struct sockaddr *) &(addr), sizeof(addr)) == 0)
				{
					mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
					mreq.imr_interface.s_addr = UPnPObject->AddressList[i];
					if (setsockopt(UPnPObject->NOTIFY_RECEIVE_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
					{
						// Does not matter if it fails, just ignore
					}
				}
			}
		}
		UPnPSendNotify(UPnPObject);
	}
	FD_SET(UPnPObject->NOTIFY_RECEIVE_sock,socketset);
	for(i=0;i<5;++i)
	{
		if(UPnPObject->ReaderObjects[i].ClientSocket!=0)
		{
			if(UPnPObject->ReaderObjects[i].FinRead==0)
			{
				FD_SET(UPnPObject->ReaderObjects[i].ClientSocket,socketset);
				FD_SET(UPnPObject->ReaderObjects[i].ClientSocket,errorset);
			}
			--NumFree;
		}
	}
	
	notifytime = UPnPPeriodicNotify(UPnPObject);
	if(NumFree!=0)
	{
		FD_SET(UPnPObject->WebSocket,socketset);
		if(notifytime<*blocktime) {*blocktime=notifytime;}
	}
	else
	{
		if(*blocktime>1)
		{
			*blocktime = 1;
		}
	}
}

void UPnPWebServerTimerSink(void *data)
{
	struct HTTPReaderObject* RO = (struct HTTPReaderObject*)data;
	
	if(RO->ClientSocket!=0)
	{
		closesocket(RO->ClientSocket);
		RO->ClientSocket = 0;
	}
}
void UPnPMasterPostSelect(void* object,int slct, fd_set *socketset, fd_set *writeset, fd_set *errorset)
{
	unsigned long flags=0;
	int cnt = 0;
	int i;
	struct packetheader *packet;
	SOCKET NewSocket;
	struct sockaddr addr;
	int addrlen;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;
	
	if(slct>0)
	{
		if(FD_ISSET(UPnPObject->WebSocket,socketset)!=0)
		{
			for(i=0;i<5;++i)
			{
				if(UPnPObject->ReaderObjects[i].ClientSocket==0)
				{
					addrlen = sizeof(addr);
					NewSocket = accept(UPnPObject->WebSocket,&addr,&addrlen);
					ioctlsocket(NewSocket,FIONBIO,&flags);
					if (NewSocket != 0xFFFFFFFF)
					{
						ILibLifeTime_Add(UPnPObject->WebServerTimer,&(UPnPObject->ReaderObjects[i]),3,&UPnPWebServerTimerSink,NULL);
						if(UPnPObject->ReaderObjects[i].Body != NULL)
						{
							FREE(UPnPObject->ReaderObjects[i].Body);
							UPnPObject->ReaderObjects[i].Body = NULL;
						}
						if(UPnPObject->ReaderObjects[i].ParsedHeader!=NULL)
						{
							ILibDestructPacket(UPnPObject->ReaderObjects[i].ParsedHeader);
						}
						UPnPObject->ReaderObjects[i].ClientSocket = NewSocket;
						UPnPObject->ReaderObjects[i].HeaderIndex = 0;
						UPnPObject->ReaderObjects[i].BodyIndex = 0;
						UPnPObject->ReaderObjects[i].Body = NULL;
						UPnPObject->ReaderObjects[i].BodySize = 0;
						UPnPObject->ReaderObjects[i].FinRead = 0;
						UPnPObject->ReaderObjects[i].Parent = UPnPObject;
						UPnPObject->ReaderObjects[i].ParsedHeader = NULL;
					}
					else {break;}
				}
			}
		}
		for(i=0;i<5;++i)
		{
			if(UPnPObject->ReaderObjects[i].ClientSocket!=0)
			{
				if(FD_ISSET(UPnPObject->ReaderObjects[i].ClientSocket,socketset)!=0)
				{
					UPnPProcessHTTPSocket(&(UPnPObject->ReaderObjects[i]));
				}
				if(FD_ISSET(UPnPObject->ReaderObjects[i].ClientSocket,errorset)!=0)
				{
					/* Socket is probably closed */
					UPnPObject->ReaderObjects[i].ClientSocket = 0;
					if(UPnPObject->ReaderObjects[i].Body != NULL)
					{
						FREE(UPnPObject->ReaderObjects[i].Body);
						UPnPObject->ReaderObjects[i].Body = NULL;
					}
				}
				if(UPnPObject->ReaderObjects[i].ClientSocket==0 || UPnPObject->ReaderObjects[i].Body!=NULL || (UPnPObject->ReaderObjects[i].ParsedHeader!=NULL && UPnPObject->ReaderObjects[i].ParsedHeader->Body != NULL))
				{
					ILibLifeTime_Remove(UPnPObject->WebServerTimer,&(UPnPObject->ReaderObjects[i]));
				}
			}
		}
		if(FD_ISSET(UPnPObject->NOTIFY_RECEIVE_sock,socketset)!=0)
		{	
			cnt = recvfrom(UPnPObject->NOTIFY_RECEIVE_sock, UPnPObject->message, sizeof(UPnPObject->message), 0,
			(struct sockaddr *) &(UPnPObject->addr), &(UPnPObject->addrlen));
			if (cnt < 0)
			{
				printf("recvfrom");
				exit(1);
			}
			else if (cnt == 0)
			{
				/* Socket Closed? */
			}
			packet = ILibParsePacketHeader(UPnPObject->message,0,cnt);
			packet->Source = (struct sockaddr_in*)&(UPnPObject->addr);
			packet->ReceivingAddress = 0;
			if(packet->StatusCode==-1 && memcmp(packet->Directive,"M-SEARCH",8)==0)
			{
				UPnPProcessMSEARCH(UPnPObject, packet);
			}
			ILibDestructPacket(packet);
		}
		
	}
}
int UPnPPeriodicNotify(struct UPnPDataObject *upnp)
{
	upnp->CurrentTime = GetTickCount() / 1000;
	if(upnp->CurrentTime >= upnp->NotifyTime)
	{
		upnp->NotifyTime = upnp->CurrentTime + (upnp->NotifyCycleTime / 3);
		UPnPSendNotify(upnp);
	}
	return(upnp->NotifyTime-upnp->CurrentTime);
}
void UPnPSendNotify(const struct UPnPDataObject *upnp)
{
	int packetlength;
	char* packet = (char*)MALLOC(5000);
	int i,i2;
	struct sockaddr_in addr;
	int addrlen;
	struct in_addr interface_addr;
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	addrlen = sizeof(addr);
	
	memset((char *)&interface_addr, 0, sizeof(interface_addr));
	
	for(i=0;i<upnp->AddressListLength;++i)
	{
		interface_addr.s_addr = upnp->AddressList[i];
		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			for (i2=0;i2<2;i2++)
			{
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"","uuid:",upnp->UDN,upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:BinaryLight:1","urn:schemas-upnp-org:device:BinaryLight:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:SwitchPower:1","urn:schemas-upnp-org:service:SwitchPower:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:DimmingService:1","urn:schemas-upnp-org:service:DimmingService:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
			}
		}
	}
	FREE(packet);
}

#define UPnPBuildSsdpByeByePacket(outpacket,outlenght,USN,USNex,NT,NTex)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.0\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n",USN,USNex,NT,NTex);\
}

void UPnPSendByeBye(const struct UPnPDataObject *upnp)
{
	int packetlength;
	char* packet = (char*)MALLOC(5000);
	int i, i2;
	struct sockaddr_in addr;
	int addrlen;
	struct in_addr interface_addr;
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	addrlen = sizeof(addr);
	
	memset((char *)&interface_addr, 0, sizeof(interface_addr));
	
	for(i=0;i<upnp->AddressListLength;++i)
	{
		
		interface_addr.s_addr = upnp->AddressList[i];
		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			
			for (i2=0;i2<2;i2++)
			{
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"","uuid:",upnp->UDN);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:device:BinaryLight:1","urn:schemas-upnp-org:device:BinaryLight:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:SwitchPower:1","urn:schemas-upnp-org:service:SwitchPower:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:DimmingService:1","urn:schemas-upnp-org:service:DimmingService:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
			}
		}
	}
	FREE(packet);
}

void UPnPResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg)
{
	char* body;
	int bodylength;
	char* head;
	int headlength;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	body = (char*)MALLOC(395 + (int)strlen(ErrorMsg));
	bodylength = sprintf(body,"<s:Envelope\r\n xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></UPnPError></detail></s:Fault></s:Body></s:Envelope>",ErrorCode,ErrorMsg);
	head = (char*)MALLOC(59);
	headlength = sprintf(head,"HTTP/1.0 500 Internal\r\nContent-Length: %d\r\n\r\n",bodylength);
	send(ReaderObject->ClientSocket,head,headlength,0);
	send(ReaderObject->ClientSocket,body,bodylength,0);
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(head);
	FREE(body);
}

int UPnPPresentationResponse(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate)
{
	int status = -1;
	SOCKET TempSocket;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	status = send(ReaderObject->ClientSocket,Data,DataLength,0);
	if (Terminate != 0)
	{
		TempSocket = ReaderObject->ClientSocket;
		ReaderObject->ClientSocket = 0;
		closesocket(TempSocket);
	}
	return status;
}

int UPnPGetLocalInterfaceToHost(const void* UPnPToken)
{
	struct sockaddr_in addr;
	int addrsize = sizeof(addr);
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	if (getsockname(ReaderObject->ClientSocket, (struct sockaddr*) &addr, &addrsize) != 0) return 0;
	return (addr.sin_addr.s_addr);
}

void UPnPResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params)
{
	char* packet;
	int packetlength;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	
	packet = (char*)MALLOC(67+strlen(ServiceURI)+strlen(Params)+(strlen(MethodName)*2));
	packetlength = sprintf(packet,"<u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
	send(ReaderObject->ClientSocket,"HTTP/1.0 200 OK\r\nEXT:\r\nCONTENT-TYPE: text/xml\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1181\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>",275,0);
	send(ReaderObject->ClientSocket,packet,packetlength,0);
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(packet);}

void UPnPResponse_SwitchPower_GetStatus(const void* UPnPToken, const int ResultStatus)
{
	char* body;
	
	body = (char*)MALLOC(31);
	sprintf(body,"<ResultStatus>%d</ResultStatus>",(ResultStatus!=0?1:0));
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:SwitchPower:1","GetStatus",body);
	FREE(body);
}

void UPnPResponse_SwitchPower_SetTarget(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:SwitchPower:1","SetTarget","");
}

void UPnPResponse_DimmingService_GetLoadLevelStatus(const void* UPnPToken, const unsigned char RetLoadLevelStatus)
{
	char* body;
	
	body = (char*)MALLOC(46);
	sprintf(body,"<RetLoadLevelStatus>%u</RetLoadLevelStatus>",RetLoadLevelStatus);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:DimmingService:1","GetLoadLevelStatus",body);
	FREE(body);
}

void UPnPResponse_DimmingService_GetMinLevel(const void* UPnPToken, const unsigned char MinLevel)
{
	char* body;
	
	body = (char*)MALLOC(26);
	sprintf(body,"<MinLevel>%u</MinLevel>",MinLevel);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:DimmingService:1","GetMinLevel",body);
	FREE(body);
}

void UPnPResponse_DimmingService_SetLoadLevelTarget(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:DimmingService:1","SetLoadLevelTarget","");
}

void UPnPSendEventSink(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* subscriber, void *upnp)
{
	if(done!=0 && ((struct SubscriberInfo*)subscriber)->Disposing==0)
	{
		sem_wait(&(((struct UPnPDataObject*)upnp)->EventLock));
		--((struct SubscriberInfo*)subscriber)->RefCount;
		if(((struct SubscriberInfo*)subscriber)->RefCount==0)
		{
			LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			UPnPDestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
		}
		else if(header==NULL)
		{
			LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			// Could not send Event, so unsubscribe the subscriber
			((struct SubscriberInfo*)subscriber)->Disposing = 1;
			UPnPExpireSubscriberInfo(upnp,subscriber);
		}
		sem_post(&(((struct UPnPDataObject*)upnp)->EventLock));
	}
}
void UPnPSendEvent_Body(void *upnptoken,char *body,int bodylength,struct SubscriberInfo *info)
{
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
	struct sockaddr_in dest;
	int packetLength;
	char *packet;
	int ipaddr;
	
	memset(&dest,0,sizeof(dest));
	dest.sin_addr.s_addr = info->Address;
	dest.sin_port = htons(info->Port);
	dest.sin_family = AF_INET;
	ipaddr = info->Address;
	
	packet = (char*)MALLOC(info->PathLength + bodylength + 383);
	packetLength = sprintf(packet,"NOTIFY %s HTTP/1.0\r\nHOST: %d.%d.%d.%d:%d\r\nContent-Type: text/xml\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),info->Port,info->SID,info->SEQ,bodylength+137,body);
	++info->SEQ;
	
	++info->RefCount;
	ILibAddRequest_Direct(UPnPObject->EventClient,packet,packetLength,&dest,&UPnPSendEventSink,info,upnptoken);
}
void UPnPSendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
	struct SubscriberInfo *info = NULL;
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
	struct sockaddr_in dest;
	LVL3DEBUG(struct timeval tv;)
	
	if(UPnPObject==NULL)
	{
		FREE(body);
		return;
	}
	sem_wait(&(UPnPObject->EventLock));
	if(strncmp(eventname,"DimmingService",14)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_DimmingService;
	}
	if(strncmp(eventname,"SwitchPower",11)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_SwitchPower;
	}
	memset(&dest,0,sizeof(dest));
	while(info!=NULL)
	{
		if(!UPnPSubscriptionExpired(info))
		{
			UPnPSendEvent_Body(upnptoken,body,bodylength,info);
		}
		else
		{
			//Remove Subscriber
			LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",GetTickCount()/1000);)
			LVL3DEBUG(printf("Did not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\r\n\r\n",((struct SubscriberInfo*)info)->SID,(((struct SubscriberInfo*)info)->Address&0xFF),((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((struct SubscriberInfo*)info)->Address>>16)&0xFF),((((struct SubscriberInfo*)info)->Address>>24)&0xFF),((struct SubscriberInfo*)info)->Port,info);)
		}
		
		info = info->Next;
	}
	
	sem_post(&(UPnPObject->EventLock));
}

void UPnPSetState_SwitchPower_Status(void *upnptoken, int val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	if (val != 0) valstr = "true"; else valstr = "false";
	UPnPObject->SwitchPower_Status = valstr;
	body = (char*)MALLOC(22 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","Status",valstr,"Status");
	UPnPSendEvent(upnptoken,body,bodylength,"SwitchPower");
	FREE(body);
}

void UPnPSetState_DimmingService_LoadLevelStatus(void *upnptoken, unsigned char val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(10);
	sprintf(valstr,"%d",val);
	if (UPnPObject->DimmingService_LoadLevelStatus != NULL) FREE(UPnPObject->DimmingService_LoadLevelStatus);
	UPnPObject->DimmingService_LoadLevelStatus = valstr;
	body = (char*)MALLOC(40 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","LoadLevelStatus",valstr,"LoadLevelStatus");
	UPnPSendEvent(upnptoken,body,bodylength,"DimmingService");
	FREE(body);
}


void UPnPDestroyMicroStack(void *object)
{
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)object;
	struct SubscriberInfo  *sinfo,*sinfo2;
	int i;
	UPnPSendByeBye(upnp);
	
	sem_destroy(&(upnp->EventLock));
	FREE(upnp->DimmingService_LoadLevelStatus);
	
	FREE(upnp->AddressList);
	FREE(upnp->NOTIFY_SEND_socks);
	FREE(upnp->UUID);
	FREE(upnp->Serial);
	FREE(upnp->DeviceDescription);
	
	sinfo = upnp->HeadSubscriberPtr_DimmingService;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_SwitchPower;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	
	for(i=0;i<5;++i)
	{
		if(upnp->ReaderObjects[i].Body!=NULL) {FREE(upnp->ReaderObjects[i].Body);}
		if(upnp->ReaderObjects[i].ParsedHeader!=NULL) {ILibDestructPacket(upnp->ReaderObjects[i].ParsedHeader);}
	}
	WSACleanup();
}
int UPnPGetLocalPortNumber(void *token)
{
	return(((struct UPnPDataObject*)token)->WebSocketPortNumber);
}
void *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)
{
	struct UPnPDataObject* RetVal = (struct UPnPDataObject*)MALLOC(sizeof(struct UPnPDataObject));
	char* DDT;
	WORD wVersionRequested;
	WSADATA wsaData;
	
	srand((int)GetTickCount());
	wVersionRequested = MAKEWORD( 1, 1 );
	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}
	UPnPInit(RetVal,NotifyCycleSeconds,PortNum);
	RetVal->ForceExit = 0;
	RetVal->PreSelect = &UPnPMasterPreSelect;
	RetVal->PostSelect = &UPnPMasterPostSelect;
	RetVal->Destroy = &UPnPDestroyMicroStack;
	RetVal->InitialNotify = 0;
	if (UDN != NULL)
	{
		RetVal->UUID = (char*)MALLOC((int)strlen(UDN)+6);
		sprintf(RetVal->UUID,"uuid:%s",UDN);
		RetVal->UDN = RetVal->UUID + 5;
	}
	if (SerialNumber != NULL)
	{
		RetVal->Serial = (char*)MALLOC((int)strlen(SerialNumber)+1);
		strcpy(RetVal->Serial,SerialNumber);
	}
	
	RetVal->DeviceDescription = (char*)MALLOC(UPnPDeviceDescriptionTemplateLengthUX + (int)strlen(FriendlyName) + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * 1));
	DDT = UPnPDecompressString((char*)UPnPDeviceDescriptionTemplate,UPnPDeviceDescriptionTemplateLength,UPnPDeviceDescriptionTemplateLengthUX);
	RetVal->DeviceDescriptionLength = sprintf(RetVal->DeviceDescription,DDT,FriendlyName,RetVal->Serial,RetVal->UDN);
	FREE(DDT);
	RetVal->DimmingService_LoadLevelStatus = NULL;
	RetVal->SwitchPower_Status = NULL;
	
	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
	
	ILibAddToChain(Chain,RetVal);
	RetVal->EventClient = ILibCreateHTTPClientModule(Chain,5);
	RetVal->Chain = Chain;
	RetVal->UpdateFlag = 0;
	
	sem_init(&(RetVal->EventLock),0,1);
	return(RetVal);
}

void UPnPSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate)
{
	int escsize;
	char* buf;
	
	escsize = (int)ILibXmlEscapeLength(Data);
	buf = (char*)MALLOC(escsize);
	
	if (buf != NULL)
	{
		escsize = ILibXmlEscape(buf,Data);
		UPnPSendData(UPnPToken,buf,escsize,Terminate);
		FREE(buf);
	}
}

void UPnPSendData(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate)
{
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	send(ReaderObject->ClientSocket,Data,DataLength,0);
	if (Terminate != 0)
	{
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}

