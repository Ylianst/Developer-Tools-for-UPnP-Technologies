#ifndef _MS_CP_H
#define _MS_CP_H

#include "UPnPControlPointStructs.h"
#include "MSCP_ControlPoint.h"
#include "ControlPoint_Wrapper_Common.h"

#define MSCP_CLASS_OBJECT_TYPE_LEN	3
#define MSCP_CLASS_MAJOR_TYPE_LEN	13 
#define MSCP_CLASS_MINOR_TYPE_LEN	13

extern const char* MSCP_CLASS_OBJECT_TYPE[];
extern const char* MSCP_CLASS_MAJOR_TYPE[];
extern const char* MSCP_CLASS_MINOR_TYPE[];

/*
 *	Bit mapping of MediaClass types. Assumes 'unsigned int' is 64 bits wide.
 *
 *	0:		object has bad class
 *	1:		object is item
 *	2:		object is container
 *
 *	16-31:	minor 2 class type
 *			Extract these bits out, shift right MSCP_SHIFT_MINOR2_TYPE bits 
 *			and use the value as an index into a custom array. String should append to
 *			the rest of the string that could be formed from the other bits.
 *
 *	32-47:	minor 1 class type
 *			Extract these bits out, shift right MSCP_SHIFT_MINOR1_TYPE bits 
 *			and use the value as an index into the as MSCP_CLASS_MINOR_TYPE array.
 *
 *	48-63:	major class type
 *			Extract these bits out, shift right MSCP_SHIFT_MAJOR_TYPE bits 
 *			and use the value as an index into the as MSCP_CLASS_MAJOR_TYPE array.
 */

#define MSCP_SHIFT_MAJOR_TYPE						24
#define MSCP_SHIFT_MINOR1_TYPE						16
#define MSCP_SHIFT_MINOR2_TYPE						8

#define MSCP_CLASS_MASK_MAJOR						0xFF000000
#define MSCP_CLASS_MASK_MINOR1						0x00FF0000
#define MSCP_CLASS_MASK_MINOR2						0x0000FF00
#define MSCP_CLASS_MASK_OBJECTTYPE					0x00000003

#define MSCP_CLASS_MASK_BADCLASS					0x00000000
#define MSCP_CLASS_MASK_ITEM						0x00000001
#define MSCP_CLASS_MASK_CONTAINER					0x00000002

#define MSCP_CLASS_MASK_MAJOR_IMAGEITEM				0x01000000
#define MSCP_CLASS_MASK_MAJOR_AUDIOITEM				0x02000000
#define MSCP_CLASS_MASK_MAJOR_VIDEOITEM				0x03000000
#define MSCP_CLASS_MASK_MAJOR_PLAYLISTITEM			0x04000000
#define MSCP_CLASS_MASK_MAJOR_TEXTITEM				0x05000000
#define MSCP_CLASS_MASK_MAJOR_PERSON				0x06000000
#define MSCP_CLASS_MASK_MAJOR_PLAYLISTCONTAINER		0x07000000
#define MSCP_CLASS_MASK_MAJOR_ALBUM					0x08000000
#define MSCP_CLASS_MASK_MAJOR_GENRE					0x09000000
#define MSCP_CLASS_MASK_MAJOR_STRGSYS				0x0A000000
#define MSCP_CLASS_MASK_MAJOR_STRGVOL				0x0B000000
#define MSCP_CLASS_MASK_MAJOR_STRGFOL				0x0C000000

#define MSCP_CLASS_MASK_MINOR_PHOTO					0x00010000
#define MSCP_CLASS_MASK_MINOR_MUSICTRACK			0x00020000
#define MSCP_CLASS_MASK_MINOR_AUDIOBROADCAST		0x00030000
#define MSCP_CLASS_MASK_MINOR_AUDIOBOOK				0x00040000
#define MSCP_CLASS_MASK_MINOR_MOVIE					0x00050000
#define MSCP_CLASS_MASK_MINOR_VIDEOBROADCAST		0x00060000
#define MSCP_CLASS_MASK_MINOR_MUSICVIDEOCLIP		0x00070000
#define MSCP_CLASS_MASK_MINOR_MUSICARTIST			0x00080000
#define MSCP_CLASS_MASK_MINOR_MUSICALBUM			0x00090000
#define MSCP_CLASS_MASK_MINOR_PHOTOALBUM			0x000A0000
#define MSCP_CLASS_MASK_MINOR_MUSICGENRE			0x000B0000
#define MSCP_CLASS_MASK_MINOR_MOVIEGENRE			0x000C0000

/*
 *	None of these error codes are allowed to overlap
 *	with the UPnP, UPnP-AV error code ranges.
 */
enum MSCP_NonstandardErrorCodes
{
	MSC_Error_XmlNotWellFormed		= 1000,
};

/*
 *	Provides mapping for MSCP_Mediaobject's Flags field.
 *	Values must be friendly for bit operations.
 */
enum MSCP_Enum_Flags
{
	MSCP_Flags_Restricted = 1,			/* restricted attribute of media object */
	MSCP_Flags_Searchable = 2			/* container is searchable */
};

enum MSCP_Enum_BrowseFlag
{
	MSCP_BrowseFlag_Metadata = 0,		/* browse metadata */
	MSCP_BrowseFlag_Children			/* browse children */
};

/*
 *	Minimalistic representation of a resource.
 */
struct MSCP_MediaResource
{
	char *Uri;
	char *ProtocolInfo;

	char *Resolution;
	char *Duration;
	long Bitrate;				/* if negative, has not been set */

	long ColorDepth;	/* if negative, has not been set */
	long Size;					/* if negative, has not been set */

	struct MSCP_MediaResource *Next;
};

/*
 *	Minimalistic representation of a media object.
 */
struct MSCP_MediaObject
{
	char *ID;			/* Object ID */
	char *ParentID;		/* Parent object ID */
	char *RefID;		/* Object ID of underlying item: for reference item only*/

	char *Title;		/* Title metadata */
	char *Creator;		/* Creator metadata */

	/* media class of object: masked values */
	unsigned int MediaClass;

	/* Boolean flags, bits mapped by MSCP_Enum_Flags */
	unsigned int Flags;

	struct MSCP_MediaResource *Res;		/* first resource for the media object*/
	struct MSCP_MediaObject *Next;			/* next media object in list */
};

/*
 *	Browse results are always encapsulated in this struct.
 */
struct MSCP_ResultsList
{
	/* points to the first media object in the results list */
	struct MSCP_MediaObject *FirstObject;
	unsigned int NumberReturned;
	unsigned int TotalMatches;
	unsigned int UpdateID;

	/* number of media objects that were successfully parsed */
	int NumberParsed;

};

/*
 *	Represents a Browse request.
 */
struct MSCP_BrowseArgs
{
	char *ObjectID;
	enum MSCP_Enum_BrowseFlag BrowseFlag;
	char *Filter;
	unsigned int StartingIndex;
	unsigned int RequestedCount;
	char *SortCriteria;

	/* browse request initiator can attach a misc field for use in results processing */
	void *UserObject;				
};

typedef void (*MSCP_Fn_Result_Browse) (void *serviceObj, struct MSCP_BrowseArgs *args, int errorCode, struct MSCP_ResultsList *results);


/*
 *	Use this method to destroy the results of a Browse request.
 */
void MSCP_DestroyResultsList (struct MSCP_ResultsList *resultsList);

/*
 *	Must call this method once at the very beginning.
 *
 *	Caller registers callbacks for Browse responses and when MediaServers enter/leave the UPnP network.
 *		chain			: thread chain, obtained from ILibCreateChain
 *		callbackBrowse	: execute this method when results for a browse request are received
 *		callbackDeviceAdd : execute this method when a MediaServer enters the UPnP network
 *		callbackDeviceRemove : execute this method when a MediaServer leaves the UPnP network
 *
 *  Note that the last 2 callbacks (device add and device remove) can be null if you don't need
 *		notification of devices being added and removed.
 */
void * MSCP_Init(void *chain, 
				MSCP_Fn_Result_Browse callbackBrowse, 
				CP_Fn_Device_Add callbackDeviceAdd,
				CP_Fn_Device_Remove callbackDeviceRemove);

/*
 *	Call this method to perform a browse request.
 *
 *	serviceObj		: the CDS service object for the MediaServer
 *	args			: the arguments of the browse request.
 */
void MSCP_Invoke_Browse(void *serviceObj, struct MSCP_BrowseArgs *args);

/*
 *	Use this method to select the best matched CdsMediaResource.
 *	The resource object's IP-based URI can then be used to actually acquire the content.
 *
 *		mediaObj		: the CDS object with zero or more resources
 *		protocolInfoSet	: comma-delimited set of protocolInfo, sorted with target's preferred formats frist
 *		ipAddress		: desired ipAddress, in network byte order form.
 *
 *	Returns NULL if no acceptable resource was found.
 */
struct MSCP_MediaResource* MSCP_SelectBestIpNetworkResource(
	const struct MSCP_MediaObject *mediaObj, 
	const char *protocolInfoSet, 
	int *ipAddressList, 
	int ipAddressListLen);

/*
 *	Call this method for cleanup.
 */
void MSCP_Uninit();

#endif
