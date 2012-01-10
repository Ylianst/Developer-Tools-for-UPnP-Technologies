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
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer;
using OpenSource.UPnP.AV.MediaServer.CP;
using System.Windows.Forms;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	public class ContextInfo : ICloneable
	{
		internal Stack m_Context = new Stack();
		internal CpMediaServer m_ServerContext = null;

		/// <summary>
		/// Returns the server that is currently being browsed.
		/// </summary>
		public CpMediaServer ServerContext
		{
			get
			{
				return this.m_ServerContext;
			}
		}

		public IMediaContainer ContainerContext
		{
			get
			{
				return (IMediaContainer) this.m_Context.Peek();
			}
		}
		/// <summary>
		/// Returns the entire stack context as an array.
		/// Returned value is a shallow copy.
		/// </summary>
		public IMediaContainer[] EntireContext
		{
			get
			{
				IMediaContainer[] retVal = new IMediaContainer[this.m_Context.Count];
				this.m_Context.CopyTo(retVal, 0);
				return retVal;
			}
		}

		/// <summary>
		/// Provides a listing of the available contexts that could be added
		/// to the entire/current context. (Essentially, all child containers.)
		/// </summary>
		public IMediaContainer[] ForwardContexts
		{
			get
			{
				IMediaContainer[] mcs = null;
				IList containers = this.ContainerContext.Containers;
				mcs = new IMediaContainer[containers.Count];
				for (int i=0; i < containers.Count; i++)
				{
					mcs[i] = (IMediaContainer) containers[i];
				}
				return mcs;
			}
		}

		/// <summary>
		/// Provides the list of legal contexts that this browser can change to.
		/// Essentially, a merging of <see cref="MediaBrowser.ForwardContexts"/> and
		/// <see cref="MediaBrowser.EntireContext"/>
		/// </summary>
		public IMediaContainer[] AvailableContexts
		{
			get
			{
				int i;
				IMediaContainer[] mcs = null;
				
				IList containers = this.ContainerContext.Containers;
				IMediaContainer[] ec = this.EntireContext;
				
				mcs = new IMediaContainer[containers.Count + ec.Length];
				for (i=0; i < containers.Count; i++)
				{
					mcs[i] = (IMediaContainer) containers[i];
				}
				for (int j=0; j < ec.Length; j++)
				{
					mcs[i] = (IMediaContainer) ec[j];
					i++;
				}
				return mcs;
			}
		}

		#region ICloneable Members

		public object Clone()
		{
			ContextInfo ci = new ContextInfo();
			ci.m_Context = this.m_Context;
			ci.m_ServerContext = this.m_ServerContext;
			return ci;
		}

		#endregion
	}

	/// <summary>
	/// Summary description for MediaBrowser.
	/// </summary>
	public class MediaBrowser
	{
		public delegate void Delegate_ContentFound(MediaBrowser sender, IUPnPMedia[] added);
		public event Delegate_ContentFound OnIncrementalUpdate; 
		public event Delegate_ContentFound OnRefreshComplete;

		private ContainerDiscovery m_Roots = null;

		private ContextInfo m_Context = null;
		private ContextInfo m_TargetContext = null;
		private ContextInfo m_PendingContext = null;

		private uint m_CurrentIndex = 0;
		private uint m_BrowseSize = 20;
		private string m_Filter = "*";
		private string m_SortString = "";

		private ArrayList m_Children = null;
		private ArrayList m_History = null;
		private int m_HistorySize = -1;

		private void Init(int capacity)
		{
			this.m_Children = new ArrayList();
			this.m_Roots = ContainerDiscovery.GetInstance();
			this.m_Context = new ContextInfo();
			this.m_Context.m_Context.Push(this.m_Roots.AllRoots);
			this.m_HistorySize = capacity;
			if (this.m_HistorySize > 0)
			{
				this.m_History = new ArrayList(capacity);
			}
			else
			{
				this.m_History = new ArrayList();
			}
		}

		public MediaBrowser()
		{
			Init(-1);
		}

		public MediaBrowser(int historyCapacity)
		{
			Init(historyCapacity);
		}

		/// <summary>
		/// Returns the current browsing context.
		/// </summary>
		public ContextInfo CurrentContext
		{
			get
			{
				return (ContextInfo) this.m_Context.Clone();
			}
		}

		/// <summary>
		/// Provides the list of contexts that the browser has been used.
		/// Unlike other properties of this class that return contexts, 
		/// this property can return contexts from other MediaServers.
		/// </summary>
		public ContextInfo[] ContextHistory
		{
			get
			{
				return (ContextInfo[]) this.m_History.ToArray(typeof(ContextInfo));
			}
		}

		/// <summary>
		/// Go back/up to the previous context/container.
		/// </summary>
		public void Back()
		{
			if (!(this.CurrentContext is CpRootCollectionContainer))
			{
				lock (this)
				{
					this.m_Context.m_Context.Pop();
					if (this.m_Context.m_Context.Count == 1)
					{
						this.m_Context.m_ServerContext = null;
					}
					this.RefreshChildren();
				}
			}
		}

		/// <summary>
		/// Go back/up to the previous context/container.
		/// </summary>
		/// <param name="thisMany">Go back this many times.</param>
		public void Back(int thisMany)
		{
			if (!(this.CurrentContext is CpRootCollectionContainer))
			{
				lock (this)
				{
					if (thisMany >= this.m_Context.m_Context.Count)
					{
						thisMany = this.m_Context.m_Context.Count - 1;
					}

					while ((thisMany > 0) && (this.m_Context.m_Context.Count > 1))
					{
						this.m_Context.m_Context.Pop();
						thisMany--;
					}
					if (this.m_Context.m_Context.Count == 1)
					{
						this.m_Context.m_ServerContext = null;
					}
					this.RefreshChildren();
				}
			}
		}
		
		private bool IsValidContext(IMediaContainer checkThis, object[] againstThis, bool isContextInfo)
		{
			for (int i=0; i < againstThis.Length; i++)
			{
				if (isContextInfo)
				{
					ContextInfo ci = (ContextInfo) againstThis[i];
					if ((IMediaContainer)(ci.m_Context.Peek()) == checkThis)
					{
						return true;
					}
				}
				else if (againstThis[i] == checkThis)
				{
					return true;
				}
			}

			return false;
		}

		public void SetContext(ContextInfo ci)
		{
			// we'll track our progress using m_PendingContext
			this.m_PendingContext = new ContextInfo();
			this.m_PendingContext.m_Context.Push(this.m_Roots.AllRoots);
			this.m_TargetContext = ci;

			// iterate through the container context in reverse order,
			// try to change context each time
			IList list = ci.EntireContext;
			for (int i= list.Count-1; i > 0; i--)
			{
				//TODO:
			}
		}

		private bool SetContainerContext(ContextInfo ci, IMediaContainer context)
		{
			bool retVal = false;
			bool fwd = false, bk = false;

			// check against forward contexts
			fwd = IsValidContext(context, ci.ForwardContexts, false);

			// check against current/entire context
			if (fwd == false)
			{
				bk = IsValidContext(context, ci.EntireContext, false);
			}

			// if selecting a valid context...
			if (fwd)
			{
				// going forward, so push the desired context
				// to our stack context
				lock (this)
				{
					ci.m_Context.Push(context);

					// if new context is a root container, set the server context
					if ((context.IsRootContainer) && (!(context is CpRootCollectionContainer)))
					{
						CpRootContainer root = context as CpRootContainer;
						if (root != null)
						{
							ci.m_ServerContext = root.Server;
						}
					}
				}
				retVal = true;
			}
			else if (bk)
			{
				lock(this)
				{
					// going back, so pop the stack until
					// we get to the desired context
					// and then refresh
					while ((this.CurrentContext != context) && (this.m_Context.m_Context.Count > 1))
					{
						this.m_Context.m_Context.Pop();
					}
					if (this.m_Context.m_Context.Count == 1)
					{
						this.m_Context.m_ServerContext = null;
					}
				}
				retVal = true;
			}

			return retVal;
		}

		/// <summary>
		/// Sets the current context. The specified context
		/// must be from <see cref="MediaBrowser.AvailableContexts"/>.
		/// If it is not, then nothing will happen.
		/// </summary>
		/// <param name="context"></param>
		public void SetContainerContext(IMediaContainer context)
		{
			if (this.SetContainerContext(this.m_Context, context))
			{
				this.RefreshChildren();
			}
		}

		/// <summary>
		/// Refreshes the children associated with the current context.
		/// </summary>
		public void RefreshChildren()
		{
			if (!(this.CurrentContext is CpRootCollectionContainer))
			{
				// rebrowse current container
				lock (this)
				{
					this.m_CurrentIndex = 0;
					IMediaContainer[] ec = this.CurrentContext.EntireContext;

					// clear listing of current children, since we're rebrowsing
					this.m_Children.Clear();

					this.CurrentContext.ServerContext.RequestBrowse(
						this.CurrentContext.ContainerContext.ID,	
						CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN,
						this.m_Filter,
						this.m_CurrentIndex,
						this.m_BrowseSize,
						this.m_SortString,
						ec,
						new CpMediaServer.Delegate_OnBrowseDone1 (Sink_OnBrowse)
						);
				}
			}
			else
			{
				// update servers
			}
		}

		private void Sink_OnBrowse(CpMediaServer server, System.String ObjectID, OpenSource.UPnP.AV.CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, UPnPInvokeException e, Exception parseError, object _Tag, IUPnPMedia[] Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID)
		{
			IMediaContainer[] tagC, ec; 
			bool ok = true;
			bool nomore = false;

			// ensure we're processing results for the current context
			lock (this)
			{
				tagC = (IMediaContainer[]) _Tag;
				ec = this.CurrentContext.EntireContext;

				for (int i=0; i < ec.Length; i++)
				{
					if (ec[i] != tagC[i])
					{
						ok = false;
						break;
					}
				}
			}

			if (ok)
			{
				// results are for current context, merge metadata results
				// with existing child list

				if ((e == null) && (parseError == null))
				{
					lock (this)
					{
						// add to our media object
						this.m_Children.AddRange(Result);
						this.CurrentContext.ContainerContext.AddObjects(Result, true);

						if (
							((this.m_Children.Count < TotalMatches) && (NumberReturned > 0)) ||
							((TotalMatches == 0) && (NumberReturned > 0))
							)
						{
							// more items to come
							this.m_CurrentIndex = NumberReturned;
						}
						else
						{
							// no more items, prune children from m_Container
							ArrayList remove = new ArrayList();
							foreach (IUPnPMedia m1 in this.CurrentContext.ContainerContext.CompleteList)
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
							this.CurrentContext.ContainerContext.RemoveObjects(remove);
							nomore = true;
						}
					}

					if (this.OnIncrementalUpdate != null)
					{
						this.OnIncrementalUpdate(this, Result);
					}

					if (nomore)
					{
						if (this.OnRefreshComplete != null)
						{
							IUPnPMedia[] list = (IUPnPMedia[]) this.m_Children.ToArray(typeof(IUPnPMedia));
							this.OnRefreshComplete(this, list);
						}
					}
				}
				else
				{
					// error occurred with the results... 
					// how should we report this?
				}
			}
		}
	}
}
