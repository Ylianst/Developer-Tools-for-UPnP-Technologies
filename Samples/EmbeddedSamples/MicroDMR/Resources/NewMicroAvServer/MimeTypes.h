#ifndef MIMETYPES_H

#include "CdsMediaClass.h"

/*
 *	Provides forward declarations for methods in
 *	MimeTypes.c
 */

#define CLASS_ITEM			"object.item"
#define CLASS_AUDIO_ITEM	"object.item.audioItem"
#define CLASS_PLAYLIST_M3U	"object.container.playlistContainer"
#define CLASS_PLAYLIST_ASX	"object.container.playlistContainer"
#define CLASS_VIDEO_ITEM	"object.item.videoItem"
#define CLASS_IMAGE_ITEM	"object.item.imageItem"


#define EXTENSION_AUDIO_MPEG		".mp3"
#define MIME_TYPE_AUDIO_MPEG		"audio/mpeg"
#define PROTINFO_AUDIO_MPEG			"http-get:*:audio/mpeg:*"

#define EXTENSION_AUDIO_WMA			".wma"
#define MIME_TYPE_AUDIO_WMA			"audio/x-ms-wma"
#define PROTINFO_AUDIO_WMA			"http-get:*:audio/x-ms-wma:*"

#define EXTENSION_AUDIO_OGG			".ogg"
#define MIME_TYPE_AUDIO_OGG			"audio/x-ogg"
#define PROTINFO_AUDIO_OGG			"http-get:*:audio/x-ogg:*"

#define EXTENSION_VIDEO_ASF			".asf"
#define MIME_TYPE_VIDEO_ASF			"video/x-ms-asf"
#define PROTINFO_VIDEO_ASF			"http-get:*:video/x-ms-asf:*"

#define EXTENSION_VIDEO_WMV			".wmv"
#define MIME_TYPE_VIDEO_WMV			"video/x-ms-wmv"
#define PROTINFO_VIDEO_WMV			"http-get:*:video/x-ms-wmv:*"

#define EXTENSION_VIDEO_MPEG		".mpg"
#define MIME_TYPE_VIDEO_MPEG		"video/mpeg"
#define PROTINFO_VIDEO_MPEG			"http-get:*:video/mpeg:*"

#define EXTENSION_VIDEO_MOV			".mov"
#define MIME_TYPE_VIDEO_MOV			"video/quicktime"
#define PROTINFO_VIDEO_MOV			"http-get:*:video/quicktime:*"

#define EXTENSION_IMAGE_JPG			".jpg"
#define MIME_TYPE_IMAGE_JPG			"image/jpeg"
#define PROTINFO_IMAGE_JPG			"http-get:*:image/jpeg:*"

#define EXTENSION_IMAGE_JPEG		".jpeg"
#define MIME_TYPE_IMAGE_JPEG		"image/jpeg"
#define PROTINFO_IMAGE_JPEG			"http-get:*:image/jpeg:*"

#define EXTENSION_IMAGE_BMP			".bmp"
#define MIME_TYPE_IMAGE_BMP			"image/bmp"
#define PROTINFO_IMAGE_BMP			"http-get:*:image/bmp:*"

#define EXTENSION_PLAYLIST_M3U		".m3u"
#define MIME_TYPE_PLAYLIST_M3U		"audio/mpegurl"
#define PROTINFO_PLAYLIST_M3U		"http-get:*:audio/mpegurl:*"

#define EXTENSION_PLAYLIST_ASX		".asx"
#define MIME_TYPE_PLAYLIST_ASX		"audio/x-ms-asx"
#define PROTINFO_PLAYLIST_ASX		"http-get:*:audio/x-ms-asx:*"


/*
 *	GetMimeType()
 *		extension				: the file extension, including the dot '.' char
 *
 *	Returns the mime-type of a file with the given file extension.
 *	The method returns static values.
 *	Returns NULL if mapping cannot be determined.
 *	DO NOT CALL FREE ON THE RETURNED VALUE.
 */
char* FileExtensionToMimeType (char* extension, int wide);

char* FileExtensionToProtocolInfo (char* extension, int wide);

/*
 *	MimeTypeToFileExtension()
 *		mime_type				: the mime-type
 *
 *	Returns the file extension of a file with the given mime type.
 *	The method returns static values.
 *	Returns NULL if mapping cannot be determined.
 *	DO NOT CALL FREE ON THE RETURNED VALUE.
 */
char* MimeTypeToFileExtension (char* mime_type);

/*
 *	FileExtensionToUpnpClass()
 *		extension				: the file extension, including the dot '.' char
 *
 *	Returns the upnp:class of a file with the given file extension.
 *	The method returns static values.
 *	Returns NULL if mapping cannot be determined.
 *	DO NOT CALL FREE ON THE RETURNED VALUE.
 */
char* FileExtensionToUpnpClass (char* extension, int wide);

/*
 *	FileExtensionToClassCode()
 */
unsigned int FileExtensionToClassCode (char* extension, int wide);

#endif
