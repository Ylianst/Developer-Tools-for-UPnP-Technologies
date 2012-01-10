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
using System.Reflection;
using System.Collections;
using System.Diagnostics;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Interface for all DvMediaXXX classes, including:
	/// <list type="bullet">
	/// <item><term><see cref="DvMediaContainer"/></term></item>
	/// <item><term><see cref="DvMediaItem"/></term></item>
	/// <item><term><see cref="DvRootContainer"/></term></item>
	/// Public programmers should not attempt to implement classes
	/// that implement this interface with intention to use those
	/// derived classes with DvMediaContainer, DvMediaItem, and
	/// DvRootContainer.
	/// </list>
	/// 
	/// Programmers should restrict their use of 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.IUPnPMedia"/>
	/// and use ICpMedia or IDvMedia when attempting to 
	/// consolidate code with the use of interfaces.
	/// </summary>
	public interface IDvMedia : IUPnPMedia
	{
		/// <summary>
		/// Used internally.
		/// The purpose of this method is for the thread
		/// to traverse up to the root container of the
		/// object that changed and to have the root
		/// container properly execute code that will
		/// cause the <see cref="MediaServerDevice">
		/// class to properly event a change event
		/// onto the UPnP network.
		/// </summary>
		void NotifyRootOfChange();

		/// <summary>
		/// Changes this object's parent to another container.
		/// Strongly recommended that this container already
		/// share the object's current hierarchy.
		/// </summary>
		/// <param name="diffParent"></param>
		void ChangeParent(IDvContainer diffParent);
	}

	/// <summary>
	/// Represents a media item for use with device-side representations
	/// of content hierarchies.
	/// </summary>
	public interface IDvItem : IMediaItem, IDvMedia
	{
		/// <summary>
		/// Returns true if an item has been marked for removal from
		/// the hierarchy but has yet to be removed. There can sometimes be 
		/// a short delay between the time an item is marked and when it actually
		/// can get removed because an item must always instruct its 
		/// referring items to remove themselves before the underlying/referenced item
		/// can be removed.
		/// </summary>
		bool IsDeletePending { get; }

		/// <summary>
		/// Creates a reference to this IDvItem.
		/// </summary>
		IDvItem CreateReference();
		
		/// <summary>
		/// Thread-synchronization method that should be called before <see cref="IDvItem.CreateReference"/>. 
		/// Items need to be thread-safe because
		/// items should not have logic errors from add, remove, and create reference operations.
		/// </summary>
		void LockReferenceList();

		/// <summary>
		/// Thread-synchronization method that should be called after <see cref="IDvItem.CreateReference"/>. 
		/// Items need to be thread-safe because
		/// items should not have logic errors from add, remove, and create reference operations.
		/// </summary>
		void UnlockReferenceList();

		/// <summary>
		/// This method tells all other items that refer to this item to remove themselves
		/// from their parent containers.
		/// </summary>
		void NotifyPendingDelete();

		/// <summary>
		/// Returns a shallow-copy thread-safe listing of <see cref="IDvItem"/>
		/// objects that point to this item.
		/// </summary>
		IList ReferenceItems { get; }
	}

	/// <summary>
	/// Represents a media container for use with device-side representations
	/// of content hierarchies.
	/// </summary>
	public interface IDvContainer : IMediaContainer, IDvMedia
	{
		/// <summary>
		/// This event fires whenever a child object has been removed.
		/// Programmers may prefer to use <see cref="IDvContainer.OnChildrenToRemove"/>
		/// because this event fires after a media object has been
		/// formerly removed from its parent container.
		/// 
		/// As a result the properties that point to other media objects may be
		/// null. As a corollary, merged properties and resources may
		/// no longer contain information about a formerly referenced item.
		/// </summary>
		event DvDelegates.Delegate_OnChildrenRemove OnChildrenRemoved;

		/// <summary>
		/// This event fires whenever a child object is about to be removed.
		/// </summary>
		event DvDelegates.Delegate_OnChildrenRemove OnChildrenToRemove;

		/// <summary>
		/// Adds a new child item that points to another item in the content hierarchy.
		/// </summary>
		IDvItem AddReference (IDvItem refItem);

		/// <summary>
		/// Adds an item, container, or a complete content subtree to this container.
		/// If the branch is a multiple-item subtree, then the programmer should
		/// take care to ensure that the proposed sub-tree is stable by itself.
		/// <para>
		/// This method is somewhat different than <see cref="IMediaContainer.AddObject"/>()
		/// in that the added branch is assigned a new unique object id.
		/// </para>
		/// </summary>
		/// <param name="branch">An item, container, or multiple-item subtree.</param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the branch is not a <see cref="IDvItem"/> or a <see cref="IDvContainer"/>.
		/// </exception>
		void AddBranch(IDvMedia branch);

		/// <summary>
		/// Similar to <see cref="IDvContainer.AddBranch"/> except that it adds multiple
		/// items, containers, or subtrees to the container.
		/// </summary>
		/// <param name="branches"></param>
		void AddBranches(ICollection branches);

		/// <summary>
		/// <para>
		/// Removes an item, container, or a complete content subtree from this container.
		/// If the branch is a multiple-item subtree, then the effect is
		/// recursive and immediately reflected in the advertised content hierarchy.
		/// </para>
		/// <para>
		/// For all intents and purposes, this method behaves exactly like
		/// <see cref="IMediaContainer.RemoveObject"/>.
		/// </para>
		/// </summary>
		void RemoveBranch(IDvMedia branch);
	}

	/// <summary>
	/// Provides a means of defining delegates that are required by
	/// <see cref="IDvMedia"/> types.
	/// </summary>
	public abstract class DvDelegates
	{
		/// <summary>
		/// Delegate is used when child objects are about to be removed or have been removed.
		/// The collection contains elements of type <see cref="IUPnPMedia"/>.
		/// </summary>
		public delegate void Delegate_OnChildrenRemove (IDvContainer parent, ICollection removedThese);
	}
}
