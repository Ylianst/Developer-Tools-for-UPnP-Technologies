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
using System.Xml;
using System.IO;
using System.Text;
using System.Reflection;
using System.Collections;
using OpenSource.UPnP.AV;
using System.Diagnostics;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Interface for all CpMediaXXX classes, including:
	/// <list type="bullet">
	/// <item><term><see cref="CpMediaContainer"/></term></item>
	/// <item><term><see cref="CpMediaItem"/></term></item>
	/// <item><term><see cref="CpRootContainer"/></term></item>
	/// </list>
	/// 
	/// Programmers should restrict their use of 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.IUPnPMedia"/>
	/// and use ICpMedia or IDvMedia when attempting to 
	/// consolidate code with the use of interfaces.
	/// </summary>
	public interface ICpMedia : IUPnPMedia
	{
		/// <summary>
		/// This method updates an <see cref="ICpMedia"/> object's metadata by using the CDS browse action,
		/// and the method may optionally update the metadata of its children (if the object has children). 
		/// The method's exact behavior is not defined by this interface,
		/// although implementations of this interface can customize the manner in which things
		/// are updated.
		/// <para>
		/// For example, the <see cref="CpMediaContainer"/> class will update its own metadata
		/// according to some internally managed state object.
		/// </para>
		/// <para>
		/// This method has no asynchronous callback.
		/// </para>
		/// </summary>
		void Update();

		/// <summary>
		/// Allows ICpMedia objects to request a browse on the media server from a media object.
		/// This method is slightly different from the <see cref="ICpMedia.Update"/> because 
		/// <see cref="ICpMedia.RequestBrowse"/> makes a browse request and hands the results
		/// directly to the application caller, whereas the <see cref="ICpMedia.Update"/> method 
		/// will make a browse request to update a media object's metadata and possibly its 
		/// children's metadata.
		/// </summary>
		/// <param name="BrowseFlag">browse metadata or direct children</param>
		/// <param name="Filter">
		/// Comma-separated value list of metadata names to include
		/// in the response. For all metadata, use * character.
		/// </param>
		/// <param name="StartingIndex">
		/// If obtaining children, the start index of the results.
		/// Otherwise set to zero.
		/// </param>
		/// <param name="RequestedCount">
		/// If obtaining children the max number of child objects
		/// to retrieve. Otherwise use zero.
		/// </param>
		/// <param name="SortCriteria">
		/// Comma-separated value list of metadata names to use for
		/// sorting, such that preceding each metadata name (but after
		/// the comma) a + or - character is present to indicate
		/// ascending or descending sort order for that property.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestBrowse (CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, string Filter, uint StartingIndex, uint RequestedCount, string SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultBrowse callback);

		/// <summary>
		/// Allows ICpMedia objects to request changes to its metadata.
		/// </summary>
		/// <param name="useThisMetadata">describes the desired metadata for the media object</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">callback executes when the results are available</param>
		void RequestUpdateObject (IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback);

		/// <summary>
		/// Allows ICpMedia objects to request the remote server to remove them
		/// from the content hierarchy.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">callback executes when the results are available</param>
		void RequestDestroyObject(object Tag, CpMediaDelegates.Delegate_ResultDestroyObject callback);

		/// <summary>
		/// Requests a remote media server to delete a resource from its local file system.
		/// </summary>
		/// <param name="deleteThisResource">the resource to request for deletion</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">callback to execute when results have been obtained</param>
		void RequestDeleteResource(ICpResource deleteThisResource, object Tag, CpMediaDelegates.Delegate_ResultDeleteResource callback);

		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// the resource URI represented by this object.
		/// </summary>
		/// <param name="sourceUri">the URI where binary should be pulled</param>
		/// <param name="importHere">the <see cref="ICpResource"/> object where the binary should go to</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestImportResource(System.Uri sourceUri, ICpResource importHere, object Tag, CpMediaDelegates.Delegate_ResultImportResource callback);

		/// <summary>
		/// Makes a request to a remote media server to export one of its binary files
		/// to another location.
		/// </summary>
		/// <param name="exportThis">
		/// The resource (of this media object) that should be exported.
		/// </param>
		/// <param name="sendHere">
		/// The uri where the binary should be sent.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestExportResource(ICpResource exportThis, System.Uri sendHere, object Tag, CpMediaDelegates.Delegate_ResultExportResource callback);

	}

	/// <summary>
	/// Simply used to represent media items for control-point use.
	/// </summary>
	public interface ICpItem : ICpMedia, IMediaItem
	{
	}

	/// <summary>
	/// Defines the request methods for media objects that apply only to container objects.
	/// </summary>
	public interface ICpContainer : ICpMedia, IMediaContainer
	{
		/// <summary>
		/// This method requests a remote media server to do a search from a specified container.
		/// </summary>
		/// <param name="SearchCriteria">the CDS-compliant search expression string</param>
		/// <param name="Filter">
		/// Comma-separated value list of metadata names to include
		/// in the response. For all metadata, use * character.
		/// </param>
		/// <param name="StartingIndex">
		/// If obtaining children, the start index of the results.
		/// Otherwise set to zero.
		/// </param>
		/// <param name="RequestedCount">
		/// If obtaining children the max number of child objects
		/// to retrieve. Otherwise use zero.
		/// </param>
		/// <param name="SortCriteria">
		/// Comma-separated value list of metadata names to use for
		/// sorting, such that preceding each metadata name (but after
		/// the comma) a + or - character is present to indicate
		/// ascending or descending sort order for that property.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestSearch (System.String SearchCriteria, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultSearch callback);

		/// <summary>
		/// Allow ICpMedia container objects to request that a new child object be
		/// created such that it references another object in the content hierarchy.
		/// </summary>
		/// <param name="referencedItem">the new child item should point to this</param>
		/// <param name="Tag">user-provided object, for convenience in tracking the asynchronous results</param>
		/// <param name="callback">callback executes when the results are available</param>
		/// <exception cref="UPnPInvokeException">
		/// Thrown when attempting to create a reference to a container.
		/// </exception>
		void RequestCreateReference(ICpMedia referencedItem, object Tag, CpMediaDelegates.Delegate_ResultCreateReference callback);

		/// <summary>
		/// Makes a request to a remote media server to create a new child object
		/// as a child of this container.
		/// </summary>
		/// <param name="newObject">
		/// The CDS-compliant metadata that represents the new object.
		/// The argument will not be completely DIDL-Lite valid, in accordance
		/// with CreateObject rules described in the ContentDirectory specification.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestCreateObject(IUPnPMedia newObject, object Tag, CpMediaDelegates.Delegate_ResultCreateObject callback);

		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// the resource URI represented by this object. The difference with this version
		/// is that it's primarily intended to be used when importing resources after
		/// doing a CreateObject.
		/// </summary>
		/// <param name="sourceUri">the URI where binary should be pulled</param>
		/// <param name="importHere">the <see cref="IMediaResource"/> object where the binary should go: assumes importUri is valid</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestImportResource2(System.Uri sourceUri, IMediaResource importHere, object Tag, CpMediaDelegates.Delegate_ResultImportResource2 callback);

	}

	/// <summary>
	/// Declares delegates used by <see cref="ICpMedia"/>
	/// </summary>
	public abstract class CpMediaDelegates
	{
		/// <summary>
		/// Delegate defines the signature for a callback designed to process the results of
		/// an <see cref="ICpMedia.RequestBrowse"/>() call.
		/// <para>
		/// The "mediaObjects" argument is an array of <see cref="IUPnPMedia"/> instead of <see cref="ICpMedia"/>
		/// because the resulting media objects are simple metadata representations of the 
		/// results. <see cref="ICpMedia"/> objects infer a degree of consistency in mirroring
		/// a remote content hierarchy with a local copy, and an application's direct use of
		/// the Browse method does not lend itself to ensuring such consistency between media objects.
		/// </para>
		/// <para>
		/// Should an application require the use of <see cref="ICpMedia"/> objects, it should use 
		/// <see cref="ICpMedia"/> container objects directly and call the <see cref="ICpMedia.Update"/>
		/// implementation on such objects. In the case <see cref="CpMediaContainer"/> objects,
		/// the application needs to use <see cref="CdsSpider"/> objects to properly obtain child
		/// <see cref="ICpMedia"/> objects.
		/// </para>
		/// </summary>
		public delegate void Delegate_ResultBrowse(ICpMedia browseThis, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, string Filter, uint StartIndex, uint RequestCount, string SortCriteria, string ResultXml, IUPnPMedia[] mediaObjects, uint NumberReturned, uint TotalMatches, uint UpdateID, object Tag, UPnPInvokeException error, Exception resultException);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the results of
		/// an <see cref="ICpContainer.RequestSearch"/>() call.
		/// <para>
		/// The "mediaObjects" argument is an array of <see cref="IUPnPMedia"/> instead of <see cref="ICpMedia"/>
		/// because the resulting media objects are simple metadata representations of the 
		/// results. <see cref="ICpMedia"/> objects infer a degree of consistency in mirroring
		/// a remote content hierarchy with a local copy, and an application's direct use of
		/// the Search method does not lend itself to ensuring such consistency between media objects. 
		/// </para>
		/// <para>
		/// Should an application require the use of <see cref="ICpMedia"/> objects, it should use 
		/// <see cref="ICpMedia"/> container objects directly and call the <see cref="ICpMedia.Update"/>
		/// implementation on such objects. In the case <see cref="CpMediaContainer"/> objects,
		/// the application needs to use <see cref="CdsSpider"/> objects to properly obtain child
		/// <see cref="ICpMedia"/> objects.
		/// </para>
		/// </summary>
		public delegate void Delegate_ResultSearch(ICpContainer searchFrom, System.String SearchCriteria, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, System.String Result, IUPnPMedia[] mediaObjects, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID, object _Tag, UPnPInvokeException e, Exception resultException);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the results of
		/// an <see cref="ICpMedia.RequestUpdateObject"/>() call.
		/// </summary>
		public delegate void Delegate_ResultUpdateObject(ICpMedia attemptChangeOnThis, IUPnPMedia usedThisMetadata, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of
		/// an <see cref="ICpMedia.RequestDestroyObject"/> call.
		/// </summary>
		public delegate void Delegate_ResultDestroyObject(ICpMedia destroyThis, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="ICpContainer.RequestCreateReference"/> call.
		/// </summary>
		public delegate void Delegate_ResultCreateReference(ICpMedia parent, ICpMedia referencedItem, string NewID, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegates defines the signature for a callback designed to process the result of an
		/// <see cref="ICpMedia.RequestDeleteResource"/> or <see cref="ICpResource.RequestDeleteResource"/>
		/// method call.
		/// </summary>
		public delegate void Delegate_ResultDeleteResource(ICpMedia owner, ICpResource requestDeleteOnThis, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="ICpMedia.RequestImportResource"/> or <see cref="ICpResource.RequestImportResource"/>
		/// method call.
		/// </summary>
		public delegate void Delegate_ResultImportResource(System.Uri importFromThis, ICpMedia owner, ICpResource importToThis, IResourceTransfer transferObject, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="ICpContainer.RequestImportResource2"/>.
		/// </summary>
		public delegate void Delegate_ResultImportResource2(System.Uri importFromThis, IUPnPMedia owner, IMediaResource importToThis, IResourceTransfer transferObject, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="ICpContainer.RequestCreateObject"/> method call.
		/// </summary>
		public delegate void Delegate_ResultCreateObject(ICpContainer parent, IUPnPMedia newObject, string newObjectID, string ResultXml, IUPnPMedia returnedObject, object Tag, UPnPInvokeException error, Exception xmlToObjectError);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of a
		/// extended version of <see cref="ICpContainer.RequestCreateObject"/>, where 
		/// one or more branches can be specified for creation in a single call.
		/// </summary>
		public delegate void Delegate_ResultCreateObjectEx(ICpMedia parent, IList newBranches, string newObjectIDs, string ResultXml, IList returnedBranches, object Tag, UPnPInvokeException error, Exception xmlToObjectErrors);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="ICpMedia.RequestExportResource"/> method call.
		/// </summary>
		public delegate void Delegate_ResultExportResource(ICpResource exportThis, System.Uri sendHere, IResourceTransfer transferObject, object Tag, UPnPInvokeException error);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="IResourceTransfer.RequestGetTransferProgress"/> method call.
		/// </summary>
		public delegate void Delegate_ResultGetTransferProgress(IResourceTransfer transferObject, CpContentDirectory.Enum_A_ARG_TYPE_TransferStatus transferStatus, System.Int64 transferLength, System.Int64 transferTotal, object Tag, UPnPInvokeException error, Exception castError);

		/// <summary>
		/// Delegate defines the signature for a callback designed to process the result of an
		/// <see cref="IResourceTransfer.RequestStopTransferResource"/> method call.
		/// </summary>
		public delegate void Delegate_ResultStopTransferResource(IResourceTransfer transferObject, object Tag, UPnPInvokeException error);

	}
}
