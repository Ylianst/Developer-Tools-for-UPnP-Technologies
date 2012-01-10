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

#include "DB2CDS.h"
#include "MediaServerLogic.h"
#include "CDsObjectToDidl.h"
#include "MyString.h"
#include "UpnpMicroStack.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>

// POSIX-style synchronization
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#ifdef _DEBUG
#define DB2CDS_MALLOC db2cds_malloc
#define DB2CDS_FREE db2cds_free

int DB2CDS_malloc_counter = 0;
void* db2cds_malloc(int size)
{
	++DB2CDS_malloc_counter;
	return malloc(size);
}
void db2cds_free(void *freeThis)
{
	--DB2CDS_malloc_counter;
	free(freeThis);
}
#endif


#ifndef _DEBUG
#define DB2CDS_MALLOC malloc
#define DB2CDS_FREE free
#endif

/*
 *	Deallocates the DB2CDS_CdsQuery obtained from DB2CDS_EnqueueCdsQuery().
 */
void DB2CDS_DeallocateCdsQuery(struct DB2CDS_CdsQuery *query);

/*
 *	Returns the DB2CDS_CdsQuery  that is at the head
 *	and removes it from the queue.
 */
struct DB2CDS_CdsQuery* DB2CDS_DequeueCdsQuery();

/*
 *	Returns the DB2CDS_CdsQuery that was at the head before
 *	this object was added.
 */
struct DB2CDS_CdsQuery* DB2CDS_EnqueueCdsQuery(struct MSL_CdsQuery *cdsQuery, int ipAddress, int ipPort);

/* If nonzero, DB2CDS_StartCdsQueryProcessing() will continue to execute. */
int DB2CDS_ContinueProcessing = 0;

/* Ensures thread-safe enqueue/dequeue operations on DB2CDS_RequestTail and DB2CDS_RequestTail */
sem_t DB2CDS_QueryLock;

/* Provides pointers for the head/tail of a linked FIFO queue of CDS queries. */
struct DB2CDS_CdsQuery *DB2CDS_QueryHead, *DB2CDS_QueryTail=NULL;

/* This lock is used to block the thread if there's nothing to process. */
sem_t DB2CDS_ProcessQueriesLock;

/* Function callback that can be executed right before responding to a CDS query */
DB2CDS_Callback_OnQuery DB2CDS_OnQuery = NULL;




void DB2CDS_DeallocateCdsQuery(struct DB2CDS_CdsQuery *query)
{
	/* deallocate memory */
	MSL_DeallocateCdsQuery(query->MslCdsQuery);

	if (query->Filter != NULL) DB2CDS_FREE(query->Filter);
	if (query->ObjectID != NULL) DB2CDS_FREE(query->ObjectID);
	if (query->SearchCriteria != NULL) DB2CDS_FREE(query->SearchCriteria);
	if (query->SortCriteria != NULL) DB2CDS_FREE(query->SortCriteria);

	DB2CDS_FREE(query);
}

struct DB2CDS_CdsQuery* DB2CDS_DequeueCdsQuery()
{
	struct DB2CDS_CdsQuery *retVal = NULL;

	/*
	 *	Return the DB2CDS_CdsQuery at the head.
	 *	Appropriately move the DB2CDS_QueryHead pointer
	 *	forward.
	 *	If nothing left in the queue, set the 
	 *	DB2CDS_QueryTail to NULL.
	 */

	sem_wait(&DB2CDS_QueryLock);
	
	retVal = DB2CDS_QueryHead;
	if (retVal!=NULL)
	{
		DB2CDS_QueryHead = retVal->Next;
		if (DB2CDS_QueryHead == NULL)
		{
			DB2CDS_QueryTail = NULL; 
		}
	}
	retVal->Next = NULL;
	sem_post(&DB2CDS_QueryLock);

	return retVal;
}

struct DB2CDS_CdsQuery* DB2CDS_EnqueueCdsQuery(struct MSL_CdsQuery *cdsQuery, int ipAddress, int ipPort)
{
	int len, wSize;
	struct DB2CDS_CdsQuery* newQuery, *retVal;
	
	/*
	 *	Allocate a DB2CDS_CdsQuery object (newQuery) and 
	 *	copy all of the arguments from cdsQuery.
	 *	Ensure that all strings, except cdsQuery->Filter,
	 *	are converted from their UTF-8 encoding to
	 *	a wide/unicode encoding. Afterwards, we'll keep
	 *	cdsQuery and assign it as a field of newQuery.
	 */

	newQuery = (struct DB2CDS_CdsQuery*) DB2CDS_MALLOC(sizeof(struct DB2CDS_CdsQuery));
	memset(newQuery, 0, sizeof(struct DB2CDS_CdsQuery));

	len = (int) strlen(cdsQuery->Filter);
	newQuery->Filter = (char*) DB2CDS_MALLOC(len+1);
	memcpy(newQuery->Filter, cdsQuery->Filter, len+1);

	len = (int) strlen(cdsQuery->ObjectID);
	wSize = (len+1)<<1;
	newQuery->ObjectID = (wchar_t*) DB2CDS_MALLOC(wSize);
	Utf8ToWide(newQuery->ObjectID, cdsQuery->ObjectID, wSize);

	newQuery->QueryType = cdsQuery->QueryType;
	newQuery->RequestedCount = cdsQuery->RequestedCount;
	
	newQuery->RequestedOnAddress = ipAddress;
	newQuery->RequestedOnPort = ipPort;

	len = (int) strlen(cdsQuery->SearchCriteria);
	wSize = (len+1)<<1;
	newQuery->SearchCriteria = (wchar_t*) DB2CDS_MALLOC(wSize);
	Utf8ToWide(newQuery->SearchCriteria, cdsQuery->SearchCriteria, wSize);

	len = (int) strlen(cdsQuery->SortCriteria);
	wSize = (len+1)<<1;
	newQuery->SortCriteria = (wchar_t*) DB2CDS_MALLOC(wSize);
	Utf8ToWide(newQuery->SortCriteria, cdsQuery->SortCriteria, wSize);

	newQuery->StartingIndex = cdsQuery->StartingIndex;
	newQuery->UpnpToken = cdsQuery->UpnpToken;

	newQuery->MslCdsQuery = cdsQuery;

	/*
	 *	Add newQuery to this module's list of CDS requests
	 *	that need to be processed. Ensure that the
	 *	query object is added in a thread-safe manner.
	 */
	sem_wait(&DB2CDS_QueryLock);
	
	retVal = DB2CDS_QueryHead;
	if (DB2CDS_QueryTail == NULL)
	{
		DB2CDS_QueryHead = DB2CDS_QueryTail = newQuery;
	}
	else
	{
		DB2CDS_QueryTail = DB2CDS_QueryTail->Next = newQuery;
	}

	sem_post(&DB2CDS_QueryLock);

	return retVal;
}

void DB2CDS_StartCdsQueryProcessing(DB2CDS_Callback_OnQuery queryCallback)
{
	struct DB2CDS_CdsQuery *query;

	sem_init(&DB2CDS_QueryLock, 0, 1);
	sem_init(&DB2CDS_ProcessQueriesLock, 0, 1);
	DB2CDS_OnQuery = queryCallback;

	DB2CDS_ContinueProcessing = 1;
	while (DB2CDS_ContinueProcessing != 0)
	{
		query = DB2CDS_DequeueCdsQuery();
		if (query != NULL)
		{
			//TODO: Query the database and respond.

			if (DB2CDS_OnQuery != NULL)
			{
				DB2CDS_OnQuery(query);
			}

			DB2CDS_DeallocateCdsQuery(query);
		}
		else
		{
			sem_wait(&DB2CDS_ProcessQueriesLock);
		}
	}

	sem_destroy(&DB2CDS_ProcessQueriesLock);
	sem_destroy(&DB2CDS_QueryLock);
}

void DB2CDS_StopCdsQueryProcessing()
{
	DB2CDS_ContinueProcessing = 0;
}

void DB2CDS_OnBrowseSearch (struct MSL_CdsQuery *cdsQuery)
{
	struct DB2CDS_CdsQuery *front;

	front = DB2CDS_EnqueueCdsQuery
		(
		cdsQuery, 
		UpnpGetLocalInterfaceToHost(cdsQuery->UpnpToken),
		UpnpGetLocalPortNumber(cdsQuery->UpnpToken)
		);

	if (front == NULL)
	{
		sem_post(&DB2CDS_ProcessQueriesLock);
	}
}
