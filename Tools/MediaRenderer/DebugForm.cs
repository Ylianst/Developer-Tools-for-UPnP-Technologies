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
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPRenderer
{
	/// <summary>
	/// Summary description for DebugForm.
	/// </summary>
	public class DebugForm : System.Windows.Forms.Form
	{
		private UPnPDeviceWatcher device;
		private bool OK = false;

		private System.Windows.Forms.TextBox DebugText;
		private System.Windows.Forms.ContextMenu ClearMenu;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem StartItem;

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public DebugForm(UPnPDeviceWatcher d)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			device = d;
			device.OnSniff += new UPnPDeviceWatcher.SniffHandler(SniffSink);

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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(DebugForm));
			this.DebugText = new System.Windows.Forms.TextBox();
			this.ClearMenu = new System.Windows.Forms.ContextMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.StartItem = new System.Windows.Forms.MenuItem();
			this.SuspendLayout();
			// 
			// DebugText
			// 
			this.DebugText.ContextMenu = this.ClearMenu;
			this.DebugText.Dock = System.Windows.Forms.DockStyle.Fill;
			this.DebugText.Multiline = true;
			this.DebugText.Name = "DebugText";
			this.DebugText.ReadOnly = true;
			this.DebugText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.DebugText.Size = new System.Drawing.Size(472, 334);
			this.DebugText.TabIndex = 0;
			this.DebugText.Text = "";
			// 
			// ClearMenu
			// 
			this.ClearMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1,
																					  this.StartItem});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.Text = "Clear";
			this.menuItem1.Click += new System.EventHandler(this.menuItem1_Click);
			// 
			// StartItem
			// 
			this.StartItem.Index = 1;
			this.StartItem.Text = "Start";
			this.StartItem.Click += new System.EventHandler(this.StartItem_Click);
			// 
			// DebugForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(472, 334);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.DebugText});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "DebugForm";
			this.Text = "Intel\'s UPnP Renderer - Debug Viewer";
			this.ResumeLayout(false);

		}
		#endregion

		private void menuItem1_Click(object sender, System.EventArgs e)
		{
			DebugText.Text = "";
		}
		protected void SniffSink(byte[] raw, int offset, int length)
		{
			if(OK)
			{
				UTF8Encoding U = new UTF8Encoding();
				DebugText.Text = DebugText.Text + U.GetString(raw,offset,length);
			}
		}

		private void StartItem_Click(object sender, System.EventArgs e)
		{
			if(StartItem.Text == "Start")
			{
				StartItem.Text = "Stop";
				OK = true;
			}
			else
			{
				StartItem.Text = "Start";
				OK = false;
			}
		}
	}
}
