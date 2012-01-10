/*   
Copyright 2006 - 2010 Intel Corporation

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

using System;
using System.Collections;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Keeps the input values associated with a browse request.
	/// </summary>
	public struct BrowseRequest
	{
		/// <summary>
		/// The media object to browse identified by its ID.
		/// </summary>
		public string ObjectID;

		/// <summary>
		/// Used to indicate whether the browse 
		/// should obtain metadata of an object or 
		/// obtain metadata of direct children.
		/// </summary>
		public CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag;
		
		/// <summary>
		/// If obtaining children, the start index of the results.
		/// Otherwise set to zero.
		/// </summary>
		public uint StartIndex;
		/// <summary>
		/// If obtaining children the max number of child objects
		/// to retrieve. Otherwise use zero.
		/// </summary>
		public uint RequestCount;
		
		/// <summary>
		/// Comma-separated value list of metadata names to include
		/// in the response. For all metadata, use * character.
		/// </summary>
		public string Filter;
		/// <summary>
		/// Comma-separated value list of metadata names to use for
		/// sorting, such that preceding each metadata name (but after
		/// the comma) a + or - character is present to indicate
		/// ascending or descending sort order for that property.
		/// </summary>
		public string SortCriteria;

		/// <summary>
		/// Not an input value - but can be used to optionally track
		/// the updateID value at the time of the request.
		/// </summary>
		public uint UpdateID;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Stores the input values associated with a CreateObject request.
	/// </summary>
	public struct CreateObjectRequest
	{
		/// <summary>
		/// ID of the parent container where the new object should appear.
		/// </summary>
		public string ParentID;

		/// <summary>
		/// CDS-compliant subset of pseudo-DIDL-Lite elements that
		/// describe the new media objects. Certain fields, like
		/// ID, will be blank - thus making the XML values not
		/// truly schema-compliant with DIDL-Lite.
		/// </summary>
		public string Elements;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Keeps the input values associated with a CreateReference request.
	/// </summary>
	public struct CreateReferenceRequest
	{
		/// <summary>
		/// The ID of the parent container.
		/// </summary>
		public string ContainerID;

		/// <summary>
		/// The ID of the referenced item.
		/// </summary>
		public string ObjectID;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Keeps the input values associated with a DeleteResource request.
	/// </summary>
	public struct DeleteResourceRequest
	{
		/// <summary>
		/// The URI value of the resource. (eg, contentUri property on resource object).
		/// </summary>
		public Uri ResourceUri;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Keeps the input values associated with a destroy object request.
	/// </summary>
	public struct DestroyObjectRequest
	{
		/// <summary>
		/// The object to destroy, referred by ID.
		/// </summary>
		public string objectID;
		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Stores the input values associated with an ExportResource Request.
	/// </summary>
	public struct ExportResourceRequest
	{
		/// <summary>
		/// The URI (the contentUri value advertised by the mediaserver) of the source binary.
		/// </summary>
		public Uri SourceUri;
		/// <summary>
		/// The URI (the importUri value advertised by a remote mediaserver) 
		/// where the file should be sent.
		/// </summary>
		public Uri DestinationUri;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Stores the input values associated with an ImportResource Request.
	/// </summary>
	public struct ImportResourceRequest
	{
		/// <summary>
		/// The URI of the source binary.
		/// </summary>
		public Uri SourceUri;
		/// <summary>
		/// The URI (the importUri value advertised by a mediaserver) where the file should be saved.
		/// </summary>
		public Uri DestinationUri;

		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Keeps the input values associated with a search request.
	/// </summary>
	public struct SearchRequest
	{
		/// <summary>
		/// The container to recursively searhc from.
		/// </summary>
		public string ContainerID;
		/// <summary>
		/// The CDS compliant search expression string.
		/// </summary>
		public string SearchCriteria;
		/// <summary>
		/// If obtaining children, the start index of the results.
		/// Otherwise set to zero.
		/// </summary>
		public uint StartIndex;
		/// <summary>
		/// If obtaining children the max number of child objects
		/// to retrieve. Otherwise use zero.
		/// </summary>
		public uint RequestCount;
		
		/// <summary>
		/// Comma-separated value list of metadata names to include
		/// in the response. For all metadata, use * character.
		/// </summary>
		public string Filter;
		/// <summary>
		/// Comma-separated value list of metadata names to use for
		/// sorting, such that preceding each metadata name (but after
		/// the comma) a + or - character is present to indicate
		/// ascending or descending sort order for that property.
		/// </summary>
		public string SortCriteria;
		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}

	/// <summary>
	/// Keeps the input values associated with an update object request.
	/// </summary>
	public struct UpdateObjectRequest
	{
		/// <summary>
		/// The object whose metadata should be updated, referred by ID.
		/// </summary>
		public string objectID;
		/// <summary>
		/// Comma separated value list of XML elements that should be changed.
		/// Empty string after a comma or between comma pairs indicates 
		/// the creation of a new metadata xml element.
		/// </summary>
		public string CurrentTagValues;
		/// <summary>
		/// Comma separated value list of new XML elements that should replace the old values.
		/// Empty string after a comma or between comma pairs indicates 
		/// the removal of an old xml element.
		/// </summary>
		public string NewTagValues;
		/// <summary>
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </summary>
		public object Tag;
	}
}
