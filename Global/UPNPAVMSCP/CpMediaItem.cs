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
using System.Threading;
using System.Reflection;
using System.Diagnostics;
using System.Collections;
using OpenSource.UPnP.AV;
using OpenSource.Utilities;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// <para>
	/// This class inherits all basic metadata of a ContentDirectory media item entry 
	/// (representing "object.item" and derived UPNP media classes),
	/// for use in representing such objects for control-point interactions.
	/// </para>
	/// 
	/// <para>
	/// The state of CpMediaItem objects is largely managed through the
	/// <see cref="CpMediaServer"/>
	/// class. On occasions where the content hierarchy of the remote MediaServer
	/// needs modification, the programmer can use the 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaBuilder"/>.CreateCpXXX methods
	/// to instantiate media items to enable such interactions. Such 
	/// programming scenarios follow this general pattern.
	/// <list type="number">
	/// <item>
	/// <description>Programmer instantiates a CpMediaXXX object and sets the desired metadata appropriately.</description>
	/// </item>
	/// <item>
	/// <description>Programmer calls a RequstXXX method on a CpMediaContainer or CpRootContainer, passing the CpMediaXXX object as a parameter.</description>
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
	/// <see cref="CpMediaContainer"/> and <see cref="CdsSpider"/>. 
	/// The class has evolved to be a resource
	/// that can be shared amongst multiple plug-in DLL/applications that
	/// use the same memory space. The <see cref="CdsSpider"/> objects help
	/// turn this static-shared resource into an application/dll resource
	/// by specifying which items and containers are of interest. 
	/// </para>
	/// <para>
	/// When a spider starts monitoring a container, the container
	/// will notify the spiders of child objects (which can include
	/// instances of <see cref="CpMediaItem"/> that would be of interest
	/// to the spider. Similarly, the container will notify the spiders
	/// of any child objects that have disappeared. 
	/// </para>
	/// <para>
	/// When this a container notifies a spider that this item
	/// is of interest to the spider, the item is marked as match
	/// and an internal counter is incremented to indicate the total
	/// number of spiders that want this item persisted.
	/// If an item ever reaches a state where zero spiders have marked the item
	/// as a match, then the item is no longer persisted by the
	/// parent container.
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
	public sealed class CpMediaItem : MediaItem, IUPnPMedia, ICpMedia, ICpItem
	{
		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		private CpMediaItem(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
			: base (info, context)
		{
			// Base class constructor calls Init() so all fields are
			// initialized. Nothing else was serialized.
		}

		/// <summary>
		/// Custom serializer - required for ISerializable.
		/// Serializes all fields that are not marked as [NonSerialized()].
		/// Some fields were originally marked as [NonSerialized()] because
		/// this class did not implement ISerializable. I've continued to
		/// use the attribute in the code.
		/// 
		/// CpMediaContainer objects do not serialize their child objects,
		/// information about spiders, or the 
		/// <see cref="CpMediaItem.GetUnderlyingItem"/> field.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		public override void GetObjectData(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
		{
			base.GetObjectData(info, context);
			
			// nothing in addition to serialize
		}

		/// <summary>
		/// Calls base class implementation of Init()
		/// and then initializes fields for this class.
		/// </summary>
		protected override void Init()
		{
			base.Init();
			this.GetUnderlyingItem = null;
			this.m_SpiderClients = 0;
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
		/// Calls base class implementation for get.
		/// Checks access rights first and then calls base class implementation for set.
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// <para>
		/// This constructor calls the base class constructor(XmlElement), and 
		/// if and only if the type of the instances is a DvMediaItem will
		/// the constructor call the base class implementation of UpdateEverything().
		/// Any derived classes that use this constructor will have to make the 
		/// calls to UpdateEverything() if appropriate.
		/// </para>
		/// </summary>
		/// <param name="xmlElement">XmlElement representing a DIDL-Lite item element</param>
		public CpMediaItem (XmlElement xmlElement)
			: base (xmlElement)
		{
		}

		/// <summary>
		/// Makes it so that a CpMediaItem instantiated from an XmlElement
		/// instantiates its child resources as <see cref="CpMediaResource"/> objects.
		/// 
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
		}

		/// <summary>
		/// Default constructor. No metadata is initialized in this
		/// method. It is STRONGLY recommended that programmers
		/// use the <see cref="CpMediaBuilder"/>.CreateXXX methods to instantiate
		/// CpMediaItem objects.
		/// </summary>
		public CpMediaItem() : base()
		{
		}

		/// <summary>
		/// Returns true, if the item is a reference to another item.
		/// </summary>
		public override bool IsReference
		{
			get
			{
				return (this.m_RefID != "");
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
				CpMediaContainer p = this.Parent as CpMediaContainer;
				if (p != null)
				{
					p.CheckRuntimeBindings(new StackTrace());
				}
				base.IsRestricted = value;
			}
		}

		/// <summary>
		/// Returns the ID of the referred item. If the item
		/// does not reference anything, it returns the empty string.
		/// Set only allows internal callers. 
		/// </summary>
		/// <exception cref="Error_MetadataCallerViolation">
		/// Thrown if set() caller is not internal to assembly/namespace.
		/// </exception>
		public override string RefID
		{
			get
			{
				return this.m_RefID;
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());
				this.m_RefID = value;
			}
		}


		/// <summary>
		/// If the item is a reference item, it can have two generalized states:
		/// Valid or Invalid. If the state is valid, then the referred item is
		/// has been virtualized and its metadata can be obtained. Otherwise,
		/// the state is invalid and attempting to retrieve referred item's metadata
		/// will cause errors. This method always returns true if the item is not a reference.
		/// </summary>
		public bool IsValid
		{
			get
			{
				if (IsReference == false)
				{
					return true;
				}
				return (this.RefItem != null);
			}
		}

		/// <summary>
		/// This method will invoke a CDS browse request and provide the results
		/// directly the application-caller.
		/// <para>
		/// Implementation simply calls the parent owner's implementation of 
		/// <see cref="CpMediaContainer.RequestBrowse "/>(ICpMedia, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag, string, uint, uint, string).
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
		/// <exception cref="ApplicationException">
		/// Thrown if the BrowseFlag value is BrowseDirectChildren because only the
		/// object's metadata can be obtained use browse.
		/// </exception>
		public void RequestBrowse (CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag, string Filter, uint StartingIndex, uint RequestedCount, string SortCriteria, object Tag, CpMediaDelegates.Delegate_ResultBrowse callback)
		{
			if (BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN)
			{
				throw new ApplicationException("BrowseFlag cannot be BROWSEDIRECTCHILDREN");
			}

			CpMediaContainer parent = (CpMediaContainer) this.Parent;

			parent.RequestBrowse(this, BrowseFlag, Filter, StartingIndex, RequestedCount, SortCriteria, Tag, callback);
		}

		/// <summary>
		/// Simply calls the owner object's implementation of Update().
		/// </summary>
		/// <exception cref="NullReferenceException">
		/// Thrown if the parent object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the parent object is not a <see cref="CpMediaContainer"/>,
		/// which should always be the case.
		/// </exception>
		public void Update ()
		{
			CpMediaContainer parent = (CpMediaContainer) this.Parent;
			parent.Update();
		}


		/// <summary>
		/// Allows a programmer to request a remote mediaserver to change the metadata 
		/// for this item.
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
		/// <param name="callback">
		/// Delegate executes when the results for the method are available.
		/// </param>
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if this object's parent is not a 
		/// <see cref="CpMediaContainer"/> object.
		/// </exception>
		public void RequestUpdateObject (IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback)
		{
			CpMediaContainer p = (CpMediaContainer) this.Parent;

			if (p != null)
			{
				p.RequestUpdateObject(this, useThisMetadata, Tag, callback);
			}
			else
			{
				throw new Error_CannotGetParent(this);
			}
		}

		/// <summary>
		/// Makes a request on the remote media server to delete this
		/// object from its parent.
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
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if this object's parent is not a 
		/// <see cref="CpMediaContainer"/> object.
		/// </exception>
		public void RequestDestroyObject (object Tag, CpMediaDelegates.Delegate_ResultDestroyObject callback)
		{
			CpMediaContainer p = (CpMediaContainer) this.Parent;

			if (p != null)
			{
				p.RequestDestroyObject(this, Tag, callback);
			}
			else
			{
				throw new Error_CannotGetParent(this);
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
		/// <exception cref="Error_ResourceNotOnServer">
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a remove a resource that has
		/// a null/empty value for its contentUri.
		/// </exception>
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the parent of this object is not a <see cref="CpMediaContainer"/>.
		/// </exception>
		public void RequestDeleteResource(ICpResource deleteThisResource, object Tag, CpMediaDelegates.Delegate_ResultDeleteResource callback)
		{
			// calls parent's implementation of method
			CpMediaContainer parent = (CpMediaContainer) this.Parent;
			parent.RequestDeleteResource(deleteThisResource, Tag, callback);
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
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		public void RequestExportResource(ICpResource exportThis, System.Uri sendHere, object Tag, CpMediaDelegates.Delegate_ResultExportResource callback)
		{
			// calls parent's implementation of method
			CpMediaContainer parent = (CpMediaContainer) this.Parent;
			parent.RequestExportResource(exportThis, sendHere, Tag, callback);
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
		/// Thrown when attempting to delete a resource object that
		/// is not part of the server's content hierarchy.
		/// </exception>
		/// <exception cref="NullReferenceException">
		/// Thrown when attempting a remove a resource that has
		/// a null/empty value for its contentUri.
		/// </exception>
		/// <exception cref="Error_CannotGetParent">
		/// Thrown if the parent of this object is null.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the parent of this object is not a <see cref="CpMediaContainer"/>.
		/// </exception>
		public void RequestImportResource (System.Uri sourceUri, ICpResource importHere, object Tag, CpMediaDelegates.Delegate_ResultImportResource callback)
		{
			// calls parent's implementation of method
			CpMediaContainer parent = (CpMediaContainer) this.Parent;
			parent.RequestImportResource(sourceUri, importHere, Tag, callback);
		}

		/// <summary>
		/// Calls the parent object's implementation of <see cref="CpMediaContainer.CheckRuntimeBindings"/>.
		/// </summary>
		/// <param name="st">stack trace object, created in the method that desires runtime checking</param>
		public override void CheckRuntimeBindings(StackTrace st)
		{
			if (this.Parent != null)
			{
				CpMediaContainer parent = (CpMediaContainer) this.Parent;

				parent.CheckRuntimeBindings(st);
			}
		}

		/// <summary>
		/// Control-point applications are never guaranteed to know the underlying
		/// item of a refererring item. For this reason, the LookupRefItem() method
		/// is overridden to do a lookup of the referred item any item the
		/// referred item is requested.
		/// </summary>
		/// <returns>an instance of CpMediaItem, if the referred item was found</returns>
		public override IMediaItem RefItem
		{
			get
			{
				if (this.GetUnderlyingItem != null)
				{
					return this.GetUnderlyingItem(this, this.m_RefID);
				}
				else
				{
					return null;
				}
			}
			set
			{
				this.CheckRuntimeBindings(new StackTrace());

			}
		}
		
		/// <summary>
		/// The ability to modify objects directly to a container/item is not available
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// for a public programmer. Each CpMediaItem object is responsible
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
		/// Increments the number of spiders that have indicated
		/// this item to be of interest to them.
		/// This method is called by Sink_ResultBrowse.
		/// </summary>
		internal void IncrementSpiderMatches()
		{
			System.Threading.Interlocked.Increment(ref this.m_SpiderClients);
		}

		/// <summary>
		/// Decrements the number of spiders that have indicated
		/// this item to be of interest to them.
		/// This method is called by CdsSpider to indicate
		/// that the spider is not interested in the container anymore.
		/// If no spiders are interested in the item, then
		/// the item is removed.
		/// </summary>
		internal void DecrementSpiderMatches()
		{
			System.Threading.Interlocked.Decrement(ref this.m_SpiderClients);
			if (this.m_SpiderClients == 0)
			{
				CpMediaContainer p = (CpMediaContainer) this.Parent;
				if (p != null)
				{
					p.RemoveObject(this);
				}
			}
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
		/// Indicates that no spiders have marked this item to remain in memory.
		/// </summary>
		internal bool PleaseRemove
		{
			get
			{
				return this.m_bools[(int) EnumBoolsCpMediaItem.PleaseRemove];
			}
			set
			{
				this.m_bools[(int) EnumBoolsCpMediaItem.PleaseRemove] = value;
			}
		}

		/// <summary>
		/// The number of spiders that are interested in this item.
		/// </summary>
		[NonSerialized()] private long m_SpiderClients = 0;

		public long SpiderClients { get { return this.m_SpiderClients; } }

		/// <summary>
		/// Enumerates through m_bools
		/// </summary>
		private enum EnumBoolsCpMediaItem
		{
			PleaseRemove = MediaItem.EnumBoolsMediaItem.IgnoreLast,
			IgnoreLast
		}

		/// <summary>
		/// This delegate is a means to perform a lookup on a application managed table
		/// of <see cref="CpMediaItem"/> objects. 
		/// Programmers who wish to allow <see cref="CpMediaItem"/> to know about
		/// their underlying items will need to track that information themselves
		/// and wire this delegate up to a method that will return the appropriate
		/// object for a given item id.
		/// </summary>
		[NonSerialized()] public LookupUnderlyingItemHandler GetUnderlyingItem = null;

		/// <summary>
		/// Used for GetUnderlyingItem field.
		/// </summary>
		public delegate CpMediaItem LookupUnderlyingItemHandler (CpMediaItem caller, string itemID);

		/// <summary>
		/// A static reference to the type of a CpMediaResource instance.
		/// </summary>
		private static System.Type TYPE_CP_RESOURCE = typeof (CpMediaResource);
	}
}
