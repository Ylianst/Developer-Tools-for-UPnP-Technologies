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
#include <math.h>
#include <winerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include <crtdbg.h>
#include "ILibParsers.h"
#include "UpnpMicroStack.h"
#include "ILibWebServer.h"
#include "ILibWebClient.h"
#include "ILibAsyncSocket.h"

#define UPNP_HTTP_MAXSOCKETS 5


#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define UpnpMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)

const int UpnpDeviceDescriptionTemplateLengthUX = 1496;
const int UpnpDeviceDescriptionTemplateLength = 571;
const char UpnpDeviceDescriptionTemplate[571]={
	0x5A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22
	,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C
	,0x72,0x6F,0x6F,0x74,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65
	,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x64,0x65,0x76,0x69,0x63,0x65,0x2D
	,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0xC6,0x14,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F
	,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02
	,0x02,0x3C,0x2F,0x8D,0x0B,0x00,0x06,0x12,0x00,0x08,0x02,0x05,0x54,0x79,0x70,0x65,0x3E,0x1B,0x1C,0x12
	,0x3A,0x4D,0x65,0x64,0x69,0x61,0x52,0x65,0x6E,0x64,0x65,0x72,0x65,0x72,0x3A,0x31,0x3C,0x2F,0x0B,0x0E
	,0x12,0x3C,0x66,0x72,0x69,0x65,0x6E,0x64,0x6C,0x79,0x4E,0x61,0x6D,0x65,0x3E,0x25,0x73,0x3C,0x2F,0x4D
	,0x04,0x21,0x3C,0x6D,0x61,0x6E,0x75,0x66,0x61,0x63,0x74,0x75,0x72,0x65,0x72,0x3E,0x49,0x6E,0x74,0x65
	,0x6C,0x20,0x43,0x6F,0x72,0x70,0x6F,0x72,0x61,0x74,0x69,0x6F,0x6E,0x3C,0x2F,0x0D,0x08,0x00,0x8D,0x0B
	,0x10,0x55,0x52,0x4C,0x3E,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x69,0x04,0x0F,0x06
	,0x2E,0x63,0x6F,0x6D,0x3C,0x2F,0x90,0x09,0x0D,0x3C,0x6D,0x6F,0x64,0x65,0x6C,0x44,0x65,0x73,0x63,0x72
	,0x69,0x70,0xC4,0x15,0x04,0x3E,0x41,0x56,0x20,0x45,0x2D,0x01,0x20,0x88,0x2D,0x02,0x3C,0x2F,0x11,0x09
	,0x00,0x86,0x0D,0x00,0xC4,0x2D,0x02,0x20,0x2F,0x48,0x03,0x05,0x75,0x6D,0x62,0x65,0x72,0xC4,0x03,0x05
	,0x73,0x65,0x72,0x69,0x61,0x07,0x04,0x00,0x85,0x35,0x00,0x4D,0x04,0x0A,0x3C,0x55,0x44,0x4E,0x3E,0x75
	,0x75,0x69,0x64,0x3A,0x44,0x3C,0x03,0x55,0x44,0x4E,0x45,0x0C,0x00,0x04,0x67,0x04,0x4C,0x69,0x73,0x74
	,0x49,0x03,0x00,0x89,0x05,0x00,0x9A,0x58,0x00,0xC7,0x0D,0x07,0x3A,0x43,0x6F,0x6E,0x6E,0x65,0x63,0xC4
	,0x44,0x05,0x4D,0x61,0x6E,0x61,0x67,0xC6,0x59,0x00,0x8C,0x0F,0x00,0x48,0x18,0x02,0x49,0x64,0xC5,0x6A
	,0x00,0x50,0x10,0x0B,0x49,0x64,0x3A,0x4D,0x52,0x5F,0x43,0x4D,0x53,0x3C,0x2F,0x4A,0x0A,0x05,0x3C,0x53
	,0x43,0x50,0x44,0xC4,0x51,0x00,0x4D,0x70,0x01,0x5F,0x11,0x1B,0x0B,0x2F,0x73,0x63,0x70,0x64,0x2E,0x78
	,0x6D,0x6C,0x3C,0x2F,0x88,0x0C,0x08,0x3C,0x63,0x6F,0x6E,0x74,0x72,0x6F,0x6C,0xA4,0x0F,0x00,0xC7,0x0A
	,0x02,0x3C,0x2F,0x0B,0x0D,0x09,0x3C,0x65,0x76,0x65,0x6E,0x74,0x53,0x75,0x62,0xE4,0x1F,0x00,0x05,0x0B
	,0x02,0x3C,0x2F,0xCC,0x0C,0x02,0x3C,0x2F,0x10,0x51,0x00,0x6B,0x53,0x00,0xC6,0xAA,0x04,0x69,0x6E,0x67
	,0x43,0x46,0x31,0x00,0x35,0x53,0x02,0x52,0x43,0x24,0x53,0x00,0xD0,0x1A,0x00,0xED,0x52,0x00,0x51,0x0F
	,0x00,0xAF,0x52,0x00,0x51,0x1F,0x00,0x7F,0x52,0x00,0x91,0xA5,0x0B,0x41,0x56,0x54,0x72,0x61,0x6E,0x73
	,0x70,0x6F,0x72,0x74,0x35,0xA4,0x03,0x41,0x56,0x54,0x23,0xA4,0x00,0x8B,0x19,0x00,0xAD,0xA2,0x00,0x0C
	,0x0E,0x00,0x2F,0xA1,0x00,0xCC,0x1C,0x00,0x9E,0x9F,0x01,0x2F,0x4D,0xF6,0x03,0x2F,0x64,0x65,0xC7,0x05
	,0x02,0x72,0x6F,0x00,0x00,0x03,0x6F,0x74,0x3E,0x00,0x00};
/* MediaRenderer_ConnectionManager */
const int UpnpMediaRenderer_ConnectionManagerDescriptionLengthUX = 3516;
const int UpnpMediaRenderer_ConnectionManagerDescriptionLength = 933;
const char UpnpMediaRenderer_ConnectionManagerDescription[933] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7A,0x2E,0x31,0x32,0x39,0x38,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x33,0x33,0x39,0x35,0x0D,0x0A,0x0D,0x0A
	,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20
	,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x73
	,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D
	,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x2D
	,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F
	,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02
	,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x16,0x3E
	,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x43,0x6F,0x6E,0x6E
	,0x65,0xC5,0x09,0x06,0x49,0x6E,0x66,0x6F,0x3C,0x2F,0xC5,0x07,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65
	,0x6E,0x74,0xC7,0x0E,0x00,0x87,0x03,0x00,0x47,0x0F,0x00,0xCB,0x0C,0x01,0x44,0x48,0x0C,0x03,0x64,0x69
	,0x72,0x86,0x11,0x05,0x3E,0x69,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64
	,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F
	,0x84,0x63,0x01,0x5F,0x4E,0x13,0x00,0x95,0x0B,0x02,0x3C,0x2F,0x4A,0x20,0x00,0xCF,0x22,0x03,0x52,0x63
	,0x73,0x14,0x21,0x03,0x6F,0x75,0x74,0x6D,0x21,0x03,0x52,0x63,0x73,0xB4,0x1F,0x0B,0x41,0x56,0x54,0x72
	,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0xBF,0x21,0x00,0xC5,0x42,0x00,0xCF,0x13,0x00,0x30,0x43,0x08,0x50
	,0x72,0x6F,0x74,0x6F,0x63,0x6F,0x6C,0x0C,0x72,0x00,0xFA,0x44,0x00,0x8E,0x13,0x00,0x31,0x23,0x03,0x65
	,0x65,0x72,0x8A,0x96,0x07,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72,0x3F,0x6A,0x03,0x50,0x45,0x5F,0xD3,0x14
	,0x00,0xBE,0x26,0x00,0x7F,0x8F,0x00,0xBF,0xB0,0x00,0x84,0xE2,0x01,0x44,0x48,0xCE,0x00,0xBF,0xB1,0x03
	,0x50,0x45,0x5F,0xCB,0x12,0x00,0x30,0xD2,0x00,0xC4,0xE7,0x02,0x75,0x73,0x7F,0x68,0x00,0x8D,0xF3,0x00
	,0x88,0x14,0x00,0xA1,0xF4,0x00,0x49,0xF7,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x07,0xED,0x00,0x08,0x02
	,0x00,0x05,0xFA,0x03,0x47,0x65,0x74,0xD4,0xB7,0x00,0xCE,0x0E,0x00,0x10,0x35,0x05,0x6F,0x75,0x72,0x63
	,0x65,0xF7,0xE5,0x00,0x46,0x0F,0x00,0x7E,0xC1,0x04,0x53,0x69,0x6E,0x6B,0xF8,0x1F,0x03,0x69,0x6E,0x6B
	,0xAF,0xE0,0x00,0x28,0x52,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x0C,0xC1,0x01,0x73,0x25,0x54,0x00
	,0x95,0x0C,0x00,0x2F,0xF3,0x00,0x56,0x1F,0x00,0x39,0x89,0x00,0x47,0x8B,0x00,0x86,0x8E,0x07,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x05,0xEA,0x01,0x54,0xC6,0xDD,0x01,0x73,0x0C,0xED,0x10,0x20,0x73,0x65,0x6E
	,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x47,0xE0,0x00,0x8B,0xF2,0x00,0xD4,0x98
	,0x11,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04
	,0x03,0x3C,0x2F,0x73,0x4E,0xF9,0x00,0xAF,0x1B,0x00,0x12,0xCB,0x00,0xA1,0x1C,0x0C,0x61,0x6C,0x6C,0x6F
	,0x77,0x65,0x64,0x56,0x61,0x6C,0x75,0x65,0x07,0xBD,0x00,0x8B,0x04,0x05,0x3E,0x4F,0x4B,0x3C,0x2F,0x4D
	,0x04,0x00,0xCE,0x07,0x15,0x43,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x46,0x6F,0x72,0x6D,0x61,0x74,0x4D,0x69
	,0x73,0x6D,0x61,0x74,0x63,0x68,0x9D,0x0C,0x14,0x49,0x6E,0x73,0x75,0x66,0x66,0x69,0x63,0x69,0x65,0x6E
	,0x74,0x42,0x61,0x6E,0x64,0x77,0x69,0x64,0x74,0x9E,0x0C,0x05,0x55,0x6E,0x72,0x65,0x6C,0x45,0xF9,0x07
	,0x43,0x68,0x61,0x6E,0x6E,0x65,0x6C,0x9F,0x0B,0x05,0x6B,0x6E,0x6F,0x77,0x6E,0x90,0x2D,0x01,0x2F,0x12
	,0x3A,0x00,0x3F,0x5B,0x0D,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x49,0x44,0xD1,0x76
	,0x02,0x69,0x34,0xFF,0x75,0x00,0x4C,0x91,0x03,0x52,0x63,0x73,0xFF,0x18,0x00,0xAB,0x8E,0x00,0xBF,0x1A
	,0x00,0x2B,0xA9,0x07,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72,0xFF,0xC5,0x00,0x4E,0xE1,0x03,0x79,0x65,0x73
	,0x88,0xE1,0x06,0x53,0x6F,0x75,0x72,0x63,0x65,0x7F,0xE0,0x00,0xA6,0x1A,0x03,0x69,0x6E,0x6B,0x7F,0xFA
	,0x00,0x6F,0xFA,0x03,0x44,0x69,0x72,0x06,0xFA,0x00,0xBF,0xF8,0x00,0x45,0xE4,0x03,0x70,0x75,0x74,0x5D
	,0xF9,0x06,0x4F,0x75,0x74,0x70,0x75,0x74,0xBF,0xD4,0x00,0xD9,0x69,0x07,0x43,0x75,0x72,0x72,0x65,0x6E
	,0x74,0x4C,0xA0,0x01,0x73,0xF3,0x84,0x09,0x2F,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x53,0x44,0xF5,0x01
	,0x54,0x08,0x05,0x01,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00,0x00};
/* MediaRenderer_RenderingControl */
const int UpnpMediaRenderer_RenderingControlDescriptionLengthUX = 2865;
const int UpnpMediaRenderer_RenderingControlDescriptionLength = 777;
const char UpnpMediaRenderer_RenderingControlDescription[777] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7A,0x2E,0x31,0x32,0x39,0x38,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x32,0x37,0x34,0x34,0x0D,0x0A,0x0D,0x0A
	,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20
	,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x73
	,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D
	,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x2D
	,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F
	,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02
	,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x12,0x3E
	,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x53,0x65,0x74,0x56,0x6F,0x6C,0x75,0x6D,0x65,0x3C,0x2F,0x05,0x04,0x09
	,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65,0x6E,0x74,0x07,0x0B,0x00,0x87,0x03,0x00,0x87,0x0B,0x0A,0x49,0x6E
	,0x73,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0xC8,0x0B,0x04,0x64,0x69,0x72,0x65,0x06,0x14,0x04,0x69,0x6E
	,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56,0x61
	,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0x44,0x5F,0x01,0x5F,0xCC,0x12,0x00
	,0x15,0x0B,0x02,0x3C,0x2F,0x4A,0x1F,0x00,0xCF,0x21,0x07,0x43,0x68,0x61,0x6E,0x6E,0x65,0x6C,0x3F,0x21
	,0x02,0x45,0x5F,0x09,0x12,0x00,0x70,0x20,0x07,0x44,0x65,0x73,0x69,0x72,0x65,0x64,0x8E,0x4E,0x00,0xEE
	,0x42,0x00,0x88,0x5D,0x00,0x21,0x3F,0x01,0x2F,0x8E,0x64,0x04,0x2F,0x61,0x63,0x74,0xCB,0x74,0x00,0xC7
	,0x71,0x06,0x47,0x65,0x74,0x4D,0x75,0x74,0x7F,0x71,0x00,0x7F,0x71,0x00,0x7F,0x71,0x00,0x7F,0x71,0x00
	,0xB3,0x91,0x05,0x75,0x72,0x72,0x65,0x6E,0x0D,0x4E,0x00,0x8A,0xB3,0x03,0x6F,0x75,0x74,0xE2,0xB3,0x03
	,0x4D,0x75,0x74,0xBF,0x70,0x00,0x4D,0xE2,0x00,0xBF,0x70,0x00,0xFF,0xE1,0x00,0xFF,0xE1,0x00,0xFF,0xE1
	,0x00,0xFC,0xE1,0x03,0x4D,0x75,0x74,0x77,0xE1,0x00,0x7F,0x70,0x00,0xD0,0xE0,0x00,0x08,0xF5,0x00,0x7F
	,0xE1,0x00,0x7F,0xE1,0x00,0x7F,0xE1,0x00,0x7F,0xE1,0x00,0x76,0xE1,0x00,0x8E,0x4E,0x00,0xEF,0xE1,0x00
	,0xC8,0x5D,0x00,0x79,0xE2,0x00,0x87,0xE4,0x00,0xC6,0xE7,0x07,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x85
	,0xFA,0x01,0x54,0x06,0xF3,0x01,0x73,0x8C,0xFD,0x10,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E,0x74
	,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x07,0xF0,0x00,0x8F,0x30,0x0D,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E
	,0x75,0x69,0x32,0x3C,0x2F,0x89,0x03,0x1F,0x3C,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x56,0x61,0x6C,0x75
	,0x65,0x52,0x61,0x6E,0x67,0x65,0x3E,0x3C,0x6D,0x69,0x6E,0x69,0x6D,0x75,0x6D,0x3E,0x30,0x3C,0x2F,0xC8
	,0x02,0x04,0x3C,0x6D,0x61,0x78,0x05,0x05,0x05,0x31,0x30,0x30,0x3C,0x2F,0x48,0x03,0x09,0x3C,0x73,0x74
	,0x65,0x70,0x3E,0x31,0x3C,0x2F,0x05,0x02,0x02,0x3C,0x2F,0x13,0x13,0x02,0x2F,0x73,0x0E,0xEF,0x00,0x64
	,0x2E,0x00,0x97,0x92,0x00,0x11,0x32,0x01,0x34,0x0C,0x32,0x00,0x7F,0x1A,0x00,0xD0,0x9D,0x00,0x88,0x4B
	,0x06,0x73,0x74,0x72,0x69,0x6E,0x67,0x58,0x4C,0x00,0x47,0xD2,0x00,0xCB,0x50,0x07,0x3E,0x4D,0x61,0x73
	,0x74,0x65,0x72,0x0E,0x43,0x00,0xCF,0x08,0x02,0x4C,0x46,0xDD,0x07,0x01,0x52,0xD1,0x07,0x01,0x2F,0x12
	,0x1D,0x00,0x2A,0x56,0x03,0x79,0x65,0x73,0x88,0x84,0x04,0x4C,0x61,0x73,0x74,0x04,0xD8,0x02,0x67,0x65
	,0x23,0x3A,0x00,0xB4,0x6E,0x03,0x4D,0x75,0x74,0x52,0x9C,0x07,0x62,0x6F,0x6F,0x6C,0x65,0x61,0x6E,0x5C
	,0x6B,0x01,0x2F,0xD3,0xB8,0x03,0x2F,0x73,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00,0x00};
/* MediaRenderer_AVTransport */
const int UpnpMediaRenderer_AVTransportDescriptionLengthUX = 14566;
const int UpnpMediaRenderer_AVTransportDescriptionLength = 2960;
const char UpnpMediaRenderer_AVTransportDescription[2960] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7B,0x2E,0x31,0x32,0x39,0x38,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x31,0x34,0x34,0x34,0x34,0x0D,0x0A,0x0D
	,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22
	,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C
	,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65
	,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65
	,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A
	,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46
	,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x1B
	,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x54,0x72,0x61
	,0x6E,0x73,0x70,0x6F,0x72,0x74,0x41,0x05,0x0B,0x03,0x73,0x3C,0x2F,0x45,0x08,0x09,0x3C,0x61,0x72,0x67
	,0x75,0x6D,0x65,0x6E,0x74,0x47,0x0F,0x00,0x87,0x03,0x00,0xC7,0x0F,0x0A,0x49,0x6E,0x73,0x74,0x61,0x6E
	,0x63,0x65,0x49,0x44,0xC8,0x0B,0x04,0x64,0x69,0x72,0x65,0x46,0x18,0x04,0x69,0x6E,0x3C,0x2F,0x8A,0x03
	,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62
	,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0xC4,0x63,0x01,0x5F,0xCC,0x12,0x00,0x15,0x0B,0x02,0x3C
	,0x2F,0x4A,0x1F,0x00,0xCF,0x21,0x00,0xCF,0x2C,0x00,0x0A,0x21,0x03,0x6F,0x75,0x74,0x62,0x21,0x00,0x59
	,0x40,0x00,0xE1,0x21,0x01,0x2F,0x4E,0x47,0x04,0x2F,0x61,0x63,0x74,0xCB,0x5B,0x00,0xC7,0x58,0x04,0x50
	,0x6C,0x61,0x79,0x7F,0x53,0x00,0x7F,0x53,0x00,0x6E,0x53,0x05,0x53,0x70,0x65,0x65,0x64,0xF6,0x73,0x00
	,0x09,0x91,0x00,0x04,0x3D,0x00,0x07,0x12,0x00,0x7F,0x51,0x00,0x0A,0xAA,0x02,0x44,0x65,0x04,0xC3,0x0B
	,0x43,0x61,0x70,0x61,0x62,0x69,0x6C,0x69,0x74,0x69,0x65,0xFF,0xA8,0x00,0xFF,0xA8,0x00,0xEF,0xA8,0x00
	,0x84,0x81,0x05,0x4D,0x65,0x64,0x69,0x61,0x77,0xA9,0x08,0x50,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x84
	,0x93,0x0B,0x62,0x61,0x63,0x6B,0x53,0x74,0x6F,0x72,0x61,0x67,0x65,0xC7,0x14,0x00,0x70,0xCC,0x03,0x52
	,0x65,0x63,0x7F,0x23,0x00,0x45,0x23,0x06,0x52,0x65,0x63,0x6F,0x72,0x64,0xFF,0x22,0x0E,0x65,0x63,0x51
	,0x75,0x61,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64,0x65,0x73,0xBF,0x24,0x00,0x86,0x24,0x00,0x4E,0x14,0x00
	,0xBF,0xA0,0x00,0x8A,0xA0,0x00,0x05,0x70,0x04,0x49,0x6E,0x66,0x6F,0xFF,0xF3,0x00,0xFF,0xF3,0x00,0xEE
	,0xF3,0x07,0x4E,0x72,0x54,0x72,0x61,0x63,0x6B,0x78,0x56,0x08,0x4E,0x75,0x6D,0x62,0x65,0x72,0x4F,0x66
	,0x48,0x11,0x00,0x30,0xBE,0x00,0x05,0xBD,0x04,0x44,0x75,0x72,0x61,0x44,0xF5,0x00,0x37,0xBF,0x07,0x43
	,0x75,0x72,0x72,0x65,0x6E,0x74,0xCF,0x12,0x00,0xB0,0xE0,0x00,0x87,0x11,0x03,0x55,0x52,0x49,0xF7,0xE0
	,0x0A,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x46,0x11,0x00,0x7A,0x20,0x08,0x4D,0x65,0x74
	,0x61,0x44,0x61,0x74,0x61,0x7F,0x22,0x02,0x6F,0x72,0x4E,0x13,0x00,0xF1,0x86,0x02,0x65,0x78,0xFB,0x43
	,0x00,0x84,0x0F,0x00,0xFF,0x44,0x00,0x88,0x20,0x00,0x3F,0x44,0x00,0x92,0x22,0x00,0x3A,0x45,0x04,0x50
	,0x6C,0x61,0x79,0x04,0xFA,0x02,0x75,0x6D,0x77,0xCC,0x00,0x44,0x10,0x0B,0x62,0x61,0x63,0x6B,0x53,0x74
	,0x6F,0x72,0x61,0x67,0x65,0x08,0x13,0x00,0xF0,0xED,0x06,0x52,0x65,0x63,0x6F,0x72,0x64,0xBD,0x22,0x00
	,0xC6,0x10,0x00,0x3F,0x22,0x05,0x57,0x72,0x69,0x74,0x65,0x84,0xFB,0x02,0x75,0x73,0xFD,0x21,0x00,0x06
	,0x55,0x00,0x8D,0x13,0x00,0x61,0xF0,0x00,0x09,0xF3,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x87,0xE7,0x00
	,0x08,0x02,0x00,0x46,0x6C,0x05,0x72,0x65,0x76,0x69,0x6F,0x8A,0x27,0x00,0x0E,0x0D,0x00,0xCF,0xE0,0x0A
	,0x49,0x6E,0x73,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0xD2,0xDE,0x02,0x69,0x6E,0xA3,0xDE,0x0A,0x5F,0x41
	,0x52,0x47,0x5F,0x54,0x59,0x50,0x45,0x5F,0xCC,0x12,0x00,0xBF,0x32,0x00,0xCB,0xE3,0x00,0xBF,0x31,0x00
	,0xBF,0x31,0x00,0x3F,0x64,0x00,0xC5,0xF4,0x03,0x47,0x65,0x74,0x89,0xE2,0x07,0x53,0x65,0x74,0x74,0x69
	,0x6E,0x67,0x3F,0x67,0x00,0x3F,0x67,0x00,0x6F,0xDE,0x00,0x04,0xF0,0x04,0x4D,0x6F,0x64,0x65,0xF7,0xFF
	,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x8A,0x11,0x00,0x73,0xFE,0x06,0x51,0x75,0x61,0x6C,0x69,0x74
	,0xBF,0x21,0x00,0x84,0x21,0x00,0xC6,0xDE,0x00,0xCD,0x13,0x00,0x7F,0xDD,0x00,0x47,0xDD,0x05,0x53,0x65
	,0x74,0x41,0x56,0xC9,0x79,0x03,0x55,0x52,0x49,0xBF,0x78,0x00,0xBF,0x78,0x00,0xAE,0x78,0x00,0xC7,0x68
	,0x03,0x55,0x52,0x49,0xF7,0xCF,0x00,0x8F,0x3E,0x00,0x3A,0x20,0x08,0x4D,0x65,0x74,0x61,0x44,0x61,0x74
	,0x61,0x3F,0x22,0x01,0x72,0x0E,0x13,0x00,0x3F,0xF2,0x00,0x08,0xC2,0x04,0x61,0x75,0x73,0x65,0x7F,0xEE
	,0x00,0x7F,0xEE,0x00,0xBF,0xAA,0x00,0xC5,0xF3,0x07,0x47,0x65,0x74,0x50,0x6F,0x73,0x69,0x44,0xF0,0x04
	,0x49,0x6E,0x66,0x6F,0x3F,0xAA,0x00,0x3F,0xAA,0x00,0x2E,0xAA,0x05,0x54,0x72,0x61,0x63,0x6B,0x92,0xCA
	,0x03,0x6F,0x75,0x74,0xE2,0xCA,0x00,0x07,0xB9,0x00,0xC7,0x10,0x00,0xB5,0x1E,0x04,0x44,0x75,0x72,0x61
	,0xC4,0xFE,0x00,0xBF,0x20,0x00,0xCE,0x12,0x00,0x35,0x41,0x00,0xDA,0xC9,0x00,0x31,0x43,0x00,0xAB,0xC9
	,0x00,0x94,0x63,0x03,0x55,0x52,0x49,0x7F,0x64,0x00,0x89,0x11,0x00,0xB0,0x83,0x07,0x52,0x65,0x6C,0x54
	,0x69,0x6D,0x65,0x37,0x84,0x01,0x52,0x44,0xDE,0x03,0x69,0x76,0x65,0xC4,0x10,0x00,0x08,0xC4,0x00,0xB2
	,0xA4,0x03,0x41,0x62,0x73,0x3B,0x21,0x07,0x41,0x62,0x73,0x6F,0x6C,0x75,0x74,0x3F,0x21,0x08,0x52,0x65
	,0x6C,0x43,0x6F,0x75,0x6E,0x74,0x7F,0x42,0x00,0x05,0x11,0x02,0x65,0x72,0x3D,0x43,0x00,0x3C,0x22,0x00
	,0x48,0x43,0x00,0x32,0x22,0x00,0xC9,0xED,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x87,0xE1,0x00,0x08,0x02
	,0x00,0x85,0xF0,0x04,0x53,0x65,0x65,0x6B,0x48,0xEE,0x00,0x0E,0x0C,0x00,0xCF,0xFA,0x0A,0x49,0x6E,0x73
	,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0x12,0xFA,0x02,0x69,0x6E,0x63,0x75,0x0A,0x5F,0x41,0x52,0x47,0x5F
	,0x54,0x59,0x50,0x45,0x5F,0xCC,0x12,0x00,0x30,0xFA,0x04,0x55,0x6E,0x69,0x74,0x7F,0x20,0x02,0x45,0x5F
	,0x44,0x3D,0x04,0x4D,0x6F,0x64,0x65,0x73,0xF7,0x04,0x61,0x72,0x67,0x65,0x7F,0x20,0x00,0x47,0x20,0x00
	,0xC8,0x12,0x00,0x3F,0x72,0x00,0x87,0xFD,0x10,0x47,0x65,0x74,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72
	,0x74,0x49,0x6E,0x66,0x6F,0x3F,0x75,0x00,0x3F,0x75,0x00,0xAE,0xEA,0x06,0x43,0x75,0x72,0x72,0x65,0x6E
	,0x0A,0x30,0x00,0xC5,0xF8,0x00,0xF7,0xED,0x00,0x50,0x11,0x00,0x3F,0x23,0x00,0x05,0x23,0x02,0x75,0x73
	,0x7F,0x23,0x00,0x89,0x11,0x00,0xB7,0x46,0x05,0x53,0x70,0x65,0x65,0x64,0x7F,0x44,0x05,0x74,0x50,0x6C
	,0x61,0x79,0x47,0x12,0x00,0xFF,0x9C,0x00,0xC7,0xE2,0x02,0x53,0x65,0x05,0x15,0x03,0x4D,0x6F,0x64,0x49
	,0x6B,0x00,0xBF,0x9B,0x00,0xBF,0x9B,0x00,0xE6,0xF0,0x03,0x4E,0x65,0x77,0xD0,0x2D,0x00,0x2E,0xF2,0x00
	,0xC7,0xAB,0x00,0x0A,0x3F,0x00,0xFF,0x53,0x00,0xC8,0x53,0x03,0x74,0x6F,0x70,0xBF,0xED,0x00,0xBF,0xED
	,0x00,0x77,0x85,0x00,0x87,0x87,0x00,0xC6,0x8A,0x07,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x05,0xFF,0x01
	,0x54,0x46,0xFE,0x01,0x73,0x4C,0xE8,0x10,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D
	,0x22,0x6E,0x6F,0x22,0xCE,0xDD,0x00,0x51,0x66,0x10,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74
	,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x0D,0x3C,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x56,0x61,0x6C
	,0x75,0x65,0xC7,0x9B,0x00,0x8B,0x04,0x09,0x3E,0x4E,0x4F,0x52,0x4D,0x41,0x4C,0x3C,0x2F,0x4D,0x05,0x00
	,0xCE,0x08,0x0A,0x52,0x45,0x50,0x45,0x41,0x54,0x5F,0x4F,0x4E,0x45,0xE4,0x09,0x02,0x41,0x4C,0x9F,0x13
	,0x05,0x41,0x4E,0x44,0x4F,0x4D,0x50,0x1C,0x01,0x2F,0xD2,0x29,0x07,0x64,0x65,0x66,0x61,0x75,0x6C,0x74
	,0xCE,0x29,0x00,0x4D,0x05,0x03,0x3C,0x2F,0x73,0x8E,0xE6,0x00,0xA4,0x50,0x13,0x52,0x65,0x63,0x6F,0x72
	,0x64,0x53,0x74,0x6F,0x72,0x61,0x67,0x65,0x4D,0x65,0x64,0x69,0x75,0x6D,0xBF,0x51,0x0A,0x75,0x65,0x3E
	,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0xDD,0x51,0x02,0x44,0x56,0x9D,0x59,0x05,0x4D,0x49,0x4E,0x49,0x2D
	,0x1F,0x09,0x03,0x56,0x48,0x53,0x9D,0x6A,0x02,0x57,0x2D,0xA0,0x08,0x01,0x53,0xA1,0x08,0x01,0x44,0x21
	,0x11,0x04,0x56,0x48,0x53,0x43,0xDE,0x29,0x05,0x49,0x44,0x45,0x4F,0x38,0x1D,0x95,0x02,0x48,0x49,0x1E
	,0x08,0x06,0x43,0x44,0x2D,0x52,0x4F,0x4D,0xE0,0x08,0x02,0x44,0x41,0x61,0x11,0x00,0xA1,0x19,0x01,0x57
	,0xE2,0x32,0x03,0x2D,0x43,0x44,0x5E,0x55,0x01,0x41,0x5F,0x08,0x08,0x4D,0x44,0x2D,0x41,0x55,0x44,0x49
	,0x4F,0x60,0x09,0x07,0x50,0x49,0x43,0x54,0x55,0x52,0x45,0xDF,0x91,0x00,0xE2,0x46,0x00,0x04,0x09,0x00
	,0x45,0x61,0x00,0xA2,0x12,0x00,0x20,0x1B,0x03,0x2B,0x52,0x57,0xE2,0x23,0x00,0xE3,0x08,0x01,0x41,0xA2
	,0x2C,0x00,0xE2,0x48,0x03,0x44,0x41,0x54,0xDD,0xD8,0x01,0x4C,0x1E,0x6A,0x02,0x48,0x44,0xDF,0x69,0x06
	,0x49,0x43,0x52,0x4F,0x2D,0x4D,0x1E,0xEA,0x07,0x4E,0x45,0x54,0x57,0x4F,0x52,0x4B,0x1E,0x09,0x03,0x4F
	,0x4E,0x45,0x5F,0x08,0x0C,0x54,0x5F,0x49,0x4D,0x50,0x4C,0x45,0x4D,0x45,0x4E,0x54,0x45,0x91,0x97,0x00
	,0x0D,0xF9,0x16,0x4C,0x69,0x73,0x74,0x3E,0x3C,0x2F,0x73,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61
	,0x62,0x6C,0x65,0x3E,0x3C,0xCD,0x03,0x24,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D
	,0x22,0x79,0x65,0x73,0x22,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x4C,0x61,0x73,0x74,0x43,0x68,0x61,0x6E
	,0x67,0x65,0x3C,0x2F,0x45,0x04,0x12,0x3C,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72
	,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x00,0xAB,0x18,0x02,0x6E,0x6F,0x48,0x18,0x14,0x52,0x65,0x6C,0x61
	,0x74,0x69,0x76,0x65,0x54,0x69,0x6D,0x65,0x50,0x6F,0x73,0x69,0x74,0x69,0x6F,0x6E,0xFF,0x1A,0x00,0xD8
	,0x1A,0x0F,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x54,0x72,0x61,0x63,0x6B,0x55,0x52,0x49,0xBF,0x19,0x00
	,0xA4,0x19,0x04,0x44,0x75,0x72,0x61,0x7F,0x34,0x00,0x63,0x34,0x10,0x52,0x65,0x63,0x6F,0x72,0x64,0x51
	,0x75,0x61,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64,0xE4,0x6A,0x00,0xD2,0x87,0x00,0xCD,0xFF,0x04,0x30,0x3A
	,0x45,0x50,0x9D,0xFE,0x03,0x31,0x3A,0x4C,0x5E,0x08,0x03,0x32,0x3A,0x53,0x9E,0x10,0x07,0x30,0x3A,0x42
	,0x41,0x53,0x49,0x43,0x9F,0x19,0x06,0x4D,0x45,0x44,0x49,0x55,0x4D,0x9F,0x1A,0x04,0x48,0x49,0x47,0x48
	,0x7F,0xCB,0x00,0x7A,0xCB,0x00,0x11,0x98,0x05,0x4D,0x65,0x64,0x69,0x61,0xBF,0x7E,0x00,0xA0,0xCD,0x0F
	,0x41,0x62,0x73,0x6F,0x6C,0x75,0x74,0x65,0x43,0x6F,0x75,0x6E,0x74,0x65,0x72,0x59,0xCE,0x02,0x69,0x34
	,0x3F,0xE8,0x00,0x09,0xE8,0x00,0xBF,0x1A,0x00,0x24,0x35,0x14,0x5F,0x41,0x52,0x47,0x5F,0x54,0x59,0x50
	,0x45,0x5F,0x49,0x6E,0x73,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0x51,0xE9,0x01,0x75,0x7F,0x1A,0x00,0x44
	,0x4F,0x0D,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x55,0x52,0x49,0x3F,0xE7,0x00,0x1F,0xE7
	,0x0D,0x54,0x72,0x61,0x63,0x6B,0x4D,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x7F,0x83,0x00,0x26,0x34,0x00
	,0x7F,0x1B,0x00,0x60,0xB9,0x00,0xC9,0x4E,0x09,0x50,0x6C,0x61,0x79,0x53,0x70,0x65,0x65,0x64,0xE3,0xB8
	,0x00,0x12,0xD8,0x00,0xCE,0xF9,0x00,0x7F,0xE4,0x00,0x58,0xE4,0x04,0x4E,0x65,0x78,0x74,0x7F,0x7B,0x00
	,0xA6,0xFE,0x1A,0x50,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x52,0x65,0x63,0x6F,0x72,0x64,0x51,0x75,0x61
	,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64,0x65,0x73,0x7F,0x1C,0x00,0x66,0x1C,0x0B,0x53,0x74,0x6F,0x72,0x61
	,0x67,0x65,0x4D,0x65,0x64,0x69,0x3F,0x99,0x00,0x5A,0xE7,0x13,0x62,0x73,0x6F,0x6C,0x75,0x74,0x65,0x54
	,0x69,0x6D,0x65,0x50,0x6F,0x73,0x69,0x74,0x69,0x6F,0x6E,0xBF,0xCE,0x00,0xAA,0x6D,0x00,0x3F,0xD0,0x00
	,0xA1,0x6F,0x07,0x6C,0x61,0x79,0x62,0x61,0x63,0x6B,0xCB,0x51,0x02,0x75,0x6D,0xBF,0xB5,0x0A,0x75,0x65
	,0x3E,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0x10,0xB7,0x00,0x8D,0xBE,0x02,0x44,0x56,0xDD,0x07,0x05,0x4D
	,0x49,0x4E,0x49,0x2D,0x1F,0x09,0x03,0x56,0x48,0x53,0xDD,0x18,0x02,0x57,0x2D,0xA0,0x08,0x01,0x53,0xA1
	,0x08,0x01,0x44,0x21,0x11,0x04,0x56,0x48,0x53,0x43,0xDE,0x29,0x05,0x49,0x44,0x45,0x4F,0x38,0x5D,0x43
	,0x02,0x48,0x49,0x1E,0x08,0x06,0x43,0x44,0x2D,0x52,0x4F,0x4D,0xE0,0x08,0x02,0x44,0x41,0x61,0x11,0x00
	,0xA1,0x19,0x01,0x57,0xE2,0x32,0x03,0x2D,0x43,0x44,0x5E,0x55,0x01,0x41,0x5F,0x08,0x08,0x4D,0x44,0x2D
	,0x41,0x55,0x44,0x49,0x4F,0x60,0x09,0x07,0x50,0x49,0x43,0x54,0x55,0x52,0x45,0xDF,0x91,0x00,0xE2,0x46
	,0x00,0x04,0x09,0x00,0x45,0x61,0x00,0xA2,0x12,0x00,0x20,0x1B,0x03,0x2B,0x52,0x57,0xE2,0x23,0x00,0xE3
	,0x08,0x01,0x41,0xA2,0x2C,0x00,0xE2,0x48,0x03,0x44,0x41,0x54,0xDD,0xD8,0x01,0x4C,0x1E,0x6A,0x02,0x48
	,0x44,0xDF,0x69,0x06,0x49,0x43,0x52,0x4F,0x2D,0x4D,0x1E,0xEA,0x07,0x4E,0x45,0x54,0x57,0x4F,0x52,0x4B
	,0x1E,0x09,0x03,0x4F,0x4E,0x45,0x5F,0x08,0x0C,0x54,0x5F,0x49,0x4D,0x50,0x4C,0x45,0x4D,0x45,0x4E,0x54
	,0x45,0x91,0x97,0x00,0x0D,0xF9,0x16,0x4C,0x69,0x73,0x74,0x3E,0x3C,0x2F,0x73,0x74,0x61,0x74,0x65,0x56
	,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x3C,0xCD,0x03,0x30,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65
	,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x43,0x75,0x72,0x72,0x65
	,0x6E,0x74,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x41,0x63,0x74,0x69,0x6F,0x6E,0x73,0x3C,0x2F
	,0x85,0x07,0x12,0x3C,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C
	,0x2F,0x49,0x04,0x00,0xB5,0x1B,0x16,0x52,0x65,0x63,0x6F,0x72,0x64,0x4D,0x65,0x64,0x69,0x75,0x6D,0x57
	,0x72,0x69,0x74,0x65,0x53,0x74,0x61,0x74,0x75,0xA4,0x1B,0x00,0x92,0x3B,0x00,0xCD,0xFD,0x07,0x57,0x52
	,0x49,0x54,0x41,0x42,0x4C,0xDE,0xC5,0x09,0x50,0x52,0x4F,0x54,0x45,0x43,0x54,0x45,0x44,0x21,0x5E,0x00
	,0xE5,0x13,0x07,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0x7F,0x71,0x00,0x7F,0x71,0x00,0x45,0x71,0x17,0x50
	,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x50,0x6C,0x61,0x79,0x62,0x61,0x63,0x6B,0x53,0x74,0x6F,0x72,0x61
	,0x67,0x65,0x04,0x5A,0x01,0x61,0xBF,0x72,0x00,0x18,0x8E,0x00,0x49,0x8C,0x00,0x84,0x70,0x01,0x65,0x7F
	,0x70,0x08,0x75,0x65,0x3E,0x53,0x54,0x4F,0x50,0x50,0x9F,0x66,0x0F,0x50,0x41,0x55,0x53,0x45,0x44,0x5F
	,0x50,0x4C,0x41,0x59,0x42,0x41,0x43,0x4B,0x24,0x0B,0x09,0x52,0x45,0x43,0x4F,0x52,0x44,0x49,0x4E,0x47
	,0x5E,0x86,0x03,0x4C,0x41,0x59,0x20,0x09,0x00,0xA6,0x12,0x0A,0x54,0x52,0x41,0x4E,0x53,0x49,0x54,0x49
	,0x4F,0x4E,0x20,0x1D,0x10,0x4E,0x4F,0x5F,0x4D,0x45,0x44,0x49,0x41,0x5F,0x50,0x52,0x45,0x53,0x45,0x4E
	,0x54,0x3F,0xF8,0x00,0x18,0xF8,0x0D,0x4E,0x75,0x6D,0x62,0x65,0x72,0x4F,0x66,0x54,0x72,0x61,0x63,0x6B
	,0xD2,0xF5,0x03,0x75,0x69,0x34,0x98,0xD9,0x12,0x52,0x61,0x6E,0x67,0x65,0x3E,0x3C,0x6D,0x69,0x6E,0x69
	,0x6D,0x75,0x6D,0x3E,0x30,0x3C,0x2F,0xC8,0x02,0x04,0x3C,0x6D,0x61,0x78,0x05,0x05,0x06,0x34,0x30,0x30
	,0x30,0x3C,0x2F,0x88,0x03,0x02,0x3C,0x2F,0xD3,0x0F,0x00,0xF4,0xB3,0x12,0x41,0x5F,0x41,0x52,0x47,0x5F
	,0x54,0x59,0x50,0x45,0x5F,0x53,0x65,0x65,0x6B,0x4D,0x6F,0x64,0x7F,0x98,0x00,0x06,0x70,0x06,0x4C,0x5F
	,0x54,0x49,0x4D,0x45,0xE0,0x6F,0x05,0x43,0x4B,0x5F,0x4E,0x52,0x3F,0xEA,0x00,0x18,0xEA,0x07,0x43,0x75
	,0x72,0x72,0x65,0x6E,0x74,0x05,0x63,0x00,0xFF,0x62,0x00,0xDF,0x62,0x08,0x73,0x74,0x65,0x70,0x3E,0x31
	,0x3C,0x2F,0x05,0x02,0x00,0x7F,0x66,0x00,0x57,0xFD,0x02,0x75,0x73,0xBF,0xFD,0x04,0x75,0x65,0x3E,0x4F
	,0x5E,0xF1,0x0E,0x45,0x52,0x52,0x4F,0x52,0x5F,0x4F,0x43,0x43,0x55,0x52,0x52,0x45,0x44,0xBF,0xC8,0x00
	,0xA7,0x9B,0x06,0x54,0x61,0x72,0x67,0x65,0x74,0x23,0x9C,0x00,0x90,0xE3,0x08,0x2F,0x73,0x65,0x72,0x76
	,0x69,0x63,0x65,0x04,0x47,0x02,0x65,0x54,0x08,0x05,0x01,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00,0x00
};

struct UpnpDataObject;

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
struct UpnpDataObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	
	void *EventClient;
	void *Chain;
	int UpdateFlag;
	
	
	int ForceExit;
	char *UUID;
	char *UDN;
	char *Serial;
	
	void *WebServerTimer;
	void *HTTPServer;
	
	char *DeviceDescription;
	int DeviceDescriptionLength;
	int InitialNotify;
	char* MediaRenderer_RenderingControl_LastChange;
	char* MediaRenderer_ConnectionManager_SourceProtocolInfo;
	char* MediaRenderer_ConnectionManager_SinkProtocolInfo;
	char* MediaRenderer_ConnectionManager_CurrentConnectionIDs;
	char* MediaRenderer_AVTransport_LastChange;
	struct sockaddr_in addr;
	int addrlen;
	SOCKET MSEARCH_sock;
	struct ip_mreq mreq;
	char message[4096];
	int *AddressList;
	int AddressListLength;
	
	int _NumEmbeddedDevices;
	int WebSocketPortNumber;
	SOCKET *NOTIFY_SEND_socks;
	SOCKET NOTIFY_RECEIVE_sock;
	
	int SID;
	
	unsigned int CurrentTime;
	int NotifyCycleTime;
	unsigned int NotifyTime;
	
	sem_t EventLock;
	struct SubscriberInfo *HeadSubscriberPtr_MediaRenderer_RenderingControl;
	int NumberOfSubscribers_MediaRenderer_RenderingControl;
	struct SubscriberInfo *HeadSubscriberPtr_MediaRenderer_ConnectionManager;
	int NumberOfSubscribers_MediaRenderer_ConnectionManager;
	struct SubscriberInfo *HeadSubscriberPtr_MediaRenderer_AVTransport;
	int NumberOfSubscribers_MediaRenderer_AVTransport;
};

struct MSEARCH_state
{
	char *ST;
	int STLength;
	void *upnp;
	struct sockaddr_in dest_addr;
};

/* Pre-declarations */
void UpnpSendNotify(const struct UpnpDataObject *upnp);
void UpnpSendByeBye();
void UpnpMainInvokeSwitch();
void UpnpSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UpnpSendData(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
int UpnpPeriodicNotify(struct UpnpDataObject *upnp);
void UpnpSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);

void* UpnpGetWebServerToken(const void *MicroStackToken)
{
	return(((struct UpnpDataObject*)MicroStackToken)->HTTPServer);
}

#define UpnpBuildSsdpResponsePacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"HTTP/1.1 200 OK\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nEXT:\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,ST,NTex);\
}

#define UpnpBuildSsdpNotifyPacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n",(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,NT,NTex);\
}

void UpnpIPAddressListChanged(void *MicroStackToken)
{
	((struct UpnpDataObject*)MicroStackToken)->UpdateFlag = 1;
	ILibForceUnBlockChain(((struct UpnpDataObject*)MicroStackToken)->Chain);
}
void UpnpInit(struct UpnpDataObject *state,const int NotifyCycleSeconds,const unsigned short PortNumber)
{
	int ra = 1;
	int i;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	/* Complete State Reset */
	memset(state,0,sizeof(struct UpnpDataObject));
	
	/* Setup Notification Timer */
	state->NotifyCycleTime = NotifyCycleSeconds;
	state->CurrentTime = GetTickCount() / 1000;
	state->NotifyTime = state->CurrentTime  + (state->NotifyCycleTime/2);
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
void UpnpPostMX_Destroy(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	FREE(mss->ST);
	FREE(mss);
}
void UpnpPostMX_MSEARCH(void *object)
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
	struct UpnpDataObject *upnp = (struct UpnpDataObject*)mss->upnp;
	
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
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	else if(STLength==8 && memcmp(ST,"ssdp:all",8)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength==(int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=43 && memcmp(ST,"urn:schemas-upnp-org:device:MediaRenderer:1",43)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=48 && memcmp(ST,"urn:schemas-upnp-org:service:ConnectionManager:1",48)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=47 && memcmp(ST,"urn:schemas-upnp-org:service:RenderingControl:1",47)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=42 && memcmp(ST,"urn:schemas-upnp-org:service:AVTransport:1",42)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UpnpBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
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
	FREE(b);
}
void UpnpProcessMSEARCH(struct UpnpDataObject *upnp, struct packetheader *packet)
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
				
				ILibLifeTime_Add(upnp->WebServerTimer,mss,MX,&UpnpPostMX_MSEARCH,&UpnpPostMX_Destroy);
			}
			else
			{
				FREE(ST);
			}
		}
	}
}
void UpnpDispatch_MediaRenderer_RenderingControl_SetVolume(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredVolume = NULL;
	int p_DesiredVolumeLength = 0;
	unsigned short _DesiredVolume = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==13 && memcmp(VarName,"DesiredVolume",13) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredVolume = temp3->LastResult->data;
					p_DesiredVolumeLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	OK = ILibGetULong(p_DesiredVolume,p_DesiredVolumeLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UpnpResponse_Error(ReaderObject,402,"Illegal value");
			return;
		}
		_DesiredVolume = (unsigned short)TempULong;
	}
	UpnpMediaRenderer_RenderingControl_SetVolume((void*)ReaderObject,_InstanceID,_Channel,_DesiredVolume);
}

void UpnpDispatch_MediaRenderer_RenderingControl_GetMute(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UpnpMediaRenderer_RenderingControl_GetMute((void*)ReaderObject,_InstanceID,_Channel);
}

void UpnpDispatch_MediaRenderer_RenderingControl_SetMute(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredMute = NULL;
	int p_DesiredMuteLength = 0;
	int _DesiredMute = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==11 && memcmp(VarName,"DesiredMute",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredMute = temp3->LastResult->data;
					p_DesiredMuteLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	OK=0;
	if(p_DesiredMuteLength==4)
	{
		if(_strnicmp(p_DesiredMute,"true",4)==0)
		{
			OK = 1;
			_DesiredMute = 1;
		}
	}
	if(p_DesiredMuteLength==5)
	{
		if(_strnicmp(p_DesiredMute,"false",5)==0)
		{
			OK = 1;
			_DesiredMute = 0;
		}
	}
	if(p_DesiredMuteLength==1)
	{
		if(memcmp(p_DesiredMute,"0",1)==0)
		{
			OK = 1;
			_DesiredMute = 0;
		}
		if(memcmp(p_DesiredMute,"1",1)==0)
		{
			OK = 1;
			_DesiredMute = 1;
		}
	}
	if(OK==0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UpnpMediaRenderer_RenderingControl_SetMute((void*)ReaderObject,_InstanceID,_Channel,_DesiredMute);
}

void UpnpDispatch_MediaRenderer_RenderingControl_GetVolume(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UpnpMediaRenderer_RenderingControl_GetVolume((void*)ReaderObject,_InstanceID,_Channel);
}

void UpnpDispatch_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	int OK = 0;
	char *p_ConnectionID = NULL;
	int p_ConnectionIDLength = 0;
	int _ConnectionID = 0;
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
			if(VarNameLength==12 && memcmp(VarName,"ConnectionID",12) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ConnectionID = temp3->LastResult->data;
					p_ConnectionIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo((void*)ReaderObject,_ConnectionID);
}

#define UpnpDispatch_MediaRenderer_ConnectionManager_GetProtocolInfo(xml, session)\
{\
	UpnpMediaRenderer_ConnectionManager_GetProtocolInfo((void*)session);\
}

#define UpnpDispatch_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(xml, session)\
{\
	UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs((void*)session);\
}

void UpnpDispatch_MediaRenderer_AVTransport_GetCurrentTransportActions(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetCurrentTransportActions((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_Play(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Speed = NULL;
	int p_SpeedLength = 0;
	char* _Speed = "";
	int _SpeedLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==5 && memcmp(VarName,"Speed",5) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Speed = temp3->LastResult->data;
					p_SpeedLength = temp3->LastResult->datalength;
					p_Speed[p_SpeedLength] = 0;
				}
				else
				{
					p_Speed = temp3->LastResult->data;
					p_SpeedLength = 0;
					p_Speed[p_SpeedLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_SpeedLength = ILibInPlaceXmlUnEscape(p_Speed);
	_Speed = p_Speed;
	if(memcmp(_Speed, "1\0",2) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UpnpMediaRenderer_AVTransport_Play((void*)ReaderObject,_InstanceID,_Speed);
}

void UpnpDispatch_MediaRenderer_AVTransport_GetDeviceCapabilities(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetDeviceCapabilities((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_GetMediaInfo(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetMediaInfo((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_Previous(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_Previous((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_Next(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_Next((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_GetTransportSettings(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetTransportSettings((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_SetAVTransportURI(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_CurrentURI = NULL;
	int p_CurrentURILength = 0;
	char* _CurrentURI = "";
	int _CurrentURILength;
	char *p_CurrentURIMetaData = NULL;
	int p_CurrentURIMetaDataLength = 0;
	char* _CurrentURIMetaData = "";
	int _CurrentURIMetaDataLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==10 && memcmp(VarName,"CurrentURI",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_CurrentURI = temp3->LastResult->data;
					p_CurrentURILength = temp3->LastResult->datalength;
					p_CurrentURI[p_CurrentURILength] = 0;
				}
				else
				{
					p_CurrentURI = temp3->LastResult->data;
					p_CurrentURILength = 0;
					p_CurrentURI[p_CurrentURILength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==18 && memcmp(VarName,"CurrentURIMetaData",18) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_CurrentURIMetaData = temp3->LastResult->data;
					p_CurrentURIMetaDataLength = temp3->LastResult->datalength;
					p_CurrentURIMetaData[p_CurrentURIMetaDataLength] = 0;
				}
				else
				{
					p_CurrentURIMetaData = temp3->LastResult->data;
					p_CurrentURIMetaDataLength = 0;
					p_CurrentURIMetaData[p_CurrentURIMetaDataLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_CurrentURILength = ILibInPlaceXmlUnEscape(p_CurrentURI);
	_CurrentURI = p_CurrentURI;
	_CurrentURIMetaDataLength = ILibInPlaceXmlUnEscape(p_CurrentURIMetaData);
	_CurrentURIMetaData = p_CurrentURIMetaData;
	UpnpMediaRenderer_AVTransport_SetAVTransportURI((void*)ReaderObject,_InstanceID,_CurrentURI,_CurrentURIMetaData);
}

void UpnpDispatch_MediaRenderer_AVTransport_Pause(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_Pause((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_GetPositionInfo(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetPositionInfo((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_Seek(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Unit = NULL;
	int p_UnitLength = 0;
	char* _Unit = "";
	int _UnitLength;
	char *p_Target = NULL;
	int p_TargetLength = 0;
	char* _Target = "";
	int _TargetLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==4 && memcmp(VarName,"Unit",4) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Unit = temp3->LastResult->data;
					p_UnitLength = temp3->LastResult->datalength;
					p_Unit[p_UnitLength] = 0;
				}
				else
				{
					p_Unit = temp3->LastResult->data;
					p_UnitLength = 0;
					p_Unit[p_UnitLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==6 && memcmp(VarName,"Target",6) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Target = temp3->LastResult->data;
					p_TargetLength = temp3->LastResult->datalength;
					p_Target[p_TargetLength] = 0;
				}
				else
				{
					p_Target = temp3->LastResult->data;
					p_TargetLength = 0;
					p_Target[p_TargetLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_UnitLength = ILibInPlaceXmlUnEscape(p_Unit);
	_Unit = p_Unit;
	if(memcmp(_Unit, "REL_TIME\0",9) != 0
	&& memcmp(_Unit, "TRACK_NR\0",9) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_TargetLength = ILibInPlaceXmlUnEscape(p_Target);
	_Target = p_Target;
	UpnpMediaRenderer_AVTransport_Seek((void*)ReaderObject,_InstanceID,_Unit,_Target);
}

void UpnpDispatch_MediaRenderer_AVTransport_GetTransportInfo(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_GetTransportInfo((void*)ReaderObject,_InstanceID);
}

void UpnpDispatch_MediaRenderer_AVTransport_SetPlayMode(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_NewPlayMode = NULL;
	int p_NewPlayModeLength = 0;
	char* _NewPlayMode = "";
	int _NewPlayModeLength;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==11 && memcmp(VarName,"NewPlayMode",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_NewPlayMode = temp3->LastResult->data;
					p_NewPlayModeLength = temp3->LastResult->datalength;
					p_NewPlayMode[p_NewPlayModeLength] = 0;
				}
				else
				{
					p_NewPlayMode = temp3->LastResult->data;
					p_NewPlayModeLength = 0;
					p_NewPlayMode[p_NewPlayModeLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_NewPlayModeLength = ILibInPlaceXmlUnEscape(p_NewPlayMode);
	_NewPlayMode = p_NewPlayMode;
	if(memcmp(_NewPlayMode, "NORMAL\0",7) != 0
	&& memcmp(_NewPlayMode, "REPEAT_ONE\0",11) != 0
	&& memcmp(_NewPlayMode, "REPEAT_ALL\0",11) != 0
	&& memcmp(_NewPlayMode, "RANDOM\0",7) != 0
	)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UpnpMediaRenderer_AVTransport_SetPlayMode((void*)ReaderObject,_InstanceID,_NewPlayMode);
}

void UpnpDispatch_MediaRenderer_AVTransport_Stop(struct parser_result *xml, struct ILibWebServer_Session *ReaderObject)
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
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
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
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UpnpResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UpnpMediaRenderer_AVTransport_Stop((void*)ReaderObject,_InstanceID);
}

void UpnpProcessPOST(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)

{
	struct packetheader_field_node *f = header->FirstField;
	char* HOST;
	char* SOAPACTION = NULL;
	int SOAPACTIONLength = 0;
	struct parser_result *r;
	struct parser_result *xml;
	
	xml = ILibParseString(bodyBuffer,offset,bodyBufferLength,"<",1);
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
	if(header->DirectiveObjLength==40 && memcmp((header->DirectiveObj)+1,"MediaRenderer_ConnectionManager/control",39)==0)
	{
		if(SOAPACTIONLength==24 && memcmp(SOAPACTION,"GetCurrentConnectionInfo",24)==0)
		{
			UpnpDispatch_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(xml, session);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetProtocolInfo",15)==0)
		{
			UpnpDispatch_MediaRenderer_ConnectionManager_GetProtocolInfo(xml, session);
		}
		else if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"GetCurrentConnectionIDs",23)==0)
		{
			UpnpDispatch_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(xml, session);
		}
	}
	else if(header->DirectiveObjLength==39 && memcmp((header->DirectiveObj)+1,"MediaRenderer_RenderingControl/control",38)==0)
	{
		if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"SetVolume",9)==0)
		{
			UpnpDispatch_MediaRenderer_RenderingControl_SetVolume(xml, session);
		}
		else if(SOAPACTIONLength==7 && memcmp(SOAPACTION,"GetMute",7)==0)
		{
			UpnpDispatch_MediaRenderer_RenderingControl_GetMute(xml, session);
		}
		else if(SOAPACTIONLength==7 && memcmp(SOAPACTION,"SetMute",7)==0)
		{
			UpnpDispatch_MediaRenderer_RenderingControl_SetMute(xml, session);
		}
		else if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"GetVolume",9)==0)
		{
			UpnpDispatch_MediaRenderer_RenderingControl_GetVolume(xml, session);
		}
	}
	else if(header->DirectiveObjLength==34 && memcmp((header->DirectiveObj)+1,"MediaRenderer_AVTransport/control",33)==0)
	{
		if(SOAPACTIONLength==26 && memcmp(SOAPACTION,"GetCurrentTransportActions",26)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetCurrentTransportActions(xml, session);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Play",4)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Play(xml, session);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetDeviceCapabilities",21)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetDeviceCapabilities(xml, session);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"GetMediaInfo",12)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetMediaInfo(xml, session);
		}
		else if(SOAPACTIONLength==8 && memcmp(SOAPACTION,"Previous",8)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Previous(xml, session);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Next",4)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Next(xml, session);
		}
		else if(SOAPACTIONLength==20 && memcmp(SOAPACTION,"GetTransportSettings",20)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetTransportSettings(xml, session);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"SetAVTransportURI",17)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_SetAVTransportURI(xml, session);
		}
		else if(SOAPACTIONLength==5 && memcmp(SOAPACTION,"Pause",5)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Pause(xml, session);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetPositionInfo",15)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetPositionInfo(xml, session);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Seek",4)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Seek(xml, session);
		}
		else if(SOAPACTIONLength==16 && memcmp(SOAPACTION,"GetTransportInfo",16)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_GetTransportInfo(xml, session);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"SetPlayMode",11)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_SetPlayMode(xml, session);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Stop",4)==0)
		{
			UpnpDispatch_MediaRenderer_AVTransport_Stop(xml, session);
		}
	}
	ILibDestructParserResults(xml);
}
struct SubscriberInfo* UpnpRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers,char* SID, int SIDLength)
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

#define UpnpDestructSubscriberInfo(info)\
{\
	FREE(info->Path);\
	FREE(info->SID);\
	FREE(info);\
}

#define UpnpDestructEventObject(EvObject)\
{\
	FREE(EvObject->PacketBody);\
	FREE(EvObject);\
}

#define UpnpDestructEventDataObject(EvData)\
{\
	FREE(EvData);\
}
void UpnpExpireSubscriberInfo(struct UpnpDataObject *d, struct SubscriberInfo *info)
{
	struct SubscriberInfo *t = info;
	while(t->Previous!=NULL)
	{
		t = t->Previous;
	}
	if(d->HeadSubscriberPtr_MediaRenderer_RenderingControl==t)
	{
		--(d->NumberOfSubscribers_MediaRenderer_RenderingControl);
	}
	else if(d->HeadSubscriberPtr_MediaRenderer_ConnectionManager==t)
	{
		--(d->NumberOfSubscribers_MediaRenderer_ConnectionManager);
	}
	else if(d->HeadSubscriberPtr_MediaRenderer_AVTransport==t)
	{
		--(d->NumberOfSubscribers_MediaRenderer_AVTransport);
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
		if(d->HeadSubscriberPtr_MediaRenderer_RenderingControl==info)
		{
			d->HeadSubscriberPtr_MediaRenderer_RenderingControl = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_MediaRenderer_RenderingControl;
			}
		}
		else if(d->HeadSubscriberPtr_MediaRenderer_ConnectionManager==info)
		{
			d->HeadSubscriberPtr_MediaRenderer_ConnectionManager = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_MediaRenderer_ConnectionManager;
			}
		}
		else if(d->HeadSubscriberPtr_MediaRenderer_AVTransport==info)
		{
			d->HeadSubscriberPtr_MediaRenderer_AVTransport = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_MediaRenderer_AVTransport;
			}
		}
		else
		{
			// Error
			return;
		}
	}
	--info->RefCount;
	if(info->RefCount==0)
	{
		UpnpDestructSubscriberInfo(info);
	}
}

int UpnpSubscriptionExpired(struct SubscriberInfo *info)
{
	int RetVal = 0;
	if(info->RenewByTime < GetTickCount()/1000) {RetVal = -1;}
	return(RetVal);
}
void UpnpGetInitialEventBody_MediaRenderer_RenderingControl(struct UpnpDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(25+(int)strlen(UPnPObject->MediaRenderer_RenderingControl_LastChange));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"LastChange>%s</LastChange",UPnPObject->MediaRenderer_RenderingControl_LastChange);
}
void UpnpGetInitialEventBody_MediaRenderer_ConnectionManager(struct UpnpDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(177+(int)strlen(UPnPObject->MediaRenderer_ConnectionManager_SourceProtocolInfo)+(int)strlen(UPnPObject->MediaRenderer_ConnectionManager_SinkProtocolInfo)+(int)strlen(UPnPObject->MediaRenderer_ConnectionManager_CurrentConnectionIDs));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"SourceProtocolInfo>%s</SourceProtocolInfo></e:property><e:property><SinkProtocolInfo>%s</SinkProtocolInfo></e:property><e:property><CurrentConnectionIDs>%s</CurrentConnectionIDs",UPnPObject->MediaRenderer_ConnectionManager_SourceProtocolInfo,UPnPObject->MediaRenderer_ConnectionManager_SinkProtocolInfo,UPnPObject->MediaRenderer_ConnectionManager_CurrentConnectionIDs);
}
void UpnpGetInitialEventBody_MediaRenderer_AVTransport(struct UpnpDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(25+(int)strlen(UPnPObject->MediaRenderer_AVTransport_LastChange));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"LastChange>%s</LastChange",UPnPObject->MediaRenderer_AVTransport_LastChange);
}
void UpnpProcessUNSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
{
	char* SID = NULL;
	int SIDLength = 0;
	struct SubscriberInfo *Info;
	struct packetheader_field_node *f;
	char* packet = (char*)MALLOC(sizeof(char)*50);
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
	sem_wait(&(((struct UpnpDataObject*)session->User)->EventLock));
	if(header->DirectiveObjLength==37 && memcmp(header->DirectiveObj + 1,"MediaRenderer_RenderingControl/event",36)==0)
	{
		Info = UpnpRemoveSubscriberInfo(&(((struct UpnpDataObject*)session->User)->HeadSubscriberPtr_MediaRenderer_RenderingControl),&(((struct UpnpDataObject*)session->User)->NumberOfSubscribers_MediaRenderer_RenderingControl),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UpnpDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
	}
	else if(header->DirectiveObjLength==38 && memcmp(header->DirectiveObj + 1,"MediaRenderer_ConnectionManager/event",37)==0)
	{
		Info = UpnpRemoveSubscriberInfo(&(((struct UpnpDataObject*)session->User)->HeadSubscriberPtr_MediaRenderer_ConnectionManager),&(((struct UpnpDataObject*)session->User)->NumberOfSubscribers_MediaRenderer_ConnectionManager),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UpnpDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
	}
	else if(header->DirectiveObjLength==32 && memcmp(header->DirectiveObj + 1,"MediaRenderer_AVTransport/event",31)==0)
	{
		Info = UpnpRemoveSubscriberInfo(&(((struct UpnpDataObject*)session->User)->HeadSubscriberPtr_MediaRenderer_AVTransport),&(((struct UpnpDataObject*)session->User)->NumberOfSubscribers_MediaRenderer_AVTransport),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UpnpDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
		}
	}
	sem_post(&(((struct UpnpDataObject*)session->User)->EventLock));
}
void UpnpTryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength,struct ILibWebServer_Session *session)
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
	
	struct UpnpDataObject *dataObject = (struct UpnpDataObject*)session->User;
	
	if(strncmp(ServiceName,"MediaRenderer_RenderingControl",30)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_MediaRenderer_RenderingControl);
		HeadPtr = &(dataObject->HeadSubscriberPtr_MediaRenderer_RenderingControl);
	}
	if(strncmp(ServiceName,"MediaRenderer_ConnectionManager",31)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_MediaRenderer_ConnectionManager);
		HeadPtr = &(dataObject->HeadSubscriberPtr_MediaRenderer_ConnectionManager);
	}
	if(strncmp(ServiceName,"MediaRenderer_AVTransport",25)==0)
	{
		TotalSubscribers = &(dataObject->NumberOfSubscribers_MediaRenderer_AVTransport);
		HeadPtr = &(dataObject->HeadSubscriberPtr_MediaRenderer_AVTransport);
	}
	if(*HeadPtr!=NULL)
	{
		NewSubscriber = *HeadPtr;
		while(NewSubscriber!=NULL)
		{
			if(UpnpSubscriptionExpired(NewSubscriber)!=0)
			{
				TempSubscriber = NewSubscriber->Next;
				NewSubscriber = UpnpRemoveSubscriberInfo(HeadPtr,TotalSubscribers,NewSubscriber->SID,NewSubscriber->SIDLength);
				UpnpDestructSubscriberInfo(NewSubscriber);
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
		SIDNumber = ++dataObject->SID;
		SID = (char*)MALLOC(10 + 6);
		sprintf(SID,"uuid:%d",SIDNumber);
		p = ILibParseString(URL,0,URLLength,"://",3);
		if(p->NumResults==1)
		{
			ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n",55,1,1);
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
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,Timeout);
		if(strcmp(ServiceName,"MediaRenderer_RenderingControl")==0)
		{
			UpnpGetInitialEventBody_MediaRenderer_RenderingControl(dataObject,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"MediaRenderer_ConnectionManager")==0)
		{
			UpnpGetInitialEventBody_MediaRenderer_ConnectionManager(dataObject,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"MediaRenderer_AVTransport")==0)
		{
			UpnpGetInitialEventBody_MediaRenderer_AVTransport(dataObject,&packetbody,&packetbodyLength);
		}
		if (packetbody != NULL)	    {
			ILibWebServer_Send_Raw(session,packet,packetlength,0,1);
			
			UpnpSendEvent_Body(dataObject,packetbody,packetbodyLength,NewSubscriber);
			FREE(packetbody);
		} 
	}
	else
	{
		/* Too many subscribers */
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Too Many Subscribers\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}
}
void UpnpSubscribeEvents(char* path,int pathlength,char* Timeout,int TimeoutLength,char* URL,int URLLength,struct ILibWebServer_Session* session)
{
	long TimeoutVal;
	char* buffer = (char*)MALLOC(1+sizeof(char)*pathlength);
	
	ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
	memcpy(buffer,path,pathlength);
	buffer[pathlength] = '\0';
	FREE(buffer);
	if(TimeoutVal>7200) {TimeoutVal=7200;}
	
	if(pathlength==38 && memcmp(path+1,"MediaRenderer_ConnectionManager/event",37)==0)
	{
		UpnpTryToSubscribe("MediaRenderer_ConnectionManager",TimeoutVal,URL,URLLength,session);
	}
	else if(pathlength==37 && memcmp(path+1,"MediaRenderer_RenderingControl/event",36)==0)
	{
		UpnpTryToSubscribe("MediaRenderer_RenderingControl",TimeoutVal,URL,URLLength,session);
	}
	else if(pathlength==32 && memcmp(path+1,"MediaRenderer_AVTransport/event",31)==0)
	{
		UpnpTryToSubscribe("MediaRenderer_AVTransport",TimeoutVal,URL,URLLength,session);
	}
	else
	{
		ILibWebServer_Send_Raw(session,"HTTP/1.1 412 Invalid Service Name\r\nContent-Length: 0\r\n\r\n",56,1,1);
	}
}
void UpnpRenewEvents(char* path,int pathlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct ILibWebServer_Session *ReaderObject)
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
	if(pathlength==37 && memcmp(path+1,"MediaRenderer_RenderingControl/event",36)==0)
	{
		info = ((struct UpnpDataObject*)ReaderObject->User)->HeadSubscriberPtr_MediaRenderer_RenderingControl;
	}
	else if(pathlength==38 && memcmp(path+1,"MediaRenderer_ConnectionManager/event",37)==0)
	{
		info = ((struct UpnpDataObject*)ReaderObject->User)->HeadSubscriberPtr_MediaRenderer_ConnectionManager;
	}
	else if(pathlength==32 && memcmp(path+1,"MediaRenderer_AVTransport/event",31)==0)
	{
		info = ((struct UpnpDataObject*)ReaderObject->User)->HeadSubscriberPtr_MediaRenderer_AVTransport;
	}
	while(info!=NULL && strcmp(info->SID,SID)!=0)
	{
		info = info->Next;
	}
	if(info!=NULL)
	{
		ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
		info->RenewByTime = TimeoutVal + (GetTickCount() / 1000);
		packet = (char*)MALLOC(134 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,TimeoutVal);
		ILibWebServer_Send_Raw(ReaderObject,packet,packetlength,0,1);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n",TimeoutVal,info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		ILibWebServer_Send_Raw(ReaderObject,"HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n\r\n",55,1,1);
	}
	FREE(SID);
}
void UpnpProcessSUBSCRIBE(struct packetheader *header, struct ILibWebServer_Session *session)
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
		UpnpSubscribeEvents(header->DirectiveObj,header->DirectiveObjLength,Timeout,TimeoutLength,URL,URLLength,session);
	}
	else
	{
		/* Renew */
		UpnpRenewEvents(header->DirectiveObj,header->DirectiveObjLength,SID,SIDLength,Timeout,TimeoutLength,session);
	}
}
void UpnpProcessHTTPPacket(struct ILibWebServer_Session *session, struct packetheader* header, char *bodyBuffer, int offset, int bodyBufferLength)

{
	struct UpnpDataObject *dataObject = (struct UpnpDataObject*)session->User;
	char *responseHeader = 	"\r\nCONTENT-TYPE:  text/xml\r\nServer: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298";
	char *errorTemplate = "HTTP/1.1 %d %s\r\nServer: %s\r\nContent-Length: 0\r\n\r\n";
	char *errorPacket;
	int errorPacketLength;
	char *buffer;
	/* Virtual Directory Support */
	if(header->DirectiveObjLength>=4 && memcmp(header->DirectiveObj,"/web",4)==0)
	{
		UpnpPresentationRequest((void*)session,header);
	}
	else if(header->DirectiveLength==3 && memcmp(header->Directive,"GET",3)==0)
	{
		if(header->DirectiveObjLength==1 && memcmp(header->DirectiveObj,"/",1)==0)
		{
			ILibWebServer_StreamHeader_Raw(session,200,"OK",responseHeader,1);
			ILibWebServer_StreamBody(session,dataObject->DeviceDescription,dataObject->DeviceDescriptionLength,1,1);
			return;
		}
		else if(header->DirectiveObjLength==41 && memcmp((header->DirectiveObj)+1,"MediaRenderer_ConnectionManager/scpd.xml",40)==0)
		{
			buffer = ILibDecompressString((char*)UpnpMediaRenderer_ConnectionManagerDescription,UpnpMediaRenderer_ConnectionManagerDescriptionLength,UpnpMediaRenderer_ConnectionManagerDescriptionLengthUX);
			ILibWebServer_Send_Raw(session,buffer,UpnpMediaRenderer_ConnectionManagerDescriptionLengthUX,0,1);
		}
		else if(header->DirectiveObjLength==40 && memcmp((header->DirectiveObj)+1,"MediaRenderer_RenderingControl/scpd.xml",39)==0)
		{
			buffer = ILibDecompressString((char*)UpnpMediaRenderer_RenderingControlDescription,UpnpMediaRenderer_RenderingControlDescriptionLength,UpnpMediaRenderer_RenderingControlDescriptionLengthUX);
			ILibWebServer_Send_Raw(session,buffer,UpnpMediaRenderer_RenderingControlDescriptionLengthUX,0,1);
		}
		else if(header->DirectiveObjLength==35 && memcmp((header->DirectiveObj)+1,"MediaRenderer_AVTransport/scpd.xml",34)==0)
		{
			buffer = ILibDecompressString((char*)UpnpMediaRenderer_AVTransportDescription,UpnpMediaRenderer_AVTransportDescriptionLength,UpnpMediaRenderer_AVTransportDescriptionLengthUX);
			ILibWebServer_Send_Raw(session,buffer,UpnpMediaRenderer_AVTransportDescriptionLengthUX,0,1);
		}
		else
		{
			errorPacket = (char*)MALLOC(128);
			errorPacketLength = sprintf(errorPacket,errorTemplate,404,"File Not Found","WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298");
			ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,0,1);
			return;
		}
	}
	else if(header->DirectiveLength==4 && memcmp(header->Directive,"POST",4)==0)
	{
		UpnpProcessPOST(session,header,bodyBuffer,offset,bodyBufferLength);
	}
	else if(header->DirectiveLength==9 && memcmp(header->Directive,"SUBSCRIBE",9)==0)
	{
		UpnpProcessSUBSCRIBE(header,session);
	}
	else if(header->DirectiveLength==11 && memcmp(header->Directive,"UNSUBSCRIBE",11)==0)
	{
		UpnpProcessUNSUBSCRIBE(header,session);
	}
	else
	{
		errorPacket = (char*)MALLOC(128);
		errorPacketLength = sprintf(errorPacket,errorTemplate,400,"Bad Request","WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298");
		ILibWebServer_Send_Raw(session,errorPacket,errorPacketLength,1,1);
		return;
	}
}
void UpnpMasterPreSelect(void* object,fd_set *socketset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	int i;
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)object;
	int notifytime;
	
	int ra = 1;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	if(UPnPObject->InitialNotify==0)
	{
		UPnPObject->InitialNotify = -1;
		UpnpSendByeBye(UPnPObject);
		UpnpSendNotify(UPnPObject);
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
		UpnpSendNotify(UPnPObject);
	}
	FD_SET(UPnPObject->NOTIFY_RECEIVE_sock,socketset);
	notifytime = UpnpPeriodicNotify(UPnPObject);
	if(*blocktime>notifytime){*blocktime=notifytime;}
}

void UpnpMasterPostSelect(void* object,int slct, fd_set *socketset, fd_set *writeset, fd_set *errorset)
{
	unsigned long flags=0;
	int cnt = 0;
	struct packetheader *packet;
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)object;
	
	if(slct>0)
	{
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
				UpnpProcessMSEARCH(UPnPObject, packet);
			}
			ILibDestructPacket(packet);
		}
		
	}
}
int UpnpPeriodicNotify(struct UpnpDataObject *upnp)
{
	upnp->CurrentTime = GetTickCount() / 1000;
	if(upnp->CurrentTime >= upnp->NotifyTime)
	{
		upnp->NotifyTime = upnp->CurrentTime + (upnp->NotifyCycleTime / 3);
		UpnpSendNotify(upnp);
	}
	return(upnp->NotifyTime-upnp->CurrentTime);
}
void UpnpSendNotify(const struct UpnpDataObject *upnp)
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
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"","uuid:",upnp->UDN,upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UpnpBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
			}
		}
	}
	FREE(packet);
}

#define UpnpBuildSsdpByeByePacket(outpacket,outlenght,USN,USNex,NT,NTex)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.0\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n",USN,USNex,NT,NTex);\
}

void UpnpSendByeBye(const struct UpnpDataObject *upnp)
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
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"","uuid:",upnp->UDN);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UpnpBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
			}
		}
	}
	FREE(packet);
}

void UpnpResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg)
{
	char* body;
	int bodylength;
	char* head;
	int headlength;
	body = (char*)MALLOC(395 + (int)strlen(ErrorMsg));
	bodylength = sprintf(body,"<s:Envelope\r\n xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></UPnPError></detail></s:Fault></s:Body></s:Envelope>",ErrorCode,ErrorMsg);
	head = (char*)MALLOC(59);
	headlength = sprintf(head,"HTTP/1.1 500 Internal\r\nContent-Length: %d\r\n\r\n",bodylength);
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)UPnPToken,head,headlength,0,0);
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)UPnPToken,body,bodylength,0,1);
}

int UpnpGetLocalInterfaceToHost(const void* UPnPToken)
{
	return(ILibWebServer_GetLocalInterface((struct ILibWebServer_Session*)UPnPToken));
}

void UpnpResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params)
{
	char* packet;
	int packetlength;
	struct ILibWebServer_Session *session = (struct ILibWebServer_Session*)UPnPToken;
	int RVAL=0;
	
	packet = (char*)MALLOC(239+strlen(ServiceURI)+strlen(Params)+(strlen(MethodName)*2));
	packetlength = sprintf(packet,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
	RVAL=ILibWebServer_StreamHeader_Raw(session,200,"OK","\r\nEXT:\r\nCONTENT-TYPE: text/xml\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1298",1);
	if(RVAL!=ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR && RVAL != ILibWebServer_SEND_RESULTED_IN_DISCONNECT)
	{
		RVAL=ILibWebServer_StreamBody(session,packet,packetlength,0,1);
	}
}

void UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* unescaped_ProtocolInfo, const char* unescaped_PeerConnectionManager, const int PeerConnectionID, const char* unescaped_Direction, const char* unescaped_Status)
{
	char* body;
	char *ProtocolInfo = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ProtocolInfo));
	char *PeerConnectionManager = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PeerConnectionManager));
	char *Direction = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Direction));
	char *Status = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Status));
	
	ILibXmlEscape(ProtocolInfo,unescaped_ProtocolInfo);
	ILibXmlEscape(PeerConnectionManager,unescaped_PeerConnectionManager);
	ILibXmlEscape(Direction,unescaped_Direction);
	ILibXmlEscape(Status,unescaped_Status);
	body = (char*)MALLOC(233+strlen(ProtocolInfo)+strlen(PeerConnectionManager)+strlen(Direction)+strlen(Status));
	sprintf(body,"<RcsID>%d</RcsID><AVTransportID>%d</AVTransportID><ProtocolInfo>%s</ProtocolInfo><PeerConnectionManager>%s</PeerConnectionManager><PeerConnectionID>%d</PeerConnectionID><Direction>%s</Direction><Status>%s</Status>",RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionInfo",body);
	FREE(body);
	FREE(ProtocolInfo);
	FREE(PeerConnectionManager);
	FREE(Direction);
	FREE(Status);
}

void UpnpResponse_MediaRenderer_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* unescaped_Source, const char* unescaped_Sink)
{
	char* body;
	char *Source = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Source));
	char *Sink = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Sink));
	
	ILibXmlEscape(Source,unescaped_Source);
	ILibXmlEscape(Sink,unescaped_Sink);
	body = (char*)MALLOC(31+strlen(Source)+strlen(Sink));
	sprintf(body,"<Source>%s</Source><Sink>%s</Sink>",Source,Sink);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetProtocolInfo",body);
	FREE(body);
	FREE(Source);
	FREE(Sink);
}

void UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* unescaped_ConnectionIDs)
{
	char* body;
	char *ConnectionIDs = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ConnectionIDs));
	
	ILibXmlEscape(ConnectionIDs,unescaped_ConnectionIDs);
	body = (char*)MALLOC(32+strlen(ConnectionIDs));
	sprintf(body,"<ConnectionIDs>%s</ConnectionIDs>",ConnectionIDs);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionIDs",body);
	FREE(body);
	FREE(ConnectionIDs);
}

void UpnpResponse_MediaRenderer_RenderingControl_SetVolume(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetVolume","");
}

void UpnpResponse_MediaRenderer_RenderingControl_GetMute(const void* UPnPToken, const int CurrentMute)
{
	char* body;
	
	body = (char*)MALLOC(29);
	sprintf(body,"<CurrentMute>%d</CurrentMute>",(CurrentMute!=0?1:0));
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetMute",body);
	FREE(body);
}

void UpnpResponse_MediaRenderer_RenderingControl_SetMute(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetMute","");
}

void UpnpResponse_MediaRenderer_RenderingControl_GetVolume(const void* UPnPToken, const unsigned short CurrentVolume)
{
	char* body;
	
	body = (char*)MALLOC(38);
	sprintf(body,"<CurrentVolume>%u</CurrentVolume>",CurrentVolume);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetVolume",body);
	FREE(body);
}

void UpnpResponse_MediaRenderer_AVTransport_GetCurrentTransportActions(const void* UPnPToken, const char* unescaped_Actions)
{
	char* body;
	char *Actions = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Actions));
	
	ILibXmlEscape(Actions,unescaped_Actions);
	body = (char*)MALLOC(20+strlen(Actions));
	sprintf(body,"<Actions>%s</Actions>",Actions);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetCurrentTransportActions",body);
	FREE(body);
	FREE(Actions);
}

void UpnpResponse_MediaRenderer_AVTransport_Play(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Play","");
}

void UpnpResponse_MediaRenderer_AVTransport_GetDeviceCapabilities(const void* UPnPToken, const char* unescaped_PlayMedia, const char* unescaped_RecMedia, const char* unescaped_RecQualityModes)
{
	char* body;
	char *PlayMedia = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMedia));
	char *RecMedia = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecMedia));
	char *RecQualityModes = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecQualityModes));
	
	ILibXmlEscape(PlayMedia,unescaped_PlayMedia);
	ILibXmlEscape(RecMedia,unescaped_RecMedia);
	ILibXmlEscape(RecQualityModes,unescaped_RecQualityModes);
	body = (char*)MALLOC(80+strlen(PlayMedia)+strlen(RecMedia)+strlen(RecQualityModes));
	sprintf(body,"<PlayMedia>%s</PlayMedia><RecMedia>%s</RecMedia><RecQualityModes>%s</RecQualityModes>",PlayMedia,RecMedia,RecQualityModes);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetDeviceCapabilities",body);
	FREE(body);
	FREE(PlayMedia);
	FREE(RecMedia);
	FREE(RecQualityModes);
}

void UpnpResponse_MediaRenderer_AVTransport_GetMediaInfo(const void* UPnPToken, const unsigned int NrTracks, const char* unescaped_MediaDuration, const char* unescaped_CurrentURI, const char* unescaped_CurrentURIMetaData, const char* unescaped_NextURI, const char* unescaped_NextURIMetaData, const char* unescaped_PlayMedium, const char* unescaped_RecordMedium, const char* unescaped_WriteStatus)
{
	char* body;
	char *MediaDuration = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_MediaDuration));
	char *CurrentURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentURI));
	char *CurrentURIMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentURIMetaData));
	char *NextURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_NextURI));
	char *NextURIMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_NextURIMetaData));
	char *PlayMedium = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMedium));
	char *RecordMedium = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecordMedium));
	char *WriteStatus = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_WriteStatus));
	
	ILibXmlEscape(MediaDuration,unescaped_MediaDuration);
	ILibXmlEscape(CurrentURI,unescaped_CurrentURI);
	ILibXmlEscape(CurrentURIMetaData,unescaped_CurrentURIMetaData);
	ILibXmlEscape(NextURI,unescaped_NextURI);
	ILibXmlEscape(NextURIMetaData,unescaped_NextURIMetaData);
	ILibXmlEscape(PlayMedium,unescaped_PlayMedium);
	ILibXmlEscape(RecordMedium,unescaped_RecordMedium);
	ILibXmlEscape(WriteStatus,unescaped_WriteStatus);
	body = (char*)MALLOC(265+strlen(MediaDuration)+strlen(CurrentURI)+strlen(CurrentURIMetaData)+strlen(NextURI)+strlen(NextURIMetaData)+strlen(PlayMedium)+strlen(RecordMedium)+strlen(WriteStatus));
	sprintf(body,"<NrTracks>%u</NrTracks><MediaDuration>%s</MediaDuration><CurrentURI>%s</CurrentURI><CurrentURIMetaData>%s</CurrentURIMetaData><NextURI>%s</NextURI><NextURIMetaData>%s</NextURIMetaData><PlayMedium>%s</PlayMedium><RecordMedium>%s</RecordMedium><WriteStatus>%s</WriteStatus>",NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetMediaInfo",body);
	FREE(body);
	FREE(MediaDuration);
	FREE(CurrentURI);
	FREE(CurrentURIMetaData);
	FREE(NextURI);
	FREE(NextURIMetaData);
	FREE(PlayMedium);
	FREE(RecordMedium);
	FREE(WriteStatus);
}

void UpnpResponse_MediaRenderer_AVTransport_Previous(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Previous","");
}

void UpnpResponse_MediaRenderer_AVTransport_Next(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Next","");
}

void UpnpResponse_MediaRenderer_AVTransport_GetTransportSettings(const void* UPnPToken, const char* unescaped_PlayMode, const char* unescaped_RecQualityMode)
{
	char* body;
	char *PlayMode = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMode));
	char *RecQualityMode = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecQualityMode));
	
	ILibXmlEscape(PlayMode,unescaped_PlayMode);
	ILibXmlEscape(RecQualityMode,unescaped_RecQualityMode);
	body = (char*)MALLOC(55+strlen(PlayMode)+strlen(RecQualityMode));
	sprintf(body,"<PlayMode>%s</PlayMode><RecQualityMode>%s</RecQualityMode>",PlayMode,RecQualityMode);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetTransportSettings",body);
	FREE(body);
	FREE(PlayMode);
	FREE(RecQualityMode);
}

void UpnpResponse_MediaRenderer_AVTransport_SetAVTransportURI(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","SetAVTransportURI","");
}

void UpnpResponse_MediaRenderer_AVTransport_Pause(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Pause","");
}

void UpnpResponse_MediaRenderer_AVTransport_GetPositionInfo(const void* UPnPToken, const unsigned int Track, const char* unescaped_TrackDuration, const char* unescaped_TrackMetaData, const char* unescaped_TrackURI, const char* unescaped_RelTime, const char* unescaped_AbsTime, const int RelCount, const int AbsCount)
{
	char* body;
	char *TrackDuration = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackDuration));
	char *TrackMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackMetaData));
	char *TrackURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackURI));
	char *RelTime = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RelTime));
	char *AbsTime = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_AbsTime));
	
	ILibXmlEscape(TrackDuration,unescaped_TrackDuration);
	ILibXmlEscape(TrackMetaData,unescaped_TrackMetaData);
	ILibXmlEscape(TrackURI,unescaped_TrackURI);
	ILibXmlEscape(RelTime,unescaped_RelTime);
	ILibXmlEscape(AbsTime,unescaped_AbsTime);
	body = (char*)MALLOC(212+strlen(TrackDuration)+strlen(TrackMetaData)+strlen(TrackURI)+strlen(RelTime)+strlen(AbsTime));
	sprintf(body,"<Track>%u</Track><TrackDuration>%s</TrackDuration><TrackMetaData>%s</TrackMetaData><TrackURI>%s</TrackURI><RelTime>%s</RelTime><AbsTime>%s</AbsTime><RelCount>%d</RelCount><AbsCount>%d</AbsCount>",Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetPositionInfo",body);
	FREE(body);
	FREE(TrackDuration);
	FREE(TrackMetaData);
	FREE(TrackURI);
	FREE(RelTime);
	FREE(AbsTime);
}

void UpnpResponse_MediaRenderer_AVTransport_Seek(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Seek","");
}

void UpnpResponse_MediaRenderer_AVTransport_GetTransportInfo(const void* UPnPToken, const char* unescaped_CurrentTransportState, const char* unescaped_CurrentTransportStatus, const char* unescaped_CurrentSpeed)
{
	char* body;
	char *CurrentTransportState = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentTransportState));
	char *CurrentTransportStatus = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentTransportStatus));
	char *CurrentSpeed = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentSpeed));
	
	ILibXmlEscape(CurrentTransportState,unescaped_CurrentTransportState);
	ILibXmlEscape(CurrentTransportStatus,unescaped_CurrentTransportStatus);
	ILibXmlEscape(CurrentSpeed,unescaped_CurrentSpeed);
	body = (char*)MALLOC(126+strlen(CurrentTransportState)+strlen(CurrentTransportStatus)+strlen(CurrentSpeed));
	sprintf(body,"<CurrentTransportState>%s</CurrentTransportState><CurrentTransportStatus>%s</CurrentTransportStatus><CurrentSpeed>%s</CurrentSpeed>",CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetTransportInfo",body);
	FREE(body);
	FREE(CurrentTransportState);
	FREE(CurrentTransportStatus);
	FREE(CurrentSpeed);
}

void UpnpResponse_MediaRenderer_AVTransport_SetPlayMode(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","SetPlayMode","");
}

void UpnpResponse_MediaRenderer_AVTransport_Stop(const void* UPnPToken)
{
	UpnpResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Stop","");
}

void UpnpSendEventSink(
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
	if(done!=0 && ((struct SubscriberInfo*)subscriber)->Disposing==0)
	{
		sem_wait(&(((struct UpnpDataObject*)upnp)->EventLock));
		--((struct SubscriberInfo*)subscriber)->RefCount;
		if(((struct SubscriberInfo*)subscriber)->RefCount==0)
		{
			LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			UpnpDestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
		}
		else if(header==NULL)
		{
			LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			// Could not send Event, so unsubscribe the subscriber
			((struct SubscriberInfo*)subscriber)->Disposing = 1;
			UpnpExpireSubscriberInfo(upnp,subscriber);
		}
		sem_post(&(((struct UpnpDataObject*)upnp)->EventLock));
	}
}
void UpnpSendEvent_Body(void *upnptoken,char *body,int bodylength,struct SubscriberInfo *info)
{
	struct UpnpDataObject* UPnPObject = (struct UpnpDataObject*)upnptoken;
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
	packetLength = sprintf(packet,"NOTIFY %s HTTP/1.1\r\nHOST: %d.%d.%d.%d:%d\r\nContent-Type: text/xml\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),info->Port,info->SID,info->SEQ,bodylength+137,body);
	++info->SEQ;
	
	++info->RefCount;
	ILibWebClient_PipelineRequestEx(UPnPObject->EventClient,&dest,packet,packetLength,0,NULL,0,0,&UpnpSendEventSink,info,upnptoken);
}
void UpnpSendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
	struct SubscriberInfo *info = NULL;
	struct UpnpDataObject* UPnPObject = (struct UpnpDataObject*)upnptoken;
	struct sockaddr_in dest;
	LVL3DEBUG(struct timeval tv;)
	
	if(UPnPObject==NULL)
	{
		FREE(body);
		return;
	}
	sem_wait(&(UPnPObject->EventLock));
	if(strncmp(eventname,"MediaRenderer_RenderingControl",30)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_MediaRenderer_RenderingControl;
	}
	if(strncmp(eventname,"MediaRenderer_ConnectionManager",31)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_MediaRenderer_ConnectionManager;
	}
	if(strncmp(eventname,"MediaRenderer_AVTransport",25)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_MediaRenderer_AVTransport;
	}
	memset(&dest,0,sizeof(dest));
	while(info!=NULL)
	{
		if(!UpnpSubscriptionExpired(info))
		{
			UpnpSendEvent_Body(upnptoken,body,bodylength,info);
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

void UpnpSetState_MediaRenderer_ConnectionManager_SourceProtocolInfo(void *upnptoken, char* val)
{
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->MediaRenderer_ConnectionManager_SourceProtocolInfo != NULL) FREE(UPnPObject->MediaRenderer_ConnectionManager_SourceProtocolInfo);
	UPnPObject->MediaRenderer_ConnectionManager_SourceProtocolInfo = valstr;
	body = (char*)MALLOC(46 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SourceProtocolInfo",valstr,"SourceProtocolInfo");
	UpnpSendEvent(upnptoken,body,bodylength,"MediaRenderer_ConnectionManager");
	FREE(body);
}

void UpnpSetState_MediaRenderer_ConnectionManager_SinkProtocolInfo(void *upnptoken, char* val)
{
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->MediaRenderer_ConnectionManager_SinkProtocolInfo != NULL) FREE(UPnPObject->MediaRenderer_ConnectionManager_SinkProtocolInfo);
	UPnPObject->MediaRenderer_ConnectionManager_SinkProtocolInfo = valstr;
	body = (char*)MALLOC(42 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SinkProtocolInfo",valstr,"SinkProtocolInfo");
	UpnpSendEvent(upnptoken,body,bodylength,"MediaRenderer_ConnectionManager");
	FREE(body);
}

void UpnpSetState_MediaRenderer_ConnectionManager_CurrentConnectionIDs(void *upnptoken, char* val)
{
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->MediaRenderer_ConnectionManager_CurrentConnectionIDs != NULL) FREE(UPnPObject->MediaRenderer_ConnectionManager_CurrentConnectionIDs);
	UPnPObject->MediaRenderer_ConnectionManager_CurrentConnectionIDs = valstr;
	body = (char*)MALLOC(50 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","CurrentConnectionIDs",valstr,"CurrentConnectionIDs");
	UpnpSendEvent(upnptoken,body,bodylength,"MediaRenderer_ConnectionManager");
	FREE(body);
}

void UpnpSetState_MediaRenderer_RenderingControl_LastChange(void *upnptoken, char* val)
{
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->MediaRenderer_RenderingControl_LastChange != NULL) FREE(UPnPObject->MediaRenderer_RenderingControl_LastChange);
	UPnPObject->MediaRenderer_RenderingControl_LastChange = valstr;
	body = (char*)MALLOC(30 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","LastChange",valstr,"LastChange");
	UpnpSendEvent(upnptoken,body,bodylength,"MediaRenderer_RenderingControl");
	FREE(body);
}

void UpnpSetState_MediaRenderer_AVTransport_LastChange(void *upnptoken, char* val)
{
	struct UpnpDataObject *UPnPObject = (struct UpnpDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->MediaRenderer_AVTransport_LastChange != NULL) FREE(UPnPObject->MediaRenderer_AVTransport_LastChange);
	UPnPObject->MediaRenderer_AVTransport_LastChange = valstr;
	body = (char*)MALLOC(30 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","LastChange",valstr,"LastChange");
	UpnpSendEvent(upnptoken,body,bodylength,"MediaRenderer_AVTransport");
	FREE(body);
}


void UpnpDestroyMicroStack(void *object)
{
	struct UpnpDataObject *upnp = (struct UpnpDataObject*)object;
	struct SubscriberInfo  *sinfo,*sinfo2;
	UpnpSendByeBye(upnp);
	
	sem_destroy(&(upnp->EventLock));
	FREE(upnp->MediaRenderer_RenderingControl_LastChange);
	FREE(upnp->MediaRenderer_ConnectionManager_SourceProtocolInfo);
	FREE(upnp->MediaRenderer_ConnectionManager_SinkProtocolInfo);
	FREE(upnp->MediaRenderer_ConnectionManager_CurrentConnectionIDs);
	FREE(upnp->MediaRenderer_AVTransport_LastChange);
	
	FREE(upnp->AddressList);
	FREE(upnp->NOTIFY_SEND_socks);
	FREE(upnp->UUID);
	FREE(upnp->Serial);
	FREE(upnp->DeviceDescription);
	
	sinfo = upnp->HeadSubscriberPtr_MediaRenderer_RenderingControl;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UpnpDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_MediaRenderer_ConnectionManager;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UpnpDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_MediaRenderer_AVTransport;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UpnpDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	
	WSACleanup();
}
int UpnpGetLocalPortNumber(void *token)
{
	return(((struct UpnpDataObject*)((struct ILibWebServer_Session*)token)->Parent)->WebSocketPortNumber);
}
void UpnpSessionReceiveSink(
struct ILibWebServer_Session *sender,
int InterruptFlag,
struct packetheader *header,
char *bodyBuffer,
int *beginPointer,
int endPointer,
int done)
{
	if(header!=NULL && done !=0 && InterruptFlag==0)
	{
		UpnpProcessHTTPPacket(sender,header,bodyBuffer,beginPointer==NULL?0:*beginPointer,endPointer);
		if(beginPointer!=NULL) {*beginPointer = endPointer;}
	}
}
void UpnpSessionSink(struct ILibWebServer_Session *SessionToken, void *user)
{
	SessionToken->OnReceive = &UpnpSessionReceiveSink;
	SessionToken->User = user;
}
void *UpnpCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)
{
	struct UpnpDataObject* RetVal = (struct UpnpDataObject*)MALLOC(sizeof(struct UpnpDataObject));
	char* DDT;
	WORD wVersionRequested;
	WSADATA wsaData;
	
	srand((int)GetTickCount());
	wVersionRequested = MAKEWORD( 1, 1 );
	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}
	UpnpInit(RetVal,NotifyCycleSeconds,PortNum);
	RetVal->ForceExit = 0;
	RetVal->PreSelect = &UpnpMasterPreSelect;
	RetVal->PostSelect = &UpnpMasterPostSelect;
	RetVal->Destroy = &UpnpDestroyMicroStack;
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
	
	RetVal->DeviceDescription = (char*)MALLOC(UpnpDeviceDescriptionTemplateLengthUX + (int)strlen(FriendlyName) + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * 1));
	DDT = ILibDecompressString((char*)UpnpDeviceDescriptionTemplate,UpnpDeviceDescriptionTemplateLength,UpnpDeviceDescriptionTemplateLengthUX);
	RetVal->DeviceDescriptionLength = sprintf(RetVal->DeviceDescription,DDT,FriendlyName,RetVal->Serial,RetVal->UDN);
	FREE(DDT);
	RetVal->MediaRenderer_RenderingControl_LastChange = NULL;
	RetVal->MediaRenderer_ConnectionManager_SourceProtocolInfo = NULL;
	RetVal->MediaRenderer_ConnectionManager_SinkProtocolInfo = NULL;
	RetVal->MediaRenderer_ConnectionManager_CurrentConnectionIDs = NULL;
	RetVal->MediaRenderer_AVTransport_LastChange = NULL;
	
	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
	
	RetVal->HTTPServer = ILibWebServer_Create(Chain,UPNP_HTTP_MAXSOCKETS,PortNum,&UpnpSessionSink,RetVal);
	RetVal->WebSocketPortNumber=(int)ILibWebServer_GetPortNumber(RetVal->HTTPServer);
	
	ILibAddToChain(Chain,RetVal);
	RetVal->EventClient = ILibCreateWebClient(5,Chain);
	RetVal->Chain = Chain;
	RetVal->UpdateFlag = 0;
	
	sem_init(&(RetVal->EventLock),0,1);
	return(RetVal);
}

