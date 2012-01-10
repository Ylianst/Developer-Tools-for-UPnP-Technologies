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
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
    /// <summary>
    /// Summary description for SourceCodeRepository.
    /// </summary>
    public class SourceCodeRepository
    {
        public SourceCodeRepository()
        {
        }

        private static void ProcessService(UPnPService s, Hashtable RetVal)
        {
            RetVal[s] = ((ServiceGenerator.ServiceConfiguration)s.User).Name;
        }
        private static void ProcessDevice(UPnPDevice d, Hashtable RetVal)
        {
            foreach (UPnPDevice ed in d.EmbeddedDevices) { ProcessDevice(ed, RetVal); }
            foreach (UPnPService s in d.Services) { ProcessService(s, RetVal); }
        }
        public static Hashtable CreateTableOfServiceNames(UPnPDevice rootDevice)
        {
            Hashtable RetVal = new Hashtable();
            foreach (UPnPDevice d in rootDevice.EmbeddedDevices) { ProcessDevice(d, RetVal); }
            foreach (UPnPService s in rootDevice.Services) { ProcessService(s, RetVal); }
            return (RetVal);
        }

        public static string ReadString(string s) { return (UTF8Encoding.UTF8.GetString(Base64.Decode(s))); }

        public static string ReadFileStore(string filename)
        {
            DirectoryInfo di = new DirectoryInfo(Application.StartupPath + "\\FileStore");
            if (di.Exists == false) di = new DirectoryInfo(Application.StartupPath + "\\..\\..\\FileStore");
            if (di.Exists == false) di = new DirectoryInfo(Application.StartupPath + "\\..\\..\\..\\FileStore");
            FileStream f = new FileStream(di.FullName + "\\" + filename, FileMode.Open, FileAccess.Read);
            byte[] buf = new byte[f.Length];
            f.Read(buf, 0, buf.Length);
            f.Close();
            return (UTF8Encoding.UTF8.GetString(buf, 0, buf.Length));
        }

        public static byte[] ReadFileStoreBin(string filename)
        {
            DirectoryInfo di = new DirectoryInfo(Application.StartupPath + "\\FileStore");
            if (di.Exists == false) di = new DirectoryInfo(Application.StartupPath + "\\..\\..\\FileStore");
            if (di.Exists == false) di = new DirectoryInfo(Application.StartupPath + "\\..\\..\\..\\FileStore");
            FileStream f = new FileStream(di.FullName + "\\" + filename, FileMode.Open, FileAccess.Read);
            byte[] buf = new byte[f.Length];
            f.Read(buf, 0, buf.Length);
            f.Close();
            return buf;
        }

        public static byte[] Get_SampleProject_ico()        { return ReadFileStoreBin("PPC\\SampleProject.ico"); }
        public static string Get_SampleProject_vcp()        { return ReadFileStore("PPC\\SampleProject.vcp"); }
        public static string Get_SampleProject_vcw()        { return ReadFileStore("PPC\\SampleProject.vcw"); }
        public static string Get_StdAfx_cpp()               { return ReadFileStore("PPC\\StdAfx.cpp"); }
        public static string Get_StdAfx_h()                 { return ReadFileStore("PPC\\StdAfx.h"); }
        public static string Get_SampleProject_rc()         { return ReadFileStore("PPC\\SampleProject.rc"); }
        public static string Get_SampleProjectDlg_h()       { return ReadFileStore("PPC\\SampleProjectDlg.h"); }
        public static string Get_SampleProjectDlg_cpp()     { return ReadFileStore("PPC\\SampleProjectDlg.cpp"); }
        public static string Get_SampleProject_h()          { return ReadFileStore("PPC\\SampleProject.h"); }
        public static string Get_SampleProject_cpp()        { return ReadFileStore("PPC\\SampleProject.cpp"); }
        public static string Get_resource_h()               { return ReadFileStore("PPC\\resource.h"); }
        public static string Get_newres_h()                 { return ReadFileStore("PPC\\newres.h"); }
        public static string Get_Makefile()                 { return ReadFileStore("posix\\makefile"); }
        public static string Get_UPnPSample_sln()           { return ReadFileStore("Win32\\UPnPSample.sln"); }
        public static string Get_UPnPSample_vcproj()        { return ReadFileStore("Win32\\UPnPSample.vcproj"); }
        public static string Get_Win32_stdafx_h()           { return ReadFileStore("Win32\\stdafx.h"); }
        public static string Get_Win32_stdafx_cpp()         { return ReadFileStore("Win32\\stdafx.cpp"); }
        public static byte[] Get_CP_SampleProject_ico()     { return ReadFileStoreBin("Sample.ico"); }
        public static string Get_CP_SampleProject_vcp()     { return ReadFileStore("PPC_CP\\SampleProject.vcp"); }
        public static string Get_CP_SampleProject_vcw()     { return ReadFileStore("PPC_CP\\SampleProject.vcw"); }
        public static string Get_CP_StdAfx_cpp()            { return ReadFileStore("PPC_CP\\StdAfx.cpp"); }
        public static string Get_CP_StdAfx_h()              { return ReadFileStore("PPC_CP\\StdAfx.h"); }
        public static string Get_CP_SampleProject_rc()      { return ReadFileStore("PPC_CP\\SampleProject.rc"); }
        public static string Get_CP_SampleProjectDlg_h()    { return ReadFileStore("PPC_CP\\SampleProjectDlg.h"); }
        public static string Get_CP_SampleProjectDlg_cpp()  { return ReadFileStore("PPC_CP\\SampleProjectDlg.cpp"); }
        public static string Get_CP_SampleProject_h()       { return ReadFileStore("PPC_CP\\SampleProject.h"); }
        public static string Get_CP_SampleProject_cpp()     { return ReadFileStore("PPC_CP\\SampleProject.cpp"); }
        public static string Get_CP_resource_h()            { return ReadFileStore("PPC_CP\\resource.h"); }
        public static string Get_CP_newres_h()              { return ReadFileStore("PPC_CP\\newres.h"); }

        public static string Get_Generic(string VariableName)
        {
            string varData;

            System.Reflection.FieldInfo fi = typeof(SourceCodeRepository).GetField(
                VariableName,
                System.Reflection.BindingFlags.Static |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Public);
            
            if (fi != null)
            {
                varData = (string)fi.GetValue(null);
                return (ReadString(varData));
            }
            return "";
        }

        public static void Generate_ThreadPool(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "ThreadPool.h", "") + RemoveOldEULA(ReadFileStore("ILibThreadPool.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "ThreadPool.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "ThreadPool.c", "") + RemoveOldEULA(ReadFileStore("ILibThreadPool.c"));
            lib = lib.Replace("ILib", PreFix);
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "ThreadPool.c");
            writer.Write(lib);

            writer.Close();
        }

        public static string GetMain_C_Template()                       { return (GetEULA("Main.c", "") + RemoveOldEULA(ReadFileStore("Main.c"))); }
        public static string GetControlPoint_C_Template(string PreFix)  { return (GetEULA(PreFix + "ControlPoint.c", "") + RemoveOldEULA(ReadFileStore("UPnPControlPoint.c"))); }
        public static string GetControlPoint_H_Template(string PreFix)  { return (GetEULA(PreFix + "ControlPoint.h", "") + RemoveOldEULA(ReadFileStore("UPnPControlPoint.h"))); }
        public static string GetMicroStack_C_Template(string PreFix)    { return (GetEULA(PreFix + "MicroStack.c", "") + RemoveOldEULA(ReadFileStore("UPnPMicroStack.c"))); } 
        public static string GetMicroStack_H_Template(string PreFix)    { return (GetEULA(PreFix + "MicroStack.h", "") + RemoveOldEULA(ReadFileStore("UPnPMicroStack.h"))); }
        public static string GetCPlusPlus_Template_H(string PreFix)     { return (GetEULA(PreFix + "Abstraction.h", "") + RemoveOldEULA(ReadFileStore("CPlusPlus\\UPnPAbstraction.h"))); }
        public static string GetCPlusPlus_Template_CPP(string PreFix)   { return (GetEULA(PreFix + "Abstraction.cpp", "") + RemoveOldEULA(ReadFileStore("CPlusPlus\\UPnPAbstraction.cpp"))); }

        private static string GetEULA(string FileName, string Settings)
        {
            string RetVal = ReadFileStore("EULA.txt");
            if (Settings == "") { Settings = "*"; }

            RetVal = RetVal.Replace("<REVISION>", "#" + Application.ProductVersion);
            RetVal = RetVal.Replace("<DATE>", DateTime.Now.ToLongDateString());
            RetVal = RetVal.Replace("<FILE>", FileName);
            RetVal = RetVal.Replace("<SETTINGS>", Settings);

            return (RetVal);
        }

        private static string RemoveOldEULA(string InVal)
        {
            string RetVal = InVal;
            RetVal = RetVal.Substring(2 + RetVal.IndexOf("*/"));
            return (RetVal);
        }

        public static void Generate_Parsers(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "Parsers.h", "") + RemoveOldEULA(ReadFileStore("ILibParsers.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "Parsers.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "Parsers.c", "") + RemoveOldEULA(ReadFileStore("ILibParsers.c"));
            lib = lib.Replace("ILib", PreFix);
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "Parsers.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_AsyncUDPSocket(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "AsyncSocket.h", "") + RemoveOldEULA(ReadFileStore("ILibAsyncUDPSocket.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncUDPSocket.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "AsyncSocket.c", "") + RemoveOldEULA(ReadFileStore("ILibAsyncUDPSocket.c"));
            lib = lib.Replace("ILib", PreFix);
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncUDPSocket.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_AsyncSocket(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "AsyncSocket.h", "") + RemoveOldEULA(ReadFileStore("ILibAsyncSocket.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncSocket.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "AsyncSocket.c", "") + RemoveOldEULA(ReadFileStore("ILibAsyncSocket.c"));
            lib = lib.Replace("ILib", PreFix);
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncSocket.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_AsyncServerSocket(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "AsyncServerSocket.h", "") + RemoveOldEULA(ReadFileStore("ILibAsyncServerSocket.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncServerSocket.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "AsyncServerSocket.c", "") + RemoveOldEULA(ReadFileStore("ILibAsyncServerSocket.c"));
            lib = lib.Replace("ILib", PreFix);
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "AsyncServerSocket.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_WebClient(ServiceGenerator.StackConfiguration config, DirectoryInfo outputDir)
        {
            bool Legacy = !config.HTTP_1dot1;
            string PreFix = config.prefixlib;
            int i;

            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "WebClient.h", "") + RemoveOldEULA(ReadFileStore("ILibWebClient.h"));
            lib = lib.Replace("ILib", PreFix);

            //
            // Buffer Limitations
            //
            lib = ReplaceLine(lib, "#define INITIAL_BUFFER_SIZE", "#define INITIAL_BUFFER_SIZE " + config.InitialHTTPBufferSize.ToString() + "\n");
            i = lib.IndexOf("#define MAX_HTTP_HEADER_SIZE");
            if (i != -1)
            {
                if (config.MaxHTTPHeaderSize > 0)
                {
                    lib = ReplaceLine(lib, "#define MAX_HTTP_HEADER_SIZE", "#define MAX_HTTP_HEADER_SIZE " + config.MaxHTTPHeaderSize.ToString() + "\n");
                }
                else
                {
                    lib = DeleteLine(lib, "#define MAX_HTTP_HEADER_SIZE");
                }
            }
            else
            {
                if (config.MaxHTTPHeaderSize > 0)
                {
                    i = GetIndexOfNextLine(lib, "#define INITIAL_BUFFER_SIZE");
                    if (i != -1)
                    {
                        lib = lib.Insert(i, "#define MAX_HTTP_HEADER_SIZE " + config.MaxHTTPHeaderSize.ToString() + "\n");
                    }
                }
            }
            i = lib.IndexOf("#define MAX_HTTP_PACKET_SIZE");
            if (i != -1)
            {
                if (config.MaxHTTPPacketSize > 0)
                {
                    lib = ReplaceLine(lib, "#define MAX_HTTP_PACKET_SIZE", "#define MAX_HTTP_PACKET_SIZE " + config.MaxHTTPPacketSize.ToString() + "\n");
                }
                else
                {
                    lib = DeleteLine(lib, "#define MAX_HTTP_PACKET_SIZE");
                }
            }
            else
            {
                if (config.MaxHTTPPacketSize > 0)
                {
                    i = GetIndexOfNextLine(lib, "#define INITIAL_BUFFER_SIZE");
                    if (i != -1)
                    {
                        lib = lib.Insert(i, "#define MAX_HTTP_PACKET_SIZE " + config.MaxHTTPPacketSize.ToString() + "\n");
                    }
                }
            }

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "WebClient.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "WebClient.c", "") + RemoveOldEULA(ReadFileStore("ILibWebClient.c"));
            lib = lib.Replace("ILib", PreFix);

            if (Legacy)
            {
                // Remove HTTP/1.1 Specific Code
                string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
                string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
                int ix = lib.IndexOf(xx);
                int iy;
                while (ix != -1)
                {
                    iy = lib.IndexOf(yy) + yy.Length;
                    lib = lib.Remove(ix, iy - ix);
                    ix = lib.IndexOf(xx);
                }
            }

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "WebClient.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_WebServer(ServiceGenerator.StackConfiguration config, DirectoryInfo outputDir)
        {
            bool Legacy = !config.HTTP_1dot1;
            string PreFix = config.prefixlib;
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "WebServer.h", "") + RemoveOldEULA(ReadFileStore("ILibWebServer.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "WebServer.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "WebServer.c", "") + "\r\n";
            if (Legacy)
            {
                lib += "#define HTTPVERSION \"1.0\"";
            }
            else
            {
                lib += "#define HTTPVERSION \"1.1\"";
            }
            lib += RemoveOldEULA(ReadFileStore("ILibWebServer.c"));
            lib = lib.Replace("ILib", PreFix);
            if (Legacy)
            {
                // Remove HTTP/1.1 Specific Code
                string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
                string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
                int ix = lib.IndexOf(xx);
                int iy;
                while (ix != -1)
                {
                    iy = lib.IndexOf(yy) + yy.Length;
                    lib = lib.Remove(ix, iy - ix);
                    ix = lib.IndexOf(xx);
                }
            }
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "WebServer.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_Generic(string PreFix, DirectoryInfo outputDir, string FileName, string VariableName)
        {
            StreamWriter writer;
            string lib;
            string varData;

            System.Reflection.FieldInfo fi = typeof(SourceCodeRepository).GetField(
                VariableName,
                System.Reflection.BindingFlags.Static |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Public);
            if (fi != null)
            {
                varData = (string)fi.GetValue(null);

                lib = GetEULA(PreFix + FileName, "") + "\r\n" + RemoveOldEULA(ReadString(varData));
                lib = lib.Replace("ILib", PreFix);

                writer = File.CreateText(outputDir.FullName + "\\" + PreFix + FileName);
                writer.Write(lib);
                writer.Close();
            }
        }

        public static string RemoveAndClearTag(string BeginTag, string EndTag, string data)
        {
            // Remove HTTP/1.1 Specific Code
            int ix = data.IndexOf(BeginTag);
            int iy;
            while (ix != -1)
            {
                iy = data.IndexOf(EndTag) + EndTag.Length;
                if (data.Substring(iy, 2).Equals("\r\n") == true) iy += 2;
                data = data.Remove(ix, iy - ix);
                ix = data.IndexOf(BeginTag);
            }
            return data;
        }

        public static string RemoveTag(string BeginTag, string EndTag, string data)
        {
            data = data.Replace(BeginTag + "\r\n", "");
            data = data.Replace(BeginTag, "");
            data = data.Replace(EndTag + "\r\n", "");
            data = data.Replace(EndTag, "");
            return data;
        }

        public static void Generate_SSDPClient(string PreFix, DirectoryInfo outputDir, bool UPNP_1_1)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA(PreFix + "SSDPClient.h", "") + RemoveOldEULA(ReadFileStore("ILibSSDPClient.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "SSDPClient.h");
            writer.Write(lib);
            writer.Close();

            lib = GetEULA(PreFix + "SSDPClient.c", "") + RemoveOldEULA(ReadFileStore("ILibSSDPClient.c"));
            lib = lib.Replace("ILib", PreFix);
            if (!UPNP_1_1)
            {
                // Legacy
                lib = SourceCodeRepository.RemoveTag("//{{{BEGIN_UPNP_1_0}}}", "//{{{END_UPNP_1_0}}}", lib);
                lib = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_UPNP_1_1}}}", "//{{{END_UPNP_1_1}}}", lib);
            }
            else
            {
                // UPnP/1.1+
                lib = SourceCodeRepository.RemoveTag("//{{{BEGIN_UPNP_1_1}}}", "//{{{END_UPNP_1_1}}}", lib);
                lib = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_UPNP_1_0}}}", "//{{{END_UPNP_1_0}}}", lib);
            }
            writer = File.CreateText(outputDir.FullName + "\\" + PreFix + "SSDPClient.c");
            writer.Write(lib);

            writer.Close();
        }

        public static void Generate_UPnPControlPointStructs(string PreFix, DirectoryInfo outputDir)
        {
            StreamWriter writer;
            string lib;

            lib = GetEULA("UPnPControlPointStructs.h", "") + RemoveOldEULA(ReadFileStore("UPnPControlPointStructs.h"));
            lib = lib.Replace("ILib", PreFix);

            writer = File.CreateText(outputDir.FullName + "\\UPnPControlPointStructs.h");
            writer.Write(lib);
            writer.Close();
        }

        public static string GetTextBetweenTags(string WS, string BeginTag, string EndTag)
        {
            // Remove HTTP/1.1 Specific Code
            int ix = WS.IndexOf(BeginTag);
            int iy = WS.IndexOf(EndTag);

            ix += BeginTag.Length;
            string RetVal = WS.Substring(ix, iy - ix);
            return (RetVal);
        }

        public static string InsertTextBeforeTag(string WS, string Tag, string TextToInsert)
        {
            int ix = WS.IndexOf(Tag);
            return (WS.Insert(ix, TextToInsert));
        }

        public static int GetIndexOfNextLine(string WS, int StartPos)
        {
            for (int i = StartPos; i < WS.Length; ++i) if (WS[i] == '\n') return (i + 1);
            return -1;
        }

        public static int GetIndexOfNextLine(string WS, string SearchString)
        {
            int i = WS.IndexOf(SearchString);
            if (i != -1) return (GetIndexOfNextLine(WS, i)); else return -1;
        }

        public static string ReplaceLine(string WS, string SearchString, string NewString)
        {
            int i = WS.IndexOf(SearchString);
            string RetVal = WS;
            if (i != -1)
            {
                int nline = GetIndexOfNextLine(WS, i);
                if (nline != -1)
                {
                    RetVal = WS.Remove(i, nline - i);
                    RetVal = RetVal.Insert(i, NewString);
                }
            }
            return (RetVal);
        }

        public static string DeleteLine(string WS, string SearchString)
        {
            int i = WS.IndexOf(SearchString);
            string RetVal = WS;
            if (i != -1)
            {
                int nline = GetIndexOfNextLine(WS, i);
                if (nline != -1) RetVal = WS.Remove(i, nline - i);
            }
            return (RetVal);
        }
    }
}

