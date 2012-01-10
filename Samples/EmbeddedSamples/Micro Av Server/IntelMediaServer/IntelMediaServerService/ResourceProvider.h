#ifndef _RESOURCE_PROVIDER_H
#define _RESOURCE_PROVIDER_H

#include <stdlib.h>
#include "ContentDirectoryServer.h"
#include "MediaDatabase.h"
#include "MediaItemData.h"

extern "C"
{
	#include "CdsMediaObject.h"
	#include "UpnpMicroStack.h"
	#include "ILibWebServer.h"
}

//	Represents the current progress of a file transfer.
//	The class is agnostic to the protocol and the mime-type.
class TransferProgress
{
protected:
		// Negative value indicates unknown number of bytes remaining.
	long PendingBytes;
	
	// Should always be non-negative value, representing the
	// number of bytes that have been sent for the transfer.
	long SentBytes;
	long TotalBytes;
public:
	// The key of the content associated with the transfer. The key
	// can be used in a database query.
	long ContentKey;

	// The form of content that this transfer represents.
	enum MEDIA_DB_CLASS_ENUM ContentClass;

	virtual long GetPendingBytes();
	virtual long GetSentBytes();
	virtual long GetTotalBytes();
	double GetTransferPercentage();

	// The explicit or relative URI that was specified in the request. 
	// String is null-terminated.
	char* Uri;

	TransferProgress()
	{
		this->ContentClass = MEDIA_DB_CL_ALL;
		this->ContentKey = 0;
		this->PendingBytes = -1;
		this->TotalBytes = -1;
		this->SentBytes = 0;
		this->Uri = NULL;
	}

	TransferProgress(long contentKey, enum MEDIA_DB_CLASS_ENUM contentClass, long pendingBytes, long sentBytes, char* uri, int uriLength)
	{
		this->ContentClass = contentClass;
		this->ContentKey = contentKey;
		this->PendingBytes = pendingBytes;
		this->SentBytes = sentBytes;
		this->Uri = new char[uriLength+1];
		memcpy(this->Uri, uri, uriLength);
		this->Uri[uriLength] = '\0';
	}

	~TransferProgress()
	{
		if (this->Uri != NULL) delete (this->Uri);
	}
};

//	Base class for all ResourceProviders.
//
//	Classes derived from ResourceProvider are responsible for instantiating
//	CdsMediaResource objects for a ContentDirectoryServer instance.
//	Specifically, a ContentDirectoryServer instance will have one or more
//	pointers to an instance of a ResourceProvider-derived class. Every time
//	the ContentDirectoryServer object needs to acquire 'res' element metadata,
//	the ContentDirectoryServer object will call ResourceProvider.GetResources() 
//	on each of the registered ResourceProvider-derived objects. Each
//	ResourceProvider-derived object will then return a linked list of
//	CdsMediaResource objects in its response to the ResourceProvider.GetResources() 
//	call.
//
//	ResourceProviders are also responsible for allowing a ContentDirectoryServer
//	object to query the progress of any file transfers for content URIs constructed
//	through the ResourceProvider.
class ResourceProvider
{
public:
	//	Pure virtual function. Given content metadata (eg, dbEntry)
	//	and the IP addresses available for the system, the implementation
	//	of this method will return a linked list of CdsMediaResource objects
	//	to represent the 'res' elements of the content.
	//
	//	The purpose of sending the IP addresses is to allow this method's
	//	implementation to return multiple CdsMediaResource objects, so
	//	that their is one 'res' element for each IP address of the content.
	virtual struct CdsMediaResource* GetResources(const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen) = 0;

	//	Pure virtual function.
	//	Returns the number of transfers associated with this ResourceProvider.
	virtual int GetNumTransfers() = 0;

	virtual ~ResourceProvider();

	//	Pure virtual function.
	//	Returns a thread-safe array of TransferProgress objects.
	//
	//	The caller can specify the range of desired transfers. 
	//	This is useful for user-interfaces that want to request
	//	a subset of the transfers that can be displayed.
	//
	//	startIndex represents the index of the first transfer. 
	//	maxCount represents the maximum number of TransferProgress objects to return.
	//	numTransfers represents the size of the TransferProgress array.
	virtual TransferProgress** GetTransferList(int startIndex, int maxCount, int *numTransfers) = 0;
	virtual void DestroyTransferList(TransferProgress** DestroyMe) = 0;
};

#endif
