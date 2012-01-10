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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UpnpMicroStack.h"
#include "ILibParsers.h"
#include "MediaServerLogic.h"
#include "MyString.h"
#include "MimeTypes.h"
#include "CdsStrings.h"
#include <assert.h>

#ifdef UNDER_CE
#define strnicmp _strnicmp
#define assert ASSERT
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#endif

#ifdef WIN32
#ifndef UNDER_CE
#include "assert.h"
#include <crtdbg.h>
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#endif
#endif

#ifdef _POSIX
#include "assert.h"
#define strnicmp strncasecmp
#endif

/*
#ifdef _DEBUG
#define MSL_MALLOC msl_malloc
#define MSL_FREE msl_free
#define ASSERT(x) assert(x)
#else
*/
#define MSL_MALLOC malloc
#define MSL_FREE free
#define ASSERT(x)
//#endif

int msl_malloc_counter = 0;

void (*MSL_Callback_OnQuery) (struct MSL_CdsQuery *browseArgs) = NULL;
void (*MSL_OnStatsChanged) (struct MSL_Stats *stats) = NULL;

/*
void* msl_malloc(int sz)
{
	++msl_malloc_counter;
	return((void*)malloc(sz));
}
void msl_free(void* ptr)
{
	--msl_malloc_counter;
	free(ptr);	
}
*/


/*
 *	BrowseFlag related error codes - this is a UPnP layer error code,
 *	and not really an AV error code.
 */
#define MSL_ERROR_CODE_INVALID_BROWSEFLAG	402
#define MSL_ERROR_MSG_INVALID_BROWSEFLAG	"Invalid value specified for BrowseFlag."

#define MSL_ERROR_CODE_INTERNAL				500
#define	MSL_ERROR_MSG_INTERNAL				"Unknown or internal error encountered."


struct MSL_ContainerUpdate
{
	char *ContainerID;
	unsigned int UpdateID;
	struct MSL_ContainerUpdate *Next;
};

struct MSL_MediaServerObject
{
	/*
	 *	The PreSelect, PostSelect, and Destroy fields must
	 *	remain here as the object needs to be compatible with the
	 *	thread-chaining framework.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	/*
	 *	Pointer to generated upnp microstack object.
	 */
	void*				UpnpStack;

	/*
	 *	Lock for thread-safety.
	 */
	sem_t				Lock;

	/*
	 *	SystemUpdateID value of the media server.
	 */
	unsigned int		SystemUpdateID;

	/*
	 *	Linked list of container update IDs to print.
	 */
	struct MSL_ContainerUpdate *ContainerUpdateID;

	/*
	 *	Moderation flag for eventing.
	 */
	unsigned short				ModerationFlag;

	/* lifetime monitor object used in moderation */
	void* LifeTimeMonitor;


	/*
	 *	Supported abilities for sorting browse/search results.
	 *	SortCapabilitiesString:		concatenation of strings in MSL_CONFIG_SORT_CAPABILITIES
	 */
	char	*SortCapabilitiesString;

	/*
	 *	Supported abilities for searching results.
	 *	SearchCapabilitiesString:	concatenation of strings in MSL_CONFIG_SEARCH_CAPABILITIES
	 */
	char	*SearchCapabilitiesString;

	/*
	 *	Contains the list of IP addresses for the media server.
	 */
	int		*IpAddrList;
	int		IpAddrListLen;

	/*
	 *	The list of protocolInfo for this device as a source.
	 */
	char	*SourceProtocolInfo;

	/*
	 *	The list of protocolInfo for this device as a sink.
	 */
	char	*SinkProtocolInfo;
};

/*
 *	Because we use hard extern callbacks, instead of
 *	function pointers, we need to keep a globally, static
 *	variable for the mediaserver.
 */
static struct MSL_MediaServerObject *MSL_TheMslObj = NULL;




/************************************************************************************/
/* START SECTION - helper functions */

/*
 *	This method release the resources occupied
 *		mslObj					: The object returned from CreateMediaServer
 */
void MSL_DestroyMediaServer(void *mslObj)
{
	struct MSL_MediaServerObject *msl;
	struct MSL_ContainerUpdate *cu, *nextCu;

	ASSERT(mslObj);

	msl = (struct MSL_MediaServerObject*) mslObj;

	/* properly deallocate the objects for the container UpdateIDs */
	sem_wait(&(msl->Lock));

	cu = msl->ContainerUpdateID;
	while (cu != NULL)
	{
		nextCu = cu->Next;
		free(cu);
		cu = nextCu;
	}

	MSL_FREE(msl->SearchCapabilitiesString);
	MSL_FREE(msl->SortCapabilitiesString);

	sem_post(&(msl->Lock));

	/* Framework will free the data at 'mslObj' */
	MSL_TheMslObj = NULL;
	sem_destroy(&(msl->Lock));
	/*free(mslObj);*/
}


void MSL_Helper_PopulateIpInfo(struct MSL_MediaServerObject *msl, struct MSL_CdsQuery *cdsQuery)
{
	int size, i, swapValue;

	/*
	 *	Obtain the IP address and port that received this request
	 */
	cdsQuery->RequestedOnAddress = UpnpGetLocalInterfaceToHost(cdsQuery->UpnpToken);
	cdsQuery->RequestedOnPort = UpnpGetLocalPortNumber(cdsQuery->UpnpToken);

	/*
	 *	Obtain the list of active IP addresses for this machine.
 	 *	Microstack allows us to assume that the port number
	 *	will be the same for all IP addresses.
	 */
	sem_wait(&(msl->Lock));
	cdsQuery->IpAddrListLen = msl->IpAddrListLen;
	size = (int) (sizeof(int) * cdsQuery->IpAddrListLen);
	cdsQuery->IpAddrList = (int*) malloc(size);
	memcpy(cdsQuery->IpAddrList, msl->IpAddrList, size);
	sem_post(&(msl->Lock));

	/*
	 *	Reorder the list of active IP addresses so that the
	 *	IP address for the interface that received the request
	 *	is listed first.
	 */
	if (cdsQuery->IpAddrList[0] != cdsQuery->RequestedOnAddress)
	{
		swapValue = cdsQuery->IpAddrList[0];
		cdsQuery->IpAddrList[0] = cdsQuery->RequestedOnAddress;
		for (i=1; i < cdsQuery->IpAddrListLen; i++)
		{
			if (cdsQuery->IpAddrList[i] == cdsQuery->RequestedOnAddress)
			{
				cdsQuery->IpAddrList[i] = swapValue;
				break;
			}
		}
	}
}

/* number of bytes needed to represent a 32bit unsigned int as a string with a comma. */
#define MSL_MAX_BYTESIZE_UINT 13
void MSL_Helper_UpdateImmediate_ContainerUpdateID(struct MSL_MediaServerObject *msl)
{
	int size;
	struct MSL_ContainerUpdate *cu, *nextCu;
	char *sv, *var;
	int writecomma;

	/* don't bother updating if there's nothing to report */
	if (msl->ContainerUpdateID == NULL) return;

	size = 0;
	sem_wait(&(msl->Lock));

	/* calculate size needed for state variable value */
	cu = msl->ContainerUpdateID;
	while (cu != NULL)
	{
		size += (int) (MSL_MAX_BYTESIZE_UINT + strlen(cu->ContainerID));
	}
	size++;

	/*
	 *	Acquire the value of the state variable by writing it to 'var.
	 *
	 *	We progressively write 'var' by writing to 'sv'. The format
	 *	of the state variable is a comma-delimited list of
	 *	containerID/UpdateID pairs... of course, the spec authors
	 *	also made the delimiter between ContainerID and UpdateID
	 *	into a comma... how silly.
	 */
	cu = msl->ContainerUpdateID;
	var = sv = (char*) MSL_MALLOC(size);
	writecomma = 0;
	while (cu != NULL)
	{
		nextCu = cu->Next;

		if (writecomma != 0) { sv += sprintf(sv, ","); }
		sv += sprintf(sv, "%s,%u", cu->ContainerID, cu->UpdateID);

		writecomma = 1;
		free(cu);
		cu = nextCu;
	}
	msl->ContainerUpdateID = NULL;

	ASSERT(sv <= var+size);
	MSL_FREE(var);

	sem_post(&(msl->Lock));
	
}

void MSL_Helper_ModerationSink_ContainerUpdateID(void* data)
{
	struct MSL_MediaServerObject *msl;

	#ifdef _DEBUG
	printf("MSL_Helper_ModerationSink_ContainerUpdateID()\r\n");
	#endif

	ASSERT(data != NULL);

	msl = (struct MSL_MediaServerObject*) data;

	sem_wait(&(msl->Lock));

	MSL_Helper_UpdateImmediate_ContainerUpdateID(msl);
	msl->ModerationFlag = 0;

	sem_post(&(msl->Lock));
}

/*
 *	Assumes caller has locked msl->Lock.
 */
void MSL_Helper_UpdateModerated_ContainerUpdateID(struct MSL_MediaServerObject *msl)
{
	/* don't bother updating if there's nothing to report */
	if (msl->ContainerUpdateID == NULL) return;

	if (msl->ModerationFlag == 0)
	{
		msl->ModerationFlag = 1;
		MSL_Helper_UpdateImmediate_ContainerUpdateID(msl);
		ILibLifeTime_Add(msl->LifeTimeMonitor,msl,1,MSL_Helper_ModerationSink_ContainerUpdateID, NULL);
	}
}

/* END SECTION - helper functions */
/************************************************************************************/



/************************************************************************************/
/* START SECTION - Dispatch sinks generated in original main.c */

void UpnpContentDirectory_GetSearchCapabilities(void* upnptoken)
{
	/*
	 *	Reports the statically defined search capabilities of the MediaServer.
	 *	You can customize this value to the abilities of the backend database
	 *	by changing MSL_CONFIG_SEARCH_CAPABILITIES_xxx variables.
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpContentDirectory_GetSearchCapabilities();\r\n");
	#endif

	UpnpResponse_ContentDirectory_GetSearchCapabilities(upnptoken, MSL_TheMslObj->SearchCapabilitiesString);
}

void UpnpContentDirectory_GetSortCapabilities(void* upnptoken)
{
	/*
	 *	Reports the statically defined sort capabilities of the MediaServer.
	 *	You can customize this value to the abilities of the backend database
	 *	by changing MSL_CONFIG_SORT_CAPABILITIES_xxx variables.
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpContentDirectory_GetSortCapabilities();\r\n");
	#endif

	UpnpResponse_ContentDirectory_GetSortCapabilities(upnptoken, MSL_TheMslObj->SortCapabilitiesString);
}

void UpnpContentDirectory_GetSystemUpdateID(void* upnptoken)
{
	/*
	 *	Reports the known SystemUpdateID.
	 *	The SystemUpdateID is changed through MSL_IncrementSystemUpdateID().
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpContentDirectory_GetSystemUpdateID();\r\n");
	#endif

	UpnpResponse_ContentDirectory_GetSystemUpdateID(upnptoken, MSL_TheMslObj->SystemUpdateID);
}

void UpnpConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	/*
	 *	HTTP connections are stateless, from the perspective of UPnP AV.
	 *	This is largely because we can't really monitor connection lifetime
	 *	of HTTP traffic in the UPnP AV sense, without risking memory leaks.
	 *
	 *	TODO: Low priority - Add support for connection-lifetime aware protocols.
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpConnectionManager_GetCurrentConnectionInfo(%u);\r\n",ConnectionID);
	#endif

	UpnpResponse_Error(upnptoken, (int) MSL_Error_ConnectionDoesNotExist, MSL_ErrorMsg_ConnectionDoesNotExist);
}

void UpnpConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	/*
	 *	HTTP connections are stateless, from the perspective of UPnP AV.
	 *	This is largely because we can't really monitor connection lifetime
	 *	of HTTP traffic in the UPnP AV sense, without risking memory leaks.
	 *
	 *	TODO: Low priority - Add support for connection-lifetime aware protocols.
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpConnectionManager_GetCurrentConnectionIDs();\r\n");
	#endif

	UpnpResponse_ConnectionManager_GetCurrentConnectionIDs(upnptoken, "");
}

void UpnpConnectionManager_GetProtocolInfo(void* upnptoken)
{
	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpConnectionManager_GetProtocolInfo();\r\n");
	#endif

	sem_wait(&(MSL_TheMslObj->Lock));
	sem_post(&(MSL_TheMslObj->Lock));

	UpnpResponse_ConnectionManager_GetProtocolInfo(upnptoken, "", "");
}



void UpnpContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	struct MSL_CdsQuery *browseArgs;
	int errorCode;
	const char *errorMsg;
	enum MSL_Enum_QueryTypes browseChildren;
	int size;

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpContentDirectory_Browse();\r\n");
	#endif

	/*
	 *	Validate arguments.
	 */

	errorCode = 0;
	errorMsg = NULL;
	if (stricmp(BrowseFlag, CDS_STRING_BROWSE_DIRECT_CHILDREN) == 0)
	{
		browseChildren = MSL_Query_BrowseDirectChildren;
	}
	else if (stricmp(BrowseFlag, CDS_STRING_BROWSE_METADATA) == 0)
	{
		browseChildren = MSL_Query_BrowseMetadata;
	}
	else
	{
		fprintf(stderr, "WARNING: UpnpContentDirectory_Browse(): Possible error with generated microstack. Encountered BrowseFlag='%s'\r\n", BrowseFlag);
		errorCode = MSL_ERROR_CODE_INVALID_BROWSEFLAG;
		errorMsg = MSL_ERROR_MSG_INVALID_BROWSEFLAG;
	}

	if ((errorCode != 0) || (errorMsg != NULL))
	{
		/* ensure that the error code and message map to something */
		if (errorCode == 0) { errorCode = MSL_ERROR_CODE_INTERNAL; }
		if (errorMsg == NULL) {	errorMsg = MSL_ERROR_MSG_INTERNAL; }

		UpnpResponse_Error(upnptoken, errorCode, errorMsg);
	}
	else
	{
		/*
		 *	Input arguments valid at UPnP layer.
		 *	Create an MSL_CdsQuery object and execute
		 *	the browse callback so that application can return results.
		 */

		browseArgs = (struct MSL_CdsQuery*) MSL_MALLOC (sizeof(struct MSL_CdsQuery));
		memset(browseArgs, 0, sizeof(struct MSL_CdsQuery));

		browseArgs->QueryType = browseChildren;
		
		size = (int) strlen(Filter)+1;
		browseArgs->Filter = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->Filter, Filter, size);

		browseArgs->MediaServerObject = MSL_TheMslObj;
		
		size = (int) strlen(ObjectID)+1;
		browseArgs->ObjectID = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->ObjectID, ObjectID, size);
		/* Be sure to unescape it first. */
		ILibInPlaceXmlUnEscape(browseArgs->ObjectID);

		browseArgs->RequestedCount = RequestedCount;

		size = (int) strlen(SortCriteria)+1;
		browseArgs->SortCriteria = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->SortCriteria, SortCriteria, size);

		browseArgs->StartingIndex = StartingIndex;

		browseArgs->UpnpToken = upnptoken;
		browseArgs->UserObject = NULL;

		browseArgs->IpAddrList = NULL;
		MSL_Helper_PopulateIpInfo(MSL_TheMslObj, browseArgs);

		if (MSL_Callback_OnQuery != NULL)
		{
			MSL_Callback_OnQuery(browseArgs);
		}
	}
}

void UpnpContentDirectory_Search(void* upnptoken,char* ContainerID,char* SearchCriteria, char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	struct MSL_CdsQuery *searchArgs;
	int size;

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpContentDirectory_Search();\r\n");
	#endif

	/*
	 *	Validate arguments.
	 */

	/*
	 *	Input arguments valid at UPnP layer.
	 *	Create an MSL_CdsQuery object and execute
	 *	the browse callback so that application can return results.
	 */

	searchArgs = (struct MSL_CdsQuery*) MSL_MALLOC (sizeof(struct MSL_CdsQuery));
	memset(searchArgs, 0, sizeof(struct MSL_CdsQuery));

	searchArgs->QueryType = MSL_Query_Search;
	
	size = (int) strlen(Filter)+1;
	searchArgs->Filter = (char*) MSL_MALLOC(size);
	memcpy(searchArgs->Filter, Filter, size);

	searchArgs->MediaServerObject = MSL_TheMslObj;
		
	size = (int) strlen(ContainerID)+1;
	searchArgs->ObjectID = (char*) MSL_MALLOC(size);
	memcpy(searchArgs->ObjectID, ContainerID, size);

	size = (int) strlen(SearchCriteria)+1;
	searchArgs->SearchCriteria = (char*) MSL_MALLOC(size);
	memcpy(searchArgs->SearchCriteria, SearchCriteria, size);

	searchArgs->RequestedCount = RequestedCount;

	size = (int) strlen(SortCriteria)+1;
	searchArgs->SortCriteria = (char*) MSL_MALLOC(size);
	memcpy(searchArgs->SortCriteria, SortCriteria, size);

	searchArgs->StartingIndex = StartingIndex;

	searchArgs->UpnpToken = upnptoken;
	searchArgs->UserObject = NULL;

	MSL_Helper_PopulateIpInfo(MSL_TheMslObj, searchArgs);

	if (MSL_Callback_OnQuery != NULL)
	{
		MSL_Callback_OnQuery(searchArgs);
	}
}


/* END SECTION - Dispatch sinks generated in original main.c */
/************************************************************************************/






/************************************************************************************/
/* START SECTION - public methods*/


/* see header file */
void *MSL_CreateMediaServer(void *chain, void *upnpStack, void* lifetimeMonitor, const char *sinkProtocolInfo, const char *sourceProtocolInfo, const char *sortFields, const char *searchFields)
{
	struct MSL_MediaServerObject *mslObj;
	int size;

	mslObj = NULL;
	if (MSL_TheMslObj == NULL)
	{
		mslObj = MSL_TheMslObj = (struct MSL_MediaServerObject*) MSL_MALLOC(sizeof(struct MSL_MediaServerObject));
		memset(mslObj, 0, sizeof(struct MSL_MediaServerObject));
		
		mslObj->UpnpStack = upnpStack;
		mslObj->Destroy = MSL_DestroyMediaServer;
		mslObj->LifeTimeMonitor = lifetimeMonitor;
		sem_init(&(mslObj->Lock), 0, 1);

		UpnpSetState_ContentDirectory_SystemUpdateID(upnpStack, mslObj->SystemUpdateID);
		UpnpSetState_ContentDirectory_ContainerUpdateIDs(upnpStack, "");

		/* set initial sourceProtocolInfo */
		size = (int) strlen(sourceProtocolInfo)+1;
		mslObj->SourceProtocolInfo = (char*) malloc(size);
		memcpy(mslObj->SourceProtocolInfo, sourceProtocolInfo, size);
		UpnpSetState_ConnectionManager_SourceProtocolInfo(upnpStack, mslObj->SourceProtocolInfo);

		/* set initial sinkProtocolInfo */
		size = (int) strlen(sinkProtocolInfo)+1;
		mslObj->SinkProtocolInfo = (char*) malloc(size);
		memcpy(mslObj->SinkProtocolInfo, sinkProtocolInfo, size);
		UpnpSetState_ConnectionManager_SinkProtocolInfo(upnpStack,mslObj->SinkProtocolInfo);

		/* no connections */
		UpnpSetState_ConnectionManager_CurrentConnectionIDs(upnpStack, "");

		/* set sort capabilities */
		size = (int) strlen(sortFields)+1;
		MSL_TheMslObj->SortCapabilitiesString = (char*) MSL_MALLOC(size);
		memcpy(MSL_TheMslObj->SortCapabilitiesString , sortFields, size);

		/* set search cabilities */
		size = (int) strlen(searchFields)+1;
		MSL_TheMslObj->SearchCapabilitiesString = (char*) MSL_MALLOC(size);
		memcpy(MSL_TheMslObj->SearchCapabilitiesString, searchFields, size);

		ILibAddToChain(chain, mslObj);
	}

	return mslObj;
}

/* see header file */
void MSL_DeallocateCdsQuery(struct MSL_CdsQuery *cdsQuery)
{
	if (cdsQuery->Filter != NULL) MSL_FREE (cdsQuery->Filter);
	if (cdsQuery->ObjectID != NULL) MSL_FREE (cdsQuery->ObjectID);
	if (cdsQuery->SortCriteria != NULL) MSL_FREE (cdsQuery->SortCriteria);
	if (cdsQuery->SearchCriteria != NULL) MSL_FREE (cdsQuery->SearchCriteria);
	if (cdsQuery->IpAddrList != NULL) MSL_FREE (cdsQuery->IpAddrList);
	MSL_FREE (cdsQuery);
}

/* see header file */
void MSL_ForResponse_RespondError(struct MSL_CdsQuery *cdsQuery, int errorCode, const char *errorMsg)
{
	ASSERT(cdsQuery != NULL);

	UpnpResponse_Error(cdsQuery->UpnpToken, errorCode, errorMsg);
}

/* see header file */
void MSL_ForQueryResponse_Start(struct MSL_CdsQuery *cdsQuery, int sendDidlHeader)
{
	ASSERT(cdsQuery != NULL);

	if (cdsQuery->QueryType == MSL_Query_Search)
	{
		UpnpAsyncResponse_START(cdsQuery->UpnpToken, CDS_STRING_SEARCH, CDS_STRING_URN_CDS);
	}
	else
	{
		UpnpAsyncResponse_START(cdsQuery->UpnpToken, CDS_STRING_BROWSE, CDS_STRING_URN_CDS);
	}

	if (sendDidlHeader != 0)
	{
		UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_RESULT, CDS_DIDL_HEADER_ESCAPED, CDS_DIDL_HEADER_ESCAPED_LEN, 1, 0);
	}
	else
	{
		UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_RESULT, "", 0, 1, 0);
	}
}

/* see header file */
void MSL_ForQueryResponse_ResultArgument(struct MSL_CdsQuery *cdsQuery, const char *xmlEscapedUtf8Didl, int didlSize)
{
	ASSERT(cdsQuery != NULL);

	UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_RESULT, xmlEscapedUtf8Didl, didlSize, 0, 0);
}

/* see header file */
void MSL_ForQueryResponse_FinishResponse(struct MSL_CdsQuery *cdsQuery, int sendDidlFooter, unsigned int numberReturned, unsigned int totalMatches, unsigned int updateID)
{
	char numResult[30];

	ASSERT(cdsQuery != NULL);

	if (sendDidlFooter != 0)
	{
		UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_RESULT, CDS_DIDL_FOOTER_ESCAPED, CDS_DIDL_FOOTER_ESCAPED_LEN, 0, 1);
	}
	else
	{
		UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_RESULT, "", 0, 0, 1);
	}

	/*
	 *	Instruct the generated microstack to send the data for the last
	 *	three out-arguments of the Browse request.
	 */

	sprintf(numResult, "%u", numberReturned);
	UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_NUMBER_RETURNED, numResult, (int) strlen(numResult), 1,1);

	sprintf(numResult, "%u", totalMatches);
	UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_TOTAL_MATCHES, numResult, (int) strlen(numResult), 1,1);

	sprintf(numResult, "%u", updateID);
	UpnpAsyncResponse_OUT(cdsQuery->UpnpToken, CDS_STRING_UPDATE_ID, numResult, (int) strlen(numResult), 1,1);

	if (cdsQuery->QueryType == MSL_Query_Search)
	{
		UpnpAsyncResponse_DONE(cdsQuery->UpnpToken, CDS_STRING_SEARCH);
	}
	else
	{
		UpnpAsyncResponse_DONE(cdsQuery->UpnpToken, CDS_STRING_BROWSE);
	}
}

/* see header file */
void MSL_IncrementSystemUpdateID(void *mslObj)
{
	struct MSL_MediaServerObject *msl;

	ASSERT(mslObj != NULL);

	msl = (struct MSL_MediaServerObject*) mslObj;
	msl->SystemUpdateID++;
	UpnpSetState_ContentDirectory_SystemUpdateID(msl->UpnpStack, msl->SystemUpdateID);
}

/* see header file */
void MSL_UpdateContainerID(void *mslObj, const char *containerID, unsigned int containerUpdateID)
{
	struct MSL_MediaServerObject *msl;
	struct MSL_ContainerUpdate *cu;
	struct MSL_ContainerUpdate *fcu;
	struct MSL_ContainerUpdate *lcu;
	int size;

	ASSERT(mslObj != NULL);

	msl = (struct MSL_MediaServerObject*) mslObj;

	/* lock state */
	sem_wait(&(msl->Lock));

	/*
	 *	Attempt to find an existing ContainerUpdate
	 *	object for the specified containerID.
	 */
	cu = msl->ContainerUpdateID;
	lcu = fcu = NULL;
	while ((cu != NULL) && (fcu == NULL))
	{
		if (strcmp(cu->ContainerID, containerID) == 0)
		{
			fcu = cu;
		}

		lcu = cu;
		cu = cu->Next;
	}

	if (fcu == NULL)
	{
		/*
		 *	If fcu is NULL, then we need to add
		 *	a new MSL_ContainerUpdate to the object.
		 */
		fcu = lcu->Next = (struct MSL_ContainerUpdate*) MSL_MALLOC(sizeof(struct MSL_ContainerUpdate));
		
		size = (int) strlen(containerID)+1;
		fcu->ContainerID = (char*) MSL_MALLOC(size);
		memcpy(fcu->ContainerID, containerID, size);
		fcu->Next = NULL;
	}

	ASSERT(fcu != NULL);

	/*
	 *	Assign a new UpdateID for the specified containerID.
	 */
	fcu->UpdateID = containerUpdateID;

	MSL_Helper_UpdateModerated_ContainerUpdateID(msl);

	/* unlock */
	sem_post(&(msl->Lock));
}

void MSL_UpdateIpInfo(void *mslObj, int *ipAddrList, int ipAddrListLen)
{
	struct MSL_MediaServerObject *msl;
	int size;

	ASSERT(mslObj != NULL);

	msl = (struct MSL_MediaServerObject*) mslObj;

	/* copy the ip addresses to the msl object */

	sem_wait(&(msl->Lock));

	if (msl->IpAddrList != NULL)
	{
		MSL_FREE(msl->IpAddrList);
	}
	size = (int) (ipAddrListLen * sizeof(int));
	msl->IpAddrList = (int*) MSL_MALLOC(size);
	memcpy(msl->IpAddrList, ipAddrList, size);
	msl->IpAddrListLen = ipAddrListLen;

	sem_post(&(msl->Lock));
}

/* END SECTION - public methods */
/************************************************************************************/


