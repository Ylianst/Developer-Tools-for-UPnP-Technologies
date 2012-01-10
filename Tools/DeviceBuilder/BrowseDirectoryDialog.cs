using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for BrowseForm.
	/// </summary>
	public class BrowseDirectoryDialog : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TreeView folders;
		private System.Windows.Forms.ImageList treeImageList;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox eFolder;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.ComponentModel.IContainer components;

		private string directory = "";

		public BrowseDirectoryDialog()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// Start with desktop as the root directory
			//
			// get the desktop folder name
			string str = Environment.GetFolderPath( Environment.SpecialFolder.DesktopDirectory );
			DirectoryInfo desktop = new DirectoryInfo( str );
			TreeNode desktopNode = folders.Nodes.Add( desktop.Name );
			desktopNode.Tag = desktop;
			desktopNode.ImageIndex = 4;	// image 4 = desktop
			desktopNode.SelectedImageIndex = 4;

			// add all the special 'folders'
			AddDesktopSubfolders( desktopNode );
			desktopNode.Expand();
		}

		public string Directory
		{
			get {return directory;}
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(BrowseDirectoryDialog));
			this.folders = new System.Windows.Forms.TreeView();
			this.treeImageList = new System.Windows.Forms.ImageList(this.components);
			this.okButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.eFolder = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// folders
			// 
			this.folders.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.folders.ImageList = this.treeImageList;
			this.folders.Location = new System.Drawing.Point(8, 24);
			this.folders.Name = "folders";
			this.folders.SelectedImageIndex = 1;
			this.folders.Size = new System.Drawing.Size(312, 240);
			this.folders.TabIndex = 1;
			this.folders.AfterExpand += new System.Windows.Forms.TreeViewEventHandler(this.folders_AfterExpand);
			this.folders.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.folders_AfterSelect);
			// 
			// treeImageList
			// 
			this.treeImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.treeImageList.ImageSize = new System.Drawing.Size(16, 16);
			this.treeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("treeImageList.ImageStream")));
			this.treeImageList.TransparentColor = System.Drawing.Color.Teal;
			// 
			// okButton
			// 
			this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.okButton.Enabled = false;
			this.okButton.Location = new System.Drawing.Point(248, 296);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(72, 25);
			this.okButton.TabIndex = 2;
			this.okButton.Text = "OK";
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(168, 296);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(72, 25);
			this.cancelButton.TabIndex = 3;
			this.cancelButton.Text = "Cancel";
			// 
			// label2
			// 
			this.label2.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(8, 272);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(36, 13);
			this.label2.TabIndex = 4;
			this.label2.Text = "Folder";
			// 
			// eFolder
			// 
			this.eFolder.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.eFolder.Location = new System.Drawing.Point(48, 268);
			this.eFolder.Name = "eFolder";
			this.eFolder.Size = new System.Drawing.Size(272, 20);
			this.eFolder.TabIndex = 5;
			this.eFolder.Text = "";
			this.eFolder.TextChanged += new System.EventHandler(this.eFolder_TextChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(320, 16);
			this.label1.TabIndex = 6;
			this.label1.Text = "Select new shared directory network";
			// 
			// BrowseDirectoryDialog
			// 
			this.AcceptButton = this.okButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.cancelButton;
			this.ClientSize = new System.Drawing.Size(328, 326);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.eFolder,
																		  this.label2,
																		  this.cancelButton,
																		  this.okButton,
																		  this.folders});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.KeyPreview = true;
			this.Name = "BrowseDirectoryDialog";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Add Shared Directory";
			this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.BrowseForm_KeyUp);
			this.ResumeLayout(false);

		}
		#endregion

		private void AddChildrenOf( TreeNode node, DirectoryInfo dirInfo )
		{
			// recursive routine used by AddAllFolders
			TreeNode childNode;

			childNode = node.Nodes.Add( dirInfo.Name );
			childNode.ImageIndex = 0;
			childNode.SelectedImageIndex = 1;
			if ( 0 != dirInfo.GetDirectories().Length )
			{
				foreach ( DirectoryInfo childDir in dirInfo.GetDirectories() )
					AddChildrenOf( childNode, childDir );
			}
		}

		private void AddDesktopSubfolders( TreeNode desktopNode )
		{
			if ( ! ( desktopNode.Tag is DirectoryInfo ))
				return;
			DirectoryInfo desktop = (DirectoryInfo) desktopNode.Tag;

			// add 'my documents'
			string str = Environment.GetFolderPath( Environment.SpecialFolder.Personal );
			DirectoryInfo mydocs = new DirectoryInfo( str );
			TreeNode mydocsNode = new TreeNode(mydocs.Name,5,5);
			desktopNode.Nodes.Add( mydocsNode );
			mydocsNode.Tag = mydocs;
			// add a dummy entry if the folder contains subfolders, so the '+' shows
			if ( 0 != mydocs.GetDirectories().Length )
				mydocsNode.Nodes.Add( "" );

			// add 'my computer'
			string mycomputername = System.Environment.MachineName;
			TreeNode mycomputerNode = desktopNode.Nodes.Add( mycomputername );
			mycomputerNode.ImageIndex = 2;	// image 2 = computer
			mycomputerNode.SelectedImageIndex = 2;
			mycomputerNode.Nodes.Add( "" );	// my computer must have children
			mycomputerNode.Expand();

			// add 'my network places'
			//string mynetworkplaces = System.Net.

			// add each folder really on the desktop
			foreach ( DirectoryInfo dir in ((DirectoryInfo)desktopNode.Tag).GetDirectories() )
			{
				TreeNode node = desktopNode.Nodes.Add( dir.Name );
				node.Tag = dir;
				// add a dummy entry if the folder contains subfolders, so the '+' shows
				if ( 0 != dir.GetDirectories().Length )
					node.Nodes.Add( "" );
			}
		}

		private void AddMycomputerSubfolders( TreeNode mycomputerNode )
		{
			TreeNode driveNode;

			foreach ( string drive in Environment.GetLogicalDrives() )
			{
				driveNode = new TreeNode(drive.ToString(),3,3);
				driveNode.Nodes.Add( "" );
				driveNode.Tag = new DirectoryInfo( drive.ToString() );
				mycomputerNode.Nodes.Add( driveNode );
			}
		}
	
		private void folders_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			eFolder.Text = (null == folders.SelectedNode) ? "" : folders.SelectedNode.Text;
			if ( folders.SelectedNode.Tag is DirectoryInfo )
				directory = ((DirectoryInfo) folders.SelectedNode.Tag).FullName;
		}

		private void eFolder_TextChanged(object sender, System.EventArgs e)
		{
			okButton.Enabled = ( 2 > folders.SelectedNode.ImageIndex ) && // images 0,1 are folders
						   ( 0 < eFolder.Text.Length );
		}

		private void BrowseForm_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			switch ( e.KeyCode )
			{
				case Keys.Enter:	this.DialogResult = DialogResult.OK; break;
				case Keys.Escape:	this.DialogResult = DialogResult.Cancel; break;
			}
		}

		private void folders_AfterExpand(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			if ( e.Node.ImageIndex < 2 || e.Node.ImageIndex == 3 || e.Node.ImageIndex > 4 )	// ordinary folder
			{
				try
				{
					e.Node.Nodes.Clear();
					DirectoryInfo dirInfo = (DirectoryInfo) e.Node.Tag;
					foreach ( DirectoryInfo dir in dirInfo.GetDirectories() )
					{
						TreeNode node = e.Node.Nodes.Add( dir.Name );
						node.Tag = dir;
						// add a dummy entry if the folder contains subfolders, so the '+' shows
						if ( 0 != dir.GetDirectories().Length )
							node.Nodes.Add( "" );
					}
				}
				catch {}
			}
			else if ( e.Node.ImageIndex == 2 )	// my computer
			{
				e.Node.Nodes.Clear();
				AddMycomputerSubfolders( e.Node );
			}
			else if ( e.Node.ImageIndex == 4 )	// desktop folder
			{
				e.Node.Nodes.Clear();
				AddDesktopSubfolders( e.Node );
			}

		}

	}
}
