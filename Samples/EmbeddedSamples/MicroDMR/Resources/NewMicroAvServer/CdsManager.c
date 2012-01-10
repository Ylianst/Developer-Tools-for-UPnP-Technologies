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

#include "CdsManager.h"

/*see header*/
long CDSMGR_AddImage(
						  const wchar_t  *SourceUri,	/*must be non-empty, valid path*/
						  const wchar_t  *Caption,		/*must be non-empty*/
						  const wchar_t  *AlbumName,	/*should be non-empty, but can be null*/

						  int			 addHttpRes,	/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,		/*must be non-empty, valid mime type*/
						  unsigned int	 colorDepth,	/*zero indicates unknown*/
						  unsigned int	 resX,			/*zero indicates unknown*/
						  unsigned int	 resY,			/*zero indicates unknown*/
						  unsigned int	 fileSize		/*zero indicates unknown*/
						  )
{
	//TODO CDSMGR_AddImage()
	return 0;
}

/*see header*/
long CDSMGR_AddAudio(
						  const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
						  const wchar_t  *Title,			/*must be non-empty*/
						  const wchar_t  *Artist,			/*should be non-empty, but can be null*/
						  const wchar_t  *AlbumName,			/*should be non-empty, but can be null*/

						  int			 addHttpRes,		/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,			/*must be non-empty, valid mime type*/
						  unsigned int	 byteRate,			/*zero is VBR or unknown*/
						  unsigned int	 durationInSeconds,	/*zero is unknown*/
						  unsigned int	 fileSize			/*zero is unknown*/
						  )
{
	//TODO CDSMGR_AddAudio()
	return 0;
}

/*see header*/
long CDSMGR_AddVideo(
						  const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
						  const wchar_t  *Title,			/*must be non-empty*/
						  const wchar_t  *Creator,			/*should be non-empty, but can be null*/

						  int			 addHttpRes,		/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,			/*must be non-empty, valid mime type*/
						  unsigned int	 byteRate,			/*zero is VBR or unknown*/
						  unsigned int	 durationInSeconds,	/*zero is unknown*/
						  unsigned int	 resX,				/*zero indicates unknown*/
						  unsigned int	 resY,				/*zero indicates unknown*/
						  unsigned int	 fileSize			/*zero indicates unknown*/
						  )
{
	//TODO CDSMGR_AddVideo()
	return 0;
}

/*see header*/
long CDSMGR_AddPlaylist(
							 const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
							 const wchar_t  *Title,			/*must be non-empty*/
							 const wchar_t  *Creator,		/*should be non-empty, but can be null*/

							 int			addHttpRes,				/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
							 const wchar_t	*mimeType,				/*must be non-empty, valid mime type*/
							 long			*playlistEntries,		/* array of database-specified ObjectIDs */
							 unsigned int	playlistEntriesCount	/* must be greater than zero */
							 )
{
	//TODO CDSMGR_AddPlaylist()
	return 0;
}

/*see header*/
long CDSMGR_AddTranscoding(long objectID, struct CDSMGR_TranscodingResource transcodingRes)
{
	//TODO CDSMGR_AddResource()
	return 0;
}

