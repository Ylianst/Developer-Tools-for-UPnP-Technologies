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

#include "MyList.h"

struct UDNList * UDNList_Create()
{
	struct UDNList * ret = (struct UDNList*)MALLOC(sizeof(struct UDNList));
	ret->head = NULL;
	ret->tail = NULL;
	return ret;
};

static void UDNList_Destroy(struct UDNList * toDestroy)
{
	struct UDNListNode * currNode = toDestroy->head;
	struct UDNListNode * nextNode;
	while(currNode != NULL)
	{
		nextNode = currNode->next;
		FREE(currNode->UDN);
		FREE(currNode);
		currNode = nextNode;
	}
	FREE(toDestroy);
}

static void UDNList_Add(char *UDN)
{

}

static char * UDNList_Get(int index)
{
	return NULL;
}