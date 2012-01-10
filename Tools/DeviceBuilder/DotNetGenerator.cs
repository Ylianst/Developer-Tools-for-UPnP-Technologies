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
using System.Net;
using System.Text;
using System.Collections;
using System.Net.Sockets;
using OpenSource.UPnP;

using System.Collections.Generic;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for EmbeddedCGenerator.
	/// </summary>
	public class DotNetGenerator : CodeGenerator
	{
		private string nspace = "OpenSource.Sample";

		public Hashtable Settings = new Hashtable();
		public ArrayList AllServices = new ArrayList();
		public Hashtable ServiceNames;
		public string StartupPath;

		private static string cl = "\r\n";
		public string CodeNewLine 
		{
			get {return cl;}
			set {cl = value;}
		}

		public DotNetGenerator(string ns,ServiceGenerator.StackConfiguration Config):base(Config)
		{
			if (ns!="")
			{
				nspace = ns;
			}
		}

		private void AddAllServices(UPnPDevice device) 
		{
			foreach (UPnPService s in device.Services) AllServices.Add(s);
			foreach (UPnPDevice d in device.EmbeddedDevices) AddAllServices(d);
		}

		int gendevnumber = 0;
		private void GenerateAddDevice(CodeProcessor cs, UPnPDevice device)
		{
			string dev = gendevnumber.ToString();
			if (gendevnumber == 0) dev = "";

			if (device.ParentDevice == null) 
			{
				cs.Append("device = UPnPDevice.CreateRootDevice(1800,1.0,\"\\\\\");"+cl);
			}
			else
			{
				cs.Append("UPnPDevice device"+dev+" = UPnPDevice.CreateEmbeddedDevice(1,System.Guid.NewGuid().ToString());"+cl);
			}


            //
            // Add Custom Tags
            //
            foreach (string ns in device.GetCustomFieldFromDescription_Namespaces())
            {
                foreach (KeyValuePair<string, string> tag in device.GetCustomFieldsFromDescription(ns))
                {
                    cs.Append("device" + dev + ".AddCustomFieldInDescription(\"" + tag.Key + "\", \"" + tag.Value + "\", \"" + ns + "\");" + cl);
                }
            }
            cs.Append(cl);
			cs.Append("device"+dev+".FriendlyName = \""+device.FriendlyName+"\";"+cl);
			if (device.Manufacturer != null) cs.Append("device"+dev+".Manufacturer = \""+device.Manufacturer+"\";"+cl);
			if (device.ManufacturerURL != null) cs.Append("device"+dev+".ManufacturerURL = \""+device.ManufacturerURL+"\";"+cl);
			if (device.ModelName != null) cs.Append("device"+dev+".ModelName = \""+device.ModelName+"\";"+cl);
			if (device.ModelDescription != null) cs.Append("device"+dev+".ModelDescription = \""+device.ModelDescription+"\";"+cl);
			if (device.ModelURL != null) cs.Append("device"+dev+".ModelURL = new Uri(\""+device.ModelURL.ToString()+"\");"+cl);
			if (device.ModelNumber != null) cs.Append("device"+dev+".ModelNumber = \""+device.ModelNumber+"\";"+cl);
			cs.Append("device"+dev+".HasPresentation = false;"+cl);
			cs.Append("device"+dev+".DeviceURN = \""+device.DeviceURN+"\";"+cl);

			foreach (UPnPService service in device.Services) 
			{
				string servicename = (string)ServiceNames[service];
				cs.Append(nspace+".Dv" + servicename + " " + servicename + " = new "+nspace+".Dv" + servicename + "();" + cl);

				//ConnectionManager.External_ConnectionComplete(new OpenSource.DeviceBuilder.ConnectionManager.Delegate_ConnectionComplete(ConnectionCompleteAction));
				foreach (UPnPAction action in service.Actions) 
				{
					cs.Append(servicename + ".External_" + action.Name + " = new "+nspace+".Dv" + servicename + ".Delegate_" + action.Name + "(" + servicename + "_" + action.Name + ");" + cl);
				}

				cs.Append("device"+dev+".AddService(" + servicename + ");" + cl);
			}

			gendevnumber++;

			foreach (UPnPDevice embeddeddevice in device.EmbeddedDevices)
			{
				int dn = gendevnumber;
				GenerateAddDevice(cs,embeddeddevice);
				cs.Append("device"+dev+".AddDevice(device" + dn + ");" + cl);
				gendevnumber++;
			}
			cs.Append(cl);

		}

		public override bool Generate(UPnPDevice[] devices, DirectoryInfo outputDirectory)
		{
			bool ok = false;
			bool RetVal = false;
			foreach(UPnPDevice device in devices)
			{
				if (((ServiceGenerator.Configuration)device.User).ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
				{
					ok = true;
					if (Configuration.UPNP_1dot1)
					{
						device.ArchitectureVersion = "1.1";
					}
					else
					{
						device.ArchitectureVersion = "1.0";
					}

					device.ClearCustomFieldsInDescription();
					((ServiceGenerator.Configuration)device.User).AddAllCustomFieldsToDevice(device);

					RetVal = GenerateEx(device,outputDirectory,GetServiceNameTable(device));
					if (!RetVal){break;}
				}
			}
			return(ok?RetVal:true);
		}
		protected bool GenerateEx(UPnPDevice device,DirectoryInfo outputDirectory, Hashtable serviceNames)
		{
			AllServices.Clear();
			AddAllServices(device);
			StreamWriter writer;
			ServiceNames = serviceNames;

			// *** Generate Main Code
			Log("Writing main stack module...");

			foreach (UPnPService service in AllServices) 
			{
				string servicename = (string)serviceNames[service];
				string servicexml = new UTF8Encoding().GetString(service.GetSCPDXml());
				Log("Generating service class for " + servicename);
				OpenSource.UPnP.ServiceGenerator.Generate("Dv"+servicename,nspace,outputDirectory.FullName + "\\Dv" + servicename + ".cs",service.ServiceID,service.ServiceURN,servicexml);
			}

			CodeProcessor cs = new CodeProcessor(new StringBuilder(),true);
			StringBuilder cs2;

			cs.Comment("UPnP .NET Framework Device Stack, Device Module");
			cs.Comment(VersionString);
			cs.Append(cl);
			cs.Append("using System;" + cl);
			cs.Append("using OpenSource.UPnP;" + cl);
			cs.Append("using "+nspace+";" + cl);
			cs.Append(cl);
			cs.Append("namespace "+nspace+cl);
			cs.Append("{"+cl);
			cs.Append("	/// <summary>"+cl);
			cs.Append("	/// Summary description for SampleDevice."+cl);
			cs.Append("	/// </summary>"+cl);
			cs.Append("	class SampleDevice"+cl);
			cs.Append("	{"+cl);
			cs.Append(" private UPnPDevice device;" + cl);
			cs.Append(cl);
			cs.Append("	public SampleDevice()"+cl);
			cs.Append("	{"+cl);
			gendevnumber = 0;
			GenerateAddDevice(cs,device);

			cs.Comment("Setting the initial value of evented variables");
			foreach (UPnPService service in AllServices) 
			{
				string servicename = (string)ServiceNames[service];
				foreach (UPnPStateVariable variable in service.GetStateVariables()) 
				{
					if (variable.SendEvent == true) 
					{
						cs.Append(servicename + ".Evented_" + variable.Name + " = " + this.ToSampleValue(variable.GetNetType().ToString()) + ";" + cl);
					}
				}
			}

			cs.Append("	}"+cl);
			cs.Append(cl);

			cs.Append("	public void Start()"+cl);
			cs.Append("	{"+cl);
			cs.Append("		device.StartDevice();"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);

			cs.Append("	public void Stop()"+cl);
			cs.Append("	{"+cl);
			cs.Append("		device.StopDevice();"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);

			foreach (UPnPService service in AllServices) 
			{
				string servicename = (string)serviceNames[service];

				// Build MethodDelegates
				foreach(UPnPAction action in service.Actions)
				{
					cs.Append("public ");
					if (action.HasReturnValue==false)
					{
						cs.Append("void ");
					}
					else
					{
						cs.Append(action.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
					}
					cs.Append(servicename + "_" + action.Name + "(");
					UPnPArgument[] Args = action.ArgumentList;
					for(int i=0;i<Args.Length;++i)
					{
						UPnPArgument arg = Args[i];
						if (arg.IsReturnValue==false)
						{
							if (arg.Direction=="out")
							{
								cs.Append("out ");
							}
							if (arg.RelatedStateVar.AllowedStringValues==null)
							{
								cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
							}
							else
							{
								cs.Append("Dv"+servicename + ".Enum_" + arg.RelatedStateVar.Name + " ");
							}
							cs.Append(arg.Name);
							if (i<Args.Length-1)
							{
								cs.Append(", ");
							}
						}
					}
					cs.Append(")" + cl);
					cs.Append("{" + cl);

					foreach (UPnPArgument arg in action.Arguments) 
					{
						if (arg.Direction == "out" && arg.IsReturnValue==false) 
						{
							if (arg.RelatedStateVar.AllowedStringValues==null)
							{
								cs.Append(arg.Name + " = " + ToSampleValue(arg.RelatedStateVar.GetNetType().ToString()) + ";" + cl);
							}
							else 
							{
								string t = arg.RelatedStateVar.AllowedStringValues[0].ToUpper();
								t = t.Replace("-","_");
								t = t.Replace("+","_");
								t = t.Replace(" ","_");
								t = t.Replace(":","_");
								if (IsNumeric(t[0])==true) t = "_" + t;
								cs.Append(arg.Name + " = Dv" + servicename + ".Enum_" + arg.RelatedStateVar.Name + "." + t + ";" + cl);
							}
						}
					}

					cs.Append("Console.WriteLine(\"" + servicename + "_" + action.Name + "(\"");
					foreach (UPnPArgument arg in action.ArgumentList)
					{
						if (arg.Direction == "in") 
						{
							cs.Append(" + " + arg.Name + ".ToString()");
						}
					}
					cs.Append(" + \")\");" + cl);

					if (action.GetRetArg() != null)
					{
						cs.Append(cl);
						cs.Append("return " + ToSampleValue(action.GetRetArg().RelatedStateVar.GetNetType().ToString()) + ";" + cl);
					}

					cs.Append("}" + cl);
					cs.Append(cl);
				}

			}

			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);

			writer = File.CreateText(outputDirectory.FullName + "\\SampleDevice.cs");
			writer.Write(cs.ToString());
			writer.Close();

			cs = new CodeProcessor(new StringBuilder(),true);
			cs.Comment("UPnP .NET Framework Device Stack, Core Module");
			cs.Comment(VersionString);
			cs.Append(cl);
			cs.Append("using System;" + cl);
			cs.Append("using OpenSource.UPnP;" + cl);
			cs.Append("using "+nspace+";" + cl);
			cs.Append(cl);
			cs.Append("namespace "+nspace+cl);
			cs.Append("{"+cl);
			cs.Append("	/// <summary>"+cl);
			cs.Append("	/// Summary description for Main."+cl);
			cs.Append("	/// </summary>"+cl);
			cs.Append("	class SampleDeviceMain"+cl);
			cs.Append("	{"+cl);

			cs.Append("		/// <summary>"+cl);
			cs.Append("		/// The main entry point for the application."+cl);
			cs.Append("		/// </summary>"+cl);
			cs.Append("		[STAThread]"+cl);
			cs.Append("		static void Main(string[] args)"+cl);
			cs.Append("		{"+cl);

			cs.Comment("Starting UPnP Device");
			cs.Append("		System.Console.WriteLine(\"UPnP .NET Framework Stack\");"+cl);
			cs.Append("		System.Console.WriteLine(\""+VersionString+"\");"+cl);
			cs.Append("		SampleDevice device = new SampleDevice();"+cl);
			cs.Append("		device.Start();"+cl);
			cs.Append("		System.Console.WriteLine(\"Press return to stop device.\");"+cl);
			cs.Append("		System.Console.ReadLine();"+cl);
			cs.Append("		device.Stop();"+cl);
			cs.Append("     }"+cl);
			cs.Append(cl);

			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);

			writer = File.CreateText(outputDirectory.FullName + "\\Main.cs");
			writer.Write(cs.ToString());
			writer.Close();


			Log("Generating Visual Studio 7 Solution");
			cs2 = new StringBuilder();
			cs2.Append("Microsoft Visual Studio Solution File, Format Version 7.00" + cl);
			cs2.Append("Project(\"{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}\") = \"SampleDevice\", \"SampleDevice.csproj\", \"{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}\"" + cl);
			cs2.Append("EndProject" + cl);
			cs2.Append("Global" + cl);
			cs2.Append("	GlobalSection(SolutionConfiguration) = preSolution" + cl);
			cs2.Append("		ConfigName.0 = Debug" + cl);
			cs2.Append("		ConfigName.1 = Release" + cl);
			cs2.Append("	EndGlobalSection" + cl);
			cs2.Append("	GlobalSection(ProjectDependencies) = postSolution" + cl);
			cs2.Append("	EndGlobalSection" + cl);
			cs2.Append("	GlobalSection(ProjectConfiguration) = postSolution" + cl);
			cs2.Append("		{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}.Debug.ActiveCfg = Debug|.NET" + cl);
			cs2.Append("		{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}.Debug.Build.0 = Debug|.NET" + cl);
			cs2.Append("		{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}.Release.ActiveCfg = Release|.NET" + cl);
			cs2.Append("		{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}.Release.Build.0 = Release|.NET" + cl);
			cs2.Append("	EndGlobalSection" + cl);
			cs2.Append("	GlobalSection(ExtensibilityGlobals) = postSolution" + cl);
			cs2.Append("	EndGlobalSection" + cl);
			cs2.Append("	GlobalSection(ExtensibilityAddIns) = postSolution" + cl);
			cs2.Append("	EndGlobalSection" + cl);
			cs2.Append("EndGlobal" + cl);
			writer = File.CreateText(outputDirectory.FullName + "\\SampleDevice.sln");
			writer.Write(cs2.ToString());
			writer.Close();

			Log("Generating Assembly Info");
			cs2 = new StringBuilder();
			cs2.Append("using System.Reflection;" + cl);
			cs2.Append("using System.Runtime.CompilerServices;" + cl);
			cs2.Append(cl);
			cs2.Append("[assembly: AssemblyTitle(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyDescription(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyConfiguration(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyCompany(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyProduct(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyCopyright(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyTrademark(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyCulture(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyVersion(\"1.0.*\")]" + cl);
			cs2.Append("[assembly: AssemblyDelaySign(false)]" + cl);
			cs2.Append("[assembly: AssemblyKeyFile(\"\")]" + cl);
			cs2.Append("[assembly: AssemblyKeyName(\"\")]" + cl);
			writer = File.CreateText(outputDirectory.FullName + "\\AssemblyInfo.cs");
			writer.Write(cs2.ToString());
			writer.Close();

			Log("Generating Visual Studio 7 Project");
			cs2 = new StringBuilder();
			cs2.Append("<VisualStudioProject>" + cl);
			cs2.Append("    <CSHARP" + cl);
			cs2.Append("        ProjectType = \"Local\"" + cl);
			cs2.Append("        ProductVersion = \"7.0.9466\"" + cl);
			cs2.Append("        SchemaVersion = \"1.0\"" + cl);
			cs2.Append("        ProjectGuid = \"{FE5FA3F9-E2EA-40BE-8CF4-27F33CF6454E}\"" + cl);
			cs2.Append("    >" + cl);
			cs2.Append("        <Build>" + cl);
			cs2.Append("            <Settings" + cl);
			cs2.Append("                ApplicationIcon = \"\"" + cl);
			cs2.Append("                AssemblyKeyContainerName = \"\"" + cl);
			cs2.Append("                AssemblyName = \"SampleDevice\"" + cl);
			cs2.Append("                AssemblyOriginatorKeyFile = \"\"" + cl);
			cs2.Append("                DefaultClientScript = \"JScript\"" + cl);
			cs2.Append("                DefaultHTMLPageLayout = \"Grid\"" + cl);
			cs2.Append("                DefaultTargetSchema = \"IE50\"" + cl);
			cs2.Append("                DelaySign = \"false\"" + cl);
			cs2.Append("                OutputType = \"Exe\"" + cl);
			cs2.Append("                RootNamespace = \"SampleDevice\"" + cl);
			cs2.Append("                StartupObject = \"\"" + cl);
			cs2.Append("            >" + cl);
			cs2.Append("                <Config" + cl);
			cs2.Append("                    Name = \"Debug\"" + cl);
			cs2.Append("                    AllowUnsafeBlocks = \"false\"" + cl);
			cs2.Append("                    BaseAddress = \"285212672\"" + cl);
			cs2.Append("                    CheckForOverflowUnderflow = \"false\"" + cl);
			cs2.Append("                    ConfigurationOverrideFile = \"\"" + cl);
			cs2.Append("                    DefineConstants = \"DEBUG;TRACE\"" + cl);
			cs2.Append("                    DocumentationFile = \"\"" + cl);
			cs2.Append("                    DebugSymbols = \"true\"" + cl);
			cs2.Append("                    FileAlignment = \"4096\"" + cl);
			cs2.Append("                    IncrementalBuild = \"true\"" + cl);
			cs2.Append("                    Optimize = \"false\"" + cl);
			cs2.Append("                    OutputPath = \"bin\\Debug\\\"" + cl);
			cs2.Append("                    RegisterForComInterop = \"false\"" + cl);
			cs2.Append("                    RemoveIntegerChecks = \"false\"" + cl);
			cs2.Append("                    TreatWarningsAsErrors = \"false\"" + cl);
			cs2.Append("                    WarningLevel = \"4\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("                <Config" + cl);
			cs2.Append("                    Name = \"Release\"" + cl);
			cs2.Append("                    AllowUnsafeBlocks = \"false\"" + cl);
			cs2.Append("                    BaseAddress = \"285212672\"" + cl);
			cs2.Append("                    CheckForOverflowUnderflow = \"false\"" + cl);
			cs2.Append("                    ConfigurationOverrideFile = \"\"" + cl);
			cs2.Append("                    DefineConstants = \"TRACE\"" + cl);
			cs2.Append("                    DocumentationFile = \"\"" + cl);
			cs2.Append("                    DebugSymbols = \"false\"" + cl);
			cs2.Append("                    FileAlignment = \"4096\"" + cl);
			cs2.Append("                    IncrementalBuild = \"false\"" + cl);
			cs2.Append("                    Optimize = \"true\"" + cl);
			cs2.Append("                    OutputPath = \"bin\\Release\\\"" + cl);
			cs2.Append("                    RegisterForComInterop = \"false\"" + cl);
			cs2.Append("                    RemoveIntegerChecks = \"false\"" + cl);
			cs2.Append("                    TreatWarningsAsErrors = \"false\"" + cl);
			cs2.Append("                    WarningLevel = \"4\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("            </Settings>" + cl);
			cs2.Append("            <References>" + cl);
			cs2.Append("                <Reference" + cl);
			cs2.Append("                    Name = \"System\"" + cl);
			cs2.Append("                    AssemblyName = \"System\"" + cl);
			cs2.Append("                    HintPath = \"..\\..\\WINDOWS\\Microsoft.NET\\Framework\\v1.0.3705\\System.dll\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("                <Reference" + cl);
			cs2.Append("                    Name = \"System.Data\"" + cl);
			cs2.Append("                    AssemblyName = \"System.Data\"" + cl);
			cs2.Append("                    HintPath = \"..\\..\\WINDOWS\\Microsoft.NET\\Framework\\v1.0.3705\\System.Data.dll\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("                <Reference" + cl);
			cs2.Append("                    Name = \"System.XML\"" + cl);
			cs2.Append("                    AssemblyName = \"System.Xml\"" + cl);
			cs2.Append("                    HintPath = \"..\\..\\WINDOWS\\Microsoft.NET\\Framework\\v1.0.3705\\System.XML.dll\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("                <Reference" + cl);
			cs2.Append("                    Name = \"UPnP\"" + cl);
			cs2.Append("                    AssemblyName = \"UPnP\"" + cl);
			cs2.Append("                    HintPath = \"UPnP.dll\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("            </References>" + cl);
			cs2.Append("        </Build>" + cl);
			cs2.Append("        <Files>" + cl);
			cs2.Append("            <Include>" + cl);
			cs2.Append("                <File" + cl);
			cs2.Append("                    RelPath = \"AssemblyInfo.cs\"" + cl);
			cs2.Append("                    SubType = \"Code\"" + cl);
			cs2.Append("                    BuildAction = \"Compile\"" + cl);
			cs2.Append("                />" + cl);
			foreach (UPnPService service in AllServices) 
			{
				cs2.Append("                <File" + cl);
				cs2.Append("                    RelPath = \"Dv"+(string)serviceNames[service]+".cs\"" + cl);
				cs2.Append("                    SubType = \"Code\"" + cl);
				cs2.Append("                    BuildAction = \"Compile\"" + cl);
				cs2.Append("                />" + cl);
			}
			cs2.Append("                <File" + cl);
			cs2.Append("                    RelPath = \"SampleDevice.cs\"" + cl);
			cs2.Append("                    SubType = \"Code\"" + cl);
			cs2.Append("                    BuildAction = \"Compile\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("                <File" + cl);
			cs2.Append("                    RelPath = \"Main.cs\"" + cl);
			cs2.Append("                    SubType = \"Code\"" + cl);
			cs2.Append("                    BuildAction = \"Compile\"" + cl);
			cs2.Append("                />" + cl);
			cs2.Append("            </Include>" + cl);
			cs2.Append("        </Files>" + cl);
			cs2.Append("    </CSHARP>" + cl);
			cs2.Append("</VisualStudioProject>" + cl);
			writer = File.CreateText(outputDirectory.FullName + "\\SampleDevice.csproj");
			writer.Write(cs2.ToString());
			writer.Close();

			Log("Copying UPnP.dll.");
			System.IO.File.Copy(AppStartPath + "\\UPnP.dll",outputDirectory.FullName + "\\UPnP.dll",true);

			Log("UPnP Stack Generation Complete.");

			return true;
		}

		public string ToSampleValue(string t) 
		{
			switch (t) 
			{
				case "System.Boolean":
					return "false";
				case "System.Byte[]":
				case "System.String":
					return "\"Sample String\"";
				case "System.Uri":
					return "\"http://opentools.homeip.net\"";
				case "System.Byte":
					return "0";
				case "System.UInt16":
					return "0";
				case "System.UInt32":
					return "0";
				case "System.Char":
				case "System.SByte":
					return "0";
				case "System.Int16":
					return "0";
				case "System.Int32":
					return "0";
				case "System.Single":
				case "System.Double":
					return "0";
				case "System.DateTime":
					return("DateTime.Now");
				default:
					return "NULL";
			}
		}
	
		private static bool IsNumeric(char c)
		{
			int x = (int)c;
			if ((x>=48)&&(x<=57))
			{
				return(true);
			}
			else
			{
				return(false);
			}
		}

	}
}
