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
using System.Text;
using System.Xml;
using OpenSource.Utilities;
using OpenSource.UPnP.AV;
using System.Reflection;
using OpenSource.UPnP.AV.CdsMetadata;
using System.Runtime.Serialization;
using System.Diagnostics;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// <para>
	/// This class inherits all basic metadata of a ContentDirectory media container entry 
	/// (for use in representing "object.container" and derived UPNP media classes),
	/// for use in managing a content hierarchy intended for the MediaServer implementation
	/// provided in the OpenSource.UPnP.AV.CdsMetadata namespace.
	/// </para>
	/// 
	/// <para>
	/// The <see cref="MediaServerDevice"/> class
	/// owns a <see cref="DvRootContainer"/> instance. 
	/// Public programmers can add 
	/// <see cref="IDvMedia"/>
	/// objects to the root, thus building a content hierarchy.
	/// </para>
	/// 
	/// <para>
	/// All public operations are thread-safe and all returned
	/// data is copy-safe or the data objects provide 
	/// read-only public interfaces.
	/// </para>	
	/// </summary>
	[Serializable()]
	public class DvMediaContainer : MediaContainer, IUPnPMedia, IDvMedia, IDvContainer
	{
		/// <summary>
		/// Delegate is used so that media can update the metadata associated with 
		/// the container, the container's immediate children, the resources
		/// associated with the container, and the resources associated with
		/// the children.
		/// </summary>
		public delegate void Delegate_UpdateMetadata(DvMediaContainer media);

		/// <summary>
		/// <para>
		/// This delegate is executed in <see cref="DvMediaContainer.WriteInnerXml"/>()
		/// so that application-logic can update metadata for the container,
		/// its children or the resources of either.
		/// </para>
		/// </summary>
		[NonSerialized()] public Delegate_UpdateMetadata Callback_UpdateMetadata = null;

		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// Serialized MediaContainer objects do not have their child objects
		/// serialized with them.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		protected DvMediaContainer(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
			: base (info, context)
		{
			// Base class constructor calls Init() so all fields are
			// initialized. Nothing else to serialize.
		}

		/// <summary>
		/// Custom serializer - required for ISerializable.
		/// Serializes all fields that are not marked as [NonSerialized()].
		/// Some fields were originally marked as [NonSerialized()] because
		/// this class did not implement ISerializable. I've continued to
		/// use the attribute in the code.
		/// 
		/// Serialized DvMediaContainer objects do not have their child objects
		/// serialized with them.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		public override void GetObjectData(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
		{
			base.GetObjectData(info, context);
		}

		/// <summary>
		/// This event fires whenever a child object is about to be removed.
		/// </summary>
		public event DvDelegates.Delegate_OnChildrenRemove OnChildrenToRemove;

		/// <summary>
		/// This event fires whenever a child object is removed.
		/// </summary>
		public event DvDelegates.Delegate_OnChildrenRemove OnChildrenRemoved;

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

				// string.compare can handle null strings
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
		/// a IDvContainer object.
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
				this.CheckRuntimeBindings(new StackTrace());
#endif

				// do an explicit cast to ensure type safety
				// because base.Parent will take any IMediaContainer
				IDvContainer p = (IDvContainer) value;
				base.Parent = p;

				// do not call NotifyRootOfChange() because this
				// method is called internally by CpMediaContainer
				// when adding child objects... there's no need to
				// event the changes for this object since the
				// parent will event changes.
			}
		}

		/// <summary>
		/// Changes the target media object's parent to a different parent.
		/// </summary>
		/// <param name="target">target object</param>
		/// <param name="np">new parent</param>
		/// <exception cref="InvalidCastException">
		/// Thrown when the target's current parent is not a <see cref="DvMediaContainer"/>.
		/// </exception>
		internal static void ChangeParent2(IDvMedia target, DvMediaContainer np)
		{
			Exception error = null;
			IUPnPMedia errorDuplicate = null;
			IDvMedia[] removeThese = new IDvMedia[1];
			removeThese[0] = target;

			DvMediaContainer dvp = (DvMediaContainer) target.Parent;

			// fire about to remove event
			if (dvp.OnChildrenToRemove != null)
			{
				dvp.OnChildrenToRemove(dvp, removeThese);
			}

			// acquire locks
			dvp.m_LockListing.AcquireWriterLock(-1);
			np.m_LockListing.AcquireWriterLock(-1);

			try
			{
				// remove target from current parent
				int i = dvp.HashingMethod.Get(dvp.m_Listing, target);
				dvp.m_Listing.RemoveAt(i);
				target.Parent = null;
				if (dvp.m_Listing.Count == 0) { dvp.m_Listing = null; }

				// add target to new parent
				if (np.m_Listing == null) { np.m_Listing = new ArrayList(); }
				try
				{
					np.HashingMethod.Set(np.m_Listing, target, true);
					target.Parent = np;
				}
				catch (KeyCollisionException)
				{
					errorDuplicate = target;
				}
			}
			catch (Exception e)
			{
				error = e;
			}

			// release locks
			np.m_LockListing.ReleaseWriterLock();
			dvp.m_LockListing.ReleaseWriterLock();

			// throw exceptions if appropriate
			if (error != null)
			{
				throw new Exception("Unexpected rrror in DvMediaContainer.ChangeParent2", error);
			}

			if (errorDuplicate != null)
			{
				throw new Error_DuplicateIdException(errorDuplicate);
			}

			// fire children removed event
			if (dvp.OnChildrenRemoved != null)
			{
				dvp.OnChildrenRemoved (dvp, removeThese);
			}

			// notify upnp network that two containers changed.
			dvp.NotifyRootOfChange();
			np.NotifyRootOfChange();
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
				if (value != this.m_Restricted)
				{
					this.m_Restricted = value;
					this.NotifyRootOfChange();
					
				}
			}
		}

		/// <summary>
		/// Checks frame[0] of provided StackTrace to see
		/// whether the caller is of the same namespace
		/// and assembly. If not throws a MetadataCallerViolation
		/// exception. This method is used to prevent
		/// public programmers from doing a set() on the Parent
		/// property.
		/// </summary>
		/// <param name="st"></param>
		public override void CheckRuntimeBindings(StackTrace st)
		{
			StackFrame sf = st.GetFrame(0);

			MethodBase mb = sf.GetMethod();

			Type mt = mb.DeclaringType;
			Type thisType = this.GetType();
			bool ok = false;

			// mt.NameSpace == thisType.Namespace:
			//	Is true if the calling method is declared in the same namespace
			//	as this object's namespace. Keep in mind that "this" object may be an
			//	instance of a derived class that is not part of the OpenSource.UPnP.AV.MediaServer.DV
			//	namespace.
			//
			// mt.Namespace == (typeof(MediaObject)).Namespace):
			//	Is true if the calling method is declared in a class that is part of
			//	the OpenSource.UPnP.AV.CdsMetadata namespace. Keep in mind that a base class
			//	implementation in OpenSource.UPnP.AV.CdsMetadata may call this method.
			//
			// mt.Assembly == thisType.Assembly
			//	Is true if the calling method is declared in the same assembly
			//	as this object's assembly. Keep in mind that "this" object may be an
			//	instance of a derived class that is not part of the assembly that owns
			//	MediaObject.
			//
			// mt.Assembly == (mt.Assembly == (typeof(MediaObject)).Assembly)
			//	Is true if the calling method is declared in the same assembly that is part of
			//	the assembly that declared MediaObject. Keep in mind that a base class 
			//	implementation in OpenSource.UPnP.AV.CdsMetadata may call this method.

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
		/// Default constructor - 
		/// HashingMethod field is initialized to <see cref="MediaContainer.IdSorter"/>
		/// </summary>
		public DvMediaContainer() 
		{
			this.HashingMethod = MediaContainer.IdSorter;
		}

		/// <summary>
		/// This constructor calls the base class constructor(XmlElement), and 
		/// if and only if the type of the instances is a DvMediaContainer will
		/// the constructor call the base class implementation of UpdateEverything().
		/// Any derived classes that use this constructor will have to make the 
		/// calls to UpdateEverything() if appropriate.
		/// <para>
		/// HashingMethod field is also initialized to <see cref="MediaContainer.IdSorter"/>
		/// </para>
		/// </summary>
		/// <param name="xmlElement">XmlElement that represent a DIDL-Lite container element</param>
		public DvMediaContainer (XmlElement xmlElement)
			: base (xmlElement)
		{
			this.HashingMethod = MediaContainer.IdSorter;
		}
	
		/// <summary>
		/// Makes it so that a DvMediaContainer instantiated from an XmlElement
		/// instantiates its child resources as <see cref="DvMediaResource"/> objects,
		/// and child items and containers are <see cref="DvMediaItem"/> and <see cref="DvMediaContainer"/>.
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
			this.AddObjects(children, true);
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
		/// are best instantiated through the <see cref="OpenSource.UPnP.AV.CdsMetadata.ResourceBuilder"/>.CreateDvXXX methods.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown if a resource is not a <see cref="IDvResource"/> object.
		/// </exception>
		public override void AddResources(ICollection newResources)
		{
			foreach (IDvResource res in newResources);
			base.AddResources(newResources);
			this.NotifyRootOfChange();
			
		}

		public override void RemoveResource(IMediaResource removeThis)
		{
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
		/// AddObject() requires that the child be an <see cref="IDvMedia"/> object.
		/// </summary>
		/// <param name="newObject"></param>
		/// <param name="overWrite"></param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the added object is not an <see cref="IDvMedia"/> object.
		/// </exception>
		public override void AddObject(IUPnPMedia newObject, bool overWrite)
		{
			IDvMedia dv = (IDvMedia) newObject;
#if (DEBUG)
			this.ThrowExceptionIfBad(dv);
#endif
			base.AddObject(newObject, overWrite);

			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// AddObjects() requires that each child be an <see cref="IDvMedia"/> object.
		/// </summary>
		/// <param name="newObjects"></param>
		/// <param name="overWrite"></param>
		/// <exception cref="InvalidCastException">
		/// Thrown if an added object is not an <see cref="IDvMedia"/> object
		/// </exception>
		public override void AddObjects(ICollection newObjects, bool overWrite)
		{
#if (DEBUG)
			foreach (IDvMedia dv in newObjects)
			{
				this.ThrowExceptionIfBad(dv);
			}
#endif
			base.AddObjects(newObjects, overWrite);

			this.NotifyRootOfChange();
			
		}

		/// <summary>
		/// Throws an exception if the media object should not be added
		/// as a child.
		/// </summary>
		/// <param name="media"></param>
		/// <returns></returns>
		/// <exception cref="InvalidCastException">
		/// Thrown if the "media" argument is not an <see cref="IDvItem"/> or an <see cref="IDvContainer"/> object.
		/// Also thrown if the underlying referenced item of "media" is not an <see cref="IDvItem"/>.
		/// </exception>
		/// <exception cref="Error_PendingDeleteException">
		/// Thrown if the underlying referenced item of "media" is slated for deletion.
		/// </exception>
		private void ThrowExceptionIfBad(IDvMedia media)
		{
			if (media.IsItem)
			{
				// perform an explicit cast - throws InvalidCastException if error
				IDvItem dvItem = (IDvItem) media;

				if (media.IsReference)
				{
					IDvItem underlying = dvItem.RefItem as IDvItem;
					if (underlying == null)
					{
						throw new InvalidCastException("Cannot convert media.RefItem to IDvItem");
					}
					else if (underlying.IsDeletePending)
					{
						throw new Error_PendingDeleteException(underlying);
					}
				}
			}
			else
			{
				// perform an explicit cast - throws InvalidCastException if error
				IDvContainer dvContainer = (IDvContainer) media;
			}
		}
		
		/// <summary>
		/// Adds a new item as a child of this container.
		/// </summary>
		/// <param name="addThis"><see cref="IDvItem"/> that should be added.
		/// <see cref="IDvItem"/> instances are often instantiated using the 
		/// <see cref="OpenSource.UPnP.AV.CdsMetadata.DvMediaBuilder.CreateItem"/> method,
		/// or the <see cref="IDvItem.CreateReference"/>()
		/// method can be used also.
		/// </param>
		/// <exception cref="Error_PendingDeleteException">
		/// Thrown if the item being added is a reference to another item
		/// that is pending a delete.
		/// </exception>
		private void AddItem (IDvItem addThis)
		{
			this.AddObject(addThis, false);
		}

		/// <summary>
		/// <para>
		/// Adds a child container to this container.
		/// </para>
		/// 
		/// <para>
		/// Method was formerly public - now private.
		/// </para>
		/// </summary>
		/// <param name="addThis">the new child container to add, instantiated from 
		/// <see cref="DvMediaBuilder.CreateContainer"/>.
		/// </param>
		private void AddContainer (IDvContainer addThis)
		{
			this.AddObject(addThis, false);
		}

		/// <summary>
		/// Adds a new child item that points to another item in the content hierarchy.
		/// 
		/// <para>
		/// WARNING: This method does not check to see if the referred item is actually
		/// in the content hierarchy because of performance reasons of traversing
		/// up the ancestor's of the referred item. Programmers are warned
		/// to take caution.
		/// </para>
		/// 
		/// <para>
		/// The proper way to enforce consistency is to ensure that referred item
		/// and the container share a common ancestor.
		/// </para>
		/// </summary>
		/// <param name="underlyingItem">An item that shares a common ancestor as this container, or is a direct child of this parent.</param>
		/// <returns>A new media item (referring to "underlyingItem") that is a child of this container.</returns>
		public IDvItem AddReference (IDvItem underlyingItem)
		{
			underlyingItem.LockReferenceList();

			IDvItem newItem = underlyingItem.CreateReference();
			this.AddObject(newItem, false);

			underlyingItem.UnlockReferenceList();
			return newItem;
		}

		public DvMediaReference AddDvMediaReference (DvMediaItem underlyingItem)
		{
			underlyingItem.LockReferenceList();

			DvMediaReference newItem = underlyingItem.CreateDvMediaReference();
			this.AddObject(newItem, false);

			underlyingItem.UnlockReferenceList();

			return newItem;
		}

		/// <summary>
		/// Adds an item, container, or a complete content subtree to this container.
		/// If the branch is a multiple-item subtree, then the programmer should
		/// take care to ensure that the proposed sub-tree is stable by itself.
		/// <para>
		/// This method is somewhat different than <see cref="DvMediaContainer.AddObject"/>()
		/// in that the added branch is assigned a new unique id from 
		/// <see cref="MediaBuilder.GetUniqueId"/>(). Such a methodology absolves the
		/// application-logic from requiring <see cref="MediaBuilder.PrimeNextId"/>()
		/// to prevent object ID collisions. Programmers should be careful when mixing
		/// use of <see cref="DvMediaContainer.AddObject"/>() and
		/// AddBranch(), as improper use can still cause ID collisions. As a general rule,
		/// application logic that uses <see cref="DvMediaContainer.AddObject"/>()
		/// should always use <see cref="MediaBuilder.PrimeNextId"/>() to prime the
		/// media object counter and application logic that always uses AddBranch()
		/// need not do this.
		/// </para>
		/// </summary>
		/// <param name="branch">An item, container, or multiple-item subtree.</param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the branch is not a <see cref="IDvItem"/> or a <see cref="IDvContainer"/>.
		/// </exception>
		public virtual void AddBranch(IDvMedia branch)
		{
			branch.ID = MediaBuilder.GetUniqueId();
			this.AddObject(branch, false);
		}

		/// <summary>
		/// Optimized version of for adding multiple
		/// child items and containers at once.
		/// <para>
		/// This method is somewhat different than <see cref="DvMediaContainer.AddObjects"/>()
		/// in that the added branches are assigned a new unique ids from 
		/// <see cref="MediaBuilder.GetUniqueId"/>(). Such a methodology absolves the
		/// application-logic from requiring <see cref="MediaBuilder.PrimeNextId"/>()
		/// to prevent object ID collisions. Programmers should be careful when mixing
		/// use of <see cref="DvMediaContainer.AddObjects"/>() and
		/// AddBranches(), as improper use can still cause ID collisions. As a general rule,
		/// application logic that uses <see cref="DvMediaContainer.AddObjects"/>()
		/// should always use <see cref="MediaBuilder.PrimeNextId"/>() to prime the
		/// media object counter and application logic that always uses AddBranches()
		/// need not do this.
		/// </para>
		/// </summary>
		/// <param name="branches">ICollection of items that implement
		/// <see cref="IDvMedia"/>
		/// </param>
		/// <exception cref="Error_PendingDeleteException">
		/// Thrown if the item being added is a reference
		/// to another item that has been marked for removal.
		/// </exception>
		/// <exception cref="Error_ObjectIsContainerAndItem">
		/// Thrown if an IDvMedia object indicates it is an item
		/// as well as a container.
		/// </exception>
		/// <exception cref="InvalidCastException">
		/// Thrown if the branch is not a <see cref="IDvContainer"/> or a <see cref="IDvItem"/>.
		/// </exception>
		public virtual void AddBranches(ICollection branches)
		{
			foreach (IUPnPMedia branch in branches)
			{
				branch.ID = MediaBuilder.GetUniqueId();
			}
			this.AddObjects(branches, false);
		}

		
		/// <summary>
		/// <para>
		/// Removes an item, container, or a complete content subtree from this container.
		/// If the branch is a multiple-item subtree, then the effect is
		/// recursive and immediately reflected in the advertised content hierarchy.
		/// </para>
		/// 
		/// <para>
		/// Although the actual MediaResource objects are effectively removed
		/// from advertisment in the content hierarchy, the local binaries associated 
		/// with those resources are automatically deleted by this routine. The
		/// ContentDirectory spec indicates that the deletion of binaries is an
		/// implementation specific issue, and so this implementation requires that
		/// control-points (or internal control logic) delete the binaries in a
		/// manner external to this function.
		/// </para>
		/// </summary>
		/// <param name="branch">An item, container, or multiple-item subtree.</param>
		public virtual void RemoveBranch(IDvMedia branch)
		{
			this.RemoveObject(branch);
		}

		/// <summary>
		/// Implements the core logic for removing a single item from this container.
		/// The method will notify all items that refer to the removed item to
		/// remove themselves from their respective parent containers.
		/// Local binaries associated as resources are not deleted.
		/// Control points must call DeleteResource first.
		/// </summary>
		/// <param name="removeThis">the single item to remove</param>
		private void RemoveItem (IDvItem removeThis)
		{
			this.RemoveObject(removeThis);
		}

		/// <summary>
		/// This removes a container and all of its descendents.
		/// Any items that reference a deleted descendent will
		/// be notified to delete themselves from their parent containers.
		/// Local binaries associated as resources are not deleted.
		/// Control points must call DeleteResource first.
		/// </summary>
		/// <param name="removeThis">the container to remove</param>
		private void RemoveContainer (IDvContainer removeThis)
		{
			this.RemoveObject(removeThis);
		}

		/// <summary>
		/// This removes a container or item object from
		/// the child list. It is used by other
		/// RemoveXXX methods defined in this class
		/// and implements the portion that allows
		/// proper media server eventing.
		/// <para>
		/// Method properly tells items to notify all other
		/// items that reference it that it will be deleted.
		/// </para>
		/// <para>
		/// Method properly tells containers to recursively
		/// remove their child objects, so that descendent
		/// reference items are properly removed.
		/// </para>
		/// </summary>
		/// <param name="removeThis"></param>
		public override void RemoveObject (IUPnPMedia removeThis)
		{
			IDvMedia dv = removeThis as IDvMedia;
			IDvItem item = removeThis as IDvItem;
			IDvContainer container = removeThis as IDvContainer;

			// Before we go about doing any removal operations,
			// we fire the event to indicate this object is
			// about to be removed.
			ArrayList removeThese = new ArrayList(1);
			removeThese.Add(removeThis);
			if (this.OnChildrenToRemove != null)
			{
				this.OnChildrenToRemove(this, removeThese);
			}

			if (item != null)
			{
				// Notify any and all referring items that
				// this object is about to be removed
				// from its parent.
				item.NotifyPendingDelete();
			}
			else if (container != null)
			{
				// Instruct all child containers to
				// remove their own children.
				IList children = container.CompleteList;
				foreach (IDvMedia child in children)
				{
					container.RemoveObject(child);
				}
			}

			// go ahead and remove this object
			base.RemoveObject(removeThis);

			// Notify that the child has formerly been
			// removed.
			if (this.OnChildrenRemoved != null)
			{
				this.OnChildrenRemoved(this, removeThese);
			}

			this.NotifyRootOfChange();
			
		}
		
		/// <summary>
		/// This removes a list of container or item objects from
		/// the child list. It is used by other
		/// RemoveXXX methods defined in this class
		/// and implements the portion that allows
		/// proper media server eventing.
		/// <para>
		/// Method properly tells items to notify all other
		/// items that reference it that it will be deleted.
		/// </para>
		/// <para>
		/// Method properly tells containers to recursively
		/// remove their child objects, so that descendent
		/// reference items are properly removed.
		/// </para>
		/// </summary>
		/// <param name="removeThese"></param>
		public override void RemoveObjects (ICollection removeThese)
		{
			// Before we go about doing any removal operations,
			// we fire the event to indicate this object is
			// about to be removed.
			if (this.OnChildrenToRemove != null)
			{
				this.OnChildrenToRemove(this, removeThese);
			}

			foreach (IUPnPMedia obj in removeThese)
			{
				IDvMedia dv = obj as IDvMedia;
				IDvItem item = obj as IDvItem;
				IDvContainer container = obj as IDvContainer;

				if (item != null)
				{
					// Notify any and all referring items that
					// this object is about to be removed
					// from its parent.
					item.NotifyPendingDelete();
				}
				else if (container != null)
				{
					// Instruct all child containers to
					// remove their own children.
					IList children = container.CompleteList;
					foreach (IDvMedia child in children)
					{
						container.RemoveObject(child);
					}
				}
				// go ahead and remove this object
				base.RemoveObject(obj);
			}

			if (this.OnChildrenRemoved != null)
			{
				// Notify that the child has formerly been
				// removed.
				this.OnChildrenRemoved(this, removeThese);
			}

			this.NotifyRootOfChange();
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
		/// Returns a <see cref="IDvResource"/> object assocated with the item.
		/// </summary>
		/// <param name="resourceID">the resource ID of the desired <see cref="IDvResource"/> object</param>
		/// <returns>The <see cref="IDvResource"/> instance, or null if it doesn't exist.</returns>
		protected internal IDvResource GetResource(string resourceID)
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

		/// <summary>
		/// If this container changes, this method should be called
		/// so that the root container of the hierarchy knows that
		/// the state of the hierarchy has changed.
		/// </summary>
		public virtual void NotifyRootOfChange()
		{
			this.m_UpdateID++;
			IDvContainer c = this;
			while (c.Parent != null)
			{
				c = (IDvContainer) c.Parent;
			}

			if (c.GetType() == TYPE_DV_ROOT)
			{
				DvRootContainer root = (DvRootContainer) c;
				root.FireOnContainerChanged(this);
			}
		}

		/// <summary>
		/// Override - Implementation will call <see cref="DvMediaContainer.UpdateMediaMetadata"/>
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
			if (this.Callback_UpdateMetadata != null)
			{
				this.Callback_UpdateMetadata(this);
			}

			InnerXmlWriter.WriteInnerXml
				(
				this, 
				new InnerXmlWriter.DelegateWriteProperties(InnerXmlWriter.WriteInnerXmlProperties),
				new InnerXmlWriter.DelegateShouldPrintResources(this.PrintResources),
				// Write resources can throw InvalidCastException
				new InnerXmlWriter.DelegateWriteResources(InnerXmlWriterDv.WriteInnerXmlResources),
				new InnerXmlWriter.DelegateWriteDescNodes(InnerXmlWriter.WriteInnerXmlDescNodes),
				formatter,
				(ToXmlData) data,
				xmlWriter
				);

		}

		/// <summary>
		/// Method executes when m_Properties.OnMetadataChanged fires.
		/// 
		/// <para>
		/// The implementation calls the base class implementation of
		/// <see cref="Sink_OnMediaPropertiesChanged"/> and then
		/// fires the <see cref="OnObjectChanged"/> event, followed
		/// by a call to <see cref="NotifyRootOfChange"/>.
		/// </para>
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="stateNumber"></param>
		protected override void Sink_OnMediaPropertiesChanged (MediaProperties sender, int stateNumber)
		{
			base.Sink_OnMediaPropertiesChanged(sender, stateNumber);
			this.NotifyRootOfChange();
		}

		/// <summary>
		/// A static reference to the type of a DvRootContainer instance.
		/// </summary>
		private static System.Type TYPE_DV_ROOT = typeof(DvRootContainer);
	}
}
