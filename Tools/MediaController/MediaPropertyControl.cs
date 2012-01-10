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
using System.Data;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for MediaPropertyControl.
	/// </summary>
	public class MediaPropertyControl : System.Windows.Forms.UserControl
	{
		private CpMediaItem item;

		private System.Windows.Forms.PictureBox gearsDocPictureBox;
		private System.Windows.Forms.PictureBox imageDocPictureBox;
		private System.Windows.Forms.PictureBox musicDocPictureBox;
		private System.Windows.Forms.PictureBox videoDocPictureBox;
		private System.Windows.Forms.PictureBox unknownDocPictureBox;
		private System.Windows.Forms.PictureBox emptyDocPictureBox;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label authorLabel;
		private System.Windows.Forms.Label titleLabel;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.ListView valueListView;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.ListBox propListBox;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.PictureBox classTypePictureBox;
		private System.Windows.Forms.ContextMenu valueListContextMenu;
		private System.Windows.Forms.MenuItem copyMenuItem;
		private System.Windows.Forms.MenuItem openMenuItem;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CpMediaItem MediaItem 
		{
			get 
			{
				return item;
			}
			set 
			{
				item = value;
				RefreshUserInterface();
			}
		}

		public MediaPropertyControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitForm call
			RefreshUserInterface();
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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MediaPropertyControl));
			this.gearsDocPictureBox = new System.Windows.Forms.PictureBox();
			this.imageDocPictureBox = new System.Windows.Forms.PictureBox();
			this.musicDocPictureBox = new System.Windows.Forms.PictureBox();
			this.videoDocPictureBox = new System.Windows.Forms.PictureBox();
			this.unknownDocPictureBox = new System.Windows.Forms.PictureBox();
			this.emptyDocPictureBox = new System.Windows.Forms.PictureBox();
			this.panel1 = new System.Windows.Forms.Panel();
			this.authorLabel = new System.Windows.Forms.Label();
			this.titleLabel = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.valueListView = new System.Windows.Forms.ListView();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
			this.valueListContextMenu = new System.Windows.Forms.ContextMenu();
			this.copyMenuItem = new System.Windows.Forms.MenuItem();
			this.openMenuItem = new System.Windows.Forms.MenuItem();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.propListBox = new System.Windows.Forms.ListBox();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.label2 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.classTypePictureBox = new System.Windows.Forms.PictureBox();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// gearsDocPictureBox
			// 
			this.gearsDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("gearsDocPictureBox.Image")));
			this.gearsDocPictureBox.Location = new System.Drawing.Point(168, 320);
			this.gearsDocPictureBox.Name = "gearsDocPictureBox";
			this.gearsDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.gearsDocPictureBox.TabIndex = 30;
			this.gearsDocPictureBox.TabStop = false;
			this.gearsDocPictureBox.Visible = false;
			// 
			// imageDocPictureBox
			// 
			this.imageDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("imageDocPictureBox.Image")));
			this.imageDocPictureBox.Location = new System.Drawing.Point(136, 320);
			this.imageDocPictureBox.Name = "imageDocPictureBox";
			this.imageDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.imageDocPictureBox.TabIndex = 29;
			this.imageDocPictureBox.TabStop = false;
			this.imageDocPictureBox.Visible = false;
			// 
			// musicDocPictureBox
			// 
			this.musicDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("musicDocPictureBox.Image")));
			this.musicDocPictureBox.Location = new System.Drawing.Point(104, 320);
			this.musicDocPictureBox.Name = "musicDocPictureBox";
			this.musicDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.musicDocPictureBox.TabIndex = 28;
			this.musicDocPictureBox.TabStop = false;
			this.musicDocPictureBox.Visible = false;
			// 
			// videoDocPictureBox
			// 
			this.videoDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("videoDocPictureBox.Image")));
			this.videoDocPictureBox.Location = new System.Drawing.Point(72, 320);
			this.videoDocPictureBox.Name = "videoDocPictureBox";
			this.videoDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.videoDocPictureBox.TabIndex = 27;
			this.videoDocPictureBox.TabStop = false;
			this.videoDocPictureBox.Visible = false;
			// 
			// unknownDocPictureBox
			// 
			this.unknownDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("unknownDocPictureBox.Image")));
			this.unknownDocPictureBox.Location = new System.Drawing.Point(40, 320);
			this.unknownDocPictureBox.Name = "unknownDocPictureBox";
			this.unknownDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.unknownDocPictureBox.TabIndex = 26;
			this.unknownDocPictureBox.TabStop = false;
			this.unknownDocPictureBox.Visible = false;
			// 
			// emptyDocPictureBox
			// 
			this.emptyDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("emptyDocPictureBox.Image")));
			this.emptyDocPictureBox.Location = new System.Drawing.Point(8, 320);
			this.emptyDocPictureBox.Name = "emptyDocPictureBox";
			this.emptyDocPictureBox.Size = new System.Drawing.Size(32, 40);
			this.emptyDocPictureBox.TabIndex = 25;
			this.emptyDocPictureBox.TabStop = false;
			this.emptyDocPictureBox.Visible = false;
			// 
			// panel1
			// 
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.authorLabel,
																				 this.titleLabel,
																				 this.panel2,
																				 this.pictureBox2,
																				 this.label2,
																				 this.label1,
																				 this.classTypePictureBox});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(472, 168);
			this.panel1.TabIndex = 31;
			// 
			// authorLabel
			// 
			this.authorLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.authorLabel.Location = new System.Drawing.Point(80, 24);
			this.authorLabel.Name = "authorLabel";
			this.authorLabel.Size = new System.Drawing.Size(380, 16);
			this.authorLabel.TabIndex = 31;
			// 
			// titleLabel
			// 
			this.titleLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.titleLabel.Location = new System.Drawing.Point(80, 8);
			this.titleLabel.Name = "titleLabel";
			this.titleLabel.Size = new System.Drawing.Size(380, 16);
			this.titleLabel.TabIndex = 30;
			// 
			// panel2
			// 
			this.panel2.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.valueListView,
																				 this.splitter1,
																				 this.propListBox});
			this.panel2.Location = new System.Drawing.Point(8, 48);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(452, 108);
			this.panel2.TabIndex = 29;
			// 
			// valueListView
			// 
			this.valueListView.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.valueListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																							this.columnHeader1,
																							this.columnHeader2});
			this.valueListView.ContextMenu = this.valueListContextMenu;
			this.valueListView.FullRowSelect = true;
			this.valueListView.Location = new System.Drawing.Point(115, 0);
			this.valueListView.MultiSelect = false;
			this.valueListView.Name = "valueListView";
			this.valueListView.Size = new System.Drawing.Size(337, 108);
			this.valueListView.TabIndex = 9;
			this.valueListView.View = System.Windows.Forms.View.Details;
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Name";
			this.columnHeader1.Width = 95;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Value";
			this.columnHeader2.Width = 185;
			// 
			// valueListContextMenu
			// 
			this.valueListContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																								 this.copyMenuItem,
																								 this.openMenuItem});
			this.valueListContextMenu.Popup += new System.EventHandler(this.valueListContextMenu_Popup);
			// 
			// copyMenuItem
			// 
			this.copyMenuItem.Index = 0;
			this.copyMenuItem.Text = "&Copy";
			this.copyMenuItem.Click += new System.EventHandler(this.copyMenuItem_Click);
			// 
			// openMenuItem
			// 
			this.openMenuItem.Index = 1;
			this.openMenuItem.Text = "&Open URI";
			this.openMenuItem.Click += new System.EventHandler(this.openMenuItem_Click);
			// 
			// splitter1
			// 
			this.splitter1.Location = new System.Drawing.Point(112, 0);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(3, 108);
			this.splitter1.TabIndex = 8;
			this.splitter1.TabStop = false;
			// 
			// propListBox
			// 
			this.propListBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.propListBox.IntegralHeight = false;
			this.propListBox.Name = "propListBox";
			this.propListBox.Size = new System.Drawing.Size(112, 108);
			this.propListBox.TabIndex = 7;
			this.propListBox.SelectedIndexChanged += new System.EventHandler(this.propListBox_SelectedIndexChanged);
			// 
			// pictureBox2
			// 
			this.pictureBox2.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.pictureBox2.BackColor = System.Drawing.Color.Gray;
			this.pictureBox2.Location = new System.Drawing.Point(8, 43);
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.Size = new System.Drawing.Size(452, 3);
			this.pictureBox2.TabIndex = 28;
			this.pictureBox2.TabStop = false;
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(40, 24);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(40, 16);
			this.label2.TabIndex = 26;
			this.label2.Text = "Author";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(40, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(40, 16);
			this.label1.TabIndex = 25;
			this.label1.Text = "Title";
			// 
			// classTypePictureBox
			// 
			this.classTypePictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("classTypePictureBox.Image")));
			this.classTypePictureBox.Location = new System.Drawing.Point(8, 8);
			this.classTypePictureBox.Name = "classTypePictureBox";
			this.classTypePictureBox.Size = new System.Drawing.Size(32, 32);
			this.classTypePictureBox.TabIndex = 27;
			this.classTypePictureBox.TabStop = false;
			this.classTypePictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.classTypePictureBox_MouseDown);
			// 
			// MediaPropertyControl
			// 
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1,
																		  this.gearsDocPictureBox,
																		  this.imageDocPictureBox,
																		  this.musicDocPictureBox,
																		  this.videoDocPictureBox,
																		  this.unknownDocPictureBox,
																		  this.emptyDocPictureBox});
			this.Name = "MediaPropertyControl";
			this.Size = new System.Drawing.Size(472, 168);
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void RefreshUserInterface() 
		{
			propListBox.Items.Clear();
			valueListView.Items.Clear();
			if (item == null) 
			{
				authorLabel.Text = "";
				titleLabel.Text = "None";
				classTypePictureBox.Image = unknownDocPictureBox.Image;
			} 
			else 
			{
				titleLabel.Text = item.Title;
				authorLabel.Text = item.Creator;

				propListBox.Items.Add("Properties");
				int rc = 0;
				foreach (MediaResource res in item.MergedResources) 
				{
					rc++;
					propListBox.Items.Add("Resource #" + rc);
				}
				propListBox.SelectedIndex = 0;

				string classtype = "";
				if (item.MergedProperties[CommonPropertyNames.Class] != null && item.MergedProperties[CommonPropertyNames.Class].Count > 0)
				{
					classtype = item.MergedProperties[CommonPropertyNames.Class][0].ToString();
					switch (classtype) 
					{
						case "object.item":
							classTypePictureBox.Image = gearsDocPictureBox.Image;
							break;
						case "object.item.imageItem":
						case "object.item.imageItem.photo":
							classTypePictureBox.Image = imageDocPictureBox.Image;
							break;
						case "object.item.videoItem":
						case "object.item.videoItem.movie":
							classTypePictureBox.Image = videoDocPictureBox.Image;
							break;
						case "object.item.audioItem":
						case "object.item.audioItem.musicTrack":
							classTypePictureBox.Image = musicDocPictureBox.Image;
							break;
					}
				}

			}
		}

		private void propListBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if (propListBox.SelectedIndices.Count != 1) return;
			if (propListBox.SelectedItem.ToString().CompareTo("Properties") == 0)
			{
				valueListView.Items.Clear();
				foreach (string propertyName in item.MergedProperties.PropertyNames) 
				{
					int rc = 0;
					foreach (object propertyvalue in item.MergedProperties[propertyName]) 
					{
						rc++;
						if (rc == 1) 
						{
							valueListView.Items.Add(new ListViewItem(new string[] {propertyName,propertyvalue.ToString()}));
						} 
						else 
						{
							valueListView.Items.Add(new ListViewItem(new string[] {propertyName + "(" + rc + ")",propertyvalue.ToString()}));
						}
					}
				}				
			}
			if (propListBox.SelectedItem.ToString().StartsWith("Resource")) 
			{
				MediaResource res = (MediaResource)item.MergedResources[propListBox.SelectedIndex-1];
				valueListView.Items.Clear();
				valueListView.Items.Add(new ListViewItem(new string[] {"Content URI",res.ContentUri.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Import URI",res.ImportUri.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Information",res.ProtocolInfo.Info}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Mime Type",res.ProtocolInfo.MimeType}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Network",res.ProtocolInfo.Network}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Protocol",res.ProtocolInfo.Protocol}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Protocol Info",res.ProtocolInfo.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Size",res.Size.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Bit Rate",res.Bitrate.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Bits Per Sample",res.BitsPerSample.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Color Depth",res.ColorDepth.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Duration",res.Duration.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"NR Audio Channels",res.nrAudioChannels.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Protection",res.Protection}));
				valueListView.Items.Add(new ListViewItem(new string[] {"Resolution",res.Resolution.ToString()}));
				valueListView.Items.Add(new ListViewItem(new string[] {"SampleFrequency",res.SampleFrequency.ToString()}));
			}
		}
		
		private void valueListContextMenu_Popup(object sender, System.EventArgs e)
		{
			copyMenuItem.Visible = (valueListView.SelectedItems.Count > 0);
			openMenuItem.Visible = (valueListView.SelectedItems.Count > 0 && valueListView.SelectedItems[0].SubItems[1].Text.ToLower().StartsWith("http://"));
		}

		private void classTypePictureBox_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (item != null)
			{
				CpMediaItem[] items = new CpMediaItem[1];
				items[0] = item;
				classTypePictureBox.DoDragDrop(items, DragDropEffects.Copy | DragDropEffects.Move);
			}
		}

		private void copyMenuItem_Click(object sender, System.EventArgs e)
		{
			if (valueListView.SelectedItems.Count == 0) return;
			Clipboard.SetDataObject(valueListView.SelectedItems[0].SubItems[1].Text);
		}

		private void openMenuItem_Click(object sender, System.EventArgs e)
		{
			if (valueListView.SelectedItems.Count == 0) return;
			if (valueListView.SelectedItems[0].SubItems[1].Text.ToLower().StartsWith("http://") == false) return;
            try
            {
                System.Diagnostics.Process.Start("\"" + valueListView.SelectedItems[0].SubItems[1].Text + "\"");
            }
            catch (System.ComponentModel.Win32Exception) { }
		}

	}
}
