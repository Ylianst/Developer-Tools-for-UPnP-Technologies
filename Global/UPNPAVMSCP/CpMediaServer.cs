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
using System.Text;
using OpenSource.UPnP;
using System.Threading;
using OpenSource.UPnP.AV;
using System.Collections;
using OpenSource.Utilities;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// This class aggregates the objects that abstract the ConnectionManager and ContentDirectory 
	/// services to logically represent a MediaServer.
	/// TODO: Should eventually add AVTransport.
	/// </summary>
	public class CpMediaServer 
	{
		~CpMediaServer()
		{
			this.m_Root = null;
		}

		public CpMediaServer()
		{
			this.ServerID = System.Threading.Interlocked.Increment(ref NextServerID);
		}

		/// <summary>
		/// UDN's and guid values are long, so we'll just use a number
		/// that is unique enough to be able to ID a server.
		/// The value is guaranteed to be monotomically increasing.
		/// </summary>
		public readonly long ServerID;
		public readonly string UDN;


		private static long NextServerID = 0;

		/// <summary>
		/// Creates a programmer-friendly object for using a device
		/// that happens implement a MediaServer.
		/// </summary>
		/// <param name="device"></param>
		public CpMediaServer(UPnPDevice device)
		{
			UPnPService sCM = device.GetServices(CpConnectionManager.SERVICE_NAME)[0];
			UPnPService sCD = device.GetServices(CpContentDirectory.SERVICE_NAME)[0];

			CpConnectionManager cpCM = new CpConnectionManager(sCM);
			CpContentDirectory cpCD = new CpContentDirectory(sCD);

			UDN = device.UniqueDeviceName;

			if (
				(cpCD.HasAction_GetSearchCapabilities == false) ||
				(cpCD.HasAction_GetSortCapabilities == false) ||
				(cpCD.HasAction_GetSystemUpdateID == false) ||
				(cpCD.HasAction_Browse == false)
				)
			{
				throw new UPnPCustomException(0, "MediaServer does not implement minimum features.");
			}

			this.m_ConnectionManager = cpCM;
			this.m_ContentDirectory = cpCD;
			
			//create a virtualized root container with the desired settings
			m_Root = new CpRootContainer(this);
			m_Root.UDN = device.UniqueDeviceName;
		}

		/// <summary>
		/// The root container for the server.
		/// </summary>
		public CpRootContainer Root 
		{ 
			get 
			{ 
				return this.m_Root; 
			} 
		}

		/// <summary>
		/// Class is used for storing state during an action invocation.
		/// </summary>
		private class _RequestState
		{
			public Delegate_OnBrowseDone1 Callback_Browse1;
			public Delegate_OnBrowseDone2 Callback_Browse2;
			public object Tag;
		}

		/// <summary>
		/// Delegate used for asynchronously processing results of RequestBrowse.
		/// </summary>
		public delegate void Delegate_OnBrowseDone1(CpMediaServer server, System.String ObjectID, OpenSource.UPnP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, UPnPInvokeException e, Exception parseError, object _Tag, IUPnPMedia[] Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID);

		/// <summary>
		/// Delegate used for asynchronously processing results of RequestBrowse.
		/// </summary>
		public delegate void Delegate_OnBrowseDone2(CpMediaServer server, UPnPInvokeException e, Exception parseError, object _Tag, IUPnPMedia[] Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID);

		/// <summary>
		/// Allows the programmer to query the MediaServer through the Browse action.
		/// </summary>
		/// <param name="ObjectID"></param>
		/// <param name="BrowseFlag"></param>
		/// <param name="Filter"></param>
		/// <param name="StartingIndex"></param>
		/// <param name="RequestedCount"></param>
		/// <param name="SortCriteria"></param>
		/// <param name="_Tag"></param>
		/// <param name="callback">Returns input args, output args, and error info.</param>
		public void RequestBrowse (System.String ObjectID, OpenSource.UPnP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object _Tag, Delegate_OnBrowseDone1 callback)
		{
			_RequestState state = new _RequestState();
			state.Callback_Browse1 = callback;
			state.Tag = _Tag;
			this.ContentDirectory.Browse(ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, state, new OpenSource.UPnP.AV.CpContentDirectory.Delegate_OnResult_Browse(OnBrowseDone));
		}

		/// <summary>
		/// Allows a programmer to query the MediaServer through the Browse action.
		/// </summary>
		/// <param name="ObjectID"></param>
		/// <param name="BrowseFlag"></param>
		/// <param name="Filter"></param>
		/// <param name="StartingIndex"></param>
		/// <param name="RequestedCount"></param>
		/// <param name="SortCriteria"></param>
		/// <param name="_Tag"></param>
		/// <param name="callback">Returns output args and error info.</param>
		public void RequestBrowse (System.String ObjectID, OpenSource.UPnP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object _Tag, Delegate_OnBrowseDone2 callback)
		{
			_RequestState state = new _RequestState();
			state.Callback_Browse2 = callback;
			state.Tag = _Tag;
			this.ContentDirectory.Browse(ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, state, new OpenSource.UPnP.AV.CpContentDirectory.Delegate_OnResult_Browse(OnBrowseDone));
		}

		/// <summary>
		/// Processes the results of Browse requests.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="ObjectID"></param>
		/// <param name="BrowseFlag"></param>
		/// <param name="Filter"></param>
		/// <param name="StartingIndex"></param>
		/// <param name="RequestedCount"></param>
		/// <param name="SortCriteria"></param>
		/// <param name="Result"></param>
		/// <param name="NumberReturned"></param>
		/// <param name="TotalMatches"></param>
		/// <param name="UpdateID"></param>
		/// <param name="e"></param>
		/// <param name="_Tag"></param>
		private void OnBrowseDone (CpContentDirectory sender, System.String ObjectID, OpenSource.UPnP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, System.String Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID, UPnPInvokeException e, object _Tag)
		{
			_RequestState state = (_RequestState) _Tag;
			if (e != null)
			{
				if (state.Callback_Browse1 != null)
				{
					state.Callback_Browse1(this, ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, e, null, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
				}
				else if (state.Callback_Browse2 != null)
				{
					state.Callback_Browse2(this, e, null, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
				}
			}
			else
			{
				ArrayList al = null;
				try
				{
					al = CpMediaBuilder.BuildMediaBranches(Result);
				}
				catch (Exception parseError)
				{
					if (state.Callback_Browse1 != null)
					{
						state.Callback_Browse1(this, ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, e, parseError, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
					}
					else if (state.Callback_Browse2 != null)
					{
						state.Callback_Browse2(this, e, parseError, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
					}
					al = null;
				}

				if (al != null)
				{
					if (state.Callback_Browse1 != null)
					{
						state.Callback_Browse1(this, ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, e, null, state.Tag, (IUPnPMedia[]) al.ToArray(typeof(IUPnPMedia)), NumberReturned, TotalMatches, UpdateID);
					}
					else if (state.Callback_Browse2 != null)
					{
						state.Callback_Browse2(this, e, null, state.Tag, (IUPnPMedia[]) al.ToArray(typeof(IUPnPMedia)), NumberReturned, TotalMatches, UpdateID);
					}
				}
			}
		}

		/// <summary>
		/// Allows the programmer to query the progress of this transfer object.
		/// </summary>
		/// <param name="TransferID">
		/// The transfer to obtain information on, identified by its ID,
		/// which can be obtained from the CDS TransferIDs comma-separated value list
		/// state variable.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		public void RequestGetTransferProgress (uint TransferID, object Tag, CpContentDirectory.Delegate_OnResult_GetTransferProgress callback)
		{
			this.ContentDirectory.GetTransferProgress(TransferID, Tag, callback);
		}

		/// <summary>
		/// Allows the programmer to request that a transfer stop.
		/// </summary>
		/// <param name="TransferID">
		/// The transfer to stop, identified by its ID,
		/// which can be obtained from the CDS TransferIDs comma-separated value list
		/// state variable.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		public void RequestStopTransferResource (uint TransferID, object Tag, CpContentDirectory.Delegate_OnResult_StopTransferResource callback)
		{
			this.ContentDirectory.StopTransferResource(TransferID, Tag, callback);
		}

		/// <summary>
		/// Requests the remote media server to provide its search capabilities.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		public void RequestGetSearchCapabilities (object Tag, CpContentDirectory.Delegate_OnResult_GetSearchCapabilities callback)
		{
			this.ContentDirectory.GetSearchCapabilities(Tag, callback);
		}

		/// <summary>
		/// Requests the remote media server to provide its sort capabilities.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		public void RequestGetSortCapabilities (object Tag, CpContentDirectory.Delegate_OnResult_GetSortCapabilities callback)
		{
			this.ContentDirectory.GetSortCapabilities(Tag, callback);
		}

		/// <summary>
		/// Requests the remote media server to provide its SystemUpdateID value.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		public void RequestGetSystemUpdateID (object Tag, CpContentDirectory.Delegate_OnResult_GetSystemUpdateID callback)
		{
			this.ContentDirectory.GetSystemUpdateID(Tag, callback);
		}

		/// <summary>
		/// The ConnectionManager service to use when invoking methods on the server.
		/// </summary>
		public CpConnectionManager ConnectionManager { get { return this.m_ConnectionManager; } }
		/// <summary>
		/// The ContentDirectory service to use when invoking methods on the server.
		/// </summary>
		public CpContentDirectory ContentDirectory { get { return this.m_ContentDirectory; } }

		/// <summary>
		/// The ConnectionManager service to use when invoking methods on the server.
		/// </summary>
		private CpConnectionManager m_ConnectionManager;
		/// <summary>
		/// The ContentDirectory service to use when invoking methods on the server.
		/// </summary>
		private CpContentDirectory m_ContentDirectory;

		/// <summary>
		/// The root container for the server.
		/// </summary>
		private CpRootContainer m_Root;
	}

}