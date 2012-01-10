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
#include "CdsContentDiscovery.h"
#include "MyString.h"
#include "CdsStrings.h"



struct MmsMediaServerObject
{
	/*
	 *	The PreSelect, PostSelect, and Destroy fields must
	 *	remain here as the object needs to be compatible with the
	 *	thread-chaining framework.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
};



/* see header file */
void Mms_ForResponse_RespondError(struct MmsBrowseArgs *browseArgs, int errorCode, const char *errorMsg)
{
	UpnpResponse_Error(browseArgs->UpnpToken, errorCode, errorMsg);
}

/* see header file */
void Mms_ForResponse_RespondBrowse_StartResponse(struct MmsBrowseArgs *browseArgs, int sendDidlHeader)
{
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
void Mms_ForResponse_RespondBrowse_ResultArgument(struct MmsBrowseArgs *browseArgs, const char *xmlEscapedUtf8Didl, int didlSize)
{
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_RESULT, xmlEscapedUtf8Didl, didlSize, 0, 0);
}

/* see header file */
void Mms_ForResponse_RespondBrowse_FinishResponse(struct MmsBrowseArgs *browseArgs, int sendDidlFooter, unsigned int numberReturned, unsigned int totalMatches, unsigned int updateID)
{
	char numResult[30];

	UpnpAsyncResponse_START(browseArgs->UpnpToken, CDS_STRING_BROWSE, CDS_STRING_URN_CDS);

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
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_NUMBER_RETURNED, numResult, strlen(numResult), 1,1);

	sprintf(numResult, "%u", totalMatches);
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_TOTAL_MATCHES, numResult, strlen(numResult), 1,1);

	sprintf(numResult, "%u", updateID);
	UpnpAsyncResponse_OUT(browseArgs->UpnpToken, CDS_STRING_UPDATE_ID, numResult, strlen(numResult), 1,1);
}


/*
 *	This method creates the MediaServer object that abstracts state information
 *	for a MediaServer. 
 */
void *CreateMediaServer(void *chain, void *stack)
{
	struct MmsMediaServerObject *mmsObj;

	mmsObj = (struct MmsMediaServerObject*) malloc(sizeof(struct MmsMediaServerObject));
	memset(mmsObj, 0, sizeof(struct MmsMediaServerObject));

	return mmsObj;
}

/*
 *	This method release the resources occupied
 *		mmsObj					: The object returned from CreateMediaServer
 */
void DestroyMediaServer(void *mmsObj)
{

	/* Framework will free the data at 'mmsObj' */
	/*free(mmsObj);*/
}


