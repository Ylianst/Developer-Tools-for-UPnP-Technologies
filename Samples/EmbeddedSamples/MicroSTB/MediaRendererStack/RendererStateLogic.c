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


#ifdef _POSIX

	/*
	 *	Encountered some compiler failures with PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
	 *	on Linux. Defining _GNU_SOURCE resolves the problem.
	 */
	#define _GNU_SOURCE

	#include <pthread.h>
	#include <semaphore.h>
#endif


#include <stdio.h>

#ifdef _WIN32_WCE
	#define assert(x)
#else
	#include <assert.h>
#endif

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <string.h>

#include "ILibParsers.h"
#include "MicroMediaRenderer.h"
#include "RendererStateLogic.h"
#include "Utility.h"

#ifdef _DEBUG
	#define DEBUGONLY(x) x
	#define ASSERT(x) assert(x)

	#define RSL_MALLOC	rsl_malloc
	#define RSL_FREE	rsl_free

	int rsl_malloc_counter = 0;
	void* rsl_malloc (int size)
	{
		++rsl_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("rsl_malloc_counter=%d\r\n", rsl_malloc_counter);
		#endif
		return MALLOC(size);
	}

	void rsl_free (void *ptr)
	{
		--rsl_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("rsl_malloc_counter=%d\r\n", rsl_malloc_counter);
		#endif
		FREE(ptr);
	}
#endif

#ifndef _DEBUG

	#define DEBUGONLY(x) 
	#ifndef ASSERT
		#define ASSERT(x)
	#endif

	#define RSL_MALLOC	MALLOC
	#define RSL_FREE	FREE
#endif

#ifdef _TEMPDEBUG
	#define TEMPDEBUGONLY(x) x
#endif

#ifndef _TEMPDEBUG
	#define TEMPDEBUGONLY(x)
#endif

#ifdef WIN32
#include <windows.h>

#define strncasecmp strnicmp
#define strcasecmp stricmp
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#define pthread_mutexattr_t									int
#define pthread_mutexattr_init(pt_mutexattr)
#define pthread_mutexattr_settype(pt_mutexattr,mtype)		
#define pthread_mutex_t										HANDLE
#define PTHREAD_MUTEX_RECURSIVE				
#define pthread_mutex_init(pt_mutex,pt_mutexattr)			*pt_mutex = CreateMutex(NULL, FALSE, NULL);
#define pthread_mutex_destroy(pt_mutex)						(CloseHandle(*pt_mutex)==0?1:0)
#define pthread_mutex_lock(pt_mutex)						WaitForSingleObject(*pt_mutex, INFINITE)
#define pthread_mutex_unlock(pt_mutex)						ReleaseMutex(*pt_mutex)
#endif

#ifdef _WIN32_WCE
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

#define pthread_mutexattr_t									int
#define pthread_mutexattr_init(pt_mutexattr)
#define pthread_mutexattr_settype(pt_mutexattr,mtype)		
#define pthread_mutex_t										HANDLE
#define PTHREAD_MUTEX_RECURSIVE				
#define pthread_mutex_init(pt_mutex,pt_mutexattr)			*pt_mutex = CreateMutex(NULL, FALSE, NULL);
#define pthread_mutex_destroy(pt_mutex)						(CloseHandle(*pt_mutex)==0?1:0)
#define pthread_mutex_lock(pt_mutex)						WaitForSingleObject(*pt_mutex, INFINITE)
#define pthread_mutex_unlock(pt_mutex)						ReleaseMutex(*pt_mutex)
#endif

/*
 *	If transiting to a new track, possibly use an empty string to represent
 *	the device is in transit. Another option is to use the current track URI.
 */
#define PENDING_TARGET_URI ""

#define INVALID_MEDIA_URI "Error - Invalid Media URI"

#define NO_MEDIA_URI "No Media Specified"

struct RendererStateLogic
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	/*
	 *	Current media Uri.
	 */
	char *MediaUri;

	/* current track index that is being played */
	int	CurrentTrackIndex;

	/*
	 *	pending target index that has been requested - 
	 *	warning, may be outside the range of [0, totalTrackCount]
	 *	because of random selection outside current range.
	 */
	int PendingTargetIndex;

	/*
	 *	total track count
	 */
	int	TotalTrackCount;
	
	/*
	 *	current play mode
	 */
	enum MR_Enum_PlayModes PlayMode;

	/*
	 *	desired play state
	 */
	enum MR_Enum_States DesiredState;	

	/*
	 *	desired play speed
	 */
	enum MR_Enum_PlaySpeeds DesiredPlaySpeed;

	/*
	 *	If nonzero, then then mediaUri or pendingTargetIndex
	 *	has changed in some way.
	 */
	unsigned char TargetChanged;

	/*
	 *	If nonzero, then the codec & playlist-processor is assumed to be busy
	 *	setting up another stream.
	 */
	unsigned char PendingRequest;

	/*
	 *	If nonzero, then we've observed a transport error.
	 *	Flag is only reset to zero after receiving a transport
	 *	request or a media URI change.
	 */
	unsigned char TransportErrorFlag;

	/*
	 *	Execute this callback when we need to find the URI
	 *	of a particular target.
	 */
	Callback_StateLogic_InstructPlaylistLogic_FindTargetUri Method_FindTargetUri;

	/*
	 *	Execute this callback when we need to instruct the
	 *	codec to set up a stream.
	 */
	Callback_StateLogic_InstructCodec_SetupStream Method_SetupStream;

	/*
	 *	Execute these callback when we need to instruct the
	 *	codec to play, stop, or pause.
	 */
	Callback_StateLogic_InstructCodec_Play Method_Play;
	Callback_StateLogic_InstructCodec_StopPause Method_Stop;
	Callback_StateLogic_InstructCodec_StopPause Method_Pause;
	/*
	 *	Execute this callback when we need to know if the
	 *	codec/rendering framework is busy.
	 */
	Callback_StateLogic_Query_CodecBusy Method_QueryIsCodecBusy;

	/*
	 *	Execute this callback when we need to know the playback position
	 *	of the codec's current track.
	 */
	//Callback_StateLogic_Query_CodecPosition Query_CodecPosition;

	/*
	 *	Execute this callback to allow the app layer to validate
	 *	the media uri.
	 */
	Callback_StateLogic_Validate_MediaUri Method_ValidateMediaUri;

	/*
	 *	For use with MicroMediaRenderer.h interfaces.
	 */
	void* MediaRenderer;

	/*
	 *	Recursive mutex for synchronizing all calls.
	 */
	/*pthread_mutex_t Lock;*/
	sem_t Lock;
};

/* Private method declarations */
void RSL_DestroyRslObj(void *rslObj);
void _RSL_HandleNewTrackUri(struct RendererStateLogic* rsl, const char *trackUri);
void _RSL_HandleTrackChange(struct RendererStateLogic* rsl, int seekFlag, int trackDelta, int endOfTrack);
void _RSL_ModeratedTrackUpdate(struct RendererStateLogic* rsl);
void _RSL_OnCodecStateChange(struct RendererStateLogic* rsl);
void _RSL_SetNewTrackUri(struct RendererStateLogic* rsl, const char *trackUri);

/*
 *	Obsoleted. Relevant Code in _RSL_SetNewTrackUri.
void _RSL_HandleNewTrackUri(struct RendererStateLogic* rsl, const char *trackUri)
{
	char *copy;
	int len;

	DEBUGONLY(printf("_RSL_HandleNewTrackUri (%s)\r\n", trackUri);)

	if (trackUri != NULL)
	{
		len = (int) strlen(trackUri) + 1;

		copy = (char*) RSL_MALLOC(len);
		memcpy(copy, trackUri, len);
	}
	else
	{
		copy = (char*) RSL_MALLOC(1);copy[0] = '\0';
	}

	_RSL_SetNewTrackUri(rsl, trackUri);

	RSL_FREE (copy);
}
 */

/*
 *	This method executes when the RSL is told of a codec event or a UPnP action request
 *	that would require a change in the current track URI. 
 *
 *	This method will calculate a new target index for the current mediaURI.
 *
 *	If the new target indicates that a new URI is needed, RSL reports a transit state
 *	and executes the callback for finding a new target URI.
 *
 *	If the input parameters and the new target indicate that rendering should stop, then
 *	RSL reports a stopped state.
 *
 *	This method can execute an RSL callback, so all calls to this method should never
 *	have the rsl->Lock before making the call.
 */
void _RSL_HandleTrackChange(struct RendererStateLogic* rsl, int seekFlag, int trackDelta, int endOfTrack)
{
	unsigned char wrapAround = 0;
	unsigned char target = 0;
	unsigned char stop = 0;
	unsigned char moderatedTrackUpdate = 0;

	DEBUGONLY(printf("_RSL_HandleTrackChange (%d, %d, %d) - playMode=%d mediaUri='%s'\r\n", seekFlag, trackDelta, endOfTrack, rsl->PlayMode, rsl->MediaUri);)

	/* Ignore all requests to do previous and next if no valid playlist is set. */
	if (rsl->MediaUri == NULL) return;

	/* lock */
	sem_wait(&(rsl->Lock));

	/*
	 *	Given the input parameters and the current state of the RSL
	 *	object, calculate a new target index for the current mediaURI.
	 */

	if (seekFlag != 0)
	{
		switch (rsl->PlayMode)
		{
		case MR_PlayMode_Normal:
			/* wrapAround = 0; initialized at top*/ 
			break;

		case MR_PlayMode_RepeatOne:
		case MR_PlayMode_RepeatAll:
		case MR_PlayMode_Random:
		case MR_PlayMode_Shuffle:
			wrapAround = 1;
			break;
		}

		target = trackDelta;
	}
	else
	{
		switch (rsl->PlayMode)
		{
		case MR_PlayMode_Normal:
			/* wrapAround = 0; initialized at top*/ 
			
			if (
				(trackDelta == 1) && 
				(endOfTrack) && 
				(rsl->CurrentTrackIndex == rsl->TotalTrackCount)
				)
			{
				/*
				 *	If we're doing a next because the track ended,
				 *	and we just played the last track, then
				 *	go ahead and stop.
				 */

				stop = 1;
			}
			else
			{
				/*
				 *	Otherwise, set a new pending target track.
				 *	Be sure to add to pending target to ensure that
				 * 	earlier/pending previous/next requests are 
				 *	taken into account. Also ensure we're within
				 *	the known range of total tracks.
				 */
				
				target = rsl->PendingTargetIndex + trackDelta;
				if (target > rsl->TotalTrackCount)
				{
					target = rsl->TotalTrackCount;
				}
			}
			break;

		case MR_PlayMode_RepeatOne:
			wrapAround = 1;
			if ((trackDelta == 1) &&(endOfTrack))
			{
				/*
				 *	If we're advancing to the next track because
				 *	the current track ended, then simply set the
				 *	new target to be the current track.
				 */
				target = rsl->CurrentTrackIndex;
			}
			else
			{
				/*
				 *	Otherwise, change the target track. Be
				 *	sure to take pending previous/next requests
				 *	by adding to the pending target.
				 */
				target = rsl->PendingTargetIndex + trackDelta;
			}
			break;

		case MR_PlayMode_RepeatAll:
			/*
			 *	Simply enable wrapping and change the track
			 *	based on the last pending request.
			 */
			wrapAround = 1;
			target = rsl->PendingTargetIndex + trackDelta;
			break;

		case MR_PlayMode_Random:
			/*
			 *	Pick a random track.
			 */
			wrapAround = 1;
			target = rsl->CurrentTrackIndex + (rand() % rsl->TotalTrackCount);
			break;

		case MR_PlayMode_Shuffle:
			// TODO: handle shuffle
			target = rsl->CurrentTrackIndex + (rand() % rsl->TotalTrackCount);
			break;
		}
	}

	if (wrapAround != 0)
	{
		/*
		 *	Ensure that the target track is within range of the
		 *	known number of tracks by taking the modulo.
		 */
		target = (target % rsl->TotalTrackCount);
		if (target == 0) target = rsl->TotalTrackCount;
	}
	else
	{
		/*
		 *	Ensure that the target track is within range of the
		 *	known number of tracks by setting the track to
		 *	the first or last if beyond first/last track.
		 */
		if (target < 1) target = 1;
		if (target > rsl->TotalTrackCount) target = rsl->TotalTrackCount;
	}

	/* unlock rsl state */
	sem_post(&(rsl->Lock));

	
	if (stop != 0)
	{
		/*
		 *	If the logic determined that a new new track is not needed and that
		 *	the device should stop rendering because it reached the last track
		 *	in normal mode, then report a stop state and then set the
		 *	current track index to the first track.
		 */
		RSL_DoStateChange(rsl, MR_State_Stopped, MR_PlaySpeed_Ignore);
		RSL_DoSeekTrack(rsl, 1);
	}
	else
	{
		/* lock the rsl state */
		sem_wait(&(rsl->Lock));

		if ((rsl->PlayMode == MR_PlayMode_Normal) && (trackDelta > 0) && (seekFlag == 0) && (rsl->CurrentTrackIndex == rsl->TotalTrackCount))
		{
			/*
			 *	Don't do a track change because play mode is normal 
			 *	and we're trying to do a next on the last track.
			 */
		}
		else if ((rsl->PlayMode == MR_PlayMode_Normal) && (trackDelta < 0) && (seekFlag == 0) && (rsl->CurrentTrackIndex == 1))
		{
			/*
			 *	Don't do a track change because play mode is normal 
			 *	and we're trying to do a next on the last track.
			 */
		}
		else
		{
			/*
			 *	If we're not supposed to stop, then go ahead and 
			 *	find the target track. Report transitioning state
			 *	and perform a moderated track update.
			 */
			rsl->PendingTargetIndex = target;
			rsl->TargetChanged = 1;
			#ifdef _USE_TRANSIT_STATE
				MRSetState(MR_State_Transit);
			#endif
			moderatedTrackUpdate = 1;
			
			/*
			 *	To provide the appearance of responsiveness, ensure that the new
			 *	target index is reflected on the UPnP network. The number may not
			 *	be completely accurate (when compared to the final value), but it
			 *	will give the user an idea of what track they are requesting.
			 */
			MRSetTrack(PENDING_TARGET_URI, rsl->PendingTargetIndex);
		}

		/* unlock the rsl state */
		sem_post(&(rsl->Lock));


		/* always release the lock before executing another method that could fire a callback */
		if (moderatedTrackUpdate != 0)
		{
			_RSL_ModeratedTrackUpdate(rsl);
		}
	}
}

/*
 *	This method executes when the RSL wants to acquire a new trackURI because
 *	the mediaURI or the target index has changed.
 *
 *	The method will execute a callback to determine if the rendering framework
 *	can currently accept a track change. If so, then we execute the callback
 *	for finding a new target. 
 *
 *	Otherwise, we don't bother executing the callback... instead waiting,
 *	for the upper layer to notify RSL that rendering state has changed.
 *	When this happens, then we execute _RSL_OnCodecStateChange(), which
 *	will request a new targetURI if it's needed.
 *
 *	This method can execute an RSL callback, so all calls to this method should never
 *	have the rsl->Lock before making the call.
 */
void _RSL_ModeratedTrackUpdate(struct RendererStateLogic* rsl)
{
	unsigned char wrapAround = 0;
	unsigned char codecBusy = 0;
	unsigned char findTarget = 0;
	Callback_StateLogic_Query_CodecBusy queryCodec = NULL;

	DEBUGONLY(printf("_RSL_ModeratedTrackUpdate(): playMode=%d pendingTargetIndex=%d currentTrackIndex=%d targetChanged=%d\r\n", rsl->PlayMode, rsl->PendingTargetIndex, rsl->CurrentTrackIndex, rsl->TargetChanged);)
	ASSERT(rsl->Method_FindTargetUri != NULL);

	/* lock */
	sem_wait(&(rsl->Lock));

	/*
	 *	Given the current playmode, determine whether wraparound
	 *	behavior should be applied to the request to find a
	 *	target URI. Wraparound behavior means that if the index
	 *	is lesser or greater than the actual range of indices in 
	 *	the playlist, the chosen target index by the playlist parser will
	 *	actually be modulo-derived value - hence the wraparound effect.
	 *	Otherwise, the final chosen target index by the playlist parser
	 *	will be either the first or last index, if the requested index
	 *	was lesser or greater than the allowed range.
	 */

	switch (rsl->PlayMode)
	{
	case MR_PlayMode_Normal:
	case MR_PlayMode_RepeatOne:
		/* wrapAround initialized to zero */
		break;

	case MR_PlayMode_RepeatAll:
	case MR_PlayMode_Random:
	case MR_PlayMode_Shuffle:
		wrapAround = 1;
		break;

	default:
		ASSERT(rsl->PlayMode != rsl->PlayMode);
		break;
	}

	queryCodec = rsl->Method_QueryIsCodecBusy;
	
	
	/* unlock before calling queryCodec */
	sem_post(&(rsl->Lock));
	

	if (queryCodec != NULL)
	{
		/*
		 *	Check to see if the codec is busy. Do this
		 *	for rendering frameworks that are sensitive
		 *	to getting too many setup stream requests.
		 */
		codecBusy = queryCodec(rsl);
	}

	//TODO: check for playlist type before asking for target

	/*
	 *
	 *	If the codec is not busy and the target changed and we don't
	 *	already have a pending request, instruct the application
	 *	layer to find the target uri for the given mediaURI and target.
	 *
	 */

	DEBUGONLY(printf("_RSL_ModeratedTrackUpdate(cont'd): codecBusy=%d PendingRequest=%d targetChanged=%d\r\n", codecBusy, rsl->PendingRequest, rsl->TargetChanged););

	/*
	 *	Lock rsl state and determine if we need to 
	 *	request a new track URI. Then unlock afterwards.
	 */

	sem_wait(&(rsl->Lock));
	if (
		(codecBusy == 0) &&
		(rsl->TargetChanged != 0) &&
		(rsl->PendingRequest == 0)
		)
	{
		rsl->PendingRequest = 1;
		findTarget = 1;
	}
	sem_post(&(rsl->Lock));


	/* Execute the callback after we release the lock to prevent deadlocks. */
	if (findTarget != 0)
	{
		rsl->Method_FindTargetUri
			(
			rsl,
			rsl->MediaUri,
			rsl->PendingTargetIndex,
			wrapAround
			);
	}
}

/*
 *	This method executes when the RSL is notified that the codec/rendering state
 *	has changed. Entry point is actually on an RSL_OnCodecEvent_xxx method, which
 *	in turn, calls this method after reflecting the state change to the MicroMediaRenderer
 *	object.
 *
 *	This method basically checks to see if the current target index 
 *	doesn't match the pending target index. If so, then request
 *	a new target URI in a moderated fashion.
 *
 *	This method can execute an RSL callback, so all calls to this method should never
 *	have the rsl->Lock before making the call.
 */
void _RSL_OnCodecStateChange(struct RendererStateLogic* rsl)
{
	unsigned char moderatedTrackUpdate = 0;

	DEBUGONLY(printf("_RSL_OnCodecStateChange\r\n");)

	sem_wait(&(rsl->Lock));
	if (rsl->CurrentTrackIndex != rsl->PendingTargetIndex)
	{
		moderatedTrackUpdate = 1;
	}
	sem_post(&(rsl->Lock));

	/*
	 *	Always ensure that rsl->Lock is released before calling
	 *	a method that can execute an RSL callback.
	 */
	if (moderatedTrackUpdate != 0)
	{
		_RSL_ModeratedTrackUpdate(rsl);
	}
}

/*
 *	This method executes when the RSL has a new trackURI to apply.
 *
 *	The method will execute the callback, instructing the upper layer,
 *	to apply the URI to setup a new stream.
 *
 *	Originally, this method was called by more functions,
 *	but code-cleanup shows that only RSL_OnPlaylistLogicResult_FoundTargetUri()
 *	calls this method. Possible optimization may be to move this code into that method.
 */
void _RSL_SetNewTrackUri(struct RendererStateLogic* rsl, const char *trackUri)
{
	char *copy;
	int len;
	unsigned char executePlay = 0;

	DEBUGONLY(printf("_RSL_SetNewTrackUri (%s)\r\n", trackUri);)

	/* check for preconditions */
	ASSERT(rsl->Method_SetupStream != NULL);

	/* lock */
	sem_wait(&(rsl->Lock));

	/* ensure that the URI is not NULL */
	if (trackUri != NULL)
	{
		len = (int) strlen(trackUri) + 1;

		copy = (char*) RSL_MALLOC(len);
		memcpy(copy, trackUri, len);
	}
	else
	{
		copy = (char*) RSL_MALLOC(1);copy[0] = '\0';
	}

	/* report transit state because we're going to change the URI */
	#ifdef _USE_TRANSIT_STATE
		MRSetState(MR_State_Transit);
	#endif

	/* unlock before we execute callback */
	sem_post(&(rsl->Lock));

	/* execute callback to for new stream setup */
	rsl->Method_SetupStream(rsl, trackUri);

	/* lock to ensure atomic reads on rsl->DesiredState */
	sem_wait(&(rsl->Lock));

	/* if appropriate, execute callback to instruct rendering framework to play */
	if (
		(
		(rsl->DesiredState == MR_State_Playing) ||
		(rsl->DesiredState == MR_State_Paused)
		)
		&&
		(rsl->TransportErrorFlag == 0)
		)
	{
		executePlay = 1;
	}

	RSL_FREE(copy);

	/* unlock before executing callback */
	sem_post(&(rsl->Lock));

	if (executePlay != 0)
	{
		rsl->Method_Play(rsl, rsl->DesiredPlaySpeed);
	}
}

/* see header file */
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
	//Callback_StateLogic_Query_CodecBusy queryCodecPosition,
	Callback_StateLogic_Validate_MediaUri validateMediaUri
	)
{
	struct RendererStateLogic* rsl;
	/*pthread_mutexattr_t mutex_attr;*/

	DEBUGONLY(printf("CreateRendererStateLogic()\r\n");)
	ASSERT(methodFindTargetUri != NULL);
	ASSERT(methodSetupStream != NULL);
	ASSERT(methodPlay != NULL);
	ASSERT(queryCodecBusy != NULL);
	ASSERT(validateMediaUri != NULL);
    
	rsl = (struct RendererStateLogic *) RSL_MALLOC(sizeof (struct RendererStateLogic));
	memset(rsl, 0, sizeof (struct RendererStateLogic));

	rsl->MediaRenderer = MediaRenderer;
	
	rsl->PlayMode = MR_PlayMode_Normal;
	rsl->DesiredState = MR_State_Stopped;
	rsl->DesiredPlaySpeed = MR_PlaySpeed_Normal;

	rsl->Method_FindTargetUri = methodFindTargetUri;
	rsl->Method_SetupStream = methodSetupStream;
	rsl->Method_Play = methodPlay;
	rsl->Method_Stop = methodStop;
	rsl->Method_Pause = methodPause;
	rsl->Method_QueryIsCodecBusy = queryCodecBusy;
	//rsl->Query_CodecPosition = queryCodecPosition;
	rsl->Method_ValidateMediaUri = validateMediaUri;

	/* initialize recursive mutex */
	/*
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&(rsl->Lock), &mutex_attr);
	*/

	/* add to the chain */
	rsl->Destroy = RSL_DestroyRslObj;
	ILibAddToChain(Chain, rsl);

	/* initialize semaphore lock */
	sem_init(&(rsl->Lock), 0, 1);

	/* Setup Initial Values */
	MRSetVolume(MR_AudioChannel_Master,100);
	MRSetVolume(MR_AudioChannel_LF,100);
	MRSetVolume(MR_AudioChannel_RF,100);

	return rsl;
}

/* see header file */
void RSL_DestroyRslObj(void *rslObj)
{
	struct RendererStateLogic* rsl;
	
	DEBUGONLY(printf("RSL_DestroyRslObj()\r\n");)
	ASSERT(rslObj != NULL);

	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_destroy(&(rsl->Lock));*/
	sem_destroy(&(rsl->Lock));

	if (rsl->MediaUri != NULL) RSL_FREE (rsl->MediaUri);
	memset(rsl, 0, sizeof(struct RendererStateLogic));
}

/* see header file */
void RSL_DoNextPrevious(void *rslObj, int trackDelta)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_DoNextPrevious (trackDelta=%d)\r\n", trackDelta);)
	ASSERT (rslObj != NULL);

	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/
	rsl->TransportErrorFlag = 0;

	/*
	 *	Call the helper method that actually handles a track change
	 */
	_RSL_HandleTrackChange(rsl, 0, trackDelta, 0);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_DoSeekTrack(void *rslObj, int value)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_DoSeekTrack (int=%d)\r\n", value);)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	rsl = (struct RendererStateLogic*) rslObj;
	rsl->TransportErrorFlag = 0;
	_RSL_HandleTrackChange(rsl, 1, value, 0);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_DoStateChange(void* rslObj, enum MR_Enum_States desiredState, enum MR_Enum_PlaySpeeds playSpeed)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_DoStateChange (desiredState=%d)\r\n", desiredState);)
	rsl = (struct RendererStateLogic*) rslObj;
	ASSERT (rslObj != NULL);
	ASSERT (rsl->Method_Stop != NULL);
	ASSERT (rsl->Method_Pause != NULL);
	ASSERT (rsl->Method_Play != NULL);

	/*pthread_mutex_lock(&(rsl->Lock));*/
	rsl->TransportErrorFlag = 0;

	switch (desiredState)
	{
	case MR_State_Stopped:
		rsl->DesiredState = MR_State_Stopped;
		rsl->Method_Stop(rsl);
		break;
	
	case MR_State_Paused:
		rsl->DesiredState = MR_State_Paused;
		rsl->Method_Pause(rsl);
		break;
	
	case MR_State_Playing:
		rsl->DesiredState = MR_State_Playing;
		if (playSpeed != MR_PlaySpeed_Ignore)
		{
			rsl->DesiredPlaySpeed = playSpeed;
		}
		rsl->Method_Play(rsl, rsl->DesiredPlaySpeed);
		break;
	
	default:
		printf("Unexpected state change requested: %d\r\n", (int)desiredState);
		break;
	}

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Stopped(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Stopped\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	MRSetState(MR_State_Stopped);

	/*
	 *	Do not change the transport error 
	 *	value to OK, unless the error flag
	 *	is not set.
	 */
	if (rsl->TransportErrorFlag == 0)
	{
		MRSetStatus(MR_Status_OK);
	}

	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Ended(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Ended\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	_RSL_HandleTrackChange(rslObj, 0, 1, 1);
	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Error(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Error\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	rsl->TransportErrorFlag = 1;
	MRSetState(MR_State_Stopped);
	MRSetStatus(MR_Status_ERROR);
	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Transit(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Transit\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	#ifdef _USE_TRANSIT_STATE
		MRSetState(MR_State_Transit);
	#endif
	MRSetStatus(MR_Status_OK);
	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Playing(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Playing\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	MRSetState(MR_State_Playing);
	MRSetStatus(MR_Status_OK);
	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnCodecEvent_Paused(void *rslObj)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnCodecEvent_Paused\r\n");)
	ASSERT (rslObj != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	MRSetState(MR_State_Paused);
	MRSetStatus(MR_Status_OK);
	_RSL_OnCodecStateChange(rslObj);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
int RSL_OnPlaylistLogicResult_FoundTargetUri (void *rslObj, const char *mediaUri, int targetIndex, int wrapAround, int resultActualIndex, const char *resultTargetUri)
{
	struct RendererStateLogic* rsl;
	int applied = 0;

	DEBUGONLY(printf("RSL_OnPlaylistLogicResult_FoundTargetUri(%s, %d, %d, %d, %s)\r\n", mediaUri, targetIndex, wrapAround, resultActualIndex, resultTargetUri);)
	rsl = (struct RendererStateLogic*) rslObj;

	ASSERT (rslObj != NULL);
	ASSERT (mediaUri != NULL);
	ASSERT (rsl->MediaUri != NULL);
	
	/*pthread_mutex_lock(&(rsl->Lock));*/

	sem_wait(&(rsl->Lock));

	if (resultTargetUri == NULL)
	{
		printf("WARNING: RSL_OnPlaylistLogicResult_FoundTargetUri: resultTargetUri is NULL\r\n");
		/*applied = 0;*/ /*initialized to zero */
	}
	else
	{
		/*
		 *	Target was found. Set the track URI.
		 */
		if (
			(targetIndex == rsl->PendingTargetIndex) &&
			(strcmp(mediaUri, rsl->MediaUri) == 0)
			)
		{
			rsl->TargetChanged = 0;
		}

		/*
		 *	The app provides with the actual index, given that
		 *	the target index we provided in our request may have
		 *	been out-of-bounds of the playlist. Reflect
		 *	this information onto the UPnP network.
		 */
		rsl->CurrentTrackIndex = resultActualIndex;
		MRSetTrack(resultTargetUri, resultActualIndex);

		/* flag that this target should be applied */
		applied = 1;
	}
	rsl->PendingRequest = 0;
	
	/* unlock before calling any methods that might call an RSL callback */
	sem_post(&(rsl->Lock));


	if (applied != 0)
	{
		/*
		 *	The app provides us with the target URI, so
		 *	we'll appropriately reflect this information
		 *	to the UPnP network and instruct the
		 *	app layer to apply the URI to the codec/streaming
		 *	engine.
		 */
		_RSL_SetNewTrackUri(rsl, resultTargetUri);

		/*
		 *	request to change tracks is complete...
		 *	appropriately update the track again if needed
		 */
		_RSL_ModeratedTrackUpdate(rsl);
	}
	else
	{
		/* an error occurred, so stop everything */
		sem_wait(&(rsl->Lock));
		rsl->TransportErrorFlag = 1;
		rsl->TargetChanged = 0;
		rsl->PendingRequest = 0;
		rsl->DesiredState = MR_State_Stopped;

		rsl->TransportErrorFlag = 1;
		MRSetState(MR_State_Stopped);
		MRSetStatus(MR_Status_ERROR);
		sem_post(&(rsl->Lock));

		rsl->Method_Stop(rsl);
	}

	/*pthread_mutex_unlock(&(rsl->Lock));*/
	return applied;
}

/* see header file */
void RSL_OnPlaylistLogicResult_MediaUriExists (void *rslObj, enum MR_SupportedProtocolInfo protInfo, const char *mediaUri, const char *title, const char *creator, int uriExists)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnPlaylistLogicResult_MediaUriExists(%s, %d)\r\n", mediaUri, uriExists); )
	ASSERT (rslObj != NULL);
	ASSERT (title != NULL);
	ASSERT (creator != NULL);
	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	MRSetMediaInfo(protInfo, title, creator);

	if (uriExists != 0)
	{
		MRSetStatus(MR_Status_OK);
	}
	else
	{
		rsl->TransportErrorFlag = 1;
		MRSetStatus(MR_Status_ERROR);
	}

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_OnPlaylistLogicResult_SetTrackTotal(void *rslObj, const char *mediaUri, int trackTotal, int totalDuration)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_OnPlaylistLogicResult_SetTrackTotal(%s, %d, %d)\r\n", mediaUri, trackTotal, totalDuration);)
	ASSERT (rslObj != NULL);
	ASSERT (mediaUri != NULL);

	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	rsl->TotalTrackCount = trackTotal;
	MRSetMediaTotals(trackTotal, totalDuration, MR_COUNTER_UNKNOWN);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_CanSetPosition(void *rslObj, int canSetPosition)
{
	/*struct RendererStateLogic* rsl;*/
	DEBUGONLY(printf("RSL_CanSetPosition (canSetPosition=%d)\r\n", canSetPosition);)

	MrSetSeekTimePositionEnabled (canSetPosition);
}

/* see header file */
void RSL_SetMediaUri(void* rslObj, const char *mediaUri)
{
	struct RendererStateLogic* rsl;
	unsigned char valid = 1;
	unsigned char uriEmpty = 0;
	unsigned char moderatedTrackChange = 0;
	unsigned char setupBlankStream = 0;

	DEBUGONLY(printf("RSL_SetMediaUri (mediaUri=%s)\r\n", mediaUri);)
	ASSERT (rslObj != NULL);
	ASSERT (mediaUri != NULL);

	rsl = (struct RendererStateLogic*) rslObj;

	/* check for preconditions */
	ASSERT(rsl->Method_ValidateMediaUri != NULL);
	ASSERT(mediaUri != NULL);

	/*pthread_mutex_lock(&(rsl->Lock));*/
	rsl->TransportErrorFlag = 0;

	/*
	 *	Check for empty URI. 
	 *	Allow the application layer to validate the
	 *	media URI. 
	 */
	if ((mediaUri != NULL) && (mediaUri[0] == '\0'))
	{
		uriEmpty = 1;
		valid = 1;
	}
	else if (rsl->Method_ValidateMediaUri != NULL)
	{
		/* callback executed without acquiring rsl->Lock */
		valid = rsl->Method_ValidateMediaUri(rsl, mediaUri);
	}


	/* lock RSL */
	sem_wait(&(rsl->Lock));


	/*
	 *	Appropriately FREE and allocate memory for the uri.
	 *	Then copy the string.
	 */
	if (rsl->MediaUri != NULL) RSL_FREE(rsl->MediaUri);
	rsl->MediaUri = (char*) RSL_MALLOC(((int)strlen(mediaUri))+1);
	strcpy(rsl->MediaUri, mediaUri);
	

	/*
	 *	Report uri change & transitioning state to
	 *	the UPnP network.
	 */
	#ifdef _USE_TRANSIT_STATE
		MRSetState(MR_State_Transit);
	#endif
	MRSetMediaUri(rsl->MediaUri);


	/* at this point, this method has acquired rsl->Lock */
	if ((valid != 0) && (uriEmpty == 0))
	{
		/*
		 *	If the uri is valid, then set some flags.
		 */

		/* we now have a pending request */
		rsl->PendingTargetIndex = 1;

		/* target value is now dirty */
		rsl->TargetChanged = 1;

		/* we need to execute a callback to find the target */
		moderatedTrackChange = 1;
	}
	else
	{
		/*
		 *	Instruct rendering framework to
		 *	stop and clear its current stream.
		 *	Expect framework to eventually report a stopped state.
		 */
		#ifdef _USE_TRANSIT_STATE
			MRSetState(MR_State_Transit);
		#endif
		setupBlankStream = 1;

		if (valid == 0)
		{
			/* report error only if uri is not valid */
			rsl->TransportErrorFlag = 1;
			MRSetStatus(MR_Status_ERROR);
			MRSetMediaInfo(MR_PROTINFO_HTTP_UNKNOWN, INVALID_MEDIA_URI, "");
			MRSetTrackMetadata(MR_PROTINFO_HTTP_UNKNOWN, INVALID_MEDIA_URI, "");
		}
		else
		{
			MRSetMediaInfo(MR_PROTINFO_HTTP_UNKNOWN, NO_MEDIA_URI, "");
			MRSetTrackMetadata(MR_PROTINFO_HTTP_UNKNOWN, NO_MEDIA_URI, "");
		}

		/* clear current media and track info */
		rsl->CurrentTrackIndex = 0;
		rsl->TotalTrackCount = 0;

		/* tell UPnP network that this device has no active streams */
		MRSetMediaUri("");
		MRSetMediaTotals(0, 0, 0);
		MRSetTrack("", 0);
		MRSetTrackDurationInfo(0, 0);
		
		/*
		 *	Free the URI and set to  NULL so that 
		 *	subsequent next/previous requests are
		 *	ignored.
		 */
		RSL_FREE(rsl->MediaUri);
		rsl->MediaUri = NULL;
	}

	/* unlock before executing any callbacks or methods that execute RSL callbacks */
	sem_post(&(rsl->Lock));

	if (moderatedTrackChange != 0)
	{
		_RSL_ModeratedTrackUpdate(rsl);
	}
	else if (setupBlankStream != 0)
	{
		rsl->Method_SetupStream(rsl, "");
	}

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/*
 *	DEPRECATED: Architecturally does not belong here because RendererStateLogic should
 *	not handle RenderingControl related tasks.
 */
//void RSL_SetMute(void* rslObj, enum MR_Enum_AudioChannels channel, unsigned short value)
//{
//	struct RendererStateLogic* rsl;
//
//	DEBUGONLY(printf("RSL_SetMute (channel=%d, muteFlag=%d)\r\n", channel, value);)
//	ASSERT (rslObj != NULL);
//	
//	rsl = (struct RendererStateLogic*) rslObj;
//
//	/*
//	 *	Report the volume change on the UPnP network.
//	 */
//	MRSetMute(channel, value);
//}

/* see header file */
void RSL_SetPlayMode(void* rslObj, enum MR_Enum_PlayModes playMode)
{
	struct RendererStateLogic* rsl;

	DEBUGONLY(printf("RSL_SetPlayMode (%d)\r\n", playMode);)
	ASSERT (rslObj != NULL);

	rsl = (struct RendererStateLogic*) rslObj;

	/*pthread_mutex_lock(&(rsl->Lock));*/

	sem_wait(&(rsl->Lock));
	rsl->PlayMode = playMode;
	sem_post(&(rsl->Lock));

	MRSetPlayMode(playMode);

	/*pthread_mutex_unlock(&(rsl->Lock));*/
}

/* see header file */
void RSL_SetTrackDuration(void *rslObj, int duration)
{
	DEBUGONLY(printf("RSL_SetTrackDuration(%d)\r\n", duration);)
	ASSERT (rslObj != NULL);

	MRSetTrackDurationInfo(duration, MR_COUNTER_UNKNOWN);
}

/* see header file */
void RSL_SetTrackMetadata(void *rslObj, enum MR_SupportedProtocolInfo protInfo, const char *title, const char *artist)
{
	DEBUGONLY(printf("RSL_SetTrackMetadata(%s, %s)\r\n", title, artist);)
	ASSERT (rslObj != NULL);
	ASSERT(title != NULL);
	ASSERT(artist != NULL);

	MRSetTrackMetadata(protInfo, title, artist);
}

/*
 *	DEPRECATED: Architecturally does not belong here because RendererStateLogic should
 *	not handle RenderingControl related tasks.
 */
//void RSL_SetVolume(void* rslObj, enum MR_Enum_AudioChannels channel, unsigned short value)
//{
//	struct RendererStateLogic* rsl;
//
//	DEBUGONLY(printf("RSL_SetVolume (channel=%d, volumeLevel=%d)\r\n", channel, value);)
//	ASSERT (rslObj != NULL);
//
//	rsl = (struct RendererStateLogic*) rslObj;
//	
//	/*
//	 *	Report the volume change on the UPnP network.
//	 */
//	MRSetVolume(channel, value);
//}

