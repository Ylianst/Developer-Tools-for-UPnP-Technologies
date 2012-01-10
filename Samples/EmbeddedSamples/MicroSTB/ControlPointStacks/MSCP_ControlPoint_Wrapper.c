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

#include "MSCP_ControlPoint_Wrapper.h"
#include "ILibParsers.h"
#include "MyString.h"
#include "Utility.h"

#include <stdlib.h>
#include <stdio.h>

#ifndef UNDER_CE
#include "assert.h"
#else
#define stricmp _stricmp
#endif

#ifdef UNDER_CE
#define strnicmp _strnicmp
#endif

#ifdef _POSIX
#define strnicmp strncasecmp
#endif

#define UNSUPPORTED_BY_CP printf("Action is not supported by this implementation."); ASSERT(1);

#define MSCP_BROWSE_FLAG_METADATA_STRING "BrowseMetadata"
#define MSCP_BROWSE_FLAG_CHILDREN_STRING "BrowseDirectChildren"

/* CDS normative tag names and attributes */
#define MSCP_ATTRIB_ID				"id"
#define MSCP_ATTRIB_PARENTID		"parentID"
#define MSCP_ATTRIB_REFID			"refID"
#define MSCP_ATTRIB_RESTRICTED		"restricted"
#define MSCP_ATTRIB_SEARCHABLE		"searchable"
#define MSCP_ATTRIB_PROTOCOLINFO	"protocolInfo"
#define MSCP_ATTRIB_RESOLUTION		"resolution"
#define MSCP_ATTRIB_DURATION		"duration"
#define MSCP_ATTRIB_BITRATE			"bitrate"
#define MSCP_ATTRIB_COLORDEPTH		"colorDepth"
#define MSCP_ATTRIB_SIZE			"size"

#define MSCP_TAG_DIDL				"DIDL-Lite"
#define MSCP_TAG_CONTAINER			"container"
#define MSCP_TAG_ITEM				"item"
#define MSCP_TAG_RESOURCE			"res"

#define MSCP_TAG_CREATOR			"dc:creator"
#define MSCP_TAG_TITLE				"dc:title"
#define MSCP_TAG_MEDIACLASS			"upnp:class"

#ifdef _DEBUG
#define	ASSERT(x) assert (x)
#define MSCP_MALLOC(x) MSCP_malloc(x)
#define MSCP_FREE(x) MSCP_free(x)
#define DEBUGONLY(x) x
#else
#ifndef UNDER_CE
#define ASSERT(x)
#endif
#define MSCP_MALLOC(x) MALLOC(x)
#define MSCP_FREE(x) FREE(x)
#define DEBUGONLY(x)
#endif

#ifdef _TEMPDEBUG
#define TEMPDEBUGONLY(x) x
#else
#define TEMPDEBUGONLY(x) 
#endif

/***********************************************************************************************************************
 *	BEGIN: MSCP static values
 ***********************************************************************************************************************/

/*
 *	The relative order of strings within these arrays must correspond to the MSCP_CLASS_MASK_xxx bitmask mappings.
 */
const char* MSCP_CLASS_OBJECT_TYPE[] = {"object", "item", "container"};
const char* MSCP_CLASS_MAJOR_TYPE[] = {"", "imageItem", "audioItem", "videoItem", "playlistItem", "textItem", "person", "playlistContainer", "album", "genre", "storageSystem", "storageVolume", "storageFolder"};
const char* MSCP_CLASS_MINOR_TYPE[] = {"", "photo", "musicTrack", "audioBroadcast", "audioBook", "movie", "videoBroadcast", "musicVideClip", "musicArtist", "musicAlbum", "photoAlbum", "musicGenre", "movieGenre"};

#define MSCP_CLASS_FIRST_MAJOR_TYPE	MSCP_CLASS_MASK_MAJOR_IMAGEITEM
/*
 *	Maximum length of a string in 
 *	MSCP_CLASS_OBJECT_TYPE, MSCP_CLASS_MAJOR_TYPE, and MSCP_CLASS_MINOR_TYPE.
 *	Size includes null terminator.
 */
#define MSCP_MAX_CLASS_FRAGMENT_LEN	17
#define MSCP_MAX_CLASS_FRAGMENT_SIZE	18

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))

/*
 *	These are the types of strings that can be searched.
 */
enum MSCP_SearchableStringTypes
{
	MSCP_STRTYPE_ID,
	MSCP_STRTYPE_CREATOR,
	MSCP_STRTYPE_PROTOCOLINFO
};

const char* MSCP_TRUE_STRINGS[] = {"1", "true", "yes"};
const char* MSCP_FALSE_STRINGS[] = {"0", "false", "no"};
#define MSCP_TRUE_STRINGS_LEN 3
#define MSCP_FALSE_STRINGS_LEN 3

/***********************************************************************************************************************
 *	END: MSCP static values
 ***********************************************************************************************************************/




/***********************************************************************************************************************
 *	BEGIN: MSCP state variables
 ***********************************************************************************************************************/

/* Function pointer for sending Browse results back to caller */
MSCP_Fn_Result_Browse				MSCP_Callback_Browse;
CP_Fn_Device_Add					MSCP_Callback_DeviceAdd;
CP_Fn_Device_Remove					MSCP_Callback_DeviceRemove;

int MSCP_malloc_counter = 0;

/***********************************************************************************************************************
 *	END: MSCP state variables
 ***********************************************************************************************************************/




/***********************************************************************************************************************
 *	BEGIN: Helper methods
 ***********************************************************************************************************************/

/* TODO: debug MALLOC/MSCP_FREE is not thread safe */
void* MSCP_malloc(int sz)
{
	++MSCP_malloc_counter;
	return((void*)MALLOC(sz));
}
void MSCP_free(void* ptr)
{
	--MSCP_malloc_counter;
	FREE(ptr);	
}
int MSCP_malloc_GetCount()
{
	return(MSCP_malloc_counter);
}

/*
 *	Copies bytes from copyFrom to copyHere.
 *	Will not copy more than copyMaxChars bytes.
 *	Stops copying when ., <, null, or " char is found.
 */
void MSCP_CopyUntilClassFragmentTerminator(char *copyHere, const char *copyFrom, int copyMaxChars)
{
	int i;
	char c;
	
	for (i=0; i < copyMaxChars; i++)
	{
		c = copyFrom[i];
		
		if (c == '.' || c == '<' || c == '\0' || c == '"')
		{
			copyHere[i] = '\0';
			return;
		}
		else
		{
			copyHere[i] = c;
		}
	}
}

/*
 *	Given an array of strings, finds the index in that array with a matching string.
 */
int MSCP_FindStringInArray(const char* str,const char** strarray,const int strarraylen)
{
	int i;
	for (i=0;i<strarraylen;i++) {if (stricmp(str,strarray[i]) == 0) {return i;}}
	return -1;
}


void MSCP_StringFixup(char **fixThis, char** di, char *emptyStr, const char *data, const char *rangeStart, const char *rangeEnd)
{
	int len;

	if (data != NULL)
	{
		if ((rangeStart <= data) && (data <= rangeEnd))
		{
			/* store an XML-unescaped representation */

			*fixThis = *di;
			len = (int) strlen(data);
			memcpy(*di, data, len);

			ILibInPlaceXmlUnEscape(*di);

			*di = *di + len + 1;
		}
		else
		{
			*fixThis = (char*)data;
		}
	}
	else
	{
		*fixThis = emptyStr;
	}
}


int MSCP_GetRequiredSizeForMediaObject(struct MSCP_MediaObject *obj, struct MSCP_MediaObject *obj2)
{
	int retVal;
	struct MSCP_MediaResource *res;
	struct MSCP_MediaResource *res2;
	unsigned char addProtInfo;
	unsigned char addResolution;
	struct MSCP_MediaResource *resProt;
	struct MSCP_MediaResource *resRes;

	retVal = 0;

	if (obj->ID != NULL)
	{
		retVal += ((int) strlen(obj->ID) +1);
	}

	if (obj->ParentID != NULL)
	{
		if ((obj2 != NULL) && (strcmp(obj2->ParentID, obj->ParentID) == 0))
		{
			obj->ParentID = obj2->ParentID;
		}
		else
		{
			retVal += ((int) strlen(obj->ParentID) +1);
		}
	}

	if (obj->RefID != NULL)
	{
		retVal += ((int) strlen(obj->RefID) +1);
	}

	if (obj->Title != NULL)
	{
		retVal += ((int) strlen(obj->Title) +1);
	}

	if (obj->Creator != NULL)
	{
		if ((obj2 != NULL) && (strcmp(obj2->Creator, obj->Creator) == 0))
		{
			obj->Creator = obj2->Creator;
		}
		else
		{
			retVal += ((int) strlen(obj->Creator) +1);
		}
	}

	res = obj->Res;
	res2 = NULL;
	while (res != NULL)
	{
		//if (res->ProtocolInfo != NULL)
		addProtInfo = (res->ProtocolInfo != NULL);
		addResolution = (res->Resolution != NULL);
		{
			if (obj2 != NULL)
			{
				res2 = obj2->Res;
			}

			resProt = NULL;
			resRes = NULL;
			while (res2 != NULL)
			{
				if (addProtInfo && (res2->ProtocolInfo != NULL) && (strcmp(res2->ProtocolInfo, res->ProtocolInfo) == 0))
				{
					addProtInfo = 0;
					resProt = res2;
				}
				if (addResolution && (res2->Resolution != NULL) && (strcmp(res2->Resolution, res->Resolution) == 0))
				{
					addResolution = 0;
					resRes = res2;
				}

				if ((addProtInfo != 0) || (addResolution != 0))
				{
					res2 = res2->Next;
				}
				else
				{
					res2 = NULL;
				}
			}

			if (addProtInfo != 0)
			{
				retVal += ((int) strlen(res->ProtocolInfo) +1);
			}
			else if (resProt != NULL)
			{
				res->ProtocolInfo = resProt->ProtocolInfo;
			}
			
			if (addResolution != 0)
			{
				retVal += ((int) strlen(res->Resolution) +1);
			}
			else if (resRes != NULL)
			{
				res->Resolution = resRes->Resolution;
			}
		}

		if (res->Uri != NULL)
		{
			retVal += ((int) strlen(res->Uri) +1);
		}

		res = res->Next;
	}

	return retVal;
}

void MSCP_RemoveQuotFromAttribValue(struct ILibXMLAttribute *att)
{
	if ((att->Value[0] = '"') || (att->Value[0] == '\''))
	{
		att->Value++;
		att->ValueLength -= 2;
	}
}

struct MSCP_MediaObject* MSCP_CreateMediaObject(struct ILibXMLNode *node, struct ILibXMLAttribute *attribs, int isItem, struct MSCP_MediaObject *obj2, const char *rangeStart, const char *rangeEnd)
{
	struct ILibXMLNode *startNode;
	struct ILibXMLAttribute *att;

	struct MSCP_MediaObject tempObj;
	struct MSCP_MediaObject* newObj;

	struct MSCP_MediaResource *res;

	char *innerXml;
	int innerXmlLen;
	char classFragment[MSCP_MAX_CLASS_FRAGMENT_SIZE];
	int indexIntoArray;

	int dataSize;
	int mallocSize;

	char *di;
	char *emptyDI;

	#ifdef _DEBUG
	/* PRECONDITION: node is a start node*/
	if (node->StartTag == 0)
	{
		printf("MSCP_CreateMediaObject requires node->StartTag!=0.\r\n");
		ASSERT(0);
	}
	
	/* PRECONDITION: node->Name is null terminated and this node is a container or item */
	if (!(
		(stricmp(node->Name, MSCP_TAG_CONTAINER) == 0) ||
		(stricmp(node->Name, MSCP_TAG_ITEM) == 0)
		))
	{
		printf("MSCP_CreateMediaObject requires item or container node.\r\n");
		ASSERT(0);
	}
	#endif

	/* initialize temp obj to zero; init flags appropriately */
	memset(&tempObj, 0, sizeof(struct MSCP_MediaObject));
	tempObj.Flags |= MSCP_Flags_Restricted;	/* assume object is restricted */
	if (isItem == 0)
	{
		tempObj.Flags |= MSCP_Flags_Searchable;/* assume container is searchable */
	}

	/*
	 *
	 *	Parse the item/container node and set the pointers in tempObj
	 *	to point into the memory referenced by node.
	 *
	 */

	/* Parse the attributes of the item/container */
	att = attribs;
	while (att != NULL)
	{
		/* [DONOTREPARSE] null terminate name and value. */
		att->Name[att->NameLength] = '\0';
		MSCP_RemoveQuotFromAttribValue(att);
		att->Value[att->ValueLength] = '\0';

		if (stricmp(att->Name, MSCP_ATTRIB_ID) == 0)
		{
			tempObj.ID = att->Value;
		}
		else if (stricmp(att->Name, MSCP_ATTRIB_PARENTID) == 0)
		{
			tempObj.ParentID = att->Value;
		}
		else if (stricmp(att->Name, MSCP_ATTRIB_RESTRICTED) == 0)
		{
			if (MSCP_FindStringInArray(att->Value, MSCP_TRUE_STRINGS, MSCP_TRUE_STRINGS_LEN) >= 0)
			{
				/* set the restricted flag. */
				tempObj.Flags |= MSCP_Flags_Restricted;
			}
			else
			{
				tempObj.Flags &= (~MSCP_Flags_Restricted);
			}
		}
		else if ((isItem == 0) && (stricmp(att->Name, MSCP_ATTRIB_SEARCHABLE) == 0))
		{
			if (MSCP_FindStringInArray(att->Value, MSCP_TRUE_STRINGS, MSCP_TRUE_STRINGS_LEN) >= 0)
			{
				/* set the searchable flag. */
				tempObj.Flags |= MSCP_Flags_Searchable;
			}
			else
			{
				tempObj.Flags &= (~MSCP_Flags_Searchable);
			}
		}
		else if ((isItem != 0) && (stricmp(att->Name, MSCP_ATTRIB_REFID) == 0))
		{
			tempObj.RefID = att->Value;
		}
		att = att->Next;
	}

	/*
	 *
	 *	Iterate through the child nodes of the startNode
	 *	and set the title, creator, and resources for
	 *	the media object.
	 *
	 */

	startNode = node;
	node = startNode->Next;
	while (node != startNode->ClosingTag)
	{
		/* [DONOTREPARSE] null terminate name */
		attribs = ILibGetXMLAttributes(node);
		att = attribs;
		node->Name[node->NameLength] = '\0';

		if (node->StartTag != 0)
		{
			if (stricmp(node->Name, MSCP_TAG_RESOURCE) == 0)
			{
				/*
				 *
				 *	Create a new resource element and add it
				 *	to the existing list of resources for the
				 *	media object. The resource will point to 
				 *	memory in XML, but we'll change where they
				 *	point at the very end.
				 *
				 */
				
				if (tempObj.Res == NULL)
				{
					tempObj.Res = (struct MSCP_MediaResource*) MSCP_MALLOC (sizeof(struct MSCP_MediaResource));
					res = tempObj.Res;
				}
				else
				{
					res->Next = (struct MSCP_MediaResource*) MSCP_MALLOC (sizeof(struct MSCP_MediaResource));
					res = res->Next;
				}

				/* initialize everything to zero */
				memset(res, 0, sizeof(struct MSCP_MediaResource));
				res->Bitrate = res->ColorDepth = res->Size = -1;

				/* Extract the protocolInfo from the element */
				while (att != NULL)
				{
					/* [DONOTREPARSE] */
					att->Name[att->NameLength] = '\0';
					MSCP_RemoveQuotFromAttribValue(att);
					att->Value[att->ValueLength] = '\0';
								
					if (stricmp(att->Name, MSCP_ATTRIB_PROTOCOLINFO) == 0)
					{
						res->ProtocolInfo = att->Value;
						break;
					}
					else if (stricmp(att->Name, MSCP_ATTRIB_RESOLUTION) == 0)
					{
						res->Resolution = att->Value;
					}
					else if (stricmp(att->Name, MSCP_ATTRIB_DURATION) == 0)
					{
						res->Duration = att->Value;
					}
					else if (stricmp(att->Name, MSCP_ATTRIB_BITRATE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, &(res->Bitrate));
					}
					else if (stricmp(att->Name, MSCP_ATTRIB_COLORDEPTH) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, &(res->ColorDepth));
					}
					else if (stricmp(att->Name, MSCP_ATTRIB_SIZE) == 0)
					{
						ILibGetLong(att->Value, att->ValueLength, &(res->Size));
					}
					
					att = att->Next;
				}

				/* grab the URI */
				innerXmlLen = ILibReadInnerXML(node, &innerXml);
				innerXml[innerXmlLen] = '\0';
				res->Uri = innerXml;
			}
			else if (stricmp(node->NSTag, MSCP_TAG_MEDIACLASS) == 0)
			{
				/* Figure out proper enum value given the specified media class */
				innerXmlLen = ILibReadInnerXML(node, &innerXml);

				/* initialize to bad class */
				tempObj.MediaClass = MSCP_CLASS_MASK_BADCLASS;
							
				/* determine object type */
				MSCP_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, MSCP_MAX_CLASS_FRAGMENT_LEN));
				indexIntoArray = MSCP_FindStringInArray(classFragment, MSCP_CLASS_OBJECT_TYPE, MSCP_CLASS_OBJECT_TYPE_LEN);

				if (indexIntoArray == 0)
				{
					innerXml += ((int) strlen(MSCP_CLASS_OBJECT_TYPE[indexIntoArray]) + 1);
					MSCP_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, MSCP_MAX_CLASS_FRAGMENT_LEN));
					indexIntoArray = MSCP_FindStringInArray(classFragment, MSCP_CLASS_OBJECT_TYPE, MSCP_CLASS_OBJECT_TYPE_LEN);

					if (indexIntoArray > 0)
					{
						innerXml += ((int) strlen(MSCP_CLASS_OBJECT_TYPE[indexIntoArray]) + 1);
						tempObj.MediaClass = indexIntoArray;
									
						/* Determine major type */
						MSCP_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, MSCP_MAX_CLASS_FRAGMENT_LEN));
						indexIntoArray = MSCP_FindStringInArray(classFragment, MSCP_CLASS_MAJOR_TYPE, MSCP_CLASS_MAJOR_TYPE_LEN);
						if (indexIntoArray > 0)
						{
							innerXml += ((int) strlen(MSCP_CLASS_MAJOR_TYPE[indexIntoArray]) + 1);
							tempObj.MediaClass |= (indexIntoArray << MSCP_SHIFT_MAJOR_TYPE);

							/* Determine minor type */
							MSCP_CopyUntilClassFragmentTerminator(classFragment, innerXml, MIN(innerXmlLen, MSCP_MAX_CLASS_FRAGMENT_LEN));
							indexIntoArray = MSCP_FindStringInArray(classFragment, MSCP_CLASS_MAJOR_TYPE, MSCP_CLASS_MAJOR_TYPE_LEN);
							if (indexIntoArray > 0)
							{
								tempObj.MediaClass |= (indexIntoArray << MSCP_SHIFT_MINOR1_TYPE);
								/* TODO : Add vendor-specific supported minor types parsing here */
							}
						}
					}
				}
			}
			else if (stricmp(node->NSTag, MSCP_TAG_CREATOR) == 0)
			{
				innerXmlLen = ILibReadInnerXML(node, &innerXml);
				innerXml[innerXmlLen] = '\0';
				tempObj.Creator = innerXml;
			}
			else if (stricmp(node->NSTag, MSCP_TAG_TITLE) == 0)
			{
				innerXmlLen = ILibReadInnerXML(node, &innerXml);
				innerXml[innerXmlLen] = '\0';
				tempObj.Title = innerXml;
			}
		}

		node = node->Next;
		#ifdef _DEBUG
		if (node == NULL)
		{
			printf("MSCP_CreateMediaObject: Unexpected null node.\r\n");
			ASSERT(0);
		}
		#endif

		/* FREE attribute mapping */
		ILibDestructXMLAttributeList(attribs);
	}

	/*
	 *
	 *	At this point, we have a temp media object and possibly some media resources.
	 *	All string data is simply a pointer into the XML string. In order to
	 *	maximize on efficient memory usage, we do the following.
	 *
	 *	1)	Determine size needed for all new strings in results set. Also note which strings need to be copied in this step.
	 *	2)	Create a new media object, with additional memory for storing new string data.
	 *	3)	Point new media object's fields to either the new memory or to existing memory from a previous media object.
	 *	4)	Connect new media object to resource objects (attached to temp)
	 *	5)	Point each field of each resource to memory in new memory to existing memory from a previous media object.
	 *
	 *
	 */

	/*
	 *	Create the new media object, with additional memory for string data appended at the end.
	 */
	dataSize = MSCP_GetRequiredSizeForMediaObject(&tempObj, obj2);
	mallocSize = dataSize + sizeof(struct MSCP_MediaObject) + 1;
	newObj = (struct MSCP_MediaObject*) MSCP_MALLOC(mallocSize);
	memset(newObj, 0, mallocSize);

	newObj->MediaClass = tempObj.MediaClass;
	newObj->Flags = tempObj.Flags;

	/* di will point to where it's safe to write string data */
	di = (char*)newObj;
	di += sizeof(struct MSCP_MediaObject);
	emptyDI = di;
	di ++;

	MSCP_StringFixup(&(newObj->ID),		&di, emptyDI, tempObj.ID,		rangeStart, rangeEnd);
	MSCP_StringFixup(&(newObj->ParentID),	&di, emptyDI, tempObj.ParentID,	rangeStart, rangeEnd);
	MSCP_StringFixup(&(newObj->RefID),		&di, emptyDI, tempObj.RefID,	rangeStart, rangeEnd);
	MSCP_StringFixup(&(newObj->Title),		&di, emptyDI, tempObj.Title,	rangeStart, rangeEnd);
	MSCP_StringFixup(&(newObj->Creator),	&di, emptyDI, tempObj.Creator,	rangeStart, rangeEnd);

	newObj->Res = tempObj.Res;
	res = newObj->Res;
	while (res != NULL)
	{
		/*
		 *	Since resources are already allocated, we send the same parameters
		 *	for arg1 and arg3.
		 */
		MSCP_StringFixup(&(res->ProtocolInfo), &di, emptyDI, res->ProtocolInfo,	rangeStart, rangeEnd);
		MSCP_StringFixup(&(res->Resolution),	&di, emptyDI, res->Resolution,		rangeStart, rangeEnd);
		MSCP_StringFixup(&(res->Uri),			&di, emptyDI, res->Uri,				rangeStart, rangeEnd);
		res = res->Next;
	}

	/* prevent memory corruption in debug version */
//	ASSERT((unsigned int)di <= ((unsigned int)newObj) + mallocSize);

	return newObj;
}


/***********************************************************************************************************************
 *	END: Helper methods
 ***********************************************************************************************************************/

 
 
 
/***********************************************************************************************************************
 *	BEGIN: UPnP Callback Sinks
 *	These methods are callback sinks that are wired to the underlying UPNP stack.
 ***********************************************************************************************************************/

#ifndef MSCP_LEAN_AND_MEAN
static void MSCP_ProcessResponse_GetCurrentConnectionInfo(struct UPnPService* Service,int ErrorCode,void *User,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status)
{
	printf("MSCP Invoke Response: ConnectionManager/GetCurrentConnectionInfo(%d,%d,%s,%s,%d,%s,%s)\r\n",RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	UNSUPPORTED_BY_CP;
}

static void MSCP_ProcessResponse_GetProtocolInfo(struct UPnPService* Service,int ErrorCode,void *User,char* Source,char* Sink)
{
	printf("MSCP Invoke Response: ConnectionManager/GetProtocolInfo(%s,%s)\r\n",Source,Sink);
	UNSUPPORTED_BY_CP;
}

static void MSCP_ProcessResponse_GetCurrentConnectionIDs(struct UPnPService* Service,int ErrorCode,void *User,char* ConnectionIDs)
{
	printf("MSCP Invoke Response: ConnectionManager/GetCurrentConnectionIDs(%s)\r\n",ConnectionIDs);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_ExportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("MSCP Invoke Response: ContentDirectory/ExportResource(%u)\r\n",TransferID);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_StopTransferResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/StopTransferResource()\r\n");
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_DestroyObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/DestroyObject()\r\n");
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_UpdateObject(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/UpdateObject()\r\n");
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_GetSystemUpdateID(struct UPnPService* Service,int ErrorCode,void *User,unsigned int Id)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSystemUpdateID(%u)\r\n",Id);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_GetTransferProgress(struct UPnPService* Service,int ErrorCode,void *User,char* TransferStatus,char* TransferLength,char* TransferTotal)
{
	printf("MSCP Invoke Response: ContentDirectory/GetTransferProgress(%s,%s,%s)\r\n",TransferStatus,TransferLength,TransferTotal);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_GetSortCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SortCaps)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSortCapabilities(%s)\r\n",SortCaps);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_GetSearchCapabilities(struct UPnPService* Service,int ErrorCode,void *User,char* SearchCaps)
{
	printf("MSCP Invoke Response: ContentDirectory/GetSearchCapabilities(%s)\r\n",SearchCaps);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_CreateObject(struct UPnPService* Service,int ErrorCode,void *User,char* ObjectID,char* Result)
{
	printf("MSCP Invoke Response: ContentDirectory/CreateObject(%s,%s)\r\n",ObjectID,Result);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_Search(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	printf("MSCP Invoke Response: ContentDirectory/Search(%s,%u,%u,%u)\r\n",Result,NumberReturned,TotalMatches,UpdateID);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_ImportResource(struct UPnPService* Service,int ErrorCode,void *User,unsigned int TransferID)
{
	printf("MSCP Invoke Response: ContentDirectory/ImportResource(%u)\r\n",TransferID);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_CreateReference(struct UPnPService* Service,int ErrorCode,void *User,char* NewID)
{
	printf("MSCP Invoke Response: ContentDirectory/CreateReference(%s)\r\n",NewID);
	UNSUPPORTED_BY_CP;
}

static void MSCPResponseSink_ContentDirectory_DeleteResource(struct UPnPService* Service,int ErrorCode,void *User)
{
	printf("MSCP Invoke Response: ContentDirectory/DeleteResource()\r\n");
	UNSUPPORTED_BY_CP;
}

static void MSCPEventSink_ConnectionManager_SourceProtocolInfo(struct UPnPService* Service,char* SourceProtocolInfo)
{
	printf("MSCP Event from %s/ConnectionManager/SourceProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SourceProtocolInfo);
	UNSUPPORTED_BY_CP;
}

static void MSCPEventSink_ConnectionManager_SinkProtocolInfo(struct UPnPService* Service,char* SinkProtocolInfo)
{
	printf("MSCP Event from %s/ConnectionManager/SinkProtocolInfo: %s\r\n",Service->Parent->FriendlyName,SinkProtocolInfo);
	UNSUPPORTED_BY_CP;
}

static void MSCPEventSink_ConnectionManager_CurrentConnectionIDs(struct UPnPService* Service,char* CurrentConnectionIDs)
{
	printf("MSCP Event from %s/ConnectionManager/CurrentConnectionIDs: %s\r\n",Service->Parent->FriendlyName,CurrentConnectionIDs);
	UNSUPPORTED_BY_CP;
}

static void MSCPEventSink_ContentDirectory_TransferIDs(struct UPnPService* Service,char* TransferIDs)
{
	printf("MSCP Event from %s/ContentDirectory/TransferIDs: %s\r\n",Service->Parent->FriendlyName,TransferIDs);
	UNSUPPORTED_BY_CP;
}

#endif

static void MSCPResponseSink_ContentDirectory_Browse(struct UPnPService* Service,int ErrorCode,void *User,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID)
{
	struct ILibXMLNode* nodeList;
	struct ILibXMLNode* node;
	struct MSCP_ResultsList *resultsList;
	struct ILibXMLAttribute *attribs;
	
	int resultLen;
	int parsePeerResult = 0;
	char *lastResultPos;

	struct MSCP_MediaObject *newObj, *lastObj;

	TEMPDEBUGONLY(printf("MSCP Invoke Response: ContentDirectory/Browse(%s,%u,%u,%u)\r\n",Result,NumberReturned,TotalMatches,UpdateID);)

	if ((ErrorCode == 0) && (Result != NULL))
	{
		lastObj = newObj = NULL;
		resultLen = ILibInPlaceXmlUnEscape(Result);
		resultsList = (struct MSCP_ResultsList*) MSCP_MALLOC (sizeof(struct MSCP_ResultsList));
		memset(resultsList, 0, sizeof(struct MSCP_ResultsList));

		lastResultPos = Result + resultLen;
		nodeList = ILibParseXML(Result, 0, resultLen);
		parsePeerResult = ILibProcessXMLNodeList(nodeList);

		if (parsePeerResult != 0)
		{
			MSCP_Callback_Browse(Service, User, (int)MSC_Error_XmlNotWellFormed, NULL);
		}
		else if (resultLen == 0)
		{
			MSCP_Callback_Browse(Service, User, ErrorCode, NULL);
		}
		else
		{
			node = nodeList;
			while (node != NULL)
			{
				if (node->StartTag != 0)
				{
					/*[DONOTREPARSE] null terminate string */
					attribs = ILibGetXMLAttributes(node);
					node->Name[node->NameLength] = '\0';

					newObj = NULL;
					if (stricmp(node->Name, MSCP_TAG_CONTAINER) == 0)
					{
						newObj = MSCP_CreateMediaObject(node, attribs, 0, lastObj, Result, lastResultPos);
						node = node->Next;
					}
					else if (stricmp(node->Name, MSCP_TAG_ITEM) == 0)
					{
						newObj = MSCP_CreateMediaObject(node, attribs, 1, lastObj, Result, lastResultPos);
						node = node->Next;
					}
					else if (stricmp(node->Name, MSCP_TAG_DIDL) == 0)
					{
						/* this is didl-lite root node, go to first child */
						node = node->Next;
					}
					else
					{
						/* this node is not supported, go to next sibling/peer */
						if (node->Peer != NULL)
						{
							node = node->Peer;
						}
						else
						{
							node = node->Parent->Peer;
						}
					}

					if (newObj != NULL)
					{
						if (resultsList->FirstObject == NULL)
						{
							lastObj = resultsList->FirstObject = newObj;
						}
						else
						{
							lastObj->Next = newObj;
							lastObj = newObj;
						}
					}

					/* FREE attribute mappings */
					ILibDestructXMLAttributeList(attribs);
				}
				else
				{
					node = node->Next;
				}
			}
		}

		resultsList->NumberReturned = NumberReturned;
		resultsList->TotalMatches = TotalMatches;
		resultsList->UpdateID = UpdateID;

		/* validate number of parsed objects against returned count */
		lastObj = resultsList->FirstObject;
		resultsList->NumberParsed = 0;
		while (lastObj != NULL)
		{
			resultsList->NumberParsed++;
			lastObj = lastObj->Next;
		}

		if ((int)resultsList->NumberParsed != (int)resultsList->NumberReturned)
		{
			printf("MSCPResponseSink_ContentDirectory_Browse: Detected mismatch with number of objects returned=%u and parsed=%d.\r\n", resultsList->NumberReturned, resultsList->NumberParsed);
		}

		/* FREE resources from XML parsing */
		ILibDestructXMLNodeList(nodeList);

		/* execute callback with results */
		MSCP_Callback_Browse(Service, User, ErrorCode, resultsList);
	}
	else
	{
		MSCP_Callback_Browse(Service, User, ErrorCode, NULL);
	}
}


static void MSCPEventSink_ContentDirectory_ContainerUpdateIDs(struct UPnPService* Service,char* ContainerUpdateIDs)
{
	printf("MSCP Event from %s/ContentDirectory/ContainerUpdateIDs: %s\r\n",Service->Parent->FriendlyName,ContainerUpdateIDs);
	UNSUPPORTED_BY_CP;
}

static void MSCPEventSink_ContentDirectory_SystemUpdateID(struct UPnPService* Service,unsigned int SystemUpdateID)
{
	printf("MSCP Event from %s/ContentDirectory/SystemUpdateID: %u\r\n",Service->Parent->FriendlyName,SystemUpdateID);
	UNSUPPORTED_BY_CP;
}


/* Called whenever a new device on the correct type is discovered */
static void MSCP_UPnPSink_DeviceAdd(struct UPnPDevice *device)
{
	printf("MSCP Device Added: %s\r\n", device->FriendlyName);
	
	if (MSCP_Callback_DeviceAdd != NULL)
	{
		MSCP_Callback_DeviceAdd(device);
	}
}

/* Called whenever a discovered device was removed from the network */
static void MSCP_UPnPSink_DeviceRemove(struct UPnPDevice *device)
{
	printf("MSCP Device Removed: %s\r\n", device->FriendlyName);

	if (MSCP_Callback_DeviceRemove != NULL)
	{
		MSCP_Callback_DeviceRemove(device);
	}
}

/***********************************************************************************************************************
 *	END: UPnP Callback Sinks
 ***********************************************************************************************************************/





/***********************************************************************************************************************
 *	BEGIN: API method implementations
 ***********************************************************************************************************************/
void MSCP_DestroyResultsList (struct MSCP_ResultsList *resultsList)
{
	struct MSCP_MediaObject *obj;
	struct MSCP_MediaObject *nextObj;
	struct MSCP_MediaResource *res;
	struct MSCP_MediaResource *nextRes;

	obj = resultsList->FirstObject;

	while (obj != NULL)
	{
		nextObj = obj->Next;

		res = obj->Res;
		while (res != NULL)
		{
			nextRes = res->Next;
			MSCP_FREE(res);
			res = nextRes;
		}

		MSCP_FREE(obj);
		obj = nextObj;
	}

	MSCP_FREE (resultsList);
}

void * MSCP_Init(void *chain, 
			   MSCP_Fn_Result_Browse callbackBrowse, 
			   CP_Fn_Device_Add callbackDeviceAdd,
			   CP_Fn_Device_Remove callbackDeviceRemove)
{
	MSCP_Callback_Browse = callbackBrowse;
	MSCP_Callback_DeviceAdd = callbackDeviceAdd;
	MSCP_Callback_DeviceRemove = callbackDeviceRemove;

	/* Event callback function registration code */
	MSCP_EventCallback_ConnectionManager_SourceProtocolInfo=&MSCPEventSink_ConnectionManager_SourceProtocolInfo;
	MSCP_EventCallback_ConnectionManager_SinkProtocolInfo=&MSCPEventSink_ConnectionManager_SinkProtocolInfo;
	MSCP_EventCallback_ConnectionManager_CurrentConnectionIDs=&MSCPEventSink_ConnectionManager_CurrentConnectionIDs;
	MSCP_EventCallback_ContentDirectory_SystemUpdateID=&MSCPEventSink_ContentDirectory_SystemUpdateID;

	/* create the underlying UPnP control point stack */
	return MSCP_CreateControlPoint(chain, &MSCP_UPnPSink_DeviceAdd, &MSCP_UPnPSink_DeviceRemove);
}

void MSCP_Invoke_Browse(void *serviceObj, struct MSCP_BrowseArgs *args)
{
	char *browseFlagString;

	if (args->BrowseFlag == MSCP_BrowseFlag_Metadata)
	{
		browseFlagString = MSCP_BROWSE_FLAG_METADATA_STRING;
	}
	else
	{
		browseFlagString = MSCP_BROWSE_FLAG_CHILDREN_STRING;
	}

	MSCP_Invoke_ContentDirectory_Browse
		(
		serviceObj, 
		MSCPResponseSink_ContentDirectory_Browse, 
		args, 
		args->ObjectID, 
		browseFlagString, 
		args->Filter, 
		args->StartingIndex, 
		args->RequestedCount, 
		args->SortCriteria
		);
}

struct MSCP_MediaResource* MSCP_SelectBestIpNetworkResource(const struct MSCP_MediaObject *mediaObj, const char *protocolInfoSet, int *ipAddressList, int ipAddressListLen)
{
	struct MSCP_MediaResource* retVal = NULL, *res;
	long ipMatch = 0, protInfoMatch = 0, bestIpMatch = 0, bestProtInfoMatch = 0;
	int protInfoCount = 0;
	char *protInfoSet;
	int protInfoSetStringSize, protInfoSetStringLen;
	int i, pi;
	short finding;
	char **protInfos;
	char *protocol, *network, *mimeType, *info;
	int protocolLen, networkLen, mimeTypeLen, infoLen;
	char *resprotocol, *resnetwork, *resmimeType, *resinfo;
	int resprotocolLen, resnetworkLen, resmimeTypeLen, resinfoLen;
	int posIpByteStart, posIpByteLength, posIpByteLength2;
	long ip1, ip2, ip3, ip4;
	unsigned long ip;
	char *rp;
	int cmpIp1, cmpIp2;

	/*
	 *	copy the list of protocolInfo strings into protInfoSet
	 */
	protInfoSetStringLen = (int) strlen(protocolInfoSet);
	protInfoSetStringSize = protInfoSetStringLen + 1;
	protInfoSet = (char*) MALLOC (protInfoSetStringSize);
	memcpy(protInfoSet, protocolInfoSet, protInfoSetStringSize);
	protInfoSet[protInfoSetStringLen] = '\0';

	/*
	 *	Replace all commas in protInfoSet to NULL chars
	 *	and count the number of protocolInfo strings in the set.
	 *	Method only works if the protocolInfo are listed in form: A,B,...Z.
	 *	If we receive malformed sets like A,,B then results are undefined.
	 */
	protInfoCount = 1;
	for (i=0; i < protInfoSetStringLen; i++)
	{
		if (protInfoSet[i] == ',')
		{
			protInfoSet[i] = '\0';
			protInfoCount++;
		}
	}

	/*
	 *	create an array of char** that will allow us easy access 
	 *	to individual protocolInfo. Also redo the count
	 *	in case of inaccuracies due to bad formatting.
	 */
	protInfos = (char**) MALLOC (sizeof(char*) * protInfoCount);
	pi = 0;
	finding = 0;
	for (i=0; i < protInfoSetStringLen; i++)
	{
		if ((finding == 0) && (protInfoSet[i] != '\0'))
		{
			protInfos[pi] = &(protInfoSet[i]);
			pi++;
			finding = 1;
			protInfoCount++;
		}
		else if ((finding == 1) && (protInfoSet[i] == '\0'))
		{
			finding = 0;
		}
	}
	if (pi < protInfoCount) { protInfoCount = pi; }


	/*
	 *	Iterate through the different resources and track the best match.
	 */
	res = mediaObj->Res;
	while (res != NULL)
	{
		/* the protocolInfo strings listed first have higher precedence */
		protInfoMatch = protInfoCount + 1;

		/* calculate a match value against protocolInfo */
		for (i=0; i < protInfoCount; i++)
		{
			protInfoMatch--;
			
			/*
			 * get pointers and lengths for the fields in the protocolInfo 
			 */
			protocol = protInfos[i];
			protocolLen = IndexOf(protocol, ":");

			network = protocol + protocolLen + 1;
			networkLen = IndexOf(network, ":");

			mimeType = network + networkLen + 1;
			mimeTypeLen = IndexOf(mimeType, ":");

			info = mimeType + mimeTypeLen + 1;
			infoLen = (int) strlen(info);

			/*
			 * get pointers and lengths for the fields in the resource's protocolInfo 
			 */
			resprotocol = res->ProtocolInfo;
			resprotocolLen = IndexOf(resprotocol, ":");

			resnetwork = resprotocol + resprotocolLen + 1;
			resnetworkLen = IndexOf(resnetwork, ":");

			resmimeType = resnetwork + resnetworkLen + 1;
			resmimeTypeLen = IndexOf(resmimeType, ":");

			resinfo = resmimeType + resmimeTypeLen + 1;
			resinfoLen = (int) strlen(resinfo);
			
			/* compare each of the fields */

			if (strnicmp(protocol, resprotocol, MIN(protocolLen, resprotocolLen)) == 0)
			{
				if (strnicmp(network, resnetwork, MIN(networkLen, resnetworkLen)) == 0)
				{
					if (
						((mimeType[0] == '*') && (mimeType[1] == ':'))
						|| (strnicmp(mimeType, resmimeType, MIN(mimeTypeLen, resmimeTypeLen)) == 0)
						)
					{
						if (
							((info[0] == '*') && (info[1] == '\0'))
							|| (strnicmp(info, resinfo, MIN(infoLen, resinfoLen)) == 0)
							)
						{
							/*
							 *	If we get here then protocolInfo matches.
							 *	Go ahead and break since protInfoMatch is
							 *	set on every iteration.
							 */
							break;
						}
					}
				}
			}
		}

		/*
		 *	At this point, we have calculated the protInfoMatch value,
		 *	but we still need to determine if the resource has a good
		 *	chance of being routable given a particular target
		 *	IP address.
		 */

		ipMatch = 0;

		/*
		 *	Convert text-based IP address to in-order int form.
		 *	Since the res->URI is assumed to be a valid URI,
		 *	it will have the form scheme://[ip address]:....
		 */
		posIpByteStart = IndexOf(res->Uri, "://") + 3;
		rp = res->Uri + posIpByteStart;
		posIpByteLength = IndexOf(rp, ".");
		ILibGetLong(rp, posIpByteLength, &ip1);

		rp += (posIpByteLength+1);
		posIpByteLength = IndexOf(rp, ".");
		ILibGetLong(rp, posIpByteLength, &ip2);
		
		rp += (posIpByteLength+1);
		posIpByteLength = IndexOf(rp, ".");
		ILibGetLong(rp, posIpByteLength, &ip3);

		rp += (posIpByteLength+1);
		posIpByteLength = IndexOf(rp, ":");
		posIpByteLength2 = IndexOf(rp, "/");
		ILibGetLong(rp, MIN(posIpByteLength,posIpByteLength2), &ip4);

		/*
		 *	Convert each network byte into a 32-bit integer,
		 *	then perform a bit mask comparison against the target ip address.
		 */
		ip = (int) (ip1 | (ip2 << 8) | (ip3 << 16) | (ip4 << 24));

		cmpIp1 = htonl(ip);

		for (i=0; i < ipAddressListLen; i++)
		{
			cmpIp2 = htonl(ipAddressList[i]);
			ipMatch = cmpIp1 & cmpIp2;

			if (
				((unsigned)ipMatch > (unsigned)bestIpMatch) ||
				((ipMatch == bestIpMatch) && (protInfoMatch > bestProtInfoMatch))
				)
			{
				retVal = res;
				bestIpMatch = ipMatch;
				bestProtInfoMatch = protInfoMatch;
			}
		}

		res = res->Next;
	}

	FREE (protInfos);
	FREE (protInfoSet);

	return retVal;
}

void MSCP_Uninit()
{
	MSCP_Callback_Browse = NULL;
	MSCP_Callback_DeviceAdd = NULL;
	MSCP_Callback_DeviceRemove = NULL;
}

/***********************************************************************************************************************
 *	END: API method implementations
 ***********************************************************************************************************************/
