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

#ifndef _WSC_PORTAB_
#define _WSC_PORTAB_

#include "WscTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/*Synchronization functions*/
uint32 WscSyncCreate(uint32 **handle);
uint32 WscSyncDestroy(uint32 *handle);
uint32 WscLock(uint32 *handle);
uint32 WscUnlock(uint32 *handle);

/*Thread functions*/
uint32 WscCreateThread(uint32 *handle, 
                        void *(*threadFunc)(void *),
                        void *arg);
void WscDestroyThread(uint32 handle);

/*Event functions*/
uint32 WscCreateEvent(uint32 *handle);
uint32 WscDestroyEvent(uint32 handle);
uint32 WscSetEvent(uint32 handle);
uint32 WscResetEvent(uint32 handle);
uint32 WscSingleWait(uint32 handle, uint32 *lock, uint32 timeout);

/*Byte swapping functions*/
uint32 WscHtonl(uint32 intlong);
uint16 WscHtons(uint16 intshort);
uint32 WscNtohl(uint32 intlong);
uint16 WscNtohs(uint16 intshort);

/*Sleep function*/
void WscSleep(uint32 seconds);

#ifdef __cplusplus
}
#endif

#endif /*_WSC_PORTAB_*/
