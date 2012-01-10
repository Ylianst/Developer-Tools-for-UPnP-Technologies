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
using System.IO;
using System.Data;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnpSpy
{
	/// <summary>
	/// Summary description for UPnpArgumentControl.
	/// </summary>
	public class UPnpArgumentControl : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.ImageList actionImageList;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.TextBox argValueTextBox;
		private System.Windows.Forms.Label actionNameLabel;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.PictureBox argDirPictureBox;
		private System.Windows.Forms.ComboBox argValueComboBox;
		private UPnPArgument arg = null;
		private System.Windows.Forms.ToolTip toolTip;
		private System.Windows.Forms.ContextMenu sizeContextMenu;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem sendToNotepadMenuItem;
		private System.Windows.Forms.MenuItem sendToExplorerMenuItem;
		private System.Windows.Forms.MenuItem menuItem7;
		private bool textBoxActive = true;

		public UPnPArgument UPnPArgument
		{
			get {return arg;}
		}

		public object ArgumentValue
		{
			get
			{
				if (textBoxActive == true) 
				{
					try
					{
						return UPnPService.CreateObjectInstance(UPnPStateVariable.ConvertFromUPnPType(arg.RelatedStateVar.ValueType),argValueTextBox.Text);
					}
					catch
					{
						return(argValueTextBox.Text);
					}
				}
				else 
				{
					return UPnPService.CreateObjectInstance(UPnPStateVariable.ConvertFromUPnPType(arg.RelatedStateVar.ValueType),argValueComboBox.Text);
				}					
			}
			set
			{
				if (value == null)
				{
					argValueTextBox.Text = "";
					argValueComboBox.Text = "";
				}
				else 
				{
					argValueTextBox.Text = UPnPService.SerializeObjectInstance(value);
					argValueComboBox.Text = UPnPService.SerializeObjectInstance(value);
				}
			}
		}

		public UPnpArgumentControl(UPnPArgument argument)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			arg = argument;

			// Set name
			actionNameLabel.Text = "(" + arg.RelatedStateVar.ValueType + ") " + arg.Name;
			toolTip.SetToolTip(actionNameLabel,"(" + arg.RelatedStateVar.ValueType + ") " + arg.Name);

			// Set input/output
			if (arg.IsReturnValue == true) 
			{
				argDirPictureBox.Image = actionImageList.Images[2];
				argValueTextBox.ReadOnly = true;
			} 
			else 
			{
				argValueTextBox.ReadOnly = (arg.Direction == "out");
				argValueComboBox.Enabled = (arg.Direction != "out");

				if (arg.Direction == "in") 
				{
					argDirPictureBox.Image = actionImageList.Images[0];
					toolTip.SetToolTip(argDirPictureBox,"Input argument");
				} 
				else 
				if (arg.Direction == "out")
				{
					argDirPictureBox.Image = actionImageList.Images[1];
					toolTip.SetToolTip(argDirPictureBox,"Ouput argument");
				} 
				else 
				{
					argDirPictureBox.Image = actionImageList.Images[2];
					toolTip.SetToolTip(argDirPictureBox,"Return argument");
				}
			}

			// Set type
			if (arg.Direction == "in") 
			{
				if (arg.RelatedStateVar.GetNetType() == typeof(bool)) 
				{
					argValueTextBox.Visible = false;
					argValueComboBox.Visible = true;
					textBoxActive = false;
					argValueComboBox.Items.Add("True");
					argValueComboBox.Items.Add("False");
					argValueComboBox.SelectedIndex = 0;
				} 
				else
				if (arg.RelatedStateVar.AllowedStringValues != null) 
				{
					argValueTextBox.Visible = false;
					argValueComboBox.Visible = true;
					textBoxActive = false;
					foreach (string s in arg.RelatedStateVar.AllowedStringValues) 
					{
						argValueComboBox.Items.Add(s);
					}
				}
			}
			else 
			{
				argValueTextBox.Visible = true;
				argValueComboBox.Visible = false;
				textBoxActive = true;
			}

			// Place string vertical scroll
			if (argValueComboBox.Visible == false && arg.RelatedStateVar.ValueType == "string") 
			{
				argValueTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
				Height = 60;
			}

			// Place default values
			if (argValueTextBox.ReadOnly == false) 
			{
				if (arg.RelatedStateVar.DefaultValue != null) 
				{
					argValueTextBox.Text = UPnPService.SerializeObjectInstance(arg.RelatedStateVar.DefaultValue);
				} 
				else
				if (arg.RelatedStateVar.ValueType == "int" || arg.RelatedStateVar.ValueType == "ui4") 
				{
					argValueTextBox.Text = "0";
				} 
				else
				if (arg.RelatedStateVar.ValueType == "uri") 
				{
					argValueTextBox.Text = "http://";
				}
			}

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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(UPnpArgumentControl));
			this.actionImageList = new System.Windows.Forms.ImageList(this.components);
			this.panel1 = new System.Windows.Forms.Panel();
			this.argValueTextBox = new System.Windows.Forms.TextBox();
			this.argValueComboBox = new System.Windows.Forms.ComboBox();
			this.actionNameLabel = new System.Windows.Forms.Label();
			this.argDirPictureBox = new System.Windows.Forms.PictureBox();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.sizeContextMenu = new System.Windows.Forms.ContextMenu();
			this.sendToNotepadMenuItem = new System.Windows.Forms.MenuItem();
			this.sendToExplorerMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// actionImageList
			// 
			this.actionImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth24Bit;
			this.actionImageList.ImageSize = ((System.Drawing.Size)(resources.GetObject("actionImageList.ImageSize")));
			this.actionImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("actionImageList.ImageStream")));
			this.actionImageList.TransparentColor = System.Drawing.Color.White;
			// 
			// panel1
			// 
			this.panel1.AccessibleDescription = ((string)(resources.GetObject("panel1.AccessibleDescription")));
			this.panel1.AccessibleName = ((string)(resources.GetObject("panel1.AccessibleName")));
			this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("panel1.Anchor")));
			this.panel1.AutoScroll = ((bool)(resources.GetObject("panel1.AutoScroll")));
			this.panel1.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("panel1.AutoScrollMargin")));
			this.panel1.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("panel1.AutoScrollMinSize")));
			this.panel1.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("panel1.BackgroundImage")));
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.argValueTextBox,
																				 this.argValueComboBox,
																				 this.actionNameLabel,
																				 this.argDirPictureBox});
			this.panel1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("panel1.Dock")));
			this.panel1.Enabled = ((bool)(resources.GetObject("panel1.Enabled")));
			this.panel1.Font = ((System.Drawing.Font)(resources.GetObject("panel1.Font")));
			this.panel1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("panel1.ImeMode")));
			this.panel1.Location = ((System.Drawing.Point)(resources.GetObject("panel1.Location")));
			this.panel1.Name = "panel1";
			this.panel1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("panel1.RightToLeft")));
			this.panel1.Size = ((System.Drawing.Size)(resources.GetObject("panel1.Size")));
			this.panel1.TabIndex = ((int)(resources.GetObject("panel1.TabIndex")));
			this.panel1.Text = resources.GetString("panel1.Text");
			this.toolTip.SetToolTip(this.panel1, resources.GetString("panel1.ToolTip"));
			this.panel1.Visible = ((bool)(resources.GetObject("panel1.Visible")));
			// 
			// argValueTextBox
			// 
			this.argValueTextBox.AccessibleDescription = ((string)(resources.GetObject("argValueTextBox.AccessibleDescription")));
			this.argValueTextBox.AccessibleName = ((string)(resources.GetObject("argValueTextBox.AccessibleName")));
			this.argValueTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("argValueTextBox.Anchor")));
			this.argValueTextBox.AutoSize = ((bool)(resources.GetObject("argValueTextBox.AutoSize")));
			this.argValueTextBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("argValueTextBox.BackgroundImage")));
			this.argValueTextBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("argValueTextBox.Dock")));
			this.argValueTextBox.Enabled = ((bool)(resources.GetObject("argValueTextBox.Enabled")));
			this.argValueTextBox.Font = ((System.Drawing.Font)(resources.GetObject("argValueTextBox.Font")));
			this.argValueTextBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("argValueTextBox.ImeMode")));
			this.argValueTextBox.Location = ((System.Drawing.Point)(resources.GetObject("argValueTextBox.Location")));
			this.argValueTextBox.MaxLength = ((int)(resources.GetObject("argValueTextBox.MaxLength")));
			this.argValueTextBox.Multiline = ((bool)(resources.GetObject("argValueTextBox.Multiline")));
			this.argValueTextBox.Name = "argValueTextBox";
			this.argValueTextBox.PasswordChar = ((char)(resources.GetObject("argValueTextBox.PasswordChar")));
			this.argValueTextBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("argValueTextBox.RightToLeft")));
			this.argValueTextBox.ScrollBars = ((System.Windows.Forms.ScrollBars)(resources.GetObject("argValueTextBox.ScrollBars")));
			this.argValueTextBox.Size = ((System.Drawing.Size)(resources.GetObject("argValueTextBox.Size")));
			this.argValueTextBox.TabIndex = ((int)(resources.GetObject("argValueTextBox.TabIndex")));
			this.argValueTextBox.Text = resources.GetString("argValueTextBox.Text");
			this.argValueTextBox.TextAlign = ((System.Windows.Forms.HorizontalAlignment)(resources.GetObject("argValueTextBox.TextAlign")));
			this.toolTip.SetToolTip(this.argValueTextBox, resources.GetString("argValueTextBox.ToolTip"));
			this.argValueTextBox.Visible = ((bool)(resources.GetObject("argValueTextBox.Visible")));
			this.argValueTextBox.WordWrap = ((bool)(resources.GetObject("argValueTextBox.WordWrap")));
			this.argValueTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.argValueTextBox_KeyDown);
			// 
			// argValueComboBox
			// 
			this.argValueComboBox.AccessibleDescription = ((string)(resources.GetObject("argValueComboBox.AccessibleDescription")));
			this.argValueComboBox.AccessibleName = ((string)(resources.GetObject("argValueComboBox.AccessibleName")));
			this.argValueComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("argValueComboBox.Anchor")));
			this.argValueComboBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("argValueComboBox.BackgroundImage")));
			this.argValueComboBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("argValueComboBox.Dock")));
			this.argValueComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.argValueComboBox.Enabled = ((bool)(resources.GetObject("argValueComboBox.Enabled")));
			this.argValueComboBox.Font = ((System.Drawing.Font)(resources.GetObject("argValueComboBox.Font")));
			this.argValueComboBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("argValueComboBox.ImeMode")));
			this.argValueComboBox.IntegralHeight = ((bool)(resources.GetObject("argValueComboBox.IntegralHeight")));
			this.argValueComboBox.ItemHeight = ((int)(resources.GetObject("argValueComboBox.ItemHeight")));
			this.argValueComboBox.Location = ((System.Drawing.Point)(resources.GetObject("argValueComboBox.Location")));
			this.argValueComboBox.MaxDropDownItems = ((int)(resources.GetObject("argValueComboBox.MaxDropDownItems")));
			this.argValueComboBox.MaxLength = ((int)(resources.GetObject("argValueComboBox.MaxLength")));
			this.argValueComboBox.Name = "argValueComboBox";
			this.argValueComboBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("argValueComboBox.RightToLeft")));
			this.argValueComboBox.Size = ((System.Drawing.Size)(resources.GetObject("argValueComboBox.Size")));
			this.argValueComboBox.TabIndex = ((int)(resources.GetObject("argValueComboBox.TabIndex")));
			this.argValueComboBox.Text = resources.GetString("argValueComboBox.Text");
			this.toolTip.SetToolTip(this.argValueComboBox, resources.GetString("argValueComboBox.ToolTip"));
			this.argValueComboBox.Visible = ((bool)(resources.GetObject("argValueComboBox.Visible")));
			// 
			// actionNameLabel
			// 
			this.actionNameLabel.AccessibleDescription = ((string)(resources.GetObject("actionNameLabel.AccessibleDescription")));
			this.actionNameLabel.AccessibleName = ((string)(resources.GetObject("actionNameLabel.AccessibleName")));
			this.actionNameLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("actionNameLabel.Anchor")));
			this.actionNameLabel.AutoSize = ((bool)(resources.GetObject("actionNameLabel.AutoSize")));
			this.actionNameLabel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("actionNameLabel.Dock")));
			this.actionNameLabel.Enabled = ((bool)(resources.GetObject("actionNameLabel.Enabled")));
			this.actionNameLabel.Font = ((System.Drawing.Font)(resources.GetObject("actionNameLabel.Font")));
			this.actionNameLabel.Image = ((System.Drawing.Image)(resources.GetObject("actionNameLabel.Image")));
			this.actionNameLabel.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("actionNameLabel.ImageAlign")));
			this.actionNameLabel.ImageIndex = ((int)(resources.GetObject("actionNameLabel.ImageIndex")));
			this.actionNameLabel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("actionNameLabel.ImeMode")));
			this.actionNameLabel.Location = ((System.Drawing.Point)(resources.GetObject("actionNameLabel.Location")));
			this.actionNameLabel.Name = "actionNameLabel";
			this.actionNameLabel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("actionNameLabel.RightToLeft")));
			this.actionNameLabel.Size = ((System.Drawing.Size)(resources.GetObject("actionNameLabel.Size")));
			this.actionNameLabel.TabIndex = ((int)(resources.GetObject("actionNameLabel.TabIndex")));
			this.actionNameLabel.Text = resources.GetString("actionNameLabel.Text");
			this.actionNameLabel.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("actionNameLabel.TextAlign")));
			this.toolTip.SetToolTip(this.actionNameLabel, resources.GetString("actionNameLabel.ToolTip"));
			this.actionNameLabel.Visible = ((bool)(resources.GetObject("actionNameLabel.Visible")));
			// 
			// argDirPictureBox
			// 
			this.argDirPictureBox.AccessibleDescription = ((string)(resources.GetObject("argDirPictureBox.AccessibleDescription")));
			this.argDirPictureBox.AccessibleName = ((string)(resources.GetObject("argDirPictureBox.AccessibleName")));
			this.argDirPictureBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("argDirPictureBox.Anchor")));
			this.argDirPictureBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("argDirPictureBox.BackgroundImage")));
			this.argDirPictureBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("argDirPictureBox.Dock")));
			this.argDirPictureBox.Enabled = ((bool)(resources.GetObject("argDirPictureBox.Enabled")));
			this.argDirPictureBox.Font = ((System.Drawing.Font)(resources.GetObject("argDirPictureBox.Font")));
			this.argDirPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("argDirPictureBox.Image")));
			this.argDirPictureBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("argDirPictureBox.ImeMode")));
			this.argDirPictureBox.Location = ((System.Drawing.Point)(resources.GetObject("argDirPictureBox.Location")));
			this.argDirPictureBox.Name = "argDirPictureBox";
			this.argDirPictureBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("argDirPictureBox.RightToLeft")));
			this.argDirPictureBox.Size = ((System.Drawing.Size)(resources.GetObject("argDirPictureBox.Size")));
			this.argDirPictureBox.SizeMode = ((System.Windows.Forms.PictureBoxSizeMode)(resources.GetObject("argDirPictureBox.SizeMode")));
			this.argDirPictureBox.TabIndex = ((int)(resources.GetObject("argDirPictureBox.TabIndex")));
			this.argDirPictureBox.TabStop = false;
			this.argDirPictureBox.Text = resources.GetString("argDirPictureBox.Text");
			this.toolTip.SetToolTip(this.argDirPictureBox, resources.GetString("argDirPictureBox.ToolTip"));
			this.argDirPictureBox.Visible = ((bool)(resources.GetObject("argDirPictureBox.Visible")));
			// 
			// sizeContextMenu
			// 
			this.sizeContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							this.sendToNotepadMenuItem,
																							this.sendToExplorerMenuItem,
																							this.menuItem7,
																							this.menuItem1,
																							this.menuItem2,
																							this.menuItem3,
																							this.menuItem4});
			this.sizeContextMenu.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("sizeContextMenu.RightToLeft")));
			// 
			// sendToNotepadMenuItem
			// 
			this.sendToNotepadMenuItem.Enabled = ((bool)(resources.GetObject("sendToNotepadMenuItem.Enabled")));
			this.sendToNotepadMenuItem.Index = 0;
			this.sendToNotepadMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("sendToNotepadMenuItem.Shortcut")));
			this.sendToNotepadMenuItem.ShowShortcut = ((bool)(resources.GetObject("sendToNotepadMenuItem.ShowShortcut")));
			this.sendToNotepadMenuItem.Text = resources.GetString("sendToNotepadMenuItem.Text");
			this.sendToNotepadMenuItem.Visible = ((bool)(resources.GetObject("sendToNotepadMenuItem.Visible")));
			this.sendToNotepadMenuItem.Click += new System.EventHandler(this.sendToNotepadMenuItem_Click);
			// 
			// sendToExplorerMenuItem
			// 
			this.sendToExplorerMenuItem.Enabled = ((bool)(resources.GetObject("sendToExplorerMenuItem.Enabled")));
			this.sendToExplorerMenuItem.Index = 1;
			this.sendToExplorerMenuItem.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("sendToExplorerMenuItem.Shortcut")));
			this.sendToExplorerMenuItem.ShowShortcut = ((bool)(resources.GetObject("sendToExplorerMenuItem.ShowShortcut")));
			this.sendToExplorerMenuItem.Text = resources.GetString("sendToExplorerMenuItem.Text");
			this.sendToExplorerMenuItem.Visible = ((bool)(resources.GetObject("sendToExplorerMenuItem.Visible")));
			this.sendToExplorerMenuItem.Click += new System.EventHandler(this.sendToExplorerMenuItem_Click);
			// 
			// menuItem7
			// 
			this.menuItem7.Enabled = ((bool)(resources.GetObject("menuItem7.Enabled")));
			this.menuItem7.Index = 2;
			this.menuItem7.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem7.Shortcut")));
			this.menuItem7.ShowShortcut = ((bool)(resources.GetObject("menuItem7.ShowShortcut")));
			this.menuItem7.Text = resources.GetString("menuItem7.Text");
			this.menuItem7.Visible = ((bool)(resources.GetObject("menuItem7.Visible")));
			// 
			// menuItem1
			// 
			this.menuItem1.Enabled = ((bool)(resources.GetObject("menuItem1.Enabled")));
			this.menuItem1.Index = 3;
			this.menuItem1.RadioCheck = true;
			this.menuItem1.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem1.Shortcut")));
			this.menuItem1.ShowShortcut = ((bool)(resources.GetObject("menuItem1.ShowShortcut")));
			this.menuItem1.Text = resources.GetString("menuItem1.Text");
			this.menuItem1.Visible = ((bool)(resources.GetObject("menuItem1.Visible")));
			this.menuItem1.Click += new System.EventHandler(this.menuItem1_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Enabled = ((bool)(resources.GetObject("menuItem2.Enabled")));
			this.menuItem2.Index = 4;
			this.menuItem2.RadioCheck = true;
			this.menuItem2.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem2.Shortcut")));
			this.menuItem2.ShowShortcut = ((bool)(resources.GetObject("menuItem2.ShowShortcut")));
			this.menuItem2.Text = resources.GetString("menuItem2.Text");
			this.menuItem2.Visible = ((bool)(resources.GetObject("menuItem2.Visible")));
			this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Enabled = ((bool)(resources.GetObject("menuItem3.Enabled")));
			this.menuItem3.Index = 5;
			this.menuItem3.RadioCheck = true;
			this.menuItem3.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem3.Shortcut")));
			this.menuItem3.ShowShortcut = ((bool)(resources.GetObject("menuItem3.ShowShortcut")));
			this.menuItem3.Text = resources.GetString("menuItem3.Text");
			this.menuItem3.Visible = ((bool)(resources.GetObject("menuItem3.Visible")));
			this.menuItem3.Click += new System.EventHandler(this.menuItem3_Click);
			// 
			// menuItem4
			// 
			this.menuItem4.Enabled = ((bool)(resources.GetObject("menuItem4.Enabled")));
			this.menuItem4.Index = 6;
			this.menuItem4.RadioCheck = true;
			this.menuItem4.Shortcut = ((System.Windows.Forms.Shortcut)(resources.GetObject("menuItem4.Shortcut")));
			this.menuItem4.ShowShortcut = ((bool)(resources.GetObject("menuItem4.ShowShortcut")));
			this.menuItem4.Text = resources.GetString("menuItem4.Text");
			this.menuItem4.Visible = ((bool)(resources.GetObject("menuItem4.Visible")));
			this.menuItem4.Click += new System.EventHandler(this.menuItem4_Click);
			// 
			// UPnpArgumentControl
			// 
			this.AccessibleDescription = ((string)(resources.GetObject("$this.AccessibleDescription")));
			this.AccessibleName = ((string)(resources.GetObject("$this.AccessibleName")));
			this.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("$this.Anchor")));
			this.AutoScroll = ((bool)(resources.GetObject("$this.AutoScroll")));
			this.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMargin")));
			this.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMinSize")));
			this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
			this.ContextMenu = this.sizeContextMenu;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1});
			this.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("$this.Dock")));
			this.Enabled = ((bool)(resources.GetObject("$this.Enabled")));
			this.Font = ((System.Drawing.Font)(resources.GetObject("$this.Font")));
			this.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("$this.ImeMode")));
			this.Location = ((System.Drawing.Point)(resources.GetObject("$this.Location")));
			this.Name = "UPnpArgumentControl";
			this.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("$this.RightToLeft")));
			this.Size = ((System.Drawing.Size)(resources.GetObject("$this.Size")));
			this.TabIndex = ((int)(resources.GetObject("$this.TabIndex")));
			this.toolTip.SetToolTip(this, resources.GetString("$this.ToolTip"));
			this.Visible = ((bool)(resources.GetObject("$this.Visible")));
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void argValueTextBox_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
		{
			if (textBoxActive == true) 
			{
				if (e.Modifiers == Keys.Control && e.KeyCode == Keys.A) 
				{
					argValueTextBox.SelectAll();
				}
				if (e.Modifiers == Keys.Control && e.KeyCode == Keys.C) 
				{
					Clipboard.SetDataObject(argValueTextBox.Text);
				}
			}
		}

		private void menuItem1_Click(object sender, System.EventArgs e)
		{
			Height = 32;
		}

		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			Height = 60;
		}

		private void menuItem3_Click(object sender, System.EventArgs e)
		{
			Height = 250;
		}

		private void menuItem4_Click(object sender, System.EventArgs e)
		{
			Height = 600;
		}

		private void sendToNotepadMenuItem_Click(object sender, System.EventArgs e)
		{
			string s = Application.StartupPath + "\\temp.txt";
			StreamWriter file = File.CreateText(s);
			file.Write(argValueTextBox.Text);
			file.Close();
            try
            {
                System.Diagnostics.Process.Start("notepad", s);
            }
            catch (System.ComponentModel.Win32Exception) { }
		}

		private void sendToExplorerMenuItem_Click(object sender, System.EventArgs e)
		{
			string s = Application.StartupPath + "\\temp.txt";
			string x = argValueTextBox.Text;
			if (x.StartsWith("<?xml") == true) 
			{
				s = Application.StartupPath + "\\temp.xml";
			}
			if (x.StartsWith("<DIDL") == true) 
			{
				x = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" + x;
				s = Application.StartupPath + "\\temp.xml";
			}
			StreamWriter file = File.CreateText(s);
			file.Write(x);
			file.Close();
            try
            {
                System.Diagnostics.Process.Start("explorer", s);
            }
            catch (System.ComponentModel.Win32Exception) { }
		}

	}
}
