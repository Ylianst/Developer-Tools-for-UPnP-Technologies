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

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for UPnPServiceLocator.
	/// </summary>
	public class UPnPServiceLocator : System.Windows.Forms.Form
	{
		private UPnPSmartControlPoint scp;
		private TreeNode UPnpRoot = new TreeNode("UPnP Devices",0,0);
		private delegate void UpdateTreeDelegate(TreeNode node);
		private UPnPService service = null;

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ImageList imageList;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.TreeView deviceTree;
		private System.ComponentModel.IContainer components;

		public UPnPService Service
		{
			get 
			{
				return service;
			}
		}

		public UPnPServiceLocator()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			deviceTree.Nodes.Add(UPnpRoot);
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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(UPnPServiceLocator));
			this.deviceTree = new System.Windows.Forms.TreeView();
			this.imageList = new System.Windows.Forms.ImageList(this.components);
			this.okButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// deviceTree
			// 
			this.deviceTree.AccessibleDescription = ((string)(resources.GetObject("deviceTree.AccessibleDescription")));
			this.deviceTree.AccessibleName = ((string)(resources.GetObject("deviceTree.AccessibleName")));
			this.deviceTree.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("deviceTree.Anchor")));
			this.deviceTree.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("deviceTree.BackgroundImage")));
			this.deviceTree.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("deviceTree.Dock")));
			this.deviceTree.Enabled = ((bool)(resources.GetObject("deviceTree.Enabled")));
			this.deviceTree.Font = ((System.Drawing.Font)(resources.GetObject("deviceTree.Font")));
			this.deviceTree.ImageIndex = ((int)(resources.GetObject("deviceTree.ImageIndex")));
			this.deviceTree.ImageList = this.imageList;
			this.deviceTree.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("deviceTree.ImeMode")));
			this.deviceTree.Indent = ((int)(resources.GetObject("deviceTree.Indent")));
			this.deviceTree.ItemHeight = ((int)(resources.GetObject("deviceTree.ItemHeight")));
			this.deviceTree.Location = ((System.Drawing.Point)(resources.GetObject("deviceTree.Location")));
			this.deviceTree.Name = "deviceTree";
			this.deviceTree.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("deviceTree.RightToLeft")));
			this.deviceTree.SelectedImageIndex = ((int)(resources.GetObject("deviceTree.SelectedImageIndex")));
			this.deviceTree.Size = ((System.Drawing.Size)(resources.GetObject("deviceTree.Size")));
			this.deviceTree.TabIndex = ((int)(resources.GetObject("deviceTree.TabIndex")));
			this.deviceTree.Text = resources.GetString("deviceTree.Text");
			this.deviceTree.Visible = ((bool)(resources.GetObject("deviceTree.Visible")));
			this.deviceTree.MouseDown += new System.Windows.Forms.MouseEventHandler(this.deviceTree_MouseDown);
			this.deviceTree.DoubleClick += new System.EventHandler(this.deviceTree_DoubleClick);
			// 
			// imageList
			// 
			this.imageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.imageList.ImageSize = ((System.Drawing.Size)(resources.GetObject("imageList.ImageSize")));
			this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
			this.imageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// okButton
			// 
			this.okButton.AccessibleDescription = ((string)(resources.GetObject("okButton.AccessibleDescription")));
			this.okButton.AccessibleName = ((string)(resources.GetObject("okButton.AccessibleName")));
			this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("okButton.Anchor")));
			this.okButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("okButton.BackgroundImage")));
			this.okButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("okButton.Dock")));
			this.okButton.Enabled = ((bool)(resources.GetObject("okButton.Enabled")));
			this.okButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("okButton.FlatStyle")));
			this.okButton.Font = ((System.Drawing.Font)(resources.GetObject("okButton.Font")));
			this.okButton.Image = ((System.Drawing.Image)(resources.GetObject("okButton.Image")));
			this.okButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("okButton.ImageAlign")));
			this.okButton.ImageIndex = ((int)(resources.GetObject("okButton.ImageIndex")));
			this.okButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("okButton.ImeMode")));
			this.okButton.Location = ((System.Drawing.Point)(resources.GetObject("okButton.Location")));
			this.okButton.Name = "okButton";
			this.okButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("okButton.RightToLeft")));
			this.okButton.Size = ((System.Drawing.Size)(resources.GetObject("okButton.Size")));
			this.okButton.TabIndex = ((int)(resources.GetObject("okButton.TabIndex")));
			this.okButton.Text = resources.GetString("okButton.Text");
			this.okButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("okButton.TextAlign")));
			this.okButton.Visible = ((bool)(resources.GetObject("okButton.Visible")));
			this.okButton.Click += new System.EventHandler(this.okButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.AccessibleDescription = ((string)(resources.GetObject("cancelButton.AccessibleDescription")));
			this.cancelButton.AccessibleName = ((string)(resources.GetObject("cancelButton.AccessibleName")));
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("cancelButton.Anchor")));
			this.cancelButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("cancelButton.BackgroundImage")));
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("cancelButton.Dock")));
			this.cancelButton.Enabled = ((bool)(resources.GetObject("cancelButton.Enabled")));
			this.cancelButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("cancelButton.FlatStyle")));
			this.cancelButton.Font = ((System.Drawing.Font)(resources.GetObject("cancelButton.Font")));
			this.cancelButton.Image = ((System.Drawing.Image)(resources.GetObject("cancelButton.Image")));
			this.cancelButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.ImageAlign")));
			this.cancelButton.ImageIndex = ((int)(resources.GetObject("cancelButton.ImageIndex")));
			this.cancelButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("cancelButton.ImeMode")));
			this.cancelButton.Location = ((System.Drawing.Point)(resources.GetObject("cancelButton.Location")));
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("cancelButton.RightToLeft")));
			this.cancelButton.Size = ((System.Drawing.Size)(resources.GetObject("cancelButton.Size")));
			this.cancelButton.TabIndex = ((int)(resources.GetObject("cancelButton.TabIndex")));
			this.cancelButton.Text = resources.GetString("cancelButton.Text");
			this.cancelButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.TextAlign")));
			this.cancelButton.Visible = ((bool)(resources.GetObject("cancelButton.Visible")));
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// label1
			// 
			this.label1.AccessibleDescription = ((string)(resources.GetObject("label1.AccessibleDescription")));
			this.label1.AccessibleName = ((string)(resources.GetObject("label1.AccessibleName")));
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("label1.Anchor")));
			this.label1.AutoSize = ((bool)(resources.GetObject("label1.AutoSize")));
			this.label1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("label1.Dock")));
			this.label1.Enabled = ((bool)(resources.GetObject("label1.Enabled")));
			this.label1.Font = ((System.Drawing.Font)(resources.GetObject("label1.Font")));
			this.label1.Image = ((System.Drawing.Image)(resources.GetObject("label1.Image")));
			this.label1.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.ImageAlign")));
			this.label1.ImageIndex = ((int)(resources.GetObject("label1.ImageIndex")));
			this.label1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("label1.ImeMode")));
			this.label1.Location = ((System.Drawing.Point)(resources.GetObject("label1.Location")));
			this.label1.Name = "label1";
			this.label1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("label1.RightToLeft")));
			this.label1.Size = ((System.Drawing.Size)(resources.GetObject("label1.Size")));
			this.label1.TabIndex = ((int)(resources.GetObject("label1.TabIndex")));
			this.label1.Text = resources.GetString("label1.Text");
			this.label1.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.TextAlign")));
			this.label1.Visible = ((bool)(resources.GetObject("label1.Visible")));
			// 
			// UPnPServiceLocator
			// 
			this.AcceptButton = this.okButton;
			this.AccessibleDescription = ((string)(resources.GetObject("$this.AccessibleDescription")));
			this.AccessibleName = ((string)(resources.GetObject("$this.AccessibleName")));
			this.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("$this.Anchor")));
			this.AutoScaleBaseSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScaleBaseSize")));
			this.AutoScroll = ((bool)(resources.GetObject("$this.AutoScroll")));
			this.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMargin")));
			this.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMinSize")));
			this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
			this.CancelButton = this.cancelButton;
			this.ClientSize = ((System.Drawing.Size)(resources.GetObject("$this.ClientSize")));
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.cancelButton,
																		  this.okButton,
																		  this.deviceTree});
			this.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("$this.Dock")));
			this.Enabled = ((bool)(resources.GetObject("$this.Enabled")));
			this.Font = ((System.Drawing.Font)(resources.GetObject("$this.Font")));
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("$this.ImeMode")));
			this.Location = ((System.Drawing.Point)(resources.GetObject("$this.Location")));
			this.MaximumSize = ((System.Drawing.Size)(resources.GetObject("$this.MaximumSize")));
			this.MinimumSize = ((System.Drawing.Size)(resources.GetObject("$this.MinimumSize")));
			this.Name = "UPnPServiceLocator";
			this.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("$this.RightToLeft")));
			this.StartPosition = ((System.Windows.Forms.FormStartPosition)(resources.GetObject("$this.StartPosition")));
			this.Text = resources.GetString("$this.Text");
			this.Visible = ((bool)(resources.GetObject("$this.Visible")));
			this.Load += new System.EventHandler(this.UPnPServiceLocator_Load);
			this.ResumeLayout(false);

		}
		#endregion


		private void HandleRemovedDevice(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			ArrayList TempList = new ArrayList();
			TreeNode tn;
			IEnumerator en = UPnpRoot.Nodes.GetEnumerator();
			while(en.MoveNext())
			{
				tn = (TreeNode)en.Current;
				if(((UPnPDevice)tn.Tag).UniqueDeviceName==device.UniqueDeviceName)
				{
					TempList.Add(tn);
				}
			}
			for(int x=0;x<TempList.Count;++x)
			{
				UPnpRoot.Nodes.Remove((TreeNode)TempList[x]);
			}
		}

		private void HandleAddedDevice(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			HandleCreate(device,device.BaseURL);
		}

		private void HandleCreate(UPnPDevice device, Uri URL)
		{
			TreeNode Parent;
			TreeNode Child;

			Parent = new TreeNode(device.FriendlyName,1,1);
			Parent.Tag = device;
			for(int cid=0;cid<device.Services.Length;++cid)
			{
				Child = new TreeNode(device.Services[cid].ServiceURN,2,2);
				Child.Tag = device.Services[cid];
				Parent.Nodes.Add(Child);
			}

			for(int cid=0;cid<device.EmbeddedDevices.Length;++cid)
			{
				Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
				Child.Tag = device.EmbeddedDevices[cid];
				Parent.Nodes.Add(Child);
			}

			Object[] args = new Object[1];
			args[0] = Parent;
			this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate),args);
		}

		private TreeNode ProcessEmbeddedDevice(UPnPDevice device)
		{
			TreeNode Parent;
			TreeNode Child;

			Parent = new TreeNode(device.FriendlyName,1,1);
			Parent.Tag = device;

			for(int cid=0;cid<device.Services.Length;++cid)
			{
				Child = new TreeNode(device.Services[cid].ServiceURN,2,2);
				Child.Tag = device.Services[cid];
				Parent.Nodes.Add(Child);
			}

			for(int cid=0;cid<device.EmbeddedDevices.Length;++cid)
			{
				Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
				Child.Tag = device.EmbeddedDevices[cid];
				Parent.Nodes.Add(Child);
			}

			return(Parent);
		}

		private void HandleTreeUpdate(TreeNode node)
		{
			UPnpRoot.Nodes.Add(node);
			UPnpRoot.Expand();
		}

		private void deviceTree_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			TreeNode node = deviceTree.GetNodeAt(e.X,e.Y);
			if (node == null) return;
			deviceTree.SelectedNode = node;
			object infoObject = node.Tag;
			if (infoObject == null) 
			{
				service = null;
				okButton.Enabled = false;
				return;
			}
			if (infoObject.GetType() == typeof(UPnPService)) 
			{
				service = (UPnPService)infoObject;
				okButton.Enabled = true;
			} 
			else 
			{
				service = null;
				okButton.Enabled = false;
			}
		}

		private void okButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = System.Windows.Forms.DialogResult.OK;
		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = System.Windows.Forms.DialogResult.Cancel;		
		}

		private void UPnPServiceLocator_Load(object sender, System.EventArgs e)
		{
			scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(HandleAddedDevice));
			scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(HandleRemovedDevice);
		}

		private void deviceTree_DoubleClick(object sender, System.EventArgs e)
		{
			if (deviceTree.SelectedNode == null) return;
			object infoObject = deviceTree.SelectedNode.Tag;
			if (infoObject.GetType() == typeof(UPnPService)) 
			{
				service = (UPnPService)infoObject;
				this.DialogResult = System.Windows.Forms.DialogResult.OK;
			}
		}

	}
}
