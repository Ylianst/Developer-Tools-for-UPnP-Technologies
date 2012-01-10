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
using OpenSource.UPnP.AV;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// <para>
	/// This class is a <see cref="DvMediaContainer"/>
	/// with the specific role of acting as a root container for the
	/// entire content hierarchy for a UPNP-AV MediaServer based
	/// on the <see cref="MediaServerDevice"/>
	/// implementation.
	/// </para>
	/// 
	/// <para>
	/// Public programmers do not instantiate an instance
	/// of this class, rather relying on the 
	/// <see cref="MediaServerDevice"/> 
	/// to provide the instance. Programmers still have
	/// the entire range of methods applicable to a 
	/// <see cref="DvMediaContainer"/>,
	/// so the management of this container's metadata, child items/containers, and
	/// resources is the same as any other container.
	/// </para>
	/// 
	/// <para>
	/// The entire purpose of a DvRootContainer is actually to enable the
	/// <see cref="MediaServerDevice"/> 
	/// implementation to know when containers and items
	/// </para>
	/// </summary>
	[Serializable()]
	internal class DvRootContainer : DvMediaContainer
	{
		/// <summary>
		/// Default constructor
		/// </summary>
		public DvRootContainer()
		{
		}

		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// Serialized MediaContainer objects do not have their child objects
		/// serialized with them.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		protected DvRootContainer(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
			: base (info, context)
		{
			// Base class constructor calls Init() so all fields are
			// initialized. Nothing else to serialize.
		}

		/// <summary>
		/// Always returns true.
		/// </summary>
		public override bool IsRootContainer
		{
			get
			{
				return true;
			}
		}
		
		/// <summary>
		/// This delegate is used when eventing changes about the content hierarchy.
		/// The root container of the system advertising the change, and the
		/// container that changed are sent as arguments.
		/// </summary>
		internal delegate void Delegate_OnContainerChanged(DvRootContainer sender, DvMediaContainer thisChanged);

		/// <summary>
		/// This event is fired when a
		/// <see cref="DvMediaContainer"/> 
		/// observes any of the following scenarios.
		/// <list type="bullet">
		/// <item><description>Parent container observes a change in a child item/container's metadata.</description></item>
		/// <item><description>Parent container observes a change in a child item/container's resources.</description></item>
		/// <item><description>Parent container observes a change in the list of child items/containers.</description></item>
		/// </list>
		/// In all cases, the parent container that observes the change is sent as an argument, along with the root container.
		/// Containers that have a change in their own metadata do not event themselves. Eventing is always done by a parent
		/// container, unless it is the root container.
		/// </summary>
		internal event Delegate_OnContainerChanged OnContainerChanged;

		/// <summary>
		/// <see cref="DvMediaContainer"/> objects
		/// use this method to event changes in their state. This
		/// method simply fires the 
		/// <see cref="DvRootContainer.OnContainerChanged"/>
		/// event.
		/// </summary>
		/// <param name="thisChanged"></param>
		internal void FireOnContainerChanged (DvMediaContainer thisChanged)
		{
			if (this.OnContainerChanged != null)
			{
				this.OnContainerChanged(this, thisChanged);
			}
		}
	}
}
