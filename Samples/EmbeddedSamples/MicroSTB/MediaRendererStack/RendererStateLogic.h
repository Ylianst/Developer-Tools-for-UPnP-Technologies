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


/*
 *	High Level Description.
 *
 *	RendererStateLogic.h and RendererStateLogic.c provide the logic for
 *	a MediaRenderer state machine. The module is probably not too useful
 *	to customers that already have a state machine for an existing rendering device.
 *	Such customers probably want to have their current solution interface
 *	directly with the MicroMediaRenderer module.
 *
 *	That being said, this module abstracts the following tasks from the
 *	upper application layers:
 *
 *		-provides callbacks to instruct the rendering framework to perform streaming
 *		 related state changes
 *
 *		-provide the upper application layer the means to instruct the
 *		 renderer state machine to handle next,previous,etc. actions.
 *		
 *		-provide the upper applicaiton layer the means to report the rendering
 *		 framework's current state
 *
 *		-provide the upper application layer the means to report a target URI and
 *		 other playlist related information
 *
 *		-provides a callback to request a target URI from a playlist
 */

#ifndef _STATE_LOGIC
#define _STATE_LOGIC

#include "UpnpMicroStack.h"

#define INVALID_MEDIA_URI_ERROR "Invalid or non-reachable media URI"

/*
 *	DEVELOPER NOTE:
 *
 *	Developers may be able to reduce code size by changing these
 *	interface methods by removing the (void* rslObj) parameter.
 *	In our experience, if the implementation uses a statically
 *	allocated token, then the compiled-linked binary is actually
 *	smaller. See MicroMediaRenderer.h for an example of this technique
 *	at work.
 */

typedef void (*Callback_StateLogic_InstructPlaylistLogic_FindTargetUri) (void *rslObj, const char *mediaUri, int targetIndex, int wrapAround);

typedef void (*Callback_StateLogic_InstructCodec_SetupStream)(void *rslObj, const char *trackUri);

typedef void (*Callback_StateLogic_InstructCodec_StopPause) (void *rslObj);

typedef void (*Callback_StateLogic_InstructCodec_Play) (void *rslObj, enum MR_Enum_PlaySpeeds playSpeed);

typedef int (*Callback_StateLogic_Query_CodecBusy) (void *rslObj);

typedef int (*Callback_StateLogic_Validate_MediaUri) (void *rslObj, const char *mediaUri);

void* RSL_CreateRendererStateLogic
	(
	void *Chain,
	void *MediaRenderer,
	Callback_StateLogic_InstructPlaylistLogic_FindTargetUri methodFindTargetUri,
	Callback_StateLogic_InstructCodec_SetupStream methodSetupStream,
	Callback_StateLogic_InstructCodec_Play methodPlay,
	Callback_StateLogic_InstructCodec_StopPause methodStop,
	Callback_StateLogic_InstructCodec_StopPause methodPause,
	Callback_StateLogic_Query_CodecBusy queryCodecBusy,
	Callback_StateLogic_Validate_MediaUri validateMediaUri
	);

/*
 *	Call this method when the renderer needs to go to the
 *	previous or next track.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	trackDelta:	The number of tracks to move ahead or behind.
 */
void RSL_DoNextPrevious(void* rslObj, int trackDelta);

/*
 *	Call this method when the renderer needs to seek to
 *	a particular track index.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	value:		Target track index.
 */
void RSL_DoSeekTrack(void *rslObj, int value);

/*
 *	Call this method when the renderer needs to perform
 *	a state change.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	desiredState:		The desired playstate.
 *
 *	playSpeed:			The desired play speed. Valid if and only if desiredState==MR_State_Playing
 */
void RSL_DoStateChange(void* rslObj, enum MR_Enum_States desiredState, enum MR_Enum_PlaySpeeds playSpeed);

/*
 *	Call these methods when the codec changes state.
 *	The methods will appropriately report state transitions
 *	and affect track transitions where appropriate.
 */
void RSL_OnCodecEvent_Stopped(void *rslObj);
void RSL_OnCodecEvent_Ended(void *rslObj);
void RSL_OnCodecEvent_Error(void *rslObj);
void RSL_OnCodecEvent_Transit(void *rslObj);
void RSL_OnCodecEvent_Playing(void *rslObj);
void RSL_OnCodecEvent_Paused(void *rslObj);

/*
 *	Call this method when the playlist logic has found the URI for a target track.
 *	This method should only be called in response to a 
 *	Callback_StateLogic_InstructPlaylistLogic_FindTargetUri method call.
 *
 *	rslObj:				Object that was sent in the original 
 *						Callback_StateLogic_InstructPlaylistLogic_FindTargetUri call.
 *
 *	mediaUri:			Playlist/media URI specified in the original
 *						Callback_StateLogic_InstructPlaylistLogic_FindTargetUri call.
 *
 *	targetIndex:		Target track index specified in the original
 *						Callback_StateLogic_InstructPlaylistLogic_FindTargetUri call.
 *
 *	wrapAround:			Indication if wrapAround behavior was applied when processing 
 *						the targetIndex.
 *
 *	resultActualIndex:	Actual index of the URI. Remember that the target index may 
 *						point to a URI that is not in the range of tracks available
 *						in the playlist. If wrapAround is nonzero, then this value
 *						will be (targetIndex % total tracks). Otherwise, this value
 *						will be zero or equal to (total tracks).
 *
 *	resultTargetUri:	URI of the requested target track.
 *
 *	Returns:			Nonzero if the information was applied. If zero, then
 *						the track request has been superceded.
 */
int RSL_OnPlaylistLogicResult_FoundTargetUri (void *rslObj, const char *mediaUri, int targetIndex, int wrapAround, int resultActualIndex, const char *resultTargetUri);

/*
 *	Call this method when the playlist logic has determined if the media URI
 *	is invalid or does not exist. Appropriately specify the metadata for the
 *	media if known. If uriExists is zero, then the caller is suggested
 *	to specify the title with INVALID_MEDIA_URI_ERROR or something similar.
 */
void RSL_OnPlaylistLogicResult_MediaUriExists (void *rslObj, enum MR_SupportedProtocolInfo protInfo, const char *mediaUri, const char *title, const char *creator, int uriExists);

/*
 *	Call this method when the playlist logic has an updated count for the
 *	number of tracks in the media/playlist.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	mediaUri:	The URI of the media/playlist.
 *
 *	trackTotal:	The number of tracks for the media.
 *
 *	totalDuration:	Total duration of the media in seconds, if known. If unknown, use MR_DURATION_UNKNOWN.
 */
void RSL_OnPlaylistLogicResult_SetTrackTotal(void *rslObj, const char *mediaUri, int trackTotal, int totalDuration);

/*
 *	Call this method when the renderer knows it can support or not support
 *	seek operations for time position on the current track.
 */
void RSL_CanSetPosition(void *rslObj, int canSetPosition);

/*
 *	Call this method when the renderer needs to set a new playlist.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	mediaUri:	The media to play. Usually a playlist.
 */
void RSL_SetMediaUri(void* rslObj, const char *mediaUri);

/*
 *	Call this method when the renderer needs to set the mute flag for
 *	the specified channel. 
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	Channel:	Audio channel to receive mute flag change. Assumes that the DeviceBuilder-generated
 *				stack lists only the supported audio channels.
 *
 *	value:		Nonzero value indicates that the channel should be muted.
 */
/*
 *	DEPRECATED: Architecturally does not belong here because RendererStateLogic should
 *	not handle RenderingControl related tasks.
 *
void RSL_SetMute(void* rslObj, enum MR_Enum_AudioChannels channel, unsigned short value);
 */

/*
 *	Call this method when the renderer needs to change its
 *	play mode.
 */
void RSL_SetPlayMode(void* rslObj, enum MR_Enum_PlayModes playMode);

/*
 *	Call this method when the duration of the current track is determined.
 *
 *	WARNING: Caller should call this method after calling 
 *	RSL_OnPlaylistLogicResult_FindTargetUri(). The call should be made 
 *	without switching threads and while in the same scope as the call 
 *	to RSL_OnPlaylistLogicResult_FindTargetUri(). Doing otherwise may
 *	result in incorrect reporting of a duration for a different track.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *	
 *	duration:	Duration of track in seconds.
 */
void RSL_SetTrackDuration(void *rslObj, int duration);

/*
 *	Call this method when the metadata of the current track is determined.
 *
 *	WARNING: Caller should call this method after calling 
 *	RSL_OnPlaylistLogicResult_FindTargetUri(). The call should be made 
 *	without switching threads and while in the same scope as the call 
 *	to RSL_OnPlaylistLogicResult_FindTargetUri(). Doing otherwise may
 *	result in incorrect reporting of metadata for the track.
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	protInfo:	ProtocolInfo for the track.
 *
 *	title:		Title of the current track. Must be non-NULL.
 *
 *	artist:		Artist of the current track. Must be non-NULL.
 */
void RSL_SetTrackMetadata(void *rslObj, enum MR_SupportedProtocolInfo protInfo, const char *title, const char *artist);

/*
 *	Call this method when the renderer needs to set the volume for
 *	the specified channel. 
 *
 *	rslObj:		Object returned from CreateRendererStateLogic().
 *
 *	Channel:	Audio channel to receive volume change. Assumes that the DeviceBuilder-generated
 *				stack lists only the supported audio channels.
 *
 *	value:		A volume level within the range specified by the DeviceBuilder-generated
 *				stack.
 *
 */
/*
 *	DEPRECATED: Architecturally does not belong here because RendererStateLogic should
 *	not handle RenderingControl related tasks.
 *
void RSL_SetVolume(void* rslObj, enum MR_Enum_AudioChannels channel, unsigned short value);
 */


#endif
