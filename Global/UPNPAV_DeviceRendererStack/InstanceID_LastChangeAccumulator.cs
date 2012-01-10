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
using System.Collections;

namespace OpenSource.UPnP.AV.RENDERER.Device
{
	/// <summary>
	/// Summary description for AVTransportLastChangeAccumulator.
	/// </summary>
	public class InstanceID_LastChangeAccumulator : OpenSource.UPnP.UPnPModeratedStateVariable.IAccumulator
	{
		private string NS = "";
		public InstanceID_LastChangeAccumulator(string ns)
		{
			NS = ns;
		}
		public object Reset()
		{
			return("");
		}
		public object Merge(object current, object newobject)
		{
			if(current==null) return(newobject);
			string CurrentEvent = UPnPStringFormatter.UnEscapeString((string)current);
			if(CurrentEvent=="") return(newobject);
			string NewEvent = UPnPStringFormatter.UnEscapeString((string)newobject);
			Hashtable T = new Hashtable();

			StringReader MyString = new StringReader(CurrentEvent);
			XmlTextReader XMLDoc = new XmlTextReader(MyString);
			int InstanceID = -1;
			string VarName = "";
			string VarValue = "";
			string AttrName = "";
			string AttrValue = "";

			XMLDoc.Read();
			XMLDoc.MoveToContent();

			if(XMLDoc.LocalName!="Event") return(newobject);

			XMLDoc.Read();
			XMLDoc.MoveToContent();

			while((XMLDoc.LocalName!="Event")&&(XMLDoc.EOF==false))
			{
				// At Start, should be InstanceID
				if(XMLDoc.LocalName=="InstanceID")
				{
					XMLDoc.MoveToAttribute("val");
					InstanceID = int.Parse(XMLDoc.GetAttribute("val"));
					if(T.ContainsKey(InstanceID)==false) T[InstanceID] = new Hashtable();
					XMLDoc.MoveToContent();

					XMLDoc.Read();
					XMLDoc.MoveToContent();

					while(XMLDoc.LocalName!="InstanceID")
					{
						VarName = XMLDoc.LocalName;
						for(int a_idx=0;a_idx<XMLDoc.AttributeCount;++a_idx)
						{
							XMLDoc.MoveToAttribute(a_idx);
							if(XMLDoc.LocalName=="val")
							{
								//VarValue = XMLDoc.GetAttribute(a_idx);
								VarValue = UPnPStringFormatter.PartialEscapeString(XMLDoc.ReadInnerXml());
							}
							else
							{
								AttrName = XMLDoc.LocalName;
								//AttrValue = XMLDoc.GetAttribute(a_idx);
								AttrValue = UPnPStringFormatter.PartialEscapeString(XMLDoc.ReadInnerXml());
							}

						}

						XMLDoc.MoveToContent();

						if(AttrName=="")
						{
							((Hashtable)T[InstanceID])[VarName] = VarValue;
						}
						else
						{
							if(((Hashtable)T[InstanceID]).ContainsKey(VarName)==false)
							{
								((Hashtable)T[InstanceID])[VarName] = new Hashtable();
							}
							if(((Hashtable)((Hashtable)T[InstanceID])[VarName]).ContainsKey(AttrName)==false)
							{
								((Hashtable)((Hashtable)T[InstanceID])[VarName])[AttrName] = new Hashtable();
							}
							((Hashtable)((Hashtable)((Hashtable)T[InstanceID])[VarName])[AttrName])[AttrValue] = VarValue;
						}
						XMLDoc.Read();
						XMLDoc.MoveToContent();
					}
				}
				else
				{
					XMLDoc.Skip();
				}
				XMLDoc.Read();
				XMLDoc.MoveToContent();
			}

			XMLDoc.Close();
			MyString = new StringReader(NewEvent);
			XMLDoc = new XmlTextReader(MyString);

			// Read New Events
			XMLDoc.Read();
			XMLDoc.MoveToContent();

			XMLDoc.Read();
			XMLDoc.MoveToContent();

			while((XMLDoc.LocalName!="Event")&&(XMLDoc.EOF==false))
			{
				// At Start, should be InstanceID
				if(XMLDoc.LocalName=="InstanceID")
				{
					XMLDoc.MoveToAttribute("val");
					InstanceID = int.Parse(XMLDoc.GetAttribute("val"));
					if(T.ContainsKey(InstanceID)==false) T[InstanceID] = new Hashtable();
					XMLDoc.MoveToContent();

					XMLDoc.Read();
					XMLDoc.MoveToContent();

					while(XMLDoc.LocalName!="InstanceID")
					{
						VarName = XMLDoc.LocalName;
						VarValue = "";
						AttrName = "";
						AttrValue = "";
						for(int a_idx=0;a_idx<XMLDoc.AttributeCount;++a_idx)
						{
							XMLDoc.MoveToAttribute(a_idx);
							if(XMLDoc.LocalName=="val")
							{
								//VarValue = XMLDoc.GetAttribute(a_idx);
								VarValue = UPnPStringFormatter.PartialEscapeString(XMLDoc.ReadInnerXml());
							}
							else
							{
								AttrName = XMLDoc.LocalName;
								//AttrValue = XMLDoc.GetAttribute(a_idx);
								AttrValue = UPnPStringFormatter.PartialEscapeString(XMLDoc.ReadInnerXml());
							}
						}

						XMLDoc.MoveToContent();

						if(AttrName=="")
						{
							((Hashtable)T[InstanceID])[VarName] = VarValue;
						}
						else
						{
							if(((Hashtable)T[InstanceID]).ContainsKey(VarName)==false)
							{
								((Hashtable)T[InstanceID])[VarName] = new Hashtable();
							}
							if(((Hashtable)((Hashtable)T[InstanceID])[VarName]).ContainsKey(AttrName)==false)
							{
								((Hashtable)((Hashtable)T[InstanceID])[VarName])[AttrName] = new Hashtable();
							}
							((Hashtable)((Hashtable)((Hashtable)T[InstanceID])[VarName])[AttrName])[AttrValue] = VarValue;
						}
						XMLDoc.Read();
						XMLDoc.MoveToContent();
					}
				}
				else
				{
					XMLDoc.Skip();
				}
				XMLDoc.Read();
				XMLDoc.MoveToContent();
			}

			XMLDoc.Close();

			// Rebuild Events

			IDictionaryEnumerator en = T.GetEnumerator();
			StringBuilder Ev = new StringBuilder();
			IDictionaryEnumerator en2;
			Ev.Append("<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/"+NS+"/\">\r\n");
			while(en.MoveNext())
			{
				VarValue = "";
				VarName = "";
				AttrName = "";
				AttrValue = "";

				Ev.Append("   <InstanceID val=\"" + en.Key.ToString() + "\">\r\n");
				en2 = ((Hashtable)en.Value).GetEnumerator();
				while(en2.MoveNext())
				{
					if(en2.Value.GetType().FullName=="System.String")
					{
						Ev.Append("        <" + en2.Key.ToString() + " val=\"" + en2.Value.ToString() + "\"/>\r\n");
					}
					else
					{
						IDictionaryEnumerator en3 = ((Hashtable)en2.Value).GetEnumerator();
						while(en3.MoveNext())
						{
							AttrName = en3.Key.ToString();
							AttrValue = "";
							IDictionaryEnumerator en4 = ((Hashtable)en3.Value).GetEnumerator();
							while(en4.MoveNext())
							{
								AttrValue = en4.Key.ToString();
								VarValue = en4.Value.ToString();
								Ev.Append("        <" + en2.Key.ToString() + " val=\"" + VarValue + "\" " + AttrName + "=\"" + AttrValue + "\"/>\r\n");
							}
						}
					}
				}
				Ev.Append("   </InstanceID>\r\n");
			}
			Ev.Append("</Event>");
			return(UPnPStringFormatter.EscapeString(Ev.ToString()));
		}
	}
}
