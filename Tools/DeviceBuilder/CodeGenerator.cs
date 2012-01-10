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
using System.Collections;
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for CodeGenerator.
	/// </summary>
	public abstract class CodeGenerator
	{
		public string AppStartPath = Application.StartupPath;
		public string UseVersion = Application.ProductVersion.Substring(0,Application.ProductVersion.LastIndexOf("."));
		public string VersionString = "Device Builder Build#" + Application.ProductVersion;
		public string ClassName = "";

		public string License = "";
		public string ProjectName = "";

		public delegate void LogOutputHandler(object sender, string msg);
		public event LogOutputHandler OnLogOutput;

		public ServiceGenerator.StackConfiguration Configuration;
		

		public abstract bool Generate(UPnPDevice[] devices,DirectoryInfo outputDirectory);
		protected Hashtable GetServiceNameTable(UPnPDevice device)
		{
			return(SourceCodeRepository.CreateTableOfServiceNames(device));
		}

		public CodeGenerator(ServiceGenerator.StackConfiguration StackConfiguration)
		{
			Configuration = StackConfiguration;
            ProjectName = StackConfiguration.projectname;
            ClassName = StackConfiguration.classname;
		}
		protected void Log(string msg) 
		{
			if (OnLogOutput != null) OnLogOutput(this,msg);
		}
		protected string CallingConvention
		{
			get
			{
				string RetVal = "";
				switch(Configuration.callconvention)
				{
					case ServiceGenerator.CALLINGCONVENTION.STDCALL:
						RetVal = "_stdcall "; break;
					case ServiceGenerator.CALLINGCONVENTION.FASTCALL:
						RetVal = "_fastcall "; break;
				}
				return(RetVal);
			}
		}
		protected string Indent
		{
			get
			{
				string RetVal = "";

				switch(Configuration.indent)
				{
					case ServiceGenerator.INDENTATION.TAB:
						RetVal = "\t"; 
						break;
					case ServiceGenerator.INDENTATION.ONESPACE:
						RetVal = " ";
						break;
					case ServiceGenerator.INDENTATION.TWOSPACES:
						RetVal = "  ";
						break;
					case ServiceGenerator.INDENTATION.THREESPACES:
						RetVal = "   ";
						break;
					case ServiceGenerator.INDENTATION.FOURSPACES:
						RetVal = "    "; 
						break;
					case ServiceGenerator.INDENTATION.FIVESPACES:
						RetVal = "     "; 
						break;
					case ServiceGenerator.INDENTATION.SIXSPACES:
						RetVal = "      "; 
						break;
				}
				return(RetVal);
			}
		}
		protected string FixPrefix_DeviceService(UPnPDevice device,string WS)
		{
			foreach(UPnPDevice ed in device.EmbeddedDevices)
			{
				WS = FixPrefix_DeviceService(ed,WS);
			}
			WS = WS.Replace(device.DeviceURN,"{{{"+device.DeviceURN.GetHashCode()+"}}}");
			foreach(UPnPService s in device.Services)
			{
				WS = WS.Replace(s.ServiceURN,"{{{"+s.ServiceURN.GetHashCode()+"}}}");
			}
			return(WS);
		}
		protected string FixPrefix2_DeviceService(UPnPDevice device,string WS)
		{
			foreach(UPnPDevice ed in device.EmbeddedDevices)
			{
				WS = FixPrefix2_DeviceService(ed,WS);
			}
			WS = WS.Replace("{{{"+device.DeviceURN.GetHashCode()+"}}}",device.DeviceURN);
			foreach(UPnPService s in device.Services)
			{
				WS = WS.Replace("{{{"+s.ServiceURN.GetHashCode()+"}}}",s.ServiceURN);
			}
			return(WS);
		}
	}
}
