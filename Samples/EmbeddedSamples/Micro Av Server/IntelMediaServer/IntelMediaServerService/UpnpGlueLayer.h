#ifndef _UPNP_GLUE_LAYER_H
#define _UPNP_GLUE_LAYER_H

/*
 *	This module is responsible for initializing the UPnP MediaServer
 *	module and setting up the method callbacks for handling
 *	Browse and Search requests.
 */

#ifdef _POSIX
	#include <semaphore.h>
#endif

#ifdef WIN32
	#include <windows.h>
	#define sem_t HANDLE
#endif

#include "MediaServerLogic.h"

#ifdef WIN32
	#define sem_t HANDLE
	#define sem_init(x,y,z)	*x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x)	(CloseHandle(*x)==0?1:0)
	#define sem_wait(x)		WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x)	((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x)		ReleaseSemaphore(*x,1,NULL)
#endif

#define UGL_SortField_Title			"dc:title"
#define UGL_SortField_TitleLen		8
#define UGL_SortField_Creator		"dc:creator"
#define UGL_SortField_CreatorLen	10
#define UGL_SortField_Album			"upnp:album"
#define UGL_SortField_AlbumLen		10
#define UGL_SortField_Genre			"upnp:genre"
#define UGL_SortField_GenreLen		10

#define UGL_SortCriteriaString	"dc:title,dc:creator,upnp:album,upnp:genre"

#define UGL_SearchField_Title		"dc:title"
#define UGL_SearchField_TitleLen	8
#define UGL_SearchField_Creator		"dc:creator"
#define UGL_SearchField_CreatorLen	10
#define UGL_SearchField_Album		"upnp:album"
#define UGL_SearchField_AlbumLen	10
#define UGL_SearchField_Genre		"upnp:genre"
#define UGL_SearchField_GenreLen	10

#define UGL_SearchCriteriaString	"dc:title,dc:creator,upnp:album,upnp:genre"

/*
 *	Synchronized access to the list of IP addresses for this UPnP device.
 */
extern sem_t UGL_IPAddressLock;
extern int UGL_IPAddressLength;
extern int *UGL_IPAddressList;

/*
 *	Callback executes when a query needs to be answered.
 *	Callback executes on a thread separate from the UPnP stack, so upnp
 *	requests will not be stalled.
 *
 *	The callbackObject will be the one provided in UGL_StartUPnP().
 *
 *	NOTE: cdsQuery->UserObject will be NULL. Be sure to free any memory
 *	(if assigning to that field) before completing the callback.
 */
extern void (*UGL_CallbackOnQuery) (void *callbackObject, struct MSL_CdsQuery *cdsQuery);

/*
 *	Callback executes whenever the stats for the MediaServer change.
 *	Callback may execute on the UPnP thread, so lengthy operations
 *	should not be done as it could stall the UPnP stack.
 *
 *	The callbackObject will be the one provided in UGL_StartUPnP().
 */
extern void (*UGL_CallbackOnStatsChanged) (void *callbackObject, struct MSL_Stats *stats);


/*
 *	Non-blocking call that starts up a MediaServer on a new thread.
 *	Never ever call this method more than once in a row.
 *	UGL_StopUPnP() must be called before calling this method again.
 *
 *	The caller is responsible for freeing the memory for 
 *	callbackObject after UGL_StopUPnP() is called.
 *
 *	Be sure to assign UGL_OnQuery and UGL_OnStatsChanged before calling
 *	UGL_StartUPnP().
 */
void UGL_StartUPnP(void* callbackObject, void *QuitLock);

/*
 *	Returns the webserver token for the microstack.
 */
void* UGL_GetServerToken();

/*
 *	Instructs the MediaServer to stop
 */
void UGL_StopUPnP();

#endif