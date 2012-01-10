#pragma once

#include "MediaDatabase.h"
#include "MediaItemData.h"

#include "ResourceProvider.h"

extern "C"
{
	#include "MediaServerLogic.h"
	#include "CdsMediaObject.h"
}

// Posix style synchronization
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)


// Assume a character max length for a child container of a special container,
// and we'll need additional bytes to represent two integers (10 bytes each) along
// with some separators, so 300 should be enough.
// Data coming from the database will already be UTF-8 encoded.
#define MAX_OBJID_LEN		300
#define SMALL_OBJID_LEN		16
#define MEDIUM_OBJID_LEN	64
#define MAX_ENTRY_NAME		255

#define MAX_RES_PROVIDERS	255

#define MAX_FILE_EXT_LEN	64
#define MAX_PROT_STRING_LEN 512

// Format: \n delimited list of data in the following format.
//	[time (since epoch); must be valid] 
//	[fileSize; negative means unknown value] 
//	[file extension; must be non-NULL and present, max length of MAX_FILE_EXT_LEN] 
//	[resolution-X; negative if unknown] 
//	[resolution-y; negative if unknown] 
//	[duration in seconds; negative if unknown] 
//	[bitrate; negative if unknown or if VBR-encoded] 
//	[colorDepth; negative if unknown] 
//	[bitsPerSample; negative if unknown] 
//	[sampleFrequency; negative if unknown] 
//	[number of audio channels; negative if unknown] 
//	[protection string; blank if unkonown, max length of MAX_PROT_STRING_LEN] 
//	Always end with \n.
#define CDS_RESFORMAT_ITEM	"%lu\n%ld\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%s\n"

// Format: [# children in container]
#define CDS_RESFORMAT_CONTAINER "%u"

// This is the function pointer type that allows this class to get CdsMediaResource objects for a given MediaItemData.
// All string-based fields on the returned CdsMediaResource must be properly XML-scaped.
typedef struct CdsMediaResource* (*FnptrResourceProvider) (void* resourceProviderObject, const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen);

enum _ContentDirectoryServer_ObjectIdFormats
{
	//	Used for any CDS object that is taken as is from the database.
	//	StorageContainers, SpecialContainers,
	//	PlaylistEntries, and items in StorageContainers use this field.
	//
	_ContentDirectoryServer_ObjIdFormat_Standard,	

	//	Used only for PlaylistContainers. Playlist containers
	//	could appear as child objects in various containers,
	//	so they are slightly than StorageContainers and SpecialContainers.
	_ContentDirectoryServer_ObjIdFormat_PlaylistContainer,

	//	Used only for PlaylistContainers children. 
	_ContentDirectoryServer_ObjIdFormat_PlaylistEntry,

	//	Used for any CDS object that represents a container that is
	//	dynamically generated from information in the database.
	_ContentDirectoryServer_ObjIdFormat_DynamicContainer,

	//	Used for any child item of a special container.
	_ContentDirectoryServer_ObjIdFormat_SpecialReference,

	//	User for any child item of a dynamic container.
	_ContentDirectoryServer_ObjIdFormat_DynamicReference
};

struct _ContentDirectoryServer_ObjectId
{
	enum _ContentDirectoryServer_ObjectIdFormats Format;

	//	Used for Standard, DynamicContainer, SpecialReference, DynamicReference
	//	Specifies the objectID that is the closest ancestor to the
	//	to the target objectID.
	int Base;

	//	Used for Dynamic containers.
	//	Specifies the title of the dynamic container.
	//	The length of this field has a maximum that matches 
	//	the Title, Creator, Album, or Genre of MediaItemData.
	char DynamicName[MAX_ENTRY_NAME];

	//	For PlaylistContainers, this value represents the key in the
	//	database for the actual playlist.
	int	PlaylistID;

	//	Used for SpecialReference and DynamicReference, and PlaylistEntries.
	//	Specifies objectID of the the underlying object ID.
	int UnderlyingObjectID;
};


class ContentDirectoryServer
{
private:
	HANDLE m_ChainQuitEvent;

	MediaDatabase*	m_database;
	int				m_allMediaObjectID;
	
	// Locks the list of resource providers
	sem_t				m_ResourceProviderLock;
	
	// Provides the list of resource providers
	class ResourceProvider	*m_ResourceProviders[MAX_RES_PROVIDERS];
	

	//	Converts the database's classValue into a value usable for
	//	a CdsMediaObject.
	unsigned int ConvertMediaClass(long classValue);

	//	Converts an objectID in string form to the _ContentDirectoryServer_ObjectId struct
	//	for convenient programming access.
	void ConvertObjectID(struct _ContentDirectoryServer_ObjectId *objectID, char *objectIdAsString);

	//	Given information about the database entry, objectID specified in the query, and the query itself,
	//	return the value of the objectID for a CDS object.
	char* GetObjectID(MediaItemData* dbEntry, SpecialContainerData* spcEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	//	Given information about the database entry, objectID specified in the query, and the query itself,
	//	return the value of the parentID for a CDS object.
	char* GetParentID(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	//	Given information about the database entry, objectID specified in the query, and the query itself,
	//	return the value of the refID for a CDS object.
	char* GetRefID(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	//	Given information about the database entry, objectID specified in the query, and the query itself,
	//	return the resources for the CDS object.
	struct CdsMediaResource* GetResources(MediaItemData* dbEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	//	Given the CDS sorting criteria, get a 32-bit bitstring representing the sort criteria for use with the
	//	database.
	unsigned int GetSortBitString(struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	//	Given a query on a standard container and the parsed objectID, return a MediaItemData* or a SpecialContainerData* to represent the results.
	void GetQueryResults(unsigned int sorting, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery, /*OUT*/ MediaItemData **dbEntry, /*OUT*/ SpecialContainerData **spcEntry);

	//	This is the method that drives the process of querying the database
	//	and formulating a DIDL-Lite response.
	void Handle_OnQuery(struct MSL_CdsQuery *cdsQuery);

	//	This is the method that does the work for reporting the stats.
	void Handle_OnStatsChanged(struct MSL_Stats *stats);

	//	Responds with no DIDL-Lite entries. Usually called because somebody did a search on an object
	//	that does not support search.
	void RespondEmptyDidl(struct MSL_CdsQuery *cdsQuery);

	//	Formulates the response for a CDS query.
	enum Enum_CdsErrors RespondToQuery(unsigned int sorting, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery, unsigned int filter);

	//	Given the metadata from the database entry (along with information about the query), 
	//	set the fields on cdsObj to the appropriate values so that they can be serialized
	//	to DIDL-Lite.
	void SetCdsObjectFields(struct CdsMediaObject *cdsObj, const char *parentID, MediaItemData* dbEntry, SpecialContainerData* spcEntry, struct _ContentDirectoryServer_ObjectId *objectID, struct MSL_CdsQuery *cdsQuery);

	// Calls Handle_OnQuery()
	static void Sink_OnQuery(void* contentDirectoryServerObject, struct MSL_CdsQuery *cdsQuery);

	// Calls Handle_OnStatsChanged()
	static void Sink_OnStatsChanged(void* contentDirectoryServerObject, struct MSL_Stats *stats);

public:
	ContentDirectoryServer(MediaDatabase* db, void *QuitLock);
	~ContentDirectoryServer();

	void* GetWebServerToken();

	//	Call this method to register a module that provides CdsMediaResource objects for a database entry.
	int ResourceProviderRegister(ResourceProvider *resProvider);

	//	Call this method to unregister a module that provides CdsMediaResource objects for a database entry.
	void ResourceProviderUnregister(ResourceProvider *resProvider);

};

/*
The database has several types of basic entries.
	StorageContainers
	PlaylistContainers
	SpecialContainers
	Images
	Audio
	Video
	Other

Of these, Images, Audio, Video, and Other
may be entries within a playlist container.

In addition to these entries, there exists the
concept of a "dynamic container". A dynamic
container is one that is not actually in the
database. A DynamicContainer is always a child
container of a SpecialContainer. For example,
the SpecialContainer for "Audio By Artist" may
have a DynamicContainer for "Billy Joel".

Since DynamicContainers are not stored in the
database, their objectIDs must be dynamically
generated. The objectID format for a dynamic
container is always as follows:
[Special Container ID]/"[Dynamic Container Name]"

DynamicContainers (like SpecialContainers) can have children.
The rule for the child entries of DynamicContainers and 
SpecialContainers is that they are ALWAYS serialized to 
DIDL-Lite as reference items. In order to properly enable this,
the child items of DynamicContainers and SpecialContainers
have dynamically generated objectIDs. The format is as follows:
[Special Container ID]/"[Dynamic Container Name]"/[Object ID]
The first portion is the objectID of the SpecialContainer 
that is ancestor to the reference item.
The second portion (which is optional) represents the title
of the DynamicContainer which is parent to the reference item.
The last portion represents the objectID of the original item
in the database.

As for child (reference) items of SpecialContainers, their
objectID has the following format.
[Special Container ID]/[Object ID]
The first portion is the objectID of the SpecialContainer 
that is ancestor to the reference item.
The last portion represents the objectID of the original item
in the database.

Note that the only time quotes are used is for the DynamicContainer name.

It should be noted that the only container that supports search is the
AllMedia container because it is the only container that resembles the
flat-list querying model of the database.
*/

