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
using System.Web.Mail;
using System.Threading;
using System.Reflection;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using OpenSource.UPnP;

namespace UPnPValidator
{
    /// <summary>
    /// The main validator form.  This class handles all the layout and 
    /// much of the event handling for Validator.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {
        private const string FILE_NAME = "DHDeviceTestResult.txt";
        private string SMTPServer = "mail.sample.com";
        private TreeNode UPnpTestRoot = new TreeNode("UPnP Tests", 0, 0);
        private delegate void UpdateDelegate(string x);
        private UPnPDevice targetdevice;
        private Thread thread = null;
        private IUPnPTestGroup displaytestinfo;

        // this variable never gets set, so I commented it out -Tom Anderl
        //private IUPnPTestGroup runningtest; 

        private ArrayList pendingtestlist = new ArrayList();
        private ArrayList testgroups = new ArrayList();
        private UPnPDeviceLocator devicelocator = new UPnPDeviceLocator();
        private bool RunAllTests = false;

        private int testcount_total = 0;
        private int testcount_ready = 0;
        private int testcount_failed = 0;
        private int testcount_warn = 0;
        private int testcount_pass = 0;
        private int testcount_skip = 0;

        #region GUI member variables thanks to the form designer
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem SelectDeviceMenuItem;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MenuItem exitMenuItem;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.ImageList treeImageList;
        private System.Windows.Forms.TreeView testTree;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label statusLabel1;
        private System.Windows.Forms.Label statusLabel2;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.ContextMenu testContextMenu;
        private System.Windows.Forms.MenuItem menuItem7;
        private System.Windows.Forms.TabControl mainTabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox descriptionTextBox;
        private System.Windows.Forms.TextBox resultTextBox;
        private System.Windows.Forms.Panel panel5;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label testHeaderLabel;
        private System.Windows.Forms.MenuItem executeMenuItem;
        private System.Windows.Forms.MenuItem skipMenuItem;
        private System.Windows.Forms.Label progressLabel;
        private System.Windows.Forms.Label progressStepLabel;
        private System.Windows.Forms.Panel progressPanel;
        private System.Windows.Forms.ProgressBar progressStepBar;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label countPassLabel;
        private System.Windows.Forms.Label countFailLabel;
        private System.Windows.Forms.Label countWarnLabel;
        private System.Windows.Forms.Label countSkipLabel;
        private System.Windows.Forms.OpenFileDialog openResultFileDialog;
        private System.Windows.Forms.SaveFileDialog saveResultFileDialog;
        private System.Windows.Forms.OpenFileDialog openModuleFileDialog;
        private System.Windows.Forms.MenuItem loadTestModuleMenuItem;
        private System.Windows.Forms.MenuItem saveResultsMenuItem;
        private System.Windows.Forms.ListView packetListView;
        private System.Windows.Forms.MenuItem resetMenuItem;
        private System.Windows.Forms.Button setTargetButton;
        private System.Windows.Forms.Button startTestsButton;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem haltTestsMenuItem;
        private System.Windows.Forms.MenuItem startTestMenuItem;
        private System.Windows.Forms.MenuItem showDebugInfoMenuItem;
        private System.Windows.Forms.MenuItem resetAllTestsMenuItem;
        private System.Windows.Forms.StatusBar mainStatusBar;
        private System.Windows.Forms.ToolTip mainToolTip;
        private System.Windows.Forms.Button resetTestGroupButton;
        private System.Windows.Forms.Button execTestGroupButton;
        private System.Windows.Forms.MenuItem helpMenuItem;
        private System.Windows.Forms.MenuItem menuItem6;
        private System.Windows.Forms.ContextMenu logContextMenu;
        private System.Windows.Forms.MenuItem copylogMenuItem;
        private System.Windows.Forms.MenuItem loadResultsMenuItem;
        private System.Windows.Forms.Button returnToTestingButton;
        private System.Windows.Forms.Label logLabel;
        private System.Windows.Forms.TextBox logDetailTextBox;
        private System.Windows.Forms.TextBox packetTextBox;
        private System.Windows.Forms.ListView logListBox;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.Splitter splitter3;
        private System.Windows.Forms.Splitter logSplitter;
        private System.ComponentModel.IContainer components;
        #endregion

        /* these three variables are used when going into the view only mode
		 * so that the state outside the view only mode can be come back to
		 * after exiting view only.
		 * */
        private bool _inViewOnlyMode = false;
        private TreeNode _prevRoot = null;
        private string _prevDeviceString = "";

        public MainForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            testTree.Nodes.Add(UPnpTestRoot);
            UPnpTestRoot.Expand();

            AddTestsInAssembly(Assembly.GetEntryAssembly());
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

        private void MainForm_Load(object sender, System.EventArgs e)
        {

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
            this.loadResultsMenuItem = new System.Windows.Forms.MenuItem();
            this.saveResultsMenuItem = new System.Windows.Forms.MenuItem();
            this.loadTestModuleMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.SelectDeviceMenuItem = new System.Windows.Forms.MenuItem();
            this.startTestMenuItem = new System.Windows.Forms.MenuItem();
            this.haltTestsMenuItem = new System.Windows.Forms.MenuItem();
            this.resetAllTestsMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.showDebugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.mainStatusBar = new System.Windows.Forms.StatusBar();
            this.testTree = new System.Windows.Forms.TreeView();
            this.testContextMenu = new System.Windows.Forms.ContextMenu();
            this.executeMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.resetMenuItem = new System.Windows.Forms.MenuItem();
            this.skipMenuItem = new System.Windows.Forms.MenuItem();
            this.treeImageList = new System.Windows.Forms.ImageList(this.components);
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.panel1 = new System.Windows.Forms.Panel();
            this.returnToTestingButton = new System.Windows.Forms.Button();
            this.setTargetButton = new System.Windows.Forms.Button();
            this.startTestsButton = new System.Windows.Forms.Button();
            this.statusLabel2 = new System.Windows.Forms.Label();
            this.statusLabel1 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.mainTabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.panel3 = new System.Windows.Forms.Panel();
            this.execTestGroupButton = new System.Windows.Forms.Button();
            this.resetTestGroupButton = new System.Windows.Forms.Button();
            this.resultTextBox = new System.Windows.Forms.TextBox();
            this.descriptionTextBox = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.testHeaderLabel = new System.Windows.Forms.Label();
            this.panel2 = new System.Windows.Forms.Panel();
            this.countSkipLabel = new System.Windows.Forms.Label();
            this.countPassLabel = new System.Windows.Forms.Label();
            this.countFailLabel = new System.Windows.Forms.Label();
            this.countWarnLabel = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.logSplitter = new System.Windows.Forms.Splitter();
            this.logListBox = new System.Windows.Forms.ListView();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.logContextMenu = new System.Windows.Forms.ContextMenu();
            this.copylogMenuItem = new System.Windows.Forms.MenuItem();
            this.logDetailTextBox = new System.Windows.Forms.TextBox();
            this.logLabel = new System.Windows.Forms.Label();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.splitter3 = new System.Windows.Forms.Splitter();
            this.packetTextBox = new System.Windows.Forms.TextBox();
            this.packetListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.panel5 = new System.Windows.Forms.Panel();
            this.label14 = new System.Windows.Forms.Label();
            this.progressPanel = new System.Windows.Forms.Panel();
            this.progressLabel = new System.Windows.Forms.Label();
            this.progressStepLabel = new System.Windows.Forms.Label();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.progressStepBar = new System.Windows.Forms.ProgressBar();
            this.openResultFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.saveResultFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.openModuleFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.mainToolTip = new System.Windows.Forms.ToolTip(this.components);
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.mainTabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.panel3.SuspendLayout();
            this.panel2.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.panel5.SuspendLayout();
            this.progressPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem2,
            this.menuItem4});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.loadResultsMenuItem,
            this.saveResultsMenuItem,
            this.loadTestModuleMenuItem,
            this.menuItem3,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            this.menuItem1.Popup += new System.EventHandler(this.menuItem1_Popup);
            // 
            // loadResultsMenuItem
            // 
            this.loadResultsMenuItem.Index = 0;
            this.loadResultsMenuItem.Text = "&Load Results";
            this.loadResultsMenuItem.Click += new System.EventHandler(this.loadResultsMenuItem_Click);
            // 
            // saveResultsMenuItem
            // 
            this.saveResultsMenuItem.Index = 1;
            this.saveResultsMenuItem.Text = "&Save  Results...";
            this.saveResultsMenuItem.Click += new System.EventHandler(this.saveResultsMenuItem_Click);
            // 
            // loadTestModuleMenuItem
            // 
            this.loadTestModuleMenuItem.Index = 2;
            this.loadTestModuleMenuItem.Text = "Load Test &Module...";
            this.loadTestModuleMenuItem.Click += new System.EventHandler(this.loadTestModuleMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 3;
            this.menuItem3.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 4;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 1;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.SelectDeviceMenuItem,
            this.startTestMenuItem,
            this.haltTestsMenuItem,
            this.resetAllTestsMenuItem});
            this.menuItem2.Text = "&Testing";
            // 
            // SelectDeviceMenuItem
            // 
            this.SelectDeviceMenuItem.Index = 0;
            this.SelectDeviceMenuItem.Text = "&Select Target";
            this.SelectDeviceMenuItem.Click += new System.EventHandler(this.SelectDeviceMenuItem_Click);
            // 
            // startTestMenuItem
            // 
            this.startTestMenuItem.Enabled = false;
            this.startTestMenuItem.Index = 1;
            this.startTestMenuItem.Text = "&Execute Tests";
            this.startTestMenuItem.Click += new System.EventHandler(this.startTestsButton_Click);
            // 
            // haltTestsMenuItem
            // 
            this.haltTestsMenuItem.Enabled = false;
            this.haltTestsMenuItem.Index = 2;
            this.haltTestsMenuItem.Text = "&Halt Testing";
            this.haltTestsMenuItem.Click += new System.EventHandler(this.startTestsButton_Click);
            // 
            // resetAllTestsMenuItem
            // 
            this.resetAllTestsMenuItem.Index = 3;
            this.resetAllTestsMenuItem.Text = "Reset All Tests";
            this.resetAllTestsMenuItem.Click += new System.EventHandler(this.resetAllTestsMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem6,
            this.showDebugInfoMenuItem});
            this.menuItem4.Text = "&Help";
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            this.helpMenuItem.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem.Text = "&Help Topics";
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 1;
            this.menuItem6.Text = "-";
            // 
            // showDebugInfoMenuItem
            // 
            this.showDebugInfoMenuItem.Index = 2;
            this.showDebugInfoMenuItem.Text = "&Show Debug Information";
            this.showDebugInfoMenuItem.Click += new System.EventHandler(this.showDebugInfoMenuItem_Click);
            // 
            // mainStatusBar
            // 
            this.mainStatusBar.Location = new System.Drawing.Point(0, 465);
            this.mainStatusBar.Name = "mainStatusBar";
            this.mainStatusBar.Size = new System.Drawing.Size(800, 16);
            this.mainStatusBar.TabIndex = 1;
            // 
            // testTree
            // 
            this.testTree.ContextMenu = this.testContextMenu;
            this.testTree.Dock = System.Windows.Forms.DockStyle.Left;
            this.testTree.ImageIndex = 0;
            this.testTree.ImageList = this.treeImageList;
            this.testTree.Location = new System.Drawing.Point(0, 40);
            this.testTree.Name = "testTree";
            this.testTree.SelectedImageIndex = 0;
            this.testTree.Size = new System.Drawing.Size(272, 425);
            this.testTree.TabIndex = 2;
            this.testTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.testTree_AfterSelect);
            this.testTree.MouseDown += new System.Windows.Forms.MouseEventHandler(this.testTree_MouseDown);
            // 
            // testContextMenu
            // 
            this.testContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.executeMenuItem,
            this.menuItem7,
            this.resetMenuItem,
            this.skipMenuItem});
            this.testContextMenu.Popup += new System.EventHandler(this.testContextMenu_Popup);
            // 
            // executeMenuItem
            // 
            this.executeMenuItem.DefaultItem = true;
            this.executeMenuItem.Index = 0;
            this.executeMenuItem.Text = "Execute Group";
            this.executeMenuItem.Click += new System.EventHandler(this.executeMenuItem_Click);
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 1;
            this.menuItem7.Text = "-";
            // 
            // resetMenuItem
            // 
            this.resetMenuItem.Index = 2;
            this.resetMenuItem.Text = "Reset Group";
            this.resetMenuItem.Click += new System.EventHandler(this.resetMenuItem_Click);
            // 
            // skipMenuItem
            // 
            this.skipMenuItem.Index = 3;
            this.skipMenuItem.Text = "Skip Group";
            this.skipMenuItem.Click += new System.EventHandler(this.skipMenuItem_Click);
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
            this.treeImageList.Images.SetKeyName(9, "");
            this.treeImageList.Images.SetKeyName(10, "");
            this.treeImageList.Images.SetKeyName(11, "");
            this.treeImageList.Images.SetKeyName(12, "");
            this.treeImageList.Images.SetKeyName(13, "");
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(272, 40);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 425);
            this.splitter1.TabIndex = 12;
            this.splitter1.TabStop = false;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.returnToTestingButton);
            this.panel1.Controls.Add(this.setTargetButton);
            this.panel1.Controls.Add(this.startTestsButton);
            this.panel1.Controls.Add(this.statusLabel2);
            this.panel1.Controls.Add(this.statusLabel1);
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(800, 40);
            this.panel1.TabIndex = 15;
            // 
            // returnToTestingButton
            // 
            this.returnToTestingButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.returnToTestingButton.Location = new System.Drawing.Point(514, 0);
            this.returnToTestingButton.Name = "returnToTestingButton";
            this.returnToTestingButton.Size = new System.Drawing.Size(142, 40);
            this.returnToTestingButton.TabIndex = 5;
            this.returnToTestingButton.TabStop = false;
            this.returnToTestingButton.Text = "Return to Testing";
            this.mainToolTip.SetToolTip(this.returnToTestingButton, "Close the Viewer and return to testing");
            this.returnToTestingButton.Visible = false;
            this.returnToTestingButton.Click += new System.EventHandler(this.returnToTestingButton_Click);
            // 
            // setTargetButton
            // 
            this.setTargetButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.setTargetButton.Location = new System.Drawing.Point(656, 0);
            this.setTargetButton.Name = "setTargetButton";
            this.setTargetButton.Size = new System.Drawing.Size(72, 40);
            this.setTargetButton.TabIndex = 3;
            this.setTargetButton.Text = "Select Target";
            this.mainToolTip.SetToolTip(this.setTargetButton, "Select a new UPnP device as validaton test target.");
            this.setTargetButton.Click += new System.EventHandler(this.SelectDeviceMenuItem_Click);
            // 
            // startTestsButton
            // 
            this.startTestsButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.startTestsButton.Enabled = false;
            this.startTestsButton.Location = new System.Drawing.Point(728, 0);
            this.startTestsButton.Name = "startTestsButton";
            this.startTestsButton.Size = new System.Drawing.Size(72, 40);
            this.startTestsButton.TabIndex = 4;
            this.startTestsButton.Text = "Execute Tests";
            this.mainToolTip.SetToolTip(this.startTestsButton, "Start and halt execution of all test groups.");
            this.startTestsButton.Click += new System.EventHandler(this.startTestsButton_Click);
            // 
            // statusLabel2
            // 
            this.statusLabel2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.statusLabel2.Location = new System.Drawing.Point(48, 21);
            this.statusLabel2.Name = "statusLabel2";
            this.statusLabel2.Size = new System.Drawing.Size(600, 16);
            this.statusLabel2.TabIndex = 2;
            // 
            // statusLabel1
            // 
            this.statusLabel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.statusLabel1.Location = new System.Drawing.Point(48, 5);
            this.statusLabel1.Name = "statusLabel1";
            this.statusLabel1.Size = new System.Drawing.Size(600, 16);
            this.statusLabel1.TabIndex = 1;
            this.statusLabel1.Text = "Validation target device is not set";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(7, 5);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(32, 32);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // mainTabControl
            // 
            this.mainTabControl.Alignment = System.Windows.Forms.TabAlignment.Bottom;
            this.mainTabControl.Controls.Add(this.tabPage1);
            this.mainTabControl.Controls.Add(this.tabPage2);
            this.mainTabControl.Controls.Add(this.tabPage3);
            this.mainTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainTabControl.Location = new System.Drawing.Point(275, 80);
            this.mainTabControl.Name = "mainTabControl";
            this.mainTabControl.SelectedIndex = 0;
            this.mainTabControl.Size = new System.Drawing.Size(525, 385);
            this.mainTabControl.TabIndex = 16;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.panel3);
            this.tabPage1.Controls.Add(this.testHeaderLabel);
            this.tabPage1.Controls.Add(this.panel2);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Location = new System.Drawing.Point(4, 4);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(517, 359);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Summary";
            // 
            // panel3
            // 
            this.panel3.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel3.Controls.Add(this.execTestGroupButton);
            this.panel3.Controls.Add(this.resetTestGroupButton);
            this.panel3.Controls.Add(this.resultTextBox);
            this.panel3.Controls.Add(this.descriptionTextBox);
            this.panel3.Controls.Add(this.label11);
            this.panel3.Controls.Add(this.label12);
            this.panel3.Location = new System.Drawing.Point(8, 120);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(504, 232);
            this.panel3.TabIndex = 11;
            // 
            // execTestGroupButton
            // 
            this.execTestGroupButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.execTestGroupButton.Enabled = false;
            this.execTestGroupButton.Location = new System.Drawing.Point(424, 24);
            this.execTestGroupButton.Name = "execTestGroupButton";
            this.execTestGroupButton.Size = new System.Drawing.Size(72, 32);
            this.execTestGroupButton.TabIndex = 15;
            this.execTestGroupButton.Text = "Execute";
            this.mainToolTip.SetToolTip(this.execTestGroupButton, "Execute all tests in this test group");
            this.execTestGroupButton.Click += new System.EventHandler(this.executeMenuItem_Click);
            // 
            // resetTestGroupButton
            // 
            this.resetTestGroupButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.resetTestGroupButton.Enabled = false;
            this.resetTestGroupButton.Location = new System.Drawing.Point(424, 56);
            this.resetTestGroupButton.Name = "resetTestGroupButton";
            this.resetTestGroupButton.Size = new System.Drawing.Size(72, 32);
            this.resetTestGroupButton.TabIndex = 14;
            this.resetTestGroupButton.Text = "Reset";
            this.mainToolTip.SetToolTip(this.resetTestGroupButton, "Reset all tests in the test group");
            this.resetTestGroupButton.Click += new System.EventHandler(this.resetMenuItem_Click);
            // 
            // resultTextBox
            // 
            this.resultTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.resultTextBox.Location = new System.Drawing.Point(8, 112);
            this.resultTextBox.Multiline = true;
            this.resultTextBox.Name = "resultTextBox";
            this.resultTextBox.ReadOnly = true;
            this.resultTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.resultTextBox.Size = new System.Drawing.Size(488, 112);
            this.resultTextBox.TabIndex = 13;
            this.mainToolTip.SetToolTip(this.resultTextBox, "Results for the selected test group.");
            // 
            // descriptionTextBox
            // 
            this.descriptionTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.descriptionTextBox.Location = new System.Drawing.Point(8, 24);
            this.descriptionTextBox.Multiline = true;
            this.descriptionTextBox.Name = "descriptionTextBox";
            this.descriptionTextBox.ReadOnly = true;
            this.descriptionTextBox.Size = new System.Drawing.Size(408, 64);
            this.descriptionTextBox.TabIndex = 12;
            this.mainToolTip.SetToolTip(this.descriptionTextBox, "Description of the selected test group.");
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(8, 8);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(384, 16);
            this.label11.TabIndex = 11;
            this.label11.Text = "Description";
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(8, 96);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(384, 16);
            this.label12.TabIndex = 10;
            this.label12.Text = "Result";
            // 
            // testHeaderLabel
            // 
            this.testHeaderLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.testHeaderLabel.Location = new System.Drawing.Point(0, 104);
            this.testHeaderLabel.Name = "testHeaderLabel";
            this.testHeaderLabel.Size = new System.Drawing.Size(416, 16);
            this.testHeaderLabel.TabIndex = 10;
            this.testHeaderLabel.Text = "Test Summary";
            // 
            // panel2
            // 
            this.panel2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel2.Controls.Add(this.countSkipLabel);
            this.panel2.Controls.Add(this.countPassLabel);
            this.panel2.Controls.Add(this.countFailLabel);
            this.panel2.Controls.Add(this.countWarnLabel);
            this.panel2.Controls.Add(this.label3);
            this.panel2.Controls.Add(this.label2);
            this.panel2.Controls.Add(this.label4);
            this.panel2.Controls.Add(this.label5);
            this.panel2.Location = new System.Drawing.Point(8, 16);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(504, 80);
            this.panel2.TabIndex = 9;
            // 
            // countSkipLabel
            // 
            this.countSkipLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.countSkipLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.countSkipLabel.Location = new System.Drawing.Point(448, 56);
            this.countSkipLabel.Name = "countSkipLabel";
            this.countSkipLabel.Size = new System.Drawing.Size(38, 16);
            this.countSkipLabel.TabIndex = 17;
            this.countSkipLabel.Text = "0";
            this.countSkipLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // countPassLabel
            // 
            this.countPassLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.countPassLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.countPassLabel.Location = new System.Drawing.Point(448, 8);
            this.countPassLabel.Name = "countPassLabel";
            this.countPassLabel.Size = new System.Drawing.Size(38, 16);
            this.countPassLabel.TabIndex = 14;
            this.countPassLabel.Text = "0";
            this.countPassLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // countFailLabel
            // 
            this.countFailLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.countFailLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.countFailLabel.Location = new System.Drawing.Point(448, 40);
            this.countFailLabel.Name = "countFailLabel";
            this.countFailLabel.Size = new System.Drawing.Size(38, 16);
            this.countFailLabel.TabIndex = 16;
            this.countFailLabel.Text = "0";
            this.countFailLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // countWarnLabel
            // 
            this.countWarnLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.countWarnLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.countWarnLabel.Location = new System.Drawing.Point(448, 24);
            this.countWarnLabel.Name = "countWarnLabel";
            this.countWarnLabel.Size = new System.Drawing.Size(38, 16);
            this.countWarnLabel.TabIndex = 15;
            this.countWarnLabel.Text = "0";
            this.countWarnLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 24);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(160, 16);
            this.label3.TabIndex = 11;
            this.label3.Text = "Passed/Warn";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(8, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(160, 16);
            this.label2.TabIndex = 10;
            this.label2.Text = "Passed";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(8, 40);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(160, 16);
            this.label4.TabIndex = 12;
            this.label4.Text = "Failed";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 56);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(160, 16);
            this.label5.TabIndex = 13;
            this.label5.Text = "Skipped";
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(416, 16);
            this.label1.TabIndex = 0;
            this.label1.Text = "Overall Summary";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.logSplitter);
            this.tabPage2.Controls.Add(this.logListBox);
            this.tabPage2.Controls.Add(this.logDetailTextBox);
            this.tabPage2.Controls.Add(this.logLabel);
            this.tabPage2.Location = new System.Drawing.Point(4, 4);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Size = new System.Drawing.Size(517, 359);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Log";
            // 
            // logSplitter
            // 
            this.logSplitter.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.logSplitter.Location = new System.Drawing.Point(0, 244);
            this.logSplitter.Name = "logSplitter";
            this.logSplitter.Size = new System.Drawing.Size(517, 3);
            this.logSplitter.TabIndex = 9;
            this.logSplitter.TabStop = false;
            // 
            // logListBox
            // 
            this.logListBox.Alignment = System.Windows.Forms.ListViewAlignment.Left;
            this.logListBox.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3});
            this.logListBox.ContextMenu = this.logContextMenu;
            this.logListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.logListBox.FullRowSelect = true;
            this.logListBox.Location = new System.Drawing.Point(0, 16);
            this.logListBox.Name = "logListBox";
            this.logListBox.Size = new System.Drawing.Size(517, 231);
            this.logListBox.SmallImageList = this.treeImageList;
            this.logListBox.TabIndex = 8;
            this.mainToolTip.SetToolTip(this.logListBox, "Complete list of events generated by the execution of the selected test group.");
            this.logListBox.UseCompatibleStateImageBehavior = false;
            this.logListBox.View = System.Windows.Forms.View.Details;
            this.logListBox.SelectedIndexChanged += new System.EventHandler(this.logListBox_SelectedIndexChanged);
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Event";
            this.columnHeader3.Width = 507;
            // 
            // logContextMenu
            // 
            this.logContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.copylogMenuItem});
            // 
            // copylogMenuItem
            // 
            this.copylogMenuItem.Index = 0;
            this.copylogMenuItem.Text = "Copy";
            this.copylogMenuItem.Click += new System.EventHandler(this.copylogMenuItem_Click);
            // 
            // logDetailTextBox
            // 
            this.logDetailTextBox.AcceptsReturn = true;
            this.logDetailTextBox.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.logDetailTextBox.HideSelection = false;
            this.logDetailTextBox.Location = new System.Drawing.Point(0, 247);
            this.logDetailTextBox.Multiline = true;
            this.logDetailTextBox.Name = "logDetailTextBox";
            this.logDetailTextBox.ReadOnly = true;
            this.logDetailTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.logDetailTextBox.Size = new System.Drawing.Size(517, 112);
            this.logDetailTextBox.TabIndex = 7;
            this.mainToolTip.SetToolTip(this.logDetailTextBox, "View of the selected log entry");
            // 
            // logLabel
            // 
            this.logLabel.Dock = System.Windows.Forms.DockStyle.Top;
            this.logLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.logLabel.Location = new System.Drawing.Point(0, 0);
            this.logLabel.Name = "logLabel";
            this.logLabel.Size = new System.Drawing.Size(517, 16);
            this.logLabel.TabIndex = 0;
            this.logLabel.Text = "Event Trace";
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.splitter3);
            this.tabPage3.Controls.Add(this.packetTextBox);
            this.tabPage3.Controls.Add(this.packetListView);
            this.tabPage3.Controls.Add(this.panel5);
            this.tabPage3.Location = new System.Drawing.Point(4, 4);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(517, 359);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Packets";
            // 
            // splitter3
            // 
            this.splitter3.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.splitter3.Location = new System.Drawing.Point(0, 260);
            this.splitter3.Name = "splitter3";
            this.splitter3.Size = new System.Drawing.Size(517, 3);
            this.splitter3.TabIndex = 6;
            this.splitter3.TabStop = false;
            // 
            // packetTextBox
            // 
            this.packetTextBox.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.packetTextBox.Location = new System.Drawing.Point(0, 263);
            this.packetTextBox.Multiline = true;
            this.packetTextBox.Name = "packetTextBox";
            this.packetTextBox.ReadOnly = true;
            this.packetTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.packetTextBox.Size = new System.Drawing.Size(517, 96);
            this.packetTextBox.TabIndex = 5;
            this.mainToolTip.SetToolTip(this.packetTextBox, "View of the selected test packet.");
            // 
            // packetListView
            // 
            this.packetListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.packetListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.packetListView.FullRowSelect = true;
            this.packetListView.Location = new System.Drawing.Point(0, 16);
            this.packetListView.MultiSelect = false;
            this.packetListView.Name = "packetListView";
            this.packetListView.Size = new System.Drawing.Size(517, 343);
            this.packetListView.SmallImageList = this.treeImageList;
            this.packetListView.TabIndex = 3;
            this.mainToolTip.SetToolTip(this.packetListView, "List of all inbound and outbound packets generated by the selected test group.");
            this.packetListView.UseCompatibleStateImageBehavior = false;
            this.packetListView.View = System.Windows.Forms.View.Details;
            this.packetListView.SelectedIndexChanged += new System.EventHandler(this.packetListView_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Size";
            this.columnHeader1.Width = 80;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Information";
            this.columnHeader2.Width = 295;
            // 
            // panel5
            // 
            this.panel5.Controls.Add(this.label14);
            this.panel5.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel5.Location = new System.Drawing.Point(0, 0);
            this.panel5.Name = "panel5";
            this.panel5.Size = new System.Drawing.Size(517, 16);
            this.panel5.TabIndex = 2;
            // 
            // label14
            // 
            this.label14.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label14.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label14.Location = new System.Drawing.Point(0, 0);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(424, 16);
            this.label14.TabIndex = 0;
            this.label14.Text = "Packet Trace";
            // 
            // progressPanel
            // 
            this.progressPanel.Controls.Add(this.progressLabel);
            this.progressPanel.Controls.Add(this.progressStepLabel);
            this.progressPanel.Controls.Add(this.progressBar);
            this.progressPanel.Controls.Add(this.progressStepBar);
            this.progressPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.progressPanel.Location = new System.Drawing.Point(275, 40);
            this.progressPanel.Name = "progressPanel";
            this.progressPanel.Size = new System.Drawing.Size(525, 40);
            this.progressPanel.TabIndex = 17;
            this.progressPanel.Visible = false;
            // 
            // progressLabel
            // 
            this.progressLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.progressLabel.Location = new System.Drawing.Point(456, 20);
            this.progressLabel.Name = "progressLabel";
            this.progressLabel.Size = new System.Drawing.Size(56, 16);
            this.progressLabel.TabIndex = 3;
            this.progressLabel.Text = "0%";
            this.progressLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // progressStepLabel
            // 
            this.progressStepLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.progressStepLabel.Location = new System.Drawing.Point(456, 4);
            this.progressStepLabel.Name = "progressStepLabel";
            this.progressStepLabel.Size = new System.Drawing.Size(56, 16);
            this.progressStepLabel.TabIndex = 2;
            this.progressStepLabel.Text = "1 of 10";
            this.progressStepLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // progressBar
            // 
            this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBar.Location = new System.Drawing.Point(8, 23);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(448, 12);
            this.progressBar.TabIndex = 1;
            this.mainToolTip.SetToolTip(this.progressBar, "Test Progress Indicator");
            // 
            // progressStepBar
            // 
            this.progressStepBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.progressStepBar.Location = new System.Drawing.Point(8, 6);
            this.progressStepBar.Name = "progressStepBar";
            this.progressStepBar.Size = new System.Drawing.Size(448, 12);
            this.progressStepBar.TabIndex = 0;
            this.mainToolTip.SetToolTip(this.progressStepBar, "Test Group Progress Indicator");
            // 
            // openResultFileDialog
            // 
            this.openResultFileDialog.DefaultExt = "upr";
            this.openResultFileDialog.Filter = "UPnP Test Results (*.upr)|*.upr";
            this.openResultFileDialog.Title = "Open UPnP Test Results";
            // 
            // saveResultFileDialog
            // 
            this.saveResultFileDialog.DefaultExt = "upr";
            this.saveResultFileDialog.FileName = "results.upr";
            this.saveResultFileDialog.Filter = "UPnP Test Results (*.upr)|*.upr";
            this.saveResultFileDialog.Title = "Save UPnP Test Results";
            // 
            // openModuleFileDialog
            // 
            this.openModuleFileDialog.DefaultExt = "dll|dsprj";
            this.openModuleFileDialog.Filter = "UPnP Test Modules (*.dll)|*.dll|Device Scriptor Script|*.dsprj";
            this.openModuleFileDialog.Title = "Open UPnP Test Module";
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(800, 481);
            this.Controls.Add(this.mainTabControl);
            this.Controls.Add(this.progressPanel);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.testTree);
            this.Controls.Add(this.mainStatusBar);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "Device Validator";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.Closed += new System.EventHandler(this.MainForm_Closed);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.mainTabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.panel3.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.panel5.ResumeLayout(false);
            this.progressPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private void SelectDeviceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (targetdevice != null)
            {
                if (DialogResult.No ==

                    MessageBox.Show(this,
                    "Reset all tests and select new test target?",
                    "Target Selection",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question))
                {
                    return;
                }
            }

            if (devicelocator.ShowDialog() == DialogResult.OK)
            {
                // Reset All
                foreach (IUPnPTestGroup t in testgroups)
                {
                    t.Reset();
                    OnStateChangedSink(t, null);
                }

                targetdevice = devicelocator.Device;
                if (targetdevice != null)
                {
                    startTestMenuItem.Enabled = true;
                    statusLabel1.Text = "Validation target is: " + targetdevice.FriendlyName + " / " + targetdevice.UniqueDeviceName;
                    startTestsButton.Enabled = true;
                }
                else
                {
                    startTestMenuItem.Enabled = false;
                    statusLabel1.Text = "Validation target device is not set.";
                    startTestsButton.Enabled = false;
                }

                SetTestInfo(displaytestinfo);
            }
        }

        public void AddTestsInAssembly(Assembly assembly)
        {
            foreach (Type type in assembly.GetTypes())
            {
                if (type != typeof(UPnPValidator.BasicTests.BasicTestGroup))
                {
                    foreach (Type i in type.GetInterfaces())
                    {
                        if (i.FullName == "UPnPValidator.IUPnPTestGroup")
                        {
                            IUPnPTestGroup test = null;
                            try
                            {
                                test = (IUPnPTestGroup)OpenSource.UPnP.UPnPService.CreateObjectInstance(type, null);
                            }
                            catch (Exception)
                            {
                                test = null;
                            }
                            if (test != null)
                            {
                                testgroups.Add(test);
                                TreeNode cat = NodeTagSearch(UPnpTestRoot, test.Category);
                                if (cat == null)
                                {
                                    cat = new TreeNode(test.Category, 6, 7);
                                    cat.Tag = test.Category;
                                    UPnpTestRoot.Nodes.Add(cat);
                                    cat.Expand();
                                }
                                test.Enabled = true;
                                TreeNode node = new TreeNode(test.GroupName, 8, 8);
                                node.Tag = test;
                                if (test.TestNames.Length > 0)
                                {
                                    foreach (string TestName in test.TestNames)
                                    {
                                        TreeNode snode = new TreeNode(TestName, 8, 8);
                                        node.Nodes.Add(snode);
                                    }
                                    node.Expand();
                                }
                                cat.Nodes.Add(node);
                                test.OnPacketTraceChanged += new PacketTraceHandler(OnPacketTraceChangedSink);
                                test.OnProgressChanged += new System.EventHandler(OnProgressChangedSink);
                                test.OnStateChanged += new System.EventHandler(OnStateChangedSink);
                                test.OnEventLog += new LogEventHandler(EventLogSink);

                                UpdateCounters();
                            }
                            break;
                        }
                    }
                }
            }
        }


        private void EventLogSink(IUPnPTestGroup sender, LogStruct log)
        {
            this.BeginInvoke(new UpdateLogForCurrentTestHandler(UpdateLogForCurrentTest), new object[] { sender, log });
        }

        private delegate void UpdateLogForCurrentTestHandler(IUPnPTestGroup sender, LogStruct log);
        private void UpdateLogForCurrentTest(IUPnPTestGroup sender, LogStruct log)
        {
            if (sender != displaytestinfo) return;

            int icon = this.getImageIndexForLogImportance(log.importance);
            logListBox.Items.Add(new ListViewItem(new string[] { log.LogEntry }, icon));
        }


        private void OnPacketTraceChangedSink(IUPnPTestGroup sender, HTTPMessage packet)
        {
            this.BeginInvoke(new UpdatePacketTraceForCurrentTestHandler(UpdatePacketTraceForCurrentTest), new object[] { sender, packet });
        }

        private delegate void UpdatePacketTraceForCurrentTestHandler(IUPnPTestGroup sender, HTTPMessage msg);
        private void UpdatePacketTraceForCurrentTest(IUPnPTestGroup sender, HTTPMessage msg)
        {
            if (sender != displaytestinfo) return;

            ListViewItem lv;
            if (msg.StatusCode == -1)
            {
                lv = new ListViewItem(new string[] { msg.RawPacket.Length.ToString() + " (" + msg.BodyBuffer.Length + ")", msg.Directive + " " + msg.DirectiveObj }, 5);
            }
            else
            {
                lv = new ListViewItem(new string[] { msg.RawPacket.Length.ToString() + " (" + msg.BodyBuffer.Length + ")", "(" + msg.StatusCode.ToString() + ") " + msg.StatusData }, 5);
            }
            lv.Tag = msg;
            packetListView.Items.Add(lv);
        }


        private void OnProgressChangedSink(object sender, System.EventArgs e)
        {
            if (InvokeRequired) { Invoke(new System.EventHandler(OnProgressChangedSink), sender, e); return; }

            IUPnPTestGroup test = (IUPnPTestGroup)sender;
            progressBar.Value = test.Progress;
            progressLabel.Text = test.Progress.ToString() + "%";
        }

        private void OnStateChangedSink(object sender, System.EventArgs e)
        {
            if (InvokeRequired) { Invoke(new System.EventHandler(OnStateChangedSink), sender, e); return; }

            IUPnPTestGroup test = (IUPnPTestGroup)sender;
            TreeNode node = NodeTagSearch(UPnpTestRoot, test);
            if (node == null) return;

            if (test.Enabled == false)
            {
                node.ImageIndex = 9;
                node.SelectedImageIndex = 9;
            }
            else
            {
                int imageIndex = this.getImageIndexForUPnPTestState(test.GroupState);
                node.ImageIndex = imageIndex;
                node.SelectedImageIndex = imageIndex;
            }

            for (int i = 0; i < test.TestNames.Length; i++)
            {
                foreach (TreeNode n in node.Nodes)
                {
                    if (n.Text == test.TestNames[i])
                    {
                        if (test.Enabled == false)
                        {
                            n.ImageIndex = 9;
                            n.SelectedImageIndex = 9;
                        }
                        else
                        {
                            int imageIndex = this.getImageIndexForUPnPTestState(test.TestStates[i]);
                            n.ImageIndex = imageIndex;
                            n.SelectedImageIndex = imageIndex;
                        }
                    }
                }
            }

            if (test == displaytestinfo) SetTestInfo(displaytestinfo);
        }

        private void MenuItem_Click(object sender, System.EventArgs e)
        {
            ((MenuItem)sender).Checked = !((MenuItem)sender).Checked;
        }

        private void MainForm_Closed(object sender, System.EventArgs e)
        {
            if (thread != null) thread.Abort();
        }

        private void exitMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void loadTestModuleMenuItem_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openModuleFileDialog.ShowDialog(this);
            String FileExtForDeviceScriptorScriptFile = ".dsprj";
            if (r == DialogResult.OK && openModuleFileDialog.FileName != null)
            {
                FileInfo Info = new FileInfo(openModuleFileDialog.FileName);
                if (FileExtForDeviceScriptorScriptFile.CompareTo(Info.Extension.ToLower()) != 0)
                {
                    Assembly testassembly = System.Reflection.Assembly.LoadFrom(openModuleFileDialog.FileName);
                    AddTestsInAssembly(testassembly);
                }
                else
                {
                    try
                    {
                        IUPnPTestGroup testgroup = (IUPnPTestGroup)new UPnPValidator.BasicTests.DeviceScriptorTest(openModuleFileDialog.FileName);
                        if (testgroup != null)
                            AddTestGroupInTheList(testgroup);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(this, ex.Message, "Script Load Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }

        private void saveResultsMenuItem_Click(object sender, System.EventArgs e)
        {
            DialogResult r = saveResultFileDialog.ShowDialog(this);
            if (r == DialogResult.OK && saveResultFileDialog.FileName != null)
            {
                System.Xml.XmlDocument doc = new System.Xml.XmlDocument();
                System.Xml.XmlElement topElement =
                    SaveRootNodeToXmlElement(doc, UPnpTestRoot);

                doc.LoadXml("<ValidatorData></ValidatorData>");
                doc.DocumentElement.AppendChild(topElement);

                System.Xml.XmlTextWriter textWriter =
                    new System.Xml.XmlTextWriter(saveResultFileDialog.FileName, null);
                textWriter.Formatting = System.Xml.Formatting.Indented;
                doc.Save(textWriter);


                #region Old code that is commented out and thus no longer used
                /* This has been commented out because we don't really want to 
				 * completely serialize the plugins anymore.  The way we really 
				 * want to save is just the log, results and packet trace to XML as
				 * done after this commented out region.
				 * 
				 * Hashtable table = new Hashtable();
				StoreNodeTagsToTable(UPnpTestRoot,table);
		
				foreach (IUPnPTestGroup test in table.Values) 
				{
					test.OnPacketTraceChanged -= new PacketTraceHandler(OnPacketTraceChangedSink);
					test.OnProgressChanged -= new System.EventHandler(OnProgressChangedSink);
					test.OnStateChanged -= new System.EventHandler(OnStateChangedSink);
					//test.OnLogChanged -= new System.EventHandler(OnLogChangedSink);
				}

				IFormatter formatter = new BinaryFormatter();
				Stream stream = new FileStream(saveResultFileDialog.FileName, FileMode.Create, FileAccess.Write, FileShare.None);
				formatter.Serialize(stream, table);
				stream.Close();

				foreach (IUPnPTestGroup test in table.Values) 
				{
					test.OnPacketTraceChanged += new PacketTraceHandler(OnPacketTraceChangedSink);
					test.OnProgressChanged += new System.EventHandler(OnProgressChangedSink);
					test.OnStateChanged += new System.EventHandler(OnStateChangedSink);
					//test.OnLogChanged += new System.EventHandler(OnLogChangedSink);
				}
				*/
                #endregion

            }
        }

        private void resetMenuItem_Click(object sender, System.EventArgs e)
        {
            if (displaytestinfo == null) return;
            displaytestinfo.Reset();
            SetTestInfo(displaytestinfo);

            OnStateChangedSink(displaytestinfo, null);
            //foreach (string testname in displaytestinfo.TestNames) 
            //{
            OnStateChangedSink(displaytestinfo, null);
            //}
        }

        private void skipMenuItem_Click(object sender, System.EventArgs e)
        {
            if (displaytestinfo == null) return;
            displaytestinfo.Enabled = skipMenuItem.Checked;

            OnStateChangedSink(displaytestinfo, null);
            //foreach (string testname in displaytestinfo.TestNames) 
            //{
            OnStateChangedSink(displaytestinfo, null);
            //}
            UpdateCounters();
        }

        private void executeMenuItem_Click(object sender, System.EventArgs e)
        {
            if (displaytestinfo == null || thread != null) return;

            displaytestinfo.Enabled = true;
            OnStateChangedSink(displaytestinfo, null);

            RunAllTests = false;
            thread = new Thread(new ThreadStart(StartSingleTest));
            thread.Start();
        }

        private void showDebugInfoMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void resetAllTestsMenuItem_Click(object sender, System.EventArgs e)
        {
            // Reset All
            foreach (IUPnPTestGroup t in testgroups)
            {
                t.Reset();
                OnStateChangedSink(t, null);
            }
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Validator");
        }

        private void loadResultsMenuItem_Click(object sender, System.EventArgs e)
        {

            DialogResult r = openResultFileDialog.ShowDialog(this);
            if (r == DialogResult.OK && openResultFileDialog.FileName != null)
            {
                if (!_inViewOnlyMode)
                {
                    _inViewOnlyMode = true;
                    setTargetButton.Hide();
                    startTestsButton.Hide();
                    execTestGroupButton.Hide();
                    resetTestGroupButton.Hide();
                    returnToTestingButton.Show();

                    menuItem2.Enabled = false;
                    saveResultsMenuItem.Enabled = false;
                    loadTestModuleMenuItem.Enabled = false;

                    _prevRoot = UPnpTestRoot;
                    UPnpTestRoot = null;
                    testTree.Nodes.Remove(_prevRoot);

                    _prevDeviceString = statusLabel1.Text;

                }

                testTree.Nodes.Clear();
                UPnpTestRoot = loadXmlResultsFile(openResultFileDialog.FileName);
                testTree.Nodes.Add(UPnpTestRoot);
                testTree.ExpandAll();

                testTree.SelectedNode = testTree.Nodes[0];


                logDetailTextBox.Text = "";
                packetTextBox.Text = "";

                this.Text = "Device Validator - " +
                    openResultFileDialog.FileName;
            }
            UpdateCounters();
        }

        private void copylogMenuItem_Click(object sender, System.EventArgs e)
        {
            if (logListBox.SelectedItems.Count > 0)
            {
                string ClippedText = null;
                foreach (ListViewItem item in logListBox.SelectedItems)
                {
                    ClippedText += item.Text + "\r\n";
                }
                Clipboard.SetDataObject(ClippedText);
            }
        }


        private TreeNode NodeTagSearch(TreeNode node, object searchTag)
        {
            if (node.Tag != null && node.Tag == searchTag) return node;
            TreeNode r = null;
            foreach (TreeNode n in node.Nodes)
            {
                r = NodeTagSearch(n, searchTag);
                if (r != null) return r;
            }
            return null;
        }

        private void SetTestInfo(IUPnPTestGroup test)
        {
            displaytestinfo = test;
            if (test == null)
            {
                descriptionTextBox.Text = "";
                resultTextBox.Text = "";
                testHeaderLabel.Text = "Test Summary";
                logListBox.Items.Clear();
                packetListView.Items.Clear();
                packetTextBox.Text = "";
                resetTestGroupButton.Enabled = false;
                execTestGroupButton.Enabled = false;
            }
            else
            {

                if (test.Enabled == false)
                {
                    resultTextBox.Text = "Test Group Skipped";
                }
                else
                {
                    string rs = "";
                    foreach (string ts in test.Result)
                    {
                        if (rs == "")
                        {
                            rs = ts;
                        }
                        else
                        {
                            rs = rs + "\r\n" + ts;
                        }
                    }
                    resultTextBox.Text = rs;
                }

                //if (test.Result == null || test.Result == "") resultTextBox.Text = "Results not available.";
                testHeaderLabel.Text = "Test Summary - " + test.GroupName;

                logListBox.Items.Clear();
                foreach (LogStruct ls in test.Log)
                {
                    int icon = getImageIndexForLogImportance(ls.importance);
                    logListBox.Items.Add(new ListViewItem(new string[] { ls.LogEntry }, icon));
                }

                //if (test.Log == null || test.Log == "") logTextBox.Text = "Test log not available.";
                packetListView.Items.Clear();
                packetTextBox.Text = "";
                foreach (HTTPMessage msg in test.PacketTrace)
                {
                    ListViewItem lv;
                    if (msg.StatusCode == -1)
                    {
                        lv = new ListViewItem(new string[] { msg.RawPacket.Length.ToString() + " (" + msg.BodyBuffer.Length + ")", msg.Directive + " " + msg.DirectiveObj }, 5);
                    }
                    else
                    {
                        lv = new ListViewItem(new string[] { msg.RawPacket.Length.ToString() + " (" + msg.BodyBuffer.Length + ")", "(" + msg.StatusCode.ToString() + ") " + msg.StatusData }, 5);
                    }
                    lv.Tag = msg;
                    packetListView.Items.Add(lv);
                }

                if (targetdevice != null)
                {
                    resetTestGroupButton.Enabled = true;
                    execTestGroupButton.Enabled = true;
                }
            }
        }

        private void testTree_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
        {
            if (testTree.SelectedNode == null)
            {
                SetTestInfo(null);
            }
            else
            {
                try
                {
                    if (testTree.SelectedNode.Tag != null)
                    {
                        IUPnPTestGroup test = (IUPnPTestGroup)testTree.SelectedNode.Tag;
                        SetTestInfo(test);
                    }
                    else
                    {
                        IUPnPTestGroup test = (IUPnPTestGroup)testTree.SelectedNode.Parent.Tag;
                        SetTestInfo(test);
                    }
                }
                catch
                {
                    SetTestInfo(null);
                }
            }
            UpdateDescription();
            UpdateDetailedPacketTextBox();
            UpdateDetailedLogTextBox();
        }

        private void testContextMenu_Popup(object sender, System.EventArgs e)
        {
            executeMenuItem.Visible = (displaytestinfo != null && targetdevice != null);
            menuItem7.Visible = (displaytestinfo != null && targetdevice != null);
            skipMenuItem.Visible = (displaytestinfo != null);
            resetMenuItem.Visible = (displaytestinfo != null);
            if (displaytestinfo != null) skipMenuItem.Checked = !displaytestinfo.Enabled;
        }

        private void testTree_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            TreeNode node = testTree.GetNodeAt(e.X, e.Y);
            if (node == null) return;
            testTree.SelectedNode = node;
        }





        private delegate void SetUserInterfaceHandler(bool visible);
        private void SetUserInterface(bool visible)
        {
            UpdateCounters();
            if (visible == true)
            {
                progressBar.Value = 0;
                progressLabel.Text = "0%";
                progressStepBar.Maximum = 1;
                progressStepBar.Value = 0;
                progressStepLabel.Text = "1 of 1";
                progressPanel.Visible = true;
                startTestsButton.Text = "Halt Tests";
                startTestMenuItem.Enabled = false;
                haltTestsMenuItem.Enabled = true;
                logListBox.Items.Clear();
            }
            else
            {
                progressStepBar.Value = 1;
                progressPanel.Visible = false;
                startTestsButton.Text = "Execute Tests";
                startTestMenuItem.Enabled = true;
                haltTestsMenuItem.Enabled = false;
            }
        }

        private delegate void SetUserInterfaceStepHandler(int val, int max);
        private void SetUserInterfaceStep(int val, int max)
        {
            progressStepBar.Maximum = max;
            progressStepBar.Value = val;
            progressStepLabel.Text = val.ToString() + " of " + max.ToString();
        }

        private delegate void SetMainStatusBarHandler(string text);
        private void SetMainStatusBar(string text)
        {
            if (InvokeRequired) { Invoke(new SetMainStatusBarHandler(SetMainStatusBar), text); return; }
            mainStatusBar.Text = text;
        }

        private delegate void SetProgressBarHandler(int v);
        private void SetProgressBar(int v)
        {
            if (InvokeRequired) { Invoke(new SetProgressBarHandler(SetProgressBar), v); return; }
            progressBar.Value = v;
        }

        private void SetProgressLabel(string text)
        {
            if (InvokeRequired) { Invoke(new SetMainStatusBarHandler(SetProgressLabel), text); return; }
            progressLabel.Text = text;
        }

        private void StartSingleTest()
        {
            try
            {
                if (targetdevice == null) return;
                if (RunAllTests == false && displaytestinfo == null) return;

                if (RunAllTests == false)
                {
                    this.Invoke(new SetUserInterfaceHandler(SetUserInterface), new object[] { true });
                    displaytestinfo.Enabled = true;
                    SetMainStatusBar("Reseting " + displaytestinfo.GroupName + ".");
                    displaytestinfo.Reset();
                    SetMainStatusBar("Executing " + displaytestinfo.GroupName + ".");
                    displaytestinfo.Start(targetdevice);
                    SetMainStatusBar(displaytestinfo.GroupName + " terminated.");
                    progressStepBar.Value = 1;
                }
                else
                {
                    // Figure out what tests need to be executed.
                    pendingtestlist.Clear();
                    foreach (IUPnPTestGroup test in testgroups)
                    {
                        if (test.Enabled == true && test.GroupState == UPnPTestStates.Ready)
                        {
                            pendingtestlist.Add(test);
                        }
                    }

                    if (pendingtestlist.Count == 0) return;

                    int testcount = pendingtestlist.Count;
                    this.BeginInvoke(new SetUserInterfaceHandler(SetUserInterface), true);
                    this.BeginInvoke(new SetUserInterfaceStepHandler(SetUserInterfaceStep), 0, testcount);

                    while (pendingtestlist.Count > 0)
                    {
                        IUPnPTestGroup test = (IUPnPTestGroup)pendingtestlist[0];
                        SetProgressBar(0);
                        SetProgressLabel("0%");
                        if (progressStepBar.Value == progressStepBar.Maximum) { progressStepBar.Maximum = progressStepBar.Maximum + 1; }
                        SetProgressBar(progressStepBar.Value + 1);
                        progressStepLabel.Text = progressStepBar.Value.ToString() + " of " + progressStepBar.Maximum.ToString();
                        haltTestsMenuItem.Enabled = true;
                        SetMainStatusBar("Reseting " + test.GroupName + ".");
                        test.Reset();
                        SetMainStatusBar("Executing " + test.GroupName + ".");
                        test.Start(targetdevice);
                        SetMainStatusBar(test.GroupName + " terminated.");
                        pendingtestlist.RemoveAt(0);
                        UpdateCounters();
                        this.BeginInvoke(new SetUserInterfaceStepHandler(SetUserInterfaceStep), testcount - pendingtestlist.Count, testcount);
                    }
                }
            }
            catch (System.Threading.ThreadInterruptedException)
            {
                SetMainStatusBar("Test group execution halted.");
            }

            this.BeginInvoke(new SetUserInterfaceHandler(SetUserInterface), new object[] { false });
            thread = null;
        }


        private TreeNode NodeTagTypeSearch(TreeNode node, string typename)
        {
            if (node.Tag != null && node.Tag.GetType().FullName == typename) return node;
            TreeNode r = null;
            foreach (TreeNode n in node.Nodes)
            {
                r = NodeTagTypeSearch(n, typename);
                if (r != null) return r;
            }
            return null;
        }

        private void menuItem1_Popup(object sender, System.EventArgs e)
        {
            startTestMenuItem.Enabled = (targetdevice != null);
        }

        private void packetListView_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            UpdateDetailedPacketTextBox();
        }

        private void logListBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            UpdateDetailedLogTextBox();
        }


        private void startTestsButton_Click(object sender, System.EventArgs e)
        {
            if (targetdevice == null) return;

            if (thread != null)
            {
                // Halt Test
                //if (runningtest != null) runningtest.Cancel();

                thread.Interrupt();
                thread = null;
            }
            else
            {
                // Start Tests
                RunAllTests = true;
                thread = new Thread(new ThreadStart(StartSingleTest));
                thread.Start();
            }
        }

        private void StopTestsButton_Click(object sender, System.EventArgs e)
        {
            if (targetdevice == null) return;

            if (thread != null)
            {
                // Halt Test
                //if (runningtest != null) runningtest.Cancel();

                thread.Interrupt();
                thread = null;
            }
        }

        private void returnToTestingButton_Click(object sender, System.EventArgs e)
        {

            if (_inViewOnlyMode)
            {
                _inViewOnlyMode = false;
                setTargetButton.Show();
                startTestsButton.Show();
                execTestGroupButton.Show();
                resetTestGroupButton.Show();
                returnToTestingButton.Hide();

                menuItem2.Enabled = true;
                saveResultsMenuItem.Enabled = true;
                loadTestModuleMenuItem.Enabled = true;

                testTree.Nodes.Clear();
                UPnpTestRoot = _prevRoot;
                testTree.Nodes.Add(UPnpTestRoot);
                _prevRoot = null;
                testTree.SelectedNode = testTree.Nodes[0];

                statusLabel1.Text = _prevDeviceString;

                logDetailTextBox.Text = "";
                packetTextBox.Text = "";

                this.Text = "Device Validator";
            }
            else
            {
                throw new Exception("This should never be possible");
            }

            UpdateCounters();
        }


        private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Device Validator");
        }

        private void AddTestGroupInTheList(IUPnPTestGroup test)
        {
            testgroups.Add(test);
            TreeNode cat = NodeTagSearch(UPnpTestRoot, test.Category);
            if (cat == null)
            {
                cat = new TreeNode(test.Category, 6, 7);
                cat.Tag = test.Category;
                UPnpTestRoot.Nodes.Add(cat);
                cat.Expand();
            }
            test.Enabled = true;
            TreeNode node = new TreeNode(test.GroupName, 8, 8);
            node.Tag = test;
            if (test.TestNames.Length > 0)
            {
                foreach (string TestName in test.TestNames)
                {
                    TreeNode snode = new TreeNode(TestName, 8, 8);
                    node.Nodes.Add(snode);
                }
                node.Expand();
            }
            cat.Nodes.Add(node);
            test.OnPacketTraceChanged += new PacketTraceHandler(OnPacketTraceChangedSink);
            test.OnProgressChanged += new System.EventHandler(OnProgressChangedSink);
            test.OnStateChanged += new System.EventHandler(OnStateChangedSink);
            test.OnEventLog += new LogEventHandler(EventLogSink);
            //test.OnLogChanged += new System.EventHandler(OnLogChangedSink);

            UpdateCounters();
        }


        private void UpdateDescription()
        {
            descriptionTextBox.Text = "";
            IUPnPTestGroup test = null;
            if (testTree.SelectedNode != null)
            {
                if (testTree.SelectedNode.Tag != null)
                {
                    test = testTree.SelectedNode.Tag as IUPnPTestGroup;
                    if (test != null)
                        descriptionTextBox.Text = test.Description;
                }
                else
                {
                    int i = 0;
                    TreeNode node = testTree.SelectedNode.Parent;
                    if (node == null)
                        return;
                    test = (IUPnPTestGroup)testTree.SelectedNode.Parent.Tag;
                    if (test == null)
                        return;
                    node = testTree.SelectedNode.Parent.FirstNode;
                    while (node != null)
                    {
                        if (node == testTree.SelectedNode)
                        {
                            descriptionTextBox.Text = test.TestDescription[i];
                            break;
                        }
                        node = node.NextNode;
                        i++;
                    }
                }
            }
        }

        private void UpdateDetailedPacketTextBox()
        {
            if (packetListView.SelectedItems.Count == 0)
            {
                packetTextBox.Text = "";
            }
            else
            {
                HTTPMessage msg = (HTTPMessage)packetListView.SelectedItems[0].Tag;
                packetTextBox.Text = msg.StringPacket;
            }
        }

        private void UpdateDetailedLogTextBox()
        {
            if (logListBox.SelectedItems.Count == 0)
            {
                logDetailTextBox.Text = "";
            }
            else
            {
                logDetailTextBox.Text =
                    logListBox.SelectedItems[0].Text;
            }
        }

        private void UpdateCounters()
        {
            if (InvokeRequired) { Invoke(new System.Threading.ThreadStart(UpdateCounters)); return; }

            testcount_total = 0;
            testcount_pass = 0;
            testcount_warn = 0;
            testcount_failed = 0;
            testcount_skip = 0;
            RecountSubTree(UPnpTestRoot);
            countPassLabel.Text = testcount_pass.ToString();
            countWarnLabel.Text = testcount_warn.ToString();
            countFailLabel.Text = testcount_failed.ToString();
            countSkipLabel.Text = testcount_skip.ToString();
        }

        private void RecountSubTree(TreeNode node)
        {
            if (node.Tag != null && node.Tag.GetType() != typeof(string))
            {
                IUPnPTestGroup test = (IUPnPTestGroup)node.Tag;

                if (test.TestNames.Length == 0)
                {
                    testcount_total++;
                    if (test.Enabled == false)
                    {
                        testcount_skip++;
                    }
                    else
                    {
                        switch (test.GroupState)
                        {
                            case UPnPTestStates.Ready:
                                testcount_ready++;
                                break;
                            case UPnPTestStates.Failed:
                                testcount_failed++;
                                break;
                            case UPnPTestStates.Pass:
                                testcount_pass++;
                                break;
                            case UPnPTestStates.Warn:
                                testcount_warn++;
                                break;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < test.TestNames.Length; i++)
                    {
                        testcount_total++;
                        if (test.Enabled == false)
                        {
                            testcount_skip++;
                        }
                        else
                        {
                            switch (test.TestStates[i])
                            {
                                case UPnPTestStates.Ready:
                                    testcount_ready++;
                                    break;
                                case UPnPTestStates.Failed:
                                    testcount_failed++;
                                    break;
                                case UPnPTestStates.Pass:
                                    testcount_pass++;
                                    break;
                                case UPnPTestStates.Warn:
                                    testcount_warn++;
                                    break;
                            }
                        }
                    }
                }
            }
            foreach (TreeNode n in node.Nodes) RecountSubTree(n);
        }


        private void GetLogInformation(TreeNode node, StreamWriter sr)
        {
            foreach (TreeNode n in node.Nodes)
            {
                IUPnPTestGroup test = null;
                if (n.Tag != null)
                    test = n.Tag as IUPnPTestGroup;
                if (test != null)
                {
                    sr.WriteLine("\r\n--------------------------------------------");
                    sr.WriteLine("\r\nTest Name :" + test.GroupName);
                    sr.WriteLine("\r\nDescrition :" + test.Description);
                    sr.WriteLine("\r\nExecution Logs : ");
                    for (int i = 0; i < test.Log.Count; i++)
                    {
                        string Importance = null;
                        if (((LogStruct)test.Log[i]).importance == LogImportance.Critical)
                            Importance = "Error: ";
                        else if (((LogStruct)test.Log[i]).importance != LogImportance.Remark)
                            Importance = "Warning: ";
                        sr.WriteLine("\r\n" + Importance + ((LogStruct)test.Log[i]).LogEntry);
                    }
                    sr.WriteLine("\r\nResults:");
                    for (int i = 0; i < test.Result.GetLength(0); i++)
                        sr.WriteLine("\r\n" + test.Result[i]);
                }
                GetLogInformation(n, sr);
            }
        }




        private TreeNode loadXmlResultsFile(string filename)
        {
            System.Xml.XmlDocument doc = new System.Xml.XmlDocument();
            System.Xml.XmlTextReader reader = new System.Xml.XmlTextReader(filename);
            reader.MoveToContent();
            doc.Load(reader);

            System.Xml.XmlElement validatorDataElement =
                doc.FirstChild as System.Xml.XmlElement;
            System.Xml.XmlElement target =
                validatorDataElement.GetElementsByTagName("Target").Item(0)
                as System.Xml.XmlElement;
            statusLabel1.Text =
                "Results from validating target: " +
                target.GetAttribute("friendlyName") + " / " +
                target.GetAttribute("uniqueDeviceName");

            return extractRootFromXmlElement(doc.DocumentElement.FirstChild as System.Xml.XmlElement);

        }

        private TreeNode extractRootFromXmlElement(System.Xml.XmlElement root)
        {
            TreeNode ret = new TreeNode(root.GetAttribute("name"));

            foreach (System.Xml.XmlElement child in root.ChildNodes)
            {
                if (child.Name == "Category")
                {
                    ret.Nodes.Add(extractCategoryFromXmlElement(child));
                }
            }

            return ret;
        }

        private TreeNode extractCategoryFromXmlElement(System.Xml.XmlElement cat)
        {
            TreeNode ret = new TreeNode(cat.GetAttribute("name"), 6, 7);

            foreach (System.Xml.XmlElement testGroup in cat.ChildNodes)
            {
                int imgIdx = getImageIndexForUPnPTestState(
                    Util.StringToUPnPTestStates(testGroup.GetAttribute("state")));

                TreeNode group = new TreeNode(testGroup.GetAttribute("name"), imgIdx, imgIdx);
                group.Tag =
                    new UPnPValidator.BasicTests.UPnPTestViewOnly(cat.GetAttribute("name"), testGroup);

                ret.Nodes.Add(group);


                foreach (System.Xml.XmlElement testCase in testGroup.ChildNodes)
                {
                    if (testCase.Name == "Test")
                    {
                        imgIdx = getImageIndexForUPnPTestState(
                            Util.StringToUPnPTestStates(testCase.GetAttribute("state")));
                        group.Nodes.Add(new TreeNode(testCase.GetAttribute("name"), imgIdx, imgIdx));
                    }
                }
            }
            return ret;
        }


        #region Unused methods that are now commented out

        /*
		private void loadResultsMenuItem_Click(object sender, System.EventArgs e)
		{
			throw new Exception("This method not implemented");
			DialogResult r = openResultFileDialog.ShowDialog(this);
			if (r == DialogResult.OK && openResultFileDialog.FileName != null) 
			{
				SetTestInfo(null);
				testTree.SelectedNode = null;

				IFormatter formatter = new BinaryFormatter();
				Stream stream = new FileStream(openResultFileDialog.FileName, FileMode.Open, FileAccess.Read, FileShare.Read);
				Hashtable table = (Hashtable) formatter.Deserialize(stream);
				stream.Close();

				foreach (string key in table.Keys) 
				{
					TreeNode node = NodeTagTypeSearch(UPnpTestRoot,key);
					if (node != null) 
					{
						IUPnPTestGroup test = (IUPnPTestGroup)node.Tag;
						test.OnPacketTraceChanged -= new PacketTraceHandler(OnPacketTraceChangedSink);
						test.OnProgressChanged -= new System.EventHandler(OnProgressChangedSink);
						test.OnStateChanged -= new System.EventHandler(OnStateChangedSink);
						//test.OnLogChanged -= new System.EventHandler(OnLogChangedSink);
						test = (IUPnPTestGroup)table[key];
						node.Tag = test;
						test.OnPacketTraceChanged += new PacketTraceHandler(OnPacketTraceChangedSink);
						test.OnProgressChanged += new System.EventHandler(OnProgressChangedSink);
						test.OnStateChanged += new System.EventHandler(OnStateChangedSink);
						//test.OnLogChanged += new System.EventHandler(OnLogChangedSink);
						//OnStateChangedSink(test,null);
					}
				}

				UpdateCounters();
			}
		}

		private void StoreNodeTagsToTable(TreeNode node, Hashtable table) 
		{
			if (node.Tag != null) 
			{
				if (node.Tag.GetType() != typeof(string))
				{
					IUPnPTestGroup test = (IUPnPTestGroup)node.Tag;
					table.Add(test.GetType().FullName,test);
				}
			} 

			foreach (TreeNode subnode in node.Nodes) 
			{
				StoreNodeTagsToTable(subnode,table);
			}
		}
*/
        #endregion


        /// <summary>
        /// Creates the XmlElement for the top level node of the test
        /// tree (UPnpTestRoot) and then puts in all the elements that belong 
        /// inside it.
        /// </summary>
        /// <param name="doc">
        /// The XmlDocument that this element will
        /// eventually become part of.  This field is required because of the way
        /// that C# makes you create XmlElement nodes.
        /// </param>
        /// <param name="node">
        ///	The root node of the tree of tests.
        /// </param>
        /// <returns>
        /// A System.Xml.XmlElement containing the data from the root node and all the sub-nodes.
        /// </returns>
        private System.Xml.XmlElement SaveRootNodeToXmlElement(
            System.Xml.XmlDocument doc,
            TreeNode node)
        {
            System.Xml.XmlElement ret = doc.CreateElement("Root");
            ret.SetAttribute("name", node.Text);


            System.Xml.XmlElement targetElement = doc.CreateElement("Target");
            targetElement.SetAttribute("friendlyName", targetdevice.FriendlyName);
            targetElement.SetAttribute("uniqueDeviceName", targetdevice.UniqueDeviceName);
            ret.AppendChild(targetElement);

            System.Xml.XmlElement summaryElement = doc.CreateElement("Summary");
            summaryElement.SetAttribute("pass", testcount_pass.ToString());
            summaryElement.SetAttribute("warn", testcount_warn.ToString());
            summaryElement.SetAttribute("failed", testcount_failed.ToString());
            summaryElement.SetAttribute("skip", testcount_skip.ToString());
            ret.AppendChild(summaryElement);

            foreach (TreeNode subnode in node.Nodes)
            {
                System.Xml.XmlElement childElement =
                    SaveCategoryNodeToXmlElement(doc, subnode);
                ret.AppendChild(childElement);
            }
            return ret;
        }

        /// <summary>
        /// Creates the element for the second level of the test tree.  This should
        /// be the level denoted by the folders in the gui and is the level directly
        /// above the test group level
        /// </summary>
        /// <param name="doc">
        /// The XmlDocument object that will eventually contain this element
        /// </param>
        /// <param name="node">
        ///	The TreeNode that represents this element
        /// </param>
        /// <returns>
        /// The XmlElement representing the TreeNode and all the children underneath
        /// </returns>
        private System.Xml.XmlElement SaveCategoryNodeToXmlElement(
            System.Xml.XmlDocument doc,
            TreeNode node)
        {
            System.Xml.XmlElement ret = doc.CreateElement("Category");
            ret.SetAttribute("name", node.Text);
            foreach (TreeNode subnode in node.Nodes)
            {
                ret.AppendChild(
                    SaveTestGroupNodeToXmlElement(doc, subnode));
            }
            return ret;
        }

        /// <summary>
        /// Creates the XmlElement for the third level, or TestGroup level of the 
        /// tree.
        /// </summary>
        /// <param name="doc">
        /// The XmlDocument object that will eventually contain this Element.
        /// </param>
        /// <param name="node">
        /// The TreeNode representation of this TestGroup
        /// </param>
        /// <returns>
        /// The XmlElement that contains the TestGroup and all its children.
        /// </returns>
        private System.Xml.XmlElement SaveTestGroupNodeToXmlElement(
            System.Xml.XmlDocument doc,
            TreeNode node)
        {
            System.Xml.XmlElement ret = doc.CreateElement("TestGroup");
            IUPnPTestGroup group = node.Tag as IUPnPTestGroup;
            if (group != null)
            {
                ret.SetAttribute("name", group.GroupName);
                ret.SetAttribute("state", Util.UPnPTestStatesToString(group.GroupState));
                ret.SetAttribute("description", group.Description);



                for (int i = 0; i < group.TestNames.GetLength(0); i++)
                {
                    string result = "";
                    if (group.Result.GetLength(0) > i)
                    {
                        result = group.Result[i];
                    }
                    System.Xml.XmlElement testElement =
                        SaveTestToXmlElement(
                        doc,
                        group.TestNames[i],
                        Util.UPnPTestStatesToString(group.TestStates[i]),
                        group.TestDescription[i],
                        result,
                        group.Log);

                    ret.AppendChild(testElement);
                }


                // output the packets.  I wish that there were a way to 
                // associate this with the individual tests, but it doesn't
                // appear to be possible with the current data structures.
                foreach (HTTPMessage httpMesg in group.PacketTrace)
                {
                    byte[] packetBytes = httpMesg.RawPacket;
                    System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
                    System.Text.Decoder utf8Dec = utf8.GetDecoder();
                    int len = packetBytes.Length;
                    char[] packetChars = new char[len];
                    utf8Dec.GetChars(packetBytes, 0, len, packetChars, 0);
                    string packetString = new String(packetChars);

                    System.Xml.XmlElement packetElement =
                        doc.CreateElement("Packet");
                    packetElement.InnerText = packetString;
                    ret.AppendChild(packetElement);
                }
            }
            return ret;
        }

        /// <summary>
        /// Creates an XmlElement for the given test.  This is a test that is 
        /// inside of the TestGroup, so it doesn't have a proper data 
        /// structure.  The information is stored in parallel arrays :P"
        /// Because of the parallel arrays, we have to get all the different
        /// attributes passed in individually.
        /// </summary>
        /// <param name="doc">
        /// The XmlDocument object that will eventually contain this XmlElement
        /// </param>
        /// <param name="name">
        /// The user readable name of the test
        /// </param>
        /// <param name="state">
        /// The string version of the state of the test.  Note that this is 
        /// usually an enum that must be converted.
        /// </param>
        /// <param name="description">
        /// The friendly description of what this test does.
        /// </param>
        /// <param name="result">
        /// The result of this test.  You should pass in an empty string if the 
        /// result does not yet exist.
        /// </param>
        /// <param name="allLogs">
        ///	The list of all the logs from the parent TestGroup.  These will be 
        ///	filtered by this function so that only the log entries pertaining to
        ///	this test will be included in this test's element.
        /// </param>
        /// <returns></returns>
        private System.Xml.XmlElement SaveTestToXmlElement(
            System.Xml.XmlDocument doc,
            string name,
            string state,
            string description,
            string result,
            IList allLogs)
        {
            System.Xml.XmlElement ret = doc.CreateElement("Test");
            ret.SetAttribute("name", name);
            ret.SetAttribute("state", state);
            ret.SetAttribute("result", result);
            ret.SetAttribute("description", description);

            foreach (LogStruct log in allLogs)
            {
                if (log.TestName.Equals(name))
                {
                    System.Xml.XmlElement logElement =
                        doc.CreateElement("LogEntry");
                    logElement.SetAttribute("importance", UPnPValidator.Util.LogImportanceToString(log.importance));
                    logElement.InnerText = log.LogEntry;
                    ret.AppendChild(logElement);
                }
            }
            return ret;
        }



        private int getImageIndexForUPnPTestState(UPnPTestStates state)
        {
            switch (state)
            {
                case UPnPTestStates.Failed:
                    return 10;
                case UPnPTestStates.Pass:
                    return 12;
                case UPnPTestStates.Warn:
                    return 11;
                default:
                    return 8;
            }
        }

        private int getImageIndexForLogImportance(LogImportance li)
        {
            switch (li)
            {
                case LogImportance.Critical:
                    return 10;
                case LogImportance.Remark:
                    return 12;
                default:
                    return 11;
            }
        }
    }
}
