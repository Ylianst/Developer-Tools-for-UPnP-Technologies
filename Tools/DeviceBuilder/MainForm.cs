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
using System.Collections.Generic;

using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing.Imaging;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {
        private string filePath;
        private System.Diagnostics.Process author;

        private bool MustExit = false;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem exitMenuItem;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem showDebugInfoMenuItem;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.StatusBar mainStatusBar;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.ImageList treeImageList;
        private System.ComponentModel.IContainer components;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.ToolTip mainToolTip;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.MenuItem newMenuItem;
        private System.Windows.Forms.MenuItem openMenuItem;
        private System.Windows.Forms.MenuItem saveMenuItem;
        private System.Windows.Forms.MenuItem saveAsMenuItem;
        private System.Windows.Forms.Label mainStatusLabel1;
        private System.Windows.Forms.ContextMenu treeContextMenu;
        private System.Windows.Forms.TreeView treeView;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.MenuItem menuItem10;
        private System.Windows.Forms.MenuItem menuItem11;
        private System.Windows.Forms.MenuItem menuItem15;
        private System.Windows.Forms.MenuItem menuItem20;
        private System.Windows.Forms.MenuItem addServiceMenuItem2;
        private System.Windows.Forms.MenuItem importServiceMenuItem2;
        private System.Windows.Forms.MenuItem removeServiceMenuItem2;
        private System.Windows.Forms.MenuItem addDeviceMenuItem2;
        private System.Windows.Forms.MenuItem removeDeviceMenuItem2;
        private System.Windows.Forms.MenuItem addServiceMenuItem;
        private System.Windows.Forms.MenuItem removeServiceMenuItem;
        private System.Windows.Forms.MenuItem addDeviceMenuItem;
        private System.Windows.Forms.MenuItem removeDeviceMenuItem;
        private System.Windows.Forms.MenuItem importServiceMenuItem;
        private System.Windows.Forms.OpenFileDialog openServiceDialog;
        private System.Windows.Forms.ContextMenu serviceContextMenu;
        private System.Windows.Forms.MenuItem actionNormalRespMenuItem;
        private System.Windows.Forms.MenuItem actionFragRespMenuItem;

        private UPnPServiceLocator serviceLocator = new UPnPServiceLocator();
        private Hashtable codeGenSettings = new Hashtable();
        private System.Windows.Forms.Label mainStatusLabel2;
        private System.Windows.Forms.MenuItem networkOpenMenuItem;
        private System.Windows.Forms.MenuItem helpMenuItem;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MenuItem exportServiceMenuItem;
        private System.Windows.Forms.MenuItem exportServiceMenuItem2;
        private System.Windows.Forms.SaveFileDialog exportSaveFileDialog;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem actionAutoEscapeMenuItem;
        private System.Windows.Forms.MenuItem actionManualEscapeMenuItem;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.Panel servicePanel;
        private System.Windows.Forms.Panel devicePanel;
        private System.Windows.Forms.TextBox rootDeviceTypeTextBox;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TextBox versionTextBox;
        private System.Windows.Forms.TextBox productCodeTextBox;
        private System.Windows.Forms.TextBox modelNumberTextBox;
        private System.Windows.Forms.TextBox modelNameTextBox;
        private System.Windows.Forms.TextBox modelDescriptionTextBox;
        private System.Windows.Forms.TextBox manufacturerUriTextBox;
        private System.Windows.Forms.TextBox manufacturerTextBox;
        private System.Windows.Forms.TextBox friendlyNameTextBox;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListView serviceListView;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.PictureBox pictureBox3;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox serviceNameTextBox;
        private System.Windows.Forms.TextBox serviceIdTextBox;
        private System.Windows.Forms.TextBox serviceTypeTextBox;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.MenuItem appendMenuItem;
        private System.Windows.Forms.MenuItem exportStacksMenuItem;
        private System.Windows.Forms.TabControl configTabControl;
        private System.Windows.Forms.TabPage configurationTabPage;
        private System.Windows.Forms.CheckBox devicePresentationPage;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.ComboBox deviceType;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox deviceCodePrefix;
        private System.Data.DataSet dataSet1;
        private System.Data.DataTable dataTable1;
        private System.Data.DataColumn dataColumn1;
        private System.Data.DataColumn dataColumn2;
        private System.Data.DataColumn dataColumn3;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.TextBox webPortTextBox;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.TextBox ssdpCycleTextBox;
        private System.Windows.Forms.Button setLargeIconButton;
        private System.Windows.Forms.DataGrid FieldGrid;
        private System.Windows.Forms.TextBox maxSubscriptionTimeoutTextBox;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.PictureBox deviceIconImageSM;
        private System.Windows.Forms.PictureBox deviceIconImageLG;
        private System.Windows.Forms.MenuItem editServiceMenuItem;
        private System.Windows.Forms.Panel emptyPanel;
        private TabPage descriptionTabPage;
        private SplitContainer splitContainer1;
        private TabControl tabControl1;
        private TabPage tabPage1;
        private TabPage tabPage2;
        private TabPage tabPage3;
        private object selectedItem = null;
        private GroupBox groupBox1;
        private GroupBox groupBox2;
        private GroupBox groupBox4;
        private GroupBox groupBox3;
        private GroupBox groupBox5;
        private GroupBox groupBox6;
        private Button setSmallIconButton;
        private Button clearIconButton;
        private Label label19;
        private Label label15;
        private MenuItem menuItem6;
        private string AppTitle;

        public MainForm(string[] args)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            AppTitle = this.Text;

            //			UPnPDevice device = UPnPDevice.CreateRootDevice(6000,1.0,".");
            //			device.FriendlyName = "Sample Device";
            //			device.DeviceURN = "urn:schemas-upnp-org:device:Sample:1";
            //			device.Manufacturer = "OpenSource";
            //			device.ManufacturerURL = "http://opentools.homeip.net";
            //			device.ModelDescription = "Sample UPnP Device Using Auto-Generated UPnP Stack";
            //			device.ModelName = "Sample Auto-Generated Device";
            //			device.ModelNumber = "X1";
            //			device.ProductCode = "Sample-X1";
            //			device.SerialNumber = "00000001";
            //
            //			deviceRootTreeNode.Tag = device;
            //			treeView.Nodes.Add(deviceRootTreeNode);

            devicePanel.Dock = DockStyle.Fill;
            emptyPanel.BringToFront();
            emptyPanel.Visible = true;

            updateStatusText();

            // Handle Open File Argument
            bool ok = false;
            bool MakeDevice = false;
            bool MakeCP = false;

            FileInfo file;
            if (args.Length > 0)
            {
                DText p = new DText();
                p.ATTRMARK = "=";
                foreach (string arg in args)
                {
                    if (ok)
                    {
                        switch (arg)
                        {
                            case "-dv":
                                {
                                    MakeDevice = true;
                                    break;
                                }
                            case "-cp":
                                {
                                    MakeCP = true;
                                    break;
                                }
                            default:
                                {
                                    p[0] = arg;
                                    string argval = p[2];
                                    switch (p[1])
                                    {
                                        case "-FriendlyName":
                                            {
                                                codeGenSettings["devicefriendlyName"] = argval;
                                                break;
                                            }
                                        case "-Manufacturer":
                                            {
                                                codeGenSettings["devicemanufacturer"] = argval;
                                                break;
                                            }
                                        case "-ManufacturerUri":
                                            {
                                                codeGenSettings["devicemanufacturerUri"] = argval;
                                                break;
                                            }
                                        case "-ModelDescription":
                                            {
                                                codeGenSettings["devicemodelDescription"] = argval;
                                                break;
                                            }
                                        case "-ModelName":
                                            {
                                                codeGenSettings["devicemodelName"] = argval;
                                                break;
                                            }
                                        case "-ModelNumber":
                                            {
                                                codeGenSettings["devicemodelNumber"] = argval;
                                                break;
                                            }
                                        case "-ProductCode":
                                            {
                                                codeGenSettings["deviceproductCode"] = argval;
                                                break;
                                            }
                                        case "-OutputPath":
                                            {
                                                codeGenSettings["outputpath"] = argval;
                                                break;
                                            }
                                        case "-HTTP/1.1":
                                            {
                                                if (argval.ToUpper() == "YES")
                                                {
                                                    codeGenSettings["HTTP11Support"] = true;
                                                }
                                                else if (argval.ToUpper() == "NO")
                                                {
                                                    codeGenSettings["HTTP11Support"] = false;
                                                }
                                                break;
                                            }
                                        case "-DefaultIPAddressMonitor":
                                            {
                                                if (argval.ToUpper() == "YES")
                                                {
                                                    codeGenSettings["DefaultIPAddressMonitor"] = true;
                                                }
                                                break;
                                            }
                                        case "-Supress":
                                            {
                                                if (argval.ToUpper() == "SAMPLE")
                                                {
                                                    codeGenSettings["SupressSample"] = true;
                                                }
                                                break;
                                            }
                                    }
                                    break;
                                }
                        }
                    }
                    else
                    {
                        file = new FileInfo(args[0]);
                        if (args.Length == 0)
                        {
                            if (file.Exists == true && file.Extension.ToLower() == ".upnpsg")
                            {
                                saveFileDialog.FileName = file.FullName;
                                ok = true;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (file.Exists == true && file.Extension.ToLower() == ".upnpsg")
                            {
                                OpenSettingsFile(file.FullName, false);
                                ok = true;
                            }
                            else
                            {
                                break;
                            }

                        }
                    }
                }
                UPnPDevice dv;
                if (MakeDevice)
                {
                    dv = BuildDevice(new TreeNode(), null); //ToDo: Fix This
                    if (dv != null)
                    {
                        //						this.Visible=false;
                        //						CodeGenerationForm form = new CodeGenerationForm(dv, stackSettings, fragRespActions,escapeActions);
                        //						form.Settings = codeGenSettings;
                        //						form.generateButton_Click(null,null);
                        //						this.MustExit=true;
                        //						Application.Exit();
                    }
                }
                if (MakeCP)
                {
                    dv = BuildDevice(new TreeNode(), null); //ToDo: Fix this
                    if (dv != null)
                    {
                        //						this.Visible=false;
                        //						CPCodeGenerationForm form = new CPCodeGenerationForm(dv, stackSettings, fragRespActions,cpEscapeActions);
                        //						form.Settings = codeCpGenSettings;
                        //						form.generateButton_Click(null,null);
                        //						this.MustExit=true;
                        //						Application.Exit();
                    }
                }
            }

            //outputPathTextBox.Text = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
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
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.newMenuItem = new System.Windows.Forms.MenuItem();
            this.openMenuItem = new System.Windows.Forms.MenuItem();
            this.networkOpenMenuItem = new System.Windows.Forms.MenuItem();
            this.appendMenuItem = new System.Windows.Forms.MenuItem();
            this.saveMenuItem = new System.Windows.Forms.MenuItem();
            this.saveAsMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.exportStacksMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem20 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.addServiceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.importServiceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.exportServiceMenuItem = new System.Windows.Forms.MenuItem();
            this.removeServiceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.addDeviceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.removeDeviceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.showDebugInfoMenuItem = new System.Windows.Forms.MenuItem();
            this.mainStatusBar = new System.Windows.Forms.StatusBar();
            this.treeImageList = new System.Windows.Forms.ImageList(this.components);
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.mainToolTip = new System.Windows.Forms.ToolTip(this.components);
            this.friendlyNameTextBox = new System.Windows.Forms.TextBox();
            this.treeView = new System.Windows.Forms.TreeView();
            this.treeContextMenu = new System.Windows.Forms.ContextMenu();
            this.addServiceMenuItem = new System.Windows.Forms.MenuItem();
            this.importServiceMenuItem = new System.Windows.Forms.MenuItem();
            this.exportServiceMenuItem2 = new System.Windows.Forms.MenuItem();
            this.editServiceMenuItem = new System.Windows.Forms.MenuItem();
            this.removeServiceMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.addDeviceMenuItem = new System.Windows.Forms.MenuItem();
            this.removeDeviceMenuItem = new System.Windows.Forms.MenuItem();
            this.panel2 = new System.Windows.Forms.Panel();
            this.mainStatusLabel2 = new System.Windows.Forms.Label();
            this.mainStatusLabel1 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.serviceContextMenu = new System.Windows.Forms.ContextMenu();
            this.actionNormalRespMenuItem = new System.Windows.Forms.MenuItem();
            this.actionFragRespMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.actionAutoEscapeMenuItem = new System.Windows.Forms.MenuItem();
            this.actionManualEscapeMenuItem = new System.Windows.Forms.MenuItem();
            this.openServiceDialog = new System.Windows.Forms.OpenFileDialog();
            this.exportSaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.servicePanel = new System.Windows.Forms.Panel();
            this.serviceListView = new System.Windows.Forms.ListView();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.label16 = new System.Windows.Forms.Label();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.label8 = new System.Windows.Forms.Label();
            this.serviceNameTextBox = new System.Windows.Forms.TextBox();
            this.serviceIdTextBox = new System.Windows.Forms.TextBox();
            this.serviceTypeTextBox = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.devicePanel = new System.Windows.Forms.Panel();
            this.configTabControl = new System.Windows.Forms.TabControl();
            this.descriptionTabPage = new System.Windows.Forms.TabPage();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label19 = new System.Windows.Forms.Label();
            this.label15 = new System.Windows.Forms.Label();
            this.clearIconButton = new System.Windows.Forms.Button();
            this.setSmallIconButton = new System.Windows.Forms.Button();
            this.deviceIconImageLG = new System.Windows.Forms.PictureBox();
            this.setLargeIconButton = new System.Windows.Forms.Button();
            this.deviceIconImageSM = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.manufacturerTextBox = new System.Windows.Forms.TextBox();
            this.manufacturerUriTextBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.modelDescriptionTextBox = new System.Windows.Forms.TextBox();
            this.rootDeviceTypeTextBox = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.modelNameTextBox = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.modelNumberTextBox = new System.Windows.Forms.TextBox();
            this.versionTextBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.productCodeTextBox = new System.Windows.Forms.TextBox();
            this.configurationTabPage = new System.Windows.Forms.TabPage();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.FieldGrid = new System.Windows.Forms.DataGrid();
            this.dataSet1 = new System.Data.DataSet();
            this.dataTable1 = new System.Data.DataTable();
            this.dataColumn1 = new System.Data.DataColumn();
            this.dataColumn2 = new System.Data.DataColumn();
            this.dataColumn3 = new System.Data.DataColumn();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.label22 = new System.Windows.Forms.Label();
            this.ssdpCycleTextBox = new System.Windows.Forms.TextBox();
            this.webPortTextBox = new System.Windows.Forms.TextBox();
            this.label18 = new System.Windows.Forms.Label();
            this.maxSubscriptionTimeoutTextBox = new System.Windows.Forms.TextBox();
            this.label20 = new System.Windows.Forms.Label();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.devicePresentationPage = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label14 = new System.Windows.Forms.Label();
            this.deviceCodePrefix = new System.Windows.Forms.TextBox();
            this.label17 = new System.Windows.Forms.Label();
            this.deviceType = new System.Windows.Forms.ComboBox();
            this.emptyPanel = new System.Windows.Forms.Panel();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.panel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.servicePanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            this.devicePanel.SuspendLayout();
            this.configTabControl.SuspendLayout();
            this.descriptionTabPage.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.deviceIconImageLG)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.deviceIconImageSM)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.configurationTabPage.SuspendLayout();
            this.groupBox6.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.FieldGrid)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataSet1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataTable1)).BeginInit();
            this.groupBox5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem11,
            this.menuItem2});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.newMenuItem,
            this.openMenuItem,
            this.networkOpenMenuItem,
            this.appendMenuItem,
            this.saveMenuItem,
            this.saveAsMenuItem,
            this.menuItem5,
            this.exportStacksMenuItem,
            this.menuItem20,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            this.menuItem1.Select += new System.EventHandler(this.FileMenu_Click);
            this.menuItem1.Click += new System.EventHandler(this.FileMenu_Click);
            // 
            // newMenuItem
            // 
            this.newMenuItem.Index = 0;
            this.newMenuItem.Text = "&New";
            this.newMenuItem.Click += new System.EventHandler(this.newMenuItem_Click);
            // 
            // openMenuItem
            // 
            this.openMenuItem.Index = 1;
            this.openMenuItem.Text = "&Open...";
            this.openMenuItem.Click += new System.EventHandler(this.openMenuItem_Click);
            // 
            // networkOpenMenuItem
            // 
            this.networkOpenMenuItem.Index = 2;
            this.networkOpenMenuItem.Text = "Open from &Network...";
            this.networkOpenMenuItem.Click += new System.EventHandler(this.networkOpenMenuItem_Click);
            // 
            // appendMenuItem
            // 
            this.appendMenuItem.Index = 3;
            this.appendMenuItem.Text = "Append...";
            this.appendMenuItem.Click += new System.EventHandler(this.appendMenuItem_Click);
            // 
            // saveMenuItem
            // 
            this.saveMenuItem.Enabled = false;
            this.saveMenuItem.Index = 4;
            this.saveMenuItem.Text = "&Save";
            this.saveMenuItem.Click += new System.EventHandler(this.saveMenuItem_Click);
            // 
            // saveAsMenuItem
            // 
            this.saveAsMenuItem.Enabled = false;
            this.saveAsMenuItem.Index = 5;
            this.saveAsMenuItem.Text = "Save &As...";
            this.saveAsMenuItem.Click += new System.EventHandler(this.saveAsMenuItem_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 6;
            this.menuItem5.Text = "-";
            // 
            // exportStacksMenuItem
            // 
            this.exportStacksMenuItem.Index = 7;
            this.exportStacksMenuItem.Text = "Generate Stack...";
            this.exportStacksMenuItem.Click += new System.EventHandler(this.exportStacksMenuItem_Click);
            // 
            // menuItem20
            // 
            this.menuItem20.Index = 8;
            this.menuItem20.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 9;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 1;
            this.menuItem11.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.addServiceMenuItem2,
            this.importServiceMenuItem2,
            this.exportServiceMenuItem,
            this.removeServiceMenuItem2,
            this.menuItem15,
            this.addDeviceMenuItem2,
            this.removeDeviceMenuItem2});
            this.menuItem11.Text = "&Edit";
            // 
            // addServiceMenuItem2
            // 
            this.addServiceMenuItem2.Enabled = false;
            this.addServiceMenuItem2.Index = 0;
            this.addServiceMenuItem2.Text = "&Add Service From File...";
            this.addServiceMenuItem2.Click += new System.EventHandler(this.addServiceMenuItem_Click);
            // 
            // importServiceMenuItem2
            // 
            this.importServiceMenuItem2.Enabled = false;
            this.importServiceMenuItem2.Index = 1;
            this.importServiceMenuItem2.Text = "Add Service From &Network...";
            this.importServiceMenuItem2.Click += new System.EventHandler(this.importServiceMenuItem_Click);
            // 
            // exportServiceMenuItem
            // 
            this.exportServiceMenuItem.Enabled = false;
            this.exportServiceMenuItem.Index = 2;
            this.exportServiceMenuItem.Text = "Export Service Description...";
            this.exportServiceMenuItem.Click += new System.EventHandler(this.exportServiceMenuItem_Click);
            // 
            // removeServiceMenuItem2
            // 
            this.removeServiceMenuItem2.Enabled = false;
            this.removeServiceMenuItem2.Index = 3;
            this.removeServiceMenuItem2.Text = "&Remove Service";
            this.removeServiceMenuItem2.Click += new System.EventHandler(this.removeServiceMenuItem_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 4;
            this.menuItem15.Text = "-";
            // 
            // addDeviceMenuItem2
            // 
            this.addDeviceMenuItem2.Index = 5;
            this.addDeviceMenuItem2.Text = "Add &Device";
            this.addDeviceMenuItem2.Click += new System.EventHandler(this.addDeviceMenuItem_Click);
            // 
            // removeDeviceMenuItem2
            // 
            this.removeDeviceMenuItem2.Enabled = false;
            this.removeDeviceMenuItem2.Index = 6;
            this.removeDeviceMenuItem2.Text = "Remove D&evice";
            this.removeDeviceMenuItem2.Click += new System.EventHandler(this.removeDeviceMenuItem_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 2;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem6,
            this.menuItem4,
            this.showDebugInfoMenuItem});
            this.menuItem2.Text = "&Help";
            this.menuItem2.Popup += new System.EventHandler(this.menuItem2_Popup);
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Enabled = false;
            this.helpMenuItem.Index = 0;
            this.helpMenuItem.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem.Text = "&Help Topics";
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.Text = "-";
            // 
            // showDebugInfoMenuItem
            // 
            this.showDebugInfoMenuItem.Index = 3;
            this.showDebugInfoMenuItem.Text = "&Show Debug Information";
            this.showDebugInfoMenuItem.Click += new System.EventHandler(this.showDebugInfoMenuItem_Click);
            // 
            // mainStatusBar
            // 
            this.mainStatusBar.Location = new System.Drawing.Point(0, 581);
            this.mainStatusBar.Name = "mainStatusBar";
            this.mainStatusBar.Size = new System.Drawing.Size(948, 20);
            this.mainStatusBar.TabIndex = 0;
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
            // openFileDialog
            // 
            this.openFileDialog.DefaultExt = "upnpsg";
            this.openFileDialog.Filter = "Stack Generation File (*.upnpsg)|*.upnpsg";
            this.openFileDialog.Title = "Open Stack Generation Settings";
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.DefaultExt = "upnpsg";
            this.saveFileDialog.FileName = "StackGenSettings.upnpsg";
            this.saveFileDialog.Filter = "Stack Generation File (*.upnpsg)|*.upnpsg";
            this.saveFileDialog.Title = "Save Stack Generation Settings";
            // 
            // friendlyNameTextBox
            // 
            this.friendlyNameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.friendlyNameTextBox.Location = new System.Drawing.Point(135, 23);
            this.friendlyNameTextBox.Name = "friendlyNameTextBox";
            this.friendlyNameTextBox.Size = new System.Drawing.Size(412, 20);
            this.friendlyNameTextBox.TabIndex = 68;
            this.friendlyNameTextBox.Text = "Sample Device";
            this.mainToolTip.SetToolTip(this.friendlyNameTextBox, "Service Friendly Name used to help generate the code. Must be unique thruout the " +
                    "entire device tree and contain only alphanumeric characters");
            this.friendlyNameTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // treeView
            // 
            this.treeView.AllowDrop = true;
            this.treeView.ContextMenu = this.treeContextMenu;
            this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeView.ImageIndex = 0;
            this.treeView.ImageList = this.treeImageList;
            this.treeView.Location = new System.Drawing.Point(0, 0);
            this.treeView.Name = "treeView";
            this.treeView.SelectedImageIndex = 0;
            this.treeView.Size = new System.Drawing.Size(319, 535);
            this.treeView.TabIndex = 2;
            this.treeView.DragDrop += new System.Windows.Forms.DragEventHandler(this.MainForm_DragDrop);
            this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_AfterSelect);
            this.treeView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.treeView_MouseDown);
            this.treeView.DragEnter += new System.Windows.Forms.DragEventHandler(this.MainForm_DragEnter);
            // 
            // treeContextMenu
            // 
            this.treeContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.addServiceMenuItem,
            this.importServiceMenuItem,
            this.exportServiceMenuItem2,
            this.editServiceMenuItem,
            this.removeServiceMenuItem,
            this.menuItem10,
            this.addDeviceMenuItem,
            this.removeDeviceMenuItem});
            // 
            // addServiceMenuItem
            // 
            this.addServiceMenuItem.Index = 0;
            this.addServiceMenuItem.Text = "&Add Service From File...";
            this.addServiceMenuItem.Click += new System.EventHandler(this.addServiceMenuItem_Click);
            // 
            // importServiceMenuItem
            // 
            this.importServiceMenuItem.Index = 1;
            this.importServiceMenuItem.Text = "Add Service From &Network...";
            this.importServiceMenuItem.Click += new System.EventHandler(this.importServiceMenuItem_Click);
            // 
            // exportServiceMenuItem2
            // 
            this.exportServiceMenuItem2.Index = 2;
            this.exportServiceMenuItem2.Text = "Export Service Description...";
            this.exportServiceMenuItem2.Click += new System.EventHandler(this.exportServiceMenuItem_Click);
            // 
            // editServiceMenuItem
            // 
            this.editServiceMenuItem.Index = 3;
            this.editServiceMenuItem.Text = "Edit Service Description";
            this.editServiceMenuItem.Click += new System.EventHandler(this.editServiceMenuItem_Click);
            // 
            // removeServiceMenuItem
            // 
            this.removeServiceMenuItem.Index = 4;
            this.removeServiceMenuItem.Text = "&Remove Service";
            this.removeServiceMenuItem.Click += new System.EventHandler(this.removeServiceMenuItem_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 5;
            this.menuItem10.Text = "-";
            // 
            // addDeviceMenuItem
            // 
            this.addDeviceMenuItem.Index = 6;
            this.addDeviceMenuItem.Text = "Add &Device";
            this.addDeviceMenuItem.Click += new System.EventHandler(this.addDeviceMenuItem_Click);
            // 
            // removeDeviceMenuItem
            // 
            this.removeDeviceMenuItem.Index = 7;
            this.removeDeviceMenuItem.Text = "Remove D&evice";
            this.removeDeviceMenuItem.Click += new System.EventHandler(this.removeDeviceMenuItem_Click);
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.mainStatusLabel2);
            this.panel2.Controls.Add(this.mainStatusLabel1);
            this.panel2.Controls.Add(this.pictureBox1);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel2.Location = new System.Drawing.Point(0, 0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(948, 46);
            this.panel2.TabIndex = 9;
            // 
            // mainStatusLabel2
            // 
            this.mainStatusLabel2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mainStatusLabel2.Location = new System.Drawing.Point(40, 24);
            this.mainStatusLabel2.Name = "mainStatusLabel2";
            this.mainStatusLabel2.Size = new System.Drawing.Size(900, 18);
            this.mainStatusLabel2.TabIndex = 2;
            // 
            // mainStatusLabel1
            // 
            this.mainStatusLabel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mainStatusLabel1.Location = new System.Drawing.Point(40, 7);
            this.mainStatusLabel1.Name = "mainStatusLabel1";
            this.mainStatusLabel1.Size = new System.Drawing.Size(900, 19);
            this.mainStatusLabel1.TabIndex = 1;
            this.mainStatusLabel1.Text = "Device Builder";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(2, 6);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(32, 32);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // splitter1
            // 
            this.splitter1.Dock = System.Windows.Forms.DockStyle.Right;
            this.splitter1.Location = new System.Drawing.Point(944, 46);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(4, 535);
            this.splitter1.TabIndex = 44;
            this.splitter1.TabStop = false;
            // 
            // serviceContextMenu
            // 
            this.serviceContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.actionNormalRespMenuItem,
            this.actionFragRespMenuItem,
            this.menuItem3,
            this.actionAutoEscapeMenuItem,
            this.actionManualEscapeMenuItem});
            this.serviceContextMenu.Popup += new System.EventHandler(this.serviceContextMenu_Popup);
            // 
            // actionNormalRespMenuItem
            // 
            this.actionNormalRespMenuItem.Index = 0;
            this.actionNormalRespMenuItem.Text = "&Normal Response";
            this.actionNormalRespMenuItem.Click += new System.EventHandler(this.actionNormalRespMenuItem_Click);
            // 
            // actionFragRespMenuItem
            // 
            this.actionFragRespMenuItem.Index = 1;
            this.actionFragRespMenuItem.Text = "&Fragmented Response";
            this.actionFragRespMenuItem.Click += new System.EventHandler(this.actionFragRespMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 2;
            this.menuItem3.Text = "-";
            // 
            // actionAutoEscapeMenuItem
            // 
            this.actionAutoEscapeMenuItem.Index = 3;
            this.actionAutoEscapeMenuItem.Text = "Auto String Escape";
            this.actionAutoEscapeMenuItem.Click += new System.EventHandler(this.actionAutoEscapeMenuItem_Click);
            // 
            // actionManualEscapeMenuItem
            // 
            this.actionManualEscapeMenuItem.Index = 4;
            this.actionManualEscapeMenuItem.Text = "Manual String Escape";
            this.actionManualEscapeMenuItem.Click += new System.EventHandler(this.actionManualEscapeMenuItem_Click);
            // 
            // exportSaveFileDialog
            // 
            this.exportSaveFileDialog.DefaultExt = "xml";
            this.exportSaveFileDialog.FileName = "service.xml";
            this.exportSaveFileDialog.Filter = "XML files|*.xml|All files|*.*";
            this.exportSaveFileDialog.Title = "Export Service Description";
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Service Element";
            this.columnHeader1.Width = 230;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Mode";
            this.columnHeader2.Width = 90;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "DvEscaping";
            this.columnHeader3.Width = 75;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "CpEscaping";
            this.columnHeader4.Width = 75;
            // 
            // servicePanel
            // 
            this.servicePanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.servicePanel.Controls.Add(this.serviceListView);
            this.servicePanel.Controls.Add(this.label16);
            this.servicePanel.Controls.Add(this.pictureBox3);
            this.servicePanel.Controls.Add(this.label8);
            this.servicePanel.Controls.Add(this.serviceNameTextBox);
            this.servicePanel.Controls.Add(this.serviceIdTextBox);
            this.servicePanel.Controls.Add(this.serviceTypeTextBox);
            this.servicePanel.Controls.Add(this.label10);
            this.servicePanel.Controls.Add(this.label12);
            this.servicePanel.Controls.Add(this.label13);
            this.servicePanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.servicePanel.Location = new System.Drawing.Point(3, 3);
            this.servicePanel.Name = "servicePanel";
            this.servicePanel.Size = new System.Drawing.Size(607, 503);
            this.servicePanel.TabIndex = 48;
            // 
            // serviceListView
            // 
            this.serviceListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serviceListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5,
            this.columnHeader6,
            this.columnHeader7});
            this.serviceListView.ContextMenu = this.serviceContextMenu;
            this.serviceListView.FullRowSelect = true;
            this.serviceListView.Location = new System.Drawing.Point(9, 150);
            this.serviceListView.MultiSelect = false;
            this.serviceListView.Name = "serviceListView";
            this.serviceListView.Size = new System.Drawing.Size(589, 346);
            this.serviceListView.SmallImageList = this.treeImageList;
            this.serviceListView.TabIndex = 68;
            this.serviceListView.UseCompatibleStateImageBehavior = false;
            this.serviceListView.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Service Element";
            this.columnHeader5.Width = 230;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Mode";
            this.columnHeader6.Width = 90;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Escaping";
            this.columnHeader7.Width = 75;
            // 
            // label16
            // 
            this.label16.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label16.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label16.Location = new System.Drawing.Point(47, 16);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(546, 19);
            this.label16.TabIndex = 63;
            this.label16.Text = "Service Information";
            // 
            // pictureBox3
            // 
            this.pictureBox3.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox3.Image")));
            this.pictureBox3.Location = new System.Drawing.Point(9, 9);
            this.pictureBox3.Name = "pictureBox3";
            this.pictureBox3.Size = new System.Drawing.Size(28, 28);
            this.pictureBox3.TabIndex = 62;
            this.pictureBox3.TabStop = false;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(9, 131);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(243, 14);
            this.label8.TabIndex = 61;
            this.label8.Text = "Service Description Document";
            // 
            // serviceNameTextBox
            // 
            this.serviceNameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serviceNameTextBox.Location = new System.Drawing.Point(103, 46);
            this.serviceNameTextBox.Name = "serviceNameTextBox";
            this.serviceNameTextBox.Size = new System.Drawing.Size(490, 20);
            this.serviceNameTextBox.TabIndex = 53;
            this.serviceNameTextBox.Text = "SampleService";
            this.serviceNameTextBox.TextChanged += new System.EventHandler(this.serviceNameTextBox_TextChanged);
            // 
            // serviceIdTextBox
            // 
            this.serviceIdTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serviceIdTextBox.Location = new System.Drawing.Point(103, 103);
            this.serviceIdTextBox.Name = "serviceIdTextBox";
            this.serviceIdTextBox.Size = new System.Drawing.Size(490, 20);
            this.serviceIdTextBox.TabIndex = 55;
            this.serviceIdTextBox.Text = "SampleServiceIdentifier";
            this.serviceIdTextBox.TextChanged += new System.EventHandler(this.serviceIdTextBox_TextChanged);
            // 
            // serviceTypeTextBox
            // 
            this.serviceTypeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.serviceTypeTextBox.Location = new System.Drawing.Point(103, 75);
            this.serviceTypeTextBox.Name = "serviceTypeTextBox";
            this.serviceTypeTextBox.Size = new System.Drawing.Size(490, 20);
            this.serviceTypeTextBox.TabIndex = 54;
            this.serviceTypeTextBox.Text = "urn:schemas-upnp-org:service:sample:1";
            this.serviceTypeTextBox.TextChanged += new System.EventHandler(this.serviceTypeTextBox_TextChanged);
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(9, 46);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(117, 17);
            this.label10.TabIndex = 60;
            this.label10.Text = "Service Name";
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(9, 103);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(117, 16);
            this.label12.TabIndex = 59;
            this.label12.Text = "Service ID";
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(9, 75);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(117, 16);
            this.label13.TabIndex = 58;
            this.label13.Text = "Service Type";
            // 
            // devicePanel
            // 
            this.devicePanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.devicePanel.Controls.Add(this.configTabControl);
            this.devicePanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.devicePanel.Location = new System.Drawing.Point(3, 3);
            this.devicePanel.Name = "devicePanel";
            this.devicePanel.Size = new System.Drawing.Size(607, 503);
            this.devicePanel.TabIndex = 64;
            // 
            // configTabControl
            // 
            this.configTabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.configTabControl.Controls.Add(this.descriptionTabPage);
            this.configTabControl.Controls.Add(this.configurationTabPage);
            this.configTabControl.Location = new System.Drawing.Point(9, 9);
            this.configTabControl.Name = "configTabControl";
            this.configTabControl.SelectedIndex = 0;
            this.configTabControl.Size = new System.Drawing.Size(588, 479);
            this.configTabControl.TabIndex = 81;
            this.configTabControl.SelectedIndexChanged += new System.EventHandler(this.OnStackTypeChanged);
            // 
            // descriptionTabPage
            // 
            this.descriptionTabPage.Controls.Add(this.groupBox2);
            this.descriptionTabPage.Controls.Add(this.groupBox1);
            this.descriptionTabPage.Location = new System.Drawing.Point(4, 22);
            this.descriptionTabPage.Name = "descriptionTabPage";
            this.descriptionTabPage.Size = new System.Drawing.Size(580, 453);
            this.descriptionTabPage.TabIndex = 4;
            this.descriptionTabPage.Text = "Description";
            this.descriptionTabPage.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.label19);
            this.groupBox2.Controls.Add(this.label15);
            this.groupBox2.Controls.Add(this.clearIconButton);
            this.groupBox2.Controls.Add(this.setSmallIconButton);
            this.groupBox2.Controls.Add(this.deviceIconImageLG);
            this.groupBox2.Controls.Add(this.setLargeIconButton);
            this.groupBox2.Controls.Add(this.deviceIconImageSM);
            this.groupBox2.Location = new System.Drawing.Point(8, 301);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(562, 149);
            this.groupBox2.TabIndex = 81;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Device Icon";
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(150, 30);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(79, 13);
            this.label19.TabIndex = 8;
            this.label19.Text = "120x120 image";
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(150, 59);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(67, 13);
            this.label15.TabIndex = 7;
            this.label15.Text = "48x48 image";
            // 
            // clearIconButton
            // 
            this.clearIconButton.Location = new System.Drawing.Point(16, 83);
            this.clearIconButton.Name = "clearIconButton";
            this.clearIconButton.Size = new System.Drawing.Size(128, 23);
            this.clearIconButton.TabIndex = 6;
            this.clearIconButton.Text = "Clear Icons";
            this.clearIconButton.Click += new System.EventHandler(this.clearIconButton_Click);
            // 
            // setSmallIconButton
            // 
            this.setSmallIconButton.Location = new System.Drawing.Point(16, 54);
            this.setSmallIconButton.Name = "setSmallIconButton";
            this.setSmallIconButton.Size = new System.Drawing.Size(128, 23);
            this.setSmallIconButton.TabIndex = 5;
            this.setSmallIconButton.Text = "Set Small Icon...";
            this.setSmallIconButton.Click += new System.EventHandler(this.setSmallIconButton_Click);
            // 
            // deviceIconImageLG
            // 
            this.deviceIconImageLG.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.deviceIconImageLG.BackgroundImage = global::UPnPStackBuilder.Properties.Resources.QuestionIcon120b;
            this.deviceIconImageLG.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.deviceIconImageLG.Location = new System.Drawing.Point(427, 19);
            this.deviceIconImageLG.Name = "deviceIconImageLG";
            this.deviceIconImageLG.Size = new System.Drawing.Size(120, 120);
            this.deviceIconImageLG.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.deviceIconImageLG.TabIndex = 4;
            this.deviceIconImageLG.TabStop = false;
            this.deviceIconImageLG.DoubleClick += new System.EventHandler(this.setLargeIconButton_Click);
            // 
            // setLargeIconButton
            // 
            this.setLargeIconButton.Location = new System.Drawing.Point(16, 25);
            this.setLargeIconButton.Name = "setLargeIconButton";
            this.setLargeIconButton.Size = new System.Drawing.Size(128, 23);
            this.setLargeIconButton.TabIndex = 1;
            this.setLargeIconButton.Text = "Set Large Icon...";
            this.setLargeIconButton.Click += new System.EventHandler(this.setLargeIconButton_Click);
            // 
            // deviceIconImageSM
            // 
            this.deviceIconImageSM.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.deviceIconImageSM.BackgroundImage = global::UPnPStackBuilder.Properties.Resources.QuestionIcon48b;
            this.deviceIconImageSM.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.deviceIconImageSM.Location = new System.Drawing.Point(373, 91);
            this.deviceIconImageSM.Name = "deviceIconImageSM";
            this.deviceIconImageSM.Size = new System.Drawing.Size(48, 48);
            this.deviceIconImageSM.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.deviceIconImageSM.TabIndex = 0;
            this.deviceIconImageSM.TabStop = false;
            this.deviceIconImageSM.DoubleClick += new System.EventHandler(this.setSmallIconButton_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.manufacturerTextBox);
            this.groupBox1.Controls.Add(this.manufacturerUriTextBox);
            this.groupBox1.Controls.Add(this.friendlyNameTextBox);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.modelDescriptionTextBox);
            this.groupBox1.Controls.Add(this.rootDeviceTypeTextBox);
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.modelNameTextBox);
            this.groupBox1.Controls.Add(this.label11);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.modelNumberTextBox);
            this.groupBox1.Controls.Add(this.versionTextBox);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.productCodeTextBox);
            this.groupBox1.Location = new System.Drawing.Point(8, 8);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(562, 287);
            this.groupBox1.TabIndex = 80;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Device Information";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(13, 28);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(117, 19);
            this.label1.TabIndex = 60;
            this.label1.Text = "Friendly Name";
            // 
            // manufacturerTextBox
            // 
            this.manufacturerTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.manufacturerTextBox.Location = new System.Drawing.Point(135, 80);
            this.manufacturerTextBox.Name = "manufacturerTextBox";
            this.manufacturerTextBox.Size = new System.Drawing.Size(412, 20);
            this.manufacturerTextBox.TabIndex = 69;
            this.manufacturerTextBox.Text = "Sample Corporation";
            this.manufacturerTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // manufacturerUriTextBox
            // 
            this.manufacturerUriTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.manufacturerUriTextBox.Location = new System.Drawing.Point(135, 108);
            this.manufacturerUriTextBox.Name = "manufacturerUriTextBox";
            this.manufacturerUriTextBox.Size = new System.Drawing.Size(412, 20);
            this.manufacturerUriTextBox.TabIndex = 70;
            this.manufacturerUriTextBox.Text = "http://opentools.homeip.net";
            this.manufacturerUriTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(13, 85);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(117, 18);
            this.label2.TabIndex = 61;
            this.label2.Text = "Manufacturer";
            // 
            // modelDescriptionTextBox
            // 
            this.modelDescriptionTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.modelDescriptionTextBox.Location = new System.Drawing.Point(135, 136);
            this.modelDescriptionTextBox.Name = "modelDescriptionTextBox";
            this.modelDescriptionTextBox.Size = new System.Drawing.Size(412, 20);
            this.modelDescriptionTextBox.TabIndex = 71;
            this.modelDescriptionTextBox.Text = "Sample UPnP Device Using Auto-Generated UPnP Stack";
            this.modelDescriptionTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // rootDeviceTypeTextBox
            // 
            this.rootDeviceTypeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.rootDeviceTypeTextBox.Location = new System.Drawing.Point(135, 52);
            this.rootDeviceTypeTextBox.Name = "rootDeviceTypeTextBox";
            this.rootDeviceTypeTextBox.Size = new System.Drawing.Size(412, 20);
            this.rootDeviceTypeTextBox.TabIndex = 77;
            this.rootDeviceTypeTextBox.Text = "urn:schemas-upnp-org:device:Sample:1";
            this.rootDeviceTypeTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(13, 252);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(117, 19);
            this.label9.TabIndex = 67;
            this.label9.Text = "Version";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(13, 112);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(117, 19);
            this.label3.TabIndex = 62;
            this.label3.Text = "Manufacturer URI";
            // 
            // modelNameTextBox
            // 
            this.modelNameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.modelNameTextBox.Location = new System.Drawing.Point(135, 163);
            this.modelNameTextBox.Name = "modelNameTextBox";
            this.modelNameTextBox.Size = new System.Drawing.Size(412, 20);
            this.modelNameTextBox.TabIndex = 72;
            this.modelNameTextBox.Text = "Sample Auto-Generated Device";
            this.modelNameTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(13, 57);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(117, 18);
            this.label11.TabIndex = 76;
            this.label11.Text = "Root Device Type";
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(13, 225);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(117, 18);
            this.label7.TabIndex = 66;
            this.label7.Text = "Product Code";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(13, 141);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(117, 18);
            this.label4.TabIndex = 63;
            this.label4.Text = "Model Description";
            // 
            // modelNumberTextBox
            // 
            this.modelNumberTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.modelNumberTextBox.Location = new System.Drawing.Point(135, 192);
            this.modelNumberTextBox.Name = "modelNumberTextBox";
            this.modelNumberTextBox.Size = new System.Drawing.Size(412, 20);
            this.modelNumberTextBox.TabIndex = 73;
            this.modelNumberTextBox.Text = "1";
            this.modelNumberTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // versionTextBox
            // 
            this.versionTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.versionTextBox.Location = new System.Drawing.Point(135, 248);
            this.versionTextBox.Name = "versionTextBox";
            this.versionTextBox.Size = new System.Drawing.Size(412, 20);
            this.versionTextBox.TabIndex = 75;
            this.versionTextBox.Text = "1.0";
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(13, 197);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(117, 19);
            this.label6.TabIndex = 65;
            this.label6.Text = "Model Number";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(13, 168);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(117, 19);
            this.label5.TabIndex = 64;
            this.label5.Text = "Model Name";
            // 
            // productCodeTextBox
            // 
            this.productCodeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.productCodeTextBox.Location = new System.Drawing.Point(135, 220);
            this.productCodeTextBox.Name = "productCodeTextBox";
            this.productCodeTextBox.Size = new System.Drawing.Size(412, 20);
            this.productCodeTextBox.TabIndex = 74;
            this.productCodeTextBox.Text = "Sample-X1";
            this.productCodeTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            // 
            // configurationTabPage
            // 
            this.configurationTabPage.Controls.Add(this.groupBox6);
            this.configurationTabPage.Controls.Add(this.groupBox5);
            this.configurationTabPage.Controls.Add(this.groupBox4);
            this.configurationTabPage.Controls.Add(this.groupBox3);
            this.configurationTabPage.Location = new System.Drawing.Point(4, 22);
            this.configurationTabPage.Name = "configurationTabPage";
            this.configurationTabPage.Size = new System.Drawing.Size(580, 453);
            this.configurationTabPage.TabIndex = 0;
            this.configurationTabPage.Text = "Configuration";
            this.configurationTabPage.UseVisualStyleBackColor = true;
            // 
            // groupBox6
            // 
            this.groupBox6.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox6.Controls.Add(this.FieldGrid);
            this.groupBox6.Location = new System.Drawing.Point(8, 277);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(562, 156);
            this.groupBox6.TabIndex = 85;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Custom Fields";
            // 
            // FieldGrid
            // 
            this.FieldGrid.CaptionVisible = false;
            this.FieldGrid.DataMember = "Table1";
            this.FieldGrid.DataSource = this.dataSet1;
            this.FieldGrid.Dock = System.Windows.Forms.DockStyle.Fill;
            this.FieldGrid.HeaderForeColor = System.Drawing.SystemColors.ControlText;
            this.FieldGrid.Location = new System.Drawing.Point(3, 16);
            this.FieldGrid.Name = "FieldGrid";
            this.FieldGrid.PreferredColumnWidth = 180;
            this.FieldGrid.RowHeaderWidth = 15;
            this.FieldGrid.Size = new System.Drawing.Size(556, 137);
            this.FieldGrid.TabIndex = 9;
            this.FieldGrid.Leave += new System.EventHandler(this.OnCustomField_Leave);
            // 
            // dataSet1
            // 
            this.dataSet1.DataSetName = "NewDataSet";
            this.dataSet1.Locale = new System.Globalization.CultureInfo("en-US");
            this.dataSet1.Tables.AddRange(new System.Data.DataTable[] {
            this.dataTable1});
            // 
            // dataTable1
            // 
            this.dataTable1.Columns.AddRange(new System.Data.DataColumn[] {
            this.dataColumn1,
            this.dataColumn2,
            this.dataColumn3});
            this.dataTable1.Constraints.AddRange(new System.Data.Constraint[] {
            new System.Data.UniqueConstraint("Constraint1", new string[] {
                        "FieldName"}, false)});
            this.dataTable1.TableName = "Table1";
            // 
            // dataColumn1
            // 
            this.dataColumn1.AllowDBNull = false;
            this.dataColumn1.Caption = "Name";
            this.dataColumn1.ColumnName = "FieldName";
            this.dataColumn1.DefaultValue = "";
            // 
            // dataColumn2
            // 
            this.dataColumn2.AllowDBNull = false;
            this.dataColumn2.Caption = "Value";
            this.dataColumn2.ColumnName = "FieldValue";
            this.dataColumn2.DefaultValue = "";
            // 
            // dataColumn3
            // 
            this.dataColumn3.Caption = "Namespace";
            this.dataColumn3.ColumnName = "FieldNameSpace";
            this.dataColumn3.DefaultValue = "";
            // 
            // groupBox5
            // 
            this.groupBox5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox5.Controls.Add(this.label22);
            this.groupBox5.Controls.Add(this.ssdpCycleTextBox);
            this.groupBox5.Controls.Add(this.webPortTextBox);
            this.groupBox5.Controls.Add(this.label18);
            this.groupBox5.Controls.Add(this.maxSubscriptionTimeoutTextBox);
            this.groupBox5.Controls.Add(this.label20);
            this.groupBox5.Location = new System.Drawing.Point(8, 168);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(562, 103);
            this.groupBox5.TabIndex = 84;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Advanced Options";
            // 
            // label22
            // 
            this.label22.Location = new System.Drawing.Point(6, 70);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(192, 16);
            this.label22.TabIndex = 44;
            this.label22.Text = "Maximum Event subscription timeout";
            // 
            // ssdpCycleTextBox
            // 
            this.ssdpCycleTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ssdpCycleTextBox.Location = new System.Drawing.Point(488, 19);
            this.ssdpCycleTextBox.Name = "ssdpCycleTextBox";
            this.ssdpCycleTextBox.Size = new System.Drawing.Size(64, 20);
            this.ssdpCycleTextBox.TabIndex = 39;
            this.ssdpCycleTextBox.Text = "1800";
            this.ssdpCycleTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.ssdpCycleTextBox.Leave += new System.EventHandler(this.OnAdvancedOptionsChanged);
            // 
            // webPortTextBox
            // 
            this.webPortTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.webPortTextBox.Location = new System.Drawing.Point(488, 43);
            this.webPortTextBox.Name = "webPortTextBox";
            this.webPortTextBox.Size = new System.Drawing.Size(64, 20);
            this.webPortTextBox.TabIndex = 41;
            this.webPortTextBox.Text = "0";
            this.webPortTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.webPortTextBox.Leave += new System.EventHandler(this.OnAdvancedOptionsChanged);
            // 
            // label18
            // 
            this.label18.Location = new System.Drawing.Point(6, 46);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(208, 19);
            this.label18.TabIndex = 42;
            this.label18.Text = "Device Web Server Port (0 for any)";
            // 
            // maxSubscriptionTimeoutTextBox
            // 
            this.maxSubscriptionTimeoutTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.maxSubscriptionTimeoutTextBox.Location = new System.Drawing.Point(488, 67);
            this.maxSubscriptionTimeoutTextBox.MaxLength = 4;
            this.maxSubscriptionTimeoutTextBox.Name = "maxSubscriptionTimeoutTextBox";
            this.maxSubscriptionTimeoutTextBox.Size = new System.Drawing.Size(64, 20);
            this.maxSubscriptionTimeoutTextBox.TabIndex = 43;
            this.maxSubscriptionTimeoutTextBox.Text = "7200";
            this.maxSubscriptionTimeoutTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.maxSubscriptionTimeoutTextBox.Leave += new System.EventHandler(this.OnAdvancedOptionsChanged);
            // 
            // label20
            // 
            this.label20.Location = new System.Drawing.Point(6, 22);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(224, 19);
            this.label20.TabIndex = 40;
            this.label20.Text = "Device SSDP notify cycle time (in seconds)";
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.devicePresentationPage);
            this.groupBox4.Location = new System.Drawing.Point(8, 108);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(562, 54);
            this.groupBox4.TabIndex = 83;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Presentation Page";
            // 
            // devicePresentationPage
            // 
            this.devicePresentationPage.Location = new System.Drawing.Point(9, 19);
            this.devicePresentationPage.Name = "devicePresentationPage";
            this.devicePresentationPage.Size = new System.Drawing.Size(320, 22);
            this.devicePresentationPage.TabIndex = 33;
            this.devicePresentationPage.Text = "Device Advertises a Presentation Web Page (/web is root)";
            this.devicePresentationPage.CheckStateChanged += new System.EventHandler(this.OnPresentationChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Controls.Add(this.label14);
            this.groupBox3.Controls.Add(this.deviceCodePrefix);
            this.groupBox3.Controls.Add(this.label17);
            this.groupBox3.Controls.Add(this.deviceType);
            this.groupBox3.Location = new System.Drawing.Point(8, 8);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(562, 94);
            this.groupBox3.TabIndex = 82;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Code Generation";
            // 
            // label14
            // 
            this.label14.Location = new System.Drawing.Point(6, 58);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(64, 16);
            this.label14.TabIndex = 30;
            this.label14.Text = "Code Prefix";
            // 
            // deviceCodePrefix
            // 
            this.deviceCodePrefix.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.deviceCodePrefix.Location = new System.Drawing.Point(236, 55);
            this.deviceCodePrefix.Name = "deviceCodePrefix";
            this.deviceCodePrefix.Size = new System.Drawing.Size(316, 20);
            this.deviceCodePrefix.TabIndex = 29;
            this.deviceCodePrefix.Text = "UPnP";
            this.deviceCodePrefix.TextChanged += new System.EventHandler(this.OnCodePrefixChanged);
            // 
            // label17
            // 
            this.label17.Location = new System.Drawing.Point(6, 31);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(88, 16);
            this.label17.TabIndex = 32;
            this.label17.Text = "Generation type";
            // 
            // deviceType
            // 
            this.deviceType.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.deviceType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.deviceType.ItemHeight = 13;
            this.deviceType.Items.AddRange(new object[] {
            "Device",
            "Control Point"});
            this.deviceType.Location = new System.Drawing.Point(236, 28);
            this.deviceType.Name = "deviceType";
            this.deviceType.Size = new System.Drawing.Size(316, 21);
            this.deviceType.TabIndex = 31;
            this.deviceType.SelectedIndexChanged += new System.EventHandler(this.OnStackTypeChanged);
            // 
            // emptyPanel
            // 
            this.emptyPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.emptyPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.emptyPanel.Location = new System.Drawing.Point(0, 0);
            this.emptyPanel.Name = "emptyPanel";
            this.emptyPanel.Size = new System.Drawing.Size(613, 509);
            this.emptyPanel.TabIndex = 81;
            this.emptyPanel.Visible = false;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.Location = new System.Drawing.Point(0, 46);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.treeView);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tabControl1);
            this.splitContainer1.Size = new System.Drawing.Size(944, 535);
            this.splitContainer1.SplitterDistance = 319;
            this.splitContainer1.TabIndex = 49;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(621, 535);
            this.tabControl1.TabIndex = 3;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.devicePanel);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(613, 509);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "tabPage1";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.servicePanel);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(613, 509);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "tabPage2";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.emptyPanel);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(613, 509);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "tabPage3";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 1;
            this.menuItem6.Text = "&Check for updates";
            this.menuItem6.Click += new System.EventHandler(this.menuItem6_Click);
            // 
            // MainForm
            // 
            this.AllowDrop = true;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(948, 601);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.mainStatusBar);
            this.Controls.Add(this.panel2);
            this.DoubleBuffered = true;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "Device Builder";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.MainForm_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.MainForm_DragEnter);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.servicePanel.ResumeLayout(false);
            this.servicePanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            this.devicePanel.ResumeLayout(false);
            this.configTabControl.ResumeLayout(false);
            this.descriptionTabPage.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.deviceIconImageLG)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.deviceIconImageSM)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.configurationTabPage.ResumeLayout(false);
            this.groupBox6.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.FieldGrid)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataSet1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataTable1)).EndInit();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Application.Run(new MainForm(args));
        }

        private void exitMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void showDebugInfoMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void SaveCustomFieldsIntoDevice(UPnPDevice device)
        {
            ((ServiceGenerator.Configuration)device.User).CustomFieldTable.Clear();

            foreach (DataRow dr in dataSet1.Tables[0].Rows)
            {
                object[] d = dr.ItemArray;

                if (!((ServiceGenerator.Configuration)device.User).CustomFieldTable.ContainsKey(d[2]))
                {
                    ((ServiceGenerator.Configuration)device.User).CustomFieldTable[d[2]] = new Hashtable();
                }

                ((Hashtable)((ServiceGenerator.Configuration)device.User).CustomFieldTable[d[2]])[d[0]] = d[1];
            }
        }

        private void UpdateCustomFields(UPnPDevice device)
        {
            ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)device.User;
            dataSet1.Tables[0].Rows.Clear();

            IDictionaryEnumerator NamespaceEnumerator = Conf.CustomFieldTable.GetEnumerator();
            while (NamespaceEnumerator.MoveNext())
            {
                IDictionaryEnumerator EntryEnumerator = ((Hashtable)NamespaceEnumerator.Value).GetEnumerator();
                while (EntryEnumerator.MoveNext())
                {
                    dataSet1.Tables[0].Rows.Add(new object[3] { EntryEnumerator.Key, EntryEnumerator.Value, NamespaceEnumerator.Key });
                }
            }
        }

        private void UnHookEventHandlers()
        {
            this.manufacturerTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.manufacturerUriTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.modelDescriptionTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.modelNameTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.modelNumberTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.productCodeTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.rootDeviceTypeTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
            this.friendlyNameTextBox.TextChanged -= new System.EventHandler(this.deviceInformationChanged);
        }
        private void HookupEventHandlers()
        {
            this.manufacturerTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.manufacturerUriTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.modelDescriptionTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.modelNameTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.modelNumberTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.productCodeTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.rootDeviceTypeTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
            this.friendlyNameTextBox.TextChanged += new System.EventHandler(this.deviceInformationChanged);
        }

        private void ViewTab(int i)
        {
            if (splitContainer1.Panel2.Controls.Count == 0) splitContainer1.Panel2.Controls.Add(emptyPanel);
            switch (i)
            {
                case 0:
                    if (splitContainer1.Panel2.Controls[0] != emptyPanel) splitContainer1.Panel2.Controls.Add(emptyPanel);
                    break;
                case 1:
                    if (splitContainer1.Panel2.Controls[0] != devicePanel) splitContainer1.Panel2.Controls.Add(devicePanel);
                    break;
                case 2:
                    if (splitContainer1.Panel2.Controls[0] != servicePanel) splitContainer1.Panel2.Controls.Add(servicePanel);
                    break;
            }
            if (splitContainer1.Panel2.Controls.Count == 2) splitContainer1.Panel2.Controls.RemoveAt(0);
        }

        private void treeView_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
        {
            if (treeView.SelectedNode == null || treeView.SelectedNode.Tag == null)
            {
                //emptyPanel.BringToFront();
                //emptyPanel.Visible = true;
                ViewTab(0);
                selectedItem = null;
                return;
            }

            UnHookEventHandlers();

            selectedItem = treeView.SelectedNode.Tag;
            if (selectedItem.GetType() == typeof(UPnPDevice))
            {
                //emptyPanel.SendToBack();
                ViewTab(1);
                UPnPDevice device = (UPnPDevice)selectedItem;
                friendlyNameTextBox.Text = device.FriendlyName;
                rootDeviceTypeTextBox.Text = device.DeviceURN;
                manufacturerTextBox.Text = device.Manufacturer;
                manufacturerUriTextBox.Text = device.ManufacturerURL;
                modelDescriptionTextBox.Text = device.ModelDescription;
                modelNameTextBox.Text = device.ModelName;
                modelNumberTextBox.Text = device.ModelNumber;
                productCodeTextBox.Text = device.ProductCode;
                versionTextBox.Text = device.Version;

                //devicePanel.Visible = true;

                addServiceMenuItem.Enabled = true;
                importServiceMenuItem.Enabled = true;
                removeServiceMenuItem.Enabled = false;
                exportServiceMenuItem.Enabled = false;
                editServiceMenuItem.Enabled = false;
                addDeviceMenuItem.Enabled = true;
                removeDeviceMenuItem.Enabled = true;

                addServiceMenuItem2.Enabled = true;
                importServiceMenuItem2.Enabled = true;
                removeServiceMenuItem2.Enabled = false;
                exportServiceMenuItem2.Enabled = false;
                addDeviceMenuItem2.Enabled = true;
                removeDeviceMenuItem2.Enabled = true;

                menuItem10.Visible = true;
                addServiceMenuItem.Visible = true;
                importServiceMenuItem.Visible = true;
                removeServiceMenuItem.Visible = false;
                exportServiceMenuItem.Visible = false;
                editServiceMenuItem.Visible = false;
                addDeviceMenuItem.Visible = true;
                removeDeviceMenuItem.Visible = true;


                //deviceConfiguration.Visible = treeView.SelectedNode.Parent==null;
                if (treeView.SelectedNode.Parent == null)
                {
                    ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)device.User;

                    deviceCodePrefix.Text = Conf.Prefix;
                    deviceType.SelectedIndex = (int)Conf.ConfigType;
                    devicePresentationPage.Checked = Conf.AdvertisesPresentationPage;
                    this.ssdpCycleTextBox.Text = Conf.SSDPCycleTime.ToString();
                    this.webPortTextBox.Text = Conf.WebPort.ToString();
                    this.maxSubscriptionTimeoutTextBox.Text = Conf.MaxSubscriptionTimeout.ToString();

                    //
                    // Update the icon, if there is one
                    //
                    this.deviceIconImageSM.Image = Conf.IconImageSM;
                    this.deviceIconImageLG.Image = Conf.IconImageLG;
                }

                UpdateCustomFields(device);
            }

            selectedItem = treeView.SelectedNode.Tag;
            if (selectedItem.GetType() == typeof(UPnPService))
            {
                //emptyPanel.SendToBack();
                ViewTab(2);
                UPnPService service = (UPnPService)selectedItem;
                SetServiceTree(service);

                //devicePanel.Visible = false;

                addServiceMenuItem.Enabled = false;
                importServiceMenuItem.Enabled = false;
                removeServiceMenuItem.Enabled = true;
                addDeviceMenuItem.Enabled = false;
                exportServiceMenuItem.Enabled = true;
                editServiceMenuItem.Enabled = true;
                removeDeviceMenuItem.Enabled = false;

                addServiceMenuItem2.Enabled = false;
                importServiceMenuItem2.Enabled = false;
                removeServiceMenuItem2.Enabled = true;
                addDeviceMenuItem2.Enabled = false;
                exportServiceMenuItem2.Enabled = true;
                removeDeviceMenuItem2.Enabled = false;

                menuItem10.Visible = false;
                addServiceMenuItem.Visible = false;
                importServiceMenuItem.Visible = false;
                removeServiceMenuItem.Visible = true;
                addDeviceMenuItem.Visible = false;
                exportServiceMenuItem.Visible = true;
                editServiceMenuItem.Visible = true;
                removeDeviceMenuItem.Visible = false;
            }

            HookupEventHandlers();
        }

        private void SetServiceTree(UPnPService service)
        {
            serviceNameTextBox.Text = service.User.ToString();
            serviceTypeTextBox.Text = service.ServiceURN;
            serviceIdTextBox.Text = service.ServiceID;

            serviceListView.Items.Clear();

            //			foreach (UPnPStateVariable var in service.GetStateVariables()) 
            //			{
            //				if (var.SendEvent == true) 
            //				{
            //					ListViewItem item = new ListViewItem(new string[] {var.Name,""},5);
            //					item.Tag = var;
            //					serviceListView.Items.Add(item);
            //				}
            //			}

            foreach (UPnPAction action in service.Actions)
            {
                string mode = "Normal";
                string escapeMode = "Auto";
                string cpEscapeMode = "Auto";
                ServiceGenerator.ServiceConfiguration Conf = (ServiceGenerator.ServiceConfiguration)service.User;

                if (Conf.Actions_Fragmented.Contains(action)) mode = "Fragment";
                if (Conf.Actions_ManualEscape.Contains(action)) escapeMode = "Manual";
                ListViewItem item = new ListViewItem(new string[] { action.Name, mode, escapeMode, cpEscapeMode }, 4);
                item.Tag = action;
                serviceListView.Items.Add(item);
            }
        }

        private void removeServiceButton_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode == null || treeView.SelectedNode.Tag == null) return;
            treeView.SelectedNode.Remove();
        }

        private void addServiceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (selectedItem.GetType() != typeof(UPnPDevice)) return;

            if (openServiceDialog.ShowDialog(this) == DialogResult.OK)
            {
                StreamReader fileData = File.OpenText(openServiceDialog.FileName);
                string xml = fileData.ReadToEnd();
                fileData.Close();

                UPnPService service = OpenSource.UPnP.ServiceGenerator.GenerateServiceFromSCPD(xml);
                service.ServiceID = "";
                service.ServiceURN = "";

                TreeNode serviceTreeNode = new TreeNode("ImportedService", 2, 2);
                serviceTreeNode.Tag = service;
                service.User = new ServiceGenerator.ServiceConfiguration("ImportedService", service);

                treeView.SelectedNode.Nodes.Add(serviceTreeNode);
                treeView.SelectedNode.Expand();

                updateStatusText();
            }
        }

        private void removeServiceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode.Tag == null || treeView.SelectedNode.Tag.GetType() != typeof(UPnPService)) return;
            UPnPService service = (UPnPService)treeView.SelectedNode.Tag;
            ClearTreeNodeTags(treeView.SelectedNode);
            treeView.SelectedNode.Remove();
            service.Dispose();

            updateStatusText();
        }

        private void treeView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            TreeNode node = treeView.GetNodeAt(e.X, e.Y);
            if (node != null)
            {
                treeView.SelectedNode = node;
            }
            else
            {
                treeView.SelectedNode = null;
                addServiceMenuItem.Enabled = false;
                importServiceMenuItem.Enabled = false;
                removeServiceMenuItem.Enabled = false;
                exportServiceMenuItem.Enabled = false;
                addDeviceMenuItem.Enabled = true;
                removeDeviceMenuItem.Enabled = false;

                addServiceMenuItem2.Enabled = false;
                importServiceMenuItem2.Enabled = false;
                removeServiceMenuItem2.Enabled = false;
                exportServiceMenuItem2.Enabled = false;
                addDeviceMenuItem2.Enabled = true;
                removeDeviceMenuItem2.Enabled = false;

                menuItem10.Visible = true;
                addServiceMenuItem.Visible = false;
                importServiceMenuItem.Visible = false;
                removeServiceMenuItem.Visible = false;
                exportServiceMenuItem.Visible = false;
                addDeviceMenuItem.Visible = true;
                removeDeviceMenuItem.Visible = false;

                //				deviceConfiguration.Visible = false;
                emptyPanel.BringToFront();
                emptyPanel.Visible = true;
            }
        }

        private void addDeviceMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDevice device = UPnPDevice.CreateRootDevice(6000, 1.0, ".");
            device.FriendlyName = "Sample Embedded Device";
            device.DeviceURN = "urn:schemas-upnp-org:device:Sample:1";
            device.Manufacturer = "OpenSource";
            device.ManufacturerURL = "http://opentools.homeip.net";
            device.ModelDescription = "Sample UPnP Device Using Auto-Generated UPnP Stack";
            device.ModelName = "Sample Auto-Generated Device";
            device.ModelNumber = "X1";
            device.ProductCode = "Sample-X1";
            device.SerialNumber = "00000001";

            TreeNode deviceTreeNode = new TreeNode("Sample Embedded Device", 1, 1);
            deviceTreeNode.Tag = device;
            if (treeView.SelectedNode == null)
            {
                treeView.Nodes.Add(deviceTreeNode);
                treeView.ExpandAll();
            }
            else
            {
                treeView.SelectedNode.Nodes.Add(deviceTreeNode);
                treeView.SelectedNode.Expand();
            }
            if (deviceType.SelectedIndex == 0)
            {
                device.User = new ServiceGenerator.Configuration(deviceCodePrefix.Text, ServiceGenerator.ConfigurationType.DEVICE);
            }
            else
            {
                device.User = new ServiceGenerator.Configuration(deviceCodePrefix.Text, ServiceGenerator.ConfigurationType.CONTROLPOINT);
            }
            updateStatusText();
        }

        //		private void RemoveServiceFromServiceTable_ByNode(TreeNode node)
        //		{
        //			foreach(TreeNode en in node.Nodes)
        //			{
        //				RemoveServiceFromServiceTable_ByNode(en);
        //			}
        //			if (node.Tag.GetType()==typeof(UPnPService))
        //			{
        //				serviceNameTable.Remove(node.Tag);
        //			}
        //		}
        private void removeDeviceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode.Tag == null || treeView.SelectedNode.Tag.GetType() != typeof(UPnPDevice))
            {
                return;
            }
            UPnPDevice device = (UPnPDevice)treeView.SelectedNode.Tag;

            ClearTreeNodeTags(treeView.SelectedNode);
            treeView.SelectedNode.Remove();

            updateStatusText();

            if (treeView.SelectedNode == null)
            {
                emptyPanel.BringToFront();
                emptyPanel.Visible = true;
            }
        }
        private void deviceInformationChanged(object sender, System.EventArgs e)
        {
            if (selectedItem.GetType() != typeof(UPnPDevice)) return;

            if (friendlyNameTextBox.Text == "" && treeView.SelectedNode != null)
            {
                treeView.SelectedNode.Text = "(No Name)";
            }
            else
            {
                treeView.SelectedNode.Text = friendlyNameTextBox.Text;
            }
            ((UPnPDevice)selectedItem).FriendlyName = treeView.SelectedNode.Text;
            ((UPnPDevice)selectedItem).DeviceURN = rootDeviceTypeTextBox.Text;
            ((UPnPDevice)selectedItem).Manufacturer = manufacturerTextBox.Text;
            ((UPnPDevice)selectedItem).ManufacturerURL = manufacturerUriTextBox.Text;
            ((UPnPDevice)selectedItem).ModelDescription = modelDescriptionTextBox.Text;
            ((UPnPDevice)selectedItem).ModelName = modelNameTextBox.Text;
            ((UPnPDevice)selectedItem).ModelNumber = modelNumberTextBox.Text;
            ((UPnPDevice)selectedItem).ProductCode = productCodeTextBox.Text;
        }

        private void serviceNameTextBox_TextChanged(object sender, System.EventArgs e)
        {
            serviceNameTextBox.Text = serviceNameTextBox.Text.Replace(" ", "");

            if (selectedItem.GetType() != typeof(UPnPService)) return;

            ((ServiceGenerator.ServiceConfiguration)((UPnPService)selectedItem).User).Name = serviceNameTextBox.Text;

            if (serviceNameTextBox.Text == "")
            {
                treeView.SelectedNode.Text = "(No Name)";
            }
            else
            {
                treeView.SelectedNode.Text = serviceNameTextBox.Text;
            }
        }

        private void serviceTypeTextBox_TextChanged(object sender, System.EventArgs e)
        {
            if (selectedItem.GetType() != typeof(UPnPService)) return;
            UPnPDebugObject dobj = new UPnPDebugObject(selectedItem);
            dobj.SetProperty("ServiceURN", serviceTypeTextBox.Text);
            serviceTypeTextBox.Text = ((UPnPService)selectedItem).ServiceURN;
        }

        private void serviceIdTextBox_TextChanged(object sender, System.EventArgs e)
        {
            if (selectedItem.GetType() != typeof(UPnPService)) return;
            ((UPnPService)selectedItem).ServiceID = serviceIdTextBox.Text;
        }

        private void importServiceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode == null) return;
            if (serviceLocator.ShowDialog(this) == DialogResult.OK)
            {
                if (serviceLocator.Service == null) return;

                UPnPService service = serviceLocator.Service;
                service.User = new ServiceGenerator.ServiceConfiguration("ImportedService", service);

                TreeNode serviceTreeNode = new TreeNode("ImportedService", 2, 2);
                serviceTreeNode.Tag = service;
                treeView.SelectedNode.Nodes.Add(serviceTreeNode);
                treeView.SelectedNode.Expand();

                updateStatusText();
            }
        }

        private void newMenuItem_Click(object sender, System.EventArgs e)
        {
            if (MessageBox.Show(this, "Reset all stack settings?", "Device Builder", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                this.Text = AppTitle;
                treeView.Nodes.Clear();
                saveMenuItem.Enabled = false;
                saveAsMenuItem.Enabled = false;
                treeView_AfterSelect(this, null);
                updateStatusText();
            }
        }

        private void encodeTreeEx(TreeNode node, Hashtable inTable)
        {
            if (node != null)
            {
                if (node.Tag.GetType() != typeof(UPnPDevice))
                {
                    return;
                }
            }
            else if (treeView.Nodes.Count > 0)
            {
                node = treeView.Nodes[0];
            }
            else
            {
                return;
            }

            UPnPDevice device = (UPnPDevice)node.Tag;
            Hashtable table;

            if (node.Parent == null)
            {
                ArrayList rootList;
                // Root
                if (inTable.Contains("RootDevices"))
                {
                    rootList = (ArrayList)inTable["RootDevices"];
                }
                else
                {
                    rootList = new ArrayList();
                    inTable["RootDevices"] = rootList;
                }
                table = new Hashtable();
                rootList.Add(table);
            }
            else
            {
                // Not Root
                table = inTable;
            }

            table["devicefriendlyName"] = device.FriendlyName;
            table["deviceType"] = device.DeviceURN;
            table["devicemanufacturer"] = device.Manufacturer;
            table["devicemanufacturerUri"] = device.ManufacturerURL;
            table["devicemodelDescription"] = device.ModelDescription;
            table["devicemodelName"] = device.ModelName;
            table["devicemodelNumber"] = device.ModelNumber;
            if (device.ModelURL != null) table["devicemodelUrl"] = device.ModelURL;
            table["deviceproductCode"] = device.ProductCode;
            table["deviceversion"] = device.Version;
            if (device.User != null)
            {
                table["deviceConfiguration"] = ((ServiceGenerator.Configuration)device.User).ToHashtable();
            }

            ArrayList services = new ArrayList();
            foreach (TreeNode n in node.Nodes)
            {
                if (n.Tag != null && n.Tag.GetType() == typeof(UPnPService))
                {
                    UPnPService service = (UPnPService)n.Tag;

                    Hashtable s = new Hashtable();
                    if (service.User != null)
                    {
                        s["ServiceConfiguration"] = ((ServiceGenerator.ServiceConfiguration)service.User).ToHashtable();
                    }
                    s["ServiceType"] = service.ServiceURN;
                    s["ServiceID"] = service.ServiceID;
                    s["ServiceXML"] = new System.Text.UTF8Encoding().GetString(service.GetSCPDXml());
                    services.Add(s);
                }
            }
            table["deviceservices"] = services;

            ArrayList devices = new ArrayList();
            foreach (TreeNode n in node.Nodes)
            {
                if (n.Tag != null && n.Tag.GetType() == typeof(UPnPDevice))
                {
                    Hashtable devtable = new Hashtable();
                    encodeTreeEx(n, devtable);
                    devices.Add(devtable);
                }
            }
            table["devicesubdevices"] = devices;

            if (node.NextNode != null && node.Parent == null)
            {
                encodeTreeEx(node.NextNode, inTable);
            }
        }

        private void saveAsMenuItem_Click(object sender, System.EventArgs e)
        {
            if (saveFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                Hashtable table = new Hashtable();
                table["codeGenSettings"] = codeGenSettings;
                encodeTreeEx(null, table);

                IFormatter formatter = new BinaryFormatter();
                Stream stream = new FileStream(saveFileDialog.FileName, FileMode.Create, FileAccess.Write, FileShare.None);
                formatter.Serialize(stream, table);
                stream.Close();

                this.Text = "Device Builder - " + new FileInfo(saveFileDialog.FileName).Name;
                saveMenuItem.Enabled = true;
            }
        }

        private void saveMenuItem_Click(object sender, System.EventArgs e)
        {
            if (saveFileDialog.FileName != "" && new FileInfo(saveFileDialog.FileName).Exists == true)
            {
                Hashtable table = new Hashtable();
                table["codeGenSettings"] = codeGenSettings;
                encodeTreeEx(null, table);

                IFormatter formatter = new BinaryFormatter();
                Stream stream = new FileStream(saveFileDialog.FileName, FileMode.Create, FileAccess.Write, FileShare.None);
                formatter.Serialize(stream, table);
                stream.Close();
            }
        }

        private void decodeTreeEx(TreeNode node, Hashtable inTable)
        {
            ArrayList rootDeviceList;

            if (inTable.Contains("RootDevices"))
            {
                rootDeviceList = (ArrayList)inTable["RootDevices"];
            }
            else
            {
                rootDeviceList = new ArrayList();
                rootDeviceList.Add(inTable);
            }


            foreach (Hashtable table in rootDeviceList)
            {
                if (node == null)
                {
                    node = new TreeNode();
                    treeView.Nodes.Add(node);
                }
                UPnPDevice device = UPnPDevice.CreateRootDevice(600, 1.0, "");

                if (table.Contains("devicefriendlyName")) device.FriendlyName = (string)table["devicefriendlyName"];
                if (table.Contains("deviceType")) device.DeviceURN = (string)table["deviceType"];
                if (table.Contains("devicemanufacturer")) device.Manufacturer = (string)table["devicemanufacturer"];
                if (table.Contains("devicemanufacturerUri")) device.ManufacturerURL = (string)table["devicemanufacturerUri"];
                if (table.Contains("devicemodelDescription")) device.ModelDescription = (string)table["devicemodelDescription"];
                if (table.Contains("devicemodelName")) device.ModelName = (string)table["devicemodelName"];
                if (table.Contains("devicemodelNumber")) device.ModelNumber = (string)table["devicemodelNumber"];
                if (table.Contains("devicemodelUrl")) device.ModelURL = new Uri((string)table["devicemodelUrl"]);
                if (table.Contains("deviceproductCode")) device.ProductCode = (string)table["deviceproductCode"];
                //if (table.Contains("deviceversion"))			device.Version			= (string)table["deviceversion"];
                if (table.Contains("deviceConfiguration") && node.Parent == null)
                {
                    device.User = ServiceGenerator.Configuration.FromHashtable((Hashtable)table["deviceConfiguration"]);
                }
                else if (node.Parent == null)
                {
                    ServiceGenerator.Configuration Conf = new ServiceGenerator.Configuration("UPnP", ServiceGenerator.ConfigurationType.DEVICE);
                    device.User = Conf;
                    if (table.Contains("Presentation")) { Conf.AdvertisesPresentationPage = (bool)table["Presentation"]; }
                }
                else if (node.Parent != null)
                {
                    device.User = ((UPnPDevice)node.Parent.Tag).User;
                }
                if (!table.Contains("deviceConfiguration") && node.Parent == null)
                {
                    ((ServiceGenerator.Configuration)device.User).LegacyFromHashtable(codeGenSettings);
                }
                node.Text = device.FriendlyName;
                node.Tag = device;

                if (table.Contains("deviceservices"))
                {
                    ArrayList services = (ArrayList)table["deviceservices"];
                    foreach (Hashtable table2 in services)
                    {
                        UPnPService s = new UPnPService(1.0, null);
                        if (table2.Contains("ServiceXML")) s = OpenSource.UPnP.ServiceGenerator.GenerateServiceFromSCPD((string)table2["ServiceXML"]);

                        if (table2.Contains("ServiceConfiguration"))
                        {
                            s.User = ServiceGenerator.ServiceConfiguration.FromHashtable((Hashtable)table2["ServiceConfiguration"]);
                        }
                        else if (table2.Contains("ServiceName"))
                        {
                            s.User = new ServiceGenerator.ServiceConfiguration((string)table2["ServiceName"], s);
                        }
                        else
                        {
                            s.User = new ServiceGenerator.ServiceConfiguration("(No Name)", s);
                        }

                        if (table2.Contains("ServiceType")) s.ServiceURN = (string)table2["ServiceType"];
                        if (table2.Contains("ServiceID")) s.ServiceID = (string)table2["ServiceID"];

                        if (table2.Contains("ServiceFragActions"))
                        {
                            ArrayList ans = (ArrayList)table2["ServiceFragActions"];
                            foreach (string an in ans)
                            {
                                foreach (UPnPAction a in s.Actions)
                                {
                                    if (a.Name == an)
                                    {
                                        ((ServiceGenerator.ServiceConfiguration)s.User).Actions_Fragmented.Add(a);
                                    }
                                }
                            }
                        }
                        if (table2.Contains("ManualEscapeActions"))
                        {
                            ArrayList ans = (ArrayList)table2["ManualEscapeActions"];
                            foreach (string an in ans)
                            {
                                foreach (UPnPAction a in s.Actions)
                                {
                                    if (a.Name == an)
                                    {
                                        ((ServiceGenerator.ServiceConfiguration)s.User).Actions_ManualEscape.Add(a);
                                    }
                                }
                            }
                        }
                        if (table2.Contains("CPManualEscapeActions"))
                        {
                            ArrayList ans = (ArrayList)table2["CPManualEscapeActions"];
                            foreach (string an in ans)
                            {
                                foreach (UPnPAction a in s.Actions)
                                {
                                    if (a.Name == an)
                                    {
                                        ((ServiceGenerator.ServiceConfiguration)s.User).Actions_ManualEscape.Add(a);
                                    }
                                }
                            }
                        }
                        TreeNode n = new TreeNode(s.User.ToString(), 2, 2);
                        n.Tag = s;
                        node.Nodes.Add(n);
                    }
                }

                if (table.Contains("devicesubdevices"))
                {
                    ArrayList devices = (ArrayList)table["devicesubdevices"];
                    foreach (Hashtable d in devices)
                    {
                        TreeNode n = new TreeNode("", 1, 1);
                        node.Nodes.Add(n);
                        decodeTreeEx(n, d);
                    }
                }
                node.Expand();
                node = null;
            }
        }

        private void openMenuItem_Click(object sender, System.EventArgs e)
        {
            if (openFileDialog.ShowDialog(this) == DialogResult.OK) OpenSettingsFile(openFileDialog.FileName, false);
        }

        public static Type GetUPnPDeviceType()
        {
            return (typeof(UPnPDevice));
        }
        public static bool GenerateEx(object[] deviceList, DirectoryInfo outputDirectory, ServiceGenerator.StackConfiguration Config)
        {
            EmbeddedCGenerator g = new EmbeddedCGenerator(Config);
            return (g.Generate((UPnPDevice[])deviceList, outputDirectory));
        }
        public static string GetPrefixFromDevice(UPnPDevice device)
        {
            if (device.User != null)
            {
                return (((ServiceGenerator.Configuration)device.User).Prefix);
            }
            else
            {
                return (null);
            }
        }
        public static void SetPrefixToDevice(UPnPDevice device, string prefix)
        {
            if (device.User != null)
            {
                ((ServiceGenerator.Configuration)device.User).Prefix = prefix;
            }
        }
        public static bool IsDevice(UPnPDevice device)
        {
            if (device.User != null)
            {
                return (((ServiceGenerator.Configuration)device.User).ConfigType == ServiceGenerator.ConfigurationType.DEVICE);
            }
            else
            {
                return (false);
            }
        }
        public static UPnPDevice[] OpenSettingsFileEx(string fileName)
        {
            ArrayList l = new ArrayList();
            IFormatter formatter = new BinaryFormatter();
            Stream stream = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read);
            Hashtable table = (Hashtable)formatter.Deserialize(stream);
            stream.Close();

            Hashtable cgs = new Hashtable();
            if (table.Contains("codeGenSettings")) cgs = (Hashtable)table["codeGenSettings"];

            MainForm mf = new MainForm(new string[0]);
            mf.decodeTreeEx(null, table);

            UPnPDevice device;
            foreach (TreeNode tn in mf.treeView.Nodes)
            {
                device = mf.BuildDevice(tn, null);
                if (device != null)
                {
                    device.Reserved = cgs;
                    l.Add(device);
                }
            }
            return (((UPnPDevice[])l.ToArray(typeof(UPnPDevice))));
        }
        public static string[] PopulateMetaData(UPnPDevice device)
        {
            UPnPDevice[] devices = new UPnPDevice[1] { device };
            ArrayList RetVal = new ArrayList();

            DeviceObjectGenerator.PrepDevice(devices);

            PopulateMetaDataEx(device, RetVal);
            return ((string[])RetVal.ToArray(typeof(string)));
        }
        private static void PopulateMetaDataEx(UPnPDevice device, ArrayList RetVal)
        {
            string deviceID;

            if (device.Root)
            {
                deviceID = "";
            }
            else
            {
                deviceID = DeviceObjectGenerator.GetDeviceIdentifier(device);
                deviceID = deviceID.Substring(deviceID.IndexOf(".") + 1);
                deviceID += "->";
            }

            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "Manufacturer = \"{{{MANUFACTURER}}}\";");
            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "ManufacturerURL = \"{{{MANUFACTURERURL}}}\";");
            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "ModelName = \"{{{MODELNAME}}}\";");
            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "ModelDescription = \"{{{MODELDESCRIPTION}}}\";");
            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "ModelNumber = \"{{{MODELNUMBER}}}\";");
            RetVal.Add(GetPrefixFromDevice(device) + "GetConfiguration()->" + deviceID + "ModelURL = \"{{{MODELURL}}}\";");

            foreach (UPnPDevice ed in device.EmbeddedDevices)
            {
                PopulateMetaDataEx(ed, RetVal);
            }
        }

        private void OpenSettingsFile(string filename, bool append)
        {
            IFormatter formatter = new BinaryFormatter();
            Stream stream = new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.Read);
            Hashtable table = (Hashtable)formatter.Deserialize(stream);
            stream.Close();

            if (table.Contains("codeGenSettings")) codeGenSettings = (Hashtable)table["codeGenSettings"];
            if (!append) treeView.Nodes.Clear();

            decodeTreeEx(null, table);
            //ToDo: FIXME selectedItem = deviceRootTreeNode.Tag;

            saveFileDialog.FileName = filename;
            this.Text = AppTitle + " - " + new FileInfo(filename).Name;
            saveMenuItem.Enabled = saveAsMenuItem.Enabled = true;

            treeView_AfterSelect(this, null);
            treeView.SelectedNode = treeView.Nodes[0];
            updateStatusText();
        }

        private void ClearTreeNodeTags(TreeNode node)
        {
            if (node.Tag.GetType() == typeof(UPnPService))
            {
                UPnPService service = (UPnPService)node.Tag;
                foreach (UPnPAction action in service.Actions)
                {
                    if (((ServiceGenerator.ServiceConfiguration)service.User).Actions_Fragmented.Contains(action))
                    {
                        ((ServiceGenerator.ServiceConfiguration)service.User).Actions_Fragmented.Remove(action);
                    }
                    if (((ServiceGenerator.ServiceConfiguration)service.User).Actions_ManualEscape.Contains(action))
                    {
                        ((ServiceGenerator.ServiceConfiguration)service.User).Actions_ManualEscape.Remove(action);
                    }
                }
            }

            node.Tag = null;
            foreach (TreeNode n in node.Nodes) ClearTreeNodeTags(n);
        }

        private UPnPDevice BuildDevice(TreeNode node, UPnPDevice parentDevice)
        {
            if (node.Tag == null || node.Tag.GetType() != typeof(UPnPDevice)) return null;

            UPnPDevice device = (UPnPDevice)node.Tag;
            UPnPDevice dev;

            if (parentDevice == null)
            {
                dev = UPnPDevice.CreateRootDevice(600, 1.0, "");
                dev.SerialNumber = "0000001";
                dev.User = device.User;
                dev.Icon = ((ServiceGenerator.Configuration)device.User).IconImageSM;
                dev.Icon2 = ((ServiceGenerator.Configuration)device.User).IconImageLG;
            }
            else
            {
                dev = UPnPDevice.CreateEmbeddedDevice(1.0, System.Guid.NewGuid().ToString());
                dev.User = device.User;
            }

            dev.FriendlyName = device.FriendlyName;
            dev.DeviceURN = device.DeviceURN;
            dev.Manufacturer = device.Manufacturer;
            dev.ManufacturerURL = device.ManufacturerURL;
            dev.ModelDescription = device.ModelDescription;
            dev.ModelName = device.ModelName;
            dev.ModelNumber = device.ModelNumber;
            dev.ProductCode = device.ProductCode;
            dev.SerialNumber = device.SerialNumber;


            foreach (TreeNode n in node.Nodes)
            {
                if (n.Tag != null && n.Tag.GetType() == typeof(UPnPService))
                {
                    dev.AddService((UPnPService)n.Tag);
                }
            }

            foreach (TreeNode n in node.Nodes)
            {
                if (n.Tag != null && n.Tag.GetType() == typeof(UPnPDevice))
                {
                    BuildDevice(n, dev);
                }
            }

            if (parentDevice != null) parentDevice.AddDevice(dev);

            return dev;
        }

        private void exportStackMenuItem_Click(object sender, System.EventArgs e)
        {
            // Check Service Names
            //			foreach (object k1 in serviceNameTable.Keys) 
            //			{
            //				foreach (object k2 in serviceNameTable.Keys) 
            //				{
            //					if ((serviceNameTable[k1].ToString().CompareTo(serviceNameTable[k2].ToString()) == 0) && (k1 != k2)) 
            //					{
            //						MessageBox.Show(this,"Service names must be unique, " + serviceNameTable[k1] + " is duplicated", "Device Builder", MessageBoxButtons.OK, MessageBoxIcon.Error);
            //						return;
            //					}
            //				}
            //			}


            UPnPDevice device;
            foreach (TreeNode tn in treeView.Nodes)
            {
                device = BuildDevice(tn, null);
                if (device != null)
                {
                    //					CodeGenerationForm form = new CodeGenerationForm(device, stackSettings, fragRespActions, escapeActions);
                    //					form.Settings = codeGenSettings;
                    //					form.ShowDialog(this);
                    //					codeGenSettings = form.Settings;
                }
            }
        }

        private void serviceListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            //serviceListView.GetItemAt(e.X,e.Y);
        }

        private void serviceContextMenu_Popup(object sender, System.EventArgs e)
        {
            ServiceGenerator.ServiceConfiguration Conf;

            if (serviceListView.SelectedItems.Count == 0 || serviceListView.SelectedItems[0].Tag.GetType() != typeof(UPnPAction))
            {
                actionNormalRespMenuItem.Visible = false;
                actionFragRespMenuItem.Visible = false;
                menuItem3.Visible = false;
                actionManualEscapeMenuItem.Visible = false;
                actionAutoEscapeMenuItem.Visible = false;
            }
            else
            {
                actionNormalRespMenuItem.Visible = true;
                actionFragRespMenuItem.Visible = true;
                menuItem3.Visible = true;
                actionManualEscapeMenuItem.Visible = true;
                actionAutoEscapeMenuItem.Visible = true;

                UPnPAction A = (UPnPAction)serviceListView.SelectedItems[0].Tag;
                Conf = (ServiceGenerator.ServiceConfiguration)A.ParentService.User;

                if (Conf.Actions_Fragmented.Contains(A))
                {
                    actionNormalRespMenuItem.Checked = false;
                    actionFragRespMenuItem.Checked = true;
                }
                else
                {
                    actionNormalRespMenuItem.Checked = true;
                    actionFragRespMenuItem.Checked = false;
                }

                if (Conf.Actions_ManualEscape.Contains(A))
                {
                    actionManualEscapeMenuItem.Checked = true;
                    actionAutoEscapeMenuItem.Checked = false;
                }
                else
                {
                    actionManualEscapeMenuItem.Checked = false;
                    actionAutoEscapeMenuItem.Checked = true;
                }
            }
        }

        private void actionNormalRespMenuItem_Click(object sender, System.EventArgs e)
        {
            if (serviceListView.SelectedItems.Count == 0 || serviceListView.SelectedItems[0].Tag.GetType() != typeof(UPnPAction)) return;

            UPnPAction A = (UPnPAction)serviceListView.SelectedItems[0].Tag;
            ServiceGenerator.ServiceConfiguration Conf = (ServiceGenerator.ServiceConfiguration)A.ParentService.User;

            Conf.Actions_Fragmented.Remove(A);
            SetServiceTree((UPnPService)selectedItem);
        }

        private void actionFragRespMenuItem_Click(object sender, System.EventArgs e)
        {
            if (serviceListView.SelectedItems.Count == 0 || serviceListView.SelectedItems[0].Tag.GetType() != typeof(UPnPAction)) return;
            UPnPAction A = (UPnPAction)serviceListView.SelectedItems[0].Tag;
            ServiceGenerator.ServiceConfiguration Conf = (ServiceGenerator.ServiceConfiguration)A.ParentService.User;

            Conf.Actions_Fragmented.Add(serviceListView.SelectedItems[0].Tag);
            if (!Conf.Actions_ManualEscape.Contains(serviceListView.SelectedItems[0].Tag))
            {
                Conf.Actions_ManualEscape.Add(serviceListView.SelectedItems[0].Tag);
            }
            SetServiceTree((UPnPService)selectedItem);
        }

        private int devicecount = 0;
        private int servicecount = 0;
        private int actioncount = 0;
        private int varcount = 0;
        private int eventvarcount = 0;
        private int argcount = 0;

        private void updateStatusTextEx(TreeNode node)
        {
            if (node == null && treeView.Nodes.Count > 0)
            {
                node = treeView.Nodes[0];
            }
            if (node == null) { return; }
            if (node.Tag == null) return;

            if (node.Tag.GetType() == typeof(UPnPDevice))
            {
                devicecount++;
                foreach (TreeNode n in node.Nodes)
                {
                    updateStatusTextEx(n);
                }
                if (node.NextNode != null) { updateStatusTextEx(node.NextNode); }
            }
            if (node.Tag.GetType() == typeof(UPnPService))
            {
                servicecount++;
                UPnPService service = (UPnPService)node.Tag;
                foreach (UPnPAction action in service.Actions)
                {
                    actioncount++;
                    argcount += action.Arguments.Count;
                }

                foreach (UPnPStateVariable var in service.GetStateVariables())
                {
                    varcount++;
                    if (var.SendEvent == true) eventvarcount++;
                }
            }
        }

        private void updateStatusText()
        {
            devicecount = 0;
            servicecount = 0;
            actioncount = 0;
            varcount = 0;
            eventvarcount = 0;
            argcount = 0;
            //			deviceConfiguration.Visible = false;

            updateStatusTextEx(null);

            mainStatusLabel2.Text = devicecount.ToString() + " devices, " + servicecount + " services, " + actioncount + " actions, " + argcount + " arguments, " + eventvarcount + " events";
        }

        private void exportControlPointMenuItem_Click(object sender, System.EventArgs e)
        {
            // Check Service Names
            //			foreach (object k1 in serviceNameTable.Keys) 
            //			{
            //				foreach (object k2 in serviceNameTable.Keys) 
            //				{
            //					if ((serviceNameTable[k1].ToString().CompareTo(serviceNameTable[k2].ToString()) == 0) && (k1 != k2)) 
            //					{
            //						MessageBox.Show(this,"Service names must be unique, " + serviceNameTable[k1] + " is duplicated", "Device Builder", MessageBoxButtons.OK, MessageBoxIcon.Error);
            //						return;
            //					}
            //				}
            //			}

            /* //ToDo: FIXME
            UPnPDevice device = BuildDevice(deviceRootTreeNode,null);
            if (device != null) 
            {
                CPCodeGenerationForm form = new CPCodeGenerationForm(device, stackSettings, fragRespActions, cpEscapeActions);
                form.Settings = codeCpGenSettings;
                form.ShowDialog(this);
                codeCpGenSettings = form.Settings;
            }
            */
        }

        private void AddEmbeddedDevice(TreeNode parentnode, UPnPDevice newdevice)
        {
            TreeNode node = new TreeNode(newdevice.FriendlyName, 1, 1);
            UPnPDevice device = UPnPDevice.CreateRootDevice(6000, 1.0, ".");
            device.FriendlyName = newdevice.FriendlyName;
            device.DeviceURN = newdevice.DeviceURN;
            device.Manufacturer = newdevice.Manufacturer;
            device.ManufacturerURL = newdevice.ManufacturerURL;
            device.ModelDescription = newdevice.ModelDescription;
            device.ModelName = newdevice.ModelName;
            device.ModelNumber = newdevice.ModelNumber;
            device.ProductCode = newdevice.ProductCode;
            device.SerialNumber = newdevice.SerialNumber;
            device.User = new ServiceGenerator.Configuration("UPnP", ServiceGenerator.ConfigurationType.DEVICE);

            node.Tag = device;
            parentnode.Nodes.Add(node);

            foreach (UPnPDevice embeddeddevice in newdevice.EmbeddedDevices)
            {
                AddEmbeddedDevice(node, embeddeddevice);
            }

            foreach (UPnPService service in newdevice.Services)
            {
                string servicename = service.ServiceURN;
                if (servicename.StartsWith("urn:schemas-upnp-org:service:"))
                {
                    servicename = servicename.Substring(29);
                }
                int i = servicename.IndexOf(":");
                if (i != -1) servicename = servicename.Substring(0, i);
                //serviceNameTable[service] = servicename;
                service.User = new ServiceGenerator.ServiceConfiguration(servicename, service);
                TreeNode serviceTreeNode = new TreeNode(servicename, 2, 2);
                serviceTreeNode.Tag = service;
                node.Nodes.Add(serviceTreeNode);
            }

            node.Expand();
        }

        private void networkOpenMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDeviceLocator devicelocator = new UPnPDeviceLocator();
            if (devicelocator.ShowDialog(this) == DialogResult.OK && devicelocator.SelectedDevice != null)
            {
                //				ClearTreeNodeTags(deviceRootTreeNode);
                //				deviceRootTreeNode.Text = devicelocator.SelectedDevice.FriendlyName;
                //				treeView.Nodes.Clear();
                //				deviceRootTreeNode.Nodes.Clear();


                UPnPDevice device = UPnPDevice.CreateRootDevice(6000, 1.0, ".");
                device.FriendlyName = devicelocator.SelectedDevice.FriendlyName;
                device.DeviceURN = devicelocator.SelectedDevice.DeviceURN;
                device.Manufacturer = devicelocator.SelectedDevice.Manufacturer;
                device.ManufacturerURL = devicelocator.SelectedDevice.ManufacturerURL;
                device.ModelDescription = devicelocator.SelectedDevice.ModelDescription;
                device.ModelName = devicelocator.SelectedDevice.ModelName;
                device.ModelNumber = devicelocator.SelectedDevice.ModelNumber;
                device.ProductCode = devicelocator.SelectedDevice.ProductCode;
                device.SerialNumber = devicelocator.SelectedDevice.SerialNumber;
                device.User = new ServiceGenerator.Configuration("UPnP", ServiceGenerator.ConfigurationType.DEVICE);


                TreeNode n = new TreeNode(device.FriendlyName);
                n.Tag = device;

                treeView.Nodes.Add(n);

                foreach (UPnPDevice embeddeddevice in devicelocator.SelectedDevice.EmbeddedDevices)
                {
                    AddEmbeddedDevice(n, embeddeddevice);
                }

                foreach (UPnPService service in devicelocator.SelectedDevice.Services)
                {
                    DText p = new DText();
                    p.ATTRMARK = ":";

                    string servicename = service.ServiceURN;
                    p[0] = servicename;

                    servicename = p[p.DCOUNT() - 1];

                    service.User = new ServiceGenerator.ServiceConfiguration(servicename, service);
                    TreeNode serviceTreeNode = new TreeNode(servicename, 2, 2);
                    serviceTreeNode.Tag = service;
                    n.Nodes.Add(serviceTreeNode);
                }

                treeView.SelectedNode = n;
                selectedItem = n.Tag;
                n.Expand();

                this.Text = AppTitle;
                saveMenuItem.Enabled = false;
                saveAsMenuItem.Enabled = true;

                treeView_AfterSelect(this, null);
                updateStatusText();
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
                if (file.Extension.ToLower() != ".upnpsg") return;
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
                if (file.Extension.ToLower() != ".upnpsg") return;
                OpenSettingsFile(file.FullName, false);
            }
        }

        private void MainForm_Load(object sender, System.EventArgs e)
        {
            if (this.MustExit) this.Close();
            else if (new FileInfo(saveFileDialog.FileName).Exists == true) OpenSettingsFile(saveFileDialog.FileName, false);
            ViewTab(0);
            treeView_AfterSelect(this, null);

            // Check for update
            if (File.Exists(Application.StartupPath + "\\AutoUpdateTool.exe"))
            {
                AutoUpdate.AutoUpdateCheck(this);
            }
            else
            {
                menuItem6.Visible = false;
            }
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "AuthoringToolsHelp.chm", HelpNavigator.KeywordIndex, "Using Device Builder");
        }

        private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "AuthoringToolsHelp.chm", HelpNavigator.KeywordIndex, "Using Device Builder");
        }

        private void exportServiceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode.Tag == null || treeView.SelectedNode.Tag.GetType() != typeof(UPnPService)) return;
            UPnPService service = (UPnPService)treeView.SelectedNode.Tag;

            if (exportSaveFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                FileStream file = File.Create(exportSaveFileDialog.FileName);
                byte[] xml = service.GetSCPDXml();
                file.Write(xml, 0, xml.Length);
            }
        }

        private void actionAutoEscapeMenuItem_Click(object sender, System.EventArgs e)
        {
            if (serviceListView.SelectedItems.Count == 0 || serviceListView.SelectedItems[0].Tag.GetType() != typeof(UPnPAction)) return;
            UPnPAction A = (UPnPAction)serviceListView.SelectedItems[0].Tag;
            ServiceGenerator.ServiceConfiguration Conf = (ServiceGenerator.ServiceConfiguration)A.ParentService.User;

            if (!Conf.Actions_Fragmented.Contains(A))
            {
                Conf.Actions_ManualEscape.Remove(A);
                SetServiceTree((UPnPService)selectedItem);
            }
        }

        private void actionManualEscapeMenuItem_Click(object sender, System.EventArgs e)
        {
            if (serviceListView.SelectedItems.Count == 0 || serviceListView.SelectedItems[0].Tag.GetType() != typeof(UPnPAction)) return;
            UPnPAction A = (UPnPAction)serviceListView.SelectedItems[0].Tag;
            ServiceGenerator.ServiceConfiguration Conf = (ServiceGenerator.ServiceConfiguration)A.ParentService.User;

            Conf.Actions_ManualEscape.Add(serviceListView.SelectedItems[0].Tag);
            SetServiceTree((UPnPService)selectedItem);
        }

        private void appendMenuItem_Click(object sender, System.EventArgs e)
        {
            if (openFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                OpenSettingsFile(openFileDialog.FileName, true);
            }
        }

        private void OnStackTypeChanged(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode == null) return;

            UPnPDevice d = (UPnPDevice)treeView.SelectedNode.Tag;
            ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)d.User;

            this.devicePresentationPage.Enabled = this.deviceType.SelectedIndex == 0;
            this.ssdpCycleTextBox.Enabled = this.deviceType.SelectedIndex == 0;
            this.webPortTextBox.Enabled = this.deviceType.SelectedIndex == 0;
            this.setLargeIconButton.Enabled = this.deviceType.SelectedIndex == 0;
            this.maxSubscriptionTimeoutTextBox.Enabled = this.deviceType.SelectedIndex == 0;

            if (deviceType.SelectedIndex != 0)
            {
                //
                // CP
                //
                Conf.IconImageLG = null;
                Conf.IconImageSM = null;
                this.deviceIconImageLG.Image = null;
                this.deviceIconImageSM.Image = null;
            }

            Conf.ConfigType = (ServiceGenerator.ConfigurationType)(int)deviceType.SelectedIndex;
        }

        private UPnPDevice[] GetDevices()
        {
            UPnPDevice device;
            ArrayList deviceList = new ArrayList();

            foreach (TreeNode tn in treeView.Nodes)
            {
                device = BuildDevice(tn, null);
                if (device != null)
                {
                    deviceList.Add(device);
                }
            }

            return ((UPnPDevice[])deviceList.ToArray(typeof(UPnPDevice)));
        }

        private bool ValidateServiceNames(UPnPDevice device, Dictionary<string, UPnPService> table)
        {
            foreach (UPnPService service in device.Services)
            {
                if (table.ContainsKey(((ServiceGenerator.ServiceConfiguration)service.User).Name))
                {
                    UPnPDevice root = service.ParentDevice;
                    while (root.ParentDevice != null)
                    {
                        root = root.ParentDevice;
                    }
                    MessageBox.Show("Duplicate ServiceName: " + ((ServiceGenerator.ServiceConfiguration)service.User).Name + " found in device: " + root.FriendlyName + "!\r\nAll Services Names must be unique within their root device heirarchy.", "Not Allowed Error!");
                    return (false);
                }
                else
                {
                    table[((ServiceGenerator.ServiceConfiguration)service.User).Name] = service;
                }
            }
            foreach (UPnPDevice eDevice in device.EmbeddedDevices)
            {
                if (!ValidateServiceNames(eDevice, table))
                {
                    return (false);
                }
            }
            return (true);
        }

        private void exportStacksMenuItem_Click(object sender, System.EventArgs e)
        {
            UPnPDevice[] devices = GetDevices();

            foreach (UPnPDevice dev in devices)
            {
                //
                // Check that each "ServiceName" within a device heirarchy is unique. The Code Generators Rely on that to be the case.
                //
                Dictionary<string, UPnPService> table = new Dictionary<string, UPnPService>();

                if (!(ValidateServiceNames(dev, table)))
                {
                    return;
                }
            }

            if (devices.Length > 0)
            {
                //
                // Verify that all the code prefixes are unique
                //
                bool ok = true;
                Hashtable test = new Hashtable();
                foreach (UPnPDevice dv in devices)
                {
                    if (test.ContainsKey(((ServiceGenerator.Configuration)dv.User).Prefix))
                    {
                        MessageBox.Show(this, "The Code Prefix for each device must be unique!", "Stack Generation Error");
                        ok = false;
                        break;
                    }
                    else if (((ServiceGenerator.Configuration)dv.User).Prefix == "UPnP" && devices.Length > 1)
                    {
                        MessageBox.Show(this, "Code prefix cannot be \"UPnP\" when generating combo-stacks!", "Stack Generation Error");
                        ok = false;
                        break;
                    }
                    else
                    {
                        test.Add(((ServiceGenerator.Configuration)dv.User).Prefix, dv);
                    }
                }

                if (ok)
                {
                    CodeGenerationForm form = new CodeGenerationForm(new GetDevicesHandler(GetDevices), codeGenSettings);
                    form.ShowDialog(this);
                    codeGenSettings = form.Settings;
                }
                test.Clear();
            }
            else
            {
                MessageBox.Show("No device description documents", "Stack Generation Error");
            }
        }

        private void OnCodePrefixChanged(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)treeView.SelectedNode.Tag;
            ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)d.User;

            Conf.Prefix = deviceCodePrefix.Text;
        }

        private void OnPresentationChanged(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)treeView.SelectedNode.Tag;
            ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)d.User;

            Conf.AdvertisesPresentationPage = this.devicePresentationPage.Checked;
        }

        private void OnAdvancedOptionsChanged(object sender, System.EventArgs e)
        {
            UPnPDevice d = (UPnPDevice)treeView.SelectedNode.Tag;
            ServiceGenerator.Configuration Conf = (ServiceGenerator.Configuration)d.User;

            Conf.SSDPCycleTime = int.Parse(this.ssdpCycleTextBox.Text);
            Conf.WebPort = int.Parse(this.webPortTextBox.Text);
            Conf.MaxSubscriptionTimeout = int.Parse(this.maxSubscriptionTimeoutTextBox.Text);
        }

        private void OnCustomFieldChanged(object sender, System.EventArgs e)
        {
            System.Diagnostics.Debug.WriteLine("Changed\r\n");
        }

        private void OnCustomField_Leave(object sender, System.EventArgs e)
        {
            UPnPDevice dv = (UPnPDevice)treeView.SelectedNode.Tag;
            SaveCustomFieldsIntoDevice(dv);
        }

        private void FileMenu_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode != null && treeView.SelectedNode.Tag != null && treeView.SelectedNode.Tag.GetType() == typeof(UPnPDevice))
            {
                UPnPDevice dv = (UPnPDevice)treeView.SelectedNode.Tag;
                SaveCustomFieldsIntoDevice(dv);
                OnAdvancedOptionsChanged(null, null);
            }
        }

        private void OnAuthorExited(object sender, System.EventArgs e)
        {
            if (InvokeRequired) { this.Invoke(new EventHandler(OnAuthorExited), new object[2] { sender, e }); return; }
            if (treeView.SelectedNode.Tag == null || treeView.SelectedNode.Tag.GetType() != typeof(UPnPService)) return;
            UPnPService oldService = (UPnPService)treeView.SelectedNode.Tag;

            //
            // Re-import the service description
            //
            StreamReader fileData = File.OpenText(filePath);
            string xml = fileData.ReadToEnd();
            fileData.Close();

            UPnPService service = OpenSource.UPnP.ServiceGenerator.GenerateServiceFromSCPD(xml);
            service.ServiceID = oldService.ServiceID;
            service.ServiceURN = oldService.ServiceURN;

            treeView.SelectedNode.Tag = service;
            service.User = oldService.User;

            updateStatusText();

            //
            // Delete the temp file
            //
            File.Delete(filePath);
            this.Show();
            treeView_AfterSelect(null, null);
        }

        private void editServiceMenuItem_Click(object sender, System.EventArgs e)
        {
            if (treeView.SelectedNode.Tag == null || treeView.SelectedNode.Tag.GetType() != typeof(UPnPService)) return;
            UPnPService service = (UPnPService)treeView.SelectedNode.Tag;


            //
            // Export the current service Description
            //
            FileStream file = File.Create("temp.scpd");
            filePath = file.Name;
            byte[] xml = service.GetSCPDXml();
            file.Write(xml, 0, xml.Length);
            file.Close();

            // Find Service Author
            FileInfo fi = new FileInfo(Application.StartupPath + "\\Service Author.exe");
            if (fi.Exists == false) fi = new FileInfo(Application.StartupPath + "\\..\\..\\..\\..\\ServiceAuthor\\bin\\x86\\debug\\Service Author.exe");
            if (fi.Exists == false)
            {
                MessageBox.Show(this, "Unable to find \"Service Author.exe\"", "Launch", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            //
            // Launch Service Author, to modify the file
            //
            author = new System.Diagnostics.Process();
            author.StartInfo.FileName = fi.FullName;
            author.StartInfo.WorkingDirectory = System.Windows.Forms.Application.StartupPath;
            author.StartInfo.Arguments = "\"" + file.Name + "\"";
            author.StartInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Normal;
            author.StartInfo.UseShellExecute = false;
            author.EnableRaisingEvents = true;

            author.Exited += new EventHandler(this.OnAuthorExited);

            this.Hide();
            author.Start();
        }

        private void setLargeIconButton_Click(object sender, EventArgs e)
        {
            UPnPDevice dv = (UPnPDevice)treeView.SelectedNode.Tag;

            System.Windows.Forms.OpenFileDialog d = new System.Windows.Forms.OpenFileDialog();
            d.Multiselect = false;
            d.Title = "Select Large Image";
            d.Filter = "Images (*.png,*.bmp,*.gif,*.jpg)|*.png;*.bmp;*.gif;*.jpg";
            if (d.ShowDialog() == DialogResult.OK)
            {
                deviceIconImageSM.Image = new Bitmap(d.FileName);
                Image UseImage = new Bitmap(d.FileName);
                if (UseImage.Width > 120 || UseImage.Height > 120)
                {
                    Image image2 = new Bitmap(UseImage, new System.Drawing.Size(48, 48));
                    UseImage = image2;
                }
                ((ServiceGenerator.Configuration)dv.User).IconImageLG = UseImage;
                if (((ServiceGenerator.Configuration)dv.User).IconImageSM == null) ((ServiceGenerator.Configuration)dv.User).IconImageSM = ((ServiceGenerator.Configuration)dv.User).IconImageLG;
            }

            deviceIconImageSM.Image = ((ServiceGenerator.Configuration)dv.User).IconImageSM;
            deviceIconImageLG.Image = ((ServiceGenerator.Configuration)dv.User).IconImageLG;
        }

        private void setSmallIconButton_Click(object sender, EventArgs e)
        {
            UPnPDevice dv = (UPnPDevice)treeView.SelectedNode.Tag;

            System.Windows.Forms.OpenFileDialog d = new System.Windows.Forms.OpenFileDialog();
            d.Multiselect = false;
            d.Title = "Select Small Image";
            d.Filter = "Images (*.png,*.bmp,*.gif,*.jpg)|*.png;*.bmp;*.gif;*.jpg";
            if (d.ShowDialog() == DialogResult.OK)
            {
                Image UseImage = new Bitmap(d.FileName);
                if (UseImage.Width > 48 || UseImage.Height > 48)
                {
                    Image image2 = new Bitmap(UseImage, new System.Drawing.Size(48, 48));
                    UseImage = image2;
                }
                ((ServiceGenerator.Configuration)dv.User).IconImageSM = UseImage;
                if (((ServiceGenerator.Configuration)dv.User).IconImageLG == null) ((ServiceGenerator.Configuration)dv.User).IconImageLG = ((ServiceGenerator.Configuration)dv.User).IconImageSM;
            }

            deviceIconImageSM.Image = ((ServiceGenerator.Configuration)dv.User).IconImageSM;
            deviceIconImageLG.Image = ((ServiceGenerator.Configuration)dv.User).IconImageLG;
        }

        private void clearIconButton_Click(object sender, EventArgs e)
        {
            UPnPDevice dv = (UPnPDevice)treeView.SelectedNode.Tag;
            ((ServiceGenerator.Configuration)dv.User).IconImageSM = null;
            ((ServiceGenerator.Configuration)dv.User).IconImageLG = null;
            deviceIconImageSM.Image = ((ServiceGenerator.Configuration)dv.User).IconImageSM;
            deviceIconImageLG.Image = ((ServiceGenerator.Configuration)dv.User).IconImageLG;
        }

        private void menuItem2_Popup(object sender, EventArgs e)
        {
            menuItem6.Checked = AutoUpdate.GetAutoUpdateCheck();
        }

        private void menuItem6_Click(object sender, EventArgs e)
        {
            AutoUpdate.SetAutoUpdateCheck(!menuItem6.Checked);
            if (!menuItem6.Checked) AutoUpdate.UpdateCheck(this);
        }
    }
}
