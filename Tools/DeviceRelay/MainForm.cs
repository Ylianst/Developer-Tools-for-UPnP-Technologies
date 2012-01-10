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
using System.Data;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for Form.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private bool Trace = false;
		private ArrayList TraceList = new ArrayList();

		private UPnPSmartControlPoint scp;

		private int PortNumber = 7255;
		private UPnPDeviceFactory devicefactory = null;
		private CpGateKeeper home = null;
		private ArrayList devicelist = new ArrayList();
		private Gatekeeper gk = null;
		private string shareuri;

		private Hashtable InboundListViewTable = Hashtable.Synchronized(new Hashtable());
		private Hashtable OutboundListViewTable = Hashtable.Synchronized(new Hashtable());
		private Hashtable OutboundEventTable = new Hashtable();
		private Hashtable OutboundActionTable = new Hashtable();

		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem exitMenuItem;
		private System.Windows.Forms.StatusBar statusBar;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ImageList imageList;
		private System.Windows.Forms.ContextMenu outboundContextMenu;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MenuItem connectMenuItem;
		private System.Windows.Forms.MenuItem addSharedMenuItem;
		private System.Windows.Forms.MenuItem removeSharedMenuItem;
		private System.Windows.Forms.ListView outboundListView;
		private System.Windows.Forms.ListView inboundListView;
		private System.Windows.Forms.MenuItem disconnectMenuItem;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem ChangePortNumberMenuItem;
		private System.Windows.Forms.Label SharingOnLabel;
		private System.Windows.Forms.MenuItem menuItem8;
		private System.Windows.Forms.MenuItem TraceMenuItem;
		private System.Windows.Forms.MenuItem ShowTraceMenuItem;
		private System.Windows.Forms.MenuItem automaticMenuItem;
		private System.Windows.Forms.PictureBox openPictureBox;
		private System.Windows.Forms.PictureBox closedPictureBox;
		private System.Windows.Forms.PictureBox iconPictureBox;
		private System.Windows.Forms.MenuItem ShowTrackerItem;
		private System.Windows.Forms.MenuItem helpMenuItem;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.MenuItem debugMenuItem;
		private System.ComponentModel.IContainer components;

		private string overrideIP = "";

		public MainForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(SCPAddSink));
			scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(SCPRemoveSink);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
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
            this.mainMenu1 = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.connectMenuItem = new System.Windows.Forms.MenuItem();
            this.disconnectMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.addSharedMenuItem = new System.Windows.Forms.MenuItem();
            this.removeSharedMenuItem = new System.Windows.Forms.MenuItem();
            this.automaticMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.ChangePortNumberMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.TraceMenuItem = new System.Windows.Forms.MenuItem();
            this.ShowTraceMenuItem = new System.Windows.Forms.MenuItem();
            this.ShowTrackerItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.debugMenuItem = new System.Windows.Forms.MenuItem();
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.panel1 = new System.Windows.Forms.Panel();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.outboundListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.outboundContextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.imageList = new System.Windows.Forms.ImageList(this.components);
            this.label1 = new System.Windows.Forms.Label();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.panel2 = new System.Windows.Forms.Panel();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.inboundListView = new System.Windows.Forms.ListView();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.label2 = new System.Windows.Forms.Label();
            this.panel3 = new System.Windows.Forms.Panel();
            this.closedPictureBox = new System.Windows.Forms.PictureBox();
            this.openPictureBox = new System.Windows.Forms.PictureBox();
            this.SharingOnLabel = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.iconPictureBox = new System.Windows.Forms.PictureBox();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.panel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            this.panel3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.closedPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.openPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.iconPictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem7,
            this.menuItem2});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.connectMenuItem,
            this.disconnectMenuItem,
            this.menuItem3,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            // 
            // connectMenuItem
            // 
            this.connectMenuItem.Index = 0;
            this.connectMenuItem.Text = "Connect to Peer Relay...";
            this.connectMenuItem.Click += new System.EventHandler(this.connectMenuItem_Click);
            // 
            // disconnectMenuItem
            // 
            this.disconnectMenuItem.Enabled = false;
            this.disconnectMenuItem.Index = 1;
            this.disconnectMenuItem.Text = "Disconnect from Peer Relay";
            this.disconnectMenuItem.Click += new System.EventHandler(this.disconnectMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 2;
            this.menuItem3.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 3;
            this.exitMenuItem.Text = "&Exit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 1;
            this.menuItem7.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.addSharedMenuItem,
            this.removeSharedMenuItem,
            this.automaticMenuItem,
            this.menuItem4,
            this.ChangePortNumberMenuItem,
            this.menuItem8,
            this.TraceMenuItem,
            this.ShowTraceMenuItem,
            this.ShowTrackerItem});
            this.menuItem7.Text = "&Sharing";
            this.menuItem7.Popup += new System.EventHandler(this.menuItem7_Popup);
            // 
            // addSharedMenuItem
            // 
            this.addSharedMenuItem.Index = 0;
            this.addSharedMenuItem.Text = "&Add Shared Device";
            this.addSharedMenuItem.Click += new System.EventHandler(this.addSharedMenuItem_Click);
            // 
            // removeSharedMenuItem
            // 
            this.removeSharedMenuItem.Index = 1;
            this.removeSharedMenuItem.Text = "&Remove Shared Device";
            this.removeSharedMenuItem.Click += new System.EventHandler(this.removeSharedMenuItem_Click);
            // 
            // automaticMenuItem
            // 
            this.automaticMenuItem.Index = 2;
            this.automaticMenuItem.Text = "Automatic Sharing";
            this.automaticMenuItem.Click += new System.EventHandler(this.automaticMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 3;
            this.menuItem4.Text = "-";
            // 
            // ChangePortNumberMenuItem
            // 
            this.ChangePortNumberMenuItem.Index = 4;
            this.ChangePortNumberMenuItem.Text = "Change Port Number";
            this.ChangePortNumberMenuItem.Click += new System.EventHandler(this.ChangePortNumberMenuItem_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 5;
            this.menuItem8.Text = "-";
            // 
            // TraceMenuItem
            // 
            this.TraceMenuItem.Index = 6;
            this.TraceMenuItem.Text = "Trace";
            this.TraceMenuItem.Click += new System.EventHandler(this.TraceMenuItem_Click);
            // 
            // ShowTraceMenuItem
            // 
            this.ShowTraceMenuItem.Index = 7;
            this.ShowTraceMenuItem.Text = "Show Trace";
            this.ShowTraceMenuItem.Click += new System.EventHandler(this.ShowTraceMenuItem_Click);
            // 
            // ShowTrackerItem
            // 
            this.ShowTrackerItem.Index = 8;
            this.ShowTrackerItem.Text = "Show Tracker";
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 2;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem10,
            this.debugMenuItem});
            this.menuItem2.Text = "&Help";
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
            this.menuItem10.Index = 1;
            this.menuItem10.Text = "-";
            // 
            // debugMenuItem
            // 
            this.debugMenuItem.Index = 2;
            this.debugMenuItem.Text = "&Show Debug Information";
            this.debugMenuItem.Click += new System.EventHandler(this.debugMenuItem_Click);
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 353);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(384, 16);
            this.statusBar.TabIndex = 2;
            this.statusBar.Text = "Device Relay";
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Controls.Add(this.outboundListView);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 48);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(384, 150);
            this.panel1.TabIndex = 9;
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.Color.Transparent;
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(8, 4);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(48, 18);
            this.pictureBox1.TabIndex = 11;
            this.pictureBox1.TabStop = false;
            // 
            // outboundListView
            // 
            this.outboundListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.outboundListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader3,
            this.columnHeader4});
            this.outboundListView.ContextMenu = this.outboundContextMenu;
            this.outboundListView.FullRowSelect = true;
            this.outboundListView.Location = new System.Drawing.Point(8, 24);
            this.outboundListView.Name = "outboundListView";
            this.outboundListView.Size = new System.Drawing.Size(364, 118);
            this.outboundListView.SmallImageList = this.imageList;
            this.outboundListView.TabIndex = 10;
            this.outboundListView.UseCompatibleStateImageBehavior = false;
            this.outboundListView.View = System.Windows.Forms.View.Details;
            this.outboundListView.KeyUp += new System.Windows.Forms.KeyEventHandler(this.outboundListView_KeyUp);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Device Name";
            this.columnHeader1.Width = 217;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Actions";
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Events";
            // 
            // outboundContextMenu
            // 
            this.outboundContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem5,
            this.menuItem6});
            this.outboundContextMenu.Popup += new System.EventHandler(this.outboundContextMenu_Popup);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 0;
            this.menuItem5.Text = "Add Shared Device";
            this.menuItem5.Click += new System.EventHandler(this.addSharedMenuItem_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 1;
            this.menuItem6.Text = "Remove Shared Device";
            this.menuItem6.Click += new System.EventHandler(this.removeSharedMenuItem_Click);
            // 
            // imageList
            // 
            this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
            this.imageList.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList.Images.SetKeyName(0, "");
            this.imageList.Images.SetKeyName(1, "");
            this.imageList.Images.SetKeyName(2, "");
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.Location = new System.Drawing.Point(56, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(316, 16);
            this.label1.TabIndex = 9;
            this.label1.Text = "Outbound Shared Devices";
            // 
            // splitter1
            // 
            this.splitter1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.splitter1.Location = new System.Drawing.Point(0, 198);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(384, 3);
            this.splitter1.TabIndex = 10;
            this.splitter1.TabStop = false;
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel2.Controls.Add(this.pictureBox2);
            this.panel2.Controls.Add(this.inboundListView);
            this.panel2.Controls.Add(this.label2);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel2.Location = new System.Drawing.Point(0, 201);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(384, 152);
            this.panel2.TabIndex = 11;
            // 
            // pictureBox2
            // 
            this.pictureBox2.BackColor = System.Drawing.Color.Transparent;
            this.pictureBox2.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox2.Image")));
            this.pictureBox2.Location = new System.Drawing.Point(8, 4);
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.Size = new System.Drawing.Size(48, 18);
            this.pictureBox2.TabIndex = 14;
            this.pictureBox2.TabStop = false;
            // 
            // inboundListView
            // 
            this.inboundListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.inboundListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2,
            this.columnHeader5,
            this.columnHeader6});
            this.inboundListView.Enabled = false;
            this.inboundListView.FullRowSelect = true;
            this.inboundListView.Location = new System.Drawing.Point(8, 24);
            this.inboundListView.Name = "inboundListView";
            this.inboundListView.Size = new System.Drawing.Size(364, 116);
            this.inboundListView.SmallImageList = this.imageList;
            this.inboundListView.TabIndex = 13;
            this.inboundListView.UseCompatibleStateImageBehavior = false;
            this.inboundListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Device Name";
            this.columnHeader2.Width = 216;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Actions";
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Events";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.Location = new System.Drawing.Point(56, 7);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(316, 16);
            this.label2.TabIndex = 12;
            this.label2.Text = "Inbound Accessible Devices";
            // 
            // panel3
            // 
            this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel3.Controls.Add(this.closedPictureBox);
            this.panel3.Controls.Add(this.openPictureBox);
            this.panel3.Controls.Add(this.SharingOnLabel);
            this.panel3.Controls.Add(this.label3);
            this.panel3.Controls.Add(this.iconPictureBox);
            this.panel3.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel3.Location = new System.Drawing.Point(0, 0);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(384, 48);
            this.panel3.TabIndex = 13;
            // 
            // closedPictureBox
            // 
            this.closedPictureBox.BackColor = System.Drawing.Color.Transparent;
            this.closedPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("closedPictureBox.Image")));
            this.closedPictureBox.Location = new System.Drawing.Point(48, 56);
            this.closedPictureBox.Name = "closedPictureBox";
            this.closedPictureBox.Size = new System.Drawing.Size(32, 32);
            this.closedPictureBox.TabIndex = 16;
            this.closedPictureBox.TabStop = false;
            this.closedPictureBox.Visible = false;
            // 
            // openPictureBox
            // 
            this.openPictureBox.BackColor = System.Drawing.Color.Transparent;
            this.openPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("openPictureBox.Image")));
            this.openPictureBox.Location = new System.Drawing.Point(8, 56);
            this.openPictureBox.Name = "openPictureBox";
            this.openPictureBox.Size = new System.Drawing.Size(32, 32);
            this.openPictureBox.TabIndex = 15;
            this.openPictureBox.TabStop = false;
            this.openPictureBox.Visible = false;
            // 
            // SharingOnLabel
            // 
            this.SharingOnLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.SharingOnLabel.Location = new System.Drawing.Point(56, 24);
            this.SharingOnLabel.Name = "SharingOnLabel";
            this.SharingOnLabel.Size = new System.Drawing.Size(316, 16);
            this.SharingOnLabel.TabIndex = 14;
            this.SharingOnLabel.Text = "Sharing on port 0";
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label3.Location = new System.Drawing.Point(56, 8);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(316, 16);
            this.label3.TabIndex = 13;
            this.label3.Text = "Device Relay Sharing 0 Devices.";
            // 
            // iconPictureBox
            // 
            this.iconPictureBox.BackColor = System.Drawing.Color.Transparent;
            this.iconPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("iconPictureBox.Image")));
            this.iconPictureBox.Location = new System.Drawing.Point(8, 8);
            this.iconPictureBox.Name = "iconPictureBox";
            this.iconPictureBox.Size = new System.Drawing.Size(32, 32);
            this.iconPictureBox.TabIndex = 12;
            this.iconPictureBox.TabStop = false;
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(384, 369);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.panel3);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.statusBar);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu1;
            this.Name = "MainForm";
            this.Text = "Device Relay";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.Closing += new System.ComponentModel.CancelEventHandler(this.OnClosing);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            this.panel3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.closedPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.openPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.iconPictureBox)).EndInit();
            this.ResumeLayout(false);

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

		private void SCPAddSink(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			if (automaticMenuItem.Checked == false) return;
			if (devicelist.Contains(device) == true) return;
			if (device.FriendlyName.StartsWith("*") == true) return;
			ListViewItem lv = new ListViewItem(new string[] {device.FriendlyName,"0","0"},1);
			lv.Tag = device;
			OutboundListViewTable[device] = lv;
			//device.OnRemoved += new UPnPDevice.OnRemovedHandler(RemovedSink);
			lock(outboundListView)
			{
				outboundListView.Items.Add(lv);
			}
			devicelist.Add(device);
			gk.AddDevice(device);
		}

		private void SCPRemoveSink(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			if (devicelist.Contains(device) == false) return;
			//if (automaticMenuItem.Checked == false) return;
			

			lock(outboundListView)
			{
				foreach (ListViewItem lv in outboundListView.Items) 
				{
					UPnPDevice d = (UPnPDevice)lv.Tag;
					if (d == device) outboundListView.Items.Remove(lv);
				}
			}
			//device.OnRemoved -= new UPnPDevice.OnRemovedHandler(RemovedSink);
			devicelist.Remove(device);
			gk.RemoveDevice(device);
		}

		private void ConnectFailedSink(UPnPDeviceFactory sender, Uri URL, Exception e, string urn)
		{
			devicefactory = null;
			statusBar.Text = "Connection Failed";
		}
		private void ConnectSink(UPnPDeviceFactory sender, UPnPDevice d, Uri ConnectUri)
		{
			string useIP = d.InterfaceToHost.ToString();
			disconnectMenuItem.Enabled = false;
			CheckIconState();

			if(this.overrideIP!="") {useIP = this.overrideIP;}

			statusBar.Text = "Connected to Peer Relay";
			devicefactory = null;
			home = new CpGateKeeper(d.GetServices(CpGateKeeper.SERVICE_NAME)[0]);
			home.Register(new Uri("http://" + useIP + ":" + this.PortNumber.ToString()),true);
		}

		private void exitMenuItem_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void addSharedMenuItem_Click(object sender, System.EventArgs e)
		{
			DeviceSelector deviceselector = new DeviceSelector();
			if (deviceselector.ShowDialog() == DialogResult.OK)
			{
				if (devicelist.Contains(deviceselector.SelectedDevice) == true) 
				{
					MessageBox.Show(deviceselector.SelectedDevice.FriendlyName + " Already Shared","Relay for UPnP Technologies");
					return;
				}
				ListViewItem lv = new ListViewItem(new string[] {deviceselector.SelectedDevice.FriendlyName,"0","0"},1);
				lv.Tag = deviceselector.SelectedDevice;
				OutboundListViewTable[deviceselector.SelectedDevice] = lv;
				//deviceselector.SelectedDevice.OnRemoved += new UPnPDevice.OnRemovedHandler(RemovedSink);
				lock(outboundListView)
				{
					outboundListView.Items.Add(lv);
				}
				devicelist.Add(deviceselector.SelectedDevice);
				gk.AddDevice(deviceselector.SelectedDevice);
			}
			deviceselector.Dispose();		
		}

		private void RemovedSink(UPnPDevice d)
		{
			return;
            /*
			devicelist.Remove(d);
			lock(outboundListView)
			{
				for(int i=0;i<outboundListView.Items.Count;++i)
				{
					if(((UPnPDevice)outboundListView.Items[i].Tag)==d)
					{
						outboundListView.Items.RemoveAt(i);
						break;
					}
				}
			}
            */
		}

		private void removeSharedMenuItem_Click(object sender, System.EventArgs e)
		{
			if (outboundListView.SelectedItems.Count == 0) return;
			ListViewItem lv = null;
			lock(outboundListView)
			{
				lv = outboundListView.SelectedItems[0];
				outboundListView.Items.Remove(lv);
			}
			UPnPDevice device = (UPnPDevice)lv.Tag;
			if (devicelist.Contains(device) == false) return;
			//device.OnRemoved -= new UPnPDevice.OnRemovedHandler(RemovedSink);
			devicelist.Remove(device);
			gk.RemoveDevice(device);
		}

		private void outboundContextMenu_Popup(object sender, System.EventArgs e)
		{
			menuItem6.Visible = (outboundListView.SelectedItems.Count > 0);
		}

		private void menuItem7_Popup(object sender, System.EventArgs e)
		{
			removeSharedMenuItem.Enabled = (outboundListView.SelectedItems.Count > 0);
		}

		private void connectMenuItem_Click(object sender, System.EventArgs e)
		{
			ConnectionForm connectionform = new ConnectionForm();
			connectionform.AddressText = "";
			if (connectionform.ShowDialog() == DialogResult.OK) 
			{
				// TODO
				Uri ConnectUri = new Uri("http://" + connectionform.AddressText);
				shareuri = ConnectUri.ToString();
                devicefactory = new UPnPDeviceFactory(ConnectUri, 1000, new UPnPDeviceFactory.UPnPDeviceHandler(ConnectSink), new UPnPDeviceFactory.UPnPDeviceFailedHandler(ConnectFailedSink), null, null);
				statusBar.Text = string.Format("Attempting Connection to {0}.", connectionform.AddressText);
			}
			connectionform.Dispose();
		}

		private void disconnectMenuItem_Click(object sender, System.EventArgs e)
		{
			if(home != null)
			{
				home.UnRegister(new Uri(shareuri));
				home.Dispose();
				home = null;
			}
			IDictionaryEnumerator en = gk.ProxyTable.GetEnumerator();
			while(en.MoveNext())
			{
				((UPnPRelayDevice)en.Value).StopDevice();
			}
			gk.ProxyTable.Clear();

			connectMenuItem.Enabled = true;
			inboundListView.Enabled = false;
			CheckIconState();
		}

		private void MainForm_Load(object sender, System.EventArgs e)
		{
			SharingOnLabel.Text = "Sharing on port " + this.PortNumber.ToString();

			gk = new Gatekeeper(this.PortNumber);
			gk.OnEvent += new Gatekeeper.OnEventHandler(GKEventSink);
			gk.OnAction += new Gatekeeper.OnActionHandler(GKActionSink);
			gk.OnUPnPRelayDevice += new Gatekeeper.OnUPnPRelayDeviceHandler(RelaySink);
			gk.OnUPnPRelayDeviceRemoved += new Gatekeeper.OnUPnPRelayDeviceHandler(RelayRemovedSink);
		}
		private void GKActionSink(UPnPDevice DeviceSender, UPnPService ServiceSender, string ActionName)
		{
			BeginInvoke(new Gatekeeper.OnActionHandler(GKActionSinkEx),new object[3]{DeviceSender,ServiceSender,ActionName});
		}
		private void GKActionSinkEx(UPnPDevice DeviceSender, UPnPService ServiceSender, string ActionName)
		{
			ListViewItem lvi = (ListViewItem)OutboundListViewTable[DeviceSender];
			if(OutboundActionTable.ContainsKey(DeviceSender)==false)
			{
				OutboundActionTable[DeviceSender] = (long)0;
			}
			long Counter = (long)OutboundActionTable[DeviceSender];
			++Counter;
			OutboundActionTable[DeviceSender] = Counter;
			lvi.SubItems[1].Text = Counter.ToString();

			if(Trace)
			{
				TraceList.Add(DeviceSender.FriendlyName + ": " + ServiceSender.ServiceURN + " [" + ServiceSender.ServiceID + "] Action: " + ActionName);
			}

		}
		private void GKEventSink(UPnPDevice sender, string Name)
		{
			BeginInvoke(new Gatekeeper.OnEventHandler(GKEventSinkEx),new Object[2]{sender,Name});
		}
		private void GKEventSinkEx(UPnPDevice sender, string Name)
		{
			ListViewItem lvi = (ListViewItem)OutboundListViewTable[sender];
			if(OutboundEventTable.ContainsKey(sender)==false)
			{
				OutboundEventTable[sender] = (long)0;
			}
			long Counter = (long)OutboundEventTable[sender];
			++Counter;
			OutboundEventTable[sender] = Counter;
			lvi.SubItems[2].Text = Counter.ToString();
		}
		private void RelayRemovedSink(Gatekeeper sender, UPnPRelayDevice d)
		{
			RemoveInboundDevice(d);
		}
		private void RelaySink(Gatekeeper sender, UPnPRelayDevice d)
		{
			AddNewInboundDevice(d);
		}

		private void RemoveInboundDevice(UPnPRelayDevice d)
		{
			ListViewItem lvi = (ListViewItem)InboundListViewTable[d];
			d.OnAction -= new UPnPRelayDevice.ActionCounterHandler(ActionSink);
			d.OnEvent -= new UPnPRelayDevice.EventCounterHandler(EventSink);

			InboundListViewTable.Remove(d);
			inboundListView.Items.Remove(lvi);
			inboundListView.Enabled = (inboundListView.Items.Count != 0);
			CheckIconState();
		}

		private void CheckIconState() 
		{
			if (inboundListView.Items.Count == 0 && disconnectMenuItem.Enabled == false)
			{
				iconPictureBox.Image = closedPictureBox.Image;
			} 
			else 
			{
				iconPictureBox.Image = openPictureBox.Image;
			}
		}

		private void AddNewInboundDevice(UPnPRelayDevice d)
		{
			ListViewItem lvi = new ListViewItem(new String[3]{d.FriendlyName,"0","0"},1);
			lvi.Tag = d;
			d.OnAction += new UPnPRelayDevice.ActionCounterHandler(ActionSink);
			d.OnEvent += new UPnPRelayDevice.EventCounterHandler(EventSink);
			this.InboundListViewTable[d] = lvi;
			this.inboundListView.Items.Add(lvi);
			inboundListView.Enabled = (inboundListView.Items.Count != 0);
			CheckIconState();
		}
		private void ActionSink(UPnPRelayDevice d, long ActionCounter)
		{
			this.BeginInvoke(new UPnPRelayDevice.ActionCounterHandler(ActionSinkEx),new object[2]{d,ActionCounter});
		}
		private void ActionSinkEx(UPnPRelayDevice d, long ActionCounter)
		{
			ListViewItem lvi = (ListViewItem)InboundListViewTable[d];
			lvi.SubItems[1].Text = ActionCounter.ToString();
		}
		private void EventSink(UPnPRelayDevice d, long EventCounter)
		{
			this.BeginInvoke(new UPnPRelayDevice.EventCounterHandler(EventSinkEx),new object[2]{d,EventCounter});
		}
		private void EventSinkEx(UPnPRelayDevice d, long EventCounter)
		{
			ListViewItem lvi = (ListViewItem)InboundListViewTable[d];
			lvi.SubItems[2].Text = EventCounter.ToString();
		}


		private void ChangePortNumberMenuItem_Click(object sender, System.EventArgs e)
		{
			PortNumberForm pnf = new PortNumberForm(this.PortNumber);
			if(pnf.ShowDialog()==DialogResult.OK)
			{
				this.overrideIP = pnf.PublicIPAddress;
				PortNumber = pnf.Port;
				gk.SwitchToPort(pnf.Port);
				gk.PublicIP = pnf.PublicIPAddress;
				SharingOnLabel.Text = "Sharing on "+pnf.PublicIPAddress+" port " + PortNumber.ToString();
			}
			pnf.Dispose();
		}

		private void TraceMenuItem_Click(object sender, System.EventArgs e)
		{
			TraceMenuItem.Checked = !TraceMenuItem.Checked;
			Trace = TraceMenuItem.Checked;
		}

		private void ShowTraceMenuItem_Click(object sender, System.EventArgs e)
		{
			TraceForm tf = new TraceForm((string[])TraceList.ToArray(typeof(string)));
			tf.ShowDialog();
			tf.Dispose();
		}

		private void automaticMenuItem_Click(object sender, System.EventArgs e)
		{
			automaticMenuItem.Checked = !automaticMenuItem.Checked;
		}

		private void outboundListView_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Delete) removeSharedMenuItem_Click(this,null);
		}

		private void OnClosing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			this.gk.Dispose();
		}

		private void helpMenuItem_Click(object sender, System.EventArgs e)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"Device Relay");
		}

		private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"Device Relay");
		}

		private void debugMenuItem_Click(object sender, System.EventArgs e)
		{
			OpenSource.Utilities.InstanceTracker.Display();
		}

	}
}
