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
using System.Data;
using System.Text;
using System.Timers;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for MainForm.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private TreeNode rendererRootNode = new TreeNode("Media Renderers",0,0);
		private TreeNode cdsRootNode = new TreeNode("Content Directories",0,0);
		private Hashtable rendererFormTable = new Hashtable();
		private MediaPropertyForm mediaPropertyForm = null;
		private Point dragStartPoint = Point.Empty;
		private Hashtable PlayToRendererMenuItemMapping = new Hashtable();
		private object listViewSelectedObject = null;
		private RendererControl dockedRendererControl = null;
		private MediaPropertyControl mediaPropertyControl = null;
		private System.Timers.Timer m_SpiderTimer;
		private object LockGuiListing = new object();
		private Hashtable m_ContainerToSpider = new Hashtable();
        private Hashtable ForceDeviceList = new Hashtable();
		
		private static IMediaComparer MatchNever = new MatchOnNever();
		private static IMediaComparer MatchContainers = new MatchOnContainers();
		private static IMediaComparer MatchAll = new MatchOnAny();
		
		private ICpContainer m_SelectedContainer = null;

		public delegate void RootContainerChangedHandler(CpRootContainer rootContainer);
		public delegate void UpdateContainerHandler(TreeNode node, MediaContainer mediaContainer);
		public delegate void RendererListChangedHandler(AVRenderer AVM);
		
		private delegate void Delegate_UpdateItemView(CdsSpider spider, IList mediaObjects);

		private Hashtable rootContainers = new Hashtable();
		private Hashtable knownContainers = new Hashtable();
		private ContainerDiscovery containerDiscovery;
		private AVRendererDiscovery rendererDiscovery;

		private System.Windows.Forms.StatusBar statusBar;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.ImageList treeImageList;
		private System.Windows.Forms.TreeView deviceTree;
		private System.Windows.Forms.ListView listInfo;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.ContextMenu listInfoContextMenu;
		private System.Windows.Forms.MenuItem copyValueCpMenuItem;
		private System.Windows.Forms.MenuItem copyTableCpMenuItem;
        private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MainMenu mainMenu;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.MenuItem menuItem18;
		private System.Windows.Forms.MenuItem rescanNetworkMenuItem;
		private System.Windows.Forms.MenuItem expandAllMenuItem2;
		private System.Windows.Forms.MenuItem collapseAllMenuItem2;
		private System.Windows.Forms.MenuItem menuItem21;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.MenuItem expandAllMenuItem;
		private System.Windows.Forms.MenuItem collapseAllMenuItem;
		private System.Windows.Forms.MenuItem menuItem16;
		private System.Windows.Forms.MenuItem viewStatusbarMenuItem;
		private System.Windows.Forms.ContextMenu treeContextMenu;
		private System.Windows.Forms.MenuItem forceRefreshMenuItem;
		private System.Windows.Forms.Panel eventPanel;
		private System.Windows.Forms.Splitter splitter2;
		private System.Windows.Forms.ListView eventListView;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.MenuItem eventPanelMenuItem;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ListView mediaListView;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.ColumnHeader columnHeader7;
		private System.Windows.Forms.MenuItem propertiesMenuItem;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.ColumnHeader columnHeader8;
		private System.Windows.Forms.MenuItem rendererControlsMenuItem;
		private System.Windows.Forms.ContextMenu mediaContextMenu;
		private System.Windows.Forms.MenuItem mediaDisplayPropMenuItem;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.MenuItem mediaPropertiesMenuItem;
		private System.Windows.Forms.MenuItem closeInstanceMenuItem;
		private System.Windows.Forms.MenuItem menuItem14;
		private System.Windows.Forms.MenuItem displayContainerProperties;
		private System.Windows.Forms.MenuItem showDebugInfoMenuItem;
		private System.Windows.Forms.Splitter mediaPropertyControlSplitter;
		private System.Windows.Forms.MenuItem deleteMediaMenuItem;
		private System.Windows.Forms.MenuItem extendedM3UMenuItem;
		private System.Windows.Forms.MenuItem helpMenuItem;
		private System.Windows.Forms.MenuItem menuItem8;
        private MenuItem menuItem6;
        private MenuItem menuItem11;
		private System.ComponentModel.IContainer components;

		public MainForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			dockedRendererControl = new RendererControl();
			dockedRendererControl.Visible = false;
			dockedRendererControl.Dock = DockStyle.Top;

			mediaPropertyControl = new MediaPropertyControl();
			mediaPropertyControl.Visible = false;
			mediaPropertyControl.Dock = DockStyle.Bottom;
			mediaPropertyControlSplitter = new System.Windows.Forms.Splitter();
			mediaPropertyControlSplitter.Visible = false;
			mediaPropertyControlSplitter.Dock = DockStyle.Bottom;

			this.Controls.Add(dockedRendererControl);
			this.Controls.Add(mediaPropertyControl);
			this.Controls.Add(mediaPropertyControlSplitter);
			this.Controls.SetChildIndex(dockedRendererControl,0);
			this.Controls.SetChildIndex(mediaPropertyControl,0);
			this.Controls.SetChildIndex(mediaPropertyControlSplitter,0);
			this.Controls.SetChildIndex(listInfo,0);
			this.Controls.SetChildIndex(mediaListView,0);
			
			deviceTree.Nodes.Add(rendererRootNode);
			containerDiscovery = ContainerDiscovery.GetInstance();
			cdsRootNode.Tag = containerDiscovery.AllRoots;
			deviceTree.Nodes.Add(cdsRootNode);
			containerDiscovery.AllRoots.OnContainerChanged += new CpRootContainer.Delegate_OnContainerChanged(ContainerChangedSink);

			eventPanel.Visible = false;
			splitter2.Visible = false;

			rendererDiscovery = new AVRendererDiscovery((new AVRendererDiscovery.DiscoveryHandler(RendererAddedSink)));
			rendererDiscovery.OnRendererRemoved += new AVRendererDiscovery.DiscoveryHandler(new AVRendererDiscovery.DiscoveryHandler(RendererRemovedSink));
			
			this.m_SpiderTimer = new System.Timers.Timer();
			this.m_SpiderTimer.Interval = 10000;
			this.m_SpiderTimer.Elapsed += new System.Timers.ElapsedEventHandler(this.OnTimeToClearSpiderCache);
			this.m_SpiderTimer.Enabled = true;
		}

		private void OnTimeToClearSpiderCache(object source, ElapsedEventArgs e)
		{
            if (InvokeRequired) { Invoke(new System.Timers.ElapsedEventHandler(OnTimeToClearSpiderCache), source, e); return; }

            if (InvokeRequired == true)
            {
                int tt = 5;
            }

			lock (this.m_ContainerToSpider)
			{
				TimeSpan ts = new TimeSpan(0,0,0,10,0);
				DateTime dt = DateTime.Now.Subtract(ts);
				ArrayList removeThese = new ArrayList();
				Hashtable cache = new Hashtable();
                if (this.m_ContainerToSpider == null)
                {
                    return;
                }
				foreach (CpMediaContainer mc in this.m_ContainerToSpider.Keys)
				{
                    if (deviceTree.SelectedNode != null && deviceTree.SelectedNode.Tag != null && deviceTree.SelectedNode.Tag.GetType() == typeof(IMediaContainer))
                    {
					    IMediaContainer selected = (IMediaContainer)deviceTree.SelectedNode.Tag;
					    if (mc != selected)
					    {
						    CdsSpider spider = (CdsSpider) this.m_ContainerToSpider[mc];
						    if (spider.Comparer == MainForm.MatchAll)
						    {
							    DateTime st = (DateTime) spider.Tag;
							    if (dt > st)
							    {
								    //removeThese.Add(mc);
								    if (selected.GetDescendent(mc.ID, cache) == mc)
								    {
									    spider.Comparer = MainForm.MatchContainers;
								    }
								    else
								    {
									    //NKIDD: Seems to cause deadlocks : spider.Comparer = MainForm.MatchNever;
									    spider.Comparer = MainForm.MatchContainers;
								    }
							    }
						    }
					    }
                    }
				}

				foreach (CpMediaContainer mc in removeThese)
				{
					CdsSpider spider = (CdsSpider) this.m_ContainerToSpider[mc];
					spider.MonitorThis = null;
					this.m_ContainerToSpider.Remove(mc);
				}
			}
			System.GC.GetTotalMemory(true);
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
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            ""}, -1, System.Drawing.SystemColors.WindowText, System.Drawing.SystemColors.Window, new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0))));
            this.deviceTree = new System.Windows.Forms.TreeView();
            this.treeContextMenu = new System.Windows.Forms.ContextMenu();
            this.rendererControlsMenuItem = new System.Windows.Forms.MenuItem();
            this.closeInstanceMenuItem = new System.Windows.Forms.MenuItem();
            this.forceRefreshMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem18 = new System.Windows.Forms.MenuItem();
            this.expandAllMenuItem2 = new System.Windows.Forms.MenuItem();
            this.collapseAllMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem21 = new System.Windows.Forms.MenuItem();
            this.rescanNetworkMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem14 = new System.Windows.Forms.MenuItem();
            this.displayContainerProperties = new System.Windows.Forms.MenuItem();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.treeImageList = new System.Windows.Forms.ImageList(this.components);
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.listInfo = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.listInfoContextMenu = new System.Windows.Forms.ContextMenu();
            this.propertiesMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.copyValueCpMenuItem = new System.Windows.Forms.MenuItem();
            this.copyTableCpMenuItem = new System.Windows.Forms.MenuItem();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.extendedM3UMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.expandAllMenuItem = new System.Windows.Forms.MenuItem();
            this.collapseAllMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.eventPanelMenuItem = new System.Windows.Forms.MenuItem();
            this.mediaPropertiesMenuItem = new System.Windows.Forms.MenuItem();
            this.viewStatusbarMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.showDebugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.eventPanel = new System.Windows.Forms.Panel();
            this.eventListView = new System.Windows.Forms.ListView();
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.splitter2 = new System.Windows.Forms.Splitter();
            this.mediaListView = new System.Windows.Forms.ListView();
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.mediaContextMenu = new System.Windows.Forms.ContextMenu();
            this.mediaDisplayPropMenuItem = new System.Windows.Forms.MenuItem();
            this.deleteMediaMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.eventPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // deviceTree
            // 
            this.deviceTree.BackColor = System.Drawing.Color.White;
            this.deviceTree.ContextMenu = this.treeContextMenu;
            this.deviceTree.Dock = System.Windows.Forms.DockStyle.Left;
            this.deviceTree.ImageIndex = 0;
            this.deviceTree.ImageList = this.treeImageList;
            this.deviceTree.Location = new System.Drawing.Point(0, 0);
            this.deviceTree.Name = "deviceTree";
            this.deviceTree.SelectedImageIndex = 0;
            this.deviceTree.Size = new System.Drawing.Size(256, 426);
            this.deviceTree.TabIndex = 0;
            this.deviceTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.OnSelectedItem);
            this.deviceTree.MouseDown += new System.Windows.Forms.MouseEventHandler(this.deviceTree_MouseDown);
            // 
            // treeContextMenu
            // 
            this.treeContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.rendererControlsMenuItem,
            this.closeInstanceMenuItem,
            this.forceRefreshMenuItem,
            this.menuItem18,
            this.expandAllMenuItem2,
            this.collapseAllMenuItem2,
            this.menuItem21,
            this.rescanNetworkMenuItem,
            this.menuItem14,
            this.displayContainerProperties,
            this.menuItem8});
            // 
            // rendererControlsMenuItem
            // 
            this.rendererControlsMenuItem.DefaultItem = true;
            this.rendererControlsMenuItem.Index = 0;
            this.rendererControlsMenuItem.Text = "Renderer Contols";
            this.rendererControlsMenuItem.Click += new System.EventHandler(this.rendererControlsMenuItem_Click);
            // 
            // closeInstanceMenuItem
            // 
            this.closeInstanceMenuItem.DefaultItem = true;
            this.closeInstanceMenuItem.Index = 1;
            this.closeInstanceMenuItem.Text = "Close Instance";
            this.closeInstanceMenuItem.Click += new System.EventHandler(this.closeInstanceMenuItem_Click);
            // 
            // forceRefreshMenuItem
            // 
            this.forceRefreshMenuItem.Index = 2;
            this.forceRefreshMenuItem.Text = "Force Node Refresh";
            // 
            // menuItem18
            // 
            this.menuItem18.Index = 3;
            this.menuItem18.Text = "-";
            // 
            // expandAllMenuItem2
            // 
            this.expandAllMenuItem2.Index = 4;
            this.expandAllMenuItem2.Text = "&Expand all devices";
            this.expandAllMenuItem2.Click += new System.EventHandler(this.expandAllMenuItem_Click);
            // 
            // collapseAllMenuItem2
            // 
            this.collapseAllMenuItem2.Index = 5;
            this.collapseAllMenuItem2.Text = "&Collapse all devices";
            this.collapseAllMenuItem2.Click += new System.EventHandler(this.collapseAllMenuItem_Click);
            // 
            // menuItem21
            // 
            this.menuItem21.Index = 6;
            this.menuItem21.Text = "-";
            // 
            // rescanNetworkMenuItem
            // 
            this.rescanNetworkMenuItem.Index = 7;
            this.rescanNetworkMenuItem.Text = "Rescan network";
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 8;
            this.menuItem14.Text = "-";
            // 
            // displayContainerProperties
            // 
            this.displayContainerProperties.Index = 9;
            this.displayContainerProperties.Text = "Display Properties";
            this.displayContainerProperties.Click += new System.EventHandler(this.displayContainerProperties_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 10;
            this.menuItem8.Text = "-";
            // 
            // treeImageList
            // 
            this.treeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("treeImageList.ImageStream")));
            this.treeImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.treeImageList.Images.SetKeyName(0, "");
            this.treeImageList.Images.SetKeyName(1, "");
            this.treeImageList.Images.SetKeyName(2, "");
            this.treeImageList.Images.SetKeyName(3, "");
            this.treeImageList.Images.SetKeyName(4, "");
            this.treeImageList.Images.SetKeyName(5, "");
            this.treeImageList.Images.SetKeyName(6, "");
            this.treeImageList.Images.SetKeyName(7, "");
            this.treeImageList.Images.SetKeyName(8, "");
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 426);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(824, 18);
            this.statusBar.TabIndex = 6;
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(256, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(4, 426);
            this.splitter1.TabIndex = 8;
            this.splitter1.TabStop = false;
            // 
            // listInfo
            // 
            this.listInfo.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listInfo.ContextMenu = this.listInfoContextMenu;
            this.listInfo.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listInfo.FullRowSelect = true;
            this.listInfo.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1});
            this.listInfo.Location = new System.Drawing.Point(260, 0);
            this.listInfo.Name = "listInfo";
            this.listInfo.Size = new System.Drawing.Size(564, 298);
            this.listInfo.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.listInfo.TabIndex = 10;
            this.listInfo.UseCompatibleStateImageBehavior = false;
            this.listInfo.View = System.Windows.Forms.View.Details;
            this.listInfo.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listInfo_KeyDown);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Name";
            this.columnHeader1.Width = 111;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Value";
            this.columnHeader2.Width = 350;
            // 
            // listInfoContextMenu
            // 
            this.listInfoContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.propertiesMenuItem,
            this.menuItem4,
            this.copyValueCpMenuItem,
            this.copyTableCpMenuItem});
            // 
            // propertiesMenuItem
            // 
            this.propertiesMenuItem.DefaultItem = true;
            this.propertiesMenuItem.Index = 0;
            this.propertiesMenuItem.Text = "Properties";
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 1;
            this.menuItem4.Text = "-";
            // 
            // copyValueCpMenuItem
            // 
            this.copyValueCpMenuItem.DefaultItem = true;
            this.copyValueCpMenuItem.Index = 2;
            this.copyValueCpMenuItem.Text = "Copy &Value to Clipboard";
            // 
            // copyTableCpMenuItem
            // 
            this.copyTableCpMenuItem.Index = 3;
            this.copyTableCpMenuItem.Text = "Copy &Table to Clipboard";
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem7,
            this.menuItem5});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem6,
            this.menuItem11,
            this.extendedM3UMenuItem,
            this.menuItem9,
            this.menuItem2});
            this.menuItem1.Text = "&File";
            // 
            // extendedM3UMenuItem
            // 
            this.extendedM3UMenuItem.Checked = true;
            this.extendedM3UMenuItem.Index = 2;
            this.extendedM3UMenuItem.Text = "&Extended M3U Support";
            this.extendedM3UMenuItem.Click += new System.EventHandler(this.extendedM3UMenuItem_Click);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 3;
            this.menuItem9.Text = "-";
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 4;
            this.menuItem2.Text = "E&xit";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 1;
            this.menuItem7.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.expandAllMenuItem,
            this.collapseAllMenuItem,
            this.menuItem16,
            this.eventPanelMenuItem,
            this.mediaPropertiesMenuItem,
            this.viewStatusbarMenuItem});
            this.menuItem7.Text = "&View";
            // 
            // expandAllMenuItem
            // 
            this.expandAllMenuItem.Index = 0;
            this.expandAllMenuItem.Text = "&Expand all devices";
            this.expandAllMenuItem.Click += new System.EventHandler(this.expandAllMenuItem_Click);
            // 
            // collapseAllMenuItem
            // 
            this.collapseAllMenuItem.Index = 1;
            this.collapseAllMenuItem.Text = "&Collapse all devices";
            this.collapseAllMenuItem.Click += new System.EventHandler(this.collapseAllMenuItem_Click);
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 2;
            this.menuItem16.Text = "-";
            // 
            // eventPanelMenuItem
            // 
            this.eventPanelMenuItem.Index = 3;
            this.eventPanelMenuItem.Text = "Event Panel";
            this.eventPanelMenuItem.Click += new System.EventHandler(this.eventPanelMenuItem_Click);
            // 
            // mediaPropertiesMenuItem
            // 
            this.mediaPropertiesMenuItem.Index = 4;
            this.mediaPropertiesMenuItem.Text = "Media Properties Panel";
            this.mediaPropertiesMenuItem.Click += new System.EventHandler(this.mediaPropertiesMenuItem_Click);
            // 
            // viewStatusbarMenuItem
            // 
            this.viewStatusbarMenuItem.Checked = true;
            this.viewStatusbarMenuItem.Index = 5;
            this.viewStatusbarMenuItem.Text = "Status &bar";
            this.viewStatusbarMenuItem.Click += new System.EventHandler(this.viewStatusbarMenuItem_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 2;
            this.menuItem5.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem10,
            this.showDebugInfoMenuItem});
            this.menuItem5.Text = "&Help";
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            this.helpMenuItem.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem.Text = "Help Topics";
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 1;
            this.menuItem10.Text = "-";
            // 
            // showDebugInfoMenuItem
            // 
            this.showDebugInfoMenuItem.Index = 2;
            this.showDebugInfoMenuItem.Text = "&Show Debug Informations";
            this.showDebugInfoMenuItem.Click += new System.EventHandler(this.showDebugInfoMenuItem_Click);
            // 
            // eventPanel
            // 
            this.eventPanel.Controls.Add(this.eventListView);
            this.eventPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.eventPanel.Location = new System.Drawing.Point(260, 298);
            this.eventPanel.Name = "eventPanel";
            this.eventPanel.Size = new System.Drawing.Size(564, 128);
            this.eventPanel.TabIndex = 11;
            // 
            // eventListView
            // 
            this.eventListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5,
            this.columnHeader3,
            this.columnHeader4});
            this.eventListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.eventListView.Location = new System.Drawing.Point(0, 0);
            this.eventListView.Name = "eventListView";
            this.eventListView.Size = new System.Drawing.Size(564, 128);
            this.eventListView.TabIndex = 0;
            this.eventListView.UseCompatibleStateImageBehavior = false;
            this.eventListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Time";
            this.columnHeader5.Width = 54;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Event Source";
            this.columnHeader3.Width = 300;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Event";
            this.columnHeader4.Width = 100;
            // 
            // splitter2
            // 
            this.splitter2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.splitter2.Location = new System.Drawing.Point(260, 294);
            this.splitter2.Name = "splitter2";
            this.splitter2.Size = new System.Drawing.Size(564, 4);
            this.splitter2.TabIndex = 12;
            this.splitter2.TabStop = false;
            // 
            // mediaListView
            // 
            this.mediaListView.AllowDrop = true;
            this.mediaListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader6,
            this.columnHeader7,
            this.columnHeader8});
            this.mediaListView.ContextMenu = this.mediaContextMenu;
            this.mediaListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mediaListView.Location = new System.Drawing.Point(260, 0);
            this.mediaListView.Name = "mediaListView";
            this.mediaListView.Size = new System.Drawing.Size(564, 298);
            this.mediaListView.SmallImageList = this.treeImageList;
            this.mediaListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.mediaListView.TabIndex = 13;
            this.mediaListView.UseCompatibleStateImageBehavior = false;
            this.mediaListView.View = System.Windows.Forms.View.Details;
            this.mediaListView.Visible = false;
            this.mediaListView.SelectedIndexChanged += new System.EventHandler(this.mediaListView_SelectedIndexChanged);
            this.mediaListView.DragDrop += new System.Windows.Forms.DragEventHandler(this.mediaListView_DragDrop);
            this.mediaListView.DragEnter += new System.Windows.Forms.DragEventHandler(this.mediaListView_DragEnter);
            this.mediaListView.DoubleClick += new System.EventHandler(this.mediaListView_DoubleClick);
            this.mediaListView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseDown);
            this.mediaListView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseMove);
            this.mediaListView.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseUp);
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Title";
            this.columnHeader6.Width = 230;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Creator";
            this.columnHeader7.Width = 170;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "Size";
            this.columnHeader8.Width = 90;
            // 
            // mediaContextMenu
            // 
            this.mediaContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.mediaDisplayPropMenuItem,
            this.deleteMediaMenuItem,
            this.menuItem3});
            this.mediaContextMenu.Popup += new System.EventHandler(this.mediaContextMenu_Popup);
            // 
            // mediaDisplayPropMenuItem
            // 
            this.mediaDisplayPropMenuItem.DefaultItem = true;
            this.mediaDisplayPropMenuItem.Index = 0;
            this.mediaDisplayPropMenuItem.Text = "&Display Properties";
            this.mediaDisplayPropMenuItem.Click += new System.EventHandler(this.mediaListView_DoubleClick);
            // 
            // deleteMediaMenuItem
            // 
            this.deleteMediaMenuItem.Index = 1;
            this.deleteMediaMenuItem.Text = "D&elete";
            this.deleteMediaMenuItem.Click += new System.EventHandler(this.deleteMediaMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 2;
            this.menuItem3.Text = "-";
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 0;
            this.menuItem6.Text = "Manually Add Device...";
            this.menuItem6.Click += new System.EventHandler(this.menuItem6_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 1;
            this.menuItem11.Text = "-";
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(824, 444);
            this.Controls.Add(this.splitter2);
            this.Controls.Add(this.mediaListView);
            this.Controls.Add(this.listInfo);
            this.Controls.Add(this.eventPanel);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.deviceTree);
            this.Controls.Add(this.statusBar);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "AV Media Controller";
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.eventPanel.ResumeLayout(false);
            this.ResumeLayout(false);

		}
		#endregion

        protected void RendererAddedSink(AVRendererDiscovery sender, AVRenderer renderer)
		{
            if (InvokeRequired) { Invoke(new AVRendererDiscovery.DiscoveryHandler(RendererAddedSink), sender, renderer); return; }

            renderer.OnRemovedConnection += new AVRenderer.ConnectionHandler(RemovedConnectionSink);
            
            TreeNode node = NodeTagSearch(rendererRootNode, renderer);
			if (node == null) 
			{
				TreeNode varNode = new TreeNode(renderer.FriendlyName,1,1);
				varNode.Tag = renderer;
				rendererRootNode.Nodes.Add(varNode);
				renderer.OnCreateConnection += new AVRenderer.ConnectionHandler(RendererCreateConnectionSink);
				renderer.OnRecycledConnection += new AVRenderer.ConnectionHandler(RendererRecycledConnectionSink);
				renderer.OnCreateConnectionFailed += new AVRenderer.FailedConnectionHandler(RendererCreateConnectionFailedSink);
				foreach (AVConnection connection in renderer.Connections) 
				{
					RendererCreateConnectionSink(renderer, connection, 0);
				}
			}
			rendererRootNode.Expand();
		}

        protected void RendererRemovedSink(AVRendererDiscovery sender, AVRenderer renderer)
		{
            if (InvokeRequired) { Invoke(new AVRendererDiscovery.DiscoveryHandler(RendererRemovedSink), sender, renderer); return; }

            renderer.OnCreateConnection -= new AVRenderer.ConnectionHandler(RendererCreateConnectionSink);

            foreach (AVConnection connection in renderer.Connections)
            {
                connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
                connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
                connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
                connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
                connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(MediaResourceChangedHandlerSink);
                connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
                connection.OnTrackChanged -= new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);

                connection.OnCurrentMetaDataChanged -= new AVConnection.CurrentMetaDataChangedHandler(MetaDataSink);
            }

			TreeNode node = NodeTagSearch(rendererRootNode,renderer);
			if (node != null) 
			{
				if (rendererFormTable.ContainsKey(node.Tag)) 
				{
					RendererControlForm form = (RendererControlForm)rendererFormTable[node.Tag];
					form.Close();
					rendererFormTable.Remove(node.Tag);
				}
				rendererRootNode.Nodes.Remove(node);
			}
		}

		protected void RendererCreateConnectionFailedSink(AVRenderer sender, AVRenderer.CreateFailedReason r, object Handle)
		{
			MessageBox.Show(this,sender.FriendlyName + " failed to create a connection. " + r.ToString());
		}

		protected void RendererRecycledConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			Object[] args = new Object[3];
			args[0] = sender;
			args[1] = r;
			args[2] = Handle;
			this.Invoke(new AVRenderer.ConnectionHandler(RendererRecycledConnectionSinkEx),args);
		}

		protected void RemovedConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			Object[] args = new Object[3];
			args[0] = sender;
			args[1] = r;
			args[2] = Handle;
			this.Invoke(new AVRenderer.ConnectionHandler(RemovedConnectionSinkEx),args);
		}

		protected void RemovedConnectionSinkEx(AVRenderer sender, AVConnection r, object Handle)
		{
			TreeNode node = NodeTagSearch(rendererRootNode,r);
			if (node != null)
			{
				node.Remove();
			}
		}
		
		protected void RendererCreateConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			Object[] args = new Object[3];
			args[0] = sender;
			args[1] = r;
			args[2] = Handle;
			this.Invoke(new AVRenderer.ConnectionHandler(RendererCreateConnectionSinkEx),args);
		}

		protected void RendererRecycledConnectionSinkEx(AVRenderer sender, AVConnection connection, object Handle)
		{
			TreeNode node = NodeTagSearch(rendererRootNode,connection);
			if (node != null)
			{
				// Connection was recycled, update it
				node.Text = connection.ConnectionID.ToString() + " - " + connection.CurrentState.ToString();
				if (listViewSelectedObject == connection) SetListInfo(connection);
			}
		}

		protected void RendererCreateConnectionSinkEx(AVRenderer sender, AVConnection connection, object Handle)
		{
			TreeNode node = NodeTagSearch(rendererRootNode,connection);
			if (node != null)
			{
				// Connection was recycled, update it
				node.Text = connection.ConnectionID.ToString() + " - " + connection.CurrentState.ToString();
				if (listViewSelectedObject == connection) SetListInfo(connection);
			}
			else 
			{
				// New connection, lets add it to the renderer
				node = NodeTagSearch(rendererRootNode,sender);
				if (node == null) MessageBox.Show(this,"Got new connection on unknown renderer");
				TreeNode varNode = new TreeNode(connection.ConnectionID.ToString() + " - " + connection.CurrentState.ToString(),1,1);
				varNode.Tag = connection;
				node.Nodes.Add(varNode);

				connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(MediaResourceChangedHandlerSink);
				connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
				connection.OnCurrentMetaDataChanged += new AVConnection.CurrentMetaDataChangedHandler(MetaDataSink);
			}
		}

		private void PlayStateChangedHandlerSink(AVConnection connection, AVConnection.PlayState NewState)
		{
            if (InvokeRequired) { Invoke(new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink), connection, NewState); return; }
			TreeNode node = NodeTagSearch(rendererRootNode, connection);
			if (node != null)  { node.Text = connection.ConnectionID.ToString() + " - " + connection.CurrentState.ToString(); }
			if (listViewSelectedObject == connection) SetListInfo(connection);
		}

		private void MuteStateChangedHandlerSink(AVConnection connection, bool NewMuteStatus)
		{
            if (InvokeRequired) { Invoke(new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink), connection, NewMuteStatus); return; }
            if (listViewSelectedObject == connection) SetListInfo(connection);
		}

		private void VolumeChangedHandlerSink(AVConnection connection, UInt16 Volume)
		{
            if (InvokeRequired) { Invoke(new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink), connection, Volume); return; }
            if (listViewSelectedObject == connection) SetListInfo(connection);
		}

		private void TrackChangedSink(AVConnection connection, UInt32 NewTrack)
		{
            if (InvokeRequired) { Invoke(new AVConnection.CurrentTrackChangedHandler(TrackChangedSink), connection, NewTrack); return; }
            if (listViewSelectedObject == connection) 
			{
				lock (listInfo) 
				{
					listInfo.Items[3].SubItems[1].Text = connection.CurrentTrack.ToString();
					listInfo.Items[13].SubItems[1].Text = connection.NumberOfTracks.ToString();
				}
			}
		}

		private void PositionChangedHandlerSink(AVConnection connection, TimeSpan time) 
		{
            if (InvokeRequired) { Invoke(new AVConnection.PositionChangedHandler(PositionChangedHandlerSink), connection, time); return; }
            if (listViewSelectedObject == connection) 
			{
				lock (listInfo) 
				{
					listInfo.Items[1].SubItems[1].Text = connection.CurrentPosition.ToString();
					listInfo.Items[4].SubItems[1].Text = connection.Duration.ToString();
				}
			}
		}

		private void MediaResourceChangedHandlerSink(AVConnection connection, IMediaResource res) 
		{
            if (InvokeRequired) { Invoke(new AVConnection.MediaResourceChangedHandler(MediaResourceChangedHandlerSink), connection, res); return; }
            if (listViewSelectedObject == connection) SetListInfo(connection);
		}

		private void RemovedConnectionHandlerSink(AVConnection connection)
		{
            if (InvokeRequired) { Invoke(new AVConnection.RendererHandler(RemovedConnectionHandlerSink), connection); return; }
            TreeNode node = NodeTagSearch(rendererRootNode, connection);
			if (node != null) node.Remove();
			connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
			connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
			connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
			connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
			connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
			connection.OnTrackChanged -= new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
			connection.OnCurrentMetaDataChanged -= new AVConnection.CurrentMetaDataChangedHandler(MetaDataSink);
		}

		private void OnSelectedItem(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			SetListInfo(deviceTree.SelectedNode.Tag);

			if (deviceTree.SelectedNode.Tag != null && deviceTree.SelectedNode.Tag.GetType() == typeof(AVConnection))
			{
				AVConnection c = (AVConnection)deviceTree.SelectedNode.Tag;
				dockedRendererControl.Connection = c;
				dockedRendererControl.Visible = true;
			} 
			else
			{
				dockedRendererControl.Visible = false;
				dockedRendererControl.Connection = null;
			}
		}

		protected void MetaDataSink(AVConnection sender)
		{
            if (InvokeRequired) { Invoke(new AVConnection.CurrentMetaDataChangedHandler(MetaDataSink), sender); return; }
			SetListInfo(listViewSelectedObject);
		}

		protected void SetListInfo(object infoObject) 
		{
			lock (listInfo)
			{
				listViewSelectedObject = infoObject;

				listInfo.BeginUpdate();

				if (infoObject == null || infoObject == containerDiscovery.AllRoots) 
				{
					listInfo.Items.Clear();
					mediaListView.Visible = false;
					mediaPropertyControl.Visible = false;
					mediaPropertyControlSplitter.Visible = false;
					mediaPropertyControl.MediaItem = null;
					listInfo.Items.Add(new ListViewItem(new string[] {"Product name", "Media Controller"}));	
					listInfo.Items.Add(new ListViewItem(new string[] {"Manufacturer", "OpenSource"}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Version", Application.ProductVersion}));
					listInfo.EndUpdate();
					return;
				}

				if (infoObject.GetType() == typeof(AVRenderer))
				{
					AVRenderer re = (AVRenderer)infoObject;
					mediaListView.Visible = false;
					mediaPropertyControl.Visible = false;
					mediaPropertyControlSplitter.Visible = false;
					mediaPropertyControl.MediaItem = null;
					listInfo.Items.Clear();
					listInfo.Items.Add(new ListViewItem(new string[] {"FriendlyName",re.FriendlyName}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Connection Count",re.Connections.Count.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Unique Device Name",re.UniqueDeviceName}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Interface",re.Interface.ToString()}));
					string ProtocolInfos = "";
					foreach (ProtocolInfoString s in re.ProtocolInfoStrings) 
					{
						if (ProtocolInfos != "") ProtocolInfos += ";";
						ProtocolInfos += s.ToString();
					}
					listInfo.Items.Add(new ListViewItem(new string[] {"Protocol Support",ProtocolInfos}));
				}

				if (infoObject.GetType() == typeof(AVConnection))
				{
					AVConnection con = (AVConnection)infoObject;
					mediaListView.Visible = false;
					mediaPropertyControl.MediaItem = null;
					mediaPropertyControlSplitter.Visible = false;
					mediaPropertyControl.Visible = false;


					listInfo.Items.Clear();
					listInfo.Items.Add(new ListViewItem(new string[] {"Connection ID",con.ConnectionID.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Current Position",con.CurrentPosition.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Current State",con.CurrentState.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Duration",con.Duration.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Muted",con.IsMute.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Supports Current Position",con.SupportsCurrentPosition.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Supports Seek",con.SupportsSeek.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Volume",con.MasterVolume.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Current Track",con.CurrentTrack.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Track Count",con.NumberOfTracks.ToString()}));
					if (con.MediaResource != null) 
					{
						if (con.MediaResource.ContentUri != null) listInfo.Items.Add(new ListViewItem(new string[] {"Res. Current URI",con.MediaResource.ContentUri.ToString()}));
						if (con.MediaResource.ImportUri != null) listInfo.Items.Add(new ListViewItem(new string[] {"Res. ImportUri",con.MediaResource.ImportUri.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Information",con.MediaResource.ProtocolInfo.Info}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Mime Type",con.MediaResource.ProtocolInfo.MimeType}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Network",con.MediaResource.ProtocolInfo.Network}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Protocol",con.MediaResource.ProtocolInfo.Protocol}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Protocol Info",con.MediaResource.ProtocolInfo.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Bitrate",con.MediaResource.Bitrate.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Bits Per Sample",con.MediaResource.BitsPerSample.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Color Depth",con.MediaResource.ColorDepth.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Duration",con.MediaResource.Duration.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Protection",con.MediaResource.Protection}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Audio Channels",con.MediaResource.nrAudioChannels.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Resolution",con.MediaResource.Resolution.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Sample Frequency",con.MediaResource.SampleFrequency.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Res. Size",con.MediaResource.Size.ToString()}));
					}
					if(con.CurrentItem!=null)
					{
						foreach(string PropName in con.CurrentItem.Properties.PropertyNames)
						{
							//string val = "";
							foreach(ICdsElement PropElement in con.CurrentItem.Properties[PropName])
							{
								listInfo.Items.Add(new ListViewItem(new string[] {"Media: " + PropName,PropElement.StringValue}));
							}
						}
					}
				}

				//				if (infoObject.GetType() == typeof(CpRootContainer)) 
				//				{
				//					CpRootContainer rc = (CpRootContainer)infoObject;
				//					mediaListView.Visible = false;
				//					mediaPropertyControl.Visible = false;
				//					mediaPropertyControlSplitter.Visible = false;
				//					mediaPropertyControl.MediaItem = null;
				//					listInfo.Items.Clear();
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Class",rc.Class.StringValue}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Creator",rc.Creator}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Full Class Name",rc.Class.StringValue}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Identifier",rc.ID}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Item Count",rc.Items.Count.ToString()}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Container Count",rc.Containers.Count.ToString()}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Server Friendly Name",rc.ServerFriendlyName}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Title",rc.Title}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Update ID",rc.UpdateID.ToString()}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Child Count",rc.ChildCount.ToString()}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Is Restricted",rc.IsRestricted.ToString()}));
				//					listInfo.Items.Add(new ListViewItem(new string[] {"Parent ID",rc.ParentID.ToString()}));
				//				}

				if (
					(infoObject.GetType() == typeof(CpMediaContainer)) ||
					(infoObject.GetType() == typeof(CpRootContainer))
					)
				{
					if (mediaPropertiesMenuItem.Checked == true) 
					{
						mediaPropertyControl.Visible = true;
						mediaPropertyControlSplitter.Visible = true;
						mediaPropertyControl.MediaItem = null;
					}
					CpMediaContainer mc = (CpMediaContainer)infoObject;

					//TODO: Look up spider in hashtable
					CdsSpider spider = this.GetSpider(mc);
					spider.Comparer = MainForm.MatchAll;

					//UpdateItemView(spider);
					this.ClearItemView(null, null);
					this.AddItemView(spider, spider.MatchedItems);
					mediaListView.Visible = true;
				}

				if (infoObject.GetType() == typeof(CpMediaItem)) 
				{
					CpMediaItem mi = (CpMediaItem)infoObject;
					mediaListView.Visible = false;
					mediaPropertyControl.Visible = false;
					mediaPropertyControlSplitter.Visible = false;
					mediaPropertyControl.MediaItem = null;
					listInfo.Items.Clear();
					listInfo.Items.Add(new ListViewItem(new string[] {"Class",mi.Class.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Creator",mi.Creator}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Full Class Name",mi.Class.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Identifier",mi.ID}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Title",mi.Title}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Reference",mi.IsReference.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Restricted",mi.IsRestricted.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Resource Count",mi.MergedResources.Length.ToString()}));
					listInfo.Items.Add(new ListViewItem(new string[] {"Properties Count",mi.MergedProperties.Count.ToString()}));
					int rc = 0;
					foreach (MediaResource res in mi.MergedResources) 
					{
						rc++;
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Content URI",res.ContentUri.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Import URI",res.ImportUri.ToString()}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Information",res.ProtocolInfo.Info}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Mime Type",res.ProtocolInfo.MimeType}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Network",res.ProtocolInfo.Network}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Protocol",res.ProtocolInfo.Protocol}));
						listInfo.Items.Add(new ListViewItem(new string[] {"Resource " + rc + " Protocol Info",res.ProtocolInfo.ToString()}));
					}
					foreach (string propertyName in mi.MergedProperties.PropertyNames) 
					{
						rc = 0;
						foreach (string propertyvalue in mi.MergedProperties[propertyName]) 
						{
							rc++;
							if (rc == 1) 
							{
								listInfo.Items.Add(new ListViewItem(new string[] {"Property " + propertyName,propertyvalue}));
							} 
							else 
							{
								listInfo.Items.Add(new ListViewItem(new string[] {"Property \"" + propertyName + "\"(" + rc + ")",propertyvalue}));
							}
						}
					}

				}

				listInfo.EndUpdate();
			}
		}

		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void viewStatusbarMenuItem_Click(object sender, System.EventArgs e)
		{
			viewStatusbarMenuItem.Checked = !viewStatusbarMenuItem.Checked;
			statusBar.Visible = viewStatusbarMenuItem.Checked;
		}

		private void expandAllMenuItem_Click(object sender, System.EventArgs e)
		{
			rendererRootNode.ExpandAll();
			cdsRootNode.ExpandAll();		
		}

		private void collapseAllMenuItem_Click(object sender, System.EventArgs e)
		{
			CollapseAll(rendererRootNode);
			CollapseAll(cdsRootNode);
		}

		private void CollapseAll(TreeNode node) 
		{
			if (node == null) return;
			node.Collapse();
			foreach (TreeNode n in node.Nodes) {CollapseAll(n);}
		}

		private void listInfo_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			/*
			if (e.Modifiers == Keys.Control && e.KeyCode == Keys.C && listInfo.SelectedItems.Count != 0) 
			{
				Clipboard.SetDataObject(listInfo.SelectedItems[0].SubItems[1].Text);				
			}
			*/
		}

		private void helpMenuItem_Click(object sender, System.EventArgs e)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaController");
		}

		private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaController");
		}

		private void deviceTree_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
            if (InvokeRequired == true)
            {
                int tt = 5;
            }

			m_SelectedContainer = null;
			ArrayList removeThese = new ArrayList();
			foreach (MenuItem mi in this.treeContextMenu.MenuItems)
			{
				if (mi.Text.StartsWith("Send to "))
				{
					removeThese.Add(mi);
				}
			}
			foreach (MenuItem mi in removeThese)
			{
				this.treeContextMenu.MenuItems.Remove(mi);
			}

			TreeNode node = deviceTree.GetNodeAt(e.X,e.Y);
			if (node == null) return;
			deviceTree.SelectedNode = node;

			rendererControlsMenuItem.Visible = false;
			closeInstanceMenuItem.Visible = false;
			forceRefreshMenuItem.Visible = false;
			menuItem18.Visible = false;
			displayContainerProperties.Visible = false;

			if (node.Tag == null) return;

			if (node.Tag.GetType() == typeof(AVRenderer))
			{
				rendererControlsMenuItem.Visible = true;
				menuItem18.Visible = true;
			}

			if (node.Tag.GetType() == typeof(AVConnection))
			{
				AVConnection connection = (AVConnection)node.Tag;
				if (connection.IsCloseSupported == true) 
				{
					closeInstanceMenuItem.Visible = true;
					menuItem18.Visible = true;
				}
			}

			if (node.Tag is ICpMedia)
			{
				displayContainerProperties.Visible = true;

				ICpContainer cpc = node.Tag as ICpContainer;

				if (cpc != null)
				{
					if (cpc.MergedResources.Length > 0)
					{
						m_SelectedContainer = cpc;
						if (rendererRootNode.Nodes.Count > 0) 
						{
							foreach (TreeNode tn in rendererRootNode.Nodes) 
							{
								AVRenderer renderer = (AVRenderer)tn.Tag;
								if (renderer != null) 
								{
									MenuItem m = new MenuItem("Send to " + renderer.FriendlyName,new EventHandler(PlayMediaMenuSelectedSink));
									PlayToRendererMenuItemMapping.Add(m,renderer);
									this.treeContextMenu.MenuItems.Add(m);
									//mediaContextMenu.MenuItems.Add(m);
								}
							}
						}
					}
				}
			}
		}

		private CdsSpider GetSpider(CpMediaContainer mc)
		{
            if (InvokeRequired == true)
            {
                int tt = 5;
            }

			CdsSpider spider;
			lock (this.m_ContainerToSpider)
			{
				spider = (CdsSpider) this.m_ContainerToSpider[mc];

				if (spider == null)
				{
					spider = new CdsSpider();
					spider.OnMatchesAdded += new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesAdded);
					spider.OnMatchesCleared += new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesCleared);
					spider.OnMatchesRemoved += new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesRemoved);
					spider.OnUpdateDone += new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesUpdated);

					if (deviceTree.SelectedNode != null)
					{
						IMediaContainer selected = deviceTree.SelectedNode.Tag as IMediaContainer;

						if (selected != null)
						{
							if (mc.Parent == selected)
							{
								spider.Comparer = MainForm.MatchContainers;
							}
							else if (mc.Parent != null)
							{
								if (
									(mc.Parent.Parent == selected) && 
									(! (selected is CpRootCollectionContainer)) && 
									(selected != null)
									)
								{
									spider.Comparer = MainForm.MatchContainers;
								}
								else
								{
									spider.Comparer = MainForm.MatchNever;
								}
							}
							else
							{
								spider.Comparer = MainForm.MatchNever;
							}
						}
						else
						{
							spider.Comparer = MainForm.MatchNever;
						}
					}
					else
					{
						spider.Comparer = MainForm.MatchNever;
					}

					spider.Sorter = null;
					spider.MonitorThis = mc;
					this.m_ContainerToSpider[mc] = spider;
				}

				spider.Tag = DateTime.Now;
			}
			return spider;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="mediaObjects"></param>
		private void Sink_SpiderOnMatchesCleared(CdsSpider sender, IList mediaObjects)
		{
            if (InvokeRequired == true) { Invoke(new CdsSpider.Delegate_OnMatchesChanged(Sink_SpiderOnMatchesCleared), sender, mediaObjects); return; }
            if (deviceTree.SelectedNode.Tag == sender.MonitorThis) ClearItemView(sender, null);
		}

		/// <summary>
		/// Clears the GUI's listing of items for the currently selected container.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="objects">should be null</param>
		private void ClearItemView(CdsSpider spider, IList emptyList)
		{
			lock (this.LockGuiListing)
			{
				mediaListView.BeginUpdate();
				mediaListView.Items.Clear();
				mediaListView.EndUpdate();
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="mediaObjects"></param>
		private void Sink_SpiderOnMatchesRemoved (CdsSpider sender, IList mediaObjects)
		{
			if (mediaObjects.Count > 0) this.Invoke(new Delegate_UpdateItemView(this.RemoveItemView), sender, mediaObjects);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="mediaObjects"></param>
		private void Sink_SpiderOnMatchesUpdated (CdsSpider sender, IList mediaObjects)
		{
            this.Invoke(new Delegate_UpdateItemView(this.UpdateItemView), sender, mediaObjects);
		}

		/// <summary>
		/// Removes from the GUI's listing of items for the currently selected container.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="removeThese"></param>
		private void RemoveItemView(CdsSpider spider, IList removeThese)
		{
			lock (this.LockGuiListing)
			{
				ArrayList removeLVI = new ArrayList(removeThese.Count);
				foreach (ListViewItem lvi in mediaListView.Items)
				{
					if (removeThese.Contains(lvi.Tag))
					{
						removeLVI.Add(lvi);
					}
				}

				if (removeLVI.Count > 0)
				{
					mediaListView.BeginUpdate();
					foreach (ListViewItem lvi in removeLVI)
					{
						mediaListView.Items.Remove(lvi);
					}
					mediaListView.EndUpdate();
				}
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="mediaObjects"></param>
		private void Sink_SpiderOnMatchesAdded(CdsSpider sender, IList mediaObjects)
		{
            if (InvokeRequired == true) { Invoke(new CdsSpider.Delegate_OnMatchesChanged(Sink_SpiderOnMatchesAdded), sender, mediaObjects); return; }
            if (deviceTree.SelectedNode.Tag == sender.MonitorThis) AddItemView(sender, mediaObjects);
		}

		private void UpdateItemView(CdsSpider spider, IList ignore)
		{
			lock (this.LockGuiListing)
			{
				ArrayList al = new ArrayList((ICollection) mediaListView.Items);
				mediaListView.BeginUpdate();
				foreach (ListViewItem lvi in al)
				{
					CpMediaItem item = (CpMediaItem) lvi.Tag;
					string artist = "";
					if (item.MergedProperties[CommonPropertyNames.creator] != null && item.MergedProperties[CommonPropertyNames.creator].Count > 0)
					{
						ICdsElement propValue = (ICdsElement) item.MergedProperties[CommonPropertyNames.creator][0];
						artist = propValue.StringValue;
					}
					string size = "";
					if (item.MergedResources.Length > 0)
					{
						MediaResource res = (MediaResource)item.MergedResources[0];
						if (res.Size.IsValid)
						{
							size = BuildSizeString((long) res.Size.m_Value);//BuildSizeString(long.Parse(res.Size.ToString()));
						}
					}
					int icon = 4;
					string classtype = item.Class.ToString();
					switch (classtype) 
					{
						case "object.item":
							icon = 8;
							break;
						case "object.item.imageItem":
						case "object.item.imageItem.photo":
							icon = 7;
							break;
						case "object.item.videoItem":
						case "object.item.videoItem.movie":
							icon = 6;
							break;
						case "object.item.audioItem":
						case "object.item.audioItem.musicTrack":
							icon = 5;
							break;
					}
					lvi.SubItems[0].Text = item.Title;
					lvi.SubItems[1].Text = artist;
					lvi.SubItems[2].Text = size;
					lvi.ImageIndex = icon;

					if (mediaPropertyForm != null)
					{
						if (item == this.mediaPropertyForm.MediaObj)
						{
							this.mediaPropertyForm.MediaObj = item;
						}
					}
				}
				mediaListView.EndUpdate();
			}
		}

		/// <summary>
		/// Adds to the GUI's listing of items for the currently selected container.
		/// </summary>
		/// <param name="spider"></param>
		/// <param name="addThese"></param>
		private void AddItemView(CdsSpider spider, IList addThese)
		{
			lock (this.LockGuiListing)
			{
				mediaListView.BeginUpdate();
				foreach (ICpMedia media in addThese)
				{
					if (media.IsItem)
					{
						CpMediaItem item = (CpMediaItem) media;
						string artist = "";
						if (item.MergedProperties[CommonPropertyNames.creator] != null && item.MergedProperties[CommonPropertyNames.creator].Count > 0)
						{
							ICdsElement propValue = (ICdsElement) item.MergedProperties[CommonPropertyNames.creator][0];
							artist = propValue.StringValue;
						}
						string size = "";
						if (item.MergedResources.Length > 0)
						{
							MediaResource res = (MediaResource)item.MergedResources[0];
							if (res.Size.IsValid)
							{
								size = BuildSizeString((long) res.Size.m_Value);//BuildSizeString(long.Parse(res.Size.ToString()));
							}
						}
						int icon = 4;
						string classtype = item.Class.ToString();
						switch (classtype) 
						{
							case "object.item":
								icon = 8;
								break;
							case "object.item.imageItem":
							case "object.item.imageItem.photo":
								icon = 7;
								break;
							case "object.item.videoItem":
							case "object.item.videoItem.movie":
								icon = 6;
								break;
							case "object.item.audioItem":
							case "object.item.audioItem.musicTrack":
								icon = 5;
								break;
						}
						ListViewItem l = new ListViewItem(new string[] {item.Title,artist,size},icon);
						l.Tag = item;
						mediaListView.Items.Add(l);
					}
				}
				mediaListView.EndUpdate();
			}
		}

		private void ContainerChangedSink(CpRootContainer sender, CpMediaContainer thisChanged) 
		{
            if (InvokeRequired) { this.Invoke(new CpRootContainer.Delegate_OnContainerChanged(ContainerChangedSink), sender, thisChanged); return; }

            // Update the tree view
            this.GetSpider(thisChanged);

            // Throw spiders on every child container
            foreach (CpMediaContainer cpc in thisChanged.Containers)
            {
                this.GetSpider(cpc);
            } 
            
            try 
			{
				lock (this.m_ContainerToSpider)
				{

					TreeNode node = NodeTagSearch(cdsRootNode,thisChanged);
					if (node == null) 
					{
						//Update on unknown node
						Event("Container " + thisChanged.Title,"Unknown Container Changed");
					} 
					else 
					{
						Event("Container " + thisChanged.Title,"Changed");

						// get the curren listing of child containers for the 
						// container reporting the change
						IList newcontainers = thisChanged.Containers;
						if (newcontainers != null) 
						{
							ArrayList noderemovelist = new ArrayList();
							ArrayList existingcontainers = new ArrayList();

							// iterate through the treeview's representation
							// of the last known set of child containers
							foreach (TreeNode n in node.Nodes) 
							{
								// get the container associated with the node
								CpMediaContainer c = (CpMediaContainer) n.Tag;

								if (newcontainers.Contains(c) == false)
								{
									// If the node's container is no longer associated
									// with the container, then the node should be removed.
									noderemovelist.Add(n);
								}
								else
								{
									// If the node's container is still associated with
									// the container, then the container and node
									// should remain.
									existingcontainers.Add(c);
								}
							}

							foreach (TreeNode n in noderemovelist) 
							{
								this.CleanupRemovedNode(n);
								
								// Remove the subtree of tree nodes.
								// For some reason, we need to remove
								// all of the nodes after we clean up.
								// I had memory leaks when the nodes
								// were removed from within the code
								// for CleanupRemovedNode()
								n.Remove();
							}

							foreach (CpMediaContainer c in newcontainers) 
							{
								// Iterate through the new list of containers, but recurse
								// and update the treeview only with containers that
								// are brand new.
								if (existingcontainers.Contains(c) == false) 
								{
									TreeNode cNode;
									// Depending on the type of container, ensure
									// that we use the right icon in the treeview.
									if (c.GetType() == typeof(CpRootContainer)) 
									{
										cNode = new TreeNode(c.Title,1,1);
									}
									else
									{
										cNode = new TreeNode(c.Title,2,3);
									}
									cNode.Tag = c;
									node.Nodes.Add(cNode);
									ContainerChangedSink(null,c);
								}
							}
						}
					}

					cdsRootNode.Expand();
				}
			} 
			catch (Exception ex) 
			{
				MessageBox.Show(ex.ToString());
			}
		}

		private void CleanupRemovedNode(TreeNode node)
		{
			CpMediaContainer c = (CpMediaContainer) node.Tag;
			
			// if the Tag has been nulled, then we've
			// already cleaned up the node in another
			// thread so we need not do anything.

			if (c != null)
			{
				node.Tag = null;
				CdsSpider spider = (CdsSpider) this.m_ContainerToSpider[c];
				spider.OnMatchesAdded -= new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesAdded);
				spider.OnMatchesCleared -= new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesCleared);
				spider.OnMatchesRemoved -= new CdsSpider.Delegate_OnMatchesChanged(this.Sink_SpiderOnMatchesRemoved);
				this.m_ContainerToSpider.Remove(c);
				foreach (TreeNode n in node.Nodes)
				{
					this.CleanupRemovedNode(n);
				}
			}
			else
			{
				node.Tag = null;
			}
		}

		private TreeNode NodeTagSearch(TreeNode node,object searchTag) 
		{
			if (node.Tag != null && node.Tag == searchTag) return node;
			TreeNode r = null;
			foreach (TreeNode n in node.Nodes) 
			{
				r = NodeTagSearch(n,searchTag);
				if (r != null) return r;
			}
			return null;
		}

		private void Event(string eventSource, string eventMsg)
		{
			lock (eventListView) 
			{
				if (eventMsg == "") eventMsg = "(Empty)";
				DateTime now = DateTime.Now;
				ListViewItem l = new ListViewItem(new string[] {now.ToShortTimeString(),eventSource,eventMsg});
				l.Tag = now;
				eventListView.Items.Insert(0,l);
				statusBar.Text = eventSource + " - " + eventMsg;
			}
		}

		private void eventPanelMenuItem_Click(object sender, System.EventArgs e)
		{
			eventPanelMenuItem.Checked = !eventPanelMenuItem.Checked;
			splitter2.Visible = eventPanelMenuItem.Checked;
			eventPanel.Visible = eventPanelMenuItem.Checked;
		}

		private void mediaListView_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if (mediaListView.SelectedItems.Count == 0) 
			{
				mediaPropertyControl.MediaItem = null;
				return;
			}

			ListViewItem l = mediaListView.SelectedItems[0];
			if (l.Tag.GetType() != typeof(CpMediaItem)) 
			{
				mediaPropertyControl.MediaItem = null;
				return;
			}

			mediaPropertyControl.MediaItem = (CpMediaItem)l.Tag;
		}

		private void mediaListView_DoubleClick(object sender, System.EventArgs e)
		{
			if (mediaListView.SelectedItems.Count != 1) return;
			
			if (mediaListView.SelectedItems[0].Tag.GetType() == typeof(CpMediaItem)) 
			{
				CpMediaItem mi = (CpMediaItem)mediaListView.SelectedItems[0].Tag;
				PopupMediaPropertyDialog(mi);
			}
		}

		public void PopupMediaPropertyDialog(ICpMedia media) 
		{
			if (mediaPropertyForm == null || mediaPropertyForm.IsDisposed == true) 
			{
				mediaPropertyForm = new MediaPropertyForm();
			}
			mediaPropertyForm.MediaObj = media;
			mediaPropertyForm.Show();
			mediaPropertyForm.Activate();
		}

		private string BuildSizeString(long size) 
		{
			double sized = (double)size;
			if (sized < 1200) return size.ToString() + " b";
			if (sized < 1200000) return Math.Round(sized/1024,1).ToString() + " Kb";
			return Math.Round((sized/1024)/1024,1).ToString() + " Mb";
		}

		private void rendererControlsMenuItem_Click(object sender, System.EventArgs e)
		{
            if (InvokeRequired == true)
            {
                int tt = 5;
            }

			TreeNode node = deviceTree.SelectedNode;
			if (node == null || node.Tag == null) return;

			if (node.Tag.GetType() == typeof(AVRenderer))
			{
				if (rendererFormTable.ContainsKey(node.Tag) == true) 
				{
					RendererControlForm form = (RendererControlForm)rendererFormTable[node.Tag];
					form.Activate();
				} 
				else 
				{
					RendererControlForm form = new RendererControlForm(this,(AVRenderer)node.Tag,null);
					rendererFormTable.Add(node.Tag,form);
					form.Show();
				}
			}
		}

		private void mediaListView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right)
			{
				if (dragStartPoint.X != e.X || dragStartPoint.Y != e.Y) 
				{
					ArrayList mediaList = new ArrayList();
					foreach (ListViewItem i in mediaListView.SelectedItems) 
					{
						if (i.Tag.GetType() == typeof(CpMediaItem)) 
						{
							mediaList.Add((CpMediaItem)i.Tag);
						}
					}
					if (mediaList.Count > 0) 
					{
						CpMediaItem[] mediaArray = (CpMediaItem[])mediaList.ToArray(typeof(CpMediaItem));
						//mediaListView.DoDragDrop(mediaArray, DragDropEffects.Copy | DragDropEffects.Move);
						string didl = MediaBuilder.BuildDidl(ToXmlFormatter.DefaultFormatter, MediaObject.ToXmlData_Default, mediaArray);
						mediaListView.DoDragDrop(didl, DragDropEffects.All);
					}
				}
			}
		}

		private void mediaListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			dragStartPoint.X = e.X;
			dragStartPoint.Y = e.Y;
		}

		private void mediaListView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			dragStartPoint.X = 0;
			dragStartPoint.Y = 0;
		}

		public void RendererControlFormClose(RendererControlForm form,AVRenderer renderer)
		{
			if (rendererFormTable.ContainsKey(renderer)) rendererFormTable.Remove(renderer);
		}

		private void mediaContextMenu_Popup(object sender, System.EventArgs e)
		{
			while (mediaContextMenu.MenuItems.Count > 3) 
			{
				mediaContextMenu.MenuItems.RemoveAt(3);
			}

			PlayToRendererMenuItemMapping.Clear();
			menuItem3.Visible = false;
			if (rendererRootNode.Nodes.Count > 0) 
			{
				menuItem3.Visible = true;
				foreach (TreeNode node in rendererRootNode.Nodes) 
				{
					AVRenderer renderer = (AVRenderer)node.Tag;
					if (renderer != null) 
					{
						MenuItem m = new MenuItem("Send to " + renderer.FriendlyName,new EventHandler(PlayMediaMenuSelectedSink));
						PlayToRendererMenuItemMapping.Add(m,renderer);
						mediaContextMenu.MenuItems.Add(m);
					}
				}
			}
		}

		private void PlayMediaMenuSelectedSink(object sender, EventArgs e)
		{
			if (mediaListView.SelectedItems.Count > 0)
			{
				AVRenderer selectedSendToRenderer = (AVRenderer)PlayToRendererMenuItemMapping[sender];

				lock (mediaListView) 
				{
					CpMediaItem[] medias = new CpMediaItem[mediaListView.SelectedItems.Count];
					int i = 0;
					foreach (ListViewItem li in mediaListView.SelectedItems) 
					{
						medias[i] = (CpMediaItem)mediaListView.SelectedItems[i].Tag;
						i++;
					}

					PopupRendererForm(selectedSendToRenderer,medias);
				}
			}
			else if (m_SelectedContainer != null)
			{
				AVRenderer selectedSendToRenderer = (AVRenderer)PlayToRendererMenuItemMapping[sender];
				PopupRendererForm(selectedSendToRenderer, m_SelectedContainer);
			}

		}

		public void PopupRendererForm(AVRenderer renderer, CpMediaItem[] playmedias) 
		{
			if (rendererFormTable.ContainsKey(renderer) == true) 
			{
				RendererControlForm form = (RendererControlForm)rendererFormTable[renderer];
				if (playmedias != null) form.SetupConnection(playmedias);
				form.Activate();
			} 
			else 
			{
				RendererControlForm form = new RendererControlForm(this,renderer,null);
				rendererFormTable.Add(renderer,form);
				if (playmedias != null) form.SetupConnection(playmedias);
				form.Show();
			}
		}

		public void PopupRendererForm(AVRenderer renderer, ICpContainer container) 
		{
			if (rendererFormTable.ContainsKey(renderer) == true) 
			{
				RendererControlForm form = (RendererControlForm)rendererFormTable[renderer];
				if (container != null) form.SetupConnection(container);
				form.Activate();
			} 
			else 
			{
				RendererControlForm form = new RendererControlForm(this,renderer,null);
				rendererFormTable.Add(renderer,form);
				if (container != null) form.SetupConnection(container);
				form.Show();
			}
		}
		
		private void searchMediaMenuItem_Click(object sender, System.EventArgs e)
		{
			MediaSearchForm searchForm = new MediaSearchForm(cdsRootNode,rendererRootNode,this);
			searchForm.Show();
		}

		private void mediaPropertiesMenuItem_Click(object sender, System.EventArgs e)
		{
			mediaPropertiesMenuItem.Checked = !mediaPropertiesMenuItem.Checked;

			if (mediaListView.Visible == true) 
			{
				if (mediaPropertiesMenuItem.Checked == true) 
				{
					mediaPropertyControl.Visible = true;
					mediaPropertyControlSplitter.Visible = true;
					mediaPropertyControl.MediaItem = null;
				} 
				else 
				{
					mediaPropertyControl.Visible = false;
					mediaPropertyControlSplitter.Visible = false;
					mediaPropertyControl.MediaItem = null;
				}
			}
		
		}

		private void closeInstanceMenuItem_Click(object sender, System.EventArgs e)
		{
            if (InvokeRequired == true)
            {
                int tt = 5;
            }

			TreeNode node = deviceTree.SelectedNode;
			if (node == null || node.Tag == null) return;

			if (node.Tag.GetType() == typeof(AVConnection))
			{
				AVConnection connection = (AVConnection)node.Tag;
				connection.Close();
			}
		}

		private void displayContainerProperties_Click(object sender, System.EventArgs e)
		{
            if (InvokeRequired == true)
            {
                int tt = 5;
            }

            if (deviceTree.SelectedNode == null || deviceTree.SelectedNode.Tag == null || deviceTree.SelectedNode == rendererRootNode || deviceTree.SelectedNode == cdsRootNode) return;
			ICpMedia icpm = deviceTree.SelectedNode.Tag as ICpMedia;
			
			if (icpm != null) 
			{
				PopupMediaPropertyDialog(icpm);
			}
		}

		private void showDebugInfoMenuItem_Click(object sender, System.EventArgs e)
		{
			OpenSource.Utilities.InstanceTracker.Display();
		}

		private void mediaListView_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (
				(listViewSelectedObject.GetType() == typeof(CpMediaContainer)) ||
				(listViewSelectedObject.GetType() == typeof(CpRootContainer))
				)
			{
				CpMediaContainer currentContainer = (CpMediaContainer)listViewSelectedObject;

				if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
				{
					string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);

					Hashtable temp = new Hashtable();
					FileInfo f;
					foreach (string filename in filenames) 
					{
						f = new FileInfo(filename);

						// TODO: Create object and send local file to CDS
					}
				}

				if (e.Data.GetDataPresent(typeof(CpMediaItem[])) == true)
				{
					IMediaItem[] items = (IMediaItem[])e.Data.GetData(typeof(CpMediaItem[]));

					foreach (IMediaItem item in items)
					{
						IMediaItem itemClone = (IMediaItem)item.MetadataCopy();

						foreach (IMediaResource resource in itemClone.Resources) 
						{
							resource.ContentUri = "";
							resource.ImportUri = "";
						}

						currentContainer.RequestCreateObject(itemClone,item,new OpenSource.UPnP.AV.MediaServer.CP.CpMediaDelegates.Delegate_ResultCreateObject(ResultCreateObjectSink));
					}
				}

				if (e.Data.GetDataPresent(typeof(string)) == true)
				{
					string stritems = (string)e.Data.GetData(typeof(string));

					ArrayList items = MediaBuilder.BuildMediaBranches(stritems,typeof(MediaItem),typeof(MediaContainer));

					foreach (IUPnPMedia item in items)
					{
						IUPnPMedia itemClone = (IUPnPMedia)item.MetadataCopy();

						foreach (IMediaResource resource in itemClone.Resources) 
						{
							resource.ContentUri = "";
							resource.ImportUri = "";
						}

						currentContainer.RequestCreateObject(itemClone,item,new OpenSource.UPnP.AV.MediaServer.CP.CpMediaDelegates.Delegate_ResultCreateObject(ResultCreateObjectSink));
					}
				}

			}
		}

		private void ResultCreateObjectSink(ICpContainer parent, IUPnPMedia newObject, string newObjectID, string ResultXml, IUPnPMedia returnedObject, object Tag, UPnPInvokeException error, Exception xmlToObjectError) 
		{
			if (error != null) 
			{
				MessageBox.Show(this,xmlToObjectError.ToString(),"Create Object Server Error",MessageBoxButtons.OK,MessageBoxIcon.Error);
				return;
			}
			if (xmlToObjectError != null) 
			{
				MessageBox.Show(this,xmlToObjectError.ToString(),"Create Object Response Error",MessageBoxButtons.OK,MessageBoxIcon.Error);
				return;
			}

			IUPnPMedia original = (IUPnPMedia) Tag;

			for (int i=0; i < original.Resources.Length; i++)
			{
				parent.RequestImportResource2(new Uri(((IMediaResource)original.Resources[i]).ContentUri), (IMediaResource) returnedObject.Resources[i], null, new OpenSource.UPnP.AV.MediaServer.CP.CpMediaDelegates.Delegate_ResultImportResource2(ResultImportResource2Sink));
			}

			//newObject
//			foreach (IMediaResource resource in returnedObject.Resources) 
//			{
//				parent.RequestImportResource2(new Uri(((IMediaResource)original.Resources[0]).ContentUri), resource, null, new OpenSource.UPnP.AV.MediaServer.CP.CpMediaDelegates.Delegate_ResultImportResource2(ResultImportResource2Sink));
//			}
		}

		private void ResultImportResource2Sink(System.Uri importFromThis, IUPnPMedia owner, IMediaResource importToThis, IResourceTransfer transferObject, object Tag, UPnPInvokeException error)
		{
			//MessageBox.Show(this,"Transfering...","Object Import",MessageBoxButtons.OK,MessageBoxIcon.Information);
			object[] args = new object[6];
			args[0] = importFromThis;
			args[1] = owner;
			args[2] = importToThis;
			args[3] = transferObject;
			args[4] = Tag;
			args[5] = error;
			this.BeginInvoke(new OpenSource.UPnP.AV.MediaServer.CP.CpMediaDelegates.Delegate_ResultImportResource2(ResultImportResource2SinkEx),args);
		}

		private void ResultImportResource2SinkEx(System.Uri importFromThis, IUPnPMedia owner, IMediaResource importToThis, IResourceTransfer transferObject, object Tag, UPnPInvokeException error)
		{
			new TransferForm(transferObject).Show();
		}

		private void mediaListView_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (
				(listViewSelectedObject.GetType() == typeof(CpMediaContainer)) ||
				(listViewSelectedObject.GetType() == typeof(CpRootContainer))
				)
			{
				CpMediaContainer currentContainer = (CpMediaContainer)listViewSelectedObject;

				if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
				{
					string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);
				
					if (filenames.Length != 0)
					{
						e.Effect = DragDropEffects.Link;
					}
				}
				
				string[] s = e.Data.GetFormats();

				if (e.Data.GetDataPresent(typeof(CpMediaItem[])) == true)
				{
					IMediaItem[] items = (IMediaItem[])e.Data.GetData(typeof(CpMediaItem[]));

					if (items != null)
					{
						e.Effect = DragDropEffects.Copy;
					}
				}

				if (e.Data.GetDataPresent(typeof(string)) == true)
				{
					string items = (string)e.Data.GetData(typeof(string));

					if (items != null && items.StartsWith("<DIDL-Lite "))
					{
						e.Effect = DragDropEffects.Copy;
					}
				}


			}
		}

		private void deleteMediaMenuItem_Click(object sender, System.EventArgs e)
		{
			if (mediaListView.SelectedItems.Count == 0) return;

			if (MessageBox.Show(this,"Delete selected media items permanently?","Delete Media",MessageBoxButtons.YesNo,MessageBoxIcon.Exclamation) == DialogResult.Yes) 
			{
				lock (mediaListView) 
				{
					CpMediaItem[] medias = new CpMediaItem[mediaListView.SelectedItems.Count];
					int i = 0;
					foreach (ListViewItem li in mediaListView.SelectedItems) 
					{
						medias[i] = (CpMediaItem)mediaListView.SelectedItems[i].Tag;
						i++;
					}

					foreach (CpMediaItem item in medias) 
					{
						item.RequestDestroyObject(null,new CpMediaDelegates.Delegate_ResultDestroyObject(ResultDestroyObjectSink));
					}
				}
			}
		}

		private void ResultDestroyObjectSink(ICpMedia destroyThis, object Tag, UPnPInvokeException error)
		{
			MessageBox.Show(this, "Media " + destroyThis.Title + " Deleted","Delete Media");
		}

		private void extendedM3UMenuItem_Click(object sender, System.EventArgs e)
		{
			extendedM3UMenuItem.Checked = !extendedM3UMenuItem.Checked;
			OpenSource.UPnP.AV.RENDERER.CP.AVPlayList.EnableExtendedM3U = extendedM3UMenuItem.Checked;
		}

        private void menuItem6_Click(object sender, EventArgs e)
        {
            ForceLoad fl = new ForceLoad();
            if (fl.ShowDialog() == DialogResult.OK)
            {
                rendererDiscovery.ForceDeviceAddition(fl.NetworkUri);
            }
            fl.Dispose();
        }

	}
}
