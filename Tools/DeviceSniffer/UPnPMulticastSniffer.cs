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
using System.Net.Sockets;
using OpenSource.UPnP;

namespace UPnPSniffer
{
    /// <summary>
    /// Summary description for UPnPMulticastSniffer.
    /// </summary>
    public class UPnPMulticastSniffer
    {
        public delegate void PacketHandler(object sender, string Packet, IPEndPoint Local, IPEndPoint From);
        public event PacketHandler OnPacket;
        private UdpClient client = null;

        public UPnPMulticastSniffer(IPEndPoint local)
        {
            client = new UdpClient(local.AddressFamily);
            try { client.ExclusiveAddressUse = false; } catch (SocketException) { }
            try { client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true); } catch (SocketException) { }
            try { client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ExclusiveAddressUse, false); } catch (SocketException) { }
            try { client.EnableBroadcast = true; } catch (SocketException) { }
            if (!Utils.IsMono())
            {
                client.Client.Bind(local);
            }
            else
            {
                if (local.AddressFamily == AddressFamily.InterNetwork) client.Client.Bind(new IPEndPoint(IPAddress.Any, local.Port));
                if (local.AddressFamily == AddressFamily.InterNetworkV6) client.Client.Bind(new IPEndPoint(IPAddress.IPv6Any, local.Port));
            }
            if (local.AddressFamily == AddressFamily.InterNetwork) client.JoinMulticastGroup(IPAddress.Parse("239.255.255.250"), local.Address);
            if (local.AddressFamily == AddressFamily.InterNetworkV6)
            {
                if (local.Address.IsIPv6LinkLocal) client.JoinMulticastGroup((int)local.Address.ScopeId, IPAddress.Parse("FF02::C"));
                else client.JoinMulticastGroup((int)local.Address.ScopeId, IPAddress.Parse("FF05::C"));
            }
            client.BeginReceive(new AsyncCallback(OnReceiveSink), local);
        }

        public void OnReceiveSink(IAsyncResult ar)
        {
            IPEndPoint ep = null;
            IPEndPoint local = (IPEndPoint)ar.AsyncState;
            byte[] buf = null;
            try { buf = client.EndReceive(ar, ref ep); } catch (Exception) { }
            if (buf != null && buf.Length > 0 && OnPacket != null)
            {
                if (!Utils.IsMono())
                {
                    if (client.Client.LocalEndPoint != null & ep != null) OnPacket(this, UTF8Encoding.UTF8.GetString(buf, 0, buf.Length), (IPEndPoint)client.Client.LocalEndPoint, ep);
                }
                else
                {
                    if (local != null && ep != null) OnPacket(this, UTF8Encoding.UTF8.GetString(buf, 0, buf.Length), local, ep);
                }
            }
            try { client.BeginReceive(new AsyncCallback(OnReceiveSink), local); }
            catch (Exception) { }
        }

        public void Dispose()
        {
            if (client == null) return;
            client.Close();
            client = null;
        }
    }
}
