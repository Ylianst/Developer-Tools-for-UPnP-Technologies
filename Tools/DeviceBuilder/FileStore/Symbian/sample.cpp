#include <e32base.h>
#include <e32cons.h>
#include <e32cmn.h>
#include <badesca.h>

extern "C"
{	//{{{STANDARD_C_APP_BEGIN}}}
	#include "ILibParsers.h"
	#include "ILibWebServer.h"
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	#include "ILibParsers.h"
	//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}
	//{{{STANDARD_C++_APP_END}}}
	//{{{BEGIN_THREADPOOL}}}#include "ILibThreadPool.h"//{{{END_THREADPOOL}}}
	//{{{STANDARD_C_APP_BEGIN}}}
	//{{{MicroStack_Include}}}
	//{{{STANDARD_C_APP_END}}}
	//{{{BEGIN_BAREBONES}}}//{{{INCLUDES}}}//{{{END_BAREBONES}}}
}

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



//{{{STANDARD_C++_APP_BEGIN}}}
//{{{CLASS_DEFINITIONS_DEVICE}}}
//{{{CLASS_IMPLEMENTATIONS_DEVICE}}}

//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}
void {{{PREFIX}}}OnAddSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
{
	printf("Added {{{DEVICE}}}: %s\r\n",device->FriendlyName);
}
void {{{PREFIX}}}OnRemoveSink(CUPnP_Manager *sender, CUPnP_ControlPoint_Device *device)
{
	printf("Removed {{{DEVICE}}}: %s\r\n",device->FriendlyName);
}
//{{{CP_DISCOVER/REMOVE_SINKS_END}}}
//{{{STANDARD_C++_APP_END}}}


//{{{STANDARD_C_APP_BEGIN}}}

//{{{CP_EventSink}}}
//{{{CP_InvokeSink}}}

//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}
/* Called whenever a new device on the correct type is discovered */
void {{{PREFIX}}}DeviceDiscoverSink(struct UPnPDevice *device)
{
	struct UPnPDevice *tempDevice = device;
	struct UPnPService *tempService;

	printf("UPnPDevice Added: %s\r\n", device->FriendlyName);
	
	/* This call will print the device, all embedded devices and service to the console. */
	/* It is just used for debugging. */
	/* 	UPnPPrintUPnPDevice(0,device); */
	
	/* The following subscribes for events on all services */
	while(tempDevice!=NULL)
	{
		tempService = tempDevice->Services;
		while(tempService!=NULL)
		{
			{{{PREFIX}}}SubscribeForUPnPEvents(tempService,NULL);
			tempService = tempService->Next;
		}
		tempDevice = tempDevice->Next;
	}
	
	/* The following will call every method of every service in the device with sample values */
	/* You can cut & paste these lines where needed. The user value is NULL, it can be freely used */
	/* to pass state information. */
	/* The UPnPGetService call can return NULL, a correct application must check this since a device */
	/* can be implemented without some services. */
	
	/* You can check for the existence of an action by calling: UPnPHasAction(serviceStruct,serviceType) */
	/* where serviceStruct is the struct like tempService, and serviceType, is a null terminated string representing */
	/* the service urn. */
	
	//{{{CP_SampleInvoke}}}
}

/* Called whenever a discovered device was removed from the network */
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

void ILib_IPAddressMonitor()
{
	//{{{STANDARD_C_APP_BEGIN}}}
	//{{{IPAddress_Changed}}}
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->IPAddressListChanged();
	//{{{STANDARD_C++_APP_END}}}
}

/**
 * Entry point for application
 * 
 * @return void
 */

//{{{BEGIN_BAREBONES}}}//{{{METHODS}}}//{{{END_BAREBONES}}}

LOCAL_C void StartSampleL()
{
  // create a scheduler for active objects
  CActiveScheduler* pScheduler = new(ELeave) CActiveScheduler;
  CleanupStack::PushL(pScheduler);
  CActiveScheduler::Install(pScheduler);

  // Start the sample code
	//{{{BEGIN_BAREBONES}}}//{{{MAIN_START}}}//{{{END_BAREBONES}}}
	//{{{BEGIN_THREADPOOL}}}
	int x;
	//{{{END_THREADPOOL}}}


	//{{{STANDARD_C_APP_BEGIN}}}
	MicroStackChain = ILibCreateChain();
	
	//{{{CreateControlPoint}}}

	/* TODO: Each device must have a unique device identifier (UDN) */
	//{{{CREATE_MICROSTACK}}}
	//{{{INVOCATION_FP}}}	
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	//{{{STATEVARIABLES_INITIAL_STATE}}}
	//{{{STANDARD_C_APP_END}}}

	//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP = new CUPnP_Manager();
	//{{{DERIVED_CLASS_INSERTION}}}
	//{{{CP_CONNECTING_ADD/REMOVE_SINKS}}}
	//{{{STANDARD_C++_APP_END}}}
	printf("Intel MicroStack 1.0 - {{{INITSTRING}}}\r\n\r\n");

	//{{{BEGIN_THREADPOOL}}}
	ILib_Pool = ILibThreadPool_Create();
	for(x=0;x<!NUMTHREADPOOLTHREADS!;++x)
	{
		//{{{BEGIN_WIN32}}}CreateThread(NULL,0,&ILibPoolThread,NULL,0,&ptid);//{{{END_WIN32}}}
		//{{{BEGIN_POSIX}}}pthread_create(&t,NULL,&ILibPoolThread,NULL);//{{{END_POSIX}}}
	}
	//{{{END_THREADPOOL}}}
	//{{{BEGIN_BAREBONES}}}//{{{INITIALIZATIONS}}}//{{{END_BAREBONES}}}

	
	//{{{STANDARD_C_APP_BEGIN}}}
	//{{{CP_EventRegistrations}}}
	//{{{STANDARD_C_APP_END}}}


	//{{{STANDARD_C_APP_BEGIN}}}
	ILibStartChain(MicroStackChain);
	//{{{STANDARD_C_APP_END}}}
	//{{{STANDARD_C++_APP_BEGIN}}}
	pUPnP->Start();
	delete pUPnP;
	pUPnP = NULL;
	//{{{STANDARD_C++_APP_END}}}

  CActiveScheduler::Start();
  CleanupStack::PopAndDestroy(2);
}

/**
 * EXE's main entry point, set up to execute application.
 * 
 * @return 0 on success, non zero on error
 */
GLDEF_C TInt E32Main()
{
  // start monitoring heap
  __UHEAP_MARK;

  // no automatic cleanup stack, create one
  CTrapCleanup* cleanup = CTrapCleanup::New();

  // call the app entry and trap any errors
  TRAPD(error, StartSampleL());

  // panic on error
  __ASSERT_ALWAYS(!error, User::Panic(_L("dlna unit test fault"), error));

  // finished with cleanup stack
  delete cleanup;

  // finshed monitoring, check for leaks
  __UHEAP_MARKEND;

  return error;
}


