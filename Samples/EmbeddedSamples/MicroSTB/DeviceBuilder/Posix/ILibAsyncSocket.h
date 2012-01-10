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

#ifndef ___ILibAsyncSocket___
#define ___ILibAsyncSocket___

#define ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR 2

enum ILibAsyncSocket_MemoryOwnership
{
	ILibAsyncSocket_MemoryOwnership_CHAIN=0,
	ILibAsyncSocket_MemoryOwnership_STATIC=1,
	ILibAsyncSocket_MemoryOwnership_USER=2
};



void* ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, void(*OnData)(void* socketModule,char* buffer,int *p_beginPointer, int endPointer,void (**InterruptPtr)(void *socketModule, void *user), void **user, int *PAUSE), void(*OnConnect)(void* socketModule, int Connected, void *user),void(*OnDisconnect)(void* socketModule, void *user),void(*OnSendOK)(void *socketModule, void *user));
unsigned int ILibAsyncSocket_GetPendingBytesToSend(void *socketModule);
unsigned int ILibAsyncSocket_GetTotalBytesSent(void *socketModule);
void ILibAsyncSocket_ResetTotalBytesSent(void *socketModule);

void ILibAsyncSocket_ConnectTo(void* socketModule, int localInterface, int remoteInterface, int remotePortNumber,void (*InterruptPtr)(void *socketModule, void *user),void *user);
int ILibAsyncSocket_Send(void* socketModule, char* buffer, int length, enum ILibAsyncSocket_MemoryOwnership UserFree);
void ILibAsyncSocket_Disconnect(void* socketModule);
void ILibAsyncSocket_GetBuffer(void *socketModule, char **buffer, int *BeginPointer, int *EndPointer);

void ILibAsyncSocket_UseThisSocket(void *socketModule,void* TheSocket,void (*InterruptPtr)(void *socketModule, void *user),void *user);

int ILibAsyncSocket_IsFree(void *socketModule);
int ILibAsyncSocket_GetLocalInterface(void *socketModule);


char* ILibGetReceivingInterface(void* ReaderObject);
void ILibAsyncSocket_Resume(void *socketModule);
#endif
