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

#ifdef _DEBUG
#define MSL_MALLOC msl_malloc
#define MSL_FREE msl_free
#define ASSERT(x) assert(x)
#else
#define MSL_MALLOC malloc
#define MSL_FREE free
#define ASSERT(x)
#endif

int msl_malloc_counter = 0;

void (*MSL_Callback_OnBrowse) (struct MSL_BrowseArgs *browseArgs) = NULL;
void (*MSL_OnStatsChanged) (struct MS_Stats *stats) = NULL;

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


/*
 *	BrowseFlag related error codes - this is a UPnP layer error code,
 *	and not really an AV error code.
 */
#define MSL_ERROR_CODE_INVALID_BROWSEFLAG	402
#define MSL_ERROR_MSG_INVALID_BROWSEFLAG	"Invalid value specified for BrowseFlag."

#define MSL_ERROR_CODE_INTERNAL				500
#define	MSL_ERROR_MSG_INTERNAL				"Unknown or internal error encountered."


/************************************************************************************/
/* START SECTION - configuration info */

/*
 *	Supported abilities for sorting browse/search results.
 *	MSL_CONFIG_SORT_CAPABILITIES:			array of metadata properties that can be used for sorting
 *	MSL_CONFIG_SORT_CAPABILITIES_LEN:		number of strings in MSL_CONFIG_SORT_CAPABILITIES
 *	MSL_CONFIG_SORT_CAPABILITIES_CAT:		concatenation of strings in MSL_CONFIG_SORT_CAPABILITIES
 */
const char *MSL_CONFIG_SORT_CAPABILITIES[]		=		{ "" }; /*{ "dc:title", "dc:creator" };*/
#define		MSL_CONFIG_SORT_CAPABILITIES_LEN			0 /*2*/
const char *MSL_CONFIG_SORT_CAPABILITIES_CAT	=		""; /* "dc:title,dc:creator"; */

/*
 *	Supported abilities for searching results.
 *	MSL_CONFIG_SEARCH_CAPABILITIES:			array of metadata properties that can be used for sorting
 *	MSL_CONFIG_SEARCH_CAPABILITIES_LEN:		number of strings in MSL_CONFIG_SEARCH_CAPABILITIES
 *	MSL_CONFIG_SEARCH_CAPABILITIES_CAT:		concatenation of strings in MSL_CONFIG_SEARCH_CAPABILITIES
 */
const char *MSL_CONFIG_SEARCH_CAPABILITIES[]	=		{ "" };
#define		MSL_CONFIG_SEARCH_CAPABILITIES_LEN			0
const char *MSL_CONFIG_SEARCH_CAPABILITIES_CAT	=		"";

/*
 *	Union set of known source protocolInfo strings.
 *	MSL_CONFIG_SOURCE_PROTOCOLINFO:			Array of protocolInfo that can be used as a source.
 *											Values should correspond to MSL_Enum_SrcProtInfo.
 *
 *	MSL_CONFIG_SOURCE_PROTOCOLINFO_LEN:		Number of strings in MSL_CONFIG_SOURCE_PROTOCOLINFO.
 *											
 *	MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_LEN:	Number of bytes needed to represent a concatenation of 
 *											MSL_CONFIG_SOURCE_PROTOCOLINFO in a comma-delimited
 *											fashion, including the null terminator
 *
 *	Current implementation supports up to 64 items in MSL_CONFIG_SOURCE_PROTOCOLINFO,
 *	assuming unsigned long values are 64bits wide.
 */
const char *MSL_CONFIG_SOURCE_PROTOCOLINFO[]	=		{ "http-get:*:*:*" };
#define		MSL_CONFIG_SOURCE_PROTOCOLINFO_LEN			1
#define		MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_SIZE		15

/* END SECTION - configuration info */
/************************************************************************************/

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
	 *	Bitstring where each position of the bitstring indicates if a corresponding
	 *	protocolInfo in MSL_CONFIG_SOURCE_PROTOCOLINFO is currently enabled.
	 *
	 *	BitIndex to ArrayIndex mapping is: bi = (ai+1), where bi is bit index and ai is array index.
	 */
	unsigned long		SourceProtocolInfo;

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

	sem_post(&(msl->Lock));

	/* Framework will free the data at 'mslObj' */
	sem_destroy(&(msl->Lock));
	/*free(mslObj);*/
}

/*
 *	Assumes caller has locked msl->Lock.
 */
int MSL_GetActiveSourceProtocolInfo(struct MSL_MediaServerObject *msl, char *data)
{
  /*
   *	Using the MSL_TheMslObj->SourceProtocolInfo bitstring,
   *	print out the list of protocolInfo strings that are currently
   *	supported.
   *
   *	TODO: Add support to allow app to enable/disable protocolInfo
   */

	int ai;
	int bi;
	char *di;
	int writeComma;

	di = data;

	/*
	 *	Iterate through the bits of MSL_TheMslObj->SourceProtocolInfo
	 *	and sprintf the active protocolInfo strings to 
	 *	'data', such that the result is a comma-delimited list
	 *	of active protocolInfo strings.
	 */
	bi = 1;
	writeComma = 0;
	for (ai=0; ai < MSL_CONFIG_SOURCE_PROTOCOLINFO_LEN; ai++)
	{
		if (msl->SourceProtocolInfo & bi)
		{
			if (writeComma != 0)
			{
				di += sprintf(di, ",");
			}

			di += sprintf(di, MSL_CONFIG_SOURCE_PROTOCOLINFO[ai]);
			writeComma = 1;
		}
		bi = bi << 2;
	}
	
	sem_post(&(MSL_TheMslObj->Lock));

	return (int) (di - data);
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

	UpnpResponse_ContentDirectory_GetSearchCapabilities(upnptoken, MSL_CONFIG_SEARCH_CAPABILITIES_CAT);
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

	UpnpResponse_ContentDirectory_GetSortCapabilities(upnptoken, MSL_CONFIG_SORT_CAPABILITIES_CAT);
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
	 *	TODO: Add support for connection-lifetime aware protocols.
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
	 *	TODO: Add support for connection-lifetime aware protocols.
	 */

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpConnectionManager_GetCurrentConnectionIDs();\r\n");
	#endif

	UpnpResponse_ConnectionManager_GetCurrentConnectionIDs(upnptoken, "");
}

void UpnpConnectionManager_GetProtocolInfo(void* upnptoken)
{
	int size;
	char data[MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_SIZE];

	#ifdef _DEBUG
	printf("UPnP Invoke: UpnpConnectionManager_GetProtocolInfo();\r\n");
	#endif

	sem_wait(&(MSL_TheMslObj->Lock));
	size = MSL_GetActiveSourceProtocolInfo(MSL_TheMslObj, data);
	sem_post(&(MSL_TheMslObj->Lock));

	ASSERT(size < (MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_SIZE));

	UpnpResponse_ConnectionManager_GetProtocolInfo(upnptoken, data, "");
}



void UpnpContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	struct MSL_BrowseArgs *browseArgs;
	int errorCode;
	const char *errorMsg;
	int browseChildren;
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
		browseChildren = 1;
	}
	else if (stricmp(BrowseFlag, CDS_STRING_BROWSE_METADATA) == 0)
	{
		browseChildren = 0;
	}
	else
	{
		fprintf(stderr, "WARNING: UpnpContentDirectory_Browse(): Possible error with generated microstack. Encountered BrowseFlag='%s'", BrowseFlag);
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
		 *	Create an MSL_BrowseArgs object and execute
		 *	the browse callback so that application can return results.
		 */

		browseArgs = (struct MSL_BrowseArgs*) MSL_MALLOC (sizeof(struct MSL_BrowseArgs));
		memset(browseArgs, 0, sizeof(struct MSL_BrowseArgs));

		browseArgs->_ReservedDidlOutArgname = CDS_STRING_RESULT;
		browseArgs->BrowseDirectChildren = browseChildren;
		
		size = (int) strlen(Filter)+1;
		browseArgs->Filter = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->Filter, Filter, size);

		browseArgs->MediaServerObject = MSL_TheMslObj;
		
		size = (int) strlen(ObjectID)+1;
		browseArgs->ObjectID = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->ObjectID, ObjectID, size);

		browseArgs->RequestedCount = RequestedCount;

		size = (int) strlen(SortCriteria)+1;
		browseArgs->SortCriteria = (char*) MSL_MALLOC(size);
		memcpy(browseArgs->SortCriteria, SortCriteria, size);

		browseArgs->StartingIndex = StartingIndex;

		browseArgs->UpnpToken = upnptoken;
		browseArgs->UserObject = NULL;

		if (MSL_Callback_OnBrowse != NULL)
		{
			MSL_Callback_OnBrowse(browseArgs);
		}
	}
}

/* END SECTION - Dispatch sinks generated in original main.c */
/************************************************************************************/






/************************************************************************************/
/* START SECTION - public methods*/


/* see header file */
void *MSL_CreateMediaServer(void *chain, void *upnpStack, void* lifetimeMonitor)
{
	struct MSL_MediaServerObject *mslObj;
	char data[MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_SIZE];
	int size;

	mslObj = NULL;
	if (MSL_TheMslObj == NULL)
	{
		mslObj = MSL_TheMslObj = (struct MSL_MediaServerObject*) MSL_MALLOC(sizeof(struct MSL_MediaServerObject));
		memset(mslObj, 0, sizeof(struct MSL_MediaServerObject));
		
		mslObj->UpnpStack = upnpStack;
		mslObj->Destroy = MSL_DestroyMediaServer;
		mslObj->LifeTimeMonitor = lifetimeMonitor;
		mslObj->SourceProtocolInfo = 0xFFFFFFFFFFFFFFFF;
		sem_init(&(mslObj->Lock), 0, 1);

		UpnpSetState_ContentDirectory_SystemUpdateID(upnpStack, mslObj->SystemUpdateID);

		/* note active protocolInfo */
		size = MSL_GetActiveSourceProtocolInfo(mslObj, data);
		ASSERT(size < (MSL_CONFIG_SOURCE_PROTOCOLINFO_CAT_SIZE));
		UpnpSetState_ConnectionManager_SourceProtocolInfo(upnpStack, data);

		UpnpSetState_ConnectionManager_SinkProtocolInfo(upnpStack,"");
		UpnpSetState_ConnectionManager_CurrentConnectionIDs(upnpStack, "");

		ILibAddToChain(chain, mslObj);
	}

	return mslObj;
}

/* see header file */
void MSL_DeallocateBrowseArgs(struct MSL_BrowseArgs **browseArgs)
{
	MSL_FREE ((*browseArgs)->Filter);
	MSL_FREE ((*browseArgs)->ObjectID);
	MSL_FREE ((*browseArgs)->SortCriteria);
	MSL_FREE ((*browseArgs));
	*browseArgs = NULL;
}

/* see header file */
void MSL_ForResponse_RespondError(struct MSL_BrowseArgs *browseArgs, int errorCode, const char *errorMsg)
{
	ASSERT(browseArgs != NULL);

	UpnpResponse_Error(browseArgs->UpnpToken, errorCode, errorMsg);
}

/* see header file */
void MSL_ForResponse_RespondBrowse_StartResponse(struct MSL_BrowseArgs *browseArgs, int sendDidlHeader)
{
	ASSERT(browseArgs != NULL);

	UpnpAsyncResponse_START(browseArgs->UpnpToken, CDS_STRING_BROWSE, CDS_STRING_URN_CDS);

	if (sendDidlHeader != 0)
	{
		UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, CDS_DIDL_HEADER_ESCAPED, CDS_DIDL_HEADER_ESCAPED_LEN, 1, 0);
	}
	else
	{
		UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, "", 0, 1, 0);
	}
}

/* see header file */
void MSL_ForResponse_RespondBrowse_ResultArgument(struct MSL_BrowseArgs *browseArgs, const char *xmlEscapedUtf8Didl, int didlSize)
{
	ASSERT(browseArgs != NULL);

	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, xmlEscapedUtf8Didl, didlSize, 0, 0);
}

/* see header file */
void MSL_ForResponse_RespondBrowse_FinishResponse(struct MSL_BrowseArgs *browseArgs, int sendDidlFooter, unsigned int numberReturned, unsigned int totalMatches, unsigned int updateID)
{
	char numResult[30];

	ASSERT(browseArgs != NULL);

	if (sendDidlFooter != 0)
	{
		UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, CDS_DIDL_FOOTER_ESCAPED, CDS_DIDL_FOOTER_ESCAPED_LEN, 0, 1);
	}
	else
	{
		UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, "", 0, 0, 1);
	}

	/*
	 *	Instruct the generated microstack to send the data for the last
	 *	three out-arguments of the Browse request.
	 */

	sprintf(numResult, "%u", numberReturned);
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_NUMBER_RETURNED, numResult, (int) strlen(numResult), 1,1);

	sprintf(numResult, "%u", totalMatches);
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_TOTAL_MATCHES, numResult, (int) strlen(numResult), 1,1);

	sprintf(numResult, "%u", updateID);
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_UPDATE_ID, numResult, (int) strlen(numResult), 1,1);

	UpnpAsyncResponse_DONE(browseArgs->UpnpToken, CDS_STRING_BROWSE);
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

/* END SECTION - public methods */
/************************************************************************************/


