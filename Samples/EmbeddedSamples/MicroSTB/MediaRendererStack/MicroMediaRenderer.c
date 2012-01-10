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


#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32_WCE
	#include <winsock.h>
	#include "ILibParsers.h"
	#include "UpnpMicroStack.h"

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif WIN32
	//#error "Windows compilation detected"
	#include <crtdbg.h>
	#include <winsock.h>
	#include "ILibParsers.h"
	#include "UpnpMicroStack.h"

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#elif _POSIX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include "ILibParsers.h"
	#include "UpnpMicroStack.h"
	#include <semaphore.h>
	#include <assert.h>
	#define strnicmp strncasecmp
#endif

#include "MicroMediaRenderer.h"
#include "MyString.h"
#include "Utility.h"


#ifndef UNDER_CE
	#include <assert.h>
	#define ASSERT assert
#endif

#ifdef UNDER_CE
	#define strnicmp _strnicmp
#endif

#ifdef _VERBOSE
	#define VERBOSEONLY(x) x
#endif

#ifndef _VERBOSE
	#define VERBOSEONLY(x) 
#endif

#ifdef _DEBUG
	#define DEBUGONLY(x)	x

	#define MMR_MALLOC	mmr_malloc
	#define MMR_FREE	mmr_free

	int mmr_malloc_counter = 0;
	void* mmr_malloc (int size)
	{
		++mmr_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("mmr_malloc_counter=%d\r\n", mmr_malloc_counter);
		#endif
		return MALLOC(size);
	}

	void mmr_free (void *ptr)
	{
		--mmr_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("mmr_malloc_counter=%d\r\n", mmr_malloc_counter);
		#endif
		FREE(ptr);
	}
#endif

#ifndef _DEBUG
	#define DEBUGONLY(x)	
	
	#define MMR_MALLOC	MALLOC
	#define MMR_FREE	FREE
#endif



#define NULL_CHAR '\0'
char* MMR_EMPTY_STRING = "";

#define AVT_ERROR_TRANSITION_NOT_AVAILABLE			701
#define AVT_ERROR_NO_CONTENTS						702
#define AVT_ERROR_READ_ERROR						703
#define AVT_ERROR_FORMAT_NOT_SUPPORTED_PLAYBACK		704
#define AVT_ERROR_TRANSPORT_IS_LOCKED				705
#define AVT_ERROR_TRANSPORT_WRITE_ERROR				706
#define AVT_ERROR_TRANSPORT_NOT_WRITABLE			707
#define AVT_ERROR_FORMAT_NOT_SUPPORTED_RECORDING	708
#define AVT_ERROR_MEDIA_FULL						709
#define AVT_ERROR_SEEK_MODE_NOT_SUPPORTED			710
#define AVT_ERROR_ILLEGAL_SEEK_TARGET				711
#define AVT_ERROR_PLAY_MODE_NOT_SUPPORTED			712
#define AVT_ERROR_ILLEGAL_MIME_TYPE					714
#define AVT_ERROR_CONTENT_BUSY						715
#define AVT_ERROR_RESOURCE_NOT_FOUND				716
#define AVT_ERROR_PLAY_SPEED_NOT_SUPPORTED			717
#define AVT_ERROR_INVALID_INSTANCE_ID				718
#define AVT_ERROR_CUSTOM_URI_TOO_LONG				810
#define AVT_ERROR_LOGIC_ERROR						899

#define AVT_ERRORSTR_TRANSITION_NOT_AVAILABLE		"Immediate transition from current transport state to desired transport state is not supported."
#define AVT_ERRORSTR_NO_CONTENTS					"Media does not contain any content that can be played."
#define AVT_ERRORSTR_READ_ERROR						"The media cannot be read because of dust/scratch."
#define AVT_ERRORSTR_FORMAT_NOT_SUPPORTED_PLAYBACK	"The storage format is not supported by this device for playback."
#define AVT_ERRORSTR_TRANSPORT_IS_LOCKED			"The transport is \"hold locked\"."
#define AVT_ERRORSTR_TRANSPORT_WRITE_ERROR			"The media cannot be written because of dust/scratch."
#define AVT_ERRORSTR_TRANSPORT_NOT_WRITABLE			"The media is write protected or not writable."
#define AVT_ERRORSTR_FORMAT_NOT_SUPPORTED_RECORDING "The storage format is not supported by this device for recording."
#define AVT_ERRORSTR_MEDIA_FULL						"No FREE space on media."
#define AVT_ERRORSTR_SEEK_MODE_NOT_SUPPORTED		"Seek mode not supported."
#define AVT_ERRORSTR_ILLEGAL_SEEK_TARGET			"Illegal seek target."
#define AVT_ERRORSTR_PLAY_MODE_NOT_SUPPORTED		"Unsupported play mode."
#define AVT_ERRORSTR_ILLEGAL_MIME_TYPE				"The specified resource has a mime-type that is unsupported.."
#define AVT_ERRORSTR_CONTENT_BUSY					"Resources is already being played by other means."
#define AVT_ERRORSTR_RESOURCE_NOT_FOUND				"The specified resource could not be found on the UPNP network."
#define AVT_ERRORSTR_PLAY_SPEED_NOT_SUPPORTED		"The specified play speed is not supported."
#define AVT_ERRORSTR_INVALID_INSTANCE_ID			"Specified instance ID does not exist."
#define AVT_ERRORSTR_CUSTOM_URI_TOO_LONG			"The specified URI is exceeds the URI length supported by this device."
#define AVT_ERRORSTR_LOGIC_ERROR					"Logic error encountered in implementation."

#define CM_ERROR_INCOMPATIBLE_PROTOCOL_INFO			701
#define CM_ERROR_INCOMPATIBLE_DIRECTIONS			702 
#define CM_ERROR_INSUFFICIENT_NETWORK_RESOURCES		703
#define CM_ERROR_LOCAL_RESTRICTIONS					704
#define CM_ERROR_ACCESS_DENIED						705
#define CM_ERROR_INVALID_CONNECTION_REFERENCE		706
#define CM_ERROR_NOT_IN_NETWORK						707
#define CM_ERROR_UNINTENDED_LOGIC_ERROR				800

#define CM_ERRORSTR_INCOMPATIBLE_PROTOCOL_INFO		"Invalid protocolInfo string specified."
#define CM_ERRORSTR_INCOMPATIBLE_DIRECTIONS			"The direction of the stream is not compatible with the specified protocolInfo string." 
#define CM_ERRORSTR_INSUFFICIENT_NETWORK_RESOURCES	"Cannot allocate the connection for network reasons."
#define CM_ERRORSTR_LOCAL_RESTRICTIONS				"Current settings or state prevent the connection from being created."
#define CM_ERRORSTR_ACCESS_DENIED					"Current settings/configuration do not allow the caller to call this method."
#define CM_ERRORSTR_INVALID_CONNECTION_REFERENCE	"Invalid connection reference."
#define CM_ERRORSTR_NOT_IN_NETWORK					"The specified network in the protocolInfo string is not accessible by this desvice."
#define CM_ERRORSTR_UNINTENDED_LOGIC_ERROR			"Not a UPnP-AV standard error - indicates an unintended logic error."

#define RCS_ERROR_INVALID_INSTANCE_ID				718
#define RCS_ERROR_INVALID_CHANNEL					402

#define RCS_ERRORSTR_INVALID_INSTANCE_ID			"Specified instance ID does not exist."
#define RCS_ERRORSTR_INVALID_CHANNEL				"Cannot specify unsupported channel."

/*
 *	Standard duration string to indicate no time.
 */
#define AVT_ZERO_TIME "00:00:00"

/*
 *	Maximum memory allocation for time string in format
 *	hh:mm:ss is 9 bytes including null terminator.
 *	We also must be able to accomodate the
 *	NOT_IMPLEMENTED string.
 */
#define TIME_STRING_SIZE 16


/*
 *	Intel Micro Renderer Settings 
 */

/*
 *	Provides a complete listing of supported protocolInfo strings.
 *	This list will be superset of the one indicated in
 *	enum MR_SupportedProtocolInfo because other vendors sometimes use
 *	variants that mean the same protocolInfo.
 */
#define RENDERER_SINKPROTOCOLINFO "http-get:*:audio/mpegurl:*,http-get:*:video/mpegurl:*,http-get:*:audio/mp3:*,http-get:*:audio/mpeg:*,http-get:*:audio/x-ms-wma:*,http-get:*:audio/wma:*,http-get:*:audio/mpeg3:*,http-get:*:video/x-ms-wmv:*,http-get:*:video/mpeg:*"

/*
 *	This device is never a source.
 */
#define RENDERER_SOURCEPROTOCOLINFO ""

/*
 *	We always render from the network.
 */
#define RENDERER_CAPABILITY_PLAYMEDIA "NETWORK"

/*
 *	Only action that can become disabled is seek... although
 *	this is not technically true, because we can always SEEK
 *	to a track index. This string is really used only for GetCurrentTransportActions
 *	and the CurrentTransportActions state variable.
 */
#define RENDERER_TRANSPORT_ACTIONS "Play,Stop,Seek,Next,Previous"


/*
 *	String Tables
 */

/*
 *	Ensure that this list matches the MR_Enum_AudioChannels enumeration.
 */
const char* MR_AUDIOCHANNELS[] = {"Master","LF","RF"};

/*
 *	Ensure that this list matches MR_Enum_PlayModes enumeration.
 */
const char* MR_PLAYMODES[]     = {"NORMAL","REPEAT_ONE","REPEAT_ALL","RANDOM","SHUFFLE"};

/*
 *	Ensure that this list matches MR_Enum_States enumeration.
 */
const char* MR_STATES[]        = {"STOPPED","PAUSED_PLAYBACK","PLAYING","TRANSITIONING","NO_MEDIA_PRESENT"};

/*
 *	Ensure that this list matches MR_Enum_Status enumeration.
 */
const char* MR_STATUS[]        = {"OK","ERROR"};

/*
 *	Ensure that this list matches the MR_Enum_SeekModes enumeration.
 */
const char* MR_SEEKMODES[]     = {"REL_TIME","TRACK_NR"};

/*
 *	Ensure that this list matches the MR_SupportedProtocolInfo enumeration.
 */
const char* MR_PROTINFO[]		= 
{
	"http-get:*:*:*",
	"http-get:*:audio/mpegurl:*", 
	"http-get:*:video/mpegurl:*", 
	"http-get:*:audio/mpeg:*", 
	"http-get:*:audio/x-ms-wma:*", 
	"http-get:*:video/mpeg:*", 
	"http-get:*:video/x-ms-wmv:*" 
};

/* Callback Pointers */

void (*MROnVolumeChangeRequest) (enum MR_Enum_AudioChannels,unsigned short) = NULL;
void (*MROnMuteChangeRequest) (enum MR_Enum_AudioChannels,int) = NULL;
void (*MROnMediaChangeRequest) (const char* mediaUri) = NULL;
void (*MROnGetPositionRequest) (int* seconds, int* absSeconds, int* count, int* absCount) = NULL;
void (*MROnMediaPositionChangeRequest) (int track,int totaltracks,int position) = NULL;
void (*MROnNextPreviousRequest) (int trackdelta) = NULL;
void (*MROnStateChangeRequest) (enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeeds) = NULL;
void (*MROnPlayModeChangeRequest) (enum MR_Enum_PlayModes playmode) = NULL;
void (*MROnSeekRequest) (enum MR_Enum_SeekModes seekMode, int seekPosition) = NULL;

/* number of volume channels: master, left-front, right-front */
#define NUM_VOL_CHANNELS 3

/* Complete Renderer State */
struct MRDataObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void* UpnpMicroStack;
	void* LifeTimeMonitor;

	/* RCS related state variables */
	unsigned short	Rcs_Volume[NUM_VOL_CHANNELS];
	int				Rcs_Mute[NUM_VOL_CHANNELS];
	
	/* AVT related state variables */
	int				Avt_State;
	int				Avt_PlayMode;
	int				Avt_Status;
	enum MR_SupportedProtocolInfo Cm_ProtInfo;

	char*			Media_URI;
	int				Media_TrackTotal;
	int				Media_DurationTotal;
	int				Media_CounterTotal;
	char*			Media_Creator;
	char*			Media_Title;

	char*			Track_URI;
	int				Track_Number;
	int				Track_Duration;
	int				Track_CounterTotal;
	char*			Track_Title;
	char*			Track_Creator;

	unsigned int	RCSChangedFlags;
	unsigned int	AVTChangedFlags;
	int				ModerationFlag;

	sem_t			LockLastChanges;

	int				SeekEnabled;
};

enum ChangedFlagsEnum
{
	AVT_STATE			= 0x00000001,
	AVT_PLAYMODE		= 0x00000002,
	AVT_STATUS			= 0x00000004,
	AVT_SEEK			= 0x00000008,

	MEDIA_URI			= 0x00000010,
	MEDIA_TOTALS		= 0x00000020,
	MEDIA_METADATA		= 0x00000040,

	TRACK_URI			= 0x00000100,
	TRACK_DURATIONS		= 0x00000200,
	TRACK_METADATA		= 0x00000400,

	RCS_VOLUME			= 0x00001000,
	RCS_MUTE			= 0x00002000
};

/*
 *	This module hard-codes a single MRDataObject because
 *	we determined it is more efficient to not send
 *	an addtional parameter on the callstack for every
 *	method in MicroMediaRenderer.h interface. 
 *
 *	That being said, developers can always modify the
 *	interface to make it scalable across multiple
 *	MediaRenderer devices running as a single UPnP device.
 *	This is appropriate if the UpnP device will support
 *	multiple simultaneous playback streams.
 */
struct MRDataObject* MR = NULL;

/*
 *	Given a string in hh:mm:ss, determine
 *	the duration in seconds.
 */
int CalculateSeconds (const char *timeString)
{
	int len = 0;
	int i;
	int factor = 0;
	int num = 0;
	int retVal = 0;
	char buffer[2];

	len = (int) strlen(timeString);
	buffer[1] = '\0';

	for (i = len-1; i > 0; i--)
	{
		buffer[0] = timeString[i];

		if (buffer[0] == ':')
		{
		}
		else if ((buffer[0] >= '0') && (buffer[0] <= '9'))
		{
			switch (factor)
			{
			case 0:
				factor = 1;		/* seconds */
				break;

			case 1:
				factor = 10;	/* 10 seconds */
				break;

			case 10:
				factor = 60;	/* 1 minute */
				break;

			case 60:
				factor = 600;	/* 1 hour */
				break;

			case 600:			
				factor = 14400;	/* 1 day */
				break;

			case 14400:
				factor = 144000;/* 10 days */
				break;
			}

			num = atoi(buffer) * factor;
			retVal += num;
		}
		else
		{
			/*
			 *	If we don't know what the char is, then there's
			 *	a logic error.
			 */
			printf("CalculateSeconds(%s) encountered illegal char.\r\n", timeString);
			return -1;
		}
	}

	return retVal;
}

/*
 *	General method that takes a string (str), an array of strings, and the length of the array
 *	and returns the index within the array if 'str' is found. Comparisons are case sensitive.
 */
int FindStringInArray(const char* str,const char** strarray,const int strarraylen)
{
	int i;
	for (i=0;i<strarraylen;i++) {if (strcmp(str,strarray[i]) == 0) {return i;}}
	return -1;
}

/*
 *	string formatters for LastChange variable. 
 */
#define LASTCHANGE_HEADER "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/%s/\"><InstanceID val=\"0\">"
#define LASTCHANGE_VAR_STRING "<%s val=\"%s\"/>"
#define LASTCHANGE_VAR_STRING_START "<%s val=\""
#define LASTCHANGE_VAR_STRING_END "\"/>"
#define LASTCHANGE_VAR_INT "<%s val=\"%d\"/>"
#define LASTCHANGE_ATTRIB_STRING_VAR_INT "<%s %s=\"%s\" val=\"%d\"/>"
#define LASTCHANGE_FOOTER "</InstanceID></Event>"

/*
#define DIDL_HEADER "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"><item id=\"X\" parentID=\"Y\" restricted=\"1\">"
#define DIDL_TITLE_START "<dc:title>"
#define DIDL_TITLE_END "</dc:title>"
#define DIDL_CREATOR_START "<dc:creator>"
#define DIDL_CREATOR_END "</dc:creator>"
#define DIDL_FOOTER "<upnp:class>object.item</upnp:class></item></DIDL-Lite>"
*/

#define DIDL_ESCAPED_HEADER "&lt;DIDL-Lite xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot; xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot;&gt;&lt;item id=&quot;X&quot; parentID=&quot;Y&quot; restricted=&quot;1&quot;&gt;"
#define DIDL_ESCAPED_TITLE_START "&lt;dc:title&gt;"
#define DIDL_ESCAPED_TITLE_END "&lt;/dc:title&gt;"
#define DIDL_ESCAPED_CREATOR_START "&lt;dc:creator&gt;"
#define DIDL_ESCAPED_CREATOR_END "&lt;/dc:creator&gt;"
#define DIDL_ESCAPED_FOOTER "&lt;upnp:class&gt;object.item&lt;/upnp:class&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;"

#define DIDL_ITEM "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"><item id=\"X\" parentID=\"Y\" restricted=\"1\"><dc:title>%s</dc:title><dc:creator>%s</dc:creator><upnp:class>object.item</upnp:class></item></DIDL-Lite>"
#define DIDL_ITEM_LEN 318

#define RCS "RCS"
#define AVT "AVT"

/*
 *	Allow 5kb staging area for LastChange
 *	variable staging area, which is suitable
 *	for a single avt/rcs instance.
 *
 *	This allows for single instance's eventing
 *	of
 *		TransportState,
 *		TransportStatus,
 *		Playode,
 *		NumberOfTracks,
 *		CurrentTrack,
 *		CurrentTrackDuration,
 *		CurrentMediaDuration,
 *		CurrentTrackUri,
 *		AVTransportUri.
 *
 *	URIs are assumed to have a 1024 byte max length,
 *	including room for XML escaped chars. 
 *
 *	Duration strings are of length TIME_STRING_LENGTH-1.
 *
 *	Assume creator and title info collectively are 1KB max.
 */
#define LASTCHANGE_SIZE 10*1024

/*
 *	buffer size used with didl lite metadata values 
 */
#define TEMP_BUFSIZE 512

/*
 *	Special value for time counters - indicates
 *	that the time value is not implemented.
 */
#define TIME_NOT_IMPLEMENTED 2147483647
#define TIME_STRING_NOT_IMPLEMENTED "NOT_IMPLEMENTED"
/*
 *	Special value for time counters - indicates
 *	that the end of the track has been reached.
 */
#define TIME_END_OF_TRACK (TIME_NOT_IMPLEMENTED - 1)
#define TIME_STRING_END_OF_TRACK "END OF TRACK"
#define SEC_PER_HOUR 3600
#define SEC_PER_MIN 60



/*
 *	Prints the number of seconds in hh:mm:ss format.
 *	Negative values return TIME_NOT_IMPLEMENTED.
 */
void SetTimeString(char* str, int intTime)
{
	if (intTime >= 0)
	{
		sprintf(str,"%02d:%02d:%02d",((intTime/3600)%60),((intTime/60)%60),(intTime%60));
	}
	else
	{
		strcpy(str, TIME_STRING_NOT_IMPLEMENTED);
	}
}

/*
 *	This method examines the 'MR' object and creates
 *	the appropriately formatted LastChange string given
 *	the current state.
 */
void UpdateAvtLastChange()
{
	char *LastChangeValue;
	char *cp;
	char timeString[TIME_STRING_SIZE];
	char *temp;
	char *headerPos;
	int lcfLen;

	#ifdef _DEBUG
	printf("UpdateAvtLastChange() continue=%d state=%d\r\n", MR->AVTChangedFlags, MR->Avt_State);
	#endif

	if (MR->AVTChangedFlags == 0) return;

	temp = (char*) MMR_MALLOC (TEMP_BUFSIZE);
	LastChangeValue = (char*) MMR_MALLOC(LASTCHANGE_SIZE);
	memset(LastChangeValue, NULL_CHAR, LASTCHANGE_SIZE);
	cp = LastChangeValue;

	cp += sprintf(LastChangeValue, LASTCHANGE_HEADER, AVT);
	headerPos = cp;

	if (MR->AVTChangedFlags & AVT_STATE)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING, "TransportState", MR_STATES[MR->Avt_State]);
	}

	if (MR->AVTChangedFlags & AVT_STATUS)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING, "TransportStatus", MR_STATUS[MR->Avt_Status]);
	}

	if (MR->AVTChangedFlags & AVT_PLAYMODE)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING, "CurrentPlayMode", MR_PLAYMODES[MR->Avt_PlayMode]);
	}

	/*
	 *	DEPRECATED:
	 *	Although this device may disable "seek on position" operations - it can't disable
	 *	the seek action because we can still seek on track targets.
	 *
	if (MR->AVTChangedFlags & AVT_SEEK)
	{
		if (MR->SeekEnabled != 0)
		{
			cp += sprintf(cp, LASTCHANGE_VAR_STRING, "CurrentTransportActions", RENDERER_TRANSPORT_ACTIONS_WITH_SEEK );
		}
		else
		{
			cp += sprintf(cp, LASTCHANGE_VAR_STRING, "CurrentTransportActions", RENDERER_TRANSPORT_ACTIONS_NO_SEEK);
		}
	}
	 */

	if (MR->AVTChangedFlags & MEDIA_URI)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_START, "AVTransportURI");
		cp += ILibXmlEscape(cp, MR->Media_URI);
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_END);
	}

	if (MR->AVTChangedFlags & MEDIA_TOTALS)
	{
		SetTimeString(timeString, MR->Media_DurationTotal);
		cp += sprintf(cp, LASTCHANGE_VAR_STRING, "CurrentMediaDuration", timeString);
		cp += sprintf(cp, LASTCHANGE_VAR_INT, "NumberOfTracks", MR->Media_TrackTotal);

		/*
		 *	AVTransport spec does not specify that maximum counter value (MR->Media_CounterTotal)
		 *	can be sent through the LastChange event. Do nothing until specification is modified.
		 */
	}

		/*
		 *	AVTransport spec does not specify that media metadata is sent
		 *	through LastChange. Do not event media metadata.
		 *
	if (MR->AVTChangedFlags & MEDIA_METADATA)
	{
	}
		 */

	if (MR->AVTChangedFlags & TRACK_URI)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_START, "CurrentTrackURI");
		cp += ILibXmlEscape(cp, MR->Track_URI);
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_END);

		cp += sprintf(cp, LASTCHANGE_VAR_INT, "CurrentTrack", MR->Track_Number);
	}

	if (MR->AVTChangedFlags & TRACK_DURATIONS)
	{
		SetTimeString(timeString, MR->Track_Duration);
		cp += sprintf(cp, LASTCHANGE_VAR_STRING, "CurrentTrackDuration", timeString);

		/* AVTransport does not event max counter for a track */
	}

	if (MR->AVTChangedFlags |= TRACK_METADATA)
	{
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_START, "CurrentTrackEmbeddedMetaData");

		/*
		 *	header, footer, DIDL elements are already XML escaped twice as required.
		 *	Still need to escape the data twice.
		 */
		cp += sprintf(cp, DIDL_ESCAPED_HEADER);

		ASSERT(MR->Track_Creator);
		ASSERT(MR->Track_Title);

		//if (MR->Track_Title[0] != NULL_CHAR)
		{
			cp += sprintf(cp, DIDL_ESCAPED_TITLE_START);
			
			/* doubly-escape creator */
			memset(temp, NULL_CHAR, TEMP_BUFSIZE);
			cp += ILibXmlEscape(cp, MR->Track_Title);
			cp += sprintf(cp, DIDL_ESCAPED_TITLE_END);
		}

		//if (MR->Track_Creator[0] != NULL_CHAR)
		{
			cp += sprintf(cp, DIDL_ESCAPED_CREATOR_START);
			
			/* doubly-escape creator */
			memset(temp, NULL_CHAR, TEMP_BUFSIZE);
			cp += ILibXmlEscape(cp, MR->Track_Creator);
			cp += sprintf(cp, DIDL_ESCAPED_CREATOR_END);
		}

		cp += sprintf(cp, DIDL_ESCAPED_FOOTER);
		cp += sprintf(cp, LASTCHANGE_VAR_STRING_END);
	}

	/* appropriately close the xml */
	if (cp != headerPos)
	{
		/* write avt event footer */
		lcfLen = (int) strlen(LASTCHANGE_FOOTER);
		memcpy(cp, LASTCHANGE_FOOTER, lcfLen+1);

		/* cp should not be beyond the last byte in LastChangeValue */
		ASSERT( (cp+lcfLen+1) <= (LastChangeValue+LASTCHANGE_SIZE) );

		#ifdef _DEBUG
		printf("UpdateAvtLastChange(): UpnpSetState_MediaRenderer_AVTransport_LastChange(%s).\r\n", LastChangeValue);
		#endif

		UpnpSetState_MediaRenderer_AVTransport_LastChange(MR->UpnpMicroStack, LastChangeValue);
	}

	MR->AVTChangedFlags = 0;

	MMR_FREE (LastChangeValue);
	MMR_FREE (temp);
}

/*
 *	This method examines the 'MR' object and creates
 *	the appropriately formatted LastChange string given
 *	the current state.
 */
void UpdateRcsLastChange()
{
	int i;
	char *LastChangeValue;
	char *cp;
	int mute;
	char *headerPos;

	if (MR->RCSChangedFlags == 0) return;

	LastChangeValue = (char*) MMR_MALLOC(LASTCHANGE_SIZE);
	memset(LastChangeValue, NULL_CHAR, LASTCHANGE_SIZE);
	cp = LastChangeValue;

	cp += sprintf(LastChangeValue, LASTCHANGE_HEADER, RCS);
	headerPos = cp;

	/* Appropriately update volume and mute for each channel. */
	if (MR->RCSChangedFlags & RCS_VOLUME)
	{
		for (i=0; i < NUM_VOL_CHANNELS; i++)
		{
			cp += sprintf(cp, LASTCHANGE_ATTRIB_STRING_VAR_INT, "Volume", "channel", MR_AUDIOCHANNELS[i], MR->Rcs_Volume[i]);
		}
	}

	if (MR->RCSChangedFlags & RCS_MUTE)
	{
		for (i=0; i < NUM_VOL_CHANNELS; i++)
		{
			mute = 0;
			if (MR->Rcs_Mute[i] == 0) { mute = 0; } else { mute = 1; }
			cp += sprintf(cp, LASTCHANGE_ATTRIB_STRING_VAR_INT, "Mute", "channel", MR_AUDIOCHANNELS[i], mute);
		}
	}

	/* appropriately close the xml */
	if (cp != headerPos)
	{
		/* write avt event footer */
		memcpy(cp, LASTCHANGE_FOOTER, strlen(LASTCHANGE_FOOTER)+1);

		UpnpSetState_MediaRenderer_RenderingControl_LastChange(MR->UpnpMicroStack, LastChangeValue);
	}

	MR->RCSChangedFlags = 0;

	MMR_FREE (LastChangeValue);
}

/*
 *	This method executes when the Lifetime monitor executes
 *	its callback. This method basically completes the moderation
 *	of the delivery of LastChange state variables.
 */
void MmrEventModerationCallbackSink(void* data)
{
#ifdef _DEBUG
	printf("MmrEventModerationCallbackSink()\r\n");
#endif

	sem_wait(&(MR->LockLastChanges));

	MR->ModerationFlag = 0;
	UpdateRcsLastChange();
	MR->ModerationFlag = 0;
	UpdateAvtLastChange();

	sem_post(&(MR->LockLastChanges));
}

/*
 *	This method enables moderation of the AVT's LastChange
 *	state variable by using lifetime monitor to
 *	moderate the calls.
 *
 *	Assumes caller has locked state.
 */
void UpdateAvtLastChangeModerated()
{
	#ifdef _DEBUG
	printf("UpdateAvtLastChangeModerated(): flags=%d moderationFlag=%d state=%d\r\n", MR->AVTChangedFlags, MR->ModerationFlag, MR->Avt_State);
	#endif

	
	/*
	 *	If nothing changed, do nothing.
	 */
	if (MR->AVTChangedFlags == 0) return;

	/*
	 *	If we don't have a moderated event pending,
	 *	then go ahead and event LastChange.
	 *	Then queue up a pending request to event
	 *	LastChange again. When the callback
	 *	executes, if nothing has changed, then 
	 *	nothing gets evented.
	 */
	if (MR->ModerationFlag == 0)
	{

		MR->ModerationFlag = 1;
		UpdateAvtLastChange();
		ILibLifeTime_Add(MR->LifeTimeMonitor,MR,1,MmrEventModerationCallbackSink, NULL);
	}
}

/*
 *	This method enables moderation of the RCS's LastChange
 *	state variable by using lifetime monitor to
 *	moderate the calls.
 *
 *	Assumes caller has locked state.
 */
void UpdateRcsLastChangeModerated()
{
	/*
	 *	If nothing changed, do nothing.
	 */
	if (MR->RCSChangedFlags == 0) return;

	/*
	 *	If we don't have a moderated event pending,
	 *	then go ahead and event LastChange.
	 *	Then queue up a pending request to event
	 *	LastChange again. When the callback
	 *	executes, if nothing has changed, then 
	 *	nothing gets evented.
	 */
	if (MR->ModerationFlag == 0)
	{
		MR->ModerationFlag = 1;
		UpdateRcsLastChange();
		ILibLifeTime_Add(MR->LifeTimeMonitor,MR,1,MmrEventModerationCallbackSink, NULL);
	}
}

/* Reports to UPnP network that specified volume channel has a new volume value.*/
void MRSetVolume(enum MR_Enum_AudioChannels channel,unsigned short volume)
{
	sem_wait(&(MR->LockLastChanges));
	if (MR->Rcs_Volume[channel] != volume)
	{
		MR->Rcs_Volume[channel] = volume;
		MR->RCSChangedFlags |= RCS_VOLUME;
		UpdateRcsLastChangeModerated();
	}
	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that specified mute channel has a new value */
void MRSetMute(enum MR_Enum_AudioChannels channel,int mute)
{
	sem_wait(&(MR->LockLastChanges));
	if (MR->Rcs_Mute[channel] != mute)
	{
		MR->Rcs_Mute[channel] = mute;
		MR->RCSChangedFlags |= RCS_MUTE;
		UpdateRcsLastChangeModerated();
	}
	sem_post(&(MR->LockLastChanges));
}

/* Reports the device's current mediaUri to UPnP network. */
void MRSetMediaUri(const char* mediaUri)
{
	ASSERT(mediaUri != NULL);
	ASSERT(MR->Media_URI != NULL);

	sem_wait(&(MR->LockLastChanges));
	if (strcmp(MR->Media_URI, mediaUri))
	{
		MMR_FREE(MR->Media_URI);
		MR->Media_URI = (char*)MMR_MALLOC((int)strlen(mediaUri)+1);
		strcpy(MR->Media_URI, mediaUri);

		MR->AVTChangedFlags |= MEDIA_URI;
		UpdateAvtLastChangeModerated();
	}
	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that current media has specified metadata. */
void MRSetMediaInfo(enum MR_SupportedProtocolInfo protInfo, const char* mediaTitle, const char* mediaCreator)
{
	int changed;

	ASSERT(mediaTitle != NULL);
	ASSERT(mediaCreator != NULL);

	ASSERT(MR->Media_Title != NULL);
	ASSERT(MR->Media_Creator != NULL);

	sem_wait(&(MR->LockLastChanges));

	changed = 0;
	if (strcmp(mediaTitle, MR->Media_Title)) 
	{
		MMR_FREE(MR->Media_Title);
		MR->Media_Title = (char*)MMR_MALLOC((int)strlen(mediaTitle)+1);
		strcpy(MR->Media_Title, mediaTitle);
		changed = 1;
	}

	if (strcmp(mediaCreator, MR->Media_Creator))
	{
		MMR_FREE(MR->Media_Creator);
		MR->Media_Creator = (char*)MMR_MALLOC((int)strlen(mediaCreator)+1);
		strcpy(MR->Media_Creator, mediaCreator);
		changed = 1;
	}

	if (changed) 
	{ 
		MR->AVTChangedFlags |= MEDIA_METADATA;
		UpdateAvtLastChangeModerated(); 
	}

	/*
	 *	ProtocolInfo does not affect avtransport.
	 *	Given that a device always has a connection - we won't bother
	 *	eventing protocolInfo, especially since the connection
	 *	remains "0" and there's no way to report protocolInfo by itself.
	 */
	if (protInfo != MR->Cm_ProtInfo)
	{
		MR->Cm_ProtInfo = protInfo;
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that current media has new track total. */
void MRSetMediaTotals(int totalTracks, int totalMediaDuration, int totalMediaCounter)
{
	int changed = 0;

	sem_wait(&(MR->LockLastChanges));
	
	if (totalTracks != MR->Media_TrackTotal)
	{
		MR->Media_TrackTotal = totalTracks;
		changed = 1;
	}

	if (totalMediaDuration != MR->Media_DurationTotal)
	{
		MR->Media_DurationTotal = totalMediaDuration;
		changed = 1;
	}

	if (totalMediaCounter != MR->Media_CounterTotal)
	{
		MR->Media_CounterTotal = totalMediaCounter;
		changed = 1;
	}

	if (changed)
	{
		MR->AVTChangedFlags |= MEDIA_TOTALS;
		UpdateAvtLastChangeModerated(); 
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that current media has new track. */
void MRSetTrack(const char* trackUri, int trackNumber)
{
	ASSERT(trackUri != NULL);
	ASSERT(MR->Track_URI != NULL);

	sem_wait(&(MR->LockLastChanges));

	if (
		(trackNumber != trackNumber) ||
		(strcmp(MR->Track_URI, trackUri))
		)
	{
		MMR_FREE(MR->Track_URI);
		MR->Track_URI = (char*)MMR_MALLOC((int)strlen(trackUri)+1);
		strcpy(MR->Track_URI,trackUri);
		MR->Track_Number = trackNumber;
		
		MR->AVTChangedFlags |= TRACK_URI;
		UpdateAvtLastChangeModerated();
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that current track has a new duration. */
void MRSetTrackDurationInfo(int trackDuration, int trackCounterTotal)
{
	int changed = 0;

	sem_wait(&(MR->LockLastChanges));

	if (MR->Track_Duration != trackDuration)
	{
		MR->Track_Duration = trackDuration;
		changed = 1;
	}

	/*
	 *	this value should not be confused with
	 *	total number of tracks. This value represents
	 *	the counter position within a track.
	 */
	if (MR->Track_CounterTotal != trackCounterTotal)
	{
		MR->Track_CounterTotal = trackCounterTotal;
		changed = 1;
	}

	if (changed) 
	{
		MR->AVTChangedFlags |= TRACK_DURATIONS;
		UpdateAvtLastChangeModerated(); 
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network that current track has a new metadata. */
void MRSetTrackMetadata(enum MR_SupportedProtocolInfo protInfo, const char* title, const char* creator)
{
	int changed = 0;

	ASSERT(title != NULL);
	ASSERT(creator != NULL);

	ASSERT(MR->Track_Title != NULL);
	ASSERT(MR->Track_Creator != NULL);

	sem_wait(&(MR->LockLastChanges));

	if (strcmp(MR->Track_Creator, creator))
	{
		MMR_FREE(MR->Track_Creator);
		MR->Track_Creator = (char*)MMR_MALLOC((int)strlen(creator)+1);
		strcpy(MR->Track_Creator,creator);
		changed = 1;
	}

	if (strcmp(MR->Track_Title, title))
	{
		MMR_FREE(MR->Track_Title);
		MR->Track_Title = (char*)MMR_MALLOC((int)strlen(title)+1);
		strcpy(MR->Track_Title,title);
		changed = 1;
	}

	if (changed) 
	{
		MR->AVTChangedFlags |= TRACK_METADATA;
		UpdateAvtLastChangeModerated(); 
	}

	/*
	 *	ProtocolInfo does not affect avtransport.
	 *	Given that a device always has a connection - we won't bother
	 *	eventing protocolInfo, especially since the connection
	 *	remains "0" and there's no way to report protocolInfo by itself.
	 */
	if (protInfo != MR->Cm_ProtInfo)
	{
		MR->Cm_ProtInfo = protInfo;
	}

	sem_post(&(MR->LockLastChanges));
}


/* Reports to UPnP network that current track does not support seek operations on position. */
void MrSetSeekTimePositionEnabled  (int seekEnabled)
{
	sem_wait(&(MR->LockLastChanges));

	if (
		((MR->SeekEnabled != 0) && (seekEnabled == 0)) ||
		((MR->SeekEnabled == 0) && (seekEnabled != 0))
		)
	{
		MR->SeekEnabled = seekEnabled;
		MR->AVTChangedFlags |= AVT_SEEK;
		UpdateAvtLastChangeModerated();
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network the new play state. */
void MRSetState(enum MR_Enum_States state)
{
	sem_wait(&(MR->LockLastChanges));

	#ifdef _DEBUG
	printf("MRSetState(%d)\r\n", state);
	#endif
	
	if (MR->Avt_State != state)
	{
		MR->Avt_State = state;
		MR->AVTChangedFlags |= AVT_STATE;
		UpdateAvtLastChangeModerated();
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network the new transport status. */
void MRSetStatus(enum MR_Enum_Status status)
{
	sem_wait(&(MR->LockLastChanges));

	if (MR->Avt_Status != status)
	{
		MR->Avt_Status = status;
		MR->AVTChangedFlags |= AVT_STATUS;
		UpdateAvtLastChangeModerated();
	}

	sem_post(&(MR->LockLastChanges));
}

/* Reports to UPnP network the new play mode. */
void MRSetPlayMode(enum MR_Enum_PlayModes playmode)
{
	int pm = (int) playmode;

	sem_wait(&(MR->LockLastChanges));

	if (MR->Avt_PlayMode != pm)
	{
		MR->Avt_PlayMode = pm;
		MR->AVTChangedFlags |= AVT_PLAYMODE;
		UpdateAvtLastChangeModerated();
	}

	sem_post(&(MR->LockLastChanges));
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the GetCurrentTransportActions request - responds with
 *	the appropriate string, depending on whether the current track
 *	supports seek operations.
 */
void UpnpMediaRenderer_AVTransport_GetCurrentTransportActions(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetCurrentTransportActions(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	/*
	 *	DEPRECATED:
	 *	Although this device may disable "seek on position" operations - it can't disable
	 *	the seek action because we can still seek on track targets.
	 *
	if (MR->SeekEnabled != 0)
	{
		UpnpResponse_MediaRenderer_AVTransport_GetCurrentTransportActions(upnptoken,RENDERER_TRANSPORT_ACTIONS_WITH_SEEK);
	}
	else
	{
		UpnpResponse_MediaRenderer_AVTransport_GetCurrentTransportActions(upnptoken,RENDERER_TRANSPORT_ACTIONS_NO_SEEK);
	}
	 */

	UpnpResponse_MediaRenderer_AVTransport_GetCurrentTransportActions(upnptoken,RENDERER_TRANSPORT_ACTIONS);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the Play action. Executes callback to report a play request.
 */
void UpnpMediaRenderer_AVTransport_Play(void* upnptoken,unsigned int InstanceID,char* Speed)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Play(%u,%s);\r\n",InstanceID,Speed));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}

	//TODO: Map 'Speed' to appropriate enum MR_Enum_PlaySpeeds value

	if (MROnStateChangeRequest != NULL) MROnStateChangeRequest(MR_State_Playing, MR_PlaySpeed_Normal);
	UpnpResponse_MediaRenderer_AVTransport_Play(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the GetDeviceCapabilities action.
 *	Returns statically defined data. 
 */
void UpnpMediaRenderer_AVTransport_GetDeviceCapabilities(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetDeviceCapabilities(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	UpnpResponse_MediaRenderer_AVTransport_GetDeviceCapabilities(upnptoken,RENDERER_CAPABILITY_PLAYMEDIA,"NOT_IMPLEMENTED","NOT_IMPLEMENTED");
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the GetMediaInfo action.
 *	Returns information on the current media.
 */
void UpnpMediaRenderer_AVTransport_GetMediaInfo(void* upnptoken,unsigned int InstanceID)
{
	char timeString[20];
	char *metadatastr;
	
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetMediaInfo(%u);\r\n",InstanceID));

	ASSERT(MR->Media_Creator);
	ASSERT(MR->Media_Title);

	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	
	SetTimeString(timeString, MR->Media_DurationTotal);

	/* Compute Metadata String */
	metadatastr = (char*)MMR_MALLOC(DIDL_ITEM_LEN + 3*(((int)strlen(MR->Media_Title)) + ((int)strlen(MR->Media_Creator))) + 1);
	sprintf(metadatastr, DIDL_ITEM, MR->Media_Title, MR->Media_Creator);

	UpnpResponse_MediaRenderer_AVTransport_GetMediaInfo(upnptoken,MR->Media_TrackTotal,timeString,MR->Media_URI,metadatastr,"","","NETWORK","NOT_IMPLEMENTED","UNKNOWN");

	MMR_FREE (metadatastr);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the Previous action.
 *	Executes callback to report the request.
 */
void UpnpMediaRenderer_AVTransport_Previous(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Previous(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	ASSERT(MROnNextPreviousRequest);
	MROnNextPreviousRequest(-1);
	UpnpResponse_MediaRenderer_AVTransport_Previous(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the Next action.
 *	Executes callback to report the request.
 */
void UpnpMediaRenderer_AVTransport_Next(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Next(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	ASSERT(MROnNextPreviousRequest);
	MROnNextPreviousRequest(1);
	UpnpResponse_MediaRenderer_AVTransport_Next(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the GetTransportSettings action.
 *	Returns the current playmode amongst other things.
 */
void UpnpMediaRenderer_AVTransport_GetTransportSettings(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetTransportSettings(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	UpnpResponse_MediaRenderer_AVTransport_GetTransportSettings(upnptoken,MR_PLAYMODES[MR->Avt_PlayMode],"NOT_IMPLEMENTED");
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the SetAVTransportURI action.
 *	Executes callback to report the URI change request.
 */
void UpnpMediaRenderer_AVTransport_SetAVTransportURI(void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_SetAVTransportURI(%u,%s,%s);\r\n",InstanceID,CurrentURI,CurrentURIMetaData));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	ASSERT(MROnMediaChangeRequest);

	/* prevent null exception errors with the URI */
	if (CurrentURI == NULL)
	{
		CurrentURI = MMR_EMPTY_STRING;
	}

	/* ensure that only certain media formats are employed */
	/* TODO: Add additional vendor-specified formats here. */
	if (
		(EndsWith(CurrentURI, EXTENSION_AUDIO_MPEG, 1) != 0)
		|| (EndsWith(CurrentURI, EXTENSION_AUDIO_WMA, 1) != 0)
		|| (EndsWith(CurrentURI, EXTENSION_VIDEO_WMV, 1) != 0)
		|| (EndsWith(CurrentURI, EXTENSION_VIDEO_MPEG, 1) != 0)
		|| (EndsWith(CurrentURI, EXTENSION_PLAYLIST_M3U, 1) != 0)
		)
	{
		MROnMediaChangeRequest(CurrentURI);
		UpnpResponse_MediaRenderer_AVTransport_SetAVTransportURI(upnptoken);
	}
	else
	{
		UpnpResponse_Error(upnptoken, MR_ERR_CODE_BAD_FILETYPE, MR_ERR_MSG_BAD_FILETYPE);
	}
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles the Pause action.
 *	Executes callback to report pause request.
 */
void UpnpMediaRenderer_AVTransport_Pause(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Pause(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	if (MROnStateChangeRequest != NULL) MROnStateChangeRequest(MR_State_Paused, MR_PlaySpeed_Ignore);
	UpnpResponse_MediaRenderer_AVTransport_Pause(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetPossitionInfo action.
 *	Gets some of the data about current track URI, 
 *	executes a callback to obtain information about the current playback position,
 *	and then sends response.
 */
void UpnpMediaRenderer_AVTransport_GetPositionInfo(void* upnptoken,unsigned int InstanceID)
{
	char durationstr[TIME_STRING_SIZE];
	char* metadatastr;
	char positionstr[TIME_STRING_SIZE];
	char absposstr[TIME_STRING_SIZE];
	int seconds, absSeconds, count, absCount;

	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetPositionInfo(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	
	ASSERT(MR->Track_Creator);
	ASSERT(MR->Track_Title);

	ASSERT(MROnGetPositionRequest != NULL);
	MROnGetPositionRequest(&seconds, &absSeconds, &count, &absCount);

	/* Compute Metadata String */
	metadatastr = (char*)MMR_MALLOC(DIDL_ITEM_LEN + 3*(((int)strlen(MR->Track_Title)) + ((int)strlen(MR->Track_Creator))) + 1);
	sprintf(metadatastr, DIDL_ITEM, MR->Track_Title, MR->Track_Creator);

	/* Compute time strings */
	SetTimeString(positionstr, seconds);
	SetTimeString(absposstr, absSeconds);
	SetTimeString(durationstr, MR->Track_Duration);

	UpnpResponse_MediaRenderer_AVTransport_GetPositionInfo
		(
		upnptoken,
		MR->Track_Number,
		durationstr,
		metadatastr,
		MR->Track_URI,
		positionstr,
		absposstr,
		count,
		absCount
		);

	MMR_FREE(metadatastr);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles Seek action.
 *	Determines the type of seek request and then executes a callback to handle the request.
 */
void UpnpMediaRenderer_AVTransport_Seek(void* upnptoken,unsigned int InstanceID,char* Unit,char* Target)
{
	int seekmode = 0, seekPosition = 0;
	int i;
	int len;
	int error = 0;
	char *msg = NULL;

	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Seek(%u,%s,%s);\r\n",InstanceID,Unit,Target));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	seekmode = FindStringInArray(Unit,MR_SEEKMODES,2);
	if (seekmode == -1) {UpnpResponse_Error(upnptoken,AVT_ERROR_SEEK_MODE_NOT_SUPPORTED,AVT_ERRORSTR_SEEK_MODE_NOT_SUPPORTED);return;}
	
	if (Target == NULL) { Target = MMR_EMPTY_STRING; }

	/*
	 *	Use this code for seek modes that take integer targets.
	 *	If an implementation has string-based seek targets, then
	 *	the vendor needs to declar a string-based version of OnSeekRequest().
	 */
	if ((seekmode == 0) || (seekmode == 1))
	{
		if (seekmode == 0)
		{
			/*
			 *	If the request is to seek on a position, but seeking on position
			 *	then respond with an error and exit method.
			 */
			if (MR->SeekEnabled == 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_TRANSITION_NOT_AVAILABLE,AVT_ERRORSTR_TRANSITION_NOT_AVAILABLE);return;}

			/* relative time */
			seekPosition = CalculateSeconds(Target);
			if (seekPosition < 0)
			{
				#define BAD_TIME_STRING "Illegal seek target: \"%s\" is not a valid ISO8601 hh:mms:ss formatted string."
				error = 711;
				msg = (char*) MMR_MALLOC (((int)strlen(BAD_TIME_STRING)) + ((int)strlen(Target)) + 1);
				sprintf(msg, BAD_TIME_STRING, Target);
			}
		}
		else if (seekmode == 1)
		{
			/* track index */
			len = (int) strlen(Target)-1;
			for (i=0; i < len; i++)
			{
				if ((Target[i] < '0') || (Target[i] > '9'))
				{
					#define BAD_INT_STRING "Illegal seek target: \"%s\" could not be converted to an integer."
					error = 711;
					msg = (char*) MMR_MALLOC(((int)strlen(BAD_INT_STRING)) + ((int)strlen(Target)) + 1);
					sprintf(msg, BAD_INT_STRING, Target);
					break;
				}
			}

			if (error == 0) { seekPosition = atoi(Target); }
		}
		
		if (error == 0)
		{
			ASSERT(MROnSeekRequest != NULL);
			MROnSeekRequest(seekmode, seekPosition);
		}
	}

	if (error == 0)
	{
        UpnpResponse_MediaRenderer_AVTransport_Seek(upnptoken);
	}
	else
	{
		UpnpResponse_Error(upnptoken, error, msg);
	}

	if (msg != NULL)
	{
		MMR_FREE (msg);
	}
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetTransportInfo action.
 *	Returns playstate information.
 */
void UpnpMediaRenderer_AVTransport_GetTransportInfo(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_GetTransportInfo(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	UpnpResponse_MediaRenderer_AVTransport_GetTransportInfo(upnptoken,MR_STATES[MR->Avt_State],"OK","1");
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles SetPlayMode action.
 *	Executes the callback with information about the play mode request.
 */
void UpnpMediaRenderer_AVTransport_SetPlayMode(void* upnptoken,unsigned int InstanceID,char* NewPlayMode)
{
	int mode;
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_SetPlayMode(%u,%s);\r\n",InstanceID,NewPlayMode));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	mode = FindStringInArray(NewPlayMode,MR_PLAYMODES,5);
	if (mode == -1) {UpnpResponse_Error(upnptoken,AVT_ERROR_PLAY_MODE_NOT_SUPPORTED,AVT_ERRORSTR_PLAY_MODE_NOT_SUPPORTED);return;}
	if (MROnPlayModeChangeRequest != NULL) MROnPlayModeChangeRequest(mode);
	UpnpResponse_MediaRenderer_AVTransport_SetPlayMode(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles Stop action.
 *	Executes the callback that requests a stop action to be performed.
 */
void UpnpMediaRenderer_AVTransport_Stop(void* upnptoken,unsigned int InstanceID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_AVTransport_Stop(%u);\r\n",InstanceID));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	if (MROnStateChangeRequest != NULL) MROnStateChangeRequest(MR_State_Stopped, MR_PlaySpeed_Ignore);
	UpnpResponse_MediaRenderer_AVTransport_Stop(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetCurrentConnectionInfo action.
 *	Simply responds with the information based on the current state.
 */
void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(%d);\r\n",ConnectionID));
	if (ConnectionID != 0) {UpnpResponse_Error(upnptoken,CM_ERROR_INVALID_CONNECTION_REFERENCE,CM_ERRORSTR_INVALID_CONNECTION_REFERENCE);return;}
	
	/* TODO: report proper protocolInfo based on the current media/track information */
	UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(upnptoken,0,0,MR_PROTINFO[MR->Cm_ProtInfo],"/",-1,"Input","Unknown");
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetProtocolInfo action.
 *	Responds with the static information about the supported protocolInfo for this media renderer.
 */
void UpnpMediaRenderer_ConnectionManager_GetProtocolInfo(void* upnptoken)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetProtocolInfo();\r\n"));
	UpnpResponse_MediaRenderer_ConnectionManager_GetProtocolInfo(upnptoken,RENDERER_SOURCEPROTOCOLINFO,RENDERER_SINKPROTOCOLINFO);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetCurrentConnectionIDs action.
 *	Responds with the static information, indicating that there's always
 *	a single connection with ID=0.
 */
void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs();\r\n"));
	UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"0");
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles SetVolume action.
 *	Executes a callback for request a volume change, with information
 *	on the volume channel and the desired volume level.
 */
void UpnpMediaRenderer_RenderingControl_SetVolume(void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume)
{
	int channel;
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_RenderingControl_SetVolume(%u,%s,%u);\r\n",InstanceID,Channel,DesiredVolume));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	channel = FindStringInArray(Channel,MR_AUDIOCHANNELS,3);
	if (channel == -1) {UpnpResponse_Error(upnptoken,RCS_ERROR_INVALID_CHANNEL,RCS_ERRORSTR_INVALID_CHANNEL);return;}
	if (MROnVolumeChangeRequest != NULL) MROnVolumeChangeRequest(channel,DesiredVolume);
	UpnpResponse_MediaRenderer_RenderingControl_SetVolume(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetMute action.
 *	Responds to control point based on upper-layer calls to MRSetMute.
 */
void UpnpMediaRenderer_RenderingControl_GetMute(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	int channel;
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_RenderingControl_GetMute(%u,%s);\r\n",InstanceID,Channel));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	channel = FindStringInArray(Channel,MR_AUDIOCHANNELS,3);
	if (channel == -1) {UpnpResponse_Error(upnptoken,RCS_ERROR_INVALID_CHANNEL,RCS_ERRORSTR_INVALID_CHANNEL);return;}
	UpnpResponse_MediaRenderer_RenderingControl_GetMute(upnptoken,MR->Rcs_Mute[channel]);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles SetMute action.
 *	Executes a callback for request a mute change, with information
 *	on the volume channel and the boolean value.
 */
void UpnpMediaRenderer_RenderingControl_SetMute(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute)
{
	int channel;
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_RenderingControl_SetMute(%u,%s,%d);\r\n",InstanceID,Channel,DesiredMute));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	channel = FindStringInArray(Channel,MR_AUDIOCHANNELS,3);
	if (channel == -1) {UpnpResponse_Error(upnptoken,RCS_ERROR_INVALID_CHANNEL,RCS_ERRORSTR_INVALID_CHANNEL);return;}
	if (MROnVolumeChangeRequest != NULL) MROnMuteChangeRequest(channel,DesiredMute);
	UpnpResponse_MediaRenderer_RenderingControl_SetMute(upnptoken);
}

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles GetVolume action.
 *	Responds to control point based on upper-layer calls to MRSetVolume.
 */
void UpnpMediaRenderer_RenderingControl_GetVolume(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	int channel;
	VERBOSEONLY(printf("Invoke: UpnpMediaRenderer_RenderingControl_GetVolume(%u,%s);\r\n",InstanceID,Channel));
	if (InstanceID != 0) {UpnpResponse_Error(upnptoken,AVT_ERROR_INVALID_INSTANCE_ID,AVT_ERRORSTR_INVALID_INSTANCE_ID);return;}
	channel = FindStringInArray(Channel,MR_AUDIOCHANNELS,3);
	if (channel == -1) {UpnpResponse_Error(upnptoken,RCS_ERROR_INVALID_CHANNEL,RCS_ERRORSTR_INVALID_CHANNEL);return;}
	UpnpResponse_MediaRenderer_RenderingControl_GetVolume(upnptoken,MR->Rcs_Volume[channel]);
}

/*
 *	ifdef allows this method to be excluded for compilation - specifically
 *	useful for MicroDMA solution, which includes both the RemoteIOClient
 *	and a MediaREnderer. Both projects have a UpnpPresentationRequest
 *	method, but the MicroDMA solution should have its own.
 */
#ifndef EXCLUDE_MMR_PRESENTATION_REQUEST

/*
 *	Sink for UpnpMicrostack callback.
 *	Handles an web request made on the UPnP devices webserver.
 *	Currently responds with HTTP-200 OK for any request.
 */
void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	VERBOSEONLY(printf("Mmr Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj));
	VERBOSEONLY(printf("HOST: %x\r\n",UpnpGetLocalInterfaceToHost(upnptoken)));
	
	/* TODO: Add Web Response Code Here... */
	
	//UpnpPresentationResponse(upnptoken, "HTTP/1.0 200 OK\r\n\r\n" , 19 , 1);
}
#endif

/*
 *	This method is called by the thread-chaining infrastructure
 *	when the MRDataObject needs to be destroyed because the
 *	thread-chain is stopping.
 */
void DestroyChain(void* object)
{
	struct MRDataObject* Renderer = (struct MRDataObject*)object;
	MMR_FREE(Renderer->Media_URI);
	MMR_FREE(Renderer->Media_Title);
	MMR_FREE(Renderer->Media_Creator);
	MMR_FREE(Renderer->Track_URI);
	MMR_FREE(Renderer->Track_Creator);
	MMR_FREE(Renderer->Track_Title);

	sem_destroy(&(Renderer->LockLastChanges));
	
	/* Framework will FREE the data at 'object' */
	/*MMR_FREE(Renderer);*/
}

/*
 *	Call this method to instantiate a MicroMediaRenderer object, which
 *	provides the really really basic interface for a UPnP MediaRender
 *	device.
 */
void* CreateMediaRenderer(void* Chain, void* Stack, void* lifetimeMonitor)
{
	/*
	 *	Instantiate the object to keep state of the MicroMediaRenderer.
	 */
	MR = (struct MRDataObject*)MMR_MALLOC(sizeof(struct MRDataObject));
	memset(MR,0,sizeof(struct MRDataObject));

	/*
	 *	Initialize all fields to their appropriately default values.
	 */
	MR->Media_URI = (char*)MMR_MALLOC(1);		MR->Media_URI[0] = NULL_CHAR;
	MR->Media_Title = (char*) MMR_MALLOC(1);	MR->Media_Title[0] = NULL_CHAR;
	MR->Media_Creator = (char*) MMR_MALLOC(1);	MR->Media_Creator[0] = NULL_CHAR;
	MR->Track_URI = (char*)MMR_MALLOC(1);		MR->Track_URI[0] = NULL_CHAR;
	MR->Track_Creator = (char*) MMR_MALLOC(1);	MR->Track_Creator[0] = NULL_CHAR;
	MR->Track_Title = (char*) MMR_MALLOC(1);	MR->Track_Title[0] = NULL_CHAR;
	MR->Destroy = &DestroyChain;
	MR->SeekEnabled = 1;
	sem_init(&(MR->LockLastChanges), 0, 1);

	/*
	 *	Assign caller provided arguments to MediaRenderer object 
	 */
	MR->LifeTimeMonitor = lifetimeMonitor;
	MR->UpnpMicroStack = Stack;

	/* add it to the chain */
	ILibAddToChain(Chain, MR);

	/*
	 *	Set initial state variables.
	 */
	UpnpSetState_MediaRenderer_RenderingControl_LastChange(MR->UpnpMicroStack,"<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/RCS/\"/>");
	UpnpSetState_MediaRenderer_AVTransport_LastChange(MR->UpnpMicroStack,"<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"/>");
	UpnpSetState_MediaRenderer_ConnectionManager_SourceProtocolInfo(MR->UpnpMicroStack,RENDERER_SOURCEPROTOCOLINFO);
	UpnpSetState_MediaRenderer_ConnectionManager_SinkProtocolInfo(MR->UpnpMicroStack,RENDERER_SINKPROTOCOLINFO);
	UpnpSetState_MediaRenderer_ConnectionManager_CurrentConnectionIDs(MR->UpnpMicroStack,"0");

	/*
	 *	Update last change variables to match current state.
	 */
	UpdateAvtLastChange();
	UpdateRcsLastChange();

	return MR;
}
