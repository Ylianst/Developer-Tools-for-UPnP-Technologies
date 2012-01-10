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
using System.Data;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPLight
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.ContextMenu contextMenu;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.ComponentModel.IContainer components;

        private UPnPDevice upnpLightDevice = null;
        private UPnPService upnpLightService = null;
        private UPnPService upnpDimmerService = null;
        private bool lightState = false;
        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MenuItem RaiseDimMenuItem;
        private System.Windows.Forms.MenuItem LowerDimMenuItem6;
        private System.Windows.Forms.MenuItem TrackerMenuItem;
        private byte dimmedLevel = 100;
        private System.Windows.Forms.ImageList iconImageList;
        private int CacheTime = 900;
        private ImageList lightImageList;
        private int PortNum = 0;

        public MainForm(string[] args)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            foreach (string parm in args)
            {
                if (parm.ToUpper().StartsWith("/CACHE:"))
                {
                    DText p = new DText();
                    p.ATTRMARK = ":";
                    p[0] = parm;
                    try
                    {
                        CacheTime = int.Parse(p[2]);
                    }
                    catch (Exception)
                    {
                    }
                }
                else if (parm.ToUpper() == "/DEBUG")
                {
                    OpenSource.Utilities.EventLogger.Enabled = true;
                    OpenSource.Utilities.EventLogger.ShowAll = true;
                    OpenSource.Utilities.InstanceTracker.Display();
                }
                else if (parm.ToUpper().StartsWith("/PORT:"))
                {
                    DText p = new DText();
                    p.ATTRMARK = ":";
                    p[0] = parm;
                    try
                    {
                        PortNum = int.Parse(p[2]);
                    }
                    catch (Exception)
                    {
                    }
                }
            }


            upnpLightDevice = UPnPDevice.CreateRootDevice(CacheTime, 1, "web\\");
            upnpLightDevice.Icon = iconImageList.Images[0];
            upnpLightDevice.HasPresentation = true;
            upnpLightDevice.PresentationURL = "/";
            upnpLightDevice.FriendlyName = this.Text + " (" + System.Windows.Forms.SystemInformation.ComputerName + ")";
            upnpLightDevice.Manufacturer = "OpenSource";
            upnpLightDevice.ManufacturerURL = "http://opentools.homeip.net";
            upnpLightDevice.ModelName = "Network Light Bulb";
            upnpLightDevice.ModelDescription = "Software Emulated Light Bulb";
            upnpLightDevice.ModelURL = new Uri("http://opentools.homeip.net");
            upnpLightDevice.ModelNumber = "XPC-L1";
            upnpLightDevice.StandardDeviceType = "DimmableLight";
            upnpLightDevice.UniqueDeviceName = System.Guid.NewGuid().ToString();

            // Switch Power
            upnpLightService = new UPnPService(1, "SwitchPower.0001", "SwitchPower", true, this);
            upnpLightService.AddMethod("SetTarget");
            upnpLightService.AddMethod("GetTarget");
            upnpLightService.AddMethod("GetStatus");
            
            UPnPStateVariable upnpStatusVar = new UPnPStateVariable("Status", typeof(bool), true);
            upnpStatusVar.AddAssociation("GetStatus", "ResultStatus");
            upnpStatusVar.Value = false;
            upnpLightService.AddStateVariable(upnpStatusVar);
            UPnPStateVariable upnpTargetVar = new UPnPStateVariable("Target", typeof(bool), false);
            upnpTargetVar.AddAssociation("SetTarget", "newTargetValue");
            upnpTargetVar.AddAssociation("GetTarget", "newTargetValue");
            upnpTargetVar.Value = false;
            upnpLightService.AddStateVariable(upnpTargetVar);

            // Dimmable device
            upnpDimmerService = new UPnPService(1, "Dimming.0001", "Dimming", true, this);
            upnpDimmerService.AddMethod("SetLoadLevelTarget");
            upnpDimmerService.AddMethod("GetLoadLevelTarget");
            upnpDimmerService.AddMethod("GetLoadLevelStatus");
            upnpDimmerService.AddMethod("GetMinLevel");

            UPnPStateVariable upnpLevelTargetVar = new UPnPStateVariable("LoadLevelTarget", typeof(byte), false);
            upnpLevelTargetVar.AddAssociation("SetLoadLevelTarget", "NewLoadLevelTarget");
            upnpLevelTargetVar.AddAssociation("GetLoadLevelTarget", "NewLoadLevelTarget");
            upnpLevelTargetVar.Value = (byte)100;
            upnpLevelTargetVar.SetRange((byte)0, (byte)100, null);
            upnpDimmerService.AddStateVariable(upnpLevelTargetVar);
            UPnPStateVariable upnpLevelStatusVar = new UPnPStateVariable("LoadLevelStatus", typeof(byte), true);
            upnpLevelStatusVar.AddAssociation("GetLoadLevelStatus", "RetLoadLevelStatus");
            upnpLevelStatusVar.Value = (byte)100;
            upnpLevelStatusVar.SetRange((byte)0, (byte)100, null);
            upnpDimmerService.AddStateVariable(upnpLevelStatusVar);
            UPnPStateVariable upnpMinLevelVar = new UPnPStateVariable("MinLevel", typeof(byte), false);
            upnpMinLevelVar.AddAssociation("GetMinLevel", "MinLevel");
            upnpMinLevelVar.Value = (byte)0;
            upnpDimmerService.AddStateVariable(upnpMinLevelVar);

            // Add Services
            upnpLightDevice.AddService(upnpLightService);
            upnpLightDevice.AddService(upnpDimmerService);
        }

        public void SetTarget(bool newTargetValue)
        {
            if (lightState == newTargetValue) return;
            upnpLightService.SetStateVariable("Target", newTargetValue);
            upnpLightService.SetStateVariable("Status", newTargetValue);
            lightState = newTargetValue;
            menuItem1.Checked = newTargetValue;
            if (lightState == false) SetLightLevel(0);
            if (lightState == true) SetLightLevel(dimmedLevel);
        }

        public void GetTarget(out bool newTargetValue)
        {
            newTargetValue = lightState;
        }
        
        public void GetStatus(out bool ResultStatus)
        {
            ResultStatus = lightState;
        }

        public void SetLoadLevelTarget(byte NewLoadLevelTarget)
        {
            dimmedLevel = NewLoadLevelTarget;
            upnpDimmerService.SetStateVariable("LoadLevelStatus", NewLoadLevelTarget);
            if (lightState == true) SetLightLevel(dimmedLevel);
        }

        public void GetLoadLevelTarget(out byte NewLoadLevelTarget)
        {
            NewLoadLevelTarget = (byte)dimmedLevel;
        }

        public void GetLoadLevelStatus(out byte RetLoadLevelStatus)
        {
            RetLoadLevelStatus = (byte)dimmedLevel;
        }

        public void GetMinLevel(out byte MinLevel)
        {
            MinLevel = 0;
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
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.contextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.RaiseDimMenuItem = new System.Windows.Forms.MenuItem();
            this.LowerDimMenuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.TrackerMenuItem = new System.Windows.Forms.MenuItem();
            this.iconImageList = new System.Windows.Forms.ImageList(this.components);
            this.lightImageList = new System.Windows.Forms.ImageList(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.ContextMenu = this.contextMenu;
            resources.ApplyResources(this.pictureBox, "pictureBox");
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.TabStop = false;
            this.pictureBox.DoubleClick += new System.EventHandler(this.menuItem1_Click);
            // 
            // contextMenu
            // 
            this.contextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem2,
            this.RaiseDimMenuItem,
            this.LowerDimMenuItem6,
            this.menuItem4,
            this.TrackerMenuItem});
            // 
            // menuItem1
            // 
            this.menuItem1.DefaultItem = true;
            this.menuItem1.Index = 0;
            resources.ApplyResources(this.menuItem1, "menuItem1");
            this.menuItem1.Click += new System.EventHandler(this.menuItem1_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 1;
            resources.ApplyResources(this.menuItem2, "menuItem2");
            // 
            // RaiseDimMenuItem
            // 
            this.RaiseDimMenuItem.Index = 2;
            resources.ApplyResources(this.RaiseDimMenuItem, "RaiseDimMenuItem");
            this.RaiseDimMenuItem.Click += new System.EventHandler(this.RaiseDimMenuItem_Click);
            // 
            // LowerDimMenuItem6
            // 
            this.LowerDimMenuItem6.Index = 3;
            resources.ApplyResources(this.LowerDimMenuItem6, "LowerDimMenuItem6");
            this.LowerDimMenuItem6.Click += new System.EventHandler(this.LowerDimMenuItem6_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 4;
            resources.ApplyResources(this.menuItem4, "menuItem4");
            // 
            // TrackerMenuItem
            // 
            this.TrackerMenuItem.Index = 5;
            resources.ApplyResources(this.TrackerMenuItem, "TrackerMenuItem");
            this.TrackerMenuItem.Click += new System.EventHandler(this.TrackerMenuItem_Click);
            // 
            // iconImageList
            // 
            this.iconImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("iconImageList.ImageStream")));
            this.iconImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.iconImageList.Images.SetKeyName(0, "");
            // 
            // lightImageList
            // 
            this.lightImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("lightImageList.ImageStream")));
            this.lightImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.lightImageList.Images.SetKeyName(0, "lamp-0.jpg");
            this.lightImageList.Images.SetKeyName(1, "lamp-10.jpg");
            this.lightImageList.Images.SetKeyName(2, "lamp-20.jpg");
            this.lightImageList.Images.SetKeyName(3, "lamp-30.jpg");
            this.lightImageList.Images.SetKeyName(4, "lamp-40.jpg");
            this.lightImageList.Images.SetKeyName(5, "lamp-50.jpg");
            this.lightImageList.Images.SetKeyName(6, "lamp-60.jpg");
            this.lightImageList.Images.SetKeyName(7, "lamp-70.jpg");
            this.lightImageList.Images.SetKeyName(8, "lamp-80.jpg");
            this.lightImageList.Images.SetKeyName(9, "lamp-90.jpg");
            this.lightImageList.Images.SetKeyName(10, "lamp-100.jpg");
            // 
            // MainForm
            // 
            resources.ApplyResources(this, "$this");
            this.BackColor = System.Drawing.Color.White;
            this.Controls.Add(this.pictureBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.Load += new System.EventHandler(this.OnLoad);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
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

        private void menuItem1_Click(object sender, System.EventArgs e)
        {
            lightState = !lightState;
            //upnpLightService.SetStateVariable("TestVariable",null);
            upnpLightService.SetStateVariable("Target", lightState);
            upnpLightService.SetStateVariable("Status", lightState);
            menuItem1.Checked = lightState;
            if (lightState == false) SetLightLevel(0);
            if (lightState == true) SetLightLevel(dimmedLevel);
        }

        private void SetLightLevel(int level)
        {
            if (level < 0) level = 0;
            if (level > 100) level = 100;
            pictureBox.Image = lightImageList.Images[level / 10];
        }

        private void RaiseDimMenuItem_Click(object sender, System.EventArgs e)
        {
            dimmedLevel += 20;
            if (dimmedLevel > 100) dimmedLevel = 100;
            upnpDimmerService.SetStateVariable("LoadLevelStatus", dimmedLevel);
            if (lightState == true) SetLightLevel(dimmedLevel);
        }

        private void LowerDimMenuItem6_Click(object sender, System.EventArgs e)
        {
            if (dimmedLevel < 20) dimmedLevel = 0; else dimmedLevel -= 20;
            upnpDimmerService.SetStateVariable("LoadLevelStatus", dimmedLevel);
            if (lightState == true) SetLightLevel(dimmedLevel);
        }

        private void TrackerMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Network Light");
        }

        private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "Network Light");
        }

        private void OnLoad(object sender, System.EventArgs e)
        {
            upnpLightDevice.StartDevice(PortNum);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            upnpLightDevice.StopDevice();
            upnpLightDevice = null;
        }
    }
}
