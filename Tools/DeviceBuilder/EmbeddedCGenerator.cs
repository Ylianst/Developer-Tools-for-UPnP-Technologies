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

namespace UPnPStackBuilder
{
    /// <summary>
    /// Summary description for EmbeddedCGenerator.
    /// </summary>
    public class EmbeddedCGenerator : CodeGenerator
    {
        //		private Hashtable SequenceTable = new Hashtable();
        //		private Hashtable ChoiceTable = new Hashtable();
        //		private int SequenceCounter = 0;
        //		private int ChoiceCounter = 0;

        private Hashtable FriendlyNameTable = new Hashtable();
        private Hashtable MasterFriendlyNameTable = new Hashtable();

        public enum PLATFORMS
        {
            WINDOWS,
            POSIX
        }

        public enum SUBTARGETS
        {
            NONE,
            PPC2003,
            NUCLEUS,
            PSOS
        }

        public enum LANGUAGES
        {
            C,
            CPP
        }

        private string UseSystem = "";

        private UPnPDevice RootDevice = null;

        public PLATFORMS Platform = PLATFORMS.POSIX;
        public SUBTARGETS SubTarget = SUBTARGETS.NONE;
        public LANGUAGES Language = LANGUAGES.C;


        private int WinSock = 0;

        public ArrayList AllServices = new ArrayList();

        private static string cl = "\r\n";
        public string CodeNewLine
        {
            get { return cl; }
            set { cl = value; }
        }

        private string pc_methodPrefix = "UPnP";
        private string pc_methodLibPrefix = "ILib";
        private string pc_methodPrefixDef = "UPnP";
        //		private string pc_inline = "";
        //		private string pc_inlineextern = "";
        private string pc_classPrefix = "";
        private static CodeProcessor PrivateClassDeclarations;
        private static CodeProcessor PublicClassDeclarations;

        public EmbeddedCGenerator(ServiceGenerator.StackConfiguration Config)
            : base(Config)
        {
            switch (Config.newline)
            {
                case ServiceGenerator.NEWLINETYPE.CRLF:
                    cl = "\r\n";
                    break;
                case ServiceGenerator.NEWLINETYPE.LF:
                    cl = "\n";
                    break;
            }
            switch (Config.TargetPlatform)
            {
                case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                    this.Platform = PLATFORMS.POSIX;
                    this.SubTarget = SUBTARGETS.NONE;
                    break;
                case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                    this.Platform = PLATFORMS.WINDOWS;
                    this.SubTarget = SUBTARGETS.NONE;
                    this.WinSock = 1;
                    break;
                case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                    this.Platform = PLATFORMS.WINDOWS;
                    this.SubTarget = SUBTARGETS.NONE;
                    this.WinSock = 2;
                    break;
                case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                    this.Platform = PLATFORMS.WINDOWS;
                    this.SubTarget = SUBTARGETS.PPC2003;
                    this.WinSock = 1;
                    break;
            }
        }

        private void AddLicense(CodeProcessor cs, string filename)
        {
            string l = License;
            l = l.Replace("<FILE>", filename);
            cs.Append(l);
        }

        private void AddAllServices(UPnPDevice device)
        {
            foreach (UPnPService s in device.Services) AllServices.Add(s);
            foreach (UPnPDevice d in device.EmbeddedDevices) AddAllServices(d);
        }

        private string SettingsAsComments
        {
            get
            {
                StringBuilder cs = new StringBuilder();

                cs.Append("/*" + cl);
                cs.Append(" *" + cl);
                cs.Append(" *	Target Platform = " + this.Platform.ToString());
                if (this.SubTarget != SUBTARGETS.NONE)
                {
                    cs.Append(" / " + this.SubTarget.ToString());
                }
                cs.Append(cl);
                if (this.Platform == PLATFORMS.WINDOWS)
                {
                    cs.Append(" *	WinSockVersion  = " + this.WinSock.ToString() + cl);
                }
                cs.Append(" *" + cl);
                cs.Append(" *	HTTP Mode = " + (Configuration.HTTP_1dot1 == false ? "1.0" : "1.1") + cl);
                cs.Append(" *	IPAddressMonitoring = " + (Configuration.DefaultIPAddressMonitor == true ? "YES" : "NO") + cl);
                cs.Append("	*" + cl);
                cs.Append(" */" + cl);
                return (cs.ToString());
            }
        }


        private void CreateMicroStackDef_Device(CodeProcessor cs, UPnPDevice d, ref int counter)
        {
            ++counter;

            cs.Append("const char *FriendlyName" + counter.ToString() + ", ");
            foreach (UPnPDevice dx in d.EmbeddedDevices)
            {
                CreateMicroStackDef_Device(cs, dx, ref counter);
            }
        }

        private void CreateMicroStack_Device_Values(CodeProcessor cs, UPnPDevice d)
        {
            cs.Append("\"" + (string)FriendlyNameTable[d] + "\", ");
            foreach (UPnPDevice dx in d.EmbeddedDevices)
            {
                CreateMicroStack_Device_Values(cs, dx);
            }
        }

        public static void BuildComplexTypeParser_Header(CodeProcessor cs, SortedList SortedServiceList, string pc_methodPrefix, string pc_methodLibPrefix)
        {
            cs.Append(cl);
            cs.Comment("Complex Type Parsers");
            IDictionaryEnumerator en = SortedServiceList.GetEnumerator();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                foreach (UPnPComplexType CT in service.GetComplexTypeList())
                {
                    cs.Append("struct " + CT.Name_LOCAL + "* " + pc_methodPrefix + "Parse_" + CT.Name_LOCAL + "(struct " + pc_methodLibPrefix + "XMLNode *node);" + cl);
                }
            }
            cs.Append(cl);
        }
        public static void BuildComplexTypeParser_Collection(string cx, Hashtable SequenceTable, Hashtable ChoiceTable, ref int SeqX, ref int ChoX, CodeProcessor cs, UPnPComplexType.ItemCollection ic, string pc_methodPrefix)
        {
            int x = 0;
            string prefix = "";
            int SeqX2 = 0;
            int ChoX2 = 0;


            if (ic.GetType() == typeof(UPnPComplexType.Sequence))
            {
                ++SeqX;
                if (cx == "")
                {
                    cx += "_sequence_" + SeqX.ToString();
                }
                else
                {
                    cx += "->_sequence_" + SeqX.ToString();
                }
                prefix = cx + "->";
            }
            else if (ic.GetType() == typeof(UPnPComplexType.Choice))
            {
                ++ChoX;
                if (cx == "")
                {
                    cx += "_choice_" + ChoX.ToString();
                }
                else
                {
                    cx += "->_choice_" + ChoX.ToString();
                }
                prefix = cx + "->";
            }

            foreach (UPnPComplexType.ContentData cd in ic.Items)
            {
                ++x;
                cs.Append("	if (node->NameLength==" + cd.Name.Length.ToString() + " && memcmp(node->Name,\"" + cd.Name + "\"," + cd.Name.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);
                if (x == 1)
                {
                    Stack st = new Stack();
                    UPnPComplexType.ItemCollection tc = ic;
                    DText pp = new DText();
                    pp.ATTRMARK = "->";
                    pp[0] = cx;
                    int ppx = pp.DCOUNT();
                    while (tc != null)
                    {
                        string ps;

                        ps = "RetVal";
                        for (int i = 1; i <= ppx; ++i)
                        {
                            ps += ("->" + pp[i]);
                        }
                        st.Push(new object[2] { ps, tc });
                        --ppx;
                        tc = tc.ParentCollection;
                    }
                    while (st.Count > 0)
                    {
                        object[] foo = (object[])st.Pop();
                        cs.Append("	if (" + (string)foo[0] + " == NULL)" + cl);
                        cs.Append("	{" + cl);
                        if (foo[1].GetType() == typeof(UPnPComplexType.Sequence))
                        {
                            cs.Append("	if ((" + (string)foo[0] + " = (struct SEQUENCE_" + SequenceTable[foo[1]].ToString() + "*)malloc(sizeof(struct SEQUENCE_" + SequenceTable[foo[1]].ToString() + "))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                            cs.Append("	memset(" + (string)foo[0] + ",0,sizeof(struct SEQUENCE_" + SequenceTable[foo[1]].ToString() + "));" + cl);
                        }
                        else if (foo[1].GetType() == typeof(UPnPComplexType.Choice))
                        {
                            cs.Append("	if ((" + (string)foo[0] + " = (struct CHOICE_" + ChoiceTable[foo[1]].ToString() + "*)malloc(sizeof(struct CHOICE_" + ChoiceTable[foo[1]].ToString() + "))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                            cs.Append("	memset(" + (string)foo[0] + ",0,sizeof(struct CHOICE_" + ChoiceTable[foo[1]].ToString() + "));" + cl);
                        }
                        cs.Append("	}" + cl);
                    }

                    //					if (ic.GetType()==typeof(UPnPComplexType.Sequence))
                    //					{
                    //						cs.Append("	RetVal->"+cx+" = (struct SEQUENCE_"+SequenceTable[ic].ToString()+"*)malloc(sizeof(struct SEQUENCE_"+SequenceTable[ic].ToString()+"));"+cl);
                    //					}
                    //					else if (ic.GetType()==typeof(UPnPComplexType.Choice))
                    //					{
                    //						cs.Append("	RetVal->"+cx+" = (struct CHOICE_"+ChoiceTable[ic].ToString()+"*)malloc(sizeof(struct CHOICE_"+ChoiceTable[ic].ToString()+"));"+cl);
                    //					}
                }
                if (cd.TypeNS == "http://www.w3.org/2001/XMLSchema")
                {
                    // XSD Simple Type
                    switch (cd.Type)
                    {
                        case "boolean":
                        case "int":
                        case "integer":
                        case "positiveInteger":
                        case "negativeInteger":
                        case "nonNegativeInteger":
                        case "nonPositiveInteger":
                        case "long":
                        case "short":
                            cs.Append("	RetVal->" + prefix + cd.Name + " = atoi(text);" + cl);
                            break;
                    }
                }
                else
                {
                    // XSD User Defined Type
                    cs.Append("	RetVal->" + prefix + cd.Name + " = " + pc_methodPrefix + "Parse_" + cd.Type + "(node->Next);" + cl);
                }
                cs.Append("	}" + cl);
            }
            foreach (UPnPComplexType.ItemCollection ec in ic.NestedCollections)
            {
                BuildComplexTypeParser_Collection(cx, SequenceTable, ChoiceTable, ref SeqX2, ref ChoX2, cs, ec, pc_methodPrefix);
            }
        }
        public static void BuildComplexTypeParser(Hashtable SequenceTable, Hashtable ChoiceTable, CodeProcessor cs, SortedList SortedServiceList, string pc_methodPrefix, string pc_methodLibPrefix)
        {
            IDictionaryEnumerator en = SortedServiceList.GetEnumerator();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                foreach (UPnPComplexType CT in service.GetComplexTypeList())
                {
                    int SeqX = 0;
                    int ChoX = 0;
                    cs.Append("struct " + CT.Name_LOCAL + "* " + pc_methodPrefix + "Parse_" + CT.Name_LOCAL + "(struct " + pc_methodLibPrefix + "XMLNode *node)" + cl);
                    cs.Append("{" + cl);
                    cs.Append("	struct " + pc_methodLibPrefix + "XMLNode *current = node;" + cl);
                    cs.Append("	struct " + CT.Name_LOCAL + " *RetVal;" + cl);
                    cs.Append("	" + cl);
                    cs.Append("	int OK;" + cl);
                    cs.Append("	char *text;" + cl);
                    cs.Append("	int textLength;" + cl);
                    cs.Append("	if ((RetVal = (struct " + CT.Name_LOCAL + "*)malloc(sizeof(struct " + CT.Name_LOCAL + "))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                    cs.Append(cl);
                    cs.Append("	memset(RetVal, 0, sizeof(struct " + CT.Name_LOCAL + "));" + cl);
                    cs.Append(cl);
                    cs.Append("	while(node != NULL)" + cl);
                    cs.Append("	{" + cl);
                    cs.Append("		textLength = " + pc_methodLibPrefix + "ReadInnerXML(node, &text);" + cl);
                    foreach (UPnPComplexType.GenericContainer gc in CT.Containers)
                    {
                        foreach (UPnPComplexType.ItemCollection ic in gc.Collections)
                        {
                            BuildComplexTypeParser_Collection("", SequenceTable, ChoiceTable, ref SeqX, ref ChoX, cs, ic, pc_methodPrefix);
                        }
                    }
                    cs.Append("	node = node->Peer;" + cl);
                    cs.Append("	}" + cl);
                    cs.Append("	return(RetVal);" + cl);
                    cs.Append("}" + cl);
                }
            }
        }
        private void BuildCreateMicroStackDefinition_sprintf(CodeProcessor cs, UPnPDevice d, int i)
        {
            cs.Append(", FriendlyName");
            if (i != 1)
            {
                cs.Append(i.ToString());
            }
            cs.Append(", RetVal->Serial, RetVal->UDN");

            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                BuildCreateMicroStackDefinition_sprintf(cs, ed, ++i);
            }
        }
        private void BuildCreateMicroStackDefinition(CodeProcessor cs, UPnPDevice d, int i)
        {
            UPnPDevice parent = d;
            while (parent.ParentDevice != null)
            {
                parent = parent.ParentDevice;
            }
            if (parent.User.Equals(d.User))
            {
                cs.Append("const char* FriendlyName");
                if (i != 1) cs.Append(i.ToString());
                cs.Append(", ");
                foreach (UPnPDevice ed in d.EmbeddedDevices) BuildCreateMicroStackDefinition(cs, ed, ++i);
            }
        }
        private void BuildObjectMetaData(CodeProcessor cs, UPnPDevice d, int i)
        {
            string deviceIdent = DeviceObjectGenerator.GetDeviceIdentifier(d);
            if (d.ParentDevice == null)
            {
                deviceIdent += ".";
            }
            else
            {
                deviceIdent += "->";
            }

            UPnPDevice parentDevice = d;
            while (parentDevice.ParentDevice != null)
            {
                parentDevice = parentDevice.ParentDevice;
            }
            if (parentDevice.User.Equals(d.User))
            {
                cs.Append("	" + deviceIdent + "FriendlyName = FriendlyName");
                if (i != 1)
                {
                    cs.Append(i.ToString());
                }
                cs.Append(";" + cl);
            }
            if (i == 1)
            {
                cs.Append("	" + deviceIdent + "UDN = UDN;" + cl);
                cs.Append("	" + deviceIdent + "Serial = SerialNumber;" + cl);
            }

            cs.Append("	if (" + deviceIdent + "Manufacturer == NULL) {" + deviceIdent + "Manufacturer = \"" + (string)((object[])d.User3)[2] + "\";}" + cs.NewLine);
            cs.Append("	if (" + deviceIdent + "ManufacturerURL == NULL) {" + deviceIdent + "ManufacturerURL = \"" + (string)((object[])d.User3)[3] + "\";}" + cs.NewLine);
            cs.Append("	if (" + deviceIdent + "ModelDescription == NULL) {" + deviceIdent + "ModelDescription = \"" + (string)((object[])d.User3)[4] + "\";}" + cs.NewLine);
            cs.Append("	if (" + deviceIdent + "ModelName == NULL) {" + deviceIdent + "ModelName = \"" + (string)((object[])d.User3)[5] + "\";}" + cs.NewLine);
            cs.Append("	if (" + deviceIdent + "ModelNumber == NULL) {" + deviceIdent + "ModelNumber = \"" + (string)((object[])d.User3)[6] + "\";}" + cs.NewLine);
            if (((object[])d.User3)[7] != null)
            {
                cs.Append("	if (" + deviceIdent + "ModelURL == NULL) {" + deviceIdent + "ModelURL = \"" + ((Uri)((object[])d.User3)[7]).AbsoluteUri + "\";}" + cs.NewLine);
            }
            else
            {
                cs.Append("	if (" + deviceIdent + "ModelURL == NULL) {" + deviceIdent + "ModelURL = \"" + (string)((object[])d.User3)[3] + "\";}" + cs.NewLine);
            }
            cs.Append("	if (" + deviceIdent + "ProductCode == NULL) {" + deviceIdent + "ProductCode = \"" + (string)((object[])d.User3)[8] + "\";}" + cs.NewLine);

            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                BuildObjectMetaData(cs, ed, ++i);
            }
        }
        private int BuildCreateMicroStackDefinition_Malloc(CodeProcessor cs, UPnPDevice d, int i)
        {
            cs.Append("+ (int)strlen(FriendlyName");
            if (i != 1)
            {
                cs.Append(i.ToString());
            }
            cs.Append(") ");
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                BuildCreateMicroStackDefinition_Malloc(cs, ed, ++i);
            }
            return (i);
        }

        protected void FriendlyName(UPnPDevice d)
        {
            if (d.FriendlyName == "%s")
            {
                d.FriendlyName = (string)MasterFriendlyNameTable[d];
            }
            else
            {
                MasterFriendlyNameTable[d] = d.FriendlyName;
            }
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                FriendlyName(ed);
            }
        }
        public override bool Generate(UPnPDevice[] devices, DirectoryInfo outputDirectory)
        {
            StreamWriter W;
            bool RetVal = false;
            string SampleApp = null;
            bool deviceOK = false;
            bool cpOK = false;

            string WS = null;
            string WS2 = null;

            if (!Configuration.SupressSampleProject)
            {
                switch (Configuration.TargetPlatform)
                {
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                        SampleApp = SourceCodeRepository.GetMain_C_Template();
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                        SampleApp = SourceCodeRepository.Get_Generic("SAMPLE_CPP");
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                        SampleApp = SourceCodeRepository.Get_SampleProjectDlg_cpp();
                        break;
                }
            }

            MasterFriendlyNameTable.Clear();
            foreach (UPnPDevice d in devices)
            {
                FriendlyName(d);
            }
            if (Configuration.CPlusPlusWrapper || Configuration.DynamicObjectModel)
            {
                DeviceObjectGenerator.PrepDevice(devices);
            }

            #region ILib File Generation
            SourceCodeRepository.Generate_Parsers(Configuration.prefixlib, outputDirectory);
            SourceCodeRepository.Generate_AsyncSocket(Configuration.prefixlib, outputDirectory);
            SourceCodeRepository.Generate_AsyncUDPSocket(Configuration.prefixlib, outputDirectory);
            SourceCodeRepository.Generate_AsyncServerSocket(Configuration.prefixlib, outputDirectory);
            SourceCodeRepository.Generate_WebClient(Configuration, outputDirectory);
            SourceCodeRepository.Generate_WebServer(Configuration, outputDirectory);

            if (Configuration.GenerateThreadPoolLibrary)
            {
                SourceCodeRepository.Generate_ThreadPool(Configuration.prefixlib, outputDirectory);
            }
            if (Configuration.TargetPlatform == ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1)
            {
                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "ChainAdaptor.h", "CHAINADAPTOR_H");
                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "ChainAdaptor.cpp", "CHAINADAPTOR_CPP");

                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "ChainEngine.h", "CHAINENGINE_H");
                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "ChainEngine.cpp", "CHAINENGINE_CPP");

                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "SocketWrapper.h", "SOCKETWRAPPER_H");
                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "SocketWrapper.cpp", "SOCKETWRAPPER_CPP");

                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "SymbianSemaphore.h", "SYMBIANSEMAPHORE_H");
                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "SymbianSemaphore.cpp", "SYMBIANSEMAPHORE_CPP");

                SourceCodeRepository.Generate_Generic(Configuration.prefixlib, outputDirectory, "ChainDefs.h", "CHAINDEFS_H");
            }
            #endregion

            #region Generate MicroStack files for each device
            foreach (UPnPDevice device in devices)
            {
                if (((ServiceGenerator.Configuration)device.User).ConfigType == ServiceGenerator.ConfigurationType.DEVICE)
                {
                    if (Configuration.UPNP_1dot1)
                    {
                        device.ArchitectureVersion = "1.1";
                        device.BootID = "%d";
                    }
                    else
                    {
                        device.ArchitectureVersion = "1.0";
                        device.BootID = "";
                    }

                    device.ClearCustomFieldsInDescription();
                    ((ServiceGenerator.Configuration)device.User).AddAllCustomFieldsToDevice(device);

                    device.HasPresentation = ((ServiceGenerator.Configuration)device.User).AdvertisesPresentationPage;
                    if (device.HasPresentation) device.PresentationURL = "/web";

                    RetVal = GenerateEx(device, outputDirectory, GetServiceNameTable(device), ref SampleApp);
                    if (!RetVal) { break; }
                }
            }
            #endregion
            #region C++ Wrapper Generation
            if (Configuration.CPlusPlusWrapper)
            {
                string CPlusPlus_H = DeviceObjectGenerator.GetCPlusPlusAbstraction_H(devices);

                #region Prefixes
                CPlusPlus_H = CPlusPlus_H.Replace("ILib", Configuration.prefixlib);
                #endregion
                #region Write to disk

                W = File.CreateText(outputDirectory.FullName + "\\UPnPAbstraction.h");
                W.Write(CPlusPlus_H);
                W.Close();
                #endregion

                foreach (UPnPDevice d in devices)
                {
                    FriendlyName(d);
                }

                string CPlusPlus_CPP = DeviceObjectGenerator.GetCPlusPlusAbstraction_CPP(devices);
                #region Prefixes
                CPlusPlus_CPP = CPlusPlus_CPP.Replace("ILib", Configuration.prefixlib);
                #endregion
                #region Write to disk

                W = File.CreateText(outputDirectory.FullName + "\\UPnPAbstraction.cpp");
                W.Write(CPlusPlus_CPP);
                W.Close();
                #endregion

            }
            #endregion

            CPEmbeddedCGenerator gen2 = new CPEmbeddedCGenerator(Configuration, SampleApp);
            gen2.Generate(devices, outputDirectory);
            SampleApp = gen2.SampleApplication;

            if (SampleApp != null)
            {
                #region Main.c / SampleProjectDlg.cpp

                #region Platform

                if (Configuration.TargetPlatform != ServiceGenerator.PLATFORMS.MICROSTACK_POSIX)
                {
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_POSIX}}}", "//{{{END_POSIX}}}", SampleApp);
                }
                else
                {
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{BEGIN_POSIX}}}", "//{{{END_POSIX}}}", SampleApp);
                }

                if (Configuration.TargetPlatform == ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1 ||
                    Configuration.TargetPlatform == ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2 ||
                    Configuration.TargetPlatform == ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
                {
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{BEGIN_WIN32}}}", "//{{{END_WIN32}}}", SampleApp);
                }
                else
                {
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_WIN32}}}", "//{{{END_WIN32}}}", SampleApp);
                }
                #endregion

                if (Configuration.CPlusPlusWrapper)
                {
                    SampleApp = SampleApp.Replace("//{{{CLASS_DEFINITIONS_DEVICE}}}", DeviceObjectGenerator.GetCPlusPlus_DerivedSampleClasses(devices));
                    SampleApp = SampleApp.Replace("//{{{CLASS_IMPLEMENTATIONS_DEVICE}}}", DeviceObjectGenerator.GetCPlusPlus_DerivedSampleClasses_Implementation(devices));
                    SampleApp = SampleApp.Replace("//{{{DERIVED_CLASS_INSERTION}}}", DeviceObjectGenerator.GetCPlusPlus_DerivedSampleClasses_Insertion(devices));
                }
                SampleApp = SampleApp.Replace("{{{INITSTRING}}}", "");
                SampleApp = SampleApp.Replace("//{{{DEVICE_INVOCATION_DISPATCH}}}", "");
                SampleApp = SampleApp.Replace("//{{{INVOCATION_FP}}}", "");
                SampleApp = SampleApp.Replace("//{{{MICROSTACK_VARIABLE}}}", "");
                SampleApp = SampleApp.Replace("//{{{MicroStack_Include}}}", "");
                SampleApp = SampleApp.Replace("//{{{CREATE_MICROSTACK}}}", "");
                SampleApp = SampleApp.Replace("//{{{STATEVARIABLES_INITIAL_STATE}}}", "");
                SampleApp = SampleApp.Replace("//{{{IPAddress_Changed}}}", "");
                SampleApp = SampleApp.Replace("//{{{PresentationRequest}}}", "");

                if (this.Configuration.InitThreadPoolInSampleApp)
                {
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{BEGIN_THREADPOOL}}}", "//{{{END_THREADPOOL}}}", SampleApp);
                    SampleApp = SampleApp.Replace("!NUMTHREADPOOLTHREADS!", Configuration.ThreadPoolThreads_InSampleApp.ToString());
                }
                else
                {
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_THREADPOOL}}}", "//{{{END_THREADPOOL}}}", SampleApp);
                }
                if (Configuration.BareBonesSample)
                {
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{BEGIN_BAREBONES}}}", "//{{{END_BAREBONES}}}", SampleApp);
                }
                else
                {
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_BAREBONES}}}", "//{{{END_BAREBONES}}}", SampleApp);
                }
                #region C or C++
                if (Configuration.CPlusPlusWrapper)
                {
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{STANDARD_C_APP_BEGIN}}}", "//{{{STANDARD_C_APP_END}}}", SampleApp);
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{STANDARD_C++_APP_BEGIN}}}", "//{{{STANDARD_C++_APP_END}}}", SampleApp);
                }
                else
                {
                    SampleApp = SourceCodeRepository.RemoveTag("//{{{STANDARD_C_APP_BEGIN}}}", "//{{{STANDARD_C_APP_END}}}", SampleApp);
                    SampleApp = SourceCodeRepository.RemoveAndClearTag("//{{{STANDARD_C++_APP_BEGIN}}}", "//{{{STANDARD_C++_APP_END}}}", SampleApp);
                }
                #endregion

                #region Write to disk
                switch (Configuration.TargetPlatform)
                {
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                        if (Configuration.CPlusPlusWrapper)
                        {
                            W = File.CreateText(outputDirectory.FullName + "\\Main.cpp");
                            SampleApp = SampleApp.Replace("Main.c", "Main.cpp");
                        }
                        else
                        {
                            W = File.CreateText(outputDirectory.FullName + "\\Main.c");
                        }
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                        W = File.CreateText(outputDirectory.FullName + "\\SampleProjectDlg.cpp");
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                        W = File.CreateText(outputDirectory.FullName + "\\Sample.cpp");
                        break;
                    default:
                        W = null;
                        break;
                }

                if (W != null)
                {
                    W.Write(SampleApp);
                    W.Close();
                }
                #endregion
                #endregion


                #region Visual Studio Files and Posix Makefile

                #region Initialize Project and Makefile
                switch (Configuration.TargetPlatform)
                {
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                        WS = SourceCodeRepository.Get_UPnPSample_vcproj().Replace("{{{WINSOCK}}}", "WINSOCK1");
                        WS = WS.Replace("{{{WINSOCK_LIB}}}", "WSock32.lib");
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                        WS = SourceCodeRepository.Get_UPnPSample_vcproj().Replace("{{{WINSOCK}}}", "WINSOCK2");
                        WS = WS.Replace("{{{WINSOCK_LIB}}}", "Psapi.lib ws2_32.lib Iphlpapi.lib");
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                        WS = SourceCodeRepository.Get_Makefile().Replace("{{{BUILD_NUMBER}}}", UseVersion);
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                        WS = SourceCodeRepository.Get_SampleProject_vcp();
                        break;
                    case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                        WS = SourceCodeRepository.Get_Generic("_MMP");
                        break;
                }
                #endregion
                if (WS != null)
                {
                    #region Building Project and Makefile

                    if (Configuration.GenerateThreadPoolLibrary)
                    {
                        WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_THREADPOOL}}}", "//{{{END_THREADPOOL}}}", WS);
                    }
                    else
                    {
                        WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_THREADPOOL}}}", "//{{{END_THREADPOOL}}}", WS);
                    }
                    if (Configuration.CPlusPlusWrapper)
                    {
                        WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_C++}}}", "//{{{END_C++}}}", WS);
                    }
                    else
                    {
                        WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_C++}}}", "//{{{END_C++}}}", WS);
                    }

                    WS = WS.Replace("ILib", Configuration.prefixlib);

                    deviceOK = false;
                    cpOK = false;
                    foreach (UPnPDevice device in devices)
                    {
                        ServiceGenerator.Configuration DeviceConf = (ServiceGenerator.Configuration)device.User;
                        switch (DeviceConf.ConfigType)
                        {
                            case ServiceGenerator.ConfigurationType.DEVICE:
                                deviceOK = true;
                                switch (Configuration.TargetPlatform)
                                {
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                                        WS = WS.Replace("{{{INCLUDE_C}}}", "{{{INCLUDE_C}}}" + "\r\n<File RelativePath=\"" + DeviceConf.Prefix + "MicroStack.c\"/>");
                                        WS = WS.Replace("{{{INCLUDE_H}}}", "{{{INCLUDE_H}}}" + "\r\n<File RelativePath=\"" + DeviceConf.Prefix + "MicroStack.h\"/>");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                                        WS = WS.Replace("{{{O_FILES}}}", DeviceConf.Prefix + "MicroStack.o\\" + "\n{{{O_FILES}}}");
                                        WS = WS.Replace("{{{H_FILES}}}", DeviceConf.Prefix + "MicroStack.h\\" + "\n{{{H_FILES}}}");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_DEFINTION}}}", "{{{END_MICROSTACK_DEFINTION}}}");
                                        WS2 = WS2.Replace("{{CODEPREFIX}}", DeviceConf.Prefix);
                                        WS2 = WS2.Replace("{{LIBPREFIX}}", Configuration.prefixlib);
                                        WS2 = WS2.Replace("{{CODEPREFIX_CAPS}}", DeviceConf.Prefix.ToUpper());
                                        WS2 = WS2.Replace("{{STACK}}", "MicroStack");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_DEFINTION}}}", WS2);

                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_H}}}", "{{{END_MICROSTACK_H}}}");
                                        WS2 = WS2.Replace("{{CODEPREFIX}}", DeviceConf.Prefix);
                                        WS2 = WS2.Replace("{{STACK}}", "MicroStack");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_H}}}", WS2);

                                        WS = WS.Replace("{{{MICROSTACK_H}}}", "{{{MICROSTACK_H}}}\r\n\t\".\\" + DeviceConf.Prefix + "MicroStack.h\"\\");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "//{{{BeginSource}}}", "//{{{EndSource}}}");
                                        WS2 = WS2.Replace("{{{SOURCE}}}", DeviceConf.Prefix + "MicroStack.c");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "//{{{BeginSource}}}", WS2);
                                        break;
                                }
                                break;
                            case ServiceGenerator.ConfigurationType.CONTROLPOINT:
                                cpOK = true;
                                switch (Configuration.TargetPlatform)
                                {
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                                        WS = WS.Replace("{{{INCLUDE_C}}}", "{{{INCLUDE_C}}}" + "\r\n<File RelativePath=\"" + DeviceConf.Prefix + "ControlPoint.c\"/>");
                                        WS = WS.Replace("{{{INCLUDE_H}}}", "{{{INCLUDE_H}}}" + "\r\n<File RelativePath=\"" + DeviceConf.Prefix + "ControlPoint.h\"/>");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                                        WS = WS.Replace("{{{O_FILES}}}", DeviceConf.Prefix + "ControlPoint.o\\" + "\n{{{O_FILES}}}");
                                        WS = WS.Replace("{{{H_FILES}}}", DeviceConf.Prefix + "ControlPoint.h\\" + "\n{{{H_FILES}}}");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_DEFINTION}}}", "{{{END_MICROSTACK_DEFINTION}}}");
                                        WS2 = WS2.Replace("{{CODEPREFIX}}", DeviceConf.Prefix);
                                        WS2 = WS2.Replace("{{LIBPREFIX}}", Configuration.prefixlib);
                                        WS2 = WS2.Replace("{{CODEPREFIX_CAPS}}", DeviceConf.Prefix.ToUpper());
                                        WS2 = WS2.Replace("{{STACK}}", "ControlPoint");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_DEFINTION}}}", WS2);

                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_H}}}", "{{{END_MICROSTACK_H}}}");
                                        WS2 = WS2.Replace("{{CODEPREFIX}}", DeviceConf.Prefix);
                                        WS2 = WS2.Replace("{{STACK}}", "ControlPoint");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_H}}}", WS2);

                                        WS = WS.Replace("{{{MICROSTACK_H}}}", "{{{MICROSTACK_H}}}\r\n\t\".\\" + DeviceConf.Prefix + "ControlPoint.h\"\\");
                                        break;
                                    case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "//{{{BeginSource}}}", "//{{{EndSource}}}");
                                        WS2 = WS2.Replace("{{{SOURCE}}}", DeviceConf.Prefix + "ControlPoint.c");
                                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "//{{{BeginSource}}}", WS2);
                                        break;
                                }
                                break;
                        }
                    }

                    if (cpOK)
                    {
                        // Insert CP related files into the project file
                        switch (Configuration.TargetPlatform)
                        {
                            case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                            case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                                WS = WS.Replace("{{{INCLUDE_C}}}", "{{{INCLUDE_C}}}" + "\r\n<File RelativePath=\"" + Configuration.prefixlib + "SSDPClient.c\"/>");
                                WS = WS.Replace("{{{INCLUDE_H}}}", "{{{INCLUDE_H}}}" + "\r\n<File RelativePath=\"" + Configuration.prefixlib + "SSDPClient.h\"/>");
                                WS = WS.Replace("{{{INCLUDE_H}}}", "{{{INCLUDE_H}}}" + "\r\n<File RelativePath=\"UPnPControlPointStructs.h\"/>");
                                break;
                            case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                                WS = WS.Replace("{{{O_FILES}}}", Configuration.prefixlib + "SSDPClient.o\\" + "\n{{{O_FILES}}}");
                                WS = WS.Replace("{{{H_FILES}}}", Configuration.prefixlib + "SSDPClient.h\\" + "\n{{{H_FILES}}}");
                                break;
                            case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                                WS = WS.Replace("{{{MICROSTACK_H}}}", "{{{MICROSTACK_H}}}\r\n\t\".\\" + Configuration.prefixlib + "SSDPClient.h\"\\");
                                WS = WS.Replace("{{{MICROSTACK_H}}}", "{{{MICROSTACK_H}}}\r\n\t\".\\UPnPControlPointStructs.h\"\\");

                                WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_H}}}", "{{{END_MICROSTACK_H}}}");
                                WS2 = WS2.Replace("{{CODEPREFIX}}", "");
                                WS2 = WS2.Replace("{{STACK}}", "UPnPControlPointStructs");
                                WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_H}}}", WS2);

                                WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "{{{BEGIN_MICROSTACK_H}}}", "{{{END_MICROSTACK_H}}}");
                                WS2 = WS2.Replace("{{CODEPREFIX}}", this.pc_methodLibPrefix);
                                WS2 = WS2.Replace("{{STACK}}", "SSDPClient");
                                WS = SourceCodeRepository.InsertTextBeforeTag(WS, "{{{BEGIN_MICROSTACK_H}}}", WS2);

                                WS = WS.Replace("{{LIBPREFIX}}", Configuration.prefixlib);
                                WS = WS.Replace("{{LIBPREFIX_CAPS}}", Configuration.prefixlib.ToUpper());
                                WS = SourceCodeRepository.RemoveTag("{{{BEGIN_SSDPCLIENT}}}", "{{{END_SSDPCLIENT}}}", WS);
                                break;
                            case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                                WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "//{{{BeginSource}}}", "//{{{EndSource}}}");
                                WS2 = WS2.Replace("{{{SOURCE}}}", Configuration.prefixlib + "SSDPClient.c");
                                WS = SourceCodeRepository.InsertTextBeforeTag(WS, "//{{{BeginSource}}}", WS2);
                                break;
                        }
                    }
                    else
                    {
                        WS = SourceCodeRepository.RemoveAndClearTag("{{{BEGIN_SSDPCLIENT}}}", "{{{END_SSDPCLIENT}}}", WS);
                    }



                    WS = WS.Replace("{{{INCLUDE_C}}}", "");
                    WS = WS.Replace("{{{INCLUDE_H}}}", "");
                    WS = WS.Replace("{{{O_FILES}}}", "");
                    WS = WS.Replace("{{{H_FILES}}}", "");

                    WS = WS.Replace("{{{MICROSTACK_H}}}", "");
                    WS = SourceCodeRepository.RemoveAndClearTag("{{{BEGIN_MICROSTACK_DEFINTION}}}", "{{{END_MICROSTACK_DEFINTION}}}", WS);
                    WS = SourceCodeRepository.RemoveAndClearTag("{{{BEGIN_MICROSTACK_H}}}", "{{{END_MICROSTACK_H}}}", WS);

                    if (!Configuration.BareBonesSample)
                    {
                        WS = WS.Replace("<-- Additional Filters -->", "");
                    }

                    #endregion

                    #region Write various files to Disc
                    switch (Configuration.TargetPlatform)
                    {
                        #region Visual Studio 2003 Solution/Project files
                        case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
                        case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
                            #region UPnPSample.vcproj
                            if (Configuration.CPlusPlusWrapper)
                            {
                                WS = WS.Replace("Main.c", "Main.cpp");
                            }
                            W = File.CreateText(outputDirectory.FullName + "\\UPnPSample.vcproj");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #region UPnPSample.sln
                            W = File.CreateText(outputDirectory.FullName + "\\UPnPSample.sln");
                            W.Write(SourceCodeRepository.Get_UPnPSample_sln());
                            W.Close();
                            #endregion
                            #region stdafx.h
                            W = File.CreateText(outputDirectory.FullName + "\\stdafx.h");
                            W.Write(SourceCodeRepository.Get_Win32_stdafx_h());
                            W.Close();
                            #endregion
                            #region stdafx.cpp
                            W = File.CreateText(outputDirectory.FullName + "\\stdafx.cpp");
                            W.Write(SourceCodeRepository.Get_Win32_stdafx_cpp());
                            W.Close();
                            #endregion
                            break;
                        #endregion
                        #region PocketPC 2003 Specific
                        case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
                            #region SampleProject.vcp
                            WS = WS.Replace("{{CODEPREFIX}}", this.pc_methodPrefix);
                            WS = WS.Replace("{{CODEPREFIX_CAPS}}", this.pc_methodPrefix.ToUpper());
                            WS = WS.Replace("{{LIBPREFIX}}", this.pc_methodLibPrefix);
                            WS = WS.Replace("{{LIBPREFIX_CAPS}}", this.pc_methodLibPrefix.ToUpper());

                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProject.vcp");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region SampleProjectDlg.h

                            WS = SourceCodeRepository.Get_SampleProjectDlg_h();

                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProjectDlg.h");
                            W.Write(WS);
                            W.Close();
                            #endregion

                            #endregion
                            #region SampleProject.cpp
                            WS = SourceCodeRepository.Get_SampleProject_cpp();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProject.cpp");
                            W.Write(WS);
                            W.Close();
                            #endregion

                            #endregion
                            #region SampleProject.h
                            WS = SourceCodeRepository.Get_SampleProject_h();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProject.h");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region newres.h
                            WS = SourceCodeRepository.Get_newres_h();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\newres.h");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region resource.h
                            WS = SourceCodeRepository.Get_resource_h();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\resource.h");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region SampleProject.rc
                            WS = SourceCodeRepository.Get_SampleProject_rc();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProject.rc");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region SampleProject.vcw
                            WS = SourceCodeRepository.Get_SampleProject_vcw();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\SampleProject.vcw");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region StdAfx.h
                            WS = SourceCodeRepository.Get_StdAfx_h();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\StdAfx.h");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region StdAfx.cpp
                            WS = SourceCodeRepository.Get_StdAfx_cpp();
                            #region Write to disk
                            W = File.CreateText(outputDirectory.FullName + "\\StdAfx.cpp");
                            W.Write(WS);
                            W.Close();
                            #endregion
                            #endregion
                            #region SampleProject.ico
                            #region Write to disk
                            byte[] b = SourceCodeRepository.Get_SampleProject_ico();
                            FileStream F = File.Create(outputDirectory.FullName + "\\SampleProject.ico", b.Length);
                            F.Write(b, 0, b.Length);
                            F.Close();
                            #endregion
                            #endregion
                            break;
                        #endregion
                        #region Makefile
                        case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
                            if (Configuration.CPlusPlusWrapper)
                            {
                                WS = WS.Replace("Main.c", "Main.cpp");
                            }
                            W = File.CreateText(outputDirectory.FullName + "\\makefile");
                            W.Write(WS);
                            W.Close();
                            break;
                        #endregion
                        #region Symbian MMP File
                        case ServiceGenerator.PLATFORMS.MICROSTACK_SYMBIANv9_1:
                            W = File.CreateText(outputDirectory.FullName + "\\Sample.mmp");
                            WS = SourceCodeRepository.RemoveAndClearTag("//{{{BeginSource}}}", "//{{{EndSource}}}", WS);
                            W.Write(WS);
                            W.Close();
                            break;
                        #endregion
                    }
                    #endregion
                }
                #endregion
            }


            return (RetVal);
        }
        protected bool DoesDeviceHaveAnyNonVersionOneComponents(UPnPDevice device)
        {
            if (device.Major > 1)
            {
                return (true);
            }
            else
            {
                foreach (UPnPDevice ed in device.EmbeddedDevices)
                {
                    if (DoesDeviceHaveAnyNonVersionOneComponents(ed))
                    {
                        return (true);
                    }
                }
                foreach (UPnPService s in device.Services)
                {
                    if (s.Major > 1)
                    {
                        return (true);
                    }
                }
            }
            return (false);
        }
        protected bool DeviceHasEvents(OpenSource.UPnP.UPnPDevice device)
        {
            foreach (UPnPDevice ed in device.EmbeddedDevices)
            {
                if (DeviceHasEvents(ed)) { return (true); }
            }
            foreach (UPnPService s in device.Services)
            {
                foreach (UPnPStateVariable sv in s.GetStateVariables())
                {
                    if (sv.SendEvent) { return (true); }
                }
            }
            return (false);
        }
        protected bool GenerateEx(UPnPDevice device, DirectoryInfo outputDirectory, Hashtable serviceNames, ref string SampleApp)
        {
            bool BuildSampleApp = SampleApp == null ? false : true;
            ServiceGenerator.Configuration DeviceConf = (ServiceGenerator.Configuration)device.User;

            #region Initialize
            string WS;
            StreamWriter W;
            Hashtable ChoTable = new Hashtable();
            Hashtable SeqTable = new Hashtable();
            int SequenceCounter = 0;
            int ChoiceCounter = 0;

            string first = "";
            RootDevice = device;
            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();


            if (this.SubTarget == SUBTARGETS.NONE)
            {
                UseSystem = this.Platform.ToString();
            }
            else
            {
                UseSystem = this.SubTarget.ToString();
            }

            pc_methodPrefix = ((ServiceGenerator.Configuration)device.User).Prefix;
            pc_methodLibPrefix = Configuration.prefixlib;

            if (this.Language == LANGUAGES.C)
            {
                pc_methodPrefixDef = CallingConvention + pc_methodPrefix;
                pc_classPrefix = "";
            }

            if (this.Language == LANGUAGES.CPP)
            {
                pc_methodPrefixDef = CallingConvention + ClassName + "::" + pc_methodPrefix;
                pc_classPrefix = ClassName + "::";
            }


            AllServices.Clear();
            AddAllServices(device);

            FriendlyNameTable.Clear();
            Fix(device, 0, serviceNames);

            PrivateClassDeclarations = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            PublicClassDeclarations = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            CodeProcessor cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.NewLine = this.CodeNewLine;

            cs.ClassDefinitions = PrivateClassDeclarations;
            cs.PublicClassDefinitions = PublicClassDeclarations;
            PrivateClassDeclarations.CodeTab = Indent;
            PublicClassDeclarations.CodeTab = Indent;
            cs.CodeTab = Indent;

            #endregion

            #region New Style UPnPMicroStack.h
            WS = SourceCodeRepository.GetMicroStack_H_Template(pc_methodPrefix);

            #region UPnP/1.1 Complex Types
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildComplexTypeDefinitionsAndHeaders(SL, cs, SeqTable, ChoTable, ref SequenceCounter, ref ChoiceCounter, this.pc_methodPrefix, this.pc_methodLibPrefix);
            WS = WS.Replace("//{{{ComplexTypeCode}}}", cs.ToString());
            #endregion
            #region Function Callbacks
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);

            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;

                foreach (UPnPAction action in service.Actions)
                {
                    cs.Append("	typedef void(*UPnP_ActionHandler_" + serviceNames[service] + "_" + action.Name + ") (void* upnptoken");
                    foreach (UPnPArgument args in action.Arguments)
                    {
                        if (args.Direction == "in")
                        {
                            if (args.RelatedStateVar.ComplexType == null)
                            {
                                cs.Append("," + ToCType(args.RelatedStateVar.GetNetType().FullName) + " " + args.Name);
                                if (args.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append(",int _" + args.Name + "Length");
                                }
                            }
                            else
                            {
                                // Complex Type
                                cs.Append(", struct " + args.RelatedStateVar.ComplexType.Name_LOCAL + " *" + args.Name);
                            }
                        }
                    }
                    cs.Append(");" + cl);
                }
            }
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                if (Configuration.EXTERN_Callbacks == true || serviceNames[service].ToString() == "DeviceSecurity")
                {
                    foreach (UPnPAction action in service.Actions)
                    {
                        if (serviceNames[service].ToString() == "DeviceSecurity")
                        {
                            cs.Append("extern void " + pc_methodLibPrefix + serviceNames[service] + "_" + action.Name + "(void* upnptoken");
                        }
                        else
                        {
                            cs.Append("extern void " + pc_methodPrefix + serviceNames[service] + "_" + action.Name + "(void* upnptoken");
                        }
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                cs.Append("," + ToCType(arg.RelatedStateVar.GetNetType().ToString()) + " " + arg.Name);
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append(",int _" + arg.Name + "Length");
                                }
                            }
                        }
                        cs.Append(");" + cl);
                    }
                }
            }
            if (Configuration.EXTERN_Callbacks == false)
            {
                cs.Comment("UPnP Set Function Pointers Methods");
                cs.Append("extern void (*" + pc_methodPrefixDef + "FP_PresentationPage) (void* upnptoken,struct packetheader *packet);" + cl);
                BuildFunctionPointerHeaders(cs, device, serviceNames);
                cs.Append(cl);
            }
            else
            {
                cs.Append("extern void " + pc_methodPrefix + "PresentationRequest(void* upnptoken, struct packetheader *packet);" + cl);
            }
            WS = WS.Replace("//{{{UPnP_Set_Function_Pointer_Methods}}}", cs.ToString());
            #endregion
            #region Invocation Response Methods
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);

            cs.Comment("Invocation Response Methods");
            cs.Append("void " + pc_methodPrefixDef + "Response_Error(const UPnPSessionToken UPnPToken, const int ErrorCode, const char* ErrorMsg);" + cl);
            cs.Append("void " + pc_methodPrefixDef + "ResponseGeneric(const UPnPSessionToken UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params);" + cl);
            if (ServiceGenerator.ServiceConfiguration.HasFragmentedActions(device))
            {
                cs.Append("int " + pc_methodPrefixDef + "AsyncResponse_START(const UPnPSessionToken UPnPToken, const char* actionName, const char* serviceUrnWithVersion);" + cl);
                cs.Append("int " + pc_methodPrefixDef + "AsyncResponse_DONE(const UPnPSessionToken UPnPToken, const char* actionName);" + cl);
                cs.Append("int " + pc_methodPrefixDef + "AsyncResponse_OUT(const UPnPSessionToken UPnPToken, const char* outArgName, const char* bytes, const int byteLength, enum ILibAsyncSocket_MemoryOwnership bytesMemoryOwnership,const int startArg, const int endArg);" + cl);
            }
            BuildUPnPResponseHeaders(cs, device, serviceNames);
            WS = WS.Replace("//{{{Invocation_Response_Methods}}}", cs.ToString());

            #endregion
            #region Eventing Methods
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Comment("State Variable Eventing Methods");
            BuildStateVariableHeaders(cs, device, serviceNames);
            WS = WS.Replace("//{{{Eventing_Methods}}}", cs.ToString());
            #endregion
            #region Multicast Eventing Methods
            if (device.ArchitectureVersion != "1.0")
            {
                cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                cs.Comment("State Variable Multicast-Eventing Methods");
                BuildMulticastStateVariableHeaders(cs, device, serviceNames);
                WS = WS.Replace("//{{{MulticastEventing_Methods}}}", cs.ToString());
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_MulticastEventing}}}", "//{{{END_MulticastEventing}}}", WS);
                WS = BuildMulticastStateVariableHeaders2(WS, device, serviceNames);
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_MulticastEventing_Specific}}}", "//{{{END_MulticastEventing_Specific}}}", WS);
            }
            else
            {
                WS = WS.Replace("//{{{MulticastEventing_Methods}}}", "");
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_MulticastEventing}}}", "//{{{END_MulticastEventing}}}", WS);
            }
            #endregion

            #region CreateMicroStack Definition
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("UPnPMicroStackToken UPnPCreateMicroStack(void *Chain, ");
            BuildCreateMicroStackDefinition(cs, device, 1);
            cs.Append("const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum);" + cl);
            WS = WS.Replace("//{{{CreateMicroStackHeader}}}", cs.ToString());
            #endregion

            #region Device Object Model
            if (this.Configuration.DynamicObjectModel)
            {
                WS = WS.Replace("//{{{ObjectDefintions}}}", DeviceObjectGenerator.GetDeviceObjectsString(device));
                WS = WS.Replace("//{{{GetConfiguration}}}", "struct UPnP_Device_" + device.User2.ToString() + "* UPnPGetConfiguration();" + cl);
            }
            else
            {
                WS = WS.Replace("//{{{ObjectDefintions}}}", "");
                WS = WS.Replace("//{{{GetConfiguration}}}", "");
            }
            #endregion


            #region Prefixes
            WS = WS.Replace("UPnP", this.pc_methodPrefix);
            WS = WS.Replace("ILib", this.pc_methodLibPrefix);
            #endregion


            #region Write to disk

            W = File.CreateText(outputDirectory.FullName + "\\" + pc_methodPrefix + "MicroStack.h");

            W.Write(WS);
            W.Close();
            #endregion
            #endregion

            #region New Style UPnPMicroStack.c
            WS = SourceCodeRepository.GetMicroStack_C_Template(pc_methodPrefix);


            #region Set Function Pointers
            if (Configuration.EXTERN_Callbacks == false)
            {
                cs.Comment("UPnP Set Function Pointers Methods");
                string staticdef = "";
                if (this.Language == LANGUAGES.CPP) staticdef = "static ";
                cs.Append("void (*" + pc_methodPrefixDef + "FP_PresentationPage) (void* upnptoken,struct packetheader *packet);" + cl);
                cs.PublicClassDefinitions.Append(staticdef + "void (*" + pc_methodPrefix + "FP_PresentationPage) (void* upnptoken,struct packetheader *packet);" + cl);
                BuildFunctionPointers(cs, device, serviceNames);
                cs.Append(cl);
                WS = WS.Replace("//{{{FunctionPointers}}}", cs.ToString());
            }
            else
            {
                WS = WS.Replace("//{{{FunctionPointers}}}", "");
            }

            #endregion
            #region Build and Compress Device Description
            //Compress Device Description
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);

            BuildDeviceDescription(cs, device);
            BuildServiceDescriptions(cs, device, serviceNames);

            WS = WS.Replace("//{{{CompressedDescriptionDocs}}}", cs.ToString());
            #endregion
            #region Object Model
            if (this.Configuration.DynamicObjectModel)
            {
                WS = WS.Replace("//{{{ObjectDefintions}}}", DeviceObjectGenerator.GetPopulatedDeviceObjectsString(device));
                WS = DeviceObjectGenerator.BuildDeviceDescriptionStreamer(device, WS);
                WS = SourceCodeRepository.RemoveTag("//{{{Device_Object_Model_BEGIN}}}", "//{{{Device_Object_Model_END}}}", WS);
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Default_Model_BEGIN}}}", "//{{{Device_Default_Model_END}}}", WS);
                cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                cs.Append("struct UPnP_Device_" + device.User2.ToString() + "* UPnPGetConfiguration()" + cl);
                cs.Append("{" + cl);
                cs.Append("	return(&(" + DeviceObjectGenerator.GetDeviceIdentifier(device) + "));" + cl);
                cs.Append("}" + cl);
                WS = WS.Replace("//{{{GetConfiguration}}}", cs.ToString());
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{Device_Object_Model_BEGIN}}}", "//{{{Device_Object_Model_END}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{Device_Default_Model_BEGIN}}}", "//{{{Device_Default_Model_END}}}", WS);
                WS = WS.Replace("//{{{GetConfiguration}}}", "");
            }
            #endregion

            #region FragmentedResponseSystem
            if (ServiceGenerator.ServiceConfiguration.HasFragmentedActions(device))
            {
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_FragmentedResponseSystem}}}", "//{{{END_FragmentedResponseSystem}}}", WS);
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_FragmentedResponseSystem}}}", "//{{{END_FragmentedResponseSystem}}}", WS);
            }
            #endregion

            #region CreateMicroStackDefinition
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("UPnPMicroStackToken UPnPCreateMicroStack(void *Chain, ");
            BuildCreateMicroStackDefinition(cs, device, 1);
            cs.Append("const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)" + cl);
            WS = WS.Replace("//{{{CreateMicroStackDefinition}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("	RetVal->DeviceDescriptionLength = snprintf(RetVal->DeviceDescription, len, DDT");
            if (device.ArchitectureVersion != "1.0")
            {
                cs.Append(", RetVal->ConfigID");
            }
            BuildCreateMicroStackDefinition_sprintf(cs, device, 1);
            cs.Append(");" + cl);
            WS = WS.Replace("//{{{CreateMicroStack_sprintf}}}", cs.ToString());


            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("	len = 10 + UPnPDeviceDescriptionTemplateLengthUX");
            int nd = BuildCreateMicroStackDefinition_Malloc(cs, device, 1);
            cs.Append(" + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * " + nd.ToString() + ");" + cl);
            cs.Append("	if ((RetVal->DeviceDescription = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);" + cl);

            WS = WS.Replace("//{{{DeviceDescriptionMalloc}}}", cs.ToString());

            if (nd == 1)
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EmbeddedDevice>0}}}", "//{{{END_EmbeddedDevices>0}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_EmbeddedDevices=0}}}", "//{{{END_EmbeddedDevices=0}}}", WS);
            }
            else
            {
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_EmbeddedDevice>0}}}", "//{{{END_EmbeddedDevices>0}}}", WS);
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EmbeddedDevices=0}}}", "//{{{END_EmbeddedDevices=0}}}", WS);
            }
            #endregion
            #region CreateMicroStack  -->  Object Meta Data
            if (Configuration.DynamicObjectModel)
            {
                cs = new CodeProcessor(new StringBuilder(), false);
                BuildObjectMetaData(cs, device, 1);
                WS = WS.Replace("//{{{ObjectModel_MetaData}}}", cs.ToString());
            }
            else
            {
                WS = WS.Replace("//{{{ObjectModel_MetaData}}}", "");
            }
            #endregion


            #region Presentation Page Support
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);

            if (DeviceConf.AdvertisesPresentationPage)
            {
                cs.Append("/* Presentation Page Support */" + cl);
                cs.Append("if (header->DirectiveObjLength>=4 && memcmp(header->DirectiveObj,\"/web\",4)==0)" + cl);
                cs.Append("{" + cl);
                if (Configuration.EXTERN_Callbacks)
                {
                    cs.Append("	UPnPPresentationRequest((void*)session,header);" + cl);
                }
                else
                {
                    cs.Append("	UPnPFP_PresentationPage((void*)session,header);" + cl);
                }
                cs.Append("}" + cl);
                cs.Append("else ");
            }

            WS = WS.Replace("//{{{PRESENTATIONPAGE}}}", cs.ToString());
            #endregion

            #region #define vs method definition, on SSDP related stuff (Embedded Devices)
            if (this.GetNumberOfTotalEmbeddedDevices(device) == 0)
            {
                // No Embedded Devices
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_EmbeddedDevices=0}}}", "//{{{END_EmbeddedDevices=0}}}", WS);
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EmbeddedDevices>0}}}", "//{{{END_EmbeddedDevices>0}}}", WS);
            }
            else
            {
                // There are Embedded Devices
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EmbeddedDevices=0}}}", "//{{{END_EmbeddedDevices=0}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_EmbeddedDevices>0}}}", "//{{{END_EmbeddedDevices>0}}}", WS);
            }
            #endregion

            #region ssdp:all
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildSSDPALL_Response(device, cs, 0);
            WS = WS.Replace("//{{{SSDP:ALL}}}", cs.ToString());
            #endregion
            #region ssdp:other

            if (DoesDeviceHaveAnyNonVersionOneComponents(device))
            {
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_VERSION>1}}}", "//{{{END_VERSION>1}}}", WS);
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_VERSION>1}}}", "//{{{END_VERSION>1}}}", WS);
            }
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildMSEARCHHandler_device(device, cs, 0);
            WS = WS.Replace("//{{{SSDP:OTHER}}}", cs.ToString());
            #endregion
            #region FragmentedSendNotify Case statements
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("					case 1:" + cl);
            cs.Append("                     " + this.pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), 0, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\");" + cl);
            cs.Append("                     break;" + cl);
            BuildFragmentedNotify_CaseStatement(cs, device, 2, 0, false);
            WS = WS.Replace("//{{{FragmentedSendNotifyCaseStatements}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("					case 1:" + cl);
            cs.Append("						" + this.pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), 0, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\");" + cl);
            cs.Append("						break;" + cl);
            BuildFragmentedNotify_CaseStatement(cs, device, 2, 0, true);
            WS = WS.Replace("//{{{FragmentedSendNotifyV6CaseStatements}}}", cs.ToString());
            #endregion
            
            #region SendNotify "For statement"
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("					" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), 0, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\");" + cl);
            BuildNotifyPackets_Device(cs, device, 0, false);
            WS = WS.Replace("//{{{SendNotifyForStatement}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("					" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), 0, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\");" + cl);
            BuildNotifyPackets_Device(cs, device, 0, true);
            WS = WS.Replace("//{{{SendNotifyV6ForStatement}}}", cs.ToString());
            WS = WS.Replace("!NUMPACKETS!", this.CountPackets(device).ToString());
            #endregion
            
            #region SendByeBye "For statement"
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("			      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\", 0);" + cl);
            BuildByeByePackets_Device(cs, device, 0, false);
            WS = WS.Replace("//{{{SendByeByeForStatement}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            cs.Append("			      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\", 0);" + cl);
            BuildByeByePackets_Device(cs, device, 0, true);
            WS = WS.Replace("//{{{SendByeByeV6ForStatement}}}", cs.ToString());
            #endregion

            #region Device Icon
            if (device.Icon != null)
            {
                WS = SourceCodeRepository.RemoveTag("//{{{DeviceIcon_Begin}}}", "//{{{DeviceIcon_End}}}", WS);
                string iconString;
                MemoryStream ms = new MemoryStream();

                device.Icon.Save(ms, System.Drawing.Imaging.ImageFormat.Png);
                HTTPMessage r;
                if (Configuration.HTTP_1dot1)
                {
                    r = new HTTPMessage("1.1");
                }
                else
                {
                    r = new HTTPMessage("1.0");
                }
                r.StatusCode = 200;
                r.StatusData = "OK";
                r.ContentType = "image/png";
                r.BodyBuffer = ms.ToArray();

                // Small PNG
                DeviceObjectGenerator.InjectBytes(out iconString, r.RawPacket, this.CodeNewLine, false);
                WS = WS.Replace("{{{IconLength_SMPNG}}}", r.RawPacket.Length.ToString());
                WS = WS.Replace("{{{IconLength_HEAD_SMPNG}}}", (r.RawPacket.Length - r.BodyBuffer.Length).ToString());
                WS = WS.Replace("{{{ICON_SMPNG}}}", iconString);

                // Small JPG
                ms = new MemoryStream();
                device.Icon.Save(ms, System.Drawing.Imaging.ImageFormat.Jpeg);
                r.ContentType = "image/jpg";
                r.BodyBuffer = ms.ToArray();
                DeviceObjectGenerator.InjectBytes(out iconString, r.RawPacket, this.CodeNewLine, false);
                WS = WS.Replace("{{{IconLength_SMJPG}}}", r.RawPacket.Length.ToString());
                WS = WS.Replace("{{{IconLength_HEAD_SMJPG}}}", (r.RawPacket.Length - r.BodyBuffer.Length).ToString());
                WS = WS.Replace("{{{ICON_SMJPG}}}", iconString);

                if (device.Icon2 != null)
                {
                    // Large PNG
                    ms = new MemoryStream();
                    device.Icon2.Save(ms, System.Drawing.Imaging.ImageFormat.Png);
                    r.ContentType = "image/png";
                    r.BodyBuffer = ms.ToArray();
                    DeviceObjectGenerator.InjectBytes(out iconString, r.RawPacket, this.CodeNewLine, false);
                    WS = WS.Replace("{{{IconLength_LGPNG}}}", r.RawPacket.Length.ToString());
                    WS = WS.Replace("{{{IconLength_HEAD_LGPNG}}}", (r.RawPacket.Length - r.BodyBuffer.Length).ToString());
                    WS = WS.Replace("{{{ICON_LGPNG}}}", iconString);

                    // Large JPG
                    ms = new MemoryStream();
                    device.Icon2.Save(ms, System.Drawing.Imaging.ImageFormat.Jpeg);
                    r.ContentType = "image/jpg";
                    r.BodyBuffer = ms.ToArray();
                    DeviceObjectGenerator.InjectBytes(out iconString, r.RawPacket, this.CodeNewLine, false);
                    WS = WS.Replace("{{{IconLength_LGJPG}}}", r.RawPacket.Length.ToString());
                    WS = WS.Replace("{{{IconLength_HEAD_LGJPG}}}", (r.RawPacket.Length - r.BodyBuffer.Length).ToString());
                    WS = WS.Replace("{{{ICON_LGJPG}}}", iconString);
                }

            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{DeviceIcon_Begin}}}", "//{{{DeviceIcon_End}}}", WS);
            }
            #endregion

            #region Dispatch Methods
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            Build_DispatchMethods(cs, serviceNames);
            WS = WS.Replace("//{{{DispatchMethods}}}", cs.ToString());
            #endregion
            #region Dispatch Controller
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            if (BuildHTTPSink_CONTROL(cs, device, serviceNames, "") != "")
            {
                cs.Append("	else" + cl);
                cs.Append("	{" + cl);
                cs.Append("		RetVal=1;" + cl);
                cs.Append("	}" + cl);
            }
            else
            {
                cs.Append("		RetVal=1;" + cl);
            }
            WS = WS.Replace("//{{{DispatchControl}}}", cs.ToString());
            #endregion

            #region Invocation Response Methods
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildUPnPResponse(cs, device, serviceNames);
            WS = WS.Replace("//{{{InvokeResponseMethods}}}", cs.ToString());
            #endregion
            #region GetInitialEventBody
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildEventHelpers_InitialEvent(cs, serviceNames);
            WS = WS.Replace("//{{{InitialEventBody}}}", cs.ToString());
            #endregion
            #region Multicast Events
            if (device.ArchitectureVersion != "1.0")
            {
                cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                BuildMulticastSoapEvents(cs, device, serviceNames);
                WS = WS.Replace("//{{{SetStateMethods}}}", "//{{{SetStateMethods}}}\r\n" + cs.ToString());

                WS = BuildMulticastSoapEventsProcessor(WS, device, serviceNames);

                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CHECK_MULTICASTVARIABLE}}}", "//{{{END_CHECK_MULTICASTVARIABLE}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_MulticastEventing}}}", "//{{{END_MulticastEventing}}}", WS);
                WS = WS.Replace("{{{VARDEFS}}}", "");
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_MulticastEventing}}}", "//{{{END_MulticastEventing}}}", WS);
            }
            #endregion
            #region SetState Methods
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildSoapEvents(cs, device, serviceNames);
            WS = WS.Replace("//{{{SetStateMethods}}}", cs.ToString());
            #endregion


            #region UnSubscribeDispatcher
            string packet = "HTTP/!HTTPVERSION! %d %s\\r\\nContent-Length: 0\\r\\n\\r\\n";

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            first = "";
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                UPnPDebugObject obj = new UPnPDebugObject(service);
                string name = (string)obj.GetField("__eventurl");

                cs.Append("	" + first + "if (header->DirectiveObjLength==" + (name.Length + 1).ToString() + " && memcmp(header->DirectiveObj + 1,\"" + name + "\"," + name.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);

                cs.Append("		Info = " + pc_methodPrefix + "RemoveSubscriberInfo(&(((struct " + pc_methodPrefix + "DataObject*)session->User)->HeadSubscriberPtr_" + (string)en.Key + "),&(((struct " + pc_methodPrefix + "DataObject*)session->User)->NumberOfSubscribers_" + (string)en.Key + "),SID,SIDLength);" + cl);

                cs.Append("		if (Info != NULL)" + cl);
                cs.Append("		{" + cl);
                cs.Append("			--Info->RefCount;" + cl);
                cs.Append("			if (Info->RefCount == 0)" + cl);
                cs.Append("			{" + cl);
                cs.Append("				" + pc_methodPrefix + "DestructSubscriberInfo(Info);" + cl);
                cs.Append("			}" + cl);
                cs.Append("			packetlength = snprintf(packet, 50, \"" + packet + "\", 200, \"OK\");" + cl);

                cs.Append("			" + this.pc_methodLibPrefix + "WebServer_Send_Raw(session, packet, packetlength, 0, 1);" + cl);

                cs.Append("		}" + cl);
                cs.Append("		else" + cl);
                cs.Append("		{" + cl);
                cs.Append("			packetlength = snprintf(packet, 50, \"" + packet + "\", 412, \"Invalid SID\");" + cl);

                cs.Append("			" + this.pc_methodLibPrefix + "WebServer_Send_Raw(session, packet, packetlength, 0, 1);" + cl);

                cs.Append("		}" + cl);
                cs.Append("	}" + cl);
                first = "else ";
            }
            WS = WS.Replace("//{{{UnSubscribeDispatcher}}}", cs.ToString());
            #endregion
            #region SubscribeDispatcher
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            first = Build_SubscribeEvents_Device("", cs, device, serviceNames);
            if (first != "")
            {
                cs.Append("	else" + cl);
                cs.Append("	{" + cl);

                cs.Append("		" + this.pc_methodLibPrefix + "WebServer_Send_Raw(session,\"HTTP/1.1 412 Invalid Service Name\\r\\nContent-Length: 0\\r\\n\\r\\n\",56,1,1);" + cl);

                cs.Append("	}" + cl);
            }
            else
            {

                cs.Append("	" + this.pc_methodLibPrefix + "WebServer_Send_Raw(session,\"HTTP/1.1 412 Invalid Service Name\\r\\nContent-Length: 0\\r\\n\\r\\n\",56,1,1);" + cl);

            }
            WS = WS.Replace("//{{{SubscribeEventsDispatcher}}}", cs.ToString());
            #endregion
            #region Maximum Subscription Timeout
            WS = WS.Replace("{{{UPnP_MAX_SUBSCRIPTION_TIMEOUT}}}", ((ServiceGenerator.Configuration)device.User).MaxSubscriptionTimeout.ToString());
            #endregion

            #region State Variables
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                UPnPService s = (UPnPService)en.Value;

                foreach (UPnPStateVariable v in s.GetStateVariables())
                {
                    if (v.SendEvent)
                    {
                        cs.Append("	char* " + name + "_" + v.Name + ";" + cl);
                    }
                    if (v.MulticastEvent)
                    {
                        cs.Append("	int " + name + "_" + v.Name + "_SEQ;" + cl);
                    }
                }
            }
            WS = WS.Replace("//{{{StateVariables}}}", cs.ToString());
            #endregion
            #region Subscriber Head Pointer
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                cs.Append("	struct SubscriberInfo *HeadSubscriberPtr_" + name + ";" + cl);
                cs.Append("	int NumberOfSubscribers_" + name + ";" + cl);
            }
            WS = WS.Replace("//{{{HeadSubscriberPointers}}}", cs.ToString());
            #endregion
            #region UPnPExpireSubscriberInfo
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            first = "";
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                cs.Append("	" + first + "if (d->HeadSubscriberPtr_" + name + "==t)" + cl);
                cs.Append("	{" + cl);
                cs.Append("		--(d->NumberOfSubscribers_" + name + ");" + cl);
                cs.Append("	}" + cl);
                first = "else ";
            }
            WS = WS.Replace("//{{{UPnPExpireSubscriberInfo1}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            first = "";
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                cs.Append("	" + first + "if (d->HeadSubscriberPtr_" + name + "==info)" + cl);
                cs.Append("	{" + cl);
                cs.Append("		d->HeadSubscriberPtr_" + name + " = info->Next;" + cl);
                cs.Append("		if (info->Next!=NULL)" + cl);
                cs.Append("		{" + cl);
                cs.Append("			info->Next->Previous = NULL;" + cl);
                cs.Append("		}" + cl);
                cs.Append("	}" + cl);
                first = "else ";
            }
            cs.Append("	" + first + cl);
            if (first != "")
            {
                cs.Append("	{" + cl);
            }
            cs.Append("		// Error" + cl);
            cs.Append("		return;" + cl);
            if (first != "")
            {
                cs.Append("	}" + cl);
            }
            WS = WS.Replace("//{{{UPnPExpireSubscriberInfo2}}}", cs.ToString());
            #endregion
            #region TryToSubscribe HeadPointer Initializer
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                cs.Append("	if (strncmp(ServiceName,\"" + name + "\"," + name.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);

                cs.Append("		TotalSubscribers = &(dataObject->NumberOfSubscribers_" + name + ");" + cl);
                cs.Append("		HeadPtr = &(dataObject->HeadSubscriberPtr_" + name + ");" + cl);

                cs.Append("	}" + cl);
            }
            WS = WS.Replace("//{{{SubscribeHeadPointerInitializer}}}", cs.ToString());
            #endregion
            #region TryToSubscribe Initial Event
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            first = "";
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                UPnPService s = (UPnPService)en.Value;
                bool HasEvents = false;
                foreach (UPnPStateVariable v in s.GetStateVariables())
                {
                    if (v.SendEvent)
                    {
                        HasEvents = true;
                        break;
                    }
                }

                if (HasEvents)
                {
                    cs.Append("	" + first + "if (strcmp(ServiceName,\"" + name + "\")==0)" + cl);
                    cs.Append("	{" + cl);
                    cs.Append("		UPnPGetInitialEventBody_" + name + "(dataObject,&packetbody,&packetbodyLength);" + cl);
                    cs.Append("	}" + cl);
                    first = "else ";
                }
            }
            WS = WS.Replace("//{{{TryToSubscribe_InitialEvent}}}", cs.ToString());
            #endregion
            #region Subscription Renewal HeadPointer Initializer
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            first = "";
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                UPnPDebugObject obj = new UPnPDebugObject(service);
                string name = (string)obj.GetField("__eventurl");
                string sname = (string)en.Key;

                cs.Append(first + " if (pathlength==" + (name.Length + 1).ToString() + " && memcmp(path+1,\"" + name + "\"," + name.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);

                cs.Append("		info = ((struct " + this.pc_methodPrefix + "DataObject*)ReaderObject->User)->HeadSubscriberPtr_" + sname + ";" + cl);

                cs.Append("	}" + cl);
                first = "else";
            }
            WS = WS.Replace("//{{{RenewHeadInitializer}}}", cs.ToString());
            #endregion
            #region SendEvent HeadPointer Initializer
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en.Reset();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                cs.Append("	if (strncmp(eventname,\"" + name + "\"," + name.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);
                cs.Append("		info = UPnPObject->HeadSubscriberPtr_" + name + ";" + cl);
                cs.Append("	}" + cl);
            }
            WS = WS.Replace("//{{{SendEventHeadPointerInitializer}}}", cs.ToString());
            #endregion

            #region HeadDispatcher
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildHTTPSink_SCPD_HEAD(cs, device, serviceNames);
            WS = WS.Replace("//{{{HeadDispatcher}}}", cs.ToString());
            #endregion
            #region GetDispatcher
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildHTTPSink_SCPD(cs, device, serviceNames);
            WS = WS.Replace("//{{{GetDispatcher}}}", cs.ToString());

            if (DeviceObjectGenerator.CalculateMaxAllowedValues(device, 0) != 0)
            {
                WS = SourceCodeRepository.RemoveTag("//{{{HASALLOWEDVALUES_BEGIN}}}", "//{{{HASALLOWEDVALUES_END}}}", WS);
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{HASALLOWEDVALUES_BEGIN}}}", "//{{{HASALLOWEDVALUES_END}}}", WS);
            }
            #endregion

            #region DestroyMicroStack
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                UPnPService s = (UPnPService)en.Value;

                foreach (UPnPStateVariable v in s.GetStateVariables())
                {
                    if (v.SendEvent && v.GetNetType() != typeof(bool))
                    {
                        cs.Append("	free(upnp->" + name + "_" + v.Name + ");" + cl);
                    }
                }
            }
            WS = WS.Replace("//{{{UPnPDestroyMicroStack_FreeEventResources}}}", cs.ToString());

            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;

                cs.Append("	sinfo = upnp->HeadSubscriberPtr_" + name + ";" + cl);
                cs.Append("	while(sinfo!=NULL)" + cl);
                cs.Append("	{" + cl);
                cs.Append("		sinfo2 = sinfo->Next;" + cl);
                cs.Append("		UPnPDestructSubscriberInfo(sinfo);" + cl);
                cs.Append("		sinfo = sinfo2;" + cl);
                cs.Append("	}" + cl);
            }
            WS = WS.Replace("//{{{UPnPDestroyMicroStack_DestructSubscriber}}}", cs.ToString());


            #endregion

            #region UPnP/1.1 Complex Types
            cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
            BuildComplexTypeParser(SeqTable, ChoTable, cs, SL, this.pc_methodPrefix, this.pc_methodLibPrefix);
            CPEmbeddedCGenerator.BuildComplexTypeSerializer(SeqTable, ChoTable, cs, SL, this.pc_methodPrefix, this.pc_methodLibPrefix);
            WS = WS.Replace("//{{{ComplexTypeCode}}}", cs.ToString());
            #endregion

            #region HTTP Version
            if (!Configuration.HTTP_1dot1)
            {
                WS = WS.Replace("!HTTPVERSION!", "1.0");
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}", "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}", WS);
            }
            else
            {
                WS = WS.Replace("!HTTPVERSION!", "1.1");
                WS = SourceCodeRepository.RemoveTag("//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}", "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}", WS);
            }
            WS = WS.Replace("!MICROSTACKVERSION!", this.UseVersion);
            #endregion
            #region UPnP Specific Version
            if (device.ArchitectureVersion == "1.0")
            {
                // UPnP/1.0
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_UPnP/1.1_Specific}}}", "//{{{END_UPnP/1.1_Specific}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_UPnP/1.0_Specific}}}", "//{{{END_UPnP/1.0_Specific}}}", WS);
                WS = WS.Replace("!UPNPVERSION!", "1.0");
            }
            else
            {
                // UPnP/1.1
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_UPnP/1.0_Specific}}}", "//{{{END_UPnP/1.0_Specific}}}", WS);
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_UPnP/1.1_Specific}}}", "//{{{END_UPnP/1.1_Specific}}}", WS);
                WS = WS.Replace("!UPNPVERSION!", "1.1");
            }
            #endregion


            #region Remove Event Processing if no evented State Variables
            if (DeviceHasEvents(device))
            {
                WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_EVENTPROCESSING}}}", "//{{{END_EVENTPROCESSING}}}", WS);
            }
            else
            {
                WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_EVENTPROCESSING}}}", "//{{{END_EVENTPROCESSING}}}", WS);
            }
            #endregion

            #region Prefixes
            WS = FixPrefix_DeviceService(device, WS);
            WS = WS.Replace("UPnP/", "upnp/");
            WS = WS.Replace("UPnPError", "_upnperror_");
            WS = WS.Replace(" UPnP ", " _upnp_ ");
            WS = WS.Replace("UPnP", this.pc_methodPrefix);
            WS = WS.Replace("ILib", this.pc_methodLibPrefix);
            WS = WS.Replace("_upnperror_", "UPnPError");
            WS = WS.Replace("upnp/", "UPnP/");
            WS = WS.Replace(" _upnp_ ", " UPnP ");
            WS = FixPrefix2_DeviceService(device, WS);
            #endregion

            #region Reformat String
            WS = CodeProcessor.ProcessCode(WS, Indent);
            #endregion

            #region Write to disk
            if (this.Language == LANGUAGES.C)
            {
                W = File.CreateText(outputDirectory.FullName + "\\" + pc_methodPrefix + "MicroStack.c");
            }
            else
            {
                W = File.CreateText(outputDirectory.FullName + "\\" + pc_methodPrefix + "MicroStack.cpp");
            }
            W.Write(WS);
            W.Close();
            #endregion




            #endregion

            #region Sample Application
            if (BuildSampleApp)
            {
                WS = SampleApp;

                #region Display Message
                WS = WS.Replace("{{{INITSTRING}}}", (string)FriendlyNameTable[device] + " {{{INITSTRING}}}");
                #endregion

                #region ImplementationMethods
                if (!Configuration.BareBonesSample)
                {
                    cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                    BuildMainUserCode(cs, device, serviceNames);
                    WS = WS.Replace("//{{{DEVICE_INVOCATION_DISPATCH}}}", "//{{{DEVICE_INVOCATION_DISPATCH}}}" + cl + cs.ToString());
                }
                #endregion
                #region ImplementationMethods: Function Pointer Initialization
                if (!Configuration.BareBonesSample)
                {
                    if (Configuration.EXTERN_Callbacks == false)
                    {
                        cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                        cs.ident = 1;

                        if (DeviceConf.AdvertisesPresentationPage)
                        {
                            cs.Append(pc_methodPrefix + "FP_PresentationPage=&" + pc_methodPrefix + "PresentationRequest;" + cl);
                        }
                        BuildMain_SetFunctionPointers(cs, device, serviceNames);
                        WS = WS.Replace("//{{{INVOCATION_FP}}}", "//{{{INVOCATION_FP}}}" + cl + cs.ToString());
                    }
                }
                #endregion

                #region PresentationRequest
                if (DeviceConf.AdvertisesPresentationPage)
                {
                    cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                    cs.Append("void UPnPPresentationRequest(void* upnptoken, struct packetheader *packet)" + cl);
                    cs.Append("{" + cl);
                    cs.Append("	printf(\"UPnP Presentation Request: %s %s\\r\\n\", packet->Directive,packet->DirectiveObj);" + cl);
                    cs.Append(cl);
                    cs.Append("	/* TODO: Add Web Response Code Here... */" + cl);
                    cs.Append("	printf(\"HOST: %x\\r\\n\",UPnPGetLocalInterfaceToHost(upnptoken));" + cl);
                    cs.Append(cl);
                    cs.Append("	ILibWebServer_Send_Raw((struct ILibWebServer_Session *)upnptoken, \"HTTP/1.1 200 OK\\r\\nContent-Length: 0\\r\\n\\r\\n\" , 38 , 1, 1);" + cl);
                    cs.Append("}" + cl);

                    WS = WS.Replace("//{{{PresentationRequest}}}", "//{{{PresentationRequest}}}" + cl + cs.ToString());
                }
                #endregion

                #region MicroStack.h include
                WS = WS.Replace("//{{{MicroStack_Include}}}", "//{{{MicroStack_Include}}}" + cl + "#include \"UPnPMicroStack.h\"");
                #endregion
                #region MicroStack Veriable Declaration
                WS = WS.Replace("//{{{MICROSTACK_VARIABLE}}}", "//{{{MICROSTACK_VARIABLE}}}" + cl + "void *UPnPmicroStack;");
                #endregion
                #region CreateMicroStack
                if (!Configuration.BareBonesSample)
                {
                    cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                    cs.ident = 1;
                    cs.Append("    // TODO: Each device must have a unique device identifier (UDN)" + cl);
                    cs.Append("	" + this.pc_methodPrefix + "microStack = " + pc_methodPrefix + "CreateMicroStack(MicroStackChain, ");
                    CreateMicroStack_Device_Values(cs, device);
                    cs.Append("\"" + Guid.NewGuid().ToString() + "\", \"0000001\", " + DeviceConf.SSDPCycleTime.ToString() + ", " + DeviceConf.WebPort.ToString() + ");" + cl);
                    WS = WS.Replace("//{{{CREATE_MICROSTACK}}}", "//{{{CREATE_MICROSTACK}}}" + cl + cs.ToString());
                }
                #endregion

                #region InitialEvent Initialization
                if (!Configuration.BareBonesSample)
                {
                    cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                    BuildStateVariableEventingSample(cs, device, serviceNames);
                    WS = WS.Replace("//{{{STATEVARIABLES_INITIAL_STATE}}}", "//{{{STATEVARIABLES_INITIAL_STATE}}}" + cl + cs.ToString());
                }
                #endregion

                #region IPAddress Monitor
                if (Configuration.DefaultIPAddressMonitor)
                {
                    WS = WS.Replace("//{{{IPAddress_Changed}}}", "//{{{IPAddress_Changed}}}" + cl + "UPnPIPAddressListChanged(UPnPmicroStack);");

                    if (Configuration.TargetPlatform == ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2)
                    {
                        WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}", "//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}", WS);
                        WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", "//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", WS);
                    }
                    else
                    {
                        WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", "//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", WS);
                        WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}", "//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}", WS);
                    }
                }
                else
                {
                    WS = WS.Replace("//{{{IPAddress_Changed}}}", "");
                    WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}", "//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}", WS);
                    WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", "//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}", WS);
                }
                #endregion

                #region Prefixes
                WS = WS.Replace("UPnPAbstraction.h", "_upnpabstraction.h_");
                WS = WS.Replace("pUPnP", "_pupnp_");
                WS = WS.Replace("CUPnP_", "_cupnp_");
                WS = WS.Replace("UPnP/", "upnp/");
                WS = WS.Replace("UPnPControlPointStructs.h", "upnpcontrolpointstructs.h");
                WS = WS.Replace("UPnPDevice", "upnpdevice");
                WS = WS.Replace("UPnPService", "upnpservice");
                WS = WS.Replace("SubscribeForUPnPEvents", "subscribeforupnpevents");
                WS = WS.Replace("UnSubscribeUPnPEvents", "unsubscribeupnpevents");

                WS = WS.Replace("UPnPError", "_upnperror_");
                WS = WS.Replace(" UPnP ", " _upnp_ ");
                WS = WS.Replace("UPnP", this.pc_methodPrefix);
                WS = WS.Replace("ILib", this.pc_methodLibPrefix);
                WS = WS.Replace("_upnperror_", "UPnPError");
                WS = WS.Replace("upnp/", "UPnP/");
                WS = WS.Replace(" _upnp_ ", " UPnP ");
                WS = WS.Replace("_pupnp_", "pUPnP");
                WS = WS.Replace("_cupnp_", "CUPnP_");

                WS = WS.Replace("upnpdevice", "UPnPDevice");
                WS = WS.Replace("upnpservice", "UPnPService");
                WS = WS.Replace("upnpcontrolpointstructs.h", "UPnPControlPointStructs.h");
                WS = WS.Replace("subscribeforupnpevents", "SubscribeForUPnPEvents");
                WS = WS.Replace("unsubscribeupnpevents", "UnSubscribeUPnPEvents");
                WS = WS.Replace("_upnpabstraction.h_", "UPnPAbstraction.h");

                #endregion

                SampleApp = WS;
            }
            #endregion

            Log("UPnP Stack Generation Complete.");

            return true;
        }

        private int BuildFragmentedNotify_CaseStatement(CodeProcessor cs, UPnPDevice device, int StartNum, int number, bool ipv6)
        {
            cs.Append("					case " + StartNum + ":" + cl);
            if (!ipv6)
                cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), " + number.ToString() + ", \"\", \"uuid:\", FNS->upnp->UDN);" + cl);
            else
                cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), " + number.ToString() + ", \"\", \"uuid:\", FNS->upnp->UDN);" + cl);
            cs.Append("						break;" + cl);
            ++StartNum;
            cs.Append("					case " + StartNum + ":" + cl);
            if (!ipv6)
                cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), " + number.ToString() + ", \"::" + device.DeviceURN + "\", \"" + device.DeviceURN + "\", \"\");" + cl);
            else
                cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), " + number.ToString() + ", \"::" + device.DeviceURN + "\", \"" + device.DeviceURN + "\", \"\");" + cl);
            cs.Append("						break;" + cl);
            ++StartNum;
            foreach (UPnPService service in device.Services)
            {
                cs.Append("					case " + StartNum + ":" + cl);
                if (!ipv6)
                    cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV4[i]), " + number.ToString() + ", \"::" + service.ServiceURN + "\", \"" + service.ServiceURN + "\", \"\");" + cl);
                else
                    cs.Append("						" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(FNS->upnp->NOTIFY_SEND_socks6[i], FNS->upnp, (struct sockaddr*)&(FNS->upnp->AddressListV6[i]), " + number.ToString() + ", \"::" + service.ServiceURN + "\", \"" + service.ServiceURN + "\", \"\");" + cl);
                cs.Append("						break;" + cl);
                ++StartNum;
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                StartNum = BuildFragmentedNotify_CaseStatement(cs, d, StartNum, ++number, ipv6);
            }
            return (StartNum);
        }

        private string BuildEventHelpers_GetLine(UPnPStateVariable v)
        {
            string datablock = "<e:property><" + v.Name + ">%s</" + v.Name + "></e:property>";
            return (datablock);
        }
        private void BuildEventHelpers_InitialEvent(CodeProcessor cs, Hashtable serviceNames)
        {
            //string start_block = "<?xml version=\\\"1.0\\\" encoding=\\\"utf-8\\\"?><e:propertyset xmlns:e=\\\"urn:schemas-upnp-org:event-1-0\\\">";
            //string end_block = "</e:propertyset>";
            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();
            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();
            while (en.MoveNext())
            {
                string name = (string)en.Key;
                UPnPService service = (UPnPService)en.Value;

                // Figure out if this service as any evented state variables
                bool eventedvars = false;
                foreach (UPnPStateVariable v in service.GetStateVariables())
                {
                    if (v.SendEvent == true) { eventedvars = true; break; }
                }
                if (eventedvars == false) continue;

                // Define the initial event body method for this service
                cs.Define("void " + pc_methodPrefixDef + "GetInitialEventBody_" + name + "(struct " + pc_methodPrefix + "DataObject *UPnPObject,char ** body, int *bodylength)");
                cs.Append("{" + cl);
                cs.Append("	int TempLength;" + cl);

                StringBuilder ev = new StringBuilder();
                //ev.Append(start_block);
                foreach (UPnPStateVariable V in service.GetStateVariables())
                {
                    if (V.SendEvent)
                    {
                        ev.Append(this.BuildEventHelpers_GetLine(V));
                    }
                }
                //ev.Append(end_block);
                string eventbody = ev.ToString();
                if (eventbody.Length != 0)
                {
                    eventbody = eventbody.Substring(13, eventbody.Length - (13 + 14));
                }

                cs.Append("	TempLength = (int)(" + eventbody.Length.ToString());
                foreach (UPnPStateVariable V in service.GetStateVariables())
                {
                    if (V.SendEvent)
                    {
                        cs.Append("+(int)strlen(UPnPObject->" + name + "_" + V.Name + ")");
                    }
                }
                cs.Append(");" + cl);
                cs.Append("	if ((*body = (char*)malloc(sizeof(char) * TempLength)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("	*bodylength = snprintf(*body, sizeof(char) * TempLength, \"" + eventbody + "\"");
                foreach (UPnPStateVariable V in service.GetStateVariables())
                {
                    if (V.SendEvent)
                    {
                        cs.Append(",UPnPObject->" + name + "_" + V.Name);
                    }
                }
                cs.Append(");" + cl);
                cs.Append("}" + cl);
            }

        }

        private int GetNumberOfTotalEmbeddedDevices(UPnPDevice device)
        {
            int RetVal = 0;
            if (device.Root == false) RetVal = 1;

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                RetVal += GetNumberOfTotalEmbeddedDevices(d);
            }
            return (RetVal);
        }


        private void TypeCheckURI(CodeProcessor cs, UPnPArgument args)
        {
            cs.Append("	TempParser = " + pc_methodLibPrefix + "ParseString(p_" + args.Name + ", 0, p_" + args.Name + "Length, \"://\",3);" + cl);
            cs.Append("	if (TempParser->NumResults!=2)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Argument[" + args.Name + "] illegal format\");" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
            }
            cs.Append("		" + this.pc_methodLibPrefix + "DestructParserResults(TempParser);" + cl);
            cs.Append("		return;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	else" + cl);
            cs.Append("	{" + cl);
            cs.Append("		_" + args.Name + " = p_" + args.Name + ";" + cl);
            cs.Append("		_" + args.Name + "Length = p_" + args.Name + "Length;" + cl);
            cs.Append("		" + this.pc_methodLibPrefix + "DestructParserResults(TempParser);" + cl);
            cs.Append("	}" + cl);
        }

        private void Build_TypeCheckIntegral(CodeProcessor cs)
        {
            cs.Define("int " + pc_methodPrefixDef + "TypeCheckIntegral(char* inVar, int inVarLength, long MinVal, long MaxVal, void *outVar, char *varName,struct HTTPReaderObject *ReaderObject)");
            cs.Append("{" + cl);
            cs.Append("	long TempLong;" + cl);
            //cs.Append("	char* msg;"+cl);
            cs.Append("	int OK = 0;" + cl);
            cs.Append("	if (" + pc_methodLibPrefix + "GetLong(inVar, inVarLength, &TempLong)!=0)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		OK=-1;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (!(TempLong >= MinVal && TempLong <= MaxVal))" + cl);
            cs.Append("	{" + cl);
            cs.Append("		OK = -2;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK == -1)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		if ((msg = (char*)malloc(25 + (int)strlen(varName))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("		snprintf(msg, 25 + (int)strlen(varName), \"Argument[%s] illegal value\", varName);" + cl);
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                cs.Append("		free(msg);" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
            }
            cs.Append("		return -1;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK == -2)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		if ((msg = (char*)malloc(25 + (int)strlen(varName))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("		snprintf(msg, 25 + (int)strlen(varName), \"Argument[%s] out of range\", varName);" + cl);
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                cs.Append("		free(msg);" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
            }
            cs.Append("		return -1;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	*((long*)outVar) = TempLong;" + cl);
            cs.Append("	return 0;" + cl);
            cs.Append("}" + cl);
        }
        private void Build_TypeCheckUnsignedIntegral(CodeProcessor cs)
        {
            cs.Define("int " + pc_methodPrefixDef + "TypeCheckUnsignedIntegral(char* inVar, int inVarLength, unsigned long MinVal, unsigned long MaxVal, void *outVar, char *varName,struct HTTPReaderObject *ReaderObject)");
            cs.Append("{" + cl);
            cs.Append("	unsigned long TempULong;" + cl);
            cs.Append("	int OK = 0;" + cl);
            cs.Append("	char *msg;" + cl);
            cs.Append("	if (" + pc_methodLibPrefix + "GetULong(inVar, inVarLength, &TempULong)!=0)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		OK=-1;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (!(TempULong >= MinVal && TempULong <= MaxVal))" + cl);
            cs.Append("	{" + cl);
            cs.Append("		OK=-2;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK==-1)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		if ((msg = (char*)malloc(25 + (int)strlen(varName))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("		snprintf(msg, 25 + (int)strlen(varName), \"Argument[%s] illegal value\", varName);" + cl);
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                cs.Append("		free(msg);" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
            }
            cs.Append("		return(-1);" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK==-2)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		if ((msg = (char*)malloc(25 + (int)strlen(varName))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("		snprintf(msg, 25 + (int)strlen(varName), \"Argument[%s] out of range\", varName);" + cl);
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                cs.Append("		free(msg);" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
            }
            cs.Append("		return(-1);" + cl);
            cs.Append("	}" + cl);
            cs.Append("	*((unsigned long*)outVar) = TempULong;" + cl);
            cs.Append("	return(0);" + cl);
            cs.Append("}" + cl);
        }

        private void Build_TypeCheckString(CodeProcessor cs, Hashtable serviceNames)
        {
            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();

            cs.Define("int " + pc_methodPrefixDef + "TypeCheckString(char* inVar, int inVarLength, char* ServiceName, char* StateVariable, char** outVar, int* outVarLength, char* varName, struct HTTPReaderObject *ReaderObject)");
            cs.Append("{" + cl);
            cs.Append("	int OK = 0;" + cl);
            cs.Append("	char* msg;" + cl);
            while (en.MoveNext())
            {
                UPnPService S = (UPnPService)en.Value;
                string key = (string)en.Key;

                bool Needed = false;
                foreach (UPnPStateVariable V in S.GetStateVariables())
                {
                    if (V.AllowedStringValues != null)
                    {
                        Needed = true;
                        break;
                    }
                }

                if (Needed)
                {
                    cs.Append("	if (strncmp(ServiceName,\"" + key + "\"," + key.Length.ToString() + ") == 0)" + cl);
                    cs.Append("	{" + cl);
                    foreach (UPnPStateVariable V in S.GetStateVariables())
                    {
                        if (V.AllowedStringValues != null)
                        {
                            cs.Append("		if (strncmp(StateVariable,\"" + V.Name + "\"," + V.Name.Length.ToString() + ") == 0)" + cl);
                            cs.Append("		{" + cl);
                            cs.Append("			OK = -1;" + cl);
                            bool first = true;
                            foreach (string AllowedString in V.AllowedStringValues)
                            {
                                if (first == false) cs.Append("else ");
                                first = false;
                                cs.Append("			if (inVarLengt h== " + AllowedString.Length.ToString() + ")" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				if (memcmp(inVar,\"" + AllowedString + "\"," + AllowedString.Length.ToString() + ") == 0) {OK = 0;}" + cl);
                                cs.Append("			}" + cl);
                            }
                            cs.Append("			if (OK != 0)" + cl);
                            cs.Append("			{" + cl);
                            if (Configuration.ExplicitErrorEncoding == true)
                            {
                                cs.Append("				if ((msg = (char*)malloc(65)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("				snprintf(msg, 65, \"Argument[%s] contains a value that is not in AllowedValueList\", varName);" + cl);
                                cs.Append("				" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                                cs.Append("				free(msg);" + cl);
                            }
                            else
                            {
                                cs.Append("				" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
                            }
                            cs.Append("				return -1;" + cl);
                            cs.Append("			}" + cl);
                            cs.Append("			*outVar = inVar;" + cl);
                            cs.Append("			*outVarLength = inVarLength;" + cl);
                            cs.Append("			return(0);" + cl);
                            cs.Append("		}" + cl);
                        }
                    }
                    cs.Append("	}" + cl);
                }
            }
            cs.Append("}" + cl);
        }

        private void TypeCheckBoolean(CodeProcessor cs, UPnPArgument args)
        {
            cs.Append("	OK=0;" + cl);
            cs.Append("	if (p_" + args.Name + "Length == 4)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (strncasecmp(p_" + args.Name + ",\"true\",4) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			_" + args.Name + " = 1;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (p_" + args.Name + "Length == 5)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (strncasecmp(p_" + args.Name + ",\"false\",5) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			_" + args.Name + " = 0;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (p_" + args.Name + "Length == 1)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (memcmp(p_" + args.Name + ",\"0\",1) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			_" + args.Name + " = 0;" + cl);
            cs.Append("		}" + cl);
            cs.Append("		if (memcmp(p_" + args.Name + ",\"1\",1) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			_" + args.Name + " = 1;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK == 0)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] illegal value\");" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
            }
            cs.Append("		return;" + cl);
            cs.Append("	}" + cl);
        }
        private void TypeCheckIntegral(CodeProcessor cs, UPnPArgument args)
        {
            UPnPDebugObject obj = new UPnPDebugObject(args.RelatedStateVar.GetNetType());
            switch (args.RelatedStateVar.GetNetType().FullName)
            {
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    cs.Append("	OK = " + pc_methodLibPrefix + "GetLong(p_" + args.Name + ",p_" + args.Name + "Length, &TempLong);" + cl);
                    break;
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    cs.Append("	OK = " + pc_methodLibPrefix + "GetULong(p_" + args.Name + ",p_" + args.Name + "Length, &TempULong);" + cl);
                    break;
            }
            cs.Append("	if (OK!=0)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Argument[" + args.Name + "] illegal value\");" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
            }
            cs.Append("		return;" + cl);
            cs.Append("	}" + cl);

            bool endtag = false;
            switch (args.RelatedStateVar.GetNetType().FullName)
            {
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    if (args.RelatedStateVar.Minimum == null && args.RelatedStateVar.Maximum == null)
                    {
                        // No need to check anything since this is without bounds.
                    }
                    else
                    {
                        // Check lower and upper bounds.
                        endtag = true;
                        cs.Append("	else" + cl);
                        cs.Append("	{" + cl);
                        if (!Configuration.DynamicObjectModel)
                        {
                            cs.Append("		if (!(TempLong>=");
                            if (args.RelatedStateVar.Minimum != null)
                            {
                                cs.Append("(long)0x" + ToHex(args.RelatedStateVar.Minimum));
                            }
                            else
                            {
                                cs.Append("(long)0x" + ToHex(obj.GetStaticField("MinValue")));
                            }
                            cs.Append(" && TempLong<=");
                            if (args.RelatedStateVar.Maximum != null)
                            {
                                cs.Append("(long)0x" + ToHex(args.RelatedStateVar.Maximum));
                            }
                            else
                            {
                                cs.Append("(long)0x" + ToHex(obj.GetStaticField("MaxValue")));
                            }
                            cs.Append("))" + cl);
                        }
                        else
                        {
                            string vIdent = DeviceObjectGenerator.GetStateVariableIdentifier(args.RelatedStateVar);

                            cs.Append("	OK = 0;" + cs.NewLine);
                            cs.Append("	if (" + vIdent + "->MinMaxStep[0]!=NULL)" + cs.NewLine);
                            cs.Append("	{" + cs.NewLine);
                            cs.Append("		" + pc_methodLibPrefix + "GetLong(" + vIdent + "->MinMaxStep[0],(int)strlen(" + vIdent + "->MinMaxStep[0]), &TempLong2);" + cl);
                            cs.Append("		if (TempLong<TempLong2){OK=1;}" + cs.NewLine);
                            cs.Append("	}" + cs.NewLine);
                            cs.Append("	if (" + vIdent + "->MinMaxStep[1]!=NULL)" + cs.NewLine);
                            cs.Append("	{" + cs.NewLine);
                            cs.Append("		" + pc_methodLibPrefix + "GetLong(" + vIdent + "->MinMaxStep[1],(int)strlen(" + vIdent + "->MinMaxStep[1]), &TempLong2);" + cl);
                            cs.Append("		if (TempLong>TempLong2){OK=1;}" + cs.NewLine);
                            cs.Append("	}" + cs.NewLine);
                            cs.Append("	if (OK!=0)" + cs.NewLine);
                        }
                        cs.Append("		{" + cl);
                        if (Configuration.ExplicitErrorEncoding == true)
                        {
                            cs.Append("		  " + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Argument[" + args.Name + "] out of Range\");" + cl);
                        }
                        else
                        {
                            cs.Append("		  " + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
                        }
                        cs.Append("		  return;" + cl);
                        cs.Append("		}" + cl);
                    }
                    break;
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    if (args.RelatedStateVar.Minimum == null && args.RelatedStateVar.Maximum == null)
                    {
                        // No need to check anything since this is an int without bounds.
                    }
                    else
                    {
                        endtag = true;
                        cs.Append("	else" + cl);
                        cs.Append("	{" + cl);
                        if (!this.Configuration.DynamicObjectModel)
                        {
                            cs.Append("		if (!(TempULong>=");
                            if (args.RelatedStateVar.Minimum != null)
                            {
                                cs.Append("(unsigned long)0x" + ToHex(args.RelatedStateVar.Minimum));
                            }
                            else
                            {
                                cs.Append("(unsigned long)0x" + ToHex(obj.GetStaticField("MinValue")));
                            }
                            cs.Append(" && TempULong<=");
                            if (args.RelatedStateVar.Maximum != null)
                            {
                                cs.Append("(unsigned long)0x" + ToHex(args.RelatedStateVar.Maximum));
                            }
                            else
                            {
                                cs.Append("(unsigned long)0x" + ToHex(obj.GetStaticField("MaxValue")));
                            }
                            cs.Append("))" + cl);
                        }
                        else
                        {
                            string vIdent = DeviceObjectGenerator.GetStateVariableIdentifier(args.RelatedStateVar);

                            cs.Append("	OK = 0;" + cs.NewLine);
                            cs.Append("	if (" + vIdent + "->MinMaxStep[0]!=NULL)" + cs.NewLine);
                            cs.Append("	{" + cs.NewLine);
                            cs.Append("		" + pc_methodLibPrefix + "GetULong(" + vIdent + "->MinMaxStep[0],(int)strlen(" + vIdent + "->MinMaxStep[0]), &TempULong2);" + cl);
                            cs.Append("		if (TempULong<TempULong2){OK=1;}" + cs.NewLine);
                            cs.Append("	}" + cs.NewLine);
                            cs.Append("	if (" + vIdent + "->MinMaxStep[1]!=NULL)" + cs.NewLine);
                            cs.Append("	{" + cs.NewLine);
                            cs.Append("		" + pc_methodLibPrefix + "GetULong(" + vIdent + "->MinMaxStep[1],(int)strlen(" + vIdent + "->MinMaxStep[1]), &TempULong2);" + cl);
                            cs.Append("		if (TempULong>TempULong2){OK=1;}" + cs.NewLine);
                            cs.Append("	}" + cs.NewLine);
                            cs.Append("	if (OK!=0)" + cs.NewLine);
                        }
                        cs.Append("		{" + cl);
                        if (Configuration.ExplicitErrorEncoding == true)
                        {
                            cs.Append("		  " + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Argument[" + args.Name + "] out of Range\");" + cl);
                        }
                        else
                        {
                            cs.Append("		  " + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
                        }
                        cs.Append("		  return;" + cl);
                        cs.Append("		}" + cl);
                    }
                    break;
            }

            switch (args.RelatedStateVar.GetNetType().FullName)
            {
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    cs.Append("	_" + args.Name + " = (" + ToCType(args.RelatedStateVar.GetNetType().FullName) + ")TempLong;" + cl);
                    break;
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    cs.Append("	_" + args.Name + " = (" + ToCType(args.RelatedStateVar.GetNetType().FullName) + ")TempULong;" + cl);
                    break;
            }
            if (endtag == true) cs.Append(" }" + cl);
        }
        private void TypeCheckDateTime(CodeProcessor cs, UPnPArgument args)
        {
            cs.Append("	p_" + args.Name + "[p_" + args.Name + "Length]=0;" + cl);
            cs.Append("	_" + args.Name + " = " + this.pc_methodLibPrefix + "Time_Parse(p_" + args.Name + ");" + cl);
        }
        private void TypeCheckString(CodeProcessor cs, UPnPArgument args)
        {
            cs.Append("	_" + args.Name + "Length = " + this.pc_methodLibPrefix + "InPlaceXmlUnEscape(p_" + args.Name + ");" + cl);
            cs.Append("	_" + args.Name + " = p_" + args.Name + ";" + cl);

            if (args.RelatedStateVar.AllowedStringValues != null)
            {
                if (!Configuration.DynamicObjectModel)
                {
                    cs.Append("	if (");
                    bool first = true;
                    foreach (string val in args.RelatedStateVar.AllowedStringValues)
                    {
                        if (first == false) cs.Append("&& ");
                        first = false;
                        cs.Append("memcmp(_" + args.Name + ", \"" + val + "\\0\"," + (val.Length + 1).ToString() + ") != 0" + cl);
                    }
                    cs.Append("	)" + cl);
                }
                else
                {
                    string vIdent = DeviceObjectGenerator.GetStateVariableIdentifier(args.RelatedStateVar);

                    cs.Append("	for(OK=0;OK<UPnP_StateVariable_AllowedValues_MAX;++OK)" + cs.NewLine);
                    cs.Append("	{" + cs.NewLine);
                    cs.Append("		if (" + vIdent + "->AllowedValues[OK]!=NULL)" + cs.NewLine);
                    cs.Append("		{" + cs.NewLine);
                    cs.Append("			if (strcmp(_" + args.Name + "," + vIdent + "->AllowedValues[OK])==0)" + cs.NewLine);
                    cs.Append("			{" + cs.NewLine);
                    cs.Append("				OK=0;" + cs.NewLine);
                    cs.Append("				break;" + cs.NewLine);
                    cs.Append("			}" + cs.NewLine);
                    cs.Append("		}" + cs.NewLine);
                    cs.Append("		else" + cs.NewLine);
                    cs.Append("		{" + cs.NewLine);
                    cs.Append("			break;" + cs.NewLine);
                    cs.Append("		}" + cs.NewLine);
                    cs.Append("	}" + cs.NewLine);
                    cs.Append("	if (OK!=0)" + cs.NewLine);
                }

                cs.Append("	{" + cl);
                if (Configuration.ExplicitErrorEncoding == true)
                {
                    cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Argument[" + args.Name + "] contains a value that is not in AllowedValueList\");" + cl);
                }
                else
                {
                    cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,402,\"Illegal value\");" + cl);
                }
                cs.Append("		return;" + cl);
                cs.Append("	}" + cl);
            }
        }
        private void Build_TypeCheckBoolean(CodeProcessor cs)
        {
            cs.Define("int " + pc_methodPrefixDef + "TypeCheckBoolean(char *inVar, int inVarLength, int* BoolValue, char* varName, struct HTTPReaderObject *ReaderObject)");
            cs.Append("{" + cl);
            cs.Append("	int OK = 0;" + cl);
            cs.Append("	char* msg;" + cl);
            cs.Append("	if (inVarLength == 4)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (strncasecmp(inVar, \"true\", 4) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			*BoolValue = 1;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (inVarLength == 5)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (strncasecmp(inVar, \"false\", 5) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			*BoolValue = 0;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (inVarLength==1)" + cl);
            cs.Append("	{" + cl);
            cs.Append("		if (memcmp(inVar, \"0\", 1) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			*BoolValue = 0;" + cl);
            cs.Append("		}" + cl);
            cs.Append("		if (memcmp(inVar, \"1\", 1) == 0)" + cl);
            cs.Append("		{" + cl);
            cs.Append("			OK = 1;" + cl);
            cs.Append("			*BoolValue = 1;" + cl);
            cs.Append("		}" + cl);
            cs.Append("	}" + cl);
            cs.Append("	if (OK == 0)" + cl);
            cs.Append("	{" + cl);
            if (Configuration.ExplicitErrorEncoding == true)
            {
                cs.Append("		if ((msg = (char*)malloc(25 + (int)strlen(varName))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                cs.Append("		snprintf(msg, 25 + (int)strlen(varName), \"Argument[%s] illegal value\", varName);" + cl);
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, msg);" + cl);
                cs.Append("		free(msg);" + cl);
            }
            else
            {
                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
            }
            cs.Append("		return -1;" + cl);
            cs.Append("	}" + cl);
            cs.Append("	else" + cl);
            cs.Append("	{" + cl);
            cs.Append("		return 0;" + cl);
            cs.Append("	}" + cl);
            cs.Append("}" + cl);
        }


        private void Build_DispatchMethods(CodeProcessor cs, Hashtable serviceNames)
        {
            SortedList SL = new SortedList();

            IDictionaryEnumerator en = serviceNames.GetEnumerator();
            UPnPService service;
            string name;

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }

            en = SL.GetEnumerator();

            while (en.MoveNext())
            {
                int numArgs = 0;
                service = (UPnPService)en.Value;
                name = (string)en.Key;
                foreach (UPnPAction action in service.Actions)
                {
                    numArgs = 0;
                    foreach (UPnPArgument args in action.ArgumentList)
                    {
                        if (args.Direction == "in") { ++numArgs; }
                    }

                    // Define a macro version
                    if (numArgs == 0)
                    {
                        cs.Define("#define " + pc_methodPrefixDef + "Dispatch_" + name + "_" + action.Name + "(buffer,offset,bufferLength, session)\\");
                        cs.Append("{\\" + cl);
                        if (name != "DeviceSecurity")
                        {
                            if (Configuration.EXTERN_Callbacks == false)
                            {
                                cs.Append("	if (" + pc_methodPrefix + "FP_" + name + "_" + action.Name + " == NULL)\\" + cl);
                                cs.Append("		" + pc_methodPrefix + "Response_Error(session,501,\"No Function Handler\");\\" + cl);
                                cs.Append("	else\\" + cl);
                                cs.Append("		" + pc_methodPrefix + "FP_" + name + "_" + action.Name + "((void*)session);\\" + cl);
                            }
                            else
                            {
                                cs.Append("	" + pc_methodPrefix + name + "_" + action.Name + "((void*)session);\\" + cl);
                            }
                        }
                        else
                        {
                            cs.Append("	" + pc_methodLibPrefix + name + "_" + action.Name + "((void*)session);\\" + cl);
                        }
                        cs.Append("}" + cl);
                        cs.Append(cl);
                    }

                    if (numArgs > 0)
                    {
                        cs.Define("void " + pc_methodPrefixDef + "Dispatch_" + name + "_" + action.Name + "(char *buffer, int offset, int bufferLength, struct " + this.pc_methodLibPrefix + "WebServer_Session *ReaderObject)");
                        cs.Append("{" + cl);

                        bool varlong = false;
                        bool varlongtemp = false;
                        bool varulong = false;
                        bool varulongtemp = false;
                        bool varuuri = false;
                        bool varok = false;
                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in")
                            {
                                varok = true;
                                switch (args.RelatedStateVar.GetNetType().ToString())
                                {
                                    case "System.Uri":
                                        varuuri = true;
                                        break;
                                    case "System.Byte":
                                    case "System.UInt16":
                                    case "System.UInt32":
                                        varulong = true;
                                        if (args.RelatedStateVar.Maximum != null || args.RelatedStateVar.Minimum != null)
                                        {
                                            varulongtemp = true;
                                        }
                                        break;
                                    case "System.SByte":
                                    case "System.Int16":
                                    case "System.Int32":
                                        varlong = true;
                                        if (args.RelatedStateVar.Maximum != null || args.RelatedStateVar.Minimum != null)
                                        {
                                            varlongtemp = true;
                                        }
                                        break;
                                    case "System.Boolean":
                                    case "System.Char":
                                    case "System.Single":
                                    case "System.Double":
                                    case "System.Byte[]":
                                    case "System.String":
                                        break;
                                }
                            }
                        }

                        //cs.Append("	char *TempString;"+cl);
                        if (varlong == true)
                        {
                            cs.Append("	long TempLong;" + cl);
                            if (varlongtemp && Configuration.DynamicObjectModel)
                            {
                                cs.Append("	long TempLong2;" + cl);
                            }
                        }
                        if (varulong == true)
                        {
                            cs.Append("	unsigned long TempULong;" + cl);
                            if (varulongtemp && Configuration.DynamicObjectModel)
                            {
                                cs.Append("	unsigned long TempULong2;" + cl);
                            }
                        }
                        if (varuuri == true) cs.Append("	struct parser_result *TempParser;" + cl);
                        if (varok == true) cs.Append("	int OK = 0;" + cl);

                        //cs.Comment("Service Variables");

                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in")
                            {
                                cs.Append("	char *p_" + args.Name + " = NULL;" + cl);
                                cs.Append("	int p_" + args.Name + "Length = 0;" + cl);
                                if (args.RelatedStateVar.ComplexType == null)
                                {
                                    cs.Append("	" + ToCType(args.RelatedStateVar.GetNetType().FullName) + " _" + args.Name + " = " + ToEmptyValue(args.RelatedStateVar.GetNetType().FullName) + ";" + cl);
                                    if (ToCType(args.RelatedStateVar.GetNetType().FullName) == "char*" || ToCType(args.RelatedStateVar.GetNetType().FullName) == "unsigned char*")
                                    {
                                        cs.Append("	int _" + args.Name + "Length;" + cl);
                                    }
                                }
                                else
                                {
                                    cs.Append(" struct " + args.RelatedStateVar.ComplexType.Name_LOCAL + " *_" + args.Name + "=NULL;" + cl);
                                }
                            }
                        }


                        //
                        // Setup the XML Parsing
                        //
                        cs.Append("	struct " + this.pc_methodLibPrefix + "XMLNode *xnode = " + this.pc_methodLibPrefix + "ParseXML(buffer, offset, bufferLength);" + cl);
                        cs.Append("	struct " + this.pc_methodLibPrefix + "XMLNode *root = xnode;" + cl);
                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in" && args.RelatedStateVar.ComplexType != null)
                            {
                                cs.Append("	struct " + this.pc_methodLibPrefix + "XMLNode *tnode, *tnode_root;" + cl);
                                cs.Append("	char* tempText;" + cl);
                                cs.Append("	int tempTextLength;" + cl);
                                break;
                            }
                        }
                        cs.Append("	if (" + this.pc_methodLibPrefix + "ProcessXMLNodeList(root)!=0)" + cl);
                        cs.Append("	{" + cl);
                        cs.Comment("The XML is not well formed!");
                        cs.Append("	" + this.pc_methodLibPrefix + "DestructXMLNodeList(root);" + cl);
                        cs.Append("	" + this.pc_methodPrefix + "Response_Error(ReaderObject, 501, \"Invalid XML\");" + cl);
                        cs.Append("	return;" + cl);
                        cs.Append("	}" + cl);



                        cs.Append("	while(xnode != NULL)" + cl);
                        cs.Append("	{" + cl);
                        cs.Append("		if (xnode->StartTag != 0 && xnode->NameLength == 8 && memcmp(xnode->Name, \"Envelope\", 8)==0)" + cl);
                        cs.Append("		{" + cl);
                        cs.Append("			// Envelope" + cl);
                        cs.Append("			xnode = xnode->Next;" + cl);
                        cs.Append("			while(xnode != NULL)" + cl);
                        cs.Append("			{" + cl);
                        cs.Append("				if (xnode->StartTag!=0 && xnode->NameLength == 4 && memcmp(xnode->Name, \"Body\", 4) == 0)" + cl);
                        cs.Append("				{" + cl);
                        cs.Append("					// Body" + cl);
                        cs.Append("					xnode = xnode->Next;" + cl);
                        cs.Append("					while(xnode != NULL)" + cl);
                        cs.Append("					{" + cl);
                        cs.Append("						if (xnode->StartTag != 0 && xnode->NameLength == " + action.Name.Length.ToString() + " && memcmp(xnode->Name, \"" + action.Name + "\"," + action.Name.Length.ToString() + ") == 0)" + cl);
                        cs.Append("						{" + cl);
                        cs.Append("							// Inside the interesting part of the SOAP" + cl);
                        cs.Append("							xnode = xnode->Next;" + cl);
                        cs.Append("							while(xnode != NULL)" + cl);
                        cs.Append("							{" + cl);

                        int argflag = 1;
                        string eLsE = "";
                        foreach (UPnPArgument arg in action.ArgumentList)
                        {
                            if (arg.Direction == "in")
                            {
                                cs.Append("								" + eLsE + "if (xnode->NameLength == " + arg.Name.Length.ToString() + " && memcmp(xnode->Name, \"" + arg.Name + "\"," + arg.Name.Length.ToString() + ")==0)" + cl);
                                cs.Append("								{" + cl);
                                if (arg.RelatedStateVar.ComplexType == null)
                                {
                                    cs.Append("									p_" + arg.Name + "Length = " + this.pc_methodLibPrefix + "ReadInnerXML(xnode, &p_" + arg.Name + ");" + cl);
                                    if ((arg.RelatedStateVar.GetNetType().FullName == "System.String") || (arg.RelatedStateVar.GetNetType().FullName == "System.Uri"))
                                    {
                                        cs.Append("									p_" + arg.Name + "[p_" + arg.Name + "Length]=0;" + cl);
                                    }
                                }
                                else
                                {
                                    // Complex Type
                                    cs.Append("									tempTextLength = " + this.pc_methodLibPrefix + "ReadInnerXML(xnode, &tempText);" + cl);
                                    cs.Append("									tempText[tempTextLength] = 0;" + cl);
                                    cs.Append("									if (ReaderObject->Reserved9 == 0)" + cl);
                                    cs.Append("									{" + cl);
                                    cs.Append("										// Legacy" + cl);
                                    cs.Append("										tempTextLength = " + this.pc_methodLibPrefix + "InPlaceXmlUnEscape(tempText);" + cl);
                                    cs.Append("										tnode_root = tnode = " + this.pc_methodLibPrefix + "ParseXML(tempText,0,tempTextLength);" + cl);
                                    cs.Append("										" + this.pc_methodLibPrefix + "ProcessXMLNodeList(tnode_root);" + cl);
                                    cs.Append("										_" + arg.Name + " = " + this.pc_methodPrefix + "Parse_" + arg.RelatedStateVar.ComplexType.Name_LOCAL + "(tnode);" + cl);
                                    cs.Append("										" + this.pc_methodLibPrefix + "DestructXMLNodeList(tnode_root);" + cl);
                                    cs.Append("									}" + cl);
                                    cs.Append("									else" + cl);
                                    cs.Append("									{" + cl);
                                    cs.Append("										// UPnP/1.1 Enabled" + cl);
                                    cs.Append("										_" + arg.Name + " = " + this.pc_methodPrefix + "Parse_" + arg.RelatedStateVar.ComplexType.Name_LOCAL + "(xnode->Next);" + cl);
                                    cs.Append("									}" + cl);
                                }
                                cs.Append("										OK |= " + argflag + ";" + cl);
                                argflag = argflag << 1;
                                cs.Append("								}" + cl);
                                eLsE = "else ";
                            }
                        }

                        cs.Append("								if (xnode->Peer == NULL)" + cl);
                        cs.Append("								{" + cl);
                        cs.Append("									xnode = xnode->Parent;" + cl);
                        cs.Append("									break;" + cl);
                        cs.Append("								}" + cl);
                        cs.Append("								else" + cl);
                        cs.Append("								{" + cl);
                        cs.Append("									xnode = xnode->Peer;" + cl);
                        cs.Append("								}" + cl);

                        cs.Append("							}" + cl);
                        cs.Append("						}" + cl);
                        cs.Append("						if (xnode != NULL)" + cl);
                        cs.Append("						{" + cl);
                        cs.Append("							if (xnode->Peer == NULL)" + cl);
                        cs.Append("							{" + cl);
                        cs.Append("								xnode = xnode->Parent;" + cl);
                        cs.Append("								break;" + cl);
                        cs.Append("							}" + cl);
                        cs.Append("							else" + cl);
                        cs.Append("							{" + cl);
                        cs.Append("								xnode = xnode->Peer;" + cl);
                        cs.Append("							}" + cl);
                        cs.Append("						}" + cl);
                        cs.Append("					}" + cl);
                        cs.Append("				}" + cl);
                        cs.Append("				if (xnode != NULL)" + cl);
                        cs.Append("				{" + cl);
                        cs.Append("					if (xnode->Peer == NULL)" + cl);
                        cs.Append("					{" + cl);
                        cs.Append("						xnode = xnode->Parent;" + cl);
                        cs.Append("						break;" + cl);
                        cs.Append("					}" + cl);
                        cs.Append("					else" + cl);
                        cs.Append("					{" + cl);
                        cs.Append("						xnode = xnode->Peer;" + cl);
                        cs.Append("					}" + cl);
                        cs.Append("				}" + cl);
                        cs.Append("			}" + cl);
                        cs.Append("		}" + cl);
                        cs.Append("		if (xnode != NULL){xnode = xnode->Peer;}" + cl);
                        cs.Append("	}" + cl);
                        cs.Append("	" + this.pc_methodLibPrefix + "DestructXMLNodeList(root);" + cl);



                        cs.Append("	if (OK != " + (argflag - 1) + ")" + cl);
                        cs.Append("	{" + cl);
                        if (Configuration.ExplicitErrorEncoding == true)
                        {
                            cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Incorrect Arguments\");" + cl);
                        }
                        else
                        {
                            cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject, 402, \"Illegal value\");" + cl);
                        }
                        cs.Append("		return;" + cl);
                        cs.Append("	}" + cl);
                        cs.Append(cl);

                        cs.Comment("Type Checking");

                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in")
                            {
                                switch (args.RelatedStateVar.GetNetType().FullName)
                                {
                                    case "System.Boolean":
                                        TypeCheckBoolean(cs, args);
                                        break;
                                    case "System.Int16":
                                    case "System.Int32":
                                    case "System.UInt16":
                                    case "System.UInt32":
                                    case "System.Byte":
                                    case "System.SByte":
                                        TypeCheckIntegral(cs, args);
                                        break;
                                    case "System.Uri":
                                        TypeCheckURI(cs, args);
                                        break;
                                    case "System.DateTime":
                                        TypeCheckDateTime(cs, args);
                                        break;
                                    case "System.Byte[]":
                                        cs.Append("	_" + args.Name + "Length = " + this.pc_methodLibPrefix + "Base64Decode(p_" + args.Name + ",p_" + args.Name + "Length,&_" + args.Name + ");" + cl);
                                        break;
                                    case "System.String":
                                    default:
                                        if (args.RelatedStateVar.ComplexType == null)
                                        {
                                            TypeCheckString(cs, args);
                                        }
                                        break;
                                }
                            }
                        }

                        string FPtrType = "(void (__cdecl *)(void *";
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                FPtrType += ("," + ToCType(arg.RelatedStateVar.GetNetType().ToString()));
                            }
                        }
                        FPtrType += "))";

                        if (name != "DeviceSecurity")
                        {
                            if (Configuration.EXTERN_Callbacks == false)
                            {
                                cs.Append("	if (" + pc_methodPrefix + "FP_" + name + "_" + action.Name + " == NULL)" + cl);
                                cs.Append("		" + pc_methodPrefix + "Response_Error(ReaderObject,501,\"No Function Handler\");" + cl);
                                cs.Append("	else" + cl);
                                cs.Append("		" + pc_methodPrefix + "FP_" + name + "_" + action.Name + "((void*)ReaderObject");
                            }
                            else
                            {
                                cs.Append("	" + pc_methodPrefix + name + "_" + action.Name + "((void*)ReaderObject");
                            }
                        }
                        else
                        {
                            cs.Append("	" + pc_methodLibPrefix + name + "_" + action.Name + "((void*)ReaderObject");
                        }
                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in")
                            {
                                cs.Append(",_" + args.Name);
                                if (args.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append(",_" + args.Name + "Length");
                                }
                            }
                        }

                        cs.Append(");" + cl);

                        foreach (UPnPArgument args in action.ArgumentList)
                        {
                            if (args.Direction == "in")
                            {
                                if (args.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append("free(_" + args.Name + ");" + cl);
                                }
                            }
                        }

                        cs.Append("}" + cl);
                        cs.Append(cl);
                    }
                }
            }
        }
        private string BuildHTTPSink_CONTROL(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames, string f1)
        {
            UPnPDebugObject obj;
            string first1 = f1;

            foreach (UPnPService service in device.Services)
            {
                obj = new UPnPDebugObject(service);
                string CONTROLURL = (string)obj.GetField("__controlurl");

                cs.Append("	" + first1 + " if (header->DirectiveObjLength==" + (CONTROLURL.Length + 1).ToString() + " && memcmp((header->DirectiveObj)+1,\"" + CONTROLURL + "\"," + CONTROLURL.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);
                string first2 = "";
                foreach (UPnPAction action in service.Actions)
                {
                    cs.Append("		" + first2 + " if (SOAPACTIONLength==" + action.Name.Length.ToString() + " && memcmp(SOAPACTION,\"" + action.Name + "\"," + action.Name.Length.ToString() + ")==0)" + cl);
                    cs.Append("		{" + cl);

                    cs.Append("			" + pc_methodPrefix + "Dispatch_" + (string)serviceNames[service] + "_" + action.Name + "(bodyBuffer, offset, bodyBufferLength, session);" + cl);

                    cs.Append("		}" + cl);
                    first2 = "else";
                }
                if (service.Actions.Count > 0)
                {
                    cs.Append("		else" + cl);
                    cs.Append("		{" + cl);
                }
                cs.Append("			RetVal=1;" + cl);
                if (service.Actions.Count > 0)
                {
                    cs.Append("		}" + cl);
                }

                cs.Append("	}" + cl);
                first1 = "else";

            }
            //			if (device.Services.Length>0)
            //			{
            //				cs.Append("	else"+cl);
            //				cs.Append("	{"+cl);
            //				cs.Append("		RetVal=1;"+cl);
            //				cs.Append("	}"+cl);
            //			}
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                first1 = this.BuildHTTPSink_CONTROL(cs, d, serviceNames, first1);
            }
            return (first1);
        }
        private void BuildHTTPSink_SCPD_HEAD(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            UPnPDebugObject obj;

            foreach (UPnPService service in device.Services)
            {
                obj = new UPnPDebugObject(service);
                string SCPDURL = (string)obj.GetField("SCPDURL");
                cs.Append("	else if (header->DirectiveObjLength==" + (SCPDURL.Length + 1).ToString() + " && memcmp((header->DirectiveObj)+1,\"" + SCPDURL + "\"," + SCPDURL.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);

                cs.Append("		" + this.pc_methodLibPrefix + "WebServer_StreamHeader_Raw(session,200,\"OK\",responseHeader,1);" + cl);
                cs.Append("		" + this.pc_methodLibPrefix + "WebServer_StreamBody(session,NULL,0," + this.pc_methodLibPrefix + "AsyncSocket_MemoryOwnership_STATIC,1);" + cl);

                cs.Append("	}" + cl);
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildHTTPSink_SCPD_HEAD(cs, d, serviceNames);
            }
        }
        private void BuildHTTPSink_SCPD(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            UPnPDebugObject obj;

            foreach (UPnPService service in device.Services)
            {
                obj = new UPnPDebugObject(service);
                string SCPDURL = (string)obj.GetField("SCPDURL");
                cs.Append("	else if (header->DirectiveObjLength==" + (SCPDURL.Length + 1).ToString() + " && memcmp((header->DirectiveObj)+1,\"" + SCPDURL + "\"," + SCPDURL.Length.ToString() + ")==0)" + cl);
                cs.Append("	{" + cl);
                if (this.Configuration.DynamicObjectModel)
                {
                    cs.Append("		ILibWebServer_StreamHeader_Raw(session,200,\"OK\",responseHeader,1);" + cl);
                    cs.Append("		UPnPStreamDescriptionDocument_SCPD(session,1,NULL,0,0,0,0);" + cl);

                    if (service.Actions.Count > 0)
                    {
                        cs.Append("		buffer = ILibDecompressString((unsigned char*)UPnP_ActionTable_" + serviceNames[service] + "_Impl.Reserved,UPnP_ActionTable_" + serviceNames[service] + "_Impl.ReservedXL,UPnP_ActionTable_" + serviceNames[service] + "_Impl.ReservedUXL);" + cl);
                        foreach (UPnPAction A in service.Actions)
                        {
                            string serviceIdent = DeviceObjectGenerator.GetServiceIdentifier(service);

                            serviceIdent += ("->" + A.Name);
                            cs.Append("		if (" + serviceIdent + "!=NULL){UPnPStreamDescriptionDocument_SCPD(session,0,buffer," + serviceIdent + "->Reserved," + serviceIdent + "->Reserved2,0,0);}" + cl);
                        }
                        cs.Append("		free(buffer);" + cl);
                    }
                    cs.Append("		UPnPStreamDescriptionDocument_SCPD(session,0,NULL,0,0,1,0);" + cl);
                    if (service.GetStateVariables().Length > 0)
                    {
                        cs.Append("		buffer = ILibDecompressString((unsigned char*)UPnP_StateVariableTable_" + serviceNames[service] + "_Impl.Reserved,UPnP_StateVariableTable_" + serviceNames[service] + "_Impl.ReservedXL,UPnP_StateVariableTable_" + serviceNames[service] + "_Impl.ReservedUXL);" + cl);
                        foreach (UPnPStateVariable V in service.GetStateVariables())
                        {
                            string vIdent = DeviceObjectGenerator.GetStateVariableIdentifier(V);
                            cs.Append("		if (" + vIdent + "!=NULL)" + cl);
                            cs.Append("		{" + cl);
                            cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved1," + vIdent + "->Reserved1L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                            if (V.Minimum != null || V.Maximum != null)
                            {
                                cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved4," + vIdent + "->Reserved4L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("			if (" + vIdent + "->MinMaxStep[0]!=NULL)" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"<minimum>\",9,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session," + vIdent + "->MinMaxStep[0],(int)strlen(" + vIdent + "->MinMaxStep[0]),ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"</minimum>\",10,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("			}" + cl);
                                cs.Append("			if (" + vIdent + "->MinMaxStep[1]!=NULL)" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"<maximum>\",9,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session," + vIdent + "->MinMaxStep[1],(int)strlen(" + vIdent + "->MinMaxStep[1]),ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"</maximum>\",10,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("			}" + cl);
                                cs.Append("			if (" + vIdent + "->MinMaxStep[2]!=NULL)" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"<step>\",6,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session," + vIdent + "->MinMaxStep[2],(int)strlen(" + vIdent + "->MinMaxStep[2]),ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,\"</step>\",7,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("			}" + cl);
                                cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved5," + vIdent + "->Reserved5L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                            }
                            if (V.DefaultValue != null)
                            {
                                cs.Append("			if (" + vIdent + "->DefaultValue!=NULL)" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved6," + vIdent + "->Reserved6L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session," + vIdent + "->DefaultValue,(int)strlen(" + vIdent + "->DefaultValue),ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("				ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved7," + vIdent + "->Reserved7L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("			}" + cl);
                            }
                            if (V.AllowedStringValues != null)
                            {
                                cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved2," + vIdent + "->Reserved2L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("			for(i=0;i<UPnP_StateVariable_AllowedValues_MAX;++i)" + cl);
                                cs.Append("			{" + cl);
                                cs.Append("				if (" + vIdent + "->AllowedValues[i]!=NULL)" + cl);
                                cs.Append("				{" + cl);
                                cs.Append("					ILibWebServer_StreamBody(session,\"<allowedValue>\",14,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("					ILibWebServer_StreamBody(session," + vIdent + "->AllowedValues[i],(int)strlen(" + vIdent + "->AllowedValues[i]),ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                                cs.Append("					ILibWebServer_StreamBody(session,\"</allowedValue>\",15,ILibAsyncSocket_MemoryOwnership_STATIC,0);" + cl);
                                cs.Append("				}" + cl);
                                cs.Append("			}" + cl);
                                cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved3," + vIdent + "->Reserved3L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                            }
                            cs.Append("			ILibWebServer_StreamBody(session,buffer+" + vIdent + "->Reserved8," + vIdent + "->Reserved8L,ILibAsyncSocket_MemoryOwnership_USER,0);" + cl);
                            cs.Append("		}" + cl);
                        }
                        cs.Append("		free(buffer);" + cl);
                    }
                    cs.Append("		UPnPStreamDescriptionDocument_SCPD(session,0,NULL,0,0,0,1);" + cl);
                }
                else
                {
                    cs.Append("		buffer = " + this.pc_methodLibPrefix + "DecompressString((unsigned char*)" + pc_methodPrefix + serviceNames[service] + "Description," + pc_methodPrefix + serviceNames[service] + "DescriptionLength," + pc_methodPrefix + serviceNames[service] + "DescriptionLengthUX);" + cl);
                    cs.Append("		" + this.pc_methodLibPrefix + "WebServer_Send_Raw(session,buffer," + pc_methodPrefix + serviceNames[service] + "DescriptionLengthUX,0,1);" + cl);
                }
                cs.Append("	}" + cl);
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildHTTPSink_SCPD(cs, d, serviceNames);
            }
        }


        private string Build_SubscribeEvents_Device(string first, CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                bool HasEvent = false;
                foreach (UPnPStateVariable sv in service.GetStateVariables())
                {
                    if (sv.SendEvent)
                    {
                        HasEvent = true;
                        break;
                    }
                }

                if (HasEvent)
                {
                    UPnPDebugObject obj = new UPnPDebugObject(service);
                    string name = (string)obj.GetField("__eventurl");
                    cs.Append(first + " if (pathlength==" + (name.Length + 1).ToString() + " && memcmp(path+1,\"" + name + "\"," + name.Length.ToString() + ")==0)" + cl);
                    cs.Append("	{" + cl);

                    cs.Append("		" + pc_methodPrefix + "TryToSubscribe(\"" + (string)serviceNames[service] + "\",TimeoutVal,URL,URLLength,session);" + cl);

                    cs.Append("	}" + cl);
                    first = "else";
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                first = Build_SubscribeEvents_Device(first, cs, d, serviceNames);
            }
            return (first);
        }


        private void BuildSSDPALL_Response(UPnPDevice device, CodeProcessor cs, int number)
        {
            if (number == 0)
            {
                cs.Append("							rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0, \"::upnp:rootdevice\", \"upnp:rootdevice\", \"\");" + cl);
            }

            // Device UUID Response
            if (number == 0)
            {
                cs.Append("							rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"\", upnp->UUID, \"\");" + cl);
            }
            else
            {
                cs.Append("							rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"\", ST, \"\");" + cl);
            }

            // Device URN
            cs.Append("						rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"::" + device.DeviceURN + "\", \"" + device.DeviceURN + "\", \"\");" + cl);

            foreach (UPnPService service in device.Services)
            {
                // Service URN
                cs.Append("						rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"::" + service.ServiceURN + "\", \"" + service.ServiceURN + "\", \"\");" + cl);
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                BuildSSDPALL_Response(d, cs, ++number);
            }
        }

        private void BuildMSEARCHHandler_device(UPnPDevice device, CodeProcessor cs, int number)
        {
            if (number == 0)
            {
                cs.Append("				else if (STLength == (int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)" + cl);
                cs.Append("				{" + cl);
                cs.Append("						rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), 0,\"\",upnp->UUID,\"\");" + cl);
                cs.Append("				}" + cl);
            }
            else if (number > 0)
            {
                cs.Append("				else if (STLength == (int)strlen(upnp->UUID) + " + (number.ToString().Length + 1).ToString() + ")" + cl);
                cs.Append("				{" + cl);
                cs.Append("					if (memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)" + cl);
                cs.Append("					{" + cl);
                cs.Append("						if (memcmp(ST+(int)strlen(upnp->UUID),\"_" + number.ToString() + "\"," + (number.ToString().Length + 1).ToString() + ")==0)" + cl);
                cs.Append("						{" + cl);
                cs.Append("								rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"\", ST, \"\");" + cl);
                cs.Append("						}" + cl);
                cs.Append("					}" + cl);
                cs.Append("				}" + cl);
            }
            cs.Append("				else if (STLength >= " + device.DeviceURN_Prefix.Length.ToString() + " && memcmp(ST,\"" + device.DeviceURN_Prefix + "\"," + device.DeviceURN_Prefix.Length.ToString() + ")==0 && atoi(ST+" + device.DeviceURN_Prefix.Length.ToString() + ")<=" + device.Version.ToString() + ")" + cl);
            cs.Append("				{" + cl);
            cs.Append("						rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"::" + device.DeviceURN_Prefix + "1\", ST, \"\");" + cl);
            if (device.Major > 1)
            {
                DText p = new DText();
                p.ATTRMARK = ":";
                p[0] = device.DeviceURN;
                cs.Append("						// packetlength = " + pc_methodPrefix + "FixVersion(b, \"" + p[p.DCOUNT() - 1] + ":1\", atoi(ST + " + device.DeviceURN_Prefix.Length.ToString() + "));" + cl);
            }
            cs.Append("				}" + cl);

            foreach (UPnPService service in device.Services)
            {
                cs.Append("				else if (STLength >= " + service.ServiceURN_Prefix.Length.ToString() + " && memcmp(ST,\"" + service.ServiceURN_Prefix + "\"," + service.ServiceURN_Prefix.Length.ToString() + ")==0 && atoi(ST+" + service.ServiceURN_Prefix.Length.ToString() + ")<=" + service.Version.ToString() + ")" + cl);
                cs.Append("				{" + cl);
                cs.Append("						rcode = " + pc_methodPrefix + "BuildSendSsdpResponsePacket(response_socket, upnp, (struct sockaddr*)&(mss->localIPAddress), (struct sockaddr*)&(mss->dest_addr), " + number.ToString() + ", \"::" + service.ServiceURN_Prefix + "1\", ST, \"\");" + cl);
                if (service.Major > 1)
                {
                    DText p = new DText();
                    p.ATTRMARK = ":";
                    p[0] = service.ServiceURN;
                    cs.Append("						// packetlength = " + pc_methodPrefix + "FixVersion(b, \"" + p[p.DCOUNT() - 1] + ":1\", atoi(ST + " + service.ServiceURN_Prefix.Length.ToString() + "));" + cl);
                }
                cs.Append("				}" + cl);
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                BuildMSEARCHHandler_device(d, cs, ++number);
            }
        }

        private int CountPackets(UPnPDevice d)
        {
            int RetVal = d.Root == true ? 3 : 2;

            RetVal += d.Services.Length;
            foreach (UPnPDevice e in d.EmbeddedDevices)
            {
                RetVal += CountPackets(e);
            }
            return (RetVal);
        }


        private void BuildDeviceDescription(CodeProcessor cs, UPnPDevice device)
        {
            UTF8Encoding U = new UTF8Encoding();
            string deviceDescription = (new UTF8Encoding().GetString(device.GetRootDeviceXML(new IPEndPoint(new IPAddress(0x0100007F), 80))));

            for (int i = 0; i < 40; i++) deviceDescription = deviceDescription.Replace("\r\n ", "\r\n");
            deviceDescription = deviceDescription.Replace("\r\n", "");
            //deviceDescription = deviceDescription;

            //			if (this.BasicHTTP)
            //			{
            //				deviceDescription = "HTTP/1.0 200  OK\r\nCONTENT-TYPE:  text/xml\r\nServer: " + UseSystem + ", UPnP/1.0, MicroStack/" + UseVersion + "\r\n\r\n" + deviceDescription;
            //			}

            byte[] deviceDescriptionX = OpenSource.Utilities.StringCompressor.CompressString(deviceDescription);

            cs.Append("const int " + this.pc_methodPrefix + "DeviceDescriptionTemplateLengthUX = " + U.GetByteCount(deviceDescription).ToString() + ";" + cl);
            cs.Append("const int " + this.pc_methodPrefix + "DeviceDescriptionTemplateLength = " + deviceDescriptionX.Length.ToString() + ";" + cl);
            cs.Append("const char " + this.pc_methodPrefix + "DeviceDescriptionTemplate[" + deviceDescriptionX.Length.ToString() + "]={" + cl);
            bool _first = true;
            int _ctr = 0;
            foreach (byte b in deviceDescriptionX)
            {
                if (_first == false)
                {
                    cs.Append(",");
                }
                else
                {
                    _first = false;
                }
                string hx = b.ToString("X");
                cs.Append("0x");
                if (hx.Length == 1) { cs.Append("0"); }
                cs.Append(hx);

                ++_ctr;
                if (_ctr % 20 == 0)
                {
                    cs.Append("\r\n");
                }
            }
            cs.Append("};\r\n");
        }
        private void BuildServiceDescriptions(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            UTF8Encoding U = new UTF8Encoding();
            Log("  Service description blocks.");
            string http_200_header;

            UPnPDevice root = device;
            while (root.ParentDevice != null)
            {
                root = root.ParentDevice;
            }

            if (!Configuration.HTTP_1dot1)
            {
                http_200_header = "HTTP/1.0 200  OK\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: " + UseSystem + ", UPnP/" + root.ArchitectureVersion + ", MicroStack/" + UseVersion + "\r\n";
            }
            else
            {
                http_200_header = "HTTP/1.1 200  OK\r\nCONTENT-TYPE:  text/xml; charset=\"utf-8\"\r\nServer: " + UseSystem + ", UPnP/" + root.ArchitectureVersion + ", MicroStack/" + UseVersion + "\r\n";
            }
            foreach (UPnPService service in device.Services)
            {
                string servicexml = new UTF8Encoding().GetString(service.GetSCPDXml());
                for (int i = 0; i < 40; i++) servicexml = servicexml.Replace("\r\n ", "\r\n");
                servicexml = servicexml.Replace("\r\n", "");
                string servicehttpresponse = http_200_header + "Content-Length: " + U.GetByteCount(servicexml).ToString() + "\r\n\r\n" + servicexml;

                cs.Comment(serviceNames[service].ToString());

                byte[] _sr = OpenSource.Utilities.StringCompressor.CompressString(servicehttpresponse);
                cs.Append("const int " + pc_methodPrefix + serviceNames[service] + "DescriptionLengthUX = " + U.GetByteCount(servicehttpresponse).ToString() + ";" + cl);
                cs.Append("const int " + pc_methodPrefix + serviceNames[service] + "DescriptionLength = " + (_sr.Length).ToString() + ";" + cl);
                cs.Append("const char " + pc_methodPrefix + serviceNames[service] + "Description[" + (_sr.Length).ToString() + "] = {" + cl);
                bool _first = true;
                int _ctr = 0;
                foreach (byte b in _sr)
                {
                    if (_first == false)
                    {
                        cs.Append(",");
                    }
                    else
                    {
                        _first = false;
                    }
                    string hx = b.ToString("X");
                    cs.Append("0x");
                    if (hx.Length == 1) { cs.Append("0"); }
                    cs.Append(hx);

                    ++_ctr;
                    if (_ctr % 20 == 0)
                    {
                        cs.Append("\r\n");
                    }
                }
                cs.Append("};" + cl);
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                BuildServiceDescriptions(cs, d, serviceNames);
            }
        }


        private void BuildNotifyPackets_Device(CodeProcessor cs, UPnPDevice device, int number, bool ipv6)
        {
            if (!ipv6)
            {
                cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), " + number.ToString() + ", \"\", \"uuid:\", upnp->UDN);" + cl);
                cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), " + number.ToString() + ", \"::" + device.DeviceURN + "\", \"" + device.DeviceURN + "\", \"\");" + cl);
            }
            else
            {
                cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), " + number.ToString() + ", \"\", \"uuid:\", upnp->UDN);" + cl);
                cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), " + number.ToString() + ", \"::" + device.DeviceURN + "\", \"" + device.DeviceURN + "\", \"\");" + cl);
            }

            foreach (UPnPService service in device.Services)
            {
                if (!ipv6)
                {
                    cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->AddressListV4[i]), " + number.ToString() + ", \"::" + service.ServiceURN + "\", \"" + service.ServiceURN + "\", \"\");" + cl);
                }
                else
                {
                    cs.Append("			" + pc_methodPrefix + "BuildSendSsdpNotifyPacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->AddressListV6[i]), " + number.ToString() + ", \"::" + service.ServiceURN + "\", \"" + service.ServiceURN + "\", \"\");" + cl);
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                BuildNotifyPackets_Device(cs, d, ++number, ipv6);
            }
        }

        private void BuildByeByePackets_Device(CodeProcessor cs, UPnPDevice device, int dn, bool ipv6)
        {
            if (!ipv6)
            {
                cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, \"\", \"uuid:\", upnp->UDN, " + dn.ToString() + ");" + cl);
                cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, \"::" + device.DeviceURN_Prefix + "1\", \"" + device.DeviceURN + "\", \"\", " + dn.ToString() + ");" + cl);
            }
            else
            {
                //cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV6), UPNP_MCASTv6_GROUPB, \"\", \"uuid:\", upnp->UDN, " + dn.ToString() + ");" + cl);
                //cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV6), UPNP_MCASTv6_GROUPB, \"::" + device.DeviceURN_Prefix + "1\", \"" + device.DeviceURN + "\", \"\", " + dn.ToString() + ");" + cl);
                cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, \"\", \"uuid:\", upnp->UDN, " + dn.ToString() + ");" + cl);
                cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, \"::" + device.DeviceURN_Prefix + "1\", \"" + device.DeviceURN + "\", \"\", " + dn.ToString() + ");" + cl);
            }

            foreach (UPnPService service in device.Services)
            {
                if (!ipv6)
                {
                    cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks[i], upnp, (struct sockaddr*)&(upnp->MulticastAddrV4), UPNP_MCASTv4_GROUP, \"::" + service.ServiceURN + "\", \"" + service.ServiceURN_Prefix + "1\", \"\", " + dn.ToString() + ");" + cl);
                }
                else
                {
                    cs.Append("      " + pc_methodPrefix + "BuildSendSsdpByeByePacket(upnp->NOTIFY_SEND_socks6[i], upnp, t1, t2, \"::" + service.ServiceURN + "\", \"" + service.ServiceURN_Prefix + "1\", \"\", " + dn.ToString() + ");" + cl);
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                BuildByeByePackets_Device(cs, d, ++dn, ipv6);
            }
        }


        private void BuildUPnPResponse(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                ServiceGenerator.ServiceConfiguration SConf = (ServiceGenerator.ServiceConfiguration)service.User;
                foreach (UPnPAction action in service.Actions)
                {
                    StringBuilder SB = new StringBuilder();

                    SB.Append(pc_methodPrefixDef + "Response_" + serviceNames[service] + "_" + action.Name + "(const UPnPSessionToken UPnPToken");
                    int argcount = 0;

                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            if (arg.RelatedStateVar.ComplexType == null)
                            {
                                // Primitive
                                SB.Append(", const " + ToCType(arg.RelatedStateVar.GetNetType().ToString()) + " ");
                                if (arg.RelatedStateVar.GetNetType() == typeof(string) && !SConf.Actions_ManualEscape.Contains(action))
                                {
                                    SB.Append("unescaped_" + arg.Name);
                                }
                                else
                                {
                                    SB.Append(arg.Name);
                                }
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    SB.Append(", const int _" + arg.Name + "Length");
                                }
                            }
                            else
                            {
                                // Complex
                                SB.Append(", struct " + arg.RelatedStateVar.ComplexType.Name_LOCAL + " *_" + arg.Name);
                            }
                            argcount++;
                        }
                    }

                    SB.Append(")");
                    cs.Append("/*! \\fn " + SB.ToString() + cl);
                    cs.Append("	\\brief Response Method for " + serviceNames[service] + " >> " + service.ServiceURN + " >> " + action.Name + cl);
                    cs.Append("	\\param UPnPToken MicroStack token" + cl);
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            cs.Append(" \\param ");
                            if (arg.RelatedStateVar.ComplexType == null)
                            {
                                // Primitive
                                if (arg.RelatedStateVar.GetNetType() == typeof(string) && !SConf.Actions_ManualEscape.Contains(action))
                                {
                                    cs.Append("unescaped_" + arg.Name + " Value of argument " + arg.Name + " \\b     Note: Automatically Escaped" + cl);
                                }
                                else
                                {
                                    cs.Append(arg.Name + " Value of argument " + arg.Name);
                                    if (arg.RelatedStateVar.GetNetType() == typeof(string))
                                    {
                                        cs.Append(" \\b     Note: Must be escaped" + cl);
                                    }
                                    else
                                    {
                                        cs.Append(cl);
                                    }
                                }
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append("	\\param " + arg.Name + "Length Length of \\a " + arg.Name + cl);
                                }
                            }
                            else
                            {
                                // Complex
                                cs.Append(" _" + arg.Name + " Value of argument " + arg.Name + cl);
                            }
                        }
                    }
                    cs.Append("*/" + cl);
                    cs.DefinePublic("void " + SB.ToString());

                    cs.Append("{" + cl);

                    if (argcount == 0)
                    {
                        cs.Append(pc_methodPrefixDef + "ResponseGeneric(UPnPToken,\"" + service.ServiceURN + "\",\"" + action.Name + "\",\"\");" + cl);
                        cs.Append("}" + cl);
                        cs.Append(cl);
                        continue;
                    }
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.RelatedStateVar.ComplexType != null)
                        {
                            cs.Append(" char *tempString;" + cl);
                            break;
                        }
                    }
                    cs.Append("  char* body;" + cl);
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            if (arg.RelatedStateVar.ComplexType == null)
                            {
                                // Simple
                                if (arg.RelatedStateVar.GetNetType() == typeof(string) && !SConf.Actions_ManualEscape.Contains(action))
                                {
                                    cs.Append("	char *" + arg.Name + " = (char*)malloc(1+" + this.pc_methodLibPrefix + "XmlEscapeLength(unescaped_" + arg.Name + "));" + cl);
                                }
                            }
                            else
                            {
                                //Complex
                                cs.Append("	char *" + arg.Name + ";" + cl);
                            }
                        }
                    }
                    cs.Append(cl);
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.RelatedStateVar.ComplexType == null)
                        {
                            // If this is a simple string, we need to escape it
                            if (arg.Direction == "out" && arg.RelatedStateVar.GetNetType() == typeof(string) && !SConf.Actions_ManualEscape.Contains(action))
                            {
                                cs.Append("	" + this.pc_methodLibPrefix + "XmlEscape(" + arg.Name + ", unescaped_" + arg.Name + ");" + cl);
                            }
                        }
                    }
                    bool needSpecial = false;
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.RelatedStateVar.ComplexType != null)
                        {
                            needSpecial = true;
                            break;
                        }
                    }
                    if (needSpecial)
                    {
                        cs.Comment("For Complex Types, we need to:");
                        cs.Comment("1.) Serialize the XML structure");
                        cs.Comment("2.) Escape the serialization for legacy CPs");
                        cs.Append(cl);
                        foreach (UPnPArgument a in action.Arguments)
                        {
                            if (a.RelatedStateVar.ComplexType != null)
                            {
                                cs.Append("	" + a.Name + "= " + this.pc_methodPrefix + "Serialize_" + a.RelatedStateVar.ComplexType.Name_LOCAL + "(_" + a.Name + ");" + cl);
                            }
                        }
                        cs.Append("	if (((struct ILibWebServer_Session*)UPnPToken)->Reserved9 == 0)" + cl);
                        cs.Append("	{" + cl);
                        cs.Comment("Serialization for Legacy CP");
                        foreach (UPnPArgument a in action.Arguments)
                        {
                            if (a.RelatedStateVar.ComplexType != null)
                            {
                                cs.Append(cl);
                                cs.Append("	if ((tempString = (char*)malloc(1+" + this.pc_methodLibPrefix + "XmlEscapeLength(" + a.Name + "))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("	tempString[" + this.pc_methodLibPrefix + "XmlEscape(tempString, " + a.Name + ")] = 0;" + cl);
                                cs.Append("	free(" + a.Name + ");" + cl);
                                cs.Append("	" + a.Name + " = tempString;" + cl);
                            }
                        }
                        cs.Append("	}" + cl);
                    }


                    string soap_invokeResponse = "";
                    int soap_size = 1;
                    string soap_size_str = "";
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            soap_invokeResponse += "<" + arg.Name + ">";
                            soap_invokeResponse += this.ToSPrintfType(arg.RelatedStateVar.GetNetType().ToString());
                            soap_invokeResponse += "</" + arg.Name + ">";

                            soap_size += (5 + (2 * arg.Name.Length));

                            switch (arg.RelatedStateVar.GetNetType().ToString())
                            {
                                case "System.Boolean":
                                    soap_size += 1;
                                    break;
                                case "System.Byte[]":
                                    soap_size_str += "+strlen(" + arg.Name + "_Base64)";
                                    break;
                                case "System.String":
                                case "System.Uri":
                                    soap_size_str += "+strlen(" + arg.Name + ")";
                                    break;
                                case "System.DateTime":
                                    soap_size += 20;
                                    break;
                                case "System.Byte":
                                case "System.SByte":
                                case "System.Char":
                                    soap_size += 4;
                                    break;
                                case "System.UInt16":
                                case "System.Int16":
                                    soap_size += 6;
                                    break;
                                case "System.UInt32":
                                case "System.Int32":
                                    soap_size += 11;
                                    break;
                                case "System.Single":
                                case "System.Double":
                                    soap_size += 16;
                                    break;
                            }
                        }
                    }

                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out" && arg.RelatedStateVar.GetNetType() == typeof(byte[]))
                        {
                            cs.Append("  unsigned char* " + arg.Name + "_Base64;" + cl);
                        }
                        if (arg.Direction == "out" && arg.RelatedStateVar.GetNetType() == typeof(DateTime))
                        {
                            cs.Append("  char* " + arg.Name + "_DateTime;" + cl);
                        }
                    }

                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out" && arg.RelatedStateVar.GetNetType() == typeof(byte[]))
                        {
                            cs.Append("  " + pc_methodLibPrefix + "Base64Encode((unsigned char*)" + arg.Name + ", _" + arg.Name + "Length, &" + arg.Name + "_Base64);" + cl);
                        }
                        if (arg.Direction == "out" && arg.RelatedStateVar.GetNetType() == typeof(DateTime))
                        {
                            cs.Append("  " + arg.Name + "_DateTime = " + pc_methodLibPrefix + "Time_Serialize(" + arg.Name + ");" + cl);
                        }
                    }

                    cs.Append("  if ((body = (char*)malloc(" + soap_size.ToString() + soap_size_str + ")) == NULL) ILIBCRITICALEXIT(254);" + cl);
                    cs.Append("  snprintf(body, " + soap_size.ToString() + soap_size_str + ", \"" + PrintfTransform(soap_invokeResponse) + "\"");

                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            if (arg.RelatedStateVar.GetNetType() != typeof(bool))
                            {
                                cs.Append(", " + arg.Name);
                                if (arg.RelatedStateVar.GetNetType() == typeof(byte[]))
                                {
                                    cs.Append("_Base64");
                                }
                                if (arg.RelatedStateVar.GetNetType() == typeof(System.DateTime))
                                {
                                    cs.Append("_DateTime");
                                }
                            }
                            else
                            {
                                cs.Append(", (" + arg.Name + "!=0?1:0)");
                            }
                        }
                    }
                    cs.Append(");" + cl);

                    cs.Append("  " + this.pc_methodPrefix + "ResponseGeneric(UPnPToken, \"" + service.ServiceURN + "\", \"" + action.Name + "\", body);" + cl);
                    cs.Append("  free(body);" + cl);

                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            if (arg.RelatedStateVar.GetNetType() == typeof(byte[]))
                            {
                                cs.Append("  free(" + arg.Name + "_Base64);" + cl);
                            }
                            if (arg.RelatedStateVar.GetNetType() == typeof(DateTime))
                            {
                                cs.Append("  free(" + arg.Name + "_DateTime);" + cl);
                            }
                            if (arg.RelatedStateVar.GetNetType() == typeof(string) && !SConf.Actions_ManualEscape.Contains(action))
                            {
                                cs.Append("	free(" + arg.Name + ");" + cl);
                            }
                        }
                    }

                    cs.Append("}" + cl);
                    cs.Append(cl);
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildUPnPResponse(cs, d, serviceNames);
            }
        }

        private int GetAbsoluteTotalNumberOfEventedStateVariables(UPnPDevice device)
        {
            int RetVal = 0;

            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.SendEvent == true) ++RetVal;
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                RetVal += GetAbsoluteTotalNumberOfEventedStateVariables(d);
            }
            return (RetVal);
        }
        string BuildMulticastSoapEventsProcessor(string WS, UPnPDevice device, Hashtable serviceNames)
        {
            string RetVal = WS;
            string WS2;
            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.MulticastEvent)
                    {
                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "//{{{BEGIN_CHECK_MULTICASTVARIABLE}}}", "//{{{END_CHECK_MULTICASTVARIABLE}}}");
                        WS2 = WS2.Replace("{{{VARNAME}}}", statevar.Name);
                        WS2 = WS2.Replace("{{{SERVICENAME}}}", (string)serviceNames[service]);
                        WS2 = WS2.Replace("{{{SERVICETYPE}}}", service.ServiceURN_Prefix.Substring(0, service.ServiceURN_Prefix.Length - 1));
                        WS2 = WS2.Replace("{{{SERVICETYPELENGTH}}}", (service.ServiceURN_Prefix.Length - 1).ToString());
                        WS2 = WS2.Replace("{{{VARDISPATCH}}}", EmbeddedCGenerator.ToCTypeFromStateVar_Dispatch(statevar));
                        WS2 = WS2.Replace("{{{VARSERIALIZE}}}", EmbeddedCGenerator.ToCTypeFromStateVar_Serialize("VariableValue", "VariableValueLength", "OK", this.pc_methodLibPrefix, statevar));
                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "//{{{BEGIN_CHECK_MULTICASTVARIABLE}}}", WS2);
                        WS = WS.Replace("{{{VARDEFS}}}", "{{{VARDEFS}}}" + cl + EmbeddedCGenerator.ToCTypeFromStateVar(statevar) + ";");
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                WS = this.BuildMulticastSoapEventsProcessor(WS, d, serviceNames);
            }
            return (WS);
        }
        private void BuildMulticastSoapEvents(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.MulticastEvent)
                    {
                        #region Initialize
                        string eventname = "UPnPObject->" + serviceNames[service] + "_" + statevar.Name;

                        string binaryextra = "";
                        if (statevar.GetNetType().ToString() == "System.Byte[]") binaryextra = ",int vallen";
                        #endregion
                        cs.DefinePublic("void " + pc_methodPrefixDef + "SetMulticastState_" + serviceNames[service] + "_" + statevar.Name + "(UPnPMicroStackToken upnptoken, enum MULTICAST_EVENT_TYPE eventType, " + ToCType(statevar.GetNetType().ToString()) + " val" + binaryextra + ")");
                        cs.Append("{" + cl);
                        cs.Append("	struct " + pc_methodPrefix + "DataObject *UPnPObject = (struct " + pc_methodPrefix + "DataObject*)upnptoken;" + cl);
                        cs.Append("	char *b;" + cl);
                        cs.Append("	int bLength;" + cl);
                        cs.Append("	void *response_socket;" + cl);
                        cs.Append("	void *subChain;" + cl);
                        cs.Append("	int *addrList;" + cl);
                        cs.Append("	int addrListLength;" + cl);
                        cs.Append("	int i;" + cl);
                        cs.Append("	char newVal[32];" + cl); //ToDo: Magic Value
                        cs.Append("	if ((b = (char*)malloc(5000)) == NULL) ILIBCRITICALEXIT(254);" + cl); //ToDo: Magic Value
                        cs.Append(cl);
                        cs.Append("	subChain = ILibCreateChain();" + cl);
                        cs.Append("	response_socket = ILibAsyncUDPSocket_Create(" + cl);
                        cs.Append("		subChain," + cl);
                        cs.Append("		UPNP_MAX_SSDP_HEADER_SIZE," + cl);
                        cs.Append("		0," + cl);
                        cs.Append("		0," + cl);
                        cs.Append("		ILibAsyncUDPSocket_Reuse_SHARED," + cl);
                        cs.Append("		NULL," + cl);
                        cs.Append("		NULL," + cl);
                        cs.Append("		subChain);" + cl);
                        cs.Append(cl);
                        cs.Append("	++" + pc_methodPrefix + "Object->" + serviceNames[service] + "_" + statevar.Name + "_SEQ;" + cl);
                        cs.Append(cl);
                        cs.Append("	snprintf(newVal, 32, \"%d\", val);" + cl);
                        cs.Append("	bLength = snprintf(b, 5000, UPnPMulticastPacketTemplate," + cl); //ToDo: Magic Value
                        cs.Append("		UPNP_GROUP," + cl);
                        cs.Append("		UPNP_MULTICASTEVENT_PORT," + cl);
                        cs.Append("		UPnPObject->UDN," + cl);
                        cs.Append("		\"" + service.ServiceURN + "\"," + cl);
                        cs.Append("		UPnPObject->" + serviceNames[service] + "_" + statevar.Name + "_SEQ," + cl);
                        cs.Append("		MULTICAST_EVENT_TYPE_DESCRIPTION[(int)eventType]," + cl);
                        cs.Append("		UPnPObject->InitialNotify," + cl);
                        cs.Append("		\"" + statevar.Name + "\"," + cl);
                        cs.Append("		newVal," + cl);
                        cs.Append("		\"" + statevar.Name + "\");" + cl);


                        cs.Append("	addrListLength = ILibGetLocalIPAddressList(&addrList);" + cl);
                        cs.Append("	ILibAsyncUDPSocket_JoinMulticastGroup(response_socket, 0, inet_addr(UPNP_GROUP));" + cl);
                        cs.Append("	for(i = 0; i < addrListLength; ++i)" + cl);
                        cs.Append("	{" + cl);
                        cs.Append("		ILibAsyncUDPSocket_SetMulticastInterface(response_socket, addrList[i]);" + cl);
                        cs.Append("		ILibAsyncUDPSocket_SendTo(response_socket, inet_addr(UPNP_GROUP), UPNP_MULTICASTEVENT_PORT, b, bLength, ILibAsyncSocket_MemoryOwnership_USER);" + cl);
                        cs.Append("	}" + cl);
                        cs.Append("	free(addrList);" + cl);


                        cs.Append("	ILibChain_DestroyEx(subChain);" + cl);
                        cs.Append("}" + cl);
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildMulticastSoapEvents(cs, d, serviceNames);
            }
        }
        private void BuildSoapEvents(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            //string soap_eventblock = "<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s>%s</%s></e:property></e:propertyset>";
            string soap_eventblock = "%s>%s</%s";
            foreach (UPnPService service in device.Services)
            {
                #region Calculate number of evented variables
                int eventedStateVariables = 0;
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.SendEvent == true) eventedStateVariables++;
                }
                #endregion

                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.SendEvent == true)
                    {
                        #region Initialize
                        string eventname = "UPnPObject->" + serviceNames[service] + "_" + statevar.Name;

                        string binaryextra = "";
                        if (statevar.GetNetType().ToString() == "System.Byte[]") binaryextra = ",int vallen";
                        #endregion
                        #region SetState
                        cs.Append("/*! \\fn " + pc_methodPrefixDef + "SetState_" + serviceNames[service] + "_" + statevar.Name + "(UPnPMicroStackToken upnptoken, " + ToCType(statevar.GetNetType().ToString()) + " val" + binaryextra + ")" + cl);
                        cs.Append("	\\brief Sets the state of " + statevar.Name + " << " + statevar.OwningService.ServiceURN + " << " + serviceNames[service] + " \\par" + cl);
                        cs.Append("	\\b Note: Must be called at least once prior to start" + cl);
                        cs.Append("	\\param upnptoken The MicroStack token" + cl);
                        cs.Append("	\\param val The new value of the state variable" + cl);
                        if (binaryextra != "")
                        {
                            cs.Append("	\\param vallen Length of \\a val" + cl);
                        }
                        cs.Append("*/" + cl);
                        cs.DefinePublic("void " + pc_methodPrefixDef + "SetState_" + serviceNames[service] + "_" + statevar.Name + "(UPnPMicroStackToken upnptoken, " + ToCType(statevar.GetNetType().ToString()) + " val" + binaryextra + ")");
                        cs.Append("{" + cl);
                        cs.Append("	 struct " + pc_methodPrefix + "DataObject *UPnPObject = (struct " + pc_methodPrefix + "DataObject*)upnptoken;" + cl);
                        cs.Append("  char* body;" + cl);
                        cs.Append("  int bodylength;" + cl);
                        if (statevar.GetNetType().ToString() == "System.Byte[]")
                        {
                            cs.Append("  unsigned char* valstr;" + cl);
                        }
                        else
                        {
                            cs.Append("  char* valstr;" + cl);
                        }

                        #region Data Handling
                        // Data Type Handling Code
                        switch (statevar.GetNetType().ToString())
                        {
                            case "System.Boolean":
                                cs.Append("  if (val != 0) valstr = \"true\"; else valstr = \"false\";" + cl);
                                break;
                            case "System.Byte[]":
                                cs.Append("  " + pc_methodLibPrefix + "Base64Encode(val, vallen, &valstr);" + cl);
                                break;
                            case "System.Uri":
                            case "System.String":
                                cs.Append("  if ((valstr = (char*)malloc(" + pc_methodLibPrefix + "XmlEscapeLength(val) + 1)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("  " + this.pc_methodLibPrefix + "XmlEscape(valstr, val);" + cl);
                                break;
                            case "System.Byte":
                            case "System.Int16":
                            case "System.Int32":
                                cs.Append("  if ((valstr = (char*)malloc(10)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("  snprintf(valstr, 10, \"%d\", val);" + cl);
                                break;
                            case "System.Char":
                            case "System.SByte":
                            case "System.UInt16":
                            case "System.UInt32":
                                cs.Append("  if ((valstr = (char*)malloc(10)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("  snprintf(valstr, 10, \"%u\", val);" + cl);
                                break;
                            case "System.Single":
                            case "System.Double":
                                cs.Append("  if ((valstr = (char*)malloc(30)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                cs.Append("  snprintf(valstr, 30, \"%f\", val);" + cl);
                                break;
                            case "System.DateTime":
                                cs.Append("  valstr = " + this.pc_methodLibPrefix + "Time_Serialize(val);" + cl);
                                break;
                            default:
                                cs.Append("  char* valuestr = NULL;" + cl);
                                break;
                        }
                        #endregion
                        #region Memory Handling
                        // Data Type Handling Code
                        switch (statevar.GetNetType().ToString())
                        {
                            case "System.Uri":
                            case "System.String":
                            case "System.Byte[]":
                            case "System.Byte":
                            case "System.Int16":
                            case "System.Int32":
                            case "System.Char":
                            case "System.SByte":
                            case "System.UInt16":
                            case "System.UInt32":
                            case "System.Single":
                            case "System.Double":
                                cs.Append("  if (" + eventname + " != NULL) free(" + eventname + ");" + cl);
                                break;
                        }
                        #endregion

                        cs.Append("  " + eventname + " = valstr;" + cl);

                        cs.Append("  bodylength = " + (soap_eventblock.Length + (statevar.Name.Length * 2) + 1) + " + (int)strlen(valstr);" + cl);
                        cs.Append("  if ((body = (char*)malloc(bodylength)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                        cs.Append("  bodylength = snprintf(body, bodylength, \"" + PrintfTransform(soap_eventblock) + "\", \"" + statevar.Name + "\", valstr, \"" + statevar.Name + "\");" + cl);
                        cs.Append("  " + pc_methodPrefix + "SendEvent(upnptoken, body, bodylength, \"" + (string)serviceNames[service] + "\");" + cl);
                        cs.Append("  free(body);" + cl);
                        cs.Append("}" + cl);
                        cs.Append(cl);
                        #endregion
                    }
                }

            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildSoapEvents(cs, d, serviceNames);
            }
        }

        private void BuildFunctionPointers(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            string staticdef = "";
            if (this.Language == LANGUAGES.CPP) staticdef = "static ";
            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();

            while (en.MoveNext())
            {
                UPnPService S = (UPnPService)en.Value;
                string name = (string)en.Key;

                if (name != "DeviceSecurity")
                {
                    foreach (UPnPAction A in S.Actions)
                    {
                        string d = "";
                        d += "/*! \\var " + pc_methodPrefixDef + "FP_" + name + "_" + A.Name + cl;
                        d += "	\\brief Dispatch Pointer for " + name + " >> " + S.ServiceURN + " >> " + A.Name + cl;
                        d += "*/" + cl;
                        d += "UPnP_ActionHandler_" + serviceNames[S] + "_" + A.Name + " " + pc_methodPrefixDef + "FP_" + name + "_" + A.Name + ";" + cl;
                        cs.Append(d);
                        cs.PublicClassDefinitions.Append(staticdef + d);
                    }
                }
            }
        }

        private void BuildFunctionPointerHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();

            while (en.MoveNext())
            {
                UPnPService S = (UPnPService)en.Value;
                string name = (string)en.Key;

                if (name != "DeviceSecurity")
                {
                    foreach (UPnPAction A in S.Actions)
                    {
                        cs.Append("extern UPnP_ActionHandler_" + serviceNames[S] + "_" + A.Name + " " + pc_methodPrefixDef + "FP_" + name + "_" + A.Name + ";" + cl);
                    }
                }
            }
        }
        private void BuildUPnPResponseHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                ServiceGenerator.ServiceConfiguration SConf = (ServiceGenerator.ServiceConfiguration)service.User;
                foreach (UPnPAction action in service.Actions)
                {
                    cs.Append("void " + pc_methodPrefixDef + "Response_" + serviceNames[service] + "_" + action.Name + "(const UPnPSessionToken UPnPToken");
                    /*
                    if (action.HasReturnValue)
                    {
                        cs.Append(", const " + ToCType(action.GetRetArg().RelatedStateVar.GetNetType().FullName) + " __ReturnValue");
                        if (action.GetRetArg().RelatedStateVar.GetNetType().FullName=="System.Byte[]")
                        {
                            cs.Append(", const int __ReturnValueLength");
                        }
                    }
                    */
                    foreach (UPnPArgument arg in action.Arguments)
                    {
                        if (arg.Direction == "out")
                        {
                            if (arg.RelatedStateVar.ComplexType == null)
                            {
                                // Simple Type
                                cs.Append(", const " + ToCType(arg.RelatedStateVar.GetNetType().ToString()) + " " + arg.Name);
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append(", const int _" + arg.Name + "Length");
                                }
                            }
                            else
                            {
                                // Complex Type
                                cs.Append(", struct " + arg.RelatedStateVar.ComplexType.Name_LOCAL + " *" + arg.Name);
                            }
                        }
                    }
                    cs.Append(");" + cl);
                }
                bool NeedManualComment = false;
                foreach (UPnPAction action in service.Actions)
                {
                    if (SConf.Actions_ManualEscape.Contains(action))
                    {
                        // Manual Escape
                        NeedManualComment = true;
                        break;
                    }
                }
                if (NeedManualComment)
                {
                    cs.Append(cl);
                    cs.Comment("The string parameters for the following response methods MUST be MANUALLY escaped");
                    foreach (UPnPAction action in service.Actions)
                    {
                        if (SConf.Actions_ManualEscape.Contains(action))
                        {
                            cs.Comment("void " + pc_methodPrefixDef + "Response_" + serviceNames[service] + "_" + action.Name);
                        }
                    }
                    cs.Append(cl);
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildUPnPResponseHeaders(cs, d, serviceNames);
            }
        }
        private string BuildMulticastStateVariableHeaders2(string WS, UPnPDevice device, Hashtable serviceNames)
        {
            string WS2;
            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.MulticastEvent)
                    {
                        WS2 = SourceCodeRepository.GetTextBetweenTags(WS, "//{{{BEGIN_MulticastEventing_Specific}}}", "//{{{END_MulticastEventing_Specific}}}");
                        WS2 = WS2.Replace("{{{SERVICENAME}}}", (string)serviceNames[service]);
                        WS2 = WS2.Replace("{{{VARNAME}}}", statevar.Name);
                        WS2 = WS2.Replace("{{{ARGLIST}}}", EmbeddedCGenerator.ToCTypeFromStateVar(statevar));
                        WS = SourceCodeRepository.InsertTextBeforeTag(WS, "//{{{BEGIN_MulticastEventing_Specific}}}", WS2);
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                WS = this.BuildMulticastStateVariableHeaders2(WS, d, serviceNames);
            }
            return (WS);
        }
        private void BuildMulticastStateVariableHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.MulticastEvent)
                    {
                        cs.Append("void " + pc_methodPrefixDef + "SetMulticastState_" + serviceNames[service] + "_" + statevar.Name + "(UPnPMicroStackToken upnptoken, enum MULTICAST_EVENT_TYPE eventType," + EmbeddedCGenerator.ToCTypeFromStateVar(statevar) + ");" + cl);
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildMulticastStateVariableHeaders(cs, d, serviceNames);
            }
        }
        private void BuildStateVariableHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            foreach (UPnPService service in device.Services)
            {
                foreach (UPnPStateVariable statevar in service.GetStateVariables())
                {
                    if (statevar.SendEvent == true)
                    {
                        string binaryextra = "";
                        if (statevar.GetNetType().ToString() == "System.Byte[]") binaryextra = ", int _" + statevar.Name + "Length";
                        cs.Append("void " + pc_methodPrefixDef + "SetState_" + serviceNames[service] + "_" + statevar.Name + "(UPnPMicroStackToken microstack," + ToCType(statevar.GetNetType().ToString()) + " val" + binaryextra + ");" + cl);
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildStateVariableHeaders(cs, d, serviceNames);
            }
        }

        private void BuildStateVariableEventingSample(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            string cppobj = "";
            if (this.Language == LANGUAGES.CPP) { cppobj = "microstack->"; }

            bool present = false;
            foreach (UPnPService service in device.Services)
            {
                if (serviceNames[service].ToString() != "DeviceSecurity")
                {
                    foreach (UPnPStateVariable statevar in service.GetStateVariables()) if (statevar.SendEvent == true) present = true;
                }
            }
            if (present) cs.Append("    // All evented state variables MUST be initialized before UPnPStart is called." + cl);

            foreach (UPnPService service in device.Services)
            {
                if (serviceNames[service].ToString() != "DeviceSecurity")
                {
                    foreach (UPnPStateVariable statevar in service.GetStateVariables())
                    {
                        if (statevar.SendEvent == true)
                        {
                            cs.Append("    " + cppobj + pc_methodPrefix + "SetState_" + serviceNames[service] + "_" + statevar.Name + "(" + this.pc_methodPrefix + "microStack, " + ToSampleValue(statevar.GetNetType().ToString()) + ");" + cl);
                        }
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildStateVariableEventingSample(cs, d, serviceNames);
            }
        }


        private void BuildMainUserCode(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            string cppobj = "";

            foreach (UPnPService service in device.Services)
            {
                ServiceGenerator.ServiceConfiguration SConf = (ServiceGenerator.ServiceConfiguration)service.User;
                if (serviceNames[service].ToString() != "DeviceSecurity")
                {
                    foreach (UPnPAction action in service.Actions)
                    {

                        #region Invoke

                        #region Header
                        cs.Append("void " + pc_methodPrefix + serviceNames[service] + "_" + action.Name + "(" + this.pc_methodPrefix + "SessionToken upnptoken");
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                if (arg.RelatedStateVar.ComplexType == null)
                                {
                                    cs.Append("," + ToCType(arg.RelatedStateVar.GetNetType().ToString()) + " " + arg.Name);
                                    if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                    {
                                        cs.Append(",int _" + arg.Name + "Length");
                                    }
                                }
                                else
                                {
                                    cs.Append(", struct " + arg.RelatedStateVar.ComplexType.Name_LOCAL + " *" + arg.Name);
                                }
                            }
                        }
                        cs.Append(")" + cl);
                        #endregion
                        #region Body
                        cs.Append("{" + cl);

                        #region printf

                        if (this.SubTarget == SUBTARGETS.PPC2003)
                        {
                            cs.Append("	CString display;" + cl);
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "in" && (arg.RelatedStateVar.GetNetType() == typeof(string) ||
                                    arg.RelatedStateVar.GetNetType() == typeof(System.Uri)))
                                {
                                    cs.Append("	wchar_t *wc_" + arg.Name + " = NULL;" + cl);
                                    cs.Append("	int wc_" + arg.Name + "Length = 0;" + cl);
                                }
                            }
                            cs.Append(cl);
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "in" && (arg.RelatedStateVar.GetNetType() == typeof(string) ||
                                    arg.RelatedStateVar.GetNetType() == typeof(System.Uri)))
                                {
                                    cs.Append("	if (" + arg.Name + " != NULL)" + cl);
                                    cs.Append("	{" + cl);
                                    cs.Append("		wc_" + arg.Name + "Length = MultiByteToWideChar(CP_UTF8, 0, " + arg.Name + ", -1, wc_" + arg.Name + ", 0);" + cl);
                                    cs.Append("		if ((wc_" + arg.Name + " = (wchar_t*)malloc(sizeof(wchar_t)*wc_" + arg.Name + "Length)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                                    cs.Append("		MultiByteToWideChar(CP_UTF8, 0 ," + arg.Name + ", -1, wc_" + arg.Name + ", wc_" + arg.Name + "Length);" + cl);
                                    cs.Append("	}" + cl);
                                }
                            }
                            cs.Append("  display.Format(_T(");
                        }
                        else
                        {
                            cs.Append("  printf(");
                        }

                        cs.Append("\"Invoke: " + pc_methodPrefix + serviceNames[service] + "_" + action.Name + "(");

                        bool firstArg = true;
                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                if (firstArg == false) cs.Append(",");
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append("BINARY(%d)");
                                }
                                else
                                {
                                    cs.Append(ToPrintfType(arg.RelatedStateVar.GetNetType().ToString()));
                                }
                                firstArg = false;
                            }
                        }

                        cs.Append(");\\r\\n\"");
                        if (this.SubTarget == SUBTARGETS.PPC2003)
                        {
                            cs.Append(")");
                        }

                        foreach (UPnPArgument arg in action.Arguments)
                        {
                            if (arg.Direction == "in")
                            {
                                if (arg.RelatedStateVar.GetNetType().FullName == "System.Byte[]")
                                {
                                    cs.Append(", _" + arg.Name + "Length");
                                }
                                else
                                {
                                    if ((arg.RelatedStateVar.GetNetType() == typeof(Uri) || arg.RelatedStateVar.GetNetType() == typeof(string)) && this.SubTarget == SUBTARGETS.PPC2003)
                                    {
                                        cs.Append(", wc_" + arg.Name);
                                    }
                                    else
                                    {
                                        cs.Append(", " + arg.Name);
                                    }
                                }
                            }
                        }
                        cs.Append(");" + cl);
                        if (this.SubTarget == SUBTARGETS.PPC2003)
                        {
                            cs.Append("	if (that->m_Text.GetLength() > 16384)" + cl);
                            cs.Append("	{" + cl);
                            cs.Append("		that->m_Text = display;" + cl);
                            cs.Append("	}" + cl);
                            cs.Append("	else" + cl);
                            cs.Append("	{" + cl);
                            cs.Append("		that->m_Text += display;" + cl);
                            cs.Append("	}" + cl);
                            cs.Append("	that->SendMessage(WM_USER_UPDATE);" + cl);
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "in" && (arg.RelatedStateVar.GetNetType() == typeof(string) ||
                                    arg.RelatedStateVar.GetNetType() == typeof(System.Uri)))
                                {
                                    cs.Append("	if (wc_" + arg.Name + " != NULL) {free(wc_" + arg.Name + ");}" + cl);
                                }
                            }
                        }
                        #endregion

                        cs.Append(cl);
                        cs.Comment("If you intend to make the response later, you MUST reference count upnptoken with calls to " + this.pc_methodLibPrefix + "WebServer_AddRef()");
                        cs.Comment("and " + this.pc_methodLibPrefix + "WebServer_Release()");
                        cs.Append(cl);
                        cs.Comment("TODO: Place Action Code Here...");
                        cs.Append(cl);

                        cs.Comment(cppobj + pc_methodPrefix + "Response_Error(upnptoken, 404, \"Method Not Implemented\");");

                        if (SConf.Actions_Fragmented.Contains(action) == false)
                        {
                            // Standard Response System Only
                            #region Standard Response
                            cs.Append("  " + cppobj + pc_methodPrefix + "Response_" + serviceNames[service] + "_" + action.Name + "(upnptoken");
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "out")
                                {
                                    cs.Append("," + ToSampleValue(arg.RelatedStateVar.GetNetType().ToString()));
                                }
                            }
                            cs.Append(");" + cl);
                            #endregion
                        }
                        else
                        {
                            // Fragmented Response System
                            #region Standard Response, Commented out
                            cs.Append("  /* " + cppobj + pc_methodPrefix + "Response_" + serviceNames[service] + "_" + action.Name + "(upnptoken");
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "out")
                                {
                                    cs.Append("," + ToSampleValue(arg.RelatedStateVar.GetNetType().ToString()));
                                }
                            }
                            cs.Append("); */" + cl);
                            #endregion
                            #region Fragmented Response
                            cs.Append(cl);
                            cs.Comment("Fragmented response system, action result is constructed and sent on-the-fly.");
                            cs.Append("  " + cppobj + pc_methodPrefix + "AsyncResponse_START(upnptoken, \"" + action.Name + "\", \"" + service.ServiceURN + "\");" + cl);
                            foreach (UPnPArgument arg in action.Arguments)
                            {
                                if (arg.Direction == "out")
                                {
                                    if (!Configuration.HTTP_1dot1)
                                    {
                                        cs.Append("  " + cppobj + pc_methodPrefix + "AsyncResponse_OUT(upnptoken, \"" + arg.Name + "\", \"\", 0, 1, 1);" + cl);
                                    }
                                    else
                                    {
                                        cs.Append("  " + cppobj + pc_methodPrefix + "AsyncResponse_OUT(upnptoken, \"" + arg.Name + "\", \"\", 0, " + pc_methodLibPrefix + "AsyncSocket_MemoryOwnership_STATIC, 1, 1);" + cl);
                                    }
                                }
                            }
                            cs.Append("  " + cppobj + pc_methodPrefix + "AsyncResponse_DONE(upnptoken, \"" + action.Name + "\");" + cl);
                            #endregion
                        }

                        cs.Append("}" + cl);
                        #endregion

                        #endregion

                        cs.Append(cl);
                    }
                }
            }
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                this.BuildMainUserCode(cs, d, serviceNames);
            }
        }

        private void BuildMain_SetFunctionPointers(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
        {
            string cppobj = "";
            if (this.Language == LANGUAGES.CPP) { cppobj = "microstack->"; }

            SortedList SL = new SortedList();
            IDictionaryEnumerator en = serviceNames.GetEnumerator();

            while (en.MoveNext())
            {
                SL[en.Value] = en.Key;
            }
            en = SL.GetEnumerator();

            while (en.MoveNext())
            {
                UPnPService S = (UPnPService)en.Value;
                string name = (string)en.Key;

                if (name != "DeviceSecurity")
                {
                    foreach (UPnPAction A in S.Actions)
                    {
                        cs.Append("    " + cppobj + pc_methodPrefix + "FP_" + name + "_" + A.Name + " = (UPnP_ActionHandler_" + name + "_" + A.Name + ")&" + pc_methodPrefix + name + "_" + A.Name + ";" + cl);
                    }
                }
            }
            cs.Append(cl);
        }

        private int GetTotalNumberOfDevices(UPnPDevice device)
        {
            int RetVal = 1;

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                RetVal += GetTotalNumberOfDevices(d);
            }

            return (RetVal);
        }

        private void Fix(UPnPDevice device, int number, Hashtable serviceNameTable)
        {
            device.User3 = new object[9]{
											device.SerialNumber,
											device.FriendlyName,
											device.Manufacturer,
											device.ManufacturerURL,
											device.ModelDescription,
											device.ModelName,
											device.ModelNumber,
											device.ModelURL,
											device.ProductCode};
            if (device.FriendlyName != "%s")
            {
                FriendlyNameTable[device] = device.FriendlyName;
            }
            if (device.Root)
            {
                device.UniqueDeviceName = "%s";
            }
            else
            {
                device.UniqueDeviceName = "%s_" + number.ToString();
            }
            device.SerialNumber = "%s";
            device.FriendlyName = "%s";
            if (Configuration.DynamicObjectModel)
            {
                device.Manufacturer = "%s";
                device.ManufacturerURL = "%s";
                device.ModelDescription = "%s";
                device.ModelName = "%s";
                device.ModelNumber = "%s";
                device.ModelURL = new Uri("http://255.255.255.255:255/");
                device.ProductCode = "%s";
            }
            foreach (UPnPService service in device.Services)
            {
                UPnPDebugObject obj = new UPnPDebugObject(service);
                obj.SetField("SCPDURL", (string)serviceNameTable[service] + "/scpd.xml");
                obj.SetField("__controlurl", (string)serviceNameTable[service] + "/control");
                bool eventOK = false;
                foreach (UPnPStateVariable sv in service.GetStateVariables())
                {
                    if (sv.SendEvent)
                    {
                        eventOK = true;
                        break;
                    }
                }
                if (eventOK)
                {
                    obj.SetField("__eventurl", (string)serviceNameTable[service] + "/event");
                }
                else
                {
                    obj.SetField("__eventurl", "");
                }
            }

            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                Fix(d, ++number, serviceNameTable);
            }
        }

        public string PrintfTransform(string data)
        {
            data = data.Replace("\\", "\\\\");
            data = data.Replace("\r", "\\r");
            data = data.Replace("\n", "\\n");
            data = data.Replace("\"", "\\\"");
            return data;
        }

        public string ToCType(string t)
        {
            return (Static_ToCType(t));
        }
        public static string Static_ToCType(string t)
        {
            switch (t)
            {
                case "System.Char": return "char";
                case "System.String": return "char*";
                case "System.Boolean": return "int";
                case "System.Uri": return "char*";
                case "System.Byte": return "unsigned char";
                case "System.UInt16": return "unsigned short";
                case "System.UInt32": return "unsigned int";
                case "System.Int32": return "int";
                case "System.Int16": return "short";
                case "System.SByte": return "char";
                case "System.Single": return "float";
                case "System.Double": return "double";
                case "System.Byte[]": return "unsigned char*";
                case "System.DateTime": return "time_t";
                default: return "char*";
            }
        }
        public static string ToCTypeFromStateVar(UPnPStateVariable V)
        {
            if (V.ComplexType == null)
            {
                string RetVal = Static_ToCType(V.GetNetType().ToString()) + " " + V.Name;
                if (Static_ToCType(V.GetNetType().ToString()) == "unsigned char*")
                {
                    RetVal += ", int " + V.Name + "Length";
                }
                return (RetVal);
            }
            else
            {
                return ("struct " + V.ComplexType.Name_LOCAL + "* " + V.Name);
            }
        }
        public static string ToCTypeFromStateVar_Dispatch(UPnPStateVariable V)
        {
            if (V.ComplexType == null)
            {
                string RetVal = V.Name;
                if (Static_ToCType(V.GetNetType().ToString()) == "unsigned char*")
                {
                    RetVal += ", " + V.Name + "Length";
                }
                return (RetVal);
            }
            else
            {
                return (V.Name);
            }
        }
        public static string ToCTypeFromStateVar_Serialize(string InVar, string InVarLength, string OK, string LibPrefix, UPnPStateVariable A)
        {
            StringBuilder cs = new StringBuilder();

            if (OK != "")
            {
                OK = OK + " = ";
            }

            switch (A.GetNetType().FullName)
            {
                case "System.DateTime":
                    cs.Append("	" + A.Name + " = " + LibPrefix + "Time_Parse(" + InVar + ");" + cl);
                    break;
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    cs.Append("	" + OK + LibPrefix + "GetLong(" + InVar + ", " + InVarLength + ", (long*)&" + A.Name + ");" + cl);
                    break;
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    cs.Append("	" + OK + LibPrefix + "GetULong(" + InVar + ", " + InVarLength + ", (unsigned long*)&" + A.Name + ");" + cl);
                    break;
                case "System.Boolean":
                    if (OK != "")
                    {
                        cs.Append(OK + "0;" + cl);
                    }
                    cs.Append("	if (" + InVarLength + "==4)" + cl);
                    cs.Append("	{" + cl);
                    cs.Append("		if (strncasecmp(" + InVar + ", \"true\", 4)==0)" + cl);
                    cs.Append("		{" + cl);
                    if (OK != "")
                    {
                        cs.Append(OK + "1;" + cl);
                    }
                    cs.Append("			" + A.Name + " = 1;" + cl);
                    cs.Append("		}" + cl);
                    cs.Append("	}" + cl);
                    cs.Append("	if (" + InVarLength + " == 5)" + cl);
                    cs.Append("	{" + cl);
                    cs.Append("		if (strncasecmp(" + InVar + ", \"false\", 5)==0)" + cl);
                    cs.Append("		{" + cl);
                    if (OK != "")
                    {
                        cs.Append(OK + "1;" + cl);
                    }
                    cs.Append("			" + A.Name + " = 0;" + cl);
                    cs.Append("		}" + cl);
                    cs.Append("	}" + cl);
                    cs.Append("	if (" + InVarLength + " == 1)" + cl);
                    cs.Append("	{" + cl);
                    cs.Append("		if (memcmp(" + InVar + ", \"0\", 1) == 0)" + cl);
                    cs.Append("		{" + cl);
                    if (OK != "")
                    {
                        cs.Append(OK + "1;" + cl);
                    }
                    cs.Append("			" + A.Name + " = 0;" + cl);
                    cs.Append("		}" + cl);
                    cs.Append("		if (memcmp(" + InVar + ", \"1\", 1) == 0)" + cl);
                    cs.Append("		{" + cl);
                    if (OK != "")
                    {
                        cs.Append(OK + "1;" + cl);
                    }
                    cs.Append("			" + A.Name + " = 1;" + cl);
                    cs.Append("		}" + cl);
                    cs.Append("	}" + cl);
                    break;
                case "System.Byte[]":
                    cs.Append(A.Name + "Length = " + LibPrefix + "Base64Decode(" + InVar + ", " + InVarLength + ", &" + A.Name + ");" + cl);
                    break;
                case "System.Uri":
                case "System.String":
                default:
                    if (A.ComplexType == null)
                    {
                        cs.Append("	" + A.Name + "Length = " + LibPrefix + "InPlaceXmlUnEscape(" + InVar + ");" + cl);
                        cs.Append("	" + A.Name + " = " + InVar + ";" + cl);
                    }
                    else
                    {
                        cs.Append(cl + "//ToDo: DeviceBuilder needs to be modified to include a ComplexTypeParsre here!");
                    }
                    break;
            }
            return (cs.ToString());
        }
        public static string ToCTypeFromArg(UPnPArgument A)
        {
            if (A.RelatedStateVar.ComplexType == null)
            {
                string RetVal = Static_ToCType(A.RelatedStateVar.GetNetType().ToString()) + " " + A.Name;
                if (Static_ToCType(A.RelatedStateVar.GetNetType().ToString()) == "unsigned char*")
                {
                    RetVal += ", int " + A.Name + "Length";
                }
                return (RetVal);
            }
            else
            {
                return ("struct " + A.RelatedStateVar.ComplexType.Name_LOCAL + "* " + A.Name);
            }
        }
        public static string ToCTypeFromArg_Dispatch(UPnPArgument A)
        {
            if (A.RelatedStateVar.ComplexType == null)
            {
                string RetVal = A.Name;
                if (Static_ToCType(A.RelatedStateVar.GetNetType().ToString()) == "unsigned char*")
                {
                    RetVal += ", " + A.Name + "Length";
                }
                return (RetVal);
            }
            else
            {
                return (A.Name);
            }
        }


        public static string Static_ToPrintfType(string t)
        {
            switch (t)
            {
                case "System.Byte[]":
                case "System.String":
                case "System.Uri":
                    return "%s";
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    return "%u";
                case "System.Boolean":
                case "System.DateTime":
                case "System.Char":
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    return "%d";
                case "System.Single":
                case "System.Double":
                    return "%f";
                default:
                    return "void";
            }
        }
        public string ToPrintfType(string t)
        {
            return (Static_ToPrintfType(t));
        }
        public static string Static_ToSPrintfType(string t)
        {
            switch (t)
            {
                case "System.DateTime":
                case "System.Byte[]":
                case "System.String":
                case "System.Uri":
                    return "%s";
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                    return "%u";
                case "System.Boolean":
                case "System.Char":
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                    return "%d";
                case "System.Single":
                case "System.Double":
                    return "%f";
                default:
                    return "void";
            }
        }
        public string ToSPrintfType(string t)
        {
            return (Static_ToSPrintfType(t));
        }

        public static string Static_ToSampleValue(string t)
        {
            switch (t)
            {
                case "System.Boolean":
                    return "1";
                case "System.Byte[]":
                    return "\"Sample Binary\",13";
                case "System.String":
                    return "\"Sample String\"";
                case "System.Uri":
                    return "\"http://opentools.homeip.net\"";
                case "System.Byte":
                    return "250";
                case "System.UInt16":
                    return "250";
                case "System.UInt32":
                    return "250";
                case "System.Char":
                case "System.SByte":
                    return "250";
                case "System.Int16":
                    return "25000";
                case "System.Int32":
                    return "25000";
                case "System.Single":
                case "System.Double":
                    return "0.01";
                case "System.DateTime":
                    return "0";
                default:
                    return "NULL";
            }
        }
        public string ToSampleValue(string t)
        {
            return (Static_ToSampleValue(t));
        }

        public string ToEmptyValue(string t)
        {
            switch (t)
            {
                case "System.Byte[]":
                    return "NULL";
                case "System.String":
                case "System.Uri":
                    return "\"\"";
                case "System.DateTime":
                case "System.Boolean":
                case "System.Byte":
                case "System.UInt16":
                case "System.UInt32":
                case "System.Char":
                case "System.SByte":
                case "System.Int16":
                case "System.Int32":
                case "System.Single":
                case "System.Double":
                    return "0";
                default:
                    return "NULL";
            }
        }

        public int FromHex(string hn)
        {
            return (int.Parse(hn.ToUpper(), System.Globalization.NumberStyles.HexNumber));
        }
        public string ToHex(object obj)
        {
            if (obj.GetType().FullName == "System.UInt32")
            {
                UInt32 unumber = UInt32.Parse(obj.ToString());
                return (unumber.ToString("X"));
            }
            else
            {
                Int32 number = Int32.Parse(obj.ToString());
                return (number.ToString("X"));
            }
        }
        private int CalculateLength(string s)
        {
            int ln = s.Length;
            int c = 0;

            while (s.IndexOf("\\r", c) != -1)
            {
                c = s.IndexOf("\\r", c) + 1;
                ln -= 1;
            }

            c = 0;
            while (s.IndexOf("\\n", c) != -1)
            {
                c = s.IndexOf("\\n", c) + 1;
                ln -= 1;
            }

            c = 0;
            while (s.IndexOf("\\0", c) != -1)
            {
                c = s.IndexOf("\\0", c) + 1;
                ln -= 1;
            }

            c = 0;
            while (s.IndexOf("\\\"", c) != -1)
            {
                c = s.IndexOf("\\\"", c) + 1;
                ln -= 1;
            }

            return (ln);
        }


        #region Complex Type Methods


        public static void BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Nested(CodeProcessor cs, UPnPComplexType.ItemCollection[] icList, Hashtable SequenceTable, Hashtable ChoiceTable)
        {
            int x = 0;
            foreach (UPnPComplexType.ItemCollection ic in icList)
            {
                if (ic.GetType() == typeof(UPnPComplexType.Sequence))
                {
                    cs.Append("	struct SEQUENCE_" + SequenceTable[ic].ToString() + " *_sequence_" + (++x).ToString() + ";" + cl);
                }
                else if (ic.GetType() == typeof(UPnPComplexType.Choice))
                {
                    cs.Append("	struct CHOICE_" + ChoiceTable[ic].ToString() + " *_choice_" + (++x).ToString() + ";" + cl);
                }
                //ToDo: Insert MaxOccurs Logic
            }
        }

        public static void BuildComplexTypeDefinitionsAndHeaders_InnerCollections(CodeProcessor cs, UPnPService service, UPnPComplexType.ItemCollection ic, Hashtable SequenceTable, Hashtable ChoiceTable)
        {
            if (ic.Items.Length > 0 || ic.NestedCollections.Length > 0)
            {
                if (ic.GetType() == typeof(UPnPComplexType.Sequence))
                {
                    cs.Append("struct SEQUENCE_" + SequenceTable[ic].ToString() + cl);
                    cs.Append("{" + cl);
                    BuildComplexTypeDefinitionsAndHeaders_FillInner(cs, service, ic.Items);
                    BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Nested(cs, ic.NestedCollections, SequenceTable, ChoiceTable);
                    cs.Append("};" + cl);
                }
                else if (ic.GetType() == typeof(UPnPComplexType.Choice))
                {
                    cs.Append("struct CHOICE_" + ChoiceTable[ic].ToString() + cl);
                    cs.Append("{" + cl);
                    BuildComplexTypeDefinitionsAndHeaders_FillInner(cs, service, ic.Items);
                    BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Nested(cs, ic.NestedCollections, SequenceTable, ChoiceTable);
                    cs.Append("};" + cl);
                }
            }

            foreach (UPnPComplexType.ItemCollection nc in ic.NestedCollections)
            {
                BuildComplexTypeDefinitionsAndHeaders_InnerCollections(cs, service, nc, SequenceTable, ChoiceTable);
            }
        }

        public static void BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Number(UPnPComplexType CT, CodeProcessor cs, UPnPComplexType.ItemCollection ic, Hashtable SequenceTable, Hashtable ChoiceTable, ref int SequenceCounter, ref int ChoiceCounter)
        {
            if (ic.GetType() == typeof(UPnPComplexType.Sequence))
            {
                SequenceTable[ic] = ++SequenceCounter;
                ((Hashtable)SequenceTable[CT])[ic] = SequenceCounter;
            }
            else if (ic.GetType() == typeof(UPnPComplexType.Choice))
            {
                ChoiceTable[ic] = ++ChoiceCounter;
                ((Hashtable)ChoiceTable[CT])[ic] = ChoiceCounter;
            }

            foreach (UPnPComplexType.ItemCollection nc in ic.NestedCollections)
            {
                BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Number(CT, cs, nc, SequenceTable, ChoiceTable, ref SequenceCounter, ref ChoiceCounter);
            }
        }

        public static void BuildComplexTypeDefinitionsAndHeaders(SortedList SL, CodeProcessor cs, Hashtable SequenceTable, Hashtable ChoiceTable, ref int SequenceCounter, ref int ChoiceCounter, string pc_methodPrefix, string pc_methodLibPrefix)
        {
            IDictionaryEnumerator en = SL.GetEnumerator();

            // Build all the Inner Structs and Headers that are used by Sequences/Choices
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                foreach (UPnPComplexType CT in service.GetComplexTypeList())
                {
                    SequenceTable[CT] = new Hashtable();
                    ChoiceTable[CT] = new Hashtable();
                    foreach (UPnPComplexType.GenericContainer gc in CT.Containers)
                    {
                        foreach (UPnPComplexType.ItemCollection ic in gc.Collections)
                        {
                            BuildComplexTypeDefinitionsAndHeaders_InnerCollections_Number(CT, cs, ic, SequenceTable, ChoiceTable, ref SequenceCounter, ref ChoiceCounter);
                        }
                    }
                }
            }

            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                foreach (UPnPComplexType CT in service.GetComplexTypeList())
                {
                    foreach (UPnPComplexType.GenericContainer gc in CT.Containers)
                    {
                        foreach (UPnPComplexType.ItemCollection ic in gc.Collections)
                        {
                            BuildComplexTypeDefinitionsAndHeaders_InnerCollections(cs, service, ic, SequenceTable, ChoiceTable);
                        }
                    }
                }
            }


            // Build the Main Structs and Headers
            en.Reset();
            while (en.MoveNext())
            {
                UPnPService service = (UPnPService)en.Value;
                foreach (UPnPComplexType CT in service.GetComplexTypeList())
                {
                    int idx = 0;
                    cs.Append("struct " + CT.Name_LOCAL + cl);
                    cs.Append("{" + cl);

                    foreach (UPnPComplexType.GenericContainer gc in CT.Containers)
                    {
                        BuildComplexTypeDefinitionsAndHeaders_Containers(ref idx, cs, gc, service, SequenceTable, ChoiceTable);
                    }

                    cs.Append("};" + cl);
                }
            }
            BuildComplexTypeParser_Header(cs, SL, pc_methodPrefix, pc_methodLibPrefix);
            CPEmbeddedCGenerator.BuildComplexTypeSerializer_Header(cs, SL, pc_methodPrefix);
        }

        public static void BuildComplexTypeDefinitionsAndHeaders_Containers(ref int idx, CodeProcessor cs, UPnPComplexType.GenericContainer gc, UPnPService s, Hashtable SequenceTable, Hashtable ChoiceTable)
        {
            foreach (UPnPComplexType.ItemCollection ic in gc.Collections)
            {
                BuildComplexTypeDefinitionsAndHeaders_Collections(ref idx, cs, ic, s, SequenceTable, ChoiceTable);
            }
        }
        public static void BuildComplexTypeDefinitionsAndHeaders_Collections(ref int idx, CodeProcessor cs, UPnPComplexType.ItemCollection ic, UPnPService service, Hashtable SequenceTable, Hashtable ChoiceTable)
        {
            if (ic.GetType() == typeof(UPnPComplexType.Choice))
            {
                ++idx;
                UPnPComplexType.Choice ch = (UPnPComplexType.Choice)ic;
                cs.Append("	struct CHOICE_" + ChoiceTable[ic].ToString() + " *_choice_" + idx.ToString() + ";" + cl);
                if (ch.MaxOccurs != "" && (ch.MaxOccurs.ToLower() == "unbounded" || int.Parse(ch.MaxOccurs) > 1))
                {
                    cs.Append("	int _choice_" + idx.ToString() + "_Length;" + cl);
                }
            }
            else if (ic.GetType() == typeof(UPnPComplexType.Sequence))
            {
                ++idx;
                UPnPComplexType.Sequence sequ = (UPnPComplexType.Sequence)ic;
                cs.Append("	struct SEQUENCE_" + SequenceTable[ic].ToString() + " *_sequence_" + idx.ToString() + ";" + cl);
                if (sequ.MaxOccurs != "" && (sequ.MaxOccurs.ToLower() == "unbounded" || int.Parse(sequ.MaxOccurs) > 1))
                {
                    cs.Append("	int _sequence_" + idx.ToString() + "_Length;" + cl);
                }
            }
            else
            {
                BuildComplexTypeDefinitionsAndHeaders_FillInner(cs, service, ic.Items);
            }
        }

        public static void BuildComplexTypeDefinitionsAndHeaders_FillInner(CodeProcessor cs, UPnPService service, UPnPComplexType.ContentData[] Items)
        {
            string varType = "";

            foreach (UPnPComplexType.ContentData cd in Items)
            {
                switch (cd.TypeNS)
                {
                    case "http://www.w3.org/2001/XMLSchema":
                        // XSD Simple Types
                        switch (cd.Type)
                        {
                            case "unsignedByte":
                                varType = "unsigned byte";
                                break;
                            case "byte":
                                varType = "byte";
                                break;
                            case "unsignedInt":
                                varType = "unsigned int";
                                break;
                            case "unsignedShort":
                                varType = "unsigned short";
                                break;
                            case "unsignedLong":
                                varType = "unsigned long";
                                break;
                            case "boolean":
                            case "int":
                            case "integer":
                            case "positiveInteger":
                            case "negativeInteger":
                            case "nonNegativeInteger":
                            case "nonPositiveInteger":
                                varType = "int";
                                break;
                            case "long":
                                varType = "long";
                                break;
                            case "short":
                                varType = "short";
                                break;
                            case "decimal":
                            case "float":
                                varType = "single";
                                break;
                            case "double":
                                varType = "double";
                                break;
                            default:
                                varType = "char*";
                                break;
                        }
                        if (varType != "char*" && cd.MinOccurs == "0")
                        {
                            varType = varType + "*";
                        }
                        break;
                    default:
                        // User Defined Types
                        UPnPComplexType temp = service.GetComplexType(cd.TypeNS, cd.Type);
                        if (temp != null)
                        {
                            varType = "struct " + cd.Type + "*";
                        }
                        else
                        {
                            // Unknown type
                            varType = "char*";
                        }
                        break;
                }
                if (varType != "")
                {
                    cs.Append("	" + varType + " " + cd.Name + ";" + cl);
                }
            }
        }
        #endregion



    }
}
