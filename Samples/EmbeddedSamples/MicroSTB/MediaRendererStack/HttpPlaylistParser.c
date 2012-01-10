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

#define _PLAYLIST_TRACE

#ifndef _DEBUG
#define _DEBUG
#endif

#ifdef WIN32
	#define _CRTDBG_MAP_ALLOC
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32_WCE
	#include <assert.h>
#endif

#ifdef _WIN32_WCE
	#define strncasecmp _strnicmp
	#define strcasecmp _stricmp

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#elif WIN32
	#include <crtdbg.h>
	#define strncasecmp strnicmp
	#define strcasecmp stricmp
	#define ASSERT assert

	#define sem_t HANDLE
	#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#endif

#ifdef _POSIX
	#define ASSERT assert
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <semaphore.h>
#endif

#include "HttpPlaylistParser.h"
#include "ILibParsers.h"
#include "ILibWebClient.h"
#include "Utility.h"


#ifdef _DEBUG
	#define DEBUGONLY(x) x

	#define HPP_MALLOC	MALLOC
	#define HPP_FREE	FREE

	int hpp_malloc_counter = 0;
	void* hpp_malloc (int size)
	{
		++hpp_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("hpp_malloc_counter=%d\r\n", hpp_malloc_counter);
		#endif
		return MALLOC(size);
	}

	void hpp_free (void *ptr)
	{
		--hpp_malloc_counter;
		#ifdef TRACK_MALLOC_VERBOSE
			printf("hpp_malloc_counter=%d\r\n", hpp_malloc_counter);
		#endif
		FREE(ptr);
	}
#endif

#ifndef _DEBUG
	#define DEBUGONLY(x) 

	#define HPP_MALLOC	MALLOC
	#define HPP_FREE	FREE
#endif

#ifdef _TEMPDEBUG
	#define TEMPDEBUGONLY(x) x
#endif

#ifndef _TEMPDEBUG
	#define TEMPDEBUGONLY(x)
#endif

/*
 *	Indicates the data type that is used to key by the track index; 
 *	could be int, ushort, uint, depending on allowed memory.
 */
#define PLAYLIST_INDEXER_TYPE int

/*
 *	Indicates the number of indices in the hashtable.
 *
 *	For a hashtable of 2520 elements, we can provide map the positions of playlist items
 *	in playlists with up to 7129 items. If hashtable size is 2520, then it affords us
 *	9 means we can make 9 passes before we have to reset the passes to the (passes+1)%hashtableSize.
 */
#define PLAYLIST_HASHTABLE_SIZE 5
#define PLAYLIST_POSITION_TYPE int

#define FILE_EXTENSION_ExtendedM3U	".m3u"
#define CONTENT_TYPE_ExtendedM3U	"audio/mpegurl"

struct PlaylistState;
void IssueRequest(struct PlaylistState* ps, int attempts, int maxAttempts, int expectedRangeTrack, int rangeStart, int rangeEnd, void *callback);
void HttpRequestCleanup(void *sender, void *user1, void *user2);

struct HttpRequestPlaylist
{
	unsigned int Flags;

	/* The number of iterations done in callback */
	int Passes;

	/* the current state of the playlist */
	struct PlaylistState *State;

	/*
	 *	If the playlist has not been changed, then dereferencing
	 *	this field should give State->PlaylistUri.
	 */
	char** PlaylistUriAddress;

	/*
	 *	Last known item index; zero indicates no track are known.
	 */
	int LastTrackIndex;

	/*
	 *	If the HTTP response is ranged, then set
	 *	LastTrackIndex to this value on the first
	 *	pass. If playlist item 'x' begins at byte position 'y',
	 *	then this field will be 'x-1' if we did
	 *	a range request beginning with 'y'.
	 */
	int RangeExpectedTrackIndexMinusOne;


	/*
	 *	The number of times this request has been made.
	 */
	int Attempts;

	/*
	 *	Indicates the # of requests at time of issue.
	 */
	int RequestNumber;

	/*
	 *	The number of allowed attempts this request is allowed.
	 */
	int AllowedAttempts;


	PLAYLIST_POSITION_TYPE RangeStart;
	PLAYLIST_POSITION_TYPE RangeEnd;

	/*
	 *	Number of bytes processed for the media 
	 */
	int BytesProcessed;

	#ifdef _SAVE_PLAYLIST_BYTES
		/* actually save playlist bytes for debugging */
		char Bytes[PLAYLIST_BYTES_SIZE];
	#endif

	#ifdef _M3U_BUFFER
		/*
		 *	These fields only apply if (Flags & PP_IsLtmCallback) is nonzero.
		 */
	#endif
};

struct PlaylistIndexer
{
	int Passes;
	int HashIndex;
	int HopSize;
	int NumSkipped;
	PLAYLIST_INDEXER_TYPE TrackIndices[PLAYLIST_HASHTABLE_SIZE];
	PLAYLIST_POSITION_TYPE Positions[PLAYLIST_HASHTABLE_SIZE];
	int FirstHashIndex;
};

#ifdef _M3U_BUFFER
#define NUM_CACHED 15 /* should be odd. Digital Home wants at least 15. */
#define HALF_CACHED 7 /* half of NUM_CACHED, round down */
struct M3U_Cache
{
	int NumClean;
	int FirstTrackIndex;
	//int LastTrackIndex;
	int TrackDuration[NUM_CACHED];
	char* TrackUri[NUM_CACHED];
	char* TrackMetadata[NUM_CACHED];
	void* LifeTimeMonitor;
};
#endif

struct PlaylistState
{
	/*
	 *	PreSelect, PostSelect, and Destroy must be the first 3 fields and in this order.
	 *	This is a requirement of any module that is part of thread chain.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	/* This is the object that allows us to make HTTP requests */
	void* HttpClient; 
	
	unsigned int Flags;

	int		InputIndex;
	int		AdjustedInputIndex;
	int		InputWrapAround;
	void	*InputTag;

	HttpPlaylistParser_Callback_PlaylistUriExists CallbackUriExists;
	HttpPlaylistParser_Callback_OnUpdate_ItemCount CallbackItemCount;
	HttpPlaylistParser_Callback_OnResult_FindTargetUri CallbackFoundTarget;

	char	*PlaylistUri;
	int		ItemCount;
	int		RequestNumber;

	struct HttpRequestPlaylist* LastRequest;

	struct PlaylistIndexer Indexer;

	/*
	 *	Thread synchronization object.
	 *	We only have 5 entry points into this module:
	 *		HttpPlaylistParser_FindTargetUri() - called externally
	 *		Sink_HttpRequestProcessor - callback for microstack
	 *		Destroy_Parser_ExtendedM3U - callback when destroying object
	 *		HPP_LTM - callback for executing callbacks with cached entries
	 *		HPP_LTMStop - callback to destroy callback request if chain is destroyed
	 */
	sem_t Lock;

	#ifdef _M3U_BUFFER
	struct M3U_Cache TrackCache;
	#endif
};


enum PP_StateFlags
{
	PP_Completed				= 0x00000100
};

enum PP_RequestFlags
{
	PP_FoundTarget				= 0x00000001,
	PP_BodySeen					= 0x00000002,
	PP_PacketHeaderProcessed	= 0x00000004,
	PP_UriExists				= 0x00000008,
	PP_ReportedTarget			= 0x00000010,
	
	PP_Terminate				= 0x00000100,

	#ifdef _M3U_BUFFER
		PP_IsLtmCallback		= 0x00000200,
	#endif

	PP_PlaylistType_Unknown		= 0x00010000,
	PP_PlaylistType_ExtendedM3U = 0x00020000
};

/* function declaration */
int FindTrackUri(struct PlaylistState* ps, int attempts, int maxAttempts);

/* for quicksort */
void insertSort(struct PlaylistIndexer *pi, PLAYLIST_INDEXER_TYPE lb, PLAYLIST_INDEXER_TYPE ub)
{
	PLAYLIST_INDEXER_TYPE key;
	PLAYLIST_POSITION_TYPE pos;
    PLAYLIST_INDEXER_TYPE i, j;

   /*
    *  Sort array pi->Positions and pi->TrackIndices for this range: [lb..ub]
	*/
    for (i = lb + 1; i <= ub; i++) 
	{ 
		key = pi->TrackIndices[i];
		pos = pi->Positions[i];

        /* Shift elements down until insertion point found. */
		for (j = i-1; j >= lb && (pi->TrackIndices[j] > key); j--)
		{
			pi->TrackIndices[j+1] = pi->TrackIndices[j];
			pi->Positions[j+1] = pi->Positions[j];
		}

        /* insert */
		pi->TrackIndices[j+1] = key;
		pi->Positions[j+1] = pos;
    }
}

/* for quicksort */
PLAYLIST_INDEXER_TYPE partition(struct PlaylistIndexer *pi, PLAYLIST_INDEXER_TYPE lb, PLAYLIST_INDEXER_TYPE ub) 
{
    PLAYLIST_INDEXER_TYPE key, pivot, keyValue, pivotValue;
    PLAYLIST_INDEXER_TYPE i, j, p;

   /*
    *	Partition pi->TrackIndices and pi->Positions for: [lb..ub]
	*/

    /* select pivot and exchange with 1st element */
    p = lb + ((ub - lb)>>1);
	
	pivot = pi->TrackIndices[p];
	pivotValue = pi->Positions[p];

	pi->TrackIndices[p] = pi->TrackIndices[lb];
	pi->Positions[p] = pi->Positions[lb];

    /* sort lb+1..ub based on pivot */
    i = lb+1;
    j = ub;
    while (1) {
		while (i < j && (pivot > pi->TrackIndices[i])) i++;
		while (j >= i && (pi->TrackIndices[j] > pivot)) j--;
        if (i >= j) break;
        
		key = pi->TrackIndices[i];
		keyValue = pi->Positions[i];

		pi->TrackIndices[i] = pi->TrackIndices[j];
		pi->Positions[i] = pi->Positions[j];

		pi->TrackIndices[j] = key;
		pi->Positions[j] = keyValue;
        
		j--;
		i++;
    }

    /* pivot belongs in pi->TrackIndices[j] */
	pi->TrackIndices[lb] = pi->TrackIndices[j];
	pi->Positions[lb] = pi->Positions[j];

	pi->TrackIndices[j] = pivot;
	pi->Positions[j] = pivotValue;

    return j;
}

/*	quicksort algorithm to sort the playlist indexer table */
void quickSort(struct PlaylistIndexer *pi, PLAYLIST_INDEXER_TYPE lb, PLAYLIST_INDEXER_TYPE ub) 
{
    PLAYLIST_INDEXER_TYPE m;

   /*
    *	Sort the hashtable from indices a[lb..ub]
	*/
    while (lb < ub)
	{
        /* quickly sort short lists */
        if (ub - lb <= 12) 
		{
            insertSort(pi, lb, ub);
            return;
        }

        /* partition into two segments */
        m = partition (pi, lb, ub);

        /* sort the smallest partition    */
        /* to minimize stack requirements */
        if (m - lb <= ub - m) 
		{
            quickSort(pi, lb, m - 1);
            lb = m + 1;
        }
		else 
		{
            quickSort(pi, m + 1, ub);
            ub = m - 1;
        }
    }
}


/*
 *	Clears the hashtable mapping of TrackIndex to BytePosition,
 *	and resets the other fields to their proper initial state.
 */
void ClearIndexer(struct PlaylistIndexer *pi)
{
	int i;

	pi->HashIndex = 0;
	pi->Passes = 0;
	pi->HopSize = 1;
	pi->NumSkipped = 0;
	pi->FirstHashIndex = 0;
	for (i=0; i < PLAYLIST_HASHTABLE_SIZE; i++)
	{
		pi->TrackIndices[i] = 0;
		pi->Positions[i] = 0;
	}
}

/*
 *	Adds an entry indicating a trackIndex and its byte position
 *	within the playlist.
 */
void AddIndex (PLAYLIST_INDEXER_TYPE trackIndex, PLAYLIST_POSITION_TYPE position, struct PlaylistIndexer *pi)
{
	pi->NumSkipped++;
	if ((pi->HashIndex < PLAYLIST_HASHTABLE_SIZE) && (pi->NumSkipped == pi->Passes+1))
	{
		if (pi->HashIndex >= 0)
		{
			pi->NumSkipped = 0;
			pi->TrackIndices[pi->HashIndex] = trackIndex;
			pi->Positions[pi->HashIndex] = position;

			/*
			 *	Increment the hash index.
			 *	If the hash index is out of range, then reset it to zero
			 *	and increment the passes count.
			 */
			pi->HashIndex += pi->HopSize;

			if (pi->HashIndex >= PLAYLIST_HASHTABLE_SIZE)
			{
				pi->Passes++;
				if (pi->Passes < PLAYLIST_HASHTABLE_SIZE)
				{
					pi->HopSize = pi->Passes+1;
					pi->HashIndex = pi->Passes;
				}
				else
				{
					pi->HopSize = ((pi->Passes+1) % (PLAYLIST_HASHTABLE_SIZE));
					pi->HashIndex = pi->HopSize;
				}
			}
		}
		else
		{
			fprintf(stderr, "AddIndex() error: pi->HashIndex=%d\r\n", pi->HashIndex);
		}
	}
}

/*
 *	This method sorts the elements in the array
 *	according to the key/trackIndex value.
 *	The method should only be called after
 *	the playlist's contents have been completely
 *	processed.
 */
void SortIndexer(struct PlaylistIndexer *pi)
{
	int i;

	quickSort(pi, 0, PLAYLIST_HASHTABLE_SIZE-1);

	pi->HashIndex = 0;
	for (i=0; i < PLAYLIST_HASHTABLE_SIZE; i++)
	{
		if (pi->TrackIndices[i] > 0)
		{
			pi->FirstHashIndex = i;
			break;
		}
	}

#ifdef _PLAYLIST_TRACE
	printf("\r\nSorted Indexer: ");
	for (i=pi->FirstHashIndex; i < PLAYLIST_HASHTABLE_SIZE; i++)
	{
		printf("%d->%d ", pi->TrackIndices[i], pi->Positions[i]);
	}
	printf("\r\nFirstHashIndex=%d \r\n\r\n", pi->HashIndex);
#endif
}

/*
 *	Given a track index, provides the greatest byte position from which the 
 *	track can be found, the actual track index that is mapped
 *	at that position, and the byte position for the track after the specified position.
 *
 *	Caller is expected to provide the memory for indexedTrack and position and position2
 *	output arguments.
 *
 *	Method assumes it is called only when the hashtable has been sorted.
 */
void GetPositions(PLAYLIST_INDEXER_TYPE trackIndex, struct PlaylistIndexer *pi, PLAYLIST_INDEXER_TYPE *indexedTrack, PLAYLIST_POSITION_TYPE *position, PLAYLIST_POSITION_TYPE *position2)
{
	int i, j;
	PLAYLIST_INDEXER_TYPE ti;

	*position = 0;
	*position2 = 0;
	*indexedTrack = 0;

	if (pi->TrackIndices[0] <= trackIndex)
	{
		/*
		 *	If the trackIndex is in a range that is cached,
		 *	then search the hashtable for a cached byte position.
		 *
		 *	It's entirely possible that the hashtable does not have
		 *	the first xxx playlist items marked in its table.
		 */
		for (i = pi->FirstHashIndex; i < PLAYLIST_HASHTABLE_SIZE; i++)
		{
			j = i+1;

			ti = pi->TrackIndices[i];

			if (ti <= trackIndex)
			{
				*indexedTrack = ti;
				*position = pi->Positions[i];

				if (j < PLAYLIST_HASHTABLE_SIZE)
				{
					*position2 = pi->Positions[j];
				}
			}
			else if (ti > trackIndex)
			{
				break;
			}
		}
	}
	else
	{
		/*
		 *	This line put here for debugging.
		 */
		*indexedTrack = 1;
		*position = 0;
		*position2 = pi->Positions[0];
	}
}


void PP_ParseUri(char* URI, char** IP, int* Port, char** Path)
{
	struct parser_result *result,*result2,*result3;
	char *TempString,*TempString2;
	int TempStringLength,TempStringLength2;
	
	result = ILibParseString(URI, 0, ((int)strlen(URI)), "://", 3);
	TempString = result->LastResult->data;
	TempStringLength = result->LastResult->datalength;
	
	/* Parse Path */
	result2 = ILibParseString(TempString,0,TempStringLength,"/",1);
	TempStringLength2 = TempStringLength-result2->FirstResult->datalength;
	*Path = (char*)HPP_MALLOC(TempStringLength2+1);
	memcpy(*Path,TempString+(result2->FirstResult->datalength),TempStringLength2);
	(*Path)[TempStringLength2] = '\0';
	
	/* Parse Port Number */
	result3 = ILibParseString(result2->FirstResult->data,0,result2->FirstResult->datalength,":",1);
	if(result3->NumResults==1)
	{
		*Port = 80;
	}
	else
	{
		TempString2 = (char*)HPP_MALLOC(result3->LastResult->datalength+1);
		memcpy(TempString2,result3->LastResult->data,result3->LastResult->datalength);
		TempString2[result3->LastResult->datalength] = '\0';
		*Port = atoi(TempString2);
		HPP_FREE(TempString2);
	}
	/* Parse IP Address */
	TempStringLength2 = result3->FirstResult->datalength;
	*IP = (char*)HPP_MALLOC(TempStringLength2+1);
	memcpy(*IP,result3->FirstResult->data,TempStringLength2);
	(*IP)[TempStringLength2] = '\0';
	ILibDestructParserResults(result3);
	ILibDestructParserResults(result2);
	ILibDestructParserResults(result);
}

/*
 *	Helper function for Sink_HttpRequestProcessor().
 *	Processes the HTTP packet header, and appropriately sets flags.
 *
 *	Method assumes that caller has acquired ps->LockState.
 */
void ProcessHttpResponseHeader
	(
	struct HttpRequestPlaylist* request, 
	struct PlaylistState* ps, 
	struct packetheader *header,
	HttpPlaylistParser_Callback_PlaylistUriExists *p_executeOnUriExists,
	HttpPlaylistParser_Callback_OnUpdate_ItemCount *p_executeOnTrackCounted,
	HttpPlaylistParser_Callback_OnResult_FindTargetUri *p_executeOnTargetUpdated
	)
{
	int uriLen;
	char *line;
	char *uri;
	
	uri = ps->PlaylistUri;
	uriLen = ((int)strlen(uri));
	line = NULL; /* http header line */

	/*
	 *	This is the first ever time the HTTP client called us for
 	 *	this request, so we need to inform the state machine whether 
 	 *	the URI exists. We're assured that the HTTP client will only call
	 *	us if it receives a complete packet header.
	 */
	*p_executeOnUriExists = ps->CallbackUriExists;
		
	/*
	 *	Check to see if the response indicates the uri exists.
	 *	This includes checking for a NULL header as well
	 *	as a valid status code.
	 */
	if (header != NULL)
	{
		if (header->StatusCode == 200)
		{
			request->Flags |= PP_UriExists;
		}
		else if (header->StatusCode == 206)
		{
			/*
			 *	This is a partial response with the section we care about.
			 */
			request->Flags |= PP_UriExists;

			if ((request->Flags & PP_PacketHeaderProcessed) == 0)
			{
				/*
				 *	If we haven't processed the header, then set the last track index
				 *	to be the index-1 of the expected track for this response.
				 */
				request->LastTrackIndex = request->RangeExpectedTrackIndexMinusOne;
	
				if (request->LastTrackIndex < 0)
				{
					fprintf(stderr, "Expected invalid target track %d\r\n", request->LastTrackIndex);
				}
			}
		}
	}

	/*
	 *	If the media URI exists, and we have not processed the header, 
	 *	then go ahead and process the HTTP headers.
	 */
	if (request->Flags & PP_UriExists)
	{
		/*
		 *	Specify that we're going to event the number of tracks
		 *	after the lock is released... we will do this regardless
		 *	of what we determine about the response.
		 */
		if ((ps->Flags & PP_Completed)==0) { *p_executeOnTrackCounted = ps->CallbackItemCount; }

		/*
		 *	Try to figure out what type of playlist this data represents.
		 */

		if (strncasecmp(uri+uriLen-4, FILE_EXTENSION_ExtendedM3U, 4) == 0)
		{
			request->Flags |= PP_PlaylistType_ExtendedM3U;
		}
		/*
		 *	TODO: ASX processing
		else if (strncasecmp(uri+uriLen-4, FILE_EXTENSION_ASX, 4) == 0)
		{
		}
		 */
		else
		{
			/* can't figure out type based on file extension, check mime-type */
			line = ILibGetHeaderLine(header, "content-type", 12);

			if (strcasecmp(line, CONTENT_TYPE_ExtendedM3U) == 0)
			{
				request->Flags |= PP_PlaylistType_ExtendedM3U;
			}
			/*
			 *	TODO: ASX
			else if if (strcasecmp(line, CONTENT_TYPE_ASX) == 0)
			{
			}
			 */
			else
			{
				request->Flags |= PP_PlaylistType_Unknown;
			}
		}
	}

	/* indicate we've processed the header */
	request->Flags |= PP_PacketHeaderProcessed;
}

void ProcessHttpResponseBody_AsExtendedM3U
	(
	struct HttpRequestPlaylist* request, 
	struct PlaylistState* ps, 
	char* buffer,
	int *p_BeginPointer, 
	int EndPointer,
	int done, 
	HttpPlaylistParser_Callback_PlaylistUriExists *p_executeOnUriExists,
	HttpPlaylistParser_Callback_OnUpdate_ItemCount *p_executeOnTrackCounted,
	HttpPlaylistParser_Callback_OnResult_FindTargetUri *p_executeOnTargetUpdated,

	char **trackComment,
	char **trackUri,
	int	*trackDuration
	)
{
	int i, j, k;
	int eolPos;
	int cmp;
	short isHttp, newLine, isExtInf;
	int uriStart=-1; /* assigned when isHttp is nonzero */
	int uriLen=-1; /* assigned when isHttp!=0 and newLine != 0 */
	
	int extInfStart=-1;
	int extInfEnd=-1;
	int extInfLen = -1;
	int ei;
	int commaPos;
	int commentLen;

	long longVal;

	char *ch;

	if ((buffer != NULL) && (EndPointer >= 7))
	{
		/*
		 *	Process the playlist contents here.
		 *	Specifically, we're counting the number of tracks
		 *	in the playlist.
		 */
		i=0;			/* left index for beggining of line */
		eolPos = -1;	/* end of line position */
		isExtInf = 0;	/* line is an m3u comment */
		extInfStart = -1;
		extInfEnd = -1;
		isHttp = 0;		/* line contains http url */
		newLine = 1;	/* we are parsing a new line */

		for (j=0; j < EndPointer; j++)
		{
			k=1;			/* k=j+1 at beginning of each iteration*/

			/*
			 *	If we're parsing a new line, then we need to check
			 *	to see if the line is a comment. 
			 */
			if (newLine != 0)
			{
				cmp = strncasecmp(buffer+j, "#EXTINF:", 8);
				if (cmp == 0) 
				{	
					isExtInf = 1; 
					extInfStart = j+8; 
					extInfEnd = -1;
				}
				else
				{
					isExtInf = 0;
				}
			}

			/* do an optimized scan for the 'http://' string if we're reading a new line*/
			if (newLine != 0)
			{
				cmp = strncasecmp(buffer+j, "http://", 7);
				if (cmp == 0)
				{
					isHttp = 1;
					uriStart = j;
				}
			}

			/* we've started parsing a line, so it can't be a new line */
			newLine = 0;

			/*
			 *	If we haven't seen "http://" yet, then 
			 *	check the last 7 bytes to see if 'http://' is the string 
			 */
			if ((isHttp == 0) && (j >= i+6))
			{
				cmp = strncasecmp(&(buffer[j-6]), "http://", 7);
				if (cmp == 0)
				{
					isHttp = 1;
					uriStart = j-6;
				}
			}

			/*
			 *	Look for an end-of-line (eol) marker. Either "\r\n" or "\n".
			 *	If we found an eol, do the following:
			 *
			 *	Adjust eolPos so that its position is the first byte of the eol.
			 *	Adjust 'i' so that its position is the first byte on the next line.
			 *	Adjust j so that its position is the last byte of the current line.
			 *	Set newLine flag to 1.
			 */
			k = j+1;
			if (buffer[j] == '\n')
			{
				/*
				 *	We found an eol at current point.
				 *	We'll check for \r\n in next if-block.
				 */
				newLine = 1;
				eolPos = j;
				i = k;
			}
			else if (k < EndPointer)
			{
				/*
 				 *	Executes when there are at least 2 bytes
				 *	left for processing in the buffer. 
				 *	We check for a \r\n here.
				 */
				if ((buffer[j] == '\r') && (buffer[k] == '\n'))
				{
					newLine = 1;
					eolPos = j;
					i = k+1;
					j = k;
				}
			}
			else if ((j == EndPointer) && (done != 0))
			{
				/*
				 *	We need to handle a special case when the
				 *	current index points to the last index in
				 *	the current buffer and the server has indicated
				 *	that it's done sending stuff. In such a scenario,
				 *	the current line terminates at the last byte.
				 */
				newLine = 1;
				eolPos = j+1;
				i = k;
				j = k;
			}

			if (newLine != 0)
			{
				/*
				 *	If we have a new line, then it means
			 	 *	the current line is either a comment,
				 *	an HTTP uri, or garbage.
				 */

				if (isHttp != 0)
				{
					/* 
					 *	We've finished reading a line and its
					 *	began with HTTP...
					 */
					uriLen = eolPos - uriStart;

					/*
					 *	Ensure that this line contains a URI of appropriate size.
					 *	Otherwise, just treat it as a line of garbage and ignore it.
					 */
					if (uriLen < MAX_URI_SIZE)
					{
						/* Note our current position in the playlist for this request */
						request->LastTrackIndex++;

						if (((ps->Flags & PP_Completed) == 0) && (request->RequestNumber == 0))
						{
							/*
							 *	If we haven't informed the state machine about this
							 *	playlist's total track count, then increment the
							 *	current count and flag that we should update
							 * 	the state machine with a callback.
							 */
							ps->ItemCount++;
							*p_executeOnTrackCounted = ps->CallbackItemCount;

							/*
							 *	Add the track index and position to the hashtable/cache.
							 *
							 *	TODO: Include comment
							 */
							if (
								((uriStart - extInfEnd) <= 2) &&
								(extInfEnd - extInfStart > 0)
								)
							{
								AddIndex(ps->ItemCount, request->BytesProcessed + extInfStart, &(ps->Indexer));
							}
							else
							{
								AddIndex(ps->ItemCount, request->BytesProcessed + uriStart, &(ps->Indexer));
							}
						}

						#ifdef _M3U_BUFFER
						if (ps->TrackCache.NumClean < NUM_CACHED)
						{
							if (
								(request->LastTrackIndex >= ps->TrackCache.FirstTrackIndex) //&& 
								//(request->LastTrackIndex <= ps->TrackCache.LastTrackIndex)
								)
							{
								/* deallocate memory */
								if (ps->TrackCache.TrackUri[ps->TrackCache.NumClean] != NULL)
								{
									HPP_FREE(ps->TrackCache.TrackUri[ps->TrackCache.NumClean]);
									ps->TrackCache.TrackUri[ps->TrackCache.NumClean] = NULL;
								}
								if (ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean] != NULL)
								{
									HPP_FREE(ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean]);
									ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean] = NULL;
								}

								/*
								 *	Copy the uri. Do not trim because UTF8 may have nonprintable
								 *	but valid characters.
								 */
								ps->TrackCache.TrackUri[ps->TrackCache.NumClean] = (char*) HPP_MALLOC(uriLen+1);
								memcpy(ps->TrackCache.TrackUri[ps->TrackCache.NumClean], buffer+uriStart, uriLen);
								ps->TrackCache.TrackUri[ps->TrackCache.NumClean][uriLen] = '\0';

								/* Parse previous comment line for #EXTINF data */
								if (
									((uriStart - extInfEnd) <= 2) &&
									(extInfEnd - extInfStart > 0)
									)
								{
									extInfLen = extInfEnd - extInfStart;
									commaPos = -1;
									for (ch = buffer+extInfStart; ch < buffer+extInfEnd; ch++) 
									{
										if (*ch == ',')
										{
											/*
											 *	The difference between ch and buffer should not exceed
											 *	the size of a 32 bit integer, especially since they
											 *	reference a contiguous memory buffer.
											 */
											commaPos = ((int) (ch - buffer));
											break;
										}
									}

									if (commaPos < 0)
									{
										ps->TrackCache.TrackDuration[ps->TrackCache.NumClean] = -1;
										ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean] = (char*) HPP_MALLOC(1);
										ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean][0] = '\0';
									}
									else
									{
										/* parse duration */
										ei = ILibGetLong(buffer+extInfStart, commaPos-extInfStart, &longVal);
										if (ei == 0)
										{
											ps->TrackCache.TrackDuration[ps->TrackCache.NumClean] = (int) longVal;
										}
										else
										{
											ps->TrackCache.TrackDuration[ps->TrackCache.NumClean] = -1;
										}

										/* parse comment */
										commentLen = extInfEnd - commaPos;
										ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean] = (char*) HPP_MALLOC(commentLen);
										memcpy(ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean], buffer+commaPos+1, commentLen);
										ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean][commentLen-1] = '\0';
									}
								}
								else
								{
									ps->TrackCache.TrackDuration[ps->TrackCache.NumClean] = -1;
									ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean] = (char*) HPP_MALLOC(1);
									ps->TrackCache.TrackMetadata[ps->TrackCache.NumClean][0] = '\0';
								}

								ps->TrackCache.NumClean++;
							}
						}
						#endif

						/*
						 *	Check if the trackindex of this HTTP matches
						 *	the desired track index. 
						 *
						 * 	There are two values to check - an adjusted value
						 *	and a nonadjusted value. 
						 *
						 *	The nonadjusted value represents the target track
						 *	number from the state machine's perspective, but since
						 *	the state machine isn't guaranteed to know the total
						 *	number of tracks we have to ensure that we inform
						 * 	that a track was found based on its non adjusted index.
						 * 	An adjusted index value can only be determined when
						 *	the playlist has been completely parsed.
						 *
						 *	An adjusted value represents the target track in
						 *	the known playlist range.
						 */
						if (
							((request->LastTrackIndex == ps->InputIndex) && ((ps->Flags & PP_Completed) == 0)) ||
							((request->LastTrackIndex == ps->AdjustedInputIndex) && ((ps->Flags & PP_Completed)))
							)
						{
							if ((request->Flags & PP_FoundTarget) == 0)
							{
								/*
								 *	We've just found the target track for the first time 
								 *	for this request, so indicate that we
								 *	should execute the callback for finding the target.
								 */
								*p_executeOnTargetUpdated = ps->CallbackFoundTarget;

								if ((ps->Flags & PP_Completed) == 0)
								{
									/*
									 *	Assign the adjusted target index only if going by 
									 *	the nonadjusted track value.
									 */
									ps->AdjustedInputIndex = request->LastTrackIndex;
								}

								/*
								 *	Copy the uri. Do not trim because UTF8 may have nonprintable
								 *	but valid characters.
								 */
								*trackUri = (char*) HPP_MALLOC(uriLen+1);
								memcpy((*trackUri), buffer+uriStart, uriLen);
								(*trackUri)[uriLen] = '\0';
								request->Flags |= PP_FoundTarget;

								/* Parse previous comment line for #EXTINF data */
								if (
									((uriStart - extInfEnd) <= 2) &&
									(extInfEnd - extInfStart > 0)
									)
								{
									extInfLen = extInfEnd - extInfStart;
									commaPos = -1;
									for (ch = buffer+extInfStart; ch < buffer+extInfEnd; ch++) 
									{
										if (*ch == ',')
										{
											/*
											 *	The difference between ch and buffer should not exceed
											 *	the size of a 32 bit integer, especially since they
											 *	reference a contiguous memory buffer.
											 */
											commaPos = ((int) (ch - buffer));
											break;
										}
									}

									if (commaPos < 0)
									{
										//*trackComment = (char*) HPP_MALLOC(extInfLen+1);
										//memcpy((*trackComment), buffer+extInfStart, extInfLen);
										//(*trackComment)[extInfLen] = '\0';
										(*trackDuration) = -1;
										(*trackComment) = (char*) HPP_MALLOC(1);
										(*trackComment)[0] = '\0';
									}
									else
									{
										/* parse duration */
										ei = ILibGetLong(buffer+extInfStart, commaPos-extInfStart, &longVal);
										if (ei == 0)
										{
											*trackDuration = (int) longVal;
										}
										else
										{
											*trackDuration = -1;
										}

										/* parse comment */
										commentLen = extInfEnd - commaPos;
										*trackComment = (char*) HPP_MALLOC(commentLen);
										memcpy((*trackComment), buffer+commaPos+1, commentLen);
										(*trackComment)[commentLen-1] = '\0';
									}
								}
								else
								{
									(*trackDuration) = -1;
									(*trackComment) = (char*) HPP_MALLOC(1);
									(*trackComment)[0] = '\0';
								}
							}
						}
					}
				}
				else if (isExtInf)
				{
					/*
					 *	The line is a comment, so note the end position.
					 */
					extInfEnd = eolPos;
				}
				else
				{
					/*
					 *	It's neither a comment nor an HTTP line,
					 *	so toss information about where the last
					 *	comment starts and ends.
					 */
					extInfStart = -1;
					extInfEnd = -1;
				}

				/*
				 *	We've finished analyzing the complete line, so
				 *	assume that the next line is not a comment nor
				 *	an http url.
				 */
				isExtInf = 0;
				isHttp = 0;
			}
		} /* end for loop */

		/*
		 *	Be kind on memory; allow HttpClient module to 
		 *	FREE the buffer memory that it sent to me.
		 */
		if (i >= EndPointer)
		{
#ifdef _SAVE_PLAYLIST_BYTES
			memcpy(request->Bytes + request->BytesProcessed, buffer, EndPointer);
#endif

			/*
			 *	The next byte is beyond the allocated buffer
			 *	so move the begin pointer to match the end pointer.
			 */
			*p_BeginPointer = EndPointer;
			request->BytesProcessed += EndPointer;
		}
		else
		{
#ifdef _SAVE_PLAYLIST_BYTES
			memcpy(request->Bytes + request->BytesProcessed, buffer, i);
#endif
			/*
			 *	The next byte is still within range of the
			 *	allocated buffer so indicate that we're only
			 *	ready to deallocate the portion we've processed.
			 */
			*p_BeginPointer = i;
			request->BytesProcessed += i;
		}

#ifdef _SAVE_PLAYLIST_BYTES
		byteLen = strlen(request->Bytes);
		if (byteLen != request->BytesProcessed)
		{
			fprintf(stderr, "There was an error in the number of bytes processed: %d != %d\r\n", byteLen, request->BytesProcessed);
		}
		else
		{
			for (bi=0; bi < request->BytesProcessed; bi++)
			{
				if (request->Bytes[bi] == '\0')
				{
					fprintf(stderr, "Null found at %d\r\n", bi);
				}
			}
		}
#endif
	}
}

/*
 *	Helper function for Sink_HttpRequestProcessor().
 *	
 */
void PrepareCallbacksAndCleanup
	(
	struct HttpRequestPlaylist* request, 
	struct PlaylistState* ps, 

	int *p_BeginPointer, 
	int EndPointer,
	int done,

	short *p_freeRequest,
	short *p_retryRequest,

	char **trackComment,
	char **trackUri,
	int	*trackDuration,

	/*
	int *eventNumTracks,
	char *eventPlaylistUri,
	char *eventTrackUri,
	void **p_eventPlaylistTag,
	int *eventNonAdjustedTargetIndex,
	int *eventTargetIndex,
	*/

	HttpPlaylistParser_Callback_PlaylistUriExists *p_executeOnUriExists,
	HttpPlaylistParser_Callback_OnUpdate_ItemCount *p_executeOnTrackCounted,
	HttpPlaylistParser_Callback_OnResult_FindTargetUri *p_executeOnTargetUpdated
	)
{
	/* increment the number of times we've been called for this request */
	request->Passes++;

	/*
	 *	Copy the values needed for eventing before we release the lock
	 *	and also before we possibly release the request.
	 */
	/*
	*eventNumTracks = ps->ItemCount;
	strcpy(eventPlaylistUri, ps->PlaylistUri);
	strcpy(eventTrackUri, ps->TrackUri);
	HPP_FREE(ps->TrackUri);
	ps->TrackUri = NULL;
	*p_eventPlaylistTag = ps->InputTag;
	*eventNonAdjustedTargetIndex = ps->InputIndex;
	*eventTargetIndex = ps->AdjustedInputIndex;
	*/

	if ((done != 0) || (request->Flags & PP_Terminate))
	{
		/* if there's no more data, then we've parsed this playlist completely */
		if (done != 0)
		{
			/* Flag the playlist object as having been processed completely. */
			if (request->RequestNumber == 0)
			{
				ps->Flags |= PP_Completed;

				/*
				 *	Sort the hashtable of track indices to 
				 *	byte positions within the playlist.
				 */
				SortIndexer(&(ps->Indexer));
			}

			if (
				((*trackUri == NULL) || ((*trackUri)[0] == '\0')) &&
				((request->Flags & PP_Terminate))
				)
			{
				/*
				 *	The track uri is still null or empty, so issue a request to 
				 *	find the track so that we can find the right track.
				 *
				 *	This code is really conditional for the case where the user 
				 *	set the desired track to a track index outside the playlist's
				 *	range before we were able to parse the entire playlist. Now 
				 *	that we know the complete track count, we can properly
				 *	issue another request that includes an adjusted target track.
				 */
				if (request == ps->LastRequest)
				{
#ifdef _PLAYLIST_TRACE
					printf("\r\nRetryRequest(TargetNotFound): na=%d tt=%d lt=%d nt=%d\r\n\r\n", 
						ps->InputIndex, 
						ps->AdjustedInputIndex, 
						request->LastTrackIndex,
						ps->ItemCount
						);
#endif
					*p_retryRequest = FindTrackUri(ps, request->Attempts, request->AllowedAttempts);
					if (*p_retryRequest == 0)
					{
						*p_executeOnTargetUpdated = ps->CallbackFoundTarget;
					}
				}
			}
		}

		/*
		 *	If server closed the connection or we're going to close the connection,
		 *	then we need to tidy up.
		 */

		/*
		 *	Set the last request to null if and only if the
		 *	this request is the last request. This code is
		 *	heavily dependent on ps-LockState being synchronized
		 *	properly.
		 */
		if (ps->LastRequest == request)
		{
			ps->LastRequest = NULL;
		}

		/*
		 *	FREE the memory associated with the request
		 *	because we're done. We flag for a FREE instead
		 *	of actually freeing because we may need to read
		 *	from the request if we make a retry attempt.
		 */
		*p_freeRequest = 1;

		/*
		 *	Instruct the http client that we no longer 
		 *	care about the allocated body memory.
		 */
		*p_BeginPointer = EndPointer;
	}
}

/*void Sink_HttpRequestProcessor(void *reader, struct packetheader *header, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user)*/
//void Sink_HttpRequestProcessor  (void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2)
void Sink_HttpRequestProcessor(
		void *reader,
		int IsInterrupt,
		struct packetheader *header,
		char *buffer,
		int *p_BeginPointer,
		int EndPointer,
		int done,
		void *user,
		void *user2,
		int *PAUSE)
{
	struct HttpRequestPlaylist* request = NULL;		/* the http request that we're processing */
	struct PlaylistState* ps = NULL;				/* the playlist state associated with the request */

	char *trackComment;
	char *trackUri;
	int trackDuration;


	/*
	 *	If any of these callbacks are non-null, it means that we need to
	 *	execute the callback at the end of this method. Assume we don't
	 *	need to event anything.
	 */
	HttpPlaylistParser_Callback_OnUpdate_ItemCount executeOnItemCounted = NULL;			/* executed when item count was updated */
	HttpPlaylistParser_Callback_PlaylistUriExists executeOnUriExists = NULL;				/* executed when uri's existence is determined */
	HttpPlaylistParser_Callback_OnResult_FindTargetUri executeOnTargetUpdated = NULL;	/* executed when target uri is found or determined to be unlisted in playlist body*/

	/*
	 *	If nonzero, we should deallocate the request object.
	 *	We flag for a FREE instead of actually freeing because we may need to read
	 *	from the request before deallocating because of an attempt to retry.
	 */
	short freeRequest = 0;

	/*
	 *	If nonzero, we should attempt a retry on the request.
	 */
	short retryRequest = 0;

	/*
	 *	If nonzero, we should process the request.
	 */
	short processRequest = 0;

	/*
	 *	Microstack changes its interface so that there is not a separate callback
	 *	for interrupted communciations.
	 */
	if (IsInterrupt)
	{
		HttpRequestCleanup(reader, user, user2);
		return;
	}

	/*
	 *	If the user object is null, then we've cancelled the request.
	 */
	if (user == NULL)
	{
		#ifdef _DEBUG
		printf("Sink_HttpRequestProcessor(): request=NULL\r\n");
		#endif

		return;
	}

	/* get convenient pointers that represent the state of the playlist */
	request = (struct HttpRequestPlaylist*) user;
	ps = (struct PlaylistState*) request->State;

	/*
	 *	Added because we don't close the socket for HTTP 1.1 anymore.
	 *	Instead, we simply ignore.
	 */
	if (request->Flags & PP_Terminate)
	{
		*p_BeginPointer = EndPointer;

		if (done != 0)
		{
			HPP_FREE(request);
		}
		return;
	}

	#ifdef _DEBUG
	printf("Sink_HttpRequestProcessor(): request=%p ps->LastRequest=%p &ps->PlaylistUri=%p request->PlaylistUriAddress=%p request->RequestNumber=%d\r\n", request, ps->LastRequest, &ps->PlaylistUri, request->PlaylistUriAddress, request->RequestNumber);
	#endif

	/* lock the state because we're processing stuff */
	sem_wait(&(ps->Lock));

	/*
	 *	Initialize some basic vars.
	 */
	trackComment	= NULL;
	trackUri		= NULL;
	trackDuration	= -1;

	/*
	 *	Determine if we should process this request.
	 *	We process requests only if they are the last request issued,
	 *	or if it is the first request issued for the current playlist uri.
	 *	We always continue processing the first request for the current playlist
	 *	because that is when we calculate the number of tracks.
	 */
	processRequest = 
		(
		(request == ps->LastRequest) || 
		((request->PlaylistUriAddress == &(ps->PlaylistUri)) && (request->RequestNumber == 0))
		);

	/* If we're to process the request... */
	if (processRequest != 0)
	{
		/* If the HTTP packet's header hasn't been analyzed, then process the header */
		if ((request->Flags & PP_PacketHeaderProcessed)==0)
		{
			ProcessHttpResponseHeader(request, ps, header, &executeOnUriExists, &executeOnItemCounted, &executeOnTargetUpdated);
		}

		/* If appropriate, process the body of the playlist */
		if ((request->Flags & PP_UriExists) && ((request->Flags & PP_Terminate)==0) && (EndPointer > 0) )
		{
			if ((request->Flags & PP_PlaylistType_ExtendedM3U) != 0)
			{
				ProcessHttpResponseBody_AsExtendedM3U(request, ps, buffer, p_BeginPointer, EndPointer, done, &executeOnUriExists, &executeOnItemCounted, &executeOnTargetUpdated, &trackComment, &trackUri, &trackDuration);
			}
			/*
			 * TODO: ASX
			else if ((request->Flags & PP_PlaylistType_ASX) != 0)
			{
			}
			*/
			else if ((request->Flags & PP_PlaylistType_Unknown) != 0)
			{
			}
			else
			{
				ASSERT((request->Flags & PP_PlaylistType_Unknown) != 0);
			}

		}
	}
	else
	{
		request->Flags |= PP_Terminate;
	}

	/* Prepare arguments for executing callbacks; also tidy things up before method ends */
	PrepareCallbacksAndCleanup
		(
		request,
		ps,
		
		p_BeginPointer,
		EndPointer,
		done,
	
		&freeRequest,
		&retryRequest,

		&trackComment,
		&trackUri,
		&trackDuration,
		/*
		&eventNumTracks,
		eventPlaylistUri,
		eventTrackUri,
		&eventPlaylistTag,
		&eventInputIndex,
		&eventActualIndex,
		*/
		
		&executeOnUriExists,
		&executeOnItemCounted,
		&executeOnTargetUpdated
		);


	if (request->Flags & PP_Terminate)
	{
		/*
		 *	Tell the HTTP client to close the socket.
		 */
		*p_BeginPointer = EndPointer;

		#ifdef _M3U_BUFFER
		if ((done == 0) && (ps->TrackCache.NumClean >= NUM_CACHED))
		#else
		if (done == 0)
		#endif
		{
			#ifdef _PLAYLIST_TRACE
			printf("Terminating request=%d\r\n", request->RequestNumber);
			#endif

			#ifdef _DEBUG
			printf("Closing request=%p requestNumber=%d\r\n", request, request->RequestNumber);
			#endif

			//ILibCloseRequest(reader);
		}
	}

	/* execute callbacks */
	if (executeOnUriExists != NULL)
	{
		/*
		 *	If we're allowed a retry, attempt it.
		 *	Otherwise, simply event
		 */
		if ((request->Attempts < request->AllowedAttempts) && ((request->Flags & PP_UriExists)==0))
		{
			/*
			 *	Be sure to lock the playlist state because we need
			 *	to see if there's a request that supercedes the
			 *	request we want to make. Furthermore, IssueRequest()
			 *	assumes that ps->LockState has been locked.
			 */
			if ((ps->LastRequest == NULL) || (ps->LastRequest == request))
			{
#ifdef _PLAYLIST_TRACE
				printf("\r\nRetry request\r\n\r\n");
#endif
				IssueRequest(
					ps, 
					request->Attempts, 
					request->AllowedAttempts, 
					request->RangeExpectedTrackIndexMinusOne, 
					request->RangeStart,
					request->RangeEnd,
					Sink_HttpRequestProcessor
					);
			}
		}
		else
		{
			executeOnUriExists(ps, ps->PlaylistUri, ps->InputTag, (request->Flags & PP_UriExists));
		}
	}

	if (executeOnItemCounted != NULL)
	{
		executeOnItemCounted(ps, ps->PlaylistUri, ps->InputTag, (ps->Flags & PP_Completed), ps->ItemCount);
	}

	if (executeOnTargetUpdated != NULL)
	{
		/*
		 *	Be sure to lock the playlist state because we need
		 *	to see if there's a request that supercedes the
		 *	request we want to make. Furthermore, IssueRequest()
		 *	assumes that ps->LockState has been locked.
		 */
		if (! ((ps->LastRequest == NULL) || (ps->LastRequest == request)))
		{
			executeOnTargetUpdated = NULL;
		}

		if (executeOnTargetUpdated != NULL)
		{
			executeOnTargetUpdated
				(
				ps,
				ps->InputWrapAround,
				
				ps->PlaylistUri,
				ps->InputIndex,	
				ps->InputTag,
				ps->AdjustedInputIndex,	

				trackUri,
				trackDuration,
				trackComment
				);

			request->Flags |= PP_ReportedTarget;
		}
	}
	else
	{
		if ((freeRequest != 0) && (processRequest != 0) && ((request->Flags & PP_ReportedTarget)==0))
		{
			#ifdef _DEBUG
			printf("WARNING: Sink_HttpRequestProcessor(request=%p, requestNumber=%d, expectedTrackMinusOne=%d, lastTrackIndex=%d, inputIndex=%d, adjInputIndex=%d, reported=%d) did not report target and we are freeing request.\r\n", request, request->RequestNumber, request->RangeExpectedTrackIndexMinusOne, request->LastTrackIndex, ps->InputIndex, ps->AdjustedInputIndex, request->Flags & PP_ReportedTarget);
			#endif

			/*
			 *	This code here provides a contingency for when a target cannot be found
			 *	even if the playlist was processed.
			 */
			ps->CallbackFoundTarget
				(
				ps,
				ps->InputWrapAround,
				
				ps->PlaylistUri,
				ps->InputIndex,	
				ps->InputTag,
				ps->AdjustedInputIndex,	

				NULL,
				-1,
				NULL
				);
		}
	}

	if (trackUri != NULL) HPP_FREE(trackUri);
	if (trackComment != NULL) HPP_FREE(trackComment);

	if ((freeRequest != 0) && (done != 0))
	{
		/*
		 *	Free the request if we no longer need it.
		 *	We FREE here because we need to read from
		 *	the request object if we intend to issue
		 *	a retry attempt.
		 */

		/*
		 *	We FREE the request when we reach the very end
		 *	of the data.
		 */
		HPP_FREE(request);
	}

	/* unlock the state because we're done */
	sem_post(&(ps->Lock));
}

/*
 *	Method called whenever unexpected cleanup needs to occur.
 *	Method is provided as a cleanup callback in ILibAddRequest.
 *	user1 should be (struct HttpRequestPlaylist) and user2 should be null.
 */
void HttpRequestCleanup(void *sender, void *user1, void *user2)
{
	/*
	 *	It looks like user1 is never
	if (user1 != NULL)
	{
		HPP_FREE(user1);
	}
	else
	{
		#ifdef _DEBUG
		printf("WARNING: HttpRequestCleanup() - user1 is null.\r\n");
		#endif
		fprintf(stderr, "WARNING: HttpRequestCleanup() - user1 is null.\r\n");
	}
	 */

	if (user2 != NULL)
	{
		#ifdef _DEBUG
		printf("ERROR: HttpRequestCleanup() - user2 is not null.\r\n");
		#endif
		fprintf(stderr, "ERROR: HttpRequestCleanup() - user2 is not null.\r\n");
	}
}

/*
 *	This method formulates an HTTP request for download.
 *
 *	Method assumes that caller has locked ps->LockState.
 */
void IssueRequest(struct PlaylistState* ps, int attempts, int maxAttempts, int expectedRangeTrack, int rangeStart, int rangeEnd, void *callback)
{
	struct packetheader *packet;
	int len;
	char *ip_address, *path;
	int port;
	struct HttpRequestPlaylist *request;
	struct sockaddr_in destAddr;
	char rangeLine[100];

#ifdef _DEBUG
	printf("IssueRequest(%p, %d, %d, %d, %d, %d, %p) - Target=%d\r\n", ps, attempts, maxAttempts, expectedRangeTrack, rangeStart, rangeEnd, callback, ps->InputIndex);
#endif

	/*
	 *	Create an empty packet, set the directive and host field.
	 *	Be sure to FREE memory returned from ParseUri.
	 */
	packet = ILibCreateEmptyPacket();
	PP_ParseUri(ps->PlaylistUri, &ip_address, &port, &path);
	ILibAddHeaderLine(packet, "HOST", 4, ip_address, (int)strlen(ip_address));
	len = (int)strlen(path);
	ILibSetDirective(packet, "GET", 3, path, len);

	/*
	 *	If caller specified range options, add them.
	 */
	if ((rangeStart >= 0) && (rangeEnd > 0) && (rangeEnd > rangeStart))
	{
		sprintf(rangeLine, "bytes=%d-%d", rangeStart, rangeEnd);
		ILibAddHeaderLine(packet, "RANGE", 5, rangeLine, (int)strlen(rangeLine));
	}
	else if (rangeStart > 0)
	{
		sprintf(rangeLine, "bytes=%d-", rangeStart);
		ILibAddHeaderLine(packet, "RANGE", 5, rangeLine, (int)strlen(rangeLine));
	}
	else
	{
		rangeLine[0] = '\0';
	}


	/* specify the destination socket */
	memset((char *)&destAddr, sizeof(destAddr), 0);
	destAddr.sin_family = AF_INET;
	destAddr.sin_addr.s_addr = inet_addr(ip_address);
	destAddr.sin_port = htons(port);

	/*
	 *	Create a request object and initialize it.
	 *	Note the associated playlist object,
	 *	the address of the uri so we can compare it on the callback,
	 *	and add the request.
	 */
	request = (struct HttpRequestPlaylist*) HPP_MALLOC(sizeof(struct HttpRequestPlaylist));
	request->LastTrackIndex = 0;
	request->RangeExpectedTrackIndexMinusOne = expectedRangeTrack;
	request->Passes = 0;
	request->State = ps;
	request->Attempts = attempts+1;
	request->AllowedAttempts = maxAttempts;
	request->RangeStart = rangeStart;
	request->RangeEnd = rangeEnd;
	request->RequestNumber = ps->RequestNumber;ps->RequestNumber++;
	request->BytesProcessed = 0;
	request->Flags = 0;
	request->PlaylistUriAddress = &(ps->PlaylistUri);

#ifdef _SAVE_PLAYLIST_BYTES
	for (i=0; i < PLAYLIST_BYTES_SIZE; i++)
	{
		request->Bytes[i] = '\0';
	}
#endif

	ps->LastRequest = request;

#ifdef _DEBUG
	printf("IssueRequest(cont'd) request=%p RequestNumber=%d (%s)\r\n", request, request->RequestNumber, rangeLine);
#endif
	/*ILibAddRequest(ps->HttpClient, packet, &destAddr, callback, HttpRequestCleanup, request, NULL);*/
	//ILibAddRequest(ps->HttpClient, packet, &destAddr, callback, request, NULL);
	ILibWebClient_PipelineRequest(ps->HttpClient,&destAddr,packet,callback,request,NULL);

	/*
	 *	Free memory from ParseUri
	 */
	HPP_FREE(ip_address);
	HPP_FREE(path);
}

#ifdef _M3U_BUFFER
	void HPP_SinkLTM(void *request)
	{
		struct HttpRequestPlaylist *req = (struct HttpRequestPlaylist*) request;
		struct PlaylistState *ps = req->State;
		char *targetUri;
		int duration;
		char *comment;
		int dti;
		
		/* lock req->State to ensure consistent callback pointers */
		sem_wait(&(ps->Lock));

		/*
		 *	Only execute the callbacks if this is the most recent request.
		 */
		if (req == ps->LastRequest)
		{
			/* ensure that we can access info about the target */

			dti = ps->AdjustedInputIndex - ps->TrackCache.FirstTrackIndex;

			if ((dti < 0) || (dti > NUM_CACHED-1) )
			{
				fprintf(stderr, "HPP_SinkLTM() Error. dti=%d and NUM_CACHED-1=%d\r\n", dti, NUM_CACHED-1);
			}
			else
			{
				/* get pointers to the appropriate trackUri and metadata */
				duration = ps->TrackCache.TrackDuration[dti];
				targetUri = ps->TrackCache.TrackUri[dti];
				comment = ps->TrackCache.TrackMetadata[dti];

				ps->CallbackFoundTarget(ps, ps->InputWrapAround, ps->PlaylistUri, ps->InputIndex, ps->InputTag, ps->AdjustedInputIndex, targetUri, duration, comment);
			}

			ps->LastRequest = NULL;
		}

		HPP_FREE(req);
		sem_post(&(ps->Lock));
	}

	void HPP_SinkLTMStop(void *request)
	{
		struct HttpRequestPlaylist *req = (struct HttpRequestPlaylist*) request;

		/*
		 *	Do not lock req->State->Lock because it may already be destroyed.
		 *	We don't need to worry about thread synchronization if
		 *	the thread chain is being destroyed because we don't
		 *	change the state of req->State in any way here.
		 */

		if ((req->Flags & PP_IsLtmCallback) != 0)
		{
			/*
			 *	The only fields that should be set are:
			 *		req->PlaylistUriAddress;
			 *		req->Flags
			 *	Both of these fields are not allocated fields
			 *	owned by this object.
			 */
			HPP_FREE(req);
		}
		else
		{
			fprintf(stderr, "HPP_SinkLTMStop() error. %p->Flags & PP_IsLtmCallback is zero.\r\n", req);
		}
	}
#endif

/*
 *	Returns:
 *		0: Did not issue an HTTP request because playlist was empty.
 *		1: Issued an HTTP request to acquire the target.
 *		-1: Did not issue an HTTP request because we already have the target.
 */
int FindTrackUri(struct PlaylistState* ps, int attempts, int maxAttempts)
{
	int retVal = 1; 

	#ifdef _M3U_BUFFER
		PLAYLIST_POSITION_TYPE ignore1, ignore2;
		struct HttpRequestPlaylist *request;
		int lti;
	#endif

	PLAYLIST_POSITION_TYPE startRange = 0;
	PLAYLIST_POSITION_TYPE rangeEnd = 0;
	PLAYLIST_INDEXER_TYPE expectedRangeTrack = 0;
	 

	if (ps->Flags & PP_Completed)
	{
		/*
		 *	If we've completed processing the playlist, then
		 *	we know the number of tracks in the playlist.
		 */
		if (ps->ItemCount == 0)
		{
			/*
			 *	This is an empty playlist, so report the uri and index
			 *	to indicate there's nothing here.
			 */
			if (ps->CallbackFoundTarget != NULL)
			{
				ps->CallbackFoundTarget(ps, ps->InputWrapAround, ps->PlaylistUri, ps->InputIndex, ps->InputTag, 0, NULL, -1, NULL);
			}
			retVal = 0;
		}
		else
		{
			/*
			 *	Ensure that ps->AdjustedInputIndex is within range.
			 *	If wraparound behavior is enabled, then the track number
			 *	will cycle from the beginning until it's in range.
			 *	Otherwise, the track is simply the last track.
			 */
			ps->AdjustedInputIndex = ps->InputIndex;
			if (ps->InputWrapAround != 0)
			{
				/* wraparound is enabled */
				if (ps->AdjustedInputIndex > ps->ItemCount)
				{
					while (ps->AdjustedInputIndex > ps->ItemCount)
					{
						ps->AdjustedInputIndex -= ps->ItemCount;
					}
				}
				else if ((ps->AdjustedInputIndex < 1) && (ps->ItemCount > 0))
				{
					while (ps->AdjustedInputIndex < 1)
					{
						ps->AdjustedInputIndex += ps->ItemCount;
					}
				}
				else if ((ps->AdjustedInputIndex < 1) && (ps->ItemCount == 0))
				{
					ps->AdjustedInputIndex = 0;
				}
			}
			else
			{
				/* wrapround is not enabled*/
				if (ps->AdjustedInputIndex > ps->ItemCount)
				{
					/* point to the last track if index exceeds max value */
					ps->AdjustedInputIndex = ps->ItemCount;
				}
				else if ((ps->AdjustedInputIndex < 1) && (ps->ItemCount > 0))
				{
					/* point to the first track if index is less than 1 */
					ps->AdjustedInputIndex = 1;
				}
			}

			/*
			 *	Examine the hashtable to obtain the appropriate range values.
			 */
			#ifndef _M3U_BUFFER
				/* 
				 *	If we don't have an M3U buffer, we always issue an HTTP
				 *	request to acquire the track URI.
				 */
				GetPositions(ps->AdjustedInputIndex, &(ps->Indexer), &expectedRangeTrack, &startRange, &rangeEnd);
			#else
				/*
				 *	If we have an M3U buffer, then check if we need to 
				 *	issue an HTTP request. Only issue an HTTP request
				 *	if the target track number is outside of our
				 *	cached range.
				 */

				if (
					(ps->AdjustedInputIndex < ps->TrackCache.FirstTrackIndex) ||
					//(ps->AdjustedInputIndex > ps->TrackCache.LastTrackIndex)
					(ps->AdjustedInputIndex >= ps->TrackCache.FirstTrackIndex+ps->TrackCache.NumClean)
					)
				{
					/*
					 *	We need to issue an HTTP request.
					 *	Each time we issue an HTTP request, we intend to populate
					 *	the ps->TrackCache structure. This requires that we
					 *	determine the first and last track entries that
					 *	will go in the structure.
					 */
					ps->TrackCache.NumClean = 0;

					ps->TrackCache.FirstTrackIndex = ps->AdjustedInputIndex - HALF_CACHED;
					if (ps->TrackCache.FirstTrackIndex < 1) 
					{ 
						ps->TrackCache.FirstTrackIndex = 1; 
					}

					//ps->TrackCache.LastTrackIndex = ps->TrackCache.FirstTrackIndex + NUM_CACHED - 1;
					//if (ps->TrackCache.LastTrackIndex > ps->ItemCount)
					//{
					//	ps->TrackCache.LastTrackIndex = ps->ItemCount;
					//}
					lti = ps->TrackCache.FirstTrackIndex + NUM_CACHED - 1;
					if (lti > ps->ItemCount)
					{
						lti = ps->ItemCount;
					}

					/* obtain the byte range necessary to acquire the desired range */
					GetPositions(ps->TrackCache.FirstTrackIndex, &(ps->Indexer), &expectedRangeTrack, &startRange, &ignore1);
					//GetPositions(ps->TrackCache.LastTrackIndex, &(ps->Indexer), &ignore1, &ignore2, &rangeEnd);
					GetPositions(lti, &(ps->Indexer), &ignore1, &ignore2, &rangeEnd);
				}
				else
				{
					/*
					 *	We don't need to issue an HTTP request.
					 *	The means we need to execute the callback for
					 *	reporting the target URI.
					 *	To provide applications with a consistent 
					 *	threading model, we'll use the lifetime
					 *	monitor object to execute the callback, so that
					 *	the callbacks always execute on the microstack thread.
					 */
					retVal = -1;

					request = (struct HttpRequestPlaylist*) HPP_MALLOC(sizeof(struct HttpRequestPlaylist));
					request->Flags = PP_IsLtmCallback;
					request->PlaylistUriAddress = &(ps->PlaylistUri);
					request->State = ps;
					ps->LastRequest = request;

					ILibLifeTime_Add(ps->TrackCache.LifeTimeMonitor, request, 0, HPP_SinkLTM, HPP_SinkLTMStop);
				}
			#endif

			if (retVal == 1)
			{
				if (expectedRangeTrack < 0)
				{
					fprintf(stderr, "Playlist Indexer Cache has errors in it.\r\n");
					startRange = 0;
					rangeEnd = 0;
					expectedRangeTrack = 0;				
				}
				/*
				 *	Decrement expected range so that the first track we find will increment
				 *	to match the expected track number.
				 */
				expectedRangeTrack--;
			}
		}
	}
	else
	{
		/*
		 *	We don't know the real length of this playlist
		 *	so go ahead and do a brute force search
		 *	through the entire thing. To accomplish this
		 *	we simply ensure that startRange/rangeLen == 0.
		 */
		startRange = 0;
		rangeEnd = 0;
		expectedRangeTrack = 0;
	}

	if (retVal == 1)
	{
		/*
		 *	Formulate the HTTP request to download the appropriate subset of the playlist.
		 *	Set the callback so that the method doesn't process
		 *	the playlist, rather it specifically looks for the trackUri.
		 *
		 */
		IssueRequest(ps, attempts, maxAttempts, expectedRangeTrack, startRange, rangeEnd, Sink_HttpRequestProcessor);
	}

	return retVal;
}

void HttpPlaylistParser_FindTargetUri
	(
	void *parserObject, 
	int wrapAround, 
	const char* playlistUri, 
	int itemIndex, 
	void *userObject,
	HttpPlaylistParser_Callback_PlaylistUriExists CallbackUriExists,
	HttpPlaylistParser_Callback_OnUpdate_ItemCount CallbackItemCount,
	HttpPlaylistParser_Callback_OnResult_FindTargetUri CallbackFoundTarget
	)
{
	struct PlaylistState *ps;
	int size, uriChanged, targetChanged, ftu;

	DEBUGONLY(printf("HttpPlaylistParser_FindTargetUri(%p, %d, %s, %d, %p, %p, %p, %p)\r\n", \
		parserObject, \
		wrapAround, \
		playlistUri, \
		itemIndex, \
		userObject, \
		CallbackUriExists, \
		CallbackItemCount, \
		CallbackFoundTarget);)

	ps = (struct PlaylistState*) parserObject;
	ps->InputTag = userObject;

	/* lock the playlist state */
	sem_wait(&(ps->Lock));

	ps->CallbackFoundTarget = CallbackFoundTarget;
	ps->CallbackItemCount = CallbackItemCount;
	ps->CallbackUriExists = CallbackUriExists;

	uriChanged = 0;
	targetChanged =0;
	ftu = 0;

	if (strcmp(playlistUri, ps->PlaylistUri) != 0)
	{
		HPP_FREE(ps->PlaylistUri);
		size = ((int)strlen(playlistUri))+1;
		ps->PlaylistUri = (char*) HPP_MALLOC(size);
		memcpy(ps->PlaylistUri, playlistUri, size);

		/* new playlist requires we reset playlist-wide stats */
		ps->ItemCount = 0;
		ps->RequestNumber = 0;
		ps->Flags = 0;

		ClearIndexer(&(ps->Indexer));

		uriChanged = 1;
	}

	/* end lock */

	if (
		(itemIndex	!= ps->InputIndex) || 
		(wrapAround != ps->InputWrapAround)
		)
	{
		ps->InputIndex = itemIndex;
		ps->InputWrapAround = wrapAround;
		targetChanged = 1;
	}

	/*if ((uriChanged != 0) || (targetChanged != 0))*/
	{
		if (uriChanged != 0)
		{
			/*
			 *	The playlist's uri has changed, so we need to process the playlist.
			 *	IssueRequest requires that ps->LockState has been locked, which
			 *	the loop suffices.
			 */
			#ifdef _M3U_BUFFER
			ps->TrackCache.NumClean = 0;
			ps->TrackCache.FirstTrackIndex = 1;
			//ps->TrackCache.LastTrackIndex = NUM_CACHED;
			#endif
			IssueRequest (ps, 0, 1, 0, 0, 0, Sink_HttpRequestProcessor);
		}
		else /*if (targetChanged != 0)*/
		{
			/*
			 *	The playlist uri has not changed, but the target track index has changed,
			 *	so issue a request to find the target track. 
			 */
			FindTrackUri(ps,0,1);
		}
	}

	/* unlock playlist staet */
	sem_post(&(ps->Lock));
}

void Destroy_Parser_ExtendedM3U(void* object)
{
	struct PlaylistState *ps;
//	int i;

	/* create a parser instance, set everything to zero */
	ps = (struct PlaylistState*)object;

	sem_wait(&(ps->Lock));

	#ifdef _M3U_BUFFER
	ps->TrackCache.NumClean = 0;
	for (i=0; i < NUM_CACHED; i++)
	{
		ps->TrackCache.TrackDuration[i] = -1;
		if (ps->TrackCache.TrackMetadata[i] != NULL) 
		{ 
			HPP_FREE(ps->TrackCache.TrackMetadata[i]); 
			ps->TrackCache.TrackMetadata[i] = NULL;
		}
		if (ps->TrackCache.TrackUri[i] != NULL) 
		{
			HPP_FREE(ps->TrackCache.TrackUri[i]); 
			ps->TrackCache.TrackUri[i] = NULL;
		}
	}
	#endif

	sem_post(&(ps->Lock));

	sem_destroy(&(ps->Lock));
	
	/* don't need to destroy lifetime monitor field on ps->TrackCache because it will diestroy itself. */

	if (ps->PlaylistUri != NULL) HPP_FREE(ps->PlaylistUri);
}

void* CreatePlaylistParser(void* Chain, int numberOfSockets)
{
	struct PlaylistState *ps;

	/* create a parser instance, set everything to zero */
	ps = (struct PlaylistState*)HPP_MALLOC(sizeof(struct PlaylistState));
	memset(ps,0,sizeof(struct PlaylistState));
	sem_init(&(ps->Lock), 0, 1);

	#ifdef _M3U_BUFFER
	memset(&(ps->TrackCache), 0, sizeof (struct M3U_Cache));
	ps->TrackCache.LifeTimeMonitor = ILibCreateLifeTime(Chain);
	#endif

	/*
	 *	Create the HTTP client that handles HTTP Targets.
	 */
	ps->HttpClient = ILibCreateWebClient(numberOfSockets,Chain);

	ps->PlaylistUri = (char*) HPP_MALLOC(1);
	ps->PlaylistUri[0] = '\0';

	ClearIndexer(&(ps->Indexer));
	/*
	 *	Initialize standard chaining stuff.
	 *	Set the destroy function, and add the http client
	 *	before adding the parser to the chain.
	 */
	ps->Destroy = &Destroy_Parser_ExtendedM3U;
	ILibAddToChain(Chain,ps);

	return ps;
}

