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
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPValidator
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
			scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(HandleAddedDevice));
			scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(HandleRemovedDevice);
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
			this.deviceTree.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.deviceTree.ImageList = this.imageList;
			this.deviceTree.Location = new System.Drawing.Point(8, 40);
			this.deviceTree.Name = "deviceTree";
			this.deviceTree.Size = new System.Drawing.Size(328, 264);
			this.deviceTree.TabIndex = 0;
			this.deviceTree.MouseDown += new System.Windows.Forms.MouseEventHandler(this.deviceTree_MouseDown);
			this.deviceTree.DoubleClick += new System.EventHandler(this.deviceTree_DoubleClick);
			// 
			// imageList
			// 
			this.imageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.imageList.ImageSize = new System.Drawing.Size(16, 16);
			this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
			this.imageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// okButton
			// 
			this.okButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.okButton.Enabled = false;
			this.okButton.Location = new System.Drawing.Point(232, 312);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(96, 24);
			this.okButton.TabIndex = 1;
			this.okButton.Text = "OK";
			this.okButton.Click += new System.EventHandler(this.okButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(136, 312);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(96, 24);
			this.cancelButton.TabIndex = 2;
			this.cancelButton.Text = "Cancel";
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(328, 32);
			this.label1.TabIndex = 3;
			this.label1.Text = "Select a network service from which the SCPD will be loaded from.";
			// 
			// UPnPServiceLocator
			// 
			this.AcceptButton = this.okButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.cancelButton;
			this.ClientSize = new System.Drawing.Size(344, 342);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.cancelButton,
																		  this.okButton,
																		  this.deviceTree});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "UPnPServiceLocator";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "UPnP Service Editor - SCPD Locator";
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

			if (this.IsHandleCreated == true) 
			{
				Object[] args = new Object[1];
				args[0] = Parent;
				this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate),args);
			} 
			else 
			{
				HandleTreeUpdate(Parent);
			}
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
			UPnpRoot.Expand();
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
