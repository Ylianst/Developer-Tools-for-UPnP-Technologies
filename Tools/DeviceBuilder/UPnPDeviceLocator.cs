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

namespace UPnPStackBuilder
{
    /// <summary>
    /// Summary description for DeviceSelector.
    /// </summary>
    public class UPnPDeviceLocator : System.Windows.Forms.Form
    {
        public UPnPDevice SelectedDevice = null;
        private UPnPSmartControlPoint scp;

        protected TreeNode UPnpRoot = new TreeNode("UPnP Devices", 0, 0);
        private System.Windows.Forms.TreeView DeviceTree;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.ImageList imageList;
        private System.ComponentModel.IContainer components;

        public delegate void UpdateTreeDelegate(TreeNode node);

        public UPnPDeviceLocator()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            DeviceTree.Nodes.Add(UPnpRoot);
            scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(AddSink));
            scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(RemoveSink);
        }

        private void AddSink(UPnPSmartControlPoint sender, UPnPDevice d)
        {
            HandleCreate(d, d.BaseURL);
        }

        private void RemoveSink(UPnPSmartControlPoint sender, UPnPDevice device)
        {
            ArrayList TempList = new ArrayList();
            TreeNode tn;
            IEnumerator en = UPnpRoot.Nodes.GetEnumerator();
            while (en.MoveNext())
            {
                tn = (TreeNode)en.Current;
                if (((UPnPDevice)tn.Tag).UniqueDeviceName == device.UniqueDeviceName)
                {
                    TempList.Add(tn);
                }
            }
            for (int x = 0; x < TempList.Count; ++x)
            {
                UPnpRoot.Nodes.Remove((TreeNode)TempList[x]);
            }
        }

        protected void HandleCreate(UPnPDevice device, Uri URL)
        {
            TreeNode Parent;
            TreeNode Child;
            SortedList TempList = new SortedList();

            Parent = new TreeNode(device.FriendlyName, 1, 1);
            Parent.Tag = device;

            for (int cid = 0; cid < device.EmbeddedDevices.Length; ++cid)
            {
                Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
                Child.Tag = device.EmbeddedDevices[cid];
                Parent.Nodes.Add(Child);
            }

            Object[] args = new Object[1];
            args[0] = Parent;
            if (this.IsHandleCreated == true)
            {
                this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate), args);
            }
            else
            {
                HandleTreeUpdate(Parent);
            }
        }

        protected TreeNode ProcessEmbeddedDevice(UPnPDevice device)
        {
            SortedList TempList = new SortedList();
            TreeNode Parent;
            TreeNode Child;

            Parent = new TreeNode(device.FriendlyName, 1, 1);
            Parent.Tag = device;

            for (int cid = 0; cid < device.EmbeddedDevices.Length; ++cid)
            {
                Child = ProcessEmbeddedDevice(device.EmbeddedDevices[cid]);
                Child.Tag = device.EmbeddedDevices[cid];
                Parent.Nodes.Add(Child);
            }

            return (Parent);
        }

        protected void HandleTreeUpdate(TreeNode node)
        {
            // Insert this node into the tree
            if (UPnpRoot.Nodes.Count == 0)
            {
                UPnpRoot.Nodes.Add(node);
            }
            else
            {
                for (int i = 0; i < UPnpRoot.Nodes.Count; ++i)
                {
                    if (UPnpRoot.Nodes[i].Text.CompareTo(node.Text) > 0)
                    {
                        UPnpRoot.Nodes.Insert(i, node);
                        break;
                    }
                    if (i == UPnpRoot.Nodes.Count - 1)
                    {
                        UPnpRoot.Nodes.Add(node);
                        break;
                    }
                }
            }
            UPnpRoot.Expand();
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null) { components.Dispose(); }
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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(UPnPDeviceLocator));
            this.DeviceTree = new System.Windows.Forms.TreeView();
            this.label1 = new System.Windows.Forms.Label();
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.imageList = new System.Windows.Forms.ImageList(this.components);
            this.SuspendLayout();
            // 
            // DeviceTree
            // 
            this.DeviceTree.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.DeviceTree.ImageList = this.imageList;
            this.DeviceTree.Location = new System.Drawing.Point(8, 40);
            this.DeviceTree.Name = "DeviceTree";
            this.DeviceTree.Size = new System.Drawing.Size(330, 266);
            this.DeviceTree.TabIndex = 0;
            this.DeviceTree.DoubleClick += new System.EventHandler(this.OnDoubleClick);
            this.DeviceTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.DeviceTree_AfterSelect);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.label1.Location = new System.Drawing.Point(8, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(330, 32);
            this.label1.TabIndex = 4;
            this.label1.Text = "Select a UPnP network device from which the devices templated will be loaded from" +
                ".";
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(136, 314);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(96, 24);
            this.cancelButton.TabIndex = 6;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // okButton
            // 
            this.okButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
            this.okButton.Enabled = false;
            this.okButton.Location = new System.Drawing.Point(232, 314);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(96, 24);
            this.okButton.TabIndex = 5;
            this.okButton.Text = "OK";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // imageList
            // 
            this.imageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
            this.imageList.ImageSize = new System.Drawing.Size(16, 16);
            this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
            this.imageList.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // UPnPDeviceLocator
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(344, 342);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.cancelButton,
																		  this.okButton,
																		  this.label1,
																		  this.DeviceTree});
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "UPnPDeviceLocator";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "UPnP Device Locator";
            this.ResumeLayout(false);

        }
        #endregion

        private void OnDoubleClick(object sender, System.EventArgs e)
        {
            TreeNode Selected = DeviceTree.SelectedNode;
            if (Selected.Tag != null && Selected.Tag.GetType() == typeof(OpenSource.UPnP.UPnPDevice))
            {
                SelectedDevice = (UPnPDevice)Selected.Tag;
                this.DialogResult = DialogResult.OK;
            }
        }

        private void DeviceTree_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
        {
            TreeNode Selected = DeviceTree.SelectedNode;
            if (Selected.Tag != null && Selected.Tag.GetType() == typeof(OpenSource.UPnP.UPnPDevice))
            {
                SelectedDevice = (UPnPDevice)Selected.Tag;
                okButton.Enabled = true;
            }
            else
            {
                SelectedDevice = null;
                okButton.Enabled = false;
            }
        }

        private void okButton_Click(object sender, System.EventArgs e)
        {
            if (SelectedDevice != null) this.DialogResult = DialogResult.OK;
        }

        private void cancelButton_Click(object sender, System.EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
        }
    }
}
