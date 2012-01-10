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
using System.Text;
using System.Collections;
using System.Net.Sockets;
using OpenSource.UPnP;

namespace UPnPSniffer
{
    /// <summary>
    /// Summary description for UPnPSearchSniffer.
    /// </summary>
    public class UPnPSearchSniffer
    {
        public delegate void PacketHandler(object sender, string Packet, IPEndPoint Local, IPEndPoint From);
        public event PacketHandler OnPacket;
        protected Hashtable SSDPSessions = new Hashtable();

        public UPnPSearchSniffer()
        {
            IPAddress[] LocalAddresses = Dns.GetHostEntry(Dns.GetHostName()).AddressList;
            ArrayList temp = new ArrayList();
            foreach (IPAddress i in LocalAddresses) temp.Add(i);
            temp.Add(IPAddress.Loopback);
            LocalAddresses = (IPAddress[])temp.ToArray(typeof(IPAddress));

            for (int id = 0; id < LocalAddresses.Length; ++id)
            {
                try
                {
                    UdpClient ssdpSession = new UdpClient(new IPEndPoint(LocalAddresses[id], 0));
                    ssdpSession.EnableBroadcast = true;

                    if (!Utils.IsMono())
                    {
                        uint IOC_IN = 0x80000000;
                        uint IOC_VENDOR = 0x18000000;
                        uint SIO_UDP_CONNRESET = IOC_IN | IOC_VENDOR | 12;
                        ssdpSession.Client.IOControl((int)SIO_UDP_CONNRESET, new byte[] { Convert.ToByte(false) }, null);
                    }

                    ssdpSession.BeginReceive(new AsyncCallback(OnReceiveSink), ssdpSession);
                    SSDPSessions[ssdpSession] = ssdpSession;
                }
                catch (Exception) { }
            }
        }

        public void Search(string SearchString)
        {
            Search(SearchString, Utils.UpnpMulticastV4EndPoint);
            Search(SearchString, Utils.UpnpMulticastV6EndPoint1); // Site local
            Search(SearchString, Utils.UpnpMulticastV6EndPoint2); // Link local
        }

        public void SearchV4(string SearchString)
        {
            Search(SearchString, Utils.UpnpMulticastV4EndPoint);
        }

        public void SearchV6(string SearchString)
        {
            Search(SearchString, Utils.UpnpMulticastV6EndPoint1); // Site local
            Search(SearchString, Utils.UpnpMulticastV6EndPoint2); // Link local
        }

        public void Search(string SearchString, IPEndPoint ep)
        {
            HTTPMessage request = new HTTPMessage();
            request.Directive = "M-SEARCH";
            request.DirectiveObj = "*";
            if (ep.AddressFamily == AddressFamily.InterNetwork) request.AddTag("HOST", ep.ToString()); // "239.255.255.250:1900"
            if (ep.AddressFamily == AddressFamily.InterNetworkV6) request.AddTag("HOST", string.Format("[{0}]:{1}", ep.Address.ToString(), ep.Port)); // "[FF05::C]:1900" or "[FF02::C]:1900"
            request.AddTag("MAN", "\"ssdp:discover\"");
            request.AddTag("MX", "10");
            request.AddTag("ST", SearchString);
            SearchEx(System.Text.UTF8Encoding.UTF8.GetBytes(request.StringPacket), ep);
        }

        public void SearchEx(string text, IPEndPoint ep)
        {
            SearchEx(System.Text.UTF8Encoding.UTF8.GetBytes(text), ep);
        }

        public void SearchEx(byte[] buf, IPEndPoint ep)
        {
            foreach (UdpClient ssdpSession in SSDPSessions.Values)
            {
                try
                {
                    if (ssdpSession.Client.AddressFamily != ep.AddressFamily) continue;
                    if ((ssdpSession.Client.AddressFamily == AddressFamily.InterNetworkV6) && (((IPEndPoint)ssdpSession.Client.LocalEndPoint).Address.IsIPv6LinkLocal == true && ep.Address != Utils.UpnpMulticastV6Addr2)) continue;
                    if ((ssdpSession.Client.AddressFamily == AddressFamily.InterNetworkV6) && (((IPEndPoint)ssdpSession.Client.LocalEndPoint).Address.IsIPv6LinkLocal == false && ep.Address != Utils.UpnpMulticastV6Addr1)) continue;

                    IPEndPoint lep = (IPEndPoint)ssdpSession.Client.LocalEndPoint; // Seems can throw: System.Net.Sockets.SocketException: The requested address is not valid in its context
                    if (ssdpSession.Client.AddressFamily == AddressFamily.InterNetwork)
                    {
                        ssdpSession.Client.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastInterface, lep.Address.GetAddressBytes());
                    }
                    else if (ssdpSession.Client.AddressFamily == AddressFamily.InterNetworkV6)
                    {
                        ssdpSession.Client.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.MulticastInterface, BitConverter.GetBytes((int)lep.Address.ScopeId));
                    }

                    ssdpSession.Send(buf, buf.Length, ep);
                    ssdpSession.Send(buf, buf.Length, ep);
                }
                catch (SocketException) { }
            }
        }

        public void OnReceiveSink(IAsyncResult ar)
        {
            IPEndPoint ep = null;
            UdpClient client = (UdpClient)ar.AsyncState;
            byte[] buf = null;
            try
            {
                buf = client.EndReceive(ar, ref ep);
            }
            catch (Exception) { }
            try
            {
                if (buf != null && OnPacket != null) OnPacket(this, UTF8Encoding.UTF8.GetString(buf, 0, buf.Length), (IPEndPoint)client.Client.LocalEndPoint, ep);
            }
            catch (Exception) { }
            try
            {
                client.BeginReceive(new AsyncCallback(OnReceiveSink), client);
            }
            catch (Exception) { }
        }

    }
}
