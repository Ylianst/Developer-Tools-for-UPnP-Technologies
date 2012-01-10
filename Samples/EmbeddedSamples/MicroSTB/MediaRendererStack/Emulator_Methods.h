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
 *	Emulator.h and Emulator.c implement what normally belongs as part of the application layer. 
 *	This module provides the common application-level source code used in both _POSIX and WIN32
 *	MediaRenderer solutions. This module exists primarily to improve maintenability across
 *	different platform solutions.
 */

#ifndef _EMULATOR_METHODS_H
#define _EMULATOR_METHODS_H

/* [CODEC]: BEGIN */
/* Only 1 stream possible for our implementation. Other frameworks may allow multiple streams. */
#define STREAM_INDEX 0
#define MAX_STREAMS 1
/* [CODEC]: END */

/*
 *	This is the thread-chain used for the rendering device.
 */
// extern void* MR_RendererChain;

/*
 *	This is the generated microstack state object.
 */
// extern void* MR_UpnpStack;

/*
 *	This is the MicroMediaRenderer object, obtained from CreateMediaRenderer().
 */
extern void* MR_MediaRenderer;

/*
 *	This is the rendering state logic object, obtained from RSL_CreateRendererStateLogic().
 */
extern void* MR_RendererLogic;

/*
 *	This is the Extended-M3U playlist parser object, obtained from CreatePlaylistParser().
 */
extern void* MR_ExtendedM3uProcessor;

/*
 *	This methods need to be extern'd because they actually behave as callback sinks,
 *	and some compilers complain if attempting to assign a function pointer from
 *	a function declaration name.
 */

/*
 *	RendererStateLogic instructs instructs the solution to find a target URI,
 *	presumably for playback purposes.
 */
extern void InstructPlaylistLogic_FindTargetUri(void *rslObj, const char *mediaUri, int targetIndex, int wrapAround);

/*
 *	RendererStateLogic instructs the rendering
 *	framework to setup a stream with a specific URI.
 */
extern void InstructCodec_SetupStream(void *rslObj, const char *trackUri);

/*
 *	RendererStateLogic instructs the codec to play the current stream. 
 */
extern void InstructCodec_Play(void *rslObj, enum MR_Enum_PlaySpeeds playSpeed);

/*
 *	RendererStateLogic instructs the codec to stop the current stream. 
 */
extern void InstructCodec_Stop(void *rslObj);

/*
 *	RendererStateLogic instructs the codec to pause the current stream. 
 */
extern void InstructCodec_Pause(void *rslObj);

/*
 *	Queries the codec to see if it's busy doing something. 
 *	This is useful in that the RendererStateLogic will attempt
 *	to moderate calls to setup streams if the codec framework
 *	is busy handling another request. Assumption is that
 *	if the rendering framework is busy, it will event
 *	a state change so that the RendererStateLogic knows
 *	it can make its attempt because the codec is avaialble.
 */
extern int QueryCodec_IsBusy(void *rslObj);

/*
 *	RendererStateLogic will execute this method to verify that a URI
 *	is "valid" - this can range from a simple check on its formatting
 *	or can be as exhaustive as downloading parts of it to validate
 *	that it can be renderered.
 */
extern int Validate_MediaUri(void *rslObj, const char *mediaUri);

/*
 *	For optimization purposes, several of the sinks for
 *	MicroMediaRenderer callbacks go directly to the application layer.
 *
 *	Specifically, the callbacks that affect device state 
 *	go to the application layer first. Doing otherwise
 *	is slightly more inefficient because it means that the
 *	MicroMediaRenderer layer will do trivial checks, then forward
 *	execution to the RendererStateLogic, which would do a complete-pass
 *	through and forward to the application level. This would then continue
 *	by application level calling methods on processing the request,
 *	calling methods on RendererStateLogic, which would in turn call methods
 *	on MicroMediaRenderer and then finally on UpnpMicrostack. Thus the
 *	call stack would be a bit higher.
 *
 *	In other words, some callbacks don't need to go through the RendererStateLogic
 *	before reaching the application layer.
 */

/*
 *	UpnpMicrostack calls this method when it receives a request to change the volume levels.
 */
extern void MROnVolumeChangeRequestSink(enum MR_Enum_AudioChannels Channel,unsigned short Value);

/*
 *	UpnpMicrostack calls this method when it receives a request to change the mute settings.
 */
extern void MROnMuteChangeRequestSink(enum MR_Enum_AudioChannels Channel,int Value);

/*
 *	UpnpMicrostack calls this method when it receives a request to change the media (eg, playlistURI).
 *	Single track URIs can be sent here too. In our implementation, the RendererStateLogic will 
 *	handle both single-track and playlist URIs accordingly.
 */
extern void MROnMediaChangeRequestSink(const char* MediaUri);

/*
 *	UPnP AV is set up so that the control points poll for position - so the application
 *	layer receives this callback from UpnpMicrostack, and provides the output values
 *	for a proper response to GetPositionInfo. The reason why this read-only method is here
 *	is that the application layer can have access to the rendering framework. We don't
 *	assume that the Microstack tracks this information. 
 */
extern void MROnGetPositionRequestSink(int* seconds, int* absSeconds, int* count, int* absCount);

/*
 *	UpnpMicrostack calls this method when it receives a request to perform a seek operation.
 *	The seek operation can be for a new track target or a target position within the
 *	current track.
 */
extern void MROnSeekRequestSink(enum MR_Enum_SeekModes seekMode, int seekPosition);

/*
 *	UpnpMicrostack calls this method when it receives a request for Next or Previous.
 *	A positive value indicates to perform the next operation 'trackDelta' times.
 *	A negative value indicates to perform the previous operation 'trackDelta' times.
 */
extern void MROnNextPreviousRequestSink(int trackDelta);

/*
 *	UpnpMicrostack calls this method when it receives a request for state change,
 *	such as play, stop, pause.
 */
extern void MROnStateChangeRequestSink(enum MR_Enum_States state, enum MR_Enum_PlaySpeeds playSpeed);

/*
 *	UpnpMicrostack calls this method when it receives a request to change the
 *	play mode.
 */
extern void MROnPlayModeChangeRequestSink(enum MR_Enum_PlayModes playmode);

#endif
