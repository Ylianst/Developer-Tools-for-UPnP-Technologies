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
using OpenSource.UPnP.AV;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// <para>
	/// This class is a <see cref="CpMediaContainer"/>
	/// with the specific role of acting as a root container for the
	/// entire content hierarchy found a remote UPNP-AV MediaServer.
	/// </para>
	/// 
	/// <para>
	/// An instance of this class is owned and managed by an instance of
	/// a <see cref="CpMediaServer"/>.
	/// Public programmers should not need to instantiate an instance
	/// of this class, rather relying on the 
	/// <see cref="CpMediaServer"/> 
	/// to manage the resources, children, and metadata of the container.
	/// </para>
	/// 
	/// <para>
	/// Public programmers can subscribe to the 
	/// <see cref="CpRootContainer.OnContainerChanged"/>
	/// event for notifications about descendents. Individual 
	/// <see cref="CpMediaContainer"/> instances 
	/// do not event changes in their state, primarily to prevent race-conditions
	/// between construction time and subscription time.
	/// </para>
	/// </summary>
	[Serializable()]
	public class CpRootContainer : CpMediaContainer, IUPnPMedia
	{
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
		public delegate void Delegate_OnContainerChanged(CpRootContainer sender, CpMediaContainer thisChanged);

		/// <summary>
		/// This event is fired when 
		/// <see cref="CpMediaServer"/> 
		/// observes any of the following scenarios from a parent or root container.
		/// <list type="bullet">
		/// <item><description>Parent container observes a change in a child item/container's metadata.</description></item>
		/// <item><description>Parent container observes a change in a child item/container's resources.</description></item>
		/// <item><description>Parent container observes a change in the list of child items/containers.</description></item>
		/// </list>
		/// In all cases, the parent container that observes the change is sent as an argument, along with the root container.
		/// Containers that have a change in their own metadata do not event themselves. Eventing is always done by a parent
		/// container, unless it is the root container.
		/// </summary>
		public event Delegate_OnContainerChanged OnContainerChanged;

		/// <summary>
		/// <see cref="CpMediaContainer"/> objects
		/// use this method to event changes in their state. This
		/// method simply fires the 
		/// <see cref="CpRootContainer.OnContainerChanged"/>
		/// event.
		/// </summary>
		/// <param name="thisChanged"><see cref="CpMediaContainer"/> that observed the change.</param>
		internal void FireOnContainerChanged (CpMediaContainer thisChanged)
		{
			if (this.OnContainerChanged != null)
			{
				this.OnContainerChanged(this, thisChanged);
			}
		}

		/// <summary>
		/// <see cref="CpMediaServer"/>
		/// always instantiates a CpRootContainer with a pointer
		/// to itself. This is primarily so that a CpRootContainer
		/// can provide the server's friendly name, although
		/// future features may require more information
		/// from the CpMediaServer object.
		/// </summary>
		/// <param name="sourceMediaServer">the 
		/// <see cref="CpMediaServer"/>
		/// object that owns this root container object.
		/// </param>
		internal CpRootContainer(CpMediaServer sourceMediaServer) : base()
		{
			// base class constructor chain will call Init()
			this.m_ID = "0";
			this.m_wrSourceMediaServer = new WeakReference(sourceMediaServer, true);
			InitWeakEvents(sourceMediaServer);
		}

		/// <summary>
		/// Special ISerializable constructor.
		/// Do basic initialization and then serialize from the info object.
		/// </summary>
		/// <param name="info"></param>
		/// <param name="context"></param>
		protected CpRootContainer(SerializationInfo info, System.Runtime.Serialization.StreamingContext context)
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
		/// CpRootContainer objects do not serialize their child objects,
		/// information about spiders, events/callbacks, or even the
		/// underlying pointer to the <see cref="CpRootContainer.Server"/>
		/// property. 
		/// 
		/// In fact, it's strongly recommended that programmers NOT serialize
		/// <see cref="CpRootContainers"/> unless it's simply to cache the
		/// metadata because serializing to a CpRootContainer will not yield
		/// a CpRootContainer that is capable of making requests on a remote
		/// media server.
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
		}


		[NonSerialized()] private CpContentDirectory.StateVariableModifiedHandler_ContainerUpdateIDs weakEvent_ContainerUpdateIDs;
		[NonSerialized()] private CpContentDirectory.StateVariableModifiedHandler_SystemUpdateID weakEvent_SystemUpdateId;

		/// <summary>
		/// Method executes when a media server indicates that the tree
		/// hierarchy has changed... only executes when the service
		/// doesn't implement the OnContainerUpdateIDs state variable
		/// </summary>
		private void Sink_OnSystemUpdateIDChanged(CpContentDirectory sender, System.UInt32 NewValue)
		{
			if (sender.HasStateVariable_ContainerUpdateIDs == false)
			{
				this.ForceUpdate(false);
			}
		}

		/// <summary>
		/// Method executes when a contentdirectory events a change in a container.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="NewValue"></param>
		private void Sink_OnContainerUpdateIDsChanged(CpContentDirectory sender, System.String NewValue)
		{
			string csv_containers = NewValue;
			Hashtable cache = new Hashtable();
			DText parser = new DText();
			DText parser2 = new DText();
			parser.ATTRMARK = ",";
			parser2.ATTRMARK = ",";

			if (csv_containers != "")
			{
				parser[0] = csv_containers;
				int dcnt = parser.DCOUNT();

				for (int i=1; i <= dcnt; i++)
				{
					string id, update;
					if (Accumulator_ContainerUpdateIDs.Delimitor == ",")
					{
						id = parser[i++];
						update = parser[i];
					}
					else
					{
						string pair = parser[i];
						parser2[0] = pair;
						id = parser2[1];
						update = parser2[2];
					}

					CpMediaContainer cpc = (CpMediaContainer) this.GetDescendent(id, cache);

					if (cpc !=null)
					{
						try
						{
							UInt32 updateId = UInt32.Parse(update);
							if (updateId != cpc.UpdateID)
							{
								cpc.ForceUpdate(false);
							}
						}
						catch
						{
							cpc.ForceUpdate(false);
						}
					}
				}
			}

			cache.Clear();
		}



		
		internal CpRootContainer() : base()
		{
			//int x = 3;
		}

		private void InitWeakEvents(CpMediaServer sourceMediaServer)
		{
			this.weakEvent_ContainerUpdateIDs = new CpContentDirectory.StateVariableModifiedHandler_ContainerUpdateIDs (this.Sink_OnContainerUpdateIDsChanged);
			this.weakEvent_SystemUpdateId = new CpContentDirectory.StateVariableModifiedHandler_SystemUpdateID (this.Sink_OnSystemUpdateIDChanged);

			sourceMediaServer.ContentDirectory.OnStateVariable_ContainerUpdateIDs += this.weakEvent_ContainerUpdateIDs;
			sourceMediaServer.ContentDirectory.OnStateVariable_SystemUpdateID += this.weakEvent_SystemUpdateId;
		}

		/// <summary>
		/// Returns the friendly name of the remote MediaServer that
		/// owns the entire content hierarchy.
		/// </summary>
		public virtual string ServerFriendlyName
		{
			get
			{
				try
				{
					CpMediaServer server = this.Server;
					if (server != null)
					{
						return server.ContentDirectory.GetUPnPService().ParentDevice.FriendlyName;
					}
					return "";
				}
				catch
				{
					return "";
				}
			}
		}

		internal CpMediaServer Server
		{
			get
			{
				if (this.m_wrSourceMediaServer != null)
				{
					try
					{
						CpMediaServer server = (CpMediaServer) this.m_wrSourceMediaServer.Target;
						if (this.m_wrSourceMediaServer.IsAlive)
						{
							return server;
						}
					}
					catch
					{
						return null;
					}
				}
				return null;
			}
		}

		[NonSerialized()] private WeakReference m_wrSourceMediaServer;

		/// <summary>
		/// Returns the actual title or a server friendly name,
		/// in the case of title's that are "root", "root container", or "rootcontainer".
		/// </summary>
		public override string Title
		{
			get
			{
				string name = base.Title.ToLower();

				if (
					(name == "root") ||
					(name == "root container") ||
					(name == "rootcontainer") ||
					(name.Trim() == "")
					)
				{
					return this.ServerFriendlyName;
				}

				return base.Title;
			}
		}

		public string UDN;

		/// <summary>
		/// Returns the actual title of the root container.
		/// </summary>
		public virtual string ActualTitle
		{
			get
			{
				return base.Title;
			}
		}

		/// <summary>
		/// for token delimited text parsing
		/// </summary>
		private static DText DT = new DText();
	}
}
