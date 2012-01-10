#ifndef _CDS_MEDIACLASS_H
#define _CDS_MEDIACLASS_H

/*
 *	The purpose of this module is to abstract the concept of a upnp media class,
 *	(eg, the data stored in the upnp:class DIDL-Lite element) into an unsigned int
 *	form. 
 *
 *	The module also provides the means to allow vendors to add their own custom
 *	media classes to the existing infrastructure. 
 *
 */

/*
 *	A media class is represented in string form for UPnP, but can be exposed to applications
 *	through an unsigned int of 64 bits. If the value of the media class is CDS_CLASS_MASK_BADCLASS (zero), 
 *	then it should mean that the media class is unassigned or invalid in some form.
 *
 *	Described below is a description of how the bits are used in the value.
 *
 *	Bit:	Description
 *  -----------------------------------------------------------------------------------------------
 *	0:		Flag: object is item. 
 *	1:		Flag: object is container. 
 *
 *			Notes about bits 0-1.
 *			Extract bits [0,1] using a bitwise-AND of the media class value & CDS_CLASS_MASK_OBJECTTYPE,
 *			and use the resulting value as an index into CDS_CLASS_OBJECT_TYPE to get a string
 *			representation of the object type.
 *
 *	16-31:	minor 2 class type
 *			Extract these bits out using a bitwise-AND operation of media class & CDS_SHIFT_MINOR2_TYPE,
 *			shift right the resulting value by CDS_SHIFT_MINOR2_TYPE bits,
 *			and use the value as an index into a custom array. String should append to
 *			the rest of the string that could be formed from the other bits.
 *			This is the field that vendors can use to create custom media classes.
 *
 *	32-47:	minor 1 class type
 *			Extract these bits out using a bitwise-AND operation of media class & CDS_CLASS_MASK_MINOR1,
 *			shift right the resulting value by CDS_SHIFT_MINOR1_TYPE bits,
 *			and use the value as an index into the as CDS_CLASS_MINOR1_TYPE array.
 *
 *	48-63:	major class type
 *			Extract these bits out using a bitwise-AND operation of media class & CDS_CLASS_MASK_MAJOR,
 *			shift right the resulting value by CDS_SHIFT_MAJOR_TYPE bits,
 *			and use the value as an index into the as CDS_CLASS_MAJOR_TYPE array.
 *
 *	For all intents and purposes, if a bitwise-AND operation with a CDS_CLASS_MASK_xxx value yields
 *	a value of zero, then it means that the particulate portion of the media class remains unassigned.
 *
 *	To obtain a string representation of a media class, concatenate the strings that represent the
 *	object-type, major-type, minor type, and minor2 types with each substring separated with a 
 *	a period (.) character. There is no trailing dot character. For example, the
 *	media class of 0x2020001 would have a corresponding string representation of "object.item.audioItem.musicTrack"
 *	because:
 *		CDS_CLASS_OBJECT_TYPE[0x2020001 & CDS_CLASS_MASK_OBJECTTYPE] == "object.item"
 *		CDS_CLASS_MAJOR_TYPE[(0x2020001 & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MAJOR_TYPE] == "audioItem"
 *		CDS_CLASS_MINOR1_TYPE[(0x2020001 & CDS_CLASS_MASK_MAJOR) >> CDS_SHIFT_MINOR1_TYPE] == "musicTrack"
 *
 *	Another added benefit of this architecture is that it's friendly to boolean operations that involve
 *	the media class. For example, the following are common operations.
 *
 *		(mediaClass & CDS_CLASS_MASK_ITEM)				: Indicates if the media class is some kind of item
 *		(mediaClass & CDS_CLASS_MASK_CONTAINER)			: Indicates if the media class is some kind of container
 *		(mediaClass & CDS_CLASS_MASK_MAJOR_AUDIOITEM)	: Indicates if the media class is some kind of audio related item
 *
 *	The header file defines a set of CDS_CLASS_MASK_xxx values to match up with the normative
 *	set of media classes defined in UPnP AV.
 *
 *	To obtain a value for a normative media class, use one of the CDS_MEDIACLASS_xxx values. 
 *	For example, to get the media class value for a an audio broadcast, use CDS_MEDIACLASS_AUDIOBROADCAST.
 *	Using CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST will only set the minor1 portion of the media class,
 *	which is an incomplete media class. 
 */




/* Normative set of object types: eg, object.item or object.container */
extern const char* CDS_CLASS_OBJECT_TYPE[];	

/* Normative set of major classes - first sublevel from item or container - ex: object.item.audioItem*/
extern const char* CDS_CLASS_MAJOR_TYPE[];	

/* Normative set of minor class designations - second sublevel from item or container - ex: object.item.audioItem.musicTrack */
extern const char* CDS_CLASS_MINOR1_TYPE[];	

/* Custom set of minor designations - third sublevel from item or container. */
extern const char* CDS_CLASS_MINOR2_TYPE[];	

/* The number of strings in CDS_CLASS_OBJECT_TYPE */
#define CDS_CLASS_OBJECT_TYPE_LEN	3

/* The number of strings in CDS_CLASS_MAJOR_TYPE */
#define CDS_CLASS_MAJOR_TYPE_LEN	13 

/* The number of strings in CDS_CLASS_MINOR1_TYPE */
#define CDS_CLASS_MINOR1_TYPE_LEN	13

/* The number of string in CDS_CLASS_MINOR2_TYPE */
#define CDS_CLASS_MINOR2_TYPE_LEN	1

#define CDS_SHIFT_MAJOR_TYPE					24
#define CDS_SHIFT_MINOR1_TYPE					16
#define CDS_SHIFT_MINOR2_TYPE					8

/*
 *	Use of class masks are described above.
 */

#define CDS_CLASS_MASK_MAJOR					0xFF000000
#define CDS_CLASS_MASK_MINOR1					0x00FF0000
#define CDS_CLASS_MASK_MINOR2					0x0000FF00

#define CDS_CLASS_MASK_OBJECTTYPE				0x00000003

#define CDS_CLASS_MASK_BADCLASS					0x00000000
#define CDS_CLASS_MASK_ITEM						0x00000001
#define CDS_CLASS_MASK_CONTAINER				0x00000002

#define CDS_CLASS_MASK_MAJOR_IMAGEITEM			0x01000000
#define CDS_CLASS_MASK_MAJOR_AUDIOITEM			0x02000000
#define CDS_CLASS_MASK_MAJOR_VIDEOITEM			0x03000000
#define CDS_CLASS_MASK_MAJOR_PLAYLISTITEM		0x04000000
#define CDS_CLASS_MASK_MAJOR_TEXTITEM			0x05000000

#define CDS_CLASS_MASK_MAJOR_PERSON				0x06000000
#define CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER	0x07000000
#define CDS_CLASS_MASK_MAJOR_ALBUM				0x08000000
#define CDS_CLASS_MASK_MAJOR_GENRE				0x09000000
#define CDS_CLASS_MASK_MAJOR_STRGSYS			0x0A000000
#define CDS_CLASS_MASK_MAJOR_STRGVOL			0x0B000000
#define CDS_CLASS_MASK_MAJOR_STRGFOL			0x0C000000

#define CDS_CLASS_MASK_MINOR1_PHOTO				0x00010000
#define CDS_CLASS_MASK_MINOR1_MUSICTRACK		0x00020000
#define CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST	0x00030000
#define CDS_CLASS_MASK_MINOR1_AUDIOBOOK			0x00040000
#define CDS_CLASS_MASK_MINOR1_MOVIE				0x00050000
#define CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST	0x00060000
#define CDS_CLASS_MASK_MINOR1_MUSICVIDEOCLIP	0x00070000

#define CDS_CLASS_MASK_MINOR1_MUSICARTIST		0x00080000
#define CDS_CLASS_MASK_MINOR1_MUSICALBUM		0x00090000
#define CDS_CLASS_MASK_MINOR1_PHOTOALBUM		0x000A0000
#define CDS_CLASS_MASK_MINOR1_MUSICGENRE		0x000B0000
#define CDS_CLASS_MASK_MINOR1_MOVIEGENRE		0x000C0000

/*
 *	If the application needs to get a mediaClass value for a specific type of
 *	media object, it should use these macros.
 */

#define CDS_MEDIACLASS_ITEM						(CDS_CLASS_MASK_ITEM)
#define CDS_MEDIACLASS_CONTAINER				(CDS_CLASS_MASK_CONTAINER)

#define CDS_MEDIACLASS_IMAGEITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_IMAGEITEM			)
#define CDS_MEDIACLASS_AUDIOITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM			)
#define CDS_MEDIACLASS_VIDEOITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM			)
#define CDS_MEDIACLASS_PLAYLISTITEM				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_PLAYLISTITEM		)
#define CDS_MEDIACLASS_TEXTITEM					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_TEXTITEM			)

#define CDS_MEDIACLASS_PERSON					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PERSON				)
#define CDS_MEDIACLASS_PLAYLISTCONTAINER		(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PLAYLISTCONTAINER	)
#define CDS_MEDIACLASS_ALBUM					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM				)
#define CDS_MEDIACLASS_GENRE					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE				)
#define CDS_MEDIACLASS_STRGSYS					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STRGSYS			)
#define CDS_MEDIACLASS_STRGVOL					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STRGVOL			)
#define CDS_MEDIACLASS_STRGFOL					(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_STRGFOL			)

#define CDS_MEDIACLASS_PHOTO					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_IMAGEITEM | CDS_CLASS_MASK_MINOR1_PHOTO			)
#define CDS_MEDIACLASS_MUSICTRACK				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_MUSICTRACK	)
#define CDS_MEDIACLASS_AUDIOBROADCAST			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_AUDIOBROADCAST)
#define CDS_MEDIACLASS_AUDIOBOOK				(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_AUDIOITEM | CDS_CLASS_MASK_MINOR1_AUDIOBOOK		)
#define CDS_MEDIACLASS_MOVIE					(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_MOVIE			)
#define CDS_MEDIACLASS_VIDEOBROADCAST			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_VIDEOBROADCAST)
#define CDS_MEDIACLASS_MUSICVIDEOCLIP			(CDS_CLASS_MASK_ITEM | CDS_CLASS_MASK_MAJOR_VIDEOITEM | CDS_CLASS_MASK_MINOR1_MUSICVIDEOCLIP)

#define CDS_MEDIACLASS_MUSICARTIST				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_PERSON | CDS_CLASS_MASK_MINOR1_MUSICARTIST	)
#define CDS_MEDIACLASS_MUSICALBUM				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM  | CDS_CLASS_MASK_MINOR1_MUSICALBUM	)
#define CDS_MEDIACLASS_PHOTOALBUM				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_ALBUM  | CDS_CLASS_MASK_MINOR1_PHOTOALBUM	)
#define CDS_MEDIACLASS_MUSICGENRE				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE  | CDS_CLASS_MASK_MINOR1_MUSICGENRE	)
#define CDS_MEDIACLASS_MOVIEGENRE				(CDS_CLASS_MASK_CONTAINER | CDS_CLASS_MASK_MAJOR_GENRE  | CDS_CLASS_MASK_MINOR1_MOVIEGENRE	)

#endif

