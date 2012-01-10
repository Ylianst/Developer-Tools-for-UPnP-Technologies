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

#ifndef ___ILibAsyncServerSocket___
#define ___ILibAsyncServerSocket___

void *ILibCreateAsyncServerSocketModule(void *Chain, int MaxConnections, int PortNumber, int initialBufferSize, void (*OnConnect)(void *AsyncServerSocketModule, void *ConnectionToken,void **user),void (*OnDisconnect)(void *AsyncServerSocketModule, void *ConnectionToken, void *user),void (*OnReceive)(void *AsyncServerSocketModule, void *ConnectionToken,char* buffer,int *p_beginPointer, int endPointer,void (**OnInterrupt)(void *AsyncServerSocketMoudle, void *ConnectionToken, void *user), void **user, int *PAUSE),void (*OnInterrupt)(void *AsyncServerSocketModule, void *ConnectionToken, void *user),void (*OnSendOK)(void *AsyncServerSocketModule,void *ConnectionToken, void *user));

void *ILibAsyncServerSocket_GetTag(void *AsyncSocketModule);
void ILibAsyncServerSocket_SetTag(void *AsyncSocketModule, void *user);

unsigned short ILibAsyncServerSocket_GetPortNumber(void *ServerSocketModule);

#define ILibAsyncServerSocket_Send(ServerSocketModule, ConnectionToken, buffer, bufferLength, UserFreeBuffer) ILibAsyncSocket_Send(ConnectionToken,buffer,bufferLength,UserFreeBuffer)
#define ILibAsyncServerSocket_Disconnect(ServerSocketModule, ConnectionToken) ILibAsyncSocket_Disconnect(ConnectionToken)
#define ILibAsyncServerSocket_GetPendingBytesToSend(ServerSocketModule, ConnectionToken) ILibAsyncSocket_GetPendingBytesToSend(ConnectionToken)
#define ILibAsyncServerSocket_GetTotalBytesSent(ServerSocketModule, ConnectionToken) ILibAsyncSocket_GetTotalBytesSent(ConnectionToken)
#define ILibAsyncServerSocket_ResetTotalBytesSent(ServerSocketModule, ConnectionToken) ILibAsyncSocket_ResetTotalBytesSent(ConnectionToken)

#endif