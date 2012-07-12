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
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnpSpy
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {
        private int SubscribeTime = 300;
        protected UPnPSmartControlPoint scp;
        protected TreeNode UPnpRoot = new TreeNode("Devices", 0, 0);

        private Hashtable ForceDeviceList = new Hashtable();

        public delegate void UpdateTreeDelegate(TreeNode node);
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.ImageList treeImageList;
        private System.Windows.Forms.TreeView deviceTree;
        private System.Windows.Forms.ListView listInfo;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.Splitter splitter2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ListView eventListView;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ContextMenu deviceContextMenu;
        private System.Windows.Forms.MenuItem eventSubscribeMenuItem;
        private System.Windows.Forms.ContextMenu listInfoContextMenu;
        private System.Windows.Forms.MenuItem copyValueCpMenuItem;
        private System.Windows.Forms.MenuItem copyTableCpMenuItem;
        private System.Windows.Forms.ContextMenu eventListViewContextMenu;
        private System.Windows.Forms.MenuItem ClearEventLogMenuItem;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem menuItem7;
        private System.Windows.Forms.MenuItem copyEventCpMenuItem;
        private System.Windows.Forms.MenuItem copyEventLogCpMenuItem;
        private System.Windows.Forms.MenuItem menuItem11;
        private System.Windows.Forms.MenuItem menuItem9;
        private System.Windows.Forms.MenuItem menuItem12;
        private System.Windows.Forms.MenuItem viewStatusbarMenuItem;
        private System.Windows.Forms.MenuItem menuItem10;
        private System.Windows.Forms.MenuItem menuItem13;
        private System.Windows.Forms.MenuItem menuItem14;
        private System.Windows.Forms.MenuItem expandAllMenuItem;
        private System.Windows.Forms.MenuItem menuItem16;
        private System.Windows.Forms.MenuItem menuItem19;
        private System.Windows.Forms.MenuItem rescanMenuItem;
        private System.Windows.Forms.MenuItem menuItem18;
        private System.Windows.Forms.MenuItem rescanNetworkMenuItem;
        private System.Windows.Forms.MenuItem invokeActionMenuItem;
        private System.Windows.Forms.MenuItem expandAllMenuItem2;
        private System.Windows.Forms.MenuItem collapseAllMenuItem2;
        private System.Windows.Forms.MenuItem menuItem21;
        private System.Windows.Forms.MenuItem collapseAllMenuItem;
        private System.Windows.Forms.MenuItem presPageMenuItem;
        private System.Windows.Forms.MenuItem DeviceXMLmenuItem;
        private System.Windows.Forms.MenuItem ServiceXMLmenuItem;
        private System.Windows.Forms.MenuItem ValidateActionMenuItem;
        private System.Windows.Forms.MenuItem showDebugInfoMenuItem;
        private System.Windows.Forms.MenuItem helpMenuItem;
        private System.Windows.Forms.MenuItem removeDeviceMenuItem;
        private System.Windows.Forms.MenuItem manuallyAddDeviceMenuItem;
        private System.Windows.Forms.MenuItem menuItem15;
        private MenuItem menuItem6;
        private MenuItem openWebPageMenuItem;
        private System.ComponentModel.IContainer components;

        public MainForm(string[] args)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            deviceTree.Nodes.Add(UPnpRoot);

            foreach (string parm in args)
            {
                if (parm.ToUpper() == "/DEBUG")
                {
                    OpenSource.Utilities.InstanceTracker.Enabled = true;
                    OpenSource.Utilities.EventLogger.Enabled = true;
                    OpenSource.Utilities.EventLogger.ShowAll = true;
                    OpenSource.Utilities.InstanceTracker.Display();
                }
                if (parm.ToUpper().StartsWith("/ST:"))
                {
                    DText p = new DText();
                    p.ATTRMARK = ":";
                    p[0] = parm;
                    try
                    {
                        SubscribeTime = int.Parse(p[2]);
                    }
                    catch (Exception)
                    { }
                }
            }

            scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(HandleAddedDevice));
            scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(HandleRemovedDevice);
            //			scp.OnUpdatedDevice += new UPnPSmartControlPoint.DeviceHandler(HandleUpdatedDevice);
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
            this.deviceTree = new System.Windows.Forms.TreeView();
            this.deviceContextMenu = new System.Windows.Forms.ContextMenu();
            this.presPageMenuItem = new System.Windows.Forms.MenuItem();
            this.eventSubscribeMenuItem = new System.Windows.Forms.MenuItem();
            this.invokeActionMenuItem = new System.Windows.Forms.MenuItem();
            this.ValidateActionMenuItem = new System.Windows.Forms.MenuItem();
            this.DeviceXMLmenuItem = new System.Windows.Forms.MenuItem();
            this.ServiceXMLmenuItem = new System.Windows.Forms.MenuItem();
            this.removeDeviceMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem18 = new System.Windows.Forms.MenuItem();
            this.expandAllMenuItem2 = new System.Windows.Forms.MenuItem();
            this.collapseAllMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem21 = new System.Windows.Forms.MenuItem();
            this.rescanNetworkMenuItem = new System.Windows.Forms.MenuItem();
            this.treeImageList = new System.Windows.Forms.ImageList(this.components);
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.listInfo = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.listInfoContextMenu = new System.Windows.Forms.ContextMenu();
            this.copyValueCpMenuItem = new System.Windows.Forms.MenuItem();
            this.copyTableCpMenuItem = new System.Windows.Forms.MenuItem();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.manuallyAddDeviceMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.menuItem12 = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.menuItem13 = new System.Windows.Forms.MenuItem();
            this.menuItem14 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.rescanMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem19 = new System.Windows.Forms.MenuItem();
            this.expandAllMenuItem = new System.Windows.Forms.MenuItem();
            this.collapseAllMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.viewStatusbarMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.showDebugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.eventListView = new System.Windows.Forms.ListView();
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.eventListViewContextMenu = new System.Windows.Forms.ContextMenu();
            this.copyEventCpMenuItem = new System.Windows.Forms.MenuItem();
            this.copyEventLogCpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.ClearEventLogMenuItem = new System.Windows.Forms.MenuItem();
            this.splitter2 = new System.Windows.Forms.Splitter();
            this.openWebPageMenuItem = new System.Windows.Forms.MenuItem();
            this.SuspendLayout();
            // 
            // deviceTree
            // 
            this.deviceTree.BackColor = System.Drawing.Color.White;
            this.deviceTree.ContextMenu = this.deviceContextMenu;
            resources.ApplyResources(this.deviceTree, "deviceTree");
            this.deviceTree.ImageList = this.treeImageList;
            this.deviceTree.ItemHeight = 16;
            this.deviceTree.Name = "deviceTree";
            this.deviceTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.OnSelectedItem);
            this.deviceTree.DoubleClick += new System.EventHandler(this.OnInvokeAction);
            this.deviceTree.MouseDown += new System.Windows.Forms.MouseEventHandler(this.deviceTree_MouseDown);
            // 
            // deviceContextMenu
            // 
            this.deviceContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.presPageMenuItem,
            this.eventSubscribeMenuItem,
            this.invokeActionMenuItem,
            this.ValidateActionMenuItem,
            this.DeviceXMLmenuItem,
            this.ServiceXMLmenuItem,
            this.removeDeviceMenuItem,
            this.menuItem18,
            this.expandAllMenuItem2,
            this.collapseAllMenuItem2,
            this.menuItem21,
            this.rescanNetworkMenuItem});
            this.deviceContextMenu.Popup += new System.EventHandler(this.deviceContextMenu_Popup);
            // 
            // presPageMenuItem
            // 
            this.presPageMenuItem.DefaultItem = true;
            this.presPageMenuItem.Index = 0;
            resources.ApplyResources(this.presPageMenuItem, "presPageMenuItem");
            this.presPageMenuItem.Click += new System.EventHandler(this.presPageMenuItem_Click);
            // 
            // eventSubscribeMenuItem
            // 
            this.eventSubscribeMenuItem.DefaultItem = true;
            this.eventSubscribeMenuItem.Index = 1;
            resources.ApplyResources(this.eventSubscribeMenuItem, "eventSubscribeMenuItem");
            this.eventSubscribeMenuItem.Click += new System.EventHandler(this.OnSubscribeService);
            // 
            // invokeActionMenuItem
            // 
            this.invokeActionMenuItem.DefaultItem = true;
            this.invokeActionMenuItem.Index = 2;
            resources.ApplyResources(this.invokeActionMenuItem, "invokeActionMenuItem");
            this.invokeActionMenuItem.Click += new System.EventHandler(this.OnInvokeAction);
            // 
            // ValidateActionMenuItem
            // 
            this.ValidateActionMenuItem.Index = 3;
            resources.ApplyResources(this.ValidateActionMenuItem, "ValidateActionMenuItem");
            this.ValidateActionMenuItem.Click += new System.EventHandler(this.ValidateActionMenuItem_Click);
            // 
            // DeviceXMLmenuItem
            // 
            this.DeviceXMLmenuItem.Index = 4;
            resources.ApplyResources(this.DeviceXMLmenuItem, "DeviceXMLmenuItem");
            this.DeviceXMLmenuItem.Click += new System.EventHandler(this.DeviceXMLmenuItem_Click);
            // 
            // ServiceXMLmenuItem
            // 
            this.ServiceXMLmenuItem.Index = 5;
            resources.ApplyResources(this.ServiceXMLmenuItem, "ServiceXMLmenuItem");
            this.ServiceXMLmenuItem.Click += new System.EventHandler(this.ServiceXMLmenuItem_Click);
            // 
            // removeDeviceMenuItem
            // 
            this.removeDeviceMenuItem.Index = 6;
            resources.ApplyResources(this.removeDeviceMenuItem, "removeDeviceMenuItem");
            this.removeDeviceMenuItem.Click += new System.EventHandler(this.removeDeviceMenuItem_Click);
            // 
            // menuItem18
            // 
            this.menuItem18.Index = 7;
            resources.ApplyResources(this.menuItem18, "menuItem18");
            // 
            // expandAllMenuItem2
            // 
            this.expandAllMenuItem2.Index = 8;
            resources.ApplyResources(this.expandAllMenuItem2, "expandAllMenuItem2");
            this.expandAllMenuItem2.Click += new System.EventHandler(this.expandAllMenuItem_Click);
            // 
            // collapseAllMenuItem2
            // 
            this.collapseAllMenuItem2.Index = 9;
            resources.ApplyResources(this.collapseAllMenuItem2, "collapseAllMenuItem2");
            this.collapseAllMenuItem2.Click += new System.EventHandler(this.collapseAllMenuItem_Click);
            // 
            // menuItem21
            // 
            this.menuItem21.Index = 10;
            resources.ApplyResources(this.menuItem21, "menuItem21");
            // 
            // rescanNetworkMenuItem
            // 
            this.rescanNetworkMenuItem.Index = 11;
            resources.ApplyResources(this.rescanNetworkMenuItem, "rescanNetworkMenuItem");
            this.rescanNetworkMenuItem.Click += new System.EventHandler(this.rescanMenuItem_Click);
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
            // 
            // statusBar
            // 
            resources.ApplyResources(this.statusBar, "statusBar");
            this.statusBar.Name = "statusBar";
            // 
            // splitter1
            // 
            resources.ApplyResources(this.splitter1, "splitter1");
            this.splitter1.Name = "splitter1";
            this.splitter1.TabStop = false;
            // 
            // listInfo
            // 
            this.listInfo.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.listInfo.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listInfo.ContextMenu = this.listInfoContextMenu;
            resources.ApplyResources(this.listInfo, "listInfo");
            this.listInfo.FullRowSelect = true;
            this.listInfo.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            ((System.Windows.Forms.ListViewItem)(resources.GetObject("listInfo.Items")))});
            this.listInfo.Name = "listInfo";
            this.listInfo.UseCompatibleStateImageBehavior = false;
            this.listInfo.View = System.Windows.Forms.View.Details;
            this.listInfo.DoubleClick += new System.EventHandler(this.listInfo_DoubleClick);
            this.listInfo.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listInfo_KeyDown);
            // 
            // columnHeader1
            // 
            resources.ApplyResources(this.columnHeader1, "columnHeader1");
            // 
            // columnHeader2
            // 
            resources.ApplyResources(this.columnHeader2, "columnHeader2");
            // 
            // listInfoContextMenu
            // 
            this.listInfoContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.openWebPageMenuItem,
            this.copyValueCpMenuItem,
            this.copyTableCpMenuItem});
            this.listInfoContextMenu.Popup += new System.EventHandler(this.listInfoContextMenu_Popup);
            // 
            // copyValueCpMenuItem
            // 
            this.copyValueCpMenuItem.Index = 1;
            resources.ApplyResources(this.copyValueCpMenuItem, "copyValueCpMenuItem");
            this.copyValueCpMenuItem.Click += new System.EventHandler(this.copyValueCpMenuItem_Click);
            // 
            // copyTableCpMenuItem
            // 
            this.copyTableCpMenuItem.Index = 2;
            resources.ApplyResources(this.copyTableCpMenuItem, "copyTableCpMenuItem");
            this.copyTableCpMenuItem.Click += new System.EventHandler(this.copyTableCpMenuItem_Click);
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
            this.manuallyAddDeviceMenuItem,
            this.menuItem15,
            this.menuItem12,
            this.menuItem9,
            this.menuItem4,
            this.menuItem13,
            this.menuItem14,
            this.menuItem2});
            resources.ApplyResources(this.menuItem1, "menuItem1");
            // 
            // manuallyAddDeviceMenuItem
            // 
            this.manuallyAddDeviceMenuItem.Index = 0;
            resources.ApplyResources(this.manuallyAddDeviceMenuItem, "manuallyAddDeviceMenuItem");
            this.manuallyAddDeviceMenuItem.Click += new System.EventHandler(this.manuallyAddDeviceMenuItem_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 1;
            resources.ApplyResources(this.menuItem15, "menuItem15");
            // 
            // menuItem12
            // 
            this.menuItem12.Index = 2;
            resources.ApplyResources(this.menuItem12, "menuItem12");
            this.menuItem12.Click += new System.EventHandler(this.copyTableCpMenuItem_Click);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 3;
            resources.ApplyResources(this.menuItem9, "menuItem9");
            this.menuItem9.Click += new System.EventHandler(this.copyEventLogCpMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 4;
            resources.ApplyResources(this.menuItem4, "menuItem4");
            // 
            // menuItem13
            // 
            this.menuItem13.Index = 5;
            resources.ApplyResources(this.menuItem13, "menuItem13");
            this.menuItem13.Click += new System.EventHandler(this.ClearEventLogMenuItem_Click);
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 6;
            resources.ApplyResources(this.menuItem14, "menuItem14");
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 7;
            resources.ApplyResources(this.menuItem2, "menuItem2");
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 1;
            this.menuItem7.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.rescanMenuItem,
            this.menuItem19,
            this.expandAllMenuItem,
            this.collapseAllMenuItem,
            this.menuItem16,
            this.menuItem3,
            this.viewStatusbarMenuItem});
            resources.ApplyResources(this.menuItem7, "menuItem7");
            // 
            // rescanMenuItem
            // 
            this.rescanMenuItem.Index = 0;
            resources.ApplyResources(this.rescanMenuItem, "rescanMenuItem");
            this.rescanMenuItem.Click += new System.EventHandler(this.rescanMenuItem_Click);
            // 
            // menuItem19
            // 
            this.menuItem19.Index = 1;
            resources.ApplyResources(this.menuItem19, "menuItem19");
            // 
            // expandAllMenuItem
            // 
            this.expandAllMenuItem.Index = 2;
            resources.ApplyResources(this.expandAllMenuItem, "expandAllMenuItem");
            this.expandAllMenuItem.Click += new System.EventHandler(this.expandAllMenuItem_Click);
            // 
            // collapseAllMenuItem
            // 
            this.collapseAllMenuItem.Index = 3;
            resources.ApplyResources(this.collapseAllMenuItem, "collapseAllMenuItem");
            this.collapseAllMenuItem.Click += new System.EventHandler(this.collapseAllMenuItem_Click);
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 4;
            resources.ApplyResources(this.menuItem16, "menuItem16");
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 5;
            resources.ApplyResources(this.menuItem3, "menuItem3");
            this.menuItem3.Click += new System.EventHandler(this.menuItem3_Click);
            // 
            // viewStatusbarMenuItem
            // 
            this.viewStatusbarMenuItem.Checked = true;
            this.viewStatusbarMenuItem.Index = 6;
            resources.ApplyResources(this.viewStatusbarMenuItem, "viewStatusbarMenuItem");
            this.viewStatusbarMenuItem.Click += new System.EventHandler(this.viewStatusbarMenuItem_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 2;
            this.menuItem5.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem6,
            this.menuItem10,
            this.showDebugInfoMenuItem});
            resources.ApplyResources(this.menuItem5, "menuItem5");
            this.menuItem5.Popup += new System.EventHandler(this.menuItem5_Popup);
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            resources.ApplyResources(this.helpMenuItem, "helpMenuItem");
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 1;
            resources.ApplyResources(this.menuItem6, "menuItem6");
            this.menuItem6.Click += new System.EventHandler(this.menuItem6_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 2;
            resources.ApplyResources(this.menuItem10, "menuItem10");
            // 
            // showDebugInfoMenuItem
            // 
            this.showDebugInfoMenuItem.Index = 3;
            resources.ApplyResources(this.showDebugInfoMenuItem, "showDebugInfoMenuItem");
            this.showDebugInfoMenuItem.Click += new System.EventHandler(this.showDebugInfoMenuItem_Click);
            // 
            // eventListView
            // 
            this.eventListView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.eventListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader6,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5});
            this.eventListView.ContextMenu = this.eventListViewContextMenu;
            resources.ApplyResources(this.eventListView, "eventListView");
            this.eventListView.FullRowSelect = true;
            this.eventListView.Name = "eventListView";
            this.eventListView.UseCompatibleStateImageBehavior = false;
            this.eventListView.View = System.Windows.Forms.View.Details;
            this.eventListView.KeyDown += new System.Windows.Forms.KeyEventHandler(this.eventListView_KeyDown);
            // 
            // columnHeader6
            // 
            resources.ApplyResources(this.columnHeader6, "columnHeader6");
            // 
            // columnHeader3
            // 
            resources.ApplyResources(this.columnHeader3, "columnHeader3");
            // 
            // columnHeader4
            // 
            resources.ApplyResources(this.columnHeader4, "columnHeader4");
            // 
            // columnHeader5
            // 
            resources.ApplyResources(this.columnHeader5, "columnHeader5");
            // 
            // eventListViewContextMenu
            // 
            this.eventListViewContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.copyEventCpMenuItem,
            this.copyEventLogCpMenuItem,
            this.menuItem11,
            this.ClearEventLogMenuItem});
            this.eventListViewContextMenu.Popup += new System.EventHandler(this.eventListViewContextMenu_Popup);
            // 
            // copyEventCpMenuItem
            // 
            this.copyEventCpMenuItem.DefaultItem = true;
            this.copyEventCpMenuItem.Index = 0;
            resources.ApplyResources(this.copyEventCpMenuItem, "copyEventCpMenuItem");
            this.copyEventCpMenuItem.Click += new System.EventHandler(this.copyEventCpMenuItem_Click);
            // 
            // copyEventLogCpMenuItem
            // 
            this.copyEventLogCpMenuItem.Index = 1;
            resources.ApplyResources(this.copyEventLogCpMenuItem, "copyEventLogCpMenuItem");
            this.copyEventLogCpMenuItem.Click += new System.EventHandler(this.copyEventLogCpMenuItem_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 2;
            resources.ApplyResources(this.menuItem11, "menuItem11");
            // 
            // ClearEventLogMenuItem
            // 
            this.ClearEventLogMenuItem.Index = 3;
            resources.ApplyResources(this.ClearEventLogMenuItem, "ClearEventLogMenuItem");
            this.ClearEventLogMenuItem.Click += new System.EventHandler(this.ClearEventLogMenuItem_Click);
            // 
            // splitter2
            // 
            resources.ApplyResources(this.splitter2, "splitter2");
            this.splitter2.Name = "splitter2";
            this.splitter2.TabStop = false;
            // 
            // openWebPageMenuItem
            // 
            this.openWebPageMenuItem.DefaultItem = true;
            this.openWebPageMenuItem.Index = 0;
            resources.ApplyResources(this.openWebPageMenuItem, "openWebPageMenuItem");
            this.openWebPageMenuItem.Click += new System.EventHandler(this.openWebPageMenuItem_Click);
            // 
            // MainForm
            // 
            resources.ApplyResources(this, "$this");
            this.Controls.Add(this.listInfo);
            this.Controls.Add(this.splitter2);
            this.Controls.Add(this.eventListView);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.deviceTree);
            this.Controls.Add(this.statusBar);
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.ResumeLayout(false);

        }
        #endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            for (int i = 0; i < (args.Length); i++)
            {
                if (args[i].ToLower() == "-en") System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("en");
                if (args[i].ToLower() == "-fr") System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("fr");
            }
            Application.Run(new MainForm(args));
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            listInfo.GetType().GetProperty("DoubleBuffered", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance).SetValue(listInfo, true, null);
            eventListView.GetType().GetProperty("DoubleBuffered", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance).SetValue(eventListView, true, null);

            // Check for update
            if (File.Exists(Application.StartupPath + "\\AutoUpdateTool.exe"))
            {
                AutoUpdate.AutoUpdateCheck(this);
            }
            else
            {
                menuItem6.Visible = false;
            }

            //this.Width = 820; this.Height = 480;
        }

        protected void HandleCreate(UPnPDevice device, Uri URL)
        {
            TreeNode Parent;
            TreeNode Child;
            SortedList TempList = new SortedList();

            Parent = new TreeNode(device.FriendlyName, 1, 1);
            Parent.Tag = device;
            for (int cid = 0; cid < device.Services.Length; ++cid)
            {
                Child = new TreeNode(device.Services[cid].ServiceURN, 2, 2);
                Child.Tag = device.Services[cid];

                TreeNode stateVarNode = new TreeNode("State variables", 6, 6);
                Child.Nodes.Add(stateVarNode);

                UPnPStateVariable[] varList = device.Services[cid].GetStateVariables();
                TempList.Clear();
                foreach (UPnPStateVariable var in varList)
                {
                    TreeNode varNode = new TreeNode(var.Name, 5, 5);
                    varNode.Tag = var;
                    TempList.Add(var.Name, varNode);
                    //stateVarNode.Nodes.Add(varNode);
                }
                IDictionaryEnumerator sve = TempList.GetEnumerator();
                while (sve.MoveNext())
                {
                    stateVarNode.Nodes.Add((TreeNode)sve.Value);
                }

                TempList.Clear();
                foreach (UPnPAction action in device.Services[cid].GetActions())
                {
                    string argsstr = "";
                    foreach (UPnPArgument arg in action.ArgumentList)
                    {
                        if (arg.IsReturnValue == false)
                        {
                            if (argsstr != "") argsstr += ", ";
                            argsstr += arg.RelatedStateVar.ValueType + " " + arg.Name;
                        }
                    }

                    TreeNode methodNode = new TreeNode(action.Name + "(" + argsstr + ")", 4, 4);
                    methodNode.Tag = action;
                    //Child.Nodes.Add(methodNode);
                    TempList.Add(action.Name, methodNode);
                }

                IDictionaryEnumerator ide = TempList.GetEnumerator();
                while (ide.MoveNext())
                {
                    Child.Nodes.Add((TreeNode)ide.Value);
                }
                Parent.Nodes.Add(Child);
            }

            for (int cid = 0; cid < device.EmbeddedDevices.Length; ++cid)
            {
                Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
                Child.Tag = device.EmbeddedDevices[cid];
                Parent.Nodes.Add(Child);
            }

            Object[] args = new Object[1];
            args[0] = Parent;
            this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate), args);
        }

        protected TreeNode ProcessEmbeddedDevice(UPnPDevice device)
        {
            SortedList TempList = new SortedList();
            TreeNode Parent;
            TreeNode Child;

            Parent = new TreeNode(device.FriendlyName, 1, 1);
            Parent.Tag = device;

            /*
            for(int x=0;x<device.Services.Length;++x)
            {
                device.Services[x].OnInvokeError += new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError);
                device.Services[x].OnInvokeResponse += new UPnPService.UPnPServiceInvokeHandler(HandleInvoke);
            }
            */

            for (int cid = 0; cid < device.Services.Length; ++cid)
            {
                Child = new TreeNode(device.Services[cid].ServiceURN, 2, 2);
                Child.Tag = device.Services[cid];

                TreeNode stateVarNode = new TreeNode("State variables", 6, 6);
                Child.Nodes.Add(stateVarNode);

                UPnPStateVariable[] varList = device.Services[cid].GetStateVariables();
                TempList.Clear();
                foreach (UPnPStateVariable var in varList)
                {
                    TreeNode varNode = new TreeNode(var.Name, 5, 5);
                    varNode.Tag = var;
                    TempList.Add(var.Name, varNode);
                    //stateVarNode.Nodes.Add(varNode);
                }
                IDictionaryEnumerator sve = TempList.GetEnumerator();
                while (sve.MoveNext())
                {
                    stateVarNode.Nodes.Add((TreeNode)sve.Value);
                }


                TempList.Clear();
                foreach (UPnPAction action in device.Services[cid].GetActions())
                {
                    string argsstr = "";
                    foreach (UPnPArgument arg in action.ArgumentList)
                    {
                        if (arg.IsReturnValue == false)
                        {
                            if (argsstr != "") argsstr += ", ";
                            argsstr += arg.RelatedStateVar.ValueType + " " + arg.Name;
                        }
                    }

                    TreeNode methodNode = new TreeNode(action.Name + "(" + argsstr + ")", 4, 4);
                    methodNode.Tag = action;
                    //Child.Nodes.Add(methodNode);
                    TempList.Add(action.Name, methodNode);
                }

                IDictionaryEnumerator ide = TempList.GetEnumerator();
                while (ide.MoveNext())
                {
                    Child.Nodes.Add((TreeNode)ide.Value);
                }
                Parent.Nodes.Add(Child);
            }

            for (int cid = 0; cid < device.EmbeddedDevices.Length; ++cid)
            {
                Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
                Child.Tag = device.EmbeddedDevices[cid];
                Parent.Nodes.Add(Child);
            }

            return (Parent);
        }

        protected void HandleTreeUpdate(TreeNode node)
        {
            //UPnpRoot.Nodes.Add(node);
            //UPnpRoot.Expand();

            // Insert this node into the tree
            if (UPnpRoot.Nodes.Count == 0)
            {
                UPnpRoot.Nodes.Add(node);
            }
            else
            {
                for (int i = 0; i < UPnpRoot.Nodes.Count; ++i)
                {
                    if (UPnpRoot.Nodes[i].Text.CompareTo(node.Text) > 0)
                    {
                        UPnpRoot.Nodes.Insert(i, node);
                        break;
                    }
                    if (i == UPnpRoot.Nodes.Count - 1)
                    {
                        UPnpRoot.Nodes.Add(node);
                        break;
                    }
                }
            }
            UPnpRoot.Expand();
        }

        //		protected void HandleUpdatedDevice(UPnPSmartControlPoint sender, UPnPDevice device)
        //		{
        //			MessageBox.Show("Updated: " + device.FriendlyName);
        //		}

        protected void HandleExpiredDevice(UPnPSmartControlPoint sender, UPnPDevice device)
        {
            int cnt = UPnpRoot.Nodes.Count;
            for (int x = 0; x < cnt; ++x)
            {
                if (UPnpRoot.Nodes[x].Tag.GetHashCode() == device.GetHashCode())
                {
                    UPnpRoot.Nodes.RemoveAt(x);
                    break;
                }
            }
            MessageBox.Show("Expired: " + device.FriendlyName);
        }

        protected void HandleRemovedDevice(UPnPSmartControlPoint sender, UPnPDevice device)
        {
            this.Invoke(new UPnPSmartControlPoint.DeviceHandler(HandleRemovedDeviceEx), new object[2] {sender, device});
        }

        protected void HandleRemovedDeviceEx(UPnPSmartControlPoint sender, UPnPDevice device)
        {
            ArrayList TempList = new ArrayList();
            TreeNode tn;
            IEnumerator en = UPnpRoot.Nodes.GetEnumerator();
            while (en.MoveNext())
            {
                tn = (TreeNode)en.Current;
                if (((UPnPDevice)tn.Tag).UniqueDeviceName == device.UniqueDeviceName)
                {
                    TempList.Add(tn);
                }
            }
            for (int x = 0; x < TempList.Count; ++x)
            {
                TreeNode n = (TreeNode)TempList[x];
                CleanTags(n);
                UPnpRoot.Nodes.Remove(n);
            }
        }

        private void CleanTags(TreeNode n)
        {
            n.Tag = null;
            foreach (TreeNode sn in n.Nodes)
            {
                CleanTags(sn);
            }
        }

        protected void HandleAddedDevice(UPnPSmartControlPoint sender, UPnPDevice device)
        {
            /*
            for(int x=0;x<device.Services.Length;++x)
            {
                device.Services[x].OnInvokeError += new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError);
                device.Services[x].OnInvokeResponse += new UPnPService.UPnPServiceInvokeHandler(HandleInvoke);

            }
            */
            HandleCreate(device, device.BaseURL);
        }

        private void button1_Click(object sender, System.EventArgs e)
        {
            TreeNode[] x = new TreeNode[5];
            for (int y = 0; y < 5; ++y)
            {
                x[y] = new TreeNode("Child" + y.ToString());
            }
            deviceTree.Nodes.Add(new TreeNode("Root", x));
        }

        private void OnSelectedItem(object sender, System.Windows.Forms.TreeViewEventArgs e)
        {
            TreeNode node = deviceTree.SelectedNode;
            SetListInfo(node.Tag);
        }

        protected void SetListInfo(object infoObject)
        {
            ArrayList Items = new ArrayList();
            if (infoObject == null)
            {
                Items.Add(new ListViewItem(new string[] { "Product name", "Device Spy" }));
                Items.Add(new ListViewItem(new string[] { "Version", AutoUpdate.VersionString }));
                //Items.Add(new ListViewItem(new string[] {"UPnP devices found",DeviceList.Length.ToString()}));
            }
            else if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPDevice))
            {
                UPnPDevice d = (UPnPDevice)infoObject;
                Items.Add(new ListViewItem(new string[] { "Friendly name", d.FriendlyName }));
                Items.Add(new ListViewItem(new string[] { "Unique device name", d.UniqueDeviceName }));
                Items.Add(new ListViewItem(new string[] { "Has presentation", d.HasPresentation.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Manufacturer", d.Manufacturer }));
                Items.Add(new ListViewItem(new string[] { "Manufacturer URL", d.ManufacturerURL }));
                Items.Add(new ListViewItem(new string[] { "Model description", d.ModelDescription }));
                Items.Add(new ListViewItem(new string[] { "Model name", d.ModelName }));
                Items.Add(new ListViewItem(new string[] { "Model number", d.ModelNumber }));
                if (d.ModelURL != null)
                {
                    Items.Add(new ListViewItem(new string[] { "Model URL", d.ModelURL.ToString() }));
                }
                Items.Add(new ListViewItem(new string[] { "Product code", d.ProductCode }));
                Items.Add(new ListViewItem(new string[] { "Proprietary type", d.ProprietaryDeviceType }));
                Items.Add(new ListViewItem(new string[] { "Serial number", d.SerialNumber }));
                Items.Add(new ListViewItem(new string[] { "Services", d.Services.Length.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Embedded devices", d.EmbeddedDevices.Length.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Base URL", d.BaseURL.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Device URN", d.DeviceURN }));
                Items.Add(new ListViewItem(new string[] { "Expiration timeout", d.ExpirationTimeout.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Version", d.Major.ToString() + "." + d.Minor.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Remote endpoint", d.RemoteEndPoint.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Standard type", d.StandardDeviceType }));
                if (d.Icon != null)
                {
                    Items.Add(new ListViewItem(new string[] { "Device icon", "Present, " + d.Icon.Width + "x" + d.Icon.Height }));
                }
                else
                {
                    Items.Add(new ListViewItem(new string[] { "Device icon", "None" }));
                }
                if (d.InterfaceToHost != null)
                {
                    Items.Add(new ListViewItem(new string[] { "Interface to host", d.InterfaceToHost.ToString() }));
                }
                else
                {
                    Items.Add(new ListViewItem(new string[] { "Interface to host", "(Embedded device)" }));
                }

                string deviceURL = "";
                try
                {
                    if (d.PresentationURL != null)
                    {
                        if (d.PresentationURL.StartsWith("/") == true)
                        {
                            deviceURL = "http://" + d.RemoteEndPoint.Address.ToString() + ":" + d.PresentationURL;
                        }
                        else
                        {
                            if (d.PresentationURL.ToUpper().StartsWith("HTTP://") == false)
                            {
                                deviceURL = "http://" + d.RemoteEndPoint.Address.ToString() + ":" + d.RemoteEndPoint.Port.ToString() + "/" + d.PresentationURL;
                            }
                            else
                            {
                                deviceURL = d.PresentationURL;
                            }
                        }
                    }
                }
                catch { }
                Items.Add(new ListViewItem(new string[] { "Presentation URL", deviceURL }));
            }
            else if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPService))
            {
                UPnPService s = (UPnPService)infoObject;
                Items.Add(new ListViewItem(new string[] { "Parent UDN", s.ParentDevice.DeviceURN }));
                Items.Add(new ListViewItem(new string[] { "Version", s.Major.ToString() + "." + s.Minor.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Methods", s.Actions.Count.ToString() }));
                Items.Add(new ListViewItem(new string[] { "State variables", s.GetStateVariables().Length.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Service ID", s.ServiceID }));
                //Items.Add(new ListViewItem(new string[] {"Service XML",s.GetServiceXML()}));
                Items.Add(new ListViewItem(new string[] { "Service URL", (string)(new UPnPDebugObject(s)).GetField("SCPDURL") }));

                string deviceURL = null;
                try
                {
                    if (s.ParentDevice.PresentationURL != null)
                    {
                        if (s.ParentDevice.PresentationURL.ToLower().StartsWith("http://") || s.ParentDevice.PresentationURL.ToLower().StartsWith("https://"))
                        {
                            deviceURL = s.ParentDevice.PresentationURL;
                        }
                        else
                        {
                            if (s.ParentDevice.PresentationURL.StartsWith("/"))
                            {
                                deviceURL = "http://" + s.ParentDevice.RemoteEndPoint.Address.ToString() + ":" + s.ParentDevice.RemoteEndPoint.Port.ToString() + s.ParentDevice.PresentationURL;
                            }
                            else
                            {
                                deviceURL = "http://" + s.ParentDevice.RemoteEndPoint.Address.ToString() + ":" + s.ParentDevice.RemoteEndPoint.Port.ToString() + "/" + s.ParentDevice.PresentationURL;
                            }
                        }
                    }
                }
                catch { }
                if (deviceURL != null) Items.Add(new ListViewItem(new string[] { "Parent presentation URL", deviceURL }));
            }
            else if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPAction))
            {
                listInfo.Sorting = SortOrder.None;
                UPnPAction a = (UPnPAction)infoObject;
                Items.Add(new ListViewItem(new string[] { "Action name", a.Name }));
                if (a.HasReturnValue == false)
                {
                    Items.Add(new ListViewItem(new string[] { "Return argument", "<none>" }));
                }
                else
                {
                    Items.Add(new ListViewItem(new string[] { "Return argument ASV", a.GetRetArg().RelatedStateVar.Name }));
                    Items.Add(new ListViewItem(new string[] { "Return Type", a.GetRetArg().RelatedStateVar.ValueType }));
                }

                int argnum = 1;
                string dataType;
                foreach (UPnPArgument arg in a.ArgumentList)
                {
                    if (arg.IsReturnValue == false)
                    {
                        dataType = arg.DataType;
                        if (dataType == null || dataType == "") dataType = "Unknown type";
                        Items.Add(new ListViewItem(new string[] { "Argument " + argnum, "(" + arg.RelatedStateVar.ValueType + ") " + arg.Name }));
                        Items.Add(new ListViewItem(new string[] { "Argument " + argnum + " ASV", arg.RelatedStateVar.Name }));
                        argnum++;
                    }
                }
            }
            else if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPStateVariable))
            {
                UPnPStateVariable var = (UPnPStateVariable)infoObject;
                Items.Add(new ListViewItem(new string[] { "Variable name", var.Name }));
                Items.Add(new ListViewItem(new string[] { "Evented", var.SendEvent.ToString() }));
                Items.Add(new ListViewItem(new string[] { "Data type", var.ValueType }));
                try
                {
                    Items.Add(new ListViewItem(new string[] { "Last known value", var.Value.ToString() }));
                }
                catch (Exception)
                {
                    Items.Add(new ListViewItem(new string[] { "Last known value", "<unknown>" }));
                }
                if ((var.Minimum != null) && (var.Maximum == null))
                {
                    if (var.Step != null)
                    {
                        Items.Add(new ListViewItem(new string[] { "Value range", "Not below " + var.Minimum.ToString() + ", Step " + var.Step.ToString() }));
                    }
                    else
                    {
                        Items.Add(new ListViewItem(new string[] { "Value range", "Not below " + var.Minimum.ToString() }));
                    }
                }
                else
                    if ((var.Minimum == null) && (var.Maximum != null))
                    {
                        if (var.Step != null)
                        {
                            Items.Add(new ListViewItem(new string[] { "Value range", "Not above " + var.Maximum.ToString() + ", Step " + var.Step.ToString() }));
                        }
                        else
                        {
                            Items.Add(new ListViewItem(new string[] { "Value range", "Not above " + var.Maximum.ToString() }));
                        }
                    }
                    else
                        if ((var.Minimum != null) || (var.Maximum != null))
                        {
                            if (var.Step != null)
                            {
                                Items.Add(new ListViewItem(new string[] { "Value range", "From " + var.Minimum.ToString() + " to " + var.Maximum.ToString() + ", Step " + var.Step.ToString() }));
                            }
                            else
                            {
                                Items.Add(new ListViewItem(new string[] { "Value range", "From " + var.Minimum.ToString() + " to " + var.Maximum.ToString() }));
                            }
                        }

                if (var.AllowedStringValues != null && var.AllowedStringValues.Length > 0)
                {
                    string AllowedValues = "";
                    foreach (string x in var.AllowedStringValues)
                    {
                        if (AllowedValues != "") AllowedValues += ", ";
                        AllowedValues += x;
                    }
                    Items.Add(new ListViewItem(new string[] { "Allowed values", AllowedValues }));
                }

                if (var.DefaultValue != null)
                {
                    Items.Add(new ListViewItem(new string[] { "Default value", UPnPService.SerializeObjectInstance(var.DefaultValue) }));
                }
            }

            listInfo.Sorting = SortOrder.Ascending;
            listInfo.Items.Clear();
            listInfo.Items.AddRange((ListViewItem[])Items.ToArray(typeof(ListViewItem)));
        }

        protected void HandleSubscribe(UPnPService sender, bool succes)
        {
            if (this.InvokeRequired) { Invoke(new UPnPService.UPnPEventSubscribeHandler(HandleSubscribe), sender, succes); return; }

            if (succes == false)
            {
                statusBar.Text = "FAILED Subscription - " + sender.ParentDevice.FriendlyName + ", " + sender.ServiceID;
            }
            else
            {
                statusBar.Text = "Subscribed to " + sender.ParentDevice.FriendlyName + ", " + sender.ServiceID;
            }
        }

        protected void HandleEvents(UPnPStateVariable sender, object EventValue)
        {
            if (this.InvokeRequired) { this.Invoke(new UPnPStateVariable.ModifiedHandler(HandleEvents), new object[2] { sender, EventValue }); return; }

            string eventSource = sender.OwningService.ParentDevice.FriendlyName + "/" + sender.OwningService.ServiceID;
            string eventValue = UPnPService.SerializeObjectInstance(EventValue);
            if (eventValue == "") eventValue = "(Empty)";
            DateTime now = DateTime.Now;
            ListViewItem l = new ListViewItem(new string[] { now.ToShortTimeString(), eventSource, sender.Name, eventValue });
            l.Tag = now;
            eventListView.Items.Insert(0, l);

            if (deviceTree.SelectedNode != null)
            {
                if (deviceTree.SelectedNode.Tag.GetType() == typeof(OpenSource.UPnP.UPnPStateVariable))
                {
                    if (((UPnPStateVariable)deviceTree.SelectedNode.Tag).SendEvent == true)
                    {
                        if (deviceTree.SelectedNode.Tag.GetHashCode() == sender.GetHashCode())
                        {
                            SetListInfo(deviceTree.SelectedNode.Tag);
                        }
                    }
                }
            }

            TreeNode fNode = deviceTree.Nodes[0].FirstNode;
            while (fNode != null)
            {
                ScanDeviceNode(fNode, sender.OwningService);
                fNode = fNode.NextNode;
            }
        }

        /// <summary>
        /// Scans all service nodes of the visual tree and updated any state variable values that
        /// may have been updated by an incoming event.
        /// </summary>
        /// <param name="dNode"></param>
        /// <param name="s"></param>
        private void ScanDeviceNode(TreeNode dNode, UPnPService s)
        {
            TreeNode pNode = dNode.FirstNode;
            TreeNode vNode;
            while (pNode != null)
            {
                if (pNode.Tag.GetType() == typeof(OpenSource.UPnP.UPnPDevice))
                {
                    ScanDeviceNode(pNode, s);
                }
                else
                {
                    if (pNode.Tag.GetHashCode() == s.GetHashCode())
                    {
                        // This Service
                        vNode = pNode.FirstNode.FirstNode;
                        while (vNode != null)
                        {
                            if (((UPnPStateVariable)vNode.Tag).SendEvent == true)
                            {
                                string stringValue = UPnPService.SerializeObjectInstance(((UPnPStateVariable)vNode.Tag).Value);
                                if (stringValue.Length > 10) stringValue = stringValue.Substring(0, 10) + "...";
                                if (stringValue.Length > 0) stringValue = " [" + stringValue + "]";
                                vNode.Text = ((UPnPStateVariable)vNode.Tag).Name + stringValue;
                            }
                            vNode = vNode.NextNode;
                        }
                        break;
                    }
                }
                pNode = pNode.NextNode;
            }
        }

        private void OnSubscribeService(object sender, System.EventArgs e)
        {
            if (deviceTree.SelectedNode == null) return;
            if (deviceTree.SelectedNode.Tag == null) return;

            Object obj = deviceTree.SelectedNode.Tag;
            if (obj.GetType() == typeof(OpenSource.UPnP.UPnPService))
            {
                if (deviceTree.SelectedNode.Checked == false)
                {
                    splitter2.Visible = true;
                    eventListView.Visible = true;
                    menuItem3.Checked = true;

                    deviceTree.SelectedNode.ImageIndex = 3;
                    deviceTree.SelectedNode.SelectedImageIndex = 3;
                    deviceTree.SelectedNode.Checked = true;
                    ((UPnPService)obj).OnSubscribe += new UPnPService.UPnPEventSubscribeHandler(HandleSubscribe);
                    foreach (UPnPStateVariable V in ((UPnPService)obj).GetStateVariables())
                    {
                        if (V.SendEvent)
                        {
                            V.OnModified += new UPnPStateVariable.ModifiedHandler(HandleEvents);
                        }
                    }
                    ((UPnPService)obj).Subscribe(SubscribeTime, null);
                }
                else
                {
                    deviceTree.SelectedNode.ImageIndex = 2;
                    deviceTree.SelectedNode.SelectedImageIndex = 2;
                    deviceTree.SelectedNode.Checked = false;
                    ((UPnPService)obj).OnSubscribe -= new UPnPService.UPnPEventSubscribeHandler(HandleSubscribe);
                    foreach (UPnPStateVariable V in ((UPnPService)obj).GetStateVariables())
                    {
                        if (V.SendEvent)
                        {
                            V.OnModified -= new UPnPStateVariable.ModifiedHandler(HandleEvents);
                        }
                    }
                    ((UPnPService)obj).UnSubscribe(null);
                }
            }
        }

        private void OnInvokeAction(object sender, System.EventArgs e)
        {
            Object obj = deviceTree.SelectedNode.Tag;
            if (obj != null)
            {
                if (obj.GetType() == typeof(UPnPAction))
                {
                    MethodInvoke mi = new MethodInvoke((UPnPAction)obj, (UPnPService)deviceTree.SelectedNode.Parent.Tag);
                    mi.Show();
                }
            }
        }

        private void menuItem2_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void menuItem3_Click(object sender, System.EventArgs e)
        {
            splitter2.Visible = !eventListView.Visible;
            eventListView.Visible = !eventListView.Visible;
            menuItem3.Checked = eventListView.Visible;
        }

        private void deviceContextMenu_Popup(object sender, System.EventArgs e)
        {
        }

        private void listInfoContextMenu_Popup(object sender, System.EventArgs e)
        {
            if (listInfo.SelectedItems.Count != 0)
            {
                copyValueCpMenuItem.Visible = true;
                string link = listInfo.SelectedItems[0].SubItems[1].Text.ToLower();
                openWebPageMenuItem.Visible = (link.StartsWith("http://") || link.StartsWith("https://"));
            }
            else
            {
                copyValueCpMenuItem.Visible = false;
                openWebPageMenuItem.Visible = false;
            }
        }

        private void copyValueCpMenuItem_Click(object sender, System.EventArgs e)
        {
            if (listInfo.SelectedItems.Count == 0) return;
            Clipboard.SetDataObject(listInfo.SelectedItems[0].SubItems[1].Text);
        }

        private void copyTableCpMenuItem_Click(object sender, System.EventArgs e)
        {
            if (listInfo.Items.Count == 0) return;

            string table = "";
            foreach (ListViewItem l in listInfo.Items)
            {
                table += l.Text + "\t" + l.SubItems[1].Text + "\r\n";
            }
            Clipboard.SetDataObject(table);
        }

        private void ClearEventLogMenuItem_Click(object sender, System.EventArgs e)
        {
            GC.Collect();
            eventListView.Items.Clear();
        }

        private void eventListViewContextMenu_Popup(object sender, System.EventArgs e)
        {
            copyEventCpMenuItem.Visible = (eventListView.SelectedItems.Count != 0);
        }

        private void copyEventCpMenuItem_Click(object sender, System.EventArgs e)
        {
            if (eventListView.SelectedItems.Count == 0) return;
            Clipboard.SetDataObject(
                eventListView.SelectedItems[0].SubItems[0].Text + "\t" +
                eventListView.SelectedItems[0].SubItems[1].Text + "\t" +
                eventListView.SelectedItems[0].SubItems[2].Text + "\t" +
                eventListView.SelectedItems[0].SubItems[3].Text
            );
        }

        private void copyEventLogCpMenuItem_Click(object sender, System.EventArgs e)
        {
            if (eventListView.Items.Count == 0) return;

            string table = "";
            foreach (ListViewItem l in eventListView.Items)
            {
                table += l.SubItems[0].Text + "\t";
                table += l.SubItems[1].Text + "\t";
                table += l.SubItems[2].Text + "\t";
                table += l.SubItems[3].Text + "\r\n";
            }
            Clipboard.SetDataObject(table);
        }

        private void viewStatusbarMenuItem_Click(object sender, System.EventArgs e)
        {
            viewStatusbarMenuItem.Checked = !viewStatusbarMenuItem.Checked;
            statusBar.Visible = viewStatusbarMenuItem.Checked;
        }

        private void expandAllMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnpRoot.ExpandAll();
        }

        private void collapseAllMenuItem_Click(object sender, System.EventArgs e)
        {
            CollapseAll(UPnpRoot);
        }

        private void CollapseAll(TreeNode node)
        {
            if (node == null) return;
            node.Collapse();
            foreach (TreeNode n in node.Nodes)
            {
                CollapseAll(n);
            }
        }

        private void rescanMenuItem_Click(object sender, System.EventArgs e)
        {
            scp.Rescan();
        }

        private void RescanSink(LifeTimeMonitor sender, object obj)
        {
            if (this.InvokeRequired) { Invoke(new LifeTimeMonitor.LifeTimeHandler(RescanSink), sender, obj); return; }
            statusBar.Text = "";
        }

        private void listInfo_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.Modifiers == Keys.Control && e.KeyCode == Keys.C && listInfo.SelectedItems.Count != 0)
            {
                Clipboard.SetDataObject(listInfo.SelectedItems[0].SubItems[1].Text);
            }
        }

        private void eventListView_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.Modifiers == Keys.Control && e.KeyCode == Keys.C && eventListView.SelectedItems.Count != 0)
            {
                Clipboard.SetDataObject(
                    eventListView.SelectedItems[0].SubItems[0].Text + "\t" +
                    eventListView.SelectedItems[0].SubItems[1].Text + "\t" +
                    eventListView.SelectedItems[0].SubItems[2].Text + "\t" +
                    eventListView.SelectedItems[0].SubItems[3].Text
                );
            }
        }

        private void deviceTree_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            eventSubscribeMenuItem.Visible = false;
            invokeActionMenuItem.Visible = false;
            presPageMenuItem.Visible = false;
            menuItem18.Visible = false;
            DeviceXMLmenuItem.Visible = false;
            ServiceXMLmenuItem.Visible = false;
            ValidateActionMenuItem.Visible = false;
            removeDeviceMenuItem.Visible = false;

            TreeNode node = deviceTree.GetNodeAt(e.X, e.Y);
            if (node == null) return;
            deviceTree.SelectedNode = node;
            object infoObject = node.Tag;
            if (infoObject == null) return;

            if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPDevice))
            {
                if (((UPnPDevice)infoObject).ParentDevice == null)
                {
                    removeDeviceMenuItem.Visible = true;
                }
                if (((UPnPDevice)infoObject).HasPresentation == true)
                {
                    presPageMenuItem.Visible = true;
                    menuItem18.Visible = true;
                }
                if (((UPnPDevice)infoObject).LocationURL != null)
                {
                    DeviceXMLmenuItem.Visible = true;
                    menuItem18.Visible = true;
                }
            }

            if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPService))
            {
                ValidateActionMenuItem.Visible = true;
                eventSubscribeMenuItem.Visible = true;
                ServiceXMLmenuItem.Visible = true;
                menuItem18.Visible = true;
                eventSubscribeMenuItem.Checked = node.Checked;
            }

            if (infoObject.GetType() == typeof(OpenSource.UPnP.UPnPAction))
            {
                invokeActionMenuItem.Visible = true;
                menuItem18.Visible = true;
            }
        }

        private void presPageMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)deviceTree.SelectedNode.Tag;
            String URL = d.BaseURL.AbsoluteUri;
            if (d.PresentationURL.StartsWith("/") == true)
            {
                URL += d.PresentationURL.Substring(1);
            }
            else
            {
                if (d.PresentationURL.ToUpper().StartsWith("HTTP://") == true)
                {
                    URL = d.PresentationURL;
                }
                else
                {
                    URL += d.PresentationURL;
                }
            }
            try
            {
                System.Diagnostics.Process.Start(URL);
            }
            catch (System.ComponentModel.Win32Exception) { }
        }

        private void DeviceXMLmenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)deviceTree.SelectedNode.Tag;
            try
            {
                System.Diagnostics.Process.Start(d.LocationURL);
            }
            catch (System.ComponentModel.Win32Exception) { }
        }

        private void ServiceXMLmenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPService s = (UPnPService)deviceTree.SelectedNode.Tag;
            try
            {
                System.Diagnostics.Process.Start((string)(new UPnPDebugObject(s)).GetField("SCPDURL"));
            }
            catch (System.ComponentModel.Win32Exception) { }
        }

        private void listInfo_DoubleClick(object sender, System.EventArgs e)
        {
            if (listInfo.SelectedItems.Count != 1) return;
            if (listInfo.SelectedItems[0].SubItems[1].Text.StartsWith("http://") == true)
            {
                try
                {
                    System.Diagnostics.Process.Start(listInfo.SelectedItems[0].SubItems[1].Text);
                }
                catch (System.ComponentModel.Win32Exception) { }
            }
        }

        private void ValidateActionMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPService s = (UPnPService)deviceTree.SelectedNode.Tag;
            ValidationForm vf = new ValidationForm(s);
            vf.ShowDialog();
        }

        private void showDebugInfoMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Spy");
        }

        private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Spy");
        }

        private void removeDeviceMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)deviceTree.SelectedNode.Tag;
            scp.ForceDisposeDevice(d);
        }

        private void manuallyAddDeviceMenuItem_Click(object sender, System.EventArgs e)
        {
            ForceLoad fl = new ForceLoad();
            if (fl.ShowDialog() == DialogResult.OK)
            {
                UPnPDeviceFactory df = new UPnPDeviceFactory(fl.NetworkUri, 1800, new UPnPDeviceFactory.UPnPDeviceHandler(ForceDeviceOKSink), new UPnPDeviceFactory.UPnPDeviceFailedHandler(ForceDeviceFailSink), null, null);
                ForceDeviceList[df] = df;
            }
            fl.Dispose();
        }
        private void ForceDeviceOKSink(UPnPDeviceFactory sender, UPnPDevice d, Uri LocationUri)
        {
            ForceDeviceList.Remove(sender);
            this.HandleCreate(d, LocationUri);
        }
        private void ForceDeviceFailSink(UPnPDeviceFactory sender, Uri LocationUri, Exception e, string urn)
        {
            ForceDeviceList.Remove(sender);
            MessageBox.Show("Could not load device!");
        }

        private void menuItem5_Popup(object sender, EventArgs e)
        {
            menuItem6.Checked = AutoUpdate.GetAutoUpdateCheck();
        }

        private void menuItem6_Click(object sender, EventArgs e)
        {
            AutoUpdate.SetAutoUpdateCheck(!menuItem6.Checked);
            if (!menuItem6.Checked) AutoUpdate.UpdateCheck(this);
        }

        private void openWebPageMenuItem_Click(object sender, EventArgs e)
        {
            if (listInfo.SelectedItems.Count == 0) return;
            string link = listInfo.SelectedItems[0].SubItems[1].Text;
            string llink = link.ToLower();
            if (llink.StartsWith("http://") || llink.StartsWith("https://"))
            {
                try
                {
                    System.Diagnostics.Process.Start(link);
                }
                catch (System.ComponentModel.Win32Exception) { }
            }
        }

    }
}
