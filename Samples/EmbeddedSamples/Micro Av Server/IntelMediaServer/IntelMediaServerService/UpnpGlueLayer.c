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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WINSOCK2
	#include <winsock2.h>
#endif

#ifdef WIN32
	#include <crtdbg.h>
#endif

#ifdef _POSIX
	#include <semaphore.h>
#endif

#include "UpnpGlueLayer.h"
#include "UpnpMicroStack.h"
#include "ILibParsers.h"
#include "PortingFunctions.h"


#ifdef _DEBUG
	int UGL_malloc_counter = 0;
	#define UGL_MALLOC UGL_malloc
	#define UGL_FREE UGL_free

	void* UGL_malloc(int size)
	{
		UGL_malloc_counter++;
		return malloc(size);
	}

	void UGL_free(void *freeThis)
	{
		UGL_malloc_counter--;
		free(freeThis);
	}
#endif

#ifndef _DEBUG
	#define UGL_MALLOC malloc
	#define UGL_FREE free
#endif

#ifndef WINSOCK2
	void UGL_IPAddressMonitor(void *data);
#endif

#ifdef WINSOCK2
	void CALLBACK UGL_IPAddressMonitor
	(
	IN DWORD dwError, 
	IN DWORD cbTransferred, 
	IN LPWSAOVERLAPPED lpOverlapped, 
	IN DWORD dwFlags 
	);
#endif

sem_t UGL_QuitLock;

struct MSL_CdsQuery* UGL_QueriesAdd(struct MSL_CdsQuery *cdsQuery);
void UGL_QueriesClear();
struct MSL_CdsQuery* UGL_QueriesGet();

void UGL_StartMediaServer(void *ignored);
void UGL_StartQueryProcessorAndMediaServer(void* ignored);

void UGL_OnQuery(struct MSL_CdsQuery *cdsQuery);
void UGL_OnStats(struct MSL_Stats *stats);

void *UGL_MicroStackChain;		/* thread chain object for the upnp stack */
void *UGL_MicroStack;			/* object representing the upnp stack*/
void *UGL_Monitor;				/* lifetime monitor object, used for various upnp components */
void *UGL_MediaServer;			/* MediaServerLogic object */

#ifdef WINSOCK2
SOCKET UGL_MonitorSocket;
DWORD UGL_MonitorSocketReserved;
WSAOVERLAPPED UGL_MonitorSocketStateObject;
#endif

sem_t UGL_IPAddressLock;		/* read/write lock for list of IP addresses */
int UGL_IPAddressLength;		/* number of IP addresses for the machine */
int *UGL_IPAddressList;			/* array of IP addresses for the machine */

int	UGL_Running, UGL_Running2 = 0;	/* nonzero means that the MediaServer is running */
sem_t UGL_InitLock;				/* lock used when initializing this glue layer */
sem_t UGL_ProcessRequestsLock;	/* lock used to block the thread that processes queries when there is nothing to process */
int UGL_ContinueProcessing = 0;	/* flag to indicate if the glue layer should continue processing queries */

sem_t UGL_QueryLock;			/* all queries are stuffed into a queue, synchronized by this lock*/
struct MSL_CdsQuery *UGL_Queue_Head = NULL, *UGL_Queue_Tail = NULL;	/* head/tail pointers for the queue of queries */

void *UGL_CDS_Object;			/* points to whatever gets sent in UGL_StartUPnP()*/

/* Callback pointers - provided in StartUPnP()*/
void (*UGL_CallbackOnQuery) (void *callbackObject, struct MSL_CdsQuery *cdsQuery) = NULL;
void (*UGL_CallbackOnStatsChanged) (void *callbackObject, struct MSL_Stats *stats) = NULL;



#ifndef WINSOCK2
void UGL_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	sem_wait(&UGL_IPAddressLock);

	// get the list of IP addresses 
	length = ILibGetLocalIPAddressList(&list);

	if(length!=UGL_IPAddressLength || memcmp((void*)list,(void*)UGL_IPAddressList,sizeof(int)*length)!=0)
	{
		// tells microstack to readvertise on new interfaces
		UpnpIPAddressListChanged(UGL_MicroStack);
		
		// Get pointer to the the IP address list.
		// Be sure to delete the old memory.
		UGL_FREE(UGL_IPAddressList);
		UGL_IPAddressList = list;
		UGL_IPAddressLength = length;

		// Tell mediaServerLogic to update its list of IP addresses
		MSL_UpdateIpInfo(UGL_MediaServer, UGL_IPAddressList, UGL_IPAddressLength);
	}
	else
	{
		UGL_FREE(list);
	}
	
	sem_post(&UGL_IPAddressLock);

	// reregister for the event
	ILibLifeTime_Add(UGL_Monitor,NULL,4,&UGL_IPAddressMonitor,NULL);
}
#endif

#ifdef WINSOCK2
void CALLBACK UGL_IPAddressMonitor
(
IN DWORD dwError, 
IN DWORD cbTransferred, 
IN LPWSAOVERLAPPED lpOverlapped, 
IN DWORD dwFlags 
)
{
	int length;
	int *list;
	
	sem_wait(&UGL_IPAddressLock);

	// tells microstack to readvertise its IP addresses
	UpnpIPAddressListChanged(UGL_MicroStack);

	// get the list of IP addresses
	length = ILibGetLocalIPAddressList(&list);

	// tell the microstack to change its advertisements to match the new IP address list
	UpnpIPAddressListChanged(UGL_MicroStackChain);
	
	// free the existing list
	// and assign the new list
	UGL_FREE(UGL_IPAddressList);
	UGL_IPAddressList = list;
	UGL_IPAddressLength = length;

	// Tell MediaServerLogic to update its list of IP addresses
	MSL_UpdateIpInfo(UGL_MediaServer, UGL_IPAddressList, UGL_IPAddressLength);
	
	sem_post(&UGL_IPAddressLock);

	// reregister the event
	WSAIoctl(UGL_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UGL_MonitorSocketReserved,&UGL_MonitorSocketStateObject,&UGL_IPAddressMonitor);
}
#endif

/*
 *	This method executes when the MediaServer received a query.
 *	Because we don't want the database to stall the UPnP thread,
 *	we're going to send cdsQuery to a different thread by
 *	sticking it into a queue. The data paramters on cdsQuery 
 *	have already been deep copied so we will not need to copy anything.
 */
void UGL_OnQuery (struct MSL_CdsQuery *cdsQuery)
{
	struct MSL_CdsQuery *tail;

	/*
	 *	NOTE: Do not set cdsQuery->UserObject to anything
	 *	as that's the field that maintains the linked
	 *	list for the queue.
	 */

	tail = UGL_QueriesAdd(cdsQuery);
	if (tail == NULL)
	{
		sem_post(&UGL_ProcessRequestsLock);
	}
}

/*
 *	This method executes when the invocation stats on a MediaServer changes.
 */
void UGL_OnStats (struct MSL_Stats *stats)
{
	if (UGL_CallbackOnStatsChanged != NULL)
	{
		UGL_CallbackOnStatsChanged(UGL_CDS_Object, stats);
	}
}


void UGL_StartMediaServer(void *ignored)
{
	char udn[20];
	char friendlyname[100];
	int i, error = 0;
	WSADATA wsaData;

	/*
	 *	Be sure that this method doesn't do its work
	 *	until after UGL_StartQueryProcessorAndMediaServer()
	 *	finishes its initialization.
	 */
	sem_wait(&UGL_InitLock);
	UGL_Running2 = 1;

	/* Randomized udn generation */
	srand((unsigned int)time(NULL));
	for (i=0;i<19;i++)
	{
		udn[i] = (rand() % 25) + 66;
	}
	udn[19] = 0;

	/* get friendly name with hostname */
	if (WSAStartup(MAKEWORD(1,1), &wsaData) == 0)
	{
		memcpy(friendlyname,"Intel MediaServer (\0",19);
		gethostname(friendlyname+24,60);
		memcpy(friendlyname+strlen(friendlyname),")\0",2);

		/* create the microstack for the media server */
		UGL_MicroStackChain = ILibCreateChain();
		UGL_MicroStack = UpnpCreateMicroStack(UGL_MicroStackChain, friendlyname, udn, "0000001",1800,0);

		sem_init(&UGL_IPAddressLock, 0, 1);

		/* create the lifetime monitor object used for the MediaServer as well as to update the list of IP addresses */
		UGL_Monitor = ILibCreateLifeTime(UGL_MicroStackChain);

		/* grab the current list of IP addresses */
		UGL_IPAddressLength = ILibGetLocalIPAddressList(&UGL_IPAddressList);

		/* create the media server logic that wires up the callback and sinks for CDS queries */
		UGL_MediaServer = MSL_CreateMediaServer(UGL_MicroStackChain, UGL_MicroStack, UGL_Monitor, "http-get:*:*:*", "", UGL_SortCriteriaString, UGL_SearchCriteriaString);

		/* obtain the list of IP addresses */
		MSL_UpdateIpInfo(UGL_MediaServer, UGL_IPAddressList, UGL_IPAddressLength);

		#ifndef WINSOCK2
		/* set the IP address updater to poll every 4 seconds) */
		ILibLifeTime_Add(UGL_Monitor,NULL,4,&UGL_IPAddressMonitor,NULL);
		#endif

		#ifdef WINSOCK2
		UGL_MonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
		WSAIoctl(UGL_MonitorSocket,SIO_ADDRESS_LIST_CHANGE,NULL,0,NULL,0,&UGL_MonitorSocketReserved,&UGL_MonitorSocketStateObject,&UGL_IPAddressMonitor);
		#endif

		/*
		 *	Wire up callbacks from MSL to sinks in this file, so that they can 
		 *	dispatched to static methods.
		 */
		MSL_Callback_OnQuery = UGL_OnQuery;
		MSL_OnStatsChanged = UGL_OnStats;
	}
	else
	{
		error = 1;
		UGL_ContinueProcessing = 0;
	}

	/* release the init lock*/
	sem_post(&UGL_InitLock);

	if (error == 0)
	{
		/* start the media server */
		ILibStartChain(UGL_MicroStackChain);
	}

	sem_destroy(&UGL_IPAddressLock);
	WSACleanup();
	UGL_Running2 = 0;
	sem_post(&UGL_ProcessRequestsLock);
	sem_post(&UGL_QuitLock);
}

void UGL_StartQueryProcessorAndMediaServer(void* ignored)
{
	struct MSL_CdsQuery *query = NULL;
	sem_t *QuitLock = (sem_t*)ignored;

	// Initialize the QuitLock, and set the initial value to locked
	sem_init(&UGL_QuitLock,0,0);

	/* initialize the init lock and acquire it */
	sem_init(&UGL_InitLock, 0, 1);
	sem_wait(&UGL_InitLock);

	/*
	 *	Spawn a new thread that will execute the UPnP stack.
	 */
	SpawnNormalThread(&UGL_StartMediaServer, (void*) NULL);

	/* set this method to loop until told to stop */
	UGL_ContinueProcessing = 1;

	/*
	 *	Create a lock that will block this thread if 
	 *	there is nothing to process.
	 */
	sem_init(&UGL_ProcessRequestsLock, 0, 1);

	/*
	 *	Create a lock that will serialize calls to
	 *	the get, add, clear methods for the query queue.
	 */
	sem_init(&UGL_QueryLock, 0, 1);

	/*
	 *	Indicate that we're done initializing, this will allow 
	 *	UGL_StartMediaServer() to do its work.
	 */
	sem_post(&UGL_InitLock);

	/*
	 *	Loop until told to stop.
	 *	We're told to stop through UGL_StopUPnP().
	 */
	while (UGL_ContinueProcessing != 0)
	{
		query = UGL_QueriesGet();
		
		if(query != NULL)
		{
			/*
			 *	NOTE: Callback sink is free to use query->UserObject
			 */

			if (UGL_CallbackOnQuery != NULL)
			{
				UGL_CallbackOnQuery(UGL_CDS_Object, query);
			}
			
			/* free the memory for the query */
			MSL_DeallocateCdsQuery(query);
		}
		else
		{
			/* there are no queries to process, so block */
			sem_wait(&UGL_ProcessRequestsLock);
		}
	}

	/* be sure to properly destroy all of the pending queries */
	UGL_QueriesClear();

	/* destroy the locks when we're done. */
	sem_destroy(&UGL_QueryLock);
	sem_destroy(&UGL_ProcessRequestsLock);
	sem_destroy(&UGL_InitLock);

	/* spin here until the other thread stops */
	sem_wait(&UGL_QuitLock);
	sem_destroy(&UGL_QuitLock);
	//while (UGL_Running2 != 0)
	//{
	//	//TODO: Low priority - make this delay POSIX friendly
	//	Sleep(100);
	//}

	/* when this thread stops, then it's ok to call StartUPnP() */
	UGL_Running = 0;
	UGL_CDS_Object = NULL;
	sem_post(QuitLock);
}

void UGL_StartUPnP(void* callbackObject, void *QuitLock)
{
	if (UGL_Running == 0)
	{
		UGL_Running = 1;
		UGL_CDS_Object = callbackObject;
		SpawnNormalThread(&UGL_StartQueryProcessorAndMediaServer, QuitLock);

		while (MSL_Callback_OnQuery == NULL)
		{
			Sleep(100);
		}
	}
}

void* UGL_GetServerToken()
{
	void *retVal = NULL;

	if (UGL_Running == 1)
	{
		retVal = UpnpGetWebServerToken(UGL_MicroStack);
	}

	return retVal;
}

void UGL_StopUPnP()
{
	if (UGL_Running == 1)
	{
		/*
		*	Instruct the threads executing the UPnP stack
		*	and the CDS query processing to stop.
		*	Clean up the IP address lock.
		*/

		ILibStopChain(UGL_MicroStackChain);
		UGL_ContinueProcessing = 0;
	}
}


/*
 *	Adds a new query to the queue and returns the previous tail.
 */
struct MSL_CdsQuery* UGL_QueriesAdd(struct MSL_CdsQuery *cdsQuery)
{
	struct MSL_CdsQuery* retVal = NULL;

	if (cdsQuery->UserObject == NULL)
	{
		sem_wait(&UGL_QueryLock);
		if (UGL_Queue_Head == NULL)
		{
			UGL_Queue_Head = UGL_Queue_Tail = cdsQuery;
		}
		else
		{
			retVal = UGL_Queue_Tail;
			UGL_Queue_Tail = UGL_Queue_Tail->UserObject = cdsQuery;
		}
		sem_post(&UGL_QueryLock);
	}

	return retVal;
}

/*
 *	Clears the current queue of queries and properly deallocates them.
 */
void UGL_QueriesClear()
{
	struct MSL_CdsQuery *query, *next;

	sem_wait(&UGL_QueryLock);
	query = UGL_Queue_Head;
	while (query != NULL)
	{
		next = (struct MSL_CdsQuery*) query->UserObject;

		query->UserObject = NULL;
		MSL_DeallocateCdsQuery(query);
		query = next;
	}
	UGL_Queue_Head = UGL_Queue_Tail = NULL;
	sem_post(&UGL_QueryLock);
}

/*
 *	Gets the query at the queue and removes it from the queue.
 */
struct MSL_CdsQuery* UGL_QueriesGet()
{
	struct MSL_CdsQuery* retVal = NULL;

	sem_wait(&UGL_QueryLock);
	if (UGL_Queue_Head != NULL)
	{
		retVal = UGL_Queue_Head;
		UGL_Queue_Head = (struct MSL_CdsQuery*) UGL_Queue_Head->UserObject;
		if (UGL_Queue_Head == NULL)	{ UGL_Queue_Tail = NULL; }
		retVal->UserObject = NULL;
	}
	sem_post(&UGL_QueryLock);

	return retVal;
}
