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
using System.Drawing;
using System.Collections;

namespace OpenSource.UPnP
{
    /// <summary>
    /// CodeGenerator class that parses SCPD files, and returns Device/Control Point implementations
    /// </summary>
    public sealed class ServiceGenerator
    {
        public enum PLATFORMS
        {
            MICROSTACK_POSIX = 0,
            MICROSTACK_WINSOCK1 = 1,
            MICROSTACK_WINSOCK2 = 2,
            MICROSTACK_POCKETPC = 3,
            DOTNET = 5,
            MICROSTACK_SYMBIANv9_1 = 4,
            ANDROID = 6,
        }
        public enum CALLINGCONVENTION
        {
            DEFAULT = 0,
            STDCALL = 1,
            FASTCALL = 2,
        }
        public enum INDENTATION
        {
            TAB = 0,
            ONESPACE = 1,
            TWOSPACES = 2,
            THREESPACES = 3,
            FOURSPACES = 4,
            FIVESPACES = 5,
            SIXSPACES = 6,
        }
        public enum NEWLINETYPE
        {
            CRLF = 0,
            LF = 1,
        }
        public enum ConfigurationType
        {
            DEVICE,
            CONTROLPOINT
        }
        public enum XSDSchemaGeneration
        {
            NONE = 0,
            WITH_ANY_TAGS = 1,
            WITHOUT_ANY_TAGS = 2,
        }

        public class Configuration
        {
            public override bool Equals(object obj)
            {
                return (this.GetHashCode() == ((Configuration)obj).GetHashCode());
            }

            public override int GetHashCode()
            {
                return base.GetHashCode();
            }

            public Image IconImageSM = null;
            public Image IconImageLG = null;
            public string Prefix;
            public ConfigurationType ConfigType;
            public bool AdvertisesPresentationPage;

            public Hashtable CustomFieldTable;

            public int MaxHTTPHeaderSize = 0;
            public int MaxSOAPBodySize = 0;
            public int SSDPCycleTime = 1800;
            public int WebPort = 0;
            public int MaxSubscriptionTimeout = 7200;

            public Hashtable ToHashtable()
            {
                Hashtable t = new Hashtable();

                t["Prefix"] = Prefix;
                t["ConfigType"] = (int)ConfigType;
                t["AdvertisesPresentationPage"] = AdvertisesPresentationPage;
                t["CustomFieldTable"] = CustomFieldTable;
                t["MaxHTTPHeaderSize"] = MaxHTTPHeaderSize;
                t["MaxSOAPBodySize"] = MaxSOAPBodySize;
                t["SSDPCycleTime"] = SSDPCycleTime;
                t["WebPort"] = WebPort;
                t["IconImage"] = IconImageSM;
                t["IconImageLG"] = IconImageLG;
                t["MaxSubscriptionTimeout"] = MaxSubscriptionTimeout;

                return (t);
            }
            public static Configuration FromHashtable(Hashtable t)
            {
                Configuration RetVal = new Configuration((string)t["Prefix"], (ServiceGenerator.ConfigurationType)(int)t["ConfigType"]);

                RetVal.AdvertisesPresentationPage = (bool)t["AdvertisesPresentationPage"];
                RetVal.CustomFieldTable = (Hashtable)t["CustomFieldTable"];
                RetVal.MaxHTTPHeaderSize = (int)t["MaxHTTPHeaderSize"];
                RetVal.MaxSOAPBodySize = (int)t["MaxSOAPBodySize"];
                RetVal.SSDPCycleTime = (int)t["SSDPCycleTime"];
                RetVal.WebPort = (int)t["WebPort"];
                if (t.Contains("IconImage")) { RetVal.IconImageSM = (Image)t["IconImage"]; }
                if (t.Contains("IconImageLG"))
                {
                    RetVal.IconImageLG = (Image)t["IconImageLG"];
                }
                else if (RetVal.IconImageSM != null)
                {
                    RetVal.IconImageLG = RetVal.IconImageSM;
                }
                if (t.Contains("MaxSubscriptionTimeout")) { RetVal.MaxSubscriptionTimeout = (int)t["MaxSubscriptionTimeout"]; }

                return (RetVal);
            }

            public Configuration(string desiredPrefix, ConfigurationType desiredConfigType)
            {
                Prefix = desiredPrefix;
                ConfigType = desiredConfigType;
                CustomFieldTable = new Hashtable();
                AdvertisesPresentationPage = false;
            }

            public void LegacyFromHashtable(Hashtable t)
            {
                if (t.Contains("MaxHttpHeaderTextBox")) { MaxHTTPHeaderSize = (int)t["MaxHttpHeaderTextBox"]; }
                if (t.Contains("MaxSoapBodySize")) { MaxSOAPBodySize = (int)t["MaxSoapBodySize"]; }
                if (t.Contains("SsdpCycleTime")) { SSDPCycleTime = (int)t["SsdpCycleTime"]; }
                if (t.Contains("WebPort")) { WebPort = (int)t["WebPort"]; }

                if (t.Contains("CustomFields"))
                {
                    CustomFieldTable.Clear();
                    IDictionaryEnumerator e = ((Hashtable)t["CustomFields"]).GetEnumerator();
                    while (e.MoveNext())
                    {
                        if (e.Value.GetType() == typeof(Hashtable))
                        {
                            CustomFieldTable[e.Key] = new Hashtable();
                            IDictionaryEnumerator ee = ((Hashtable)e.Value).GetEnumerator();
                            while (ee.MoveNext())
                            {
                                ((Hashtable)CustomFieldTable[e.Key])[ee.Key] = ee.Value;
                            }
                        }
                    }
                }
            }
            public void AddCustomField(string FieldNamespace, string CustomFieldName, string CustomFieldValue)
            {
                Hashtable lookupTable;

                if (CustomFieldTable.Contains(FieldNamespace))
                {
                    lookupTable = (Hashtable)CustomFieldTable[FieldNamespace];
                }
                else
                {
                    lookupTable = new Hashtable();
                    CustomFieldTable[FieldNamespace] = lookupTable;
                }
                lookupTable[CustomFieldName] = CustomFieldValue;
            }
            public void AddAllCustomFieldsToDevice(UPnPDevice d)
            {
                IDictionaryEnumerator NamespaceEnumerator;
                IDictionaryEnumerator EntryEnumerator;

                d.ClearCustomFieldsInDescription();
                NamespaceEnumerator = CustomFieldTable.GetEnumerator();
                while (NamespaceEnumerator.MoveNext())
                {
                    EntryEnumerator = ((Hashtable)NamespaceEnumerator.Value).GetEnumerator();
                    while (EntryEnumerator.MoveNext())
                    {
                        d.AddCustomFieldInDescription((string)EntryEnumerator.Key, (string)EntryEnumerator.Value, (string)NamespaceEnumerator.Key);
                    }
                }
            }
        }

        public class ServiceConfiguration
        {
            protected UPnPService Service;
            public string Name;
            public ArrayList Actions_Fragmented;
            public ArrayList Actions_ManualEscape;
            public ServiceConfiguration(string ServiceName, UPnPService s)
            {
                this.Service = s;
                Actions_Fragmented = new ArrayList();
                Actions_ManualEscape = new ArrayList();
                Name = ServiceName;
            }
            public override string ToString()
            {
                return (Name);
            }
            public Hashtable ToHashtable()
            {
                Hashtable t = new Hashtable();
                ArrayList Frag = new ArrayList();
                ArrayList Escape = new ArrayList();

                foreach (UPnPAction A in Actions_Fragmented)
                {
                    Frag.Add(A.Name);
                }
                foreach (UPnPAction A in Actions_ManualEscape)
                {
                    Escape.Add(A.Name);
                }

                t["Name"] = Name;
                t["Actions_Fragmented"] = Frag;
                t["Actions_ManualEscape"] = Escape;
                t["Parent"] = new System.Text.UTF8Encoding().GetString(this.Service.GetSCPDXml());
                t["Parent_Type"] = Service.ServiceURN;
                t["Parent_ID"] = Service.ServiceID;

                return (t);
            }
            public static ServiceConfiguration FromHashtable(Hashtable t)
            {
                UPnPService s = ServiceGenerator.GenerateServiceFromSCPD((string)t["Parent"]);
                ServiceConfiguration RetVal = new ServiceConfiguration((string)t["Name"], s);

                s.ServiceID = (string)t["Parent_ID"];
                s.ServiceURN = (string)t["Parent_Type"];

                foreach (string name in (ArrayList)t["Actions_Fragmented"])
                {
                    RetVal.Actions_Fragmented.Add(s.GetAction(name));
                }
                foreach (string name in (ArrayList)t["Actions_ManualEscape"])
                {
                    RetVal.Actions_ManualEscape.Add(s.GetAction(name));
                }

                return (RetVal);
            }
            private static bool HasFragmentedActionsEx(UPnPDevice d)
            {
                bool RetVal = false;
                foreach (UPnPService s in d.Services)
                {
                    if (s.User != null)
                    {
                        if (((ServiceConfiguration)s.User).Actions_Fragmented.Count > 0)
                        {
                            RetVal = true;
                            break;
                        }
                    }
                }

                if (!RetVal)
                {
                    foreach (UPnPDevice ed in d.EmbeddedDevices)
                    {
                        RetVal = HasFragmentedActionsEx(ed);
                        if (RetVal) { break; }
                    }
                }
                return (RetVal);
            }
            public static bool HasFragmentedActions(UPnPDevice d)
            {
                return (HasFragmentedActionsEx(d));
            }
        }

        public class StackConfiguration
        {
            public bool UPNP_1dot1;
            public PLATFORMS TargetPlatform;

            public string outputpath;
            public string projectname;
            public NEWLINETYPE newline;
            public CALLINGCONVENTION callconvention;
            public string prefixlib;
            public INDENTATION indent;
            public string classname;

            public bool ExplicitErrorEncoding;
            public bool EXTERN_Callbacks;

            public bool DefaultIPAddressMonitor;
            public bool HTTP_1dot1;
            public bool SupressSampleProject;
            public bool BareBonesSample = false;

            public XSDSchemaGeneration SchemaGeneration;

            public bool GenerateThreadPoolLibrary;
            public bool InitThreadPoolInSampleApp;
            public int ThreadPoolThreads_InSampleApp;

            public bool DynamicObjectModel = false;
            public bool CPlusPlusWrapper = false;

            public int MaxHTTPPacketSize = 0;
            public int MaxHTTPHeaderSize = 0;
            public int InitialHTTPBufferSize = 65535;

            public bool GenerateCertToolFiles = false;

            public StackConfiguration()
            {
                UPNP_1dot1 = false;
                TargetPlatform = PLATFORMS.MICROSTACK_POSIX;

                outputpath = "";
                projectname = "";
                newline = NEWLINETYPE.LF;
                callconvention = CALLINGCONVENTION.DEFAULT;
                prefixlib = "ILib";
                indent = INDENTATION.TAB;
                classname = "";

                ExplicitErrorEncoding = false;
                EXTERN_Callbacks = false;

                DefaultIPAddressMonitor = true;
                HTTP_1dot1 = true;
                SupressSampleProject = false;

                SchemaGeneration = XSDSchemaGeneration.NONE;
            }

            public Hashtable ToHashtable()
            {
                Hashtable t = new Hashtable();

                t["outputpath"] = outputpath;
                t["platform"] = (int)TargetPlatform;
                t["projectname"] = projectname;
                t["newline"] = (int)newline;
                t["callconvention"] = (int)callconvention;
                t["prefixlib"] = prefixlib;
                t["indent"] = (int)indent;
                t["classname"] = classname;
                t["ExplicitErrorEncoding"] = ExplicitErrorEncoding;
                t["ExternCallbacks"] = EXTERN_Callbacks;
                t["DefaultIPAddressMonitor"] = DefaultIPAddressMonitor;
                t["HTTP11Support"] = HTTP_1dot1;
                t["SupressSample"] = SupressSampleProject;
                t["UPnP1dot1Enabled"] = UPNP_1dot1;
                t["XSDSchemaGeneration"] = (int)SchemaGeneration;
                t["GenerateThreadPoolLibrary"] = GenerateThreadPoolLibrary;
                t["InitThreadPoolInSampleApp"] = InitThreadPoolInSampleApp;
                t["ThreadPoolThreads_InSampleApp"] = ThreadPoolThreads_InSampleApp;

                t["DynamicObjectModel"] = DynamicObjectModel;
                t["CPlusPlusWrapper"] = CPlusPlusWrapper;
                t["BareBonesSample"] = BareBonesSample;

                t["MaxHTTPPacketSize"] = MaxHTTPPacketSize;
                t["MaxHTTPHeaderSize"] = MaxHTTPHeaderSize;
                t["InitialHTTPBufferSize"] = InitialHTTPBufferSize;
                return (t);
            }
            public static StackConfiguration FromHashTable(Hashtable t)
            {
                StackConfiguration RetVal = new StackConfiguration();

                if (t.Contains("outputpath")) RetVal.outputpath = (string)t["outputpath"];
                if (t.Contains("platform")) RetVal.TargetPlatform = (PLATFORMS)(int)t["platform"];
                if (t.Contains("projectname")) RetVal.projectname = (string)t["projectname"];
                if (t.Contains("newline")) RetVal.newline = (NEWLINETYPE)(int)t["newline"];
                if (t.Contains("callconvention")) RetVal.callconvention = (CALLINGCONVENTION)(int)t["callconvention"];
                if (t.Contains("prefixlib")) RetVal.prefixlib = (string)t["prefixlib"];
                if (t.Contains("indent")) RetVal.indent = (INDENTATION)(int)t["indent"];
                if (t.Contains("classname")) RetVal.classname = (string)t["classname"];
                if (t.Contains("ExplicitErrorEncoding")) { RetVal.ExplicitErrorEncoding = (bool)t["ExplicitErrorEncoding"]; }
                if (t.Contains("ExternCallbacks")) { RetVal.EXTERN_Callbacks = (bool)t["ExternCallbacks"]; }
                if (t.Contains("DefaultIPAddressMonitor")) { RetVal.DefaultIPAddressMonitor = (bool)t["DefaultIPAddressMonitor"]; }
                if (t.Contains("HTTP11Support")) { RetVal.HTTP_1dot1 = (bool)t["HTTP11Support"]; }
                if (t.Contains("SupressSample")) { RetVal.SupressSampleProject = (bool)t["SupressSample"]; }
                if (t.Contains("UPnP1dot1Enabled")) { RetVal.UPNP_1dot1 = (bool)t["UPnP1dot1Enabled"]; }
                if (t.Contains("XSDSchemaGeneration")) { RetVal.SchemaGeneration = (XSDSchemaGeneration)(int)t["XSDSchemaGeneration"]; }
                if (t.Contains("GenerateThreadPoolLibrary")) { RetVal.GenerateThreadPoolLibrary = (bool)t["GenerateThreadPoolLibrary"]; }
                if (t.Contains("InitThreadPoolInSampleApp")) { RetVal.InitThreadPoolInSampleApp = (bool)t["InitThreadPoolInSampleApp"]; }
                if (t.Contains("ThreadPoolThreads_InSampleApp")) { RetVal.ThreadPoolThreads_InSampleApp = (int)t["ThreadPoolThreads_InSampleApp"]; }
                if (t.Contains("DynamicObjectModel")) { RetVal.DynamicObjectModel = (bool)t["DynamicObjectModel"]; }
                if (t.Contains("CPlusPlusWrapper")) { RetVal.CPlusPlusWrapper = (bool)t["CPlusPlusWrapper"]; }
                if (t.Contains("BareBonesSample")) { RetVal.BareBonesSample = (bool)t["BareBonesSample"]; }
                if (t.Contains("MaxHTTPHeaderSize")) { RetVal.MaxHTTPHeaderSize = (int)t["MaxHTTPHeaderSize"]; }
                if (t.Contains("MaxHTTPPacketSize")) { RetVal.MaxHTTPPacketSize = (int)t["MaxHTTPPacketSize"]; }
                if (t.Contains("InitialHTTPBufferSize")) { RetVal.InitialHTTPBufferSize = (int)t["InitialHTTPBufferSize"]; }

                return (RetVal);
            }

        }



        private struct VarData
        {
            public string VariableName;
            public ArrayList Enumerations;
        }
        private struct EnumStruct
        {
            public string EnumName;
            public string EnumValue;
        }

        private ServiceGenerator()
        {
        }

        /// <summary>
        /// Generates a UPnPService object from an SCPD XML
        /// </summary>
        /// <param name="SCPD_XML">XML String</param>
        /// <returns>UPnPService</returns>
        public static UPnPService GenerateServiceFromSCPD(string SCPD_XML)
        {
            if (SCPD_XML == null) return null;
            UPnPService s = new UPnPService(1, null);
            UPnPDebugObject obj = new UPnPDebugObject(s);
            obj.InvokeNonStaticMethod("ParseSCPD", new Object[1] { SCPD_XML });

            return (s);
        }

        private static bool IsNumeric(char c)
        {
            int x = (int)c;
            if ((x >= 48) && (x <= 57))
            {
                return (true);
            }
            else
            {
                return (false);
            }
        }

        private static Hashtable BuildEnumerations(UPnPStateVariable[] SV)
        {
            Hashtable h = new Hashtable();
            Hashtable hlist = new Hashtable();
            EnumStruct d;
            VarData vd;
            string t;

            foreach (UPnPStateVariable V in SV)
            {
                if (V.AllowedStringValues != null)
                {
                    vd = new VarData();
                    vd.VariableName = V.Name;
                    vd.Enumerations = new ArrayList();
                    hlist.Add(V.Name, vd);
                    h.Clear();

                    foreach (string val in V.AllowedStringValues)
                    {
                        t = val.ToUpper();
                        t = t.Replace("-", "_");
                        t = t.Replace("+", "_");
                        t = t.Replace(" ", "_");
                        t = t.Replace(":", "_");
                        if (IsNumeric(t[0]) == true) { t = "_" + t; }
                        if (h.ContainsKey(t) == true)
                        {
                            t = val.ToUpper();
                            t = t.Replace("-", "_minus_");
                            t = t.Replace("+", "_plus_");
                            t = t.Replace(" ", "_");
                            t = t.Replace(":", "_colon_");
                        }
                        h.Add(t, t);
                        d = new EnumStruct();
                        d.EnumName = t;
                        d.EnumValue = val;
                        ((VarData)hlist[V.Name]).Enumerations.Add(d);
                    }
                }
            }
            return (hlist);
        }

        /// <summary>
        /// Generates Control Point Implementation
        /// </summary>
        /// <param name="ClassName">Class Name to generate</param>
        /// <param name="ns">Namespace to use</param>
        /// <param name="SavePath">Path to save source</param>
        /// <param name="ServiceURN">Service URN to use</param>
        /// <param name="SCPD_XML">SCPD XML</param>
        public static void GenerateCP(String ClassName, String ns, String SavePath, String ServiceURN, String SCPD_XML)
        {
            string urn = ServiceURN.Substring(0,
                ServiceURN.LastIndexOf(":")) + ":";

            UPnPService s = ServiceGenerator.GenerateServiceFromSCPD(SCPD_XML);
            DText p = new DText();
            ArrayList tlist = new ArrayList();

            String cl = "\r\n";

            StringBuilder cs = new StringBuilder();
            UPnPArgument[] Args;
            UPnPArgument arg;
            UPnPStateVariable[] SV = s.GetStateVariables();
            Hashtable elist = BuildEnumerations(SV);

            cs.Append("using System;\r\n");
            cs.Append("using System.Collections;\r\n");
            cs.Append("using System.Threading;\r\n");
            cs.Append("using OpenSource.Utilities;\r\n");
            cs.Append("using OpenSource.UPnP;" + cl + cl);
            cs.Append("namespace " + ns + cl);
            cs.Append("{\r\n");
            cs.Append("    /// <summary>" + cl);
            cs.Append("    /// Transparent ClientSide UPnP Service" + cl);
            cs.Append("    /// </summary>" + cl);
            cs.Append("    public class " + ClassName + "\r\n");
            cs.Append("    {" + cl);
            cs.Append("       private Hashtable UnspecifiedTable = Hashtable.Synchronized(new Hashtable());\r\n");
            cs.Append("       internal UPnPService _S;\r\n\r\n");
            cs.Append("       public UPnPService GetUPnPService()\r\n");
            cs.Append("       {\r\n");
            cs.Append("            return(_S);\r\n");
            cs.Append("       }\r\n");
            cs.Append("       public static string SERVICE_NAME = \"" + urn + "\";\r\n");
            cs.Append("       public double VERSION\r\n");
            cs.Append("       {\r\n");
            cs.Append("           get\r\n");
            cs.Append("           {\r\n");
            cs.Append("               return(double.Parse(_S.Version));\r\n");
            cs.Append("           }\r\n");
            cs.Append("       }\r\n\r\n");

            foreach (UPnPStateVariable v in SV)
            {
                if (v.SendEvent == true)
                {
                    cs.Append("       public delegate void StateVariableModifiedHandler_" + v.Name + "(" + ClassName + " sender, ");
                    if (elist.ContainsKey(v.Name) == true)
                    {
                        cs.Append("Enum_" + v.Name + " NewValue");
                    }
                    else
                    {
                        cs.Append(v.GetNetType().FullName + " NewValue");
                    }
                    cs.Append(");\r\n");
                    cs.Append("       private WeakEvent OnStateVariable_" + v.Name + "_Event = new WeakEvent();" + cl);
                    cs.Append("       public event StateVariableModifiedHandler_" + v.Name + " OnStateVariable_" + v.Name + "\r\n");
                    cs.Append("       {" + cl);
                    cs.Append("			add{OnStateVariable_" + v.Name + "_Event.Register(value);}" + cl);
                    cs.Append("			remove{OnStateVariable_" + v.Name + "_Event.UnRegister(value);}" + cl);
                    cs.Append("       }" + cl);
                    cs.Append("       protected void " + v.Name + "_ModifiedSink(UPnPStateVariable Var, object NewValue)\r\n");
                    cs.Append("       {\r\n");
                    cs.Append("            OnStateVariable_" + v.Name + "_Event.Fire(this, " + v.Name + ");" + cl);
                    cs.Append("       }\r\n");
                }
            }
            cs.Append("       public delegate void SubscribeHandler(" + ClassName + " sender, bool Success);\r\n");
            cs.Append("       public event SubscribeHandler OnSubscribe;\r\n");

            // Build Events/Delegates
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("       public delegate void Delegate_OnResult_" + A.Name + "(" + ClassName + " sender, ");
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        if (_arg.RelatedStateVar.AllowedStringValues == null)
                        {
                            cs.Append(_arg.RelatedStateVar.GetNetType().FullName + " ");
                        }
                        else
                        {
                            cs.Append("Enum_" + _arg.RelatedStateVar.Name + " ");
                        }
                        cs.Append(_arg.Name + ", ");
                    }
                }
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
                    }
                    else
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name + " ");
                    }
                    cs.Append("ReturnValue, ");
                }
                cs.Append("UPnPInvokeException e, object _Tag);\r\n");
                cs.Append("       private WeakEvent OnResult_" + A.Name + "_Event = new WeakEvent();" + cl);
                cs.Append("       public event Delegate_OnResult_" + A.Name + " OnResult_" + A.Name + "\r\n");
                cs.Append("       {" + cl);
                cs.Append("			add{OnResult_" + A.Name + "_Event.Register(value);}" + cl);
                cs.Append("			remove{OnResult_" + A.Name + "_Event.UnRegister(value);}" + cl);
                cs.Append("       }" + cl);
            }


            // Build Constructor
            cs.Append("\r\n");
            cs.Append("        public " + ClassName + "(UPnPService s)\r\n");
            cs.Append("        {\r\n");
            cs.Append("            _S = s;\r\n");
            cs.Append("            _S.OnSubscribe += new UPnPService.UPnPEventSubscribeHandler(_subscribe_sink);\r\n");
            foreach (UPnPStateVariable v in SV)
            {
                if (v.SendEvent == true)
                {
                    cs.Append("            if (HasStateVariable_" + v.Name + ") _S.GetStateVariableObject(\"" + v.Name + "\").OnModified += new UPnPStateVariable.ModifiedHandler(" + v.Name + "_ModifiedSink);\r\n");
                }
            }
            cs.Append("        }\r\n");
            // Build Dispose
            cs.Append("        public void Dispose()\r\n");
            cs.Append("        {\r\n");
            cs.Append("            _S.OnSubscribe -= new UPnPService.UPnPEventSubscribeHandler(_subscribe_sink);\r\n");
            cs.Append("            OnSubscribe = null;\r\n");

            foreach (UPnPStateVariable v in SV)
            {
                if (v.SendEvent == true)
                {
                    cs.Append("            if (HasStateVariable_" + v.Name + ") _S.GetStateVariableObject(\"" + v.Name + "\").OnModified -= new UPnPStateVariable.ModifiedHandler(" + v.Name + "_ModifiedSink);\r\n");
                }
            }
            cs.Append("        }\r\n");

            cs.Append("        public void _subscribe(int Timeout)\r\n");
            cs.Append("        {\r\n");
            cs.Append("            _S.Subscribe(Timeout, null);\r\n");
            cs.Append("        }\r\n");
            cs.Append("        protected void _subscribe_sink(UPnPService sender, bool OK)\r\n");
            cs.Append("        {\r\n");
            cs.Append("            if (OnSubscribe!=null)\r\n");
            cs.Append("            {\r\n");
            cs.Append("                OnSubscribe(this, OK);\r\n");
            cs.Append("            }\r\n");
            cs.Append("        }\r\n");

            // Build Enumerations
            cs.Append("        public void SetUnspecifiedValue(string EnumType, string val)\r\n");
            cs.Append("        {\r\n");
            cs.Append("            string hash = Thread.CurrentThread.GetHashCode().ToString() + \":\" + EnumType;\r\n");
            cs.Append("            UnspecifiedTable[hash] = val;\r\n");
            cs.Append("        }\r\n");
            cs.Append("        public string GetUnspecifiedValue(string EnumType)\r\n");
            cs.Append("        {\r\n");
            cs.Append("            string hash = Thread.CurrentThread.GetHashCode().ToString() + \":\" + EnumType;\r\n");
            cs.Append("            if (UnspecifiedTable.ContainsKey(hash)==false)\r\n");
            cs.Append("            {\r\n");
            cs.Append("               return(\"\");\r\n");
            cs.Append("            }\r\n");
            cs.Append("            string RetVal = (string)UnspecifiedTable[hash];\r\n");
            cs.Append("            return(RetVal);\r\n");
            cs.Append("        }\r\n");
            IDictionaryEnumerator el = elist.GetEnumerator();
            VarData vd;
            while (el.MoveNext())
            {
                vd = (VarData)el.Value;

                cs.Append("        public string[] Values_" + vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("                UPnPStateVariable sv = _S.GetStateVariableObject(\"" + vd.VariableName + "\");\r\n");
                cs.Append("                return(sv.AllowedStringValues);\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
                cs.Append("        public string Enum_" + vd.VariableName + "_ToString(Enum_" + vd.VariableName + " en)\r\n");
                cs.Append("        {\r\n");
                cs.Append("            string RetVal = \"\";\r\n");
                cs.Append("            switch(en)\r\n");
                cs.Append("            {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("                case Enum_" + vd.VariableName + "." + vs.EnumName + ":\r\n");
                    cs.Append("                    RetVal = \"" + vs.EnumValue + "\";\r\n");
                    cs.Append("                    break;\r\n");
                }
                cs.Append("                case Enum_" + vd.VariableName + "._UNSPECIFIED_:\r\n");
                cs.Append("                    RetVal = GetUnspecifiedValue(\"Enum_" + vd.VariableName + "\");\r\n");
                cs.Append("                    break;\r\n");
                cs.Append("            }\r\n");
                cs.Append("            return(RetVal);\r\n");
                cs.Append("        }\r\n");


                cs.Append("        public enum Enum_" + vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            _UNSPECIFIED_,\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("            " + vs.EnumName + ",\r\n");
                }
                cs.Append("        }\r\n");


                cs.Append("        public Enum_" + vd.VariableName + " " + vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("               Enum_" + vd.VariableName + " RetVal = 0;\r\n");
                cs.Append("               string v = (string)_S.GetStateVariable(\"" + vd.VariableName + "\");\r\n");
                cs.Append("               switch(v)\r\n");
                cs.Append("               {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("                  case \"" + vs.EnumValue + "\":\r\n");
                    cs.Append("                     RetVal = Enum_" + vd.VariableName + "." + vs.EnumName + ";\r\n");
                    cs.Append("                     break;\r\n");
                }
                cs.Append("                  default:\r\n");
                cs.Append("                     RetVal = Enum_" + vd.VariableName + "._UNSPECIFIED_;\r\n");
                cs.Append("                     SetUnspecifiedValue(\"Enum_" + vd.VariableName + "\", v);\r\n");
                cs.Append("                     break;\r\n");
                cs.Append("               }\r\n");
                cs.Append("               return(RetVal);\r\n");
                cs.Append("           }\r\n");
                cs.Append("        }\r\n");
            }

            foreach (UPnPStateVariable V in SV)
            {
                if (elist.ContainsKey(V.Name) == false)
                {
                    // Build Accessors
                    cs.Append("        public " + V.GetNetType().FullName + " " + V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("            get\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("               return((" + V.GetNetType().FullName + ")_S.GetStateVariable(\"" + V.Name + "\"));\r\n");
                    cs.Append("            }\r\n");
                    cs.Append("        }\r\n");
                }
            }

            foreach (UPnPStateVariable V in SV)
            {
                // Range Stuff
                if ((V.Maximum != null) || (V.Minimum != null))
                {
                    cs.Append("        public bool HasMaximum_" + V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("             get\r\n");
                    cs.Append("             {\r\n");
                    cs.Append("                 return(_S.GetStateVariableObject(\"" + V.Name + "\").Maximum!=null);\r\n");
                    cs.Append("             }\r\n");
                    cs.Append("        }\r\n");
                    cs.Append("        public bool HasMinimum_" + V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("             get\r\n");
                    cs.Append("             {\r\n");
                    cs.Append("                 return(_S.GetStateVariableObject(\"" + V.Name + "\").Minimum!=null);\r\n");
                    cs.Append("             }\r\n");
                    cs.Append("        }\r\n");
                    cs.Append("        public " + V.GetNetType().FullName + " Maximum_" + V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("             get\r\n");
                    cs.Append("             {\r\n");
                    cs.Append("                 return((" + V.GetNetType().FullName + ")_S.GetStateVariableObject(\"" + V.Name + "\").Maximum);\r\n");
                    cs.Append("             }\r\n");
                    cs.Append("        }\r\n");
                    cs.Append("        public " + V.GetNetType().FullName + " Minimum_" + V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("             get\r\n");
                    cs.Append("             {\r\n");
                    cs.Append("                 return((" + V.GetNetType().FullName + ")_S.GetStateVariableObject(\"" + V.Name + "\").Minimum);\r\n");
                    cs.Append("             }\r\n");
                    cs.Append("        }\r\n");
                }
                // Has Stuff
                cs.Append("        public bool HasStateVariable_" + V.Name + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("               if (_S.GetStateVariableObject(\"" + V.Name + "\")==null)\r\n");
                cs.Append("               {\r\n");
                cs.Append("                   return(false);\r\n");
                cs.Append("               }\r\n");
                cs.Append("               else\r\n");
                cs.Append("               {\r\n");
                cs.Append("                   return(true);\r\n");
                cs.Append("               }\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
            }
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("        public bool HasAction_" + A.Name + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("               if (_S.GetAction(\"" + A.Name + "\")==null)\r\n");
                cs.Append("               {\r\n");
                cs.Append("                   return(false);\r\n");
                cs.Append("               }\r\n");
                cs.Append("               else\r\n");
                cs.Append("               {\r\n");
                cs.Append("                   return(true);\r\n");
                cs.Append("               }\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
            }

            foreach (UPnPAction A in s.Actions)
            {
                // Build Sync Version
                cs.Append("        public ");
                if (A.HasReturnValue == false)
                {
                    cs.Append("void ");
                }
                else
                {
                    if (elist.ContainsKey(A.GetRetArg().RelatedStateVar.Name) == true)
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name + " ");
                    }
                    else
                    {
                        cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
                    }
                }
                cs.Append("Sync_" + A.Name + "(");
                tlist.Clear();
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        tlist.Add(_arg);
                    }
                }
                Args = (UPnPArgument[])tlist.ToArray(typeof(UPnPArgument));
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.Direction == "out")
                    {
                        cs.Append("out ");
                    }
                    if (elist.ContainsKey(arg.RelatedStateVar.Name))
                    {
                        cs.Append("Enum_" + arg.RelatedStateVar.Name + " ");
                    }
                    else
                    {
                        cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                    }
                    cs.Append(arg.Name);
                    if (i < Args.Length - 1)
                    {
                        cs.Append(", ");
                    }
                }
                cs.Append(")\r\n");
                cs.Append("        {\r\n");
                cs.Append("           UPnPArgument[] args = new UPnPArgument[");
                if (A.HasReturnValue == false)
                {
                    cs.Append(A.ArgumentList.Length.ToString());
                }
                else
                {
                    cs.Append((A.ArgumentList.Length - 1).ToString());
                }
                cs.Append("];\r\n");
                int ia = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        if (_arg.Direction == "in")
                        {
                            if (elist.ContainsKey(_arg.RelatedStateVar.Name) == true)
                            {
                                cs.Append("           switch(" + _arg.Name + ")\r\n");
                                cs.Append("           {\r\n");
                                foreach (EnumStruct ess in ((VarData)elist[_arg.RelatedStateVar.Name]).Enumerations)
                                {
                                    cs.Append("               case Enum_" + _arg.RelatedStateVar.Name + "." + ess.EnumName + ":\r\n");
                                    cs.Append("                   args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", \"" + ess.EnumValue + "\");\r\n");
                                    cs.Append("                   break;\r\n");
                                }
                                cs.Append("               default:\r\n");
                                cs.Append("                  args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", GetUnspecifiedValue(\"Enum_" + _arg.RelatedStateVar.Name + "\"));\r\n");
                                cs.Append("                  break;\r\n");
                                cs.Append("           }\r\n");
                            }
                            else
                            {
                                cs.Append("           args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", " + _arg.Name + ");\r\n");
                            }
                        }
                        else
                        {
                            cs.Append("           args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", \"\");\r\n");
                        }
                        ++ia;
                    }
                }
                if (A.HasReturnValue == true)
                {
                    cs.Append("           object RetVal = null;\r\n");
                    cs.Append("           RetVal = ");
                }
                else
                {
                    cs.Append("            ");
                }
                cs.Append("_S.InvokeSync(\"" + A.Name + "\", args);\r\n");
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues != null)
                    {
                        cs.Append("           Enum_" + A.GetRetArg().RelatedStateVar.Name + " RetEnum = 0;\r\n");
                        cs.Append("           switch((string)RetVal)\r\n");
                        cs.Append("           {\r\n");
                        foreach (EnumStruct ess in ((VarData)elist[A.GetRetArg().RelatedStateVar.Name]).Enumerations)
                        {
                            cs.Append("               case \"" + ess.EnumValue + "\":\r\n");
                            cs.Append("                   RetEnum = Enum_" + A.GetRetArg().RelatedStateVar.Name + "." + ess.EnumName + ";\r\n");
                            cs.Append("                   break;\r\n");
                        }
                        cs.Append("               default:\r\n");
                        cs.Append("                   RetEnum = Enum_" + A.GetRetArg().RelatedStateVar.Name + "._UNSPECIFIED_;\r\n");
                        cs.Append("                   SetUnspecifiedValue(\"Enum_" + A.GetRetArg().RelatedStateVar.Name + "\",(string)RetVal);\r\n");
                        cs.Append("                   break;\r\n");
                        cs.Append("           }\r\n");
                    }
                }

                // Check if this is necessary
                bool IsNeccessary = false;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if ((_arg.RelatedStateVar.AllowedStringValues != null) &&
                        (_arg.IsReturnValue == false))
                    {
                        IsNeccessary = true;
                        break;
                    }
                }
                if (IsNeccessary == true)
                {
                    cs.Append("            for(int i=0;i<args.Length;++i)\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("                switch(args[i].Name)\r\n");
                    cs.Append("                {\r\n");
                    tlist.Clear();
                    foreach (UPnPArgument _arg in A.ArgumentList)
                    {
                        if (_arg.IsReturnValue == false)
                        {
                            tlist.Add(_arg);
                        }
                    }

                    Args = (UPnPArgument[])tlist.ToArray(typeof(UPnPArgument));
                    foreach (UPnPArgument _arg in Args)
                    {
                        if (_arg.RelatedStateVar.AllowedStringValues != null)
                        {
                            cs.Append("                    case \"" + _arg.Name + "\":\r\n");
                            cs.Append("                        switch((string)args[i].DataValue)\r\n");
                            cs.Append("                        {\r\n");
                            foreach (EnumStruct ess in ((VarData)elist[_arg.RelatedStateVar.Name]).Enumerations)
                            {
                                cs.Append("                            case \"" + ess.EnumValue + "\":\r\n");
                                cs.Append("                                args[i].DataValue = Enum_" + _arg.RelatedStateVar.Name + "." + ess.EnumName + ";\r\n");
                                cs.Append("                                break;\r\n");
                            }
                            cs.Append("                            default:\r\n");
                            cs.Append("                               SetUnspecifiedValue(\"Enum_" + _arg.RelatedStateVar.Name + "\", (string)args[i].DataValue);\r\n");
                            cs.Append("                               args[i].DataValue = Enum_" + _arg.RelatedStateVar.Name + "._UNSPECIFIED_;\r\n");
                            cs.Append("                               break;\r\n");
                            cs.Append("                        }\r\n");
                            cs.Append("                        break;\r\n");
                        }
                    }
                    cs.Append("                }\r\n"); // End of Switch
                    cs.Append("            }\r\n"); // End of For Loop

                }
                int argid = 0;
                foreach (UPnPArgument _arg in A.Arguments)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        cs.Append("            " + _arg.Name + " = (");
                        if (_arg.RelatedStateVar.AllowedStringValues == null)
                        {
                            cs.Append(_arg.RelatedStateVar.GetNetType().FullName);
                        }
                        else
                        {
                            cs.Append("Enum_" + _arg.RelatedStateVar.Name);
                        }
                        cs.Append(") args[" + argid.ToString() + "].DataValue;\r\n");
                    }
                    ++argid;
                }
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append("           return((" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + ")RetVal);\r\n");
                    }
                    else
                    {
                        cs.Append("           return(RetEnum);\r\n");
                    }
                }
                else
                {
                    cs.Append("            return;\r\n");
                }
                cs.Append("        }\r\n"); // End of SyncAction Class

                // Build Async Version [Overload]
                cs.Append("        public ");
                cs.Append("void ");
                cs.Append(A.Name + "(");
                tlist.Clear();
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if ((_arg.IsReturnValue == false) &&
                        (_arg.Direction == "in"))
                    {
                        tlist.Add(_arg);
                    }
                }
                Args = (UPnPArgument[])tlist.ToArray(typeof(UPnPArgument));
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (elist.ContainsKey(arg.RelatedStateVar.Name))
                    {
                        cs.Append("Enum_" + arg.RelatedStateVar.Name + " ");
                    }
                    else
                    {
                        cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                    }
                    cs.Append(arg.Name);
                    if (i < Args.Length - 1)
                    {
                        cs.Append(", ");
                    }
                }
                cs.Append(")\r\n");
                cs.Append("        {\r\n");
                cs.Append("            " + A.Name + "(");
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    cs.Append(arg.Name);
                    cs.Append(", ");
                }
                cs.Append("null, null);\r\n");

                cs.Append("        }\r\n");

                // Build Async Version
                cs.Append("        public ");
                cs.Append("void ");
                cs.Append(A.Name + "(");
                tlist.Clear();
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if ((_arg.IsReturnValue == false) &&
                        (_arg.Direction == "in"))
                    {
                        tlist.Add(_arg);
                    }
                }
                Args = (UPnPArgument[])tlist.ToArray(typeof(UPnPArgument));
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (elist.ContainsKey(arg.RelatedStateVar.Name))
                    {
                        cs.Append("Enum_" + arg.RelatedStateVar.Name + " ");
                    }
                    else
                    {
                        cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                    }
                    cs.Append(arg.Name);
                    cs.Append(", ");

                }
                cs.Append("object _Tag, Delegate_OnResult_" + A.Name + " _Callback");
                cs.Append(")\r\n");
                cs.Append("        {\r\n");
                cs.Append("           UPnPArgument[] args = new UPnPArgument[");
                if (A.HasReturnValue == false)
                {
                    cs.Append(A.ArgumentList.Length.ToString());
                }
                else
                {
                    cs.Append((A.ArgumentList.Length - 1).ToString());
                }
                cs.Append("];\r\n");

                ia = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        if (_arg.Direction == "in")
                        {
                            if (elist.ContainsKey(_arg.RelatedStateVar.Name) == true)
                            {
                                cs.Append("           switch(" + _arg.Name + ")\r\n");
                                cs.Append("           {\r\n");
                                foreach (EnumStruct ess in ((VarData)elist[_arg.RelatedStateVar.Name]).Enumerations)
                                {
                                    cs.Append("               case Enum_" + _arg.RelatedStateVar.Name + "." + ess.EnumName + ":\r\n");
                                    cs.Append("                   args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", \"" + ess.EnumValue + "\");\r\n");
                                    cs.Append("                   break;\r\n");
                                }
                                cs.Append("               default:\r\n");
                                cs.Append("                  args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", GetUnspecifiedValue(\"Enum_" + _arg.RelatedStateVar.Name + "\"));\r\n");
                                cs.Append("                  break;\r\n");
                                cs.Append("           }\r\n");
                            }
                            else
                            {
                                cs.Append("           args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", " + _arg.Name + ");\r\n");
                            }
                        }
                        else
                        {
                            cs.Append("           args[" + ia.ToString() + "] = new UPnPArgument(\"" + _arg.Name + "\", \"\");\r\n");
                        }
                        ++ia;
                    }
                }
                cs.Append("           _S.InvokeAsync(\"" + A.Name + "\", args, new object[2]{_Tag,_Callback}, new UPnPService.UPnPServiceInvokeHandler(Sink_" + A.Name + "), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_" + A.Name + "));\r\n");
                cs.Append("        }\r\n"); // End of Action Class

                // Build Async Sink
                cs.Append("        private void Sink_" + A.Name + "(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)\r\n");
                cs.Append("        {\r\n");
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues != null)
                    {
                        cs.Append("           Enum_" + A.GetRetArg().RelatedStateVar.Name + " RetEnum = 0;\r\n");
                        cs.Append("           switch((string)RetVal)\r\n");
                        cs.Append("           {\r\n");
                        foreach (EnumStruct ess in ((VarData)elist[A.GetRetArg().RelatedStateVar.Name]).Enumerations)
                        {
                            cs.Append("               case \"" + ess.EnumValue + "\":\r\n");
                            cs.Append("                   RetEnum = Enum_" + A.GetRetArg().RelatedStateVar.Name + "." + ess.EnumName + ";\r\n");
                            cs.Append("                   break;\r\n");
                        }
                        cs.Append("               default:\r\n");
                        cs.Append("                   RetEnum = Enum_" + A.GetRetArg().RelatedStateVar.Name + "._UNSPECIFIED_;\r\n");
                        cs.Append("                   SetUnspecifiedValue(\"Enum_" + A.GetRetArg().RelatedStateVar.Name + "\",(string)RetVal);\r\n");
                        cs.Append("                   break;\r\n");
                        cs.Append("           }\r\n");
                    }
                }

                // Check if this is necessary
                IsNeccessary = false;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if ((_arg.RelatedStateVar.AllowedStringValues != null) &&
                        (_arg.IsReturnValue == false))
                    {
                        IsNeccessary = true;
                        break;
                    }
                }

                if (IsNeccessary == true)
                {
                    cs.Append("            for(int i=0;i<Args.Length;++i)\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("                switch(Args[i].Name)\r\n");
                    cs.Append("                {\r\n");
                    tlist.Clear();
                    foreach (UPnPArgument _arg in A.ArgumentList)
                    {
                        if (_arg.IsReturnValue == false)
                        {
                            tlist.Add(_arg);
                        }
                    }

                    Args = (UPnPArgument[])tlist.ToArray(typeof(UPnPArgument));
                    foreach (UPnPArgument _arg in Args)
                    {
                        if (_arg.RelatedStateVar.AllowedStringValues != null)
                        {
                            cs.Append("                    case \"" + _arg.Name + "\":\r\n");
                            cs.Append("                        switch((string)Args[i].DataValue)\r\n");
                            cs.Append("                        {\r\n");
                            foreach (EnumStruct ess in ((VarData)elist[_arg.RelatedStateVar.Name]).Enumerations)
                            {
                                cs.Append("                            case \"" + ess.EnumValue + "\":\r\n");
                                cs.Append("                                Args[i].DataValue = Enum_" + _arg.RelatedStateVar.Name + "." + ess.EnumName + ";\r\n");
                                cs.Append("                                break;\r\n");
                            }
                            cs.Append("                            default:\r\n");
                            cs.Append("                               SetUnspecifiedValue(\"Enum_" + _arg.RelatedStateVar.Name + "\", (string)Args[i].DataValue);\r\n");
                            cs.Append("                               Args[i].DataValue = Enum_" + _arg.RelatedStateVar.Name + "._UNSPECIFIED_;\r\n");
                            cs.Append("                               break;\r\n");
                            cs.Append("                        }\r\n");
                            cs.Append("                        break;\r\n");
                        }
                    }
                    cs.Append("                }\r\n"); // End of Switch
                    cs.Append("            }\r\n"); // End of For Loop
                }
                cs.Append("            object[] StateInfo = (object[])_Tag;\r\n");
                cs.Append("            if (StateInfo[1]!=null)\r\n");
                cs.Append("            {\r\n");
                cs.Append("                ((Delegate_OnResult_" + A.Name + ")StateInfo[1])(this, ");
                argid = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        cs.Append("(");
                        if (_arg.RelatedStateVar.AllowedStringValues == null)
                        {
                            cs.Append(_arg.RelatedStateVar.GetNetType().FullName + " ");
                        }
                        else
                        {
                            cs.Append("Enum_" + _arg.RelatedStateVar.Name + " ");
                        }
                        cs.Append(")");
                        cs.Append("Args[" + argid.ToString() + "].DataValue, ");
                        ++argid;
                    }
                }
                if (A.HasReturnValue == true)
                {
                    cs.Append("(");
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName);
                        cs.Append(")");
                        cs.Append("RetVal, ");
                    }
                    else
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name);
                        cs.Append(")");
                        cs.Append("RetEnum, ");
                    }
                }
                cs.Append("null, StateInfo[0]);\r\n");
                cs.Append("            }\r\n");
                cs.Append("            else\r\n");
                cs.Append("            {\r\n");
                cs.Append("                OnResult_" + A.Name + "_Event.Fire(this, ");
                argid = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        cs.Append("(");
                        if (_arg.RelatedStateVar.AllowedStringValues == null)
                        {
                            cs.Append(_arg.RelatedStateVar.GetNetType().FullName + " ");
                        }
                        else
                        {
                            cs.Append("Enum_" + _arg.RelatedStateVar.Name + " ");
                        }
                        cs.Append(")");
                        cs.Append("Args[" + argid.ToString() + "].DataValue, ");
                        ++argid;
                    }
                }
                if (A.HasReturnValue == true)
                {
                    cs.Append("(");
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName);
                        cs.Append(")");
                        cs.Append("RetVal, ");
                    }
                    else
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name);
                        cs.Append(")");
                        cs.Append("RetEnum, ");
                    }
                }
                cs.Append("null, StateInfo[0]);\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");

                //Build Error Sink
                cs.Append("        private void Error_Sink_" + A.Name + "(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)\r\n");
                cs.Append("        {\r\n");

                // Check if this is necessary
                IsNeccessary = false;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if ((_arg.RelatedStateVar.AllowedStringValues != null) &&
                        (_arg.Direction == "in"))
                    {
                        IsNeccessary = true;
                        break;
                    }
                }
                if (IsNeccessary == true)
                {
                    cs.Append("            for(int i=0;i<Args.Length;++i)\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("                switch(Args[i].Name)\r\n");
                    cs.Append("                {\r\n");
                    foreach (UPnPArgument _arg in A.ArgumentList)
                    {
                        if (_arg.IsReturnValue == false)
                        {
                            if (_arg.RelatedStateVar.AllowedStringValues != null)
                            {
                                cs.Append("                    case \"" + _arg.Name + "\":\r\n");
                                cs.Append("                        switch((string)Args[i].DataValue)\r\n");
                                cs.Append("                        {\r\n");
                                foreach (EnumStruct ess in ((VarData)elist[_arg.RelatedStateVar.Name]).Enumerations)
                                {
                                    cs.Append("                            case \"" + ess.EnumValue + "\":\r\n");
                                    cs.Append("                                Args[i].DataValue = Enum_" + _arg.RelatedStateVar.Name + "." + ess.EnumName + ";\r\n");
                                    cs.Append("                                break;\r\n");
                                }
                                cs.Append("                        }\r\n");

                                cs.Append("                        break;\r\n");
                            }
                        }
                    }
                    cs.Append("                }\r\n"); // End of Switch
                    cs.Append("            }\r\n"); // End of For Loop
                }
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues != null)
                    {
                        cs.Append("            SetUnspecifiedValue(\"Enum_" + A.GetRetArg().RelatedStateVar.Name + "\",\"* error *\");\r\n");
                    }
                }
                cs.Append("            object[] StateInfo = (object[])_Tag;\r\n");
                cs.Append("            if (StateInfo[1]!=null)\r\n");
                cs.Append("            {\r\n");
                cs.Append("                ((Delegate_OnResult_" + A.Name + ")StateInfo[1])(this, ");
                argid = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        if (_arg.Direction == "in")
                        {
                            cs.Append("(");
                            if (_arg.RelatedStateVar.AllowedStringValues == null)
                            {
                                cs.Append(_arg.RelatedStateVar.GetNetType().FullName + " ");
                            }
                            else
                            {
                                cs.Append("Enum_" + _arg.RelatedStateVar.Name + " ");
                            }
                            cs.Append(")");
                            cs.Append("Args[" + argid.ToString() + "].DataValue, ");
                        }
                        else
                        {
                            if (_arg.RelatedStateVar.AllowedStringValues != null)
                            {
                                cs.Append("(Enum_" + _arg.RelatedStateVar.Name + ")0, ");
                            }
                            else
                            {
                                cs.Append("(" + _arg.RelatedStateVar.GetNetType().FullName + ")UPnPService.CreateObjectInstance(typeof(" + _arg.RelatedStateVar.GetNetType().FullName + "),null), ");
                            }
                        }
                        ++argid;
                    }
                }
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append("(" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + ")UPnPService.CreateObjectInstance(typeof(" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + "),null), ");
                    }
                    else
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name + "._UNSPECIFIED_, ");
                    }
                }
                cs.Append("e, StateInfo[0]);\r\n");
                cs.Append("            }\r\n");



                cs.Append("            else\r\n");
                cs.Append("            {\r\n");
                cs.Append("                OnResult_" + A.Name + "_Event.Fire(this, ");
                argid = 0;
                foreach (UPnPArgument _arg in A.ArgumentList)
                {
                    if (_arg.IsReturnValue == false)
                    {
                        if (_arg.Direction == "in")
                        {
                            cs.Append("(");
                            if (_arg.RelatedStateVar.AllowedStringValues == null)
                            {
                                cs.Append(_arg.RelatedStateVar.GetNetType().FullName + " ");
                            }
                            else
                            {
                                cs.Append("Enum_" + _arg.RelatedStateVar.Name + " ");
                            }
                            cs.Append(")");
                            cs.Append("Args[" + argid.ToString() + "].DataValue, ");
                        }
                        else
                        {
                            if (_arg.RelatedStateVar.AllowedStringValues != null)
                            {
                                cs.Append("(Enum_" + _arg.RelatedStateVar.Name + ")0, ");
                            }
                            else
                            {
                                cs.Append("(" + _arg.RelatedStateVar.GetNetType().FullName + ")UPnPService.CreateObjectInstance(typeof(" + _arg.RelatedStateVar.GetNetType().FullName + "),null), ");
                            }
                        }
                        ++argid;
                    }
                }
                if (A.HasReturnValue == true)
                {
                    if (A.GetRetArg().RelatedStateVar.AllowedStringValues == null)
                    {
                        cs.Append("(" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + ")UPnPService.CreateObjectInstance(typeof(" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + "),null), ");
                    }
                    else
                    {
                        cs.Append("Enum_" + A.GetRetArg().RelatedStateVar.Name + "._UNSPECIFIED_, ");
                    }
                }
                cs.Append("e, StateInfo[0]);\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");

                // End of Error Sink
            }

            cs.Append("    }\r\n");
            cs.Append("}");

            UTF8Encoding UTF8 = new UTF8Encoding();
            FileStream ws = new FileStream(SavePath, FileMode.Create, FileAccess.Write);
            byte[] buffer = UTF8.GetBytes(cs.ToString());
            ws.Write(buffer, 0, buffer.Length);
            ws.Close();
        }

        /// <summary>
        /// Generates Device side implementation from SCPD XML
        /// </summary>
        /// <param name="ClassName">Class Name to build</param>
        /// <param name="ns">Namespace to use</param>
        /// <param name="SavePath">Path to save source</param>
        /// <param name="ServiceID">Service ID to use</param>
        /// <param name="ServiceURN">Service URN to use</param>
        /// <param name="SCPD_XML">SCPD XML String</param>
        public static void Generate(String ClassName, String ns, String SavePath, String ServiceID, String ServiceURN, String SCPD_XML)
        {
            UPnPService s = ServiceGenerator.GenerateServiceFromSCPD(SCPD_XML);
            UPnPDebugObject dobj = new UPnPDebugObject(s);

            int ServiceURversion = 1;
            string[] xx = ServiceURN.Split(':');
            if (xx.Length > 2) int.TryParse(xx[xx.Length - 1], out ServiceURversion);

            DText p = new DText();
            p.ATTRMARK = ":";
            p[0] = ServiceURN;

            string v = p[p.DCOUNT()];
            dobj.InvokeNonStaticMethod("SetVersion", new Object[1] { v });
            String cl = "\r\n";

            StringBuilder cs = new StringBuilder();
            UPnPArgument[] Args;
            UPnPArgument arg;
            UPnPStateVariable[] SV = s.GetStateVariables();

            cs.Append("using OpenSource.UPnP;" + cl + cl);
            cs.Append("namespace " + ns + cl);
            cs.Append("{\r\n");
            cs.Append("    /// <summary>" + cl);
            cs.Append("    /// Transparent DeviceSide UPnP Service" + cl);
            cs.Append("    /// </summary>" + cl);
            cs.Append("    public class " + ClassName + " : IUPnPService" + cl);
            cs.Append("    {" + cl + cl);
            cs.Append("        // Place your declarations above this line\r\n");
            cs.Append("\r\n");
            cs.Append("        #region AutoGenerated Code Section [Do NOT Modify, unless you know what you're doing]\r\n");
            cs.Append("        //{{{{{ Begin Code Block\r\n");
            cs.Append("\r\n");
            cs.Append("        private _" + ClassName + " _S;\r\n");
            cs.Append("        public static string URN = \"" + ServiceURN + "\";\r\n");
            cs.Append("        public double VERSION\r\n");
            cs.Append("        {\r\n");
            cs.Append("           get\r\n");
            cs.Append("           {\r\n");
            cs.Append("               return(double.Parse(_S.GetUPnPService().Version));\r\n");
            cs.Append("           }\r\n");
            cs.Append("        }\r\n\r\n");

            // Build Enumerations
            Hashtable elist = BuildEnumerations(SV);
            IDictionaryEnumerator el = elist.GetEnumerator();
            VarData vd;
            while (el.MoveNext())
            {
                vd = (VarData)el.Value;
                cs.Append("        public enum Enum_" + vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("            " + vs.EnumName + ",\r\n");
                }
                cs.Append("        }\r\n");

                cs.Append("        public Enum_" + vd.VariableName + " ");
                if (s.GetStateVariableObject(vd.VariableName).SendEvent == true)
                {
                    cs.Append("Evented_");
                }
                cs.Append(vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            set\r\n");
                cs.Append("            {\r\n");
                cs.Append("               string v = \"\";\r\n");
                cs.Append("               switch(value)\r\n");
                cs.Append("               {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("                  case Enum_" + vd.VariableName + "." + vs.EnumName + ":\r\n");
                    cs.Append("                     v = \"" + vs.EnumValue + "\";\r\n");
                    cs.Append("                     break;\r\n");
                }
                cs.Append("               }\r\n");
                cs.Append("               _S.SetStateVariable(\"" + vd.VariableName + "\",v);\r\n");
                cs.Append("            }\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("               Enum_" + vd.VariableName + " RetVal = 0;\r\n");
                cs.Append("               string v = (string)_S.GetStateVariable(\"" + vd.VariableName + "\");\r\n");
                cs.Append("               switch(v)\r\n");
                cs.Append("               {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("                  case \"" + vs.EnumValue + "\":\r\n");
                    cs.Append("                     RetVal = Enum_" + vd.VariableName + "." + vs.EnumName + ";\r\n");
                    cs.Append("                     break;\r\n");
                }
                cs.Append("               }\r\n");
                cs.Append("               return(RetVal);\r\n");
                cs.Append("           }\r\n");
                cs.Append("        }\r\n");
            }

            el.Reset();
            while (el.MoveNext())
            {
                vd = (VarData)el.Value;
                cs.Append("        static public string Enum_" + vd.VariableName + "_ToString(Enum_" + vd.VariableName + " en)\r\n");
                cs.Append("        {\r\n");
                cs.Append("            string RetVal = \"\";\r\n");
                cs.Append("            switch(en)\r\n");
                cs.Append("            {\r\n");
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    cs.Append("                case Enum_" + vd.VariableName + "." + vs.EnumName + ":\r\n");
                    cs.Append("                    RetVal = \"" + vs.EnumValue + "\";\r\n");
                    cs.Append("                    break;\r\n");
                }
                cs.Append("            }\r\n");
                cs.Append("            return(RetVal);\r\n");
                cs.Append("        }\r\n");

                // Build Easy Way to get All Values
                cs.Append("        static public string[] Values_" + vd.VariableName + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("                string[] RetVal = new string[" + vd.Enumerations.Count.ToString() + "]{");
                string EasyStrings = "";
                foreach (EnumStruct vs in vd.Enumerations)
                {
                    if (EasyStrings == "")
                    {
                        EasyStrings = "\"" + vs.EnumValue + "\"";
                    }
                    else
                    {
                        EasyStrings = "\"" + vs.EnumValue + "\"," + EasyStrings;
                    }
                }
                cs.Append(EasyStrings + "};\r\n");
                cs.Append("                return(RetVal);\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
            }

            cs.Append("        public delegate void OnStateVariableModifiedHandler(" + ClassName + " sender);\r\n");
            foreach (UPnPStateVariable V in SV)
            {
                cs.Append("        public event OnStateVariableModifiedHandler OnStateVariableModified_" + V.Name + ";\r\n");
            }

            foreach (UPnPStateVariable V in SV)
            {
                if (elist.ContainsKey(V.Name) == false)
                {
                    // Build Accessors
                    cs.Append("        public " + V.GetNetType().FullName + " ");
                    if (V.SendEvent == true)
                    {
                        cs.Append("Evented_");
                    }
                    cs.Append(V.Name + "\r\n");
                    cs.Append("        {\r\n");
                    cs.Append("            get\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("               return((" + V.GetNetType().FullName + ")_S.GetStateVariable(\"" + V.Name + "\"));\r\n");
                    cs.Append("            }\r\n");
                    cs.Append("            set\r\n");
                    cs.Append("            {\r\n");
                    cs.Append("               _S.SetStateVariable(\"" + V.Name + "\", value);\r\n");
                    cs.Append("            }\r\n");
                    cs.Append("        }\r\n");
                }
            }
            foreach (UPnPStateVariable V in SV)
            {
                cs.Append("        public UPnPModeratedStateVariable.IAccumulator Accumulator_");
                cs.Append(V.Name + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("                 return(((UPnPModeratedStateVariable)_S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\")).Accumulator);\r\n");
                cs.Append("            }\r\n");
                cs.Append("            set\r\n");
                cs.Append("            {\r\n");
                cs.Append("                 ((UPnPModeratedStateVariable)_S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\")).Accumulator = value;\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
                cs.Append("        public double ModerationDuration_" + V.Name + "\r\n");
                cs.Append("        {\r\n");
                cs.Append("            get\r\n");
                cs.Append("            {\r\n");
                cs.Append("                 return(((UPnPModeratedStateVariable)_S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\")).ModerationPeriod);\r\n");
                cs.Append("            }\r\n");
                cs.Append("            set\r\n");
                cs.Append("            {\r\n");
                cs.Append("                 ((UPnPModeratedStateVariable)_S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\")).ModerationPeriod = value;\r\n");
                cs.Append("            }\r\n");
                cs.Append("        }\r\n");
            }

            // Build MethodDelegates
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("        public delegate ");
                if (A.HasReturnValue == false)
                {
                    cs.Append("void ");
                }
                else
                {
                    cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
                }
                cs.Append("Delegate_" + A.Name + "(");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("out ");
                        }
                        if (arg.RelatedStateVar.AllowedStringValues == null)
                        {
                            cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                        }
                        else
                        {
                            cs.Append(ClassName + ".Enum_" + arg.RelatedStateVar.Name + " ");
                        }
                        cs.Append(arg.Name);
                        if (i < Args.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                }
                cs.Append(");\r\n");
            }

            // Build Overriding Delegates
            cs.Append("\r\n");
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("        public Delegate_" + A.Name + " External_" + A.Name + " = null;\r\n");
            }
            cs.Append("\r\n");

            // Build Ability to remove Optional State Variables
            foreach (UPnPStateVariable V in s.GetStateVariables())
            {
                cs.Append("        public void RemoveStateVariable_" + V.Name + "()\r\n");
                cs.Append("        {\r\n");
                cs.Append("            _S.GetUPnPService().RemoveStateVariable(_S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\"));\r\n");
                cs.Append("        }\r\n");
            }

            // Build Ability to remove Optional Actions
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("        public void RemoveAction_" + A.Name + "()\r\n");
                cs.Append("        {\r\n");
                cs.Append("             _S.GetUPnPService().RemoveMethod(\"" + A.Name + "\");\r\n");
                cs.Append("        }\r\n");
            }
            // Add Helper Methods
            cs.Append("        public System.Net.IPEndPoint GetCaller()\r\n");
            cs.Append("        {\r\n");
            cs.Append("             return(_S.GetUPnPService().GetCaller());\r\n");
            cs.Append("        }\r\n");
            cs.Append("        public System.Net.IPEndPoint GetReceiver()\r\n");
            cs.Append("        {\r\n");
            cs.Append("             return(_S.GetUPnPService().GetReceiver());\r\n");
            cs.Append("        }\r\n");
            cs.Append("\r\n");

            // Build Private Class
            cs.Append("        private class _" + ClassName + "\r\n");
            cs.Append("        {\r\n");
            cs.Append("            private " + ClassName + " Outer = null;\r\n");
            cs.Append("            private UPnPService S;\r\n");
            cs.Append("            internal _" + ClassName + "(" + ClassName + " n)\r\n");
            cs.Append("            {\r\n");
            cs.Append("                Outer = n;\r\n");
            cs.Append("                S = BuildUPnPService();\r\n");
            cs.Append("            }\r\n");
            cs.Append("            public UPnPService GetUPnPService()\r\n");
            cs.Append("            {\r\n");
            cs.Append("                return(S);\r\n");
            cs.Append("            }\r\n");
            cs.Append("            public void SetStateVariable(string VarName, object VarValue)\r\n");
            cs.Append("            {\r\n");
            cs.Append("               S.SetStateVariable(VarName,VarValue);\r\n");
            cs.Append("            }\r\n");
            cs.Append("            public object GetStateVariable(string VarName)\r\n");
            cs.Append("            {\r\n");
            cs.Append("               return(S.GetStateVariable(VarName));\r\n");
            cs.Append("            }\r\n");
            cs.Append("            protected UPnPService BuildUPnPService()\r\n");
            cs.Append("            {\r\n");
            cs.Append("                UPnPStateVariable[] RetVal = new UPnPStateVariable[" + SV.Length.ToString() + "];\r\n");
            for (int i = 0; i < SV.Length; ++i)
            {
                cs.Append("                RetVal[" + i.ToString() + "] = new UPnPModeratedStateVariable(\"" + SV[i].Name + "\", typeof(" + SV[i].GetNetType().FullName + "), " + SV[i].SendEvent.ToString().ToLower() + ");\r\n");

                if ((SV[i].Maximum != null) ||
                    (SV[i].Minimum != null) ||
                    (SV[i].Step != null))
                {
                    cs.Append("                RetVal[" + i.ToString() + "].SetRange(");
                    if (SV[i].Minimum == null)
                    {
                        cs.Append("null");
                    }
                    else
                    {
                        cs.Append("(" + SV[i].Minimum.GetType().FullName + ")(" + SV[i].Minimum.ToString() + ")");
                    }
                    cs.Append(",");

                    if (SV[i].Maximum == null)
                    {
                        cs.Append("null");
                    }
                    else
                    {
                        cs.Append("(" + SV[i].Maximum.GetType().FullName + ")(" + SV[i].Maximum.ToString() + ")");
                    }
                    cs.Append(",");

                    if (SV[i].Step == null)
                    {
                        cs.Append("null");
                    }
                    else
                    {
                        cs.Append("(" + SV[i].Step.GetType().FullName + ")" + SV[i].Step.ToString());
                    }
                    cs.Append(");\r\n");
                }

                if (SV[i].DefaultValue != null)
                {
                    cs.Append("                RetVal[" + i.ToString() + "].DefaultValue = UPnPService.CreateObjectInstance(typeof(" + SV[i].GetNetType().FullName + "),\"" + UPnPService.SerializeObjectInstance(SV[i].DefaultValue) + "\");\r\n");
                    //cs.Append("                RetVal[" + i.ToString() + "].DefaultValue = (" + SV[i].GetNetType().FullName + ")(\"" + UPnPService.SerializeObjectInstance(SV[i].DefaultValue) + "\";\r\n");
                }
                if (SV[i].AllowedStringValues != null)
                {
                    cs.Append("                RetVal[" + i.ToString() + "].AllowedStringValues = new string[" +
                        SV[i].AllowedStringValues.Length.ToString() + "]{");
                    for (int ai = 0; ai < SV[i].AllowedStringValues.Length; ++ai)
                    {
                        cs.Append("\"" + SV[i].AllowedStringValues[ai] + "\"");
                        if (ai < SV[i].AllowedStringValues.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                    cs.Append("};\r\n");
                }

                System.Collections.IList e = s.Actions;
                foreach (UPnPAction A in e)
                {
                    foreach (UPnPArgument ag in A.ArgumentList)
                    {
                        if (ag.RelatedStateVar.Name == SV[i].Name)
                        {
                            cs.Append("                RetVal[" + i.ToString() + "].AddAssociation(\"" + A.Name + "\", \"" + ag.Name + "\");\r\n");
                        }
                    }
                }
            }
            // Build UPnPService
            cs.Append("\r\n");
            cs.Append("                UPnPService S = new UPnPService(" +
                ServiceURversion.ToString() + ", \"" + ServiceID + "\", URN, true, this);\r\n");           // WAS: s.Version + ", \"" + ServiceID + "\", \"" + ServiceURN + "\", true, this);\r\n");
            cs.Append("                for(int i=0;i<RetVal.Length;++i)\r\n");
            cs.Append("                {\r\n");
            cs.Append("                   S.AddStateVariable(RetVal[i]);\r\n");
            cs.Append("                }\r\n");
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("                S.AddMethod(\"" + A.Name + "\");\r\n");
            }

            cs.Append("                return(S);\r\n");
            cs.Append("            }\r\n\r\n");

            System.Collections.IList ee = s.Actions;
            foreach (UPnPAction A in ee)
            {
                if (A.HasReturnValue)
                {
                    cs.Append("            [OpenSource.UPnP.ReturnArgument(\"" + A.GetRetArg().Name + "\")]" + cl);
                }
                cs.Append("            public ");
                if (A.HasReturnValue == false)
                {
                    cs.Append("void ");
                }
                else
                {
                    cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
                }

                cs.Append(A.Name + "(");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("out ");
                        }
                        cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                        cs.Append(arg.Name);
                        if (i < Args.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                }
                cs.Append(")" + cl);
                cs.Append("            {\r\n");

                // Convert to Enum if neccessary
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if ((arg.IsReturnValue == false) &&
                    (arg.RelatedStateVar.AllowedStringValues != null))
                    {
                        cs.Append("                Enum_" + arg.RelatedStateVar.Name + " e_" + arg.Name + ";\r\n");
                        if (arg.Direction == "in")
                        {
                            cs.Append("                switch(" + arg.Name + ")\r\n");
                            cs.Append("                {\r\n");
                            vd = (VarData)elist[arg.RelatedStateVar.Name];
                            foreach (EnumStruct ss in vd.Enumerations)
                            {
                                cs.Append("                    case \"" + ss.EnumValue + "\":\r\n");
                                cs.Append("                        e_" + arg.Name + " = Enum_" + arg.RelatedStateVar.Name + "." + ss.EnumName + ";\r\n");
                                cs.Append("                        break;\r\n");
                            }
                            cs.Append("                    default:\r\n");
                            cs.Append("                        e_" + arg.Name + " = 0;\r\n");
                            cs.Append("                        break;\r\n");
                            cs.Append("                }\r\n");

                        }
                    }
                }

                // Make Method Call
                if (A.HasReturnValue == true)
                {
                    cs.Append("                object RetObj = null;\r\n");
                }
                cs.Append("                if (Outer.External_" + A.Name + " != null)\r\n");
                cs.Append("                {\r\n");
                cs.Append("                    ");
                if (A.HasReturnValue == true)
                {
                    cs.Append("RetObj = ");
                }
                cs.Append("Outer.External_" + A.Name + "(");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("out ");
                        }
                        if (arg.RelatedStateVar.AllowedStringValues != null)
                        {
                            cs.Append("e_");
                        }
                        cs.Append(arg.Name);
                        if (i < Args.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                }
                cs.Append(");\r\n");
                cs.Append("                }\r\n");
                cs.Append("                else\r\n");
                cs.Append("                {\r\n");
                cs.Append("                    ");
                if (A.HasReturnValue == true)
                {
                    cs.Append("RetObj = ");
                }
                cs.Append("Sink_" + A.Name + "(");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("out ");
                        }
                        if (arg.RelatedStateVar.AllowedStringValues != null)
                        {
                            cs.Append("e_");
                        }
                        cs.Append(arg.Name);
                        if (i < Args.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                }
                cs.Append(");\r\n");
                cs.Append("                }\r\n");

                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if ((arg.IsReturnValue == false) &&
                        (arg.RelatedStateVar.AllowedStringValues != null))
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("                switch(e_" + arg.Name + ")\r\n");
                            cs.Append("                {\r\n");
                            vd = (VarData)elist[arg.RelatedStateVar.Name];
                            foreach (EnumStruct ss in vd.Enumerations)
                            {
                                cs.Append("                    case Enum_" + arg.RelatedStateVar.Name + "." + ss.EnumName + ":\r\n");
                                cs.Append("                        " + arg.Name + " = \"" + ss.EnumValue + "\";\r\n");
                                cs.Append("                        break;\r\n");
                            }
                            cs.Append("                    default:\r\n");
                            cs.Append("                        " + arg.Name + " = \"\";\r\n");
                            cs.Append("                        break;\r\n");
                            cs.Append("                }\r\n");

                        }
                    }
                }

                if (A.HasReturnValue == true)
                {
                    cs.Append("                return((" + A.GetRetArg().RelatedStateVar.GetNetType().FullName + ")RetObj);\r\n");
                }
                cs.Append("            }\r\n");
            }

            cs.Append("\r\n");
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("            public Delegate_" + A.Name + " Sink_" + A.Name + ";\r\n");
            }
            cs.Append("        }\r\n"); // End of Private Class

            // Build Constructor
            cs.Append("        public " + ClassName + "()\r\n");
            cs.Append("        {\r\n");
            cs.Append("            _S = new _" + ClassName + "(this);\r\n");
            foreach (UPnPStateVariable V in SV)
            {
                cs.Append("            _S.GetUPnPService().GetStateVariableObject(\"" + V.Name + "\").OnModified += new UPnPStateVariable.ModifiedHandler(OnModifiedSink_" + V.Name + ");\r\n");
            }
            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("            _S.Sink_" + A.Name + " = new Delegate_" + A.Name + "(" + A.Name + ");\r\n");
            }


            cs.Append("        }\r\n");
            cs.Append("        public " + ClassName + "(string ID):this()\r\n");
            cs.Append("        {\r\n");
            cs.Append("            _S.GetUPnPService().ServiceID = ID;\r\n");
            cs.Append("        }\r\n");
            cs.Append("        public UPnPService GetUPnPService()\r\n");
            cs.Append("        {\r\n");
            cs.Append("            return(_S.GetUPnPService());\r\n");
            cs.Append("        }\r\n");
            foreach (UPnPStateVariable V in SV)
            {
                cs.Append("        private void OnModifiedSink_" + V.Name + "(UPnPStateVariable sender, object NewValue)\r\n");
                cs.Append("        {\r\n");
                cs.Append("            if (OnStateVariableModified_" + V.Name + " != null) OnStateVariableModified_" + V.Name + "(this);\r\n");
                cs.Append("        }\r\n");
            }
            cs.Append("        //}}}}} End of Code Block\r\n\r\n");
            cs.Append("        #endregion\r\n");
            cs.Append("\r\n");

            foreach (UPnPAction A in s.Actions)
            {
                cs.Append("        /// <summary>\r\n");
                cs.Append("        /// Action: " + A.Name + "\r\n");
                cs.Append("        /// </summary>\r\n");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        cs.Append("        /// <param name=\"" + arg.Name + "\">Associated State Variable: " + arg.RelatedStateVar.Name + "</param>\r\n");
                    }
                }
                if (A.HasReturnValue == true)
                {
                    cs.Append("        /// <returns>Associated StateVariable: " + A.GetRetArg().RelatedStateVar.Name + "</returns>\r\n");
                }
                cs.Append("        public ");
                if (A.HasReturnValue == false)
                {
                    cs.Append("void ");
                }
                else
                {
                    cs.Append(A.GetRetArg().RelatedStateVar.GetNetType().FullName + " ");
                }

                cs.Append(A.Name + "(");
                Args = A.ArgumentList;
                for (int i = 0; i < Args.Length; ++i)
                {
                    arg = Args[i];
                    if (arg.IsReturnValue == false)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append("out ");
                        }

                        if (arg.RelatedStateVar.AllowedStringValues != null)
                        {
                            cs.Append("Enum_" + arg.RelatedStateVar.Name + " ");
                        }
                        else
                        {
                            cs.Append(arg.RelatedStateVar.GetNetType().FullName + " ");
                        }
                        cs.Append(arg.Name);
                        if (i < Args.Length - 1)
                        {
                            cs.Append(", ");
                        }
                    }
                }
                cs.Append(")" + cl);
                cs.Append("        {\r\n");
                cs.Append("            //ToDo: Add Your implementation here, and remove exception\r\n");
                cs.Append("            throw(new UPnPCustomException(800,\"This method has not been completely implemented...\"));\r\n");
                cs.Append("        }\r\n");
            }


            cs.Append("    }\r\n");
            cs.Append("}");

            UTF8Encoding UTF8 = new UTF8Encoding();
            FileStream ws = new FileStream(SavePath, FileMode.Create, FileAccess.Write);
            byte[] buffer = UTF8.GetBytes(cs.ToString());
            ws.Write(buffer, 0, buffer.Length);
            ws.Close();
        }
    }
}
