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

#include "stdafx.h"

#ifdef _WIN32_WCE
	#include "PlayerOCX.h"
#elif WIN32
	#include <errno.h>
	#include "mediaplayer1.h"
#endif

extern "C"
{
	#include "MediaPlayerVersions_Methods.h"
	#include "ILibParsers.h"
	#include "HttpPlaylistParser.h"
	#include "RendererStateLogic.h"
	#include "MyString.h"
}

#ifdef _DEBUG
	#define DEBUGONLY(x) x

	#define MPV_MALLOC	mpv_malloc
	#define MPV_FREE	mpv_free

	int mpv_malloc_counter = 0;
	void* mpv_malloc (int size)
	{
		++mpv_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("mpv_malloc_counter=%d\r\n", mpv_malloc_counter);
		#endif
		return malloc(size);
	}

	void mpv_free (void *ptr)
	{
		--mpv_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("mpv_malloc_counter=%d\r\n", mpv_malloc_counter);
		#endif
		free(ptr);
	}
#endif

#ifndef _DEBUG
	#define DEBUGONLY(x) 

	#define MPV_MALLOC	malloc
	#define MPV_FREE	free
#endif


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

	if (EndsWith(targetUri, ".m3u", 1) != 0)
	{
		protInfo = MR_PROTINFO_HTTP_AUDIO_M3U;
	}
	else if (EndsWith(targetUri, ".mp3", 1) != 0)
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


/******************************************************************************************
 *	BEGIN: App variables
 *			Variables needed so app can coordinate activities of 
 *			MediaPlayer, RendererLogic modules, and UPnP-related modules.
 ******************************************************************************************/

enum MR_Enum_States SC_LastCommand = MR_State_NoMedia;
int SC_PendingSetFileName = 0;								
int   SC_VolumeMaster = 0;
int   SC_VolumeLeft = 0;
int   SC_VolumeRight = 0;
void* The_RendererChain = NULL;
void* The_UpnpStack = NULL;
void* The_MediaRenderer = NULL;
void* The_RendererLogic = NULL;
void* The_PlaylistProcessor = NULL;
HWND MainWindow = NULL;

#ifdef _WIN32_WCE
	CComPtr<IWMP>	*The_MediaPlayer;
	#define TheMediaPlayer (*The_MediaPlayer)
	int SC_ExpectingEndOfTrack = 0;
	int SC_IgnoreLastCommand = 0;
#elif WIN32
	CMediaplayer1	*The_MediaPlayer = NULL;
	#define TheMediaPlayer (The_MediaPlayer)
#endif

/******************************************************************************************
 *	END: App variables
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: MmrMicroStack callback sinks for request
 *
 *	Wire the UPnP stack's callbacks for UPnP action requests to these methods.
 ******************************************************************************************/

void SinkQuery_CurrentPosition(int* seconds, int* absSeconds, int* count, int* absCount)
{
	double val;

	if (TheMediaPlayer != NULL)
	{
		#ifdef _WIN32_WCE
				TheMediaPlayer->get_CurrentPosition(&val);
				*seconds = (int) val;
		#elif WIN32
				*seconds = (int)TheMediaPlayer->get_CurrentPosition();
		#endif

		*absSeconds = MR_DURATION_UNKNOWN;
		*count = MR_COUNTER_UNKNOWN;
		*absCount = MR_COUNTER_UNKNOWN;
	}
	else
	{
		*seconds = MR_DURATION_UNKNOWN;
		*absSeconds = MR_DURATION_UNKNOWN;
		*count = MR_COUNTER_UNKNOWN;
		*absCount = MR_COUNTER_UNKNOWN;
	}

	/*
	 *	Sometimes the MediaPlayer control
	 *	updates its duration after the initial
	 *	time we set the duration, so update
	 *	it now.
	 */
	#ifdef _WIN32_WCE
		TheMediaPlayer->get_Duration(&val);
	#elif WIN32
		val = TheMediaPlayer->get_Duration();
	#endif
	RSL_SetTrackDuration(The_RendererLogic, (int) val);
}

void SinkRequest_MediaUriChange(const char* url)
{
	char *uri;
	uri = (char*) MPV_MALLOC((int)strlen(url)+1);
	strcpy(uri, url);
	PostMessage(MainWindow, WM_MPINVOKE, RLINV_INV_SET_MEDIA_URI, (LPARAM)uri);
}

void SinkRequest_MuteChange(enum MR_Enum_AudioChannels Channel,int Mute)
{
	PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_MUTE, Mute);
}

void SinkRequest_NextPrevious(int trackdelta)
{
	PostMessage(MainWindow, WM_MPINVOKE, RLINV_NEXT_PREVIOUS, trackdelta);
}

void SinkRequest_PlayModeChange(MR_Enum_PlayModes playmode)
{
	PostMessage(MainWindow, WM_MPINVOKE, RLINV_SET_PLAYMODE, playmode);
}

void SinkRequest_Seek(enum MR_Enum_SeekModes seekMode, int seekPosition)
{
	switch (seekMode)
	{
	case MR_SeekMode_RelTime:
		PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_CURRENT_POSITION, seekPosition);
		break;

	case MR_SeekMode_TrackNr:
		PostMessage(MainWindow, WM_MPINVOKE, RLINV_SET_TRACK_INDEX, seekPosition);
		break;
	}
}

void SinkRequest_StateChange(enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeed)
{
	// play speed is ignored because it's not supported in Media Player
	PostMessage(MainWindow, WM_MPINVOKE, RLINV_SET_STATE, state);
}

void SinkRequest_VolumeChange(enum MR_Enum_AudioChannels Channel,unsigned short Volume)
{
	PostMessage(MainWindow, WM_MPINVOKE, Channel+MPINV_SET_VOLUME_MASTER, Volume);
}

/******************************************************************************************
 *	END: MmrMicroStack callback sinks for request
 ******************************************************************************************/



/******************************************************************************************
 *	BEGIN: Media Player related control handlers and callback sinks
 *
 *	These methods provide the actual implementations for 
 *	handling the Windows Media Player interactions.
 ******************************************************************************************/

void HandleMediaPlayer_SetState(enum MR_Enum_States desiredState)
{
	switch (desiredState)
	{
	case MR_State_Playing:
		TheMediaPlayer->Play();
		break;

	case MR_State_Stopped:
		TheMediaPlayer->Stop();
		break;

	case MR_State_Paused:
		TheMediaPlayer->Pause();
		break;
	}
}

void HandleMediaPlayer_SetVolume()
{
	long balance, volume;
	double b,v;
	b = (((double)SC_VolumeLeft - (double)SC_VolumeRight) / 10);
	balance = (long)(b*b*b*b);
	if (SC_VolumeLeft > SC_VolumeRight) balance = balance * (-1);
	TheMediaPlayer->put_Balance(balance);

	v = ((double)max(SC_VolumeLeft,SC_VolumeRight)*(double)SC_VolumeMaster)/100;
	v = (double)((100 - ((((double)v) / 2.2) + 20)) / 10);
	volume = (long)(v*v*v*v*(-1));
	TheMediaPlayer->put_Volume(volume);
}

void HandleMediaPlayer_OnError()
{
	#ifdef _WIN32_WCE
		VARIANT_BOOL hasError;
		TheMediaPlayer->get_HasError(&hasError);

		if (hasError)
		{
			RSL_OnCodecEvent_Error(The_RendererLogic);
			//TODO: show msg box
		}

	#elif WIN32
		char errorMsg[1024];
		char errorCode[100];
		BOOL hasError;
		hasError = TheMediaPlayer->get_HasError();
		
		if (hasError)
		{
			RSL_OnCodecEvent_Error(The_RendererLogic);

			sprintf(errorCode, "%d", TheMediaPlayer->get_ErrorCode());
			sprintf(errorMsg, "%s", TheMediaPlayer->get_ErrorDescription());

			MessageBox(MainWindow, errorMsg, errorCode, MB_OK);
		}
	#endif
}

void HandleMediaPlayer_OnStateChange(long newState)
{
	double cp, dur;

	switch (newState)
	{
	case 8:		//media ended
	case 0:		//stopped
	case 10:	//ready
			#ifdef _WIN32_WCE
				TheMediaPlayer->get_CurrentPosition(&cp);
				TheMediaPlayer->get_Duration(&dur);
			#elif WIN32
				cp = TheMediaPlayer->get_CurrentPosition();
				dur = TheMediaPlayer->get_Duration();
			#endif

			if (
				((cp >= dur) && (cp > 0))
				)
			{
				RSL_OnCodecEvent_Ended(The_RendererLogic);
			}
			#ifdef _WIN32_WCE
			else if ((SC_LastCommand == MR_State_Playing) && (SC_ExpectingEndOfTrack != 0) && (SC_IgnoreLastCommand == 0))
			{
				SC_ExpectingEndOfTrack = 0;
				RSL_OnCodecEvent_Ended(The_RendererLogic);
			}
			#endif
			else
			{
				RSL_OnCodecEvent_Stopped(The_RendererLogic);
			}

			if (SC_PendingSetFileName == 0)
			{
				TheMediaPlayer->put_CurrentPosition(0);
			}
		break;

	case 2:
		#ifdef _WIN32_WCE
		SC_ExpectingEndOfTrack = 1;
		SC_IgnoreLastCommand = 0;
		#endif
		RSL_OnCodecEvent_Playing(The_RendererLogic);
		break;

	case 1:
		RSL_OnCodecEvent_Paused(The_RendererLogic);
		break;

	default:
		RSL_OnCodecEvent_Transit(The_RendererLogic);
		break;
	}
}

/******************************************************************************************
 *	END: Media Player related callback implementations
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: Intermediate methods for coordinating between renderer logic and media player.
 ******************************************************************************************/
void IM_FindTargetUri(void *rslObj, const char *mediaUri, int targetIndex, int wrapAround)
{
	enum MR_SupportedProtocolInfo protInfo;
	int ok;

	ok = 1;
	protInfo = UriToProtocolInfo(mediaUri);
	if ((protInfo == MR_PROTINFO_HTTP_AUDIO_M3U) || (protInfo == MR_PROTINFO_HTTP_VIDEO_M3U))
	{
		HttpPlaylistParser_FindTargetUri
			(
			The_PlaylistProcessor,
			wrapAround,
			mediaUri,
			targetIndex,
			rslObj,
			(IM_OnResult_PlaylistUriExists), 
			(IM_OnResult_UpdateItemCount), 
			(IM_OnResult_FindTargetUri)
			);
	}
	else if 
		(
		(protInfo == MR_PROTINFO_HTTP_AUDIO_MPEG) || 
		(protInfo == MR_PROTINFO_HTTP_VIDEO_MPEG) ||
		(protInfo == MR_PROTINFO_HTTP_AUDIO_WMA) || 
		(protInfo == MR_PROTINFO_HTTP_VIDEO_WMV)
		)
	{
		/*
		 *	Would be nice to be able to parse metadata from file.
		 */
		RSL_OnPlaylistLogicResult_MediaUriExists(rslObj, protInfo, mediaUri, "Same as track title", "Same as track artist", 1);
		RSL_OnPlaylistLogicResult_SetTrackTotal(rslObj, mediaUri, 1, MR_DURATION_UNKNOWN);
		RSL_OnPlaylistLogicResult_FoundTargetUri(rslObj, mediaUri, targetIndex, wrapAround, 1, mediaUri);
		RSL_SetTrackMetadata(rslObj, protInfo, "Unknown Track Title", "Unknown Artist");
	}
	else
	{
		// report that media is invalid
		IM_OnResult_PlaylistUriExists(NULL, mediaUri, rslObj, 0);
	}
}


int IM_IsCodecBusy(void *rslObj)
{
	long state;

	#ifdef _WIN32_WCE
		TheMediaPlayer->get_PlayState(&state);
	#elif WIN32
		state = TheMediaPlayer->get_PlayState();
	#endif

	return ((state == 9) || (SC_PendingSetFileName != 0));
}

void IM_Pause(void *rslObj)
{
	PM2_Pause();
}

void IM_Play(void *rslObj, enum MR_Enum_PlaySpeeds playSpeed)
{
	PM2_Play();
}

void IM_Stop(void *rslObj)
{
	SC_LastCommand = MR_State_Stopped;
	PM2_Stop();
}

void IM_StreamThis(void *rslObj, const char *trackUri)
{
	char *uri;
	uri = (char*)MPV_MALLOC((int)strlen(trackUri)+1);
	strcpy(uri, trackUri);
	PostMessage(MainWindow, WM_MPINVOKE, MPINV_SET_URI, (LPARAM)uri);
}

void IM_OnResult_FindTargetUri (void *parserObject, int wrapAround, const char* playlistUri, int itemIndex, void *userObject, /*OUT*/ int actualIndex, /*OUT - MUST COPY THIS*/ const char *targetUri, /*OUT*/int duration, /*OUT - MUST COPY THIS*/ const char* comment)
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

		#ifdef _WIN32_WCE
			double val;
			TheMediaPlayer->get_Duration(&val);
			codecDuration = (int)val;
		#elif WIN32
			codecDuration = (int)TheMediaPlayer->get_Duration();
		#endif

		if (m3uDuration > codecDuration)
		{
			duration = m3uDuration;
		}
		else
		{
			duration = codecDuration;
		}

		RSL_SetTrackDuration(userObject, duration);

		dashPos = -1;
		commentLen = (int)strlen(comment);

		data = (char*) MPV_MALLOC(commentLen+1);
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

		/* get protocolInfo for the target uri */
		protInfo = UriToProtocolInfo(targetUri);

		RSL_SetTrackMetadata(userObject, protInfo, data+tStart, data+cStart);
		MPV_FREE(data);
	}
}

void IM_OnResult_PlaylistUriExists (void* parserObject, const char* playlistUri, void* userObject, int uriExists)
{
	enum MR_SupportedProtocolInfo protInfo;

	if (uriExists != 0)
	{
		protInfo = UriToProtocolInfo(playlistUri);
		RSL_OnPlaylistLogicResult_MediaUriExists(userObject, protInfo, playlistUri, "", "", uriExists);
	}
	else
	{
		RSL_OnPlaylistLogicResult_MediaUriExists(userObject, MR_PROTINFO_HTTP_UNKNOWN, playlistUri, INVALID_MEDIA_URI_ERROR, "", uriExists);
	}
}

void IM_OnResult_UpdateItemCount (void *parserObject, const char* playlistUri, void* userObject, /*OUT*/int maxIndexNotKnown, /*OUT*/int itemCount)
{
	RSL_OnPlaylistLogicResult_SetTrackTotal(userObject, playlistUri, itemCount, MR_DURATION_UNKNOWN);
}

int IM_ValidateMediaUri(void *rslObj, const char *mediaUri)
{
	//TODO: Validate the media URI - possibly do things
	//like checking for routability and proper content.

	return 1;
}

/******************************************************************************************
 *	END: Intermediate methods for coordinating between renderer logic and media player.
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: Main switch statement for handling everything.
 ******************************************************************************************/
LRESULT MainSwitch (WPARAM wp, LPARAM lp)
{
	#ifdef _WIN32_WCE
		wchar_t *wuri;
		int len;
		int size;
		int wsize;
		int converted;
	#endif

	switch (wp)
	{
		case MPINV_SET_VOLUME_MASTER:
			SC_VolumeMaster = (int)lp;
			HandleMediaPlayer_SetVolume();
			MRSetVolume(MR_AudioChannel_Master,(unsigned short)lp);
			break;

		case MPINV_SET_VOLUME_LEFT:
			SC_VolumeLeft = (int)lp;
			HandleMediaPlayer_SetVolume();
			MRSetVolume(MR_AudioChannel_LF,(unsigned short)lp);
			break;

		case MPINV_SET_VOLUME_RIGHT:
			SC_VolumeRight = (int)lp;
			HandleMediaPlayer_SetVolume();
			MRSetVolume(MR_AudioChannel_RF,(unsigned short)lp);
			break;

		case MPINV_SET_MUTE:
			TheMediaPlayer->put_Mute((BOOL)lp);
			MRSetMute(MR_AudioChannel_Master,(int)lp);
			break;
		
		case MPINV_SET_URI: 
			SC_PendingSetFileName = 1;

			#ifdef _WIN32_WCE
				len = strlen((char*)lp);
				size = len + 1;
				wsize = size * sizeof(wchar_t);

				wuri = (wchar_t*)MPV_MALLOC(wsize);
				memset(wuri, 0, wsize);

				converted = mbstowcs(wuri, (char*)lp, len);
				TheMediaPlayer->put_FileName(wuri);
				MPV_FREE(wuri);
			
			#elif WIN32
				TheMediaPlayer->put_FileName((char*)lp);
			#endif
			
			if (
				(EndsWith((char*)lp, ".wma", 1) != 0) ||
				(EndsWith((char*)lp, ".wmv", 1) != 0)
				)
			{
				MrSetSeekTimePositionEnabled (0);
			}
			else
			{
				MrSetSeekTimePositionEnabled (1);
			}

			MPV_FREE ((char*)lp);
			break;

		case MPINV_SET_CURRENT_POSITION: 
			TheMediaPlayer->put_CurrentPosition((int)lp);
			break;

		case MPINV_SET_STATE:
			if ((IM_IsCodecBusy(The_RendererLogic) == 0) && (SC_PendingSetFileName == 0))
			{
				HandleMediaPlayer_SetState(SC_LastCommand);
			}
			break;


		case MPEVT_STATE_CHANGE: 
			HandleMediaPlayer_OnStateChange((long)lp);

			#ifdef _WIN32_WCE
			if ( ((lp == 6) || (lp == 3)) && (SC_PendingSetFileName != 0))
			#elif WIN32
			if ((lp == 6) && (SC_PendingSetFileName != 0))
			#endif
			{
				// If we get a skipForward after we set the
				// filename, then it means that mediaplayer
				// is ready to handle transport controls.
				SC_PendingSetFileName = 0;

				#ifdef _WIN32_WCE
					double dval;
					int ival;
					TheMediaPlayer->get_Duration(&dval);
					ival = (int) dval;
					RSL_SetTrackDuration(The_RendererLogic, ival);
				#elif WIN32
					RSL_SetTrackDuration(The_RendererLogic, (int)TheMediaPlayer->get_Duration());
				#endif
				
				HandleMediaPlayer_SetState(SC_LastCommand);
			}
			break;


		case RLINV_INV_SET_MEDIA_URI:
			#ifdef _WIN32_WCE
			SC_IgnoreLastCommand = 1;
			#endif
			RSL_SetMediaUri(The_RendererLogic, (char*)lp);
			MPV_FREE((char*)lp);
			break;

		case RLINV_NEXT_PREVIOUS:
			#ifdef _WIN32_WCE
			SC_IgnoreLastCommand = 1;
			#endif
			RSL_DoNextPrevious(The_RendererLogic, (int)lp);
			break;

		case RLINV_SET_TRACK_INDEX:
			#ifdef _WIN32_WCE
			SC_IgnoreLastCommand = 1;
			#endif
			RSL_DoSeekTrack(The_RendererLogic, (int)lp);
			break;
		
		case RLINV_SET_STATE:
			switch (lp)
			{
				case MR_State_Stopped:
					#ifdef _WIN32_WCE
					SC_IgnoreLastCommand = 0;
					#endif
					SC_LastCommand = MR_State_Stopped;
					RSL_DoStateChange(The_RendererLogic, MR_State_Stopped, MR_PlaySpeed_Ignore);
					break;
				case MR_State_Paused:
					#ifdef _WIN32_WCE
					SC_IgnoreLastCommand = 0;
					#endif
					SC_LastCommand = MR_State_Paused;
					RSL_DoStateChange(The_RendererLogic, MR_State_Paused, MR_PlaySpeed_Ignore);
					break;
				case MR_State_Playing:
					#ifdef _WIN32_WCE
					SC_IgnoreLastCommand = 0;
					#endif
					SC_LastCommand = MR_State_Playing;
					RSL_DoStateChange(The_RendererLogic, MR_State_Playing, MR_PlaySpeed_Ignore);
					break;
			}
			break;
		
		case RLINV_SET_PLAYMODE:
			RSL_SetPlayMode(The_RendererLogic, (enum MR_Enum_PlayModes)lp);
			break;
	}
	return 0;
}
/******************************************************************************************
 *	END: Main switch statement for handling everything.
 ******************************************************************************************/




/******************************************************************************************
 *	BEGIN: Initializer for MediaPlayerVersions_Methods
 ******************************************************************************************/
void Init_TheMediaPlayer(HWND mainWindow, void* mediaPlayer)
{
#ifdef _WIN32_WCE
	The_MediaPlayer = (CComPtr<IWMP>*) mediaPlayer;
#elif WIN32
	The_MediaPlayer = (CMediaplayer1*) mediaPlayer;
#endif

	TheMediaPlayer->put_SendErrorEvents(true);
	TheMediaPlayer->put_SendWarningEvents(true);
	TheMediaPlayer->put_ShowControls(!false);
	TheMediaPlayer->put_ShowDisplay(!false);
	TheMediaPlayer->put_ShowPositionControls(!false);
	TheMediaPlayer->put_ShowTracker(!false);
	TheMediaPlayer->put_AutoStart(false);

	MainWindow = mainWindow;
	SC_VolumeMaster = 80;
	SC_VolumeLeft   = 100;
	SC_VolumeRight  = 100;
	HandleMediaPlayer_SetVolume();
}

void Init_TheRendererVariables(void* chain, const char* friendlyname, const char* udn, const char* serialNo)
{
	void *ltm;
	The_RendererChain = chain;
	The_UpnpStack = UpnpCreateMicroStack(The_RendererChain, friendlyname, udn, serialNo, 1800, 0);
	The_PlaylistProcessor = CreatePlaylistParser(The_RendererChain, 3);
	ltm = ILibCreateLifeTime(The_RendererChain);
	The_MediaRenderer = CreateMediaRenderer(The_RendererChain, The_UpnpStack, ltm);

	The_RendererLogic = RSL_CreateRendererStateLogic
		(
		chain,
		The_MediaRenderer,
		IM_FindTargetUri,
		IM_StreamThis,
		IM_Play,
		IM_Stop,
		IM_Pause,
		IM_IsCodecBusy,
		IM_ValidateMediaUri
		);
}

void Uninit_TheRendererVariables()
{
	The_RendererChain = NULL;
	The_UpnpStack = NULL;
	The_MediaRenderer = NULL;
	The_RendererLogic = NULL;
}
/******************************************************************************************
 *	END: Initializer for MediaPlayerVersions_Methods
 ******************************************************************************************/
