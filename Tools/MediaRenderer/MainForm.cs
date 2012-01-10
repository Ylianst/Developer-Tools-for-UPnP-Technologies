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
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;
using OpenSource.UPnP.AV.RENDERER.Device;

namespace UPnPRenderer
{
	/// <summary>
	/// Summary description for MainForm.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		public delegate void FormCreator(AVRenderer r, AVConnection c);
		private UPnPDevice device;
		private AVRenderer r;
		private int MaxConnections = 0;
		private Hashtable ConnectionTable = new Hashtable();
		private System.Windows.Forms.TextBox InfoStringBox;
		private System.Windows.Forms.MainMenu mainMenu;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItem16;
        private System.Windows.Forms.Label label1;
		private System.Windows.Forms.MenuItem startMenuItem;
		private System.Windows.Forms.MenuItem exitMenuItem;
		private System.Windows.Forms.MenuItem pfcNoneMenuItem;
		private System.Windows.Forms.MenuItem pfc1MenuItem;
		private System.Windows.Forms.MenuItem pfc2MenuItem;
		private System.Windows.Forms.MenuItem pfc5MenuItem;
		private System.Windows.Forms.MenuItem pfc10MenuItem;
		private System.Windows.Forms.MenuItem pfc100MenuItem;
		private System.Windows.Forms.MenuItem supportNextContentUriMenuItem;
		private System.Windows.Forms.MenuItem pfcMenuItem;
		private System.Windows.Forms.MenuItem supportRecordMenuItem;
		private System.Windows.Forms.MenuItem supportRecordQualityMenuItem;
        private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem helpMenuItem;
        private System.Windows.Forms.MenuItem menuItem6;
        private IContainer components;

		public MainForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			InfoStringBox.Text = "http-get:*:audio/mpegurl:*\r\nhttp-get:*:audio/mp3:*\r\nhttp-get:*:audio/mpeg:*\r\nhttp-get:*:audio/x-ms-wma:*\r\nhttp-get:*:audio/wma:*\r\nhttp-get:*:audio/mpeg3:*\r\nhttp-get:*:video/x-ms-wmv:*\r\nhttp-get:*:video/x-ms-asf:*\r\nhttp-get:*:video/x-ms-avi:*\r\nhttp-get:*:video/mpeg:*";
			InfoStringBox.SelectionStart = 0;
			InfoStringBox.SelectionLength = 0;

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
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
            this.InfoStringBox = new System.Windows.Forms.TextBox();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.startMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.pfcMenuItem = new System.Windows.Forms.MenuItem();
            this.pfcNoneMenuItem = new System.Windows.Forms.MenuItem();
            this.pfc1MenuItem = new System.Windows.Forms.MenuItem();
            this.pfc2MenuItem = new System.Windows.Forms.MenuItem();
            this.pfc5MenuItem = new System.Windows.Forms.MenuItem();
            this.pfc10MenuItem = new System.Windows.Forms.MenuItem();
            this.pfc100MenuItem = new System.Windows.Forms.MenuItem();
            this.supportNextContentUriMenuItem = new System.Windows.Forms.MenuItem();
            this.supportRecordMenuItem = new System.Windows.Forms.MenuItem();
            this.supportRecordQualityMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // InfoStringBox
            // 
            this.InfoStringBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.InfoStringBox.Location = new System.Drawing.Point(12, 28);
            this.InfoStringBox.Multiline = true;
            this.InfoStringBox.Name = "InfoStringBox";
            this.InfoStringBox.Size = new System.Drawing.Size(274, 245);
            this.InfoStringBox.TabIndex = 9;
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem3,
            this.menuItem2});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.startMenuItem,
            this.menuItem16,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            // 
            // startMenuItem
            // 
            this.startMenuItem.Index = 0;
            this.startMenuItem.Text = "Start AV Renderer";
            this.startMenuItem.Click += new System.EventHandler(this.startMenuItem_Click);
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 1;
            this.menuItem16.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 2;
            this.exitMenuItem.Text = "E&xit";
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 1;
            this.menuItem3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.pfcMenuItem,
            this.supportNextContentUriMenuItem,
            this.supportRecordMenuItem,
            this.supportRecordQualityMenuItem});
            this.menuItem3.Text = "&Support";
            // 
            // pfcMenuItem
            // 
            this.pfcMenuItem.Index = 0;
            this.pfcMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.pfcNoneMenuItem,
            this.pfc1MenuItem,
            this.pfc2MenuItem,
            this.pfc5MenuItem,
            this.pfc10MenuItem,
            this.pfc100MenuItem});
            this.pfcMenuItem.Text = "Prepare for Connection";
            // 
            // pfcNoneMenuItem
            // 
            this.pfcNoneMenuItem.Checked = true;
            this.pfcNoneMenuItem.Index = 0;
            this.pfcNoneMenuItem.Text = "Not Supported";
            this.pfcNoneMenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // pfc1MenuItem
            // 
            this.pfc1MenuItem.Index = 1;
            this.pfc1MenuItem.Text = "1 Instance";
            this.pfc1MenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // pfc2MenuItem
            // 
            this.pfc2MenuItem.Index = 2;
            this.pfc2MenuItem.Text = "2 Instances";
            this.pfc2MenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // pfc5MenuItem
            // 
            this.pfc5MenuItem.Index = 3;
            this.pfc5MenuItem.Text = "5 Instances";
            this.pfc5MenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // pfc10MenuItem
            // 
            this.pfc10MenuItem.Index = 4;
            this.pfc10MenuItem.Text = "10 Instances";
            this.pfc10MenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // pfc100MenuItem
            // 
            this.pfc100MenuItem.Index = 5;
            this.pfc100MenuItem.Text = "100 Instances";
            this.pfc100MenuItem.Click += new System.EventHandler(this.pfcNoneMenuItem_Click);
            // 
            // supportNextContentUriMenuItem
            // 
            this.supportNextContentUriMenuItem.Index = 1;
            this.supportNextContentUriMenuItem.Text = "Next Content URI";
            this.supportNextContentUriMenuItem.Click += new System.EventHandler(this.supportNextContentUriMenuItem_Click);
            // 
            // supportRecordMenuItem
            // 
            this.supportRecordMenuItem.Index = 2;
            this.supportRecordMenuItem.Text = "Record";
            this.supportRecordMenuItem.Click += new System.EventHandler(this.supportRecordMenuItem_Click);
            // 
            // supportRecordQualityMenuItem
            // 
            this.supportRecordQualityMenuItem.Index = 3;
            this.supportRecordQualityMenuItem.Text = "Record Quality Mode";
            this.supportRecordQualityMenuItem.Click += new System.EventHandler(this.supportRecordQualityMenuItem_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 2;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem6,
            this.menuItem4});
            this.menuItem2.Text = "&Help";
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
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.Text = "&Show Debug Information";
            this.menuItem4.Click += new System.EventHandler(this.menuItem4_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(9, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(128, 16);
            this.label1.TabIndex = 13;
            this.label1.Text = "Supported Mime Types";
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(298, 285);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.InfoStringBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "AV Media Renderer";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.MainForm_HelpRequested);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		private void ClosedConnectionSink(AVRenderer sender, AVConnection c)
		{
			RendererForm f = (RendererForm)ConnectionTable[c];
			ConnectionTable.Remove(c);
			f.Close();
		}

		private void NewConnectionSink(AVRenderer sender, AVConnection c)
		{
			object[] args = new Object[2];
			args[0] = sender;
			args[1] = c;
			Invoke(new FormCreator(FormSink),args);
		}

		private void FormSink(AVRenderer r, AVConnection c)
		{
			RendererForm f = new RendererForm(c);
			ConnectionTable[c] = f;
			f.Show();
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
		}

		private void DvSink(UPnPSmartControlPoint sender, UPnPDevice Dv)
		{
			
		}

		private void DebugButton_Click(object sender, System.EventArgs e)
		{
		
		}

		private void supportNextContentUriMenuItem_Click(object sender, System.EventArgs e)
		{
			supportNextContentUriMenuItem.Checked = !supportNextContentUriMenuItem.Checked;
		}

		private void pfcNoneMenuItem_Click(object sender, System.EventArgs e)
		{
			foreach (MenuItem i in pfcMenuItem.MenuItems) {i.Checked = false;}
			((MenuItem)sender).Checked = true;

			if (sender == pfcNoneMenuItem)	MaxConnections = 0;
			if (sender == pfc1MenuItem)		MaxConnections = 1;
			if (sender == pfc2MenuItem)		MaxConnections = 2;
			if (sender == pfc5MenuItem)		MaxConnections = 5;
			if (sender == pfc10MenuItem)	MaxConnections = 10;
			if (sender == pfc100MenuItem)	MaxConnections = 100;
		}

		public void NullMethod()
		{
		}

		private void startMenuItem_Click(object sender, System.EventArgs e)
		{
			startMenuItem.Enabled = false;
			foreach (MenuItem i in pfcMenuItem.MenuItems) {i.Enabled = false;}
			foreach (MenuItem i in menuItem3.MenuItems) {i.Enabled = false;}
			InfoStringBox.Enabled = false;

			device = UPnPDevice.CreateRootDevice(900,1,"");
			device.UniqueDeviceName = Guid.NewGuid().ToString();
			device.StandardDeviceType = "MediaRenderer";
			device.FriendlyName = "Media Renderer (" + System.Net.Dns.GetHostName() + ")";
			device.HasPresentation = false;

			device.Manufacturer = "OpenSource";
			device.ManufacturerURL = "http://opentools.homeip.net/";
			device.PresentationURL = "/";
			device.HasPresentation = true;
			device.ModelName = "AV Renderer";
			device.ModelDescription = "Media Renderer Device";
            device.ModelURL = new Uri("http://opentools.homeip.net/");
			
			UPnPService ts = new UPnPService(1, "EmptyService", "EmptyService", true, this);
			ts.AddMethod("NullMethod");
			//device.AddService(ts);


			DText p = new DText();
			p.ATTRMARK = "\r\n";
			p[0] = this.InfoStringBox.Text;
			int len = p.DCOUNT();
			ProtocolInfoString[] istring = new ProtocolInfoString[len];
			for(int i=1;i<=len;++i)
			{
				istring[i-1] = new ProtocolInfoString(p[i]);
			}
			r = new AVRenderer(MaxConnections, istring, new AVRenderer.ConnectionHandler(NewConnectionSink));

			r.OnClosedConnection += new AVRenderer.ConnectionHandler(ClosedConnectionSink);
		
			if (supportRecordMenuItem.Checked == false) 
			{
				r.AVT.RemoveAction_Record();
			}
			
			if (supportRecordQualityMenuItem.Checked == false) 
			{
				r.AVT.RemoveAction_SetRecordQualityMode();
			}

			if (supportNextContentUriMenuItem.Checked == false)
			{
				r.AVT.RemoveAction_SetNextAVTransportURI();
			}

			if (MaxConnections == 0)
			{
				r.Manager.RemoveAction_PrepareForConnection();
				r.Manager.RemoveAction_ConnectionComplete();
			}

			r.AVT.GetUPnPService().GetStateVariableObject("CurrentPlayMode").AllowedStringValues = new String[3]{"NORMAL","REPEAT_ALL","INTRO"};

			r.Control.GetUPnPService().GetStateVariableObject("A_ARG_TYPE_Channel").AllowedStringValues = new String[3]{"Master","LF","RF"};
			r.Control.GetUPnPService().GetStateVariableObject("RedVideoBlackLevel").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("GreenVideoBlackLevel").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("BlueVideoBlackLevel").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("RedVideoGain").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("GreenVideoGain").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("BlueVideoGain").SetRange((ushort)0,(ushort)100,(ushort)1);
			
			r.Control.GetUPnPService().GetStateVariableObject("Brightness").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("Contrast").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("Sharpness").SetRange((ushort)0,(ushort)100,(ushort)1);
			r.Control.GetUPnPService().GetStateVariableObject("Volume").SetRange((UInt16)0,(UInt16)100,(ushort)1);

			device.AddService(r.Control);
			device.AddService(r.AVT);
			device.AddService(r.Manager);

			//device.AddDevice(r);

			device.StartDevice();
			
			//r.Start();		
		}

		private void supportRecordMenuItem_Click(object sender, System.EventArgs e)
		{
			supportRecordMenuItem.Checked = !supportRecordMenuItem.Checked;
		}

		private void supportRecordQualityMenuItem_Click(object sender, System.EventArgs e)
		{
			supportRecordQualityMenuItem.Checked = !supportRecordQualityMenuItem.Checked;
		}

		private void menuItem4_Click(object sender, System.EventArgs e)
		{
            OpenSource.Utilities.InstanceTracker.Display();
		}

		private void helpMenuItem_Click(object sender, System.EventArgs e)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaRenderer");
		}

		private void MainForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"ToolsHelp.chm",HelpNavigator.KeywordIndex,"AV MediaRenderer");
		}

        private void MainForm_Load(object sender, EventArgs e)
        {
            //startMenuItem_Click(this, null);
        }

	}
}
