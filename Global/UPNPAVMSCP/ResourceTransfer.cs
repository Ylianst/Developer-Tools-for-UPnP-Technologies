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

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Interface for obtaining information about the result/progress of an
	/// ImportResource or ExportResource request from an object.
	/// </summary>
	public interface IResourceTransfer
	{
		/// <summary>
		/// The ID used by a MediaServer to represent the transfer of this binary
		/// </summary>
		System.UInt32 TransferID { get; }

		/// <summary>
		/// Allows the programmer to query the progress of this transfer object.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestGetTransferProgress(object Tag, CpMediaDelegates.Delegate_ResultGetTransferProgress callback);

		/// <summary>
		/// Allows the programmer to request that a transfer stop.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestStopTransferResource (object Tag, CpMediaDelegates.Delegate_ResultStopTransferResource callback);
	}

	/// <summary>
	/// This class allows a programmer to easily obtain the status and progress of
	/// transfer initiated through the CDS ImportResource or ExportResource.
	/// </summary>
	public sealed class ResourceTransfer : IResourceTransfer
	{
		/// <summary>
		/// internal constructor - public programmers should not instantiate
		/// their own transfer objects.
		/// </summary>
		/// <param name="transferID">
		/// The ID provided by the server.
		/// </param>
 		/// <param name="isImport"></param>
		/// <param name="res"></param>
		/// <param name="uri"></param>
		/// <param name="server"></param>
		internal ResourceTransfer(System.UInt32 transferID, bool isImport, IMediaResource res, System.Uri uri, CpMediaServer server)
		{
			this.m_TransferID = transferID;
			this.m_IsImport = isImport;
			this.m_Resource = res;
			this.m_Uri = uri;
			this.mwr_Server = new WeakReference(server);
		}
		
		/// <summary>
		/// Allows the programmer to query the progress of this transfer object.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="ApplicationException">
		/// Thrown if the resource object associated with thsi transfer is null. 
		/// Such an exception would indicate that the system has improperly instantiated the transfer object.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown if the transfer object cannot find its media server.
		/// </exception>
		public void RequestGetTransferProgress (object Tag, CpMediaDelegates.Delegate_ResultGetTransferProgress callback)
		{
			if (this.m_Resource == null)
			{
				throw new ApplicationException("Bad Evil. The resource associated with this transfer object is null.");
			}

			CpMediaServer server = (CpMediaServer) this.mwr_Server.Target;
			if(this.mwr_Server.IsAlive == false)
			{
				throw new NullReferenceException("The media server object is null. Media server may have disappeared from UPnP network.");
			}

			GetTransferProgressRequestTag rtag = new GetTransferProgressRequestTag();
			rtag.Tag = Tag;
			rtag.Callback = callback;

			server.RequestGetTransferProgress(this.m_TransferID, rtag, new CpContentDirectory.Delegate_OnResult_GetTransferProgress(this.SinkResult_GetTransferProgress));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="ResourceTransfer.RequestGetTransferProgress"/>
		/// when the results return from <see cref="ResourceTransfer.RequestGetTransferProgress"/>.
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="TransferID">caller specified ID of the transfer at invoke time</param>
		/// <param name="TransferStatus">output status for the transfer</param>
		/// <param name="TransferLength">output progress of the transfer</param>
		/// <param name="TransferTotal">output expected length of the transfer</param>
		/// <param name="e">errors reported by media server</param>
		/// <param name="_Tag">
		/// A <see cref="GetTransferProgressRequestTag"/> object that has information about the callback
		/// and user provided state.
		/// </param>
		private void SinkResult_GetTransferProgress (CpContentDirectory sender, System.UInt32 TransferID, CpContentDirectory.Enum_A_ARG_TYPE_TransferStatus TransferStatus, System.String TransferLength, System.String TransferTotal, UPnPInvokeException e, object _Tag)
		{
			GetTransferProgressRequestTag rtag = (GetTransferProgressRequestTag) _Tag;

			if (rtag.Callback != null)
			{
				System.Int64 length = -1;
				System.Int64 total = -1;
				
				Exception castError = null;
				try
				{
					System.Int64.Parse(TransferLength);
					System.Int64.Parse(TransferTotal);
				}
				catch (Exception ce)
				{
					castError = ce;
				}

				rtag.Callback(this, TransferStatus, length, total, rtag.Tag, e, castError);
			}
		}

		/// <summary>
		/// Allows the programmer to request that a transfer stop.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="ApplicationException">
		/// Thrown if the resource object associated with thsi transfer is null. 
		/// Such an exception would indicate that the system has improperly instantiated the transfer object.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown if the transfer object cannot find its media server.
		/// </exception>
		public void RequestStopTransferResource (object Tag, CpMediaDelegates.Delegate_ResultStopTransferResource callback)
		{
			if (this.m_Resource == null)
			{
				throw new ApplicationException("Bad Evil. The resource associated with this transfer object is null.");
			}

			CpMediaServer server = (CpMediaServer) this.mwr_Server.Target;
			if(this.mwr_Server.IsAlive == false)
			{
				throw new NullReferenceException("The media server object is null. Media server may have disappeared from UPnP network.");
			}

			StopTransferResourceRequestTag rtag = new StopTransferResourceRequestTag();
			rtag.Tag = Tag;
			rtag.Callback = callback;

			server.RequestStopTransferResource (this.m_TransferID, rtag, new CpContentDirectory.Delegate_OnResult_StopTransferResource(this.SinkResult_StopTransferResource));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="ResourceTransfer.RequestStopTransferResource"/>
		/// when the results return from <see cref="ResourceTransfer.RequestStopTransferResource"/>.
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="TransferID">the caller-specified ID of the transfer to stop</param>
		/// <param name="e">media server reported errors</param>
		/// <param name="_Tag">
		/// A <see cref="StopTransferResourceRequestTag"/> object that has information about the callback
		/// and user provided state.
		/// </param>
		private void SinkResult_StopTransferResource (CpContentDirectory sender, System.UInt32 TransferID, UPnPInvokeException e, object _Tag)
		{
			StopTransferResourceRequestTag rtag = (StopTransferResourceRequestTag) _Tag;

			if (rtag.Callback != null)
			{
				rtag.Callback(this, rtag.Tag, e);
			}
		}
 
		/// <summary>
		/// Read-only value of the ID's transfer
		/// </summary>
		public System.UInt32 TransferID
		{
			get
			{
				return this.m_TransferID;
			}
		}

		public string SourceUri
		{
			get
			{
				if (this.m_IsImport)
				{
					return this.m_Uri.ToString();
				}
				else
				{
					return this.m_Resource.ContentUri;
				}
			}
		}

		public string DestinationUri
		{
			get
			{
				if (this.m_IsImport)
				{
					return this.m_Resource.ImportUri;
				}
				else
				{
					return this.m_Uri.ToString();
				}
			}
		}

		/// <summary>
		/// The ID of the transfer, returned by the Import/Export Resource request.
		/// </summary>
		public System.UInt32 m_TransferID;

		/// <summary>
		/// The resource related to the request to import or export.
		/// If <see cref="m_IsImport"/> is true, then this field is the destination.
		/// Otherwise, this field is providing the source binary.
		/// </summary>
		private IMediaResource m_Resource;

		/// <summary>
		/// The URI related to the request to import or export.
		/// If <see cref="m_IsImport"/> is true, then this field is providing the source binary.
		/// Otherwise, this field is the destination of an export.
		/// </summary>
		private System.Uri m_Uri;

		/// <summary>
		/// If true, then the request was an import. If false, then the request was as export.
		/// </summary>
		private bool m_IsImport;

		/// <summary>
		/// Weak ref to the server that can make request. The reason for keeping a weak
		/// reference is taht media server implementations might report the removal of 
		/// a media object but still keep a pending transfer object valid.
		/// </summary>
		private WeakReference mwr_Server;

		/// <summary>
		/// Used strictly for routing <see cref="ResourceTransfer.RequestGetTransferProgress"/>.
		/// </summary>
		private struct GetTransferProgressRequestTag
		{
			public object Tag;
			public CpMediaDelegates.Delegate_ResultGetTransferProgress Callback;
		}
		/// <summary>
		/// Used strictly for routing <see cref="ResourceTransfer.RequestGetTransferProgress"/>.
		/// </summary>
		private struct StopTransferResourceRequestTag
		{
			public object Tag;
			public CpMediaDelegates.Delegate_ResultStopTransferResource Callback;
		}
	}
}
