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

#ifndef __ILibWebClient__
#define __ILibWebClient__

#define WEBCLIENT_DESTROYED 5
#define WEBCLIENT_DELETED 6

void *ILibCreateWebClient(int PoolSize,void *Chain);
void *ILibCreateWebClientEx(void (*OnResponse)(
						void *WebReaderToken,
						int InterruptFlag,
						struct packetheader *header,
						char *bodyBuffer,
						int *beginPointer,
						int endPointer,
						int done,
						void *user1,
						void *user2,
						int *PAUSE), void *socketModule, void *user1, void *user2);

void ILibWebClient_OnData(void* socketModule,char* buffer,int *p_beginPointer, int endPointer,void (**InterruptPtr)(void *socketModule, void *user), void **user, int *PAUSE);
void ILibDestroyWebClient(void *object);

void ILibWebClient_DestroyWebClientDataObject(void *token);
struct packetheader *ILibWebClient_GetHeaderFromDataObject(void *token);

void ILibWebClient_PipelineRequestEx(
	void *WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	char *headerBuffer,
	int headerBufferLength,
	int headerBuffer_FREE,
	char *bodyBuffer,
	int bodyBufferLength,
	int bodyBuffer_FREE,
	void (*OnResponse)(
		void *WebReaderToken,
		int InterruptFlag,
		struct packetheader *header,
		char *bodyBuffer,
		int *beginPointer,
		int endPointer,
		int done,
		void *user1,
		void *user2,
		int *PAUSE),
	void *user1,
	void *user2);
void ILibWebClient_PipelineRequest(
	void *WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	struct packetheader *packet,
	void (*OnResponse)(
		void *WebReaderToken,
		int InterruptFlag,
		struct packetheader *header,
		char *bodyBuffer,
		int *beginPointer,
		int endPointer,
		int done,
		void *user1,
		void *user2,
		int *PAUSE),
	void *user1,
	void *user2);

void ILibWebClient_FinishedResponse_Server(void *wcdo);
void ILibWebClient_DeleteRequests(void *WebClientToken,char *IP,int Port);
void ILibWebClient_Resume(void *wcdo);

#endif
