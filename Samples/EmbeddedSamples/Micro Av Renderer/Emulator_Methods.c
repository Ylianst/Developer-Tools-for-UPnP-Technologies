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
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <assert.h>
#include <string.h>

#include "ILibParsers.h"
#include "MicroMediaRenderer.h"
#include "RendererStateLogic.h"
#include "Emulator_Methods.h"
#include "HttpPlaylistParser.h"
#include "MyString.h"

/* [CODEC]: BEGIN */
#include "CodecWrapper.h"
/* [CODEC]: END */

#ifdef _DEBUG
	#define DEBUGONLY(x) x

	#define EMM_MALLOC	emm_malloc
	#define EMM_FREE	emm_free

	int emm_malloc_counter = 0;
	void* emm_malloc (int size)
	{
		++emm_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("emm_malloc_counter=%d\r\n", emm_malloc_counter);
		#endif
		return malloc(size);
	}

	void emm_free (void *ptr)
	{
		--emm_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("emm_malloc_counter=%d\r\n", emm_malloc_counter);
		#endif
		free(ptr);
	}
#endif

#ifndef _DEBUG
	#define DEBUGONLY(x) 

	#define EMM_MALLOC	malloc
	#define EMM_FREE	free
#endif

#ifdef _TEMPDEBUG
	#define TEMPDEBUGONLY(x) x
#endif

#ifndef _TEMPDEBUG
	#define TEMPDEBUGONLY(x)
#endif



void* MR_RendererChain;
void* MR_UpnpStack;
void* MR_MediaRenderer;
void* MR_RendererLogic;
void* MR_ExtendedM3uProcessor;

/* function declarations */
void OnCodecWrapper_DurationUpdate (int duration, void* codecSpecific, void* streamTag);
void OnCodecWrapper_StateChange (enum CodecWrapperState newState, void* codecSpecific, void* streamTag);
void OnResult_FindTargetUri (void *parserObject, int wrapAround, const char* playlistUri, int itemIndex, void *userObject, /*OUT*/ int actualIndex, /*OUT - MUST COPY THIS*/ const char *targetUri, /*OUT*/int duration, /*OUT - MUST COPY THIS*/ const char* comment);
void OnResult_UpdateItemCount (void *parserObject, const char* playlistUri, void* userObject, /*OUT*/int maxIndexNotKnown, /*OUT*/int itemCount);
void OnResult_PlaylistUriExists (void* parserObject, const char* playlistUri, void* userObject, int uriExists);
enum MR_SupportedProtocolInfo UriToProtocolInfo(const char* uri);

/*
 *	Given a URI, determine it's protocolInfo.
 */
enum MR_SupportedProtocolInfo UriToProtocolInfo(const char* targetUri)
{
	enum MR_SupportedProtocolInfo protInfo;

	/*
	 *	Determine protocolInfo based on file extension.
	 *
	 *	TODO: need more robust means of determining protocolInfo.
	 */

	if (EndsWith(targetUri, ".mp3", 1) != 0)
	{
		protInfo = MR_PROTINFO_HTTP_AUDIO_MPEG;
	}
	else if (EndsWith(targetUri, ".wma", 1) != 0)
	{
		protInfo = MR_PROTINFO_HTTP_AUDIO_WMA;
	}
	else if (EndsWith(targetUri, ".mpeg", 1) != 0)
	{
		protInfo = MR_PROTINFO_HTTP_VIDEO_MPEG;
	}
	else if (EndsWith(targetUri, ".wmv", 1) != 0)
	{
		protInfo = MR_PROTINFO_HTTP_VIDEO_WMV;
	}
	else
	{
		/* just assume something http. */
		protInfo = MR_PROTINFO_HTTP_UNKNOWN;
	}

	return protInfo;
}

/*
 *	This method is a callback sink for the CodecWrapper when it determines
 *	whether the track supports seek operations on time positions. The
 *	method instructs the RendererStateLogic about the condition.
 */
void OnCodecWrapper_CanSetPosition (int canSetPosition, void *codecSpecific, void *streamTag)
{
	RSL_CanSetPosition(streamTag, canSetPosition);
}

/*
 *	This method is a callback sink for the CodecWrapper when the duration for the current
 *	stream gets updated. The method propogates the information to the RendererStateLogic,
 *	so that it can reflect the duration information onto the UPnP network. This callback
 *	is provided because some rendering frameworks need an asynchronous means to report
 *	the duration.
 */
void OnCodecWrapper_DurationUpdate (int duration, void* codecSpecific, void* streamTag)
{
	RSL_SetTrackDuration(streamTag, duration);
}

/*
 *	This method is a callback sink for the CodecWrapper when the state changes for the current stream.
 *	The method propogates the execution to the RendererStateLogic, so that it can properly
 *	determine what the renderer state machine should do in response to the event. Sometimes,
 *	it simply reflects the state information onto the UPnP network.
 */
void OnCodecWrapper_StateChange (enum CodecWrapperState newState, void* codecSpecific, void* streamTag)
{
	DEBUGONLY(printf("OnCodecWrapper_StateChange (%d, %p, %p)\r\n", newState, codecSpecific, streamTag);)

	switch (newState)
	{
	case MEDIA_UNINITIALIZED:
	case MEDIA_STOPPED:
		RSL_OnCodecEvent_Stopped(streamTag);
		break;

	case MEDIA_ENDED:
		RSL_OnCodecEvent_Ended(streamTag);
		break;

	case MEDIA_ERROR:
		RSL_OnCodecEvent_Error(streamTag);
		break;

	case MEDIA_TRANSIT:
		RSL_OnCodecEvent_Transit(streamTag);
		break;

	case MEDIA_PLAYING:
		RSL_OnCodecEvent_Playing(streamTag);
		break;

	case MEDIA_PAUSED:
		RSL_OnCodecEvent_Paused(streamTag);
		break;

	default:
		printf("Unexpected state change reported by codec (%d)\r\n", (int)newState);
		RSL_OnCodecEvent_Error(streamTag);
		break;
	}
}

/*
 *	This callback sink executes when the HttpPlaylistParser finds the target track information
 *	given a playlist URI and a target index. The callback also includes the duration, which
 *	may be negative if the playlist does not provide duration metadata.
 */
void OnResult_FindTargetUri (void *parserObject, int wrapAround, const char* playlistUri, int itemIndex, void *userObject, /*OUT*/ int actualIndex, /*OUT - MUST COPY THIS*/ const char *targetUri, /*OUT*/int duration, /*OUT - MUST COPY THIS*/ const char* comment)
{
	int i;
	int commentLen;
	char *data;
	int dashPos;
	int cStart, tStart;
	int applied;
	int codecDuration;
	int m3uDuration;
	enum MR_SupportedProtocolInfo protInfo;

	DEBUGONLY(printf("OnResult_FindTargetUri(%p, %d, %s, %d, %p, %d, %s, %d, %s\r\n", parserObject, wrapAround, playlistUri, itemIndex, userObject, actualIndex, targetUri, duration, comment);)

	applied = RSL_OnPlaylistLogicResult_FoundTargetUri
		(
		userObject,
		playlistUri,
		itemIndex,
		wrapAround,
		actualIndex,
		targetUri
		);

	if (applied != 0)
	{
		m3uDuration = duration;

		/*
		 *	Given the duration time in seconds, get a string that represents that duration
		 *	in hh:mm:ss.
		 */
		codecDuration = CodecWrapper_GetTimeTotalLength(STREAM_INDEX);

		/*
		 *	Compare the duration from the playlist metadata with the duration
		 *	from the rendering framework. Take the larger of the two as
		 *	the more accurate.
		 */
		if (m3uDuration > codecDuration)
		{
			duration = m3uDuration;
		}
		else
		{
			duration = codecDuration;
		}

		/* report chosen duration to the UPnP network */
		RSL_SetTrackDuration(userObject, duration);


		/*
		 *	Parse the 'comment' field/metadata for information.
		 *	For digital home, the format of the comment field
		 *	is '[creator] - [title]'.
		 */

		dashPos = -1;
		commentLen = (int)strlen(comment);

		data = (char*) EMM_MALLOC(commentLen+1);
		memcpy(data, comment, commentLen+1);

		for (i=0; i < commentLen; i++)
		{
			if (comment[i] == '-')
			{
				dashPos = i;
				break;
			}
		}

		if (dashPos < 0)
		{
			/* entire comment is title */
			cStart = commentLen;
			tStart = 0;
		}
		else
		{
			/* comment is [creator]-[title] */
			cStart = 0;
			tStart = dashPos+1;
			data[dashPos] = '\0';
		}

		/* get protocolInfo for this uri*/
		protInfo = UriToProtocolInfo(targetUri);

		/*
		 *	Report creator/title metadata information to UPnP network.
		 */
		RSL_SetTrackMetadata(userObject, protInfo, data+tStart, data+cStart);
		EMM_FREE(data);
	}
}

/*
 *	This callback sink executes when the HttpPlaylistParser has an updated
 *	count on the number of items for the playlist.
 */
void OnResult_UpdateItemCount (void *parserObject, const char* playlistUri, void* userObject, /*OUT*/int maxIndexNotKnown, /*OUT*/int itemCount)
{
	DEBUGONLY(printf("OnResult_UpdateItemCount(%p, %s, %p, %d, %d)\r\n", parserObject, playlistUri, userObject, maxIndexNotKnown, itemCount);)
	RSL_OnPlaylistLogicResult_SetTrackTotal(userObject, playlistUri, itemCount, MR_DURATION_UNKNOWN);
}

/*
 *	This callback sink executes when the HttpPlaylistParser determines that a file
 *	does or does not exist at the playlistUri address.
 */
void OnResult_PlaylistUriExists (void* parserObject, const char* playlistUri, void* userObject, int uriExists)
{
	enum MR_SupportedProtocolInfo protInfo;

	DEBUGONLY(printf("OnResult_PlaylistUriExists(%p, %s, %p, %d)\r\n", parserObject, playlistUri, userObject, uriExists); )

	/* get protocolInfo for the playlist uri*/
	protInfo = UriToProtocolInfo(playlistUri);

	if (uriExists != 0)
	{
		RSL_OnPlaylistLogicResult_MediaUriExists(userObject, protInfo, playlistUri, "", "", uriExists);
	}
	else
	{
		RSL_OnPlaylistLogicResult_MediaUriExists(userObject, MR_PROTINFO_HTTP_UNKNOWN, playlistUri, INVALID_MEDIA_URI_ERROR, "", uriExists);
	}
}

/*	see Emulator_Methods.h */
void InstructPlaylistLogic_FindTargetUri(void *rslObj, const char *mediaUri, int targetIndex, int wrapAround)
{
	enum MR_SupportedProtocolInfo protInfo;

	DEBUGONLY(printf("InstructPlaylistLogic_FindTargetUri(%s, %d, %d)\r\n", mediaUri, targetIndex, wrapAround);)

	/*
	 *	If the uri ends with M3U, then process it as an
	 *	M3U playlist by finding the first URI
	 *	in the playlist.
	 */

	if (EndsWith(mediaUri, ".M3U", 1) != 0)
	{
		HttpPlaylistParser_FindTargetUri
			(
			MR_ExtendedM3uProcessor,
			wrapAround,
			mediaUri,
			targetIndex,
			rslObj,
			(OnResult_PlaylistUriExists), 
			(OnResult_UpdateItemCount), 
			(OnResult_FindTargetUri)
			);
	}
	else if (
		(EndsWith(mediaUri, ".MP3", 1) != 0) || 
		(EndsWith(mediaUri, ".WMA", 1) != 0)
		)
	{
		/*
		 *	If the URI indicates it is a single mp3 or wma file,
		 *	skip playlist stuff and kick off process for
		 *	streaming the content.
		 *
		 *	Also fire callbacks that normally execute when the
		 *	playlist parser processes a playlist.
		 */

		InstructCodec_SetupStream(rslObj, mediaUri);

		/* get protocolInfo for this uri */
		protInfo = UriToProtocolInfo(mediaUri);

		RSL_OnPlaylistLogicResult_MediaUriExists(rslObj, protInfo, mediaUri, "Same as track title", "Same as track artist", 1);
		RSL_OnPlaylistLogicResult_SetTrackTotal(rslObj, mediaUri, 1, MR_DURATION_UNKNOWN);
		RSL_OnPlaylistLogicResult_FoundTargetUri(rslObj, mediaUri, targetIndex, wrapAround, 1, mediaUri);

		/*
		 *	Would be nice to be able to parse metadata from file.
		 */
		RSL_SetTrackMetadata(rslObj, protInfo, "Unknown Track Title", "Unknown Artist");
	}
	else
	{
		// report that media is invalid
		OnResult_PlaylistUriExists(NULL, mediaUri, rslObj, 0);
	}
}

/*	see Emulator_Methods.h */
void InstructCodec_SetupStream(void *rslObj, const char *trackUri)
{
	DEBUGONLY(printf("InstructCodec_SetupStream(%s)\r\n", trackUri);)
	CodecWrapper_SetupStream(STREAM_INDEX, trackUri, rslObj, OnCodecWrapper_StateChange, OnCodecWrapper_DurationUpdate, OnCodecWrapper_CanSetPosition);
}

/*	see Emulator_Methods.h */
void InstructCodec_Play(void *rslObj, enum MR_Enum_PlaySpeeds playSpeed)
{
	DEBUGONLY(printf("InstructCodec_Play\r\n"););

	//TODO: enable trick modes by mapping playSpeed to appropriate play speed in enum CodecWrapperPlaySpeed.
	CodecWrapper_Play(STREAM_INDEX, CW_PlaySpeed_Normal);
}

/*	see Emulator_Methods.h */
void InstructCodec_Stop(void *rslObj)
{
	DEBUGONLY(printf("InstructCodec_Stop\r\n"););
	CodecWrapper_Stop(STREAM_INDEX);
}

/*	see Emulator_Methods.h */
void InstructCodec_Pause(void *rslObj)
{
	DEBUGONLY(printf("InstructCodec_Pause\r\n"););
	CodecWrapper_Pause(STREAM_INDEX);
}

/*	see Emulator_Methods.h */
int QueryCodec_IsBusy(void *rslObj)
{
	enum CodecWrapperState state;

	DEBUGONLY(printf("QueryCodec_IsBusy\r\n"););
	
	state = CodecWrapper_GetPlayState(STREAM_INDEX);

	DEBUGONLY(printf("QueryCodec_IsBusy (cont'd): retVal=%d state=%d\r\n", (state == MEDIA_TRANSIT), state););
	return (state == MEDIA_TRANSIT);
}

/*	see Emulator_Methods.h */
int Validate_MediaUri(void *rslObj, const char *mediaUri)
{
	DEBUGONLY(printf("Validate_MediaUri\r\n"););

	//TODO: Validate the media URI - possibly do things
	//like checking for routability and proper content.

	return 1;
}

/*	see Emulator_Methods.h */
void MROnVolumeChangeRequestSink(enum MR_Enum_AudioChannels Channel,unsigned short Value)
{
	DEBUGONLY(printf("MROnVolumeChangeRequestSink: Channel = %d, DataLength = %d\r\n",Channel,Value);)
	
	/*
	 *	Instruct rendering framework to change volume, then
	 *	report the volume change on the UPNP network.
	 */
	CodecWrapper_SetVolume(STREAM_INDEX, Value);
	
	/*
	 *	DEPRECATED:
	 *	Call MrSetVolume instead.
	 *
	RSL_SetVolume(MR_RendererLogic, Channel,Value);
	 */
	MRSetVolume(Channel, Value);
}

/*	see Emulator_Methods.h */
void MROnMuteChangeRequestSink(enum MR_Enum_AudioChannels Channel,int Value)
{
	DEBUGONLY(printf("MROnMuteChangeRequestSink: Channel = %d, DataLength = %d\r\n",Channel,Value);)

	/*
	 *	Instruct rendering framework to change mute, then
	 *	report the mute change on the UPNP network.
	 */
	CodecWrapper_SetMute(STREAM_INDEX, Value);
	
	/*
	 *	DEPRECATED:
	 *	Call MRSetMute instead.
	 *
	RSL_SetMute(MR_RendererLogic, Channel,Value);
	 */
	MRSetMute(Channel, Value);
}

/*	see Emulator_Methods.h */
void MROnMediaChangeRequestSink(const char* MediaUri)
{
	DEBUGONLY(printf("MROnMediaChangeRequestSink: URI = %s\r\n",MediaUri);)

	/*
	 *	Instruct RendererStateLogic to change the URI to the specified
	 *	URI. MediaURI can point ot a playlist URI or an individual track.
	 */
	RSL_SetMediaUri(MR_RendererLogic, MediaUri);
}

/*	see Emulator_Methods.h */
void MROnGetPositionRequestSink(int* seconds, int* absSeconds, int* count, int* absCount)
{
	TEMPDEBUGONLY(printf("MROnGetPositionRequestSink\r\n");)

	/* playback position in seconds */
	*seconds = CodecWrapper_GetTimePosition(STREAM_INDEX);

	/* playback position within entire media/playlist in seconds */
	*absSeconds = MR_DURATION_UNKNOWN;

	/*
	 *	Playback counter position - we use bytes, 
	 *	but others may use something else like a video image frame.
	 */
	*count = CodecWrapper_GetBytePosition(STREAM_INDEX);

	/* Playback counter position - relative to within the entire playlist/media. */
	*absCount = MR_COUNTER_UNKNOWN;
}

/*	see Emulator_Methods.h */
void MROnSeekRequestSink(enum MR_Enum_SeekModes seekMode, int seekPosition)
{
	DEBUGONLY(printf("MROnSeekRequestSink: SeekMode = %d, SeekPosition = %d\r\n",seekMode,seekPosition);)

	if (seekMode == 0)
	{
		/* seek operation within a track */
		CodecWrapper_SetPosition(STREAM_INDEX, seekPosition);
	}
	else
	{
		/* seek operation for a specific track index */
		RSL_DoSeekTrack(MR_RendererLogic, seekPosition);
	}
}

/*	see Emulator_Methods.h */
void MROnNextPreviousRequestSink(int trackDelta)
{
	DEBUGONLY(printf("MROnNextPreviousRequest: DeltaTrack = %d\r\n",trackDelta);)
	RSL_DoNextPrevious(MR_RendererLogic, trackDelta);
}

/*	see Emulator_Methods.h */
void MROnStateChangeRequestSink(enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeed)
{
	DEBUGONLY(printf("MROnStateChangeRequest: State = %d\r\n",state);)
	RSL_DoStateChange(MR_RendererLogic, state, playSpeed);
}

/*	see Emulator_Methods.h */
void MROnPlayModeChangeRequestSink(enum MR_Enum_PlayModes playmode)
{
	DEBUGONLY(printf("MROnPlayModeChangeRequest: PlayMode = %d\r\n",playmode);)
	RSL_SetPlayMode(MR_RendererLogic,playmode);
}

