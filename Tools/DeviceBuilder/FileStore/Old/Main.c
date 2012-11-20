/*   
Copyright 2006 - 2010 Intel Corporation

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

#if defined(WIN32)
	#ifndef MICROSTACK_NO_STDAFX
		#include "stdafx.h"
	#endif
	#define _CRTDBG_MAP_ALLOC
	#include <TCHAR.h>
#endif

#if defined(WINSOCK2)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(WINSOCK1)
	#include <winsock.h>
	#include <wininet.h>
#endif
//{{{STANDARD_C_APP_BEGIN}}}
#include "ILibParsers.h"
//{{{MicroStack_Include}}}
#include "ILibWebServer.h"
#include "ILibAsyncSocket.h"
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
#include "UPnPAbstraction.h"
//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
extern "C"
{
	#include "ILibParsers.h"
}
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
//{{{STANDARD_C++_APP_END}}}

//{{{BEGIN_THREADPOOL}}}
#include "ILibThreadPool.h"
//{{{END_THREADPOOL}}}
//{{{BEGIN_THREADPOOL}}}
//{{{BEGIN_POSIX}}}
#include <pthread.h>
//{{{END_POSIX}}}
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{INCLUDES}}}//{{{END_BAREBONES}}}
#if defined(WIN32)
	#include <crtdbg.h>
#endif

//{{{STANDARD_C_APP_BEGIN}}}
void *MicroStackChain;
//{{{MICROSTACK_VARIABLE}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
CUPnP_Manager *pUPnP;
//{{{STANDARD_C++_APP_END}}}

//{{{BEGIN_THREADPOOL}}}
void *ILib_Pool;
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{GLOBALS}}}//{{{END_BAREBONES}}}
//{{{BEGIN_POSIX}}}
int WaitForExit = 0;
//{{{END_POSIX}}}
//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
void *ILib_Monitor;
int ILib_IPAddressLength;
int *ILib_IPAddressList;
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}
HANDLE ILib_IPAddressMonitorTerminator;
HANDLE ILib_IPAddressMonitorThread;
DWORD ILib_MonitorSocketReserved;
WSAOVERLAPPED ILib_MonitorSocketStateObject;
SOCKET ILib_MonitorSocket;
//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}
//{{{STANDARD_C++_APP_BEGIN}}}
//{{{CLASS_DEFINITIONS_DEVICE}}}
//{{{CLASS_IMPLEMENTATIONS_DEVICE}}}
//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}
void {{{PREFIX}}}OnAddSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
{
	printf("Added {{{DEVICE}}}: %s\r\n", device->FriendlyName);
}
void {{{PREFIX}}}OnRemoveSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
{
	printf("Removed {{{DEVICE}}}: %s\r\n", device->FriendlyName);
}
//{{{CP_DISCOVER/REMOVE_SINKS_END}}}
//{{{STANDARD_C++_APP_END}}}
//{{{STANDARD_C_APP_BEGIN}}}
//{{{CP_EventSink}}}
//{{{CP_InvokeSink}}}
//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}
// Called whenever a new device on the correct type is discovered
void {{{PREFIX}}}DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;

	printf("UPnPDevice Added: %s\r\n", device->FriendlyName);
	
	// This call will print the device, all embedded devices and service to the console. It is just used for debugging.
	// UPnPPrintUPnPDevice(0, device);
	
	// The following subscribes for events on all services
	while(tempDevice != NULL)
	{
		tempService = tempDevice->Services;
		while(tempService != NULL)
		{
			{{{PREFIX}}}SubscribeForUPnPEvents(tempService, NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	// The following will call every method of every service in the device with sample values
	// You can cut & paste these lines where needed. The user value is NULL, it can be freely used
	// to pass state information.
	// The UPnPGetService call can return NULL, a correct application must check this since a device
	// can be implemented without some services.

	// You can check for the existence of an action by calling: UPnPHasAction(serviceStruct,serviceType)
	// where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing
	// the service urn.
//{{{CP_SampleInvoke}}}
}

// Called whenever a discovered device was removed from the network
void {{{PREFIX}}}DeviceRemoveSink(struct UPnPDevice *device)
{
	printf("UPnPDevice Removed: %s\r\n", device->FriendlyName);
}
//{{{END_CP_DISCOVER_REMOVE_SINK}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C_APP_BEGIN}}}
//{{{DEVICE_INVOCATION_DISPATCH}}}
//{{{PresentationRequest}}}
//{{{STANDARD_C_APP_END}}}
//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
void ILib_IPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	UNREFERENCED_PARAMETER( data );

	length = ILibGetLocalIPAddressList(&list);
	if (length != ILib_IPAddressLength || memcmp((void*)list, (void*)ILib_IPAddressList, sizeof(int)*length) != 0)
	{
//{{{STANDARD_C_APP_BEGIN}}}
//{{{IPAddress_Changed}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
		pUPnP->IPAddressListChanged();
//{{{STANDARD_C++_APP_END}}}
		free(ILib_IPAddressList);
		ILib_IPAddressList = list;
		ILib_IPAddressLength = length;
	}
	else
	{
		free(list);
	}
	ILibLifeTime_Add(ILib_Monitor, NULL, 4, (void*)&ILib_IPAddressMonitor, NULL);
}
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}

void CALLBACK ILib_IPAddressMonitor(
IN DWORD dwError, 
IN DWORD cbTransferred, 
IN LPWSAOVERLAPPED lpOverlapped, 
IN DWORD dwFlags 
)
{
	UNREFERENCED_PARAMETER( dwError );
	UNREFERENCED_PARAMETER( cbTransferred );
	UNREFERENCED_PARAMETER( lpOverlapped );
	UNREFERENCED_PARAMETER( dwFlags );
//{{{STANDARD_C_APP_BEGIN}}}
//{{{IPAddress_Changed}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->IPAddressListChanged();
//{{{STANDARD_C++_APP_END}}}
	WSAIoctl(ILib_MonitorSocket, SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0, &ILib_MonitorSocketReserved, &ILib_MonitorSocketStateObject, &ILib_IPAddressMonitor);
}

DWORD WINAPI ILib_IPAddressMonitorLoop(LPVOID args)
{
	UNREFERENCED_PARAMETER( args );

	ILib_MonitorSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(ILib_MonitorSocket, SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0, &ILib_MonitorSocketReserved, &ILib_MonitorSocketStateObject, &ILib_IPAddressMonitor);
	while(WaitForSingleObjectEx(ILib_IPAddressMonitorTerminator, INFINITE, TRUE) != WAIT_OBJECT_0);
	return 0;
}
//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}

//{{{BEGIN_POSIX}}}
void ILib_LinuxQuit(void *data)
{
	UNREFERENCED_PARAMETER( data );

//{{{BEGIN_THREADPOOL}}}
	if(ILib_Pool != NULL)
	{
		printf("Stopping Thread Pool...\r\n");
		ILibThreadPool_Destroy(ILib_Pool);
		printf("Thread Pool Destroyed...\r\n");
	}
//{{{END_THREADPOOL}}}
//{{{STANDARD_C_APP_BEGIN}}}
	if(MicroStackChain != NULL)
	{
		ILibStopChain(MicroStackChain);
		MicroStackChain = NULL;
	}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
	if(pUPnP != NULL) pUPnP->Stop();
//{{{STANDARD_C++_APP_END}}}
}
void BreakSink(int s)
{
	if(WaitForExit == 0)
	{
		ILibLifeTime_Add(ILib_Monitor, NULL, 0, (void*)&ILib_LinuxQuit, NULL);
		WaitForExit = 1;
	}
}
//{{{END_POSIX}}}
//{{{BEGIN_WIN32}}}
DWORD WINAPI Run(LPVOID args)
{
	UNREFERENCED_PARAMETER( args );
	
	getchar();
//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}
	closesocket(ILib_MonitorSocket);
	SetEvent(ILib_IPAddressMonitorTerminator);
	WaitForSingleObject(ILib_IPAddressMonitorThread, INFINITE);
	CloseHandle(ILib_IPAddressMonitorTerminator);
//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}
//{{{STANDARD_C_APP_BEGIN}}}
	ILibStopChain(MicroStackChain);
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->Stop();
//{{{STANDARD_C++_APP_END}}}
	return 0;
}
//{{{END_WIN32}}}

//{{{BEGIN_THREADPOOL}}}
//{{{BEGIN_WIN32}}}DWORD WINAPI //{{{END_WIN32}}}//{{{BEGIN_POSIX}}}void* //{{{END_POSIX}}}ILibPoolThread(void *args)
{
	ILibThreadPool_AddThread(ILib_Pool);
	return(0);
}
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{METHODS}}}//{{{END_BAREBONES}}}
//{{{BEGIN_WIN32}}}int _tmain(int argc, _TCHAR* argv[])//{{{END_WIN32}}}//{{{BEGIN_POSIX}}}int main(void)//{{{END_POSIX}}} 
{
//{{{BEGIN_BAREBONES}}}//{{{MAIN_START}}}//{{{END_BAREBONES}}}
//{{{BEGIN_THREADPOOL}}}
	int x;
//{{{END_THREADPOOL}}}
//{{{BEGIN_POSIX}}}
	struct sigaction setup_action;
    sigset_t block_mask;
//{{{BEGIN_THREADPOOL}}}pthread_t t;//{{{END_THREADPOOL}}}
//{{{END_POSIX}}}     
//{{{BEGIN_WIN32}}}
	DWORD ptid = 0;
	DWORD ptid2 = 0;
	UNREFERENCED_PARAMETER( argc );
	UNREFERENCED_PARAMETER( argv );

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//{{{END_WIN32}}}
//{{{STANDARD_C_APP_BEGIN}}}
	MicroStackChain = ILibCreateChain();
//{{{CreateControlPoint}}}
//{{{CREATE_MICROSTACK}}}
//{{{INVOCATION_FP}}}	
//{{{STATEVARIABLES_INITIAL_STATE}}}
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP = new CUPnP_Manager();
//{{{DERIVED_CLASS_INSERTION}}}
//{{{CP_CONNECTING_ADD/REMOVE_SINKS}}}
//{{{STANDARD_C++_APP_END}}}
	printf("MicroStack 1.0 - {{{INITSTRING}}}\r\n\r\n");
//{{{BEGIN_THREADPOOL}}}
	ILib_Pool = ILibThreadPool_Create();
	for(x = 0; x < !NUMTHREADPOOLTHREADS!; ++x)
	{
		//{{{BEGIN_WIN32}}}CreateThread(NULL, 0 ,&ILibPoolThread, NULL, 0, &ptid);//{{{END_WIN32}}}
		//{{{BEGIN_POSIX}}}pthread_create(&t, NULL, &ILibPoolThread, NULL);//{{{END_POSIX}}}
	}
//{{{END_THREADPOOL}}}
//{{{BEGIN_BAREBONES}}}//{{{INITIALIZATIONS}}}//{{{END_BAREBONES}}}
//{{{BEGIN_WIN32}}}
	CreateThread(NULL, 0, &Run, NULL, 0, &ptid);
//{{{END_WIN32}}}
//{{{STANDARD_C_APP_BEGIN}}}
//{{{CP_EventRegistrations}}}
//{{{STANDARD_C_APP_END}}}
//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
//{{{STANDARD_C_APP_BEGIN}}}ILib_Monitor = ILibCreateLifeTime(MicroStackChain);//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}ILib_Monitor = ILibCreateLifeTime(pUPnP->GetChain());//{{{STANDARD_C++_APP_END}}}
	ILib_IPAddressLength = ILibGetLocalIPAddressList(&ILib_IPAddressList);
	ILibLifeTime_Add(ILib_Monitor, NULL, 4, (void*)&ILib_IPAddressMonitor, NULL);
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}
	ILib_IPAddressMonitorTerminator = CreateEvent(NULL, TRUE, FALSE, NULL);
	ILib_IPAddressMonitorThread = CreateThread(NULL, 0, &ILib_IPAddressMonitorLoop, NULL, 0, &ptid2);
//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}
//{{{BEGIN_POSIX}}}
	sigemptyset(&block_mask);
	// Block other terminal-generated signals while handler runs.
    sigaddset(&block_mask, SIGINT);
    sigaddset(&block_mask, SIGQUIT);
    setup_action.sa_handler = BreakSink;
    setup_action.sa_mask = block_mask;
    setup_action.sa_flags = 0;
    sigaction(SIGINT, &setup_action, NULL);
	WaitForExit = 0;
//{{{END_POSIX}}}
//{{{STANDARD_C_APP_BEGIN}}}
	ILibStartChain(MicroStackChain);
//{{{STANDARD_C_APP_END}}}
//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->Start();
	delete pUPnP;
	pUPnP = NULL;
//{{{STANDARD_C++_APP_END}}}
//{{{BEGIN_WIN32}}}
//{{{BEGIN_THREADPOOL}}}
	if(ILib_Pool != NULL)
	{
		printf("Stopping Thread Pool...\r\n");
		ILibThreadPool_Destroy(ILib_Pool);
		printf("Thread Pool Destroyed...\r\n");
	}
//{{{END_THREADPOOL}}}
//{{{END_WIN32}}}
//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	free(ILib_IPAddressList);
//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	return 0;
}

