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
using System.Data;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPSniffer
{
    /// <summary>
    /// Summary description for MainForm.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {
        //private UPnPMulticastSniffer Sniffer = null;
        //private UPnPMulticastSniffer Sniffer6 = null;
        private ArrayList Sniffers = new ArrayList();
        private UPnPSearchSniffer SSniffer;

        private int ReceivedPacketCount = 0;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.MenuItem exitMenuItem;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.ListView packetListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.MenuItem clearPacketsMenuItem;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button PauseReceiveButton;
        private System.Windows.Forms.Label infoLabel1;
        private System.Windows.Forms.Label infoLabel2;
        private System.Windows.Forms.ImageList packetImageList;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem ssdpSearchAllMenuItem;
        private System.Windows.Forms.MenuItem ssdpSearchRootsMenuItem;
        private System.Windows.Forms.MenuItem ssdpSearchSpecificMenuItem;
        private System.Windows.Forms.MenuItem ssdpSearchRenderer1MenuItem;
        private System.Windows.Forms.MenuItem ssdpSearchCDS1MenuItem;
        private System.Windows.Forms.MenuItem ssdpSearchIGD1MenuItem;
        private System.Windows.Forms.MenuItem debugInfoMenuItem;
        private System.Windows.Forms.ContextMenu packetListContextMenu;
        private System.Windows.Forms.ContextMenu packetContextMenu;
        private System.Windows.Forms.MenuItem clearPacketsContextMenuItem;
        private System.Windows.Forms.MenuItem captureModeMenuItem;
        private System.Windows.Forms.MenuItem copyPacketMenuItem;
        private System.Windows.Forms.TextBox packetDetailTextBox;
        private System.Windows.Forms.MenuItem packetMenuItemSeparator;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.ComponentModel.IContainer components;
        private System.Windows.Forms.MenuItem showHttpRequestorMenuItem;
        private System.Windows.Forms.MenuItem captureMenuItem;
        private System.Windows.Forms.MenuItem packetListMenuItemSeparator;
        private HttpRequestor httprequestor = new HttpRequestor();
        private DateTime notifyStatStart;
        private System.Windows.Forms.MenuItem menuItem6;
        private System.Windows.Forms.MenuItem menuItem7;
        private System.Windows.Forms.MenuItem useRequestorMenuItem;
        private System.Windows.Forms.MenuItem useBrowserMenuItem;
        private System.Windows.Forms.ContextMenu statContextMenu;
        private System.Windows.Forms.MenuItem menuItem8;
        private System.Windows.Forms.MenuItem showDeviceTrackingMenuItem;
        private System.Windows.Forms.Splitter splitter2;
        private System.Windows.Forms.MenuItem showPacketMenuItem;
        private System.Windows.Forms.ContextMenu deviceTrackingContextMenu;
        private System.Windows.Forms.MenuItem resetDeviceTrackingMenuItem;
        private System.Windows.Forms.ListView deviceListView;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader8;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.ColumnHeader columnHeader9;
        private System.Windows.Forms.MenuItem addressFilterMenuItem;
        private int notifyStatCount = 0;
        private System.Windows.Forms.MenuItem filterOnIpMenuItem;
        private System.Windows.Forms.MenuItem filterOnIp2MenuItem;
        private System.Windows.Forms.MenuItem helpMenuItem;
        private System.Windows.Forms.MenuItem menuItem10;
        private System.Windows.Forms.MenuItem wolMenuItem;
        private System.Windows.Forms.MenuItem menuItem11;
        private System.Windows.Forms.MenuItem SearchCustomTypeMenuItem;
        private System.Windows.Forms.MenuItem menuItem9;
        private System.Windows.Forms.MenuItem wsdProbeMenuItem;
        private MenuItem menuItem12;
        private MenuItem menuItem13;
        private MenuItem menuItem14;
        private MenuItem menuItem15;
        private MenuItem menuItem16;
        private IPAddress filteraddress = null;

        public MainForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
        }

        protected void SniffSink(object sender, string Packet, IPEndPoint Local, IPEndPoint From)
        {
            BeginInvoke(new UPnPSearchSniffer.PacketHandler(SniffSinkEx), new object[4] { sender, Packet, Local, From });
        }
        protected void SniffSinkEx(object sender, string Packet, IPEndPoint Local, IPEndPoint From)
        {
            //System.Console.WriteLine(Local.ToString() + " / " + From.ToString());

            if (From.AddressFamily == System.Net.Sockets.AddressFamily.InterNetworkV6 && menuItem14.Checked) return;
            if (From.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork && menuItem15.Checked) return;

            DText p = new DText();
            UTF8Encoding U = new UTF8Encoding();
            string PType = "Unknown";
            int i = Packet.IndexOf(" ");
            if (i > 0) PType = Packet.Substring(0, i);
            string NT = "Unknown";

            if (PType == "NOTIFY")
            {
                notifyStatCount++;
                TimeSpan delta = DateTime.Now.Subtract(notifyStatStart);
                statusBar.Text = "Received " + notifyStatCount + " notifications in " + ((int)delta.TotalMilliseconds) + " ms, averaging " + (((double)((int)((notifyStatCount / delta.TotalSeconds) * 100))) / 100) + " notify/sec.";

                int pos1 = Packet.IndexOf("\r\nNT:");
                if (pos1 == -1) pos1 = Packet.IndexOf("\r\nnt:");
                if (pos1 > 0)
                {
                    int pos2 = Packet.IndexOf("\r\n", pos1 + 5);
                    NT = Packet.Substring(pos1 + 5, pos2 - (pos1 + 5));
                }
                NT = NT.Trim();

                string USN = "";
                pos1 = Packet.IndexOf("\r\nUSN:");
                if (pos1 == -1) pos1 = Packet.IndexOf("\r\nusn:");
                if (pos1 > 0)
                {
                    int pos2 = Packet.IndexOf("\r\n", pos1 + 6);
                    USN = Packet.Substring(pos1 + 6, pos2 - (pos1 + 6));
                }
                USN = USN.Trim();
                int UsnEndPos = USN.IndexOf("::");
                if (USN.StartsWith("uuid:") == true && UsnEndPos != -1)
                {
                    string uuid = USN.Substring(5, UsnEndPos - 5);

                    bool found = false;
                    foreach (ListViewItem l in deviceListView.Items)
                    {
                        if (l.SubItems[4].Text.CompareTo(uuid) == 0)
                        {
                            l.SubItems[1].Text = ((int.Parse(l.SubItems[1].Text)) + 1).ToString();
                            l.SubItems[2].Text = DateTime.Now.ToShortTimeString();
                            found = true;
                            break;
                        }
                    }

                    if (found == false && NT.StartsWith("urn:schemas-upnp-org:device:") == true)
                    {
                        ListViewItem lvi = new ListViewItem(new string[] { From.Address.ToString(), "1", DateTime.Now.ToShortTimeString(), NT.Substring(28), uuid }, 1);
                        lvi.Tag = From.Address.ToString();
                        deviceListView.Items.Add(lvi);
                    }
                }
            }

            if (PType == "HTTP/1.1")
            {
                int pos1 = Packet.IndexOf("\r\nST:");
                if (pos1 == -1) pos1 = Packet.IndexOf("\r\nst:");
                if (pos1 > 0)
                {
                    int pos2 = Packet.IndexOf("\r\n", pos1 + 5);
                    NT = Packet.Substring(pos1 + 5, pos2 - (pos1 + 5));
                }
                NT = NT.Trim();

                string USN = "";
                pos1 = Packet.IndexOf("\r\nUSN:");
                if (pos1 == -1) pos1 = Packet.IndexOf("\r\nusn:");
                if (pos1 > 0)
                {
                    int pos2 = Packet.IndexOf("\r\n", pos1 + 6);
                    USN = Packet.Substring(pos1 + 6, pos2 - (pos1 + 6));
                }
                USN = USN.Trim();
                int UsnEndPos = USN.IndexOf("::");
                if (USN.StartsWith("uuid:") == true && UsnEndPos != -1)
                {
                    string uuid = USN.Substring(5, UsnEndPos - 5);

                    lock (deviceListView)
                    {
                        bool found = false;
                        foreach (ListViewItem l in deviceListView.Items)
                        {
                            if (l.SubItems[4].Text.CompareTo(uuid) == 0)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (found == false && NT.StartsWith("urn:schemas-upnp-org:device:") == true)
                        {
                            ListViewItem lvi = new ListViewItem(new string[] { From.Address.ToString(), "0", DateTime.Now.ToShortTimeString(), NT.Substring(28), uuid }, 1);
                            lvi.Tag = From.Address.ToString();
                            deviceListView.Items.Add(lvi);
                        }
                    }
                }
            }

            if (captureModeMenuItem.Checked == false && sender.GetType().FullName == "UPnPSniffer.UPnPMulticastSniffer")
            {
                return;
            }

            if (filteraddress != null && !filteraddress.Equals(From.Address)) return;

            lock (this.PauseReceiveButton)
            {
                if (PauseReceiveButton.Text == "Pause")
                {
                    bool OK = true;

                    /*
                    if (FilterMode.SelectedIndex == 0) OK = true;
                    if (FilterMode.SelectedIndex == 1)
                    {
                        // From IPAddress:Port
                        if(From.Address.ToString()==FilterBox.Text)
                        {
                            OK = true;
                        }
                    }
                    if (FilterMode.SelectedIndex == 2)
                    {
                        // Packet Type
                        HTTPMessage m = HTTPMessage.ParseByteArray(U.GetBytes(Packet));
                        if(m.Directive==FilterBox.Text) OK=true;
                    }
                    if (OK == true && HeaderBox.Text != "")
                    {
                        OK = false;
                        HTTPMessage mm = HTTPMessage.ParseByteArray(U.GetBytes(Packet));
                        string hd = mm.GetTag(HeaderBox.Text);
                        if(hd.ToUpper()==HeaderValueBox.Text.ToUpper()) OK = true;
                    }
                    */
                    if (OK == true)
                    {
                        ReceivedPacketCount++;
                        if (PType.StartsWith("HTTP"))
                        {
                            int pos1 = Packet.IndexOf("\r\nUSN:");
                            if (pos1 == -1) pos1 = Packet.IndexOf("\r\nusn:");
                            if (pos1 > 0)
                            {
                                int pos2 = Packet.IndexOf("\r\n", pos1 + 6);
                                NT = Packet.Substring(pos1 + 6, pos2 - (pos1 + 6));
                            }
                        }
                        if (PType.StartsWith("M-SEARCH"))
                        {
                            int pos1 = Packet.IndexOf("\r\nST:");
                            if (pos1 == -1) pos1 = Packet.IndexOf("\r\nst:");
                            if (pos1 > 0)
                            {
                                int pos2 = Packet.IndexOf("\r\n", pos1 + 5);
                                NT = Packet.Substring(pos1 + 5, pos2 - (pos1 + 5));
                            }
                        }
                        if (PType == "NOTIFY")
                        {
                            int pos1 = Packet.IndexOf("ssdp:byebye\r\n");
                            if (pos1 > 0)
                            {
                                PType = "NOTIFY/BYE";
                            }
                        }
                        NT = NT.Trim();
                        infoLabel2.Text = ReceivedPacketCount + " Packets Captured";
                        DateTime present = DateTime.Now;
                        string fromstr = From.ToString();
                        if (From.AddressFamily == System.Net.Sockets.AddressFamily.InterNetworkV6) fromstr = string.Format("[{0}]:{1}", From.Address.ToString(), From.Port);
                        ListViewItem lvi = new ListViewItem(new string[] { present.ToLongTimeString() + " (" + present.Millisecond.ToString() + ")", fromstr, PType, NT }, 0);
                        lvi.Tag = "Received " + present.ToShortDateString() + " at " + present.ToLongTimeString() + " (" + present.Millisecond + ")\r\n\r\n" + Packet;
                        packetListView.Items.Insert(0, lvi);
                        while (packetListView.Items.Count > 500) packetListView.Items.RemoveAt(500);
                    }
                }
            }
        }
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.packetDetailTextBox = new System.Windows.Forms.TextBox();
            this.packetContextMenu = new System.Windows.Forms.ContextMenu();
            this.copyPacketMenuItem = new System.Windows.Forms.MenuItem();
            this.packetMenuItemSeparator = new System.Windows.Forms.MenuItem();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.clearPacketsMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.showPacketMenuItem = new System.Windows.Forms.MenuItem();
            this.showDeviceTrackingMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.ssdpSearchAllMenuItem = new System.Windows.Forms.MenuItem();
            this.ssdpSearchRootsMenuItem = new System.Windows.Forms.MenuItem();
            this.ssdpSearchSpecificMenuItem = new System.Windows.Forms.MenuItem();
            this.ssdpSearchRenderer1MenuItem = new System.Windows.Forms.MenuItem();
            this.ssdpSearchCDS1MenuItem = new System.Windows.Forms.MenuItem();
            this.ssdpSearchIGD1MenuItem = new System.Windows.Forms.MenuItem();
            this.SearchCustomTypeMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.wsdProbeMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.captureMenuItem = new System.Windows.Forms.MenuItem();
            this.captureModeMenuItem = new System.Windows.Forms.MenuItem();
            this.addressFilterMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem12 = new System.Windows.Forms.MenuItem();
            this.menuItem13 = new System.Windows.Forms.MenuItem();
            this.menuItem14 = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.showHttpRequestorMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.useRequestorMenuItem = new System.Windows.Forms.MenuItem();
            this.useBrowserMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.debugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.packetListView = new System.Windows.Forms.ListView();
            this.columnHeader9 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.packetListContextMenu = new System.Windows.Forms.ContextMenu();
            this.clearPacketsContextMenuItem = new System.Windows.Forms.MenuItem();
            this.filterOnIp2MenuItem = new System.Windows.Forms.MenuItem();
            this.packetListMenuItemSeparator = new System.Windows.Forms.MenuItem();
            this.packetImageList = new System.Windows.Forms.ImageList(this.components);
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.statContextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.panel1 = new System.Windows.Forms.Panel();
            this.infoLabel2 = new System.Windows.Forms.Label();
            this.infoLabel1 = new System.Windows.Forms.Label();
            this.PauseReceiveButton = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.splitter2 = new System.Windows.Forms.Splitter();
            this.deviceTrackingContextMenu = new System.Windows.Forms.ContextMenu();
            this.filterOnIpMenuItem = new System.Windows.Forms.MenuItem();
            this.wolMenuItem = new System.Windows.Forms.MenuItem();
            this.resetDeviceTrackingMenuItem = new System.Windows.Forms.MenuItem();
            this.deviceListView = new System.Windows.Forms.ListView();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader8 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // packetDetailTextBox
            // 
            this.packetDetailTextBox.ContextMenu = this.packetContextMenu;
            this.packetDetailTextBox.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.packetDetailTextBox.Location = new System.Drawing.Point(0, 310);
            this.packetDetailTextBox.Multiline = true;
            this.packetDetailTextBox.Name = "packetDetailTextBox";
            this.packetDetailTextBox.ReadOnly = true;
            this.packetDetailTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.packetDetailTextBox.Size = new System.Drawing.Size(771, 176);
            this.packetDetailTextBox.TabIndex = 0;
            // 
            // packetContextMenu
            // 
            this.packetContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.copyPacketMenuItem,
            this.packetMenuItemSeparator});
            this.packetContextMenu.Popup += new System.EventHandler(this.packetContextMenu_Popup);
            // 
            // copyPacketMenuItem
            // 
            this.copyPacketMenuItem.Index = 0;
            this.copyPacketMenuItem.Text = "&Copy Packet to Clipboard";
            this.copyPacketMenuItem.Click += new System.EventHandler(this.copyPacketMenuItem_Click);
            // 
            // packetMenuItemSeparator
            // 
            this.packetMenuItemSeparator.Index = 1;
            this.packetMenuItemSeparator.Text = "-";
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem3,
            this.menuItem4,
            this.menuItem6,
            this.menuItem2});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.clearPacketsMenuItem,
            this.menuItem5,
            this.showPacketMenuItem,
            this.showDeviceTrackingMenuItem,
            this.menuItem11,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            // 
            // clearPacketsMenuItem
            // 
            this.clearPacketsMenuItem.Index = 0;
            this.clearPacketsMenuItem.Text = "&Clear Packet Capture";
            this.clearPacketsMenuItem.Click += new System.EventHandler(this.clearPacketsMenuItem_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 1;
            this.menuItem5.Text = "-";
            // 
            // showPacketMenuItem
            // 
            this.showPacketMenuItem.Checked = true;
            this.showPacketMenuItem.Index = 2;
            this.showPacketMenuItem.Text = "Show &Packet Details";
            this.showPacketMenuItem.Click += new System.EventHandler(this.showPacketMenuItem_Click);
            // 
            // showDeviceTrackingMenuItem
            // 
            this.showDeviceTrackingMenuItem.Index = 3;
            this.showDeviceTrackingMenuItem.Text = "Show &Device Tracking";
            this.showDeviceTrackingMenuItem.Click += new System.EventHandler(this.showDeviceTrackingMenuItem_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 4;
            this.menuItem11.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 5;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 1;
            this.menuItem3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.ssdpSearchAllMenuItem,
            this.ssdpSearchRootsMenuItem,
            this.ssdpSearchSpecificMenuItem,
            this.SearchCustomTypeMenuItem,
            this.menuItem9,
            this.wsdProbeMenuItem});
            this.menuItem3.Text = "&Search";
            // 
            // ssdpSearchAllMenuItem
            // 
            this.ssdpSearchAllMenuItem.Index = 0;
            this.ssdpSearchAllMenuItem.Shortcut = System.Windows.Forms.Shortcut.F5;
            this.ssdpSearchAllMenuItem.Text = "Search &All Devices";
            this.ssdpSearchAllMenuItem.Click += new System.EventHandler(this.ssdpSearchAllMenuItem_Click);
            // 
            // ssdpSearchRootsMenuItem
            // 
            this.ssdpSearchRootsMenuItem.Index = 1;
            this.ssdpSearchRootsMenuItem.Shortcut = System.Windows.Forms.Shortcut.F6;
            this.ssdpSearchRootsMenuItem.Text = "Search &Root Devices";
            this.ssdpSearchRootsMenuItem.Click += new System.EventHandler(this.ssdpSearchRootsMenuItem_Click);
            // 
            // ssdpSearchSpecificMenuItem
            // 
            this.ssdpSearchSpecificMenuItem.Index = 2;
            this.ssdpSearchSpecificMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.ssdpSearchRenderer1MenuItem,
            this.ssdpSearchCDS1MenuItem,
            this.ssdpSearchIGD1MenuItem});
            this.ssdpSearchSpecificMenuItem.Text = "Search &Specific Type";
            // 
            // ssdpSearchRenderer1MenuItem
            // 
            this.ssdpSearchRenderer1MenuItem.Index = 0;
            this.ssdpSearchRenderer1MenuItem.Shortcut = System.Windows.Forms.Shortcut.F7;
            this.ssdpSearchRenderer1MenuItem.Text = "AV &Renderers 1.0";
            this.ssdpSearchRenderer1MenuItem.Click += new System.EventHandler(this.ssdpSearchRenderer1MenuItem_Click);
            // 
            // ssdpSearchCDS1MenuItem
            // 
            this.ssdpSearchCDS1MenuItem.Index = 1;
            this.ssdpSearchCDS1MenuItem.Shortcut = System.Windows.Forms.Shortcut.F8;
            this.ssdpSearchCDS1MenuItem.Text = "AV &Media Servers 1.0";
            this.ssdpSearchCDS1MenuItem.Click += new System.EventHandler(this.ssdpSearchCDS1MenuItem_Click);
            // 
            // ssdpSearchIGD1MenuItem
            // 
            this.ssdpSearchIGD1MenuItem.Index = 2;
            this.ssdpSearchIGD1MenuItem.Shortcut = System.Windows.Forms.Shortcut.F9;
            this.ssdpSearchIGD1MenuItem.Text = "Internet &Gateways 1.0";
            this.ssdpSearchIGD1MenuItem.Click += new System.EventHandler(this.ssdpSearchIGD1MenuItem_Click);
            // 
            // SearchCustomTypeMenuItem
            // 
            this.SearchCustomTypeMenuItem.Index = 3;
            this.SearchCustomTypeMenuItem.Text = "Search Custom Type";
            this.SearchCustomTypeMenuItem.Click += new System.EventHandler(this.SearchCustomTypeMenuItem_Click);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 4;
            this.menuItem9.Text = "-";
            // 
            // wsdProbeMenuItem
            // 
            this.wsdProbeMenuItem.Index = 5;
            this.wsdProbeMenuItem.Text = "WSD Probe wsdp:Device";
            this.wsdProbeMenuItem.Click += new System.EventHandler(this.wsdProbeMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.captureMenuItem,
            this.captureModeMenuItem,
            this.addressFilterMenuItem,
            this.menuItem12});
            this.menuItem4.Text = "&Filter";
            // 
            // captureMenuItem
            // 
            this.captureMenuItem.Checked = true;
            this.captureMenuItem.Index = 0;
            this.captureMenuItem.Shortcut = System.Windows.Forms.Shortcut.F2;
            this.captureMenuItem.Text = "&Enable Packet Capture";
            this.captureMenuItem.Click += new System.EventHandler(this.PauseReceiveButton_Click);
            // 
            // captureModeMenuItem
            // 
            this.captureModeMenuItem.Checked = true;
            this.captureModeMenuItem.Index = 1;
            this.captureModeMenuItem.Shortcut = System.Windows.Forms.Shortcut.F3;
            this.captureModeMenuItem.Text = "&Capture Multicasts";
            this.captureModeMenuItem.Click += new System.EventHandler(this.captureModeMenuItem_Click);
            // 
            // addressFilterMenuItem
            // 
            this.addressFilterMenuItem.Index = 2;
            this.addressFilterMenuItem.Text = "&Address Filter...";
            this.addressFilterMenuItem.Click += new System.EventHandler(this.addressFilterMenuItem_Click);
            // 
            // menuItem12
            // 
            this.menuItem12.Index = 3;
            this.menuItem12.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem13,
            this.menuItem14,
            this.menuItem15});
            this.menuItem12.Text = "Network Filter";
            // 
            // menuItem13
            // 
            this.menuItem13.Checked = true;
            this.menuItem13.Index = 0;
            this.menuItem13.RadioCheck = true;
            this.menuItem13.Text = "IPv4 && IPv6";
            this.menuItem13.Click += new System.EventHandler(this.menuItem13_Click);
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 1;
            this.menuItem14.RadioCheck = true;
            this.menuItem14.Text = "IPv4 only";
            this.menuItem14.Click += new System.EventHandler(this.menuItem14_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 2;
            this.menuItem15.RadioCheck = true;
            this.menuItem15.Text = "IPv6 only";
            this.menuItem15.Click += new System.EventHandler(this.menuItem15_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 3;
            this.menuItem6.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.showHttpRequestorMenuItem,
            this.menuItem7,
            this.useRequestorMenuItem,
            this.useBrowserMenuItem});
            this.menuItem6.Text = "Http";
            // 
            // showHttpRequestorMenuItem
            // 
            this.showHttpRequestorMenuItem.Index = 0;
            this.showHttpRequestorMenuItem.Text = "&Show HTTP Requestor";
            this.showHttpRequestorMenuItem.Click += new System.EventHandler(this.showHttpRequestorMenuItem_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 1;
            this.menuItem7.Text = "-";
            // 
            // useRequestorMenuItem
            // 
            this.useRequestorMenuItem.Index = 2;
            this.useRequestorMenuItem.RadioCheck = true;
            this.useRequestorMenuItem.Text = "Use &Requestor for HTTP Requests";
            this.useRequestorMenuItem.Click += new System.EventHandler(this.useRequestorMenuItem_Click);
            // 
            // useBrowserMenuItem
            // 
            this.useBrowserMenuItem.Checked = true;
            this.useBrowserMenuItem.Index = 3;
            this.useBrowserMenuItem.RadioCheck = true;
            this.useBrowserMenuItem.Text = "Use &Browser for HTTP Requests";
            this.useBrowserMenuItem.Click += new System.EventHandler(this.useBrowserMenuItem_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 4;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem16,
            this.menuItem10,
            this.debugInfoMenuItem});
            this.menuItem2.Text = "&Help";
            this.menuItem2.Popup += new System.EventHandler(this.menuItem2_Popup);
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            this.helpMenuItem.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem.Text = "&Help Topics";
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 2;
            this.menuItem10.Text = "-";
            // 
            // debugInfoMenuItem
            // 
            this.debugInfoMenuItem.Index = 3;
            this.debugInfoMenuItem.Text = "&Show Debug Information";
            this.debugInfoMenuItem.Click += new System.EventHandler(this.debugInfoMenuItem_Click);
            // 
            // splitter1
            // 
            this.splitter1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.splitter1.Location = new System.Drawing.Point(0, 307);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(771, 3);
            this.splitter1.TabIndex = 13;
            this.splitter1.TabStop = false;
            // 
            // packetListView
            // 
            this.packetListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader9,
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3});
            this.packetListView.ContextMenu = this.packetListContextMenu;
            this.packetListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.packetListView.FullRowSelect = true;
            this.packetListView.Location = new System.Drawing.Point(0, 40);
            this.packetListView.MultiSelect = false;
            this.packetListView.Name = "packetListView";
            this.packetListView.Size = new System.Drawing.Size(771, 267);
            this.packetListView.SmallImageList = this.packetImageList;
            this.packetListView.TabIndex = 14;
            this.packetListView.UseCompatibleStateImageBehavior = false;
            this.packetListView.View = System.Windows.Forms.View.Details;
            this.packetListView.SelectedIndexChanged += new System.EventHandler(this.packetListView_SelectedIndexChanged);
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "Time";
            this.columnHeader9.Width = 120;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Source Address";
            this.columnHeader1.Width = 200;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Packet Type";
            this.columnHeader2.Width = 80;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Packet Information";
            this.columnHeader3.Width = 345;
            // 
            // packetListContextMenu
            // 
            this.packetListContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.clearPacketsContextMenuItem,
            this.filterOnIp2MenuItem,
            this.packetListMenuItemSeparator});
            this.packetListContextMenu.Popup += new System.EventHandler(this.packetListContextMenu_Popup);
            // 
            // clearPacketsContextMenuItem
            // 
            this.clearPacketsContextMenuItem.Index = 0;
            this.clearPacketsContextMenuItem.Text = "&Clear Packet Capture";
            this.clearPacketsContextMenuItem.Click += new System.EventHandler(this.clearPacketsMenuItem_Click);
            // 
            // filterOnIp2MenuItem
            // 
            this.filterOnIp2MenuItem.Index = 1;
            this.filterOnIp2MenuItem.Text = "&Clear Filter";
            this.filterOnIp2MenuItem.Click += new System.EventHandler(this.filterOnIp2MenuItem_Click);
            // 
            // packetListMenuItemSeparator
            // 
            this.packetListMenuItemSeparator.Index = 2;
            this.packetListMenuItemSeparator.Text = "-";
            // 
            // packetImageList
            // 
            this.packetImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("packetImageList.ImageStream")));
            this.packetImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.packetImageList.Images.SetKeyName(0, "");
            this.packetImageList.Images.SetKeyName(1, "");
            // 
            // statusBar
            // 
            this.statusBar.ContextMenu = this.statContextMenu;
            this.statusBar.Location = new System.Drawing.Point(0, 617);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(771, 16);
            this.statusBar.TabIndex = 15;
            // 
            // statContextMenu
            // 
            this.statContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem8});
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 0;
            this.menuItem8.Text = "&Reset";
            this.menuItem8.Click += new System.EventHandler(this.menuItem8_Click);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.infoLabel2);
            this.panel1.Controls.Add(this.infoLabel1);
            this.panel1.Controls.Add(this.PauseReceiveButton);
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(771, 40);
            this.panel1.TabIndex = 17;
            // 
            // infoLabel2
            // 
            this.infoLabel2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.infoLabel2.Location = new System.Drawing.Point(48, 21);
            this.infoLabel2.Name = "infoLabel2";
            this.infoLabel2.Size = new System.Drawing.Size(641, 16);
            this.infoLabel2.TabIndex = 23;
            this.infoLabel2.Text = "0 Captured Packets";
            this.infoLabel2.DoubleClick += new System.EventHandler(this.clearPacketsMenuItem_Click);
            // 
            // infoLabel1
            // 
            this.infoLabel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.infoLabel1.Location = new System.Drawing.Point(48, 5);
            this.infoLabel1.Name = "infoLabel1";
            this.infoLabel1.Size = new System.Drawing.Size(641, 16);
            this.infoLabel1.TabIndex = 22;
            this.infoLabel1.Text = "Multicast && Unicast Reception Mode";
            this.infoLabel1.DoubleClick += new System.EventHandler(this.captureModeMenuItem_Click);
            // 
            // PauseReceiveButton
            // 
            this.PauseReceiveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.PauseReceiveButton.Location = new System.Drawing.Point(692, 4);
            this.PauseReceiveButton.Name = "PauseReceiveButton";
            this.PauseReceiveButton.Size = new System.Drawing.Size(75, 32);
            this.PauseReceiveButton.TabIndex = 21;
            this.PauseReceiveButton.Text = "Pause";
            this.PauseReceiveButton.Click += new System.EventHandler(this.PauseReceiveButton_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(8, 8);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(32, 32);
            this.pictureBox1.TabIndex = 20;
            this.pictureBox1.TabStop = false;
            // 
            // splitter2
            // 
            this.splitter2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.splitter2.Location = new System.Drawing.Point(0, 486);
            this.splitter2.Name = "splitter2";
            this.splitter2.Size = new System.Drawing.Size(771, 3);
            this.splitter2.TabIndex = 19;
            this.splitter2.TabStop = false;
            this.splitter2.Visible = false;
            // 
            // filterOnIpMenuItem
            // 
            this.filterOnIpMenuItem.Index = -1;
            this.filterOnIpMenuItem.Text = "&Clear Filter";
            this.filterOnIpMenuItem.Click += new System.EventHandler(this.filterOnIpMenuItem_Click);
            // 
            // wolMenuItem
            // 
            this.wolMenuItem.Index = -1;
            this.wolMenuItem.Text = "&Wake-up";
            // 
            // resetDeviceTrackingMenuItem
            // 
            this.resetDeviceTrackingMenuItem.Index = -1;
            this.resetDeviceTrackingMenuItem.Text = "&Reset Device Tracking";
            this.resetDeviceTrackingMenuItem.Click += new System.EventHandler(this.resetDeviceTrackingMenuItem_Click);
            // 
            // deviceListView
            // 
            this.deviceListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4,
            this.columnHeader8,
            this.columnHeader5,
            this.columnHeader6,
            this.columnHeader7});
            this.deviceListView.ContextMenu = this.deviceTrackingContextMenu;
            this.deviceListView.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.deviceListView.FullRowSelect = true;
            this.deviceListView.Location = new System.Drawing.Point(0, 489);
            this.deviceListView.Name = "deviceListView";
            this.deviceListView.Size = new System.Drawing.Size(771, 128);
            this.deviceListView.SmallImageList = this.packetImageList;
            this.deviceListView.TabIndex = 18;
            this.deviceListView.UseCompatibleStateImageBehavior = false;
            this.deviceListView.View = System.Windows.Forms.View.Details;
            this.deviceListView.Visible = false;
            this.deviceListView.SelectedIndexChanged += new System.EventHandler(this.deviceListView_SelectedIndexChanged);
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Address";
            this.columnHeader4.Width = 110;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "Notify";
            this.columnHeader8.Width = 40;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Last Notify";
            this.columnHeader5.Width = 70;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Device Type";
            this.columnHeader6.Width = 160;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Unique Identifier";
            this.columnHeader7.Width = 215;
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 1;
            this.menuItem16.Text = "&Check for updates";
            this.menuItem16.Click += new System.EventHandler(this.menuItem16_Click);
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(771, 633);
            this.Controls.Add(this.packetListView);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.packetDetailTextBox);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.splitter2);
            this.Controls.Add(this.deviceListView);
            this.Controls.Add(this.statusBar);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "Device Sniffer";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.Run(new MainForm());
        }

        private void PauseReceiveButton_Click(object sender, System.EventArgs e)
        {
            lock (PauseReceiveButton)
            {
                if (PauseReceiveButton.Text == "Receive")
                {
                    PauseReceiveButton.Text = "Pause";
                    captureMenuItem.Checked = true;
                }
                else
                {
                    PauseReceiveButton.Text = "Receive";
                    captureMenuItem.Checked = false;
                }
            }

            UpdateStatusLine1();
        }

        private void UpdateStatusLine1()
        {
            if (PauseReceiveButton.Text != "Pause")
            {
                infoLabel1.Text = "Packet Capture Paused";
            }
            else
            {
                if (captureModeMenuItem.Checked)
                {
                    infoLabel1.Text = "Multicast && Unicast Reception Mode";
                }
                else
                {
                    infoLabel1.Text = "Unicast Only Reception Mode";
                }
            }
            if (this.filteraddress != null)
            {
                infoLabel1.Text += ", Filtering on " + filteraddress.ToString();
            }
            if (menuItem14.Checked) infoLabel1.Text += ", IPv4 only";
            if (menuItem15.Checked) infoLabel1.Text += ", IPv6 only";
        }

        private void exitMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void clearPacketsMenuItem_Click(object sender, System.EventArgs e)
        {
            ReceivedPacketCount = 0;
            infoLabel2.Text = ReceivedPacketCount + " Packets Captured";
            packetDetailTextBox.Clear();
            packetListView.Items.Clear();
        }

        private void packetListView_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            if (packetListView.SelectedItems.Count == 1)
            {
                packetDetailTextBox.Text = (string)packetListView.SelectedItems[0].Tag;
            }
        }

        private void ssdpSearchAllMenuItem_Click(object sender, System.EventArgs e)
        {
            SSniffer.Search("ssdp:all");
        }

        private void ssdpSearchRootsMenuItem_Click(object sender, System.EventArgs e)
        {
            SSniffer.Search("upnp:rootdevice");
        }

        private void ssdpSearchIGD1MenuItem_Click(object sender, System.EventArgs e)
        {
            SSniffer.Search("urn:schemas-upnp-org:device:InternetGatewayDevice:1");
        }

        private void ssdpSearchCDS1MenuItem_Click(object sender, System.EventArgs e)
        {
            SSniffer.Search("urn:schemas-upnp-org:device:MediaServer:1");
        }

        private void ssdpSearchRenderer1MenuItem_Click(object sender, System.EventArgs e)
        {
            SSniffer.Search("urn:schemas-upnp-org:device:MediaRenderer:1");
        }

        private void debugInfoMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void captureModeMenuItem_Click(object sender, System.EventArgs e)
        {
            captureModeMenuItem.Checked = !captureModeMenuItem.Checked;
            UpdateStatusLine1();
        }

        private void copyPacketMenuItem_Click(object sender, System.EventArgs e)
        {
            Clipboard.SetDataObject(packetDetailTextBox.Text);
        }

        private void WebPageOpenMenuSink(object sender, System.EventArgs arg)
        {
            MenuItem mi = (MenuItem)sender;
            if (useRequestorMenuItem.Checked == false)
            {
                try
                {
                    System.Diagnostics.Process.Start(mi.Text.Substring(5));
                }
                catch (Exception) { }
            }
            else
            {
                httprequestor.Request(mi.Text.Substring(5));
            }
        }

        private void packetContextMenu_Popup(object sender, System.EventArgs e)
        {
            while (packetContextMenu.MenuItems.Count > 2)
            {
                packetContextMenu.MenuItems.RemoveAt(2);
            }
            packetMenuItemSeparator.Visible = false;

            int pos = 0;
            pos = packetDetailTextBox.Text.IndexOf("http://", pos);
            while (pos != -1)
            {
                int posfin = 0;
                int posfinX = 0;
                posfin = packetDetailTextBox.Text.Length;

                posfinX = packetDetailTextBox.Text.IndexOf("\n", pos);
                if (posfinX != -1 && posfinX < posfin) posfin = posfinX;

                posfinX = packetDetailTextBox.Text.IndexOf("\r", pos);
                if (posfinX != -1 && posfinX < posfin) posfin = posfinX;

                posfinX = packetDetailTextBox.Text.IndexOf("<", pos);
                if (posfinX != -1 && posfinX < posfin) posfin = posfinX;

                posfinX = packetDetailTextBox.Text.IndexOf("\"", pos);
                if (posfinX != -1 && posfinX < posfin) posfin = posfinX;
                
                MenuItem mi = new MenuItem("Open " + packetDetailTextBox.Text.Substring(pos, posfin - pos), new EventHandler(WebPageOpenMenuSink));
                packetContextMenu.MenuItems.Add(mi);
                packetMenuItemSeparator.Visible = true;
                pos = packetDetailTextBox.Text.IndexOf("http://", posfin);
            }
        }

        private void packetListContextMenu_Popup(object sender, System.EventArgs e)
        {
            while (packetListContextMenu.MenuItems.Count > 3)
            {
                packetListContextMenu.MenuItems.RemoveAt(3);
            }
            packetListMenuItemSeparator.Visible = false;

            if (packetListView.SelectedItems.Count == 1)
            {
                packetDetailTextBox.Text = (string)packetListView.SelectedItems[0].Tag;

                int pos = 0;
                pos = packetDetailTextBox.Text.IndexOf("http://", pos);
                while (pos != -1)
                {
                    int posfin1 = packetDetailTextBox.Text.IndexOf("\r\n", pos);
                    int posfin2 = packetDetailTextBox.Text.IndexOf("<", pos);
                    int posfin = -1;
                    if (posfin1 == -1) posfin = posfin2; else if (posfin2 == -1) posfin = posfin1; else posfin = Math.Min(posfin1, posfin2);
                    MenuItem mi = new MenuItem("Open " + packetDetailTextBox.Text.Substring(pos, posfin - pos), new EventHandler(WebPageOpenMenuSink));
                    packetListContextMenu.MenuItems.Add(mi);
                    packetListMenuItemSeparator.Visible = true;
                    pos = packetDetailTextBox.Text.IndexOf("http://", posfin);
                }

                string addr = packetListView.SelectedItems[0].SubItems[1].Text.Substring(0, packetListView.SelectedItems[0].SubItems[1].Text.LastIndexOf(":"));
                filterOnIp2MenuItem.Text = "&Filter on " + addr;
            }
            else
            {
                filterOnIp2MenuItem.Text = "&Clear Filter";
            }
        }

        private void showHttpRequestorMenuItem_Click(object sender, System.EventArgs e)
        {
            if (httprequestor.Visible == true)
            {
                httprequestor.Activate();
            }
            else
            {
                httprequestor.Show();
            }
        }

        private void useRequestorMenuItem_Click(object sender, System.EventArgs e)
        {
            useRequestorMenuItem.Checked = true;
            useBrowserMenuItem.Checked = false;
        }

        private void useBrowserMenuItem_Click(object sender, System.EventArgs e)
        {
            useRequestorMenuItem.Checked = false;
            useBrowserMenuItem.Checked = true;
        }

        private void menuItem8_Click(object sender, System.EventArgs e)
        {
            notifyStatStart = DateTime.Now;
            notifyStatCount = 0;
            statusBar.Text = "";
        }

        private void showDeviceTrackingMenuItem_Click(object sender, System.EventArgs e)
        {
            showDeviceTrackingMenuItem.Checked = !showDeviceTrackingMenuItem.Checked;
            splitter2.Visible = showDeviceTrackingMenuItem.Checked;
            deviceListView.Visible = showDeviceTrackingMenuItem.Checked;
        }

        private void showPacketMenuItem_Click(object sender, System.EventArgs e)
        {
            showPacketMenuItem.Checked = !showPacketMenuItem.Checked;
            splitter1.Visible = showPacketMenuItem.Checked;
            packetDetailTextBox.Visible = showPacketMenuItem.Checked;
        }

        private void resetDeviceTrackingMenuItem_Click(object sender, System.EventArgs e)
        {
            deviceListView.Items.Clear();
        }

        private void addressFilterMenuItem_Click(object sender, System.EventArgs e)
        {
            AddressFilterForm form = new AddressFilterForm();
            form.FilterAddress = filteraddress;
            if (form.ShowDialog(this) == DialogResult.OK)
            {
                filteraddress = form.FilterAddress;
            }
            UpdateStatusLine1();
        }

        private void filterOnIpMenuItem_Click(object sender, System.EventArgs e)
        {
            if (filterOnIpMenuItem.Text == "&Clear Filter")
            {
                filteraddress = null;
            }
            else
            {
                filteraddress = IPAddress.Parse(filterOnIpMenuItem.Text.Substring(11));
            }
            UpdateStatusLine1();
        }

        private void filterOnIp2MenuItem_Click(object sender, System.EventArgs e)
        {
            if (filterOnIp2MenuItem.Text == "&Clear Filter")
            {
                filteraddress = null;
            }
            else
            {
                filteraddress = IPAddress.Parse(filterOnIp2MenuItem.Text.Substring(11));
            }
            UpdateStatusLine1();
        }

        private void deviceListView_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            if (deviceListView.SelectedItems.Count == 1)
            {
                filterOnIpMenuItem.Text = "&Filter on " + deviceListView.SelectedItems[0].Text;
            }
            else
            {
                filterOnIpMenuItem.Text = "&Clear Filter";
            }
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Sniffer");
        }

        private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Sniffer");
        }

        private void SearchCustomTypeMenuItem_Click(object sender, System.EventArgs e)
        {
            CustomSearch cs = new CustomSearch();
            if (cs.ShowDialog() == DialogResult.OK)
            {
                if (cs.UnicastAddress.Text != null && cs.UnicastAddress.Text.Length > 0)
                {
                    SSniffer.Search(cs.SearchTaget, new IPEndPoint(IPAddress.Parse(cs.UnicastAddress.Text), 1900));
                }
                else
                {
                    SSniffer.Search(cs.SearchTaget);
                }
            }
            cs.Dispose();
        }

        private void wsdProbeMenuItem_Click(object sender, System.EventArgs e)
        {
            string g = Guid.NewGuid().ToString();
            string packet = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:wsdp=\"http://schemas.xmlsoap.org/ws/2006/02/devprof\"><soap:Header><wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action><wsa:MessageID>urn:uuid:" + g + "</wsa:MessageID></soap:Header><soap:Body><wsd:Probe><wsd:Types>wsdp:Device</wsd:Types></wsd:Probe></soap:Body></soap:Envelope>";

            SSniffer.SearchEx(packet, new IPEndPoint(IPAddress.Parse("239.255.255.250"), 3702));
            SSniffer.SearchEx(packet, new IPEndPoint(IPAddress.Parse("FF05::C"), 3702));
        }

        private void menuItem13_Click(object sender, EventArgs e)
        {
            menuItem13.Checked = true;
            menuItem14.Checked = false;
            menuItem15.Checked = false;
            UpdateStatusLine1();
        }

        private void menuItem14_Click(object sender, EventArgs e)
        {
            menuItem13.Checked = false;
            menuItem14.Checked = true;
            menuItem15.Checked = false;
            UpdateStatusLine1();
        }

        private void menuItem15_Click(object sender, EventArgs e)
        {
            menuItem13.Checked = false;
            menuItem14.Checked = false;
            menuItem15.Checked = true;
            UpdateStatusLine1();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            // Force the ListView to use double buffering to remove flicker.
            packetListView.GetType().GetProperty("DoubleBuffered", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance).SetValue(packetListView, true, null);
            deviceListView.GetType().GetProperty("DoubleBuffered", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance).SetValue(deviceListView, true, null);

            if (!Utils.IsMono())
            {
                // On Windows bind to each individual interface
                IPAddress[] LocalAddresses = Dns.GetHostEntry(Dns.GetHostName()).AddressList;
                ArrayList temp = new ArrayList();
                foreach (IPAddress i in LocalAddresses) temp.Add(i);
                temp.Add(IPAddress.Loopback);
                temp.Add(IPAddress.IPv6Loopback);
                LocalAddresses = (IPAddress[])temp.ToArray(typeof(IPAddress));
                foreach (IPAddress addr in LocalAddresses)
                {
                    try
                    {
                        UPnPMulticastSniffer Sniffer = new UPnPMulticastSniffer(new IPEndPoint(addr, 1900));
                        Sniffer.OnPacket += new UPnPMulticastSniffer.PacketHandler(SniffSink);
                        Sniffers.Add(Sniffer);
                    }
                    catch (Exception) { }
                }
            }
            else
            {
                // On Linux, bind to any (TODO: IPv6)
                UPnPMulticastSniffer Sniffer = new UPnPMulticastSniffer(new IPEndPoint(IPAddress.Any, 1900));
                Sniffer.OnPacket += new UPnPMulticastSniffer.PacketHandler(SniffSink);
                Sniffers.Add(Sniffer);
            }

            SSniffer = new UPnPSearchSniffer();
            SSniffer.OnPacket += new UPnPSearchSniffer.PacketHandler(SniffSink);

            notifyStatStart = DateTime.Now;
            notifyStatCount = 0;

            // Check for update
            if (File.Exists(Application.StartupPath + "\\AutoUpdateTool.exe"))
            {
                AutoUpdate.AutoUpdateCheck(this);
            }
            else
            {
                menuItem16.Visible = false;
            }

            //this.Height = 480; this.Width = 820;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            foreach (UPnPMulticastSniffer sniffer in Sniffers) sniffer.Dispose();
            Sniffers.Clear();
        }

        private void menuItem2_Popup(object sender, EventArgs e)
        {
            menuItem16.Checked = AutoUpdate.GetAutoUpdateCheck();
        }

        private void menuItem16_Click(object sender, EventArgs e)
        {
            AutoUpdate.SetAutoUpdateCheck(!menuItem16.Checked);
            if (!menuItem16.Checked) AutoUpdate.UpdateCheck(this);
        }

    }
}
