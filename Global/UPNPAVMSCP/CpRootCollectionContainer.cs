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
using System.Reflection;
using System.Diagnostics;
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
	/// WARNING: DO NOT EVER EVER EVER EVER add a CpRootCollectionContainer
	/// as a child to another CpRootCollectionContainer.
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
	/// This class is not designed to be managed in the same manner
	/// as other <see cref="CpMediaContainer"/> objects because
	/// an aggregation of all root containers cannot have an associated
	/// <see cref="CpMediaServer"/> object. Thus the class is designed
	/// to always persist all root containers, irrespective of what
	/// spiders may be monitoring the collection.
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
	/// 
	/// <para>
	/// CpRootCollectionContainers are subject to the same 
	/// limitations as <see cref="CpRootContainer"/> when
	/// it comes to serialization. Programmers are instructed
	/// to not serialize CpRootCollectionContainers as this
	/// serializing from a storage will not yield the desired children.
	/// </para>
	/// </summary>
	[Serializable()]
	public sealed class CpRootCollectionContainer : CpRootContainer, IUPnPMedia
	{
		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		public override void ClearItems()
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="addThis">the CpMediaResource object with a corresponding resource advertised by the MediaServer</param>
		public override void AddResource(IMediaResource addThis)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="removeThis">the MediaResource to remove</param>
		public override void RemoveResource(IMediaResource removeThis)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="newResources">a collection of CpMediaResource</param>
		public override void AddResources(ICollection newResources)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="removeThese"></param>
		public override void RemoveResources(ICollection removeThese)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="addThis"></param>
		/// <param name="overwrite"></param>
		public override void AddObject(IUPnPMedia addThis, bool overwrite)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="removeThis"></param>
		public override void RemoveObject(IUPnPMedia removeThis)
		{
			CheckProtection();
		}		

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="addThese"></param>
		/// <param name="overwrite"></param>
		public override void AddObjects(ICollection addThese, bool overwrite)
		{
			CheckProtection();
		}
		
		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="removeThese"></param>
		public override void RemoveObjects(ICollection removeThese)
		{
			CheckProtection();
		}		

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateMetadata(XmlElement xmlElement)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		public override void UpdateMetadata(string DidlLiteXml)
		{
			CheckProtection();
		}
		
		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="newObj"></param>
		public override void UpdateObject(IUPnPMedia newObj)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="xmlElement"></param>
		public override void UpdateObject(XmlElement xmlElement)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		public override void UpdateObject (string DidlLiteXml)
		{
			CheckProtection();
		}
		
		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="writeStatus"></param>
		public override void SetWriteStatus(EnumWriteStatus writeStatus)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="classType"></param>
		/// <param name="friendlyName"></param>
		public override void SetClass(string classType, string friendlyName)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue (string propertyName, IList values)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_String(string propertyName, string[] values)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_String(string propertyName, string val)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_Int(string propertyName, int[] values)
		{
			CheckProtection();
		}
		
		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_Int(string propertyName, int val)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_Long(string propertyName, long[] values)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_Long(string propertyName, long val)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="values"></param>
		public override void SetPropertyValue_MediaClass(string propertyName, MediaClass[] values)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception. CpRootCollectionContainers
		/// are not like normal containers because they
		/// aggregate root containers. For this reason
		/// the ability to modify properties, resources,
		/// and metadata is NEVER allowed since 
		/// there is no actual metadata to retrieve or set.
		/// </summary>
		/// <param name="propertyName"></param>
		/// <param name="val"></param>
		public override void SetPropertyValue_MediaClass(string propertyName, MediaClass val)
		{
			CheckProtection();
		}

		/// <summary>
		/// Throws an exception because no state can be
		/// associated with a collection of containers.
		/// </para>
		/// </summary>
		public new StateInfo State 
		{ 
			get 
			{
				throw new Error_MetadataCallerViolation();
			} 
			set
			{
				
			}
		}

		private void CheckProtection()
		{
			StackTrace st = new StackTrace();

			StackFrame sf = st.GetFrame(1);

			MethodBase mb = sf.GetMethod();

			Type mt = mb.DeclaringType;
			Type thisType = this.GetType();
			bool ok = false;
			if (mt.Namespace == (thisType.Namespace))
			{
				if (mt.Assembly == thisType.Assembly)
				{
					if (mt.DeclaringType == thisType.DeclaringType)
					{
						ok = true;
					}
				}
			}

			if (!ok)
			{
				throw new Error_MetadataCallerViolation();
			}
		}

		public override string ServerFriendlyName
		{
			get
			{
				System.Text.StringBuilder sb = new System.Text.StringBuilder();

				lock (this.m_Roots)
				{
					int x = 0;
					foreach (CpRootContainer root in this.m_Roots)
					{
						if (x > 0)
						{
							sb.AppendFormat(",{0}", root.ServerFriendlyName);
						}
						else
						{
							sb.Append(root.ServerFriendlyName);
						}
						x++;
					}
				}

				return sb.ToString();
			}
		}
		internal void AddRootContainer(CpRootContainer root)
		{
			lock (this.m_Roots)
			{
				if (root.Parent != null)
				{
					throw new Error_MediaObjectHasParent(root);
				}
				this.m_Roots.Add(root);
				root.SetParent(this);
			}

			IList spiders = this.GetActiveSpiders();
			if (spiders != null)
			{
				Hashtable table = new Hashtable(spiders.Count);
				ArrayList addThis = new ArrayList(1);
				addThis.Add(root);
				foreach (CdsSpider spider in spiders)
				{
					if (spider.IsMatch(root))
					{
						root.IncrementSpiderMatches();
						table[spider] = addThis;
					}
				}

				this.NotifySpidersAdd(table);
			}
		}

		internal void RemoveRootContainer(CpRootContainer root)
		{
			lock (this.m_Roots)
			{
				this.m_Roots.Remove(root);
				root.SetParent (null);
				root.NotifySpidersOfGoneContainer();
			}

			IList spiders = this.GetActiveSpiders();
			if (spiders != null)
			{
				ArrayList removeThese = new ArrayList(1);
				removeThese.Add(root);

				this.NotifySpidersRemove(spiders, removeThese);
			}
		}
		
		public CpRootCollectionContainer()
		{
			PropertyString ps = new PropertyString(T[CdsMetadata._DC.title], "All MediaServers");
			this.m_ID = System.Guid.NewGuid().ToString();
			ArrayList al = new ArrayList();
			al.Add(ps);
			this.SetClass("object.container", "");
			this.m_Properties[T[CdsMetadata._DC.title]] = al;
		}

		public override IList CompleteList
		{
			get
			{
				ArrayList result;
				lock (this.m_Roots)
				{
					result = (ArrayList) this.m_Roots.Clone();
				}
				return result;
			}
		}

		public override IList Items
		{
			get
			{
				return new ArrayList();
			}
		}

		public override IList Containers
		{
			get
			{
				return this.CompleteList;
			}
		}

		/// <summary>
		/// No-op.
		/// </summary>
		/// <param name="br"></param>
		/// <param name="callback"></param>
		public override void RequestBrowse(BrowseRequest br, CpContentDirectory.Delegate_OnResult_Browse callback)
		{
		}

		public override void RequestDestroyObject (DestroyObjectRequest request, CpContentDirectory.Delegate_OnResult_DestroyObject callback)
		{
		}

		public override void RequestDestroyObject (object Tag, CpMediaDelegates.Delegate_ResultDestroyObject callback)
		{
		}

		public override void RequestUpdateObject (ICpMedia changeThisChildObject, IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback)
		{
		}

		public override void RequestUpdateObject (IUPnPMedia useThisMetadata, object Tag, CpMediaDelegates.Delegate_ResultUpdateObject callback)
		{
		}

		public override void RequestUpdateObject (UpdateObjectRequest request, CpContentDirectory.Delegate_OnResult_UpdateObject callback)
		{
		}

		/// <summary>
		/// Overridden because a root collection container is not
		/// managed in the same manner as a typical
		/// <see cref="CpMediaContainer"/>.
		/// Immediately after adding the spider to the container's listing
		/// of spiders, the current listing of roots is compared against
		/// the spider's IsMatch() method and then the spider is notified
		/// of its matches.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="stateObject">allows additional information to be sent along with the spider</param>
		protected override void SubscribeSpider (CdsSpider spider, object stateObject)
		{
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

			ArrayList matched;
			lock (this.m_Roots)
			{
				matched = new ArrayList(this.m_Roots.Count);
				foreach (CpRootContainer r in this.m_Roots)
				{
					if (spider.IsMatch(r) == true)
					{
						matched.Add(r);
						r.IncrementSpiderMatches();
					}
				}
			}
			spider.NotifySinkAdd(this, matched);
		}

		/// <summary>
		/// Simply decrements, but never removes the root collection container.
		/// </summary>
		protected override void DecrementSpiderMatches(object ignored)
		{
			this.DecrementSpiderMatches(null);
		}

		private ArrayList m_Roots = new ArrayList();
	}
}
