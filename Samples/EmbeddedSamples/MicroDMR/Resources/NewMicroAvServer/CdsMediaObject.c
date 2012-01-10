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

struct CdsMediaObject* CDS_AllocateObject()
{
	struct CdsMediaObject *cdsObj = (struct CdsMediaObject *) malloc (sizeof(struct CdsMediaObject));
	memset(cdsObj, 0, sizeof(struct CdsMediaObject));
	return cdsObj;
}

struct CdsMediaResource* CDS_AllocateResource()
{
	struct CdsMediaResource *res = (struct CdsMediaResource *) malloc (sizeof(struct CdsMediaResource));
	memset(res, 0, sizeof(struct CdsMediaResource));
	return res;
}

void CDS_DestroyObjects(struct CdsMediaObject *cdsObjList)
{
	struct CdsMediaObject *cdsObj = cdsObjList, *nextCds;

	while (cdsObj != NULL)
	{
		nextCds = cdsObj->Next;

		if (cdsObj->Creator != NULL)	{ free(cdsObj->Creator); }
		if (cdsObj->ID != NULL)			{ free(cdsObj->ID); }
		if (cdsObj->ParentID != NULL)	{ free(cdsObj->ParentID); }
		if (cdsObj->RefID != NULL)		{ free(cdsObj->RefID); }
		if (cdsObj->Title != NULL)		{ free(cdsObj->Title); }
		CDS_DestroyResources(cdsObj->Res);
		
		free (cdsObj);
		cdsObj = nextCds;
	}
}

void CDS_DestroyResources(struct CdsMediaResource *resList)
{
	struct CdsMediaResource *res = resList, *next;

	while (res != NULL)
	{
		next = res->Next;
		if (res->Value != NULL) { free (res->Value); }
		
		if ((res->ProtocolInfoAllocated != 0) && (res->ProtocolInfo != NULL)) 
		{
			free (res->ProtocolInfo); 
		}
		
		free (res);
		res = next;
	}
}