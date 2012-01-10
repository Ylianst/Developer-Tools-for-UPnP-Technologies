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
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for ModifyDevice.
	/// </summary>
	public class ModifyDevice : System.Windows.Forms.Form
	{
		private UPnPDevice OriginalDevice = null;
		public UPnPDevice NewDevice = null;

		private System.Windows.Forms.Button ResetButton;
		private System.Windows.Forms.Button OKButton;
		private System.Windows.Forms.TreeView DeviceView;
		private System.Windows.Forms.Label label1;
		private System.ComponentModel.Container components = null;

		public ModifyDevice(UPnPDevice d)
		{
			OriginalDevice = d;
			NewDevice = OriginalDevice;
		
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			ShowDevice(d);
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ModifyDevice));
			this.DeviceView = new System.Windows.Forms.TreeView();
			this.ResetButton = new System.Windows.Forms.Button();
			this.OKButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// DeviceView
			// 
			this.DeviceView.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.DeviceView.ImageIndex = -1;
			this.DeviceView.Location = new System.Drawing.Point(8, 40);
			this.DeviceView.Name = "DeviceView";
			this.DeviceView.SelectedImageIndex = -1;
			this.DeviceView.Size = new System.Drawing.Size(312, 216);
			this.DeviceView.TabIndex = 0;
			// 
			// ResetButton
			// 
			this.ResetButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.ResetButton.Location = new System.Drawing.Point(152, 264);
			this.ResetButton.Name = "ResetButton";
			this.ResetButton.Size = new System.Drawing.Size(80, 24);
			this.ResetButton.TabIndex = 1;
			this.ResetButton.Text = "Reset";
			// 
			// OKButton
			// 
			this.OKButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.OKButton.Location = new System.Drawing.Point(240, 264);
			this.OKButton.Name = "OKButton";
			this.OKButton.Size = new System.Drawing.Size(80, 24);
			this.OKButton.TabIndex = 2;
			this.OKButton.Text = "OK";
			this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label1.Location = new System.Drawing.Point(9, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(312, 32);
			this.label1.TabIndex = 5;
			this.label1.Text = "Select the devices actions that will be accesible for outside the network.";
			// 
			// ModifyDevice
			// 
			this.AcceptButton = this.OKButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(330, 296);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.OKButton,
																		  this.ResetButton,
																		  this.DeviceView});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "ModifyDevice";
			this.Text = "Modify Device Permissions";
			this.ResumeLayout(false);

		}
		#endregion

		private void OKButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.OK;
		}

		private void ShowDevice(UPnPDevice d)
		{
			TreeNode root = new TreeNode(d.FriendlyName);
			root.Tag = d;
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				AddDeviceToNode(root,ed);
			}
			foreach(UPnPService s in d.Services)
			{
				AddServiceToNode(root,s);
			}
			DeviceView.Nodes.Add(root);
		}

		private void AddDeviceToNode(TreeNode n, UPnPDevice d)
		{
			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				AddDeviceToNode(n,ed);
			}

			TreeNode NewNode = new TreeNode(d.FriendlyName);
			NewNode.Tag = d;
			foreach(UPnPService s in d.Services)
			{
				AddServiceToNode(NewNode,s);
			}

			n.Nodes.Add(NewNode);
		}

		private void AddServiceToNode(TreeNode n, UPnPService s)
		{
			TreeNode snode = new TreeNode(s.ServiceURN);
			snode.Tag = s;
			foreach(UPnPAction A in s.Actions)
			{
				TreeNode an = new TreeNode(A.Name);
				an.Tag = A;
				snode.Nodes.Add(an);
			}
			n.Nodes.Add(snode);
		}

	}
}
