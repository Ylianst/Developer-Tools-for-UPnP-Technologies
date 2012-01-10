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

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#endif

#include "Utility.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
	#include "Windows.h"
#endif

#ifdef _DEBUG

	int utl_malloc_counter = 0;
	void* utl_malloc (size_t size)
	{
		void *ret = malloc(size);
		utl_malloc_counter++;
		
		#ifdef TRACK_MALLOC_VERBOSE
			printf("utl_malloc_counter=%d\r\n", utl_malloc_counter);
		#endif
		
		if(ret == NULL)
		{
			printf("AUGH!! Failed malloc!\r\n");
		}
		return ret;
	}

	void utl_free (void *ptr)
	{
		--utl_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("utl_malloc_counter=%d\r\n", utl_malloc_counter);
		#endif
		free(ptr);
	}
#endif


void* CopyArray(int elementSize, int numElements, const void* data)
{
	int size;
	void* dataCopy = NULL;

	size =  elementSize * numElements;
	dataCopy = (void*) MALLOC (size);
	memcpy(dataCopy, data, size);

	return dataCopy;
}


void _SafeFree (void* freeThis)
{
	if (freeThis != NULL)
	{
		utl_free (freeThis);
	}
}

void SafeFree (void** freeThis)
{
	_SafeFree(*freeThis);

	*freeThis = NULL;
}


char* SafeStringCopy (char* storeHere, const char* str)
{
	char* retVal = storeHere;
	int size = 1;
	
	if (str != NULL)
	{
		if (storeHere == NULL)
		{
			size = (int) strlen(str) + 1;
			retVal = CopyArray(1, size, str);
		}
		else
		{
			strcpy(retVal, str);
		}
	}
	else
	{
		if (storeHere == NULL)
		{
			retVal = (char*) MALLOC(1);
		}
		retVal[0] = '\0';
	}

	return retVal;
}
