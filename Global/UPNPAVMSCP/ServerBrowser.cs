using System;
using System.Collections;
using Intel.UPNP;
using Intel.UPNP.AV;
using Intel.UPNP.AV.CdsMetadata;
using Intel.UPNP.AV.MediaServer;
using Intel.UPNP.AV.MediaServer.CP;
using System.Windows.Forms;

namespace Intel.UPNP.AV.MediaServer.CP
{
	/// <summary>
	/// Summary description for ServerBrowser.
	/// </summary>
	public class ServerBrowser
	{
		public delegate void Delegate_ContentFound(ServerBrowser sender, IUPnPMedia[] added);

		public event Delegate_ContentFound OnIncrementalUpdate; 
		public event Delegate_ContentFound OnRefreshComplete;

		private CpMediaServer m_Server = null;
		private uint m_BrowseSize = 20;
		private uint m_CurrentIndex = 0;
		private string m_Filter = "*";
		private string m_SortString = "";
		private string m_Context = null;
		private IMediaContainer m_Container = null;
		private ArrayList m_Children = new ArrayList();

		/// <summary>
		/// Sets the server that this object browses.
		/// Must set <see cref="ServerBrowser.Context"/>
		/// and call <see cref="Refresh"/>.
		/// </summary>
		public CpMediaServer Server
		{
			get
			{
				return this.m_Server;
			}
			set
			{
				if (this.m_Server != value)
				{
					this.m_Server = value;
					this.m_Container = this.m_Server.Root;
				}
			}
		}

		/// <summary>
		/// <para>
		/// Changes this object's browsing context to provided container.
		/// Must set <see cref="ServerBrowser.Server"/> beforehand
		/// and must call <see cref="ServerBrowser.Refresh"/> afterwards.
		/// </para>
		/// </summary>
		public IMediaContainer Context
		{
			get
			{
				return this.m_Container;
			}
			set
			{
				CpRootCollectionContainer cprc = value as CpRootCollectionContainer;
				this.m_Container = value;

				if (cprc == null)
				{
					this.SetContext(this.m_Container.ID);
				}
				else
				{
					this.SetContext(null);
				}
			}
		}

		public void Refresh()
		{
			CpRootCollectionContainer cprc = this.m_Container as CpRootCollectionContainer;

			if (cprc == null)
			{
				lock (this)
				{
					this.m_CurrentIndex = 0;
					this.m_Server.RequestBrowse(
						this.m_Context,	
						CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN,
						this.m_Filter,
						this.m_CurrentIndex,
						this.m_BrowseSize,
						this.m_SortString,
						this.m_Container,
						new CpMediaServer.Delegate_OnBrowseDone1 (Sink_OnBrowse)
						);
				}
			}
		}

		/// <summary>
		/// Manually sets the context, using the provided objectID value.
		/// </summary>
		/// <param name="context">objectID of a container object</param>
		private void SetContext(string context)
		{
			this.m_Context = context;
		}

		private void DoNextBrowse()
		{
			lock (this)
			{
				this.m_Server.RequestBrowse(
					this.m_Context,	
					CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN,
					this.m_Filter,
					this.m_CurrentIndex,
					this.m_BrowseSize,
					this.m_SortString,
					this.m_Container,
					new CpMediaServer.Delegate_OnBrowseDone1 (Sink_OnBrowse)
					);
			}
		}

		/// <summary>
		/// Returns the currentcontext (objectID) in string form.
		/// </summary>
		/// <param name="context"></param>
		private string GetContext(string context)
		{
			return this.m_Context;
		}

		private void Sink_OnBrowse(CpMediaServer server, System.String ObjectID, Intel.UPNP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, UPnPInvokeException e, Exception parseError, object _Tag, IUPnPMedia[] Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID)
		{
			if (server == this.m_Server)
			{
				if (ObjectID == this.m_Context)
				{
					if (_Tag == this.m_Container)
					{
						if ((e != null) || (parseError != null))
						{
							//error encountered
						}
						else
						{
							// add children 
							this.m_Children.AddRange(Result);
							this.m_Container.AddObjects(Result, true);

							if (this.OnIncrementalUpdate != null)
							{
								this.OnIncrementalUpdate(this, Result);
							}

							if (
								((this.m_Children.Count == TotalMatches) && (NumberReturned > 0)) ||
								((TotalMatches == 0) && (NumberReturned > 0))
								)
							{
								// more items to come
								this.m_CurrentIndex = NumberReturned;
								DoNextBrowse();
							}
							else
							{
								lock (this)
								{
									// no more items, prune children from m_Container
									ArrayList remove = new ArrayList();
									foreach (IUPnPMedia m1 in this.m_Container.CompleteList)
									{
										bool found = false;
										foreach (IUPnPMedia m2 in this.m_Children)
										{
											if (m1 == m2)
											{
												found = true;
												break;
											}
										}
										if (found == false)
										{
											remove.Add(m1);
										}
									}
									this.m_Container.RemoveObjects(remove);
								}

								if (this.OnRefreshComplete != null)
								{
									IUPnPMedia[] list = (IUPnPMedia[]) this.m_Children.ToArray(typeof(IUPnPMedia));
									this.OnRefreshComplete(this, list);
								}
							}
						}
					}
				}
			}
		}

		/// <summary>
		/// Changes the number of objects that this objects requests
		/// when browsing a server. The change is immediate, causing
		/// all subsequent browse requests to use the new browsing size.
		/// </summary>
		public uint BrowseSize
		{
			get
			{
				return this.m_BrowseSize;
			}
			set
			{
				this.m_BrowseSize = value;
			}
		}
	}
}
