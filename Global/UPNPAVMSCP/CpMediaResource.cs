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
using System.Diagnostics;
using OpenSource.UPnP.AV;
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Adds the RequestDeleteResource() method.
	/// </summary>
	public interface ICpResource : IMediaResource
	{
		/// <summary>
		/// Makes a request to a remote media server to delete the resource from its local file system.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestDeleteResource(object Tag, CpMediaDelegates.Delegate_ResultDeleteResource callback);

		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// the resource URI represented by this object.
		/// </summary>
		/// <param name="sourceUri">the URI where binary should be pulled</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		void RequestImportResource(System.Uri sourceUri, object Tag, CpMediaDelegates.Delegate_ResultImportResource callback);
	}

	/// <summary>
	/// <para>
	/// This class is a 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource"/>
	/// implementation intended for use with the 
	/// <see cref="CpMediaServer"/>
	/// class. CpMediaResource objects can be used with
	/// <list type="bullet">
	/// <item>
	/// <description>
	/// <see cref="CpMediaContainer.RequestAddResource"/>
	/// </description>
	/// </item>
	/// 
	/// <item>
	/// <description>
	/// <see cref="CpMediaContainer.RequestRemoveResource"/>
	/// </description>
	/// </item>
	/// 
	/// <item>
	/// <description>
	/// <see cref="CpMediaItem.RequestAddResource"/>
	/// </description>
	/// </item>
	/// 
	/// <item>
	/// <description>
	/// <see cref="CpMediaItem.RequestRemoveResource"/>
	/// </description>
	/// </item>
	/// </list>
	/// methods to request a remote MediaServer to add resources (if it
	/// so supports the feature).
	/// </para>
	/// 
	/// <para>
	/// CpMediaResource objects are also instantiated and added directly to
	/// <see cref="CpMediaItem"/> and
	/// <see cref="CpMediaContainer"/>
	/// objects by a
	/// <see cref="CpMediaServer"/>
	/// object. Public programmers should never attempt to add resources
	/// directly, as will cause mismatches in the mirroring of a content
	/// hierarchy on the control-point side.
	/// </para>
	/// 
	/// <para>
	/// A public programmer can instantiate a CpMediaResource object
	/// by using the 
	/// <see cref="CpResourceBuilder"/>.CreateXXX 
	/// methods, or can instantiate one using a
	/// constructor and setting values manually.
	/// </para>
	/// </summary>
	[Serializable()]
	public sealed class CpMediaResource : MediaResource, ICpResource
	{
		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override object this[_RESATTRIB attrib]
		{
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base[attrib] = value;
			}
		}
		
		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override object this[string attrib]
		{
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base[attrib] = value;
			}
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override string ContentUri	
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.m_ContentUri = value;
			}
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override string ImportUri 
		{
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.ImportUri = value;
			}
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override ProtocolInfoString ProtocolInfo	
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.m_ProtocolInfo = value;
			}
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _UInt Bitrate 
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.Bitrate = value;
			}
		}
		
		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _UInt BitsPerSample 
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.BitsPerSample = value;
			}		
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _UInt ColorDepth 
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.ColorDepth = value;
			}			
		}
		
		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _TimeSpan Duration
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.Duration = value;
			}			
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _UInt nrAudioChannels
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.nrAudioChannels = value;
			}			
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override string Protection
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.Protection = value;
			}			
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override ImageDimensions Resolution
		{
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.Resolution = value;
			}		
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _UInt SampleFrequency
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.SampleFrequency = value;
			}			
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override _ULong Size
		{ 
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.Size = value;
			}			
		}
		
		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override IUPnPMedia Owner
		{
			get
			{
				return this.m_Owner;
			}
			set
			{
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
				base.m_Owner = value;
			}
		}


		/// <summary>
		/// Beware of using this.
		/// </summary>
		internal CpMediaResource () {}

		/// <summary>
		/// Allows instantiation of a CpMediaResource, given an XML element
		/// conforming to the syntax and semantics defined by the
		/// UPNP-AV ContentDirectory specification.
		/// </summary>
		/// <param name="xml">"res" element, in xml form</param>
		public CpMediaResource (XmlElement xml)
			: base(xml)
		{
		}

		/// <summary>
		/// Instantiates CpMediaResource.
		/// </summary>
		/// <param name="contentUri">
		/// The URI of a resource.
		/// </param>
		/// <param name="protocolInfo">
		/// A valid protocolInfo string must have the format
		/// "[protocol]:[network]:[mime type]:[info]".
		/// </param>
		internal CpMediaResource (string contentUri, string protocolInfo)
			: base(contentUri, protocolInfo)
		{
		}

		/// <summary>
		/// Allows internal components to find the ConnectionManager
		/// service attached the MediaServer that owns this content.
		/// </summary>
		internal CpConnectionManager ServerConnectionManager
		{
			get
			{
				if (this.m_Owner != null)
				{
					CpMediaContainer cpcm;
					if (this.m_Owner.GetType() == ItemType)
					{
						CpMediaItem owner = (CpMediaItem) this.m_Owner;
						cpcm = (CpMediaContainer) owner.Parent;
					}
					else if (this.m_Owner.GetType() == ContainerType)
					{
						cpcm = (CpMediaContainer) this.m_Owner;
					}
					else throw new ApplicationException("Unexpected MediaObject type.");

					while (cpcm.Parent != null)
					{
						cpcm = (CpMediaContainer) cpcm.Parent;
					}

					CpRootContainer root = (CpRootContainer) cpcm;

					CpMediaServer server = root.Server;
					if (server != null)
					{
						return server.ConnectionManager;
					}
				}

				return null;
			}
		}

		/// <summary>
		/// Makes a request to a remote media server to delete the resource from its local file system.
		/// </summary>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the owner of this object is not an <see cref="ICpMedia"/> instance.
		/// </exception>
		public void RequestDeleteResource(object Tag, CpMediaDelegates.Delegate_ResultDeleteResource callback)
		{
			// simpy calls the owner object's implementation of the method by the same name
			ICpMedia owner = (ICpMedia) this.Owner;
			owner.RequestDeleteResource(this, Tag, callback);
		}

		/// <summary>
		/// Makes a request to a remote media server to import the binary file from a URI to 
		/// the resource URI represented by this object.
		/// </summary>
		/// <param name="sourceUri">the URI where binary should be pulled</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the owner of this object is not an <see cref="ICpMedia"/> instance.
		/// </exception>
		public void RequestImportResource (System.Uri sourceUri, object Tag, CpMediaDelegates.Delegate_ResultImportResource callback)
		{
			// simpy calls the owner object's implementation of the method by the same name
			ICpMedia owner = (ICpMedia) this.Owner;
			owner.RequestImportResource(sourceUri, this, Tag, callback);
		}

		/// <summary>
		/// Makes a request to a remote media server to export the binary file from a resource
		/// to a specified URI.
		/// </summary>
		/// <param name="destinationUri">the URI where the binary should be sent</param>
		/// <param name="Tag">
		/// Miscellaneous, user-provided object for tracking this 
		/// asynchronous call. Can be used as a means to pass a 
		/// user-defined "state object" at invoke-time so that 
		/// the executed callback during results-processing can be
		/// aware of the component's state at the time of the call.
		/// </param>
		/// <param name="callback">the callback to execute when results become available</param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the owner of this object is not an <see cref="ICpMedia"/> instance.
		/// </exception>
		public void RequestExportResource (System.Uri destinationUri, object Tag, CpMediaDelegates.Delegate_ResultExportResource callback)
		{
			// simpy calls the owner object's implementation of the method by the same name
			ICpMedia owner = (ICpMedia) this.Owner;
			owner.RequestExportResource(this, destinationUri, Tag, callback);
		}

		/// <summary>
		/// Has the System.Type value for a CpMediaItem.
		/// </summary>
		private static System.Type ItemType = typeof (CpMediaItem);

		/// <summary>
		/// Has the System.Type value for a CpMediaContainer.
		/// </summary>
		private static System.Type ContainerType = typeof (CpMediaContainer);
	}
}
