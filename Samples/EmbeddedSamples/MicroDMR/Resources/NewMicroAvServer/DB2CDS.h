#ifndef _DB2CDS_H
#define _DB2CDS_H

#include <ctype.h>

struct DB2CDS_CdsQuery;
typedef void (*DB2CDS_Callback_OnQuery) (struct DB2CDS_CdsQuery *cdsQuery);


/*
 *	Encapsulates input arguments for a Browse or Search request.
 *	Most arguments have been request to wide-char format.
 */
struct DB2CDS_CdsQuery
{
	/*
	 *	Specifies what type of query.
	 */
	enum MSL_Enum_QueryTypes QueryType;

	/*
	 *	Application can use this token when calling UpnpResponse_xxx methods.
	 */
	void *UpnpToken;

	/*
	 *	ObjectID specified by the control point as a wide/unicode string.
	 */
	wchar_t *ObjectID;

	/*
	 *	Metadata filter settings. Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
	 *	that describe what CDS metadata to return in the response. In this framework,
	 *	the application layer is responsible for enforcing metadata filtering.
	 *
	 *	This field remains in UTF-8 formatted string because it will be
	 *	used with CdsToDidl_GetFilterBitString().
	 */
	char *Filter;

	/*
	 *	The index of the first media object to return. Zero-based value.
	 *	Only applicable when BrowseDirectChildren is nonzero.
	 */
	unsigned int StartingIndex;
	
	/*
	 *	The maximum number of media objects to return. Zero means return all media objects.
	 *	Only applicable when BrowseDirectChildren is nonzero.
	 */
	unsigned int RequestedCount;

	/*
	 *	SortCriteria string in wide/unicode form, with the following form:
	 *	[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 
	 */
	wchar_t *SortCriteria;

	/*
	 *	If QueryType == MS_Query_Search, then this field
	 *	specifies the search query (in wide/unicode encoding) 
	 *	as specified in the format of the CDS specification.
	 */
	wchar_t *SearchCriteria;

	/*
	 *	Original MSL_CdsQuery object.
	 */
	struct MSL_CdsQuery  *MslCdsQuery;

	/*
	 *	The IP address that received the request.
	 *	Used to ensure the appropriate <res>
	 *	elements appear first.
	 */
	int	RequestedOnAddress;

	/*
	 *	The port number that received the request.
	 *	Used when ordering <res> elements.
	 */
	int RequestedOnPort;

	/*
	 *	Next CDS query in the queue.
	 */
	struct DB2CDS_CdsQuery *Next;
};



/*
 *	This method should be assigned to MSL_Callback_OnQuery.
 */
void DB2CDS_OnBrowseSearch (struct MSL_CdsQuery *cdsQuery);

/*
 *	This method is blocking.
 *	When this method is called, the module will process CDS queries
 *	in a serialized fashion. 
 *
 *	Developer can specify a callback for when an invocation is about to happen.
 */
void DB2CDS_StartCdsQueryProcessing(DB2CDS_Callback_OnQuery queryCallback);

/*
 *	This method returns immediately.
 *	When this method is called, DB2CDS_StartCdsQueryProcessing() will
 *	finish the current response and then quit, leaving any
 *	pending CDS queries unanswered.
 */
void DB2CDS_StopCdsQueryProcessing();

#endif
