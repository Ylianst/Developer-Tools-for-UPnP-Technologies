using System;
using System.Collections;
using Intel.UPNP;
using Intel.Utilities;
using Intel.UPNP.AV;
using Intel.UPNP.AV.CdsMetadata;

namespace Intel.UPNP.AV.MediaServer.CP
{
	/// <summary>
	/// Summary description for CdsContentFinder.
	/// </summary>
	public class CdsContentFinder
	{
		/// <summary>
		/// Class is used for storing state during an action invocation.
		/// </summary>
		private class _RequestState
		{
			public Delegate_OnBrowseDone Callback;
			public object Tag;
			public CpMediaServer Server;
		}

		/// <summary>
		/// Delegate for reporting when lsit of mediaservers change.
		/// </summary>
		public delegate void Delegate_OnMediaServersChange(CdsContentFinder sender);

		/// <summary>
		/// Delegate used for asynchronously processing browse commands.
		/// </summary>
		public delegate void Delegate_OnBrowseDone(CpMediaServer server, UPnPInvokeException e, Exception parseError, object _Tag, IUPnPMedia[] Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID);

		/// <summary>
		/// Fires whenever the list of MediaServers change.
		/// </summary>
		public event Delegate_OnMediaServersChange OnMediaServersChanged;

		public CdsContentFinder()
		{
			m_MediaServers = new MediaServerDiscovery(null,null,
			  new MediaServerDiscovery.Delegate_OnGoodServersChange(Sink_OnServerAdded),
			  new MediaServerDiscovery.Delegate_OnGoodServersChange(Sink_OnServerRemoved)
			  );
		}

		/// <summary>
		/// Used to track active media servers.
		/// </summary>
		private MediaServerDiscovery m_MediaServers = null;

		private void Sink_OnServerAdded (MediaServerDiscovery sender, CpMediaServer server)
		{
			if (this.OnMediaServersChanged != null)
			{
				this.OnMediaServersChanged(this);
			}
		}
		private void Sink_OnServerRemoved (MediaServerDiscovery sender, CpMediaServer server)
		{
			if (this.OnMediaServersChanged != null)
			{
				this.OnMediaServersChanged(this);
			}
		}


		/// <summary>
		/// Returns the list of known MediaServers.
		/// </summary>
		public CpMediaServer[] MediaServers
		{
			get
			{
				return this.m_MediaServers.GoodServers;
			}
		}

		public void Browse(CpMediaServer server, System.String ObjectID, Intel.UPNP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object _Tag, Delegate_OnBrowseDone _Callback)
		{
			_RequestState state = new _RequestState();
			state.Callback = _Callback;
			state.Tag = _Tag;
			state.Server = server; 
			server.ContentDirectory.Browse(ObjectID, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, state, new Intel.UPNP.AV.CpContentDirectory.Delegate_OnResult_Browse(_OnBrowseDone));
		}

		private void _OnBrowseDone (CpContentDirectory sender, System.String ObjectID, Intel.UPNP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, System.String Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID, UPnPInvokeException e, object _Tag)
		{
			_RequestState state = (_RequestState) _Tag;
			if (e != null)
			{
				state.Callback(state.Server, e, null, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
			}
			else
			{
				try
				{
					ArrayList al = CpMediaBuilder.BuildMediaBranches(Result);
					state.Callback(state.Server, e, null, state.Tag, (IUPnPMedia[]) al.ToArray(typeof(IUPnPMedia)), NumberReturned, TotalMatches, UpdateID);
				}
				catch (Exception parseError)
				{
					state.Callback(state.Server, e, parseError, state.Tag, null, NumberReturned, TotalMatches, UpdateID);
				}
			}
		}
	}
}
