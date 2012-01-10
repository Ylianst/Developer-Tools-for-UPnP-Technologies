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
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Thrown when attempting to set the URI of a resource that maps to
	/// a local file intended for serving with http-get from the internal
	/// webserver of a
	/// <see cref="MediaServerDevice"/>
	/// object.
	/// <see cref="DvMediaResource"/> uses this exception.
	/// </summary>
	public class Error_CannotSetImportUri : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor specific UPNP error code of 875.
		/// </summary>
		/// <param name="contentUri">The contentUri value of the MediaResource when the importUri modification was attempted.</param>
		/// <param name="importUri">The value of the intended importUri.</param>
		public Error_CannotSetImportUri(string contentUri, string importUri)
			: base(875, "Cannot set the importUri value for a resource that maps to ("+contentUri+"). Set the ContentUri instead.")
		{
			ContentUri = contentUri;
			ImportUri = importUri;
		}

		/// <summary>
		/// The contentUri value of the MediaResource when the importUri was attempted for modification.
		/// </summary>
		public string ContentUri;
		
		/// <summary>
		/// The value of the intended importUri.
		/// </summary>
		public string ImportUri;
	}

	/// <summary>
	/// Thrown when attempting to set the ProtocolInfo string of a resource
	/// if the protocolInfo string is not "http-get", when the contentUri
	/// starts with 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>.
	/// <see cref="DvMediaResource"/> uses this exception.
	/// </summary>
	public class Error_CannotSetProtocolInfo : UPnPCustomException
	{

		/// <summary>
		/// Indicates vendor specific UPNP error code of 876.
		/// </summary>
		/// <param name="contentUri">The contentUri value of the MediaResource when the protocolInfo modification was attempted.</param>
		/// <param name="protocolInfo">The value of the intended protocolInfo string.</param>
		public Error_CannotSetProtocolInfo(string contentUri, ProtocolInfoString protocolInfo)
			: base (876, "Cannot set the protocolInfo string (" +protocolInfo.ToString()+ ") to something other than \"http-get\", if the contentUri value starts with (" +MediaResource.AUTOMAPFILE+").")
		{
			this.ContentUri = contentUri;
			this.ProtocolInfo = protocolInfo;
		}

		/// <summary>
		/// The contentUri value of the MediaResource when the protocolInfo modification was attempted.
		/// </summary>
		public readonly string ContentUri;

		/// <summary>
		/// The value of the intended protocolInfo string.
		/// </summary>
		public readonly ProtocolInfoString ProtocolInfo;
	}

	/// <summary>
	/// Thrown when attempting to set the contentUri of a resource if the newUri string begins with 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>
	/// and the
	/// protocol is not "http-get".
	/// <see cref="DvMediaResource"/> uses this exception.
	/// </summary>
	public class Error_CannotSetContentUri : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor specific UPNP error code of 877.
		/// </summary>
		public Error_CannotSetContentUri(ProtocolInfoString protocolInfo, string contentUri)
			: base (877, "Cannot set the contentUri ("+contentUri+") to a string beginning with ("+MediaResource.AUTOMAPFILE+") if the protocolInfo ("+protocolInfo.ToString()+") does not indicate a protocol of \"http-get\"")
		{
			this.ContentUri = contentUri;
			this.ProtocolInfo = protocolInfo;
		}

		/// <summary>
		/// The value of the intended contentUri.
		/// </summary>
		public readonly string ContentUri;

		/// <summary>
		/// The protocolInfo value of the MediaResource when the contentUri modification was attempted.
		/// </summary>
		public readonly ProtocolInfoString ProtocolInfo;
	}

	/// <summary>
	/// Thrown when attempting to create a reference to an item that is queued for
	/// deletion from the content hierarchy.
	/// <see cref="DvMediaItem"/> uses this exception.
	/// </summary>
	public class Error_PendingDeleteException : UPnPCustomException 
	{
		/// <summary>
		/// Indicates vendor specific UPNP error code of 878.
		/// </summary>
		/// <param name="item">The item that cannot have a reference created to it.</param>
		public Error_PendingDeleteException(IMediaItem item)
			: base(878, "The item (@id=\""+item.ID+"\") has been deleted or is about to be deleted.")
		{
			Item = item;
		}

		/// <summary>
		/// The item that cannot have a reference created to it.
		/// </summary>
		public readonly IMediaItem Item;
	}

	/// <summary>
	/// Thrown when the MediaServer has not been configured correctly
	/// <see cref="MediaServerDevice"/> uses this exception.
	/// </summary>
	public class Error_InvalidServerConfiguration : UPnPCustomException
	{
		/// <summary>
		/// Indicates UPNP-AV action-specific error code 879.
		/// </summary>
		/// <param name="info">Message with additional info.</param>
		public Error_InvalidServerConfiguration (string info) : base (879, "Invalid server configuration. " + info) {}
	}

	/// <summary>
	/// Thrown when a general error occurs during the import resource process.
	/// </summary>
	public class Error_ImportError : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor-specific error code of 880.
		/// </summary>
		/// <param name="info"></param>
		public Error_ImportError (string info)
			: base(880, info)
		{
		}
	}

	/// <summary>
	/// Thrown when a general error occurred when connecting.
	/// </summary>
	public class Error_ConnectionProblem : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor-specific error code of 881.
		/// </summary>
		/// <param name="info"></param>
		public Error_ConnectionProblem (string info)
			: base (881, "Connection error. "+info)
		{
		}
	}

	/// <summary>
	/// Thrown when ImportResource destination Uri is not HTTP based.
	/// </summary>
	public class Error_NonHttpImport : UPnPCustomException
	{
		public Error_NonHttpImport (string info)
			: base (881, "Cannot import from a URI that is not HTTP based. " + info)
		{
			Info = info;
		}

		public string Info;
	}

	/// <summary>
	/// Thrown when a general error occurred when doing an http-get.
	/// </summary>
	public class Error_GetRequestError : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor-specific error code of 881.
		/// </summary>
		/// <param name="directiveObj"></param>
		/// <param name="res"></param>
		public Error_GetRequestError (string directiveObj, IMediaResource res)
			: base (881, "Error fulfilling request for " + directiveObj)
		{
			Resource = res;
			DirectiveObj = directiveObj;
		}

		/// <summary>
		/// The resource that was requested
		/// </summary>
		public IMediaResource Resource;

		public string DirectiveObj;
	}

	/// <summary>
	/// Thrown when an error occurred when transfering binaries.
	/// </summary>
	public class Error_TransferProblem : UPnPCustomException
	{
		/// <summary>
		/// Indicates vendor-specific error code of 881.
		/// </summary>
		/// <param name="id"></param>
		/// <param name="transferInfo"></param>
		public Error_TransferProblem (UInt32 id, MediaServerDevice.HttpTransfer transferInfo)
			: base(881, "Error with transfer " + id.ToString())
		{
			TransferId = id;
			TransferInfo = transferInfo;
		}

		/// <summary>
		/// the transfer id that was specified
		/// </summary>
		UInt32 TransferId;

		/// <summary>
		/// The object representing the transfer in question.
		/// </summary>
		MediaServerDevice.HttpTransfer TransferInfo;
	}
}
