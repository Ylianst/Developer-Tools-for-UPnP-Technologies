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
using OpenSource.UPnP.AV.CdsMetadata;
using System.Text;
using System.Collections;
using System.Xml;
using System.IO;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// <para>
	/// This class is an ultra-lightweight representation of a media item that
	/// points to another media item.
	/// </para>
	/// 
	/// <para>
	/// This class must never have another item point to it.
	/// </para>
	/// 
	/// <para>
	/// This class must NEVER EVER EVER EVER be used if content management is enabled.
	/// </para>
	/// </summary>
	[Serializable()]
	public sealed class DvMediaReference : IDvMedia, IDvItem
	{
		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void AddDescNode(string ignored){}
		
		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void AddDescNode(string[] ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void RemoveDescNode(string ignored){}
		
		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void AddResource(IMediaResource ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void AddResources(ICollection ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void AddResources(IMediaResource[] ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void RemoveResource(IMediaResource ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void RemoveResources(ICollection ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void SetMetadata(MediaBuilder.CoreMetadata ignored){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		/// <param name="ignored2"></param>
		public void SetPropertyValue(string ignored, IList ignored2){}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void CheckRuntimeBindings(System.Diagnostics.StackTrace ignored) {}

		/// <summary>
		/// Returns underlying media item's class.
		/// 
		/// <para>
		/// Set operator not implemented.
		/// </para>
		/// </summary>
		public MediaClass Class
		{
			get
			{
				return this.m_Underlying.Class;
			}
			set
			{
				//not implemented.
			}
		}

		/// <summary>
		/// Returns underlying media item's creator.
		/// 
		/// <para>
		/// Set operator not implemented.
		/// </para>
		/// </summary>
		public string Creator
		{
			get
			{
				return this.m_Underlying.Creator;
			}
			set
			{
			}
		}

		/// <summary>
		/// Returns null;
		/// </summary>
		public IList DescNodes
		{
			get
			{
				return null;
			}
		}

		/// <summary>
		/// Returns underlying list of desc nodes.
		/// </summary>
		public IList MergedDescNodes
		{
			get
			{
				return this.m_Underlying.DescNodes;
			}
		}

		/// <summary>
		/// Returns null.
		/// </summary>
		public IMediaProperties Properties
		{
			get
			{
				return null;
			}
		}

		/// <summary>
		/// Returns underlying set of media properties.
		/// </summary>
		public IMediaProperties MergedProperties
		{
			get
			{
				return this.m_Underlying.Properties;
			}
		}

		/// <summary>
		/// Returns null.
		/// </summary>
		public IMediaResource[] Resources
		{
			get
			{
				return null;
			}
		}

		/// <summary>
		/// Returns underyling list of resources.
		/// </summary>
		public IMediaResource[] MergedResources
		{
			get
			{
				return this.m_Underlying.Resources;
			}
		}

		/// <summary>
		/// Returns object ID.
		/// 
		/// <para>
		/// Set operation not implemented.
		/// </para>
		/// </summary>
		public string ID
		{
			get
			{
				return m_uniqueId;
			}
			set
			{
			}
		}

		/// <summary>
		/// Returns false.
		/// </summary>
		public bool IsContainer { get { return false; } }

		/// <summary>
		/// Returns true.
		/// </summary>
		public bool IsItem { get { return true;  } }

		/// <summary>
		/// Returns true.
		/// </summary>
		public bool IsRestricted { get { return true;  } set {} }

		/// <summary>
		/// Returns false.
		/// </summary>
		public bool IsSearchable { get { return false; } set {}  }

		/// <summary>
		/// Returns true.
		/// </summary>
		public bool IsReference { get { return true; } }

		/// <summary>
		/// Returns parent container.
		/// 
		/// <para>
		/// Set IS implemented. Be careful when using it.
		/// </para>
		/// </summary>
		public IMediaContainer Parent { get { return this.m_Parent; } set{ this.m_Parent = value; } }

		/// <summary>
		/// Returns parent container ID.
		/// 
		/// <para>
		/// Set not implemented.
		/// </para>
		/// </summary>
		public string ParentID { get { return this.m_Parent.ID; } set{} }

		/// <summary>
		/// Returns null.
		/// <para>Set not implemented.</para>
		/// </summary>
		public object Tag { get { return null; } set{} }

		/// <summary>
		/// Returns underlying title.
		/// <para>Set not implemented.</para>
		/// </summary>
		public string Title { get { return this.m_Underlying.Title; } set{}}

		/// <summary>
		/// Returns string representation of item.
		/// </summary>
		/// <returns></returns>
		public string ToDidl()
		{
			ArrayList properties = new ArrayList();

			StringBuilder sb = null;
			StringWriter sw = null;
			MemoryStream ms = null;
			XmlTextWriter xmlWriter = null;
			
			if (MediaObject.ENCODE_UTF8)
			{
				ms = new MemoryStream(MediaObject.XML_BUFFER_SIZE);
				xmlWriter = new XmlTextWriter(ms, System.Text.Encoding.UTF8);
			}
			else
			{
				sb = new StringBuilder(MediaObject.XML_BUFFER_SIZE);
				sw = new StringWriter(sb);
				xmlWriter = new XmlTextWriter(sw);
			}
			
			xmlWriter.Formatting = System.Xml.Formatting.Indented;
			xmlWriter.Namespaces = true;
			xmlWriter.WriteStartDocument();
			
			MediaObject.WriteResponseHeader(xmlWriter);

			ToXmlData _d = (ToXmlData) MediaObject.ToXmlData_AllRecurse.Clone();
			_d.IsRecursive = false;
			this.ToXml(ToXmlFormatter.DefaultFormatter, _d, xmlWriter);

			xmlWriter.WriteEndDocument();
			xmlWriter.Flush();
			
			string xmlResult;
			if (MediaObject.ENCODE_UTF8)
			{
				int len = (int) ms.ToArray().Length - MediaObject.TruncateLength_UTF8;
				UTF8Encoding utf8e = new UTF8Encoding(false, true);
				xmlResult = utf8e.GetString(ms.ToArray(), MediaObject.TruncateLength_UTF8, len);
			}
			else
			{
				xmlResult = sb.ToString();
			}
			xmlWriter.Close();

			int crpos = xmlResult.IndexOf("\r\n");
			crpos = xmlResult.IndexOf('<', crpos);
			string trunc = xmlResult.Remove(0, crpos);
			return trunc;
		}

		/// <summary>
		/// Always prints the full representation as required by the CDS specification.
		/// No custom formatting options are honored, although desired properties
		/// are supported in the data param.
		/// </summary>
		/// <param name="formatter">only options related to CDS-normative formatting are honored</param>
		/// <param name="data">Must be <see cref="ToXmlData"/> instance.</param>
		/// <param name="xmlWriter"></param>
		public void ToXml(ToXmlFormatter formatter, object data, XmlTextWriter xmlWriter)
		{
			xmlWriter.WriteStartElement(T[_DIDL.Item]);
			xmlWriter.WriteAttributeString(T[_ATTRIB.id], this.ID);
			xmlWriter.WriteAttributeString(T[_ATTRIB.refID], this.m_Underlying.ID);
			xmlWriter.WriteAttributeString(T[_ATTRIB.parentID], this.m_Parent.ID);
			xmlWriter.WriteAttributeString(T[_ATTRIB.restricted], "1");

			InnerXmlWriter.WriteInnerXml
				(
				this, 
				new InnerXmlWriter.DelegateWriteProperties(InnerXmlWriter.WriteInnerXmlProperties),
				new InnerXmlWriter.DelegateShouldPrintResources(MediaObject.ShouldPrintResources),
				new InnerXmlWriter.DelegateWriteResources(InnerXmlWriter.WriteInnerXmlResources),
				new InnerXmlWriter.DelegateWriteDescNodes(InnerXmlWriter.WriteInnerXmlDescNodes),
				formatter,
				(ToXmlData) data,
				xmlWriter
				);

			xmlWriter.WriteEndElement();
		}

		/// <summary>
		/// Returns true if the desiredProperties array
		/// indicates the need to print resource elements.
		/// </summary>
		/// <param name="desiredProperties"></param>
		/// <returns></returns>
		private bool PrintResources(ArrayList desiredProperties)
		{
			if (desiredProperties != null)
			{
				if (desiredProperties.Count == 0)
				{
					return true;
				}
				else
				{
					foreach (string key in desiredProperties)
					{
						string lowerKey = key.ToLower();
						if (lowerKey.StartsWith(T[_DIDL.Res]))
						{
							return true;
						}
						else
						{
							// check for @[attribname] notation.
							foreach (string attrib in MediaResource.GetPossibleAttributes())
							{
								string lowerAttrib = "@" + attrib.ToLower();

								if (lowerAttrib == lowerKey)
								{
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <returns>null</returns>
		public IUPnPMedia MetadataCopy(){return null;}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void UpdateObject(IUPnPMedia ignored) {}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void UpdateObject(string ignored) {}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void UpdateObject(XmlElement ignored) {}

		/// <summary>
		/// Returns EnumWriteStatus.NOT_WRITABLE.
		/// <para>Set not implemented</para>
		/// </summary>
		public EnumWriteStatus WriteStatus { get { return EnumWriteStatus.NOT_WRITABLE; } set{}}

		/// <summary>
		/// Not implemented.
		/// </summary>
		/// <param name="ignored"></param>
		public void ChangeParent(IDvContainer ignored){}

		public DvMediaReference(DvMediaItem underlyingItem)
		{
			m_uniqueId = MediaBuilder.GetUniqueId();
			this.m_Underlying = underlyingItem;
		}

		/// <summary>
		/// Returns underlying item's ID.
		/// <para>Set operation not implemented.</para>
		/// </summary>
		public string RefID { get { return this.m_Underlying.ID; } set{} }

		/// <summary>
		/// Returns underyling item.
		/// <para>Set operation not implemented.</para>
		/// </summary>
		public IMediaItem RefItem { get { return this.m_Underlying; } set{} }

		/// <summary>
		/// Throws an application exception. This method should never be called.
		/// </summary>
		/// <returns></returns>
		public IDvItem CreateReference() { throw new ApplicationException("Calling this method is not legal."); }

		/// <summary>
		/// Always returns false because this item should never have any other items pointing to it.
		/// </summary>
		/// <returns></returns>
		public bool IsDeletePending { get { return false; }}

		/// <summary>
		/// Does nothing. Object has no reference list because no items can point to it.
		/// </summary>
		public void LockReferenceList() { }

		/// <summary>
		/// Does nothing. Object has no reference list because no items can point to it.
		/// </summary>
		public void UnlockReferenceList() { }

		/// <summary>
		/// Does nothing. Object has no reference list because no items can point to it.
		/// </summary>
		public void NotifyPendingDelete() { }

		/// <summary>
		/// Always returns null. No reference list.
		/// </summary>
		public IList ReferenceItems { get { return null; } }

		/// <summary>
		/// Not implemented because nothing can change.
		/// </summary>
		public void NotifyRootOfChange(){}

		private string m_uniqueId = null;
		private DvMediaItem m_Underlying = null;

		//private static int RefItemNumber = 0;
		private IMediaContainer m_Parent = null;

		/// <summary>
		/// Used for obtaining the attribute and tag names of standard metadata properties.
		/// </summary>
		private static Tags T = Tags.GetInstance();

	}
}
