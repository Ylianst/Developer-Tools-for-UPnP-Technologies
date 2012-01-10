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
using System.Text;
using System.Collections;
using OpenSource.UPnP;
using OpenSource.Utilities;

namespace UPnPStackBuilder
{
    /// <summary>
    /// Summary description for CPDotNetGenerator.
    /// </summary>
    public class CPDotNetGenerator : CodeGenerator
    {
        private string nspace = "OpenSource.Sample";

        public Hashtable Settings = new Hashtable();
        public Hashtable ServiceNames;

        private static string cl = "\r\n";

        public CPDotNetGenerator(string ns, ServiceGenerator.StackConfiguration Config) : base(Config)
        {
            if (ns != "") nspace = ns;
        }

        public string CodeNewLine
        {
            get { return cl; }
            set { cl = value; }
        }

        public override bool Generate(UPnPDevice[] devices, DirectoryInfo outputDirectory)
        {
            bool ok = false;
            bool RetVal = false;
            foreach (UPnPDevice device in devices)
            {
                if (((ServiceGenerator.Configuration)device.User).ConfigType == ServiceGenerator.ConfigurationType.CONTROLPOINT)
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

                    RetVal = GenerateEx(device, outputDirectory, GetServiceNameTable(device));
                    if (!RetVal) { break; }
                }
            }
            return (ok ? RetVal : true);
        }


        protected bool GenerateEx(UPnPDevice device, DirectoryInfo outputDirectory, Hashtable serviceNames)
        {
            string sampleService = "Sample";
            StreamWriter writer;
            ServiceNames = serviceNames;
            DText dt = new DText();
            dt.ATTRMARK = ":";
            dt[0] = device.DeviceURN;

            // *** Generate Main Code
            Log("Writing main stack module...");

            IDictionaryEnumerator en = serviceNames.GetEnumerator();
            while (en.MoveNext())
            {
                string servicename = (string)en.Value;
                sampleService = servicename;
                UPnPService service = (UPnPService)en.Key;
                string servicexml = new UTF8Encoding().GetString(service.GetSCPDXml());
                Log("Generating service class for " + servicename);
                OpenSource.UPnP.ServiceGenerator.GenerateCP("Cp" + servicename, nspace, outputDirectory.FullName + "\\Cp" + servicename + ".cs", service.ServiceURN, servicexml);
            }

            CodeProcessor cs = new CodeProcessor(new StringBuilder(), true);
            StringBuilder cs2;

            cs.Comment("UPnP .NET Framework Control Point Stack, Discovery Module");
            cs.Comment(VersionString);
            cs.Append(cl);
            cs.Append("using System;" + cl);
            cs.Append("using System.Net;" + cl);
            cs.Append("using OpenSource.UPnP;" + cl);
            cs.Append("using OpenSource.Utilities;" + cl);
            cs.Append("using " + nspace + ";" + cl);
            cs.Append(cl);
            cs.Append("namespace " + nspace + cl);
            cs.Append("{" + cl);
            cs.Append("	/// <summary>" + cl);
            cs.Append("	/// Summary description for " + dt[4] + "Discovery." + cl);
            cs.Append("	/// </summary>" + cl);
            cs.Append("	class " + dt[4] + "Discovery" + cl);
            cs.Append("	{" + cl);
            cs.Append("		private UPnPSmartControlPoint scp;" + cl);
            cs.Append("		private WeakEvent AddEvent = new WeakEvent();" + cl);
            cs.Append("		private WeakEvent RemoveEvent = new WeakEvent();" + cl);
            cs.Append(cl);
            cs.Append("		public delegate void DiscoveryHandler(" + dt[4] + "Discovery sender, UPnPDevice dev);" + cl);
            cs.Append("		public event DiscoveryHandler OnAddedDevice" + cl);
            cs.Append("		{" + cl);
            cs.Append("			add" + cl);
            cs.Append("			{" + cl);
            cs.Append("				AddEvent.Register(value);" + cl);
            cs.Append("			}" + cl);
            cs.Append("			remove" + cl);
            cs.Append("			{" + cl);
            cs.Append("				AddEvent.UnRegister(value);" + cl);
            cs.Append("			}" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public event DiscoveryHandler OnRemovedDevice" + cl);
            cs.Append("		{" + cl);
            cs.Append("			add" + cl);
            cs.Append("			{" + cl);
            cs.Append("				RemoveEvent.Register(value);" + cl);
            cs.Append("			}" + cl);
            cs.Append("			remove" + cl);
            cs.Append("			{" + cl);
            cs.Append("				RemoveEvent.UnRegister(value);" + cl);
            cs.Append("			}" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public " + dt[4] + "Discovery()" + cl);
            cs.Append("		{" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public void Start()" + cl);
            cs.Append("		{" + cl);
            cs.Append("			scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(OnAddSink), null ,\"" + device.DeviceURN + "\");" + cl);
            cs.Append("			scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(OnRemoveSink);" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public void ReScan()" + cl);
            cs.Append("		{" + cl);
            cs.Append("			scp.Rescan();" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public void UnicastSearch(IPAddress address)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			scp.UnicastSearch(address);" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		public void ForceDisposeDevice(UPnPDevice dev)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			if (dev.ParentDevice != null)" + cl);
            cs.Append("			{" + cl);
            cs.Append("				ForceDisposeDevice(dev.ParentDevice);" + cl);
            cs.Append("			}" + cl);
            cs.Append("			else" + cl);
            cs.Append("			{" + cl);
            cs.Append("				scp.ForceDisposeDevice(dev);" + cl);
            cs.Append("			}" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		private void OnAddSink(UPnPSmartControlPoint sender, UPnPDevice d)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			AddEvent.Fire(this, d);" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);
            cs.Append("		private void OnRemoveSink(UPnPSmartControlPoint sender, UPnPDevice d)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			RemoveEvent.Fire(this, d);" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("}" + cl);
            cs.Append(cl);

            writer = File.CreateText(outputDirectory.FullName + "\\" + dt[4] + "Discovery.cs");
            writer.Write(cs.ToString());
            writer.Close();

            cs = new CodeProcessor(new StringBuilder(), true);
            cs.Comment("UPnP .NET Framework Control Point Stack, Core Module");
            cs.Comment(VersionString);
            cs.Append(cl);
            cs.Append("using System;" + cl);
            cs.Append("using OpenSource.UPnP;" + cl);
            cs.Append("using " + nspace + ";" + cl);
            cs.Append(cl);
            cs.Append("namespace " + nspace + cl);
            cs.Append("{" + cl);
            cs.Append("	/// <summary>" + cl);
            cs.Append("	/// Summary description for Main." + cl);
            cs.Append("	/// </summary>" + cl);
            cs.Append("	class SampleDeviceMain" + cl);
            cs.Append("	{" + cl);
            cs.Append("		/// <summary>" + cl);
            cs.Append("		/// The main entry point for the application." + cl);
            cs.Append("		/// </summary>" + cl);
            cs.Append("		[STAThread]" + cl);
            cs.Append("		static void Main(string[] args)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			System.Console.WriteLine(\"UPnP .NET Framework Stack\");" + cl);
            cs.Append("			System.Console.WriteLine(\"StackBuilder Build#" + this.VersionString + "\");" + cl);
            cs.Append("		" + cl);
            cs.Append("			" + dt[4] + "Discovery disco = new " + dt[4] + "Discovery();" + cl);
            cs.Append("			disco.OnAddedDevice += new " + dt[4] + "Discovery.DiscoveryHandler(AddSink);" + cl);
            cs.Append("			disco.OnRemovedDevice += new " + dt[4] + "Discovery.DiscoveryHandler(RemoveSink);" + cl);
            cs.Append("		" + cl);
            cs.Append("			System.Console.WriteLine(\"Press return to stop CP.\");" + cl);
            cs.Append("			disco.Start();" + cl);
            cs.Append("		" + cl);
            cs.Append("			System.Console.ReadLine();" + cl);
            cs.Append("		}" + cl);
            cs.Append("		" + cl);
            cs.Append("		private static void AddSink(" + dt[4] + "Discovery sender, UPnPDevice d)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			Console.WriteLine(\"Added Device: \" + d.FriendlyName);" + cl);
            cs.Append(cl);
            cs.Comment("To interace with a service, instantiate the appropriate wrapper class on the appropriate service");
            cs.Comment("Traverse the device heirarchy to the correct device, and invoke 'GetServices', passing in the static field 'SERVICE_NAME'");
            cs.Comment("of the appropriate wrapper class. This method returns an array of all services with this service type. For most purposes,");
            cs.Comment("there will only be one service, in which case you can use array index 0.");
            cs.Comment("Save a reference to this instance of the wrapper class for later use.");
            cs.Append("			//Cp" + sampleService + " " + sampleService + " = new Cp" + sampleService + "(d.GetServices(Cp" + sampleService + ".SERVICE_NAME)[0]);" + cl);
            cs.Append(cl);
            cs.Comment("To subscribe to Events, call the '_subscribe' method of the wrapper class. The only parameter is");
            cs.Comment("the duration of the event. A good value is 300 seconds.");
            cs.Append("			//" + sampleService + "._subscribe(300);" + cl);
            cs.Append(cl);
            cs.Comment("The wrapper class exposes all the evented state variables through events in the form 'OnStateVariable_xx', where xx is the variable name.");
            cs.Append(cl);
            cs.Comment("The wrapper class exposes methods in two formats. Asyncronous and Syncronous. The Async method calls are exposed simply");
            cs.Comment("by the name of the method. The Syncronous version is the same, except with the word, 'Sync_' prepended to the name.");
            cs.Comment("Asyncronous responses to th async method calls are dispatched through the event in the format, 'OnResult_x' where x is the method name.");
            cs.Append(cl);
            cs.Comment("Note: All arguments are automatically type checked. Allowed Values are abstracted through enumerations, that are defined in the wrapper class.");
            cs.Comment("To access the list of allowed values or ranges for a given device, refer to the property 'Values_XXX' for a list of the allowed values for a");
            cs.Comment("given state variable. Similarly, refer to the properties 'HasMaximum_XXX', 'HasMinimum_XXX', 'Maximum_XXX', and 'Minimum_XXX' where XXX is the variable name, for the Max/Min values.");
            cs.Append(cl);
            cs.Comment("To determine if a given service implements a particular StateVariable or Method, use the properties, 'HasStateVariableXXX' and 'HasActionXXX' where XXX is the method/variable name.");
            cs.Append("		}" + cl);
            cs.Append("		private static void RemoveSink(" + dt[4] + "Discovery sender, UPnPDevice d)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			Console.WriteLine(\"Removed Device: \" + d.FriendlyName);" + cl);
            cs.Append("		}" + cl);
            cs.Append(cl);

            cs.Append("	}" + cl);
            cs.Append("}" + cl);
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
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Key;

                cs2.Append("                <File" + cl);
                cs2.Append("                    RelPath = \"Cp" + (string)en.Value + ".cs\"" + cl);
                cs2.Append("                    SubType = \"Code\"" + cl);
                cs2.Append("                    BuildAction = \"Compile\"" + cl);
                cs2.Append("                />" + cl);
            }
            cs2.Append("                <File" + cl);
            cs2.Append("                    RelPath = \"" + dt[4] + "Discovery.cs\"" + cl);
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
            System.IO.File.Copy(AppStartPath + "\\UPnP.dll", outputDirectory.FullName + "\\UPnP.dll", true);

            Log("UPnP Stack Generation Complete.");

            return true;
        }


    }

}

