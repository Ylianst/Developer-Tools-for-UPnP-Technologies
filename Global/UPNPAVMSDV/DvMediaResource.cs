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
using System.Runtime.Serialization;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Defines additional methods and properties for a <see cref="DvMediaResource"/>.
	/// </summary>
	public interface IDvResource : IMediaResource
	{
		/// <summary>
		/// Provides the relative path as reported by resource when listed in the CDS.
		/// The string value is a properly escaped UTF8 encoded URI.
		/// </summary>
		string RelativeContentUri { get; }

		/// <summary>
		/// Provides the resource ID, which uniquely identifies a resource in a device-implementation
		/// of a CDS. This field is not normative to UPnP-AV, and is used specifically for 
		/// this implementation of CDS on device-side implementations.
		/// </summary>
		string ResourceID { get; }

		/// <summary>
		/// Set this to true if the importUri attribute should be displayed in Browse/Search responses.
		/// Determines generally if the resource allows control points to post/import/replace a resource with a new binary.
		/// </summary>
		bool AllowImport { get; set; }

		/// <summary>
		/// Method takes the current ContentUri value and checks
		/// to see if the uri is an automapped file using the <see cref="MediaResource.AUTOMAPFILE"/> convention
		/// and also that the automapped file exists. If both are true
		/// then the result is true.
		/// </summary>
		/// <returns></returns>
		bool CheckLocalFileExists();

	}

	/// <summary>
	/// <para>
	/// This class is a 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource"/>
	/// implementation intended for use with the 
	/// <see cref="MediaServerDevice"/>
	/// class. DvMediaResource objects can be added to
	/// <see cref="DvMediaContainer"/>
	/// and
	/// <see cref="DvMediaItem"/>
	/// objects.
	/// </para>
	/// 
	/// <para>
	/// This class inherits all of the metadata properties for a resource, 
	/// but adds methods to modify the values associated with the metadata.
	/// </para>
	/// 
	/// <para>
	/// A public programmer can instantiate a DvMediaResource object
	/// by using the 
	/// <see cref="ResourceBuilder"/>.CreateDvXXX 
	/// methods, or can instantiate one using a
	/// constructor and setting values manually.
	/// </para>
	/// </summary>
	[Serializable()]
	public class DvMediaResource : MediaResource, IDvResource
	{
		/// <summary>
		/// This delegate is used when a DvMediaResource needs to know if
		/// an automapped file still exists. 
		/// </summary>
		public delegate bool Delegate_AutomapFileExists(DvMediaResource res);

		/// <summary>
		/// This delegate field is executed when a DvMediaResource needs to know
		/// if an automapped file still exists. This method is only called when
		/// the contentUri does not translate into a local file (after removing the AUTOMAPFILE
		/// string and a trailing query string). 
		/// 
		/// <para>
		/// For example the framework cannot tell if
		/// AUTOMAPFILE + "nonExistentLocalFile.jpg?res=x,y" exists, so it asks the application layer
		/// to provide the answer. It may actually be that the application layer determined
		/// that nonExistentLocalFile.jpg actually does exist.
		/// </para>
		/// 
		/// <para>
		/// On the other hand, if the path was AUTOMAPFILE+"c:\temp.mp3" or AUTOMAPFILE+"c:\temp.mp3?transcode=wav"
		/// and temp.mp3 actually existed, then either path representation will automatically be
		/// detected by the framework as being a file that exists.
		/// </para>
		/// </summary>
		public Delegate_AutomapFileExists CheckAutomapFileExists;

		/// <summary>
		/// Returns the relative URI path for a resource that uses the
		/// <see cref="MediaResource.AUTOMAPFILE"/> convention.
		/// Added primarily as a hook so that <see cref="TagExtractor"/>
		/// can properly extract the ContentUri value for an automapped
		/// resource so that <see cref="MediaSorter"/> can compare
		/// the values as they are actually seen from the perspective
		/// of a control point.
		/// </summary>
		public string RelativeContentUri
		{
			get
			{
				return EnsureNonAutoMapProtocol("");
			}
		}

		/// <summary>
		/// Returns a listing of attributes that have been set.
		/// Override is used to check the value of the 
		/// <see cref="DvMediaResource.HasImportUri"/>
		/// override to determine if the importUri attribute is valid.
		/// </summary>
		public override IList ValidAttributes 
		{
			get
			{
				IList attribs = base.ValidAttributes;

				if (this.HasImportUri)
				{
					if (attribs.Contains(T[_RESATTRIB.importUri]) == false)
					{
						attribs.Add(T[_RESATTRIB.importUri]);
					}
				}
				return attribs;
			}
		}

		/// <summary>
		/// Adds extra protection on base class implementation
		/// to prevent AUTOMAPPED FILES from having the importUri attribute
		/// set.
		/// </summary>
		/// <param name="newMetadata"></param>
		public override void UpdateResource (IMediaResource newMetadata)
		{
			if (this.m_ContentUri.StartsWith(MediaResource.AUTOMAPFILE))
			{
				if (newMetadata[T[_RESATTRIB.importUri]] != null)
				{
					throw new ApplicationException("Cannot set the importUri attribute for a resource that is automapped.");
				}
			}

			base.UpdateResource(newMetadata);
		}

		/// <summary>
		/// Tells the owning object that something has changed.
		/// </summary>
		/// <exception cref="InvalidCastException">
		/// thrown if the owner is not a <see cref="IDvMedia"/> object.
		/// </exception>
		private void NotifyOwnerOfChange()
		{
			if (this.Owner != null)
			{
				IDvMedia owner = (IDvMedia) this.Owner;
				owner.NotifyRootOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override object this[_RESATTRIB attrib]
		{
			set
			{
				base[attrib] = value;
				this.NotifyOwnerOfChange();
			}
		}
		
		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override object this[string attrib]
		{
			set
			{
				base[attrib] = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// <para>
		/// This property does nothing different from the inherited version, but
		/// some additional semantics are added in a DvMediaResource.
		/// </para>
		/// 
		/// <para>
		/// The set() method is exactly the same as using the SetContentUri() method.
		/// </para>
		/// 
		/// <para>
		/// <see cref="MediaServerDevice"/>
		/// has an implementation detail designed to optimize the webserving
		/// of ContentDirectory resources mapped local binaries. If public
		/// programmers instantiate a DvMediaResource such that the
		/// ContentUri returns a string in the format of 
		/// "<see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>+[full local path]" 
		/// then
		/// <see cref="MediaServerDevice"/>
		/// will automatically translate the URI into an appropriate
		/// http URL and serve the file when requests are made for the specified URL.
		/// </para>
		/// </summary>
		public override string ContentUri
		{
			set
			{
				this.SetContentUri(value);
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Method takes the current ContentUri value and checks
		/// to see if the uri is an automapped file and also
		/// that the automapped file exists. If both are true
		/// then the result is true.
		/// <para>
		/// This method could potentially be a performance bottleneck
		/// when translating automapped files into relative Uris.
		/// I've opted to use this solution because the alternative 
		/// would be that MediaServerDevice be modified to track
		/// all 
		/// </para>
		/// </summary>
		/// <returns></returns>
		public bool CheckLocalFileExists()
		{
			string full = this.ContentUri;
			bool fileExists = false;

			if (full.StartsWith(MediaResource.AUTOMAPFILE))
			{
				string filename = full.Substring(MediaResource.AUTOMAPFILE.Length);

				if (Directory.Exists(filename))
				{
					fileExists = true;
				}
				else if (File.Exists(filename))
				{
					fileExists = true;
				}
				else if (this.MakeStreamAtHttpGetTime)
				{
					int queryPos = full.IndexOf("?");

					if (queryPos > 0)
					{
						filename = filename.Remove(queryPos, filename.Length-queryPos);

						if (Directory.Exists(filename) == false)
						{
							fileExists = true;
						}
						else if (File.Exists(filename))
						{
							fileExists = true;
						}
					}

					if (fileExists == false)
					{
						if (this.CheckAutomapFileExists != null)
						{
							fileExists = this.CheckAutomapFileExists(this);
						}
					}
				}
			}
			

			this.m_LocalFileExists = fileExists;
			return this.m_LocalFileExists;
		}

		/// <summary>
		/// This property indicates if the contentUri value of
		/// the resource maps to a binary stream generated at
		/// run-time when handling an HTTP-GET request, contrasted
		/// with a binary that's stored on the local file.
		/// </summary>
		public bool MakeStreamAtHttpGetTime
		{
			get
			{
				return this.m_bools[(int) Bits.MakeStreamAtHttpGetTime];
			}
			set
			{
				this.m_bools[(int) Bits.MakeStreamAtHttpGetTime] = value;
				this.CheckLocalFileExists();
			}
		}

		/// <summary>
		/// Since URI's are dynamically mapped for resources that
		/// have a mapping through <see cref="MediaResource.AUTOMAPFILE"/>,
		/// importUri really has no meaning for the programmer.
		/// For resources not mapped in such a way, the base implementation is called.
		/// </summary>
		/// <summary>
		/// The property returns one of the following.
		/// <list type="bullet">
		/// <item>
		/// <term>relative URL path (dynamically generated by system)</term>
		/// <description>
		/// Resources that map to local files that are served through http-get are given
		/// a relative path to the file, from the 
		/// <see cref="MediaServerDevice"/>'s 
		/// relative web path for content.
		/// </description>
		/// </item>
		/// <item>
		/// <term>URI path (specified explicitly)</term>
		/// <description>Resources that were explicitly set with a URI are</description>
		/// </item>
		/// </list>
		/// </summary>
		/// <exception cref="Error_CannotSetImportUri">
		/// Thrown when attempting to set the URI of a resource that maps to
		/// a local file intended for serving with http-get from the internal
		/// webserver of a
		/// <see cref="MediaServerDevice"/>
		/// object.
		/// </exception>
		public override string ImportUri
		{
			get
			{
				if (this.AllowImport)
				{
					if (
						(this.m_ContentUri.StartsWith(AUTOMAPFILE)) ||
						(this.m_ContentUri == "")
						)
					{
						StringBuilder path = new StringBuilder(20);
						path.AppendFormat ("/{0}/{1}/", this.m_ResourceID, this.m_Owner.ID);
						return path.ToString();
					}
					else
					{
						return base.ImportUri;
					}
				}
				else
				{
					return "";
				}
			}
			set
			{
				if (this.m_ContentUri.StartsWith(AUTOMAPFILE))
				{
					throw new ApplicationException("Cannot set the importUri for a resource that is mapped through MediaResource.AUTOMAPFILE.");
				}

				base.ImportUri = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override ProtocolInfoString ProtocolInfo	
		{ 
			set
			{
				base.ProtocolInfo = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _UInt Bitrate 
		{ 
			set
			{
				base.Bitrate = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _UInt BitsPerSample 
		{
			set
			{
				base.BitsPerSample = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _UInt ColorDepth
		{
			set
			{
				base.ColorDepth = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _TimeSpan Duration
		{
			set
			{
				base.Duration = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _UInt nrAudioChannels
		{
			set
			{
				base.nrAudioChannels = value;
				this.NotifyOwnerOfChange();
			}
		}
			
		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override string Protection
		{
			set
			{
				base.Protection = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override ImageDimensions Resolution
		{
			set
			{
				base.Resolution = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _UInt SampleFrequency
		{
			set
			{
				base.SampleFrequency = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Notifies the owner of change after calling
		/// base class implementation.
		/// </summary>
		public override _ULong Size
		{
			set
			{
				base.Size = value;
				this.NotifyOwnerOfChange();
			}
		}

		/// <summary>
		/// Override set: Checks protection access through this.Owner.CheckProtection()
		/// and then calls base class implementation.
		/// </summary>
		public override IUPnPMedia Owner
		{
			set
			{
#if (DEBUG)
				if (this.Owner != null)
				{
					this.Owner.CheckRuntimeBindings(new StackTrace());
				}
#endif
				base.m_Owner = value;
			}
		}

		/// <summary>
		/// Returns the same value as AllowImport.
		/// </summary>
		public override bool HasImportUri
		{
			get
			{
				return this.AllowImport;
			}
		}

		/// <summary>
		/// Set this to true if the ImportUri field should be displayed.
		/// Determines generally if the resource allows control points
		/// to post/import/replace a resource with a new binary.
		/// </summary>
		public bool AllowImport { get { return this._AllowImport; } set { this._AllowImport = value; } }
		/// <summary>
		/// Stores value of <see cref="DvMediaResource.AllowImport"/>.
		/// </summary>
		private bool _AllowImport = false;

		/// <summary>
		/// Set this to true if the ContentUri should not be reported in browse/search
		/// requests. This is necessary when creating items from a control point and the 
		/// related local file does not yet exist.
		/// </summary>
		public bool HideContentUri = false;

		/// <summary>
		/// Allows a public programmer to set the URI of an object.
		/// </summary>
		/// <param name="newUri"></param>
		/// <exception cref="Error_CannotSetContentUri">
		/// Thrown when the newUri string begins with 
		/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>
		/// and the
		/// protocol is not "http-get".
		/// </exception>
		public void SetContentUri(string newUri)
		{
			if (newUri.StartsWith(AUTOMAPFILE))
			{
				if (string.Compare (this.ProtocolInfo.Protocol, "http-get") != 0)
				{
					if (string.Compare (this.ProtocolInfo.Protocol, "*") == 0)
					{
						this.ProtocolInfo = new ProtocolInfoString("http-get:" + this.ProtocolInfo.Network + ":" + this.ProtocolInfo.MimeType + ":" + this.ProtocolInfo.Info);
					}
					else
					{
						throw new Error_CannotSetContentUri(this.ProtocolInfo, newUri);
					}
				}
			}

			this.m_ContentUri = newUri;
			this.CheckLocalFileExists();
		}

		/// <summary>
		/// Allows a programmet to set the "protocolInfo" string.
		/// </summary>
		/// <param name="protocolInfo">
		/// A valid protocolInfo string must have the format
		/// "[protocol]:[network]:[mime type]:[info]".
		/// </param>
		/// <exception cref="Error_CannotSetProtocolInfo">
		/// Thrown if the protocolInfo string is not "http-get", when the contentUri
		/// starts with 
		/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>.
		/// </exception>
		public void SetProtocolInfo(ProtocolInfoString protocolInfo)
		{
			if (this.ContentUri.StartsWith(AUTOMAPFILE))
			{
				if (string.Compare (protocolInfo.Protocol, "http-get") != 0)
				{
					throw new Error_CannotSetProtocolInfo(this.ContentUri, protocolInfo);
				}
			}

			this.m_ProtocolInfo = protocolInfo;
		}

		/// <summary>
		/// Instantiates a DvMediaResource.
		/// </summary>
		/// <param name="contentUri">
		/// The URI of a resource, or a string with the format 
		/// "<see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>+[full local path]" 
		/// and is intended to be served through the http-get
		/// protocol).
		/// </param>
		/// <param name="protocolInfo">
		/// A valid protocolInfo string must have the format
		/// "[protocol]:[network]:[mime type]:[info]".
		/// </param>
		/// <param name="allowImport">
		/// True, if the resource allows control points to replace the resource.
		/// If false, then "importUri" attribute is not exposed in ContentDirectory's Browse/Search
		/// responses.
		/// </param>
		/// <exception cref="Error_CannotSetProtocolInfo">
		/// Thrown if the contentUri value maps to a local file, 
		/// but the protocolInfo string indicates a protocol other than "http-get".
		/// </exception>
		public DvMediaResource(string contentUri, string protocolInfo, bool allowImport)
			: base(contentUri, protocolInfo)
		{
			InitBools();
			this.m_ResourceID = GetResourceID();
			this.AllowImport = allowImport;

			// Set the protocolInfo string again, this time throwing
			// exception if there's an http-get conflict with a locally mapped content uri.
			this.SetProtocolInfo(new ProtocolInfoString(protocolInfo));
		}

		internal DvMediaResource ()
		{
			InitBools();
			this.m_ResourceID = GetResourceID();
		}

		/// <summary>
		/// DvMediaResources can be instantiated from an XmlElement that matches
		/// the ContentDirectory's schema for resource metadata. This method
		/// is for internal use only because of instability from improper use
		/// of the method. If enough demand ensues for its relase as a public
		/// method, I can oblige.
		/// </summary>
		/// <param name="xml">
		/// An XML element conforming to the syntax and semantics of a ContentDirectory resource.
		/// </param>
		public DvMediaResource (XmlElement xml)
			: base(xml)
		{
			InitBools();
			this.AllowImport = false;
			if (this.ContentUri != null)
			{
				if (this.ContentUri == "")
				{
					this.AllowImport = true;
				}
			}
			this.m_ResourceID = GetResourceID();
		}

		/// <summary>
		/// Instructs the "xmlWriter" argument to start the "res" element.
		/// Override properly allows mapping of local paths to relative URIs
		/// using the <see cref="MediaResource.AUTOMAPFILE"/> convention.
		/// </summary>
		/// <param name="formatter">
		/// A <see cref="ToXmlFormatter"/> object that
		/// specifies method implementations for printing
		/// media objects and metadata.
		/// </param>
		/// <param name="data">
		/// This object should be a <see cref="ToXmlData"/>
		/// object that contains additional instructions used
		/// by the "formatter" argument.
		/// </param>
		/// <param name="xmlWriter">
		/// The <see cref="XmlTextWriter"/> object that
		/// will format the representation in an XML
		/// valid way.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown if the "data" argument is not a <see cref="ToXmlData"/> object.
		/// </exception>
		/// <exception cref="ApplicationException">
		/// Thrown if the resource's importUri attribute has been explicitly set for
		/// a resource that uses the <see cref="MediaResource.AUTOMAPFILE"/> convention.
		/// </exception>
		public override void StartElement(ToXmlFormatter formatter, object data, XmlTextWriter xmlWriter)
		{
			ToXmlData _d = (ToXmlData) data;
			this.StartResourceXml(_d.DesiredProperties, xmlWriter);
			this.WriteAttributesXml(data, _d.DesiredProperties, xmlWriter);
			this.WriteImportUriXml(_d.BaseUri, _d.DesiredProperties, xmlWriter);
		}

        /// <summary>
		/// Overrides the base class method so that the importUri field can be dynamically generated
		/// based on a local file path.
		/// </summary>
		/// <param name="baseUriInfo">a string that has the base URL in form: http:[ip address: port]/[virtualdir]</param>
		/// <param name="desiredProperties"></param>
		/// <param name="xmlWriter"></param>
		private void WriteImportUriXml(string baseUriInfo, ArrayList desiredProperties, XmlTextWriter xmlWriter)
		{
			if (desiredProperties != null)
			{
				if (this.HasImportUri)
				{
					string key = T[_ATTRIB.importUri];
					if (this.m_ContentUri.StartsWith(MediaResource.AUTOMAPFILE))
					{
						if (base[key] != null)
						{
							throw new ApplicationException("The ImportUri attribute has been explicitly set for a resource that is automapped, which is illegal.");
						}
					}
					if (desiredProperties.Contains(Tags.PropertyAttributes.res_importUri) || desiredProperties.Count == 0)
					{
						string importUri = baseUriInfo;
						importUri += this.ImportUri;
						xmlWriter.WriteAttributeString(key, importUri);
					}
				}
			}
		}

		/// <summary>
		/// Overrides the base class method so that the contentUri can be dynamically generated
		/// based on a local file path.
		/// </summary>
		/// <param name="baseUriInfo">a string that has the base URL in form: http:[ip address: port]/[virtualdir]</param>
		/// <param name="xmlWriter"></param>
		protected override void WriteContentUriXml(string baseUriInfo, XmlTextWriter xmlWriter)
		{
			xmlWriter.WriteString(this.EnsureNonAutoMapProtocol(baseUriInfo));
		}


		/// <summary>
		/// This method returns a resource URI in string form, such that the
		/// URI is not a URI using the 
		/// <see cref="MediaResource.AUTOMAPFILE"/>
		/// protocol.
		/// </summary>
		/// <param name="baseUri">the base http URL. If null, then the original local path is printed. If empty string, then only the relative portion of URI is printed.</param>
		/// <returns>A URI not using the
		/// <see cref="MediaResource.AUTOMAPFILE"/>
		/// protocol.
		/// </returns>
		internal string EnsureNonAutoMapProtocol(string baseUri)
		{
			IUPnPMedia item = this.m_Owner;
			if (this.ContentUri.StartsWith(AUTOMAPFILE))
			{
				if (baseUri == null)
				{
					return this.ContentUri;
				}

				if (this.m_LocalFileExists)
				{
					StringBuilder uri = new StringBuilder(this.ContentUri.Length);
					int dotpos = this.ContentUri.LastIndexOf('.');

					string ext = "";
					if (this.OverrideFileExtenstion == null)
					{
						if ((dotpos > 0) && (dotpos < this.ContentUri.Length - 1))
						{
							ext = this.ContentUri.Substring(dotpos);
						}

						int querypos = ext.IndexOf('?');
						if ((querypos > 0) && (querypos < ext.Length - 1))
						{
							ext = ext.Substring(0, querypos);
						}
					}
					else
					{
						if (this.OverrideFileExtenstion.StartsWith("."))
						{
							ext = this.OverrideFileExtenstion;
						}
						else
						{
							ext = "." + this.OverrideFileExtenstion;
						}
					}

					uri.Append(baseUri);
					uri.AppendFormat("/{0}/{1}/{2} - {3}{4}", this.m_ResourceID, item.ID, item.Creator, item.Title, ext);
					string uriString = HTTPMessage.EscapeString(uri.ToString());

					return uriString;
				}
				else
				{
					return "";
				}
			}
			else
			{
				return this.ContentUri;
			}
		}

		/// <summary>
		/// Public programmers can use this method to acquire a unique name for
		/// a binary associated with a resource created by a control-point.
		/// The method attempts to provide a name using the resource owner's
		/// creator, title, and a few additional numbers. Applications
		/// are not required to use this method to generate a name.
		/// </summary>
		/// <param name="baseDirectory">the base directory with where the </param>
		/// <returns></returns>
		public string GenerateLocalFilePath(string baseDirectory)
		{
			StringBuilder filePath = new StringBuilder(256);
			string base_name = "";

			try
			{
				base_name = this.Owner.Creator.Trim() +" -- "+ this.Owner.Title.Trim();

				base_name = base_name.Replace("/", "_");
				base_name = base_name.Replace("\\", "_");
				base_name = base_name.Replace(":", "-");
				base_name = base_name.Replace("*", "_");
				base_name = base_name.Replace("?", "_");
				base_name = base_name.Replace(">", "_");
				base_name = base_name.Replace("<", "_");
				base_name = base_name.Replace("|", "_");
			}
			catch
			{
			}

			filePath.AppendFormat("{0}{1}{2} -- {3}", AUTOMAPFILE, baseDirectory, base_name, this.ResourceID);
			if ((this.ProtocolInfo.MimeType != "*") && (this.ProtocolInfo.MimeType != ""))
			{
				filePath.AppendFormat("{0}", MimeTypes.MimeToExtension(this.ProtocolInfo.MimeType));
			}

			/// Ensure the file is unique.
			/// 
			if (File.Exists(filePath.ToString()))
			{
				int x = 1;
				while (File.Exists(filePath.ToString() + x.ToString()))
				{
					x++;
				}

				filePath.AppendFormat("_{0}", x);
			}

			return filePath.ToString();
		}

		/// <summary>
		/// <see cref="MediaServerDevice"/>
		/// uses a ResourceID value to enforce uniqueness of resources served
		/// through it's HTTP server. This method ensures that unique
		/// ID's are given to each resource.
		/// </summary>
		/// <returns></returns>
		private static string GetResourceID()
		{
			lock (TheNextID)
			{
				int n = (int)TheNextID;
				n++;
				TheNextID = n;
			}
			return TheNextID.ToString();
		}

		/// <summary>
		/// Provides the resource ID, which uniquely identifies a resource in a device-implementation
		/// of a CDS. This field is not normative to UPnP-AV, and is used specifically for 
		/// this implementation of CDS on device-side implementations.
		/// <para>
		/// To ensure that resources are unique, we use
		/// a ResourceID value. Public programmers
		/// should not depend on this value, as it is
		/// a system value.
		/// </para>
		/// </summary>
		public string ResourceID { get { return this.m_ResourceID; } }
		/// <summary>
		/// The resourceID... available for internal use only.
		/// Either has the format of "[long]" or "[long]_[long]".
		/// </summary>
		internal string m_ResourceID;

		/// <summary>
		/// If true, then an automapped file has been
		/// determined to have been an existing file
		/// (at least at one point).
		/// </summary>
		private bool m_LocalFileExists
		{
			get
			{
				return this.m_bools[(int) Bits.LocalFileExists];
			}
			set
			{
				this.m_bools[(int) Bits.LocalFileExists] = value;
			}
		}

		/// <summary>
		/// Enumeration indexer into m_bools.
		/// </summary>
		private enum Bits
		{
			LocalFileExists = 0,
			MakeStreamAtHttpGetTime
		}

		/// <summary>
		/// Initializes values in m_bools.
		/// </summary>
		/// <returns></returns>
		protected virtual void InitBools()
		{
			this.m_bools[(int) Bits.LocalFileExists] = false;
			this.m_bools[(int) Bits.MakeStreamAtHttpGetTime] = false;
		}

		/// <summary>
		/// Use a bit array instead of individual booleans to save space.
		/// The BitArray seems to have some weird bugs. Switching
		/// back to bools.
		/// </summary>
		//private BitArray m_bools = new BitArray(2);
		private bool[] m_bools = new bool[2];

		private static object TheNextID = 0;

		/// <summary>
		/// Set this field if the contentUri contains an AUTOMAPFILE location,
		/// and the file extension associated with the file isn't the one
		/// you want to advertise. This is used primarily for resources that
		/// are transcoded. 
		/// 
		/// <para>
		/// Developers are encouraged to use this, as it has positive performance benefits.
		/// If left equal to null, the library's translation from a an automapped file path
		/// to a relative uri will involve parsing out the first file extension before a
		/// query (?) symbol. 
		/// </para>
		/// </summary>
		public string OverrideFileExtenstion = null;
	}
}
