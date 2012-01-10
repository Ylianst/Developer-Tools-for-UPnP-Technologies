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

#ifndef REMOTE_IO_CLIENT_STACK_H
#define REMOTE_IO_CLIENT_STACK_H


// Remote IO Events
extern void (*RemoteIOConnectionChanged) (char* PeerConnection);
extern void (*RemoteIOReset) ();
extern void (*RemoteIOCommand) (unsigned short command, char* data, int datalength);

// Remote IO Settings
// Must be set before RemoteIOStart() is called
extern char*			RemoteIO_Application;
extern unsigned int		RemoteIO_MaxCommandSize;
extern int				RemoteIO_DisplayEncoding;
extern unsigned int		RemoteIO_DisplayWidth;
extern unsigned int		RemoteIO_DisplayHeight;
extern char*			RemoteIO_DeviceInformation;

// Remote IO Methods
void* CreateRemoteIO(void* Chain, void* UpnpStack);
void RemoteIO_SendCommand(unsigned short command, char* data, int datalength, int userfree);

void RemoteIO_Lock();
void RemoteIO_UnLock();

void RemoteIO_SendKeyPress(int key);
void RemoteIO_SendKeyUp(int key);
void RemoteIO_SendKeyDown(int key);
void RemoteIO_SendMouseUp(int X,int Y,int Button);
void RemoteIO_SendMouseDown(int X,int Y,int Button);
void RemoteIO_SendMouseMove(int X,int Y);

// Remote IO Command Numbers
enum RemoteIOCommandNumber
{
	// Device setup
	RIO_RESET		  =     1,
	RIO_REQUEST		  =	    2,
	RIO_QUERY_INFO	  =	    3,
	RIO_DEVICE_INFO	  =     4,
	RIO_EXIT		  =     5,
	RIO_JUMBO         =     6,

	// Display remoting
	RIO_FLUSH		  =  1002,
	RIO_REPAINT		  =  1003,
	RIO_DRAWFILLBOX	  =  1004,
	RIO_DRAWIMAGE	  =  1005,
	RIO_MOVEIMAGE	  =  1006,
	RIO_ALLOCATE	  =  1007,

	// Input remoting
	RIO_KEY_DOWN	  =  2001,
	RIO_KEY_UP		  =  2002,
	RIO_KEY_PRESS	  =  2003,
	RIO_MOUSE_MOVE	  =  2004,
	RIO_MOUSE_DOWN	  =  2005,
	RIO_MOUSE_UP	  =  2006,
	RIO_CUSTOM_INPUT  =  3000,

	RIO_SETOBJECT	  =  4001,
	RIO_COPYOBJECT    =  4002,
	RIO_CLEAROBJECT	  =  4003,
	RIO_SETVECTOR     =  4004,
	RIO_CLEARVECTOR   =  4005,

	// Custom messages
	RIO_CUSTOM_DATA	  = 10000,

	RIO_XWPC_BIGIMAGE = 30001,
	RIO_XWPC_PING     = 30010,
	RIO_XWPC_PONG     = 30011
};

#pragma pack(1)
struct RIO_COMMAND_DRAWFILLBOX
{
	unsigned short x;
	unsigned short y;
	unsigned short w;
	unsigned short h;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct RIO_COMMAND_DRAWIMAGE
{
	unsigned short x;
	unsigned short y;
};

struct RIO_COMMAND_MOVEIMAGE
{
	unsigned short x1;
	unsigned short y1;
	unsigned short x2;
	unsigned short y2;
	unsigned short w;
	unsigned short h;
};

struct RIO_COMMAND_ALLOCATE
{
	unsigned short x;
	unsigned short y;
	unsigned short w;
	unsigned short h;
	unsigned short id;		// Surface Identifier
};

struct RIO_COMMAND_OBJECT
{
	short id;		// Object Identifier
};

struct RIO_COMMAND_COPYOBJECT
{
	short id;		// Object Identifier
	short x;
	short y;
};

struct RIO_COMMAND_SETVECTOR
{
	short id;		// Object Identifier
	short xs;		// X Start
	short ys;		// Y Start
	short xt;		// X Target
	short yt;		// Y Target
	short pc;		// Position XY Count
	short ss;		// Scale Start
	short st;		// Scale Target
	short sc;		// Scale Count
	short flag;		// Flags
};

struct RIO_XWPC_BIGIMAGE
{
	unsigned int   datasize; // Data size of x,y,width,height and the following image.
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
};

// Remote IO Command Numbers
enum RemoteIOImageFormat
{
	RemoteIO_JPEG = 1,
	RemoteIO_PNG  = 2,
	RemoteIO_BMP  = 3,
	RemoteIO_GIF  = 4,
	RemoteIO_TIFF = 5
};

#pragma pack()

#endif
