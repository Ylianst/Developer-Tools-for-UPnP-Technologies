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
 *	This CodecWrapper.h and CodecWrapper.c implement a simulated rendering framework.
 *	Callers can specify the framework to setup streams using URIs and
 *	can instruct the rendering framework to play, stop, pause, etc.
 *
 *	All methods for this interface complete asynchronously. Furthermore, no CodecWrapper callbacks 
 *	should execute on a thread that has a CodecWrapper call in the call stack. In other words,
 *	if a thread executes CodecWrapper_Play(), then the execution path of that thread should not
 *	result in execution of Callback_CodecWrapper_StateChange callback.
 *
 *	Another recommended practice for implementing a new CodecWrapper, is to never event a 
 *	state change until after the calling thread leaves the scope of the method that caused
 *	the state change. Therefore, a Callback_CodecWrapper_StateChange callback should not execute
 *	until after the CodecWrapper_Play() method completes.
 *
 *	The CodecWrapper is useful primarily as an interface for the RendererStateLogic
 *	to make calls that cause the rendering framework to actually do some rendering work.
 *
 *	It should be noted that the codec wrapper only needs to support the playback of
 *	of a single content item. Logic for handling playlists is including in the
 *	RendererStateLogic. This methodology provides a consistent state machine across
 *	different playlists, allowing for consistent behavior.
 */

#ifndef CODEC_WRAPPER
#define CODEC_WRAPPER

/*
 *	Maximum size of the URI, including null character.
 */
#define MAX_URI_SIZE 1024

/*
 *	Defines various states that the rendering framework can be.
 */
enum CodecWrapperState
{
	MEDIA_UNINITIALIZED = 0,	/* codec library has no media or is not initialized */
	MEDIA_STOPPED,				/* codec library responded to stop method */
	MEDIA_TRANSIT,				/* codec library is busy in a transition */
	MEDIA_PLAYING,				/* codec library is currently playing */
	MEDIA_PAUSED,				/* codec library is in the paused state for playback */
	MEDIA_ENDED,				/* codec library finished decoding media, media has stopped without user request. */
	MEDIA_ERROR,				/* codec library was given a URI that could not be played */

	MEDIA_UNKNOWN				/* unknown playstate */
};

enum CodecWrapperPlaySpeed
{
	CW_PlaySpeed_Normal
};

/*
 *	The codec-framework defines a callback for processing a change in playstate for the codec-framework.
 *	The callback executes on a thread owned by the codec-framework, so sink methods should only 
 *	perform lightweight tasks. The 'streamTag' is the value that was sent in the CodecWrapper_Start() call.
 *
 *	The 'codecSpecific' argument allows the underlying implementation to return additional information
 *	about the state change (such as additional error information). The exact structure used in the return
 *	is not defined by this interface.
 */
typedef void (*Callback_CodecWrapper_StateChange)(enum CodecWrapperState newState, void* codecSpecific, void* streamTag);

/*
 *	The codec-framework defines a callback for updating the total duration of the current uri.
 *	This is sometimes necessary as some filtergraphs determine the total duration of a track
 *	asynchronously.
 */
typedef void (*Callback_CodecWrapper_DurationChange)(int duration, void* codecSpecific, void* streamTag);

/*
 *	The codec-framework defines a callback for indicating if the current stream supports 
 *	SetPosition.
 */
typedef void (*Callback_CodecWrapper_CanSetPosition)(int canSetPosition, void* codecSpecific, void* streamTag);

/*
 *	The codec-framework defines a callback for processing a change in position for the codec-framework.
 *	The callback executes on a thread owned by the codec-framework, so sink methods should only 
 *	perform lightweight tasks. The 'streamTag' is the value that was sent in the CodecWrapper_Start() call.
 *
 *	The 'bytesPosition' indicates how many byte position in the decode process.
 *
 *	The 'timePosition' indicates the number of playback seconds that are represented from byte-0 to 'bytesPosition'.
 *	In cases where the exact time value cannot be determined from the byte position, the caller
 *	will have to make do with an approximation. It is extremely important that an implementation of
 *	the codec-framework use 'bytesPosition' to derive 'timePosition' in order to accomodate seeking
 *	within a track.
 *
 *	The 'trackDuration' is the duration of the track in seconds.
 *
 *	The 'codecSpecific' argument allows the underlying implementation to return additional information
 *	about the position (such as an exact frame number or a millisecond portion of the time). 
 *	The exact contents of the argument are not defined by this interface.
 */
/*DEPRECATED:
typedef void (*Callback_CodecWrapper_PositionChange)(long bytesPosition, long timePosition, long trackDuration, void* codecSpecific, void* streamTag);
*/

/*
 *	Initializes the media playback library.
 *	Caller must specify the maximum number of streams
 *	that the codec library must be able to handle.
 */
void CodecWrapper_Init(int maxStreams);

/*
 *	Cleans up the media playback library.
 */
void CodecWrapper_UnInit();

/*
 *	Play, stop, pause the streaming and output of media
 *  CodecWrapper_Init must be called before these methods work.
 */

/*
 *	Call this method to start playing media at the specified 'URI'.
 *	This call is non-blocking and does very little other than
 *	set up the library for playback of a single URI. Heavyweight
 *	processing/decoding occurs on a separate thread.
 *
 *	The 'streamIndex' identifies which stream to play. The value
 *	is zero-based, and should not excede the value 'maxStreams'
 *	value used in CodecWrapper_Init().
 *
 *	The 'URI' argument is the single track item that the codec-framework
 *	should render. Implementations are expected to keep their own
 *	copy of this memory. Implementations must also be ready to
 *	accept an empty string ("") or NULL pointer. These two values should
 *	result in the filtergraph/codec layer from unloading the
 *	current track.
 *
 *	The 'streamTag' argument is used for passing the 'streamTag'
 *	parameter when the playstate and positionchange callbacks are executed.
 *	The caller must provide a non-NULL value in order to accomodate an
 *	implementation that uses a given set of platform resources
 *	even after switchin URIs.
 *
 *	The 'streamTag' argument is also useful for codec-frameworks that allow
 *	multiple streams, as the argument provides a means to identify
 *	a stream when given a command. Regardless of implementation,
 *	the caller is responsible for keeping the memory of 'streamTag'
 *	valid so that the codec-framework can do simple address comparisons
 *	for its own needs. 
 *
 *	Application-logic can send the avtransport instance that the codec
 *	should associate with the playing URI, as a means of passing the
 *	avtransport instance to the callback handler when processing
 *	state changes.
 *
 *	The 'stateChangeCallback' argument allows the caller to specify
 *	what method to execute when the play state of the codec-framework
 *	changes. Method implementations for the callback should be 
 *	very lightweight as execution of the callback is likely to be
 *	on the same thread as the decoding thread of the codec-framework.
 *	state changes.
 *
 *	DEPRECATED:
 *	The 'positionChangeCallback' argument allows the caller to specify
 *	what method to execute when the position of the current media
 *	changes. Method implementations for the callback should be 
 *	very lightweight as execution of the callback is likely to be
 *	on the same thread as the decoding thread of the codec-framework.
 *
 *	Returns nonzero if the stream cannot be set up. The method
 *	implementation is also expected to event its state to the
 *	stopped state when its setup operations are complete. The
 *	eventing can be done asynchronously, and need not execute
 *	on the thread that was used to call this method.
 */
int CodecWrapper_SetupStream(int streamIndex, const char* URI, void* streamTag, Callback_CodecWrapper_StateChange playstateChangeCallback, Callback_CodecWrapper_DurationChange durationChangeCallback, Callback_CodecWrapper_CanSetPosition canSetPositionCallback);

/*
 *	Call this method to transition to playing state. The caller
 *	must specify which stream should receive the command.
 *
 *	Returns nonzero if an error occurred.
 */
int CodecWrapper_Play(int streamIndex, enum CodecWrapperPlaySpeed playSpeed);

/*
 *	Call this method to stop playing media. The caller
 *	must specify which stream should receive the command.
 *
 *	Returns nonzero if an error occurred.
 */
int CodecWrapper_Stop(int streamIndex);

/*
 *	Call this method to pause the playing media. The caller
 *	must specify which stream should receive the command.
 *
 *	Returns nonzero if an error occurred.
 */
int CodecWrapper_Pause(int streamIndex);

/*
 *	Returns the playstate of the codec library given the 'streamTag'.
 *	Returns MEDIA_UNKNOWN if the stream could not be found.
 */
enum CodecWrapperState CodecWrapper_GetPlayState(int streamIndex);

/*
 *	Sets the time position in seconds.
 */
int CodecWrapper_SetPosition (int streamIndex, int timePosition);

/*
 *	Returns the estimated current playing position in seconds given the 'streamTag'.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
long CodecWrapper_GetTimePosition(int streamIndex);

/*
 *	Returns the estimated total playing time of the track in seconds given the 'streamTag'.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
long CodecWrapper_GetTimeTotalLength(int streamIndex);

/*
 *	Returns the current byte position of the decoding process given the 'streamTag'.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
long CodecWrapper_GetBytePosition(int streamIndex);

/*
 *	Returns the total number of bytes for the file given the 'streamTag'.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
long CodecWrapper_GetByteTotalLength(int streamIndex);

/*
 *	Get the media volume level given the 'streamTag'. Volume level is from 0 to 100.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
int CodecWrapper_GetVolume(int streamIndex);

/*
 *	set the media volume level given the 'streamTag'. Volume level is from 0 to 100.
 *	Returns a negative value if the stream could not be found or the value is unknown.
 */
int CodecWrapper_SetVolume(int streamIndex, int level);

/*
 *	Returns nonzero if the codec has the mute flag enabled.
 */
int CodecWrapper_GetMute(int streamIndex);

/*
 *	Sets the mute flag for the specified stream.
 *	Nonzero value indicates mute flag is on.
 */
int CodecWrapper_SetMute(int streamIndex, int muteFlag);

#endif

