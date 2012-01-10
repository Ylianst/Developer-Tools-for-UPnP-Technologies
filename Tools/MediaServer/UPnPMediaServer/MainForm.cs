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
using System.Data;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using System.ServiceProcess;
using System.Runtime.Remoting;
using System.Runtime.Serialization;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.Runtime.Remoting.Channels.Http;
using System.Runtime.Serialization.Formatters.Binary;
using UPnPMediaServerCore;

namespace UPnPMediaServerController
{
	/// <summary>
	/// Summary description for MainForm.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private bool standAloneMode = false;
		private bool registeredServerType = false;
		private UPnPMediaServer mediaServer = null;
		private MediaServerCore mediaServerCore = null;
		private ServiceController serviceController;
		private ServiceControllerStatus serviceStatus;
		private int mediaHttpTransfersUpdateId = -1;
        private int mediaSharedDirectoryUpdateId = -1;
		private System.Windows.Forms.MenuItem exitMenuItem;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.StatusBar mainStatusBar;
		private System.Windows.Forms.ListView listView;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ToolTip toolTip;
		private System.Windows.Forms.ImageList folderImageList;
		private System.Windows.Forms.Label serverStatus2;
		private System.Windows.Forms.Label serverStatus1;
		private System.Windows.Forms.MenuItem stopShareMenuItem;
		private System.Windows.Forms.MenuItem addDirMenuItem;
		private System.Windows.Forms.MenuItem removeDirMenuItem;
		private System.Windows.Forms.MainMenu mainMenu;
		private System.Windows.Forms.MenuItem viewTransfersPanelMenuItem;
		private System.Windows.Forms.ListView transfersListView;
		private System.Windows.Forms.Splitter transfersSplitter;
		private System.Windows.Forms.ImageList transfersImageList;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.Timer transferTimer;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.MenuItem openMenuItem;
		private System.Windows.Forms.MenuItem menuItem8;
		private System.Windows.Forms.MenuItem startServerMenuItem;
		private System.Windows.Forms.MenuItem stopServerMenuItem;
		private System.Windows.Forms.ContextMenu directoryContextMenu;
		private System.Windows.Forms.MenuItem menuItem11;
		private System.Windows.Forms.MenuItem serviceMenuItem;
		private System.Windows.Forms.MenuItem sharingMenuItem;
		private System.Windows.Forms.MenuItem helpMenuItem;
		private System.Windows.Forms.MenuItem fileMenuItem;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem showDebugInfoMenuItem;
		private System.Windows.Forms.MenuItem pauseServerMenuItem;
		private System.Windows.Forms.MenuItem addDir2MenuItem;
		private System.Windows.Forms.MenuItem restricted2MenuItem;
		private System.Windows.Forms.MenuItem restrictedMenuItem;
		private System.Windows.Forms.MenuItem readOnly2MenuItem;
		private System.Windows.Forms.MenuItem readOnlyMenuItem;
		private System.Windows.Forms.MenuItem menu_Deserialize;
		private System.Windows.Forms.MenuItem menu_Serialize;
		private System.Windows.Forms.SaveFileDialog saveTreeDlg;
		private System.Windows.Forms.OpenFileDialog openTreeDlg;
		private System.Windows.Forms.MenuItem closeHierarchy;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem helpMenuItem2;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem6;
        private FolderBrowserDialog folderBrowserDialog;
		private System.ComponentModel.IContainer components;

		public MainForm(string[] args)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			OpenSource.UPnP.DText p = new OpenSource.UPnP.DText();
			p.ATTRMARK = "=";
			string[] filePaths = new string[1];

			foreach(string arg in args)
			{
				p[0] = arg;
				switch(p[1].ToUpper())
				{
					case "-UDN":
						MediaServerCore.CustomUDN = p[2];
						break;
					case "-CACHETIME":
						MediaServerCore.CacheTime = int.Parse(p[2]);
						break;
					case "-INMPR":
						MediaServerCore.INMPR = !(p[2].ToUpper()=="NO");
						break;
				}
			}



			// Setup the UI
			transfersSplitter.Visible = viewTransfersPanelMenuItem.Checked;		
			transfersListView.Visible = viewTransfersPanelMenuItem.Checked;

			try 
			{
				serviceController = new ServiceController("UPnP Media Server");
				serviceStatus = serviceController.Status;
				OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"Service control mode...");
			} 
			catch (System.InvalidOperationException) 
			{
				serviceController = null;
				serviceStatus = ServiceControllerStatus.Stopped;
				OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"Stand alone mode...");
			}

			if (serviceController != null)
			{
				// Service controller mode
				serviceMenuItem.Visible = true;

				// Pause State
				pauseServerMenuItem.Visible = false;

				System.Collections.Specialized.ListDictionary channelProperties = new System.Collections.Specialized.ListDictionary();
				channelProperties.Add("port", 12330);
				HttpChannel channel = new HttpChannel(channelProperties,
					new SoapClientFormatterSinkProvider(),
					new SoapServerFormatterSinkProvider());
				ChannelServices.RegisterChannel(channel, false);
				OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"RegisterChannel");

				if (serviceStatus == ServiceControllerStatus.Running) 
				{
					OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"RegisterWellKnownClientType");
					RemotingConfiguration.RegisterWellKnownClientType(
						typeof(UPnPMediaServer),
						"http://localhost:12329/UPnPMediaServer/UPnPMediaServer.soap"
						);
					registeredServerType = true;
				}
			} 
			else 
			{
				// Stand alone mode
				if (registeredServerType == true || standAloneMode == true) return;

				standAloneMode = true;
				serviceMenuItem.Visible = false;

				// Stand alone mode
				mediaServerCore = new MediaServerCore("Media Server (" + System.Windows.Forms.SystemInformation.ComputerName + ")");
				this.mediaServerCore.OnDirectoriesChanged += new MediaServerCore.MediaServerCoreEventHandler(this.Sink_OnDirectoriesChanged);
				mediaServer = new UPnPMediaServer();

				// Pause State
				pauseServerMenuItem.Checked = this.mediaServerCore.IsPaused;
			}

			UpdateServiceUI();

			foreach(string arg in args)
			{
				p[0] = arg;
				switch(p[1].ToUpper())
				{
					case "-P":
						filePaths[0] = p[2];
						try
						{
							this.AddDirs(filePaths);
						}
						catch (Exception ex)
						{
							MessageBox.Show(ex.ToString());
						}
						break;
				}
			}

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
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.fileMenuItem = new System.Windows.Forms.MenuItem();
            this.viewTransfersPanelMenuItem = new System.Windows.Forms.MenuItem();
            this.pauseServerMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.menu_Deserialize = new System.Windows.Forms.MenuItem();
            this.menu_Serialize = new System.Windows.Forms.MenuItem();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.serviceMenuItem = new System.Windows.Forms.MenuItem();
            this.startServerMenuItem = new System.Windows.Forms.MenuItem();
            this.stopServerMenuItem = new System.Windows.Forms.MenuItem();
            this.sharingMenuItem = new System.Windows.Forms.MenuItem();
            this.addDirMenuItem = new System.Windows.Forms.MenuItem();
            this.removeDirMenuItem = new System.Windows.Forms.MenuItem();
            this.closeHierarchy = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.restricted2MenuItem = new System.Windows.Forms.MenuItem();
            this.readOnly2MenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.helpMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.showDebugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.panel1 = new System.Windows.Forms.Panel();
            this.serverStatus2 = new System.Windows.Forms.Label();
            this.serverStatus1 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.mainStatusBar = new System.Windows.Forms.StatusBar();
            this.listView = new System.Windows.Forms.ListView();
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.directoryContextMenu = new System.Windows.Forms.ContextMenu();
            this.openMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.restrictedMenuItem = new System.Windows.Forms.MenuItem();
            this.readOnlyMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.addDir2MenuItem = new System.Windows.Forms.MenuItem();
            this.stopShareMenuItem = new System.Windows.Forms.MenuItem();
            this.folderImageList = new System.Windows.Forms.ImageList(this.components);
            this.transfersListView = new System.Windows.Forms.ListView();
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.transfersImageList = new System.Windows.Forms.ImageList(this.components);
            this.transfersSplitter = new System.Windows.Forms.Splitter();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.transferTimer = new System.Windows.Forms.Timer(this.components);
            this.saveTreeDlg = new System.Windows.Forms.SaveFileDialog();
            this.openTreeDlg = new System.Windows.Forms.OpenFileDialog();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.fileMenuItem,
            this.serviceMenuItem,
            this.sharingMenuItem,
            this.helpMenuItem});
            // 
            // fileMenuItem
            // 
            this.fileMenuItem.Index = 0;
            this.fileMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.viewTransfersPanelMenuItem,
            this.pauseServerMenuItem,
            this.menuItem5,
            this.menu_Deserialize,
            this.menu_Serialize,
            this.menuItem1,
            this.exitMenuItem});
            this.fileMenuItem.Text = "&File";
            // 
            // viewTransfersPanelMenuItem
            // 
            this.viewTransfersPanelMenuItem.Checked = true;
            this.viewTransfersPanelMenuItem.Index = 0;
            this.viewTransfersPanelMenuItem.Text = "Show &Transfers Panel";
            this.viewTransfersPanelMenuItem.Click += new System.EventHandler(this.viewTransfersPanelMenuItem_Click);
            // 
            // pauseServerMenuItem
            // 
            this.pauseServerMenuItem.Index = 1;
            this.pauseServerMenuItem.Text = "Pause Media Server";
            this.pauseServerMenuItem.Click += new System.EventHandler(this.pauseServerMenuItem_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 2;
            this.menuItem5.Text = "-";
            // 
            // menu_Deserialize
            // 
            this.menu_Deserialize.Index = 3;
            this.menu_Deserialize.Text = "L&oad Content Hierarchy...";
            this.menu_Deserialize.Click += new System.EventHandler(this.menu_Deserialize_Click);
            // 
            // menu_Serialize
            // 
            this.menu_Serialize.Index = 4;
            this.menu_Serialize.Text = "S&ave Content Hierarchy...";
            this.menu_Serialize.Click += new System.EventHandler(this.menu_Serialize_Click);
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 5;
            this.menuItem1.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 6;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // serviceMenuItem
            // 
            this.serviceMenuItem.Index = 1;
            this.serviceMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.startServerMenuItem,
            this.stopServerMenuItem});
            this.serviceMenuItem.Text = "&Service";
            this.serviceMenuItem.Popup += new System.EventHandler(this.serviceMenuItem_Popup);
            // 
            // startServerMenuItem
            // 
            this.startServerMenuItem.Index = 0;
            this.startServerMenuItem.Text = "&Start Media Server Service";
            this.startServerMenuItem.Click += new System.EventHandler(this.startServerMenuItem_Click);
            // 
            // stopServerMenuItem
            // 
            this.stopServerMenuItem.Index = 1;
            this.stopServerMenuItem.Text = "S&top Media Server Service";
            this.stopServerMenuItem.Click += new System.EventHandler(this.stopServerMenuItem_Click);
            // 
            // sharingMenuItem
            // 
            this.sharingMenuItem.Index = 2;
            this.sharingMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.addDirMenuItem,
            this.removeDirMenuItem,
            this.closeHierarchy,
            this.menuItem3,
            this.restricted2MenuItem,
            this.readOnly2MenuItem,
            this.menuItem6,
            this.menuItem2});
            this.sharingMenuItem.Text = "&Sharing";
            this.sharingMenuItem.Popup += new System.EventHandler(this.sharingMenuItem_Popup);
            // 
            // addDirMenuItem
            // 
            this.addDirMenuItem.Index = 0;
            this.addDirMenuItem.Text = "&Add Shared Directory";
            this.addDirMenuItem.Click += new System.EventHandler(this.addDirMenuItem_Click);
            // 
            // removeDirMenuItem
            // 
            this.removeDirMenuItem.Index = 1;
            this.removeDirMenuItem.Text = "&Remove Sharing Directory";
            this.removeDirMenuItem.Click += new System.EventHandler(this.removeDirMenuItem_Click);
            // 
            // closeHierarchy
            // 
            this.closeHierarchy.Index = 2;
            this.closeHierarchy.Text = "&Clear All Shared Directories";
            this.closeHierarchy.Click += new System.EventHandler(this.closeHierarchy_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 3;
            this.menuItem3.Text = "-";
            // 
            // restricted2MenuItem
            // 
            this.restricted2MenuItem.Index = 4;
            this.restricted2MenuItem.Text = "Restricted Access";
            this.restricted2MenuItem.Click += new System.EventHandler(this.restricted2MenuItem_Click);
            // 
            // readOnly2MenuItem
            // 
            this.readOnly2MenuItem.Index = 5;
            this.readOnly2MenuItem.Text = "Read Only Access";
            this.readOnly2MenuItem.Click += new System.EventHandler(this.readOnly2MenuItem_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 6;
            this.menuItem6.Text = "-";
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 7;
            this.menuItem2.Text = "C&onfigure Search  Sort Capabilities";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click_1);
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 3;
            this.helpMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem2,
            this.menuItem4,
            this.showDebugInfoMenuItem});
            this.helpMenuItem.Text = "&Help";
            // 
            // helpMenuItem2
            // 
            this.helpMenuItem2.Index = 0;
            this.helpMenuItem2.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem2.Text = "&Help Topics";
            this.helpMenuItem2.Click += new System.EventHandler(this.helpMenuItem2_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 1;
            this.menuItem4.Text = "-";
            // 
            // showDebugInfoMenuItem
            // 
            this.showDebugInfoMenuItem.Index = 2;
            this.showDebugInfoMenuItem.Text = "&Show Debug Information";
            this.showDebugInfoMenuItem.Click += new System.EventHandler(this.showDebugInfoMenuItem_Click);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.serverStatus2);
            this.panel1.Controls.Add(this.serverStatus1);
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(496, 56);
            this.panel1.TabIndex = 0;
            // 
            // serverStatus2
            // 
            this.serverStatus2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serverStatus2.Location = new System.Drawing.Point(72, 32);
            this.serverStatus2.Name = "serverStatus2";
            this.serverStatus2.Size = new System.Drawing.Size(416, 16);
            this.serverStatus2.TabIndex = 2;
            // 
            // serverStatus1
            // 
            this.serverStatus1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serverStatus1.Location = new System.Drawing.Point(72, 16);
            this.serverStatus1.Name = "serverStatus1";
            this.serverStatus1.Size = new System.Drawing.Size(416, 16);
            this.serverStatus1.TabIndex = 1;
            this.serverStatus1.Text = "Media Server Serving 0 Files in 0 Directories";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(8, 0);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(56, 50);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // mainStatusBar
            // 
            this.mainStatusBar.Location = new System.Drawing.Point(0, 261);
            this.mainStatusBar.Name = "mainStatusBar";
            this.mainStatusBar.Size = new System.Drawing.Size(496, 16);
            this.mainStatusBar.TabIndex = 1;
            // 
            // listView
            // 
            this.listView.AllowDrop = true;
            this.listView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.columnHeader2,
            this.columnHeader1});
            this.listView.ContextMenu = this.directoryContextMenu;
            this.listView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView.FullRowSelect = true;
            this.listView.Location = new System.Drawing.Point(0, 56);
            this.listView.Name = "listView";
            this.listView.Size = new System.Drawing.Size(496, 105);
            this.listView.SmallImageList = this.folderImageList;
            this.listView.TabIndex = 2;
            this.listView.UseCompatibleStateImageBehavior = false;
            this.listView.View = System.Windows.Forms.View.Details;
            this.listView.DragDrop += new System.Windows.Forms.DragEventHandler(this.btnStart_DragDrop);
            this.listView.DragEnter += new System.Windows.Forms.DragEventHandler(this.btnStart_DragEnter);
            this.listView.DoubleClick += new System.EventHandler(this.listView_DoubleClick);
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Name";
            this.columnHeader3.Width = 83;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Permissions";
            this.columnHeader2.Width = 125;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Path";
            this.columnHeader1.Width = 270;
            // 
            // directoryContextMenu
            // 
            this.directoryContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.openMenuItem,
            this.menuItem8,
            this.restrictedMenuItem,
            this.readOnlyMenuItem,
            this.menuItem11,
            this.addDir2MenuItem,
            this.stopShareMenuItem});
            this.directoryContextMenu.Popup += new System.EventHandler(this.directoryContextMenu_Popup);
            // 
            // openMenuItem
            // 
            this.openMenuItem.DefaultItem = true;
            this.openMenuItem.Index = 0;
            this.openMenuItem.Text = "Open";
            this.openMenuItem.Click += new System.EventHandler(this.openMenuItem_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 1;
            this.menuItem8.Text = "-";
            // 
            // restrictedMenuItem
            // 
            this.restrictedMenuItem.Index = 2;
            this.restrictedMenuItem.Text = "Restricted Access";
            this.restrictedMenuItem.Click += new System.EventHandler(this.restricted2MenuItem_Click);
            // 
            // readOnlyMenuItem
            // 
            this.readOnlyMenuItem.Index = 3;
            this.readOnlyMenuItem.Text = "Read Only Access";
            this.readOnlyMenuItem.Click += new System.EventHandler(this.readOnly2MenuItem_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 4;
            this.menuItem11.Text = "-";
            // 
            // addDir2MenuItem
            // 
            this.addDir2MenuItem.Index = 5;
            this.addDir2MenuItem.Text = "&Add Shared Directory";
            this.addDir2MenuItem.Click += new System.EventHandler(this.addDirMenuItem_Click);
            // 
            // stopShareMenuItem
            // 
            this.stopShareMenuItem.Index = 6;
            this.stopShareMenuItem.Text = "&Remove Shared Directory";
            this.stopShareMenuItem.Click += new System.EventHandler(this.removeDirMenuItem_Click);
            // 
            // folderImageList
            // 
            this.folderImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("folderImageList.ImageStream")));
            this.folderImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.folderImageList.Images.SetKeyName(0, "");
            this.folderImageList.Images.SetKeyName(1, "");
            // 
            // transfersListView
            // 
            this.transfersListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4,
            this.columnHeader6,
            this.columnHeader5});
            this.transfersListView.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.transfersListView.Location = new System.Drawing.Point(0, 164);
            this.transfersListView.Name = "transfersListView";
            this.transfersListView.Size = new System.Drawing.Size(496, 97);
            this.transfersListView.SmallImageList = this.transfersImageList;
            this.transfersListView.TabIndex = 3;
            this.transfersListView.UseCompatibleStateImageBehavior = false;
            this.transfersListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Target";
            this.columnHeader4.Width = 120;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Position";
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Content";
            this.columnHeader5.Width = 300;
            // 
            // transfersImageList
            // 
            this.transfersImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("transfersImageList.ImageStream")));
            this.transfersImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.transfersImageList.Images.SetKeyName(0, "");
            this.transfersImageList.Images.SetKeyName(1, "");
            // 
            // transfersSplitter
            // 
            this.transfersSplitter.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.transfersSplitter.Location = new System.Drawing.Point(0, 161);
            this.transfersSplitter.Name = "transfersSplitter";
            this.transfersSplitter.Size = new System.Drawing.Size(496, 3);
            this.transfersSplitter.TabIndex = 4;
            this.transfersSplitter.TabStop = false;
            // 
            // transferTimer
            // 
            this.transferTimer.Enabled = true;
            this.transferTimer.Interval = 1000;
            this.transferTimer.Tick += new System.EventHandler(this.transferTimer_Tick);
            // 
            // saveTreeDlg
            // 
            this.saveTreeDlg.DefaultExt = "cds";
            this.saveTreeDlg.FileName = "MediaTree.cds";
            this.saveTreeDlg.Filter = "CDS file|*.cds|All files|*.*";
            this.saveTreeDlg.Title = "AV Media Server - Save CDS File";
            // 
            // openTreeDlg
            // 
            this.openTreeDlg.DefaultExt = "cds";
            this.openTreeDlg.FileName = "MediaTree.cds";
            this.openTreeDlg.Filter = "CDS file|*.cds|All files|*.*";
            this.openTreeDlg.Title = "AV Media Server - Open CDS File";
            // 
            // folderBrowserDialog
            // 
            this.folderBrowserDialog.Description = "Select a directory to share. That directory and all sub-folders will also be made" +
                " available on the network.";
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(496, 277);
            this.Controls.Add(this.listView);
            this.Controls.Add(this.transfersSplitter);
            this.Controls.Add(this.transfersListView);
            this.Controls.Add(this.mainStatusBar);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "AV Media Server";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);

		}
		#endregion

		private void Sink_OnDirectoriesChanged(MediaServerCore sender)
		{
			this.UpdateDirectoriesUI();
		}

		private void exitMenuItem_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void btnStart_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if ((IsRunning() == true || standAloneMode == true) && e.Data.GetDataPresent(DataFormats.FileDrop) == true)
			{
				e.Effect = DragDropEffects.All;
			}
		}

		private void btnStart_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (IsRunning() == false && standAloneMode == false) return;
			try 
			{
				lock(listView) 
				{
					if (e.Data.GetDataPresent(DataFormats.FileDrop) == false) return;

					string[] filePaths = (string[]) e.Data.GetData(DataFormats.FileDrop);
					this.AddDirs(filePaths);
				}
			} 
			catch (Exception ex)
			{
				MessageBox.Show(ex.ToString());
			}
		}

		private void AddDirs(string[] filePaths)
		{
			foreach (string path in filePaths)
			{
				if (Directory.Exists(path) == true) 
				{
					mainStatusBar.Text = "Sharing Directory " + path;

					OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"Adding Shared Directory: " + path);
					DirectoryInfo dirInfo = new DirectoryInfo(path);
					Exception error = mediaServer.AddDirectory(dirInfo, true, false);
					mainStatusBar.Text = "";
					if (error != null)
					{
						MessageBox.Show(this.GetExceptionMessage(error), "Add Shared Directory Failure");
					}
				}
			}
		}

		private void UpdateTransfersUI()
		{
			lock (transfersListView)
			{
				if (mediaServer != null && (IsRunning() == true || standAloneMode == true))
				{
					transfersListView.BeginUpdate();
					transfersListView.Items.Clear();
					mediaHttpTransfersUpdateId = mediaServer.MediaHttpTransfersUpdateId;

					if (mediaServer.HttpTransfers.Count > 1) 
					{
						serverStatus2.Text = mediaServer.HttpTransfers.Count.ToString() + " File Transfers";
					}
					else if (mediaServer.HttpTransfers.Count == 1)
					{
						serverStatus2.Text = "1 File Transfer";
					} 
					else if (mediaServer.HttpTransfers.Count == 0)
					{
						serverStatus2.Text = "No File Transfers";
					} 

					foreach (MediaServerCore.TransferStruct transfer in mediaServer.HttpTransfers)
					{
						string res = transfer.ResourceName;
						if (res.StartsWith("file://") == true) {
							res = res.Substring(7);
							res = new FileInfo(res).Name;
						}
						int imageIndex = 0;
						if (transfer.Incoming == true) imageIndex = 1;

						string positionString = "";
						if (transfer.ResourceLength < 1) 
						{
							positionString = BuildSizeString(transfer.ResourcePosition);
						}
						else
						{
							int pos = (int)(((double)((double)transfer.ResourcePosition / (double)transfer.ResourceLength) * (double)100));
							positionString = pos + "%";
						}

						transfersListView.Items.Add(new ListViewItem(new string[] {transfer.Destination.ToString(),positionString,res + " (" + transfer.ResourceLength.ToString() + " bytes)"},imageIndex));
					}
					transfersListView.EndUpdate();
				}
				else
				{
					listView.Items.Clear();
					mediaHttpTransfersUpdateId = -1;
					if (IsRunning() == false && standAloneMode == false) 
					{
						serverStatus2.Text = "Use \"Standalone Mode\" for autonomous operation";
					}
				}
			}
		}

		private void UpdateDirectoriesUI()
		{
			lock (listView)
			{
				if (IsRunning() == true || standAloneMode == true) 
				{
					if (mediaSharedDirectoryUpdateId != mediaServer.MediaSharedDirectoryUpdateId) 
					{
						/*
						int selindex = -1;
						if (listView.SelectedItems.Count > 0)
						{
							selindex = listView.SelectedIndices[0];
						}
						*/

						listView.BeginUpdate();
						listView.Items.Clear();
						mediaSharedDirectoryUpdateId = mediaServer.MediaSharedDirectoryUpdateId;
						IList dirList = mediaServer.GetSharedDirectories();
						if (dirList != null)
						{
							foreach (MediaServerCore.SharedDirectoryInfo dir in dirList) 
							{
								if ((dir.directory == null) || (dir.directory == ""))
								{
									throw new Exception("null directory");
								}
								else
								{
									DirectoryInfo dirinfo = new DirectoryInfo(dir.directory);
									string accessString = "";

									if (dir.restricted == true) 
									{
										accessString += "Restricted";
									} 
									else 
									{
										accessString += "Updatable";
									}
									if (dir.readOnly == true) 
									{
										accessString += ", ReadOnly";
									} 
									else 
									{
										accessString += ", Writable";
									}
									if (accessString == "") accessString = "None";

									ListViewItem lv = new ListViewItem(new string[] {dirinfo.Name,accessString,dirinfo.FullName},1);
									lv.Tag = dir;
									listView.Items.Add(lv);
								}
							}
						}
						listView.EndUpdate();
					}
				} 
				else 
				{
					listView.Items.Clear();
					mediaSharedDirectoryUpdateId = -1;
				}
			}
		}

		private bool IsRunning() 
		{
			if (mediaServer == null || serviceController == null) return false;
			ServiceController serviceController2 = new ServiceController("AV Media Server");
			serviceStatus = serviceController2.Status;
			return (serviceStatus == ServiceControllerStatus.Running);
		}

		private void UpdateServiceUI() 
		{
			if (mediaServer != null && standAloneMode == true) 
			{
				serverStatus1.Text = "Serving " + mediaServer.TotalFileCount + " Files in " + mediaServer.TotalDirectoryCount + " Directories";
				serverStatus2.Text = "";
				UpdateDirectoriesUI();
				return;
			}

			if (serviceController == null) 
			{
				try 
				{
					serviceController = new ServiceController("AV Media Server");
					serviceStatus = serviceController.Status;
				} 
				catch (System.InvalidOperationException) 
				{
					serverStatus1.Text = "Media Server Service Not Installed";
					serverStatus2.Text = "Use \"Standalone Mode\" for autonomous operation";
					serviceController = null;
					serviceStatus = ServiceControllerStatus.Stopped;
					return;
				}
			}

			ServiceController serviceController2 = new ServiceController("AV Media Server");
			serviceStatus = serviceController2.Status;
			switch (serviceStatus) 
			{
				case ServiceControllerStatus.Running:
					if (mediaServer == null) 
					{
						serverStatus1.Text = "Connecting...";
						serverStatus2.Text = "";
						Connect();
					} 
					else 
					{
						serverStatus1.Text = "Media Server Serving "+mediaServer.TotalFileCount+" Files in "+mediaServer.TotalDirectoryCount+" Directories";
						serverStatus2.Text = "";
					}
					break;
				case ServiceControllerStatus.Stopped:
					serverStatus1.Text = "Media Server Service is Stopped";
					serverStatus2.Text = "";
					break;
				default:
					serverStatus1.Text = "Media Server Service is " + serviceStatus.ToString();
					serverStatus2.Text = "";
					break;
			}
			serviceController2.Close();
			serviceController2.Dispose();
			serviceController2 = null;
			UpdateDirectoriesUI();
		}

		private void MediaServerCoreDebugSink(UPnPMediaServer sender, string msg) 
		{
			OpenSource.Utilities.EventLogger.Log(sender,System.Diagnostics.EventLogEntryType.Information,msg);
		}

		private void removeDirMenuItem_Click(object sender, System.EventArgs e)
		{
			lock(listView) 
			{
				if (listView.SelectedItems.Count != 1) return;
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
				mainStatusBar.Text = "Removing Sharing Directory " + selectedDirectory.directory;
				bool result = mediaServer.RemoveDirectory(new DirectoryInfo(selectedDirectory.directory));
				mainStatusBar.Text = "";
				if (result == false) MessageBox.Show(this,"Remove Shared Directory Failed");
			}
		}

		private void viewTransfersPanelMenuItem_Click(object sender, System.EventArgs e)
		{
			viewTransfersPanelMenuItem.Checked = !viewTransfersPanelMenuItem.Checked;
			transfersSplitter.Visible = viewTransfersPanelMenuItem.Checked;		
			transfersListView.Visible = viewTransfersPanelMenuItem.Checked;
		}

		private void transferTimer_Tick(object sender, System.EventArgs e)
		{
			UpdateServiceUI();
			UpdateTransfersUI();
		}

		private void openMenuItem_Click(object sender, System.EventArgs e)
		{
			if (listView.SelectedItems.Count == 1) 
			{
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
                try
                {
                    System.Diagnostics.Process.Start(selectedDirectory.directory);
                }
                catch (System.ComponentModel.Win32Exception) { }
			}
		}

		private void listView_DoubleClick(object sender, System.EventArgs e)
		{
			if (listView.SelectedItems.Count == 1) 
			{
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
                try
                {
                    System.Diagnostics.Process.Start(selectedDirectory.directory);
                }
                catch (System.ComponentModel.Win32Exception) { }
			}
		}

		private void startServerMenuItem_Click(object sender, System.EventArgs e)
		{
			serviceController.Start();
		}

		private void stopServerMenuItem_Click(object sender, System.EventArgs e)
		{
			Disconnect();
			serviceController.Stop();
		}

		private void Connect() 
		{
			if (mediaServer != null) Disconnect();

			if (registeredServerType == false) 
			{
				OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"RegisterWellKnownClientType");
				RemotingConfiguration.RegisterWellKnownClientType(
					typeof(UPnPMediaServer),
					"http://localhost:12329/UPnPMediaServer/UPnPMediaServer.soap"
					);
				registeredServerType = true;
			}

			mediaServer = new UPnPMediaServer();

			OpenSource.Utilities.EventLogger.Log(this,System.Diagnostics.EventLogEntryType.Information,"New UPnPMediaServer");

			/*
			mediaServer.OnStatsChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaServerStatsChangedSink);
			mediaServer.OnHttpTransfersChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaHttpTransferssChangedSink);
			mediaServer.OnDebugMessage += new MediaServerCore.MediaServerCoreDebugHandler(MediaServerCoreDebugSink);
			*/
		}

		private void Disconnect() 
		{
			/*
			mediaServer.OnStatsChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaServerStatsChangedSink);
			mediaServer.OnHttpTransfersChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaHttpTransferssChangedSink);
			mediaServer.OnDebugMessage += new MediaServerCore.MediaServerCoreDebugHandler(MediaServerCoreDebugSink);
			*/
			mediaServer = null;
		}

		private void directoryContextMenu_Popup(object sender, System.EventArgs e)
		{
			if (listView.SelectedItems.Count == 1) 
			{
				foreach (MenuItem m in directoryContextMenu.MenuItems) {m.Visible = true;}
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
				restrictedMenuItem.Checked   = selectedDirectory.restricted;
				readOnlyMenuItem.Checked = selectedDirectory.readOnly;
			}
			else
			{
				foreach (MenuItem m in directoryContextMenu.MenuItems) {m.Visible = false;}
				addDir2MenuItem.Visible = true;
			}
		}

		private void serviceMenuItem_Popup(object sender, System.EventArgs e)
		{
			if (standAloneMode == true) 
			{
				startServerMenuItem.Enabled = false;
				stopServerMenuItem.Enabled = false;
			} 
			else 
			{
				startServerMenuItem.Enabled = (serviceStatus == ServiceControllerStatus.Stopped || serviceStatus == ServiceControllerStatus.Paused);
				stopServerMenuItem.Enabled = (serviceStatus == ServiceControllerStatus.Running);
			}
		}

		private void sharingMenuItem_Popup(object sender, System.EventArgs e)
		{
			if (listView.SelectedItems.Count == 1) 
			{
				removeDirMenuItem.Enabled = true;
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
				restricted2MenuItem.Enabled = true;
				restricted2MenuItem.Checked = selectedDirectory.restricted;
				readOnly2MenuItem.Enabled = true;
				readOnly2MenuItem.Checked = selectedDirectory.readOnly;
			} 
			else 
			{
				removeDirMenuItem.Enabled = false;
				restricted2MenuItem.Checked = false;
				restricted2MenuItem.Enabled = false;
				readOnly2MenuItem.Enabled = false;
				readOnly2MenuItem.Checked = false;
			}
		}

		private const string TxtStartServer = "Start Server";
		private const string TxtStopServer = "Stop Server";

		private string BuildSizeString(long size) 
		{
			double sized = (double)size;
			if (sized < 1200) return size.ToString() + " b";
			if (sized < 1200000) return Math.Round(sized/1024,1).ToString() + " Kb";
			return Math.Round((sized/1024)/1024,1).ToString() + " Mb";
		}

		private void showDebugInfoMenuItem_Click(object sender, System.EventArgs e)
		{
			OpenSource.Utilities.InstanceTracker.Display();
		}

		private void pauseServerMenuItem_Click(object sender, System.EventArgs e)
		{
			this.mediaServerCore.ChangePauseState();
			pauseServerMenuItem.Checked = this.mediaServerCore.IsPaused;
		}

		private void addDirMenuItem_Click(object sender, System.EventArgs e)
		{
            if (folderBrowserDialog.ShowDialog(this) == System.Windows.Forms.DialogResult.OK) 
			{
                DirectoryInfo dirInfo = new DirectoryInfo(folderBrowserDialog.SelectedPath);
				string path = dirInfo.FullName;
				if (Directory.Exists(path) == true) 
				{
					mainStatusBar.Text = "Sharing Directory " + dirInfo.FullName;
					OpenSource.Utilities.EventLogger.Log(this, System.Diagnostics.EventLogEntryType.Information,"Adding Shared Directory: " + path);
					Exception error = mediaServer.AddDirectory(dirInfo, true, false);
					mainStatusBar.Text = "";
					if (error != null) MessageBox.Show(this.GetExceptionMessage(error), "Add Shared Directory Failure");
				}
			}
		}

		private void restricted2MenuItem_Click(object sender, System.EventArgs e)
		{
			lock(listView) 
			{
				if (listView.SelectedItems.Count != 1) return;
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
				mainStatusBar.Text = "Changing Permissions On Sharing Directory " + selectedDirectory.directory;
				bool result = mediaServer.UpdatePermissions(new DirectoryInfo(selectedDirectory.directory),!selectedDirectory.restricted,selectedDirectory.readOnly);
				mainStatusBar.Text = "";
				if (result == false) MessageBox.Show(this,"Remove Shared Directory Failed");
				UpdateDirectoriesUI();
			}
		}

		private void readOnly2MenuItem_Click(object sender, System.EventArgs e)
		{
			lock(listView) 
			{
				if (listView.SelectedItems.Count != 1) return;
				MediaServerCore.SharedDirectoryInfo selectedDirectory = (MediaServerCore.SharedDirectoryInfo)(listView.SelectedItems[0].Tag);
				mainStatusBar.Text = "Changing Permissions On Sharing Directory " + selectedDirectory.directory;
				bool result = mediaServer.UpdatePermissions(new DirectoryInfo(selectedDirectory.directory),selectedDirectory.restricted,!selectedDirectory.readOnly);
				mainStatusBar.Text = "";
				if (result == false) MessageBox.Show(this,"Remove Shared Directory Failed");
				UpdateDirectoriesUI();
			}
		}

		private void menu_Deserialize_Click(object sender, System.EventArgs e)
		{
			if (this.openTreeDlg.ShowDialog() == DialogResult.OK)
			{
				BinaryFormatter formatter = new BinaryFormatter();
				FileStream fstream = null;
				try
				{
					fstream = new FileStream(this.openTreeDlg.FileName, System.IO.FileMode.OpenOrCreate);
					this.mediaServer.DeserializeTree(formatter, fstream);
					this.UpdateDirectoriesUI();
				}
				catch (Exception error)
				{
					StringBuilder sb = new StringBuilder();
					Exception ce = error;
					while (ce != null)
					{
						sb.AppendFormat("{0}\r\n{1}\r\n{2}\r\n\r\n", ce.Source, ce.Message, ce.StackTrace);
						ce = ce.InnerException;
					}
					MessageBox.Show(sb.ToString(), "Deserialization Error");
				}
				finally
				{
					if (fstream != null)
					{
						fstream.Close();
					}
				}
			}
		}

		private string GetExceptionMessage(Exception error)
		{
			StringBuilder sb = new StringBuilder();
			Exception ce = error;
			while (ce != null)
			{
				sb.AppendFormat("{0}\r\n{1}\r\n{2}\r\n\r\n", ce.Source, ce.Message, ce.StackTrace);
				ce = ce.InnerException;
			}

			return sb.ToString();
		}

		private void menu_Serialize_Click(object sender, System.EventArgs e)
		{
			if (this.saveTreeDlg.ShowDialog() == DialogResult.OK)
			{
				BinaryFormatter formatter = new BinaryFormatter();
				FileStream fstream = null;
				try
				{
					fstream = new FileStream(this.saveTreeDlg.FileName, System.IO.FileMode.OpenOrCreate);
					this.mediaServer.SerializeTree(formatter, fstream);
				}
				catch (Exception error)
				{
					MessageBox.Show(this.GetExceptionMessage(error), "Serialization Error");
				}
				finally
				{
					if (fstream != null)
					{
						fstream.Close();
					}
				}
			}
		}

		private void closeHierarchy_Click(object sender, System.EventArgs e)
		{
			if (MessageBox.Show(this,"Stop sharing all content?","Media Server",MessageBoxButtons.OKCancel,MessageBoxIcon.Question) == DialogResult.OK)
			{
				string[] dirs = this.mediaServer.GetSharedDirectoryNames();
				bool error = false;
				foreach (string dir in dirs)
				{
					if (this.mediaServer.RemoveDirectory(new DirectoryInfo(dir)) == false)
					{
						error = true;
					}
				}

				if (error)
				{
					MessageBox.Show("An error occurred when closing the loaded tree.", "Close Tree Error");
				}
				else
				{
					this.mediaServer.ResetTree();
				}
			}
		}

		private void helpMenuItem2_Click(object sender, System.EventArgs e)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaServer");
		}

		private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaServer");
		}

		private void menuItem2_Click_1(object sender, System.EventArgs e)
		{
			ConfigForm cf = new ConfigForm(this.mediaServerCore.SearchCapabilities, this.mediaServerCore.SortCapabilities);
			if (DialogResult.OK == cf.ShowDialog())
			{
				this.mediaServerCore.SearchCapabilities = cf.SearchCapabilities;
				this.mediaServerCore.SortCapabilities = cf.SortCapabilities;
			}
		}

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            mediaServerCore.Stop();
        }

	}
}
