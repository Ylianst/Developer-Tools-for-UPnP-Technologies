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

namespace ServiceAuthor
{
	/// <summary>
	/// Summary description for UPnpArgumentControl.
	/// </summary>
	public class UPnpArgumentEditControl : System.Windows.Forms.UserControl
	{
		private UPnPArgument arg = null;
		private UPnPAction parentaction;
		private ActionEditForm parentform;
		private System.Windows.Forms.ImageList actionImageList;
		private System.Windows.Forms.Panel panel1;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.PictureBox argDirPictureBox;
		private System.Windows.Forms.ToolTip toolTip;
		private System.Windows.Forms.TextBox argNameTextBox;
		private System.Windows.Forms.ComboBox stateVariablesComboBox;
		private System.Windows.Forms.MenuItem inputMenuItem;
		private System.Windows.Forms.MenuItem outputMenuItem;
		private System.Windows.Forms.MenuItem returnMenuItem;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem removeArgumentMenuItem;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button upButton;
		private System.Windows.Forms.Button downButton;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem sendToTopMenuItem;
		private System.Windows.Forms.MenuItem bringToBottomMenuItem;
		private System.Windows.Forms.ContextMenu ioContextMenu;

		public UPnPArgument UPnPArgument
		{
			get
			{
				arg.ParentAction = parentaction;
				arg.RelatedStateVar = (UPnPStateVariable)stateVariablesComboBox.SelectedItem;
				arg.Name = argNameTextBox.Text;
				arg.Direction = "out";
				arg.IsReturnValue = (returnMenuItem.Checked == true);
				if (inputMenuItem.Checked == true)
				{
					arg.Direction = "in";
				} 
				return arg;
			}
		}

		public UPnpArgumentEditControl(ActionEditForm parentform, UPnPAction action, UPnPArgument argument)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			this.parentform = parentform;
			this.parentaction = action;
			this.arg = argument;

			// Set name
			argNameTextBox.Text = arg.Name;
			//toolTip.SetToolTip(actionNameLabel,"(" + arg.RelatedStateVar.ValueType + ") " + arg.Name);

			// Set input/output
			inputMenuItem.Checked = false;
			outputMenuItem.Checked = false;
			returnMenuItem.Checked = false;

			if (arg.IsReturnValue == true) 
			{
				returnMenuItem.Checked = true;
				argDirPictureBox.Image = actionImageList.Images[2];
				toolTip.SetToolTip(argDirPictureBox,"Return argument");
			} 
			else 
			{
				if (arg.Direction == "in") 
				{
					inputMenuItem.Checked = true;
					argDirPictureBox.Image = actionImageList.Images[0];
					toolTip.SetToolTip(argDirPictureBox,"Input argument");
				} 
				else 
				if (arg.Direction == "out")
				{
					outputMenuItem.Checked = true;
					argDirPictureBox.Image = actionImageList.Images[1];
					toolTip.SetToolTip(argDirPictureBox,"Ouput argument");
				} 
				else 
				{
					returnMenuItem.Checked = true;
					argDirPictureBox.Image = actionImageList.Images[2];
					toolTip.SetToolTip(argDirPictureBox,"Return argument");
				}
			}

			// Fill state variables
			UPnPStateVariable[] vars = action.ParentService.GetStateVariables();
			bool selitemset = false;
			foreach (UPnPStateVariable var in vars) 
			{
				stateVariablesComboBox.Items.Add(var);
				if (arg.RelatedStateVar != null && var.Name == arg.RelatedStateVar.Name) 
				{
					stateVariablesComboBox.SelectedItem = var;
					selitemset = true;
				}
			}
			if (selitemset == false)
			{
				stateVariablesComboBox.SelectedIndex = 0;
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(UPnpArgumentEditControl));
			this.actionImageList = new System.Windows.Forms.ImageList(this.components);
			this.panel1 = new System.Windows.Forms.Panel();
			this.downButton = new System.Windows.Forms.Button();
			this.ioContextMenu = new System.Windows.Forms.ContextMenu();
			this.inputMenuItem = new System.Windows.Forms.MenuItem();
			this.outputMenuItem = new System.Windows.Forms.MenuItem();
			this.returnMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.removeArgumentMenuItem = new System.Windows.Forms.MenuItem();
			this.upButton = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.stateVariablesComboBox = new System.Windows.Forms.ComboBox();
			this.argNameTextBox = new System.Windows.Forms.TextBox();
			this.argDirPictureBox = new System.Windows.Forms.PictureBox();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.sendToTopMenuItem = new System.Windows.Forms.MenuItem();
			this.bringToBottomMenuItem = new System.Windows.Forms.MenuItem();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// actionImageList
			// 
			this.actionImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth24Bit;
			this.actionImageList.ImageSize = new System.Drawing.Size(41, 18);
			this.actionImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("actionImageList.ImageStream")));
			this.actionImageList.TransparentColor = System.Drawing.Color.White;
			// 
			// panel1
			// 
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.downButton,
																				 this.upButton,
																				 this.label2,
																				 this.label1,
																				 this.stateVariablesComboBox,
																				 this.argNameTextBox,
																				 this.argDirPictureBox});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(344, 56);
			this.panel1.TabIndex = 3;
			// 
			// downButton
			// 
			this.downButton.ContextMenu = this.ioContextMenu;
			this.downButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("downButton.Image")));
			this.downButton.Location = new System.Drawing.Point(30, 29);
			this.downButton.Name = "downButton";
			this.downButton.Size = new System.Drawing.Size(20, 19);
			this.downButton.TabIndex = 10;
			this.downButton.Click += new System.EventHandler(this.downButton_Click);
			// 
			// ioContextMenu
			// 
			this.ioContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						  this.inputMenuItem,
																						  this.outputMenuItem,
																						  this.returnMenuItem,
																						  this.menuItem1,
																						  this.sendToTopMenuItem,
																						  this.bringToBottomMenuItem,
																						  this.menuItem2,
																						  this.removeArgumentMenuItem});
			// 
			// inputMenuItem
			// 
			this.inputMenuItem.Index = 0;
			this.inputMenuItem.Text = "Input";
			this.inputMenuItem.Click += new System.EventHandler(this.inputMenuItem_Click);
			// 
			// outputMenuItem
			// 
			this.outputMenuItem.Index = 1;
			this.outputMenuItem.Text = "Output";
			this.outputMenuItem.Click += new System.EventHandler(this.outputMenuItem_Click);
			// 
			// returnMenuItem
			// 
			this.returnMenuItem.Index = 2;
			this.returnMenuItem.Text = "Return";
			this.returnMenuItem.Click += new System.EventHandler(this.returnMenuItem_Click);
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 3;
			this.menuItem1.Text = "-";
			// 
			// removeArgumentMenuItem
			// 
			this.removeArgumentMenuItem.Index = 7;
			this.removeArgumentMenuItem.Text = "Remove Argument";
			this.removeArgumentMenuItem.Click += new System.EventHandler(this.removeArgumentMenuItem_Click);
			// 
			// upButton
			// 
			this.upButton.ContextMenu = this.ioContextMenu;
			this.upButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("upButton.Image")));
			this.upButton.Location = new System.Drawing.Point(9, 29);
			this.upButton.Name = "upButton";
			this.upButton.Size = new System.Drawing.Size(20, 19);
			this.upButton.TabIndex = 9;
			this.upButton.Click += new System.EventHandler(this.upButton_Click);
			// 
			// label2
			// 
			this.label2.ContextMenu = this.ioContextMenu;
			this.label2.Location = new System.Drawing.Point(56, 32);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(48, 16);
			this.label2.TabIndex = 8;
			this.label2.Text = "Variable";
			// 
			// label1
			// 
			this.label1.ContextMenu = this.ioContextMenu;
			this.label1.Location = new System.Drawing.Point(56, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(40, 16);
			this.label1.TabIndex = 7;
			this.label1.Text = "Name";
			// 
			// stateVariablesComboBox
			// 
			this.stateVariablesComboBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.stateVariablesComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.stateVariablesComboBox.Location = new System.Drawing.Point(104, 27);
			this.stateVariablesComboBox.Name = "stateVariablesComboBox";
			this.stateVariablesComboBox.Size = new System.Drawing.Size(232, 21);
			this.stateVariablesComboBox.TabIndex = 6;
			this.toolTip.SetToolTip(this.stateVariablesComboBox, "State variable");
			// 
			// argNameTextBox
			// 
			this.argNameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.argNameTextBox.Location = new System.Drawing.Point(104, 4);
			this.argNameTextBox.Multiline = true;
			this.argNameTextBox.Name = "argNameTextBox";
			this.argNameTextBox.Size = new System.Drawing.Size(232, 20);
			this.argNameTextBox.TabIndex = 5;
			this.argNameTextBox.Text = "";
			this.toolTip.SetToolTip(this.argNameTextBox, "Argument Name");
			// 
			// argDirPictureBox
			// 
			this.argDirPictureBox.ContextMenu = this.ioContextMenu;
			this.argDirPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("argDirPictureBox.Image")));
			this.argDirPictureBox.Location = new System.Drawing.Point(4, 4);
			this.argDirPictureBox.Name = "argDirPictureBox";
			this.argDirPictureBox.Size = new System.Drawing.Size(48, 24);
			this.argDirPictureBox.TabIndex = 3;
			this.argDirPictureBox.TabStop = false;
			this.toolTip.SetToolTip(this.argDirPictureBox, "Argument I/O type");
			this.argDirPictureBox.DoubleClick += new System.EventHandler(this.argDirPictureBox_DoubleClick);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 6;
			this.menuItem2.Text = "-";
			// 
			// sendToTopMenuItem
			// 
			this.sendToTopMenuItem.Index = 4;
			this.sendToTopMenuItem.Text = "Send To Top";
			this.sendToTopMenuItem.Click += new System.EventHandler(this.sendToTopMenuItem_Click);
			// 
			// bringToBottomMenuItem
			// 
			this.bringToBottomMenuItem.Index = 5;
			this.bringToBottomMenuItem.Text = "Bring To Bottom";
			this.bringToBottomMenuItem.Click += new System.EventHandler(this.bringToBottomMenuItem_Click);
			// 
			// UPnpArgumentEditControl
			// 
			this.BackColor = System.Drawing.SystemColors.ControlLight;
			this.ContextMenu = this.ioContextMenu;
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1});
			this.Name = "UPnpArgumentEditControl";
			this.Size = new System.Drawing.Size(344, 56);
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void inputMenuItem_Click(object sender, System.EventArgs e)
		{
			inputMenuItem.Checked = true;
			outputMenuItem.Checked = false;
			returnMenuItem.Checked = false;
			argDirPictureBox.Image = actionImageList.Images[0];
			toolTip.SetToolTip(argDirPictureBox,"Input argument");
		}

		private void outputMenuItem_Click(object sender, System.EventArgs e)
		{
			inputMenuItem.Checked = false;
			outputMenuItem.Checked = true;
			returnMenuItem.Checked = false;
			argDirPictureBox.Image = actionImageList.Images[1];
			toolTip.SetToolTip(argDirPictureBox,"Output argument");
		}

		private void returnMenuItem_Click(object sender, System.EventArgs e)
		{
			inputMenuItem.Checked = false;
			outputMenuItem.Checked = false;
			returnMenuItem.Checked = true;
			argDirPictureBox.Image = actionImageList.Images[2];
			toolTip.SetToolTip(argDirPictureBox,"Return argument");
		}

		private void argDirPictureBox_DoubleClick(object sender, System.EventArgs e)
		{
			if (inputMenuItem.Checked == true) {outputMenuItem_Click(this,null);return;}
			if (outputMenuItem.Checked == true) {returnMenuItem_Click(this,null);return;}
			if (returnMenuItem.Checked == true) {inputMenuItem_Click(this,null);return;}
		}

		private void upButton_Click(object sender, System.EventArgs e)
		{
			parentform.moveArgDown(this);
		}

		private void downButton_Click(object sender, System.EventArgs e)
		{
			parentform.moveArgUp(this);
		}

		private void removeArgumentMenuItem_Click(object sender, System.EventArgs e)
		{
			parentform.RemoveArg(this);
		}

		private void sendToTopMenuItem_Click(object sender, System.EventArgs e)
		{
			parentform.moveArgBottom(this);		
		}

		private void bringToBottomMenuItem_Click(object sender, System.EventArgs e)
		{
			parentform.moveArgTop(this);		
		}

	}
}
