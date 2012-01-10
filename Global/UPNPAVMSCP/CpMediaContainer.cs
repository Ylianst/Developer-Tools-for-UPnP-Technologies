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
using System.IO;
using System.Xml;
using System.Text;
using System.Threading;
using System.Reflection;
using System.Collections;
using OpenSource.UPnP.AV;
using System.Diagnostics;
using OpenSource.Utilities;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// <para>
	/// This class inherits all basic metadata of a ContentDirectory media container entry 
	/// (representing "object.container" and derived UPNP media classes),
	/// for use in representing such objects for control-point interactions.
	/// </para>
	/// 
	/// <para>
	/// The state of CpMediaContainer objects is largely managed through the
	/// <see cref="CpMediaServer"/>
	/// class. On occasions where the content hierarchy of the remote MediaServer
	/// needs modification, the programmer can use the 
	/// <see cref="CpMediaBuilder"/>.CreateXXX methods
	/// to instantiate media containers to enable such interactions. Such 
	/// programming scenarios follow this general pattern.
	/// <list type="number">
	/// <item>
	/// <description>Programmer instantiates a CpMediaXXX object and sets the desired metadata appropriately.</description>
	/// </item>
	/// <item>
	/// <description>Programmer calls a RequestXXX method on a CpMediaContainer or CpRootContainer, passing the CpMediaXXX object as a parameter.</description>
	/// </item>
	/// <item>
	/// <description>The framework translates the programmatic interaction into a UPNP ContentDirectory action request.</description>
	/// </item>
	/// <item>
	/// <description>The remote MediaServer approves or denies the request, eventing the changes appropriately.</description>
	/// </item>
	/// <item>
	/// <description>The <see cref="CpMediaServer"/>
	/// consumes those notifications and updates the virtualized content hierarchy
	/// for the control point application.</description>
	/// </item>
	/// </list>
	/// </para>
	/// <para>
	/// It should be noted that this class has been largely designed for use with
	/// <see cref="MediaServerDiscovery"/>, <see cref="ContainerDiscovery"/>,
	/// and <see cref="CdsSpider"/>. The class has evolved to be a resource
	/// that can be shared amongst multiple plug-in DLL/applications that
	/// use the same memory space. The <see cref="CdsSpider"/> objects help
	/// turn this static-shared resource into an application/dll resource
	/// by specifying which items and containers are of interest. 
	/// </para>
	/// <para>
	/// When a spider starts monitoring this container, the container
	/// will notify the spiders of child objects that would be of interest
	/// to the spider. Similarly, the container will notify the spiders
	/// of any child objects that have disappeared. 
	/// </para>
	/// <para>
	/// In addition to being monitored by a spider, a container can also
	/// be marked as a match by one or more spiders. If a container
	/// ever reaches a state where zero spiders have marked the container
	/// as a match, then the container is no longer persisted by the
	/// parent container and the entire subtree (starting from this container)
	/// will be removed. 
	/// </para>
	/// <para>
	/// Which thus leads to a particular warning to programmers. Programmers
	/// are responsible for using spiders to maintain the content hierarchy branches
	/// of interest to them. If an application configures a spider to 
	/// monitor a particular container, and no spiders have been configured
	/// to monitor any of the ancestor containers, then its very likely that
	/// the spider-monitored container will be removed from its parent.
	/// </para>
	/// </summary>
	[Serializable()]
	public class CpMediaContainer : MediaContainer, IUPnPMedia, ICpMedia, ICpContainer
	{
		[NonSerialized()] protected internal UInt32 m_CacheUpdateID = 0;

		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// Serialized CpMediaContainer objects do not have their child objects
		/// serialized with them. Furthermore the information about
		/// the <see cref="CdsSpider"/> objects monitoring the container
		/// is not serialized either.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		protected CpMediaContainer(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
			: base (info, context)
		{
			// Base class constructor calls Init() so all fields are
			// initialized. Simply serialize from info.
			this.RequestStepping = info.GetUInt32("RequestStepping");
		}

		/// <summary>
		/// Custom serializer - required for ISerializable.
		/// Serializes all fields that are not marked as [NonSerialized()].
		/// Some fields were originally marked as [NonSerialized()] because
		/// this class did not implement ISerializable. I've continued to
		/// use the attribute in the code.
		/// 
		/// Serialized CpMediaContainer objects do not have their child objects
		/// serialized with them.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		public override void GetObjectData(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
		{
			base.GetObjectData(info, context);
			info.AddValue("RequestStepping", this.RequestStepping);
		}

		/// <summary>
		/// Calls base class implementation of Init()
		/// and then initializes fields for this class.
		/// </summary>
		protected override void Init()
		{
			base.Init();
			this.m_LockSinkBrowseResult = new object();
			this.m_LockState = new object();
			this.m_SpiderClients = 0;
			this.m_Spiders = new ArrayList(2);
			this.m_SpidersOnDescendents = 0;
			this.RequestStepping = 15;
			InitStateInfo(new StateInfo(false));
		}

		/// <summary>
		/// Override set: Prevents public acess to set on property.
		/// </summary>
		public override string Creator
		{
			get
			{
				return base.Creator;
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.Creator = value;
			}
		}

		/// <summary>
		/// Override set: Prevents public acess to set on property.
		/// </summary>
		public override string Title
		{
			get
			{
				return base.Title;
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.Title = value;
			}
		}
		
		/// <summary>
		/// Override set: Prevents public acess to set on property.
		/// </summary>
		public override string ID
		{
			get
			{
				return base.ID;
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.ID = value;
			}
		}

		/// <summary>
		/// This override has the behavior of reporting the total number of
		/// child objects that the container should have according to the most
		/// recent browse result. The property does not reflect the
		/// number of child objects found in CompleteList, as CompleteList
		/// only provides a cache of objects that are currently stored in memory.
		/// </summary>
		public override int ChildCount
		{
			get
			{
				uint v = this.m_state.TotalCount;
				if (v > int.MaxValue)
				{
					throw new ApplicationException("Bad Evil. Number of child objects is greater than int.MaxValue. Is there really a media server with a single container that has more than 2 billion items?");
				}
				return (int) this.m_state.TotalCount;
			}
		}
		/// <summary>
		/// Calls base class implementation for get.
		/// Checks access rights first and then calls base class implementation for set.
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		public override IMediaContainer Parent 
		{ 
			get 
			{
				return base.Parent; 
			} 
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.Parent = value;
			}
		}
		
		/// <summary>
		/// Override set to prevent access to public programmers.
		/// </summary>
		/// <exception cref="Error_MetadataCallerViolation">
		/// Thrown when the stack trace indicates the caller
		/// was not defined in this namespace and assembly.
		/// </exception>
		public override bool IsSearchable 
		{
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.IsSearchable = value;
			}
		}

		/// <summary>
		/// Override set to prevent access to public programmers.
		/// </summary>
		/// <exception cref="Error_MetadataCallerViolation">
		/// Thrown when the stack trace indicates the caller
		/// was not defined in this namespace and assembly.
		/// </exception>
		public override bool IsRestricted 
		{ 
			get
			{
				return base.IsRestricted;
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				base.IsRestricted = value;
			}
		}


		/// <summary>
		/// Returns the root container for this container.
		/// </summary>
		public CpRootContainer RootOfThis
		{
			get
			{
				IList roots = this.GetRootAncestors();
				if (roots.Count > 0)
				{
					CpRootContainer root = (CpRootContainer) roots[0];
					return root;
				}
				
				return null;
			}
		}

		/// <summary>
		/// <para>
		/// This property returns a reliable list ONLY IF
		/// the application has complete control over what
		/// child objects get persisted. The
		/// <see cref="MediaServerDiscovery"/> object was
		/// built with the assumption that applications using
		/// Remote I/O would be in the same process memory space,
		/// thus all be sharing the same static ContainerDiscovery
		/// object... thus it was imperative that containers
		/// be designed to cache children, whereas 
		/// <see cref="CdsSpider"/> objects would persist the
		/// items of interest without duplicating the actual
		/// media object instances.
		/// </para>
		/// 
		/// <para>
		/// The listing returned is only a cache of child objects
		/// that the container currently has or intends to persist.
		/// Containers must be designated with the appropriate
		/// desired state through setting the
		/// <see cref="CpMediaContainer.State"/> property.
		/// If the container is used in conjunction with
		/// <see cref="ContainerDiscovery"/> then 
		/// the only children that are persisted long term
		/// are the child containers.
		/// </para>
		/// </summary>
		public override IList CompleteList
		{
			get
			{
				IList results;

				this.m_LockListing.AcquireReaderLock(-1);
				if (this.m_Listing != null)
				{
					results = new ArrayList(this.m_Listing.Count);
					for (int i=0; i < this.m_Listing.Count; i++)
					{
						ICpMedia obj = this.m_Listing[i] as ICpMedia;
						if (obj != null)
						{
							results.Add(obj);
						}
					}
				}
				else
				{
					results = new ICpMedia[0];
				}
				this.m_LockListing.ReleaseReaderLock();

				return results;
			}
		}

		/// <summary>
		/// <para>
		/// This property returns a reliable list ONLY IF
		/// the application has complete control over what
		/// child objects get persisted. The
		/// <see cref="MediaServerDiscovery"/> object was
		/// built with the assumption that applications using
		/// Remote I/O would be in the same process memory space,
		/// thus all be sharing the same static ContainerDiscovery
		/// object... thus it was imperative that containers
		/// be designed to cache children, whereas 
		/// <see cref="CdsSpider"/> objects would persist the
		/// items of interest without duplicating the actual
		/// media object instances.
		/// </para>
		/// 
		/// <para>
		/// The listing returned is only a cache of child items
		/// that the container currently has or intends to persist.
		/// Containers must be designated with the appropriate
		/// desired state through setting the
		/// <see cref="CpMediaContainer.State"/> property.
		/// If the container is used in conjunction with
		/// <see cref="ContainerDiscovery"/> then 
		/// the only children that are persisted long term
		/// are the child containers.
		/// </para>
		/// </summary>
		public override IList Items
		{
			get
			{
				IList results;

				this.m_LockListing.AcquireReaderLock(-1);
				if (this.m_Listing != null)
				{
					results = new ArrayList(this.m_Listing.Count);
					for (int i=0; i < this.m_Listing.Count; i++)
					{
						ICpMedia obj = this.m_Listing[i] as ICpMedia;
						if (obj != null)
						{
							if (obj.IsItem)
							{
								results.Add(obj);
							}
						}
					}
				}
				else
				{
					results = new ICpMedia[0];
				}
				this.m_LockListing.ReleaseReaderLock();

				return results;
			}
		}

		/// <summary>
		/// <para>
		/// This property returns a reliable list ONLY IF
		/// the application has complete control over what
		/// child objects get persisted. The
		/// <see cref="MediaServerDiscovery"/> object was
		/// built with the assumption that applications using
		/// Remote I/O would be in the same process memory space,
		/// thus all be sharing the same static ContainerDiscovery
		/// object... thus it was imperative that containers
		/// be designed to cache children, whereas 
		/// <see cref="CdsSpider"/> objects would persist the
		/// items of interest without duplicating the actual
		/// media object instances.
		/// </para>
		/// 
		/// <para>
		/// The listing returned is only a cache of child containers
		/// that the container currently has or intends to persist.
		/// Containers must be designated with the appropriate
		/// desired state through setting the
		/// <see cref="CpMediaContainer.State"/> property.
		/// If the container is used in conjunction with
		/// <see cref="ContainerDiscovery"/> then 
		/// the only children that are persisted long term
		/// are the child containers.
		/// </para>
		/// </summary>
		public override IList Containers
		{
			get
			{
				IList results;

				this.m_LockListing.AcquireReaderLock(-1);
				if (this.m_Listing != null)
				{
					results = new ArrayList(this.m_Listing.Count);
					for (int i=0; i < this.m_Listing.Count; i++)
					{
						ICpMedia obj = this.m_Listing[i] as ICpMedia;
						if (obj != null)
						{
							if (obj.Class != null)
							{
								if (obj.Class.IsContainer)
								{
									results.Add(obj);
								}
							}
						}
					}
				}
				else
				{
					results = new ICpMedia[0];
				}
				this.m_LockListing.ReleaseReaderLock();

				return results;
			}
		}

		/// <summary>
		/// Aggregates the information needed to represent a container's 
		/// desired state and actual state.
		/// </summary>
		public struct StateInfo
		{
			/// <summary>
			/// Indicates if the container object has pending browse.
			/// </summary>
			internal bool BrowsePending;

			/// <summary>
			/// Enumerates through m_bools
			/// </summary>
			private enum EnumBools
			{
				IsMetadataClean,
				m_LastBrowseWasMetadata,
			}

			/// <summary>
			/// Merges a bunch of bool values into a bitarray
			/// for lower memory overhead.
			/// </summary>
			private BitArray m_bools;

			/// <summary>
			/// 
			/// </summary>
			/// <param name="ignored">ignored param</param>
			public StateInfo (bool ignored)
			{
				this.m_bools = new BitArray(8, false);
				this._NumberClean = 0;
				this._CleanCount = 0;
				this.TotalCount = 0;
				this.UpdateID = 0;
				this.LastBrowse = new DateTime(0);
				this.BrowsePending = false;
				
				this.IsMetadataClean = false;
				this.m_LastBrowseWasMetadata = false;
			}

			/// <summary>
			/// Initializes the fields with their default values,
			/// excepting those that are specified by the user
			/// to indicate a desired state.
			/// </summary>
			/// <param name="desiredInitialState"></param>
			public StateInfo (StateInfo desiredInitialState)
			{
				this.m_bools = new BitArray(8, false);
				this._NumberClean = 0;
				this._CleanCount = 0;
				this.TotalCount = 0;
				this.UpdateID = 0;
				this.BrowsePending = false;

				this.LastBrowse = new DateTime(0);
				this.IsMetadataClean = false;
				this.m_LastBrowseWasMetadata = false;
			}
			/// <summary>
			/// Last known UpdateID.
			/// </summary>
			public UInt32 UpdateID;

			/// <summary>
			/// Timestamp for when the last browse was completed.
			/// </summary>
			internal DateTime LastBrowse;

			/// <summary>
			/// If AlwaysIncludeContainers is true,
			/// then we have to browse the entire
			/// container to find all of the child containers.
			/// We use this value to track how
			/// many entries we've requested so far.
			/// </summary>
			internal UInt32 _NumberClean;

			/// <summary>
			/// This returns the next index to be used
			/// with ResolveStateWithBrowse.
			/// </summary>
			internal UInt32 NextBrowseStartIndex
			{
				get
				{
					return this._NumberClean;
				}
			}

			/// <summary>
			/// The number of children that the container
			/// has obtained so far that are known to be
			/// up-to-date. 
			/// </summary>
			internal UInt32 _CleanCount;
			
			/// <summary>
			/// The total number of children the container
			/// could have, according to the server's
			/// most recent response to browse.
			/// </summary>
			internal UInt32 TotalCount;

			/// <summary>
			/// If true, then the metadata for the container is up-to-date.
			/// </summary>
			public bool IsMetadataClean
			{
				get
				{
					return this.m_bools[(int) EnumBools.IsMetadataClean];
				}
				set
				{
					this.m_bools[(int) EnumBools.IsMetadataClean] = value;
				}
			}
			/// <summary>
			/// Set to true, if the last browse request was for metadata
			/// and not children.
			/// </summary>
			internal bool m_LastBrowseWasMetadata
			{
				get
				{
					return this.m_bools[(int) EnumBools.m_LastBrowseWasMetadata];
				}
				set
				{
					this.m_bools[(int) EnumBools.m_LastBrowseWasMetadata] = value;
				}
			}		
		}

		/// <summary>
		/// Returns an indication of the container's state, in terms
		/// of the range chilren it should have, the children it does have.
		/// Ranges are always relative to the container's complete list of unsorted
		/// items.
		/// <para>
		/// When setting this property, the set causes the container to update
		/// itself according to its new desired settings. The following
		/// fields are not copied over from the set operation because
		/// the caller is only allowed to specify a desired state.
		/// <list type="bullet">
		/// <item><description>CleanCount</description></item>
		/// <item><description>CleanStartIndex</description></item>
		/// <item><description>IsMetadataClean</description></item>
		/// <item><description>TotalCount</description></item>
		/// <item><description>UpdateID</description></item>
		/// </list>
		/// </para>
		/// <para>
		/// Sometimes the object that instantiates this class can
		/// specify that the StateInfo be read-only. This is
		/// generally the case if using this container in
		/// conjunction with <see cref="CdsSpider"/>. 
		/// </para>
		/// </summary>
		public virtual StateInfo State 
		{ 
			get 
			{
				return this.m_state; 
			} 
		}

		/// <summary>
		/// Enumerates through m_bools
		/// </summary>
		protected enum EnumBoolsCpMediaContainer
		{
			ReadOnlyState = MediaContainer.EnumBoolsMediaContainer.IgnoreLast,
			PleaseRemove,
			IgnoreLast
		}

		/// <summary>
		/// Tell this container to refresh itself, according
		/// to the settings in its State property.
		/// </summary>
		public virtual void Update()
		{
			this.DoNextBrowse(null);
		}

		/// <summary>
		/// Tell this container to refresh itself, even
		/// if the container thinks nothing has changed.
		/// </summary>
		/// <param name="childObjectsOnly">
		/// If true, then only the child objects are requested for updating.
		/// </param>
		public virtual void ForceUpdate(bool childObjectsOnly)
		{
			lock (this.m_LockState)
			{
				this.m_state._NumberClean = 0;
				this.m_state._CleanCount = 0;
				this.m_state.IsMetadataClean = childObjectsOnly;
				this.m_state.TotalCount = 0;
				this.m_state.m_LastBrowseWasMetadata = childObjectsOnly;
			}
			this.Update();
		}

		/// <summary>
		/// Returns the <see cref="CpMediaServer"/> object that owns the
		/// root container that is the ancestor of this container.
		/// </summary>
		/// <returns></returns>
		public CpMediaServer GetServer()
		{
			IList roots = this.GetRootAncestors();
			if (roots != null)
			{
				if (roots.Count > 0)
				{
					CpRootContainer root = (CpRootContainer) roots[0];

					CpMediaServer server = root.Server;
					return server;
				}
			}
			return null;
		}

		/// <summary>
		/// Call this method to invoke a browse on the remote mediaserver for this container.
		/// </summary>
		/// <param name="br">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the Tag object in the callback.
		/// </param>
		/// <param name="callback"></param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestBrowse(BrowseRequest br, CpContentDirectory.Delegate_OnResult_Browse callback)
		{
			CpMediaServer server = this.GetServer();
			if (server != null)
			{
				CpContentDirectory cds = server.ContentDirectory;
				cds.Browse(this.m_ID, br.BrowseFlag, br.Filter, br.StartIndex, br.RequestCount, br.SortCriteria, br, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// This method will invoke a CDS browse request and provide the results
		/// directly the application-caller.
		/// <para>
		/// Implementation simply calls <see cref="CpMediaContainer.RequestBrowse "/>(ICpMedia, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag, string, uint, uint, string).
		/// </para>
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
		public virtual void RequestBrowse (CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, string Filter, uint StartingIndex, uint RequestedCount, string SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultBrowse callback)
		{
			this.RequestBrowse(this, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, Tag, callback);
		}

		/// <summary>
		/// This method will invoke a CDS browse request and provide the results
		/// directly the application-caller.
		/// </summary>
		/// <param name="browseThis">the media object to browse from</param>
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
		public virtual void RequestBrowse (ICpMedia browseThis, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, string Filter, uint StartingIndex, uint RequestedCount, string SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultBrowse callback)
		{
			BrowseRequestTag rtag = new BrowseRequestTag();
			rtag.BrowseFlag = BrowseFlag;
			rtag.BrowseThis = browseThis;
			rtag.Filter = Filter;
			rtag.RequestedCount = RequestedCount;
			rtag.SortCriteria = SortCriteria;
			rtag.StartingIndex = StartingIndex;
			rtag.Tag = Tag;
			rtag.Callback = callback;

			BrowseRequest request = new BrowseRequest();
			request.BrowseFlag = rtag.BrowseFlag;
			request.Filter = rtag.Filter;
			request.ObjectID = rtag.BrowseThis.ID;
			request.RequestCount = rtag.RequestedCount;
			request.SortCriteria = rtag.SortCriteria;
			request.StartIndex = rtag.StartingIndex;
			request.Tag = rtag;
			request.UpdateID = this.UpdateID;

			this.RequestBrowse(request, new CpContentDirectory.Delegate_OnResult_Browse(this.SinkResult_Browse));
		}

		/// <summary>
		/// This method executes when a result from a Browse invocation is completed.
		/// The method will take the XML results from the action and return actual objects
		/// if a delegate was provided.
		/// </summary>
		/// <param name="sender">the <see cref="IUPnPService"/> object that made the call</param>
		/// <param name="ObjectID">the object ID that was browsed</param>
		/// <param name="BrowseFlag">browsed metadata or direct children</param>
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
		/// <param name="Result">
		/// DIDL-Lite response from server
		/// </param>
		/// <param name="NumberReturned">
		/// Number of object entries that should be in the Result.
		/// </param>
		/// <param name="TotalMatches">
		/// Total possible matches for the browse result.
		/// </param>
		/// <param name="UpdateID">
		/// The UpdateID of the container - ignore if browsed metadata on item.
		/// </param>
		/// <param name="e">Error information reported by server</param>
		/// <param name="_Tag">
		/// A <see cref="BrowseRequest"/> object, whose <see cref="BrowseRequest.Tag"/>
		/// field is a <see cref="BrowseRequestTag"/> object.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown when the _Tag argument is not a <see cref="BrowseRequest"/> object.
		/// Thrown when the _Tag.Tag field is not a <see cref="BrowseRequestTag"/> object.
		/// </exception>
		protected virtual void SinkResult_Browse(
			CpContentDirectory sender, 
			System.String ObjectID, 
			CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, 
			System.String Filter, 
			System.UInt32 StartingIndex, 
			System.UInt32 RequestedCount, 
			System.String SortCriteria, 
			System.String Result, 
			System.UInt32 NumberReturned, 
			System.UInt32 TotalMatches, 
			System.UInt32 UpdateID, 
			UPnPInvokeException e, 
			object _Tag)
		{
			BrowseRequest request = (BrowseRequest) _Tag;
			BrowseRequestTag rtag = (BrowseRequestTag) request.Tag;

			if (rtag.Callback != null)
			{
				if (e != null)
				{
					rtag.Callback(rtag.BrowseThis, rtag.BrowseFlag, rtag.Filter, rtag.StartingIndex, rtag.RequestedCount, rtag.SortCriteria, Result, null, NumberReturned, TotalMatches, UpdateID, rtag.Tag, e, null);
				}
				else
				{
					Exception resultException = null;
					ArrayList mediaObjects = null;
					try
					{
						mediaObjects = MediaBuilder.BuildMediaBranches(Result, typeof(MediaItem), typeof(MediaContainer));
					}
					catch (Exception buildException)
					{
						resultException = buildException;
					}

					IUPnPMedia[] objs = null;
					
					if (mediaObjects != null)
					{
						if (mediaObjects.Count > 0)
						{
							objs = (IUPnPMedia[]) mediaObjects.ToArray(typeof(IUPnPMedia));
						}
					}
					rtag.Callback(rtag.BrowseThis, rtag.BrowseFlag, rtag.Filter, rtag.StartingIndex, rtag.RequestedCount, rtag.SortCriteria, Result, objs, NumberReturned, TotalMatches, UpdateID, rtag.Tag, e, resultException);
				}
			}
		}

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
		/// <param name="callback">the callback to execute when results become available</param>
		public virtual void RequestSearch (System.String SearchCriteria, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultSearch callback)
		{
			this.RequestSearch(this, SearchCriteria, Filter, StartingIndex, RequestedCount, SortCriteria, Tag, callback);
		}

		/// <summary>
		/// This method requests a remote media server to do a search from a specified container.
		/// </summary>
		/// <param name="searchFrom">the container object to recursively search from</param>
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
		public virtual void RequestSearch (ICpContainer searchFrom, System.String SearchCriteria, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultSearch callback)
		{
			SearchRequestTag rtag = new SearchRequestTag();
			rtag.Callback = callback;
			rtag.Filter = Filter;
			rtag.RequestedCount = RequestedCount;
			rtag.SearchCriteria = SearchCriteria;
			rtag.SearchFrom = searchFrom;
			rtag.SortCriteria = SortCriteria;
			rtag.StartingIndex = StartingIndex;
			rtag.Tag = Tag;

			SearchRequest request = new SearchRequest();
			request.ContainerID = rtag.SearchFrom.ID;
			request.Filter = rtag.Filter;
			request.RequestCount = rtag.RequestedCount;
			request.SearchCriteria = rtag.SearchCriteria;
			request.SortCriteria = rtag.SortCriteria;
			request.StartIndex = rtag.StartingIndex;
			request.Tag = rtag.Tag;

		}

		/// <summary>
		/// This method executes when a result from a Search invocation is completed.
		/// The method will take the XML results from the action and return actual objects
		/// if a delegate was provided.
		/// </summary>
		/// <param name="sender">the <see cref="IUPnPService"/> object that made the call</param>
		/// <param name="ContainerID">the container where to recursively search from</param>
		/// <param name="SearchCriteria">the CDS-compliant search expression</param>
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
		/// <param name="Result">
		/// DIDL-Lite response from server
		/// </param>
		/// <param name="NumberReturned">
		/// Number of object entries that should be in the Result.
		/// </param>
		/// <param name="TotalMatches">
		/// Total possible matches for the browse result.
		/// </param>
		/// <param name="UpdateID">
		/// The UpdateID of the container - ignore if browsed metadata on item.
		/// </param>
		/// <param name="e">Error information reported by server</param>
		/// <param name="_Tag">
		/// A <see cref="SearchRequest"/> object, whose <see cref="SearchRequest.Tag"/>
		/// field is a <see cref="SearchRequestTag"/> object.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown when the _Tag argument is not a <see cref="SearchRequest"/> object.
		/// Thrown when the _Tag.Tag field is not a <see cref="SearchRequestTag"/> object.
		/// </exception>
		protected virtual void SinkResult_Search(CpContentDirectory sender, System.String ContainerID, System.String SearchCriteria, System.String Filter, System.UInt32 StartingIndex, System.UInt32 RequestedCount, System.String SortCriteria, System.String Result, System.UInt32 NumberReturned, System.UInt32 TotalMatches, System.UInt32 UpdateID, UPnPInvokeException e, object _Tag)
		{
			SearchRequest request = (SearchRequest) _Tag;
			SearchRequestTag rtag = (SearchRequestTag) request.Tag;

			if (rtag.Callback != null)
			{
				if (e != null)
				{
					rtag.Callback(rtag.SearchFrom, rtag.SearchCriteria, rtag.Filter, rtag.StartingIndex, rtag.RequestedCount, rtag.SortCriteria, Result, null, NumberReturned, TotalMatches, UpdateID, rtag.Tag, e, null);
				}
				else
				{
					Exception resultException = null;
					ArrayList mediaObjects = null;
					try
					{
						mediaObjects = MediaBuilder.BuildMediaBranches(Result, typeof(MediaItem), typeof(MediaContainer));
					}
					catch (Exception buildException)
					{
						resultException = buildException;
					}

					IUPnPMedia[] objs = null;
					
					if (mediaObjects != null)
					{
						if (mediaObjects.Count > 0)
						{
							objs = (IUPnPMedia[]) mediaObjects.ToArray(typeof(IUPnPMedia));
						}
					}
					rtag.Callback(rtag.SearchFrom, rtag.SearchCriteria, rtag.Filter, rtag.StartingIndex, rtag.RequestedCount, rtag.SortCriteria, Result, objs, NumberReturned, TotalMatches, UpdateID, rtag.Tag, e, resultException);
				}
			}

		}

		/// <summary>
		/// Call this method to invoke a search on the remote mediaserver for this container.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the Tag object in the callback.
		/// </param>
		/// <param name="callback"></param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestSearch (SearchRequest request, CpContentDirectory.Delegate_OnResult_Search callback)
		{
			CpMediaServer server = this.GetServer();
			if (server != null)
			{
				CpContentDirectory cds = server.ContentDirectory;
				cds.Search(request.ContainerID, request.SearchCriteria, request.Filter, request.StartIndex, request.RequestCount, request.SortCriteria, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// Keeps track of the range the information regarding what's
		/// clean and what isn't.
		/// </summary>
		[NonSerialized()] private StateInfo m_state;

		/// <summary>
		/// For use when locking the container's state.
		/// </summary>
		[NonSerialized()] private object m_LockState = new object();

		/// <summary>
		/// Locks the Sink_ResultBrowse method, so as to prevent
		/// the results from a browse request to process until
		/// after the previous results have been fully processed.
		/// We need a lock because we still want to be able to
		/// issue the next request even before we're done processing
		/// the results... just that we don't want to process
		/// results until AFTER we're done.
		/// </summary>
		[NonSerialized()] private object m_LockSinkBrowseResult = new object();

		/// <summary>
		/// Indicates that no spiders have marked the container to remain in memory.
		/// </summary>
		internal bool PleaseRemove
		{
			get
			{
				return this.m_bools[(int) EnumBoolsCpMediaContainer.PleaseRemove];
			}
			set
			{
				this.m_bools[(int) EnumBoolsCpMediaContainer.PleaseRemove] = value;
			}
		}


		/// <summary>
		/// This method executes when the metadata of this object or its child objects
		/// can be updated with results from a browse request.
		/// </summary>
		/// <param name="sender">the <see cref="IUPnPService"/> object that made the call</param>
		/// <param name="ObjectID">the object ID that was browsed</param>
		/// <param name="BrowseFlag">browsed metadata or direct children</param>
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
		/// <param name="Result">
		/// DIDL-Lite response from server
		/// </param>
		/// <param name="NumberReturned">
		/// Number of object entries that should be in the Result.
		/// </param>
		/// <param name="TotalMatches">
		/// Total possible matches for the browse result.
		/// </param>
		/// <param name="UpdateID">
		/// The UpdateID of the container - ignore if browsed metadata on item.
		/// </param>
		/// <param name="e">Error information reported by server</param>
		/// <param name="_Tag">
		/// A <see cref="BrowseRequest"/> object.
		/// </param>
		protected virtual void ThisResult_Browse(
			CpContentDirectory sender, 
			System.String ObjectID, 
			CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, 
			System.String Filter, 
			System.UInt32 StartingIndex, 
			System.UInt32 RequestedCount, 
			System.String SortCriteria, 
			System.String Result, 
			System.UInt32 NumberReturned, 
			System.UInt32 TotalMatches, 
			System.UInt32 UpdateID, 
			UPnPInvokeException e, 
			object _Tag)
		{
			Exception error = null;
			// We're not supposed to fire events within a lock, BUT
			// the only time this method is ever going to get called
			// is when this instance/object receives the results
			// for a browse request so this essentially guarantees
			// that we'll never deadlock.
			Hashtable spiderMatches;
			lock (this.m_LockSinkBrowseResult)
			{
				try
				{
					// allow this container to issue another browse from another thread
					StateInfo info = this.UnmarkPendingBrowse();

					IList notifyThese = new CdsSpider[0];
				
					// do not bother doing any processing if an error occurred.
					if (e == null)
					{
						// Create a hashtable called spiderMatches that will
						// contain the results for children that have been 
						// added to this container. The hashtable will be
						// keyed from a CdsSpider to an ArrayList of new children.

						lock (this.m_Spiders.SyncRoot)
						{
							IList temp = this.GetActiveSpiders();
							if (temp != null)
							{
								notifyThese = temp;
							}
							spiderMatches = new Hashtable(notifyThese.Count);
							foreach (CdsSpider spider in notifyThese)
							{
								spiderMatches[spider] = new ArrayList();
							}
						}


						// Cast the tag object into a browse request, and assume
						// for now that nothing has changed.

						BrowseRequest br = (BrowseRequest) _Tag;
						bool childrenChanged = false;

						// Create empty lists that will store any new child
						// objects and provide a list specifically for
						// new containers.
						int nRet = (int)NumberReturned;
						ArrayList foundChildren = new ArrayList(nRet);

						if (BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN)
						{
							// If the browse results are for children of this container...

							// prevent ourselves from writing to a null memory cache
							if (this.m_Listing == null)
							{
								this.m_Listing = new ArrayList(nRet);
							}

							// Cast the Result string into an XMLDOM object
							// and iterate through the XML.

							XmlDocument xml = MediaObject.DidlLiteXmlToXmlDOM(Result);

							XmlNodeList didlnodes = xml.GetElementsByTagName(T[_DIDL.DIDL_Lite]);

							foreach (XmlElement element in didlnodes[0].ChildNodes)
							{
								// Obtain the ID for the media object represented in
								// the current element. If the ID is null, throw
								// an exception because that is not schema compliant
								// and we have no obligation to fix the errors of
								// a MediaServer.

								XmlAttribute attrib = element.Attributes[T[_ATTRIB.id]];
								string id = attrib.Value;

								ICpMedia obj = null;
								bool isNew = false;

								BlankObject blank = new BlankObject();
								blank.m_ID = id;

								CpMediaContainer cpc;
								CpMediaItem cpi;
								if (id != null)
								{
									int objI = HashingMethod.Get(this.m_Listing, blank);

									if (objI >= 0)
									{
										if (objI < this.m_Listing.Count)
										{
											obj = this.m_Listing[objI] as ICpMedia;
										}
									}

									if (obj == null)
									{
										// The object's ID does not map to an existing
										// media object, therefore we need to instantiate
										// a new media object. If the element does not
										// map to a container or item, throw an exception
										// because this is not schema-compliant.

										isNew = true;
										// we don't have an object by that id, so simply create a new one
										if (string.Compare (element.Name, T[_DIDL.Container], true) == 0)
										{
											obj = new CpMediaContainer(element);
										}
										else if (string.Compare (element.Name, T[_DIDL.Item], true) == 0)
										{
											obj = new CpMediaItem(element);
										}
										else
										{
											throw new Error_BadMetadata("Non-DIDL-Lite element found: " + element.Name);
										}
									}
									else
									{
										// The specified ID maps to an existing object.
										// Update the resources and metadata of this object.
										// Afterwards, flag the object so that it is
										// not marked for removal.

										MediaObject mo = (MediaObject) obj;
										mo.UpdateObject(element);
									}

									cpc = obj as CpMediaContainer;
									cpi = obj as CpMediaItem;
								}
								else
								{
									throw new Error_BadMetadata("Missing value for item@id.");
								}


								// Ask the spiders of this container, whether this object is
								// of interest. If any of the spiders have an interest in the
								// object, the UpdateSpiderMatches() method will properly
								// add the objects to the ArrayLists in spiderMatches.
								// If it turns out that no spiders are interested in the
								// object, then mark the object for removal.
							
								bool keepObj = this.UpdateSpiderMatches(notifyThese, cpc, cpi, isNew, spiderMatches);

								foundChildren.Add(obj);

								if (keepObj)
								{
									if (isNew)
									{
										childrenChanged = true;
									}
								}
							}

							// Add all of the children to the memory cache.
							// We will remove the undesired ones from the
							// memory cache after the objects are written to disk.

							this.AddObjects(foundChildren, true);

							ArrayList al = (ArrayList) br.Tag;
							if (al == null)
							{
								al = new ArrayList(nRet);
							}
							this.HashingMethod.Add(al, foundChildren, true);
							br.Tag = al;
						}
						else
						{
							// simply update the metadata for the container
							base.UpdateObject(Result);
						}

						// Update the state of this container to match the new results.

						StateInfo newState = this.UpdateStateInfo(br, NumberReturned, UpdateID, TotalMatches);

						// Asynchronously request a new browse. If another browse was 
						// not requested, then it means that the container has completed
						// obtaining all of its child objects. 
					
						bool anotherBrowseRequested = this.DoNextBrowse(br.Tag);

						// We also want to save the objects that were found on this pass...
						// but we don't want to remove anything unless we're done
						// obtaining all child objects for this particular value
						// of UPdateID.
						
						AdjustCacheObject adjust = new AdjustCacheObject();
						adjust.AnotherBrowseRequested = anotherBrowseRequested;
						adjust.UpdateID = UpdateID;
						adjust.notifyTheseSpiders = notifyThese;
						adjust.childObjects = (ArrayList) br.Tag;
						adjust.childrenThisPass = foundChildren;
						this.AdjustCache(adjust);

						if (childrenChanged)
						{
							// Notify the spiders and the root containers
							// of the changes.
							this.NotifyRootsOfChange();
						}
						this.NotifySpidersAdd(spiderMatches);

						// Moved from AdjustCache() because we
						// should notify done only after 
						// we notify of the things added.
						if (adjust.AnotherBrowseRequested == false)
						{
							foreach (CdsSpider spider in adjust.notifyTheseSpiders)
							{
								spider.NotifySinkUpdateDone(this);
							}
						}
					}
					else
					{
						error = e;
					}
				}
				catch (Exception bad)
				{
					error = bad;
				}
			}

			try
			{
				if (error != null)
				{
					StringBuilder errMsg = new StringBuilder();
					errMsg.AppendFormat("CpMediaContainer.ThisResult_Browse() encountered an error.\r\nFriendlyName='{0}'\r\nDevice UDN='{1}'\r\nService ID='{2}'", sender.GetUPnPService().ParentDevice.FriendlyName, sender.GetUPnPService().ParentDevice.UniqueDeviceName, sender.GetUPnPService().ServiceID);
					errMsg.AppendFormat("\r\n\r\nBrowse(ObjectID='{0}', BrowseFlag={1}, Filter='{2}', StartingIndex={3}, RequestedCount={4}, SortCriteria='{5}')", ObjectID, BrowseFlag, Filter, StartingIndex.ToString(), RequestedCount.ToString(), SortCriteria);
					errMsg.AppendFormat("\r\n\r\nResult='{0}'", Result);
					errMsg.AppendFormat("\r\nNumberReturned='{0}'", NumberReturned.ToString());
					errMsg.AppendFormat("\r\nTotalMatches='{0}'", TotalMatches.ToString());
					errMsg.AppendFormat("\r\nUpdateID='{0}'", UpdateID.ToString());
					throw new Exception(errMsg.ToString(), error);
				}
			}
			catch (Exception longError)
			{
				lock (this.m_Spiders.SyncRoot)
				{
					IList temp = this.GetActiveSpiders();
					foreach (CdsSpider spider in temp)
					{
						spider.NotifySinkBrowsingError(this, longError);
					}
				}

				OpenSource.Utilities.EventLogger.Log(longError);
			}
		}

		/// <summary>
		/// We use this struct to determine whether it's okay to 
		/// remove unwanted items from the cache.
		/// </summary>
		private struct AdjustCacheObject
		{
			public bool AnotherBrowseRequested;
			public UInt32 UpdateID;
			public IList notifyTheseSpiders;
			public ArrayList childObjects;
			public ArrayList childrenThisPass;
		}

		/// <summary>
		/// After a bunch of media objects are saved to disk,
		/// we execute this method. The method will remove
		/// the appropriate child objects from the disk
		/// and memory caches.
		/// </summary>
		/// <param name="Tag"></param>
		private void AdjustCache(AdjustCacheObject Tag)
		{
			lock (this.m_LockSinkBrowseResult)
			{
				AdjustCacheObject adjust = (AdjustCacheObject) Tag;

				if (adjust.AnotherBrowseRequested == false)
				{
					if (adjust.UpdateID == this.UpdateID)
					{
						// synchronize with Sink_BrowseResult()
						// because we don't ever want to erase stuff
						// from the memory cache while we're adding
						// stuff to it

						ArrayList removeThese = new ArrayList();
						ArrayList goodAndBadListing = this.m_Listing;
						ArrayList goodListing = (ArrayList) adjust.childObjects;
						foreach (ICpMedia child in goodAndBadListing)
						{
							bool removeObj = false;
							CpMediaContainer cpc = child as CpMediaContainer;
							CpMediaItem cpi = child as CpMediaItem;

							int index = HashingMethod.Get(goodListing, child);
							if (index < 0)
							{
								// the object has been removed altogether from
								// so remove it from memory as well as the disk.
								removeObj = true;
							}
							else if (cpc != null)
							{
								removeObj = cpc.PleaseRemove;
							}
							else if (cpi != null)
							{
								removeObj = cpi.PleaseRemove;
							}
							else
							{
								throw new ApplicationException("bad evil. must be container or item");
							}
							
							if (removeObj)
							{
								if (cpc != null)
								{
									cpc.NotifySpidersOfGoneContainer();
								}
								removeThese.Add(child);
							}
						}

						if (removeThese.Count > 0)
						{
							this.NotifySpidersRemove(adjust.notifyTheseSpiders, removeThese);
							this.RemoveObjects(removeThese);

							if (adjust.UpdateID != this.m_CacheUpdateID)
							{
								this.NotifyRootsOfChange();
							}
							this.m_CacheUpdateID = adjust.UpdateID;
						}

						System.GC.Collect();
					}
				}
			}
		}

		/// <summary>
		/// This resets the StateInfo object for the container.
		/// </summary>
		/// <param name="br">the original browse request</param>
		/// <param name="numReturned">the number of entries returned in the result set</param>
		/// <param name="newUpdateID">the new value for the updateID</param>
		/// <param name="total"></param>
		/// <returns>the new state</returns>
		private StateInfo UpdateStateInfo(BrowseRequest br, UInt32 numReturned, UInt32 newUpdateID, UInt32 total)
		{
			StateInfo newState;
			lock (this.m_LockState)
			{
				this.m_state.UpdateID = newUpdateID;

				if (this.m_UpdateID != this.m_state.UpdateID)
				{
					this.m_UpdateID = this.m_state.UpdateID;
					this.m_state._CleanCount = 0;
					this.m_state._NumberClean = 0;

					if (br.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN)
					{
						this.m_state.IsMetadataClean = false;
					}
					else
					{
						this.m_state.IsMetadataClean = true;
					}
				}
				else
				{
					if (br.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN)
					{
						// then just note that we've cleared more children
						this.m_state._NumberClean += numReturned;
					}
				}

				if (br.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN)
				{
					this.m_state.TotalCount = total;
					this.m_state.m_LastBrowseWasMetadata = false;
				}

				if (br.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA)
				{
					this.m_state.IsMetadataClean = true;
					this.m_state.m_LastBrowseWasMetadata = true;
				}

				newState = this.m_state;
			}

			return newState;
		}


		/// <summary>
		/// Returns true if the request to mark the container as having
		/// a pending browse succeeded. The request to mark a container
		/// with a pending browse will fail if a pending browse
		/// already exists.
		/// </summary>
		/// <returns></returns>
		private bool MarkPendingBrowse()
		{
			bool retVal = false;
			lock (this.m_LockState)
			{
				if (this.m_state.BrowsePending == false)
				{
					this.m_state.BrowsePending = true;
					retVal = true;
				}
				else
				{
					retVal = false;
				}
			}

			return retVal;
		}

		/// <summary>
		/// Sets the flag (to false), indicating if a browse request is still pending.
		/// Also sets the time for the last browse.
		/// </summary>
		/// <returns></returns>
		private StateInfo UnmarkPendingBrowse()
		{
			StateInfo retVal;
			lock (this.m_LockState)
			{
				this.m_state.LastBrowse = DateTime.Now;
				retVal = this.m_state;
				if (this.m_state.BrowsePending == false)
				{
					throw new UPnPCustomException(666, "Bad Evil. Threading error when browsing.");
				}
				else
				{
					this.m_state.BrowsePending = false;
				}
			}

			return retVal;
		}

		/// <summary>
		/// After a Browse invocation completes, then this method executes
		/// and attempts to grab the 
		/// </summary>
		/// <param name="Tag">
		/// Object is used to pass some state information about the previous browse.
		/// For now it's only an arraylist of children for all previous browse
		/// requests for a particular UpdateID period.
		/// </param>
		/// <returns>
		/// Returns true if the container needs to issue another browse
		/// to further get more information about its state.
		/// </returns>
		private bool DoNextBrowse(object Tag)
		{
			CpContentDirectory.Delegate_OnResult_Browse callback = new CpContentDirectory.Delegate_OnResult_Browse(this.ThisResult_Browse);
			bool browsed = false;

			if ((this.m_state.IsMetadataClean == false))
			{
				// The metadata for the container needs to be updated.

				BrowseRequest br = new BrowseRequest();
				br.ObjectID = this.ID;
				br.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA;
				br.StartIndex = 0;
				br.Filter = "*";
				br.RequestCount = 0;
				br.SortCriteria = "";
				br.StartIndex = 0;
				br.UpdateID = this.m_UpdateID;
				br.Tag = null;

				if (this.MarkPendingBrowse())
				{
					this.RequestBrowse (br, callback);
					browsed = true;
				}
			}
			else 
			{
				StateInfo info;
				lock (this.m_LockState)
				{
					info = this.m_state;
				}

				BrowseRequest br = new BrowseRequest();
				br.ObjectID = this.ID;
				br.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN;
				br.Filter = "*";
				br.SortCriteria = "";
				br.UpdateID = info.UpdateID;
				br.StartIndex = info.NextBrowseStartIndex;
				br.RequestCount = this.RequestStepping;
				br.Tag = Tag;

				// If we haven't cleared all of the items yet...
				if (
					(info._NumberClean < info.TotalCount) ||
					(info.m_LastBrowseWasMetadata)
					)
				{
					// choose the next logical index in the desired subset
					if (this.MarkPendingBrowse())
					{
						this.RequestBrowse (br, callback);
						browsed = true;
					}
				}
			}
			return browsed;
		}

		/// <summary>
		/// The maximum number of items to request at any one time.
		/// </summary>
		public UInt32 RequestStepping = 15;

		/// <summary>
		/// Set this boolean to true if the state is read-only.
		/// </summary>
		private bool ReadOnlyState
		{
			get
			{
				return this.m_bools[(int) EnumBoolsCpMediaContainer.ReadOnlyState];
			}
			set
			{
				this.m_bools[(int) EnumBoolsCpMediaContainer.ReadOnlyState] = value;
			}
		}

		/// <summary>
		/// Default constructor - no fields initialized.
		/// </summary>
		public CpMediaContainer() : base()
		{
			// Base class default constructor is called,
			// so InitStateInfo() should have been called
			// through Init().
		}

		/// <summary>
		/// Creates a completely empty container, save that the id is set.
		/// </summary>
		/// <param name="id"></param>
		public CpMediaContainer(string id)
		{
			this.m_ID = id;
			InitStateInfo(new StateInfo(false));
		}

		/// <summary>
		/// This constructor calls the base class constructor(XmlElement), and 
		/// if and only if the type of the instances is a DvMediaContainer will
		/// the constructor call the base class implementation of UpdateEverything().
		/// Any derived classes that use this constructor will have to make the 
		/// calls to UpdateEverything() if appropriate.
		/// </summary>
		/// <param name="xmlElement">XmlElement that represent a DIDL-Lite container element</param>
		public CpMediaContainer (XmlElement xmlElement)
			: base (xmlElement)
		{
			InitStateInfo(new StateInfo(false));
		}

		/// <summary>
		/// Makes it so that a CpMediaContainer instantiated from an XmlElement
		/// instantiates its child resources as <see cref="CpMediaResource"/> objects,
		/// and child items and containers are <see cref="CpMediaItem"/> and <see cref="CpMediaContainer"/>.
		/// <para>
		/// Derived classes that expect different types for their resources and child
		/// media objects need to override this method.
		/// </para>
		/// </summary>
		/// <param name="xmlElement"></param>
		protected override void FinishInitFromXml(XmlElement xmlElement)
		{
			ArrayList children;
			base.UpdateEverything(true, true, typeof(CpMediaResource), typeof(CpMediaItem), typeof(CpMediaContainer), xmlElement, out children);
			this.AddObjects(children, true);
		}


		/// <summary>
		/// Method initializes the container once, and subscribes to notifications
		/// for when its corresponding container changes on the remote server.
		/// </summary>
		/// <param name="desiredStateInfo"></param>
		private void InitStateInfo(StateInfo desiredStateInfo)
		{
			this.HashingMethod = MediaContainer.HashWithIdNoSorting;
			this.m_UpdateID = uint.MaxValue;
			this.m_state = new StateInfo(desiredStateInfo);
			this.m_state.UpdateID = this.m_UpdateID;
		}

		/// <summary>
		/// Returns true if the specified media object and this container
		/// share the same root container attached to a media server.
		/// </summary>
		/// <param name="obj"></param>
		/// <returns></returns>
		/// <exception cref="InvalidCastException">
		/// Thrown when the parent of the object is not a <see cref="CpMediaContainer"/>
		/// object.
		/// </exception>
		public bool IsMediaObjectOnThisServer(ICpMedia obj)
		{
			CpMediaServer s1 = this.GetServer();
			CpMediaContainer p = (CpMediaContainer) obj.Parent;
			CpMediaServer s2 = p.GetServer();

			return (s1 == s2);
		}

		/// <summary>
		/// Method throws an <see cref="Error_MediaNotOnServer"/>
		/// object if the object does not belong to the
		/// same content hiearchy as this container.
		/// </summary>
		/// <param name="obj"></param>
		protected void ErrorIfMediaNotOnServer(ICpMedia obj)
		{
			bool same = this.IsMediaObjectOnThisServer(obj);

			if (same == false)
			{
				throw new Error_MediaNotOnServer(obj, this.GetServer());
			}
		}

		/// <summary>
		/// Requests a remote media server to change the metadata for this container.
		/// </summary>
		/// <param name="useThisMetadata">
		/// Media object that represents what the new metadata should be for the object.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">callback where the results should return</param>
		public virtual void RequestUpdateObject (IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback)
		{
			this.RequestUpdateObject(this, useThisMetadata, Tag, callback);
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestBrowse"/> (ICpMedia, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag, string, uint, uint, string)
		/// </summary>
		protected struct BrowseRequestTag
		{
			public ICpMedia BrowseThis;
			public CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag;
			public string Filter;
			public uint StartingIndex;
			public uint RequestedCount;
			public string SortCriteria;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultBrowse Callback;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestCreateObject"/> (IUPnPMedia, object, CpMediaDelegates.Delegate_ResultCreateObject)
		/// </summary>
		protected struct CreateObjectRequestTag
		{
			public ICpContainer Parent;
			public IUPnPMedia NewObject;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultCreateObject Callback;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestCreateReference"/> (ICpMedia, object, CpMediaDelegates.Delegate_ResultCreateReference)
		/// </summary>
		protected struct CreateReferenceRequestTag
		{
			public ICpMedia Parent;
			public ICpMedia ReferencedItem;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultCreateReference Callback;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestDeleteResource"/> (ICpResource, object, CpMediaDelegates.Delegate_ResultDeleteResource)
		/// </summary>
		protected struct DeleteResourceRequestTag
		{
			public ICpMedia RequestedFrom;
			public ICpMedia Owner;
			public ICpResource DeleteThis;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultDeleteResource Callback;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestDestroyObject"/> (object, CpMediaDelegates.Delegate_ResultDestroyObject)
		/// </summary>
		protected struct DestroyObjectRequestTag
		{
			public ICpMedia DestroyThis;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultDestroyObject Callback;
		}

		protected struct ExportResourceRequestTag
		{
			public ICpResource ExportThis;
			public Uri ExportHere;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultExportResource Callback;
			public CpMediaServer Server;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestImportResource"/> (System.Uri sourceUri, ICpResource, object, CpMediaDelegates.Delegate_ResultImportResource)
		/// </summary>
		protected struct ImportResourceRequestTag
		{
			public ICpMedia RequestedFrom;
			public ICpMedia Owner;
			public ICpResource ImportHere;
			public System.Uri ImportFrom;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultImportResource Callback;
			public CpMediaServer Server;
		}

		protected struct ImportResourceRequestTag2
		{
			public IUPnPMedia RequestedFrom;
			public IUPnPMedia Owner;
			public IMediaResource ImportHere;
			public System.Uri ImportFrom;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultImportResource2 Callback;
			public CpMediaServer Server;
		}
		
		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestSearch"/> 
		/// </summary>
		protected struct SearchRequestTag
		{
			public ICpContainer SearchFrom;
			public string SearchCriteria;
			public string Filter;
			public uint StartingIndex;
			public uint RequestedCount;
			public string SortCriteria;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultSearch Callback;
		}

		/// <summary>
		/// Used strictly for routing <see cref="CpMediaContainer.RequestUpdateObject"/> (ICpMedia, MediaObject, object, CpMediaDelegates.Delegate_ResultUpdateObject).
		/// </summary>
		protected struct UpdateObjectRequestTag
		{
			public ICpMedia ChangeThis;
			public IUPnPMedia Metadata;
			public object Tag;
			public CpMediaDelegates.Delegate_ResultUpdateObject Callback;
		}

		/// <summary>
		/// <see cref="ICpMedia"/>.RequestUpdateObject() methods call this object
		/// to do all of the heavy lifting. The method is responsible for 
		/// figuring out the differences between the two media objects
		/// and translating those differences into a request for the remote media server.
		/// </summary>
		/// <param name="changeThisObject">
		/// Media object (most likely obtained through a <see cref="CdsSpider"/> object) 
		/// that should have its metadata changed.</param>
		/// <param name="useThisMetadata">
		/// The new metadata that should replace the existing metadata.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">
		/// Callback where the results of the invoke should go.
		/// </param>
		/// <exception cref="Error_MediaNotOnServer">
		/// Thrown if the specified changeThisObject is not on the same server
		/// as this container.
		/// </exception>
		public virtual void RequestUpdateObject (ICpMedia changeThisObject, IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback)
		{
			ICpMedia obj1 = changeThisObject;
			IUPnPMedia obj2 = useThisMetadata;

			this.ErrorIfMediaNotOnServer(obj1);

			// Enumerate through properties, resources, and desc nodes. 
			// Determine sets of elements that need to be added, removed, and changed.
			ArrayList changed = new ArrayList();
			DeterminePropertyDifferences (obj1, obj2, changed);
			DetermineResourceDifferences (obj1, obj2, changed);
			DetermineDescNodeDifferences (obj1, obj2, changed);

			// The changed arraylist has the information on how we need to formulate the request.
			// Build the CSV list of xml fragments for old and new values.
			StringBuilder oldValues = new StringBuilder(changed.Count * 50);
			StringBuilder newValues = new StringBuilder(changed.Count * 50);

			bool first = true;
			foreach (ChangePair pair in changed)
			{
				if (first)
				{
					first = false;
				}
				else
				{
					oldValues.Append(",");
					newValues.Append(",");
				}
				oldValues.Append(pair.OldXml);
				newValues.Append(pair.NewXml);
			}

			UpdateObjectRequestTag rtag = new UpdateObjectRequestTag();
			rtag.Callback = callback;
			rtag.ChangeThis = obj1;
			rtag.Metadata = obj2;
			rtag.Tag = Tag;

			// formulate the request
			UpdateObjectRequest r = new UpdateObjectRequest();
			r.objectID = obj1.ID;
			r.CurrentTagValues = oldValues.ToString();
			r.NewTagValues = newValues.ToString();
			r.Tag = rtag;
			
			this.RequestUpdateObject(r, new CpContentDirectory.Delegate_OnResult_UpdateObject(this.SinkResult_UpdateObject));
		}

		/// <summary>
		/// This method is used when formulating a request to change an object's metadata.
		/// The method determines the differences in all of the resources.
		/// </summary>
		/// <param name="obj1">original media object metadata</param>
		/// <param name="obj2">proposed media object metadata</param>
		/// <param name="changed">
		/// list of <see cref="CpMediaContainer.ChangePair"/> objects
		/// that describe old-new pairs of XML string values.
		/// </param>
		protected void DetermineDescNodeDifferences (IUPnPMedia obj1, IUPnPMedia obj2, ArrayList changed)
		{
			IList xmls1 = obj1.DescNodes;
			IList xmls2 = obj2.DescNodes;
			
			ArrayList added = new ArrayList();
			ArrayList removed = new ArrayList();

			foreach (string xml1 in xmls1)
			{
				int pos = xmls2.IndexOf(xml1);
				if (pos >= 0)
				{
					// both objects have the xml node
					xmls2.RemoveAt(pos);
				}
				else
				{
					//the proposed object does not have the element
					removed.Add(xml1);
				}
			}

			foreach (string xml2 in xmls2)
			{
				int pos = xmls1.IndexOf(xml2);
				if (pos >= 0)
				{
					// both objects have the xml node
					throw new ApplicationException("Bad evil. This should have been removed earlier.");
				}
				else
				{
					//the proposed object has a new element
					added.Add(xml2);
				}
			}

			// convert added/removed lists into a single array
			// indicating an oldValue/NewValue replacement pair.
			this.GetChangePairs(added, removed, changed);
		}

		/// <summary>
		/// This method is used when formulating a request to change an object's metadata.
		/// The method determines the differences in all of the resources.
		/// </summary>
		/// <param name="obj1">original object with resources</param>
		/// <param name="obj2">object with proposed resources</param>
		/// <param name="changed">results of ChangePair values are stored in here</param>
		protected void DetermineResourceDifferences (IUPnPMedia obj1, IUPnPMedia obj2, ArrayList changed)
		{
			// get shallow copy of the resources
			ArrayList res1 = new ArrayList((ICollection) obj1.Resources);
			ArrayList res2 = new ArrayList((ICollection) obj2.Resources);

			string xml;
			ArrayList added = new ArrayList();
			ArrayList removed = new ArrayList();

			foreach (MediaResource r1 in res1)
			{
				// IList.IndexOf() calls the object.Equals() method for comparison,
				// so if a resource that is effectively the same as another resource
				// in res2, then the resource appears in both objects.
				int pos = res2.IndexOf(r1);
				if (pos >= 0)
				{
					// remove it from the proposed object since it's not relevant to us
					res2.RemoveAt(pos);
				}
				else
				{
					// the resource has been removed from the proposed object
					xml = this.GetXmlFragment(r1);
					removed.Add(xml);
				}
			}

			foreach (MediaResource r2 in res2)
			{
				// IList.IndexOf() calls the object.Equals() method for comparison,
				// so if a resource that is effectively the same as another resource
				// in res2, then the resource appears in both objects.
				int pos = res1.IndexOf(r2);
				if (pos >= 0)
				{
					throw new ApplicationException("Bad evil. This resource should have been removed from the list already.");
				}
				else
				{
					// the resource has been added to the proposed object
					xml = this.GetXmlFragment(r2);
					added.Add(xml);
				}
			}

			// convert added/removed lists into a single array
			// indicating an oldValue/NewValue replacement pair.
			this.GetChangePairs(added, removed, changed);
		}

		/// <summary>
		/// This method is used when formulating a request to change an object's metadata.
		/// The method determines the differences in all of the metadata properties,
		/// excluding desc nodes and resources.
		/// </summary>
		/// <param name="obj1">original object's metadata</param>
		/// <param name="obj2">proposed object's metadata</param>
		/// <param name="changed">listing of ChangePair values that indicate how the request should be made</param>
		protected void DeterminePropertyDifferences (IUPnPMedia obj1, IUPnPMedia obj2, ArrayList changed)
		{
			IMediaProperties mp1 = obj1.Properties;
			IMediaProperties mp2 = obj2.Properties;

			ArrayList props1 = new ArrayList();
			foreach (string key in mp1.PropertyNames)
			{
				props1.Add(key);
			}
			ArrayList props2 = new ArrayList();
			foreach (string key in mp2.PropertyNames)
			{
				props2.Add(key);
			}
			string xml;

			// compare old object's properties against proposed object's properties
			foreach (string prop1 in props1)
			{
				ArrayList added = new ArrayList();
				ArrayList removed = new ArrayList();

				ArrayList vals1 = new ArrayList((ICollection) mp1[prop1]);
				ArrayList vals2 = new ArrayList((ICollection) mp2[prop1]);

				if ((vals2 != null) && (vals1.Count > 0))
				{
					props2.Remove(prop1);

					foreach (ICdsElement v1 in vals1)
					{
						int pos = vals2.IndexOf(v1);
						if (pos >= 0)
						{
							// the proposed object has an identical
							// value in its list
							vals2.RemoveAt(pos);
						}
						else
						{
							// the proposed object is missing the 
							// value in its list 
							xml = this.GetXmlFragment(v1);
							removed.Add(xml);
						}
					}
					foreach (ICdsElement v2 in vals2)
					{
						int pos = vals1.IndexOf(v2);
						if (pos >= 0)
						{
							// the proposed object has the same element,
							// but this should never happen
							throw new ApplicationException("Bad evil. Should have been removed earlier.");
						}
						else
						{
							// the original object is missing the proposed element
							xml = this.GetXmlFragment(v2);
							added.Add(xml);
						}
					}
				}
				else
				{
					// proposed object has no metadata for that property
					foreach (ICdsElement v1 in vals1)
					{
						xml = this.GetXmlFragment(v1);
						removed.Add(xml);
					}
				}

				// convert added/removed lists into a single array
				// indicating an oldValue/NewValue replacement pair.
				// We have to call this method here because
				// the property name needs to be consistent whenever
				// adding values to the changed array.
				this.GetChangePairs(added, removed, changed);
			}

			// compare new object's properties against old object's properties
			foreach (string prop2 in props2)
			{
				ArrayList added = new ArrayList();
				ArrayList removed = new ArrayList();
				ArrayList vals1 = new ArrayList((ICollection) mp1[prop2]);
				ArrayList vals2 = new ArrayList((ICollection) mp2[prop2]);

				if ((vals1 != null) && (vals1.Count > 0))
				{
					// both objects have values for this property, so we should have taken care of it already
					throw new ApplicationException("Bad evil. This property should have been removed already.");
				}
				else
				{
					// proposed object has an additional property list
					foreach (ICdsElement v2 in vals2)
					{
						xml = this.GetXmlFragment(v2);
						added.Add(xml);
					}
				}

				// We have to call this method here because
				// the property name needs to be consistent whenever
				// adding values to the changed array.
				this.GetChangePairs(added, removed, changed);
			}
		}



		/// <summary>
		/// Takes an ICdsElement object and casts it into its string form.
		/// </summary>
		/// <param name="element"></param>
		/// <returns></returns>
		protected string GetXmlFragment (ICdsElement element)
		{
			StringBuilder sbXml = null;
			StringWriter sw = null;
			XmlTextWriter xmlWriter = null;

			// Write the string representation of the xml element
			sbXml = new StringBuilder(XML_BUFFER_SIZE);
			sw = new StringWriter(sbXml);
			xmlWriter = new XmlTextWriter(sw);
			element.ToXml(ToXmlFormatter.DefaultFormatter, MediaObject.ToXmlData_Default, xmlWriter);
			xmlWriter.Flush();
			
			string result = sbXml.ToString();
			xmlWriter.Close();

			return result;
		}


		/// <summary>
		/// Used when attempting to formulate a request to change metadata for an object.
		/// This method takes an array of elements that need to be added and removed
		/// and adds ChangePair objects to the changed array to indicate how 
		/// we should formulate the request.
		/// </summary>
		/// <param name="added">
		/// List of values that have been added for a specific metadata property;
		/// metadata property is assumed to be of the same type as in "removed"
		/// argument.
		/// </param>
		/// <param name="removed">
		/// List of values that have been removed for a specific metadata property;
		/// metadata property is assumed to be of the same type as in "added"
		/// argument.
		/// </param>
		/// <param name="changed">results get added to to this object</param>
		protected void GetChangePairs(ArrayList added, ArrayList removed, ArrayList changed)
		{
			int maxI = Math.Max(added.Count, removed.Count);

			for (int i=0; i < maxI; i++)
			{
				ChangePair cp = new ChangePair();
				if (i < removed.Count)
				{
					cp.OldXml = (string) removed[i];
				}
				else
				{
					cp.OldXml = "";
				}
				if (i < added.Count)
				{
					cp.NewXml = (string) added[i];
				}
				else
				{
					cp.NewXml = "";
				}
				changed.Add(cp);
			}
		}

		/// <summary>
		/// Describes a pair of XML elements in string form
		/// where the oldXml is the old value that should be replaced
		/// and NewXml is the new value to replace it with.
		/// This struct is used when requesting a remote media server
		/// to change metadata.
		/// </summary>
		protected struct ChangePair
		{
			public string OldXml;
			public string NewXml;
		}


		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestUpdateObject"/>(ICpMedia, MediaObject, object, CpMediaDelegates.Delegate_RequestForUpdateObject),
		/// when the results return from <see cref="CpMediaContainer.RequestUpdateObject"/>(UpdateObjectRequest, CpContentDirectory.Delegate_OnResult_UpdateObject).
		/// </summary>
		/// <param name="sender">
		/// The <see cref="IUPnPService"/> object that routed the request on the wire.
		/// </param>
		/// <param name="ObjectID">
		/// The ID of the object that was requested for updating.
		/// </param>
		/// <param name="CurrentTagValue">
		/// Comma-separated value list of XML elements (in string form) representing old values.
		/// An empty string value within the list indicates the creation of a new XML element.
		/// </param>
		/// <param name="NewTagValue">
		/// Comma-separated value list of XML elements (in string form) representing new values.
		/// An empty string value within the list indicates the removal of an XML element.
		/// </param>
		/// <param name="e">errors reported by the server during invoke</param>
		/// <param name="_Tag">
		/// A <see cref="UpdateObjectRequest"/> object, whose <see cref="UpdateObjectRequest.Tag"/>
		/// field is a <see cref="UpdateObjectRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_UpdateObject (CpContentDirectory sender, System.String ObjectID, System.String CurrentTagValue, System.String NewTagValue, UPnPInvokeException e, object _Tag)
		{
			UpdateObjectRequest tag = (UpdateObjectRequest) _Tag;
			UpdateObjectRequestTag rtag = (UpdateObjectRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				rtag.Callback(rtag.ChangeThis, rtag.Metadata, rtag.Tag, e);
			}
		}

		/// <summary>
		/// <para>
		/// Public programmers should be careful when using this version of the method.
		/// It's provided for convenience, but careless specification of the argument
		/// values can cause undesired results. It is best to use the other
		/// versions of this method as they help ensure proper use.
		/// </para>
		/// Allows a programmer to request a remote mediaserver to change the metadata 
		/// for this container.
		/// </summary>
		/// <param name="request">
		/// <see cref="UpdateObject"/>
		/// Request that describes the input params used at invoke time.
		/// The object is sent as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Callback that should be executed to receive results</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// UpdateObject action.
		/// </exception>
		public virtual void RequestUpdateObject (UpdateObjectRequest request, CpContentDirectory.Delegate_OnResult_UpdateObject callback)
		{
			CpMediaServer server = this.GetServer();
			if (server != null)
			{
				server.ContentDirectory.UpdateObject(request.objectID, request.CurrentTagValues, request.NewTagValues, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}


		/// <summary>
		/// Makes a request on the remote media server to delete this
		/// container and all of its sub containers.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">
		/// Delegate executes when the results for the method are available.
		/// </param>
		/// <exception cref="UPnPInvokeException">
		/// Thrown when the server does not implement DestroyObject.
		/// </exception>
		public virtual void RequestDestroyObject (object Tag, CpMediaDelegates.Delegate_ResultDestroyObject callback)
		{
			this.RequestDestroyObject(this, Tag, callback);
		}

		/// <summary>
		/// Used by <see cref="CpMediaContainer.RequestDestroyObject"/>(object, CpMediaDelegates.Delegate_ResultDestroyObject)
		/// and <see cref="CpMediaItem.RequestDestroyObject"/>(object, CpMediaDelegates.Delegate_ResultDestroyObject)
		/// to actually formlulate a request to destroy the object.
		/// </summary>
		/// <param name="destroyThis">the media object that should be destroyed</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">
		/// The results for this asynchronous call are executed through this method.
		/// </param>
		/// <exception cref="UPnPInvokeException">
		/// Thrown when the server does not implement DestroyObject.
		/// </exception>
		/// <exception cref="Error_MediaNotOnServer">
		/// Thrown if the specified media object is not on the same server
		/// as this container.
		/// </exception>
		public virtual void RequestDestroyObject (ICpMedia destroyThis, object Tag, CpMediaDelegates.Delegate_ResultDestroyObject callback)
		{
			ErrorIfMediaNotOnServer(destroyThis);

			DestroyObjectRequestTag rtag = new DestroyObjectRequestTag();
			rtag.DestroyThis = destroyThis;
			rtag.Tag = Tag;

			DestroyObjectRequest r = new DestroyObjectRequest();
			r.objectID = destroyThis.ID;
			r.Tag = rtag;
			this.RequestDestroyObject(r, new CpContentDirectory.Delegate_OnResult_DestroyObject(this.SinkResult_DestroyObject));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestDestroyObject"/>(ICpMedia, object, CpMediaDelegates.Delegate_ResultDestroyObject)
		/// when the results return from <see cref="CpMediaContainer.RequestDestroyObject"/>(DestroyObjectRequest, CpContentDirectory.Delegate_OnResult_DestroyObject).
		/// </summary>
		/// <param name="sender">
		/// The <see cref="IUPnPService"/> object that invoked the method.
		/// </param>
		/// <param name="ObjectID">
		/// The object ID specified for destruction.
		/// </param>
		/// <param name="e">MediaServer reported errors during invoke.</param>
		/// <param name="_Tag">
		/// A <see cref="DestroyObjectRequest"/> object, whose <see cref="DestroyObjectRequest.Tag"/>
		/// field is a <see cref="DestroyObjectRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_DestroyObject(CpContentDirectory sender, System.String ObjectID, UPnPInvokeException e, object _Tag)
		{
			DestroyObjectRequest tag = (DestroyObjectRequest) _Tag;
			DestroyObjectRequestTag rtag = (DestroyObjectRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				rtag.Callback(rtag.DestroyThis, rtag.Tag, e);
			}
		}

		/// <summary>
		/// <para>
		/// Public programmers should be careful when using this version of the method.
		/// It's provided for convenience, but careless specification of the argument
		/// values can cause undesired results. It is best to use the other
		/// versions of this method as they help ensure proper use.
		/// </para>
		/// Allows a programmer to request a remote mediaserver to delete an object
		/// by a particular objectID. The objectID is not assumed to be a direct
		/// child or a descendent of this container.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Results for the request are retrieved through the execution of this delegate.</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestDestroyObject (DestroyObjectRequest request, CpContentDirectory.Delegate_OnResult_DestroyObject callback)
		{
			CpMediaServer server = this.GetServer();
			if (server != null)
			{
				server.ContentDirectory.DestroyObject(request.objectID, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// Allows a programmer to create a child item that points to another item
		/// in the content hierarchy.
		/// </summary>
		/// <param name="referencedItem">
		/// The media object that is part of the content hierarchy,
		/// which would act as the underlying/referenced item
		/// of the new item.
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">callback to execute when results have been obtained</param>
		/// <exception cref="Error_MediaNotOnServer">
		/// Thrown if the specified referencedItem is not on the same server
		/// as this container.
		/// </exception>
		/// <exception cref="Error_CannotRequestCreate">
		/// Thrown when the underlying referencedItem is a container.
		/// </exception>
		public virtual void RequestCreateReference(ICpMedia referencedItem, object Tag, CpMediaDelegates.Delegate_ResultCreateReference callback)
		{
			CreateReferenceRequestTag rtag = new CreateReferenceRequestTag();
			rtag.Callback = callback;
			rtag.Parent = this;
			rtag.ReferencedItem = referencedItem;
			rtag.Tag = Tag;

			CreateReferenceRequest request = new CreateReferenceRequest();
			request.ContainerID = rtag.Parent.ID;
			request.ObjectID = rtag.ReferencedItem.ID;
			request.Tag = rtag;
			this.RequestCreateReference(request, new CpContentDirectory.Delegate_OnResult_CreateReference (this.SinkResult_CreateReference));
		}

		/// <summary>
		/// <para>
		/// Public programmers should be careful when using this version of the method.
		/// It's provided for convenience, but careless specification of the argument
		/// values can cause undesired results. It is best to use the other
		/// versions of this method as they help ensure proper use.
		/// </para>
		/// </summary>
		/// <param name="request"><see cref="CreateReferenceRequest"/>
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">when results are available, this delegate should execute</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestCreateReference(CreateReferenceRequest request,CpContentDirectory.Delegate_OnResult_CreateReference callback)
		{
			CpMediaServer server = this.GetServer();
			if (server != null)
			{
				server.ContentDirectory.CreateReference(request.ContainerID, request.ObjectID, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestCreateReference"/>(ICpMedia, object, CpMediaDelegates.Delegate_ResultCreateReference)
		/// when the results return from <see cref="CpMediaContainer.RequestCreateReference">(CreateReferenceRequest, CpContentDirectory.Delegate_OnResult_CreateReference).
		/// </summary>
		/// <param name="sender"><see cref="IUPnPService"/> object that routed the actual call on the wire</param>
		/// <param name="ContainerID">the ID of the parent container where the new reference was created</param>
		/// <param name="ObjectID">the ID of the referenced/underlying item</param>
		/// <param name="NewID">the ID of the new child/reference item</param>
		/// <param name="e">error information reported by the server</param>
		/// <param name="_Tag">
		/// A <see cref="CreateReferenceRequest"/> object, whose <see cref="CreateReferenceRequest.Tag"/>
		/// field is a <see cref="CreateReferenceRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_CreateReference(CpContentDirectory sender, string ContainerID, string ObjectID, string NewID, UPnPInvokeException e, object _Tag)
		{
			CreateReferenceRequest tag = (CreateReferenceRequest) _Tag;
			CreateReferenceRequestTag rtag = (CreateReferenceRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				rtag.Callback(rtag.Parent, rtag.ReferencedItem, NewID, rtag.Tag, e);
			}
		}


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
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a remove a resource that has
		/// a null/empty value for its contentUri.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if deleteThisResource.Owner is not an <see cref="ICpMedia"/> object.
		/// </exception>
		public virtual void RequestDeleteResource(ICpResource deleteThisResource, object Tag, CpMediaDelegates.Delegate_ResultDeleteResource callback)
		{
			DeleteResourceRequestTag rtag = new DeleteResourceRequestTag();
			rtag.Callback = callback;
			rtag.DeleteThis = deleteThisResource;
			rtag.Owner = (ICpMedia) deleteThisResource.Owner;
			rtag.RequestedFrom = this;
			rtag.Tag = Tag;

			DeleteResourceRequest request = new DeleteResourceRequest();
			string uriString = rtag.DeleteThis.ContentUri;
			if ((uriString == null) || (uriString == ""))
			{
				throw new NullReferenceException("deleteThisResource.ContentUri cannot be empty or null.");
			}

			Uri uri = new System.Uri(uriString);

			request.ResourceUri = uri;
			request.Tag = rtag;
			this.RequestDeleteResource(request, new CpContentDirectory.Delegate_OnResult_DeleteResource(this.SinkResult_DeleteResource));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestDeleteResource"/>(ICpResource, object, CpMediaDelegates.Delegate_ResultDeleteResource)
		/// when the results return from <see cref="CpMediaContainer.RequestDeleteResource">(DeleteResourceRequest, CpContentDirectory.Delegate_OnResult_DeleteResource).
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="ResourceURI">the resource (identified by its uri value) to delete</param>
		/// <param name="e">errors reported by the remote media server</param>
		/// <param name="_Tag">
		/// A <see cref="DeleteResourceRequest"/> object, whose <see cref="DeleteResourceRequest.Tag"/>
		/// field is a <see cref="DeleteResourceRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_DeleteResource (CpContentDirectory sender, System.Uri ResourceURI, UPnPInvokeException e, object _Tag)
		{
			DeleteResourceRequest tag = (DeleteResourceRequest) _Tag;
			DeleteResourceRequestTag rtag = (DeleteResourceRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				rtag.Callback(rtag.Owner, rtag.DeleteThis, rtag.Tag, e);
			}
		}

		/// <summary>
		/// Requests a remote media server to delete a resource from its local file system.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Results for the request are retrieved through the execution of this delegate.</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a remove a resource that has
		/// a null/empty value for its contentUri.
		/// </exception>
		public virtual void RequestDeleteResource(DeleteResourceRequest request, CpContentDirectory.Delegate_OnResult_DeleteResource callback)
		{
			CpMediaServer server = this.GetServer();

			if (server != null)
			{
				server.ContentDirectory.DeleteResource(request.ResourceUri, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// a resource that is part of the server's content hierarchy.
		/// </summary>
		/// <param name="sourceUri">the URI where binary should be pulled</param>
		/// <param name="importHere">the <see cref="ICpResource"/> object that represents the
		/// destination of the imported binary
		/// </param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to import to a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a import to a resource that has
		/// a null/empty value for its importUri.
		/// </exception>
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the owner of this object is not an <see cref="ICpMedia"/>.
		/// </exception>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting to import to a resource that has
		/// a null/empty value for its importUri value.
		/// </exception>
		public virtual void RequestImportResource (System.Uri sourceUri, ICpResource importHere, object Tag, CpMediaDelegates.Delegate_ResultImportResource callback)
		{
			ICpMedia owner = (ICpMedia) importHere.Owner;

			ImportResourceRequestTag rtag = new ImportResourceRequestTag();
			rtag.Callback = callback;
			rtag.ImportFrom = sourceUri;
			rtag.ImportHere = importHere;
			rtag.Owner = owner;
			rtag.RequestedFrom = this;
			rtag.Tag = Tag;
			rtag.Server = this.GetServer();

			ImportResourceRequest request = new ImportResourceRequest();
			string importToString = importHere.ImportUri;

			if ((importToString == null) || (importToString == ""))
			{
				throw new NullReferenceException("importHere.ImportUri cannot be empty or null.");
			}

			System.Uri importTo = new System.Uri(importToString);

			request.DestinationUri = importTo;
			request.SourceUri = sourceUri;
			request.Tag = rtag;

			this.RequestImportResource(request, new CpContentDirectory.Delegate_OnResult_ImportResource(this.SinkResult_ImportResource));
		}


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
		public virtual void RequestImportResource2(System.Uri sourceUri, IMediaResource importHere, object Tag, CpMediaDelegates.Delegate_ResultImportResource2 callback)
		{
			IUPnPMedia owner = (IUPnPMedia) importHere.Owner;

			ImportResourceRequestTag2 rtag = new ImportResourceRequestTag2();
			rtag.Callback = callback;
			rtag.ImportFrom = sourceUri;
			rtag.ImportHere = importHere;
			rtag.Owner = owner;
			rtag.RequestedFrom = this;
			rtag.Tag = Tag;
			rtag.Server = this.GetServer();

			ImportResourceRequest request = new ImportResourceRequest();
			string importToString = importHere.ImportUri;

			if ((importToString == null) || (importToString == ""))
			{
				throw new NullReferenceException("importHere.ImportUri cannot be empty or null.");
			}

			System.Uri importTo = new System.Uri(importToString);

			request.DestinationUri = importTo;
			request.SourceUri = sourceUri;
			request.Tag = rtag;

			this.RequestImportResource(request, new CpContentDirectory.Delegate_OnResult_ImportResource(this.SinkResult_ImportResource2));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestImportResource"/>(System.Uri, ICpResource, object, CpMediaDelegates.Delegate_ResultImportResource)
		/// when the results return from <see cref="CpMediaContainer.RequestImportResource">(ImportResourceRequest, CpContentDirectory.Delegate_OnResult_ImportResource).
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="SourceURI">
		/// The source URI where the binary should be obtained.
		/// </param>
		/// <param name="DestinationURI">
		/// The destination resource, identified by its importUri value.
		/// </param>
		/// <param name="TransferID">
		/// The output value representing the transfer's ID.
		/// </param>
		/// <param name="e">errors reported by the remote media server</param>
		/// <param name="_Tag">
		/// A <see cref="ImportResourceRequest"/> object, whose <see cref="ImportResourceRequest.Tag"/>
		/// field is a <see cref="ImportResourceRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_ImportResource (CpContentDirectory sender, System.Uri SourceURI, System.Uri DestinationURI, System.UInt32 TransferID, UPnPInvokeException e, object _Tag)
		{
			ImportResourceRequest tag = (ImportResourceRequest) _Tag;
			ImportResourceRequestTag rtag = (ImportResourceRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				ResourceTransfer rt = null;
				if (e == null)
				{
					rt = new ResourceTransfer(TransferID, true, rtag.ImportHere, rtag.ImportFrom, rtag.Server);
				}

				rtag.Callback(rtag.ImportFrom, rtag.Owner, rtag.ImportHere, rt, rtag.Tag, e);
			}
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestImportResource"/>(System.Uri, ICpResource, object, CpMediaDelegates.Delegate_ResultImportResource)
		/// when the results return from <see cref="CpMediaContainer.RequestImportResource">(ImportResourceRequest, CpContentDirectory.Delegate_OnResult_ImportResource).
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="SourceURI">
		/// The source URI where the binary should be obtained.
		/// </param>
		/// <param name="DestinationURI">
		/// The destination resource, identified by its importUri value.
		/// </param>
		/// <param name="TransferID">
		/// The output value representing the transfer's ID.
		/// </param>
		/// <param name="e">errors reported by the remote media server</param>
		/// <param name="_Tag">
		/// A <see cref="ImportResourceRequest"/> object, whose <see cref="ImportResourceRequest.Tag"/>
		/// field is a <see cref="ImportResourceRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_ImportResource2 (CpContentDirectory sender, System.Uri SourceURI, System.Uri DestinationURI, System.UInt32 TransferID, UPnPInvokeException e, object _Tag)
		{
			ImportResourceRequest tag = (ImportResourceRequest) _Tag;
			ImportResourceRequestTag2 rtag = (ImportResourceRequestTag2) tag.Tag;
			if (rtag.Callback != null)
			{
				ResourceTransfer rt = null;
				if (e == null)
				{
					rt = new ResourceTransfer(TransferID, true, rtag.ImportHere, rtag.ImportFrom, rtag.Server);
				}

				rtag.Callback(rtag.ImportFrom, rtag.Owner, rtag.ImportHere, rt, rtag.Tag, e);
			}
		}


		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// a resource that is part of the server's content hierarchy.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Results for the request are retrieved through the execution of this delegate.</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a remove a resource that has
		/// a null/empty value for its contentUri.
		/// </exception>
		public virtual void RequestImportResource(ImportResourceRequest request, CpContentDirectory.Delegate_OnResult_ImportResource callback)
		{
			CpMediaServer server = this.GetServer();

			if (server != null)
			{
				server.ContentDirectory.ImportResource (request.SourceUri, request.DestinationUri, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

		/// <summary>
		/// Makes a request to a remote media server to create a new child object
		/// as a child of this object.
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
		public virtual void RequestCreateObject(IUPnPMedia newObject, object Tag, CpMediaDelegates.Delegate_ResultCreateObject callback)
		{
			// Note the caller's callback and input args
			CreateObjectRequestTag rtag = new CreateObjectRequestTag();
			rtag.Callback = callback;
			rtag.NewObject = newObject;
			rtag.Parent = this;
			rtag.Tag = Tag;

			// variables for printing DIDL
			StringBuilder sbXml = null;
			StringWriter sw = null;
			MemoryStream ms = null;
			XmlTextWriter xmlWriter = null;

			// set up the xml printing to do things in a CreateObject compliant manner
			ToXmlData _d = new ToXmlData();
			_d.CreateObjectParentID = this.ID;
			_d.DesiredProperties = new ArrayList(0);

			// Write the xml representation for the request
			if (ENCODE_UTF8)
			{
				ms = new MemoryStream(XML_BUFFER_SIZE);
				xmlWriter = new XmlTextWriter(ms, System.Text.Encoding.UTF8);
			}
			else
			{
				sbXml = new StringBuilder(XML_BUFFER_SIZE);
				sw = new StringWriter(sbXml);
				xmlWriter = new XmlTextWriter(sw);
			}

			// write media object with DIDL header
			MediaObject.WriteResponseHeader(xmlWriter);
			rtag.NewObject.ToXml(ToXmlFormatter.DefaultFormatter, _d, xmlWriter);
			MediaObject.WriteResponseFooter(xmlWriter);
			xmlWriter.Flush();

			string xmlResult;
			if (ENCODE_UTF8)
			{
				int len = (int) ms.ToArray().Length - 3;
				UTF8Encoding utf8e = new UTF8Encoding(false, true);
				xmlResult = utf8e.GetString(ms.ToArray(), 3, len);
			}
			else
			{
				xmlResult = sbXml.ToString();
			}
			xmlWriter.Close();

			CreateObjectRequest request = new CreateObjectRequest();
			request.Elements = xmlResult.ToString();
			request.ParentID = rtag.Parent.ID;
			request.Tag = rtag;

			//make request on wire
			this.RequestCreateObject(request, new CpContentDirectory.Delegate_OnResult_CreateObject(this.SinkResult_CreateObject));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestCreateObject"/>(IUPnPMedia, object, CpMediaDelegates.Delegate_ResultCreateObject)
		/// when the results return from <see cref="CpMediaContainer.RequestCreateObject">(CreateObjectRequest, CpContentDirectory.Delegate_OnResult_CreateObject).
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="ContainerID">ID of the parent container where the new object should appear.</param>
		/// <param name="Elements">
		/// CDS-compliant subset of pseudo-DIDL-Lite elements that
		/// describe the new media objects. Certain fields, like
		/// ID, will be blank - thus making the XML values not
		/// truly schema-compliant with DIDL-Lite.
		/// </param>
		/// <param name="ObjectID">media server provided ID of the new object</param>
		/// <param name="Result">media server provided XML for the new object</param>
		/// <param name="e">media server reported errors</param>
		/// <param name="_Tag">
		/// A <see cref="CreateObjectRequest"/> object, whose <see cref="CreateObjectRequest.Tag"/>
		/// field is a <see cref="CreateobjectRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_CreateObject (CpContentDirectory sender, System.String ContainerID, System.String Elements, System.String ObjectID, System.String Result, UPnPInvokeException e, object _Tag)
		{
			CreateObjectRequest tag = (CreateObjectRequest) _Tag;
			CreateObjectRequestTag rtag = (CreateObjectRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				ArrayList newObjects = new ArrayList();
				Exception xmlToObjectError = null;
				try
				{
					newObjects = MediaBuilder.BuildMediaBranches(Result, typeof(MediaItem), typeof(MediaContainer));
				}
				catch (Exception error2)
				{
					xmlToObjectError = error2;
				}

				IUPnPMedia newObject = null;
				if (newObjects.Count > 0)
				{
					newObject = (IUPnPMedia) newObjects[0];
				}

				rtag.Callback(rtag.Parent, rtag.NewObject, ObjectID, Result, newObject, rtag.Tag, e, xmlToObjectError);
			}
		}

		/// <summary>
		/// Makes a request to a remote media server to create a new child object
		/// as a child of this object.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Results for the request are retrieved through the execution of this delegate.</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestCreateObject(CreateObjectRequest request, CpContentDirectory.Delegate_OnResult_CreateObject callback)
		{
			CpMediaServer server = this.GetServer();

			if (server != null)
			{
				server.ContentDirectory.CreateObject(request.ParentID, request.Elements, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}
		}

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
		public virtual void RequestExportResource(ICpResource exportThis, System.Uri sendHere, object Tag, CpMediaDelegates.Delegate_ResultExportResource callback)
		{
			ExportResourceRequestTag rtag = new ExportResourceRequestTag();
			rtag.Callback = callback;
			rtag.ExportHere = sendHere;
			rtag.ExportThis = exportThis;
			rtag.Tag = Tag;
			rtag.Server = this.GetServer();

			ExportResourceRequest request = new ExportResourceRequest();
			request.DestinationUri = rtag.ExportHere;
			request.SourceUri = new System.Uri( rtag.ExportThis.ContentUri );
			request.Tag = rtag;

			this.RequestExportResource(request, new CpContentDirectory.Delegate_OnResult_ExportResource(this.SinkResult_ExportResource));
		}

		/// <summary>
		/// This routes execution to the user-provided callback, provided in a call to
		/// <see cref="CpMediaContainer.RequestCreateObject"/>(IUPnPMedia, object, CpMediaDelegates.Delegate_ResultCreateObject)
		/// when the results return from <see cref="CpMediaContainer.RequestCreateObject">(CreateObjectRequest, CpContentDirectory.Delegate_OnResult_CreateObject).
		/// </summary>
		/// <param name="sender">the <see cref="CpContentDirectory"/> that made the request on the wire</param>
		/// <param name="SourceURI">the uri on a mediaserver where the binary can be obtained</param>
		/// <param name="DestinationURI">the uri on another mediaserver where the binary should be sent</param>
		/// <param name="TransferID">the output transfer ID reported by the media server that is exporting</param>
		/// <param name="e">the errors reported by the media server</param>
		/// <param name="_Tag">
		/// A <see cref="ExportResourceRequest"/> object, whose <see cref="ExportResourceRequest.Tag"/>
		/// field is a <see cref="ExportResourceRequestTag"/> object.
		/// </param>
		protected virtual void SinkResult_ExportResource (CpContentDirectory sender, System.Uri SourceURI, System.Uri DestinationURI, System.UInt32 TransferID, UPnPInvokeException e, object _Tag)
		{
			ExportResourceRequest tag = (ExportResourceRequest) _Tag;
			ExportResourceRequestTag rtag = (ExportResourceRequestTag) tag.Tag;
			if (rtag.Callback != null)
			{
				ResourceTransfer rt = null;
				if (e == null)
				{
					rt = new ResourceTransfer(TransferID, false, rtag.ExportThis, rtag.ExportHere, rtag.Server);
				}

				rtag.Callback(rtag.ExportThis, rtag.ExportHere, rt, rtag.Tag, e);
			}
		}

		/// <summary>
		/// Makes a request on a remote media server to export one of its local resources to a URI
		/// expecting an HTTP-POST command.
		/// </summary>
		/// <param name="request">
		/// The input paramters for the action invocation. 
		/// Object will be returned as the "Tag" argument in the callback.
		/// </param>
		/// <param name="callback">Results for the request are retrieved through the execution of this delegate.</param>
		/// <exception cref="Error_CannotGetServer">
		/// Thrown if the <see cref="CpMediaServer"/> object, where the request
		/// is routed through, could not be obtained.
		/// </exception>
		/// <exception cref="UPnPInvokeException">
		/// Thrown if the remote media server does not implement the
		/// specified action.
		/// </exception>
		public virtual void RequestExportResource(ExportResourceRequest request, CpContentDirectory.Delegate_OnResult_ExportResource callback)
		{
			CpMediaServer server = this.GetServer();

			if (server != null)
			{
				server.ContentDirectory.ExportResource(request.SourceUri, request.DestinationUri, request, callback);
			}
			else
			{
				throw new Error_CannotGetServer(this);
			}		
		}

		/// <summary>
		/// Checks frame[0] of provided StackTrace to see
		/// whether the caller is of the same namespace
		/// and assembly. If not throws a MetadataCallerViolation
		/// exception.
		/// </summary>
		/// <param name="st">stack trace obtained in the method that wants to prevent public class</param>
		/// <exception cref="Error_MetadataCallerViolation">
		/// Thrown when the stack trace indicates the caller
		/// was not defined in this namespace and assembly.
		/// </exception>
		public override void CheckRuntimeBindings(StackTrace st)
		{
			StackFrame sf = st.GetFrame(0);

			MethodBase mb = sf.GetMethod();

			Type mt = mb.DeclaringType;
			Type thisType = this.GetType();
			bool ok = false;
			if ((mt.Namespace == (thisType.Namespace)) || (mt.Namespace == (typeof(MediaObject)).Namespace))
			{
				if ((mt.Assembly == thisType.Assembly) || (mt.Assembly == (typeof(MediaObject)).Assembly))
				{
					ok = true;
				}
			}

			if (!ok)
			{
				throw new Error_MetadataCallerViolation();
			}
		}


		/// <summary>
		/// Get a shallow copy of the listing hashtable
		/// from ID to MediaObject.
		/// Simply calls base class.
		/// </summary>
		/// <returns></returns>
		protected new virtual internal Hashtable GetChildren ()
		{
			return base.GetChildren ();
		}

		/// <summary>
		/// Swaps the current listing of children with a new listing,
		/// where the key is the ID and the value is the MediaObject.
		/// Simply calls base class.
		/// </summary>
		/// <param name="newChildren"></param>
		protected new virtual internal void SwapChildren (ArrayList newChildren)
		{
			base.SwapChildren (newChildren);
		}

		/// <summary>
		/// Used to clear items. Not available for public use.
		/// </summary>
		public override void ClearItems()
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.ClearItems();
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="addThis">the CpMediaResource object with a corresponding resource advertised by the MediaServer</param>
		/// <exception cref="InvalidCastException">
		/// Thrown when attempting to add a non-CpMediaResource object to this container.
		/// </exception>
		public override void AddResource(IMediaResource addThis)
		{
			this.CheckRuntimeBindings(new StackTrace());
			CpMediaResource res = (CpMediaResource) addThis;
			base.AddResource(addThis);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="removeThis">the MediaResource to remove</param>
		public override void RemoveResource(IMediaResource removeThis)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.RemoveResource(removeThis);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="newResources">a collection of CpMediaResource</param>
		/// <exception cref="InvalidCastException">
		/// Thrown when attempting to add a non-CpMediaResource object to this container.
		/// </exception>
		public override void AddResources(ICollection newResources)
		{
			this.CheckRuntimeBindings(new StackTrace());
			foreach (CpMediaResource res in newResources);
			this.m_LockResources.AcquireWriterLock(-1);
			base.AddResources(newResources);
			this.m_LockResources.ReleaseWriterLock();
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="removeThese"></param>
		public override void RemoveResources(ICollection removeThese)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.RemoveResources(removeThese);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </para>
		/// </summary>
		/// <param name="element">The metadata block must be in xml form.</param>
		public override void AddDescNode(string element)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.AddDescNode(element);
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </para>
		/// </summary>
		/// <param name="elements">The metadata blocks must be in xml form.</param>
		public override void AddDescNode(string[] elements)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.AddDescNode(elements);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="element">The metadata blocks must be in xml form.</param>
		public override void RemoveDescNode(string element)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.RemoveDescNode(element);
		}


		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="addThis"></param>
		public override void AddObject(IUPnPMedia addThis, bool overwrite)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.AddObject(addThis, true);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="removeThis"></param>
		public override void RemoveObject(IUPnPMedia removeThis)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.RemoveObject(removeThis);
		}		

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="addThese"></param>
		/// <param name="overwrite"></param>
		public override void AddObjects(ICollection addThese, bool overwrite)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.AddObjects(addThese, overwrite);
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.	
		/// </summary>
		/// <param name="removeThese"></param>
		public override void RemoveObjects(ICollection removeThese)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.RemoveObjects(removeThese);
		}		

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// <para>
		/// After checking the caller's binding/permissions, the method
		/// simply calls <see cref="MediaObject.UpdateEverything"/>
		/// with the following code
		/// <code>
		/// ArrayList proposedChildren;
		/// this.UpdateEverything(false, false, typeof(CpMediaResource), typeof(CpMediaItem), typeof(CpMediaContainer), xmlElement, out proposedChildren);
		/// </code>
		/// </para>
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateMetadata(XmlElement xmlElement)
		{
			this.CheckRuntimeBindings(new StackTrace());
			ArrayList proposedChildren;
			this.UpdateEverything(false, false, typeof(CpMediaResource), typeof(CpMediaItem), typeof(CpMediaContainer), xmlElement, out proposedChildren);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		public override void UpdateMetadata(string DidlLiteXml)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.UpdateMetadata(DidlLiteXml);
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="newObj"></param>
		public override void UpdateObject(IUPnPMedia newObj)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.UpdateObject(newObj);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// <para>
		/// After checking the caller's binding/permissions, the method
		/// simply calls <see cref="MediaObject.UpdateEverything"/>
		/// with the following code
		/// <code>
		/// ArrayList proposedChildren;
		/// this.UpdateEverything(true, false, typeof(CpMediaResource), typeof(CpMediaItem), typeof(CpMediaContainer), xmlElement, out proposedChildren);
		/// </code>
		/// </para>
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateObject(XmlElement xmlElement)
		{
			this.CheckRuntimeBindings(new StackTrace());
			ArrayList proposedChildren;
			this.UpdateEverything(true, false, typeof(CpMediaResource), typeof(CpMediaItem), typeof(CpMediaContainer), xmlElement, out proposedChildren);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		public override void UpdateObject (string DidlLiteXml)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.UpdateObject(DidlLiteXml);
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="writeStatus"></param>
		public override void SetWriteStatus(EnumWriteStatus writeStatus)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetWriteStatus(writeStatus);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="classType"></param>
		/// <param name="friendlyName"></param>
		public override void SetClass(string classType, string friendlyName)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetClass(classType, friendlyName);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue (string propertyName, IList values)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue (propertyName, values);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_String(string propertyName, string[] values)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_String (propertyName, values);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_String(string propertyName, string val)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_String(propertyName, val);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_Int(string propertyName, int[] values)
		{
			this.CheckRuntimeBindings(new StackTrace());
			this.SetPropertyValue_Int(propertyName, values);
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_Int(string propertyName, int val)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_Int(propertyName, val);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_Long(string propertyName, long[] values)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_Long(propertyName, values);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_Long(string propertyName, long val)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_Long(propertyName, val);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_MediaClass(string propertyName, MediaClass[] values)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_MediaClass(propertyName, values);
		}

		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaContainer object is responsible
		/// for maintaining its own state.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_MediaClass(string propertyName, MediaClass val)
		{
			this.CheckRuntimeBindings(new StackTrace());
			base.SetPropertyValue_MediaClass(propertyName, val);
		}


		/// <summary>
		/// <para>
		/// Used to notify
		/// the root containers to event the changes of descendent containers.
		/// Changes of containers are always evented through the root to prevent
		/// race conditions between the creation of a container and the public
		/// code that subscribes to the notification. 
		/// </para>
		/// 
		/// <para>
		/// A container can have multiple roots by way of root
		/// containers that aggregate other root containers. All
		/// CpRootContainer objects that are an ancestor
		/// to the container will fire the event.
		/// </para>
		/// </summary>
		internal void NotifyRootsOfChange()
		{
			IList roots = this.GetRootAncestors();
			if (roots != null)
			{
				while (roots.Count > 0)
				{
					CpRootContainer root = (CpRootContainer) roots[0];
					root.FireOnContainerChanged(this);
					roots.RemoveAt(0);
				}
			}
		}

		/// <summary>
		/// Helper function to acquire the root containers of this instance.
		/// A virtualized container may have multiple ancestors, all
		/// claiming to be the root container because the application
		/// may want to aggregate a set of root containers with
		/// another root container.
		/// </summary>
		/// <returns>the listing of CpRootContainer ancestors of this container</returns>
		internal IList GetRootAncestors()
		{
			CpMediaContainer c = this;
			ArrayList roots = new ArrayList();

			System.Type type = c.GetType();
			if (
				(type == TYPE_CP_ROOT) ||
				(type.IsSubclassOf(TYPE_CP_ROOT))
				)
			{
				roots.Add(c);
			}
			
			while (c.Parent != null)
			{
				c = (CpMediaContainer) c.Parent;
				type = c.GetType();
				if (
					(type == TYPE_CP_ROOT) ||
					(type.IsSubclassOf(TYPE_CP_ROOT))
					)
				{
					roots.Add(c);
				}
			}

			return roots;
		}

		/// <summary>
		/// Sets the container to a new parent.
		/// </summary>
		/// <param name="newParent"></param>
		internal void SetParent(CpMediaContainer newParent)
		{
			this.m_Parent = newParent;
		}


		/// <summary>
		/// Returns a weak reference with the spider if the
		/// spider object is monitoring this container.
		/// Otherwise, it returns null.
		/// Method assumes that the caller has locked the spiders.
		/// </summary>
		/// <param name="spider"></param>
		/// <returns></returns>
		internal WeakReference GetSpiderWeakRef (CdsSpider spider)
		{
			WeakReference spiderRef = null;
			ArrayList removeThese = new ArrayList(this.m_Spiders.Count);
			foreach (WeakReference wr in this.m_Spiders)
			{
				try
				{
					CdsSpider cdsSpider = (CdsSpider) wr.Target;
					if (wr.IsAlive)
					{
						if (cdsSpider == spider)
						{
							spiderRef = wr;
							break;
						}
					}
					else
					{
						// the weak reference points to a spider
						// that's been finalized so mark it for removal
						removeThese.Add(wr);
					}
				}
				catch
				{
					removeThese.Add(wr);
				}
			}

			// remove the garbage collected spiders from the container's list
			foreach (WeakReference wr in removeThese)
			{
				this.m_Spiders.Remove(wr);
			}

			return spiderRef;
		}

		/// <summary>
		/// Returns true if the spider is monitoring this container.
		/// Method assumes that the caller has locked the spiders.
		/// </summary>
		/// <param name="spider"></param>
		/// <returns></returns>
		internal bool IsSpiderSubscribed(CdsSpider spider)
		{
			bool result = (GetSpiderWeakRef(spider) != null);
			return result;
		}

		/// <summary>
		/// Spider objects call this method to indicate
		/// that a spider is interested in this container.
		/// The method simply calls the implementation
		/// of <see cref="CpMediaContainer.SubscribeSpider"/>(CdsSpider, object),
		/// which may have an override.
		/// </summary>
		/// <param name="spider"></param>
		internal void SubscribeSpider (CdsSpider spider)
		{
			this.SubscribeSpider(spider, null);
		}

		/// <summary>
		/// <see cref="CpMediaContainer.SubscribeSpider"/>(CdsSpider)
		/// always calls this method, allowing derived classes
		/// to change the implementation of the SubscribeSpider
		/// call without changing the access limitations.
		/// The implementation simply adds a spider to this 
		/// container's list of spider objects interested in 
		/// monitoring this container. The actual code is shown below.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="stateObject">allows additional information to be sent along with the spider</param>
		protected virtual void SubscribeSpider (CdsSpider spider, object stateObject)
		{
			this.NotifyAncestorsOfNewSpider();

			lock (this.m_Spiders.SyncRoot)
			{
				// attempt to find the spider in the current list
				bool hasSpider = this.IsSpiderSubscribed(spider);

				// don't add the spider if it's already added
				if (hasSpider == false)
				{
					this.m_Spiders.Add(new WeakReference(spider, true));
				}
			}
		}

		/// <summary>
		/// Helper function to acquire the ancestral containers of this instance.
		/// </summary>
		/// <returns>the listing of CpMediaContainer ancestors of this container</returns>
		protected IList GetAncestors()
		{
			CpMediaContainer c = this;
			ArrayList roots = new ArrayList();

			while (c.Parent != null)
			{
				c = (CpMediaContainer) c.Parent;
				roots.Add(c);
			}

			return roots;
		}

		/// <summary>
		/// Spider objects call this method to indicate
		/// that a spider is no longer interested in this container.
		/// The method simply calls the implementation
		/// of <see cref="CpMediaContainer.UnsubscribeSpider"/>(CdsSpider, object),
		/// which may have an override.
		/// </summary>
		/// <param name="spider"></param>
		internal void UnsubscribeSpider (CdsSpider spider)
		{
			this.UnsubscribeSpider(spider, null);
		}

		/// <summary>
		/// <see cref="CpMediaContainer.UnsubscribeSpider"/>(CdsSpider)
		/// always calls this method, allowing derived classes
		/// to change the implementation of the SubscribeSpider
		/// call without changing the access limitations.
		/// The implementation simply removes the spider from the container's list of
		/// spiders interested in monitoring this container.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="stateObject">allows additional information to be sent along with the spider</param>
		protected virtual void UnsubscribeSpider (CdsSpider spider, object stateobject)
		{
			this.NotifyAncestorsofGoneSpider();
			
			lock (this.m_Spiders)
			{
				if (this.m_Spiders != null)
				{
					WeakReference wr = this.GetSpiderWeakRef(spider);

					if (wr != null)
					{
						this.m_Spiders.Remove(wr);
					}
				}
			}		
		}

		/// <summary>
		/// Returns a listing of strong references to <see cref="CdsSpider"/>
		/// objects of spiders that are actively monitoring this container.
		/// </summary>
		/// <returns></returns>
		protected IList GetActiveSpiders()
		{
			ArrayList notifyThese = new ArrayList(0);
			lock (this.m_Spiders.SyncRoot)
			{
				if (this.m_Spiders != null)
				{
					ArrayList removeThese = new ArrayList(this.m_Spiders.Count);
					notifyThese = new ArrayList(this.m_Spiders.Count);
					foreach (WeakReference wr in this.m_Spiders)
					{
						CdsSpider spider = (CdsSpider) wr.Target;
						if (wr.IsAlive)
						{
							notifyThese.Add(spider);
						}
						else
						{
							//spider has been garbage collected
							removeThese.Add(wr);
						}
					}
					// remove the garbage collected spiders from the container's list
					foreach (WeakReference wr in removeThese)
					{
						this.m_Spiders.Remove(wr);
					}
				}
			}

			return notifyThese;
		}

		/// <summary>
		/// Determines if the object is of interest to the spiders
		/// monitoring this container.
		/// </summary>
		/// <param name="notifyThese">listing of spiders monitoring this container</param>
		/// <param name="cpc"><see cref="CpMediaContainer"/> object will be null if cpi != null</param>
		/// <param name="cpi"><see cref="CpMediaItem"/> object will be null if cpc != null</param>
		/// <param name="isNew">
		/// Set this value to true if the object is not an existing child of the container.
		/// </param>
		/// <param name="matchResults">a hashtable where matches for all spiders are stored while processing the results of the Browse request</param>
		/// <returns>true, if any of the spiders marked the item of interest</returns>
		private bool UpdateSpiderMatches(IList notifyThese, CpMediaContainer cpc, CpMediaItem cpi, bool isNew, Hashtable matchResults)
		{
			bool keepInMemory = false;
			if (notifyThese != null)
			{
				ICpMedia obj = cpc;
				if (obj == null)
				{
					obj = cpi;
				}

				foreach (CdsSpider spider in notifyThese)
				{
					ArrayList al = (ArrayList) matchResults[spider];

					bool isMatch = spider.IsMatch(obj, false);

					if (isMatch)
					{
						if (isNew || spider.ExpectingResults)
						{
							if (cpc != null)
							{
								cpc.IncrementSpiderMatches();
							}
							else if (cpi != null)
							{
								cpi.IncrementSpiderMatches();
							}
							al.Add(obj);
						}

						keepInMemory = true;
					}
				}
			}

			if (cpc != null)
			{
				// if a child container (or any of its descendents)
				// has a spider, explicitly keep the container.

				if (cpc.SpiderClients > 0)
				{
					keepInMemory = true;
				}
			}

			//if (!keepInMemory)
			//{
			//	int x = 3;
			//}

			if (cpc != null)
			{
				cpc.PleaseRemove = !keepInMemory;
			}
			else if (cpi != null)
			{
				cpi.PleaseRemove = !keepInMemory;
			}

			return keepInMemory;
		}

		/// <summary>
		/// Call this method to notify a set of spiders that
		/// objects have been removed.
		/// </summary>
		/// <param name="notifyThese">listing of strong references to <see cref="CdsSpider"/> objects</param>
		/// <param name="removeThese">ArrayList of strong references to <see cref="ICpMedia"/> objects</param>
		protected void NotifySpidersRemove(IList notifyThese, ArrayList removeThese)
		{
			foreach (CdsSpider spider in notifyThese)
			{
				spider.NotifySinkRemove(this, removeThese);
			}
		}

		/// <summary>
		/// Call this method to notify a set of spiders that
		/// objects have been added.
		/// </summary>
		/// <param name="matchResults">Hashtable where the key is a <see cref="CdsSpider"/> and value is an ArrayList of WEAK REFERENCES to <see cref="ICpMedia"/> objects</param>
		protected void NotifySpidersAdd(Hashtable matchResults)
		{
			foreach (CdsSpider spider in matchResults.Keys)
			{
				ArrayList al = (ArrayList) matchResults[spider];
				if (al.Count > 0)
				{
					spider.NotifySinkAdd(this, al);
				}
			}
		}

		/// <summary>
		/// Increments the number of spiders that have indicated
		/// this container to be of interest to them. A container
		/// is considered of interest to a spider if the
		/// spider's IsMatch() method passes.
		/// This method is called by Sink_ResultBrowse.
		/// </summary>
		internal void IncrementSpiderMatches()
		{
			this.IncrementSpiderMatches(null);
		}

		/// <summary>
		/// <see cref="CpMediaContainer.IncrementSpiderMatches"/>() calls
		/// this method (or the overridden implementation). This implementation
		/// simply
		/// increments the number of spiders that have indicated
		/// this container to be of interest to them.
		/// </summary>
		/// <param name="stateObject"></param>
		protected virtual void IncrementSpiderMatches(object stateObject)
		{
			System.Threading.Interlocked.Increment(ref this.m_SpiderClients);
		}

		/// <summary>
		/// This method is called a <see cref="CdsSpider"/> object
		/// to indicate that the spider is no longer interested in the
		/// container because it fails the matching criteria.
		/// This method simply calls the implementation
		/// of <see cref="DecrementSpiderMatches"/>(object),
		/// which may have an override.
		/// </summary>
		internal void DecrementSpiderMatches()
		{
			this.DecrementSpiderMatches(null);
		}

		private void NotifyAncestorsOfNewSpider()
		{
			IList ancestors = this.GetAncestors();
			foreach (CpMediaContainer ancestor in ancestors)
			{
				System.Threading.Interlocked.Increment(ref ancestor.m_SpidersOnDescendents);
			}
		}

		private void NotifyAncestorsofGoneSpider()
		{
			IList ancestors = this.GetAncestors();
			foreach (CpMediaContainer ancestor in ancestors)
			{
				System.Threading.Interlocked.Decrement(ref ancestor.m_SpidersOnDescendents);
			}
		}

		internal void NotifySpidersOfGoneContainer()
		{
			foreach (CpMediaContainer child in this.Containers)
			{
				child.NotifySpidersOfGoneContainer();
			}

			IList spiders = this.GetActiveSpiders();

			foreach (CdsSpider spider in spiders)
			{
				spider.NotifySinkContainerGone(this);
			}
		}

		/// <summary>
		/// <see cref="CpMediaContainer.DecrementSpiderMatches"/>() calls
		/// this method (or the overridden implementation). This implementation
		/// simply
		/// decrements the number of spiders that have indicated
		/// this container to be of interest to a CdsSpider.
		/// </summary>
		/// <param name="stateObject">ignored</param>
		protected virtual void DecrementSpiderMatches(object stateObject)
		{
			System.Threading.Interlocked.Decrement(ref this.m_SpiderClients);
			if (this.m_SpiderClients == 0)
			{
				if (this.m_SpidersOnDescendents == 0)
				{
					lock (this.m_Spiders.SyncRoot)
					{
						for (int i=0; i < this.m_Spiders.Count; i++)
						{
							WeakReference wr = (WeakReference) this.m_Spiders[i];
							if (wr.IsAlive == false)
							{
								this.m_Spiders.RemoveAt(i);
								i--;
							}
						}

						if (this.m_Spiders.Count == 0)
						{
							CpMediaContainer p = this.Parent as CpMediaContainer;
							if (p != null)
							{
								this.Parent.RemoveObject(this);
							}
						}
					}
				}
				else if (this.m_SpidersOnDescendents < 0)
				{
					throw new ApplicationException("bad evil: m_SpidersOnDescendents should never be < 0");
				}
			}		
			else if (this.m_SpiderClients < 0)
			{
				throw new ApplicationException("bad evil: m_SpiderClients should never be < 0");
			}
		}

		/// <summary>
		/// Returns number of spiders that need this container in memory.
		/// </summary>
		public long SpiderClients 
		{ 
			get 
			{ 
				return this.m_SpiderClients + this.m_SpidersOnDescendents + this.m_Spiders.Count;
			} 
		}

		/// <summary>
		/// The number of spiders that are interested in this item.
		/// </summary>
		[NonSerialized()] protected long m_SpiderClients = 0;

		/// <summary>
		/// Keeps a listing of weak references that point
		/// to all of the spiders interested in this container.
		/// </summary>
		[NonSerialized()] protected ArrayList m_Spiders = new ArrayList(2);

		/// <summary>
		/// The number of spiders that are interested in containers
		/// that are descendents of this container.
		/// </summary>
		[NonSerialized()] protected long m_SpidersOnDescendents = 0;

		/// <summary>
		/// A static reference to the type of a CpRootContainer instance.
		/// </summary>
		private static System.Type TYPE_CP_ROOT = typeof (CpRootContainer);
		/// <summary>
		/// A static reference to the type of a CpMediaResource instance.
		/// </summary>
		private static System.Type TYPE_CP_RESOURCE = typeof (CpMediaResource);
		/// <summary>
		/// A static reference to the type of a CpMediaContainer instance.
		/// </summary>
		private static System.Type TYPE_CP_CONTAINER = typeof (CpMediaContainer);
		/// <summary>
		/// A static reference to the type of a CpMediaItem instance.
		/// </summary>
		private static System.Type TYPE_CP_ITEM = typeof(CpMediaItem);
	}
}
