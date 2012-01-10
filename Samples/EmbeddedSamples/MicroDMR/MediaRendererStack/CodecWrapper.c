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
#include "CodecWrapper.h"

#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef _POSIX
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#endif

#ifdef WIN32
/*
 *	These #define preprocessors provide POSIX APIs for doing
 *	semaphore and thread operations in WIN32.
 */
#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define pthread_t HANDLE
#define pthread_create(a,b,c,d) *a = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)c, d, 0, NULL);
#define pthread_detach(a) 
#endif

#include <string.h>
#include "Utility.h"
#include "PortFn_MsTime.h"

#ifdef _DEBUG_CODECWRAPPER
#define DEBUG_CODECWRAPPER_ONLY(x) x
#endif

#ifndef _DEBUG_CODECWRAPPER
#define DEBUG_CODECWRAPPER_ONLY(x) 
#endif


/*
 *	This counter tracks memory allocations used for
 *	the codec wrapper.
 */
int g_codecwrapper_malloc_counter  = 0;

/*
 *	Codec wrapper malloc.
 */
void* CODEC_WRAPPER_MALLOC(int size)
{
	g_codecwrapper_malloc_counter++;
	#ifdef TRACK_MALLOC_VERBOSE
		printf("g_codecwrapper_malloc_counter=%d\r\n", g_codecwrapper_malloc_counter);
	#endif
	//ToDo: Memory Appears to be leaking here
	return malloc(size); 
}

/*
 *	Codec wrapper free.
 */
void CODEC_WRAPPER_FREE(void *freeThis)
{
	g_codecwrapper_malloc_counter--;
	#ifdef TRACK_MALLOC_VERBOSE
		printf("g_codecwrapper_malloc_counter=%d\r\n", g_codecwrapper_malloc_counter);
	#endif
	free(freeThis); 
}

/*
 *	Codec wrapper string copy.
 */
char* CODEC_WRAPPER_SAFE_STRING_COPY(char* storeHere, const char* copyThis)
{
	if (storeHere == NULL) g_codecwrapper_malloc_counter++;
	#ifdef TRACK_MALLOC_VERBOSE
		printf("g_codecwrapper_malloc_counter=%d\r\n", g_codecwrapper_malloc_counter);
	#endif
	//ToDo: Memory Appear to be leaking here
	return SafeStringCopy(storeHere, copyThis);
}

/*
 *	Codec wrapper free that will reassign the 
 *	provided pointer (after dereferencing) to
 *	a NULL value.
 */
void CODEC_WRAPPER_SAFE_FREE(void** freeThis)
{
	if (*freeThis != NULL) g_codecwrapper_malloc_counter--; 
	#ifdef TRACK_MALLOC_VERBOSE
		printf("g_codecwrapper_malloc_counter=%d\r\n", g_codecwrapper_malloc_counter);
	#endif
	SafeFree(freeThis); 
}

/*
 *	This structure tracks the state of a stream.
 */
struct StreamInstance
{
	/*
	 *	User-specified void*. This value represents the
	 *	void* that the caller wants returned in 
	 *	codecwrapper callbacks.
	 */
	void*	Input_StreamTag;
	
	/*
	 *	If this value is nonzero, then the codec wrapper 
	 *	needs to start a new stream.
	 */
	int		Input_ResetFlag;

	/*
	 *	Method callback when the rendering framework changes state.
	 */
	Callback_CodecWrapper_StateChange		Input_StateChangeCallback;

	/*
	 *	Method callback when the rendering framework determines the
	 *	duration of the track.
	 */
	Callback_CodecWrapper_DurationChange	Input_DurationChangeCallback;

	/*
	 *	Method callback when the rendering framework determines the
	 *	duration of the track.
	 */
	Callback_CodecWrapper_CanSetPosition	Input_CanSetPositionCallback;


	/*
	 *	DEPRECATED
	Callback_CodecWrapper_PositionChange	Input_PositionChangeCallback;
	 */

	/*
	 *	This is the URI specified by the caller.
	 */
	char*	Input_TrackUri;

	/*
	 *	Semaphore to synchronize the input (executed on caller threads) and
	 *	the codec wrapper thread, which performs the tasks for emulating
	 *	a rendering operation.
	 */
	sem_t	LockInput;


	/*
	 *	Index into codewrapper_Streams.
	 *	If the value is -1, it means the stream's thread
	 *	should terminate.
	 *	If the value is -2, then it means the stream has ended.
	 */
	int Index;

	/*
	 *	streamTag that the caller provided in CodecWrapper_SetupStream().
	 *	Copied from the Input_StreamTag.
	 */
	void* StreamTag;

	/*
	 *	The current byte position in the stream decoding.
	 */
	long BytePosition;

	/*
	 *	The current time position in the stream decoding in seconds.
	 */
	long TimePosition;

	/*
	 *	The total number of bytes in the stream.
	 */
	long TotalByteLength;

	/*
	 *	The total time duration of the stream in seconds.
	 */
	long TotalTimeLength;

	/*
	 *	Current volume setting, between 0 and 100.
	 */
	long Volume;

	/*
	 *	Current URI for playback. Copied from Input_TrackUri.
	 */
	char* Uri;

	/*
	 *	Current play state - set only on the thread that executes
	 *	in DecodeStreamLoop().
	 */
	enum CodecWrapperState PlayState;

	/*
	 *	Target play state - value is set on caller threads
	 *	as well as codec/rendering thread 
	 *	(which executes in DecodeStreamLoop method). 
	 *	Thread safety achieved through LockInput.
	 */
	enum CodecWrapperState TargetPlayState;

	/*
	 *	This semaphore blocks when there is no media, state is stopped or paused.
	 *	Effectively provides "idle" behavior for rendering.
	 */
	sem_t BlockLock;

	/*
	 *	The thread that is performing the work for rendering the stream.
	 *	This thread executes in the DecodeStreamLoop method.
	 */
	pthread_t Thread;

	/*
	 *	Miscellaneous pointer for use in callbacks. 
	 *	Not used now, but could be used later when
	 *	a rendering framework returns data in a struct
	 *	from a callback.
	 */
	void* CodecStuff;

	/*
	 *	Execute this callback when the state changes.
	 */
	Callback_CodecWrapper_StateChange StateChangeCallback;

	/*
	 *	Execute this callback when the track's duration information
	 *	is updated. (Sometimes, rendering frameworks need
	 *	an asynchronous way to report the duration of the track.)
	 */
	Callback_CodecWrapper_DurationChange DurationChangeCallback;

	/*
	 *	Average bytes per second - used for estimating time.
	 */
	int BytesPerSecond;

	/*
	 *	Nonzero value indicates that the rendering should be muted.
	 */
	int MuteFlag;
};

/*
 *	Maximum number of simultaneous streams.
 *	Generally speaking, most rendering frameworks only
 *	allow one rendering stream. However this implementation
 *	it can support multiple simultaneous streams
 *	when calling CodecWrapper_Init() the first time.
 */
int MAX_MEDIA_STREAMS;


/*
 *	Dynamic allocation of streams that can be handled by the wrapper.
 *	Created in CodecWrapper_Init().
 */
struct StreamInstance *codecwrapper_Streams;

/*
 *	This method changes the rendering framework's reported byte position.
 */
void SetDecodeBytePosition(struct StreamInstance* si, long bytePos)
{
	/*
	 *	DEPRECATED: (see notes below)
	 *
	Callback_CodecWrapper_PositionChange posCallback;
	 *	END OF DEPRECATION */

	/* position can be changed from multipel threads */
	sem_wait(&(si->LockInput));

	/*
 	 *	When using the callback, we want to assign it to a 
	 *	another pointer for use in case another thread
	 *	assigns the pointer to NULL or changes it.
	 *	We really don't care if this happens, as we can recover
	 * 	on the next iteration.
	 */
	si->BytePosition = bytePos;
	if (si->BytesPerSecond != 0)
	{
		si->TimePosition = si->BytePosition / si->BytesPerSecond;
	}
	else
	{
		si->TimePosition = 0;
	}

	/*	DEPRECATED:
	 *	Originally, the codec wrapper
	 *	continuously reported its position through
	 *	a callback. Determined that it was easier
	 *	to follow UPnP-AV model by letting the
	 *	caller query the position.
	 *
	 *	That being said, there's no harm in enabling
	 *	such a feature.
	 *
	posCallback = si->PositionChangeCallback;
	if (posCallback != NULL)
	{
		if (si->StreamTag != &g_PENDING_STREAM_TAG)
		{
			// event everything, and send the internal state for kicks //
			posCallback(si->BytePosition, si->TimePosition, si->TotalTimeLength, si, si->StreamTag);
		}
	}
	 *	END OF DEPRECATION */

	sem_post(&(si->LockInput));
}

/*
 *	This method changes the rendering framework's reported playstate.
 */
void SetDecodePlayState(struct StreamInstance* si, enum CodecWrapperState newState)
{
	/*  Apply new state. */
	si->PlayState = newState;

	if (si->StateChangeCallback != NULL)
	{
		/* event state, and send the internal state for kicks */
		si->StateChangeCallback(si->PlayState, si, si->StreamTag);
	}
}

/*
 *	The codecwrapper/rendering thread sometimes enters a blocked state
 *	to simulate idle play states, such stop, no media, or pause.
 *	This method unblocks the thread so that it can respond to
 *	a caller's request.
 */
void PulseCodecWrapperThread(struct StreamInstance *si)
{
	sem_post(&(si->BlockLock));
}

/* see header file */
int CodecWrapper_SetupStream(int streamIndex, const char* URI, void* streamTag, Callback_CodecWrapper_StateChange playstateChangeCallback, Callback_CodecWrapper_DurationChange durationChangeCallback, Callback_CodecWrapper_CanSetPosition canSetPositionCallback)
{
	struct StreamInstance *si = NULL;
	int retVal = -1;

	/*
	 *	Find the appropriate stream.
	 */
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		/*
		 *	Lock the stream instance so that we can copy 
		 *	the method arguments into the input parameters
		 *	on the stream's struct.
		 */
		sem_wait(&(si->LockInput));
		si->Input_ResetFlag = 1;
		si->Input_StreamTag = streamTag;
		si->Input_TrackUri = CODEC_WRAPPER_SAFE_STRING_COPY(NULL, URI);
		si->Input_StateChangeCallback = playstateChangeCallback;
		si->Input_DurationChangeCallback = durationChangeCallback;
		si->Input_CanSetPositionCallback = canSetPositionCallback;
		sem_post(&(si->LockInput));
		
		/* ensure that the codec wrapper thread is executing */
		PulseCodecWrapperThread(si);
		retVal = 0;
	}

	return retVal;
}

/*
 *	This method can execute on caller-threads or on the codecwrapper
 *	thread that executes in DecodeStreamLoop.
 *	This method sets the desired state for the rendering framework
 *	to the specified state. 
 */
int SetDesiredState(int streamIndex, enum CodecWrapperState state)
{
	int retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		/*
		 *	Lock the stream instance so that we can copy 
		 *	the desired state to the stream's struct.
		 */
		sem_wait(&(si->LockInput));
		si->TargetPlayState = state;
		if ((si->Uri == NULL) || (si->Uri[0] == '\0'))
		{
			si->TargetPlayState = MEDIA_UNINITIALIZED;
		}
		sem_post(&(si->LockInput));

		PulseCodecWrapperThread(si);
		retVal = 0;
	}

	return retVal;
}

/*	Instructs rendering framework to play a specified stream. */
int CodecWrapper_Play(int streamIndex, enum CodecWrapperPlaySpeed playSpeed)
{
	//TODO: enable trick modes with playSpeed
	return SetDesiredState(streamIndex, MEDIA_PLAYING);
}

/*	Instructs rendering framework to stop a specified stream. */
int CodecWrapper_Stop(int streamIndex)
{
	return SetDesiredState(streamIndex, MEDIA_STOPPED);
}

/*	Instructs rendering framework to pause a specified stream. */
int CodecWrapper_Pause(int streamIndex)
{
	return SetDesiredState(streamIndex, MEDIA_PAUSED);
}

/*	Returns the current playstate for a given stream. */
enum CodecWrapperState CodecWrapper_GetPlayState(int streamIndex)
{
	enum CodecWrapperState retVal = MEDIA_UNKNOWN;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->PlayState;
	}

	return retVal;
}

/*	Sets the desired playback position (seek operation) for a given stream. */
int CodecWrapper_SetPosition (int streamIndex, int timePosition)
{
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		SetDecodeBytePosition(si, si->BytesPerSecond * timePosition);
	}
	return 0;
}

/*	Returns the current playback time position for a given stream. */
long CodecWrapper_GetTimePosition(int streamIndex)
{
	long retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->TimePosition;
	}

	return retVal;
}

/*	Returns the total time duration for a given stream. */
long CodecWrapper_GetTimeTotalLength(int streamIndex)
{
	long retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->TotalTimeLength;
	}

	return retVal;
}

/*	Returns the current playback position, in terms of bytes, for a given stream. -1 means unknown. */
long CodecWrapper_GetBytePosition(int streamIndex)
{
	long retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->BytePosition;
	}

	return retVal;
}

/*	Returns the total byte length of a given for a given stream. -1 means unknown. */
long CodecWrapper_GetByteTotalLength(int streamIndex)
{
	long retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->TotalByteLength;
	}

	return retVal;
}

/*	Gets the volume level of the specified stream. */
int  CodecWrapper_GetVolume(int streamIndex)
{
	int retVal = -1;
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		retVal = si->Volume;
	}

	return retVal;
}

/*	Sets the volume for a specified stream. */
int CodecWrapper_SetVolume(int streamIndex, int level)
{
	struct StreamInstance *si = NULL;
	
	si = &(codecwrapper_Streams[streamIndex]);

	if (si != NULL)
	{
		si->Volume = level;
	}

	return 0;
}

/*
 *	This is where all of the interesting work occurs.
 *	This method provides the execution loop for simulated
 *	rendering.
 */
void* DecodeStreamLoop(void* streamInstance)
{
	struct StreamInstance *si;
	int uriChanged = 0;
	long newPos;
	enum CodecWrapperState oldState;
	int uriEmpty =0;
	
	int intervalUS = 100000/*100000*/;			/* interval in microseconds */
	int intervalMS = intervalUS / 1000; /* interval in milliseconds */
	int intervalsPerSecond = 1000 / intervalMS;	/* # intervals per second */
	int bytesPerSecond = 17000;	/* process this many bytes every second; */
	int bytesPerInterval = bytesPerSecond / intervalsPerSecond;

	si = (struct StreamInstance*) streamInstance;
	si->BytesPerSecond = bytesPerSecond;

	while (si->Index >= 0)
	{
		if (
			(si->PlayState == MEDIA_UNINITIALIZED) ||
			(si->PlayState == MEDIA_STOPPED) ||
			(si->PlayState == MEDIA_PAUSED) ||
			(si->PlayState == MEDIA_ENDED)
			)
		{
			/*
			 *	Block until signaled, only when we're
			 *	in a state that doesn't require decoding
			 *	of a stream.
			 */
			sem_wait(&(si->BlockLock));
		}

		sem_wait(&(si->LockInput));

		/*
		 *	Determine if the input values have been set.
		 *	If so, reset values in the state.
		 */
		uriChanged =0;
		if (si->Input_ResetFlag != 0)
		{
			/* transfer the URI to the official state */
			CODEC_WRAPPER_SAFE_FREE((void**) &(si->Uri));
			si->Uri = si->Input_TrackUri;
			si->Input_TrackUri = NULL;
			si->StateChangeCallback = si->Input_StateChangeCallback;
			si->DurationChangeCallback = si->Input_DurationChangeCallback;
			si->StreamTag = si->Input_StreamTag;
			si->Input_ResetFlag = 0;
			uriChanged = 1;
		}

		sem_post(&(si->LockInput));
		

		if (uriChanged != 0)
		{
			/*
			 *	If the uri changed, then it we have to reset the state
			 *	of the codec. This part of the loop executes when
			 *	the caller has changed streams.
			 */
			si->CodecStuff = NULL;
			si->BytePosition = 0;
			si->TimePosition = 0;

			/*
			 *	The total byte length really determines the total time length
			 *	in this implementation - real rendering framework can
			 *	behave differently.
			 */
			oldState = si->TargetPlayState;
			if ((si->Uri == NULL) || (si->Uri[0] == '\0'))
			{
				si->TotalByteLength = 0;
				si->TotalTimeLength = 0;

				uriEmpty = 1;
				si->TargetPlayState = MEDIA_UNINITIALIZED;
			}
			else
			{
				//si->TotalByteLength = bytesPerSecond * (10 + (1 + (int) (5.0 * rand() / (RAND_MAX+1.0))));	 /*fake a 10+ second song*/
				si->TotalByteLength = bytesPerSecond * 60 * (1 + (int) (5.0 * rand() / (RAND_MAX+1.0)));	/* frake a 1-5 minute song */
				si->TotalTimeLength = (int) (si->TotalByteLength / bytesPerSecond);

				uriEmpty = 0;
				si->TargetPlayState = MEDIA_TRANSIT;
			}

			si->DurationChangeCallback(si->TotalTimeLength, si, si->StreamTag);
		
			/* set current position */
			SetDecodeBytePosition(si, 0);

			/* event that we're in transit or uninitialized */
			SetDecodePlayState(si, si->TargetPlayState);

			/*
			 *	Do something depending on what playstate 
			 *	we're transitioning from
			 */

			switch (oldState)
			{
			case MEDIA_UNINITIALIZED:
			case MEDIA_STOPPED:
				si->TargetPlayState = MEDIA_STOPPED;
				SetDecodePlayState(si, si->TargetPlayState);
				break;

			case MEDIA_ENDED:
			case MEDIA_ERROR:
			case MEDIA_PAUSED:
			case MEDIA_PLAYING:
				if (uriEmpty == 0)
				{
					si->TargetPlayState = MEDIA_PLAYING;
					SetDecodePlayState(si, si->TargetPlayState);
				}
				break;

			case MEDIA_TRANSIT:
				/* do nothing for transit state - shouldn't be executing this*/
				fprintf(stderr, "DecodeStreamLoop() - no handler for MEDIA_TRANSIT\r\n");
				break;
			case MEDIA_UNKNOWN:
				/* do nothing for unknown state - shouldn't be executing this*/
				fprintf(stderr, "DecodeStreamLoop() - no handler for MEDIA_UNKNOWN\r\n");
				break;
			}

			uriChanged = 0;
		}
		else
		{
			/*
			 *	The URI has not changed - so this section executes
			 *	when the caller hasn't done anything. This part
			 *	does most of the simulated rendering work.
			 */

			/* remember old state for debugging */
			oldState = si->PlayState;

			if (si->PlayState != si->TargetPlayState)
			{
				SetDecodePlayState(si, si->TargetPlayState);

				if ((si->PlayState == MEDIA_STOPPED) || (oldState == MEDIA_ENDED))
				{
					SetDecodeBytePosition(si, 0);
				}
			}

			/* the uri hasn't changed, so simulate a playback timeslice. */
			if (si->PlayState == MEDIA_PLAYING)
			{
			
				/*
				 *	Simulate normal playback by incrementing
				 *	byteposition, eventing position info,
				 *	and then blocking for a short bit so
				 *	as to not eat up all the CPU.
				 */
				newPos = si->BytePosition + bytesPerInterval;

				if (newPos > si->TotalByteLength)
				{
					newPos = si->TotalByteLength;
				}

				/*
				 *	Increment byte position, calculate time based on 
				 *	the byte position and the amount of time we
				 *	block when simulating the decode process.
				 */
				SetDecodeBytePosition(si, newPos);

				if (newPos == si->TotalByteLength)
				{	
					/*
					 *	Reached the end of the track,
					 *	so immediately event the state
					 *	change. Keep it locked even through
					 *	the callback to ensure that
					 *	another thread won't over
					 *
					 *	Normally, good codec implementations 
					 *	would not lock and execute a callback.
					 */
					si->TargetPlayState = MEDIA_ENDED;
					si->BytePosition = 0;
				}
				else
				{
					/*
					 *	Block for a short period of time 
					 *	to simulate rendering work.
					 */
					//DEBUG_CODECWRAPPER_ONLY(printf("CodecWrapper:DecodeStreamLoop() - Before SleepMsTime\r\n");)
					DEBUG_CODECWRAPPER_ONLY(printf(".\r\n");)
					SleepMsTime(intervalMS);
					DEBUG_CODECWRAPPER_ONLY(printf(":\r\n");)
					//DEBUG_CODECWRAPPER_ONLY(printf("CodecWrapper:DecodeStreamLoop() - After SleepMsTime\r\n");)
				}
			}
		}
	}

	si->Index = -2;
	return NULL;
}

/*
 *	Initializes the codecwrapper library to handle the specified number of streams.
 */
void CodecWrapper_Init(int maxStreams)
{
	int i;
	struct StreamInstance *si;
	
	MAX_MEDIA_STREAMS = maxStreams;
	codecwrapper_Streams = (struct StreamInstance*) CODEC_WRAPPER_MALLOC(maxStreams * sizeof(struct StreamInstance));

	/* 
	 *	Initialize and reserve resources that could
	 *	be used for playback of streams.
	 */

	for (i=0; i < MAX_MEDIA_STREAMS; i++)
	{
		si = &(codecwrapper_Streams[i]);

		si->Input_ResetFlag = 0;
		si->Input_StreamTag = NULL;
		si->Input_TrackUri = NULL;

		si->Index = i;
		si->StreamTag = NULL;
		si->BytePosition = 0;
		si->TimePosition = 0;
		si->TotalByteLength = 0;
		si->TotalTimeLength = 0;
		si->Volume = 100;
		si->PlayState = MEDIA_UNINITIALIZED;
		si->TargetPlayState = MEDIA_UNINITIALIZED;
		si->CodecStuff = NULL;
		si->StateChangeCallback = NULL;
		si->DurationChangeCallback = NULL;
		si->Input_CanSetPositionCallback = NULL;
		/*DEPRECATED: si->PositionChangeCallback = NULL;*/
		si->Uri = NULL;
		si->MuteFlag = 0;
		sem_init(&(si->BlockLock), 0, 1);
		sem_init(&(si->LockInput), 0, 1);
		//pthread_mutex_init(&(si->LockTargetState), NULL);

		pthread_create(&(si->Thread), NULL, DecodeStreamLoop, si);
		pthread_detach(si->Thread);
	}
}

/*
 *	Stops the codecwrapper thread and instructs appropriately
 *	deallocates stuff.
 */
void CodecWrapper_UnInit()
{
	int i;
	struct StreamInstance *si;
	int numThreadsRunning = MAX_MEDIA_STREAMS;

	/*
	 *	Instruct all decode threads to stop 
	 *	by setting the index==-1, be
	 *	sure to wake each thread in case it's asleep.
	 */

	for (i=0; i < MAX_MEDIA_STREAMS; i++)
	{
		si = &(codecwrapper_Streams[i]);
		si->Index = -1;
		PulseCodecWrapperThread(si);
	}

	/* spin here until all threads are done */

	while (numThreadsRunning > 0)
	{
		/* block for a short period of time */
		SleepMsTime(100);

		/*
		 *	Assume all threads are running,
		 *	and iterate through them all,
		 *	decrementing the count as we go.
		 */
		numThreadsRunning = MAX_MEDIA_STREAMS;
		for (i=0; i < MAX_MEDIA_STREAMS; i++)
		{
			si = &(codecwrapper_Streams[i]);
			if (si->Index == -2)
			{
				numThreadsRunning--;
			}
		}
	}

	/* all threads are done, finish unitializing stream instances */

	for (i=0; i < MAX_MEDIA_STREAMS; i++)
	{
		si = &(codecwrapper_Streams[i]);
		CODEC_WRAPPER_SAFE_FREE((void**) &(si->Input_TrackUri));
		CODEC_WRAPPER_SAFE_FREE((void**) &(si->Uri));
		si->StreamTag = NULL;
		si->CodecStuff = NULL;
		si->StateChangeCallback = NULL;
		si->DurationChangeCallback = NULL;
		/*DEPRECATED si->PositionChangeCallback = NULL;*/
		sem_destroy(&(si->BlockLock));
		sem_destroy(&(si->LockInput));
		//pthread_mutex_destroy(&(si->LockTargetState));
#ifdef WIN32
		si->Thread = NULL;
#endif
	}

	CODEC_WRAPPER_FREE(codecwrapper_Streams);
}

/*
 *	Returns the mute value for the specified stream.
 */
int CodecWrapper_GetMute(int streamIndex)
{
	struct StreamInstance *si;
	si = &(codecwrapper_Streams[streamIndex]);
	return si->MuteFlag;
}

/*
 *	Sets tne mute value for the specified stream.
 */
int CodecWrapper_SetMute(int streamIndex, int muteFlag)
{
	struct StreamInstance *si;
	si = &(codecwrapper_Streams[streamIndex]);
	si->MuteFlag = muteFlag;
	return 0;
}
