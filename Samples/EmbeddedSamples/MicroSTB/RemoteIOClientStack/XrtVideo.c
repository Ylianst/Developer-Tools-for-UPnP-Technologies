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
#include <string.h>
#include "RemoteIOClientStack.h"

void XrtVideo_ConnectionChangedSink(char* PeerConnection)
{
	printf("XrtVideo_ConnectionChanged: PeerConnection = %s\r\n",PeerConnection);
	
	// TODO: Generaly if PeerConnection == NULL, clear the screen. Otherwise, do nothing.
}

void XrtVideo_ResetSink()
{
	printf("XrtVideo_Reset\r\n");
	
	// TODO: Used for debugging, do nothing.
}

void XrtVideo_CommandSink(unsigned short command, char* data, int datalength)
{
	struct RIO_COMMAND_DRAWFILLBOX* cmd_fillbox;
	struct RIO_COMMAND_DRAWIMAGE* cmd_image;
	struct RIO_COMMAND_ALLOCATE* cmd_allocate;
	struct RIO_COMMAND_OBJECT* cmd_setobject;
	struct RIO_COMMAND_COPYOBJECT* cmd_copyobject;
	struct RIO_XWPC_BIGIMAGE* cmd_bigimage;

	switch (command)
	{
		case RIO_RESET:
			printf("RemoteIO RESET Command\r\n");
			// TODO: Reset, debuggin do nothing
			break;
		case RIO_EXIT:
			printf("RemoteIO EXIT Command\r\n");
			// TODO: Exit, debugging do nothing.
			break;
		case RIO_DRAWFILLBOX:
			if (datalength != 11) break;
			cmd_fillbox = (struct RIO_COMMAND_DRAWFILLBOX*)data;
			printf("RemoteIO DRAWFILLBOX Command: (X=%d,Y=%d,W=%d,H=%d,CR=%d,CG=%d,CB=%d)\r\n",cmd_fillbox->x,cmd_fillbox->y,cmd_fillbox->w,cmd_fillbox->h,cmd_fillbox->r,cmd_fillbox->g,cmd_fillbox->b);
			// TODO: Draw Fill Box on screen. Box location & color provided
			break;
		case RIO_DRAWIMAGE:
			if (datalength < 4) break;
			cmd_image = (struct RIO_COMMAND_DRAWIMAGE*)data;
			printf("RemoteIO DRAWIMAGE Command: (X=%d,Y=%d,SIZE=%d)\r\n",cmd_image->x,cmd_image->y,datalength-4);
			// TODO: Draw Image at (data+4).
			// Image width and height must be obtained by decoding the image.
			// Image format is set in the main() method.
			break;
		case RIO_ALLOCATE:
			if (datalength < 10) break;
			cmd_allocate = (struct RIO_COMMAND_ALLOCATE*)data;
			printf("RemoteIO ALLOCATE Command: (X=%d,Y=%d,W=%d,H=%d,ID=%d)\r\n",cmd_allocate->x,cmd_allocate->y,cmd_allocate->w,cmd_allocate->h,cmd_allocate->id);
			// TODO: OPTIONAL - Place video window
			// Generaly, ID = 1 for the main video window, Window is allocated but should not show
			// unless video is actualy playing.
			break;
		case RIO_SETOBJECT:
			if (datalength < 4) break;
			cmd_setobject = (struct RIO_COMMAND_OBJECT*)data;
			printf("RemoteIO SETOBJECT Command: (ID=%d,DataLength=%d)\r\n",cmd_setobject->id,datalength-2);
			// TODO: OPTIONAL - Place this object into object cache
			break;
		case RIO_CLEAROBJECT:
			if (datalength < 2) break;
			cmd_setobject = (struct RIO_COMMAND_OBJECT*)data;
			printf("RemoteIO CLEAROBJECT Command: (ID=%d)\r\n",cmd_setobject->id);
			// TODO: OPTIONAL - Clear object ID from cache
			break;
		case RIO_COPYOBJECT:
			if (datalength < 6) break;
			cmd_copyobject = (struct RIO_COMMAND_COPYOBJECT*)data;
			printf("RemoteIO COPYOBJECT Command: (ID=%d,X=%d,Y=%d)\r\n",cmd_copyobject->id,cmd_copyobject->x,cmd_copyobject->y);
			// TODO: OPTIONAL - Copy object ID to location X,Y on screen
			break;
		case RIO_XWPC_BIGIMAGE:
			cmd_bigimage = (struct RIO_XWPC_BIGIMAGE*)data;
			printf("RemoteIO XWPC_BIGIMAGE Command: (X=%d,Y=%d,DataLength=%d)\r\n",cmd_bigimage->x,cmd_bigimage->y,datalength-12);
			// TODO: PROPRIATARY OPTIONAL - Draw image (data+12) to position X,Y on screen
			break;
		case RIO_XWPC_PING:
			// Respon with PONG command
			printf("RemoteIO got PING, sending PONG\r\n");
			RemoteIO_SendCommand(RIO_XWPC_PONG,"X",1,1);
			break;
		default:
			printf("XrtVideo_CommandSink: Command = %d, DataLength = %d\r\n",command,datalength);
			// TODO: Do nothing. Used for custom commands such as an LCD panel, LED's and more.
			break;
	}
}
