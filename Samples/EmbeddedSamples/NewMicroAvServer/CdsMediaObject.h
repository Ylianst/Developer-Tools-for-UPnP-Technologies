#ifndef _CDS_MEDIA_OBJECT_H
#define _CDS_MEDIA_OBJECT_H

/*
 *	Provides the convenient means of manipulating/reading of CdsMediaObject.MediaClass field.
 */
#include "CdsMediaClass.h"

/*
 *	This module provides minimal data structures to represent
 *	media objects and their resource elements.
 *
 *	Vendors that want to increase the supported metadata elements
 *	should modify these structures appropriately.
 *
 *	This library does not presume use specifically for control point
 *	or device-implementation software.
 */

enum CdsObjectProperties
{
	CDS_OBJPROP_FLAGS_Restricted = 1,			/* restricted attribute of media object */
	CDS_OBJPROP_FLAGS_Searchable = 2			/* container is searchable */
};

/*
 *	Minimalistic representation of a resource.
 *
 *	All strings are assumed to be in UTF8 form.
 */
struct CdsMediaResource
{
	char *Value;				/* this is the text that provide's the resource's value - usually a URI */
	char *ProtocolInfo;			/* the protocolInfo of the resource */
	char ProtocolInfoAllocated;	/* if nonzero, then ProtocolInfo should be deallocated. Provied because many protocolInfo could be assigned static strings. */

	int ResolutionX;			/* the horizontal resolution. Negative value means value is not set.*/
	int ResolutionY;			/* the vertical resolution. Negative value means value is not set.*/

	int Duration;				/* the duration, in number of seconds. Negative value means value is not set.*/
	int Bitrate;				/* the bitrate of the resource, in bytes per second. If negative, treat as an unset value. */

	int ColorDepth;				/* the color depth of the resource. If negative, treat as an unset value. */
	long Size;					/* the file size of the resource. If negative, treat as an unset value. */

	struct CdsMediaResource *Next;	/* allows the struct to be used in a linked list */
};

/*
 *	Provides a minimal representation of a media object.
 */
struct CdsMediaObject
{
	char *ID;			/* Object ID */
	char *ParentID;		/* Parent object ID */
	char *RefID;		/* Object ID of underlying item: for reference item only*/

	char *Title;		/* Title metadata */
	char *Creator;		/* Creator metadata */

	/* media class of object. See [CdsMediaObject.h] */
	unsigned int MediaClass;

	/* number of children that this object has - valid only for container objects */
	int ChildCount;

	/*
	 *	Boolean flags for representing object boolean properties. 
	 *	Individual bits can be accessed through (enum CdsObjectProperties).
	 */
	unsigned int Flags;

	struct CdsMediaResource *Res;		/* first resource for the media object */
	struct CdsMediaObject *Next;		/* allows struct to be used in a linked list*/
};

/* Allocates the memory for a CdsMediaObject, with all memory set to zero */
struct CdsMediaObject* CDS_AllocateObject();

struct CdsMediaResource* CDS_AllocateResource();

/* Deallocates all of the memory for the linked list of CdsMediaObject and its associated resources. */
void CDS_DestroyObjects(struct CdsMediaObject *cdsObjList);

/* Deallocates all of the memory for the linked list of CdsResources */
void CDS_DestroyResources(struct CdsMediaResource *resList);

#endif

