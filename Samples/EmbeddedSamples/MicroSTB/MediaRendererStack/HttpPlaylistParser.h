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

#ifndef _HTTP_PLAYLIST_PARSER_H_
#define _HTTP_PLAYLIST_PARSER_H_

/* max # of bytes for a URI */
#define MAX_URI_SIZE 1024

/*
 *	Creates an instance of a playlist parser. 
 *
 *	Chain:				The thread chain where this parser belongs.
 *
 *	numberOfSockets:	Zero indicates a default value. The number of sockets
 *						largely depends on how many simultaneous HTTP requests
 *						the application wants to handle at once. 
 */
void* CreatePlaylistParser(void* Chain, int numberOfSockets);

/*
 *	Returns the result of a HttpPlaylistParser_FindTargetUri request.
 *
 *	actualIndex:		Actual index of targetUri. (itemIndex may point to something out of range)
 *
 *	targetUri:			Uri of the target specified. Caller must FREE this memory.
 *						Value is null if target was not found.
 *
 *	duration:			duration in seconds, as specified in the #EXTINF line before the targetUri.
 *						The value is -1 if duration is unknown or was not present in the M3U.
 *
 *	comment:			Comment portion of the #EXTINF line before the targetUri line.
 *						Caller must FREE this memory when no longer needed.
 */
typedef void (*HttpPlaylistParser_Callback_OnResult_FindTargetUri) (void *parserObject, int wrapAround, const char* playlistUri, int itemIndex, void *userObject, /*OUT*/ int actualIndex, /*OUT - MUST COPY THIS*/ const char *targetUri, /*OUT*/int duration, /*OUT - MUST COPY THIS*/ const char* comment);

/*
 *	Asynchronously reports the maximum number of tracks as they are discovered.
 *	
 *	maxIndexNotKnown:	Nonzero indicates that the last item index for the playlist uri is not yet known.
 *
 *	itemCount:			Number of items known to exist in the M3U.
 */
typedef void (*HttpPlaylistParser_Callback_OnUpdate_ItemCount) (void *parserObject, const char* playlistUri, void* userObject, /*OUT*/int maxIndexNotKnown, /*OUT*/int itemCount);

/*
 *	This function pointer type is used to asynchronously notify that a specified
 *	playlistUri exists. 
 *
 *	uriExists:			nonzero indicates the URI exists
 *
 *	Method implementations intended to be used as a callback must not block or
 *	execute Playlist_FindTargetUri().
 */
typedef void (*HttpPlaylistParser_Callback_PlaylistUriExists) (void* parserObject, const char* playlistUri, void* userObject, int uriExists);


/*
 *	Requests the parser to obtain the item URI for the specified playlist and item index.
 *
 *	If the caller issues request before the CallbackFoundTarget
 *	is executed, then the caller won't get a result callback for the earlier request.
 *	This design decision was chosen largely because the overhead needed to track simultaneous
 *	requests
 *
 *	parserObject:		The void* from CreatePlaylistParser().
 *
 *	playlistUri:		The URI where the M3U can be downloaded.
 *
 *	itemIndex:			The item that should be returned, identified by its position within the M3U.
 *
 *	wrapAround:			If itemIndex is out of range and wrapAround is nonzero, then an appropriate
 *						track from somewhere in the M3U will be selected. Otherwise, the last track
 *						is selected as the output.
 *
 *	userObject:			Application-specified object that can be used to key this request, or used
 *						as an input parameter for the asynchronous result callback.
 */
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
	);

#endif
