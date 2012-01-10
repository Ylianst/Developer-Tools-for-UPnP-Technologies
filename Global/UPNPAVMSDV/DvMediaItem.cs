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
using System.Collections;
using System.Diagnostics;
using OpenSource.UPnP.AV;
using OpenSource.Utilities;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// <para>
	/// This class inherits all basic metadata of a ContentDirectory media item entry 
	/// (for use in representing "object.item" and derived UPNP media classes),
	/// for use in managing a content hierarchy intended for the MediaServer implementation
	/// provided in the OpenSource.UPnP.AV.CdsMetadata namespace.
	/// </para>
	/// 
	/// <para>
	/// The <see cref="MediaServerDevice"/> class
	/// owns a <see cref="DvRootContainer"/> instance. 
	/// Public programmers can add 
	/// <see cref="DvMediaItem"/>and 
	/// <see cref="DvMediaItem"/>
	/// objects to the root, thus building a content hierarchy.
	/// </para>
	/// 
	/// <para>
	/// Although some MediaServer implementations might choose to enforce a 
	/// rule where a reference item has identical metadata and resources
	/// of an underlying item, the implementation for 
	/// <see cref="MediaServerDevice"/> does not make
	/// such an assumption. However, the programmer can opt to enforce that in their
	/// own incarnation of the content hierarchy.
	/// </para>
	/// </summary>
	[Serializable()]
	public sealed class DvMediaItem : MediaItem, IDvMedia, IDvItem
	{
		/// <summary>
		/// This method should only be used when deserializing a media items.
		/// 
		/// <para>
		/// A media item can point to another media item. The item doing the pointing
		/// is called the reference item. The item that is being pointed to is the
		/// underlying item.
		/// </para>
		/// 
		/// <para>
		/// The standard binary serialization process destroys the object reference
		/// between a reference item and its underlying item. This method provides a means
		/// to restore the reference relationship.
		/// </para>
		/// </summary>
		/// <param name="underlyingItem">
		/// The media item that is being referred.
		/// </param>
		/// <param name="refItem">
		/// The media item that is doing the referring.
		/// </param>
		public static void AttachRefItem (DvMediaItem underlyingItem, DvMediaItem refItem)
		{
			underlyingItem.LockReferenceList();
			if (underlyingItem.m_ReferringItems == null)
			{
				underlyingItem.m_ReferringItems = new ArrayList();
			}
			//underlyingItem.m_ReferringItems.Add(new WeakReference(refItem));
			underlyingItem.m_ReferringItems.Add(refItem);

			// Just to avoid confusion - a refitem is actually an underlying item.
			// The naming convention follows the AV spec - the spec opted to use
			// "refID" notation instead of using "underlyingItem"...

			refItem.m_RefItem = underlyingItem;
			refItem.m_RefID = "";
			underlyingItem.UnlockReferenceList();
		}

		/// <summary>
		/// Override for set - do not allow application layer to set the ID
		/// directly. The CDS infrastructure must ensure that ID values are
		/// unique, so this ability is blocked for public use.
		/// <para>
		/// The method also causes the MediaServer to event changes on UPNP network.
		/// </para>
		/// </summary>
		public override string ID
		{
			get
			{
				return base.ID;
			}
			set
			{
#if (DEBUG)
				this.CheckRuntimeBindings(new StackTrace());
#endif
				if (string.Compare(value, base.m_ID) != 0)
				{
					base.m_ID = value;
					this.NotifyRootOfChange();
					
				}
			}
		}

		/// <summary>
		/// Override set:
		/// Checks access rights first and then calls base class implementation for set.
		/// </summary>
		/// <exception cref="InvalidCastException">
		/// Thrown if the set-operation could not cast the parent into
		/// a DvMediaContainer object.
		/// </exception>
		public override IMediaContainer Parent 
		{ 
			get
			{
				return base.Parent;
			}
			set
			{
#if (DEBUG)
				// pulling a stack trace is slow - only implement this
				// for debug builds
				this.CheckRuntimeBindings(new StackTrace());
#endif

				// we do an explicit cast first to ensure that
				// the value is a DvMediaContainer, before 
				// assigning to base.Parent because base.Parent
				// will take anything that implements IMediaContainer.
				DvMediaContainer p = (DvMediaContainer) value;
				base.Parent = p;

				// do not call NotifyRootOfChange() because this
				// method is called internally by DvMediaContainer
				// when adding child objects... there's no need to
				// event the changes for this object since the
				// parent will event changes.
			}
		}

		/// <summary>
		/// Changes this object's parent to another container.
		/// Strongly recommended that this container already
		/// share the object's current hierarchy.
		/// </summary>
		/// <param name="diffParent"></param>
		/// <exception cref="InvalidCastException">
		/// Thrown if this object was improperly added to a 
		/// container that is not a <see cref="DvMediaContainer"/>.
		/// </exception>
		public void ChangeParent(IDvContainer diffParent)
		{
			DvMediaContainer.ChangeParent2(this, (DvMediaContainer)diffParent);
		}

		/// <summary>
		/// Override set: Changing this value causes the
		/// object to event the chagne on the UPNP network.
		/// </summary>
		public override bool IsRestricted
		{
			get
			{
				return base.IsRestricted;
			}
			set
			{
				bool val = this.m_Restricted;
				this.m_Restricted = value;

				if (val != value)
				{
					this.NotifyRootOfChange();
					
				}
			}
		}

		/// <summary>
		/// Calls DvMediaContainer.CheckProtection() if the item has a parent.
		/// This method is used to prevent
		/// public programmers from doing a set() on the Parent
		/// property.
		/// </summary>
		/// <param name="st"></param>
		public override void CheckRuntimeBindings(StackTrace st)
		{
#if (DEBUG)
			if (this.Parent != null)
			{
				DvMediaContainer parent = (DvMediaContainer) this.Parent;

				parent.CheckRuntimeBindings(st);
			}
#endif
		}

		/// <summary>
		/// <para>
		/// Calls the parent container's 
		/// <see cref="DvMediaContainer.NotifyRootOfChange"/> method.
		/// </para>
		/// 
		/// <para>
		/// Enumerates through this item's reference items and instructs
		/// each to notify their parents of the change.
		/// </para>
		/// </summary>
		/// <exception cref="InvalidCastException">
		/// Thrown when parent container is not a <see cref="DvMediaContainer"/> object.
		/// </exception>
		public void NotifyRootOfChange()
		{
			DvMediaContainer mc = (DvMediaContainer) this.m_Parent;
			if (mc != null) { mc.NotifyRootOfChange(); }

			this.LockReferenceList();
			if (this.m_ReferringItems != null)
			{
				foreach (IDvItem refItem in this.m_ReferringItems)
				{
					mc = (DvMediaContainer) refItem.Parent;
					if (mc != null) { mc.NotifyRootOfChange(); }
				}
			}
			this.UnlockReferenceList();
		}


		/// <summary>
		/// Returns the item that this object points to. Will return
		/// null if no refItem is found. 
		/// </summary>
		/// <returns></returns>
		/// <exception cref="InvalidCastException">
		/// Thrown if the set() operation has a non-DvMediaItem instance as the parameter.
		/// </exception>
		public override IMediaItem RefItem
		{
			get
			{
				return this.m_RefItem;
			}
			set
			{
#if (DEBUG)
				this.CheckRuntimeBindings(new StackTrace());
#endif
				this.m_RefItem = (DvMediaItem) value;
			}
		}
		/// <summary>
		/// Default constructor. No metadata is initialized in this
		/// method. It is STRONGLY recommended that programmers
		/// use the <see cref="DvMediaBuilder"/>.CreateXXX methods to instantiate
		/// DvMediaObjects objects.
		/// </summary>
		public DvMediaItem()
		{
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
		public DvMediaItem (XmlElement xmlElement)
			: base (xmlElement)
		{
		}

		/// <summary>
		/// Makes it so that a DvMediaItem instantiated from an XmlElement
		/// instantiates its child resources as <see cref="DvMediaResource"/> objects.
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
			base.UpdateEverything(true, true, typeof(DvMediaResource), typeof(DvMediaItem), typeof(DvMediaContainer), xmlElement, out children);
			if (this.m_ID.StartsWith(MediaBuilder.Seed) == false)
			{
				this.m_ID = MediaBuilder.GetUniqueId();
			}
		}

		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// Serialized MediaItem objects do not have their child objects
		/// serialized with them. Ignore the compiler warning about a protected
		/// member in a sealed class as the MSDN documentation says to make this
		/// constructor protected.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		private DvMediaItem(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
			: base (info, context)
		{
		}

		/// <summary>
		/// Custom serializer - required for ISerializable.
		/// Serializes all fields that are not marked as [NonSerialized()].
		/// Some fields were originally marked as [NonSerialized()] because
		/// this class did not implement ISerializable. I've continued to
		/// use the attribute in the code.
		/// 
		/// Serialized DvMediaItem objects do not save any information 
		/// about an underlying item's reference items.
		/// As a corollary, DvMediaItem instances do not save any pointers
		/// to their underlying items.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		public override void GetObjectData(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
		{
			base.GetObjectData(info, context);
		}

		/// <summary>
		/// Calls base class implementation of Init()
		/// and then initializes the fields for this class.
		/// </summary>
		protected override void Init()
		{
			base.Init();
			this.m_LockReferences = new object();
			this.m_ReferringItems = null;
			this.m_Deleting = false;
			this.m_RefItem = null;
		}

		/// <summary>
		/// Checks that the resource is a <see cref="IDvResource"/> object.
		/// Calls base class and notifies owner of change.
		/// </summary>
		/// <param name="newResource"></param>
		/// <exception cref="InvalidCastException">
		/// Thrown if newResource is not a IDvResource.
		/// </exception>
		public override void AddResource(IMediaResource newResource)
		{
			// cast and throw exception if needed
			IDvResource res = (IDvResource) newResource;
			base.AddResource(res);
			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// Checks that each resource in the collection is a <see cref="IDvResource"/> object.
		/// Calls base class and notifies owner of change.
		/// </summary>
		/// <param name="newResources">
		/// A collection of <see cref="IDvResource"/> objects to add as resources.
		/// The new <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource"/> objects
		/// are best instantiated through the <see cref="DvResourceBuilder"/>.CreateDvXXX methods.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown if a resource is not a <see cref="IDvResource"/> object.
		/// </exception>
		public override void AddResources(ICollection newResources)
		{
			// iterate through new resources; throw exception is not correct type
			foreach (IDvResource res in newResources);
			base.AddResources(newResources);
			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// Removes a resource from the media item.
		/// </summary>
		/// <param name="removeThis"></param>
		public override void RemoveResource(IMediaResource removeThis)
		{
			// ArrayList.Remove does not return a boolean to indicate
			// that the resource was remove - so just assume that
			// something was removed.

			base.RemoveResource(removeThis);
			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// Calls base class and notifies owner of change.
		/// </summary>
		/// <param name="removeThese">A collection of desired 
		/// <see cref="IDvResource"/> objects for removal.
		/// </param>
		public override void RemoveResources(ICollection removeThese)
		{
			base.RemoveResources(removeThese);
			this.NotifyRootOfChange();
			
		}
		
		/// <summary>
		/// Returns true if the object has been marked for removal from 
		/// a container. DvMediaItem instances have to instruct referring
		/// DvMediaItem instances to delete themselves. For this reason,
		/// a DvMediaItem may have a period of time between it is marked
		/// for deletion and when it is actually removed from the 
		/// parent contiainer.
		/// </summary>
		public bool IsDeletePending
		{
			get
			{
				bool retVal;
				
				lock(this.m_LockReferences)
				{
					retVal = this.m_Deleting;
				}

				return retVal;
			}
		}

		/// <summary>
		/// Returns a shallow-copy thread-safe listing of <see cref="IDvItem"/>
		/// objects that point to this item.
		/// </summary>
		public IList ReferenceItems
		{
			get
			{
				ArrayList refItems;

				this.LockReferenceList();
				refItems = (ArrayList) this.m_ReferringItems.Clone();
				this.UnlockReferenceList();

				return refItems;
			}
		}

		/// <summary>
		/// Call this method before calling CreateReference().
		/// This helps to ensure thread-safety.
		/// </summary>
		public void LockReferenceList()
		{
			System.Threading.Monitor.Enter(this.m_LockReferences);
		}

		
		/// <summary>
		/// Call this method after CreateReference() has been called
		/// and the new item has been added to container that should
		/// own it. This helps to ensure thread safety.
		/// </summary>
		public void UnlockReferenceList()
		{
			System.Threading.Monitor.Exit(this.m_LockReferences);
		}

		public DvMediaReference CreateDvMediaReference ()
		{
			lock (this.m_LockReferences)
			{
				if (this.m_Deleting == false)
				{
					DvMediaReference newItem = new DvMediaReference(this);
					if (m_ReferringItems == null)
					{
						this.m_ReferringItems = new ArrayList(1);
					}

					this.m_ReferringItems.Add(newItem);

					return newItem;
				}
				else
				{
					throw new Error_PendingDeleteException(this);
				}
			}
		}

		/// <summary>
		/// This creates a new DvMediaItem instance that refers
		/// to this instance, by simply specifying the intended ID.
		/// Generally, public programmers should use the 
		/// <see cref="DvMediaContainer.AddReference"/> method
		/// to prevent object ID collisions.
		/// </summary>
		/// <returns>a new DvMediaItem instance that refers to this instance</returns>
		/// <exception cref="Error_PendingDeleteException">
		/// Thrown if this item is marked for deletion. Cannot create a reference
		/// to an item that is pending removal from the content hierarchy.
		/// </exception>
		public IDvItem CreateReference (string id)
		{
			lock(this.m_LockReferences)
			{
				if (this.m_Deleting == false)
				{
					DvMediaItem newItem = new DvMediaItem();
					newItem.m_ID = id;
					newItem.m_RefItem = this;

					if (m_ReferringItems == null)
					{
						this.m_ReferringItems = new ArrayList(1);
					}

					//this.m_ReferringItems.Add(new WeakReference(newItem));
					this.m_ReferringItems.Add(newItem);

					// Set the remaining the base metadata by using values
					// from the underlying item.
					// 
					newItem.m_Restricted = this.m_Restricted;
					newItem.SetClass(this.Class.ToString(), this.Class.FriendlyName);
					newItem.Title = this.Title;

					return newItem;
				}
				else
				{
					throw new Error_PendingDeleteException(this);
				}
			}
		}

		/// <summary>
		/// This creates a new DvMediaItem instance that refers
		/// to this instance. Public programmers should take
		/// caution when using this method. It's possible
		/// to create an item that exists in a content
		/// hierarchy that points to another item that
		/// is not in the content hierarchy.
		/// </summary>
		/// <returns>a new DvMediaItem instance</returns>
		public IDvItem CreateReference()
		{
			string id = MediaBuilder.GetUniqueId();
			return CreateReference(id);
		}

		/// <summary>
		/// <see cref="DvMediaContainer.RemoveObject"/>
		/// and <see cref="DvMediaContainer.RemoveObjects"/>
		/// call this method before removing it from its child list.
		/// This will automatically remove items that refer to this item
		/// from their respective parents.
		/// </summary>
		public void NotifyPendingDelete()
		{
			lock (this.m_LockReferences)
			{
				this.m_Deleting = true;

				ArrayList removeThese = new ArrayList();

				if (m_ReferringItems != null)
				{
					//foreach (WeakReference wr in this.m_ReferringItems)
					foreach (IDvItem referringItem in this.m_ReferringItems)
					{
						// force a strong reference of the target before checking it's alive

						//IDvItem referringItem = (IDvItem) wr.Target;
						//if (wr.IsAlive)
						//{
							IDvContainer parent = (IDvContainer) referringItem.Parent;

							if (parent != null)
							{
								parent.RemoveObject(referringItem);
							}
							else
							{
								//removeThese.Add(wr);
								removeThese.Add(referringItem);
							}
						//}
						//else
						//{
						//	removeThese.Add(wr);
						//}
					}

					//foreach (WeakReference wr in removeThese)
					foreach (IDvItem referringItem in removeThese)
					{
						//this.m_ReferringItems.Remove(wr);
						this.m_ReferringItems.Remove(referringItem);
					}
				}

				this.m_ReferringItems = null;
				this.m_RefItem = null;
			}		
		}

		/// <summary>
		/// Updates the metadata and resources of this instance to match
		/// the information of the provided 
		/// <see cref="IDvMedia"/> instance. 
		/// </summary>
		/// <param name="newObj">the object contain</param>
		/// <exception cref="InvalidCastException">
		/// Throws an exception if the provided object's 
		/// resources include non-<see cref="IDvResource"/> objects.
		/// </exception>
		public override void UpdateObject (IUPnPMedia newObj)
		{
			foreach (IDvResource res in newObj.Resources);
			base.UpdateObject(newObj);

			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// Implementation is as follows.
		/// <para>
		/// <code>
		/// ArrayList proposedChildren;
		/// this.UpdateEverything(false, false, typeof(DvMediaResource), typeof(DvMediaItem), typeof(DvMediaContainer), xmlElement, out proposedChildren);
		/// </code>
		/// </para>
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateMetadata(XmlElement xmlElement)
		{
			ArrayList proposedChildren;
			this.UpdateEverything(false, false, typeof(DvMediaResource), typeof(DvMediaItem), typeof(DvMediaContainer), xmlElement, out proposedChildren);
		}

		/// <summary>
		/// <para>
		/// Implementation is as follows.
		/// <code>
		/// ArrayList proposedChildren;
		/// this.UpdateEverything(true, false, typeof(DvMediaResource), typeof(DvMediaItem), typeof(DvMediaContainer), xmlElement, out proposedChildren);
		/// </code>
		/// </para>
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateObject(XmlElement xmlElement)
		{
			ArrayList proposedChildren;
			this.UpdateEverything(true, false, typeof(DvMediaResource), typeof(DvMediaItem), typeof(DvMediaContainer), xmlElement, out proposedChildren);
		}


		/// <summary>
		/// Override - Implementation will call <see cref="DvMediaItem.UpdateMediaMetadata"/>
		/// if the delegate is non-null. The delegate is executed before the base class 
		/// the XML is written. The implementation is also responsible for printing
		/// the XML in such a way that each automapped resource is printed once for each
		/// network interface.
		/// </summary>
		/// <param name="formatter">
		/// A <see cref="ToXmlFormatter"/> object that
		/// specifies method implementations for printing
		/// media objects and metadata.
		/// </param>
		/// <param name="data">
		/// This object should be a <see cref="ToXmlDataDv"/>
		/// object that contains additional instructions used
		/// by this implementation.
		/// </param>
		/// <param name="xmlWriter">
		/// The <see cref="XmlTextWriter"/> object that
		/// will format the representation in an XML
		/// valid way.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the "data" argument is not a <see cref="ToXmlDataDv"/> object.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the one of the UpdateStoragexxx delegates needs to get executed
		/// whilst the provided value for the metadata is not a PropertyULong instance.
		/// </exception>
		public override void WriteInnerXml(ToXmlFormatter formatter, object data, XmlTextWriter xmlWriter)
		{
			// To prevent constant updating of metadata for a media object, 
			// we don't have a callback for updating item metadata, unlike
			// DvMediaContainer. DvMediaItem relies on the parent container
			// to update the metadata of the item.

			ToXmlDataDv txdv = (ToXmlDataDv) data;

			InnerXmlWriter.WriteInnerXml
				(
				this, 
				new InnerXmlWriter.DelegateWriteProperties(InnerXmlWriter.WriteInnerXmlProperties),
				new InnerXmlWriter.DelegateShouldPrintResources(this.PrintResources),
				new InnerXmlWriter.DelegateWriteResources(InnerXmlWriterDv.WriteInnerXmlResources),
				new InnerXmlWriter.DelegateWriteDescNodes(InnerXmlWriter.WriteInnerXmlDescNodes),
				formatter,
				txdv,
				xmlWriter
				);
		}

		/// <summary>
		/// Method executes when m_Properties.OnMetadataChanged fires.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="stateNumber"></param>
		protected override void Sink_OnMediaPropertiesChanged (MediaProperties sender, int stateNumber)
		{
			base.UpdateCache();
			this.NotifyRootOfChange();
		}

		/// <summary>
		/// Returns a IDvResource object assocated with the item.
		/// </summary>
		/// <param name="resourceID">the resource ID of the desired IDvResource</param>
		/// <returns>The IDvResource instance, or null if it doesn't exist.</returns>
		internal IDvResource GetResource(string resourceID)
		{
			IDvResource retVal = null;
			this.m_LockResources.AcquireReaderLock(-1);

			if (this.m_Resources != null)
			{
				foreach (IDvResource res in this.m_Resources)
				{
					if (res.ResourceID == resourceID)
					{
						retVal = res;
						break;
					}
				}
			}

			this.m_LockResources.ReleaseReaderLock();
			return retVal;
		}

		public override void TrimToSize()
		{
			base.TrimToSize();

			lock(this.m_LockReferences)
			{
				if (this.m_ReferringItems != null)
				{
					this.m_ReferringItems.TrimToSize();
				}
			}
		}

		/// <summary>
		/// This locks the m_ReferringItems list for reading and writing.
		/// </summary>
		[NonSerialized()] private object m_LockReferences = new object();

		/// <summary>
		/// This keeps a listing of all DvMediaItems that were created
		/// through the CreateReference() method.
		/// </summary>
		[NonSerialized()] private ArrayList m_ReferringItems = null;

		/// <summary>
		/// This flag indicates whether the item has been marked
		/// for removal from the parent container.
		/// </summary>
		[NonSerialized()] private bool m_Deleting = false;

		/// <summary>
		/// Items can refer to other items. This class allows
		/// us to keep a reference to the actual item that
		/// is being referred.
		/// </summary>
		[NonSerialized()] internal MediaItem m_RefItem = null;
	}
}
