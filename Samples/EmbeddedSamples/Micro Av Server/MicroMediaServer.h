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

#ifndef MICROMEDIASERVER_H
#define MICROMEDIASERVER_H

/*
 *	InitMms(char*)
 *		sharedRootPath:				Indicates the root directory to share for this media server.
 *
 *	The main() function should call this function. This method will call
 *	UPnPStart() and will also do standard initialization and all that
 *	other good stuff in main().
 */
void InitMms(void* chain, void *stack, char *sharedRootPath);
void StopMms();
void UpdateIPAddresses(int *addresses, int addressesLen);

struct MMSMEDIATRANSFERSTAT
{
	char* filename;
	int   length;
	int   position;
	int   download;
};

extern void (*MmsOnStatsChanged) (void);
extern void (*MmsOnTransfersChanged) (int);
extern int MmsBrowseCount;
extern int MmsHttpRequestCount;
extern int MmsCurrentTransfersCount;
#define DOWNLOAD_STATS_ARRAY_SIZE 20
extern struct MMSMEDIATRANSFERSTAT MmsMediaTransferStats[DOWNLOAD_STATS_ARRAY_SIZE];
#ifdef _DEBUG
#define MMS_MALLOC mms_malloc
#define MMS_FREE mms_free
void* mms_malloc(int sz);
void mms_free(void* ptr);
#else
#define MMS_MALLOC malloc
#define MMS_FREE free
#endif

#endif

