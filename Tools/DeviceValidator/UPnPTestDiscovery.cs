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
using System.Net;
using System.Threading;
using System.Collections;
using System.Runtime.Serialization;
using OpenSource.UPnP;

namespace UPnPValidator.BasicTests
{
    /// <summary>
    /// Summary description for UPnPTestDiscovery.
    /// </summary>
    [Serializable()]
    public class UPnPTestDiscovery : BasicTestGroup
    {
        // Specific Test Variables
        UPnPTestStates NOTIFY = UPnPTestStates.Pass;
        UPnPTestStates DISCOVERY = UPnPTestStates.Pass;
        UPnPTestStates MX = UPnPTestStates.Pass;

        private Hashtable NotifyTable = new Hashtable();
        private Hashtable MSEARCHTable = new Hashtable();
        [NonSerialized()]
        private ManualResetEvent MRE;

        private AsyncSocket ASocket, ASocket2;
        private UPnPDevice TestDevice;
        private DataPointAnalyzer DPA = new DataPointAnalyzer();
        private int Cache = -1;
        //private DateTime StartTime;
        //private int TimeLeft = 0;
        //private int TotalTime = 0;
        private string sample = "";
        private string sample2 = "";

        public UPnPTestDiscovery()
        {
            Category = "Discovery";
            GroupName = "Discovery/Notification Test Suite";
            Description = "Discovery Test Suite. Tests various aspects of UPnP device discovery, network searching, advertizing and check reply timing and correct formatting of received messages.";

            AddTest("MX Value", "");
            AddTest("Notifications", "");
            AddTest("Discovery", "");

            Reset();
        }

        public override void Reset()
        {
            base.Reset();
            if (MRE == null) MRE = new ManualResetEvent(false);
        }


        public override void Start(UPnPDevice device)
        {
            UPnPDevice d = device;
            while (d.ParentDevice != null)
            {
                d = d.ParentDevice;
            }
            TestDevice = d;

            ASocket = new AsyncSocket(4096);
            ASocket.Attach(new IPEndPoint(TestDevice.InterfaceToHost, 0), System.Net.Sockets.ProtocolType.Udp);
            ASocket.SetTTL(4);
            ASocket.AddMembership((IPEndPoint)ASocket.LocalEndPoint, IPAddress.Parse("239.255.255.250"));
            ASocket.OnReceive += new AsyncSocket.OnReceiveHandler(ReceiveSink);
            ASocket.Begin();

            ASocket2 = new AsyncSocket(4096);
            ASocket2.Attach(new IPEndPoint(TestDevice.InterfaceToHost, 1900), System.Net.Sockets.ProtocolType.Udp);
            ASocket2.SetTTL(2);
            ASocket2.AddMembership((IPEndPoint)ASocket.LocalEndPoint, IPAddress.Parse("239.255.255.250"));
            ASocket2.OnReceive += new AsyncSocket.OnReceiveHandler(ReceiveSink2);

            Validate_MSEARCH_RESPONSETIME();
            Validate_NOTIFY();
            Validate_DISCOVERY();

            UPnPTestStates RetState = UPnPTestStates.Pass;
            if (NOTIFY == UPnPTestStates.Failed || DISCOVERY == UPnPTestStates.Failed)
            {
                RetState = UPnPTestStates.Failed;
            }
            else
            {
                if (NOTIFY == UPnPTestStates.Warn || DISCOVERY == UPnPTestStates.Warn || MX == UPnPTestStates.Warn)
                {
                    RetState = UPnPTestStates.Warn;
                }
            }

            state = RetState;
        }

        private void Validate_MSEARCH_RESPONSETIME()
        {
            AddMessage(0, "Testing Notifications");
            HTTPMessage r = new HTTPMessage();
            r.Directive = "M-SEARCH";
            r.DirectiveObj = "*";
            r.AddTag("MX", "10");
            r.AddTag("ST", "uuid:" + TestDevice.UniqueDeviceName);
            r.AddTag("Host", "239.255.255.250:1900");
            r.AddTag("MAN", "\"ssdp:discover\"");

            byte[] buf = r.RawPacket;
            IPEndPoint dest = new IPEndPoint(IPAddress.Parse("239.255.255.250"), 1900);
            Cache = -1;

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(0, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(15, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(30, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(45, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(60, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            MRE.Reset();
            StartTime = DateTime.Now;
            ASocket.Send(buf, 0, buf.Length, dest);
            StartCountDown(75, 90);
            MRE.WaitOne(15000, false);
            AbortCountDown();

            double s = 0;
            try
            {
                s = DPA.StandardDeviation.TotalSeconds;
            }
            catch (DivideByZeroException) {}

            if (s < (double)1.50)
            {
                AddEvent(LogImportance.Medium, "M-SEARCH, MX Value",
                    "WARNING: Device not choosing Random interval based on MX value <<Standard Deviation: " + s.ToString() + ">>");
                Results.Add("M-SEARCH Response time not choosing Random interval based on MX value");
                MX = UPnPTestStates.Warn;
                SetState("MX Value", UPnPTestStates.Warn);
            }
            else
            {
                MX = UPnPTestStates.Pass;
                AddEvent(LogImportance.Remark, "M-SEARCH, MX Value",
                    "Random MX interval: <<Standard Deviation: " + s.ToString() + ">> OK");
                Results.Add("M-SEARCH Response time OK");
                SetState("MX Value", UPnPTestStates.Pass);
            }
        }
        private void ReceiveSink(AsyncSocket sender, Byte[] buffer, int HeadPointer, int BufferSize, int BytesRead, IPEndPoint source, IPEndPoint remote)
        {
            DateTime EndTime = DateTime.Now;
            DText P = new DText();
            HTTPMessage msg = HTTPMessage.ParseByteArray(buffer, 0, BufferSize);

            string USN = msg.GetTag("USN");
            string UDN = USN;
            if (USN.IndexOf("::") != -1)
            {
                UDN = USN.Substring(0, USN.IndexOf("::"));
            }

            UDN = UDN.Substring(5);
            sender.BufferBeginPointer = BufferSize;
            if (UDN != TestDevice.UniqueDeviceName) return;


            string cc = msg.GetTag("Cache-Control");
            P.ATTRMARK = "=";
            P[0] = cc;
            cc = P[2].Trim();
            this.Cache = int.Parse(cc);


            DPA.AddDataPoint(EndTime.Subtract(StartTime));

            MRE.Set();
        }

        private void ReceiveSink2(AsyncSocket sender, Byte[] buffer, int HeadPointer, int BufferSize, int BytesRead, IPEndPoint source, IPEndPoint remote)
        {
            HTTPMessage msg = HTTPMessage.ParseByteArray(buffer, 0, BufferSize);
            if (msg.Directive != "NOTIFY")
            {
                sender.BufferBeginPointer = BufferSize;
                return;
            }

            string USN = msg.GetTag("USN");
            string UDN;
            if (USN.IndexOf("::") != -1)
            {
                UDN = USN.Substring(0, USN.IndexOf("::"));
            }
            else
            {
                UDN = USN;
            }
            UDN = UDN.Substring(5);

            if (msg.GetTag("NTS").Trim() == "ssdp:alive")
            {
                if (TestDevice.GetDevice(UDN) != null)
                {
                    NotifyTable[msg.GetTag("NT")] = DateTime.Now;
                }
            }

            sender.BufferBeginPointer = BufferSize;
        }
        private void Validate_NOTIFY()
        {
            ManualResetEvent M = new ManualResetEvent(false);
            // Check to see if received all the NOTIFY packets
            NotifyTable.Clear();
            ASocket2.Begin();
            M.Reset();
            StartCountDown(0, Cache);
            M.WaitOne(Cache * 1000, false);
            AbortCountDown();

            ASocket2.OnReceive -= new AsyncSocket.OnReceiveHandler(ReceiveSink2);

            if (NotifyTable.ContainsKey("upnp:rootdevice"))
            {
                AddEvent(LogImportance.Remark, "Notifications", "NOTIFY <<upnp:rootdevice>> OK");
            }
            else
            {
                NOTIFY = UPnPTestStates.Failed;
                AddEvent(LogImportance.Critical, "Notifications", "NOTIFY <<upnp:rootdevice>> MISSING/LATE");
            }
            ValidateNotifyTable(TestDevice);
            if (NOTIFY == UPnPTestStates.Pass)
            {
                Results.Add("Notifications OK");
            }
            else
            {
                Results.Add("One ore more NOTIFY packets were MISSING");
            }
            SetState("Notifications", NOTIFY);
        }

        private HTTPMessage[] MSEARCH(UPnPDevice device)
        {
            ArrayList PacketList = new ArrayList();
            foreach (UPnPDevice d in device.EmbeddedDevices)
            {
                foreach (HTTPMessage m in MSEARCH(d))
                {
                    PacketList.Add(m);
                }
            }

            HTTPMessage rq;
            rq = new HTTPMessage();
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "*";
            rq.AddTag("MX", "5");
            rq.AddTag("ST", "uuid:" + device.UniqueDeviceName);
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            PacketList.Add(rq);

            rq = new HTTPMessage();
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "*";
            rq.AddTag("MX", "5");
            rq.AddTag("ST", device.DeviceURN);
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            PacketList.Add(rq);

            foreach (UPnPService s in device.Services)
            {
                rq = new HTTPMessage();
                rq.Directive = "M-SEARCH";
                rq.DirectiveObj = "*";
                rq.AddTag("MX", "5");
                rq.AddTag("ST", s.ServiceURN);
                rq.AddTag("Host", "239.255.255.250:1900");
                rq.AddTag("MAN", "\"ssdp:discover\"");
                PacketList.Add(rq);
            }

            return ((HTTPMessage[])PacketList.ToArray(typeof(HTTPMessage)));
        }

        private UPnPService FetchAService(UPnPDevice d)
        {
            UPnPService RetVal = null;
            if (d.Services.Length != 0)
            {
                return (d.Services[0]);
            }
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                RetVal = FetchAService(ed);
                if (RetVal != null)
                {
                    return (RetVal);
                }
            }
            return (null);
        }
        private void Validate_DISCOVERY()
        {
            //Test all types of M-SEARCH, both valid and invalid
            MSEARCHTable.Clear();
            ASocket.OnReceive -= new AsyncSocket.OnReceiveHandler(ReceiveSink);
            ASocket.OnReceive += new AsyncSocket.OnReceiveHandler(MSEARCHSink);

            HTTPMessage rq = new HTTPMessage();
            byte[] rbuf;
            IPEndPoint d = new IPEndPoint(IPAddress.Parse("239.255.255.250"), 1900);
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "*";
            rq.AddTag("MX", "5");
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            rq.AddTag("ST", "ssdp:all");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(0, 91);
            MRE.WaitOne(8000, false);
            AbortCountDown();

            if (MSEARCHTable.ContainsKey("upnp:rootdevice"))
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<ssdp:all / upnp:rootdevice>> OK");
            }
            else
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.Critical, "Discovery", "MSEARCH <<ssdp:all / upnp:rootdevice>> MISSING");
            }

            foreach (HTTPMessage m in MSEARCH(TestDevice))
            {
                if (MSEARCHTable.ContainsKey(m.GetTag("ST").Trim()))
                {
                    AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<ssdp:all / " + m.GetTag("ST").Trim() + ">> OK");
                }
                else
                {
                    DISCOVERY = UPnPTestStates.Failed;
                    AddEvent(LogImportance.Critical, "Discovery", "MSEARCH <<ssdp:all / " + m.GetTag("ST").Trim() + ">> MISSING");
                }
            }


            // Test MSEARCH upnp:rootdevice, and others
            MSEARCHTable.Clear();
            rq.AddTag("ST", "upnp:rootdevice");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            foreach (HTTPMessage m in MSEARCH(TestDevice))
            {
                this.sample2 += "\r\n\r\n" + m.StringPacket;
                ASocket.Send(m.RawPacket, 0, m.RawPacket.Length, d);
            }
            MRE.Reset();
            StartCountDown(8, 91);
            MRE.WaitOne(8000, false);
            AbortCountDown();
            ASocket.OnReceive -= new AsyncSocket.OnReceiveHandler(MSEARCHSink);

            if (MSEARCHTable.ContainsKey("upnp:rootdevice"))
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<upnp:rootdevice>> OK");
            }
            else
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.Critical, "Discovery", "MSEARCH <<upnp:rootdevice>> MISSING");
            }

            foreach (HTTPMessage m in MSEARCH(TestDevice))
            {
                if (MSEARCHTable.ContainsKey(m.GetTag("ST").Trim()))
                {
                    AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<" + m.GetTag("ST").Trim() + ">> OK");
                }
                else
                {
                    DISCOVERY = UPnPTestStates.Failed;
                    AddEvent(LogImportance.Critical, "Discovery", "MSEARCH <<" + m.GetTag("ST").Trim() + ">> MISSING");
                }
            }

            // Test Invalid MSEARCHes
            string ST = "";
            MSEARCHTable.Clear();
            ASocket.OnReceive += new AsyncSocket.OnReceiveHandler(BadMSEARCHSink);
            rq = new HTTPMessage();
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "*";
            rq.AddTag("MX", "2");
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            rq.AddTag("ST", "uuid:___" + TestDevice.UniqueDeviceName + "___");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(16, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<NonExistent UDN>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<NonExistent UDN>> OK");
            }
            MSEARCHTable.Clear();

            ST = TestDevice.DeviceURN;
            int i = ST.LastIndexOf(":");
            if (i == -1)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "Can't parse DeviceURN");
                return;
            }
            ST = ST.Substring(0, i);
            ST = ST + ":" + ((int)(int.Parse(TestDevice.Version) + 5)).ToString();
            rq.AddTag("ST", ST);
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(21, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<Existing Device Type, Bad Version>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<Existing Device Type, Bad Version>> OK");
            }
            MSEARCHTable.Clear();

            UPnPService _S = FetchAService(TestDevice);
            ST = _S.ServiceURN;
            ST = ST.Substring(0, ST.LastIndexOf(":"));
            ST = ST + ":" + ((int)(int.Parse(_S.Version) + 5)).ToString();
            rq.AddTag("ST", ST);
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(26, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<Existing Service Type, Bad Version>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<Existing Service Type, Bad Version>> OK");
            }

            // Test MSEARCH No *
            MSEARCHTable.Clear();
            rq = new HTTPMessage();
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "";
            rq.AddTag("MX", "2");
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            rq.AddTag("ST", "upnp:rootdevice");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(31, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<No *>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<No *>> OK");
            }

            MSEARCHTable.Clear();
            rq.DirectiveObj = "/";
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(36, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<Not *>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<Not *>> OK");
            }

            MSEARCHTable.Clear();
            rq = new HTTPMessage();
            rq.Directive = "M-SEARCH";
            rq.DirectiveObj = "";
            rq.AddTag("MX", "2");
            rq.AddTag("Host", "239.255.255.250:1900");
            rq.AddTag("MAN", "\"ssdp:discover\"");
            rq.AddTag("ST", "upnp:rootdevice");
            rq.Version = "1.0";
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(41, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<Version = 1.0>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<Version = 1.0>> OK");
            }

            MSEARCHTable.Clear();
            rq.DirectiveObj = "*";
            rq.Version = "";
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(46, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<No Version>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<No Version>> OK");
            }

            MSEARCHTable.Clear();
            rq.Version = "1.1";
            rq.RemoveTag("MAN");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(51, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<No MAN>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<No MAN>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("MAN", "\"ssdp:discover\"");
            rq.RemoveTag("MX");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(56, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<No MX>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<No MX>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("MX", "");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(61, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<MX Empty>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<MX Empty>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("MX", "Z");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(66, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<MX Not Integer>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<MX Not Integer>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("MX", "-1");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(71, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<MX Negative>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<MX Negative>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("MX", "2");
            rq.RemoveTag("ST");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(76, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<No ST>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<No ST>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("ST", "");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(81, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<ST Empty>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<ST Empty>> OK");
            }

            MSEARCHTable.Clear();
            rq.AddTag("ST", "ABCDEFG");
            rbuf = rq.RawPacket;
            ASocket.Send(rbuf, 0, rbuf.Length, d);
            MRE.Reset();
            StartCountDown(86, 91);
            MRE.WaitOne(5000, false);
            AbortCountDown();
            if (MSEARCHTable.Count != 0)
            {
                DISCOVERY = UPnPTestStates.Failed;
                AddEvent(LogImportance.High, "Discovery", "MSEARCH <<ST Invalid>> Unexpected Response");
            }
            else
            {
                AddEvent(LogImportance.Remark, "Discovery", "MSEARCH <<ST Invalid>> OK");
            }

            SetState("Discovery", DISCOVERY);
            if (DISCOVERY == UPnPTestStates.Pass)
            {
                Results.Add("Discovery mechanism OK");
            }
            else
            {
                Results.Add("Discovery mechanism is not behaving correctly");
            }
        }

        private void MSEARCHSink(AsyncSocket sender, Byte[] buffer, int HeadPointer, int BufferSize, int BytesRead, IPEndPoint source, IPEndPoint remote)
        {
            HTTPMessage msg = HTTPMessage.ParseByteArray(buffer, HeadPointer, BufferSize);
            DText P = new DText();

            string USN = msg.GetTag("USN");
            string UDN;
            if (USN.IndexOf("::") != -1)
            {
                UDN = USN.Substring(0, USN.IndexOf("::"));
            }
            else
            {
                UDN = USN;
            }
            UDN = UDN.Substring(5);

            sender.BufferBeginPointer = BufferSize;
            if (TestDevice.GetDevice(UDN) == null)
            {
                return;
            }

            lock (MSEARCHTable)
            {
                this.sample += "\r\n" + msg.GetTag("ST");
                MSEARCHTable[msg.GetTag("ST").Trim()] = "";
            }
        }
        private void BadMSEARCHSink(AsyncSocket sender, Byte[] buffer, int HeadPointer, int BufferSize, int BytesRead, IPEndPoint source, IPEndPoint remote)
        {
            HTTPMessage msg = HTTPMessage.ParseByteArray(buffer, HeadPointer, BufferSize);
            if (remote.Address.ToString() == TestDevice.RemoteEndPoint.Address.ToString())
            {
                lock (MSEARCHTable)
                {
                    MSEARCHTable[remote.Address.ToString()] = msg;
                }
            }
        }

        private void ValidateNotifyTable(UPnPDevice d)
        {
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                ValidateNotifyTable(ed);
            }

            if (NotifyTable.ContainsKey(d.DeviceURN))
            {
                AddEvent(LogImportance.Remark, "Notifications", "NOTIFY <<" + d.DeviceURN + ">> OK");
            }
            else
            {
                NOTIFY = UPnPTestStates.Failed;
                AddEvent(LogImportance.Critical, "Notifications", "NOTIFY <<" + d.DeviceURN + ">> MISSING/LATE");
            }

            if (NotifyTable.ContainsKey("uuid:" + d.UniqueDeviceName))
            {
                AddEvent(LogImportance.Remark, "Notifications", "NOTIFY <<uuid:" + d.UniqueDeviceName + ">> OK");
            }
            else
            {
                NOTIFY = UPnPTestStates.Failed;
                AddEvent(LogImportance.Critical, "Notifications", "NOTIFY <<uuid:" + d.UniqueDeviceName + ">> MISSING/LATE");
            }

            foreach (UPnPService s in d.Services)
            {
                if (NotifyTable.ContainsKey(s.ServiceURN))
                {
                    AddEvent(LogImportance.Remark, "Notifications", "NOTIFY <<" + s.ServiceURN + ">> OK");
                }
                else
                {
                    NOTIFY = UPnPTestStates.Failed;
                    AddEvent(LogImportance.Critical, "Notifications", "NOTIFY <<" + s.ServiceURN + ">> MISSING/LATE");
                }
            }
        }

    }
}
