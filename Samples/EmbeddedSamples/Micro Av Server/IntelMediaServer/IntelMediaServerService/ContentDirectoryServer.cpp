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

#include "Contentdirectoryserver.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>

#include "ResourceProvider.h"

extern "C"
{
	#include "UpnpGlueLayer.h"
	#include "CdsErrors.h"
	#include "CdsObjectToDidl.h"
}

enum _ContentDirectoryServer_QueryType
{
	_ContentDirectoryServer_QueryType_BrowseM_ObjectID,
	_ContentDirectoryServer_QueryType_BrowseM_DynamicContainer,
	_ContentDirectoryServer_QueryType_BrowseM_SpecialReference,
	_ContentDirectoryServer_QueryType_BrowseM_DynamicReference,

	_ContentDirectoryServer_QueryType_BrowseD_ObjectID,
	_ContentDirectoryServer_QueryType_BrowseD_DynamicContainer,
	_ContentDirectoryServer_QueryType_BrowseD_SpecialReference,
	_ContentDirectoryServer_QueryType_BrowseD_DynamicReference,

	_ContentDirectoryServer_QueryType_Search_ObjectID,
	_ContentDirectoryServer_QueryType_Search_DynamicContainer
};

ContentDirectoryServer::ContentDirectoryServer(MediaDatabase* db, void *QuitLock)
{
	int i;

	this->m_database = db;

	// ObjectID for the AllMedia container
	this->m_allMediaObjectID = MEDIA_DB_SCK_ALLITEMS;

	// Initialize resource providers
	sem_init(&(this->m_ResourceProviderLock), 0, 1);
	for (i=0; i < MAX_RES_PROVIDERS; i++)
	{
		this->m_ResourceProviders[i] = NULL;
	}

	// create the UPnP MediaServer and set things up so that queries are dispatched back to this object
	UGL_CallbackOnQuery = Sink_OnQuery;
	UGL_CallbackOnStatsChanged = Sink_OnStatsChanged;
	UGL_StartUPnP(this,QuitLock);
}


ContentDirectoryServer::~ContentDirectoryServer(void)
{
	int i;

	UGL_StopUPnP();
	sem_destroy(&(this->m_ResourceProviderLock));
	for (i=0; i < MAX_RES_PROVIDERS; i++)
	{
		delete this->m_ResourceProviders[i];
		this->m_ResourceProviders[i] = NULL;
	}
}


unsigned int ContentDirectoryServer::ConvertMediaClass(long classValue)
{
	unsigned int retVal = 0;

	switch (classValue)
	{
	case MEDIA_DB_CL_C_STORAGE:
		retVal = CDS_MEDIACLASS_STRGFOL;
		break;
	
	case MEDIA_DB_CL_C_PLAYLIST:
		retVal = CDS_MEDIACLASS_PLAYLISTCONTAINER;
		break;

	case MEDIA_DB_CL_C_SPECIAL:
		retVal = CDS_MEDIACLASS_CONTAINER;
		break;

	case MEDIA_DB_CL_IMAGE:
		retVal = CDS_MEDIACLASS_IMAGEITEM;
		break;

	case MEDIA_DB_CL_AUDIO:
		retVal = CDS_MEDIACLASS_AUDIOITEM;
		break;

	case MEDIA_DB_CL_VIDEO:
		retVal = CDS_MEDIACLASS_VIDEOITEM;
		break;

	case MEDIA_DB_CL_OTHER:
		retVal = CDS_MEDIACLASS_ITEM;
		break;
	}

	return retVal;
}


void ContentDirectoryServer::ConvertObjectID(struct _ContentDirectoryServer_ObjectId *objectID, char *objectIdAsString)
{
	char objid[MAX_OBJID_LEN];
	int len,i,si=0, len2=0, len3=0;
	int ps[3] = {-1, -1, -1};

	// replace all '/' with null characters
	len = (int) strlen(objectIdAsString);
	for (i=0; i <= len; i++)
	{
		if (objectIdAsString[i] == '/')
		{
			objid[i] = '\0';
			if (si < 3) { ps[si] = i+1; }
			if (si == 2) { len3 = ps[2] - ps[1]; }
			if (si == 1) { len2 = ps[1] - ps[0]; }
			si++;
		}
		else
		{
			objid[i] = objectIdAsString[i];
		}
	}

	//	At this point, ps[] knows where strings begin


	// Get the base object ID from the first null-terminated portion.
	sscanf(objid, "%u", &(objectID->Base));
	if (ps[0] > 0)
	{
		if (ps[1] > 0)
		{
			if ((ps[1] - ps[0]) == 1)
			{
				if (ps[2] > 0)
				{
					// If two / in a row and something follows, then it's a playlist entry.
					objectID->Format = _ContentDirectoryServer_ObjIdFormat_PlaylistEntry;

					// Get the objectID from the 3rd null-termated portion
					sscanf(objid+ps[1], "%u", &(objectID->PlaylistID));

					// Get the objectID from the 4th null-termated portion
					sscanf(objid+ps[2], "%u", &(objectID->UnderlyingObjectID));
				}
				else
				{
					// If two / in a row, then it's a playlist container.
					objectID->Format = _ContentDirectoryServer_ObjIdFormat_PlaylistContainer;

					// Get the objectID from the 3rd null-termated portion
					sscanf(objid+ps[1], "%u", &(objectID->PlaylistID));
				}
			}
			else
			{
				// maps to a child of a dynamic container
				objectID->Format = _ContentDirectoryServer_ObjIdFormat_DynamicReference;

				// Get the dynamic name by memcpying the second portion
				memcpy(objectID->DynamicName, objid+ps[0], len2);
				
				// Get the objectID from the 3rd null-termated portion
				sscanf(objid+ps[1], "%u", &(objectID->UnderlyingObjectID));
			}
		}
		else
		{
			// Determine if the objectID maps to a child item of a special container
			// or dynamic container child of a special container.
			if (*(objid+ps[0]) == '"')
			{
				objectID->Format = _ContentDirectoryServer_ObjIdFormat_DynamicContainer;

				// change the starting index and size for the dynamic container name
				//ps[1] = ps[1]+1;		// ???
				//len2--;				// ???

				// allocate memory for the dynamic container's name and copy
				memcpy(objectID->DynamicName, objid+ps[0]+1, len - ps[0] - 2);

				// be sure to null terminate because the last char copied will be a quote "
				objectID->DynamicName[len - ps[0] - 2] = '\0';

				// no underlying object
				objectID->UnderlyingObjectID = -1;
			}
			else
			{
				objectID->Format = _ContentDirectoryServer_ObjIdFormat_SpecialReference;

				// Get the objectID from the 2nd null-termated portion
				sscanf(objid+ps[0], "%u", &(objectID->UnderlyingObjectID));

				// no dynamic container name
				objectID->DynamicName[0] = '\0';
			}
		}
	}
	else
	{
		objectID->Format = _ContentDirectoryServer_ObjIdFormat_Standard;
		
		// Figure out if this is one of the special query containers
		if (objectID->Base == 1 || (objectID->Base >= 5 && objectID->Base <= 12)) objectID->Format = _ContentDirectoryServer_ObjIdFormat_DynamicContainer;

		// Nothing else to parse out
		objectID->DynamicName[0] = '\0';
		objectID->UnderlyingObjectID = -1;
	}
}


char* ContentDirectoryServer::GetObjectID(MediaItemData* dbEntry, SpecialContainerData* spcEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	char *retVal = NULL;

	//
	//	Use malloc for retVal as the memory will be deallocated by
	//	the CDS_DestroyObjects().
	//

	switch (cdsQuery->QueryType)
	{
	case MSL_Query_BrowseDirectChildren:
		assert(dbEntry != NULL);

		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// The objectID specified in the query is a standard object. 
			// The children's objectID values may take any format, except
			// _ContentDirectoryServer_ObjIdFormat_DynamicReference.
			
			if (dbEntry != NULL)
			{
				switch (dbEntry->Class)
				{
				case MEDIA_DB_CL_C_STORAGE:
				case MEDIA_DB_CL_C_SPECIAL:
					// Storage and special containers always use their dbEntry->Key
					// to derive their objectID.
					retVal = (char*) malloc(SMALL_OBJID_LEN);
					sprintf(retVal, "%d", dbEntry->Key);
					break;

				case MEDIA_DB_CL_C_PLAYLIST:
					// Playlist containers always use only their dbEntry->Key
					// and parentID to derive their objectID. The delimiter 
					// for a playlist container is slightly different in that 
					// it uses // instead of a /.
					retVal = (char*) malloc(MEDIUM_OBJID_LEN);
					sprintf(retVal, "%d//%d", objectID->Base, dbEntry->Key);
					break;

				case MEDIA_DB_CL_IMAGE:
				case MEDIA_DB_CL_AUDIO:
				case MEDIA_DB_CL_VIDEO:
				case MEDIA_DB_CL_OTHER:
					if (objectID->Base == this->m_allMediaObjectID)
					{
						// Individual child items of AllMedia are always
						// printed with the key specified by the dbEntry.
						// As a corollary, these entries are never viewed
						// as reference items in the database.
						retVal = (char*) malloc(SMALL_OBJID_LEN);
						sprintf(retVal, "%d", dbEntry->Key);
					}
					else
					{
						// Individual child items of any standard container are
						// printed as though they are reference items of a 
						// special container. Such reference items are 
						// printed with the parentID first, followed by the 
						retVal = (char*) malloc(MEDIUM_OBJID_LEN);
						sprintf(retVal, "%d/%d", objectID->Base, dbEntry->Key);
					}
					break;
				}
			}
			else
			{
				// The ObjectID is for a DynamicContainer
				assert(spcEntry != NULL);
				retVal = (char*) malloc(MAX_OBJID_LEN);
				sprintf(retVal, "%d/&quot;%s&quot;", objectID->Base, spcEntry->Value);
			}
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			// The objectID specified in the query is a playlist container.
			assert(dbEntry != NULL);
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d//%d/%d", objectID->Base, objectID->PlaylistID, dbEntry->Key);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// The objectID specified in the query is a dynamic container.
			// Dynamic containers only have reference items as children.
			assert(dbEntry != NULL);
			if (objectID->Base == 1)
			{   // All Items
				retVal = (char*) malloc(MAX_OBJID_LEN);
				sprintf(retVal, "%d", objectID->UnderlyingObjectID);
			}
			else
			if (objectID->Base == 6 || objectID->Base == 9 || objectID->Base == 11)
			{   // All Items of a given class
				retVal = (char*) malloc(MAX_OBJID_LEN);
				sprintf(retVal, "%d/%d", objectID->Base, objectID->UnderlyingObjectID);
			}
			else
			{
				retVal = (char*) malloc(MAX_OBJID_LEN);
				sprintf(retVal, "%d/&quot;%s&quot;/%d", objectID->Base, objectID->DynamicName, objectID->UnderlyingObjectID);
			}
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		default:
			// This code should never execute because you can't browsedirectchildren
			// on individual items.
			assert(0);
			break;
		}
		break;

	case MSL_Query_BrowseMetadata:
		// The objectID of BrowseMetadata request is determined by the
		// objectID specified in the query. 

		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			retVal = (char*) malloc(SMALL_OBJID_LEN);
			sprintf(retVal, "%d", objectID->Base);
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d//%d", objectID->Base, objectID->PlaylistID);
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d//%d/%d", objectID->Base, objectID->PlaylistID, objectID->UnderlyingObjectID);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			retVal = (char*) malloc(MAX_OBJID_LEN);
			sprintf(retVal, "%d/&quot;%s&quot;", objectID->Base, objectID->DynamicName);
			break;

		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
			retVal = (char*) malloc(MAX_OBJID_LEN);
			sprintf(retVal, "%d/%d", objectID->Base, objectID->UnderlyingObjectID);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
			retVal = (char*) malloc(MAX_OBJID_LEN);
			sprintf(retVal, "%d/&quot;%s&quot;/%d", objectID->Base, objectID->DynamicName, objectID->UnderlyingObjectID);
			break;

		default:
			// This code should never run.
			assert(0);
			break;
		}
		break;
	
	case MSL_Query_Search:
		// The objectID of every Search result will be a child item of the AllMedia container
		// because the AllMedia object is the only container that supports search.
		retVal = (char*) malloc(MEDIUM_OBJID_LEN);
		sprintf(retVal, "%d/%d", this->m_allMediaObjectID, dbEntry->Key);
		break;

	default:
		//This should never execute.
		assert(0);
		break;
	}

	if (retVal == NULL)
	{
		retVal = (char*) malloc(SMALL_OBJID_LEN);
		strcpy(retVal, "-1");
		fprintf(stderr, "WARNING: ContentDirectoryServer::GetObjectID() failed to acquire a valid ObjectID.\r\n");
	}

	return retVal;
}


char* ContentDirectoryServer::GetParentID(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	char *retVal = NULL;

	//
	//	Use malloc for retVal as the memory will be deallocated by
	//	the CDS_DestroyObjects().
	//
	switch (cdsQuery->QueryType)
	{
	case MSL_Query_BrowseDirectChildren:
		// The parentID of BrowseDirectChildren request is determined by the
		// objectID specified in the query. 

		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			retVal = (char*) malloc(SMALL_OBJID_LEN);
			sprintf(retVal, "%d", objectID->Base);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			if (objectID->Base <= 4 || objectID->Base == 6 || objectID->Base == 9 || objectID->Base == 11)
			{
				retVal = (char*) malloc(SMALL_OBJID_LEN);
				sprintf(retVal, "%d", objectID->Base);
			}
			else
			{
				retVal = (char*) malloc(MAX_OBJID_LEN);
				sprintf(retVal, "%d/&quot;%s&quot;", objectID->Base, objectID->DynamicName);
			}
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d//%d", objectID->Base, objectID->PlaylistID);
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		default:
			// This code should never run because you can't BrowseDirectChildren
			// on an item.
			assert(0);
			break;
		}
		break;

	case MSL_Query_BrowseMetadata:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// The parentID of BrowseMetadata request on a standard
			// object is determined by the dbEntry's ParentKey field.
			retVal = (char*) malloc(SMALL_OBJID_LEN);
			sprintf(retVal, "%d", dbEntry->ParentKey);
			break;

		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
			// The parentID of a BrowseMetadata request on a child of a special container
			// is determined by the special container's ID
			// embedded in the specified objectID.

			// INTENTIONAL FALL-THROUGH

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// The parentID of a BrowseMetadata request on a dynamic
			// container is determined by the parent special container
			// embedded in the specified objectID.

			// INTENTIONAL FALL-THROUGH

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			// The parentID of a BrowseMetadata request on a dynamic
			// container is determined by the special or storage container
			// embedded in the specified objectID.
			retVal = (char*) malloc(SMALL_OBJID_LEN);
			sprintf(retVal, "%d", objectID->Base);
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
			// The parentID of a BrowseMetadata request on a playlist entry
			// is determined by the playlist container ID
			// embedded in the specified objectID.
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d//%d", objectID->Base, objectID->PlaylistID);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
			// The parentID of a BrowseMetadata request on a dynamic container's child 
			// is determined by the dynamic container's ID embedded in the specified objectID.
			retVal = (char*) malloc(MEDIUM_OBJID_LEN);
			sprintf(retVal, "%d/&quot;%s&quot;", objectID->Base, objectID->DynamicName);
			break;

		default:
			// This code should never run
			assert(0);
			break;
		}
		break;
	
	case MSL_Query_Search:
		// The parentID of every Search result will be the AllMedia object
		// because the AllMedia object is the only container that supports
		// search.
		retVal = (char*) malloc(SMALL_OBJID_LEN);
		sprintf(retVal, "%d", this->m_allMediaObjectID);
		break;

	default:
		// This should never run.
		assert(0);
		break;
	}

	if (retVal == NULL)
	{
		retVal = (char*) malloc(SMALL_OBJID_LEN);
		strcpy(retVal, "-1");
		fprintf(stderr, "WARNING: ContentDirectoryServer::GetParentID() failed to acquire a valid ParentID.\r\n");
	}

	return retVal;
}


char* ContentDirectoryServer::GetRefID(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	char *retVal = NULL;

	switch (cdsQuery->QueryType)
	{
	case MSL_Query_BrowseMetadata:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
			// All BrowseMetadata requests with an item objectID
			// that is not a standard database key should easily
			// map to a database key by parsing out the underlying 
			// object's ID from the specified objectID.

			assert(objectID->UnderlyingObjectID == dbEntry->Key);

			retVal = (char*) malloc(SMALL_OBJID_LEN);
			sprintf(retVal, "%d", dbEntry->Key);
			break;

		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// All BrowseMetadata requests on a database-mapped
			// objectID may be referring to either an item or
			// a container. However, if it is an item then
			// we know that the objectID is directly referencing
			// the item through the AllMedia container. Therefore,
			// the refID remains null as no item in AllMedia
			// is ever a reference item.
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// Containers never ever have refID values.
			break;

		default:
			// This code should never execute.
			assert(0);
			break;
		}
		break;

	case MSL_Query_BrowseDirectChildren:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
			// This code should never execute because this method
			// should never get called because a BrowseDirectChildren
			// can never be performed on an individual item.
			assert(0);
			break;

		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// The only container that has children that are not
			// reference items is the AllMedia container.
			
			if (objectID->Base != this->m_allMediaObjectID)
			{
				// Child items of this container are reference items.
				// All refID's simply use the dbEntry->Key value.
				retVal = (char*) malloc(SMALL_OBJID_LEN);
				sprintf(retVal, "%d", dbEntry->Key);
			}
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// Containers never ever have refID values.
			break;

		default:
			// This code should never execute.
			assert(0);
			break;
		}
		break;

	case MSL_Query_Search:
		// The only container that is searchable is AllMedia.
		// If searching AllMedia, then we know that no children
		// in AllMedia have refID values.

		// This code should not execute unless AllMedia was specified
		// as the start container for the query.
		assert(objectID->Base == this->m_allMediaObjectID);
		break;

	default:
		//This code should never execute
		assert(0);
		break;
	}

	return retVal;
}

// +upnp:genre
// -upnp:genre+dc:title
// +upnp:genre-dc:title
unsigned int ContentDirectoryServer::GetSortBitString(struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	unsigned int retVal = 0;
	int len, i, pos;

	//0: expecting + or -; 1: expecting name
	int expecting = 0;
	int ascending = MEDIA_DB_SORT_DESCENDING;
	char c;

	len = (int) strlen (cdsQuery->SortCriteria);

	//
	//	Parse the sort criteria string and determine the sorting
	//	order criteria. We can have up to 4 sort orders.
	//

	i = 0;
	pos = 0;
	while (i < len)
	{
		c = cdsQuery->SortCriteria[i];
		
		switch (c)
		{
		case '+':
			ascending = 0;
			break;

		case '-':
			ascending = MEDIA_DB_SORT_DESCENDING;
			break;

		case 'd':
			if (strnicmp(&(cdsQuery->SortCriteria[i]), UGL_SortField_Title, UGL_SortField_TitleLen) == 0)
			{
				retVal += ((MEDIA_DB_SORT_TITLE | ascending) << (pos * 8));
				pos++;
				i += UGL_SortField_TitleLen-1;
				ascending = MEDIA_DB_SORT_DESCENDING;
			}
			else if (strnicmp(&(cdsQuery->SortCriteria[i]), UGL_SortField_Creator, UGL_SortField_CreatorLen) == 0)
			{
				retVal += ((MEDIA_DB_SORT_CREATOR | ascending) << (pos * 8));
				pos++;
				i += UGL_SortField_CreatorLen-1;
				ascending = MEDIA_DB_SORT_DESCENDING;
			}
			break;

		case 'u':
			if (strnicmp(&(cdsQuery->SortCriteria[i]), UGL_SortField_Album, UGL_SortField_AlbumLen) == 0)
			{
				retVal += ((MEDIA_DB_SORT_ALBUM | ascending) << (pos * 8));
				pos++;
				i += UGL_SortField_AlbumLen-1;
				ascending = MEDIA_DB_SORT_DESCENDING;
			}
			else if (strnicmp(&(cdsQuery->SortCriteria[i]), UGL_SortField_Genre, UGL_SortField_GenreLen) == 0)
			{
				retVal += ((MEDIA_DB_SORT_GENRE | ascending) << (pos * 8));
				pos++;
				i += UGL_SortField_GenreLen-1;
				ascending = MEDIA_DB_SORT_DESCENDING;
			}
			break;
		}

		i++;

		if (pos > 3)
		{
			break;
		}
	}

	if (pos == 0)
	{
		// use the default sorting
		if (objectID->Format == _ContentDirectoryServer_ObjIdFormat_PlaylistContainer)
		{
			// playlists should return their children ordered by their objectID key
			retVal = MEDIA_DB_SORT_KEY;
		}
		else
		{
			// all others should return their children ordered by their title
			retVal = MEDIA_DB_SORT_TITLE;
		}
	}

	return retVal;
}


void ContentDirectoryServer::GetQueryResults(unsigned int sorting, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery, /*OUT*/ MediaItemData **dbEntry, /*OUT*/ SpecialContainerData **spcEntry)
{
	//enum MEDIA_DB_CLASS_ENUM mediaClass;
	//enum MEDIA_DB_SPECIALCONTAINER_ENUM spContainerType;
	
	//
	//	This method works for ANY query.
	//
	*dbEntry = NULL;
	*spcEntry = NULL;

	switch (cdsQuery->QueryType)
	{
	case MSL_Query_BrowseMetadata:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// Standard media objects map directly to entries in a database.
			// Note that the database could return metadata for a container
			// or an item. 
			//
			// If an item was retrieved, we care about all of the metadata.
			// If a container was retrieved, we need to know the number of
			// children in the container along with any appropriate title,
			// creator, genre, and album metadata.

			//TODO: Query the database for a single entry with key=objectID->Base
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			// Playlist containers (almost) map directly to entries in a database.
			//
			// We want to know the playlist's title, (optionally creator, genre, album)
			// metadata along with the number of entries in the playlist.

			//TODO: Query the database for a single entry with key=objectID->PlaylistID
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
			// Playlist entries (almost) map directly to entries in a database.
			// Please note that UnderlyingObjectID actually maps to a unique entry
			// in the database. This is slightly different than reference items
			// in a SpecialContainer or a DynamicContainer because the child items
			// of those containers don't have objectIDs that map to an actual 
			// database entry.
			//
			// What we want to do is ensure that the metadata for the playlist
			// entry is provided, including appropriate title, creator, genre, album.

			//TODO: Query the database for a single entry with key=objectID->UnderlyingObjectID
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// Dynamic containers don't map directly to an entry in the database.
			// We essentially need to query the database for the
			// SpecialContainer mapped by objectID->Base. (This amounts to 
			// finding the set of values for a particular column.) Then,
			// we find the value in the column that is equal to the value
			// in objectID->DynamicName.
			//
			// What we want from the database is the number of child 
			// items in the dynamic container along with title. 
			// (album, genre, creator may be present)

			// TODO: see above
			break;

		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
			// Child items of a special container almost map into the database.
			// What we need is the usual metadata for an item.

			//TODO: Query the database for a single database entry mapped by key=objectID->UnderlyingObjectID
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
			// Child items of a dynamic container almost map into the database.
			// What we need is the usual metadata for an item.

			//TODO: Query the database for a single database entry mapped by key=objectID->UnderlyingObjectID
			break;

		default:
			//This should never execute
			assert(0);
			break;
		}
		break;

	case MSL_Query_BrowseDirectChildren:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
			// Get all objects in the database that have the parentKey=objectID->Base.
			*dbEntry = this->m_database->QueryContainerItems(objectID->Base, sorting);
			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
			// Get the child objects of a dynamic container.
			// In order for the query to work, we need to know 
			// the type of content (audio, video, etc.).

			if (objectID->Base == MEDIA_DB_SCK_ALLITEMS) // All Items
			{
				*dbEntry = this->m_database->QueryAllItems(sorting, MEDIA_DB_PL_NO);
			}
			else if (objectID->Base == MEDIA_DB_SCK_MUSIC_ALL) // All Audio
			{
				*dbEntry = this->m_database->QueryAllItemsOfClass(MEDIA_DB_CL_AUDIO, sorting, MEDIA_DB_PL_NO);
			}
			else if (objectID->Base == MEDIA_DB_SCK_PICTURES_ALL) // All Pictures
			{
				*dbEntry = this->m_database->QueryAllItemsOfClass(MEDIA_DB_CL_IMAGE, sorting, MEDIA_DB_PL_NO);
			}
			else if (objectID->Base == MEDIA_DB_SCK_VIDEO_ALL) // All Videos
			{
				*dbEntry = this->m_database->QueryAllItemsOfClass(MEDIA_DB_CL_VIDEO, sorting, MEDIA_DB_PL_NO);
			}
			else if (objectID->Base == MEDIA_DB_SCK_MUSIC_ALBUM)
			{
				if (objectID->DynamicName[0] == '\0')
				{
					*spcEntry = this->m_database->QuerySpecialContainer(MEDIA_DB_SC_ALBUM,MEDIA_DB_CL_AUDIO);
				}
				else
				{
					*dbEntry = this->m_database->QuerySpecialContainerItems(MEDIA_DB_SC_ALBUM,objectID->DynamicName,MEDIA_DB_CL_AUDIO, sorting, MEDIA_DB_PL_NO);
				}
			}
			else if (objectID->Base == MEDIA_DB_SCK_MUSIC_ARTIST)
			{
				if (objectID->DynamicName[0] == '\0')
				{
					*spcEntry = this->m_database->QuerySpecialContainer(MEDIA_DB_SC_CREATOR,MEDIA_DB_CL_AUDIO);
				}
				else
				{
					*dbEntry = this->m_database->QuerySpecialContainerItems(MEDIA_DB_SC_CREATOR,objectID->DynamicName,MEDIA_DB_CL_AUDIO, sorting, MEDIA_DB_PL_NO);
				}
			}
			else if (objectID->Base == MEDIA_DB_SCK_MUSIC_GENRE)
			{
				if (objectID->DynamicName[0] == '\0')
				{
					*spcEntry = this->m_database->QuerySpecialContainer(MEDIA_DB_SC_GENRE,MEDIA_DB_CL_AUDIO);
				}
				else
				{
					*dbEntry = this->m_database->QuerySpecialContainerItems(MEDIA_DB_SC_GENRE,objectID->DynamicName,MEDIA_DB_CL_AUDIO, sorting, MEDIA_DB_PL_NO);
				}
			}
			else if (objectID->Base == MEDIA_DB_SCK_VIDEO_ARTIST)
			{
				if (objectID->DynamicName[0] == '\0')
				{
					*spcEntry = this->m_database->QuerySpecialContainer(MEDIA_DB_SC_CREATOR,MEDIA_DB_CL_VIDEO);
				}
				else
				{
					*dbEntry = this->m_database->QuerySpecialContainerItems(MEDIA_DB_SC_CREATOR,objectID->DynamicName,MEDIA_DB_CL_VIDEO, sorting, MEDIA_DB_PL_NO);
				}
			}
			else if (objectID->Base == MEDIA_DB_SCK_VIDEO_GENRE)
			{
				if (objectID->DynamicName[0] == '\0')
				{
					*spcEntry = this->m_database->QuerySpecialContainer(MEDIA_DB_SC_GENRE,MEDIA_DB_CL_VIDEO);
				}
				else
				{
					*dbEntry = this->m_database->QuerySpecialContainerItems(MEDIA_DB_SC_GENRE,objectID->DynamicName,MEDIA_DB_CL_VIDEO, sorting, MEDIA_DB_PL_NO);
				}
			}

			break;

		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
		default:
			//This should never execute
			assert(0);
			break;
		}
		break;

	case MSL_Query_Search:
		switch (objectID->Format)
		{
		case _ContentDirectoryServer_ObjIdFormat_Standard:
			// This should be true
			assert (objectID->Base == this->m_allMediaObjectID);
			
			//TODO: Convert CDS SearchCriteria query into a database query
			//*item = this->m_database->QueryAllItems(
			break;

		case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
		case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
		case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
		case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
		case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		default:
			//This should never execute
			assert(0);
			break;
		}
		break;

	default:
		// This should never execute
		assert(0);
		break;
	}
}


struct CdsMediaResource* ContentDirectoryServer::GetResources(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	struct CdsMediaResource *retVal = NULL, *last, *newRes;
	int i;

	sem_wait(&(this->m_ResourceProviderLock));

	// Iterate through the list of resource providers and obtain resources from each
	for (i=0; i < MAX_RES_PROVIDERS; i++)
	{
		if (this->m_ResourceProviders[i] != NULL)
		{
			// obtain zero or more resources from this provider
			newRes = this->m_ResourceProviders[i]->GetResources(dbEntry, cdsQuery->IpAddrList, cdsQuery->IpAddrListLen);

			if (newRes != NULL)
			{
				// Ensure that retVal and last point to the proper resource.
				// retVal points to the head of the list of resources.
				// last points to the tail of the list of resources.
				if (retVal == NULL)
				{
					retVal = last = newRes;
				}
				else
				{
					last->Next = newRes;
				}
				while (last->Next != NULL)
				{
					last = last->Next;
				}
			}
		}
	}
	sem_post(&(this->m_ResourceProviderLock));

	return retVal;
}

void* ContentDirectoryServer::GetWebServerToken()
{
	return UGL_GetServerToken();
}

void ContentDirectoryServer::Handle_OnQuery(struct MSL_CdsQuery *cdsQuery)
{
	enum Enum_CdsErrors error = CdsError_None;
	unsigned int filter, sorting;
	struct _ContentDirectoryServer_ObjectId objID;
	

	// Get the bitstring that describes the desired metadata 
	// properties to include in the DIDL-Lite response.
	filter = CdsToDidl_GetFilterBitString(cdsQuery->Filter);

	// Parse the ObjectID into a struct for use later.
	this->ConvertObjectID(&objID, cdsQuery->ObjectID);

	// Determine the sorting criteria for our query
	sorting = this->GetSortBitString(&objID, cdsQuery);

	switch (objID.Format)
	{
	case _ContentDirectoryServer_ObjIdFormat_Standard:
	case _ContentDirectoryServer_ObjIdFormat_DynamicContainer:
	case _ContentDirectoryServer_ObjIdFormat_PlaylistContainer:
		if (cdsQuery->QueryType == MSL_Query_Search)
		{
			if (
				(objID.Base == m_allMediaObjectID) 
				&& (objID.Format == _ContentDirectoryServer_ObjIdFormat_Standard)
				)
			{
				// Respond to the query
				this->RespondToQuery(sorting, &objID, cdsQuery, filter);
			}
			else
			{
				// no other containers can do a search
				this->RespondEmptyDidl(cdsQuery);
			}
		}
		else
		{
			error = this->RespondToQuery(sorting, &objID, cdsQuery, filter);
		}
		break;

	case _ContentDirectoryServer_ObjIdFormat_PlaylistEntry:
		if (cdsQuery->QueryType == MSL_Query_BrowseDirectChildren)
		{
			// cannot browse direct children on playlist entries
			error = CdsError_NoSuchContainer;
		}
		else
		{
			// Use the standard browse response
			error = this->RespondToQuery(sorting, &objID, cdsQuery, filter);
		}
		break;

	case _ContentDirectoryServer_ObjIdFormat_SpecialReference:
	case _ContentDirectoryServer_ObjIdFormat_DynamicReference:
		if (
			(cdsQuery->QueryType == MSL_Query_BrowseDirectChildren)
			|| (cdsQuery->QueryType == MSL_Query_Search)
			)
		{
			// cannot browse direct children on reference items
			error = CdsError_NoSuchContainer;
		}
		else if (cdsQuery->QueryType == MSL_Query_BrowseMetadata)
		{
			// Use the standard browse response
			error = this->RespondToQuery(sorting, &objID, cdsQuery, filter);
		}
		break;

	default:
		// This code should never run.
		assert(0);
		break;
	}

	//	If an error occurred, then respond with the appropriate error code and error reason.
	if (error != CdsError_None)
	{
		MSL_ForResponse_RespondError(cdsQuery, CDS_ErrorCodes[error], CDS_ErrorStrings[error]);
	}
}

void ContentDirectoryServer::Handle_OnStatsChanged(struct MSL_Stats *stats)
{
	//TODO: Handle_OnStatsChanged
}

int ContentDirectoryServer::ResourceProviderRegister(ResourceProvider *resProvider)
{
	int retVal = 0, i;

	sem_wait(&(this->m_ResourceProviderLock));
	for (i=0; i < MAX_RES_PROVIDERS; i++)
	{
		if (this->m_ResourceProviders[i] == NULL)
		{
			this->m_ResourceProviders[i] = resProvider;
			retVal = 1;
			break;
		}
	}
	sem_post(&(this->m_ResourceProviderLock));

	return retVal;
}

void ContentDirectoryServer::ResourceProviderUnregister(ResourceProvider *resProvider)
{
	int i;

	sem_wait(&(this->m_ResourceProviderLock));
	for (i=0; i < MAX_RES_PROVIDERS; i++)
	{
		if (this->m_ResourceProviders[i] == resProvider)
		{
			this->m_ResourceProviders[i] = NULL;
			break;
		}
	}
	sem_post(&(this->m_ResourceProviderLock));
}

void ContentDirectoryServer::RespondEmptyDidl(struct MSL_CdsQuery *cdsQuery)
{
	// Respond with an empty list, as no container except
	// the AllMedia container is enabled for Search.	
	MSL_ForQueryResponse_Start(cdsQuery, 1);
	MSL_ForQueryResponse_ResultArgument(cdsQuery, "", 0);
	MSL_ForQueryResponse_FinishResponse(cdsQuery, 1, 0, 0, 0);
}

enum Enum_CdsErrors ContentDirectoryServer::RespondToQuery(unsigned int sorting, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery, unsigned int filter)
{
	enum Enum_CdsErrors error = CdsError_None;
	MediaItemData* dbEntry;
	char *didl;
	int didlLen;
	char *parentID;
	unsigned int numberReturned = 0, totalMatched = 0, updateID = 0;
	struct CdsMediaObject *cdsObj = NULL;
	SpecialContainerData* spcEntry = NULL;

	//
	//	This method works for any type of query
	//

	// Obtain the results for a Browse request on a standard container.
	this->GetQueryResults(sorting, objectID, cdsQuery, &dbEntry, &spcEntry);

	if (dbEntry != NULL)
	{
		//
		//	Respond with metadata for one or more CDS objects
		//

		// Obtain the parent - if the result is a list, then all
		// entries in the result will have the same parent.
		parentID = this->GetParentID(dbEntry, objectID, cdsQuery);

		//	Start the response
		MSL_ForQueryResponse_Start(cdsQuery, 1);

		while(1)
		{
			//	Do something with the item here
			if (dbEntry->EndOfFile()) break;

			//	Allocate memory for the cds object and assign fields appropriately
			cdsObj = CDS_AllocateObject();

			if (dbEntry != NULL)
			{
				dbEntry->SetStringLengths();
				// Set the item key
				objectID->UnderlyingObjectID = dbEntry->Key;
			}

			if (spcEntry != NULL)
			{
				spcEntry->SetStringLengths();
			}

			// set the fields on the cds object
			this->SetCdsObjectFields(cdsObj, parentID, dbEntry, spcEntry, objectID, cdsQuery);

			//	Get the DIDL-Lite representation of the object
			didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, 1, filter, 0, &didlLen);

			//	Free the memory for the cds object
			CDS_DestroyObjects(cdsObj);

			//	Respond with the metadata for the item
			if (didlLen > 0)
			{
				//	Ensure that the returned CDS object would fall into the requested range.
				//	This could be optimized in such a way that I call dbEntry->MoveNext()
				//	to the correct starting point and then respond a certain number of times,
				//	but there's always the chance that didlLen will fail because of somebody
				//	screwing up the database. This code is a bit more reliable, at the expense
				//	of performance.
				if (
					(totalMatched >= cdsQuery->StartingIndex)
					&& ((cdsQuery->RequestedCount == 0) || (numberReturned < cdsQuery->RequestedCount))
					)
				{
					MSL_ForQueryResponse_ResultArgument(cdsQuery, didl, didlLen);
					numberReturned++;
				}

				totalMatched++;
			}
			else
			{
				//	print an error message if we couldn't get the DIDL
				fprintf(stderr, "void ContentDirectoryServer::Handle_OnQuery() failed to serialize object %s. Reason=%d.\r\n", cdsObj->ID, didlLen);
			}

			// Free the didl text
			free(didl);

			// done serializing, so go to the next one
			dbEntry->MoveNext();
		}
		delete dbEntry; // Clean-up query response

		// use free to delete memory for parentID because GetParentID() uses malloc
		free(parentID);

		//	Finish up the response for the query
		MSL_ForQueryResponse_FinishResponse(cdsQuery, 1, numberReturned, totalMatched, updateID);
	}
	else if (spcEntry != NULL)
	{
		//
		// Respond with metadata for one or more dynamic containers
		//

		// Obtain the parent - if the result is a list, then all
		// entries in the result will have the same parent.
		parentID = (char*) malloc(10);
		sprintf(parentID, "%d", objectID->Base);

		//	Start the response
		MSL_ForQueryResponse_Start(cdsQuery, 1);

		while(1)
		{
			//	Do something with the item here
			if (spcEntry->EndOfFile()) break;

			if (spcEntry != NULL)
			{
				(spcEntry)->Value_length = (int) strlen((spcEntry)->Value);
			}

			if (spcEntry->Value_length != 0)
			{
				//	Allocate memory for the cds object and assign fields appropriately
				cdsObj = CDS_AllocateObject();
				this->SetCdsObjectFields(cdsObj, parentID, dbEntry, spcEntry, objectID, cdsQuery);

				//	Get the DIDL-Lite representation of the object - assumed that
				//	thet metadata copied from the database is already XML-escaped
				didl = CdsToDidl_GetMediaObjectDidlEscaped(cdsObj, 1, filter, 0, &didlLen);

				//	Free the memory for the cds object
				CDS_DestroyObjects(cdsObj);

				//	Respond with the metadata for the item
				if (didlLen > 0)
				{
					//	Ensure that the returned CDS object would fall into the requested range.
					//	This could be optimized in such a way that I call dbEntry->MoveNext()
					//	to the correct starting point and then respond a certain number of times,
					//	but there's always the chance that didlLen will fail because of somebody
					//	screwing up the database. This code is a bit more reliable, at the expense
					//	of performance.
					if (
						(totalMatched >= cdsQuery->StartingIndex)
						&& ((cdsQuery->RequestedCount == 0) || (numberReturned < cdsQuery->RequestedCount))
						)
					{
						MSL_ForQueryResponse_ResultArgument(cdsQuery, didl, didlLen);
						numberReturned++;
					}

					totalMatched++;
				}
				else
				{
					//	print an error message if we couldn't get the DIDL
					fprintf(stderr, "void ContentDirectoryServer::Handle_OnQuery() failed to serialize object %s. Reason=%d.\r\n", cdsObj->ID, didlLen);
				}

				free(didl);
			}

			// done serializing, so go to the next one
			spcEntry->MoveNext();
		}
		delete spcEntry; // Clean-up query response

		// use free to delete memory for parentID because GetParentID() uses malloc
		free(parentID);

		//	Finish up the response for the query
		MSL_ForQueryResponse_FinishResponse(cdsQuery, 1, numberReturned, totalMatched, updateID);
	}
	else
	{
		error = CdsError_NoSuchObject;
	}

	return error;
}

void ContentDirectoryServer::SetCdsObjectFields(struct CdsMediaObject *cdsObj, const char *parentID, MediaItemData* dbEntry, SpecialContainerData* spcEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery)
{
	cdsObj->RefID = NULL;

	if (dbEntry != NULL)
	{
		//	Title must be non-empty
		assert(dbEntry->Title_length > 0);
		cdsObj->Title = dbEntry->Title;

		//	Assign optional metadata if present
		if (dbEntry->Album_st == adFldOK)	{ cdsObj->Album		= dbEntry->Album; }
		if (dbEntry->Creator_st == adFldOK)	{ cdsObj->Creator	= dbEntry->Creator; }
		if (dbEntry->Genre_st == adFldOK)	{ cdsObj->Genre		= dbEntry->Genre; }

		//	Convert database's notion of a media class into a value appropriate for CdsMediaObject
		cdsObj->MediaClass = ConvertMediaClass(dbEntry->Class);

		//	Assign the object's searchable and restricted attributes 
		if (
			(cdsObj->MediaClass & CDS_MEDIACLASS_CONTAINER)
			)
		{
			//	If the CDS object is the AllMedia container, then it's searchable
			if (dbEntry->Key == this->m_allMediaObjectID)
			{
				cdsObj->Flags |= CDS_OBJPROP_FLAGS_Searchable;
			}
			
			//	Info about the number of children is always in
			//	the deserialization. This is the ONLY thing
			//	that should be in a container.
			sscanf(dbEntry->Deserialization, CDS_RESFORMAT_CONTAINER, &(cdsObj->ChildCount));
		}
		else
		{
			//	CDS object is an item, so it might have a refID
			cdsObj->RefID = this->GetRefID(dbEntry, objectID, cdsQuery);
			if (cdsObj->RefID != NULL)
			{
				cdsObj->DeallocateThese |= CDS_ALLOC_RefID;
			}
		}
		cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

		//	Obtain the objectID that will be used to represent the CDS object.
		cdsObj->ID = this->GetObjectID(dbEntry, spcEntry, objectID, cdsQuery);
		cdsObj->DeallocateThese |= CDS_ALLOC_ID;

		//	Copy the parentID that will be used.
		cdsObj->ParentID = this->GetParentID(dbEntry, objectID, cdsQuery);
		cdsObj->DeallocateThese |= CDS_ALLOC_ParentID;

		//	Obtain the resources for this CDS object.
		cdsObj->Res = this->GetResources(dbEntry, objectID, cdsQuery);
	}

	if (spcEntry != NULL)
	{
		//	Title must be non-empty
		assert(strlen(spcEntry->Value) > 0);
		
		cdsObj->Title = spcEntry->Value;
		cdsObj->MediaClass = CDS_MEDIACLASS_CONTAINER;
		cdsObj->ChildCount = spcEntry->ValueCount;
		cdsObj->Flags = CDS_OBJPROP_FLAGS_Restricted;

		//	Obtain the objectID that will be used to represent the CDS object.
		cdsObj->ID = (char*) malloc(MAX_OBJID_LEN);
		sprintf(cdsObj->ID, "%d/&quot;%s&quot;", objectID->Base, spcEntry->Value);
		cdsObj->DeallocateThese |= CDS_ALLOC_ID;

		//	Copy the parentID that will be used.
		cdsObj->ParentID = (char*) malloc(SMALL_OBJID_LEN);
		sprintf(cdsObj->ParentID, "%d", objectID->Base);
		cdsObj->DeallocateThese |= CDS_ALLOC_ParentID;
	}
}

void ContentDirectoryServer::Sink_OnQuery(void* contentDirectoryServerObject, struct MSL_CdsQuery *cdsQuery)
{
	ContentDirectoryServer *cds = (ContentDirectoryServer*) contentDirectoryServerObject;
	cds->Handle_OnQuery(cdsQuery);
}

void ContentDirectoryServer::Sink_OnStatsChanged(void* contentDirectoryServerObject, struct MSL_Stats *stats)
{
	ContentDirectoryServer *cds = (ContentDirectoryServer*) contentDirectoryServerObject;
	cds->Handle_OnStatsChanged(stats);
}
