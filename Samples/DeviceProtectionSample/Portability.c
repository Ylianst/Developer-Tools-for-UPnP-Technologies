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

#define _WIN32_WINNT 0x0500 // so the code would compile

#include <windows.h>
//#include <Winsock2.h> //Winsock.h for Windows CE
#include <stdlib.h>
#include <malloc.h>

#include "tutrace.h"
#include "WscError.h"
#include "Portability.h"

/*
 * Name        : WscSyncCreate
 * Description : Creates and initializes a lock
 * Arguments   : OUT uint32 **handle - a handle to the synchronization object
 * Return type : uint32
 */
uint32 WscSyncCreate(OUT uint32 **handle)
{
    LPCRITICAL_SECTION cs;

    // TUTRACE((TUTRACE_INFO, "uint32 WscSyncCreate(...\n"));

    if ( !handle )
    {
        TUTRACE((TUTRACE_ERR, "WscSyncCreate: Invalid Handle\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    cs = (LPCRITICAL_SECTION)calloc( 1, sizeof(CRITICAL_SECTION));

    if (!cs)
    {
        TUTRACE((TUTRACE_ERR, "WscSyncCreate: Failed to create a Sync Object,"
                " GetLastError() %X\n", GetLastError()) );
        *handle = 0; //NULL
        return WSC_ERR_OUTOFMEMORY;
    }

    InitializeCriticalSection(cs);
    *handle = (uint32 *)cs;
    // TUTRACE((TUTRACE_INFO,"WscSyncCreate: Sync Object %X Created "
    //            "successfully\n", *handle));

    return WSC_SUCCESS;
}

/*
 * Name        : WscSyncDestroy
 * Description : Deinitializes and destroys a lock
 * Arguments   : IN uint32 *handle - handle to the synchronization object
 * Return type : uint32
 */
uint32 WscSyncDestroy(IN uint32 *handle)
{
    // TUTRACE((TUTRACE_INFO, "WscSyncDestroy(....\n"));

    if ( !handle ) 
    {
        TUTRACE(( TUTRACE_ERR, "WscSyncDestroy: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }
    
    DeleteCriticalSection((LPCRITICAL_SECTION) handle);
    free((void *) handle);
    // TUTRACE(( TUTRACE_INFO, "Handle %X Destroyed Successfully\n", handle));

    return WSC_SUCCESS;
}

/*
 * Name        : WscLock
 * Description : Obtains a lock on the specified synchronization object
 * Arguments   : IN uint32 *handle - handle to the synchronization object
 * Return type : uint32
 */
uint32 WscLock(IN uint32 *handle)
{
    // TUTRACE((TUTRACE_INFO, "WscLock(....\n"));
    if (!handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscLock: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    EnterCriticalSection((LPCRITICAL_SECTION) handle);
    // TUTRACE((TUTRACE_INFO, "Successfully Locked %X\n", handle));
    return WSC_SUCCESS;
}

/*
 * Name        : WscUnlock
 * Description : Releases a lock on the specified synchronization object
 * Arguments   : IN uint32 *handle - handle to the synchronization object
 * Return type : uint32
 */
uint32 WscUnlock(IN uint32 *handle)
{
    // TUTRACE((TUTRACE_INFO, "WscUnlock(....\n"));

    if (!handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscUnlock: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    LeaveCriticalSection((LPCRITICAL_SECTION)handle);
    // TUTRACE((TUTRACE_INFO,"Successfully Unlocked. %X\n",handle));
    return WSC_SUCCESS;
}

/*Thread functions*/
/*
 * Name        : WscCreateThread
 * Description : Creates a thread with the specified parameters and returns the
 *               thread handle
 * Arguments   : OUT uint32 *handle - handle to the thread. MUST be allocated by
 *                                   the caller.
 *               IN void *(*threadFunc)(void *) - pointer to the start routine
 *               IN void *arg - pointer to the arguments to pass to the start
 *                              routine.
 * Return type : uint32
 */
uint32 WscCreateThread(OUT uint32 *handle,
                       IN void *(*threadFunc)(void *),
                       IN void *arg)
{
    DWORD threadId;

    // TUTRACE((TUTRACE_INFO, "WscUnlock(....\n"));

    if (!handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscCreateThread: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    *handle = (uint32 *)CreateThread(NULL, 
                          0,
                          (LPTHREAD_START_ROUTINE)threadFunc,
                          arg,
                          0,
                          &threadId);
    if(*handle == NULL)
    {
        TUTRACE((TUTRACE_ERR, "WscCreateThread: Thread creation failed.\n"));
        return PORTAB_ERR_THREAD;
    }

    return WSC_SUCCESS;
}

/*
 * Name        : WscDestroyThread
 * Description : Destroys a thread. In the current implementation, this 
 *               function only waits for the thread to exit.
 * Arguments   : IN uint32 handle - thread ID.
 * Return type : void
 */
void WscDestroyThread(IN uint32 handle)
{
    uint32 ret;
    // TUTRACE((TUTRACE_INFO, "WscDestroyThread(....\n"));

    if (0 == handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscDestroyThread: Invalid Handle.\n"));
        return;
    }

    ret = WaitForSingleObject((HANDLE)handle, 2000);
    if ( WAIT_TIMEOUT == ret )
    {
        TUTRACE((TUTRACE_INFO, "***** Terminating thread ****\n"));
        TerminateThread(handle, 0);
    }
    else
    {
        TUTRACE((TUTRACE_INFO, "***** Thread died ok ****\n"));
    }
    CloseHandle((HANDLE) handle);
}

uint32 WscCreateEvent(uint32 *handle)
{
    // TUTRACE((TUTRACE_INFO, "WscCreateEvent(....\n"));

    if (!handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscCreateEvent: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    *handle = (uint32 *)CreateEvent(NULL, TRUE, FALSE, NULL);
    if(NULL == *handle)
    {
        TUTRACE((TUTRACE_INFO, "WscCreateEvent: failed.\n"));
        return PORTAB_ERR_EVENT;
    }

    return WSC_SUCCESS;
}

uint32 WscDestroyEvent(uint32 handle)
{
    // TUTRACE((TUTRACE_INFO, "WscDestroyEvent(....\n"));

    if (0 == handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscDestroyEvent: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    CloseHandle((HANDLE) handle);
    return WSC_SUCCESS;
}

uint32 WscSetEvent(uint32 handle)
{
    // TUTRACE((TUTRACE_INFO, "WscSetEvent(....\n"));

    if (0 == handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscSetEvent: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    if(SetEvent((HANDLE) handle) == 0)
    {
        TUTRACE(( TUTRACE_ERR, "WscSetEvent: failed.\n"));
        return PORTAB_ERR_EVENT;
   }

    return WSC_SUCCESS;
}

uint32 WscResetEvent(uint32 handle)
{
    //TUTRACE((TUTRACE_INFO, "WscResetEvent(....\n"));

    if (0 == handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscResetEvent: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    if(ResetEvent((HANDLE) handle) == 0)
    {
        TUTRACE(( TUTRACE_ERR, "WscResetEvent: failed.\n"));
        return PORTAB_ERR_EVENT;
   }

    return WSC_SUCCESS;
}

uint32 WscSingleWait(uint32 handle, uint32 *lock, uint32 timeout)
{
    DWORD ret;
    uint32 err;

    // TUTRACE((TUTRACE_INFO, "WscSingleWait(....\n"));

    if (0 == handle)
    {
        TUTRACE(( TUTRACE_ERR, "WscSingleWait: Invalid Handle.\n"));
        return WSC_ERR_INVALID_PARAMETERS;
    }

    if(0 == timeout)
    {
        ret = WaitForSingleObject((HANDLE) handle, INFINITE);
    }
    else
    {
        ret = WaitForSingleObject((HANDLE) handle, timeout);
    }

    switch(ret)
    {
    case WAIT_ABANDONED:
        err =  PORTAB_ERR_WAIT_ABANDONED;
        break;
    case WAIT_OBJECT_0: //success
        err = WSC_SUCCESS;
        break;
    case WAIT_TIMEOUT:
        err = PORTAB_ERR_WAIT_TIMEOUT;
        break;
    case WAIT_FAILED:
    default:
        err = WSC_ERR_SYSTEM;
    }

    return err;
}

uint32 WscHtonl(uint32 intlong)
{
    return htonl(intlong);
}

uint16 WscHtons(uint16 intshort)
{
    return htons(intshort);
}

uint32 WscNtohl(uint32 intlong)
{
    return ntohl(intlong);
}

uint16 WscNtohs(uint16 intshort)
{
    return ntohs(intshort);
}

void WscSleep(uint32 seconds)
{
    Sleep(seconds*1000);
}