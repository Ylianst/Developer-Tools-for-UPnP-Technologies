using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using Intel.UPNP;

namespace UPnPValidator
{
	/// <summary>
	/// Summary description for DeviceSelector.
	/// </summary>
	public class DeviceSelector : System.Windows.Forms.Form
	{
		public UPnPDevice SelectedDevice = null;
		private UPnPSmartControlPoint scp;

		protected TreeNode UPnpRoot = new TreeNode("UPnP Devices",0,0);
		private System.Windows.Forms.TreeView DeviceTree;
	
		public delegate void UpdateTreeDelegate(TreeNode node);

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public DeviceSelector()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			DeviceTree.Nodes.Add(UPnpRoot);
			scp = new UPnPSmartControlPoint(new UPnPSmartControlPoint.DeviceHandler(AddSink));
			scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(RemoveSink);
			
			//
			// TODO: Add any constructor code after InitializeComponent call
			//
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


		protected void HandleCreate(UPnPDevice device, Uri URL)
		{
			TreeNode Parent;
			TreeNode Child;
			SortedList TempList = new SortedList();

			Parent = new TreeNode(device.FriendlyName,1,1);
			Parent.Tag = device;
			for(int cid=0;cid<device.Services.Length;++cid)
			{
				Child = new TreeNode(device.Services[cid].ServiceURN,2,2);
				Child.Tag = device.Services[cid];

				TreeNode stateVarNode = new TreeNode("State variables",6,6);
				Child.Nodes.Add(stateVarNode);

				UPnPStateVariable[] varList = device.Services[cid].GetStateVariables();
				TempList.Clear();
				foreach (UPnPStateVariable var in varList) 
				{
					TreeNode varNode = new TreeNode(var.Name,5,5);
					varNode.Tag = var;
					TempList.Add(var.Name,varNode);
					//stateVarNode.Nodes.Add(varNode);
				}
				IDictionaryEnumerator sve = TempList.GetEnumerator();
				while(sve.MoveNext())
				{
					stateVarNode.Nodes.Add((TreeNode)sve.Value);
				}

				TempList.Clear();
				foreach (UPnPAction action in device.Services[cid].GetActions()) 
				{
					string argsstr = "";
					foreach (UPnPArgument arg in action.ArgumentList)
					{
						if(arg.IsReturnValue==false)
						{
							if (argsstr != "") argsstr += ", ";
							argsstr += arg.RelatedStateVar.ValueType + " " + arg.Name;
						}
					}

					TreeNode methodNode = new TreeNode(action.Name+"(" + argsstr + ")",4,4);
					methodNode.Tag = action;
					//Child.Nodes.Add(methodNode);
					TempList.Add(action.Name,methodNode);
				}
				
				IDictionaryEnumerator ide = TempList.GetEnumerator();
				while(ide.MoveNext())
				{
					Child.Nodes.Add((TreeNode)ide.Value);
				}
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
			if(this.IsHandleCreated==true)
			{
				this.Invoke(new UpdateTreeDelegate(HandleTreeUpdate),args);
			}
			else
			{
				HandleTreeUpdate(Parent);
			}
		}

		protected TreeNode ProcessEmbeddedDevice(UPnPDevice device)
		{
			SortedList TempList =new SortedList();
			TreeNode Parent;
			TreeNode Child;

			Parent = new TreeNode(device.FriendlyName,1,1);
			Parent.Tag = device;

			/*
			for(int x=0;x<device.Services.Length;++x)
			{
				device.Services[x].OnInvokeError += new UPnPService.UPnPServiceInvokeErrorHandler(HandleInvokeError);
				device.Services[x].OnInvokeResponse += new UPnPService.UPnPServiceInvokeHandler(HandleInvoke);
			}
			*/

			for(int cid=0;cid<device.Services.Length;++cid)
			{
				Child = new TreeNode(device.Services[cid].ServiceURN,2,2);
				Child.Tag = device.Services[cid];

				TreeNode stateVarNode = new TreeNode("State variables",6,6);
				Child.Nodes.Add(stateVarNode);

				UPnPStateVariable[] varList = device.Services[cid].GetStateVariables();
				TempList.Clear();
				foreach (UPnPStateVariable var in varList) 
				{
					TreeNode varNode = new TreeNode(var.Name,5,5);
					varNode.Tag = var;
					TempList.Add(var.Name,varNode);
					//stateVarNode.Nodes.Add(varNode);
				}
				IDictionaryEnumerator sve = TempList.GetEnumerator();
				while(sve.MoveNext())
				{
					stateVarNode.Nodes.Add((TreeNode)sve.Value);
				}
				

				TempList.Clear();
				foreach (UPnPAction action in device.Services[cid].GetActions()) 
				{
					string argsstr = "";
					foreach (UPnPArgument arg in action.ArgumentList)
					{
						if(arg.IsReturnValue==false)
						{
							if (argsstr != "") argsstr += ", ";
							argsstr += arg.RelatedStateVar.ValueType + " " + arg.Name;
						}
					}

					TreeNode methodNode = new TreeNode(action.Name+"(" + argsstr + ")",4,4);
					methodNode.Tag = action;
					//Child.Nodes.Add(methodNode);
					TempList.Add(action.Name,methodNode);
				}
				
				IDictionaryEnumerator ide = TempList.GetEnumerator();
				while(ide.MoveNext())
				{
					Child.Nodes.Add((TreeNode)ide.Value);
				}
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

		protected void HandleTreeUpdate(TreeNode node)
		{
			//UPnpRoot.Nodes.Add(node);
			//UPnpRoot.Expand();

			// Insert this node into the tree
			if(UPnpRoot.Nodes.Count==0)
			{
				UPnpRoot.Nodes.Add(node);
			}
			else
			{
				for(int i=0;i<UPnpRoot.Nodes.Count;++i)
				{
					if(UPnpRoot.Nodes[i].Text.CompareTo(node.Text)>0)
					{
						UPnpRoot.Nodes.Insert(i,node);
						break;
					}
					if(i == UPnpRoot.Nodes.Count-1)
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(DeviceSelector));
			this.DeviceTree = new System.Windows.Forms.TreeView();
			this.SuspendLayout();
			// 
			// DeviceTree
			// 
			this.DeviceTree.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.DeviceTree.ImageIndex = -1;
			this.DeviceTree.Name = "DeviceTree";
			this.DeviceTree.SelectedImageIndex = -1;
			this.DeviceTree.Size = new System.Drawing.Size(400, 304);
			this.DeviceTree.TabIndex = 0;
			this.DeviceTree.DoubleClick += new System.EventHandler(this.OnDoubleClick);
			// 
			// DeviceSelector
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(400, 302);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.DeviceTree});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "DeviceSelector";
			this.Text = "UPnP Validation Device Selection";
			this.ResumeLayout(false);

		}
		#endregion

		private void OnDoubleClick(object sender, System.EventArgs e)
		{
			object Selected = DeviceTree.SelectedNode;
			if(((TreeNode)Selected).Tag.GetType().FullName=="Intel.UPNP.UPnPDevice")
			{
				SelectedDevice = (UPnPDevice)((TreeNode)Selected).Tag;
				this.DialogResult=DialogResult.OK;
			}
		}
	}
}
