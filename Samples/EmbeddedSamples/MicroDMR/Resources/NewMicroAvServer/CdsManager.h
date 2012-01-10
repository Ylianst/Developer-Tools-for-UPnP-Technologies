#ifndef _CDS_MANAGER_H
#define _CDS_MANAGER_H

#include <stdlib.h>
#include <ctype.h>

/*
 *	Enumerates the errors that can be returned when attempting
 *	to add a new database entry.
 */
enum CDSMGR_Enum_Errors
{
	CDSMGR_Error_NotEnoughMetadata = -1,
};

struct CDSMGR_TranscodingResource
{
	/* Must be non-NULL, valid protocolInfo string. */
	wchar_t		*ProtocolInfo;
	int			ProtocolInfoLength;

	/*
	 *	Protocol identifier used in the URI
	 */
	wchar_t		*Protocol;
	int			ProtocolLength;

	/* 
	 *	Network address.
	 *
	 *	If non-null, the value should represent a network address,
	 *	such as an IP Address and Port number combination.
	 *
	 *	If null, then all local IP addresses/ports will be used.
	 *	Null value instructs the DIDL-Lite serializer to provide 
	 *	multiple <res> elements for each IP address.
	 */
	wchar_t		*AddressAndPort;
	int			AddressAndPortLength;

	/* 
	 *	Path.
	 *
	 *	If non-null, the value should represent the path that is
	 *	appended to the AddressAndPort field, excluding a
	 *	query-string.
	 *
	 *	If null, then the path will be provided by the logic
	 *	that is responsible for serializing DIDL-Lite.
	 */
	wchar_t		*Path;
	int			PathLength;

	/*
	 *	Query string.
	 *
	 *	If non-null, the value will be appended after the Path.
	 */
	wchar_t		*Query;
	int			QueryLength;

	/*
	 *	Associated resource metadata is below this.
	 */

	unsigned int	 ColorDepth;	/*images: zero indicates unknown*/
	unsigned int	 ResX;			/*images, video: zero indicates unknown*/
	unsigned int	 ResY;			/*images, video: zero indicates unknown*/
	unsigned int	 FileSize;		/*any: zero indicates unknown*/
	unsigned int	 ByteRate;			/*audio, video: zero is VBR or unknown*/
	unsigned int	 DurationInSeconds;	/*audio, video: zero is unknown*/
};

/*
 *	Adds an image to the database, using an explicit URI or a local file path.
 *	The internal database will automatically make the image
 *	appear in the appropriate containers.
 *
 *	Returns a numeric ObjectID assigned to the new object.
 *
 *	If the specified LocalPath is already in the database, 
 *	then the returned value will be value for the existing item.
 *	In such a case, a new entry is not made in the database.
 *
 *	If the value is negative, then the file was not added
 *	for one or more error reasons.
 */
long CDSMGR_AddImage(
						  const wchar_t  *SourceUri,	/*must be non-empty, valid path*/
						  const wchar_t  *Caption,		/*must be non-empty*/
						  const wchar_t  *AlbumName,	/*should be non-empty, but can be null*/

						  int			 isLocalFile,	/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,		/*must be non-empty, valid mime type*/
						  unsigned int	 colorDepth,	/*zero indicates unknown*/
						  unsigned int	 resX,			/*zero indicates unknown*/
						  unsigned int	 resY,			/*zero indicates unknown*/
						  unsigned int	 fileSize		/*zero indicates unknown*/
						  );

/*
 *	Adds an audio to the database, using an explicit URI or a local file path.
 *	The internal database will automatically make the audio item
 *	appear in the appropriate containers.
 *
 *	Returns a numeric ObjectID assigned to the new object.
 *
 *	If the specified LocalPath is already in the database, 
 *	then the returned value will be value for the existing item.
 *	In such a case, a new entry is not made in the database.
 *
 *	If the value is negative, then the file was not added
 *	for one or more error reasons.
 */
long CDSMGR_AddAudio(
						  const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
						  const wchar_t  *Title,			/*must be non-empty*/
						  const wchar_t  *Artist,			/*should be non-empty, but can be null*/
						  const wchar_t  *AlbumName,		/*should be non-empty, but can be null*/

						  int			 isLocalFile,		/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,			/*must be non-empty, valid mime type*/
						  unsigned int	 byteRate,			/*zero is VBR or unknown*/
						  unsigned int	 durationInSeconds,	/*zero is unknown*/
						  unsigned int	 fileSize			/*zero is unknown*/
						  );

/*
 *	Adds a video to the database, using an explicit URI or a local file path.
 *	The internal database will automatically make the video item
 *	appear in the appropriate containers.
 *
 *	Returns a numeric ObjectID assigned to the new object.
 *
 *	If the specified LocalPath is already in the database, 
 *	then the returned value will be value for the existing item.
 *	In such a case, a new entry is not made in the database.
 *
 *	If the value is negative, then the file was not added
 *	for one or more error reasons.
 */
long CDSMGR_AddVideo(
						  const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
						  const wchar_t  *Title,			/*must be non-empty*/
						  const wchar_t  *Creator,			/*should be non-empty, but can be null*/

						  int			 isLocalFile,		/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
						  const wchar_t  *mimeType,			/*must be non-empty, valid mime type*/
						  unsigned int	 byteRate,			/*zero is VBR or unknown*/
						  unsigned int	 durationInSeconds,	/*zero is unknown*/
						  unsigned int	 resX,				/*zero indicates unknown*/
						  unsigned int	 resY,				/*zero indicates unknown*/
						  unsigned int	 fileSize			/*zero indicates unknown*/
						  );

/*
 *	Adds a playlist container to the database, using an explicit URI or a local file path.
 *	The internal database will automatically make the playlist container
 *	appear in the appropriate containers.
 *	Before calling this method, developer needs to call the appropriate
 *	CDSMGR_Addxxx methods to obtain objectIDs for all of the local files
 *	in the playlist.
 *
 *	Returns a numeric ObjectID assigned to the new object.
 *
 *	If the specified LocalPath is already in the database, 
 *	then the returned value will be value for the existing item.
 *	In such a case, a new entry is not made in the database.
 *
 *	If the value is negative, then the file was not added
 *	for one or more error reasons.
 */
long CDSMGR_AddPlaylist(
							 const wchar_t  *SourceUri,		/*must be non-empty, valid path*/
							 const wchar_t  *Title,			/*must be non-empty*/
							 const wchar_t  *Creator,		/*should be non-empty, but can be null*/

							 int			isLocalFile,			/*if nonzero, then an HTTP-GET resource will be associated using the info below*/
							 const wchar_t	*mimeType,				/*must be non-empty, valid mime type*/
							 long			*playlistEntries,		/* array of database-specified ObjectIDs */
							 unsigned int	playlistEntriesCount	/* must be greater than zero */
							 );

/*
 *	Returns nonzero value (resourceIndex) if the resource was successfully associated.
 */
long CDSMGR_AddTranscoding(long objectID, struct CDSMGR_TranscodingResource transcodingRes);

/*
 *	Removes the object entry given the 'sourceUri' that was specified in the CDSMGR_Addxxx method.
 */
long CDSMGR_RemoveObject(const wchar_t *sourceUri);

/*
 *	Removes a resource from the specified objectID/resourceIndex.
 */
long CDSMGR_RemoveResource(long objectID, long resourceIndex);

#endif
