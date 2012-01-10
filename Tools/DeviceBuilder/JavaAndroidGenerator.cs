using System;
using System.Collections.Generic;
using System.Text;

using System.Collections;
using System.IO;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
    public class JavaAndroidGenerator : CodeGenerator
    {
        private string nspace = "OpenSource.Sample";
        private string project = "SampleApplication";
        private string cl = "\r\n";
        public Hashtable Settings = new Hashtable();
        public ArrayList AllServices = new ArrayList();
        public Hashtable ServiceNames;

        private int eDeviceIndex;
        private Dictionary<UPnPDevice, int> embeddedTable = new Dictionary<UPnPDevice, int>();

        public JavaAndroidGenerator(ServiceGenerator.StackConfiguration Config) : base(Config)
		{
            nspace = ClassName;
		}
        private void AddAllServices(UPnPDevice device)
        {
            foreach (UPnPService s in device.Services) AllServices.Add(s);
            foreach (UPnPDevice d in device.EmbeddedDevices) AddAllServices(d);
        }
        public override bool Generate(UPnPDevice[] devices, DirectoryInfo outputDirectory)
        {
            bool ok = false;
            bool RetVal = false;

            List<UPnPDevice> dvList = new List<UPnPDevice>();
            List<UPnPDevice> cpList = new List<UPnPDevice>();

            foreach (UPnPDevice device in devices)
            {
                if (((ServiceGenerator.Configuration)device.User).ConfigType == ServiceGenerator.ConfigurationType.DEVICE)
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

                    dvList.Add(device);

                    //RetVal = GenerateEx(device, outputDirectory, GetServiceNameTable(device));
                    //if (!RetVal) { break; }
                }
                else if (((ServiceGenerator.Configuration)device.User).ConfigType == ServiceGenerator.ConfigurationType.CONTROLPOINT)
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

                    cpList.Add(device);

                    //RetVal = GenerateCPEx(device, outputDirectory, GetServiceNameTable(device));
                    //if (!RetVal) { break; }
                }
            }

            RetVal = GenerateEx(dvList.ToArray(), cpList.ToArray(), outputDirectory);

            return (ok ? RetVal : true);
        }
        private string GetDeviceShortName(UPnPDevice device)
        {
            string[] ret = device.DeviceURN.Split(new string[] { ":" }, StringSplitOptions.None);
            return (ret[ret.Length - 2]);
        }
        private void FriendlyNameInitForEmbeddedDevices(UPnPDevice device, CodeProcessor cs)
        {
            ++eDeviceIndex;
            cs.Append(", String uniqueIdentifier" + eDeviceIndex.ToString() + ", String friendlyName" + eDeviceIndex.ToString());
            foreach (UPnPDevice eDevice in device.EmbeddedDevices)
            {
                FriendlyNameInitForEmbeddedDevices(eDevice, cs);
            }
        }
        private void PrivateEmbeddedDeviceDeclaration(UPnPDevice eDevice, CodeProcessor cs)
        {
            ++eDeviceIndex;
            embeddedTable.Add(eDevice, eDeviceIndex);
            cs.Append("    private UPnPDevice mDevice" + eDeviceIndex.ToString() + ";" + cl);
            
            foreach (UPnPDevice d in eDevice.EmbeddedDevices)
            {
                PrivateEmbeddedDeviceDeclaration(d, cs);
            }
        }
        private void ProcessRootDeviceConstructor(UPnPDevice eDevice, StringBuilder init)
        {
            ++eDeviceIndex;
            init.Append(", UUID.randomUUID().toString(), \"" + eDevice.FriendlyName + "\"");

            foreach (UPnPDevice d2 in eDevice.EmbeddedDevices)
            {
                ProcessRootDeviceConstructor(d2, init);
            }
        }
        protected bool GenerateEx(UPnPDevice[] dvDevices, UPnPDevice[] cpDevices, DirectoryInfo outputDirectory)
        {
            StreamWriter writer;
            FileStream fs;
            CodeProcessor cs;
            StringBuilder sb;

            // *** Generate Main Code
            Log("Writing main stack module...");

            #region Create the Directory Structure
            outputDirectory.CreateSubdirectory("assets");
            outputDirectory.CreateSubdirectory("bin");
            #region "gen"
            DirectoryInfo genDir = outputDirectory.CreateSubdirectory("gen");
            string[] pName = nspace.Split(new string[] { "." }, StringSplitOptions.None);
            foreach (string pNameEx in pName)
            {
                genDir = genDir.CreateSubdirectory(pNameEx);
            }
            writer = File.CreateText(genDir.FullName + "\\R.java");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\gen\\R.java").Replace("{{{PACKAGE}}}", nspace));
            writer.Close();
            #endregion
            #region "res"
            DirectoryInfo resDir = outputDirectory.CreateSubdirectory("res");
            byte[] dpi = SourceCodeRepository.ReadFileStoreBin("Android\\res\\drawable-hdpi\\icon.png");
            fs = File.Create(resDir.CreateSubdirectory("drawable-hdpi").FullName + "\\icon.png", dpi.Length);
            fs.Write(dpi, 0, dpi.Length);
            fs.Close();

            dpi = SourceCodeRepository.ReadFileStoreBin("Android\\res\\drawable-ldpi\\icon.png");
            fs = File.Create(resDir.CreateSubdirectory("drawable-ldpi").FullName + "\\icon.png", dpi.Length);
            fs.Write(dpi, 0, dpi.Length);
            fs.Close();

            dpi = SourceCodeRepository.ReadFileStoreBin("Android\\res\\drawable-mdpi\\icon.png");
            fs = File.Create(resDir.CreateSubdirectory("drawable-mdpi").FullName + "\\icon.png", dpi.Length);
            fs.Write(dpi, 0, dpi.Length);
            fs.Close();

            writer = File.CreateText(resDir.CreateSubdirectory("layout").FullName + "\\main.xml");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\res\\layout\\main.xml"));
            writer.Close();

            writer = File.CreateText(resDir.CreateSubdirectory("values").FullName + "\\strings.xml");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\res\\values\\strings.xml").Replace("{{{PROJECTNAME}}}", ProjectName));
            writer.Close();
            #endregion
            #region "src"
            DirectoryInfo srcDir = outputDirectory.CreateSubdirectory("src");
            pName = nspace.Split(new string[] { "." }, StringSplitOptions.None);
            foreach (string pNameEx in pName)
            {
                srcDir = srcDir.CreateSubdirectory(pNameEx);
            }
            #endregion
            #region Project Files
            writer = File.CreateText(outputDirectory.FullName + "\\.classpath");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\.classpath"));
            writer.Close();

            writer = File.CreateText(outputDirectory.FullName + "\\.project");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\.project").Replace("{{{PROJECTNAME}}}", ProjectName));
            writer.Close();

            writer = File.CreateText(outputDirectory.FullName + "\\AndroidManifest.xml");
            writer.Write(
                SourceCodeRepository.ReadFileStore("Android\\AndroidManifest.xml").Replace("{{{PROJECTNAME}}}", ProjectName)
                .Replace("{{{PACKAGE}}}", ClassName));
            writer.Close();

            writer = File.CreateText(outputDirectory.FullName + "\\default.properties");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\default.properties"));
            writer.Close();

            writer = File.CreateText(outputDirectory.FullName + "\\proguard.cfg");
            writer.Write(SourceCodeRepository.ReadFileStore("Android\\proguard.cfg"));
            writer.Close();

            byte[] jar = SourceCodeRepository.ReadFileStoreBin("Android\\UPnPLibrary.jar");
            fs = File.Create(outputDirectory.FullName + "\\UPnPLibrary.jar", jar.Length);
            fs.Write(jar, 0, jar.Length);
            fs.Close();

            #endregion
            #endregion

            #region Device

            int dvNum = 0;
            StringBuilder dvDeclaration = new StringBuilder();
            StringBuilder dvInit = new StringBuilder();
            StringBuilder dvHandlers = new StringBuilder();
            StringBuilder dvStart = new StringBuilder();
            StringBuilder dvStop = new StringBuilder();

            foreach (UPnPDevice device in dvDevices)
            {
                ++dvNum;
                AllServices.Clear();
                AddAllServices(device);
                ServiceNames = GetServiceNameTable(device);

                #region Service Invocation Handlers
                cs = new CodeProcessor(new StringBuilder(), true);
                string serviceInvocationHandlers = "";
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];

                    //cs.Comment("UPnP Java/Android Device Stack, Remote Invocation Handler");
                    //cs.Comment(VersionString);
                    //cs.Append(cl);
                    //cs.Append("package " + nspace + ";" + cl);
                    //cs.Append(cl);
                    //cs.Append("import opentools.ILib.RefParameter;");
                    //cs.Append(cl);
                    cs.Append("public interface " + servicename + "_InvokeHandler" + cl);
                    cs.Append("{" + cl);
                    foreach (UPnPAction action in service.Actions)
                    {
                        cs.Append("   public ");
                        if (action.HasReturnValue)
                        {
                            cs.Append(GetJavaType(action.GetRetArg().RelatedStateVar));
                        }
                        else
                        {
                            cs.Append("void");
                        }
                        cs.Append(" " + action.Name + "(");
                        bool firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (!arg.IsReturnValue)
                            {
                                if (!firstArg)
                                {
                                    cs.Append(", ");
                                }
                                firstArg = false;

                                if (arg.Direction == "in")
                                {
                                    cs.Append(GetJavaType(arg.RelatedStateVar));
                                }
                                else
                                {
                                    cs.Append("RefParameter<");
                                    cs.Append(GetJavaType(arg.RelatedStateVar, true));
                                    cs.Append(">");
                                }
                                cs.Append(" ");
                                cs.Append(arg.Name);
                            }
                        }
                        cs.Append(");" + cl);
                    }
                    cs.Append("}" + cl);
                    //writer = File.CreateText(srcDir.FullName + "\\" + servicename + "_InvokeHandler.java");
                    //writer.Write(cs.ToString());
                    //writer.Close();

                    serviceInvocationHandlers = cs.ToString();
                }
                #endregion

                #region Build Named Inner Classes for Invocation Processing
                StringBuilder nic = new StringBuilder();
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];
                    foreach (UPnPAction action in service.Actions)
                    {
                        nic.Append("        private class " + servicename + "_" + action.Name + "_Dispatcher implements GenericRemoteInvokeHandler" + cl);
                        nic.Append("        {" + cl);
                        nic.Append("            public " + servicename + "_" + action.Name + "_Dispatcher()"+cl);
                        nic.Append("            {" + cl);
                        nic.Append("            }" + cl);
                        nic.Append(cl);
                        nic.Append("            @Override" + cl);
                        nic.Append("            public void OnGenericRemoteInvoke(UPnPAction sender, Attributes inParams, Attributes outParams)" + cl);
                        nic.Append("                   throws UPnPInvokeException" + cl);
                        nic.Append("            {" + cl);
                        nic.Append("                if(" + servicename + "_InvokeCallback != null)" + cl);
                        nic.Append("                {" + cl);

                        //Input Argument Conversion
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                nic.Append("                    " + GetJavaType(arg.RelatedStateVar) + " " + arg.Name + " = ");
                                nic.Append(DeSerializeVariable(arg.RelatedStateVar, "inParams.getValue(\"" + arg.Name + "\")"));
                                nic.Append(";" + cl);
                            }
                            if (arg.Direction == "out" && !arg.IsReturnValue)
                            {
                                nic.Append("                    " + GetJavaReferenceType(arg.RelatedStateVar) + " " + arg.Name + " = new " + GetJavaReferenceType(arg.RelatedStateVar) + "();" + cl);
                            }
                        }


                        //Dispatch
                        nic.Append(cl);
                        if (action.HasReturnValue)
                        {
                            nic.Append("                    ");
                            nic.Append(GetJavaType(action.GetRetArg().RelatedStateVar));
                            nic.Append(" " + action.GetRetArg().Name + " = ");
                            nic.Append(servicename + "_InvokeCallback." + action.Name + "(");
                        }
                        else
                        {
                            nic.Append("                    " + servicename + "_InvokeCallback." + action.Name + "(");
                        }
                        bool firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (!arg.IsReturnValue)
                            {
                                if (firstArg)
                                {
                                    nic.Append(arg.Name);
                                    firstArg = false;
                                }
                                else
                                {
                                    nic.Append(", ");
                                    nic.Append(arg.Name);
                                }
                            }
                        }
                        nic.Append(");" + cl);

                        //Convert the ReturnValue
                        if (action.HasReturnValue)
                        {
                            nic.Append("                    outParams.putValue(\"" + action.GetRetArg().Name + "\", " + SerializeVariable(action.GetRetArg()) + ");" + cl);
                        }

                        //Output Argument Conversion
                        nic.Append(cl);
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" && !arg.IsReturnValue)
                            {
                                nic.Append("                    outParams.putValue(\"" + arg.Name + "\", " + SerializeReferenceArgument(arg) + ");" + cl);
                            }
                        }

                        // Error case, when a handler wasn't set
                        nic.Append("                }" + cl);
                        nic.Append("                else" + cl);
                        nic.Append("                {" + cl);
                        nic.Append("                    throw(new UPnPInvokeException(501, \"" + action.Name + " Handler Not Specified\"));" + cl);
                        nic.Append("                }" + cl);
                        nic.Append("            }" + cl);
                        nic.Append("        }" + cl);
                    }
                }
                #endregion

                #region Build the Device Structure
                cs = new CodeProcessor(new StringBuilder(), true);
                cs.Comment("UPnP Java/Android Device Stack, Device Implementationr");
                cs.Comment(VersionString);
                cs.Append(cl);
                cs.Append("package " + nspace + ";" + cl);
                cs.Append(cl);
                cs.Append("import java.util.ArrayList;" + cl);
                cs.Append("import java.util.List;" + cl);
                cs.Append("import java.util.jar.Attributes;" + cl);
                cs.Append(cl);
                cs.Append("import opentools.ILib.RefParameter;" + cl);
                cs.Append("import opentools.upnp.ArgumentDirection;" + cl);
                cs.Append("import opentools.upnp.GenericRemoteInvokeHandler;" + cl);
                cs.Append("import opentools.upnp.UPnPAction;" + cl);
                cs.Append("import opentools.upnp.UPnPArgument;" + cl);
                cs.Append("import opentools.upnp.UPnPDevice;" + cl);
                cs.Append("import opentools.upnp.UPnPInvokeException;" + cl);
                cs.Append("import opentools.upnp.UPnPService;" + cl);
                cs.Append("import opentools.upnp.UPnPStateVariable;" + cl);
                cs.Append(cl);
                cs.Append("public class RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + cl);
                cs.Append("{" + cl);
                cs.Append("    public UPnPDevice rootDevice;" + cl);
                foreach (UPnPService service in AllServices)
                {
                    string serviceName = (string)ServiceNames[service];
                    cs.Append("    public " + serviceName + "_InvokeHandler " + serviceName + "_InvokeCallback;" + cl);
                }
                cs.Append(cl);

                eDeviceIndex = 0;
                PrivateEmbeddedDeviceDeclaration(device, cs);

                cs.Append(cl);
                cs.Append("    public RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + "(String uniqueIdentifier, String friendlyName");

                eDeviceIndex = 1;
                foreach (UPnPDevice eDevice in device.EmbeddedDevices)
                {
                    FriendlyNameInitForEmbeddedDevices(eDevice, cs);
                }

                cs.Append(")" + cl);
                cs.Append("    {" + cl);
                cs.Append("        UPnPDevice device;" + cl);
                if (device.EmbeddedDevices.Length > 0)
                {
                    cs.Append("        UPnPDevice tmpDevice;" + cl);
                }
                cs.Append("        rootDevice = device = new UPnPDevice(uniqueIdentifier, friendlyName, \"" + device.DeviceURN + "\");" + cl);
                cs.Append("        List<UPnPArgument> inParams = new ArrayList<UPnPArgument>();" + cl);
                cs.Append(cl);
                cs.Append("        UPnPService service;" + cl);
                cs.Append("        UPnPStateVariable var;" + cl);
                cs.Append("        UPnPArgument arg;" + cl);
                cs.Append("        UPnPAction action;" + cl);
                eDeviceIndex = 1;

                ProcessDevice_BuildService(device, cs);

                cs.Append(cl);
                cs.Append("	}" + cl);

                #region Start/Stop Device
                cs.Append("	public void Start(int cacheTimeout, int port)" + cl);
                cs.Append("	{" + cl);
                cs.Append("		rootDevice.StartServer(cacheTimeout, port);" + cl);
                cs.Append("	}" + cl);
                cs.Append("	public void Stop()" + cl);
                cs.Append("	{" + cl);
                cs.Append("		rootDevice.StopServer();" + cl);
                cs.Append("	}" + cl);
                #endregion
                #region Evented State Variable Setters
                foreach (UPnPService s in AllServices)
                {
                    string serviceName = (string)ServiceNames[s];
                    foreach (UPnPStateVariable var in s.GetStateVariables())
                    {
                        if (var.SendEvent)
                        {
                            //
                            // Evented State Variable
                            //
                            cs.Append("	public void SetStateVariableValue_" + serviceName + "_" + var.Name + "(" + GetJavaType(var) + " " + var.Name + ")" + cl);
                            cs.Append("{" + cl);
                            cs.Append("     mDevice" + embeddedTable[s.ParentDevice].ToString());
                            cs.Append(".GetServiceByID(\"");
                            cs.Append(s.ServiceID);
                            cs.Append("\").getStateVariable(\"");
                            cs.Append(var.Name);
                            cs.Append("\").SetValue(");
                            cs.Append(SerializeVariable(var));
                            cs.Append(");" + cl);
                            cs.Append("}" + cl);
                        }
                    }
                }
                #endregion
                #region Write Inniver Class Definition
                cs.Append(nic.ToString());
                #endregion
                #region Add Invocation Handler Inner Class
                //
                // Add Service Invocation Handler Inner Class Definitions
                //
                cs.Append(serviceInvocationHandlers);
                cs.Append(cl);
                #endregion

                cs.Append("}" + cl);

                writer = File.CreateText(srcDir.FullName + "\\RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + ".java");
                writer.Write(cs.ToString());
                writer.Close();
                #endregion

                #region Sample App

                #region Declaration
                dvDeclaration.Append("  private RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + " root" + dvNum.ToString() + ";"+cl);
                #endregion
                #region Init
                StringBuilder cinit = new StringBuilder();
                cinit.Append("UUID.randomUUID().toString(), \"" + device.FriendlyName + "\"");
                eDeviceIndex = 1;
                foreach (UPnPDevice eDevice in device.EmbeddedDevices)
                {
                    ProcessRootDeviceConstructor(eDevice, cinit);
                }
                dvInit.Append("        root" + dvNum.ToString() + " = new RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + "(" + cinit.ToString() + ");" + cl);
                #endregion

                #region Sample Invocation Handlers

                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];
                    //dvHandlers.Append("        root" + dvNum.ToString() + "." + servicename + "_InvokeCallback = new " + servicename + "_InvokeHandler()" + cl);
                    dvHandlers.Append("        root" + dvNum.ToString() + "." + servicename + "_InvokeCallback = new RootDevice" + dvNum.ToString() + "_" + GetDeviceShortName(device) + "." + servicename + "_InvokeHandler()" + cl);             
                    dvHandlers.Append("        {" + cl);
                    foreach (UPnPAction action in service.Actions)
                    {
                        dvHandlers.Append("			    @Override" + cl);
                        dvHandlers.Append("			    public void " + action.Name + "(" + cl);
                        bool first = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (!first)
                            {
                                dvHandlers.Append("," + cl);
                            }
                            else
                            {
                                first = false;
                            }
                            if (arg.Direction == "in")
                            {
                                dvHandlers.Append("                     " + GetJavaType(arg.RelatedStateVar) + " " + arg.Name);
                            }
                            else if (arg.Direction == "out" && !arg.IsReturnValue)
                            {
                                dvHandlers.Append("                     " + GetJavaReferenceType(arg.RelatedStateVar) + " " + arg.Name);
                            }
                        }
                        dvHandlers.Append(")" + cl);
                        dvHandlers.Append("			    {" + cl);
                        bool hasOutArgs = false;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" && !arg.IsReturnValue)
                            {
                                hasOutArgs = true;
                                break;
                            }
                        }
                        if (hasOutArgs)
                        {
                            dvHandlers.Append("			        // These are only sample values... " + cl);
                        }
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" && !arg.IsReturnValue)
                            {
                                dvHandlers.Append("			        " + arg.Name + ".value = " + GetSampleValue(arg.RelatedStateVar, true) + ";" + cl);
                            }
                        }
                        if (action.HasReturnValue)
                        {
                            dvHandlers.Append("			        return(" + GetSampleValue(action.GetRetArg().RelatedStateVar) + ");" + cl);
                        }
                        dvHandlers.Append("			        //" + cl);
                        dvHandlers.Append("			        // You can return a UPnP Error code by throwing the following exception:" + cl);
                        dvHandlers.Append("			        //" + cl);
                        dvHandlers.Append("			        // throw (new UPnPInvokeException(statusCode, statusDescription));" + cl);
                        dvHandlers.Append("			        //" + cl);
                        dvHandlers.Append("			    }" + cl);
                    }
                    dvHandlers.Append("        };" + cl);
                }
                #endregion
                #region Sample Evented Variable State
                bool hasEventedVars = false;
                foreach (UPnPService service in AllServices)
                {
                    foreach (UPnPStateVariable var in service.GetStateVariables())
                    {
                        if (var.SendEvent)
                        {
                            hasEventedVars = true;
                            break;
                        }
                    }
                    if (hasEventedVars)
                    {
                        break;
                    }
                }
                if (hasEventedVars)
                {
                    dvHandlers.Append("        // The following SetStateVariableValue_XXX calls are examples only. Please initialize the values with appropriate values, then call them again whenever updated" + cl);
                }
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];
                    foreach (UPnPStateVariable var in service.GetStateVariables())
                    {
                        if (var.SendEvent)
                        {
                            dvHandlers.Append("        root" + dvNum.ToString() + ".SetStateVariableValue_" + servicename + "_" + var.Name + "(" + GetSampleValue(var) + ");" + cl);
                        }
                    }
                }
                #endregion

                #region Start/Stop
                dvStart.Append("                root" + dvNum.ToString() + ".Start(" + ((ServiceGenerator.Configuration)device.User).SSDPCycleTime.ToString() + "," + ((ServiceGenerator.Configuration)device.User).WebPort.ToString() + ");" + cl);
                dvStop.Append("                 root" + dvNum.ToString() + ".Stop();" + cl);
                #endregion

                #endregion

            }
            #endregion
            #region Control Point
            foreach (UPnPDevice device in cpDevices)
            {
                AllServices.Clear();
                AddAllServices(device);
                ServiceNames = GetServiceNameTable(device);

                #region Service
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];

                    #region Event Processor
                    string eventProcessor = "";
                    sb = new StringBuilder();
                    sb.Append("         if(EventCallback!=null)" + cl);
                    sb.Append("         {" + cl);
                    foreach (UPnPStateVariable var in service.GetStateVariables())
                    {
                        if (var.SendEvent)
                        {
                            sb.Append("             if (eventedParameters.containsKey(new java.util.jar.Attributes.Name(\"" + var.Name + "\")))" + cl);
                            sb.Append("             {" + cl);
                            sb.Append("                 EventCallback.On" + var.Name + "(" + DeSerializeVariable(var, "eventedParameters.getValue(\"" + var.Name + "\")") + ");" + cl);
                            sb.Append("             }" + cl);
                        }
                    }
                    sb.Append("         }" + cl);
                    eventProcessor = sb.ToString();
                    #endregion
                    #region Methods
                    sb = new StringBuilder();
                    string methods = "";

                    foreach (UPnPAction action in service.Actions)
                    {
                        sb.Append("    public actionHelper_" + action.Name + " action_" + action.Name + ";" + cl);
                    }

                    methods = sb.ToString();
                    #endregion
                    #region Inner Classes
                    sb = new StringBuilder();
                    string innerClasses = "";

                    foreach (UPnPAction action in service.Actions)
                    {
                        sb.Append(" class actionHelper_" + action.Name + cl);
                        sb.Append(" {" + cl);

                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            sb.Append("     public variableInfo_" + arg.Name + " arg_" + arg.Name + ";" + cl);
                        }
                        sb.Append("     public actionHelper_" + action.Name + "()" + cl);
                        sb.Append("     {"+cl);
                        sb.Append("         if(mService.isScpdLoadAttempted())" + cl);
                        sb.Append("         {" + cl);
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            sb.Append("             arg_" + arg.Name + " = new variableInfo_" + arg.Name + "();" + cl);
                        }
                        sb.Append("         }" + cl);
                        sb.Append("     }" + cl);
                        sb.Append("     public void refreshVariableInfo()" + cl);
                        sb.Append("     {" + cl);
                        sb.Append("         if(mService.isScpdLoadAttempted())" + cl);
                        sb.Append("         {" + cl);
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            sb.Append("             arg_" + arg.Name + " = new variableInfo_" + arg.Name + "();" + cl);
                        }
                        sb.Append("         }" + cl);
                        sb.Append("     }" + cl);
                        sb.Append(cl);

                        sb.Append("     public void Invoke" + "(");
                        bool firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                if (!firstArg)
                                {
                                    sb.Append(", ");
                                }
                                else
                                {
                                    firstArg = false;
                                }
                                sb.Append(GetJavaType(arg.RelatedStateVar) + " " + arg.Name);
                            }
                        }
                        if (!firstArg)
                        {
                            sb.Append(", ");
                        }
                        sb.Append("Object userState, InvokeHandler userCallback)" + cl);
                        sb.Append("     {" + cl);
                        sb.Append("         Attributes inParam = new Attributes();" + cl);
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                sb.Append("         inParam.putValue(\"" + arg.Name + "\"," + SerializeVariable(arg) + ");" + cl);
                            }
                        }
                        sb.Append("         mService.GenericInvoke(\"" + action.Name + "\", inParam, userCallback, userState, new GenericInvokeHandler()" + cl);
                        sb.Append("         {" + cl);
                        sb.Append("             @Override" + cl);
                        sb.Append("             public void OnGenericInvoke(String methodName,  int errorCode, Attributes parameters, Object userState, Object userState2) " + cl);
                        sb.Append("             {" + cl);
                        sb.Append("                 InvokeHandler Callback = (InvokeHandler)userState;" + cl);
                        sb.Append("                 if(errorCode!=0)" + cl);
                        sb.Append("                 {" + cl);
                        sb.Append("                     if(Callback!=null)" + cl);
                        sb.Append("                     {" + cl);
                        sb.Append("                         Callback.On" + action.Name + "(errorCode, ");
                        firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" || arg.IsReturnValue)
                            {
                                if (!firstArg)
                                {
                                    sb.Append(", ");
                                }
                                else
                                {
                                    firstArg = false;
                                }
                                sb.Append(cl+ "                             " + GetSampleValue(arg.RelatedStateVar));
                            }
                        }
                        if (!firstArg)
                        {
                            sb.Append(", ");
                        }

                        sb.Append(cl + "                            userState2);" + cl);
                        sb.Append("                     }" + cl);
                        sb.Append("                     return;" + cl);
                        sb.Append("                 }" + cl);
                        sb.Append("                 if (Callback != null)" + cl);
                        sb.Append("                 {" + cl);
                        sb.Append("                      Callback.On" + action.Name + "(errorCode, ");
                        firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" || arg.IsReturnValue)
                            {
                                if (!firstArg)
                                {
                                    sb.Append(", ");
                                }
                                else
                                {
                                    firstArg = false;
                                }
                                sb.Append(cl + "                        " + DeSerializeVariable(arg.RelatedStateVar, "parameters.getValue(\"" + arg.Name + "\")"));
                            }
                        }
                        if (!firstArg)
                        {
                            sb.Append(", ");
                        }

                        sb.Append(cl + "                        userState2);" + cl);
                        sb.Append("                 }" + cl);
                        sb.Append("             }" + cl);
                        sb.Append("         });" + cl);
                        sb.Append("     }" + cl);

                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            sb.Append("     class variableInfo_" + arg.Name + cl);
                            sb.Append("     {" + cl);
                            sb.Append("         public String[] AllowedValues;" + cl);
                            if (IsNumericJavaType(arg.RelatedStateVar))
                            {
                                sb.Append("         public RangeInfo Range;" + cl);
                            }
                            sb.Append(cl);
                            sb.Append("         variableInfo_" + arg.Name + "()" + cl);
                            sb.Append("         {" + cl);
                            sb.Append("             UPnPStateVariable var = mService.getStateVariable(\"" + arg.RelatedStateVar.Name + "\");" + cl);
                            sb.Append("             if(var!=null)" + cl);
                            sb.Append("             {" + cl);
                            sb.Append("                 AllowedValues = var.GetAllowedValues();" + cl);
                            if (IsNumericJavaType(arg.RelatedStateVar))
                            {
                                sb.Append("                 if(var.getMinRange()!=null && var.getMaxRange()!=null)" + cl);
                                sb.Append("                 {" + cl);
                                sb.Append("                     Range = new RangeInfo();" + cl);
                                sb.Append("                 }" + cl);
                            }
                            sb.Append("             }" + cl);
                            sb.Append("         }" + cl);
                            if (IsNumericJavaType(arg.RelatedStateVar))
                            {
                                sb.Append("         class RangeInfo" + cl);
                                sb.Append("         {" + cl);
                                sb.Append("             public " + GetJavaType(arg.RelatedStateVar) + " minimum = " + this.DeSerializeVariable(arg.RelatedStateVar, "mService.getStateVariable(\"" + arg.RelatedStateVar.Name + "\").getMinRange()") + ";" + cl);
                                sb.Append("             public " + GetJavaType(arg.RelatedStateVar) + " maximum = " + this.DeSerializeVariable(arg.RelatedStateVar, "mService.getStateVariable(\"" + arg.RelatedStateVar.Name + "\").getMaxRange()") + ";" + cl);
                                sb.Append("             public " + GetJavaType(arg.RelatedStateVar) + " step = mService.getStateVariable(\"" + arg.RelatedStateVar.Name + "\").getStep()==null?(" + GetJavaType(arg.RelatedStateVar) + ")0:" + this.DeSerializeVariable(arg.RelatedStateVar, "mService.getStateVariable(\"" + arg.RelatedStateVar.Name + "\").getStep()") + ";" + cl);
                                sb.Append("         }" + cl);
                            }
                            sb.Append("     }" + cl);
                        }

                        sb.Append(" }" + cl); // End of ActionHelper
                    }
                    innerClasses = sb.ToString();
                    #endregion
                    #region Query Methods
                    sb = new StringBuilder();
                    string queryMethods = "";

                    sb.Append(" public void LoadSCPD(Object userObject, LoadedSCPDHandler userCallback)" + cl);
                    sb.Append(" {" + cl);
                    sb.Append("     mService.LoadAndProcessSCPD(new Object[]{this, userObject, userCallback}, new UPnPService_FinishedParsingSCPD()" + cl);
                    sb.Append("     {" + cl);
                    sb.Append("         @Override" + cl);
			        sb.Append("         public void OnFinishedParsingSCPD(UPnPService sender, boolean success, Object userState)"+cl);
                    sb.Append("         {"+cl);
                    sb.Append("             Cp" + servicename + " meObject = (Cp" + servicename + ")((Object[])userState)[0];"+cl);
                    sb.Append("             Object userObject = ((Object[])userState)[1];" + cl);
                    sb.Append("             LoadedSCPDHandler userCallback = (LoadedSCPDHandler)((Object[])userState)[2];" + cl);
                    sb.Append("             meObject.RefreshActions();" + cl);
                    sb.Append("             if(userCallback!=null)" + cl);
                    sb.Append("             {" + cl);
                    sb.Append("                 userCallback.OnCp" + servicename + "_LoadedSCPD(meObject, success, userObject);" + cl);
                    sb.Append("             }" + cl);
                    sb.Append("         }"+cl);
                    sb.Append("     });" + cl);
                    sb.Append(" }" + cl);
                    sb.Append(cl);

                    /*
                    foreach (UPnPAction action in service.Actions)
                    {
                        sb.Append(" public boolean hasAction_" + action.Name + "()" + cl);
                        sb.Append(" {" + cl);
                        sb.Append("     return(mService.HasAction(\"" + action.Name + "\"));" + cl);
                        sb.Append(" }" + cl);
                    }
                     */
                    foreach (UPnPStateVariable var in service.GetStateVariables())
                    {
                        sb.Append(" public boolean hasStateVariable_" + var.Name + "()" + cl);
                        sb.Append(" {" + cl);
                        sb.Append("     return(mService.HasStateVariable(\"" + var.Name + "\"));" + cl);
                        sb.Append(" }" + cl);
                    }
                    queryMethods = sb.ToString();
                    #endregion
                    #region Refresh Actions
                    string refreshActions = "";
                    sb = new StringBuilder();
                    foreach (UPnPAction action in service.Actions)
                    {
                        sb.Append("     action_" + action.Name + " = !mService.isScpdLoadAttempted()?new actionHelper_" + action.Name + "():(mService.HasAction(\"" + action.Name + "\")?(new actionHelper_" + action.Name + "()):null);" + cl);
                    }
                    refreshActions = sb.ToString();
                    #endregion

                    #region Service Handlers
                    string serviceInterfaces = "";
                    cs = new CodeProcessor(new StringBuilder(), true);
                    #region Invocation Handler                    
                    cs.Append("public interface InvokeHandler" + cl);
                    cs.Append("{" + cl);
                    foreach (UPnPAction action in service.Actions)
                    {
                        cs.Append(" public void On" + action.Name + "(int errorCode");
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" || arg.IsReturnValue)
                            {
                                cs.Append(", " + GetJavaType(arg.RelatedStateVar) + " " + arg.Name);
                            }
                        }
                        cs.Append(", Object userState);" + cl);
                    }
                    cs.Append("}" + cl);
                    #endregion
                    #region Event Handler
                    cs.Append("public interface EventHandler" + cl);
                    cs.Append("{" + cl);
                    foreach (UPnPStateVariable var in service.GetStateVariables())
                    {
                        if (var.SendEvent)
                        {
                            cs.Append(" public void On" + var.Name + "(" + GetJavaType(var) + " value);" + cl);
                        }
                    }
                    cs.Append("}" + cl);
                    #endregion
                    #region Ready Handler
                    cs.Append("public interface LoadedSCPDHandler" + cl);
                    cs.Append("{" + cl);
                    cs.Append(" public void OnCp" + servicename + "_LoadedSCPD(Cp" + servicename + " sender, boolean success, Object userObject);" + cl);
                    cs.Append("}" + cl);
                    #endregion
                    serviceInterfaces = cs.ToString();
                    #endregion

                    writer = File.CreateText(srcDir.FullName + "\\Cp" + servicename + ".java");
                    writer.Write(
                        SourceCodeRepository.ReadFileStore("Android\\CpTemplate.java")
                        .Replace("{{{PACKAGE}}}", nspace)
                        .Replace("{{{CLASSNAME}}}", "Cp" + servicename)
                        .Replace("{{{EVENTPROCESSOR}}}", eventProcessor)
                        .Replace("{{{INNER_CLASSES}}}", innerClasses)
                        .Replace("{{{SERVICETYPE}}}", service.ServiceURN)
                        .Replace("{{{QUERY_METHODS}}}", queryMethods)
                        .Replace("{{{METHODS}}}", methods)
                        .Replace("{{{REFRESH_ACTIONS}}}", refreshActions)
                        .Replace("{{{INTERFACES}}}",serviceInterfaces)
                        );
                    writer.Close();
                }
                #endregion
            }

            int cpNum = 0;
            StringBuilder declaration = new StringBuilder();
            StringBuilder handlers = new StringBuilder();
            StringBuilder init = new StringBuilder();
            StringBuilder stop = new StringBuilder();
            foreach (UPnPDevice device in cpDevices)
            {
                AllServices.Clear();
                AddAllServices(device);
                ServiceNames = GetServiceNameTable(device);

                #region Sample App

                ++cpNum;
                #region Declaration
                declaration.Append(" private UPnPControlPoint mCP" + cpNum.ToString() + ";" + cl);
                #endregion
                #region Handlers
                handlers.Append("   private UPnPDeviceHandler mCP" + cpNum.ToString() + "_Handler = new UPnPDeviceHandler()" + cl);
                handlers.Append("   {" + cl);
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];
                    handlers.Append("       Cp" + servicename + ".InvokeHandler mHandler_" + servicename + " = new Cp" + servicename + ".InvokeHandler()" + cl);
                    handlers.Append("       {" + cl);
                    foreach (UPnPAction action in service.Actions)
                    {
                        handlers.Append("           @Override" + cl);
                        handlers.Append("           public void On" + action.Name + "(int errorCode, ");
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "out" || arg.IsReturnValue)
                            {
                                handlers.Append(GetJavaType(arg.RelatedStateVar) + " " + arg.Name + ", ");
                            }
                        }
                        handlers.Append("Object userState)" + cl);
                        handlers.Append("           {" + cl);
                        handlers.Append("           }" + cl);
                    }
                    handlers.Append("       };" + cl);
                }
                handlers.Append(cl);
                handlers.Append("       @Override"+cl);
                handlers.Append("       public void OnAddedDevice(UPnPDevice device)" + cl);
                handlers.Append("       {" + cl);
                handlers.Append("           UPnPService service;" + cl);
                handlers.Append(cl);
                foreach (UPnPService service in AllServices)
                {
                    string servicename = (string)ServiceNames[service];
                    handlers.Append("           service = device.GetService(Cp" + servicename + ".ServiceType).get(0);" + cl);
                    handlers.Append("           service.userObject = new Cp" + servicename + "(service);" + cl);
                    handlers.Append("           //((Cp" + servicename + ")service.userObject).LoadSCPD(null, new Cp" + servicename + ".LoadedSCPDHandler()" + cl);
                    handlers.Append("           //{" + cl);
                    handlers.Append("        	//   @Override" + cl);
                    handlers.Append("        	//   public void On" + servicename + "_LoadedSCPD(Cp" + servicename + " sender, boolean success, Object userObject) " + cl);
                    handlers.Append("        	//   {" + cl);
                    handlers.Append("        		    /*" + cl);
                    handlers.Append("        		        * After calling LoadSCPD on this object, the complete object representation of the Device will be available." + cl);
                    handlers.Append("        		        * Action objects for Actions that are not implemented by the Device will be set to null" + cl);
                    handlers.Append("        		        * after the SCPD is loaded. Likewise, after the SCPD is loaded, the argument fields" + cl);
                    handlers.Append("        		        * will be populated with the appropriate data, reflecting any parameter restrictions exposed by the device." + cl);
                    handlers.Append("        		        * " + cl);
                    handlers.Append("        		    */" + cl);
                    handlers.Append("        	//   }" + cl);
                    handlers.Append("           //});" + cl);
                    foreach (UPnPAction action in service.Actions)
                    {
                        handlers.Append("           //((Cp" + servicename + ")service.userObject).action_" + action.Name + ".Invoke" + "(");
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                handlers.Append(GetSampleValue(arg.RelatedStateVar) + ", ");
                            }
                        }
                        handlers.Append("null, mHandler_" + servicename + ");" + cl);
                    }
                }

    	   
                handlers.Append("       }" + cl);
                handlers.Append(cl);
                handlers.Append("       @Override" + cl);
                handlers.Append("       public void OnRemovedDevice(UPnPDevice device)" + cl);
                handlers.Append("       {" + cl);
                handlers.Append("       }" + cl);
                handlers.Append("   };" + cl);
                #endregion
                #region Init
                init.Append("               mCP" + cpNum.ToString() + " = new UPnPControlPoint(\"" + device.DeviceURN + "\", mCP" + cpNum.ToString() + "_Handler);" + cl);
                #endregion
                #region Stop
                stop.Append("               mCP" + cpNum.ToString() + ".Stop();" + cl);
                #endregion

                #endregion
            }
            #endregion

            writer = File.CreateText(srcDir.FullName + "\\" + ProjectName + ".java");
            writer.Write(
                SourceCodeRepository.ReadFileStore("Android\\SampleApp.java")
                .Replace("{{{PACKAGE}}}", ClassName)
                .Replace("{{{PROJECTNAME}}}", ProjectName)
                .Replace("//{{{CP_DECLARATION}}}", declaration.ToString())
                .Replace("//{{{CP_HANDLERS}}}", handlers.ToString())
                .Replace("//{{{CP_INIT}}}", init.ToString())
                .Replace("//{{{CP_STOP}}}",stop.ToString())

                .Replace("//{{{DV_DECLARATION}}}", dvDeclaration.ToString())
                .Replace("//{{{DV_INIT}}}", dvInit.ToString())
                .Replace("//{{{DV_HANDLERS}}}", dvHandlers.ToString())
                .Replace("//{{{DV_START}}}",dvStart.ToString())
                .Replace("//{{{DV_STOP}}}", dvStop.ToString())
                );
            writer.Close();

            return (true);
        }

        #region Build Service Specific Handler (Device)
        private void ProcessDevice_BuildService(UPnPDevice device, CodeProcessor cs)
        {
            //
            // Add Custom Tags
            //
            foreach (string ns in device.GetCustomFieldFromDescription_Namespaces())
            {
                foreach (KeyValuePair<string, string> tag in device.GetCustomFieldsFromDescription(ns))
                {
                    cs.Append("        device.AddCustomTag(\"" + ns + "\", \"" + tag.Key + "\", \"" + tag.Value + "\");" + cl);
                }
            }

            //
            // Populate the rest of the Metadata
            //
            cs.Append("        device.Manufacturer = \"" + device.Manufacturer + "\";"+cl);
	        cs.Append("        device.ManufacturerURL = \"" + device.ManufacturerURL + "\";"+cl);
	        cs.Append("        device.ModelDescription = \"" + device.ModelDescription + "\";"+cl);
	        cs.Append("        device.ModelName = \"" + device.ModelName + "\";"+cl);
	        cs.Append("        device.ModelNumber = \"" + device.ModelNumber + "\";"+cl);
	        cs.Append("        device.ModelURL = \"" + device.ModelURL + "\";"+cl);
            cs.Append("        device.SerialNumber = \"" + device.SerialNumber + "\";" + cl);


            //
            // Process Services/Devices
            //
            foreach (UPnPService service in device.Services)
            {
                string serviceName = (string)ServiceNames[service];
                cs.Append("        service = new UPnPService(\"");
                cs.Append(service.ServiceURN + "\", \"" + service.ServiceID + "\");" + cl);
                foreach (UPnPStateVariable var in service.GetStateVariables())
                {
                    cs.Append("        var = new UPnPStateVariable(\"" + var.Name + "\", \"" + var.ValueType + "\", " + (var.SendEvent ? "true" : "false") + ");" + cl);
                
                    //
                    // Populate Fields
                    //
                    if (var.AllowedStringValues != null)
                    {
                        bool isFirst = true;
                        StringBuilder asb = new StringBuilder();
                        foreach (string value in var.AllowedStringValues)
                        {
                            if (!isFirst)
                            {
                                asb.Append(", ");
                            }
                            else
                            {
                                isFirst = false;
                            }
                            asb.Append("\"" + value + "\"");
                        }
                        cs.Append("        var.SetAllowedValues(new String[]{" + asb.ToString() + "});" + cl);
                    }

                    if (var.Minimum != null || var.Maximum != null)
                    {
                        // Set Range
                        cs.Append("        var.SetRange(" + (var.Minimum != null ? ("\"" + var.Minimum.ToString() + "\"") : "null") + ", " + (var.Maximum != null ? ("\"" + var.Maximum.ToString() + "\"") : "null") + ", " + (var.Step != null ? ("\"" + var.Step.ToString() + "\"") : "null") + ");" + cl);
                    }

                    // Set Default Value
                    if (var.DefaultValue != null)
                    {
                        cs.Append("        var.SetDefaultValue(\"" + var.DefaultValue.ToString() + "\");" + cl);
                    }
                    cs.Append("        service.AddStateVariable(var);" + cl);
                }

                foreach (UPnPAction action in service.Actions)
                {
                    cs.Append(cl);
                    cs.Append("        inParams.clear();" + cl);
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        cs.Append("        arg = new UPnPArgument(\"" + arg.Name + "\", ArgumentDirection.");
                        if (arg.IsReturnValue)
                        {
                            cs.Append("RETURN");
                        }
                        else
                        {
                            cs.Append(arg.Direction.ToUpper());
                        }
                        cs.Append(", service.getStateVariable(\"" + arg.RelatedStateVar.Name + "\"));" + cl);
                        cs.Append("        inParams.add(arg);" + cl);
                    }

                    cs.Append("        action = new UPnPAction(\"" + action.Name + "\", inParams, new " + serviceName + "_" + action.Name + "_Dispatcher());" + cl);
                    cs.Append("        service.AddAction(action);" + cl);
                }
                cs.Append("        device.AddService(service);" + cl);
            }


            cs.Append("        mDevice" + embeddedTable[device].ToString() + " = device;" + cl);
            foreach (UPnPDevice eDevice in device.EmbeddedDevices)
            {
                ++eDeviceIndex;

                cs.Append(cl);
                cs.Append("        tmpDevice = new UPnPDevice(uniqueIdentifier" + eDeviceIndex.ToString() + ", friendlyName" + eDeviceIndex.ToString() + ", \"" + eDevice.DeviceURN + "\");" + cl);
                cs.Append("        device.AddEmbeddedDevice(tmpDevice);" + cl);
                cs.Append("        device = tmpDevice;" + cl);
                ProcessDevice_BuildService(eDevice, cs);
            }
        }
#endregion

        #region Serialization / Deserialization Methods
        private string DeSerializeVariable(UPnPStateVariable var)
        {
            return (DeSerializeVariable(var, var.Name));
        }
        private string DeSerializeVariable(UPnPStateVariable var, string fromValue)
        {
            string retVal = "";

            switch (GetJavaType(var))
            {
                case "String":
                    retVal = fromValue;
                    break;
                case "boolean":
                    retVal = "((" + fromValue + ".equals(\"1\") || " + fromValue + ".equals(\"true\") || " + fromValue + ".equals(\"yes\"))?true:false)";
                    break;
                case "char":
                    switch (var.ValueType)
                    {
                        case "ui1":
                            retVal = "((char)Short.valueOf(" + fromValue + ").shortValue())";
                            break;
                        case "ui2":
                            retVal = "((char)Integer.valueOf(" + fromValue + ").intValue())";
                            break;
                        default:
                            retVal = "(" + fromValue + ".charAt(0))";
                            break;
                    }
                    break;
                case "byte":
                    retVal = "(Byte.valueOf(" + fromValue + ").byteValue())";
                    break;
                case "short":
                    retVal = "(Short.valueOf(" + fromValue + ").shortValue())";
                    break;
                case "int":
                    retVal = "(Integer.valueOf(" + fromValue + ").intValue())";
                    break;
                case "long":
                    retVal = "(Long.valueOf(" + fromValue + ").longValue())";
                    break;
                case "float":
                    retVal = "(Float.valueOf(" + fromValue + ").floatValue())";
                    break;
                case "double":
                    retVal = "(Double.valueOf(" + fromValue + ").doubleValue())";
                    break;
                case "byte[]":
                    retVal = "(Base64.decode(" + fromValue + "),Base64.DEFAULT))";
                    break;
                case "Date":
                    retVal = "(ILibParsers.dateFromString(" + fromValue + "))";
                    break;
                default:
                    retVal = "(" + fromValue + ").toString())";
                    break;
            }
            return (retVal);
        }
        private string SerializeReferenceArgument(UPnPArgument arg)
        {
            string retVal = "";

            switch (GetJavaType(arg.RelatedStateVar))
            {
                case "String":
                    retVal = arg.Name + ".value";
                    break;
                case "boolean":
                    retVal = "(" + arg.Name + ".value.booleanValue()?\"1\":\"0\")";
                    break;
                case "char":
                    switch (arg.RelatedStateVar.ValueType)
                    {
                        case "ui1":
                        case "ui2":
                            retVal = "String.valueOf((int)" + arg.Name + ".value)";
                            break;
                        default:
                            // Actual char type
                            retVal = "String.valueOf(" + arg.Name + ".value)";
                            break;
                    }
                    break;
                case "byte":
                case "short":
                case "int":
                case "long":
                case "float":
                case "double":
                    retVal = arg.Name + ".value.toString()";
                    break;
                case "byte[]":
                    retVal = "Base64.encodeToString(" + arg.Name + ".value, Base64.DEFAULT)";
                    break;
                case "Date":
                    if (arg.RelatedStateVar.ValueType == "date")
                    {
                        //date
                        retVal = "(new SimpleDateFormat(\"yyyy-MM-dd\").format(" + arg.Name + ".value))";
                    }
                    else
                    {
                        //dateTime
                        retVal = "(new SimpleDateFormat(\"yyyy-MM-dd'T'HH:mm:ss:SSSz\").format(" + arg.Name + ".value))";
                    }
                    break;
                default:
                    retVal = arg.Name + ".value";
                    break;
            }
            return (retVal);
        }
        private string SerializeVariable(UPnPArgument arg)
        {
            return (SerializeVariable(arg.RelatedStateVar, arg.Name));
        }
        private string SerializeVariable(UPnPStateVariable var)
        {
            return (SerializeVariable(var, var.Name));
        }
        private string SerializeVariable(UPnPStateVariable var, string varName)
        {
            string retVal = "";

            switch (GetJavaType(var))
            {
                case "String":
                    retVal = varName;
                    break;
                case "boolean":
                    retVal = "(" + varName + "?\"1\":\"0\")";
                    break;
                case "char":
                    switch (var.ValueType)
                    {
                        case "ui1":
                        case "ui2":
                            retVal = "String.valueOf((int)" + varName + ")";
                            break;
                        default:
                            retVal = "String.valueOf(" + varName + ")";
                            break;
                    }
                    break;
                case "byte":
                case "short":
                case "int":
                case "long":
                case "float":
                case "double":
                    retVal = "String.valueOf(" + varName + ")";
                    break;
                case "byte[]":
                    retVal = "(Base64.encodeToString(" + varName + ".value, Base64.DEFAULT))";
                    break;
                case "Date":
                    if (var.ValueType == "date")
                    {
                        //date
                        retVal = "(new SimpleDateFormat(\"yyyy-MM-dd\").format(" + varName + ".value))";
                    }
                    else
                    {
                        //dateTime
                        retVal = "(new SimpleDateFormat(\"yyyy-MM-dd'T'HH:mm:ss:SSSz\").format(" + varName + ".value))";
                    }
                    break;
                default:
                    retVal = varName + ".toString()";
                    break;
            }
            return (retVal);
        }
        #endregion
        #region Type Conversion Methods
        protected string GetJavaReferenceType(UPnPStateVariable var)
        {
            return ("RefParameter<" + GetJavaType(var, true) + ">");
        }
        protected string GetJavaType(UPnPStateVariable var)
        {
            return (GetJavaType(var, false));
        }
        protected bool IsNumericJavaType(UPnPStateVariable var)
        {
            bool retVal = false;
            switch (var.ValueType)
            {
                case "ui1":
                case "ui2":
                case "ui4":
                case "int":
                case "i4":
                case "i2":
                case "i1":
                case "r4":
                case "float":
                case "r8":
                case "number":
                    retVal = true;
                    break;
                default:
                    retVal = false;
                    break;
            }
            return (retVal);
        }
        protected string GetJavaType(UPnPStateVariable var, bool boxed)
        {
            string RetVal = "Object";

            switch (var.ValueType)
            {
                case "string":
                    RetVal = "String";
                    break;
                case "boolean":
                    if (boxed)
                    {
                        RetVal = "Boolean";
                    }
                    else
                    {
                        RetVal = "boolean";
                    }
                    break;
                case "uri":
                    RetVal = "String";
                    break;
                case "ui1":
                case "ui2":
                case "char":
                    if (boxed)
                    {
                        RetVal = "Character";
                    }
                    else
                    {
                        RetVal = "char";
                    }
                    break;
                case "ui4":
                    if (boxed)
                    {
                        RetVal = "Long";
                    }
                    else
                    {
                        RetVal = "long";
                    }
                    break;
                case "int":
                case "i4":
                    if (boxed)
                    {
                        RetVal = "Integer";
                    }
                    else
                    {
                        RetVal = "int";
                    }
                    break;
                case "i2":
                    if (boxed)
                    {
                        RetVal = "Short";
                    }
                    else
                    {
                        RetVal = "short";
                    }
                    break;
                case "i1":
                    if (boxed)
                    {
                        RetVal = "Byte";
                    }
                    else
                    {
                        RetVal = "byte";
                    }
                    break;
                case "r4":
                case "float":
                    if (boxed)
                    {
                        RetVal = "Float";
                    }
                    else
                    {
                        RetVal = "float";
                    }
                    break;
                case "r8":
                case "number":
                    if (boxed)
                    {
                        RetVal = "Double";
                    }
                    else
                    {
                        RetVal = "double";
                    }
                    break;
                case "bin.base64":
                    RetVal = "byte[]";
                    break;
                case "date":
                case "dateTime":
                    RetVal = "Date";
                    break;
                default:
                    RetVal = "Object";
                    break;
            }
            return (RetVal);
        }
        #endregion
        #region Sample Values
        private string GetSampleValue(UPnPStateVariable var)
        {
            return (GetSampleValue(var, false));
        }
        private string GetSampleValue(UPnPStateVariable var, Boolean boxed)
        {
            string retVal = "";

            switch (GetJavaType(var))
            {
                case "String":
                    retVal = "\"Unknown\"";
                    break;
                case "boolean":
                    if (boxed)
                    {
                        retVal = "Boolean.valueOf(false)";
                    }
                    else
                    {
                        retVal = "false";
                    }
                    break;
                case "char":
                    if (boxed)
                    {
                        retVal = "Character.valueOf((char)0)";
                    }
                    else
                    {
                        retVal = "(char)0";
                    }
                    break;
                case "byte":
                    if (boxed)
                    {
                        retVal = "Byte.valueOf((byte)0)";
                    }
                    else
                    {
                        retVal = "(byte)0";
                    }
                    break;
                case "short":
                    if (boxed)
                    {
                        retVal = "Short.valueOf((short)0)";
                    }
                    else
                    {
                        retVal = "(short)0";
                    }
                    break;
                case "int":
                    if (boxed)
                    {
                        retVal = "Integer.valueOf((int)0)";
                    }
                    else
                    {
                        retVal = "(int)0";
                    }
                    break;
                case "long":
                    if (boxed)
                    {
                        retVal = "Long.valueOf((long)0)";
                    }
                    else
                    {
                        retVal = "(long)0";
                    }
                    break;
                case "float":
                    if (boxed)
                    {
                        retVal = "Float.valueOf((float)0)";
                    }
                    else
                    {
                        retVal = "(float)0";
                    }
                    break;
                case "double":
                    if (boxed)
                    {
                        retVal = "Double.valueOf((double)0)";
                    }
                    else
                    {
                        retVal = "(double)0";
                    }
                    break;
                case "byte[]":
                    retVal = "new byte[0]";
                    break;
                case "Date":
                    retVal = "new Date()";
                    break;
                default:
                    retVal = "\"Unknown\"";
                    break;
            }
            return (retVal);
        }
        #endregion

    }
}
