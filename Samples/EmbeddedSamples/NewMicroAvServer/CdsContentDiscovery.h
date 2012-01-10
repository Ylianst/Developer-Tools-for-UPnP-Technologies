#ifndef _CDS_CONTENT_DISCOVERY_h
#define _CDS_CONTENT_DISCOVERY_h

/*
 *	General rules and concepts.
 *
 *	This module provides the means for an application to respond to 
 *	ContentDirectory requests related to content discovery. Specifically,
 *	the two relevant actions are Browse and Search. 
 *
 *	Architecturally, the module sits directly on top of a DeviceBuilder-generated
 *	microstack and routes the callback sinks for Browse and Search to methods
 *	defined in the module. The module defines two callbacks for the upper software
 *	layer: MmsCallback_OnBrowse, MmsCallback_OnSearch. The appropriate callback 
 *	executes when the upper-application layer needs to respond to a browse or search request.
 *
 *	The module can be configured one of two ways by defining or not defining
 *	the CDS_USE_WORKER_THREAD preproc variable.
 *
 *	-Each method call requires the object returned from CreateMediaServer.
 *
 *	-All strings for the interface are assumed to be UTF8 compliant. This means
 *	 that string arguments in callbacks will be sent in their UTF8-form
 *	 and that all strings sent in response to a CP's action are properly
 *	 encoded by the application layer in UTF8 form. 
 *
 */

struct MmsBrowseArgs
{
	/*
	 *	A MediaServer token. Such an object can be matched with the application's
	 *	list of active MediaServer objects, each returned from CreateMediaServer().
	 */
	void *MediaServerObject;

	/*
	 *	Application can use this token when calling UpnpResponse_xxx methods.
	 */
	void *UpnpToken;

	/*
	 *	ObjectID specified by the control point.
	 */
	const char *ObjectID;

	/*
	 *	Nonzero value indicates that the CP wants the children of the
	 *	specified objectID. Otherwise, the CP wants the metadata for
	 *	the specified object.
	 */
	int BrowseDirectChildren;

	/*
	 *	Metadata filter settings. Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
	 *	that describe what CDS metadata to return in the response. In this framework,
	 *	the application layer is responsible for enforcing metadata filtering.
	 */
	const char *Filter;

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
	 *	SortCriteria strings have the following form:
	 *	[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 
	 */
	const char *SortCriteria;

	/*
	 *	Reserved: contains the argument name to return DIDL-Lite results.
	 */
	void *_ReservedDidlOutArgname;
};


/*
 *	This application-level method executes when the UPnP layer 
 *	receives a request for a item or container's metadata.
 */
extern void (*MmsCallback_OnBrowse) (struct MmsBrowseArgs *browseArgs);


/*
 *	Application-level code can call this method within their
 *	implementation of MmsCallback_OnBrowse when they need to respond to the
 *	control point with an error.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		errorCode				: This value can be one of the values in MmsErrorCodes
 *								: or it can be be a vendor-defined value between the range
 *								: of [800,899] inclusive.
 *
 *		errorMsg				: This is the custom-message to include with the error code.
 *								: The string must be encoded with UTF8 compliance.
 *
 */
void Mms_ForResponse_RespondError(struct MmsBrowseArgs *browseArgs, int errorCode, const char *errorMsg);


/*
 *	Application-level code can call this method within their
 *	implementation of MmsCallback_OnBrowse when they need to begin the response to the
 *	control point. This method should be called before calling 
 *	Mms_ForResponse_RespondBrowse_ResultArgument or Mms_ForResponse_RespondBrowse_FinishResponse.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		sendDidlHeader			: If nonzero, the DIDL-Lite header is sent. If nonzero,
 *								: the application is responsible for sending that header
 *								: through Mms_ForResponse_RespondBrowse_ResultArgument.
 */
void Mms_ForResponse_RespondBrowse_StartResponse(struct MmsBrowseArgs *browseArgs, int sendDidlHeader);

/*
 *	Application-level code can call this method within their
 *	implementation of MmsCallback_OnBrowse when they need to respond to the
 *	control point with data in the Result argument. This method must
 *	be called after Mms_ForResponse_RespondBrowse_StartResponse but
 *	before Mms_ForResponse_RespondBrowse_FinishResponse.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		xmlEscapedUtf8Didl		: DIDL-Lite response data, where the data is properly XML-escaped and also in UTF8 encoding.
 *								: The upper application layer can call this an arbitrary number of times, so the
 *								: upper application layer can decide between making fewer calls with larger
 *								: DIDL-Lite data blocks or more calls with smaller DIDL-Lite data blocks.
 *
 *		didlSize				: The number of bytes in xmlEscapedUtf8Didl, not including a trailing null-terminator.
 */
void Mms_ForResponse_RespondBrowse_ResultArgument(struct MmsBrowseArgs *browseArgs, const char *xmlEscapedUtf8Didl, int didlSize);

/*
 *	Application-level code can call this method within their
 *	implementation of MmsCallback_OnBrowse when they need to finish the
 *	response to a control point.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		sendDidlFooter			: If nonzero, the DIDL-Lite footer is sent. If nonzero,
 *								: the application will have sent the DIDL-Lite footer in a previous
 *								: call to Mms_ForResponse_RespondBrowse_ResultArgument.
 *
 *		numberReturned			: the number of DIDL-Lite elements returned in the response.
 *
 *		totalMatches			: the number of DIDL-Lite elements that could be returned given the request.
 *
 *		updateID				: the updateID of the container. Zero, if respond to a BrowseMetadata request on an item.
 */
void Mms_ForResponse_RespondBrowse_FinishResponse(struct MmsBrowseArgs *browseArgs, int sendDidlFooter, unsigned int numberReturned, unsigned int totalMatches, unsigned int updateID);

/*
 *	This method creates the MediaServer object that abstracts state information
 *	for a MediaServer. 
 */
void *CreateMediaServer(void *chain, void *stack);


#endif

