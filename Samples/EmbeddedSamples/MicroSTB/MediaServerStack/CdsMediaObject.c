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
#include "CdsMediaObject.h"
#include "Utility.h"

#ifdef WIN32
#include <crtdbg.h>
#endif

struct CdsMediaObject* CDS_AllocateObject()
{
	struct CdsMediaObject *cdsObj = (struct CdsMediaObject *) MALLOC (sizeof(struct CdsMediaObject));
	memset(cdsObj, 0, sizeof(struct CdsMediaObject));
	return cdsObj;
}

struct CdsMediaResource* CDS_AllocateResource()
{
	struct CdsMediaResource *res = (struct CdsMediaResource *) MALLOC (sizeof(struct CdsMediaResource));
	res->Allocated = 0;
	res->Next = NULL;
	res->Value = res->Protection = res->ProtocolInfo = NULL;
	res->Size = res->ColorDepth = res->Bitrate = res->Duration = res->ResolutionX = res->ResolutionY = res->BitsPerSample = res->SampleFrequency = res->NrAudioChannels = -1;
	return res;
}

void CDS_DestroyObjects(struct CdsMediaObject *cdsObjList)
{
	struct CdsMediaObject *cdsObj = cdsObjList, *nextCds;

	while (cdsObj != NULL)
	{
		nextCds = cdsObj->Next;

		if ((cdsObj->DeallocateThese & CDS_ALLOC_Creator)	&& (cdsObj->Creator != NULL))	{ FREE(cdsObj->Creator); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_ID)		&& (cdsObj->ID != NULL))		{ FREE(cdsObj->ID); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_ParentID)	&& (cdsObj->ParentID != NULL))	{ FREE(cdsObj->ParentID); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_RefID)		&& (cdsObj->RefID != NULL))		{ FREE(cdsObj->RefID); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_Title)		&& (cdsObj->Title != NULL))		{ FREE(cdsObj->Title); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_Album)		&& (cdsObj->Album != NULL))		{ FREE(cdsObj->Album); }
		if ((cdsObj->DeallocateThese & CDS_ALLOC_Genre)		&& (cdsObj->Genre != NULL))		{ FREE(cdsObj->Genre); }
		CDS_DestroyResources(cdsObj->Res);
		
		FREE (cdsObj);
		cdsObj = nextCds;
	}
}

void CDS_DestroyResources(struct CdsMediaResource *resList)
{
	struct CdsMediaResource *res = resList, *next;

	while (res != NULL)
	{
		next = res->Next;
		if (res->Value != NULL) { FREE (res->Value); }
		
		if ((res->Allocated & CDS_ALLOC_ProtInfo) && (res->ProtocolInfo != NULL)) 
		{
			FREE (res->ProtocolInfo); 
		}
		
		if ((res->Allocated & CDS_ALLOC_Protection) && (res->Protection != NULL)) 
		{
			FREE (res->Protection); 
		}

		FREE (res);
		res = next;
	}
}