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

#ifndef __ILibHTTPClient__
#define __ILibHTTPClient__

#define ILibAddRequest_Direct(ClientModule, buffer, bufferlength,Destination, CallbackPtr, user, user2) ILibAddRequest_DirectEx(ClientModule, buffer, bufferlength,NULL,0,Destination, CallbackPtr, user, user2)

/* Forward Declaration */
struct packetheader;

void* ILibCreateHTTPClientModule(void *Chain, int MaxSockets);
void  ILibDestroyHTTPClientModule(void *ClientModule);

char* ILibGetReceivingInterface(void* ReaderObject);
void  ILibAddRequest(void *ClientModule, struct packetheader *packet,struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2), void* user, void* user2);
void  ILibAddRequest_DirectEx(void *ClientModule, char *buffer, int bufferlength,char *buffer2, int buffer2length,struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2), void* user, void* user2);
void  ILibCloseRequest(void* ReaderObject);
void  ILibDeleteRequests(void *ClientModule, void *user1);

#endif
