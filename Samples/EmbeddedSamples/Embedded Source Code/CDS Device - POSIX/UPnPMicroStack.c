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
#include <semaphore.h>
#include "ILibParsers.h"
#include "UPnPMicroStack.h"
#include "ILibHTTPClient.h"

#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define UPnPMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)

const int UPnPDeviceDescriptionTemplateLengthUX = 1218;
const int UPnPDeviceDescriptionTemplateLength = 629;
const char UPnPDeviceDescriptionTemplate[629]={
	0x3D,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x50,0x4F,0x53,0x49,0x58,0x2C,0x20,0x55
	,0x50,0x6E,0x85,0x0E,0x12,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72,0x6F,0x53,0x74
	,0x61,0x63,0x6B,0x04,0x14,0x63,0x2E,0x31,0x31,0x38,0x39,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C
	,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64
	,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x72,0x6F,0x6F,0x74,0x20,0x78
	,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70
	,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x64,0x65,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C
	,0x73,0x70,0x65,0x63,0x56,0xC6,0x14,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46
	,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x00
	,0x06,0x12,0x00,0x08,0x02,0x05,0x54,0x79,0x70,0x65,0x3E,0x1B,0x1C,0x06,0x3A,0x4D,0x65,0x64,0x69,0x61
	,0x87,0x3E,0x03,0x31,0x3C,0x2F,0x8B,0x0D,0x12,0x3C,0x66,0x72,0x69,0x65,0x6E,0x64,0x6C,0x79,0x4E,0x61
	,0x6D,0x65,0x3E,0x25,0x73,0x3C,0x2F,0x4D,0x04,0x0E,0x3C,0x6D,0x61,0x6E,0x75,0x66,0x61,0x63,0x74,0x75
	,0x72,0x65,0x72,0x3E,0xC6,0x48,0x06,0x4C,0x61,0x62,0x73,0x3C,0x2F,0x4D,0x06,0x00,0xCD,0x09,0x10,0x55
	,0x52,0x4C,0x3E,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x69,0x04,0x56,0x17,0x2E,0x63
	,0x6F,0x6D,0x2F,0x6C,0x61,0x62,0x73,0x2F,0x63,0x6F,0x6E,0x6E,0x65,0x63,0x74,0x69,0x76,0x69,0x74,0x79
	,0x2F,0x04,0x46,0x03,0x2F,0x3C,0x2F,0x90,0x0F,0x0E,0x3C,0x6D,0x6F,0x64,0x65,0x6C,0x44,0x65,0x73,0x63
	,0x72,0x69,0x70,0x74,0xC4,0x47,0x03,0x41,0x56,0x20,0x05,0x31,0x01,0x20,0xC6,0x6F,0x02,0x3C,0x2F,0x91
	,0x08,0x00,0x06,0x0D,0x00,0x84,0x31,0x02,0x20,0x2F,0x48,0x03,0x05,0x75,0x6D,0x62,0x65,0x72,0xC4,0x03
	,0x05,0x73,0x65,0x72,0x69,0x61,0x07,0x04,0x00,0x45,0x39,0x00,0x4D,0x04,0x0A,0x3C,0x55,0x44,0x4E,0x3E
	,0x75,0x75,0x69,0x64,0x3A,0x04,0x40,0x03,0x55,0x44,0x4E,0x45,0x0C,0x00,0x44,0x6A,0x04,0x4C,0x69,0x73
	,0x74,0x49,0x03,0x00,0x89,0x05,0x00,0xDA,0x5B,0x00,0xC7,0x0D,0x02,0x3A,0x43,0x47,0x39,0x07,0x6F,0x6E
	,0x4D,0x61,0x6E,0x61,0x67,0x86,0x5D,0x00,0x8C,0x0F,0x00,0x48,0x18,0x02,0x49,0x64,0x05,0x6E,0x00,0x50
	,0x10,0x0E,0x49,0x64,0x3A,0x43,0x4D,0x47,0x52,0x5F,0x30,0x2D,0x39,0x39,0x3C,0x2F,0x0A,0x0B,0x05,0x3C
	,0x53,0x43,0x50,0x44,0x04,0x58,0x00,0x51,0x18,0x0B,0x2F,0x73,0x63,0x70,0x64,0x2E,0x78,0x6D,0x6C,0x3C
	,0x2F,0x08,0x09,0x08,0x3C,0x63,0x6F,0x6E,0x74,0x72,0x6F,0x6C,0x16,0x0C,0x00,0x47,0x07,0x02,0x3C,0x2F
	,0x8B,0x09,0x09,0x3C,0x65,0x76,0x65,0x6E,0x74,0x53,0x75,0x62,0xD6,0x18,0x00,0x85,0x07,0x02,0x3C,0x2F
	,0x4C,0x09,0x02,0x3C,0x2F,0x50,0x47,0x00,0xAE,0x49,0x0D,0x74,0x65,0x6E,0x74,0x44,0x69,0x72,0x65,0x63
	,0x74,0x6F,0x72,0x79,0x73,0x49,0x02,0x44,0x53,0x1D,0x49,0x00,0xCD,0x17,0x00,0xE2,0x48,0x00,0xCE,0x0B
	,0x00,0xA4,0x48,0x00,0x4E,0x18,0x00,0x5E,0x48,0x01,0x2F,0x4D,0x95,0x01,0x2F,0xC8,0xF0,0x03,0x2F,0x72
	,0x6F,0x00,0x00,0x03,0x6F,0x74,0x3E,0x00,0x00};
/* ConnectionManager */
const int UPnPConnectionManagerDescriptionLengthUX = 4769;
const int UPnPConnectionManagerDescriptionLength = 1073;
const char UPnPConnectionManagerDescription[1073] = {
	0x3D,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x50,0x4F,0x53,0x49,0x58,0x2C,0x20,0x55
	,0x50,0x6E,0x85,0x0E,0x12,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72,0x6F,0x53,0x74
	,0x61,0x63,0x6B,0x04,0x14,0x64,0x2E,0x31,0x31,0x38,0x39,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C
	,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64
	,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x73,0x63,0x70,0x64,0x20,0x78
	,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70
	,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E
	,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F
	,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B
	,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x16,0x3E,0x3C,0x6E,0x61,0x6D,0x65
	,0x3E,0x47,0x65,0x74,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x43,0x6F,0x6E,0x6E,0x65,0xC5,0x09,0x06,0x49
	,0x6E,0x66,0x6F,0x3C,0x2F,0xC5,0x07,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65,0x6E,0x74,0xC7,0x0E,0x00
	,0x87,0x03,0x00,0x47,0x0F,0x00,0xCB,0x0C,0x01,0x44,0x48,0x0C,0x03,0x64,0x69,0x72,0x86,0x11,0x05,0x3E
	,0x69,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65
	,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0x84,0x5D,0x01,0x5F,0x4E
	,0x13,0x00,0x95,0x0B,0x02,0x3C,0x2F,0x4A,0x20,0x00,0xCF,0x22,0x03,0x52,0x63,0x73,0x14,0x21,0x03,0x6F
	,0x75,0x74,0x6D,0x21,0x03,0x52,0x63,0x73,0xB4,0x1F,0x0B,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F
	,0x72,0x74,0xBF,0x21,0x00,0xC5,0x42,0x00,0xCF,0x13,0x00,0x30,0x43,0x08,0x50,0x72,0x6F,0x74,0x6F,0x63
	,0x6F,0x6C,0x0C,0x72,0x00,0xFA,0x44,0x00,0x8E,0x13,0x00,0x31,0x23,0x03,0x65,0x65,0x72,0x8A,0x96,0x07
	,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72,0x3F,0x6A,0x03,0x50,0x45,0x5F,0xD3,0x14,0x00,0xBE,0x26,0x00,0x7F
	,0x8F,0x00,0xBF,0xB0,0x00,0x84,0xE2,0x01,0x44,0x48,0xCE,0x00,0xBF,0xB1,0x03,0x50,0x45,0x5F,0xCB,0x12
	,0x00,0x30,0xD2,0x00,0xC4,0xE7,0x02,0x75,0x73,0x7F,0x68,0x00,0x8D,0xF3,0x00,0x88,0x14,0x00,0xA1,0xF4
	,0x00,0x49,0xF7,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x07,0xED,0x00,0x08,0x02,0x00,0x07,0xB7,0x07,0x65
	,0x70,0x61,0x72,0x65,0x46,0x6F,0x8B,0x95,0x00,0xC8,0xFD,0x00,0x0E,0x10,0x00,0xCF,0xE8,0x06,0x52,0x65
	,0x6D,0x6F,0x74,0x65,0xDE,0xC6,0x02,0x69,0x6E,0xBF,0xC6,0x00,0xBF,0xC6,0x02,0x65,0x72,0x3F,0x25,0x00
	,0x7F,0xC6,0x00,0x68,0xC6,0x02,0x69,0x6E,0x3F,0xC6,0x00,0x3F,0xC6,0x00,0x37,0x6C,0x00,0xFB,0xC5,0x00
	,0x1E,0x44,0x03,0x6F,0x75,0x74,0x7F,0x44,0x00,0xEC,0xE8,0x0B,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70
	,0x6F,0x72,0x74,0x7F,0x23,0x00,0x85,0xEA,0x00,0xCF,0x13,0x00,0xF0,0xB1,0x03,0x52,0x63,0x73,0xFF,0x44
	,0x00,0x05,0xD3,0x03,0x52,0x63,0x73,0x65,0x87,0x01,0x2F,0x4E,0xF9,0x02,0x2F,0x61,0x07,0xEB,0x00,0x08
	,0x02,0x00,0x8F,0x6B,0x08,0x43,0x6F,0x6D,0x70,0x6C,0x65,0x74,0x65,0x08,0xFB,0x00,0x8E,0x0F,0x00,0x6D
	,0x79,0x00,0x7F,0xBD,0x00,0x3F,0x36,0x00,0x85,0xE6,0x0F,0x47,0x65,0x74,0x50,0x72,0x6F,0x74,0x6F,0x63
	,0x6F,0x6C,0x49,0x6E,0x66,0x6F,0x65,0x35,0x06,0x53,0x6F,0x75,0x72,0x63,0x65,0x37,0xAD,0x00,0x46,0x0F
	,0x00,0x0E,0x1D,0x00,0x30,0xF0,0x04,0x53,0x69,0x6E,0x6B,0xF8,0x1F,0x03,0x69,0x6E,0x6B,0x6F,0x1F,0x00
	,0x28,0x52,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x0C,0xF6,0x01,0x73,0x71,0x89,0x01,0x73,0xB7,0xDF
	,0x00,0x56,0x1F,0x00,0x39,0xBF,0x00,0x47,0xC1,0x00,0x86,0xC4,0x07,0x73,0x65,0x72,0x76,0x69,0x63,0x65
	,0xC5,0xFB,0x01,0x54,0x46,0xEF,0x01,0x73,0xCC,0xFE,0x10,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E
	,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0xC7,0xF1,0x00,0xCB,0xE2,0x00,0xD4,0x98,0x11,0x64,0x61,0x74,0x61
	,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x03,0x3C,0x2F,0x73,0x4E
	,0xEB,0x00,0xAF,0x1B,0x00,0x0A,0xEB,0x00,0x44,0xFD,0x02,0x75,0x73,0xA3,0x1C,0x0C,0x61,0x6C,0x6C,0x6F
	,0x77,0x65,0x64,0x56,0x61,0x6C,0x75,0x65,0x47,0xF2,0x00,0x8B,0x04,0x05,0x3E,0x4F,0x4B,0x3C,0x2F,0x4D
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
/* ContentDirectory */
const int UPnPContentDirectoryDescriptionLengthUX = 9887;
const int UPnPContentDirectoryDescriptionLength = 1862;
const char UPnPContentDirectoryDescription[1862] = {
	0x3D,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x30,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x50,0x4F,0x53,0x49,0x58,0x2C,0x20,0x55
	,0x50,0x6E,0x85,0x0E,0x12,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72,0x6F,0x53,0x74
	,0x61,0x63,0x6B,0x04,0x14,0x64,0x2E,0x31,0x31,0x38,0x39,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F,0x78,0x6D,0x6C
	,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E,0x63,0x6F,0x64
	,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x73,0x63,0x70,0x64,0x20,0x78
	,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73,0x2D,0x75,0x70
	,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30,0x22,0x3E
	,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31,0x3C,0x2F
	,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F,0x8D,0x0B
	,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x0F,0x3E,0x3C,0x6E,0x61,0x6D,0x65
	,0x3E,0x53,0x65,0x61,0x72,0x63,0x68,0x3C,0x2F,0x45,0x03,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65,0x6E
	,0x74,0x47,0x0A,0x00,0x87,0x03,0x00,0xC7,0x0A,0x0B,0x43,0x6F,0x6E,0x74,0x61,0x69,0x6E,0x65,0x72,0x49
	,0x44,0x08,0x0C,0x04,0x64,0x69,0x72,0x65,0x86,0x13,0x04,0x69,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72
	,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E
	,0x41,0x5F,0x41,0x52,0x47,0x5F,0xC4,0x58,0x07,0x5F,0x4F,0x62,0x6A,0x65,0x63,0x74,0x44,0x12,0x00,0x95
	,0x0A,0x02,0x3C,0x2F,0x0A,0x1F,0x00,0x8F,0x21,0x00,0x46,0x2C,0x08,0x43,0x72,0x69,0x74,0x65,0x72,0x69
	,0x61,0x7F,0x22,0x02,0x45,0x5F,0xD0,0x13,0x00,0xF0,0x23,0x06,0x46,0x69,0x6C,0x74,0x65,0x72,0x3F,0x44
	,0x02,0x45,0x5F,0xC8,0x11,0x00,0xB1,0x43,0x0C,0x74,0x61,0x72,0x74,0x69,0x6E,0x67,0x49,0x6E,0x64,0x65
	,0x78,0xBF,0x65,0x02,0x45,0x5F,0x87,0x11,0x00,0xF0,0x64,0x0E,0x52,0x65,0x71,0x75,0x65,0x73,0x74,0x65
	,0x64,0x43,0x6F,0x75,0x6E,0x74,0x3F,0x87,0x02,0x45,0x5F,0x87,0x11,0x00,0x71,0x86,0x03,0x6F,0x72,0x74
	,0xFF,0x85,0x00,0xCB,0x85,0x03,0x6F,0x72,0x74,0x7A,0x85,0x05,0x52,0x65,0x73,0x75,0x6C,0x53,0x42,0x03
	,0x6F,0x75,0x74,0xAD,0xC9,0x00,0x08,0x12,0x00,0x30,0xC9,0x0E,0x4E,0x75,0x6D,0x62,0x65,0x72,0x52,0x65
	,0x74,0x75,0x72,0x6E,0x65,0x64,0x3F,0x22,0x00,0xBA,0x64,0x0C,0x54,0x6F,0x74,0x61,0x6C,0x4D,0x61,0x74
	,0x63,0x68,0x65,0x73,0x7F,0x21,0x00,0xFA,0x85,0x08,0x55,0x70,0x64,0x61,0x74,0x65,0x49,0x44,0xBF,0x63
	,0x03,0x50,0x45,0x5F,0x8A,0x12,0x00,0xA1,0xE9,0x00,0x49,0xEC,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x47
	,0xE0,0x00,0x08,0x02,0x00,0x07,0xEF,0x08,0x6F,0x70,0x54,0x72,0x61,0x6E,0x73,0x66,0x04,0x6B,0x06,0x73
	,0x6F,0x75,0x72,0x63,0x65,0xC8,0xF0,0x00,0x0E,0x10,0x00,0x10,0x56,0x00,0x47,0x0D,0x02,0x49,0x44,0xBF
	,0xFC,0x02,0x45,0x5F,0xCC,0x12,0x00,0xBF,0x35,0x00,0xC7,0xE1,0x0D,0x44,0x65,0x73,0x74,0x72,0x6F,0x79
	,0x4F,0x62,0x6A,0x65,0x63,0x74,0xE5,0x33,0x00,0xC6,0x0A,0x00,0x7F,0x33,0x00,0x44,0xED,0x00,0x4A,0x12
	,0x00,0x7F,0x68,0x00,0xCD,0x8E,0x00,0xBF,0x32,0x00,0xBF,0x32,0x00,0xF0,0xFE,0x0F,0x43,0x75,0x72,0x72
	,0x65,0x6E,0x74,0x54,0x61,0x67,0x56,0x61,0x6C,0x75,0x65,0x7F,0x88,0x02,0x45,0x5F,0x48,0x12,0x03,0x4C
	,0x69,0x73,0x73,0xDF,0x03,0x4E,0x65,0x77,0xBF,0x22,0x00,0xB9,0x22,0x00,0xE5,0xE0,0x06,0x45,0x78,0x70
	,0x6F,0x72,0x74,0x6D,0xDF,0x01,0x53,0x05,0xEA,0x03,0x55,0x52,0x49,0x3F,0xDF,0x05,0x45,0x5F,0x55,0x52
	,0x49,0x32,0x78,0x00,0xC4,0xD7,0x03,0x69,0x6E,0x61,0x04,0xFB,0x00,0x3F,0x21,0x00,0x3A,0x21,0x08,0x54
	,0x72,0x61,0x6E,0x73,0x66,0x65,0x72,0xD4,0xEC,0x03,0x6F,0x75,0x74,0x2E,0x98,0x00,0x0B,0x13,0x00,0xBF
	,0xED,0x00,0x87,0xED,0x03,0x47,0x65,0x74,0x48,0x28,0x08,0x50,0x72,0x6F,0x67,0x72,0x65,0x73,0x73,0x65
	,0xEF,0x00,0x9C,0x35,0x02,0x69,0x6E,0x7F,0x35,0x00,0x72,0x57,0x00,0x84,0xFC,0x02,0x75,0x73,0x7F,0x58
	,0x03,0x50,0x45,0x5F,0x10,0x14,0x00,0x78,0x7B,0x06,0x4C,0x65,0x6E,0x67,0x74,0x68,0x7F,0x7C,0x03,0x50
	,0x45,0x5F,0x10,0x14,0x00,0x78,0x9F,0x05,0x54,0x6F,0x74,0x61,0x6C,0x3F,0xA0,0x03,0x50,0x45,0x5F,0xCF
	,0x13,0x00,0xFF,0xA0,0x00,0xCA,0xA0,0x11,0x53,0x65,0x61,0x72,0x63,0x68,0x43,0x61,0x70,0x61,0x62,0x69
	,0x6C,0x69,0x74,0x69,0x65,0x66,0xA1,0x00,0xC9,0x0D,0x00,0xB8,0x7E,0x00,0x14,0x1E,0x00,0x7F,0x35,0x00
	,0x4B,0x35,0x0D,0x79,0x73,0x74,0x65,0x6D,0x55,0x70,0x64,0x61,0x74,0x65,0x49,0x44,0xA5,0xD5,0x02,0x49
	,0x64,0x78,0x32,0x00,0x0F,0x1B,0x00,0xBF,0x66,0x00,0x47,0xF9,0x0C,0x43,0x72,0x65,0x61,0x74,0x65,0x4F
	,0x62,0x6A,0x65,0x63,0x74,0x65,0x64,0x09,0x43,0x6F,0x6E,0x74,0x61,0x69,0x6E,0x65,0x72,0x0A,0x3C,0x00
	,0x0A,0xE3,0x02,0x69,0x6E,0xED,0xE2,0x00,0xC6,0x1D,0x02,0x49,0x44,0x72,0xE1,0x03,0x45,0x6C,0x65,0x44
	,0xE7,0x01,0x73,0xFF,0x20,0x08,0x45,0x5F,0x52,0x65,0x73,0x75,0x6C,0x74,0xB2,0xDD,0x00,0xCA,0x2E,0x00
	,0x7F,0xDC,0x00,0x7B,0x41,0x00,0x08,0x2F,0x00,0xFF,0xFC,0x00,0x2A,0x41,0x00,0x29,0xFB,0x03,0x6F,0x72
	,0x74,0xB2,0xFA,0x03,0x6F,0x72,0x74,0x3C,0xFA,0x03,0x6F,0x72,0x74,0xBF,0xF9,0x00,0x95,0xF9,0x06,0x42
	,0x72,0x6F,0x77,0x73,0x65,0xE5,0xF6,0x00,0x1A,0x85,0x00,0x3F,0xC6,0x00,0x2A,0xC6,0x00,0x86,0x2B,0x04
	,0x46,0x6C,0x61,0x67,0x7F,0xE7,0x02,0x45,0x5F,0xCC,0x12,0x00,0xF0,0xE7,0x06,0x46,0x69,0x6C,0x74,0x65
	,0x72,0x7F,0xE7,0x02,0x45,0x5F,0xC8,0x11,0x00,0x70,0xE7,0x0D,0x53,0x74,0x61,0x72,0x74,0x69,0x6E,0x67
	,0x49,0x6E,0x64,0x65,0x78,0xBF,0x63,0x02,0x45,0x5F,0x87,0x11,0x00,0xB2,0xE7,0x0C,0x71,0x75,0x65,0x73
	,0x74,0x65,0x64,0x43,0x6F,0x75,0x6E,0x74,0x3F,0x85,0x02,0x45,0x5F,0x87,0x11,0x00,0xF1,0x42,0x00,0xC4
	,0xE2,0x07,0x72,0x69,0x74,0x65,0x72,0x69,0x61,0x3F,0xA6,0x02,0x45,0x5F,0x4E,0x13,0x00,0x72,0x44,0x04
	,0x73,0x75,0x6C,0x74,0xF7,0xF7,0x00,0x8B,0xC7,0x00,0x08,0x12,0x00,0x30,0xC7,0x0E,0x4E,0x75,0x6D,0x62
	,0x65,0x72,0x52,0x65,0x74,0x75,0x72,0x6E,0x65,0x64,0x3F,0x22,0x00,0xBA,0x64,0x0C,0x54,0x6F,0x74,0x61
	,0x6C,0x4D,0x61,0x74,0x63,0x68,0x65,0x73,0x7F,0x21,0x00,0xFA,0x85,0x08,0x55,0x70,0x64,0x61,0x74,0x65
	,0x49,0x44,0xBF,0x63,0x03,0x50,0x45,0x5F,0x8A,0x12,0x00,0xA1,0xE9,0x00,0x49,0xEC,0x03,0x4C,0x69,0x73
	,0xC5,0x03,0x00,0x47,0xE0,0x00,0x08,0x02,0x00,0x05,0xEF,0x0E,0x49,0x6D,0x70,0x6F,0x72,0x74,0x52,0x65
	,0x73,0x6F,0x75,0x72,0x63,0x65,0x48,0xEF,0x00,0x8E,0x0E,0x00,0x11,0xB9,0x00,0xC4,0x0A,0x03,0x55,0x52
	,0x49,0xFF,0xFA,0x05,0x45,0x5F,0x55,0x52,0x49,0x72,0xFA,0x07,0x44,0x65,0x73,0x74,0x69,0x6E,0x61,0x44
	,0xF5,0x00,0x3F,0x21,0x00,0x3A,0x21,0x08,0x54,0x72,0x61,0x6E,0x73,0x66,0x65,0x72,0xBF,0x74,0x00,0x85
	,0xF9,0x00,0x0C,0x13,0x00,0x3F,0x75,0x00,0x87,0xFE,0x0D,0x43,0x72,0x65,0x61,0x74,0x65,0x52,0x65,0x66
	,0x65,0x72,0x65,0x6E,0x67,0x75,0x07,0x43,0x6F,0x6E,0x74,0x61,0x69,0x6E,0xD6,0x34,0x00,0xEF,0x75,0x08
	,0x4F,0x62,0x6A,0x65,0x63,0x74,0x49,0x44,0x72,0xEB,0x00,0x8A,0x0E,0x00,0xFF,0x20,0x00,0xFA,0x20,0x03
	,0x4E,0x65,0x77,0x3F,0xEA,0x00,0x30,0x41,0x00,0x25,0xEA,0x06,0x44,0x65,0x6C,0x65,0x74,0x65,0x2D,0xEA
	,0x03,0x52,0x65,0x73,0xBF,0xEA,0x00,0xB0,0xEA,0x00,0x98,0xA7,0x00,0xC7,0xA9,0x00,0x06,0xAD,0x07,0x73
	,0x65,0x72,0x76,0x69,0x63,0x65,0x05,0xFC,0x01,0x54,0x46,0xFB,0x01,0x73,0x0C,0xFF,0x10,0x20,0x73,0x65
	,0x6E,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0xC7,0xFD,0x00,0xCB,0xEC,0x0C,0x53
	,0x6F,0x72,0x74,0x43,0x72,0x69,0x74,0x65,0x72,0x69,0x61,0x09,0xE0,0x10,0x61,0x74,0x61,0x54,0x79,0x70
	,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x03,0x3C,0x2F,0x73,0xCE,0xF5,0x00,0xAF
	,0x1B,0x00,0x08,0xFB,0x06,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3F,0x1C,0x00,0x8E,0x37,0x03,0x79,0x65,0x73
	,0xC8,0x37,0x00,0x88,0x19,0x03,0x49,0x44,0x73,0xFF,0x34,0x00,0x63,0x50,0x08,0x55,0x70,0x64,0x61,0x74
	,0x65,0x49,0x44,0x51,0x4F,0x03,0x75,0x69,0x34,0xBF,0x4E,0x00,0x0D,0x6A,0x05,0x65,0x61,0x72,0x63,0x68
	,0xBF,0x6A,0x00,0x2B,0x86,0x06,0x46,0x69,0x6C,0x74,0x65,0x72,0xBF,0x68,0x00,0x99,0x68,0x09,0x43,0x6F
	,0x6E,0x74,0x61,0x69,0x6E,0x65,0x72,0x88,0x4F,0x00,0x7F,0x6A,0x00,0xA4,0xBA,0x06,0x52,0x65,0x73,0x75
	,0x6C,0x74,0x3F,0xB9,0x00,0xA3,0xD4,0x05,0x49,0x6E,0x64,0x65,0x78,0xBF,0x83,0x00,0x28,0xD2,0x00,0xFF
	,0x9D,0x00,0x63,0xEC,0x0A,0x61,0x67,0x56,0x61,0x6C,0x75,0x65,0x4C,0x69,0x73,0xFF,0x4E,0x00,0x25,0xD3
	,0x02,0x52,0x49,0xD2,0xD1,0x02,0x72,0x69,0xBF,0xEB,0x00,0x8C,0xEB,0x0A,0x42,0x72,0x6F,0x77,0x73,0x65
	,0x46,0x6C,0x61,0x67,0xE3,0xD0,0x07,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x49,0x40,0x00,0x8E,0x04,0x01
	,0x3E,0x06,0x13,0x04,0x4D,0x65,0x74,0x61,0x04,0xFD,0x02,0x3C,0x2F,0x4D,0x07,0x00,0xD4,0x0A,0x0E,0x44
	,0x69,0x72,0x65,0x63,0x74,0x43,0x68,0x69,0x6C,0x64,0x72,0x65,0x6E,0x50,0x0C,0x01,0x2F,0xD2,0x1B,0x00
	,0x3F,0xF1,0x08,0x4F,0x62,0x6A,0x65,0x63,0x74,0x49,0x44,0x3F,0xD7,0x00,0x18,0xD7,0x0F,0x53,0x6F,0x72
	,0x74,0x43,0x61,0x70,0x61,0x62,0x69,0x6C,0x69,0x74,0x69,0x65,0xFF,0xF0,0x00,0xE4,0xF0,0x05,0x43,0x6F
	,0x75,0x6E,0x74,0xFF,0xD6,0x00,0xD6,0x32,0x05,0x65,0x61,0x72,0x63,0x68,0x7F,0x33,0x00,0x1A,0xF1,0x03
	,0x79,0x65,0x73,0x49,0x4D,0x0B,0x79,0x73,0x74,0x65,0x6D,0x55,0x70,0x64,0x61,0x74,0x65,0xBF,0xEF,0x00
	,0xA3,0xEF,0x0D,0x72,0x61,0x6E,0x73,0x66,0x65,0x72,0x53,0x74,0x61,0x74,0x75,0x73,0xBF,0xBC,0x0C,0x75
	,0x65,0x3E,0x43,0x4F,0x4D,0x50,0x4C,0x45,0x54,0x45,0x44,0x5D,0xBB,0x05,0x45,0x52,0x52,0x4F,0x52,0xDD
	,0xC3,0x0B,0x49,0x4E,0x5F,0x50,0x52,0x4F,0x47,0x52,0x45,0x53,0x53,0xDD,0xCD,0x07,0x53,0x54,0x4F,0x50
	,0x50,0x45,0x44,0xBF,0xCA,0x00,0x6B,0x4A,0x05,0x54,0x6F,0x74,0x61,0x6C,0xF3,0xCB,0x08,0x2F,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x44,0x5A,0x02,0x65,0x54,0x08,0x05,0x01,0x63,0x00,0x00,0x03,0x70,0x64,0x3E
	,0x00,0x00};

struct UPnPDataObject;

struct HTTPReaderObject
{
	char Header[2048];
	char* Body;
	struct packetheader *ParsedHeader;
	int BodySize;
	int HeaderIndex;
	int BodyIndex;
	int ClientSocket;
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
	
	struct timeval RenewByTime;
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
	char* ContentDirectory_TransferIDs;
	char* ContentDirectory_ContainerUpdateIDs;
	char* ContentDirectory_SystemUpdateID;
	char* ConnectionManager_SourceProtocolInfo;
	char* ConnectionManager_SinkProtocolInfo;
	char* ConnectionManager_CurrentConnectionIDs;
	struct sockaddr_in addr;
	int addrlen;
	int MSEARCH_sock;
	struct ip_mreq mreq;
	char message[4096];
	int *AddressList;
	int AddressListLength;
	
	int _NumEmbeddedDevices;
	int WebSocket;
	int WebSocketPortNumber;
	struct HTTPReaderObject ReaderObjects[5];
	int *NOTIFY_SEND_socks;
	int NOTIFY_RECEIVE_sock;
	
	int SID;
	
	struct timeval CurrentTime;
	int NotifyCycleTime;
	struct timeval NotifyTime;
	
	sem_t EventLock;
	struct SubscriberInfo *HeadSubscriberPtr_ContentDirectory;
	int NumberOfSubscribers_ContentDirectory;
	struct SubscriberInfo *HeadSubscriberPtr_ConnectionManager;
	int NumberOfSubscribers_ConnectionManager;
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
		length = (unsigned short)((*(CurrentCompressed)) & 63);
		offset = ((unsigned short)(*(CurrentCompressed+1))<<2) + (((unsigned short)(*(CurrentCompressed))) >> 6);
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
	*outlenght = sprintf(outpacket,"HTTP/1.1 200 OK\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nEXT:\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,ST,NTex);\
}

#define UPnPBuildSsdpNotifyPacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n",(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,NT,NTex);\
}

void UPnPAsyncResponse_START(const void* UPnPToken, const char* actionName, const char* serviceUrnWithVersion)
{
	char* RESPONSE_HEADER = "HTTP/1.0 200 OK\r\nEXT:\r\nCONTENT-TYPE: text/xml\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n<s:Body>\r\n<u:%sResponse xmlns:u=\"%s\">";
	
	int headLength = (int)strlen(RESPONSE_HEADER) + (int)strlen(actionName) + (int)strlen(serviceUrnWithVersion) + 1;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	char* head = (char*) malloc (headLength);
	
	sprintf(head, RESPONSE_HEADER, actionName, serviceUrnWithVersion);
	send(ReaderObject->ClientSocket,head,(int)strlen(head),0);
	FREE(head);
}

void UPnPAsyncResponse_DONE(const void* UPnPToken, const char* actionName)
{
	char* RESPONSE_FOOTER = "</u:%sResponse>\r\n   </s:Body>\r\n</s:Envelope>";
	
	int footLength = (int)strlen(RESPONSE_FOOTER) + (int)strlen(actionName);
	char* footer = (char*) malloc(footLength);
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	
	sprintf(footer, RESPONSE_FOOTER, actionName);
	send(ReaderObject->ClientSocket, footer, (int)strlen(footer),0);
	FREE(footer);
	
	close(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
}

void UPnPAsyncResponse_OUT(const void* UPnPToken, const char* outArgName, const char* bytes, const int byteLength, const int startArg, const int endArg)
{
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	
	if (startArg != 0)
	{
		send(ReaderObject->ClientSocket, "<", 1, 0);
		send(ReaderObject->ClientSocket, outArgName, (int)strlen(outArgName),0);
		send(ReaderObject->ClientSocket, ">", 1, 0);
	}
	
	send(ReaderObject->ClientSocket, bytes, byteLength, 0);
	
	if (endArg != 0)
	{
		send(ReaderObject->ClientSocket, "</", 2, 0);
		send(ReaderObject->ClientSocket, outArgName, (int)strlen(outArgName),0);
		send(ReaderObject->ClientSocket, ">\r\n      ", 9, 0);
	}
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
	gettimeofday(&(state->CurrentTime),NULL);
	(state->NotifyTime).tv_sec = (state->CurrentTime).tv_sec  + (state->NotifyCycleTime/2);
	
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
		flags = fcntl(state->WebSocket,F_GETFL,0);
		fcntl(state->WebSocket,F_SETFL,O_NONBLOCK|flags);
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
		flags = fcntl(state->WebSocket,F_GETFL,0);
		fcntl(state->WebSocket,F_SETFL,O_NONBLOCK|flags);
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
	state->NOTIFY_SEND_socks = (int*)MALLOC(sizeof(int)*(state->AddressListLength));
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
	int *response_socket;
	int cnt;
	int i;
	struct sockaddr_in dest_addr = mss->dest_addr;
	char *ST = mss->ST;
	int STLength = mss->STLength;
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)mss->upnp;
	
	response_socket = (int*)MALLOC(upnp->AddressListLength*sizeof(int));
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
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",upnp->NotifyCycleTime);
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
	if(STLength>=41 && memcmp(ST,"urn:schemas-upnp-org:device:MediaServer:1",41)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=48 && memcmp(ST,"urn:schemas-upnp-org:service:ConnectionManager:1",48)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=47 && memcmp(ST,"urn:schemas-upnp-org:service:ContentDirectory:1",47)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	for(i=0;i<upnp->AddressListLength;++i)
	{
		close(response_socket[i]);
	}
	FREE(response_socket);
	FREE(mss->ST);
	FREE(mss);
	FREE(b);
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
				if(strncasecmp(node->Field,"ST",2)==0)
				{
					ST = (char*)MALLOC(1+node->FieldDataLength);
					memcpy(ST,node->FieldData,node->FieldDataLength);
					ST[node->FieldDataLength] = 0;
					STLength = node->FieldDataLength;
				}
				else if(strncasecmp(node->Field,"MAN",3)==0 && memcmp(node->FieldData,"\"ssdp:discover\"",15)==0)
				{
					MANOK = 1;
				}
				else if(strncasecmp(node->Field,"MX",2)==0 && ILibGetULong(node->FieldData,node->FieldDataLength,&MXVal)==0)
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
void UPnPDispatch_ContentDirectory_Search(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
	char *p_ContainerID = NULL;
	int p_ContainerIDLength = 0;
	char* _ContainerID = "";
	int _ContainerIDLength;
	char *p_SearchCriteria = NULL;
	int p_SearchCriteriaLength = 0;
	char* _SearchCriteria = "";
	int _SearchCriteriaLength;
	char *p_Filter = NULL;
	int p_FilterLength = 0;
	char* _Filter = "";
	int _FilterLength;
	char *p_StartingIndex = NULL;
	int p_StartingIndexLength = 0;
	unsigned int _StartingIndex = 0;
	char *p_RequestedCount = NULL;
	int p_RequestedCountLength = 0;
	unsigned int _RequestedCount = 0;
	char *p_SortCriteria = NULL;
	int p_SortCriteriaLength = 0;
	char* _SortCriteria = "";
	int _SortCriteriaLength;
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
			if(VarNameLength==11 && memcmp(VarName,"ContainerID",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = temp3->LastResult->datalength;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				else
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = 0;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==14 && memcmp(VarName,"SearchCriteria",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_SearchCriteria = temp3->LastResult->data;
					p_SearchCriteriaLength = temp3->LastResult->datalength;
					p_SearchCriteria[p_SearchCriteriaLength] = 0;
				}
				else
				{
					p_SearchCriteria = temp3->LastResult->data;
					p_SearchCriteriaLength = 0;
					p_SearchCriteria[p_SearchCriteriaLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==6 && memcmp(VarName,"Filter",6) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Filter = temp3->LastResult->data;
					p_FilterLength = temp3->LastResult->datalength;
					p_Filter[p_FilterLength] = 0;
				}
				else
				{
					p_Filter = temp3->LastResult->data;
					p_FilterLength = 0;
					p_Filter[p_FilterLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==13 && memcmp(VarName,"StartingIndex",13) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_StartingIndex = temp3->LastResult->data;
					p_StartingIndexLength = temp3->LastResult->datalength;
				}
				OK |= 8;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==14 && memcmp(VarName,"RequestedCount",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_RequestedCount = temp3->LastResult->data;
					p_RequestedCountLength = temp3->LastResult->datalength;
				}
				OK |= 16;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==12 && memcmp(VarName,"SortCriteria",12) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_SortCriteria = temp3->LastResult->data;
					p_SortCriteriaLength = temp3->LastResult->datalength;
					p_SortCriteria[p_SortCriteriaLength] = 0;
				}
				else
				{
					p_SortCriteria = temp3->LastResult->data;
					p_SortCriteriaLength = 0;
					p_SortCriteria[p_SortCriteriaLength] = 0;
				}
				OK |= 32;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 63)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_ContainerIDLength = ILibInPlaceXmlUnEscape(p_ContainerID);
	_ContainerID = p_ContainerID;
	_SearchCriteriaLength = ILibInPlaceXmlUnEscape(p_SearchCriteria);
	_SearchCriteria = p_SearchCriteria;
	_FilterLength = ILibInPlaceXmlUnEscape(p_Filter);
	_Filter = p_Filter;
	OK = ILibGetULong(p_StartingIndex,p_StartingIndexLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_StartingIndex = (unsigned int)TempULong;
	OK = ILibGetULong(p_RequestedCount,p_RequestedCountLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_RequestedCount = (unsigned int)TempULong;
	_SortCriteriaLength = ILibInPlaceXmlUnEscape(p_SortCriteria);
	_SortCriteria = p_SortCriteria;
	UPnPContentDirectory_Search((void*)ReaderObject,_ContainerID,_SearchCriteria,_Filter,_StartingIndex,_RequestedCount,_SortCriteria);
}

void UPnPDispatch_ContentDirectory_StopTransferResource(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
	char *p_TransferID = NULL;
	int p_TransferIDLength = 0;
	unsigned int _TransferID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"TransferID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_TransferID = temp3->LastResult->data;
					p_TransferIDLength = temp3->LastResult->datalength;
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
	OK = ILibGetULong(p_TransferID,p_TransferIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_TransferID = (unsigned int)TempULong;
	UPnPContentDirectory_StopTransferResource((void*)ReaderObject,_TransferID);
}

void UPnPDispatch_ContentDirectory_DestroyObject(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	int OK = 0;
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
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
			if(VarNameLength==8 && memcmp(VarName,"ObjectID",8) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = temp3->LastResult->datalength;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				else
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = 0;
					p_ObjectID[p_ObjectIDLength] = 0;
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
	_ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	UPnPContentDirectory_DestroyObject((void*)ReaderObject,_ObjectID);
}

void UPnPDispatch_ContentDirectory_UpdateObject(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	int OK = 0;
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
	char *p_CurrentTagValue = NULL;
	int p_CurrentTagValueLength = 0;
	char* _CurrentTagValue = "";
	int _CurrentTagValueLength;
	char *p_NewTagValue = NULL;
	int p_NewTagValueLength = 0;
	char* _NewTagValue = "";
	int _NewTagValueLength;
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
			if(VarNameLength==8 && memcmp(VarName,"ObjectID",8) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = temp3->LastResult->datalength;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				else
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = 0;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==15 && memcmp(VarName,"CurrentTagValue",15) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_CurrentTagValue = temp3->LastResult->data;
					p_CurrentTagValueLength = temp3->LastResult->datalength;
					p_CurrentTagValue[p_CurrentTagValueLength] = 0;
				}
				else
				{
					p_CurrentTagValue = temp3->LastResult->data;
					p_CurrentTagValueLength = 0;
					p_CurrentTagValue[p_CurrentTagValueLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==11 && memcmp(VarName,"NewTagValue",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_NewTagValue = temp3->LastResult->data;
					p_NewTagValueLength = temp3->LastResult->datalength;
					p_NewTagValue[p_NewTagValueLength] = 0;
				}
				else
				{
					p_NewTagValue = temp3->LastResult->data;
					p_NewTagValueLength = 0;
					p_NewTagValue[p_NewTagValueLength] = 0;
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	_CurrentTagValueLength = ILibInPlaceXmlUnEscape(p_CurrentTagValue);
	_CurrentTagValue = p_CurrentTagValue;
	_NewTagValueLength = ILibInPlaceXmlUnEscape(p_NewTagValue);
	_NewTagValue = p_NewTagValue;
	UPnPContentDirectory_UpdateObject((void*)ReaderObject,_ObjectID,_CurrentTagValue,_NewTagValue);
}

void UPnPDispatch_ContentDirectory_ExportResource(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	struct parser_result *TempParser;
	int OK = 0;
	char *p_SourceURI = NULL;
	int p_SourceURILength = 0;
	char* _SourceURI = "";
	int _SourceURILength;
	char *p_DestinationURI = NULL;
	int p_DestinationURILength = 0;
	char* _DestinationURI = "";
	int _DestinationURILength;
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
			if(VarNameLength==9 && memcmp(VarName,"SourceURI",9) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_SourceURI = temp3->LastResult->data;
					p_SourceURILength = temp3->LastResult->datalength;
					p_SourceURI[p_SourceURILength] = 0;
				}
				else
				{
					p_SourceURI = temp3->LastResult->data;
					p_SourceURILength = 0;
					p_SourceURI[p_SourceURILength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==14 && memcmp(VarName,"DestinationURI",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DestinationURI = temp3->LastResult->data;
					p_DestinationURILength = temp3->LastResult->datalength;
					p_DestinationURI[p_DestinationURILength] = 0;
				}
				else
				{
					p_DestinationURI = temp3->LastResult->data;
					p_DestinationURILength = 0;
					p_DestinationURI[p_DestinationURILength] = 0;
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	TempParser = ILibParseString(p_SourceURI, 0, p_SourceURILength, "://",3);
	if(TempParser->NumResults!=2)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		_SourceURI = p_SourceURI;
		_SourceURILength = p_SourceURILength;
	}
	TempParser = ILibParseString(p_DestinationURI, 0, p_DestinationURILength, "://",3);
	if(TempParser->NumResults!=2)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		_DestinationURI = p_DestinationURI;
		_DestinationURILength = p_DestinationURILength;
	}
	UPnPContentDirectory_ExportResource((void*)ReaderObject,_SourceURI,_DestinationURI);
}

void UPnPDispatch_ContentDirectory_GetTransferProgress(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
	char *p_TransferID = NULL;
	int p_TransferIDLength = 0;
	unsigned int _TransferID = 0;
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
			if(VarNameLength==10 && memcmp(VarName,"TransferID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_TransferID = temp3->LastResult->data;
					p_TransferIDLength = temp3->LastResult->datalength;
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
	OK = ILibGetULong(p_TransferID,p_TransferIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_TransferID = (unsigned int)TempULong;
	UPnPContentDirectory_GetTransferProgress((void*)ReaderObject,_TransferID);
}

#define UPnPDispatch_ContentDirectory_GetSearchCapabilities(xml, ReaderObject)\
{\
	UPnPContentDirectory_GetSearchCapabilities((void*)ReaderObject);\
}

#define UPnPDispatch_ContentDirectory_GetSystemUpdateID(xml, ReaderObject)\
{\
	UPnPContentDirectory_GetSystemUpdateID((void*)ReaderObject);\
}

void UPnPDispatch_ContentDirectory_CreateObject(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	int OK = 0;
	char *p_ContainerID = NULL;
	int p_ContainerIDLength = 0;
	char* _ContainerID = "";
	int _ContainerIDLength;
	char *p_Elements = NULL;
	int p_ElementsLength = 0;
	char* _Elements = "";
	int _ElementsLength;
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
			if(VarNameLength==11 && memcmp(VarName,"ContainerID",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = temp3->LastResult->datalength;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				else
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = 0;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==8 && memcmp(VarName,"Elements",8) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Elements = temp3->LastResult->data;
					p_ElementsLength = temp3->LastResult->datalength;
					p_Elements[p_ElementsLength] = 0;
				}
				else
				{
					p_Elements = temp3->LastResult->data;
					p_ElementsLength = 0;
					p_Elements[p_ElementsLength] = 0;
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_ContainerIDLength = ILibInPlaceXmlUnEscape(p_ContainerID);
	_ContainerID = p_ContainerID;
	_ElementsLength = ILibInPlaceXmlUnEscape(p_Elements);
	_Elements = p_Elements;
	UPnPContentDirectory_CreateObject((void*)ReaderObject,_ContainerID,_Elements);
}

#define UPnPDispatch_ContentDirectory_GetSortCapabilities(xml, ReaderObject)\
{\
	UPnPContentDirectory_GetSortCapabilities((void*)ReaderObject);\
}

void UPnPDispatch_ContentDirectory_Browse(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
	char *p_BrowseFlag = NULL;
	int p_BrowseFlagLength = 0;
	char* _BrowseFlag = "";
	int _BrowseFlagLength;
	char *p_Filter = NULL;
	int p_FilterLength = 0;
	char* _Filter = "";
	int _FilterLength;
	char *p_StartingIndex = NULL;
	int p_StartingIndexLength = 0;
	unsigned int _StartingIndex = 0;
	char *p_RequestedCount = NULL;
	int p_RequestedCountLength = 0;
	unsigned int _RequestedCount = 0;
	char *p_SortCriteria = NULL;
	int p_SortCriteriaLength = 0;
	char* _SortCriteria = "";
	int _SortCriteriaLength;
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
			if(VarNameLength==8 && memcmp(VarName,"ObjectID",8) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = temp3->LastResult->datalength;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				else
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = 0;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==10 && memcmp(VarName,"BrowseFlag",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_BrowseFlag = temp3->LastResult->data;
					p_BrowseFlagLength = temp3->LastResult->datalength;
					p_BrowseFlag[p_BrowseFlagLength] = 0;
				}
				else
				{
					p_BrowseFlag = temp3->LastResult->data;
					p_BrowseFlagLength = 0;
					p_BrowseFlag[p_BrowseFlagLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==6 && memcmp(VarName,"Filter",6) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Filter = temp3->LastResult->data;
					p_FilterLength = temp3->LastResult->datalength;
					p_Filter[p_FilterLength] = 0;
				}
				else
				{
					p_Filter = temp3->LastResult->data;
					p_FilterLength = 0;
					p_Filter[p_FilterLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==13 && memcmp(VarName,"StartingIndex",13) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_StartingIndex = temp3->LastResult->data;
					p_StartingIndexLength = temp3->LastResult->datalength;
				}
				OK |= 8;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==14 && memcmp(VarName,"RequestedCount",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_RequestedCount = temp3->LastResult->data;
					p_RequestedCountLength = temp3->LastResult->datalength;
				}
				OK |= 16;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==12 && memcmp(VarName,"SortCriteria",12) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_SortCriteria = temp3->LastResult->data;
					p_SortCriteriaLength = temp3->LastResult->datalength;
					p_SortCriteria[p_SortCriteriaLength] = 0;
				}
				else
				{
					p_SortCriteria = temp3->LastResult->data;
					p_SortCriteriaLength = 0;
					p_SortCriteria[p_SortCriteriaLength] = 0;
				}
				OK |= 32;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 63)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	_BrowseFlagLength = ILibInPlaceXmlUnEscape(p_BrowseFlag);
	_BrowseFlag = p_BrowseFlag;
	if(memcmp(_BrowseFlag, "BrowseMetadata\0",15) != 0
	&& memcmp(_BrowseFlag, "BrowseDirectChildren\0",21) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_FilterLength = ILibInPlaceXmlUnEscape(p_Filter);
	_Filter = p_Filter;
	OK = ILibGetULong(p_StartingIndex,p_StartingIndexLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_StartingIndex = (unsigned int)TempULong;
	OK = ILibGetULong(p_RequestedCount,p_RequestedCountLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_RequestedCount = (unsigned int)TempULong;
	_SortCriteriaLength = ILibInPlaceXmlUnEscape(p_SortCriteria);
	_SortCriteria = p_SortCriteria;
	UPnPContentDirectory_Browse((void*)ReaderObject,_ObjectID,_BrowseFlag,_Filter,_StartingIndex,_RequestedCount,_SortCriteria);
}

void UPnPDispatch_ContentDirectory_ImportResource(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	struct parser_result *TempParser;
	int OK = 0;
	char *p_SourceURI = NULL;
	int p_SourceURILength = 0;
	char* _SourceURI = "";
	int _SourceURILength;
	char *p_DestinationURI = NULL;
	int p_DestinationURILength = 0;
	char* _DestinationURI = "";
	int _DestinationURILength;
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
			if(VarNameLength==9 && memcmp(VarName,"SourceURI",9) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_SourceURI = temp3->LastResult->data;
					p_SourceURILength = temp3->LastResult->datalength;
					p_SourceURI[p_SourceURILength] = 0;
				}
				else
				{
					p_SourceURI = temp3->LastResult->data;
					p_SourceURILength = 0;
					p_SourceURI[p_SourceURILength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==14 && memcmp(VarName,"DestinationURI",14) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DestinationURI = temp3->LastResult->data;
					p_DestinationURILength = temp3->LastResult->datalength;
					p_DestinationURI[p_DestinationURILength] = 0;
				}
				else
				{
					p_DestinationURI = temp3->LastResult->data;
					p_DestinationURILength = 0;
					p_DestinationURI[p_DestinationURILength] = 0;
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	TempParser = ILibParseString(p_SourceURI, 0, p_SourceURILength, "://",3);
	if(TempParser->NumResults!=2)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		_SourceURI = p_SourceURI;
		_SourceURILength = p_SourceURILength;
	}
	TempParser = ILibParseString(p_DestinationURI, 0, p_DestinationURILength, "://",3);
	if(TempParser->NumResults!=2)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		_DestinationURI = p_DestinationURI;
		_DestinationURILength = p_DestinationURILength;
	}
	UPnPContentDirectory_ImportResource((void*)ReaderObject,_SourceURI,_DestinationURI);
}

void UPnPDispatch_ContentDirectory_CreateReference(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	int OK = 0;
	char *p_ContainerID = NULL;
	int p_ContainerIDLength = 0;
	char* _ContainerID = "";
	int _ContainerIDLength;
	char *p_ObjectID = NULL;
	int p_ObjectIDLength = 0;
	char* _ObjectID = "";
	int _ObjectIDLength;
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
			if(VarNameLength==11 && memcmp(VarName,"ContainerID",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = temp3->LastResult->datalength;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				else
				{
					p_ContainerID = temp3->LastResult->data;
					p_ContainerIDLength = 0;
					p_ContainerID[p_ContainerIDLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==8 && memcmp(VarName,"ObjectID",8) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = temp3->LastResult->datalength;
					p_ObjectID[p_ObjectIDLength] = 0;
				}
				else
				{
					p_ObjectID = temp3->LastResult->data;
					p_ObjectIDLength = 0;
					p_ObjectID[p_ObjectIDLength] = 0;
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_ContainerIDLength = ILibInPlaceXmlUnEscape(p_ContainerID);
	_ContainerID = p_ContainerID;
	_ObjectIDLength = ILibInPlaceXmlUnEscape(p_ObjectID);
	_ObjectID = p_ObjectID;
	UPnPContentDirectory_CreateReference((void*)ReaderObject,_ContainerID,_ObjectID);
}

void UPnPDispatch_ContentDirectory_DeleteResource(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	struct parser_result *TempParser;
	int OK = 0;
	char *p_ResourceURI = NULL;
	int p_ResourceURILength = 0;
	char* _ResourceURI = "";
	int _ResourceURILength;
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
			if(VarNameLength==11 && memcmp(VarName,"ResourceURI",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ResourceURI = temp3->LastResult->data;
					p_ResourceURILength = temp3->LastResult->datalength;
					p_ResourceURI[p_ResourceURILength] = 0;
				}
				else
				{
					p_ResourceURI = temp3->LastResult->data;
					p_ResourceURILength = 0;
					p_ResourceURI[p_ResourceURILength] = 0;
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
	TempParser = ILibParseString(p_ResourceURI, 0, p_ResourceURILength, "://",3);
	if(TempParser->NumResults!=2)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	else
	{
		_ResourceURI = p_ResourceURI;
		_ResourceURILength = p_ResourceURILength;
	}
	UPnPContentDirectory_DeleteResource((void*)ReaderObject,_ResourceURI);
}

void UPnPDispatch_ConnectionManager_GetCurrentConnectionInfo(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	UPnPConnectionManager_GetCurrentConnectionInfo((void*)ReaderObject,_ConnectionID);
}

void UPnPDispatch_ConnectionManager_PrepareForConnection(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
	char *p_RemoteProtocolInfo = NULL;
	int p_RemoteProtocolInfoLength = 0;
	char* _RemoteProtocolInfo = "";
	int _RemoteProtocolInfoLength;
	char *p_PeerConnectionManager = NULL;
	int p_PeerConnectionManagerLength = 0;
	char* _PeerConnectionManager = "";
	int _PeerConnectionManagerLength;
	char *p_PeerConnectionID = NULL;
	int p_PeerConnectionIDLength = 0;
	int _PeerConnectionID = 0;
	char *p_Direction = NULL;
	int p_DirectionLength = 0;
	char* _Direction = "";
	int _DirectionLength;
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
			if(VarNameLength==18 && memcmp(VarName,"RemoteProtocolInfo",18) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_RemoteProtocolInfo = temp3->LastResult->data;
					p_RemoteProtocolInfoLength = temp3->LastResult->datalength;
					p_RemoteProtocolInfo[p_RemoteProtocolInfoLength] = 0;
				}
				else
				{
					p_RemoteProtocolInfo = temp3->LastResult->data;
					p_RemoteProtocolInfoLength = 0;
					p_RemoteProtocolInfo[p_RemoteProtocolInfoLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==21 && memcmp(VarName,"PeerConnectionManager",21) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_PeerConnectionManager = temp3->LastResult->data;
					p_PeerConnectionManagerLength = temp3->LastResult->datalength;
					p_PeerConnectionManager[p_PeerConnectionManagerLength] = 0;
				}
				else
				{
					p_PeerConnectionManager = temp3->LastResult->data;
					p_PeerConnectionManagerLength = 0;
					p_PeerConnectionManager[p_PeerConnectionManagerLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==16 && memcmp(VarName,"PeerConnectionID",16) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_PeerConnectionID = temp3->LastResult->data;
					p_PeerConnectionIDLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==9 && memcmp(VarName,"Direction",9) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Direction = temp3->LastResult->data;
					p_DirectionLength = temp3->LastResult->datalength;
					p_Direction[p_DirectionLength] = 0;
				}
				else
				{
					p_Direction = temp3->LastResult->data;
					p_DirectionLength = 0;
					p_Direction[p_DirectionLength] = 0;
				}
				OK |= 8;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 15)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	_RemoteProtocolInfoLength = ILibInPlaceXmlUnEscape(p_RemoteProtocolInfo);
	_RemoteProtocolInfo = p_RemoteProtocolInfo;
	_PeerConnectionManagerLength = ILibInPlaceXmlUnEscape(p_PeerConnectionManager);
	_PeerConnectionManager = p_PeerConnectionManager;
	OK = ILibGetLong(p_PeerConnectionID,p_PeerConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_PeerConnectionID = (int)TempLong;
	_DirectionLength = ILibInPlaceXmlUnEscape(p_Direction);
	_Direction = p_Direction;
	if(memcmp(_Direction, "Input\0",6) != 0
	&& memcmp(_Direction, "Output\0",7) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	UPnPConnectionManager_PrepareForConnection((void*)ReaderObject,_RemoteProtocolInfo,_PeerConnectionManager,_PeerConnectionID,_Direction);
}

void UPnPDispatch_ConnectionManager_ConnectionComplete(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
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
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	UPnPConnectionManager_ConnectionComplete((void*)ReaderObject,_ConnectionID);
}

#define UPnPDispatch_ConnectionManager_GetProtocolInfo(xml, ReaderObject)\
{\
	UPnPConnectionManager_GetProtocolInfo((void*)ReaderObject);\
}

#define UPnPDispatch_ConnectionManager_GetCurrentConnectionIDs(xml, ReaderObject)\
{\
	UPnPConnectionManager_GetCurrentConnectionIDs((void*)ReaderObject);\
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
		if(f->FieldLength==4 && strncasecmp(f->Field,"HOST",4)==0)
		{
			HOST = f->FieldData;
		}
		else if(f->FieldLength==10 && strncasecmp(f->Field,"SOAPACTION",10)==0)
		{
			r = ILibParseString(f->FieldData,0,f->FieldDataLength,"#",1);
			SOAPACTION = r->LastResult->data;
			SOAPACTIONLength = r->LastResult->datalength-1;
			ILibDestructParserResults(r);
		}
		f = f->NextField;
	}
	if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ConnectionManager/control",25)==0)
	{
		if(SOAPACTIONLength==24 && memcmp(SOAPACTION,"GetCurrentConnectionInfo",24)==0)
		{
			UPnPDispatch_ConnectionManager_GetCurrentConnectionInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==20 && memcmp(SOAPACTION,"PrepareForConnection",20)==0)
		{
			UPnPDispatch_ConnectionManager_PrepareForConnection(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==18 && memcmp(SOAPACTION,"ConnectionComplete",18)==0)
		{
			UPnPDispatch_ConnectionManager_ConnectionComplete(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetProtocolInfo",15)==0)
		{
			UPnPDispatch_ConnectionManager_GetProtocolInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"GetCurrentConnectionIDs",23)==0)
		{
			UPnPDispatch_ConnectionManager_GetCurrentConnectionIDs(xml, ReaderObject);
		}
	}
	else if(header->DirectiveObjLength==25 && memcmp((header->DirectiveObj)+1,"ContentDirectory/control",24)==0)
	{
		if(SOAPACTIONLength==6 && memcmp(SOAPACTION,"Search",6)==0)
		{
			UPnPDispatch_ContentDirectory_Search(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==20 && memcmp(SOAPACTION,"StopTransferResource",20)==0)
		{
			UPnPDispatch_ContentDirectory_StopTransferResource(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==13 && memcmp(SOAPACTION,"DestroyObject",13)==0)
		{
			UPnPDispatch_ContentDirectory_DestroyObject(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"UpdateObject",12)==0)
		{
			UPnPDispatch_ContentDirectory_UpdateObject(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==14 && memcmp(SOAPACTION,"ExportResource",14)==0)
		{
			UPnPDispatch_ContentDirectory_ExportResource(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"GetTransferProgress",19)==0)
		{
			UPnPDispatch_ContentDirectory_GetTransferProgress(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetSearchCapabilities",21)==0)
		{
			UPnPDispatch_ContentDirectory_GetSearchCapabilities(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"GetSystemUpdateID",17)==0)
		{
			UPnPDispatch_ContentDirectory_GetSystemUpdateID(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"CreateObject",12)==0)
		{
			UPnPDispatch_ContentDirectory_CreateObject(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"GetSortCapabilities",19)==0)
		{
			UPnPDispatch_ContentDirectory_GetSortCapabilities(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==6 && memcmp(SOAPACTION,"Browse",6)==0)
		{
			UPnPDispatch_ContentDirectory_Browse(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==14 && memcmp(SOAPACTION,"ImportResource",14)==0)
		{
			UPnPDispatch_ContentDirectory_ImportResource(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"CreateReference",15)==0)
		{
			UPnPDispatch_ContentDirectory_CreateReference(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==14 && memcmp(SOAPACTION,"DeleteResource",14)==0)
		{
			UPnPDispatch_ContentDirectory_DeleteResource(xml, ReaderObject);
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
	if(d->HeadSubscriberPtr_ContentDirectory==t)
	{
		--(d->NumberOfSubscribers_ContentDirectory);
	}
	else if(d->HeadSubscriberPtr_ConnectionManager==t)
	{
		--(d->NumberOfSubscribers_ConnectionManager);
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
		if(d->HeadSubscriberPtr_ContentDirectory==info)
		{
			d->HeadSubscriberPtr_ContentDirectory = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_ContentDirectory;
			}
		}
		else if(d->HeadSubscriberPtr_ConnectionManager==info)
		{
			d->HeadSubscriberPtr_ConnectionManager = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_ConnectionManager;
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
	struct timeval tv;
	gettimeofday(&tv,NULL);
	if((info->RenewByTime).tv_sec < tv.tv_sec) {RetVal = -1;}
	return(RetVal);
}
void UPnPGetInitialEventBody_ContentDirectory(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(155+(int)strlen(UPnPObject->ContentDirectory_TransferIDs)+(int)strlen(UPnPObject->ContentDirectory_ContainerUpdateIDs)+(int)strlen(UPnPObject->ContentDirectory_SystemUpdateID));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"TransferIDs>%s</TransferIDs></e:property><e:property><ContainerUpdateIDs>%s</ContainerUpdateIDs></e:property><e:property><SystemUpdateID>%s</SystemUpdateID",UPnPObject->ContentDirectory_TransferIDs,UPnPObject->ContentDirectory_ContainerUpdateIDs,UPnPObject->ContentDirectory_SystemUpdateID);
}
void UPnPGetInitialEventBody_ConnectionManager(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(177+(int)strlen(UPnPObject->ConnectionManager_SourceProtocolInfo)+(int)strlen(UPnPObject->ConnectionManager_SinkProtocolInfo)+(int)strlen(UPnPObject->ConnectionManager_CurrentConnectionIDs));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"SourceProtocolInfo>%s</SourceProtocolInfo></e:property><e:property><SinkProtocolInfo>%s</SinkProtocolInfo></e:property><e:property><CurrentConnectionIDs>%s</CurrentConnectionIDs",UPnPObject->ConnectionManager_SourceProtocolInfo,UPnPObject->ConnectionManager_SinkProtocolInfo,UPnPObject->ConnectionManager_CurrentConnectionIDs);
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
			if(strncasecmp(f->Field,"SID",3)==0)
			{
				SID = f->FieldData;
				SIDLength = f->FieldDataLength;
			}
		}
		f = f->NextField;
	}
	sem_wait(&(ReaderObject->Parent->EventLock));
	if(header->DirectiveObjLength==23 && memcmp(header->DirectiveObj + 1,"ContentDirectory/event",22)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_ContentDirectory),&(ReaderObject->Parent->NumberOfSubscribers_ContentDirectory),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	else if(header->DirectiveObjLength==24 && memcmp(header->DirectiveObj + 1,"ConnectionManager/event",23)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager),&(ReaderObject->Parent->NumberOfSubscribers_ConnectionManager),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.0 %d %s\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			close(ReaderObject->ClientSocket);
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
	
	if(strncmp(ServiceName,"ContentDirectory",16)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_ContentDirectory);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_ContentDirectory);
	}
	if(strncmp(ServiceName,"ConnectionManager",17)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_ConnectionManager);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager);
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
			close(ReaderObject->ClientSocket);
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
		gettimeofday(&(NewSubscriber->RenewByTime),NULL);
		(NewSubscriber->RenewByTime).tv_sec += (int)Timeout;
		NewSubscriber->Next = *HeadPtr;
		if(*HeadPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}
		*HeadPtr = NewSubscriber;
		++(*TotalSubscribers);
		LVL3DEBUG(printf("\r\n\r\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\r\n",NewSubscriber->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)
		LVL3DEBUG(printf("TIMESTAMP: %d <%d>\r\n\r\n",(NewSubscriber->RenewByTime).tv_sec-Timeout,NewSubscriber);)
		packet = (char*)MALLOC(132 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,Timeout);
		if(strcmp(ServiceName,"ContentDirectory")==0)
		{
			UPnPGetInitialEventBody_ContentDirectory(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"ConnectionManager")==0)
		{
			UPnPGetInitialEventBody_ConnectionManager(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		if (packetbody != NULL)	    {
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			close(ReaderObject->ClientSocket);
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
		close(ReaderObject->ClientSocket);
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
	
	if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		UPnPTryToSubscribe("ConnectionManager",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else if(pathlength==23 && memcmp(path+1,"ContentDirectory/event",22)==0)
	{
		UPnPTryToSubscribe("ContentDirectory",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else
	{
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Invalid Service Name\r\n\r\n",37,0);
		close(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}
void UPnPRenewEvents(char* path,int pathlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct HTTPReaderObject *ReaderObject)
{
	struct SubscriberInfo *info = NULL;
	long TimeoutVal;
	struct timeval tv;
	char* packet;
	int packetlength;
	char* SID = (char*)MALLOC(SIDLength+1);
	memcpy(SID,_SID,SIDLength);
	SID[SIDLength] ='\0';
	LVL3DEBUG(gettimeofday(&tv,NULL);)
	LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",tv.tv_sec);)
	LVL3DEBUG(printf("SUBSCRIBER [%s] attempting to Renew Events for %s Duration [",SID,Timeout);)
	if(pathlength==23 && memcmp(path+1,"ContentDirectory/event",22)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_ContentDirectory;
	}
	else if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager;
	}
	while(info!=NULL && strcmp(info->SID,SID)!=0)
	{
		info = info->Next;
	}
	if(info!=NULL)
	{
		ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
		gettimeofday(&tv,NULL);
		(info->RenewByTime).tv_sec = tv.tv_sec + TimeoutVal;
		packet = (char*)MALLOC(111 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\n\r\n",SID,TimeoutVal);
		send(ReaderObject->ClientSocket,packet,packetlength,0);
		FREE(packet);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n",TimeoutVal,info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Precondition Failed\r\n\r\n",36,0);
	}
	close(ReaderObject->ClientSocket);
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
		if(f->FieldLength==3 && strncasecmp(f->Field,"SID",3)==0)
		{
			SID = f->FieldData;
			SIDLength = f->FieldDataLength;
		}
		else if(f->FieldLength==8 && strncasecmp(f->Field,"Callback",8)==0)
		{
			URL = f->FieldData;
			URLLength = f->FieldDataLength;
		}
		else if(f->FieldLength==7 && strncasecmp(f->Field,"Timeout",7)==0)
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
			if(TimeoutLength==8 && strncasecmp(Timeout,"INFINITE",8)==0)
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
	char *errorTemplate = "HTTP/1.0 %d %s\r\nServer: %s\r\n\r\n";
	char errorPacket[100];
	int errorPacketLength;
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
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			return;
		}
		else if(header->DirectiveObjLength==27 && memcmp((header->DirectiveObj)+1,"ConnectionManager/scpd.xml",26)==0)
		{
			buffer = UPnPDecompressString((char*)UPnPConnectionManagerDescription,UPnPConnectionManagerDescriptionLength,UPnPConnectionManagerDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPConnectionManagerDescriptionLengthUX, 0);
			FREE(buffer);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ContentDirectory/scpd.xml",25)==0)
		{
			buffer = UPnPDecompressString((char*)UPnPContentDirectoryDescription,UPnPContentDirectoryDescriptionLength,UPnPContentDirectoryDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPContentDirectoryDescriptionLengthUX, 0);
			FREE(buffer);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else
		{
			errorPacketLength = sprintf(errorPacket,errorTemplate,404,"File Not Found","POSIX, UPnP/1.0, Intel MicroStack/1.0.1189");
			send(ReaderObject->ClientSocket,errorPacket,errorPacketLength,0);
			close(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			return;
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
	else
	{
		errorPacketLength = sprintf(errorPacket,errorTemplate,400,"Bad Request","POSIX, UPnP/1.0, Intel MicroStack/1.0.1189");
		send(ReaderObject->ClientSocket,errorPacket,errorPacketLength,0);
		close(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
		return;
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
		bytesReceived = recv(ReaderObject->ClientSocket,ReaderObject->Header+ReaderObject->HeaderIndex,2048-ReaderObject->HeaderIndex,0);
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
							if(strncasecmp(field->Field,"content-length",14)==0)
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
			close(UPnPObject->NOTIFY_SEND_socks[i]);
		}
		FREE(UPnPObject->NOTIFY_SEND_socks);
		
		/* Set up socket */
		FREE(UPnPObject->AddressList);
		UPnPObject->AddressListLength = ILibGetLocalIPAddressList(&(UPnPObject->AddressList));
		UPnPObject->NOTIFY_SEND_socks = (int*)MALLOC(sizeof(int)*(UPnPObject->AddressListLength));
		
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
		close(RO->ClientSocket);
		RO->ClientSocket = 0;
	}
}
void UPnPMasterPostSelect(void* object,int slct, fd_set *socketset, fd_set *writeset, fd_set *errorset)
{
	int cnt = 0;
	int i;
	struct packetheader *packet;
	int NewSocket;
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
				if(UPnPObject->ReaderObjects[i].ClientSocket==0 || UPnPObject->ReaderObjects[i].FinRead!=0 || UPnPObject->ReaderObjects[i].Body!=NULL || (UPnPObject->ReaderObjects[i].ParsedHeader!=NULL && UPnPObject->ReaderObjects[i].ParsedHeader->Body != NULL))
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
	gettimeofday(&(upnp->CurrentTime),NULL);
	if((upnp->CurrentTime).tv_sec >= (upnp->NotifyTime).tv_sec)
	{
		(upnp->NotifyTime).tv_sec = (upnp->CurrentTime).tv_sec + (upnp->NotifyCycleTime/3);
		UPnPSendNotify(upnp);
	}
	return((upnp->NotifyTime).tv_sec-(upnp->CurrentTime).tv_sec);
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
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","",upnp->NotifyCycleTime);
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
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:device:MediaServer:1","urn:schemas-upnp-org:device:MediaServer:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ContentDirectory:1","urn:schemas-upnp-org:service:ContentDirectory:1","");
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
	close(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(head);
	FREE(body);
}

int UPnPPresentationResponse(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate)
{
	int status = -1;
	int TempSocket;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	status = send(ReaderObject->ClientSocket,Data,DataLength,0);
	if (Terminate != 0)
	{
		TempSocket = ReaderObject->ClientSocket;
		ReaderObject->ClientSocket = 0;
		close(TempSocket);
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
	send(ReaderObject->ClientSocket,"HTTP/1.0 200 OK\r\nEXT:\r\nCONTENT-TYPE: text/xml\r\nSERVER: POSIX, UPnP/1.0, Intel MicroStack/1.0.1189\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>",273,0);
	send(ReaderObject->ClientSocket,packet,packetlength,0);
	close(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(packet);}

void UPnPResponse_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* unescaped_ProtocolInfo, const char* unescaped_PeerConnectionManager, const int PeerConnectionID, const char* unescaped_Direction, const char* unescaped_Status)
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
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionInfo",body);
	FREE(body);
	FREE(ProtocolInfo);
	FREE(PeerConnectionManager);
	FREE(Direction);
	FREE(Status);
}

void UPnPResponse_ConnectionManager_PrepareForConnection(const void* UPnPToken, const int ConnectionID, const int AVTransportID, const int RcsID)
{
	char* body;
	
	body = (char*)MALLOC(109);
	sprintf(body,"<ConnectionID>%d</ConnectionID><AVTransportID>%d</AVTransportID><RcsID>%d</RcsID>",ConnectionID,AVTransportID,RcsID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","PrepareForConnection",body);
	FREE(body);
}

void UPnPResponse_ConnectionManager_ConnectionComplete(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","ConnectionComplete","");
}

void UPnPResponse_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* unescaped_Source, const char* unescaped_Sink)
{
	char* body;
	char *Source = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Source));
	char *Sink = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Sink));
	
	ILibXmlEscape(Source,unescaped_Source);
	ILibXmlEscape(Sink,unescaped_Sink);
	body = (char*)MALLOC(31+strlen(Source)+strlen(Sink));
	sprintf(body,"<Source>%s</Source><Sink>%s</Sink>",Source,Sink);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetProtocolInfo",body);
	FREE(body);
	FREE(Source);
	FREE(Sink);
}

void UPnPResponse_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* unescaped_ConnectionIDs)
{
	char* body;
	char *ConnectionIDs = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ConnectionIDs));
	
	ILibXmlEscape(ConnectionIDs,unescaped_ConnectionIDs);
	body = (char*)MALLOC(32+strlen(ConnectionIDs));
	sprintf(body,"<ConnectionIDs>%s</ConnectionIDs>",ConnectionIDs);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionIDs",body);
	FREE(body);
	FREE(ConnectionIDs);
}

void UPnPResponse_ContentDirectory_Search(const void* UPnPToken, const char* unescaped_Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
{
	char* body;
	char *Result = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Result));
	
	ILibXmlEscape(Result,unescaped_Result);
	body = (char*)MALLOC(134+strlen(Result));
	sprintf(body,"<Result>%s</Result><NumberReturned>%u</NumberReturned><TotalMatches>%u</TotalMatches><UpdateID>%u</UpdateID>",Result,NumberReturned,TotalMatches,UpdateID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","Search",body);
	FREE(body);
	FREE(Result);
}

void UPnPResponse_ContentDirectory_StopTransferResource(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","StopTransferResource","");
}

void UPnPResponse_ContentDirectory_DestroyObject(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","DestroyObject","");
}

void UPnPResponse_ContentDirectory_UpdateObject(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","UpdateObject","");
}

void UPnPResponse_ContentDirectory_ExportResource(const void* UPnPToken, const unsigned int TransferID)
{
	char* body;
	
	body = (char*)MALLOC(37);
	sprintf(body,"<TransferID>%u</TransferID>",TransferID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","ExportResource",body);
	FREE(body);
}

void UPnPResponse_ContentDirectory_GetTransferProgress(const void* UPnPToken, const char* unescaped_TransferStatus, const char* unescaped_TransferLength, const char* unescaped_TransferTotal)
{
	char* body;
	char *TransferStatus = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TransferStatus));
	char *TransferLength = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TransferLength));
	char *TransferTotal = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TransferTotal));
	
	ILibXmlEscape(TransferStatus,unescaped_TransferStatus);
	ILibXmlEscape(TransferLength,unescaped_TransferLength);
	ILibXmlEscape(TransferTotal,unescaped_TransferTotal);
	body = (char*)MALLOC(98+strlen(TransferStatus)+strlen(TransferLength)+strlen(TransferTotal));
	sprintf(body,"<TransferStatus>%s</TransferStatus><TransferLength>%s</TransferLength><TransferTotal>%s</TransferTotal>",TransferStatus,TransferLength,TransferTotal);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","GetTransferProgress",body);
	FREE(body);
	FREE(TransferStatus);
	FREE(TransferLength);
	FREE(TransferTotal);
}

void UPnPResponse_ContentDirectory_GetSearchCapabilities(const void* UPnPToken, const char* unescaped_SearchCaps)
{
	char* body;
	char *SearchCaps = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_SearchCaps));
	
	ILibXmlEscape(SearchCaps,unescaped_SearchCaps);
	body = (char*)MALLOC(26+strlen(SearchCaps));
	sprintf(body,"<SearchCaps>%s</SearchCaps>",SearchCaps);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSearchCapabilities",body);
	FREE(body);
	FREE(SearchCaps);
}

void UPnPResponse_ContentDirectory_GetSystemUpdateID(const void* UPnPToken, const unsigned int Id)
{
	char* body;
	
	body = (char*)MALLOC(21);
	sprintf(body,"<Id>%u</Id>",Id);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSystemUpdateID",body);
	FREE(body);
}

void UPnPResponse_ContentDirectory_CreateObject(const void* UPnPToken, const char* unescaped_ObjectID, const char* unescaped_Result)
{
	char* body;
	char *ObjectID = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ObjectID));
	char *Result = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Result));
	
	ILibXmlEscape(ObjectID,unescaped_ObjectID);
	ILibXmlEscape(Result,unescaped_Result);
	body = (char*)MALLOC(39+strlen(ObjectID)+strlen(Result));
	sprintf(body,"<ObjectID>%s</ObjectID><Result>%s</Result>",ObjectID,Result);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","CreateObject",body);
	FREE(body);
	FREE(ObjectID);
	FREE(Result);
}

void UPnPResponse_ContentDirectory_GetSortCapabilities(const void* UPnPToken, const char* unescaped_SortCaps)
{
	char* body;
	char *SortCaps = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_SortCaps));
	
	ILibXmlEscape(SortCaps,unescaped_SortCaps);
	body = (char*)MALLOC(22+strlen(SortCaps));
	sprintf(body,"<SortCaps>%s</SortCaps>",SortCaps);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","GetSortCapabilities",body);
	FREE(body);
	FREE(SortCaps);
}

void UPnPResponse_ContentDirectory_Browse(const void* UPnPToken, const char* unescaped_Result, const unsigned int NumberReturned, const unsigned int TotalMatches, const unsigned int UpdateID)
{
	char* body;
	char *Result = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Result));
	
	ILibXmlEscape(Result,unescaped_Result);
	body = (char*)MALLOC(134+strlen(Result));
	sprintf(body,"<Result>%s</Result><NumberReturned>%u</NumberReturned><TotalMatches>%u</TotalMatches><UpdateID>%u</UpdateID>",Result,NumberReturned,TotalMatches,UpdateID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","Browse",body);
	FREE(body);
	FREE(Result);
}

void UPnPResponse_ContentDirectory_ImportResource(const void* UPnPToken, const unsigned int TransferID)
{
	char* body;
	
	body = (char*)MALLOC(37);
	sprintf(body,"<TransferID>%u</TransferID>",TransferID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","ImportResource",body);
	FREE(body);
}

void UPnPResponse_ContentDirectory_CreateReference(const void* UPnPToken, const char* unescaped_NewID)
{
	char* body;
	char *NewID = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_NewID));
	
	ILibXmlEscape(NewID,unescaped_NewID);
	body = (char*)MALLOC(16+strlen(NewID));
	sprintf(body,"<NewID>%s</NewID>",NewID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","CreateReference",body);
	FREE(body);
	FREE(NewID);
}

void UPnPResponse_ContentDirectory_DeleteResource(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ContentDirectory:1","DeleteResource","");
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
	if(strncmp(eventname,"ContentDirectory",16)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_ContentDirectory;
	}
	if(strncmp(eventname,"ConnectionManager",17)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_ConnectionManager;
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
			LVL3DEBUG(gettimeofday(&tv,NULL);)
			LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",tv.tv_sec);)
			LVL3DEBUG(printf("Did not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\r\n\r\n",((struct SubscriberInfo*)info)->SID,(((struct SubscriberInfo*)info)->Address&0xFF),((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((struct SubscriberInfo*)info)->Address>>16)&0xFF),((((struct SubscriberInfo*)info)->Address>>24)&0xFF),((struct SubscriberInfo*)info)->Port,info);)
		}
		
		info = info->Next;
	}
	
	sem_post(&(UPnPObject->EventLock));
}

void UPnPSetState_ConnectionManager_SourceProtocolInfo(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_SourceProtocolInfo != NULL) FREE(UPnPObject->ConnectionManager_SourceProtocolInfo);
	UPnPObject->ConnectionManager_SourceProtocolInfo = valstr;
	body = (char*)MALLOC(46 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SourceProtocolInfo",valstr,"SourceProtocolInfo");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}

void UPnPSetState_ConnectionManager_SinkProtocolInfo(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_SinkProtocolInfo != NULL) FREE(UPnPObject->ConnectionManager_SinkProtocolInfo);
	UPnPObject->ConnectionManager_SinkProtocolInfo = valstr;
	body = (char*)MALLOC(42 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SinkProtocolInfo",valstr,"SinkProtocolInfo");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}

void UPnPSetState_ConnectionManager_CurrentConnectionIDs(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_CurrentConnectionIDs != NULL) FREE(UPnPObject->ConnectionManager_CurrentConnectionIDs);
	UPnPObject->ConnectionManager_CurrentConnectionIDs = valstr;
	body = (char*)MALLOC(50 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","CurrentConnectionIDs",valstr,"CurrentConnectionIDs");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}

void UPnPSetState_ContentDirectory_TransferIDs(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ContentDirectory_TransferIDs != NULL) FREE(UPnPObject->ContentDirectory_TransferIDs);
	UPnPObject->ContentDirectory_TransferIDs = valstr;
	body = (char*)MALLOC(32 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","TransferIDs",valstr,"TransferIDs");
	UPnPSendEvent(upnptoken,body,bodylength,"ContentDirectory");
	FREE(body);
}

void UPnPSetState_ContentDirectory_ContainerUpdateIDs(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ContentDirectory_ContainerUpdateIDs != NULL) FREE(UPnPObject->ContentDirectory_ContainerUpdateIDs);
	UPnPObject->ContentDirectory_ContainerUpdateIDs = valstr;
	body = (char*)MALLOC(46 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","ContainerUpdateIDs",valstr,"ContainerUpdateIDs");
	UPnPSendEvent(upnptoken,body,bodylength,"ContentDirectory");
	FREE(body);
}

void UPnPSetState_ContentDirectory_SystemUpdateID(void *upnptoken, unsigned int val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(10);
	sprintf(valstr,"%u",val);
	if (UPnPObject->ContentDirectory_SystemUpdateID != NULL) FREE(UPnPObject->ContentDirectory_SystemUpdateID);
	UPnPObject->ContentDirectory_SystemUpdateID = valstr;
	body = (char*)MALLOC(38 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SystemUpdateID",valstr,"SystemUpdateID");
	UPnPSendEvent(upnptoken,body,bodylength,"ContentDirectory");
	FREE(body);
}


void UPnPDestroyMicroStack(void *object)
{
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)object;
	struct SubscriberInfo  *sinfo,*sinfo2;
	int i;
	UPnPSendByeBye(upnp);
	
	sem_destroy(&(upnp->EventLock));
	FREE(upnp->ContentDirectory_TransferIDs);
	FREE(upnp->ContentDirectory_ContainerUpdateIDs);
	FREE(upnp->ContentDirectory_SystemUpdateID);
	FREE(upnp->ConnectionManager_SourceProtocolInfo);
	FREE(upnp->ConnectionManager_SinkProtocolInfo);
	FREE(upnp->ConnectionManager_CurrentConnectionIDs);
	
	FREE(upnp->AddressList);
	FREE(upnp->NOTIFY_SEND_socks);
	FREE(upnp->UUID);
	FREE(upnp->Serial);
	FREE(upnp->DeviceDescription);
	
	sinfo = upnp->HeadSubscriberPtr_ContentDirectory;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_ConnectionManager;
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
}
int UPnPGetLocalPortNumber(void *token)
{
	return(((struct UPnPDataObject*)token)->WebSocketPortNumber);
}
void *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)
{
	struct UPnPDataObject* RetVal = (struct UPnPDataObject*)MALLOC(sizeof(struct UPnPDataObject));
	char* DDT;
	struct timeval tv;
	
	gettimeofday(&tv,NULL);
	srand((int)tv.tv_sec);
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
	RetVal->ContentDirectory_TransferIDs = NULL;
	RetVal->ContentDirectory_ContainerUpdateIDs = NULL;
	RetVal->ContentDirectory_SystemUpdateID = NULL;
	RetVal->ConnectionManager_SourceProtocolInfo = NULL;
	RetVal->ConnectionManager_SinkProtocolInfo = NULL;
	RetVal->ConnectionManager_CurrentConnectionIDs = NULL;
	
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
		close(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}

