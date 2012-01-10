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


#ifndef UTILITY_H
#define UTILITY_H

#ifdef _DEBUG
#define MALLOC(x) utl_malloc(x)
#define FREE(x)   utl_free(x)
void *utl_malloc(size_t size);
void utl_free(void *ptr);
#else
#define MALLOC malloc
#define FREE   free
#endif


/*
 *	Defines an empty string.
 */
#define EMPTY_STRING ""

/*
 *	Used to prevent warnings on assinign NULL
 *	to a char*
 */
#define NULL_CHAR '\0'

/*
 *	Copies memory from one location to a new location 
 *	and returns the pointer.
 */
void* CopyArray(int elementSize, int numElements, const void* data);

/*
 *	Does a normal FREE on freeThis, except
 *	that it checks for non-NULL value first.
 */
void _SafeFree (void* freeThis);

/*
 *	This macro calls _SafeFree and then assigns
 *	the pointer to NULL, for extra safety.
 */
//#define SafeFree(x) _SafeFree(x); x = NULL;
void SafeFree(void** freeThis);

/*
 *	Copies a string safely. 
 *	If str is NULL returned value is an empty string.
 *
 *	If storeHere is NULL, then memory is allocated
 *	by the method. Use SafeFree() to deallocate
 *	that memory.
 *
 *	Returns the copy of str.
 */
char* SafeStringCopy (char* storeHere, const char* str);

/*
 *	Spawns a normal thread that is detached
 *	from the calling thread.
 */
void SpawnDetachedThread(void* method, void* arg);
#endif
