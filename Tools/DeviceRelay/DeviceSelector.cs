using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for DeviceSelector.
	/// </summary>
	public class DeviceSelector : System.Windows.Forms.Form
	{
		private UPnPDevice selectedDevice = null;
		public UPnPDevice SelectedDevice {get {return selectedDevice;}}

		private UPnPSmartControlPoint scp;
		protected TreeNode UPnpRoot = new TreeNode("UPnP Devices",0,0);

		private System.Windows.Forms.TreeView DeviceTree;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.ImageList imageList;
		private System.Windows.Forms.Label label1;
		private System.ComponentModel.IContainer components;
	
		public delegate void UpdateTreeDelegate(TreeNode node);

		public DeviceSelector()
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
			HandleCreate(d,d.BaseURL);
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
			for (int x=0;x<TempList.Count;++x)
			{
				UPnpRoot.Nodes.Remove((TreeNode)TempList[x]);
			}
		}

		protected void HandleCreate(UPnPDevice device, Uri URL)
		{
			TreeNode parent = new TreeNode(device.FriendlyName,1,1);
			parent.Tag = device;
			Object[] args = new Object[1];
			args[0] = parent;
			if (this.IsHandleCreated == true) 
			{
				this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate),args);
			} 
			else 
			{
				HandleTreeUpdate(parent);
			}
		}

		protected void HandleTreeUpdate(TreeNode node)
		{
			// Insert this node into the tree
			if(UPnpRoot.Nodes.Count==0)
			{
				UPnpRoot.Nodes.Add(node);
			}
			else
			{
				for (int i=0;i<UPnpRoot.Nodes.Count;++i)
				{
					if (UPnpRoot.Nodes[i].Text.CompareTo(node.Text)>0)
					{
						UPnpRoot.Nodes.Insert(i,node);
						break;
					}
					if (i == UPnpRoot.Nodes.Count-1)
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(DeviceSelector));
			this.DeviceTree = new System.Windows.Forms.TreeView();
			this.imageList = new System.Windows.Forms.ImageList(this.components);
			this.cancelButton = new System.Windows.Forms.Button();
			this.okButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
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
			this.DeviceTree.Size = new System.Drawing.Size(312, 216);
			this.DeviceTree.TabIndex = 0;
			this.DeviceTree.DoubleClick += new System.EventHandler(this.OnDoubleClick);
			this.DeviceTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.DeviceTree_AfterSelect);
			// 
			// imageList
			// 
			this.imageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.imageList.ImageSize = new System.Drawing.Size(16, 16);
			this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
			this.imageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(152, 264);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(80, 24);
			this.cancelButton.TabIndex = 3;
			this.cancelButton.Text = "Cancel";
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// okButton
			// 
			this.okButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.okButton.Location = new System.Drawing.Point(240, 264);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(80, 24);
			this.okButton.TabIndex = 2;
			this.okButton.Text = "OK";
			this.okButton.Click += new System.EventHandler(this.okButton_Click);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(312, 32);
			this.label1.TabIndex = 4;
			this.label1.Text = "Select a UPnP network device that will be accessible from outside the network.";
			// 
			// DeviceSelector
			// 
			this.AcceptButton = this.okButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.cancelButton;
			this.ClientSize = new System.Drawing.Size(326, 292);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.cancelButton,
																		  this.okButton,
																		  this.DeviceTree});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "DeviceSelector";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "UPnP Device Selection";
			this.ResumeLayout(false);

		}
		#endregion

		private void OnDoubleClick(object sender, System.EventArgs e)
		{
			object Selected = DeviceTree.SelectedNode;
			if (((TreeNode)Selected).Tag.GetType() == typeof(UPnPDevice))
			{
				selectedDevice = (UPnPDevice)((TreeNode)Selected).Tag;
				this.DialogResult = DialogResult.OK;
			}
		}

		private void okButton_Click(object sender, System.EventArgs e)
		{
			if (selectedDevice != null) this.DialogResult = DialogResult.OK;
		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			selectedDevice = null;
			this.DialogResult = DialogResult.Cancel;
		}

		private void DeviceTree_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			TreeNode node = (TreeNode)DeviceTree.SelectedNode;
			if (node != null && node.Tag != null && node.Tag.GetType() == typeof(UPnPDevice))
			{
				selectedDevice = (UPnPDevice)node.Tag;
				okButton.Enabled = true;
			}
			else
			{
				selectedDevice = null;
				okButton.Enabled = false;
			}
		}
	}
}
