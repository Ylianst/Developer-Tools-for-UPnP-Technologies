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

#ifndef __ILibMiniWebServer__
#define __ILibMiniWebServer__

/* Forward Declaration */
struct packetheader;

void* ILibCreateMiniWebServer(void *chain,int MaxSockets,void (*OnReceive) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user),void *user);
void ILibDestroyMiniWebServer(void *WebServerModule);
void ILibStartMiniWebServerModule(void *WebServerModule);
void ILibStopMiniWebServerModule(void *WebServerModule);

void ILibMiniWebServer_SetReserved(void *MWS, void *object);
void *ILibMiniWebServer_GetReserved(void *MWS);
void *ILibMiniWebServer_GetMiniWebServerFromReader(void *Reader);

int ILibGetMiniWebServerPortNumber(void *WebServerModule);
void ILibMiniWebServerSend(void *ReaderObject, struct packetheader *packet);
void ILibMiniWebServerCloseSession(void *ReaderObject);

char* ILibGetReceivingInterface(void* ReaderObject);
void ILibCloseRequest(void* ReaderObject);	

#endif
