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
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for MediaSearchForm.
	/// </summary>
	public class MediaSearchForm : System.Windows.Forms.Form
	{
		private TreeNode cdsRootNode;
		private TreeNode rendererRootNode;
		private MainForm parent;
		private Point dragStartPoint;
		private Hashtable PlayToRendererMenuItemMapping = new Hashtable();

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.Button searchButton;
		private System.Windows.Forms.ListView mediaListView;
		private System.Windows.Forms.TextBox searchTextBox;
		private System.Windows.Forms.Panel topPanel;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ImageList treeImageList;
		private System.Windows.Forms.ContextMenu mediaContextMenu;
		private System.Windows.Forms.MenuItem mediaDisplayPropMenuItem;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.ComponentModel.IContainer components;

		public MediaSearchForm(TreeNode cdsRootNode, TreeNode rendererRootNode, MainForm parent)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			this.cdsRootNode = cdsRootNode;
			this.rendererRootNode = rendererRootNode;
			this.parent = parent;
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MediaSearchForm));
			this.mediaListView = new System.Windows.Forms.ListView();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
			this.mediaContextMenu = new System.Windows.Forms.ContextMenu();
			this.mediaDisplayPropMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.treeImageList = new System.Windows.Forms.ImageList(this.components);
			this.topPanel = new System.Windows.Forms.Panel();
			this.searchButton = new System.Windows.Forms.Button();
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.label1 = new System.Windows.Forms.Label();
			this.searchTextBox = new System.Windows.Forms.TextBox();
			this.topPanel.SuspendLayout();
			this.SuspendLayout();
			// 
			// mediaListView
			// 
			this.mediaListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																							this.columnHeader1,
																							this.columnHeader2,
																							this.columnHeader3});
			this.mediaListView.ContextMenu = this.mediaContextMenu;
			this.mediaListView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.mediaListView.FullRowSelect = true;
			this.mediaListView.Location = new System.Drawing.Point(0, 48);
			this.mediaListView.Name = "mediaListView";
			this.mediaListView.Size = new System.Drawing.Size(464, 310);
			this.mediaListView.SmallImageList = this.treeImageList;
			this.mediaListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.mediaListView.TabIndex = 0;
			this.mediaListView.View = System.Windows.Forms.View.Details;
			this.mediaListView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseDown);
			this.mediaListView.DoubleClick += new System.EventHandler(this.mediaListView_DoubleClick);
			this.mediaListView.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseUp);
			this.mediaListView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mediaListView_MouseMove);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Title";
			this.columnHeader1.Width = 183;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Creator";
			this.columnHeader2.Width = 192;
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "Size";
			this.columnHeader3.Width = 67;
			// 
			// mediaContextMenu
			// 
			this.mediaContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							 this.mediaDisplayPropMenuItem,
																							 this.menuItem3});
			this.mediaContextMenu.Popup += new System.EventHandler(this.mediaContextMenu_Popup);
			// 
			// mediaDisplayPropMenuItem
			// 
			this.mediaDisplayPropMenuItem.DefaultItem = true;
			this.mediaDisplayPropMenuItem.Index = 0;
			this.mediaDisplayPropMenuItem.Text = "Display Properties";
			this.mediaDisplayPropMenuItem.Click += new System.EventHandler(this.mediaDisplayPropMenuItem_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 1;
			this.menuItem3.Text = "-";
			// 
			// treeImageList
			// 
			this.treeImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.treeImageList.ImageSize = new System.Drawing.Size(16, 16);
			this.treeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("treeImageList.ImageStream")));
			this.treeImageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// topPanel
			// 
			this.topPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.topPanel.Controls.AddRange(new System.Windows.Forms.Control[] {
																				   this.searchButton,
																				   this.pictureBox1,
																				   this.label1,
																				   this.searchTextBox});
			this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
			this.topPanel.Name = "topPanel";
			this.topPanel.Size = new System.Drawing.Size(464, 48);
			this.topPanel.TabIndex = 3;
			// 
			// searchButton
			// 
			this.searchButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.searchButton.Location = new System.Drawing.Point(404, 0);
			this.searchButton.Name = "searchButton";
			this.searchButton.Size = new System.Drawing.Size(56, 44);
			this.searchButton.TabIndex = 6;
			this.searchButton.Text = "Search";
			this.searchButton.Click += new System.EventHandler(this.searchButton_Click);
			// 
			// pictureBox1
			// 
			this.pictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox1.Image")));
			this.pictureBox1.Location = new System.Drawing.Point(8, 8);
			this.pictureBox1.Name = "pictureBox1";
			this.pictureBox1.Size = new System.Drawing.Size(32, 32);
			this.pictureBox1.TabIndex = 5;
			this.pictureBox1.TabStop = false;
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(48, 5);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(328, 16);
			this.label1.TabIndex = 4;
			this.label1.Text = "Complete Media Title Search";
			// 
			// searchTextBox
			// 
			this.searchTextBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.searchTextBox.Location = new System.Drawing.Point(48, 22);
			this.searchTextBox.Name = "searchTextBox";
			this.searchTextBox.Size = new System.Drawing.Size(348, 20);
			this.searchTextBox.TabIndex = 3;
			this.searchTextBox.Text = "";
			this.searchTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.searchTextBox_KeyPress);
			// 
			// MediaSearchForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(464, 358);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.mediaListView,
																		  this.topPanel});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "MediaSearchForm";
			this.Text = "AV Media Controller Search";
			this.topPanel.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void searchButton_Click(object sender, System.EventArgs e)
		{
			mediaListView.Items.Clear();
			foreach (TreeNode node in cdsRootNode.Nodes) 
			{
				if (node.Tag.GetType() == typeof(CpRootContainer))
				{
					uint totalMatches;
					CpRootContainer rootContainer = (CpRootContainer)node.Tag;
					IList results = rootContainer.Search(new MediaComparer("dc:title contains \"" + searchTextBox.Text + "\""),0,100000,out totalMatches);

					mediaListView.BeginUpdate();
					foreach (IUPnPMedia item in results) 
					{
						string artist = "";
						if (item.MergedProperties[CommonPropertyNames.creator] != null && item.MergedProperties[CommonPropertyNames.creator].Count > 0)
						{
							artist = item.MergedProperties[CommonPropertyNames.creator][0].ToString();
						}
						string size = "";
						if (item.Resources.Length > 0)
						{
							MediaResource res = (MediaResource)item.Resources[0];
							//size = BuildSizeString(long.Parse(res.Size.ToString()));
							if (res.Size.IsValid)
							{
								size = BuildSizeString((long) res.Size.m_Value);//BuildSizeString(long.Parse(res.Size.ToString()));
							}
						}
						int icon = 4;
						string classtype = "";
						if (item.MergedProperties[CommonPropertyNames.Class] != null && item.MergedProperties[CommonPropertyNames.Class].Count > 0)
						{
							classtype = item.MergedProperties[CommonPropertyNames.Class][0].ToString();
							switch (classtype) 
							{
								case "object.container":
								case "object.container.storageFolder":
									icon = 2;
									break;
								case "object.item":
									icon = 8;
									break;
								case "object.item.imageItem":
								case "object.item.imageItem.photo":
									icon = 7;
									break;
								case "object.item.videoItem":
								case "object.item.videoItem.movie":
									icon = 6;
									break;
								case "object.item.audioItem":
								case "object.item.audioItem.musicTrack":
									icon = 5;
									break;
							}
						}
						ListViewItem l = new ListViewItem(new string[] {item.Title,artist,size},icon);
						l.Tag = item;
						mediaListView.Items.Add(l);
					}
					mediaListView.EndUpdate();

				}
			}
		}

		private string BuildSizeString(long size) 
		{
			double sized = (double)size;
			if (sized < 1200) return size.ToString() + " b";
			if (sized < 1200000) return Math.Round(sized/1024,1).ToString() + " Kb";
			return Math.Round((sized/1024)/1024,1).ToString() + " Mb";
		}

		private void searchTextBox_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			if (e.KeyChar == 13)
			{
				searchButton_Click(this,null);
				e.Handled = true;
			}
		}

		private void mediaListView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right)
			{
				if (dragStartPoint.X != e.X || dragStartPoint.Y != e.Y) 
				{
					ArrayList mediaList = new ArrayList();
					foreach (ListViewItem i in mediaListView.SelectedItems) 
					{
						if (i.Tag.GetType() == typeof(CpMediaItem)) 
						{
							mediaList.Add((CpMediaItem)i.Tag);
						}
					}
					if (mediaList.Count > 0) 
					{
						CpMediaItem[] mediaArray = (CpMediaItem[])mediaList.ToArray(typeof(CpMediaItem));
						mediaListView.DoDragDrop(mediaArray, DragDropEffects.Copy | DragDropEffects.Move);
					}
				}
			}
		}

		private void mediaListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			dragStartPoint.X = e.X;
			dragStartPoint.Y = e.Y;
		}

		private void mediaListView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			dragStartPoint.X = 0;
			dragStartPoint.Y = 0;
		}

		private void mediaContextMenu_Popup(object sender, System.EventArgs e)
		{
			while (mediaContextMenu.MenuItems.Count > 2) 
			{
				mediaContextMenu.MenuItems.RemoveAt(2);
			}

			PlayToRendererMenuItemMapping.Clear();
			menuItem3.Visible = false;
			if (rendererRootNode.Nodes.Count > 0) 
			{
				menuItem3.Visible = true;
				foreach (TreeNode node in rendererRootNode.Nodes) 
				{
					AVRenderer renderer = (AVRenderer)node.Tag;
					if (renderer != null)
					{
						MenuItem m = new MenuItem("Send to " + renderer.FriendlyName,new EventHandler(PlayMediaMenuSelectedSink));
						PlayToRendererMenuItemMapping.Add(m,renderer);
						mediaContextMenu.MenuItems.Add(m);
					}
				}
			}
		}

		private void PlayMediaMenuSelectedSink(object sender, EventArgs e)
		{
			if (mediaListView.SelectedItems.Count == 0) return;

			AVRenderer selectedSendToRenderer = (AVRenderer)PlayToRendererMenuItemMapping[sender];

			lock (mediaListView) 
			{
				ArrayList mediaList = new ArrayList();
				foreach (ListViewItem li in mediaListView.SelectedItems) 
				{
					if (li.Tag.GetType() == typeof(CpMediaItem)) 
					{
						mediaList.Add(li.Tag);
					}
				}
				parent.PopupRendererForm(selectedSendToRenderer,(CpMediaItem[])mediaList.ToArray(typeof(CpMediaItem)));
			}
		}

		private void mediaDisplayPropMenuItem_Click(object sender, System.EventArgs e)
		{
			if (mediaListView.SelectedItems.Count == 0) return;

			if (mediaListView.SelectedItems[0].Tag.GetType() == typeof(CpMediaItem)) 
			{
				CpMediaItem mi = (CpMediaItem)mediaListView.SelectedItems[0].Tag;
				parent.PopupMediaPropertyDialog(mi);
			}
		}

		private void mediaListView_DoubleClick(object sender, System.EventArgs e)
		{
			if (mediaListView.SelectedItems.Count == 0) return;

			if (mediaListView.SelectedItems[0].Tag.GetType() == typeof(CpMediaItem)) 
			{
				CpMediaItem mi = (CpMediaItem)mediaListView.SelectedItems[0].Tag;
				parent.PopupMediaPropertyDialog(mi);
			}
		}

	}
}
