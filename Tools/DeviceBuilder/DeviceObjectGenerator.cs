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
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for DeviceObjectGenerator.
	/// </summary>
	public class DeviceObjectGenerator
	{
		public static void InjectBytes(out string byteString, byte[] inVal, string newLine, bool withTypeCast)
		{
			CodeProcessor cs = new CodeProcessor(new StringBuilder(),false);
			cs.NewLine = newLine;
			
			cs.Append("{"+cs.NewLine);

			bool _first = true;
			int _ctr=0;
			foreach(byte b in inVal)
			{
				if (_first==false)
				{
					cs.Append(",");
				}
				else
				{
					_first = false;
				}
				string hx = b.ToString("X");
				if (withTypeCast)
				{
					cs.Append("(char)");
				}
				cs.Append("0x");
				
				if (hx.Length==1){cs.Append("0");}
				cs.Append(hx);
							
				++_ctr;
				if (_ctr%(withTypeCast?10:20)==0)
				{
					cs.Append(cs.NewLine);
				}
			}		
			cs.Append(cs.NewLine+"}");
			
			byteString = cs.ToString();
		}
		public static void InjectCompressedString(out string CompressedString, out int CompressedStringLength, string InVal, string newLine)
		{
			UTF8Encoding U = new UTF8Encoding();
			byte[] stringX = OpenSource.Utilities.StringCompressor.CompressString(InVal);
			
			CompressedStringLength = stringX.Length;
			InjectBytes(out CompressedString,stringX,newLine,true);
		}

		protected static void LabelDevices(Hashtable t, UPnPDevice d)
		{
			string deviceName = d.DeviceURN_Prefix;
			string useName;
			int counter = 2;

			deviceName = deviceName.Substring(0,deviceName.Length-1);
			deviceName = deviceName.Substring(1+deviceName.LastIndexOf(":"));
			useName = deviceName;

			while(t.ContainsKey(useName))
			{
				useName = deviceName + counter.ToString();
				++counter;
			}

			t[useName] = d;
			d.User2 = useName;

			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				LabelDevices(t,ed);
			}
		}
		protected static void GenerateStateVariableLookupTable_Service(UPnPService s, Hashtable t)
		{
			Hashtable lookupTable = new Hashtable();
			t[s] = lookupTable;

			int i=0;

			foreach(UPnPStateVariable v in s.GetStateVariables())
			{
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);
					
				UPnPDebugObject dobj = new UPnPDebugObject(v);
				dobj.InvokeNonStaticMethod("GetStateVariableXML",new object[1]{X});

				lookupTable[v] = new object[3]{i,sb.Length,sb.ToString()};
				i+=sb.Length;
			}
		}
		public static void GenerateStateVariableLookupTable(UPnPDevice d, Hashtable t)
		{			
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				GenerateStateVariableLookupTable(ed,t);
			}
			foreach(UPnPService s in d.Services)
			{
				GenerateStateVariableLookupTable_Service(s,t);
			}
		}

		protected static void GenerateActionLookupTable_Service(UPnPService s, Hashtable t)
		{
			Hashtable lookupTable = new Hashtable();
			t[s] = lookupTable;

			int i=0;

			foreach(UPnPAction a in s.Actions)
			{
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);
					
				UPnPDebugObject dobj = new UPnPDebugObject(a);
				dobj.InvokeNonStaticMethod("GetXML",new object[1]{X});

				lookupTable[a] = new object[2]{i,sb.Length};
				i+=sb.Length;
			}
		}
		public static void GenerateActionLookupTable(UPnPDevice d, Hashtable t)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				GenerateActionLookupTable(ed,t);
			}
			foreach(UPnPService s in d.Services)
			{
				GenerateActionLookupTable_Service(s,t);
			}
		}
		protected static void PopulateStateVariableStructs(CodeProcessor cs, UPnPDevice d, Hashtable VarTable)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				PopulateStateVariableStructs(cs,ed,VarTable);
			}
			foreach(UPnPService s in d.Services)
			{
				cs.Append("struct UPnP_StateVariableTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+" UPnP_StateVariableTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_Impl = "+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);

				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					UPnPDebugObject dobj = new UPnPDebugObject(v);
					dobj.InvokeNonStaticMethod("GetStateVariableXML",new object[1]{X});
				}
				X.Flush();
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);
				cs.Append("	"+stringX+","+cs.NewLine);
				cs.Append("	"+stringXLen.ToString()+","+cs.NewLine);
				cs.Append("	"+sb.Length.ToString()+cs.NewLine);
				cs.Append("};"+cs.NewLine);
				
				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					Hashtable t = (Hashtable)VarTable[s];
					int startingIndex = (int)((object[])t[v])[0];
					string varString = (string)((object[])t[v])[2];

					cs.Append("struct UPnP_StateVariable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+v.Name+" UPnP_StateVariable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+v.Name+"_Impl = "+cs.NewLine);
					cs.Append("{"+cs.NewLine);
					cs.Append("	"+startingIndex.ToString()+","+cs.NewLine); // Start Index
					cs.Append("	"+(varString.IndexOf("</dataType>")+11).ToString()+","+cs.NewLine);

					if (v.AllowedStringValues!=null)
					{
						int avlen=0;
						foreach(string avs in v.AllowedStringValues)
						{
							avlen += avs.Length;
						}
						cs.Append("	"+(varString.IndexOf("<allowedValueList>")+startingIndex).ToString()+","+cs.NewLine); // Start of Allowed Value List
						cs.Append("	18,"+cs.NewLine); // Length of allowedValueList
						cs.Append("	"+(varString.IndexOf("</allowedValueList>")+startingIndex).ToString()+","+cs.NewLine); // Start of endTag
						cs.Append("	19,"+cs.NewLine); // Length of end tag
						cs.Append("	{");
						foreach(string av in v.AllowedStringValues)
						{
							cs.Append("\""+av+"\",");
						}
						cs.Append("NULL");
						cs.Append("}");
					}
					if (v.Minimum!=null || v.Maximum!=null)
					{
						if (v.AllowedStringValues!=null)
						{
							cs.Append(","+cs.NewLine);
						}

						cs.Append("	"+(startingIndex+varString.IndexOf("<allowedValueRange>")).ToString()+","+cs.NewLine);
						cs.Append("	19,"+cs.NewLine);
						cs.Append("	"+(startingIndex+varString.IndexOf("</allowedValueRange>")).ToString()+","+cs.NewLine);
						cs.Append("	20,"+cs.NewLine);
						cs.Append("	{");
						if (v.Minimum!=null)
						{
							cs.Append("\""+v.Minimum.ToString()+"\",");
						}
						else
						{
							cs.Append("NULL,");
						}
						if (v.Maximum!=null)
						{
							cs.Append("\""+v.Maximum.ToString()+"\",");
						}
						else
						{
							cs.Append("NULL,");
						}
						if (v.Step!=null)
						{
							cs.Append("\""+v.Step.ToString()+"\"");
						}
						else
						{
							cs.Append("NULL");
						}
						cs.Append("}");
					}
					if (v.DefaultValue!=null)
					{
						if (v.AllowedStringValues!=null || v.Maximum!=null || v.Maximum!=null)
						{
							cs.Append(","+cs.NewLine);
						}
						cs.Append("	"+(startingIndex+varString.IndexOf("<defaultValue>")).ToString()+","+cs.NewLine);
						cs.Append("	14,"+cs.NewLine);
						cs.Append("	"+(startingIndex+varString.IndexOf("</defaultValue>")).ToString()+","+cs.NewLine);
						cs.Append("	15,"+cs.NewLine);
						cs.Append("\""+UPnPService.SerializeObjectInstance(v.DefaultValue)+"\"");
					}
					if (v.DefaultValue!=null || v.AllowedStringValues!=null || v.Maximum!=null || v.Maximum!=null)
					{
						cs.Append(","+cs.NewLine);
					}
					cs.Append((varString.IndexOf("</stateVariable>")+startingIndex).ToString()+","+cs.NewLine);
					cs.Append("	16"+cs.NewLine);
	
					cs.Append("};"+cs.NewLine);
				}
			}
		}
		protected static void BuildStateVariableStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				BuildStateVariableStructs(cs,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);

				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					UPnPDebugObject dobj = new UPnPDebugObject(v);
					dobj.InvokeNonStaticMethod("GetStateVariableXML",new object[1]{X});
				}
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);

				cs.Append("struct UPnP_StateVariableTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				cs.Append("	char Reserved["+stringXLen.ToString()+"];"+cs.NewLine);
				cs.Append("	int ReservedXL;"+cs.NewLine);
				cs.Append("	int ReservedUXL;"+cs.NewLine);
				cs.Append("};"+cs.NewLine);

				foreach(UPnPStateVariable v in s.GetStateVariables())
				{					
					cs.Append("struct UPnP_StateVariable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+v.Name+cs.NewLine);
					cs.Append("{"+cs.NewLine);
					cs.Append("	int Reserved1;"+cs.NewLine);
					cs.Append("	int Reserved1L;"+cs.NewLine);
					if (v.AllowedStringValues!=null)
					{
						cs.Append("	int Reserved2;"+cs.NewLine);
						cs.Append("	int Reserved2L;"+cs.NewLine);
						cs.Append("	int Reserved3;"+cs.NewLine);
						cs.Append("	int Reserved3L;"+cs.NewLine);
						cs.Append("	char *AllowedValues[UPnP_StateVariable_AllowedValues_MAX];"+cs.NewLine);
					}
					if (v.Minimum!=null || v.Maximum!=null)
					{
						cs.Append("	int Reserved4;"+cs.NewLine);
						cs.Append("	int Reserved4L;"+cs.NewLine);
						cs.Append("	int Reserved5;"+cs.NewLine);
						cs.Append("	int Reserved5L;"+cs.NewLine);
						cs.Append("	char *MinMaxStep[3];"+cs.NewLine);
					}
					if (v.DefaultValue!=null)
					{
						cs.Append("	int Reserved6;"+cs.NewLine);
						cs.Append("	int Reserved6L;"+cs.NewLine);
						cs.Append("	int Reserved7;"+cs.NewLine);
						cs.Append("	int Reserved7L;"+cs.NewLine);
						cs.Append("	char *DefaultValue;"+cs.NewLine);
					}
					cs.Append("	int Reserved8;"+cs.NewLine);
					cs.Append("	int Reserved8L;"+cs.NewLine);
					cs.Append("};"+cs.NewLine);
				}
			}
		}
		protected static void PopulateActionStructs(CodeProcessor cs, UPnPDevice d, Hashtable VarTable)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				PopulateActionStructs(cs,ed,VarTable);
			}
			foreach(UPnPService s in d.Services)
			{
				cs.Append("struct UPnP_ActionTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+" UPnP_ActionTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_Impl = "+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);

				foreach(UPnPAction a in s.Actions)
				{
					UPnPDebugObject dobj = new UPnPDebugObject(a);
					dobj.InvokeNonStaticMethod("GetXML",new object[1]{X});
				}
				X.Flush();
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);
				cs.Append("	"+stringX+","+cs.NewLine);
				cs.Append("	"+stringXLen.ToString()+","+cs.NewLine);
				cs.Append("	"+sb.Length.ToString()+cs.NewLine);
				cs.Append("};"+cs.NewLine);

				foreach(UPnPAction a in s.Actions)
				{
					Hashtable t = (Hashtable)VarTable[s];

					cs.Append("struct UPnP_Action_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+" UPnP_Action_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+"_Impl = "+cs.NewLine);
					cs.Append("{"+cs.NewLine);
					cs.Append("	"+((object[])t[a])[0].ToString()+","+cs.NewLine);
					cs.Append("	"+((object[])t[a])[1].ToString()+cs.NewLine);
					cs.Append("};"+cs.NewLine);
				}
			}
		}
		protected static void BuildActionStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				BuildActionStructs(cs,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);

				foreach(UPnPAction a in s.Actions)
				{
					UPnPDebugObject dobj = new UPnPDebugObject(a);
					dobj.InvokeNonStaticMethod("GetXML",new object[1]{X});
				}
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);

				cs.Append("struct UPnP_ActionTable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				cs.Append("	char Reserved["+stringXLen.ToString()+"];"+cs.NewLine);
				cs.Append("	int ReservedXL;"+cs.NewLine);
				cs.Append("	int ReservedUXL;"+cs.NewLine);
				cs.Append("};"+cs.NewLine);

				foreach(UPnPAction a in s.Actions)
				{
					cs.Append("struct UPnP_Action_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+cs.NewLine);
					cs.Append("{"+cs.NewLine);
					cs.Append("	int Reserved;"+cs.NewLine);
					cs.Append("	int Reserved2;"+cs.NewLine);
					cs.Append("};"+cs.NewLine);
				}
			}
		}
		protected static void PopulateServiceStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				PopulateServiceStructs(cs,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				cs.Append("struct UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+" UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_Impl ="+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				foreach(UPnPAction a in s.Actions)
				{
					cs.Append("	&UPnP_Action_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+"_Impl,"+cs.NewLine);
				}
				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					cs.Append("	&UPnP_StateVariable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+v.Name+"_Impl,"+cs.NewLine);
				}

				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);
				s.GetServiceXML(X);
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);

				cs.Append("	"+stringX+","+cs.NewLine);
				cs.Append("	"+stringXLen.ToString()+","+cs.NewLine);
				cs.Append("	"+sb.Length.ToString()+","+cs.NewLine);
				cs.Append("};"+cs.NewLine);
			}
		}
		protected static void BuildServiceStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				BuildServiceStructs(cs,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				cs.Append("struct UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+cs.NewLine);
				cs.Append("{"+cs.NewLine);
				foreach(UPnPAction a in s.Actions)
				{
					cs.Append("	struct UPnP_Action_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+" *"+a.Name+";"+cs.NewLine);
				}
				cs.Append(cs.NewLine);
				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					cs.Append("	struct UPnP_StateVariable_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+v.Name+" *StateVar_"+v.Name+";"+cs.NewLine);
				}


				string stringX;
				int stringXLen;
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter X = new XmlTextWriter(SW);
				s.GetServiceXML(X);
				InjectCompressedString(out stringX,out stringXLen,sb.ToString(),cs.NewLine);

				cs.Append(cs.NewLine);
				cs.Append("	char Reserved["+stringXLen.ToString()+"];"+cs.NewLine);
				cs.Append("	int ReservedXL;"+cs.NewLine);
				cs.Append("	int ReservedUXL;"+cs.NewLine);
				cs.Append("};"+cs.NewLine);
			}
		}
		protected static void PopulateDeviceStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				PopulateDeviceStructs(cs,ed);
			}
			cs.Append("struct UPnP_Device_"+d.User2.ToString()+" UPnP_Device_"+d.User2.ToString()+"_Impl = "+cs.NewLine);
			cs.Append("{"+cs.NewLine);
			foreach(UPnPService s in d.Services)
			{
				cs.Append("	&UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_Impl,"+cs.NewLine);
			}
			cs.Append(cs.NewLine);
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				cs.Append("	&UPnP_Device_"+ed.User2.ToString()+"_Impl,"+cs.NewLine);
			}
			cs.Append("	NULL,"+cs.NewLine); // Friendly
			if (d.ParentDevice==null)
			{
				cs.Append("	NULL,"+cs.NewLine+"	NULL,"+cs.NewLine); //UDN, Serial
			}
			cs.Append(" NULL,"+cs.NewLine); //Manufacturer
			cs.Append(" NULL,"+cs.NewLine); //ManufacturerURL
			cs.Append(" NULL,"+cs.NewLine); //ModelDescription
			cs.Append(" NULL,"+cs.NewLine); //ModelName
			cs.Append(" NULL,"+cs.NewLine); //ModelNumber
			cs.Append(" NULL,"+cs.NewLine); //ModelURL
			cs.Append(" NULL,"+cs.NewLine); //Product Code

			UPnPDevice[] embeddedDevices = d.EmbeddedDevices;
			UPnPService[] services = d.Services;

			d.EmbeddedDevices = new UPnPDevice[0];
			d.Services = new UPnPService[0];

			string xmlString;

			if (d.ParentDevice==null)
				{byte[] xml;
				xml = d.GetRootDeviceXML(null);
				xmlString = (new UTF8Encoding()).GetString(xml);
			}
			else
			{
				StringBuilder sb = new StringBuilder();
				StringWriter SW = new StringWriter(sb);
				XmlTextWriter XDoc = new XmlTextWriter(SW);
				(new UPnPDebugObject(d)).InvokeNonStaticMethod("GetNonRootDeviceXML",new object[2]{null,XDoc});
				SW.Flush();
				xmlString = sb.ToString();
			}
			string stringX;
			int stringXLen;
			InjectCompressedString(out stringX,out stringXLen,xmlString,cs.NewLine);

			cs.Append("	"+stringX+","+cs.NewLine);
			cs.Append("	"+stringXLen.ToString()+","+cs.NewLine);
			cs.Append("	"+xmlString.Length.ToString()+","+cs.NewLine);
			if (d.ParentDevice==null)
			{
				cs.Append("	NULL,"+cs.NewLine);
			}
			cs.Append("	NULL"+cs.NewLine);
			cs.Append("};"+cs.NewLine);

			d.EmbeddedDevices = embeddedDevices;
			d.Services = services;
		}
		protected static void BuildDeviceStructs(CodeProcessor cs, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				BuildDeviceStructs(cs,ed);
			}
			cs.Append("struct UPnP_Device_"+d.User2.ToString()+cs.NewLine);
			cs.Append("{"+cs.NewLine);
			foreach(UPnPService s in d.Services)
			{
				cs.Append("	struct UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+" *"+((ServiceGenerator.ServiceConfiguration)s.User).Name+";"+cs.NewLine);
			}
			cs.Append(cs.NewLine);
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				cs.Append("	struct UPnP_Device_"+ed.User2.ToString()+" *UPnP_Device_"+ed.User2.ToString()+";"+cs.NewLine);
			}
			cs.Append("	const char *FriendlyName;"+cs.NewLine);
			if (d.ParentDevice==null)
			{
				cs.Append("	const char *UDN;"+cs.NewLine);
				cs.Append("	const char *Serial;"+cs.NewLine);
			}
			cs.Append(" const char *Manufacturer;"+cs.NewLine);
			cs.Append(" const char *ManufacturerURL;"+cs.NewLine);
			cs.Append(" const char *ModelDescription;"+cs.NewLine);
			cs.Append(" const char *ModelName;"+cs.NewLine);
			cs.Append(" const char *ModelNumber;"+cs.NewLine);
			cs.Append(" const char *ModelURL;"+cs.NewLine);
			cs.Append("	const char *ProductCode;"+cs.NewLine);

			UPnPDevice[] embeddedDevices = d.EmbeddedDevices;
			UPnPService[] services = d.Services;

			d.EmbeddedDevices = new UPnPDevice[0];
			d.Services = new UPnPService[0];

			byte[] xml;
			if (d.Root)
			{
				xml = d.GetRootDeviceXML(null);
			}
			else
			{
				xml = (byte[])(new UPnPDebugObject(d)).InvokeNonStaticMethod("GetRootDeviceXML",new object[1]{null});
			}
			UTF8Encoding U = new UTF8Encoding();
			string xmlString = U.GetString(xml);
			string stringX;
			int stringXLen;
			InjectCompressedString(out stringX,out stringXLen,xmlString,cs.NewLine);
			d.EmbeddedDevices = embeddedDevices;
			d.Services = services;

			cs.Append("	char Reserved["+stringXLen.ToString()+"];"+cs.NewLine);
			cs.Append("	int ReservedXL;"+cs.NewLine);
			cs.Append("	int ReservedUXL;"+cs.NewLine);
			
			cs.Append("	void *User;"+cs.NewLine);
			if (d.ParentDevice==null)
			{
				cs.Append("	void *MicrostackToken;"+cs.NewLine);
			}
			cs.Append("};"+cs.NewLine);
		}
		public static void PrepDevice(UPnPDevice[] devices)
		{
			foreach(UPnPDevice d in devices)
			{
				Hashtable t = new Hashtable();
				LabelDevices(t,d);
			}
		}
		public static int CalculateMaxAllowedValues(UPnPDevice d, int maxVal)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				maxVal = CalculateMaxAllowedValues(ed,maxVal);
			}
			foreach(UPnPService s in d.Services)
			{
				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					if (v.AllowedStringValues!=null)
					{
						if (v.AllowedStringValues.Length>maxVal)
						{
							maxVal = v.AllowedStringValues.Length;
						}
					}
				}
			}
			return(maxVal);
		}
		public static string GetDeviceObjectsString(UPnPDevice d)
		{
			CodeProcessor cs = new CodeProcessor(new StringBuilder(),false);
			cs.NewLine = "\r\n";

			//
			// Calculate the size to initialize "UPnP_StateVariable_AllowedValues_MAX"
			//
			int max = CalculateMaxAllowedValues(d,0);
			
			if (max!=0)
			{
				++max;
				cs.Append("#define UPnP_StateVariable_AllowedValues_MAX "+max+cs.NewLine);
			}

			BuildStateVariableStructs(cs,d);
			BuildActionStructs(cs,d);
			BuildServiceStructs(cs,d);
			BuildDeviceStructs(cs,d);

			return(cs.ToString());
		}
		public static string GetPopulatedDeviceObjectsString(UPnPDevice d)
		{
			Hashtable VarTable = new Hashtable();
			Hashtable ActionTable = new Hashtable();

			CodeProcessor cs = new CodeProcessor(new StringBuilder(),false);
			cs.NewLine = "\r\n";

			DeviceObjectGenerator.GenerateStateVariableLookupTable(d,VarTable);
			DeviceObjectGenerator.GenerateActionLookupTable(d,ActionTable);

			PopulateStateVariableStructs(cs,d,VarTable);
			PopulateActionStructs(cs,d,ActionTable);
			PopulateServiceStructs(cs,d);
			PopulateDeviceStructs(cs,d);

			return(cs.ToString());
		}
		private static string BuildDeviceDescriptionStreamer_EmbeddedDevice(UPnPDevice d, string WS, string WS2)
		{
			string DeviceTemplate = WS2;
			string WS3;

			string deviceIdentifier = "";
			string rootDeviceIdentifier = "";

			UPnPDevice t = d;
			while(t.ParentDevice!=null)
			{
				t = t.ParentDevice;
			}
			
			deviceIdentifier = DeviceObjectGenerator.GetDeviceIdentifier(d);
			rootDeviceIdentifier = DeviceObjectGenerator.GetDeviceIdentifier(t)+".";

			if (d.ParentDevice==null)
			{
				deviceIdentifier += ".";
			}
			else
			{
				deviceIdentifier += "->";
			}


			WS2 = WS2.Replace("{{{DEVICE}}}",deviceIdentifier);
			WS2 = WS2.Replace("{{{DEVICE2}}}",rootDeviceIdentifier);

			if (d.ParentDevice==null)
			{
				// Root Device
				WS2 = WS2.Replace("{{{DEVICE_SUBTRACTION}}}","19");
			}
			else
			{
				// Embedded Device
				WS2 = WS2.Replace("{{{DEVICE_SUBTRACTION}}}","9");
			}


			foreach(UPnPService s in d.Services)
			{
				WS3 = SourceCodeRepository.GetTextBetweenTags(WS2,"//{{{SERVICE_BEGIN}}}","//{{{SERVICE_END}}}");
				string serviceIdentifier = "";
				UPnPDevice serviceDevice = s.ParentDevice;
				while(serviceDevice!=null)
				{
					if (serviceIdentifier=="")
					{
						serviceIdentifier = "UPnP_Device_"+serviceDevice.User2.ToString();
					}
					else
					{
						if (serviceDevice.ParentDevice!=null)
						{
							serviceIdentifier = "UPnP_Device_"+serviceDevice.User2.ToString()+"->" + serviceIdentifier;
						}
						else
						{
							serviceIdentifier = "UPnP_Device_"+serviceDevice.User2.ToString()+"_Impl." + serviceIdentifier;
						}
					}
					serviceDevice = serviceDevice.ParentDevice;
				}
				if (s.ParentDevice.ParentDevice==null)
				{
					serviceIdentifier += "_Impl."+((ServiceGenerator.ServiceConfiguration)s.User).Name;
				}
				else
				{
					serviceIdentifier += "->"+((ServiceGenerator.ServiceConfiguration)s.User).Name;
				}
				WS3 = WS3.Replace("{{{SERVICE}}}",serviceIdentifier);
				WS2 = SourceCodeRepository.InsertTextBeforeTag(WS2,"//{{{SERVICE_BEGIN}}}",WS3);
			}
		
			if (d.EmbeddedDevices.Length>0)
			{
				WS3 = "	if (";
				string WS4 = "";
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					WS3 = WS3 + WS4 + DeviceObjectGenerator.GetDeviceIdentifier(ed)+"!=NULL";
					WS4 = " || ";
				}
				WS3 += ")\r\n";
				WS3 += "{\r\n";
				WS2 = SourceCodeRepository.InsertTextBeforeTag(WS2,"//{{{BEGIN_HASEMBEDDEDDEVICES}}}",WS3);
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					WS3 = BuildDeviceDescriptionStreamer_EmbeddedDevice(ed,"",DeviceTemplate);
					WS2 = SourceCodeRepository.InsertTextBeforeTag(WS2,"//{{{EMBEDDED_DEVICES}}}",WS3);
				}
				WS2 = WS2.Replace("//{{{EMBEDDED_DEVICES}}}","");
				WS3 = "}\r\n";
				WS2 = SourceCodeRepository.InsertTextBeforeTag(WS2,"//{{{END_HASEMBEDDEDDEVICES}}}",WS3);
				WS2 = SourceCodeRepository.RemoveTag("//{{{BEGIN_HASEMBEDDEDDEVICES}}}","//{{{END_HASEMBEDDEDDEVICES}}}",WS2);
			
			}
			else
			{
				WS2 = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_HASEMBEDDEDDEVICES}}}","//{{{END_HASEMBEDDEDDEVICES}}}",WS2);
			}


			WS2 = SourceCodeRepository.RemoveAndClearTag("//{{{SERVICE_BEGIN}}}","//{{{SERVICE_END}}}",WS2);
			if (d.ParentDevice==null)
			{
				WS = WS + "\r\n" + WS2;
			}
			else
			{
				//
				// This is an embedded device, so we need to check it first
				//
				WS = WS + "\r\n" + "if ("+DeviceObjectGenerator.GetDeviceIdentifier(d)+"!=NULL)\r\n";
				WS += "{\r\n" + WS2 + "\r\n}\r\n";
			}
			return(WS);
		}
		public static string BuildDeviceDescriptionStreamer(UPnPDevice d, string templateString)
		{
			string WS = BuildDeviceDescriptionStreamer_EmbeddedDevice(d,"",SourceCodeRepository.GetTextBetweenTags(templateString,"//{{{DEVICE_BEGIN}}}","//{{{DEVICE_END}}}"));
			
			templateString = SourceCodeRepository.InsertTextBeforeTag(templateString,"//{{{DEVICE_BEGIN}}}",WS);
			templateString = SourceCodeRepository.RemoveAndClearTag("//{{{DEVICE_BEGIN}}}","//{{{DEVICE_END}}}",templateString);
			return(templateString);
		}
		public static string GetStateVariableIdentifier(UPnPStateVariable var)
		{
			string servID = GetServiceIdentifier(var.OwningService);
			return(servID+"->StateVar_"+var.Name);
		}
		public static string GetServiceIdentifier(UPnPService service)
		{
			string devID = GetDeviceIdentifier(service.ParentDevice);
			if (service.ParentDevice.ParentDevice==null)
			{
				return(devID+"."+((ServiceGenerator.ServiceConfiguration)service.User).Name);
			}
			else
			{
				return(devID+"->"+((ServiceGenerator.ServiceConfiguration)service.User).Name);
			}
		}
		public static string GetDeviceIdentifier(UPnPDevice device)
		{
			string deviceIdent = "";
			UPnPDevice pDevice = device;
			string prefix = ((ServiceGenerator.Configuration)device.User).Prefix;

			while(pDevice!=null)
			{
				if (deviceIdent=="")
				{
					if (pDevice.ParentDevice==null)
					{
						
						deviceIdent = "UPnP_Device_"+pDevice.User2.ToString()+"_Impl";
					}
					else
					{
						deviceIdent = "UPnP_Device_"+pDevice.User2.ToString();
					}
				}
				else
				{
					if (pDevice.ParentDevice==null)
					{
						deviceIdent = "UPnP_Device_"+pDevice.User2.ToString()+"_Impl."+deviceIdent;
					}
					else
					{
						deviceIdent = "UPnP_Device_"+pDevice.User2.ToString()+"->"+deviceIdent;	
					}
				}
				deviceIdent = deviceIdent.Replace("UPnP",prefix);
				pDevice = pDevice.ParentDevice;
			}
			return(deviceIdent);
		}
		protected static string GetCPlusPlusAbstraction_H_Service(string WC, UPnPService s)
		{
			string WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Service_Begin}}}","//{{{Service_End}}}");
			string WC3;
			
			foreach(UPnPStateVariable V in s.GetStateVariables())
			{
				if (V.SendEvent)
				{
					WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Service_Events_BEGIN}}}","//{{{Service_Events_END}}}");
					WC3 = WC3.Replace("{{{VARNAME}}}",V.Name);
					WC3 = WC3.Replace("{{{PARAMDEF}}}",EmbeddedCGenerator.ToCTypeFromStateVar(V));
					WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Service_Events_BEGIN}}}",WC3);
				}
			}
			WC2 = SourceCodeRepository.RemoveAndClearTag("//{{{Service_Events_BEGIN}}}","//{{{Service_Events_END}}}",WC2);


			WC2 = WC2.Replace("{{{SERVICE}}}","UPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name);
			WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+s.ParentDevice.User2.ToString());
			StringBuilder sb = new StringBuilder();
			foreach(UPnPAction a in s.Actions)
			{
				sb.Append("virtual void "+a.Name+"(void *session");
				foreach(UPnPArgument arg in a.Arguments)
				{
					if (arg.Direction=="in")
					{
						sb.Append(", "+EmbeddedCGenerator.ToCTypeFromArg(arg));
					}
				}
				sb.Append(");\r\n");
			}
			WC2 = WC2.Replace("//{{{Service_VirtualMethods}}}",sb.ToString());

			sb = new StringBuilder();
			foreach(UPnPAction a in s.Actions)
			{
				sb.Append("void Response_"+a.Name+"(void *session");
				foreach(UPnPArgument arg in a.Arguments)
				{
					if (arg.Direction=="out")
					{
						sb.Append(", "+EmbeddedCGenerator.ToCTypeFromArg(arg));
					}
				}
				sb.Append(");\r\n");
				if (((ServiceGenerator.ServiceConfiguration)s.User).Actions_Fragmented.Contains(a))
				{
					sb.Append("	void Response_Async_"+a.Name+"(void *session, char* ArgName, char *ArgValue, int ArgValueLength, int ArgStart, int ArgDone, int ResponseDone);\r\n");
				}
			}
			WC2 = WC2.Replace("//{{{Service_VirtualMethods_Response}}}",sb.ToString());
			return(WC2);
		}
		protected static string GetCPlusPlusAbstraction_H_Device(string WC, UPnPDevice d)
		{
			string WC2;

			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC = GetCPlusPlusAbstraction_H_Device(WC,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				WC2 = GetCPlusPlusAbstraction_H_Service(WC,s);
				WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Service_Begin}}}",WC2);
			}
			WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Device_Begin}}}","//{{{Device_End}}}");
			WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
			StringBuilder sSinks = new StringBuilder();
			StringBuilder sList = new StringBuilder();
			foreach(UPnPService s in d.Services)
			{
				sList.Append("CUPnP_Service_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+" *m_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+";\r\n");
				foreach(UPnPAction a in s.Actions)
				{
					sSinks.Append("static void "+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+a.Name+"_Sink(");
					sSinks.Append("void *session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							sSinks.Append(", "+EmbeddedCGenerator.ToCTypeFromArg(arg));
						}
					}
					sSinks.Append(");\r\n");
				}
			}
			WC2 = WC2.Replace("//{{{Device_ServiceList}}}",sList.ToString());
			WC2 = WC2.Replace("//{{{Device_StaticSinks}}}",sSinks.ToString());
			
			sList = new StringBuilder();
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				sList.Append("CUPnP_Device_"+ed.User2.ToString()+" *m_Device_"+ed.User2.ToString()+";\r\n");
			}
			WC2 = WC2.Replace("//{{{Device_DeviceList}}}",sList.ToString());
			
			if (d.ParentDevice!=null)
			{
				WC2 = WC2.Replace("//{{{ParentFriends}}}","friend class CUPnP_Device_"+d.ParentDevice.User2.ToString()+";\r\n");
				WC2 = WC2.Replace("{{{EMBEDDED}}}",", CUPnP_Device_"+d.ParentDevice.User2.ToString()+" *parentDevice");
			}
			else
			{
				WC2 = WC2.Replace("//{{{ParentFriends}}}","");
				WC2 = WC2.Replace("{{{EMBEDDED}}}","");
			}

			WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Device_Begin}}}",WC2);
			return(WC);
		}
		protected static string GetCPlusPlusAbstraction_CPP_Device_FriendlyName(string WC, UPnPDevice d, int i)
		{
			WC += ",\""+d.FriendlyName;
			if (i!=1)
			{
				WC += i.ToString();
			}
			WC += "\"";
			
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC = GetCPlusPlusAbstraction_CPP_Device_FriendlyName(WC,ed,++i);
			}
			return(WC);
		}
		protected static string GetCPlusPlusAbstraction_CPP_Device(string WC, UPnPDevice d)
		{
			string WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Device_Begin}}}","//{{{Device_End}}}");
			string WC3,WC4;

			if (d.ParentDevice==null)
			{
				//
				// This is the root device
				//
				WC3 = "MicrostackToken = "+((ServiceGenerator.Configuration)d.User).Prefix+"CreateMicroStack(MicrostackChain"+GetCPlusPlusAbstraction_CPP_Device_FriendlyName("",d,1);
				WC3 += ",\""+Guid.NewGuid()+"\",\"000001\","+((ServiceGenerator.Configuration)d.User).SSDPCycleTime.ToString()+","+((ServiceGenerator.Configuration)d.User).WebPort+");\r\n";
				WC2 = WC2.Replace("//{{{CreateMicroStack}}}",WC3);
			}
			else
			{
				WC2 = WC2.Replace("//{{{CreateMicroStack}}}","");
			}

			foreach(UPnPService s in d.Services)
			{
				WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Destructor_Begin}}}","//{{{Destructor_End}}}");
				WC3 = WC3.Replace("{{{NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Destructor_Begin}}}",WC3);

				WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Service_Instantiation_Begin}}}","//{{{Service_Instantiation_End}}}");
				WC3 = WC3.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Service_Instantiation_Begin}}}",WC3);
			
				foreach(UPnPAction a in s.Actions)
				{
					WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{SinkList_Begin}}}","//{{{SinkList_End}}}");
					WC3 = WC3.Replace("{{{Prefix}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC3 = WC3.Replace("{{{SERVICE_SHORT_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					WC3 = WC3.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC3 = WC3.Replace("{{{ACTION_NAME}}}",a.Name);
					WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{SinkList_Begin}}}",WC3);
				
					//
					// Dispatch
					//
					WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Dispatch_Begin}}}","//{{{Dispatch_End}}}");
					WC3 = WC3.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC3 = WC3.Replace("{{{SERVICE_SHORT_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					WC3 = WC3.Replace("{{{ACTION_NAME}}}",a.Name);
					UPnPDevice rDevice = s.ParentDevice;
					while(rDevice.ParentDevice!=null)
					{
						rDevice = rDevice.ParentDevice;
					}
					WC3 = WC3.Replace("{{{ROOTDEVICE}}}","UPnP_Device_"+rDevice.User2.ToString());
					rDevice = s.ParentDevice;
					WC4 = "";
					while(rDevice.ParentDevice!=null)
					{
						WC4 = "m_Device_"+rDevice.User2.ToString()+"->" + WC4;
						rDevice = rDevice.ParentDevice;
					}
					WC3 = WC3.Replace("{{{DEVICELIST}}}",WC4);


					StringBuilder paramList = new StringBuilder();
					paramList.Append("void *session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							paramList.Append(",");
							paramList.Append(EmbeddedCGenerator.ToCTypeFromArg(arg));
						}
					}
					WC3 = WC3.Replace("{{{PARAM_LIST}}}",paramList.ToString());
					paramList = new StringBuilder();
					paramList.Append("session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							paramList.Append(","+arg.Name);
						}
					}
					WC3 = WC3.Replace("{{{PARAM_LIST_DISPATCH}}}",paramList.ToString());
					WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Dispatch_Begin}}}",WC3);
				}
			}
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Destructor_Begin}}}","//{{{Destructor_End}}}");
				WC3 = WC3.Replace("{{{NAME}}}","Device_"+ed.User2.ToString());
				WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Destructor_Begin}}}",WC3);

				WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{Device_Instantiation_Begin}}}","//{{{Device_Instantiation_End}}}");
				WC3 = WC3.Replace("{{{DEVICE}}}","Device_"+ed.User2.ToString());
				WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{Device_Instantiation_Begin}}}",WC3);
			}

			WC2 = WC2.Replace("{{{Prefix}}}",((ServiceGenerator.Configuration)d.User).Prefix);
			//WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Device_Begin}}}",WC2);
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC3 = GetCPlusPlusAbstraction_CPP_Device(WC,ed);
				WC2 = WC3 + "\r\n" + WC2;
				//WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Device_Begin}}}",WC3);
			}

			foreach(UPnPService s in d.Services)
			{
				WC3 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Service_Begin}}}","//{{{Service_End}}}");
				WC3 = WC3.Replace("{{{SERVICE_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				WC3 = WC3.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
				WC3 = WC3.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);

				//
				// Evented State Variables
				//
				foreach(UPnPStateVariable V in s.GetStateVariables())
				{
					if (V.SendEvent)
					{
						WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{SERVICE_EVENTS_BEGIN}}}","//{{{SERVICE_EVENTS_END}}}");
						WC4 = WC4.Replace("{{{SERVICE_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
						WC4 = WC4.Replace("{{{VARNAME}}}",V.Name);
						WC4 = WC4.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
						WC4 = WC4.Replace("{{{PARAMDEF}}}",EmbeddedCGenerator.ToCTypeFromStateVar(V));
						WC4 = WC4.Replace("{{{PARAMLIST}}}","ParentDevice->GetToken(),"+EmbeddedCGenerator.ToCTypeFromStateVar_Dispatch(V));
						WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{SERVICE_EVENTS_BEGIN}}}",WC4);
					}
				}
				WC3 = SourceCodeRepository.RemoveAndClearTag("//{{{SERVICE_EVENTS_BEGIN}}}","//{{{SERVICE_EVENTS_END}}}",WC3);

				foreach(UPnPAction a in s.Actions)
				{
					WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_BEGIN}}}","//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_END}}}");
					WC4 = WC4.Replace("{{{SERVICE_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					WC4 = WC4.Replace("{{{ACTION_NAME}}}",a.Name);
					WC4 = WC4.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)s.ParentDevice.User).Prefix);
					WC4 = WC4.Replace("{{{URN}}}",s.ServiceURN);

					if (((ServiceGenerator.ServiceConfiguration)s.User).Actions_Fragmented.Contains(a))
					{
						WC4 = SourceCodeRepository.RemoveTag("//(((FragmentedResponse_Begin}}}","//(((FragmentedResponse_End}}}",WC4);
					}
					else
					{
						WC4 = SourceCodeRepository.RemoveAndClearTag("//(((FragmentedResponse_Begin}}}","//(((FragmentedResponse_End}}}",WC4);
					}
					
					StringBuilder paramList = new StringBuilder();
					
					//
					// Input Arguments
					//
					paramList.Append("void *session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							paramList.Append(","+EmbeddedCGenerator.ToCTypeFromArg(arg));
						}
					}
					WC4 = WC4.Replace("{{{PARAM_LIST}}}",paramList.ToString());

					//
					// Output Arguments Declaration
					//
					paramList = new StringBuilder();
					paramList.Append("void *session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="out")
						{
							paramList.Append(","+EmbeddedCGenerator.ToCTypeFromArg(arg));
						}
					}
					WC4 = WC4.Replace("{{{OUTPUT_PARAM_LIST}}}",paramList.ToString());

					//
					// Output Arguments Dispatch
					//
					paramList = new StringBuilder();
					paramList.Append("session");
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="out")
						{
							paramList.Append(","+EmbeddedCGenerator.ToCTypeFromArg_Dispatch(arg));
						}
					}
					WC4 = WC4.Replace("{{{OUTPUT_PARAM_LIST_DISPATCH}}}",paramList.ToString());

					WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_BEGIN}}}",WC4);
				}
				//WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Service_Begin}}}",WC3);
				WC2 = WC2 + "\r\n" + WC3;
			}
			WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
			if (d.ParentDevice!=null)
			{
				WC2 = WC2.Replace("{{{EMBEDDED}}}",", CUPnP_Device_"+d.ParentDevice.User2.ToString()+" *parentDevice");
				WC2 = SourceCodeRepository.RemoveTag("//{{{SetToken_Begin}}}","//{{{SetToken_End}}}",WC2);	
				WC2 = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Root_Begin}}}","//{{{Device_Root_End}}}",WC2);
			}
			else
			{
				WC2 = WC2.Replace("{{{EMBEDDED}}}","");
				WC2 = SourceCodeRepository.RemoveAndClearTag("//{{{SetToken_Begin}}}","//{{{SetToken_End}}}",WC2);
				WC2 = SourceCodeRepository.RemoveTag("//{{{Device_Root_Begin}}}","//{{{Device_Root_End}}}",WC2);
			}
			return(WC2);
		}
		public static string GetCPlusPlus_DerivedSampleClasses_Implementation(UPnPDevice[] devices)
		{
			string RetVal = "";
			foreach(UPnPDevice d in devices)
			{
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					RetVal += GetCPlusPlus_DerivedSampleClasses_Implementation(new UPnPDevice[1]{ed});
				}
				foreach(UPnPService S in d.Services)
				{
					StringBuilder sb = new StringBuilder();
					if (((ServiceGenerator.Configuration)S.ParentDevice.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
					{
						sb.Append("CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"::"+"CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"(CUPnP_Device_"+S.ParentDevice.User2.ToString()+" *parent):CUPnP_Service_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"(parent)\r\n");
						sb.Append("{\r\n");
						sb.Append("}\r\n");
						sb.Append("CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"::~"+"CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"()\r\n");
						sb.Append("{\r\n");
						sb.Append("}\r\n");


						foreach(UPnPAction A in S.Actions)
						{
							sb.Append("void CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"::"+A.Name+"(void *session");
							foreach(UPnPArgument arg in A.Arguments)
							{
								if (arg.Direction=="in")
								{
									sb.Append(","+EmbeddedCGenerator.ToCTypeFromArg(arg));
								}
							}
							sb.Append(")\r\n");
							sb.Append("{\r\n");
							sb.Append("	Error(session,501,\"Sample Implementation\");\r\n");
							sb.Append("}\r\n");
						}
					}
					RetVal += sb.ToString();
				}
			}
			return(RetVal);
		}
		public static string GetCPlusPlus_DerivedSampleClasses_Insertion(UPnPDevice[] devices)
		{
			string RetVal = "";
			foreach(UPnPDevice d in devices)
			{
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					RetVal += GetCPlusPlus_DerivedSampleClasses_Insertion(new UPnPDevice[1]{ed});
				}
				foreach(UPnPService S in d.Services)
				{
					if (((ServiceGenerator.Configuration)S.ParentDevice.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
					{
						string val = "";
						UPnPDevice pD = S.ParentDevice;
						while(pD.ParentDevice!=null)
						{
							if (val=="")
							{
								val = "m_Device_"+pD.User2.ToString();
							}
							else
							{
								val = "m_Device_"+pD.User2.ToString()+"->"+val;
							}
							pD = pD.ParentDevice;
						}
						if (S.ParentDevice.ParentDevice==null)
						{
							val = "pUPnP->Get_UPnP_Device_"+pD.User2.ToString()+"()";
						}
						else
						{
							val = "pUPnP->Get_UPnP_Device_"+pD.User2.ToString()+"()->"+val;
						}
						
						RetVal += "	delete " + val + "->m_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+";\r\n";
						RetVal += "	"+val+"->m_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+" = new CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"("+val+");\r\n";
					}
				}
			}
			return(RetVal);
		}
		public static string GetCPlusPlus_DerivedSampleClasses(UPnPDevice[] devices)
		{
			string RetVal = "";
			foreach(UPnPDevice d in devices)
			{
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					RetVal += GetCPlusPlus_DerivedSampleClasses(new UPnPDevice[1]{ed});
				}
				foreach(UPnPService S in d.Services)
				{
					StringBuilder sb = new StringBuilder();
					if (((ServiceGenerator.Configuration)S.ParentDevice.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
					{
						sb.Append("class CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+" : public CUPnP_Service_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"\r\n");
						sb.Append("{\r\n");
						sb.Append("	public:\r\n");
						sb.Append("	CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"(CUPnP_Device_"+S.ParentDevice.User2.ToString()+" *parent);\r\n");
						sb.Append("	virtual ~CUPnP_SampleService_"+((ServiceGenerator.ServiceConfiguration)S.User).Name+"();\r\n");
						sb.Append("\r\n");
						foreach(UPnPAction A in S.Actions)
						{
							sb.Append("	virtual void "+A.Name+"(void *session");
							foreach(UPnPArgument arg in A.Arguments)
							{
								if (arg.Direction=="in")
								{
									sb.Append(","+EmbeddedCGenerator.ToCTypeFromArg(arg));
								}
							}
							sb.Append(");\r\n");
						}
						sb.Append("};\r\n");
					}
					RetVal += sb.ToString();
				}
			}
			return(RetVal);
		}
		private static string AddCPlusPlusAbstraction_CPP_ControlPoint(string WC, UPnPDevice d)
		{
			string WC2;
			string WC3;
			string arglist,arglist2;
			string outarglist, outarglist2;

			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC = AddCPlusPlusAbstraction_CPP_ControlPoint(WC,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_ServiceCheck}}}","//{{{END_ServiceCheck}}}");
				WC3 = "if (strlen(s->ServiceType)>"+s.ServiceURN_Prefix.Length.ToString()+" && strncmp(s->ServiceType,\""+s.ServiceURN_Prefix+"\","+s.ServiceURN_Prefix.Length.ToString()+")==0)";
				WC2 = WC2.Replace("{{{COMPARESTRING}}}",WC3);
				WC2 = WC2.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_ServiceCheck}}}",WC2);
				
				// Constructor
				WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CP_Constructor}}}","//{{{END_CP_Constructor}}}");
				WC2 = WC2.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				foreach(UPnPStateVariable sv in s.GetStateVariables())
				{
					if (sv.SendEvent)
					{
						WC3 = SourceCodeRepository.GetTextBetweenTags(WC2,"//{{{BEGIN_EVENT}}}","//{{{END_EVENT}}}");
						WC3 = WC3.Replace("{{{STATEVAR}}}",sv.Name);
						WC2 = SourceCodeRepository.InsertTextBeforeTag(WC2,"//{{{BEGIN_EVENT}}}",WC3);
					}
				}							
				WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CP_Constructor}}}",WC2);


				//
				// Subscription Override
				//
				bool HasEventing = false;
				foreach(UPnPStateVariable sv in s.GetStateVariables())
				{
					if (sv.SendEvent)
					{
						HasEventing=true;
						break;
					}
				}
				if (HasEventing)
				{
					WC3 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CPEVENT_SUBSCRIBE}}}","//{{{END_CPEVENT_SUBSCRIBE}}}");
					WC3 = WC3.Replace("{{{SERVICE_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					foreach(UPnPStateVariable sv in s.GetStateVariables())
					{
						if (sv.SendEvent)
						{
							string reg = ((ServiceGenerator.Configuration)d.User).Prefix+"EventCallback_"+((ServiceGenerator.ServiceConfiguration)s.User).Name+"_"+sv.Name+" = &EventSink_"+sv.Name+";";
							WC3 = WC3.Replace("//{{{REGISTER}}}",reg+"\n"+"//{{{REGISTER}}}");
						}
					}
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CPEVENT_SUBSCRIBE}}}",WC3);

					foreach(UPnPStateVariable sv in s.GetStateVariables())
					{
						if (sv.SendEvent)
						{
							WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CPEVENT_SINK}}}","//{{{END_CPEVENT_SINK}}}");
							WC2 = WC2.Replace("{{{SERVICE_NAME}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
							WC2 = WC2.Replace("{{{STATEVAR}}}",sv.Name);
							WC2 = WC2.Replace("{{{ARGTYPE}}}",EmbeddedCGenerator.ToCTypeFromStateVar(sv));
							WC2 = WC2.Replace("{{{ARGLIST}}}",EmbeddedCGenerator.ToCTypeFromStateVar_Dispatch(sv));
							WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CPEVENT_SINK}}}",WC2);
						}
					}
				}



				// Invoke and InvokeSink
				foreach(UPnPAction a in s.Actions)
				{
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CP_Invoke}}}","//{{{END_CP_Invoke}}}");
					WC2 = WC2.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					WC2 = WC2.Replace("{{{ACTION}}}",a.Name);
					WC2 = WC2.Replace("{{{ONACTION}}}","On"+a.Name);
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					arglist = "";
					arglist2 = "";
					outarglist = "";
					outarglist2 = "";
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							arglist += ","+EmbeddedCGenerator.ToCTypeFromArg(arg);
							arglist2 += ","+EmbeddedCGenerator.ToCTypeFromArg_Dispatch(arg);
						}
						if (arg.Direction=="out")
						{
							outarglist += ","+EmbeddedCGenerator.ToCTypeFromArg(arg);
							outarglist2 += ","+EmbeddedCGenerator.ToCTypeFromArg_Dispatch(arg);
						}
					}
					WC2 = WC2.Replace("{{{INARGS}}}",arglist);
					WC2 = WC2.Replace("{{{INARGS_Values}}}",arglist2);
					WC2 = WC2.Replace("{{{OUTARGS}}}",outarglist);
					WC2 = WC2.Replace("{{{OUTARGS_Values}}}",outarglist2);
					
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CP_Invoke}}}",WC2);
				}
			}

			return(WC);
		}
		public static string GetCPlusPlusAbstraction_CPP(UPnPDevice[] devices)
		{
			string WC = SourceCodeRepository.GetCPlusPlus_Template_CPP("UPnP");
			string WC2;
			CodeProcessor sb = new CodeProcessor(new StringBuilder(),false);
			bool ok = false;
			bool cpok = false;

			foreach(UPnPDevice d in devices)
			{
				if (((ServiceGenerator.Configuration)d.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
				{
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{MicroStackInclude_Begin}}}","//{{{MicroStackInclude_End}}}");
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{MicroStackInclude_Begin}}}",WC2);

					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_Constructor_Begin}}}","//{{{Manager_Constructor_End}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_Constructor_Begin}}}",WC2);
					
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_GetDevice_Begin}}}","//{{{Manager_GetDevice_End}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_GetDevice_Begin}}}",WC2);

					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_Destructor_Begin}}}","//{{{Manager_Destructor_End}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_Destructor_Begin}}}",WC2);

					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{IPADDRESS_HANDLER_BEGIN}}}","//{{{IPADDRESS_HANDLER_END}}}");
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{IPADDRESS_HANDLER_BEGIN}}}",WC2);

					WC2 = GetCPlusPlusAbstraction_CPP_Device(WC,d);
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Device_Begin}}}",WC2);
					ok = true;	
				}
				else
				{
					// Includes
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{CPMicroStackInclude_Begin}}}","//{{{CPMicroStackInclude_End}}}");
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{CPMicroStackInclude_Begin}}}",WC2);

					// Constructor
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_CPConstructor_Begin}}}","//{{{Manager_CPConstructor_End}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}",d.User2.ToString());
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC2 = WC2.Replace("{{{DEVICEID}}}",d.GetHashCode().ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_CPConstructor_Begin}}}",WC2);

					// Discover/Remove Sinks
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CPDiscoverSink}}}","//{{{END_CPDiscoverSink}}}");
					WC2 = WC2.Replace("{{{PREFIX}}}",((ServiceGenerator.Configuration)d.User).Prefix);
					WC2 = WC2.Replace("{{{DEVICE}}}",d.User2.ToString());
					WC2 = WC2.Replace("{{{DEVICEID}}}",d.GetHashCode().ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CPDiscoverSink}}}",WC2);

					// SetControlPoint
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_SetControlPoint}}}","//{{{END_SetControlPoint}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}",d.User2.ToString());
					WC2 = WC2.Replace("{{{DEVICEID}}}",d.GetHashCode().ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_SetControlPoint}}}",WC2);


					// Device/Service Specific Stuff
					WC = AddCPlusPlusAbstraction_CPP_ControlPoint(WC,d);
					
					cpok = true;
					ok = true;
				}
			}
			if (!cpok)
			{
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{CP_BEGIN}}}","//{{{CP_END}}}",WC);
			}
			else
			{
				WC = SourceCodeRepository.RemoveTag("//{{{CP_BEGIN}}}","//{{{CP_END}}}",WC);
			}
			if (ok)
			{
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_Destructor_Begin}}}","//{{{Manager_Destructor_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_Constructor_Begin}}}","//{{{Manager_Constructor_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_GetDevice_Begin}}}","//{{{Manager_GetDevice_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Begin}}}","//{{{Device_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Service_Begin}}}","//{{{Service_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_BEGIN}}}","//{{{SERVICE_VIRTUAL_METHOD_BASE_IMPLEMENTATION_END}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Service_Instantiation_Begin}}}","//{{{Service_Instantiation_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Instantiation_Begin}}}","//{{{Device_Instantiation_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{SinkList_Begin}}}","//{{{SinkList_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Destructor_Begin}}}","//{{{Destructor_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Dispatch_Begin}}}","//{{{Dispatch_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{MicroStackInclude_Begin}}}","//{{{MicroStackInclude_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{IPADDRESS_HANDLER_BEGIN}}}","//{{{IPADDRESS_HANDLER_END}}}",WC);
				
				
				// Control Point Specific
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{CPMicroStackInclude_Begin}}}","//{{{CPMicroStackInclude_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_CPConstructor_Begin}}}","//{{{Manager_CPConstructor_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CPDiscoverSink}}}","//{{{END_CPDiscoverSink}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_ServiceCheck}}}","//{{{END_ServiceCheck}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_SetControlPoint}}}","//{{{END_SetControlPoint}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_Constructor}}}","//{{{END_CP_Constructor}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_Invoke}}}","//{{{END_CP_Invoke}}}",WC);
	
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CPEVENT_SINK}}}","//{{{END_CPEVENT_SINK}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CPEVENT_SUBSCRIBE}}}","//{{{END_CPEVENT_SUBSCRIBE}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EVENT}}}","//{{{END_EVENT}}}",WC);
				WC = WC.Replace("//{{{REGISTER}}}","");

				sb.Append(WC);
			}
			return(sb.ToString());
		}
		private static string GetCPlusPlusAbstraction_H_FriendDevice(string WC,UPnPDevice d)
		{
			string WC2;
			if (((ServiceGenerator.Configuration)d.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
			{
				WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_Friends_BEGIN}}}","//{{{Manager_Friends_END}}}");
				WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
			
				foreach(UPnPDevice ed in d.EmbeddedDevices)
				{
					WC2 += GetCPlusPlusAbstraction_H_FriendDevice(WC,ed);
				}
				return(WC2);
			}
			else
			{
				return("");
			}
		}
		private static string GetCPlusPlusAbstraction_H_Device_CP(string WC, UPnPDevice d)
		{
			string WC2 = "";
			string WC3,WC4;
			string arglist;

			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				WC2 += GetCPlusPlusAbstraction_H_Device_CP(WC,ed);
			}


			foreach(UPnPService s in d.Services)
			{
				WC3 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{BEGIN_CP_SERVICE}}}","//{{{END_CP_SERVICE}}}");

				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					if (v.SendEvent)
					{
						// Event Sink
						WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_CP_EVENTSINK}}}","//{{{END_CP_EVENTSINK}}}");
						WC4 = WC4.Replace("{{{STATEVAR}}}",v.Name);
						WC4 = WC4.Replace("{{{ARGTYPE}}}",EmbeddedCGenerator.ToCTypeFromStateVar(v));
						WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_CP_EVENTSINK}}}",WC4);
					
						//Typedef						
						WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_EVENT_TYPEDEF}}}","//{{{END_EVENT_TYPEDEF}}}");
						WC4 = WC4.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
						WC4 = WC4.Replace("{{{STATEVAR}}}",v.Name);
						WC4 = WC4.Replace("{{{ARGLIST}}}",EmbeddedCGenerator.ToCTypeFromStateVar(v));
						WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_EVENT_TYPEDEF}}}",WC4);

						//Event
						WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_EVENT}}}","//{{{END_EVENT}}}");
						WC4 = WC4.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
						WC4 = WC4.Replace("{{{STATEVAR}}}",v.Name);
						WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_EVENT}}}",WC4);
					}
				}
				foreach(UPnPAction a in s.Actions)
				{
					// Typedef
					WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_INVOKE_TYPEDEF}}}","//{{{END_INVOKE_TYPEDEF}}}");
					WC4 = WC4.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					WC4 = WC4.Replace("{{{ONACTION}}}","On"+a.Name);
					arglist = "";
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="out")
						{
							arglist += ","+EmbeddedCGenerator.ToCTypeFromArg(arg);
						}
					}
					WC4 = WC4.Replace("{{{OUTARGLIST}}}",arglist);
					WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_INVOKE_TYPEDEF}}}",WC4);	


					// InvokeSink
					WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_CP_INVOKESINK}}}","//{{{END_CP_INVOKESINK}}}");
					WC4 = WC4.Replace("{{{ACTION}}}",a.Name);
					arglist = "";
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="out")
						{
							arglist += ","+EmbeddedCGenerator.ToCTypeFromArg(arg);
						}
					}
					WC4 = WC4.Replace("{{{OUTARGLIST}}}",arglist);
					WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_CP_INVOKESINK}}}",WC4);
					
					// Invoke
					WC4 = SourceCodeRepository.GetTextBetweenTags(WC3,"//{{{BEGIN_CP_INVOKE}}}","//{{{END_CP_INVOKE}}}");
					WC4 = WC4.Replace("{{{ACTION}}}",a.Name);
					WC4 = WC4.Replace("{{{ONACTION}}}","On"+a.Name);
					WC4 = WC4.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
					arglist = "";
					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							arglist += ","+EmbeddedCGenerator.ToCTypeFromArg(arg);
						}
					}
					WC4 = WC4.Replace("{{{INARGLIST}}}",arglist);
					WC3 = SourceCodeRepository.InsertTextBeforeTag(WC3,"//{{{BEGIN_CP_INVOKE}}}",WC4);	
				}



				WC3 = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_INVOKE_TYPEDEF}}}","//{{{END_INVOKE_TYPEDEF}}}",WC3);
				WC3 = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_INVOKESINK}}}","//{{{END_CP_INVOKESINK}}}",WC3);
				WC3 = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_INVOKE}}}","//{{{END_CP_INVOKE}}}",WC3);
				WC3 = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_EVENTSINK}}}","//{{{END_CP_EVENTSINK}}}",WC3);


				WC3 = WC3.Replace("{{{SERVICE}}}",((ServiceGenerator.ServiceConfiguration)s.User).Name);
				WC3 = WC3.Replace("{{{URN}}}",s.ServiceURN);
				WC2 += WC3;
			}
			return(WC2);
		}
		public static string GetCPlusPlusAbstraction_H(UPnPDevice[] devices)
		{
			string WC = SourceCodeRepository.GetCPlusPlus_Template_H("UPnP");
			string WC2;
			CodeProcessor sb = new CodeProcessor(new StringBuilder(),false);
			bool ok = false;
			bool CPok = false;

			foreach(UPnPDevice d in devices)
			{
				if (((ServiceGenerator.Configuration)d.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
				{
					WC2 = GetCPlusPlusAbstraction_H_FriendDevice(WC,d);
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_Friends_BEGIN}}}",WC2);

					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_GetDevice_BEGIN}}}","//{{{Manager_GetDevice_END}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_GetDevice_BEGIN}}}",WC2);

					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_Device_BEGIN}}}","//{{{Manager_Device_END}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}","UPnP_Device_"+d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_Device_BEGIN}}}",WC2);
				
					WC = GetCPlusPlusAbstraction_H_Device(WC,d);
					ok = true;
				}
				else
				{
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_SetControlPoint_BEGIN}}}","//{{{Manager_SetControlPoint_END}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}",d.User2.ToString());
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_SetControlPoint_BEGIN}}}",WC2);
				
					WC2 = SourceCodeRepository.GetTextBetweenTags(WC,"//{{{Manager_ProtectedCP_Stuff_BEGIN}}}","//{{{Manager_ProtectedCP_Stuff_END}}}");
					WC2 = WC2.Replace("{{{DEVICE}}}",d.User2.ToString());
					WC2 = WC2.Replace("{{{DEVICE_ID}}}",d.GetHashCode().ToString());	
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{Manager_ProtectedCP_Stuff_BEGIN}}}",WC2);
					
					WC2 = GetCPlusPlusAbstraction_H_Device_CP(WC,d);
					WC = SourceCodeRepository.InsertTextBeforeTag(WC,"//{{{BEGIN_CP_SERVICE}}}",WC2);
					CPok = true;
					ok = true;
				}
			}

		
			WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_SetControlPoint_BEGIN}}}","//{{{Manager_SetControlPoint_END}}}",WC);
			WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_ProtectedCP_Stuff_BEGIN}}}","//{{{Manager_ProtectedCP_Stuff_END}}}",WC);
			
			if (!CPok)
			{
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{ControlPoint_Begin}}}","//{{{ControlPoint_End}}}",WC);
			}
			else
			{
				WC = SourceCodeRepository.RemoveTag("//{{{ControlPoint_Begin}}}","//{{{ControlPoint_End}}}",WC);
			}

			if (ok)
			{
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Begin}}}","//{{{Device_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Service_Begin}}}","//{{{Service_End}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_Friends_BEGIN}}}","//{{{Manager_Friends_END}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_GetDevice_BEGIN}}}","{{{Manager_GetDevice_END}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{Manager_Device_BEGIN}}}","//{{{Manager_Device_END}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_SERVICE}}}","//{{{END_CP_SERVICE}}}",WC);
				
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EVENT_TYPEDEF}}}","//{{{END_EVENT_TYPEDEF}}}",WC);
				WC = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EVENT}}}","//{{{END_EVENT}}}",WC);
				sb.Append(WC);
			}
			return(sb.ToString());
		}
	}
}
