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
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnpSpy
{
	/// <summary>
	/// Summary description for MethodInvoke.
	/// </summary>
	public class MethodInvoke : System.Windows.Forms.Form
	{
		//protected Hashtable ArgTable = Hashtable.Synchronized(new Hashtable());
		public  string args;
		public  UPnPArgument[] UPnPArgs;
		private UPnPAction action;
		private UPnPService service;

		private UPnPServiceWatcher spy;

		private DateTime invokeTime = DateTime.Now;
		private System.Windows.Forms.StatusBar statusBar;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.PictureBox pictureBox4;
		private System.Windows.Forms.Label actionLabel;
		private System.Windows.Forms.Label serviceLabel;
		private System.Windows.Forms.Label deviceLabel;
		private System.Windows.Forms.PictureBox pictureBox3;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Button invokeButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private UPnpArgumentControl[] argControlList;
		private System.Windows.Forms.Panel argPanel;
		private System.Windows.Forms.TextBox DebugTextBox;
		private System.Windows.Forms.ContextMenu invokeContextMenu;
		private System.Windows.Forms.MenuItem invokeMenuItem;
		private System.Windows.Forms.MenuItem viewArgsMenuItem;
		private System.Windows.Forms.MenuItem viewPacketsMenuItem;
		private System.Windows.Forms.MenuItem autoInvokeMenuItem;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem PreValidateMenuItem;

		public MethodInvoke(UPnPAction action, UPnPService service)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			this.action = action;
			this.service = service;

			this.Text += " - " + action.Name;
			actionLabel.Text = action.Name;
			serviceLabel.Text = service.ServiceID;
			deviceLabel.Text = service.ParentDevice.FriendlyName;

			if (action.ArgumentList.Length>0)
			{
				argControlList = new UPnpArgumentControl[action.ArgumentList.Length];

				for (int z=0;z<action.ArgumentList.Length;++z)
				{
//					if (action.ArgumentList[z].IsReturnValue == true) {returnArgPresent = true;}
					argControlList[z] = new UPnpArgumentControl(action.ArgumentList[z]);
					argControlList[z].Dock = System.Windows.Forms.DockStyle.Top;
					/*
					if (action.ArgumentList[z].RelatedStateVar.ValueType == "string") 
					{
						argControlList[z].Height = 60;
					}
					*/
					argPanel.Controls.Add(argControlList[z]);
					argPanel.Controls.SetChildIndex(argControlList[z],0);

					Splitter splitter = new Splitter();
					splitter.Height = 4;
					splitter.MinExtra = 0;
					splitter.MinSize = 32;
					splitter.BackColor = Color.Gray;
					splitter.Dock = System.Windows.Forms.DockStyle.Top;
					argPanel.Controls.Add(splitter);
					argPanel.Controls.SetChildIndex(splitter,0);
				}
			}

			service.OnInvokeError += new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError);
			service.OnInvokeResponse += new UPnPService.UPnPServiceInvokeHandler(HandleInvoke);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MethodInvoke));
			this.statusBar = new System.Windows.Forms.StatusBar();
			this.panel1 = new System.Windows.Forms.Panel();
			this.invokeButton = new System.Windows.Forms.Button();
			this.pictureBox4 = new System.Windows.Forms.PictureBox();
			this.actionLabel = new System.Windows.Forms.Label();
			this.serviceLabel = new System.Windows.Forms.Label();
			this.deviceLabel = new System.Windows.Forms.Label();
			this.pictureBox3 = new System.Windows.Forms.PictureBox();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.argPanel = new System.Windows.Forms.Panel();
			this.DebugTextBox = new System.Windows.Forms.TextBox();
			this.invokeContextMenu = new System.Windows.Forms.ContextMenu();
			this.invokeMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.autoInvokeMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem6 = new System.Windows.Forms.MenuItem();
			this.viewArgsMenuItem = new System.Windows.Forms.MenuItem();
			this.viewPacketsMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.PreValidateMenuItem = new System.Windows.Forms.MenuItem();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// statusBar
			// 
			this.statusBar.AccessibleDescription = ((string)(resources.GetObject("statusBar.AccessibleDescription")));
			this.statusBar.AccessibleName = ((string)(resources.GetObject("statusBar.AccessibleName")));
			this.statusBar.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("statusBar.Anchor")));
			this.statusBar.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("statusBar.BackgroundImage")));
			this.statusBar.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("statusBar.Dock")));
			this.statusBar.Enabled = ((bool)(resources.GetObject("statusBar.Enabled")));
			this.statusBar.Font = ((System.Drawing.Font)(resources.GetObject("statusBar.Font")));
			this.statusBar.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("statusBar.ImeMode")));
			this.statusBar.Location = ((System.Drawing.Point)(resources.GetObject("statusBar.Location")));
			this.statusBar.Name = "statusBar";
			this.statusBar.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("statusBar.RightToLeft")));
			this.statusBar.Size = ((System.Drawing.Size)(resources.GetObject("statusBar.Size")));
			this.statusBar.TabIndex = ((int)(resources.GetObject("statusBar.TabIndex")));
			this.statusBar.Text = resources.GetString("statusBar.Text");
			this.statusBar.Visible = ((bool)(resources.GetObject("statusBar.Visible")));
			// 
			// panel1
			// 
			this.panel1.AccessibleDescription = ((string)(resources.GetObject("panel1.AccessibleDescription")));
			this.panel1.AccessibleName = ((string)(resources.GetObject("panel1.AccessibleName")));
			this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("panel1.Anchor")));
			this.panel1.AutoScroll = ((bool)(resources.GetObject("panel1.AutoScroll")));
			this.panel1.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("panel1.AutoScrollMargin")));
			this.panel1.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("panel1.AutoScrollMinSize")));
			this.panel1.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("panel1.BackgroundImage")));
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.invokeButton,
																				 this.pictureBox4,
																				 this.actionLabel,
																				 this.serviceLabel,
																				 this.deviceLabel,
																				 this.pictureBox3,
																				 this.pictureBox2,
																				 this.pictureBox1});
			this.panel1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("panel1.Dock")));
			this.panel1.Enabled = ((bool)(resources.GetObject("panel1.Enabled")));
			this.panel1.Font = ((System.Drawing.Font)(resources.GetObject("panel1.Font")));
			this.panel1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("panel1.ImeMode")));
			this.panel1.Location = ((System.Drawing.Point)(resources.GetObject("panel1.Location")));
			this.panel1.Name = "panel1";
			this.panel1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("panel1.RightToLeft")));
			this.panel1.Size = ((System.Drawing.Size)(resources.GetObject("panel1.Size")));
			this.panel1.TabIndex = ((int)(resources.GetObject("panel1.TabIndex")));
			this.panel1.Text = resources.GetString("panel1.Text");
			this.panel1.Visible = ((bool)(resources.GetObject("panel1.Visible")));
			// 
			// invokeButton
			// 
			this.invokeButton.AccessibleDescription = ((string)(resources.GetObject("invokeButton.AccessibleDescription")));
			this.invokeButton.AccessibleName = ((string)(resources.GetObject("invokeButton.AccessibleName")));
			this.invokeButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("invokeButton.Anchor")));
			this.invokeButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("invokeButton.BackgroundImage")));
			this.invokeButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("invokeButton.Dock")));
			this.invokeButton.Enabled = ((bool)(resources.GetObject("invokeButton.Enabled")));
			this.invokeButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("invokeButton.FlatStyle")));
			this.invokeButton.Font = ((System.Drawing.Font)(resources.GetObject("invokeButton.Font")));
			this.invokeButton.Image = ((System.Drawing.Image)(resources.GetObject("invokeButton.Image")));
			this.invokeButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("invokeButton.ImageAlign")));
			this.invokeButton.ImageIndex = ((int)(resources.GetObject("invokeButton.ImageIndex")));
			this.invokeButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("invokeButton.ImeMode")));
			this.invokeButton.Location = ((System.Drawing.Point)(resources.GetObject("invokeButton.Location")));
			this.invokeButton.Name = "invokeButton";
			this.invokeButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("invokeButton.RightToLeft")));
			this.invokeButton.Size = ((System.Drawing.Size)(resources.GetObject("invokeButton.Size")));
			this.invokeButton.TabIndex = ((int)(resources.GetObject("invokeButton.TabIndex")));
			this.invokeButton.Text = resources.GetString("invokeButton.Text");
			this.invokeButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("invokeButton.TextAlign")));
			this.invokeButton.Visible = ((bool)(resources.GetObject("invokeButton.Visible")));
			this.invokeButton.Click += new System.EventHandler(this.invokeButton_Click);
			// 
			// pictureBox4
			// 
			this.pictureBox4.AccessibleDescription = ((string)(resources.GetObject("pictureBox4.AccessibleDescription")));
			this.pictureBox4.AccessibleName = ((string)(resources.GetObject("pictureBox4.AccessibleName")));
			this.pictureBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("pictureBox4.Anchor")));
			this.pictureBox4.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictureBox4.BackgroundImage")));
			this.pictureBox4.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("pictureBox4.Dock")));
			this.pictureBox4.Enabled = ((bool)(resources.GetObject("pictureBox4.Enabled")));
			this.pictureBox4.Font = ((System.Drawing.Font)(resources.GetObject("pictureBox4.Font")));
			this.pictureBox4.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox4.Image")));
			this.pictureBox4.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("pictureBox4.ImeMode")));
			this.pictureBox4.Location = ((System.Drawing.Point)(resources.GetObject("pictureBox4.Location")));
			this.pictureBox4.Name = "pictureBox4";
			this.pictureBox4.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("pictureBox4.RightToLeft")));
			this.pictureBox4.Size = ((System.Drawing.Size)(resources.GetObject("pictureBox4.Size")));
			this.pictureBox4.SizeMode = ((System.Windows.Forms.PictureBoxSizeMode)(resources.GetObject("pictureBox4.SizeMode")));
			this.pictureBox4.TabIndex = ((int)(resources.GetObject("pictureBox4.TabIndex")));
			this.pictureBox4.TabStop = false;
			this.pictureBox4.Text = resources.GetString("pictureBox4.Text");
			this.pictureBox4.Visible = ((bool)(resources.GetObject("pictureBox4.Visible")));
			// 
			// actionLabel
			// 
			this.actionLabel.AccessibleDescription = ((string)(resources.GetObject("actionLabel.AccessibleDescription")));
			this.actionLabel.AccessibleName = ((string)(resources.GetObject("actionLabel.AccessibleName")));
			this.actionLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("actionLabel.Anchor")));
			this.actionLabel.AutoSize = ((bool)(resources.GetObject("actionLabel.AutoSize")));
			this.actionLabel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("actionLabel.Dock")));
			this.actionLabel.Enabled = ((bool)(resources.GetObject("actionLabel.Enabled")));
			this.actionLabel.Font = ((System.Drawing.Font)(resources.GetObject("actionLabel.Font")));
			this.actionLabel.Image = ((System.Drawing.Image)(resources.GetObject("actionLabel.Image")));
			this.actionLabel.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("actionLabel.ImageAlign")));
			this.actionLabel.ImageIndex = ((int)(resources.GetObject("actionLabel.ImageIndex")));
			this.actionLabel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("actionLabel.ImeMode")));
			this.actionLabel.Location = ((System.Drawing.Point)(resources.GetObject("actionLabel.Location")));
			this.actionLabel.Name = "actionLabel";
			this.actionLabel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("actionLabel.RightToLeft")));
			this.actionLabel.Size = ((System.Drawing.Size)(resources.GetObject("actionLabel.Size")));
			this.actionLabel.TabIndex = ((int)(resources.GetObject("actionLabel.TabIndex")));
			this.actionLabel.Text = resources.GetString("actionLabel.Text");
			this.actionLabel.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("actionLabel.TextAlign")));
			this.actionLabel.UseMnemonic = false;
			this.actionLabel.Visible = ((bool)(resources.GetObject("actionLabel.Visible")));
			// 
			// serviceLabel
			// 
			this.serviceLabel.AccessibleDescription = ((string)(resources.GetObject("serviceLabel.AccessibleDescription")));
			this.serviceLabel.AccessibleName = ((string)(resources.GetObject("serviceLabel.AccessibleName")));
			this.serviceLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("serviceLabel.Anchor")));
			this.serviceLabel.AutoSize = ((bool)(resources.GetObject("serviceLabel.AutoSize")));
			this.serviceLabel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("serviceLabel.Dock")));
			this.serviceLabel.Enabled = ((bool)(resources.GetObject("serviceLabel.Enabled")));
			this.serviceLabel.Font = ((System.Drawing.Font)(resources.GetObject("serviceLabel.Font")));
			this.serviceLabel.Image = ((System.Drawing.Image)(resources.GetObject("serviceLabel.Image")));
			this.serviceLabel.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("serviceLabel.ImageAlign")));
			this.serviceLabel.ImageIndex = ((int)(resources.GetObject("serviceLabel.ImageIndex")));
			this.serviceLabel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("serviceLabel.ImeMode")));
			this.serviceLabel.Location = ((System.Drawing.Point)(resources.GetObject("serviceLabel.Location")));
			this.serviceLabel.Name = "serviceLabel";
			this.serviceLabel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("serviceLabel.RightToLeft")));
			this.serviceLabel.Size = ((System.Drawing.Size)(resources.GetObject("serviceLabel.Size")));
			this.serviceLabel.TabIndex = ((int)(resources.GetObject("serviceLabel.TabIndex")));
			this.serviceLabel.Text = resources.GetString("serviceLabel.Text");
			this.serviceLabel.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("serviceLabel.TextAlign")));
			this.serviceLabel.UseMnemonic = false;
			this.serviceLabel.Visible = ((bool)(resources.GetObject("serviceLabel.Visible")));
			// 
			// deviceLabel
			// 
			this.deviceLabel.AccessibleDescription = ((string)(resources.GetObject("deviceLabel.AccessibleDescription")));
			this.deviceLabel.AccessibleName = ((string)(resources.GetObject("deviceLabel.AccessibleName")));
			this.deviceLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("deviceLabel.Anchor")));
			this.deviceLabel.AutoSize = ((bool)(resources.GetObject("deviceLabel.AutoSize")));
			this.deviceLabel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("deviceLabel.Dock")));
			this.deviceLabel.Enabled = ((bool)(resources.GetObject("deviceLabel.Enabled")));
			this.deviceLabel.Font = ((System.Drawing.Font)(resources.GetObject("deviceLabel.Font")));
			this.deviceLabel.Image = ((System.Drawing.Image)(resources.GetObject("deviceLabel.Image")));
			this.deviceLabel.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("deviceLabel.ImageAlign")));
			this.deviceLabel.ImageIndex = ((int)(resources.GetObject("deviceLabel.ImageIndex")));
			this.deviceLabel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("deviceLabel.ImeMode")));
			this.deviceLabel.Location = ((System.Drawing.Point)(resources.GetObject("deviceLabel.Location")));
			this.deviceLabel.Name = "deviceLabel";
			this.deviceLabel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("deviceLabel.RightToLeft")));
			this.deviceLabel.Size = ((System.Drawing.Size)(resources.GetObject("deviceLabel.Size")));
			this.deviceLabel.TabIndex = ((int)(resources.GetObject("deviceLabel.TabIndex")));
			this.deviceLabel.Text = resources.GetString("deviceLabel.Text");
			this.deviceLabel.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("deviceLabel.TextAlign")));
			this.deviceLabel.UseMnemonic = false;
			this.deviceLabel.Visible = ((bool)(resources.GetObject("deviceLabel.Visible")));
			// 
			// pictureBox3
			// 
			this.pictureBox3.AccessibleDescription = ((string)(resources.GetObject("pictureBox3.AccessibleDescription")));
			this.pictureBox3.AccessibleName = ((string)(resources.GetObject("pictureBox3.AccessibleName")));
			this.pictureBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("pictureBox3.Anchor")));
			this.pictureBox3.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictureBox3.BackgroundImage")));
			this.pictureBox3.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("pictureBox3.Dock")));
			this.pictureBox3.Enabled = ((bool)(resources.GetObject("pictureBox3.Enabled")));
			this.pictureBox3.Font = ((System.Drawing.Font)(resources.GetObject("pictureBox3.Font")));
			this.pictureBox3.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox3.Image")));
			this.pictureBox3.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("pictureBox3.ImeMode")));
			this.pictureBox3.Location = ((System.Drawing.Point)(resources.GetObject("pictureBox3.Location")));
			this.pictureBox3.Name = "pictureBox3";
			this.pictureBox3.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("pictureBox3.RightToLeft")));
			this.pictureBox3.Size = ((System.Drawing.Size)(resources.GetObject("pictureBox3.Size")));
			this.pictureBox3.SizeMode = ((System.Windows.Forms.PictureBoxSizeMode)(resources.GetObject("pictureBox3.SizeMode")));
			this.pictureBox3.TabIndex = ((int)(resources.GetObject("pictureBox3.TabIndex")));
			this.pictureBox3.TabStop = false;
			this.pictureBox3.Text = resources.GetString("pictureBox3.Text");
			this.pictureBox3.Visible = ((bool)(resources.GetObject("pictureBox3.Visible")));
			// 
			// pictureBox2
			// 
			this.pictureBox2.AccessibleDescription = ((string)(resources.GetObject("pictureBox2.AccessibleDescription")));
			this.pictureBox2.AccessibleName = ((string)(resources.GetObject("pictureBox2.AccessibleName")));
			this.pictureBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("pictureBox2.Anchor")));
			this.pictureBox2.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictureBox2.BackgroundImage")));
			this.pictureBox2.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("pictureBox2.Dock")));
			this.pictureBox2.Enabled = ((bool)(resources.GetObject("pictureBox2.Enabled")));
			this.pictureBox2.Font = ((System.Drawing.Font)(resources.GetObject("pictureBox2.Font")));
			this.pictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox2.Image")));
			this.pictureBox2.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("pictureBox2.ImeMode")));
			this.pictureBox2.Location = ((System.Drawing.Point)(resources.GetObject("pictureBox2.Location")));
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("pictureBox2.RightToLeft")));
			this.pictureBox2.Size = ((System.Drawing.Size)(resources.GetObject("pictureBox2.Size")));
			this.pictureBox2.SizeMode = ((System.Windows.Forms.PictureBoxSizeMode)(resources.GetObject("pictureBox2.SizeMode")));
			this.pictureBox2.TabIndex = ((int)(resources.GetObject("pictureBox2.TabIndex")));
			this.pictureBox2.TabStop = false;
			this.pictureBox2.Text = resources.GetString("pictureBox2.Text");
			this.pictureBox2.Visible = ((bool)(resources.GetObject("pictureBox2.Visible")));
			// 
			// pictureBox1
			// 
			this.pictureBox1.AccessibleDescription = ((string)(resources.GetObject("pictureBox1.AccessibleDescription")));
			this.pictureBox1.AccessibleName = ((string)(resources.GetObject("pictureBox1.AccessibleName")));
			this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("pictureBox1.Anchor")));
			this.pictureBox1.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.BackgroundImage")));
			this.pictureBox1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("pictureBox1.Dock")));
			this.pictureBox1.Enabled = ((bool)(resources.GetObject("pictureBox1.Enabled")));
			this.pictureBox1.Font = ((System.Drawing.Font)(resources.GetObject("pictureBox1.Font")));
			this.pictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox1.Image")));
			this.pictureBox1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("pictureBox1.ImeMode")));
			this.pictureBox1.Location = ((System.Drawing.Point)(resources.GetObject("pictureBox1.Location")));
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("pictureBox1.RightToLeft")));
			this.pictureBox1.Size = ((System.Drawing.Size)(resources.GetObject("pictureBox1.Size")));
			this.pictureBox1.SizeMode = ((System.Windows.Forms.PictureBoxSizeMode)(resources.GetObject("pictureBox1.SizeMode")));
			this.pictureBox1.TabIndex = ((int)(resources.GetObject("pictureBox1.TabIndex")));
			this.pictureBox1.TabStop = false;
			this.pictureBox1.Text = resources.GetString("pictureBox1.Text");
			this.pictureBox1.Visible = ((bool)(resources.GetObject("pictureBox1.Visible")));
			// 
			// argPanel
			// 
			this.argPanel.AccessibleDescription = ((string)(resources.GetObject("argPanel.AccessibleDescription")));
			this.argPanel.AccessibleName = ((string)(resources.GetObject("argPanel.AccessibleName")));
			this.argPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("argPanel.Anchor")));
			this.argPanel.AutoScroll = ((bool)(resources.GetObject("argPanel.AutoScroll")));
			this.argPanel.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("argPanel.AutoScrollMargin")));
			this.argPanel.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("argPanel.AutoScrollMinSize")));
			this.argPanel.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("argPanel.BackgroundImage")));
			this.argPanel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("argPanel.Dock")));
			this.argPanel.Enabled = ((bool)(resources.GetObject("argPanel.Enabled")));
			this.argPanel.Font = ((System.Drawing.Font)(resources.GetObject("argPanel.Font")));
			this.argPanel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("argPanel.ImeMode")));
			this.argPanel.Location = ((System.Drawing.Point)(resources.GetObject("argPanel.Location")));
			this.argPanel.Name = "argPanel";
			this.argPanel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("argPanel.RightToLeft")));
			this.argPanel.Size = ((System.Drawing.Size)(resources.GetObject("argPanel.Size")));
			this.argPanel.TabIndex = ((int)(resources.GetObject("argPanel.TabIndex")));
			this.argPanel.Text = resources.GetString("argPanel.Text");
			this.argPanel.Visible = ((bool)(resources.GetObject("argPanel.Visible")));
			// 
			// DebugTextBox
			// 
			this.DebugTextBox.AccessibleDescription = ((string)(resources.GetObject("DebugTextBox.AccessibleDescription")));
			this.DebugTextBox.AccessibleName = ((string)(resources.GetObject("DebugTextBox.AccessibleName")));
			this.DebugTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("DebugTextBox.Anchor")));
			this.DebugTextBox.AutoSize = ((bool)(resources.GetObject("DebugTextBox.AutoSize")));
			this.DebugTextBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("DebugTextBox.BackgroundImage")));
			this.DebugTextBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("DebugTextBox.Dock")));
			this.DebugTextBox.Enabled = ((bool)(resources.GetObject("DebugTextBox.Enabled")));
			this.DebugTextBox.Font = ((System.Drawing.Font)(resources.GetObject("DebugTextBox.Font")));
			this.DebugTextBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("DebugTextBox.ImeMode")));
			this.DebugTextBox.Location = ((System.Drawing.Point)(resources.GetObject("DebugTextBox.Location")));
			this.DebugTextBox.MaxLength = ((int)(resources.GetObject("DebugTextBox.MaxLength")));
			this.DebugTextBox.Multiline = ((bool)(resources.GetObject("DebugTextBox.Multiline")));
			this.DebugTextBox.Name = "DebugTextBox";
			this.DebugTextBox.PasswordChar = ((char)(resources.GetObject("DebugTextBox.PasswordChar")));
			this.DebugTextBox.ReadOnly = true;
			this.DebugTextBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("DebugTextBox.RightToLeft")));
			this.DebugTextBox.ScrollBars = ((System.Windows.Forms.ScrollBars)(resources.GetObject("DebugTextBox.ScrollBars")));
			this.DebugTextBox.Size = ((System.Drawing.Size)(resources.GetObject("DebugTextBox.Size")));
			this.DebugTextBox.TabIndex = ((int)(resources.GetObject("DebugTextBox.TabIndex")));
			this.DebugTextBox.Text = resources.GetString("DebugTextBox.Text");
			this.DebugTextBox.TextAlign = ((System.Windows.Forms.HorizontalAlignment)(resources.GetObject("DebugTextBox.TextAlign")));
			this.DebugTextBox.Visible = ((bool)(resources.GetObject("DebugTextBox.Visible")));
			this.DebugTextBox.WordWrap = ((bool)(resources.GetObject("DebugTextBox.WordWrap")));
			// 
			// invokeContextMenu
			// 
			this.invokeContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							  this.invokeMenuItem,
																							  this.menuItem1,
																							  this.autoInvokeMenuItem,
																							  this.menuItem6,
																							  this.viewArgsMenuItem,
																							  this.viewPacketsMenuItem,
																							  this.menuItem2,
																							  this.PreValidateMenuItem});
			this.invokeContextMenu.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("invokeContextMenu.RightToLeft")));
			// 
			// invokeMenuItem
			// 
			this.invokeMenuItem.DefaultItem = true;
			this.invokeMenuItem.Enabled = ((bool)(resources.GetObject("invokeMenuItem.Enabled")));
			this.invokeMenuItem.Index = 0;
			this.invokeMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("invokeMenuItem.Shortcut")));
			this.invokeMenuItem.ShowShortcut = ((bool)(resources.GetObject("invokeMenuItem.ShowShortcut")));
			this.invokeMenuItem.Text = resources.GetString("invokeMenuItem.Text");
			this.invokeMenuItem.Visible = ((bool)(resources.GetObject("invokeMenuItem.Visible")));
			this.invokeMenuItem.Click += new System.EventHandler(this.invokeButton_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Enabled = ((bool)(resources.GetObject("menuItem1.Enabled")));
			this.menuItem1.Index = 1;
			this.menuItem1.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem1.Shortcut")));
			this.menuItem1.ShowShortcut = ((bool)(resources.GetObject("menuItem1.ShowShortcut")));
			this.menuItem1.Text = resources.GetString("menuItem1.Text");
			this.menuItem1.Visible = ((bool)(resources.GetObject("menuItem1.Visible")));
			// 
			// autoInvokeMenuItem
			// 
			this.autoInvokeMenuItem.Enabled = ((bool)(resources.GetObject("autoInvokeMenuItem.Enabled")));
			this.autoInvokeMenuItem.Index = 2;
			this.autoInvokeMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("autoInvokeMenuItem.Shortcut")));
			this.autoInvokeMenuItem.ShowShortcut = ((bool)(resources.GetObject("autoInvokeMenuItem.ShowShortcut")));
			this.autoInvokeMenuItem.Text = resources.GetString("autoInvokeMenuItem.Text");
			this.autoInvokeMenuItem.Visible = ((bool)(resources.GetObject("autoInvokeMenuItem.Visible")));
			this.autoInvokeMenuItem.Click += new System.EventHandler(this.autoInvokeMenuItem_Click);
			// 
			// menuItem6
			// 
			this.menuItem6.Enabled = ((bool)(resources.GetObject("menuItem6.Enabled")));
			this.menuItem6.Index = 3;
			this.menuItem6.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem6.Shortcut")));
			this.menuItem6.ShowShortcut = ((bool)(resources.GetObject("menuItem6.ShowShortcut")));
			this.menuItem6.Text = resources.GetString("menuItem6.Text");
			this.menuItem6.Visible = ((bool)(resources.GetObject("menuItem6.Visible")));
			// 
			// viewArgsMenuItem
			// 
			this.viewArgsMenuItem.Checked = true;
			this.viewArgsMenuItem.Enabled = ((bool)(resources.GetObject("viewArgsMenuItem.Enabled")));
			this.viewArgsMenuItem.Index = 4;
			this.viewArgsMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("viewArgsMenuItem.Shortcut")));
			this.viewArgsMenuItem.ShowShortcut = ((bool)(resources.GetObject("viewArgsMenuItem.ShowShortcut")));
			this.viewArgsMenuItem.Text = resources.GetString("viewArgsMenuItem.Text");
			this.viewArgsMenuItem.Visible = ((bool)(resources.GetObject("viewArgsMenuItem.Visible")));
			this.viewArgsMenuItem.Click += new System.EventHandler(this.viewArgsMenuItem_Click);
			// 
			// viewPacketsMenuItem
			// 
			this.viewPacketsMenuItem.Enabled = ((bool)(resources.GetObject("viewPacketsMenuItem.Enabled")));
			this.viewPacketsMenuItem.Index = 5;
			this.viewPacketsMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("viewPacketsMenuItem.Shortcut")));
			this.viewPacketsMenuItem.ShowShortcut = ((bool)(resources.GetObject("viewPacketsMenuItem.ShowShortcut")));
			this.viewPacketsMenuItem.Text = resources.GetString("viewPacketsMenuItem.Text");
			this.viewPacketsMenuItem.Visible = ((bool)(resources.GetObject("viewPacketsMenuItem.Visible")));
			this.viewPacketsMenuItem.Click += new System.EventHandler(this.viewPacketsMenuItem_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Enabled = ((bool)(resources.GetObject("menuItem2.Enabled")));
			this.menuItem2.Index = 6;
			this.menuItem2.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem2.Shortcut")));
			this.menuItem2.ShowShortcut = ((bool)(resources.GetObject("menuItem2.ShowShortcut")));
			this.menuItem2.Text = resources.GetString("menuItem2.Text");
			this.menuItem2.Visible = ((bool)(resources.GetObject("menuItem2.Visible")));
			// 
			// PreValidateMenuItem
			// 
			this.PreValidateMenuItem.Checked = true;
			this.PreValidateMenuItem.Enabled = ((bool)(resources.GetObject("PreValidateMenuItem.Enabled")));
			this.PreValidateMenuItem.Index = 7;
			this.PreValidateMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("PreValidateMenuItem.Shortcut")));
			this.PreValidateMenuItem.ShowShortcut = ((bool)(resources.GetObject("PreValidateMenuItem.ShowShortcut")));
			this.PreValidateMenuItem.Text = resources.GetString("PreValidateMenuItem.Text");
			this.PreValidateMenuItem.Visible = ((bool)(resources.GetObject("PreValidateMenuItem.Visible")));
			this.PreValidateMenuItem.Click += new System.EventHandler(this.PreValidateMenuItem_Click);
			// 
			// MethodInvoke
			// 
			this.AcceptButton = this.invokeButton;
			this.AccessibleDescription = ((string)(resources.GetObject("$this.AccessibleDescription")));
			this.AccessibleName = ((string)(resources.GetObject("$this.AccessibleName")));
			this.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("$this.Anchor")));
			this.AutoScaleBaseSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScaleBaseSize")));
			this.AutoScroll = ((bool)(resources.GetObject("$this.AutoScroll")));
			this.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMargin")));
			this.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMinSize")));
			this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
			this.ClientSize = ((System.Drawing.Size)(resources.GetObject("$this.ClientSize")));
			this.ContextMenu = this.invokeContextMenu;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.argPanel,
																		  this.DebugTextBox,
																		  this.panel1,
																		  this.statusBar});
			this.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("$this.Dock")));
			this.Enabled = ((bool)(resources.GetObject("$this.Enabled")));
			this.Font = ((System.Drawing.Font)(resources.GetObject("$this.Font")));
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("$this.ImeMode")));
			this.Location = ((System.Drawing.Point)(resources.GetObject("$this.Location")));
			this.MaximumSize = ((System.Drawing.Size)(resources.GetObject("$this.MaximumSize")));
			this.MinimumSize = ((System.Drawing.Size)(resources.GetObject("$this.MinimumSize")));
			this.Name = "MethodInvoke";
			this.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("$this.RightToLeft")));
			this.StartPosition = ((System.Windows.Forms.FormStartPosition)(resources.GetObject("$this.StartPosition")));
			this.Text = resources.GetString("$this.Text");
			this.Visible = ((bool)(resources.GetObject("$this.Visible")));
			this.Load += new System.EventHandler(this.MethodInvoke_Load);
			this.Closed += new System.EventHandler(this.MethodInvoke_Closed);
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void invokeButton_Click(object sender, System.EventArgs e)
		{
			DebugTextBox.Clear();
			UPnPArgs = null;
			ArrayList args = new ArrayList();

			if(argControlList!=null)
			{
				for(int i=0;i<argControlList.Length;++i)
				{
					if(argControlList[i].UPnPArgument.IsReturnValue==false)
					{
						if(argControlList[i].UPnPArgument.Direction=="in")
						{
							args.Add(new UPnPArgument(argControlList[i].UPnPArgument.Name,argControlList[i].ArgumentValue));
						}
						else
						{
							args.Add(new UPnPArgument(argControlList[i].UPnPArgument.Name,""));
						}
					}
				}
			}

			UPnPArgs = (UPnPArgument[])args.ToArray(typeof(UPnPArgument));
			(new UPnPDebugObject(service)).SetProperty("ValidationMode",PreValidateMenuItem.Checked);


			invokeTime = DateTime.Now;
			try
			{
				service.InvokeAsync(action.Name,UPnPArgs,
					null,
					new UPnPService.UPnPServiceInvokeHandler(HandleInvoke),
					new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError));
				statusBar.Text = "Connecting and processing invocation...";
			}
			catch(UPnPInvokeException ie)
			{
				statusBar.Text = "UPnPInvokeException: " + ie.Message;
			}
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		protected void HandleInvokeError(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object Handle)
		{
			if (sender != service) return;
			if (MethodName != action.Name) return;
			TimeSpan invokeSpan = DateTime.Now.Subtract(invokeTime);

			string timeStr;
			if (invokeSpan.TotalMilliseconds >= 1000) 
			{
				timeStr = invokeSpan.Seconds + "." +  invokeSpan.Milliseconds + "sec";
			} 
			else 
			{
				timeStr = invokeSpan.Milliseconds + "ms";
			}

			if(e.UPNP==null)
			{
				statusBar.Text = "Invocation error (" + timeStr + "): " + e.ToString();
			}
			else
			{
				statusBar.Text = "Invocation Error Code " + e.UPNP.ErrorCode.ToString() + " (" + timeStr + "): " + e.UPNP.ErrorDescription;
			}
		}

		protected void HandleInvoke(UPnPService sender, string MethodName, UPnPArgument[] Args, object ReturnValue, object Handle)
		{
			this.Invoke(new UPnPService.UPnPServiceInvokeHandler(HandleInvokeEx),new object[5]{sender,MethodName,Args,ReturnValue, Handle});
		}

		protected void HandleInvokeEx(UPnPService sender, string MethodName, UPnPArgument[] Args, object ReturnValue, object Handle)
		{
			if (sender != service) return;
			if (MethodName != action.Name) return;

			TimeSpan invokeSpan = DateTime.Now.Subtract(invokeTime);

			string timeStr;
			if (invokeSpan.TotalMilliseconds >= 1000) 
			{
				timeStr = invokeSpan.Seconds + "." +  invokeSpan.Milliseconds + "sec";
			} 
			else 
			{
				timeStr = invokeSpan.Milliseconds + "ms";
			}


			for (int i=0;i<Args.Length;++i)
			{
				for (int j=0;j<argControlList.Length;j++) 
				{
					if (Args[i].Name == argControlList[j].UPnPArgument.Name) 
					{
						argControlList[j].ArgumentValue = Args[i].DataValue;
					} 
				}
			}

			if (ReturnValue != null)
			{
				for (int j=0;j<argControlList.Length;j++) 
				{
					if (argControlList[j].UPnPArgument.IsReturnValue == true) 
					{
						argControlList[j].ArgumentValue = ReturnValue;
					} 
				}
			}

			statusBar.Text = "Invocation complete (" + timeStr + "), waiting for next invocation arguments.";

			if (autoInvokeMenuItem.Checked == true) 
			{
				invokeButton_Click(null,null);
			}
		}

		private void MethodInvoke_Load(object sender, System.EventArgs e)
		{
			spy = new UPnPServiceWatcher(this.service,new UPnPServiceWatcher.SniffHandler(PacketCaptureSink));
		}

		private void MethodInvoke_Closed(object sender, System.EventArgs e)
		{		
			spy.OnSniff -= new UPnPServiceWatcher.SniffHandler(PacketCaptureSink);
			service.OnInvokeError -= new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError);
			service.OnInvokeResponse -= new UPnPService.UPnPServiceInvokeHandler(HandleInvoke);
		}

		private void PacketCaptureSink(UPnPServiceWatcher sender, byte[] raw, int offset, int length)
		{
			string packetText = new UTF8Encoding().GetString(raw,offset,length);
			//if (DebugTextBox.Text.Length != 0) DebugTextBox.Text += "\r\n\r\n\r\n";
			DebugTextBox.Text += packetText;
		}

		private void viewArgsMenuItem_Click(object sender, System.EventArgs e)
		{
			argPanel.Visible = true;
			DebugTextBox.Visible = false;
			viewArgsMenuItem.Checked = true;
			viewPacketsMenuItem.Checked = false;
		}

		private void viewPacketsMenuItem_Click(object sender, System.EventArgs e)
		{
			argPanel.Visible = false;
			DebugTextBox.Visible = true;
			viewArgsMenuItem.Checked = false;
			viewPacketsMenuItem.Checked = true;
		}

		private void autoInvokeMenuItem_Click(object sender, System.EventArgs e)
		{
			autoInvokeMenuItem.Checked = !autoInvokeMenuItem.Checked;
		}

		private void PreValidateMenuItem_Click(object sender, System.EventArgs e)
		{
			PreValidateMenuItem.Checked = !PreValidateMenuItem.Checked;
		}
	}
}
