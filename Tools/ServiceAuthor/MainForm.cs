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
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for MainForm.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private OpenSource.UPnP.UPnPService upnpService = null;
		private System.Text.UTF8Encoding UTF8 = new System.Text.UTF8Encoding();
		private string MainFormTitle;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.ImageList iconImageList;
        private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.MenuItem loadScpdMenuItem;
		private System.Windows.Forms.MenuItem saveScpdMenuItem;
		private System.Windows.Forms.MenuItem exitMenuItem;
		private System.Windows.Forms.OpenFileDialog openScpdDialog;
		private System.Windows.Forms.SaveFileDialog saveScpdDialog;
		private System.Windows.Forms.MenuItem saveScpdAsMenuItem;
        private System.Windows.Forms.MenuItem helpMenuItem;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem13;
		private System.Windows.Forms.MenuItem addVariableMenuItem;
		private System.Windows.Forms.MenuItem removeVariableMenuItem;
		private System.Windows.Forms.ContextMenu actionContextMenu;
		private System.Windows.Forms.ContextMenu stateContextMenu;
		private System.Windows.Forms.MenuItem menuItem19;
		private System.Windows.Forms.MenuItem addActionMenuItem;
		private System.Windows.Forms.MenuItem newMenuItem;
		private System.Windows.Forms.MenuItem removeActionMenuItem;
		private System.Windows.Forms.MenuItem editStateMenuItem;
		private System.Windows.Forms.MenuItem removeStateMenuItem;
		private System.Windows.Forms.MenuItem addStateMenuItem;
		private System.Windows.Forms.MenuItem stateSeperatorMenuItem;
		private System.Windows.Forms.MenuItem editActionMenuItem;
		private System.Windows.Forms.MenuItem removeActionMenuItem2;
		private System.Windows.Forms.MenuItem addNewActionMenuItem;
		private System.Windows.Forms.MenuItem actionSeperatorMenuItem;
		private System.Windows.Forms.MenuItem openFromNetworkMenuItem;
		private System.Windows.Forms.MenuItem addComplexTypeMenuItem;
		private System.Windows.Forms.MenuItem removeComplexTypeMenuItem;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.ListView actionListView;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.ListView stateVariableListView;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.TreeView TypesView;
		private System.Windows.Forms.ContextMenu RootContextMenu;
		private System.Windows.Forms.MenuItem AddComplexType;
		private System.Windows.Forms.MenuItem RemoveComplexType;
		private System.Windows.Forms.MenuItem Properties;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.ContextMenu ComplexTypes_SubContextMenu;
		private System.Windows.Forms.MenuItem AddField;
		private System.Windows.Forms.MenuItem RemoveField;
		private System.Windows.Forms.MenuItem menuItem12;
		private System.Windows.Forms.MenuItem menuItem9;
		private System.Windows.Forms.MenuItem RemoveSequence;
		private System.Windows.Forms.MenuItem RemoveChoice;
		private System.Windows.Forms.MenuItem AddChoice;
		private System.Windows.Forms.MenuItem AddSequence;
		private System.Windows.Forms.MenuItem AddSequence_root;
		private System.Windows.Forms.MenuItem AddChoice_root;
		private System.Windows.Forms.MenuItem menuItem11;
		private System.Windows.Forms.MenuItem AddComplexContent;
		private System.Windows.Forms.MenuItem AddSimpleContent;
		private System.Windows.Forms.MenuItem menuItem14;
		private System.Windows.Forms.MenuItem RemoveComplexContent;
		private System.Windows.Forms.MenuItem RemoveSimpleContent;
		private System.Windows.Forms.ContextMenu ContentData_ContextMenu;
		private System.Windows.Forms.MenuItem RemoveFieldMenuItem;
		private System.Windows.Forms.MenuItem EditField;
		private System.Windows.Forms.MenuItem menuItem15;
		private System.Windows.Forms.MenuItem MoveUp;
		private System.Windows.Forms.MenuItem MoveDown;
		private System.Windows.Forms.MenuItem SequenceChoice_Property;
		private System.Windows.Forms.MenuItem Root_AddField;
		private System.Windows.Forms.MenuItem OpenXSDItem;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.TreeView CTPTreeView;
		private System.Windows.Forms.MainMenu mainMenu;

		public MainForm(string[] args)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			string emptyScpd = "<?xml version=\"1.0\" encoding=\"utf-8\"?><scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"><specVersion><major>1</major><minor>0</minor></specVersion><serviceStateTable></serviceStateTable></scpd>";
			upnpService = UPnPService.FromSCPD(emptyScpd);
			openScpdDialog.FileName = "";
			MainFormTitle = Text;

			if(args.Length==1)
			{
				openScpdDialog.FileName = args[0];
				this.openFile(new FileInfo(args[0]));
			}

//			TestForm t = new TestForm();
//			t.ShowDialog();
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
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.newMenuItem = new System.Windows.Forms.MenuItem();
            this.loadScpdMenuItem = new System.Windows.Forms.MenuItem();
            this.openFromNetworkMenuItem = new System.Windows.Forms.MenuItem();
            this.saveScpdMenuItem = new System.Windows.Forms.MenuItem();
            this.saveScpdAsMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem13 = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.OpenXSDItem = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.addActionMenuItem = new System.Windows.Forms.MenuItem();
            this.removeActionMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem19 = new System.Windows.Forms.MenuItem();
            this.addVariableMenuItem = new System.Windows.Forms.MenuItem();
            this.removeVariableMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.actionContextMenu = new System.Windows.Forms.ContextMenu();
            this.editActionMenuItem = new System.Windows.Forms.MenuItem();
            this.removeActionMenuItem2 = new System.Windows.Forms.MenuItem();
            this.actionSeperatorMenuItem = new System.Windows.Forms.MenuItem();
            this.addNewActionMenuItem = new System.Windows.Forms.MenuItem();
            this.iconImageList = new System.Windows.Forms.ImageList(this.components);
            this.stateContextMenu = new System.Windows.Forms.ContextMenu();
            this.editStateMenuItem = new System.Windows.Forms.MenuItem();
            this.removeStateMenuItem = new System.Windows.Forms.MenuItem();
            this.removeComplexTypeMenuItem = new System.Windows.Forms.MenuItem();
            this.stateSeperatorMenuItem = new System.Windows.Forms.MenuItem();
            this.addStateMenuItem = new System.Windows.Forms.MenuItem();
            this.addComplexTypeMenuItem = new System.Windows.Forms.MenuItem();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.openScpdDialog = new System.Windows.Forms.OpenFileDialog();
            this.saveScpdDialog = new System.Windows.Forms.SaveFileDialog();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.stateVariableListView = new System.Windows.Forms.ListView();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.actionListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.TypesView = new System.Windows.Forms.TreeView();
            this.RootContextMenu = new System.Windows.Forms.ContextMenu();
            this.AddComplexType = new System.Windows.Forms.MenuItem();
            this.RemoveComplexType = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.AddSequence_root = new System.Windows.Forms.MenuItem();
            this.AddChoice_root = new System.Windows.Forms.MenuItem();
            this.Root_AddField = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.AddComplexContent = new System.Windows.Forms.MenuItem();
            this.AddSimpleContent = new System.Windows.Forms.MenuItem();
            this.RemoveComplexContent = new System.Windows.Forms.MenuItem();
            this.RemoveSimpleContent = new System.Windows.Forms.MenuItem();
            this.menuItem14 = new System.Windows.Forms.MenuItem();
            this.Properties = new System.Windows.Forms.MenuItem();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.CTPTreeView = new System.Windows.Forms.TreeView();
            this.ComplexTypes_SubContextMenu = new System.Windows.Forms.ContextMenu();
            this.AddSequence = new System.Windows.Forms.MenuItem();
            this.AddChoice = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.RemoveSequence = new System.Windows.Forms.MenuItem();
            this.RemoveChoice = new System.Windows.Forms.MenuItem();
            this.SequenceChoice_Property = new System.Windows.Forms.MenuItem();
            this.menuItem12 = new System.Windows.Forms.MenuItem();
            this.AddField = new System.Windows.Forms.MenuItem();
            this.RemoveField = new System.Windows.Forms.MenuItem();
            this.ContentData_ContextMenu = new System.Windows.Forms.ContextMenu();
            this.RemoveFieldMenuItem = new System.Windows.Forms.MenuItem();
            this.EditField = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.MoveUp = new System.Windows.Forms.MenuItem();
            this.MoveDown = new System.Windows.Forms.MenuItem();
            this.tabControl1.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem2,
            this.menuItem6});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.newMenuItem,
            this.loadScpdMenuItem,
            this.openFromNetworkMenuItem,
            this.saveScpdMenuItem,
            this.saveScpdAsMenuItem,
            this.menuItem13,
            this.menuItem3,
            this.OpenXSDItem,
            this.menuItem7,
            this.exitMenuItem});
            resources.ApplyResources(this.menuItem1, "menuItem1");
            // 
            // newMenuItem
            // 
            this.newMenuItem.Index = 0;
            resources.ApplyResources(this.newMenuItem, "newMenuItem");
            this.newMenuItem.Click += new System.EventHandler(this.newMenuItem_Click);
            // 
            // loadScpdMenuItem
            // 
            this.loadScpdMenuItem.Index = 1;
            resources.ApplyResources(this.loadScpdMenuItem, "loadScpdMenuItem");
            this.loadScpdMenuItem.Click += new System.EventHandler(this.loadScpdMenuItem_Click);
            // 
            // openFromNetworkMenuItem
            // 
            this.openFromNetworkMenuItem.Index = 2;
            resources.ApplyResources(this.openFromNetworkMenuItem, "openFromNetworkMenuItem");
            this.openFromNetworkMenuItem.Click += new System.EventHandler(this.openFromNetworkMenuItem_Click);
            // 
            // saveScpdMenuItem
            // 
            this.saveScpdMenuItem.Index = 3;
            resources.ApplyResources(this.saveScpdMenuItem, "saveScpdMenuItem");
            this.saveScpdMenuItem.Click += new System.EventHandler(this.saveScpdMenuItem_Click);
            // 
            // saveScpdAsMenuItem
            // 
            this.saveScpdAsMenuItem.Index = 4;
            resources.ApplyResources(this.saveScpdAsMenuItem, "saveScpdAsMenuItem");
            this.saveScpdAsMenuItem.Click += new System.EventHandler(this.saveScpdAsMenuItem_Click);
            // 
            // menuItem13
            // 
            this.menuItem13.Index = 5;
            resources.ApplyResources(this.menuItem13, "menuItem13");
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 6;
            resources.ApplyResources(this.menuItem3, "menuItem3");
            this.menuItem3.Click += new System.EventHandler(this.menuItem3_Click);
            // 
            // OpenXSDItem
            // 
            this.OpenXSDItem.Index = 7;
            resources.ApplyResources(this.OpenXSDItem, "OpenXSDItem");
            this.OpenXSDItem.Click += new System.EventHandler(this.OpenXSDItem_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 8;
            resources.ApplyResources(this.menuItem7, "menuItem7");
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 9;
            resources.ApplyResources(this.exitMenuItem, "exitMenuItem");
            this.exitMenuItem.Click += new System.EventHandler(this.menuItem5_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 1;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.addActionMenuItem,
            this.removeActionMenuItem,
            this.menuItem19,
            this.addVariableMenuItem,
            this.removeVariableMenuItem});
            resources.ApplyResources(this.menuItem2, "menuItem2");
            // 
            // addActionMenuItem
            // 
            this.addActionMenuItem.Index = 0;
            resources.ApplyResources(this.addActionMenuItem, "addActionMenuItem");
            this.addActionMenuItem.Click += new System.EventHandler(this.addActionMenuItem_Click);
            // 
            // removeActionMenuItem
            // 
            this.removeActionMenuItem.Index = 1;
            resources.ApplyResources(this.removeActionMenuItem, "removeActionMenuItem");
            this.removeActionMenuItem.Click += new System.EventHandler(this.removeActionMenuItem_Click);
            // 
            // menuItem19
            // 
            this.menuItem19.Index = 2;
            resources.ApplyResources(this.menuItem19, "menuItem19");
            // 
            // addVariableMenuItem
            // 
            this.addVariableMenuItem.Index = 3;
            resources.ApplyResources(this.addVariableMenuItem, "addVariableMenuItem");
            this.addVariableMenuItem.Click += new System.EventHandler(this.addVariableMenuItem_Click);
            // 
            // removeVariableMenuItem
            // 
            this.removeVariableMenuItem.Index = 4;
            resources.ApplyResources(this.removeVariableMenuItem, "removeVariableMenuItem");
            this.removeVariableMenuItem.Click += new System.EventHandler(this.removeVariableMenuItem_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 2;
            this.menuItem6.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem});
            resources.ApplyResources(this.menuItem6, "menuItem6");
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            resources.ApplyResources(this.helpMenuItem, "helpMenuItem");
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // actionContextMenu
            // 
            this.actionContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.editActionMenuItem,
            this.removeActionMenuItem2,
            this.actionSeperatorMenuItem,
            this.addNewActionMenuItem});
            // 
            // editActionMenuItem
            // 
            this.editActionMenuItem.DefaultItem = true;
            this.editActionMenuItem.Index = 0;
            resources.ApplyResources(this.editActionMenuItem, "editActionMenuItem");
            this.editActionMenuItem.Click += new System.EventHandler(this.actionListView_DoubleClick);
            // 
            // removeActionMenuItem2
            // 
            this.removeActionMenuItem2.Index = 1;
            resources.ApplyResources(this.removeActionMenuItem2, "removeActionMenuItem2");
            this.removeActionMenuItem2.Click += new System.EventHandler(this.removeActionMenuItem_Click);
            // 
            // actionSeperatorMenuItem
            // 
            this.actionSeperatorMenuItem.Index = 2;
            resources.ApplyResources(this.actionSeperatorMenuItem, "actionSeperatorMenuItem");
            // 
            // addNewActionMenuItem
            // 
            this.addNewActionMenuItem.Index = 3;
            resources.ApplyResources(this.addNewActionMenuItem, "addNewActionMenuItem");
            this.addNewActionMenuItem.Click += new System.EventHandler(this.addActionMenuItem_Click);
            // 
            // iconImageList
            // 
            this.iconImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("iconImageList.ImageStream")));
            this.iconImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.iconImageList.Images.SetKeyName(0, "");
            this.iconImageList.Images.SetKeyName(1, "");
            // 
            // stateContextMenu
            // 
            this.stateContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.editStateMenuItem,
            this.removeStateMenuItem,
            this.removeComplexTypeMenuItem,
            this.stateSeperatorMenuItem,
            this.addStateMenuItem,
            this.addComplexTypeMenuItem});
            // 
            // editStateMenuItem
            // 
            this.editStateMenuItem.DefaultItem = true;
            this.editStateMenuItem.Index = 0;
            resources.ApplyResources(this.editStateMenuItem, "editStateMenuItem");
            this.editStateMenuItem.Click += new System.EventHandler(this.stateVariableListView_DoubleClick);
            // 
            // removeStateMenuItem
            // 
            this.removeStateMenuItem.Index = 1;
            resources.ApplyResources(this.removeStateMenuItem, "removeStateMenuItem");
            this.removeStateMenuItem.Click += new System.EventHandler(this.removeVariableMenuItem_Click);
            // 
            // removeComplexTypeMenuItem
            // 
            this.removeComplexTypeMenuItem.Index = 2;
            resources.ApplyResources(this.removeComplexTypeMenuItem, "removeComplexTypeMenuItem");
            this.removeComplexTypeMenuItem.Click += new System.EventHandler(this.removeComplexTypeMenuItem_Click);
            // 
            // stateSeperatorMenuItem
            // 
            this.stateSeperatorMenuItem.Index = 3;
            resources.ApplyResources(this.stateSeperatorMenuItem, "stateSeperatorMenuItem");
            // 
            // addStateMenuItem
            // 
            this.addStateMenuItem.Index = 4;
            resources.ApplyResources(this.addStateMenuItem, "addStateMenuItem");
            this.addStateMenuItem.Click += new System.EventHandler(this.addVariableMenuItem_Click);
            // 
            // addComplexTypeMenuItem
            // 
            this.addComplexTypeMenuItem.Index = 5;
            resources.ApplyResources(this.addComplexTypeMenuItem, "addComplexTypeMenuItem");
            this.addComplexTypeMenuItem.Click += new System.EventHandler(this.addComplexTypeMenuItem_Click);
            // 
            // splitter1
            // 
            resources.ApplyResources(this.splitter1, "splitter1");
            this.splitter1.Name = "splitter1";
            this.splitter1.TabStop = false;
            // 
            // openScpdDialog
            // 
            resources.ApplyResources(this.openScpdDialog, "openScpdDialog");
            // 
            // saveScpdDialog
            // 
            resources.ApplyResources(this.saveScpdDialog, "saveScpdDialog");
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage4);
            resources.ApplyResources(this.tabControl1, "tabControl1");
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.stateVariableListView);
            resources.ApplyResources(this.tabPage3, "tabPage3");
            this.tabPage3.Name = "tabPage3";
            // 
            // stateVariableListView
            // 
            this.stateVariableListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
            this.stateVariableListView.ContextMenu = this.stateContextMenu;
            resources.ApplyResources(this.stateVariableListView, "stateVariableListView");
            this.stateVariableListView.FullRowSelect = true;
            this.stateVariableListView.Name = "stateVariableListView";
            this.stateVariableListView.SmallImageList = this.iconImageList;
            this.stateVariableListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.stateVariableListView.UseCompatibleStateImageBehavior = false;
            this.stateVariableListView.View = System.Windows.Forms.View.Details;
            this.stateVariableListView.DoubleClick += new System.EventHandler(this.stateVariableListView_DoubleClick);
            this.stateVariableListView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.stateVariableListView_MouseDown);
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
            // columnHeader6
            // 
            resources.ApplyResources(this.columnHeader6, "columnHeader6");
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.actionListView);
            resources.ApplyResources(this.tabPage1, "tabPage1");
            this.tabPage1.Name = "tabPage1";
            // 
            // actionListView
            // 
            this.actionListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.actionListView.ContextMenu = this.actionContextMenu;
            resources.ApplyResources(this.actionListView, "actionListView");
            this.actionListView.FullRowSelect = true;
            this.actionListView.MultiSelect = false;
            this.actionListView.Name = "actionListView";
            this.actionListView.SmallImageList = this.iconImageList;
            this.actionListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.actionListView.UseCompatibleStateImageBehavior = false;
            this.actionListView.View = System.Windows.Forms.View.Details;
            this.actionListView.DoubleClick += new System.EventHandler(this.actionListView_DoubleClick);
            this.actionListView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.actionListView_MouseDown);
            // 
            // columnHeader1
            // 
            resources.ApplyResources(this.columnHeader1, "columnHeader1");
            // 
            // columnHeader2
            // 
            resources.ApplyResources(this.columnHeader2, "columnHeader2");
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.TypesView);
            resources.ApplyResources(this.tabPage2, "tabPage2");
            this.tabPage2.Name = "tabPage2";
            // 
            // TypesView
            // 
            this.TypesView.ContextMenu = this.RootContextMenu;
            resources.ApplyResources(this.TypesView, "TypesView");
            this.TypesView.ItemHeight = 16;
            this.TypesView.Name = "TypesView";
            this.TypesView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnMouseDown);
            // 
            // RootContextMenu
            // 
            this.RootContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.AddComplexType,
            this.RemoveComplexType,
            this.menuItem4,
            this.AddSequence_root,
            this.AddChoice_root,
            this.Root_AddField,
            this.menuItem11,
            this.AddComplexContent,
            this.AddSimpleContent,
            this.RemoveComplexContent,
            this.RemoveSimpleContent,
            this.menuItem14,
            this.Properties});
            // 
            // AddComplexType
            // 
            this.AddComplexType.Index = 0;
            resources.ApplyResources(this.AddComplexType, "AddComplexType");
            this.AddComplexType.Click += new System.EventHandler(this.AddComplexType_Click);
            // 
            // RemoveComplexType
            // 
            resources.ApplyResources(this.RemoveComplexType, "RemoveComplexType");
            this.RemoveComplexType.Index = 1;
            this.RemoveComplexType.Click += new System.EventHandler(this.RemoveComplexType_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            resources.ApplyResources(this.menuItem4, "menuItem4");
            // 
            // AddSequence_root
            // 
            resources.ApplyResources(this.AddSequence_root, "AddSequence_root");
            this.AddSequence_root.Index = 3;
            this.AddSequence_root.Click += new System.EventHandler(this.OnClick_AddSequence);
            // 
            // AddChoice_root
            // 
            resources.ApplyResources(this.AddChoice_root, "AddChoice_root");
            this.AddChoice_root.Index = 4;
            this.AddChoice_root.Click += new System.EventHandler(this.OnClick_AddChoice);
            // 
            // Root_AddField
            // 
            resources.ApplyResources(this.Root_AddField, "Root_AddField");
            this.Root_AddField.Index = 5;
            this.Root_AddField.Click += new System.EventHandler(this.OnAdd_Field);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 6;
            resources.ApplyResources(this.menuItem11, "menuItem11");
            // 
            // AddComplexContent
            // 
            resources.ApplyResources(this.AddComplexContent, "AddComplexContent");
            this.AddComplexContent.Index = 7;
            this.AddComplexContent.Click += new System.EventHandler(this.OnAdd_ComplexContent);
            // 
            // AddSimpleContent
            // 
            resources.ApplyResources(this.AddSimpleContent, "AddSimpleContent");
            this.AddSimpleContent.Index = 8;
            this.AddSimpleContent.Click += new System.EventHandler(this.OnAdd_SimpleContent);
            // 
            // RemoveComplexContent
            // 
            resources.ApplyResources(this.RemoveComplexContent, "RemoveComplexContent");
            this.RemoveComplexContent.Index = 9;
            this.RemoveComplexContent.Click += new System.EventHandler(this.OnRemove_SimpleComplex_Content);
            // 
            // RemoveSimpleContent
            // 
            resources.ApplyResources(this.RemoveSimpleContent, "RemoveSimpleContent");
            this.RemoveSimpleContent.Index = 10;
            this.RemoveSimpleContent.Click += new System.EventHandler(this.OnRemove_SimpleComplex_Content);
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 11;
            resources.ApplyResources(this.menuItem14, "menuItem14");
            // 
            // Properties
            // 
            resources.ApplyResources(this.Properties, "Properties");
            this.Properties.Index = 12;
            this.Properties.Click += new System.EventHandler(this.Properties_Click);
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.CTPTreeView);
            resources.ApplyResources(this.tabPage4, "tabPage4");
            this.tabPage4.Name = "tabPage4";
            // 
            // CTPTreeView
            // 
            resources.ApplyResources(this.CTPTreeView, "CTPTreeView");
            this.CTPTreeView.ItemHeight = 16;
            this.CTPTreeView.Name = "CTPTreeView";
            // 
            // ComplexTypes_SubContextMenu
            // 
            this.ComplexTypes_SubContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.AddSequence,
            this.AddChoice,
            this.menuItem9,
            this.RemoveSequence,
            this.RemoveChoice,
            this.SequenceChoice_Property,
            this.menuItem12,
            this.AddField,
            this.RemoveField});
            // 
            // AddSequence
            // 
            this.AddSequence.Index = 0;
            resources.ApplyResources(this.AddSequence, "AddSequence");
            this.AddSequence.Click += new System.EventHandler(this.OnClick_AddSequence);
            // 
            // AddChoice
            // 
            this.AddChoice.Index = 1;
            resources.ApplyResources(this.AddChoice, "AddChoice");
            this.AddChoice.Click += new System.EventHandler(this.OnClick_AddChoice);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 2;
            resources.ApplyResources(this.menuItem9, "menuItem9");
            // 
            // RemoveSequence
            // 
            this.RemoveSequence.Index = 3;
            resources.ApplyResources(this.RemoveSequence, "RemoveSequence");
            this.RemoveSequence.Click += new System.EventHandler(this.OnRemove_SequenceChoice);
            // 
            // RemoveChoice
            // 
            this.RemoveChoice.Index = 4;
            resources.ApplyResources(this.RemoveChoice, "RemoveChoice");
            this.RemoveChoice.Click += new System.EventHandler(this.OnRemove_SequenceChoice);
            // 
            // SequenceChoice_Property
            // 
            this.SequenceChoice_Property.Index = 5;
            resources.ApplyResources(this.SequenceChoice_Property, "SequenceChoice_Property");
            this.SequenceChoice_Property.Click += new System.EventHandler(this.SequenceChoice_Property_Click);
            // 
            // menuItem12
            // 
            this.menuItem12.Index = 6;
            resources.ApplyResources(this.menuItem12, "menuItem12");
            // 
            // AddField
            // 
            this.AddField.Index = 7;
            resources.ApplyResources(this.AddField, "AddField");
            this.AddField.Click += new System.EventHandler(this.OnAdd_Field);
            // 
            // RemoveField
            // 
            this.RemoveField.Index = 8;
            resources.ApplyResources(this.RemoveField, "RemoveField");
            // 
            // ContentData_ContextMenu
            // 
            this.ContentData_ContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.RemoveFieldMenuItem,
            this.EditField,
            this.menuItem15,
            this.MoveUp,
            this.MoveDown});
            // 
            // RemoveFieldMenuItem
            // 
            this.RemoveFieldMenuItem.Index = 0;
            resources.ApplyResources(this.RemoveFieldMenuItem, "RemoveFieldMenuItem");
            this.RemoveFieldMenuItem.Click += new System.EventHandler(this.RemoveFieldMenuItem_Click);
            // 
            // EditField
            // 
            this.EditField.Index = 1;
            resources.ApplyResources(this.EditField, "EditField");
            this.EditField.Click += new System.EventHandler(this.EditField_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 2;
            resources.ApplyResources(this.menuItem15, "menuItem15");
            // 
            // MoveUp
            // 
            this.MoveUp.Index = 3;
            resources.ApplyResources(this.MoveUp, "MoveUp");
            this.MoveUp.Click += new System.EventHandler(this.MoveUp_Click);
            // 
            // MoveDown
            // 
            this.MoveDown.Index = 4;
            resources.ApplyResources(this.MoveDown, "MoveDown");
            this.MoveDown.Click += new System.EventHandler(this.MoveDown_Click);
            // 
            // MainForm
            // 
            this.AllowDrop = true;
            resources.ApplyResources(this, "$this");
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.splitter1);
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.MainForm_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.MainForm_DragEnter);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.tabControl1.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args) 
		{
			for (int i=0;i<(args.Length);i++) 
			{
				if (args[i].ToLower() == "-en") System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("en");
				if (args[i].ToLower() == "-fr") System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo("fr");
			}
			Application.Run(new MainForm(args));
		}

		private void menuItem5_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void loadScpdMenuItem_Click(object sender, System.EventArgs e)
		{
			DialogResult r = openScpdDialog.ShowDialog();
			if (r == DialogResult.OK) 
			{
				openFile(new FileInfo(openScpdDialog.FileName));
			}
		}

		private void openFile(FileInfo filename) 
		{
			if (filename.Exists == false) return;
			System.IO.FileStream fs = new System.IO.FileStream(filename.FullName,System.IO.FileMode.Open,System.IO.FileAccess.Read);
			byte[] buffer = new Byte[(int)fs.Length];
			fs.Read(buffer,0,buffer.Length);
			fs.Close();
			string xml = UTF8.GetString(buffer);
			xml = xml.Substring(xml.IndexOf("<"));
			upnpService = UPnPService.FromSCPD(xml);
			updateUserInterface();
			this.Text = MainFormTitle + " - " + filename.Name;
		}

		private void updateUserInterface() 
		{
			int imidx = 0;
			actionListView.Items.Clear();
			stateVariableListView.Items.Clear();
			if (upnpService == null) return;

			foreach (OpenSource.UPnP.UPnPAction action in upnpService.Actions) 
			{
				string argsstr = "";
				foreach (UPnPArgument arg in action.ArgumentList)
				{
					if (argsstr != "") argsstr += ", ";
					if(arg.RelatedStateVar!=null)
					{
						argsstr += arg.RelatedStateVar.ValueType + " " + arg.Name;
					}
					else
					{
						MessageBox.Show("Action: " + action.Name + " contains invalid state variable declaration in arg: " + arg.Name);
					}
				}
				imidx = 0;
				if(action.Name.EndsWith(" ")) imidx = 1;
				ListViewItem item = new ListViewItem(new string[] {action.Name,argsstr},imidx);
				item.Tag = action;
				actionListView.Items.Add(item);
			}

			OpenSource.UPnP.UPnPStateVariable[] stateVariables = upnpService.GetStateVariables();
			foreach (UPnPStateVariable var in stateVariables) 
			{
				string infoString = "";

				if (var.Minimum != null) 
				{
					if (infoString.Length != 0) infoString += " ";
					infoString += "Min: " + var.Minimum.ToString();
				}

				if (var.Maximum != null) 
				{
					if (infoString.Length != 0) infoString += " ";
					infoString += "Max: " + var.Maximum.ToString();
				}

				if (var.Step != null) 
				{
					if (infoString.Length != 0) infoString += " ";
					infoString += "Step: " + var.Step.ToString();
				}

				if (var.DefaultValue != null) 
				{
					if (infoString.Length != 0) infoString += " ";
					infoString += "Default value: \"" + var.DefaultValue.ToString() + "\"";
				}

				if (var.AllowedStringValues != null) 
				{
					if (infoString.Length != 0) infoString += " ";
					infoString += "Allowed values: \"";
					bool first = true;
					foreach (string v in var.AllowedStringValues)
					{
						if (first == false) infoString += ", ";
						infoString += v;
						first = false;
					}
					infoString += "\"";
				}
				if(var.ComplexType!=null)
				{
					infoString = var.ComplexType.Name_NAMESPACE;
				}

				imidx = 1;
				ListViewItem item;
				if(var.Name.EndsWith(" ")) imidx = 0;
				if(var.ComplexType==null)
				{
					item = new ListViewItem(new string[] {var.Name,var.ValueType,var.SendEvent.ToString(),infoString},imidx);
				}
				else
				{
					item = new ListViewItem(new string[] {var.Name,var.ComplexType.Name_LOCAL,var.SendEvent.ToString(),infoString},imidx);	
				}
				item.Tag = var;
				stateVariableListView.Items.Add(item);
			}
			foreach (UPnPComplexType CT in upnpService.GetComplexTypeList())
			{
//				ListViewItem item = new ListViewItem(new string[] {CT.Name_LOCAL,"Complex","",CT.Name_NAMESPACE},0);
//				item.Tag = CT;
//				stateVariableListView.Items.Add(item);

				this.AddComplexTypeToDisplayTree(CT,TypesView);
			}
			foreach(UPnPComplexType.Group G in upnpService.GetComplexTypeList_Group())
			{
				this.AddComplexTypeToDisplayTree(G,CTPTreeView);
			}
		}

		private void saveScpdMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			if (openScpdDialog.FileName.Length == 0) 
			{
				saveScpdAsMenuItem_Click(this,e); // Call Save As...
				return;
			}

			byte[] serviceXmlByteArray = upnpService.GetSCPDXml();
			System.IO.FileStream fs = new System.IO.FileStream(openScpdDialog.FileName,System.IO.FileMode.Create,System.IO.FileAccess.Write);
			fs.Write(serviceXmlByteArray,0,serviceXmlByteArray.Length);
			fs.Close();
		}

		private void saveScpdAsMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			try
			{
				saveScpdDialog.FileName = openScpdDialog.FileName;
			}
			catch
			{}
			DialogResult r = saveScpdDialog.ShowDialog();
			if (r == DialogResult.OK)
			{
				byte[] serviceXmlByteArray = upnpService.GetSCPDXml();
				System.IO.FileStream fs = new System.IO.FileStream(saveScpdDialog.FileName,System.IO.FileMode.Create,System.IO.FileAccess.Write);
				fs.Write(serviceXmlByteArray,0,serviceXmlByteArray.Length);
				fs.Close();
				try
				{
					openScpdDialog.FileName = saveScpdDialog.FileName;
				}
				catch(Exception)
				{
					openScpdDialog.FileName = "scpd.xml";
				}

				string shortname = openScpdDialog.FileName;
				int i = shortname.IndexOf("\\");
				while (i > 0) 
				{
					shortname = shortname.Substring(i+1);
					i = shortname.IndexOf("\\");
				}
				this.Text = MainFormTitle + " - " + shortname;
			}
		}


		private void addVariableMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			StateVariableEditForm stateVariableEditForm = new StateVariableEditForm(upnpService);
			DialogResult r = stateVariableEditForm.ShowDialog(this);
			if (r == DialogResult.OK) 
			{
				UPnPStateVariable var = stateVariableEditForm.StateVariable;
				if (var != null) 
				{
					upnpService.AddStateVariable(var);
					updateUserInterface();
				}
			}
		}

		private void stateVariableListView_DoubleClick(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			if (stateVariableListView.SelectedItems.Count == 1)
			{
				UPnPStateVariable var = (UPnPStateVariable)stateVariableListView.SelectedItems[0].Tag;
				StateVariableEditForm stateVariableEditForm = new StateVariableEditForm();
				stateVariableEditForm.StateVariable = var;
				DialogResult r = stateVariableEditForm.ShowDialog(this);
				if (r == DialogResult.OK) 
				{
					if(var.Name==stateVariableEditForm.StateVariable.Name)
					{
						upnpService.AddStateVariable(stateVariableEditForm.StateVariable);
					}
					else
					{
						try
						{
							upnpService.RemoveStateVariable(var);
							upnpService.AddStateVariable(stateVariableEditForm.StateVariable);
						}
						catch(UPnPStateVariable.CannotRemoveException)
						{
							MessageBox.Show("You cannot remove this State variable because it is associated with an existing action");
							return;
						}
					}
					updateUserInterface();
				}
			}
		}

		private void actionListView_DoubleClick(object sender, System.EventArgs e)
		{
			UPnPService ts;
			if (upnpService == null) return;

			if (actionListView.SelectedItems.Count == 1)
			{
				UPnPAction act = (UPnPAction)actionListView.SelectedItems[0].Tag;
				ts = (UPnPService)upnpService.Clone();
				ActionEditForm actionEditForm = new ActionEditForm(ts);
				actionEditForm.Action = ts.GetAction(act.Name);
				DialogResult r = actionEditForm.ShowDialog(this);
				if (r == DialogResult.OK) 
				{
					upnpService.RemoveMethod(act);
					act = actionEditForm.Action;
					upnpService.AddMethod(actionEditForm.Action);
					updateUserInterface();
				}
			}
		}

		private void addActionMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			ActionEditForm actionEditForm = new ActionEditForm(upnpService);
			DialogResult r = actionEditForm.ShowDialog(this);
			if (r == DialogResult.OK) 
			{
				UPnPAction act = actionEditForm.Action;
				upnpService.AddMethod(act);
				updateUserInterface();
			}
		}

		private void newMenuItem_Click(object sender, System.EventArgs e)
		{
			DialogResult r = MessageBox.Show(this,"Are you sure you want to clear the document?","New document",MessageBoxButtons.YesNo,MessageBoxIcon.Question);
			if (r == DialogResult.Yes) 
			{
				string emptyScpd = "<?xml version=\"1.0\" encoding=\"utf-8\"?><scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"><specVersion><major>1</major><minor>0</minor></specVersion><serviceStateTable></serviceStateTable></scpd>";
				upnpService = UPnPService.FromSCPD(emptyScpd);
				this.Text = MainFormTitle;
				openScpdDialog.FileName = "";
				updateUserInterface();
			}
		}

		private void removeVariableMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			if (stateVariableListView.SelectedItems.Count == 1)
			{
				UPnPStateVariable var = (UPnPStateVariable)stateVariableListView.SelectedItems[0].Tag;

				foreach (UPnPAction action in upnpService.GetActions()) 
				{
					foreach (UPnPArgument arg in action.Arguments) 
					{
						if (var.Name == arg.RelatedStateVar.Name) 
						{
							MessageBox.Show(this,"Cannot remove, state variable used in " + action.Name + " action","Remove Argument",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
							return;
						}
					}					
				}

				upnpService.RemoveStateVariable(var);
				updateUserInterface();
			}
		}

		private void stateVariableListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			ListViewItem item = stateVariableListView.GetItemAt(e.X,e.Y);
			bool v = (item != null && (item.Tag.GetType() == typeof(UPnPStateVariable)));
			bool v2 = (item != null && (item.Tag.GetType() == typeof(UPnPComplexType)));
			editStateMenuItem.Visible = v;
			removeStateMenuItem.Visible = v;
			removeComplexTypeMenuItem.Visible = v2;
			stateSeperatorMenuItem.Visible = v||v2;
		}

		private void actionListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			ListViewItem item = actionListView.GetItemAt(e.X,e.Y);
			bool v = (item != null && (item.Tag.GetType() == typeof(UPnPAction)));
			editActionMenuItem.Visible = v;
			removeActionMenuItem2.Visible = v;
			actionSeperatorMenuItem.Visible = v;
		}

		private void removeActionMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;
			if (actionListView.SelectedItems.Count == 1)
			{
				UPnPAction var = (UPnPAction)actionListView.SelectedItems[0].Tag;
				upnpService.RemoveMethod(var);
				updateUserInterface();
			}
		}

		private void helpMenuItem_Click(object sender, System.EventArgs e)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"Service Author");
		}

		private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"Service Author");
		}

		private void openFromNetworkMenuItem_Click(object sender, System.EventArgs e)
		{
			UPnPServiceLocator locator = new UPnPServiceLocator();
			DialogResult r = locator.ShowDialog(this);
			System.Text.UTF8Encoding UTF8 = new System.Text.UTF8Encoding();
			if (r == DialogResult.OK && locator.Service != null)
			{
				upnpService = UPnPService.FromSCPD(UTF8.GetString(locator.Service.GetSCPDXml()));
				openScpdDialog.FileName = locator.Service.ServiceID + ".xml";
				this.Text = MainFormTitle + " - " + locator.Service.ServiceID + ".xml";
				updateUserInterface();
			}
		}

		private void MainForm_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
			{
				string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);
				if (filenames.Length != 1) return;
				FileInfo file = new FileInfo(filenames[0]);
				if (file.Exists == false) return;
				e.Effect = DragDropEffects.Copy;
			}
		}

		private void MainForm_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
			{
				string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);
				if (filenames.Length != 1) return;
				FileInfo file = new FileInfo(filenames[0]);
				if (file.Exists == false) return;
				openFile(file);
			}
		}

		private void addComplexTypeMenuItem_Click(object sender, System.EventArgs e)
		{
			DesignComplexType dct = new DesignComplexType(upnpService.GetComplexTypeList());
			if(dct.ShowDialog() == DialogResult.OK)
			{
				if(dct.ComplexType != null)
				{
					upnpService.AddComplexType(dct.ComplexType);
					this.updateUserInterface();
				}
			}
		}

		private void removeComplexTypeMenuItem_Click(object sender, System.EventArgs e)
		{
			if (upnpService == null) return;

			if (stateVariableListView.SelectedItems.Count == 1)
			{
				UPnPComplexType var = (UPnPComplexType)stateVariableListView.SelectedItems[0].Tag;
				upnpService.RemoveComplexType(var);
				this.updateUserInterface();
			}
		}

		private void menuItem3_Click(object sender, System.EventArgs e)
		{
			foreach(string ns in upnpService.GetSchemaNamespaces())
			{
				string schema = upnpService.GetComplexSchemaForNamespace(ns);
				System.Windows.Forms.SaveFileDialog fd = new System.Windows.Forms.SaveFileDialog();
				fd.Title = ns;
				if(fd.ShowDialog()==System.Windows.Forms.DialogResult.OK)
				{
					System.IO.FileStream fs = (FileStream)fd.OpenFile();
					System.Text.UTF8Encoding U = new System.Text.UTF8Encoding();
					byte[] buffer = U.GetBytes(schema);
					fs.Write(buffer,0,buffer.Length);
					fs.Close();
				}
			}
		}

		private void AddComplexType_Click(object sender, System.EventArgs e)
		{
			UPnPComplexType c;
			TreeNode n = new TreeNode();
			
			ComplexTypeProperty ctp = new ComplexTypeProperty();
			if(ctp.ShowDialog()==DialogResult.OK)
			{
				c = new UPnPComplexType(ctp.LocalName,ctp.Namespace);
				upnpService.AddComplexType(c);
				n.Tag = c;
				n.Text = c.ToString();
				TypesView.Nodes.Add(n);
			}
		}

		private void OnMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if(e.Button==MouseButtons.Right)
			{
				TreeNode n = TypesView.GetNodeAt(e.X,e.Y);
				TypesView.ContextMenu = RootContextMenu;
				Properties.Text = "Properties";
				Root_AddField.Enabled = false;

				if(n!=null)
				{
					TypesView.SelectedNode = n;
					if(n.Tag.GetType()==typeof(UPnPComplexType))
					{
						AddComplexType.Enabled = true;
						RemoveComplexType.Enabled = true;
						Properties.Enabled = true;
						AddSequence_root.Enabled = true;
						AddChoice_root.Enabled = true;
						AddComplexContent.Enabled = true;
						AddSimpleContent.Enabled = true;
						RemoveComplexContent.Enabled = false;
						RemoveSimpleContent.Enabled = false;
						Root_AddField.Enabled = true;
					}
					else if(n.Tag.GetType().BaseType==typeof(UPnPComplexType.ItemCollection))
					{
						TypesView.ContextMenu = this.ComplexTypes_SubContextMenu;
						RemoveSequence.Enabled = false;
						RemoveChoice.Enabled = false;

						if(n.Tag.GetType() == typeof(UPnPComplexType.Sequence))
						{
							RemoveSequence.Enabled = true;
						}
						else if (n.Tag.GetType() == typeof(UPnPComplexType.Choice))
						{
							RemoveChoice.Enabled = true;
						}
					}
					else if(n.Tag.GetType().BaseType == typeof(UPnPComplexType.GenericContainer))
					{
						Properties.Text = "Add Restriction/Extension";
						AddComplexType.Enabled = false;
						RemoveComplexType.Enabled = false;

						AddComplexContent.Enabled = false;
						AddSimpleContent.Enabled = false;
						RemoveComplexContent.Enabled = false;
						RemoveSimpleContent.Enabled = false;
						if(n.Tag.GetType() == typeof(UPnPComplexType.ComplexContent))
						{
							RemoveComplexContent.Enabled = true;
							if(((UPnPComplexType.ComplexContent)n.Tag).RestExt!=null)
							{
								Properties.Text = "Edit Restriction/Extension";
							}
						}
						else if(n.Tag.GetType() == typeof(UPnPComplexType.SimpleContent))
						{
							RemoveSimpleContent.Enabled = true;
							if(((UPnPComplexType.SimpleContent)n.Tag).RestExt!=null)
							{
								Properties.Text = "Edit Restriction/Extension";
							}
						}
					}
					else if (n.Tag.GetType().BaseType == typeof(UPnPComplexType.ContentData))
					{
						TypesView.ContextMenu = this.ContentData_ContextMenu;
					}
				}
				else
				{
					AddComplexType.Enabled = true;
					RemoveComplexType.Enabled = false;
					Properties.Enabled = false;
					AddSequence_root.Enabled = false;
					AddChoice_root.Enabled = false;
					AddComplexContent.Enabled = false;
					AddSimpleContent.Enabled = false;
					RemoveComplexContent.Enabled = false;
					RemoveSimpleContent.Enabled = false;
				}
			}
		}

		private void OnClick_AddSequence(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.Sequence s = new UPnPComplexType.Sequence();
			TreeNode n2 = new TreeNode();
			n2.Text = "Sequence";
			n2.Tag = s;
			n.Nodes.Add(n2);

			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
				UPnPComplexType c = (UPnPComplexType)n.Tag;
				c.AddContainer(new UPnPComplexType.GenericContainer());
				c.CurrentContainer.AddCollection(s);
			}
			else if(n.Tag.GetType().BaseType==typeof(UPnPComplexType.ItemCollection))
			{
				UPnPComplexType.ItemCollection ic = (UPnPComplexType.ItemCollection)n.Tag;
				ic.AddCollection(s);
			}
			else if(n.Tag.GetType().BaseType == typeof(UPnPComplexType.GenericContainer))
			{
				((UPnPComplexType.GenericContainer)n.Tag).AddCollection(s);
			}
		}
		private void OnClick_AddChoice(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.Choice s = new UPnPComplexType.Choice();
			TreeNode n2 = new TreeNode();
			n2.Text = "Choice";
			n2.Tag = s;
			n.Nodes.Add(n2);

			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
				UPnPComplexType c = (UPnPComplexType)n.Tag;
				c.AddContainer(new UPnPComplexType.GenericContainer());
				c.CurrentContainer.AddCollection(s);
			}
			else if(n.Tag.GetType().BaseType==typeof(UPnPComplexType.ItemCollection))
			{
				UPnPComplexType.ItemCollection ic = (UPnPComplexType.ItemCollection)n.Tag;
				ic.AddCollection(s);
			}
			else if(n.Tag.GetType().BaseType == typeof(UPnPComplexType.GenericContainer))
			{
				((UPnPComplexType.GenericContainer)n.Tag).AddCollection(s);
			}
		}

		private void OnAdd_ComplexContent(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.GenericContainer gc = new UPnPComplexType.ComplexContent();
			TreeNode n2 = new TreeNode();
			n2.Text = "Complex Content";
			n2.Tag = gc;
			n.Nodes.Add(n2);

			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
				UPnPComplexType c = (UPnPComplexType)n.Tag;
				c.AddContainer(gc);
			}
		}

		private void OnAdd_SimpleContent(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.GenericContainer gc = new UPnPComplexType.SimpleContent();
			TreeNode n2 = new TreeNode();
			n2.Text = "Simple Content";
			n2.Tag = gc;
			n.Nodes.Add(n2);

			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
				UPnPComplexType c = (UPnPComplexType)n.Tag;
				c.AddContainer(gc);
			}
		}

		private void OnRemove_SimpleComplex_Content(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType ct = (UPnPComplexType)n.Parent.Tag;
			ct.RemoveContainer((UPnPComplexType.GenericContainer)n.Tag);

			n.Tag = null;
			n.Parent.Nodes.Remove(n);
		}

		private void OnRemove_SequenceChoice(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.ItemCollection ic = (UPnPComplexType.ItemCollection)n.Tag;
			
			if(ic.ParentCollection != null)
			{
				ic.ParentCollection.RemoveCollection(ic);
			}
			if(ic.ParentContainer != null)
			{
				if(ic.ParentContainer.Collections.Length==1 && ic.ParentContainer.ParentComplexType.Containers[0] != ic.ParentContainer)
				{
					ic.ParentContainer.ParentComplexType.RemoveContainer(ic.ParentContainer);
				}
				ic.ParentContainer.RemoveCollection(ic);
			}

			n.Tag = null;
			n.Parent.Nodes.Remove(n);
		}

		private void OnAdd_Field(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.ItemCollection ic = null;
			
			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
                if (((UPnPComplexType)n.Tag).Containers.Length == 0) return; // TODO: This sometimes happens and should not
				ic = ((UPnPComplexType)n.Tag).Containers[0].Collections[0];
			}
			else
			{
				ic = (UPnPComplexType.ItemCollection)n.Tag;
			}
			
			FieldForm ff = new FieldForm(upnpService.GetComplexTypeList(), null);
			if(ff.ShowDialog() == DialogResult.OK)
			{
				ic.AddContentItem(ff.NewContentItem);

				TreeNode nn = new TreeNode();
				nn.Text = ff.NewContentItem.ToString();
				nn.Tag = ff.NewContentItem;
				n.Nodes.Add(nn);
			}
		}

		private void RemoveFieldMenuItem_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.ContentData cd = (UPnPComplexType.ContentData)n.Tag;
			UPnPComplexType.ItemCollection ic = cd.Parent;

			n.Tag = null;
			ic.RemoveContentItem(cd);
			n.Parent.Nodes.Remove(n);
		}

		private void EditField_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType.ContentData cd = (UPnPComplexType.ContentData)n.Tag;

			FieldForm ff = new FieldForm(upnpService.GetComplexTypeList(),cd);
			if(ff.ShowDialog()==DialogResult.OK)
			{
				n.Text = ff.NewContentItem.ToString();
				n.Tag = ff.NewContentItem;
			}
		}

		private void MoveUp_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			TreeNode p = n.Parent;
			UPnPComplexType.ItemCollection ic;

			UPnPComplexType.ContentData cd = (UPnPComplexType.ContentData)n.Tag;
			ic = cd.Parent;

			ic.MoveContentItem_UP(cd);
			RefreshFieldNodes(p,ic);
		}

		private void MoveDown_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			TreeNode p = n.Parent;

			UPnPComplexType.ContentData cd = (UPnPComplexType.ContentData)n.Tag;
			UPnPComplexType.ItemCollection ic = cd.Parent;

			ic.MoveContentItem_DOWN(cd);
			RefreshFieldNodes(p,ic);
		}
		private void RefreshFieldNodes(TreeNode ItemCollectionNode, UPnPComplexType.ItemCollection ic)
		{
			Queue q = new Queue();


			foreach(UPnPComplexType.ContentData cd in ic.Items)
			{
				q.Enqueue(cd);
			}

			foreach(TreeNode n in ItemCollectionNode.Nodes)
			{
				if(n.Tag.GetType().BaseType==typeof(UPnPComplexType.ContentData))
				{
					UPnPComplexType.ContentData d = (UPnPComplexType.ContentData)q.Dequeue();
					n.Text = d.ToString();
					n.Tag = d;
				}
			}
		}

		private void RemoveComplexType_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType c = (UPnPComplexType)n.Tag;

			upnpService.RemoveComplexType(c);
			TypesView.Nodes.Remove(n);
		}

		private void Properties_Click(object sender, System.EventArgs e)
		{
			TreeNode n = TypesView.SelectedNode;
			UPnPComplexType c;

			if(n.Tag.GetType()==typeof(UPnPComplexType))
			{
				c = (UPnPComplexType)n.Tag;

				ComplexTypeProperty ctp = new ComplexTypeProperty(c);
				if(ctp.ShowDialog()==DialogResult.OK)
				{
					UPnPComplexType NewComplexType = new UPnPComplexType(ctp.LocalName,ctp.Namespace);
					NewComplexType.ClearCollections();
					foreach(UPnPComplexType.GenericContainer gc in c.Containers)
					{
						NewComplexType.AddContainer(gc);
					}
					upnpService.RemoveComplexType(c);
					upnpService.AddComplexType(NewComplexType);
					n.Tag = NewComplexType;
					n.Text = NewComplexType.ToString();
				}
			}
			else if(n.Tag.GetType().BaseType == typeof(UPnPComplexType.GenericContainer))
			{
				ContainerProperty cp = new ContainerProperty(upnpService.GetComplexTypeList(),(UPnPComplexType.GenericContainer)n.Tag);
				if(cp.ShowDialog()==DialogResult.OK)
				{
					if(n.Tag.GetType()==typeof(UPnPComplexType.ComplexContent))
					{
						((UPnPComplexType.ComplexContent)n.Tag).RestExt = cp.re;
					}
					else if(n.Tag.GetType()==typeof(UPnPComplexType.ComplexContent))
					{
						((UPnPComplexType.SimpleContent)n.Tag).RestExt = cp.re;
					}
				}
			}
		}
		private void SequenceChoice_Property_Click(object sender, System.EventArgs e)
		{
		}
		public void AddComplexTypeToDisplayTree(UPnPComplexType ct, TreeView T)
		{
			TreeNode n = new TreeNode();
			n.Text = ct.ToString();
			n.Tag = ct;
			foreach(UPnPComplexType.GenericContainer gc in ct.Containers)
			{
				AddComplexTypeToDisplayTree_ProcessContainer(gc,n);
			}
			T.Nodes.Add(n);
		}
		private void AddComplexTypeToDisplayTree_ProcessContainer(UPnPComplexType.GenericContainer gc, TreeNode n)
		{
			if(gc.GetType()!=typeof(UPnPComplexType.GenericContainer))
			{
				TreeNode child = new TreeNode();
				child.Text = gc.GetType().Name;
				child.Tag = gc;
				n.Nodes.Add(child);
				foreach(UPnPComplexType.ItemCollection ic in gc.Collections)
				{
					AddComplexTypeToDisplayTree_ProcessCollection(ic,child);
				}
			}
			else
			{
				foreach(UPnPComplexType.ItemCollection ic in gc.Collections)
				{
					AddComplexTypeToDisplayTree_ProcessCollection(ic,n);
				}
			}
		}

		private void AddComplexTypeToDisplayTree_ProcessCollection(UPnPComplexType.ItemCollection ic, TreeNode n)
		{
			TreeNode UseNode = n;
			if(ic.GetType()!=typeof(UPnPComplexType.ItemCollection))
			{
				TreeNode child = new TreeNode();
				child.Text = ic.GetType().Name;
				child.Tag = ic;
				UseNode = child;
				n.Nodes.Add(child);
			}

			foreach(UPnPComplexType.ContentData cd in ic.Items)
			{
				TreeNode subNode = new TreeNode();
				subNode.Text = cd.ToString();
				subNode.Tag = cd;
				UseNode.Nodes.Add(subNode);
			}
			foreach(UPnPComplexType.ItemCollection ecd in ic.NestedCollections)
			{
				AddComplexTypeToDisplayTree_ProcessCollection(ecd,UseNode);
			}
		}

		private void OpenXSDItem_Click(object sender, System.EventArgs e)
		{
			// Prompt Application for local Schema Location
			System.Windows.Forms.OpenFileDialog fd = new System.Windows.Forms.OpenFileDialog();
			fd.Multiselect = false;
			fd.Title = "Load an XSD File";
			if(fd.ShowDialog()==System.Windows.Forms.DialogResult.OK)
			{
				FileStream fs = (FileStream)fd.OpenFile();
				System.Text.UTF8Encoding U = new System.Text.UTF8Encoding();
				byte[] buffer = new byte[(int)fs.Length];
				fs.Read(buffer,0,buffer.Length);
				UPnPComplexType[] complexTypes = UPnPComplexType.Parse(U.GetString(buffer));
				fs.Close();
				foreach(UPnPComplexType complexType in complexTypes)
				{
					upnpService.AddComplexType(complexType);
				}
				this.updateUserInterface();
			}
			
		}
	}
}
