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
using System.Text;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.CP;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for RendererDebugForm.
	/// </summary>
	public class RendererDebugForm : System.Windows.Forms.Form
	{
		private AVRenderer renderer;
		private UPnPServiceWatcher renderingControlWatcher;
		private UPnPServiceWatcher avTransportControlWatcher;
		private UPnPServiceWatcher connectionManagerControlWatcher;
		private UTF8Encoding utf8encoder = new UTF8Encoding();

		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.TextBox rendererControlTextBox;
		private System.Windows.Forms.TextBox connectionManagerTextBox;
		private System.Windows.Forms.TextBox avTransportTextBox;
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem clearMenuItem;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem exitMenuItem;
		private System.Windows.Forms.MenuItem onTopMenuItem;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public RendererDebugForm(AVRenderer renderer)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			this.renderer = renderer;
			
			renderingControlWatcher = renderer.RenderingControlWatcher;
			avTransportControlWatcher = renderer.AVTransportWatcher;
			connectionManagerControlWatcher = renderer.ConnectionManagerWatcher;

			renderingControlWatcher.OnSniff += new UPnPServiceWatcher.SniffHandler(renderingSniffHandlerSink);
			avTransportControlWatcher.OnSniff += new UPnPServiceWatcher.SniffHandler(avTransportSniffHandlerSink);
			connectionManagerControlWatcher.OnSniff += new UPnPServiceWatcher.SniffHandler(connectionManagerSniffHandlerSink);
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(RendererDebugForm));
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.avTransportTextBox = new System.Windows.Forms.TextBox();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.rendererControlTextBox = new System.Windows.Forms.TextBox();
			this.tabPage3 = new System.Windows.Forms.TabPage();
			this.connectionManagerTextBox = new System.Windows.Forms.TextBox();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.clearMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.exitMenuItem = new System.Windows.Forms.MenuItem();
			this.onTopMenuItem = new System.Windows.Forms.MenuItem();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.tabPage3.SuspendLayout();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					  this.tabPage1,
																					  this.tabPage2,
																					  this.tabPage3});
			this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(464, 326);
			this.tabControl1.TabIndex = 0;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				   this.avTransportTextBox});
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Size = new System.Drawing.Size(456, 300);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "AV Transport";
			// 
			// avTransportTextBox
			// 
			this.avTransportTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.avTransportTextBox.Multiline = true;
			this.avTransportTextBox.Name = "avTransportTextBox";
			this.avTransportTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.avTransportTextBox.Size = new System.Drawing.Size(456, 300);
			this.avTransportTextBox.TabIndex = 1;
			this.avTransportTextBox.Text = "";
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				   this.rendererControlTextBox});
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Size = new System.Drawing.Size(456, 300);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Renderer Control";
			// 
			// rendererControlTextBox
			// 
			this.rendererControlTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.rendererControlTextBox.Multiline = true;
			this.rendererControlTextBox.Name = "rendererControlTextBox";
			this.rendererControlTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.rendererControlTextBox.Size = new System.Drawing.Size(456, 300);
			this.rendererControlTextBox.TabIndex = 2;
			this.rendererControlTextBox.Text = "";
			// 
			// tabPage3
			// 
			this.tabPage3.Controls.AddRange(new System.Windows.Forms.Control[] {
																				   this.connectionManagerTextBox});
			this.tabPage3.Location = new System.Drawing.Point(4, 22);
			this.tabPage3.Name = "tabPage3";
			this.tabPage3.Size = new System.Drawing.Size(456, 300);
			this.tabPage3.TabIndex = 2;
			this.tabPage3.Text = "Connection Manager";
			// 
			// connectionManagerTextBox
			// 
			this.connectionManagerTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.connectionManagerTextBox.Multiline = true;
			this.connectionManagerTextBox.Name = "connectionManagerTextBox";
			this.connectionManagerTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.connectionManagerTextBox.Size = new System.Drawing.Size(456, 300);
			this.connectionManagerTextBox.TabIndex = 2;
			this.connectionManagerTextBox.Text = "";
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.clearMenuItem,
																					  this.onTopMenuItem,
																					  this.menuItem3,
																					  this.exitMenuItem});
			this.menuItem1.Text = "&File";
			// 
			// clearMenuItem
			// 
			this.clearMenuItem.Index = 0;
			this.clearMenuItem.Text = "&Clear All Packets";
			this.clearMenuItem.Click += new System.EventHandler(this.clearMenuItem_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 2;
			this.menuItem3.Text = "-";
			// 
			// exitMenuItem
			// 
			this.exitMenuItem.Index = 3;
			this.exitMenuItem.Text = "E&xit";
			this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
			// 
			// onTopMenuItem
			// 
			this.onTopMenuItem.Index = 1;
			this.onTopMenuItem.Text = "Always on Top";
			this.onTopMenuItem.Click += new System.EventHandler(this.onTopMenuItem_Click);
			// 
			// RendererDebugForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(464, 326);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.tabControl1});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Menu = this.mainMenu1;
			this.Name = "RendererDebugForm";
			this.Text = "Renderer Packet Capture";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.RendererDebugForm_Closing);
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.tabPage3.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		public void renderingSniffHandlerSink(UPnPServiceWatcher sender, byte[] raw, int offset, int length) 
		{
			lock (rendererControlTextBox) 
			{
				rendererControlTextBox.AppendText(utf8encoder.GetString(raw,offset,length));
			}
		}

		public void avTransportSniffHandlerSink(UPnPServiceWatcher sender, byte[] raw, int offset, int length) 
		{
			lock (avTransportTextBox) 
			{
				avTransportTextBox.AppendText(utf8encoder.GetString(raw,offset,length));
			}
		}

		public void connectionManagerSniffHandlerSink(UPnPServiceWatcher sender, byte[] raw, int offset, int length) 
		{
			lock (connectionManagerTextBox) 
			{
				connectionManagerTextBox.AppendText(utf8encoder.GetString(raw,offset,length));
			}
		}

		private void RendererDebugForm_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			renderingControlWatcher.OnSniff -= new UPnPServiceWatcher.SniffHandler(renderingSniffHandlerSink);
			avTransportControlWatcher.OnSniff -= new UPnPServiceWatcher.SniffHandler(avTransportSniffHandlerSink);
			connectionManagerControlWatcher.OnSniff -= new UPnPServiceWatcher.SniffHandler(connectionManagerSniffHandlerSink);		
			renderingControlWatcher = null;
			avTransportControlWatcher = null;
			connectionManagerControlWatcher = null;
			renderer = null;
		}

		private void exitMenuItem_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void clearMenuItem_Click(object sender, System.EventArgs e)
		{
			rendererControlTextBox.Clear();
			avTransportTextBox.Clear();
			connectionManagerTextBox.Clear();
		}

		private void onTopMenuItem_Click(object sender, System.EventArgs e)
		{
			onTopMenuItem.Checked = !onTopMenuItem.Checked;
			TopMost = onTopMenuItem.Checked;
		}
	
	}
}
