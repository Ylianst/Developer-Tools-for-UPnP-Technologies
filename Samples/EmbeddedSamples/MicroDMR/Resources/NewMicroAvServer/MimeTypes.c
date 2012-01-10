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

#include <string.h>
#include <wchar.h>
#include "MimeTypes.h"
#include "MyString.h"

#ifdef UNDER_CE
#define stricmp _stricmp
#endif

#ifdef _POSIX
#define stricmp strcasecmp
#endif

#define _TMP_CHAR_BUFFER 10

char* FileExtensionToMimeType (char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER], temp2[_TMP_CHAR_BUFFER];
	char *retVal = NULL;

	if (wide)
	{
		strToUtf8(temp2, extension, _TMP_CHAR_BUFFER, 1, NULL);
		extension = temp2;
	}
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = MIME_TYPE_AUDIO_MPEG;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = MIME_TYPE_AUDIO_WMA;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_OGG) == 0)
	{
		retVal = MIME_TYPE_AUDIO_OGG;
	}
	else if (stricmp(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = MIME_TYPE_PLAYLIST_ASX;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = MIME_TYPE_VIDEO_ASF;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = MIME_TYPE_VIDEO_WMV;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG) == 0)
	{
		retVal = MIME_TYPE_VIDEO_MPEG;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MOV) == 0)
	{
		retVal = MIME_TYPE_VIDEO_MOV;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = MIME_TYPE_IMAGE_JPG;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = MIME_TYPE_IMAGE_JPEG;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = MIME_TYPE_IMAGE_BMP;
	}	
	else if (strcmpi(extension, EXTENSION_PLAYLIST_M3U) == 0)
	{
		retVal = MIME_TYPE_PLAYLIST_M3U;
	}
	else
	{
		retVal = "application/octet-stream";
	}

	return retVal;
}

char* FileExtensionToProtocolInfo (char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER], temp2[_TMP_CHAR_BUFFER];
	char *retVal = NULL;

	if (wide)
	{
		strToUtf8(temp2, extension, _TMP_CHAR_BUFFER, 1, NULL);
		extension = temp2;
	}
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = PROTINFO_AUDIO_MPEG;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = PROTINFO_AUDIO_WMA;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_OGG) == 0)
	{
		retVal = PROTINFO_AUDIO_OGG;
	}
	else if (stricmp(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = PROTINFO_PLAYLIST_ASX;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = PROTINFO_VIDEO_ASF;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = PROTINFO_VIDEO_WMV;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG) == 0)
	{
		retVal = PROTINFO_VIDEO_MPEG;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MOV) == 0)
	{
		retVal = PROTINFO_VIDEO_MOV;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = PROTINFO_IMAGE_JPG;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = PROTINFO_IMAGE_JPEG;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = PROTINFO_IMAGE_BMP;
	}	
	else if (strcmpi(extension, EXTENSION_PLAYLIST_M3U) == 0)
	{
		retVal = PROTINFO_PLAYLIST_M3U;
	}
	else
	{
		retVal = "http-get:*:application/octet-stream:*";
	}

	return retVal;
}

char* MimeTypeToFileExtension (char* mime_type)
{
	if (stricmp(mime_type, MIME_TYPE_AUDIO_MPEG) == 0)
	{
		return EXTENSION_AUDIO_MPEG;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_WMA) == 0)
	{
		return EXTENSION_AUDIO_WMA;
	}
	else if (stricmp(mime_type, MIME_TYPE_AUDIO_OGG) == 0)
	{
		return EXTENSION_AUDIO_OGG;
	}
	else if (stricmp(mime_type, MIME_TYPE_PLAYLIST_ASX) == 0)
	{
		return EXTENSION_PLAYLIST_ASX;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_ASF) == 0)
	{
		return EXTENSION_VIDEO_ASF;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_WMV) == 0)
	{
		return EXTENSION_VIDEO_WMV;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_MPEG) == 0)
	{
		return EXTENSION_VIDEO_MPEG;
	}
	else if (stricmp(mime_type, MIME_TYPE_VIDEO_MOV) == 0)
	{
		return EXTENSION_VIDEO_MOV;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_JPG) == 0)
	{
		return EXTENSION_IMAGE_JPG;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_JPEG) == 0)
	{
		return EXTENSION_IMAGE_JPEG;
	}
	else if (stricmp(mime_type, MIME_TYPE_IMAGE_BMP) == 0)
	{
		return EXTENSION_IMAGE_BMP;
	}
	else if (strcmpi(mime_type, MIME_TYPE_PLAYLIST_M3U) == 0)
	{
		return EXTENSION_PLAYLIST_M3U;
	}


	return ".bin";
}

char* FileExtensionToUpnpClass (char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER], temp2[_TMP_CHAR_BUFFER];
	char *retVal = NULL;

	if (wide)
	{
		strToUtf8(temp2, extension, _TMP_CHAR_BUFFER, 1, NULL);
		extension = temp2;
	}
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = CLASS_AUDIO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = CLASS_AUDIO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_OGG) == 0)
	{
		retVal = CLASS_AUDIO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = CLASS_VIDEO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = CLASS_VIDEO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG) == 0)
	{
		retVal = CLASS_VIDEO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MOV) == 0)
	{
		retVal = CLASS_VIDEO_ITEM;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = CLASS_IMAGE_ITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = CLASS_IMAGE_ITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = CLASS_IMAGE_ITEM;
	}	
	else if (strcmpi(extension, EXTENSION_PLAYLIST_M3U) == 0)
	{
		retVal = CLASS_PLAYLIST_M3U;
	}
	else if (stricmp(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = CLASS_PLAYLIST_ASX;
	}
	else
	{
		retVal = CLASS_ITEM;
	}

	return retVal;
}

unsigned int FileExtensionToClassCode (char* extension, int wide)
{
	char tempExtension[_TMP_CHAR_BUFFER], temp2[_TMP_CHAR_BUFFER];
	unsigned int retVal;

	if (wide)
	{
		strToUtf8(temp2, extension, _TMP_CHAR_BUFFER, 1, NULL);
		extension = temp2;
	}
	
	if (extension[0] != '.')
	{
		tempExtension[0] = '.';
		strcpy(tempExtension+1, extension);
		extension = tempExtension;
	}

	if (stricmp(extension, EXTENSION_AUDIO_MPEG) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_WMA) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_AUDIO_OGG) == 0)
	{
		retVal = CDS_MEDIACLASS_AUDIOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_ASF) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_WMV) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MPEG) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_VIDEO_MOV) == 0)
	{
		retVal = CDS_MEDIACLASS_VIDEOITEM;
	}
	else if (stricmp(extension, EXTENSION_IMAGE_JPG) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_JPEG) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (stricmp(extension, EXTENSION_IMAGE_BMP) == 0)
	{
		retVal = CDS_MEDIACLASS_IMAGEITEM;
	}	
	else if (strcmpi(extension, EXTENSION_PLAYLIST_M3U) == 0)
	{
		retVal = CDS_MEDIACLASS_PLAYLISTCONTAINER;
	}
	else if (strcmpi(extension, EXTENSION_PLAYLIST_ASX) == 0)
	{
		retVal = CDS_MEDIACLASS_PLAYLISTCONTAINER;
	}
	else
	{
		retVal = CDS_MEDIACLASS_ITEM;
	}

	return retVal;
}