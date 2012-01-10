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
 *	MicroMediaRenderer.h and MicroMediaRenderer.c provide a thin abstraction layer
 *	of UPnP-AV specific material. It presumes a particular feature set of the
 *	generated Microstack (in terms of advertised device capabilities).
 *
 *	The module's primary goals are as follows:
 *		-provide programmer-friendly enumerations for well-defined values/types
 *		-allow the upper-layers to report the device's state onto the UPnP network through method calls
 *		-provides callbacks for handling action requests that actually change device state
 *		-respond to control point actions that query about device state.
 *		-handle the details of UPnP events
 *
 *	The only real exception to this rule is GetCurrentPositionInfo, which uses 
 *	a callback so taht the upper layers can poke at the rendering framework
 *	to obtain the current position.
 */

#ifndef MICROMEDIARENDERER_H
#define MICROMEDIARENDERER_H

#define MR_DURATION_UNKNOWN -1
#define MR_COUNTER_UNKNOWN 2147483647

/* List supported media formats here */
#define EXTENSION_AUDIO_MPEG		".mp3"
#define EXTENSION_AUDIO_WMA			".wma"
#define EXTENSION_VIDEO_WMV			".wmv"
#define EXTENSION_VIDEO_MPEG		".mpg"
#define EXTENSION_PLAYLIST_M3U		".m3u"

/* error message for when unsupported formats are used in AVT.SetAVTransportURI. */
#define MR_ERR_CODE_BAD_FILETYPE	714
#define MR_ERR_MSG_BAD_FILETYPE		"MediaRenderer only supported audio/mpeg (.mp3), audio/mpegurl (.m3u), audio/x-ms-wma (.wma), video/x-ms-wmv (.wmv), video/mpeg (.mpeg)."


/*
 *	This implementation supporte the following
 *	protocolInfo.
 */
enum MR_SupportedProtocolInfo
{
	MR_PROTINFO_HTTP_UNKNOWN = 0,
	MR_PROTINFO_HTTP_AUDIO_M3U,
	MR_PROTINFO_HTTP_VIDEO_M3U,
	MR_PROTINFO_HTTP_AUDIO_MPEG,
	MR_PROTINFO_HTTP_AUDIO_WMA,
	MR_PROTINFO_HTTP_VIDEO_MPEG,
	MR_PROTINFO_HTTP_VIDEO_WMV
};

/*
 *	This implementation of the rendering device has 3 audio channels: master, left front, and right front.
 */
enum MR_Enum_AudioChannels
{
	MR_AudioChannel_Master = 0,
	MR_AudioChannel_LF     = 1,
	MR_AudioChannel_RF     = 2
};

/*
 *	Rendering device has 5 standard play modes.
 */
enum MR_Enum_PlayModes
{
	MR_PlayMode_Normal     = 0,
	MR_PlayMode_RepeatOne  = 1,
	MR_PlayMode_RepeatAll  = 2,
	MR_PlayMode_Random     = 3,
	MR_PlayMode_Shuffle    = 4
};

/*
 *	Possible play speeds.
 */
enum MR_Enum_PlaySpeeds 
{
	MR_PlaySpeed_Ignore		= 0,	/* use this value for non-Play commands */
	MR_PlaySpeed_Normal		= 1,
};

/*
 *	All possible play states.
 */
enum MR_Enum_States
{
	MR_State_Stopped       = 0,
	MR_State_Paused        = 1,
	MR_State_Playing       = 2,
	MR_State_Transit       = 3,
	MR_State_NoMedia       = 4
};

/*
 *	Supported seek modes.
 */
enum MR_Enum_SeekModes
{
	MR_SeekMode_RelTime		= 0,
	MR_SeekMode_TrackNr		= 1
};

enum MR_Enum_Status
{
	MR_Status_OK			= 0,
	MR_Status_ERROR			= 1
};

/*
 *	DEVELOPER NOTE:
 *
 *	All of the methods and callbacks should have the first argument
 *	by the void* token returned in CreateMediaRenderer to comply with
 *	"good interface design"... but we discovered that actually sending
 *	the additional argument increased code size. For device hierarchies
 *	that are exposing a single UPnP MediaRenderer, a statically allocated
 *	token is sufficient. Otherwise, the interfaces and implementations 
 *	need to be modified to use the token. See RendererStateLogic.h
 *	for an example of sending the (void*) token in each call.
 */

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a volume change request.
 */
extern void (*MROnVolumeChangeRequest) (enum MR_Enum_AudioChannels,unsigned short);

/*
 *	Application-level logic can call this method to instruct the UPnP layer
 *	to report a volume change.
 */
void MRSetVolume(enum MR_Enum_AudioChannels channel,unsigned short volume);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a mute change request.
 */
extern void (*MROnMuteChangeRequest) (enum MR_Enum_AudioChannels,int);

/*
 *	Application-level logic can call this method to instruct the UPnP layer
 *	to report a mute change.
 */
void MRSetMute(enum MR_Enum_AudioChannels channel,int mute);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to change the media URI. Media uris can include
 *	collections of media, such as a URI to a playlist or a proprietary
 *	URI for the local audio-CD.
 */
extern void (*MROnMediaChangeRequest) (const char* mediaUri);

/*
 *	Application-level logic can call this method to instruct the UPnP layer
 *	to report a change in the media URI. 
 */
void MRSetMediaUri(const char* mediaUri);

/*
 *	Application logic can call this method to instruct the UPnP layer to report
 *	the media's metadata about the creator and title, which is useful when describing
 *	something things like artist, album-title, photo album names of a content collection.
 *	This method is separated from MRSetMedia() because the information may be obtained
 *	at a time after the media uri is set.
 *
 *	protInfo:			protocolInfo value for this media
 *	title:				title information for the media; must be non-null
 *	creator:			creator information for the media; must be non-null
 */
void MRSetMediaInfo(enum MR_SupportedProtocolInfo protInfo, const char* mediaTitle, const char* mediaCreator);

/*
 *	Application logic can call this method to instruct the UPnP layer to report
 *	a change in the total duration and number of track/items for the playlist/media.
 *	This method is separated from MRSetMediaInfo and MRSetMedia because the 
 *	total number of items that represent a media collection can be updated progressively.
 *
 *	Use MR_DURATION_UNKNOWN or MR_COUNTER_UNKNOWN to report unknown totalMediaDuration and totalMediaCounter.
 */
void MRSetMediaTotals(int totalTracks, int totalMediaDuration, int totalMediaCounter);

/*
 *	Application logic can call this method to instruct the UPnP layer to report
 *	a track/item change. Application provides the uri for the track and
 *	the track number.
 */
void MRSetTrack(const char* trackUri, int trackNumber);

/*
 *	Application logic can call this method to instruct the UPnP layer to report
 *	the specified duration info for the current track. 
 *
 *	trackDuration:		number of seconds in the media; MR_DURATION_UNKNOWN if unknown
 *	trackCounterTotal:	item's total for counter-based position; MR_COUNTER_UNKNOWN if unknown
 */
void MRSetTrackDurationInfo(int trackDuration, int trackCounterTotal);

/*
 *	Application logic can call this method to instruct the UPnP layer to report
 *	the specified metadata for the current track. 
 *
 *	protInfo:			protocolInfo value for this track
 *	title:				title information for the track/item; must be non-null
 *	creator:			creator information for the track/item; must be non-null
 */
void MRSetTrackMetadata(enum MR_SupportedProtocolInfo protInfo, const char* title, const char* creator);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to obtain the current position information for a track.
 *
 *	seconds:	Current playback position for a track/item. 
 *				MR_DURATION_UNKNOWN indicates unknown position.
 *
 *	absSeconds:	Current playback position relative to the absolute total time of the media. 
 *				MR_DURATION_UNKNOWN indicates unknown absolute position.
 *
 *	count:		Arbitrary application-level defined counter to represent position.
 *				Use MR_COUNTER_UNKNOWN if unknown or not supported.
 *
 *	absCount:	Arbitrary application-level defined counter to represent position relative to the absolute
 *				total counter value for the media. Use MR_COUNTER_UNKNOWN if unknown or not supported.
 */
extern void (*MROnGetPositionRequest) (int* seconds, int* absSeconds, int* count, int* absCount);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to handle a seek operation.
 */
extern void (*MROnSeekRequest) (enum MR_Enum_SeekModes seekMode, int seekPosition);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to handle a next or previous command.
 *	Negative values indicate previous requests and positive values
 *	indicate next requests.
 */
extern void (*MROnNextPreviousRequest) (int trackdelta);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to change the play state.
 */
extern void (*MROnStateChangeRequest) (enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeeds);

/*
 *	Application-logic can call this method to instruct the UPnP layer
 *	to report that the device current supports seek operations
 *	for a position in a track.
 */
void MrSetSeekTimePositionEnabled (int seekEnabled);

/*
 *	Application-logic can call this method to instruct the UPnP layer
 *	report a change in play state.
 */
void MRSetState(enum MR_Enum_States state);

/*
 *	Application-logic can call this method to instruct the UPnP layer
 *	to report a change in the transport status.
 */
void MRSetStatus(enum MR_Enum_Status status);

/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request to change the play mode.
 */
extern void (*MROnPlayModeChangeRequest) (enum MR_Enum_PlayModes playmode);
/*
 *	Application-logic can call this method to instruct the UPnP layer
 *	report a change in play mode.
 */
void MRSetPlayMode(enum MR_Enum_PlayModes playmode);

/* Renderer Creation, use with ILibParsers thread chaining system. */
void* CreateMediaRenderer(void* Chain, void* Stack, void* lifetimeMonitor);

#endif

