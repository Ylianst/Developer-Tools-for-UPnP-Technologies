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

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for ComplexItem.
	/// </summary>
	public class ComplexItem : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.TextBox variableName;
		private System.Windows.Forms.PictureBox iconBox;
		private System.Windows.Forms.ImageList actionImageList;
		private System.Windows.Forms.Button upButton;
		private System.Windows.Forms.Button downButton;
		private System.ComponentModel.IContainer components;
		
		private int min = 1;
		private string max = "";

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.RadioButton maxOccur_UNBOUNDED;
		private System.Windows.Forms.RadioButton maxOccur_FIXED;
		private System.Windows.Forms.TextBox maxOccurTextBox;
		private System.Windows.Forms.ComboBox variableType;
		private Container parent;

		public OpenSource.UPnP.UPnPComplexType.Field GetFieldInfo()
		{
			OpenSource.UPnP.UPnPComplexType.Field f = new OpenSource.UPnP.UPnPComplexType.Field();

			f.Name = variableName.Text;
			f.Type = variableType.SelectedItem.ToString();
			f.MinOccurs = this.MinOccurs.ToString();
			f.MaxOccurs = this.MaxOccurs;

			if(variableType.SelectedIndex>9)
			{
				// Complex Type
				f.TypeNS = ((OpenSource.UPnP.UPnPComplexType)variableType.SelectedItem).Name_NAMESPACE;
			}
			else
			{
				// XSD Type
				f.TypeNS = "http://www.w3.org/2001/XMLSchema";
			}
			return(f);
		}

		public ComplexItem(Container p, OpenSource.UPnP.UPnPComplexType[] typeList)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call
			iconBox.Image = actionImageList.Images[0];
			parent = p;

			variableType.SelectedIndex = 0;

			foreach(OpenSource.UPnP.UPnPComplexType ct in typeList)
			{
				variableType.Items.Add(ct);
			}
		}

		public int MinOccurs
		{
			get
			{
				return(min);
			}
		}
		public string MaxOccurs
		{
			get
			{
				return(max);
			}
		}
		public string VariableName
		{
			get
			{
				return(variableName.Text);
			}
		}
//		public ArrayList DataStructure
//		{
//			get
//			{
//			}
//		}

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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ComplexItem));
			this.variableName = new System.Windows.Forms.TextBox();
			this.variableType = new System.Windows.Forms.ComboBox();
			this.iconBox = new System.Windows.Forms.PictureBox();
			this.actionImageList = new System.Windows.Forms.ImageList(this.components);
			this.upButton = new System.Windows.Forms.Button();
			this.downButton = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.maxOccurTextBox = new System.Windows.Forms.TextBox();
			this.maxOccur_FIXED = new System.Windows.Forms.RadioButton();
			this.maxOccur_UNBOUNDED = new System.Windows.Forms.RadioButton();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// variableName
			// 
			this.variableName.Location = new System.Drawing.Point(72, 8);
			this.variableName.Name = "variableName";
			this.variableName.Size = new System.Drawing.Size(320, 20);
			this.variableName.TabIndex = 0;
			this.variableName.Text = "";
			// 
			// variableType
			// 
			this.variableType.Items.AddRange(new object[] {
															  "string",
															  "boolean",
															  "decimal",
															  "float",
															  "double",
															  "duration",
															  "dateTime",
															  "time",
															  "date",
															  "anyURI"});
			this.variableType.Location = new System.Drawing.Point(72, 32);
			this.variableType.Name = "variableType";
			this.variableType.Size = new System.Drawing.Size(320, 21);
			this.variableType.TabIndex = 1;
			// 
			// iconBox
			// 
			this.iconBox.Location = new System.Drawing.Point(8, 8);
			this.iconBox.Name = "iconBox";
			this.iconBox.Size = new System.Drawing.Size(48, 24);
			this.iconBox.TabIndex = 2;
			this.iconBox.TabStop = false;
			this.iconBox.DoubleClick += new System.EventHandler(this.iconBox_DoubleClick);
			// 
			// actionImageList
			// 
			this.actionImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth24Bit;
			this.actionImageList.ImageSize = new System.Drawing.Size(41, 18);
			this.actionImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("actionImageList.ImageStream")));
			this.actionImageList.TransparentColor = System.Drawing.Color.White;
			// 
			// upButton
			// 
			this.upButton.Image = ((System.Drawing.Image)(resources.GetObject("upButton.Image")));
			this.upButton.Location = new System.Drawing.Point(4, 36);
			this.upButton.Name = "upButton";
			this.upButton.Size = new System.Drawing.Size(24, 22);
			this.upButton.TabIndex = 3;
			this.upButton.Text = "button1";
			this.upButton.Click += new System.EventHandler(this.upButton_Click);
			// 
			// downButton
			// 
			this.downButton.Image = ((System.Drawing.Image)(resources.GetObject("downButton.Image")));
			this.downButton.Location = new System.Drawing.Point(32, 36);
			this.downButton.Name = "downButton";
			this.downButton.Size = new System.Drawing.Size(24, 22);
			this.downButton.TabIndex = 4;
			this.downButton.Text = "button2";
			this.downButton.Click += new System.EventHandler(this.downButton_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.maxOccurTextBox);
			this.groupBox1.Controls.Add(this.maxOccur_FIXED);
			this.groupBox1.Controls.Add(this.maxOccur_UNBOUNDED);
			this.groupBox1.Location = new System.Drawing.Point(416, 1);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(104, 56);
			this.groupBox1.TabIndex = 5;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "MaxOccurs";
			// 
			// maxOccurTextBox
			// 
			this.maxOccurTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 6.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.maxOccurTextBox.Location = new System.Drawing.Point(24, 32);
			this.maxOccurTextBox.Name = "maxOccurTextBox";
			this.maxOccurTextBox.Size = new System.Drawing.Size(56, 18);
			this.maxOccurTextBox.TabIndex = 2;
			this.maxOccurTextBox.Text = "1";
			// 
			// maxOccur_FIXED
			// 
			this.maxOccur_FIXED.Checked = true;
			this.maxOccur_FIXED.Location = new System.Drawing.Point(8, 33);
			this.maxOccur_FIXED.Name = "maxOccur_FIXED";
			this.maxOccur_FIXED.Size = new System.Drawing.Size(16, 16);
			this.maxOccur_FIXED.TabIndex = 1;
			this.maxOccur_FIXED.TabStop = true;
			this.maxOccur_FIXED.Text = "radioButton2";
			this.maxOccur_FIXED.CheckedChanged += new System.EventHandler(this.maxOccur_FIXED_CheckedChanged);
			// 
			// maxOccur_UNBOUNDED
			// 
			this.maxOccur_UNBOUNDED.Location = new System.Drawing.Point(8, 16);
			this.maxOccur_UNBOUNDED.Name = "maxOccur_UNBOUNDED";
			this.maxOccur_UNBOUNDED.Size = new System.Drawing.Size(88, 16);
			this.maxOccur_UNBOUNDED.TabIndex = 0;
			this.maxOccur_UNBOUNDED.Text = "Unbounded";
			this.maxOccur_UNBOUNDED.CheckedChanged += new System.EventHandler(this.maxOccur_UNBOUNDED_CheckedChanged);
			// 
			// ComplexItem
			// 
			this.BackColor = System.Drawing.SystemColors.ControlLight;
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.downButton);
			this.Controls.Add(this.upButton);
			this.Controls.Add(this.iconBox);
			this.Controls.Add(this.variableType);
			this.Controls.Add(this.variableName);
			this.Name = "ComplexItem";
			this.Size = new System.Drawing.Size(528, 64);
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void ToggleIcon()
		{
			if(min==1)
			{
				min=0;
				iconBox.Image=actionImageList.Images[1];
			}
			else
			{
				min=1;
				iconBox.Image=actionImageList.Images[0];
			}
		}


		private void iconBox_DoubleClick(object sender, System.EventArgs e)
		{
			ToggleIcon();
		}

		private void upButton_Click(object sender, System.EventArgs e)
		{
			parent.moveUp(this);
		}

		private void downButton_Click(object sender, System.EventArgs e)
		{
			parent.moveDown(this);
		}

		private void maxOccur_UNBOUNDED_CheckedChanged(object sender, System.EventArgs e)
		{
			if(maxOccur_UNBOUNDED.Checked)
			{
				max = "unbounded";
			}
		}

		private void maxOccur_FIXED_CheckedChanged(object sender, System.EventArgs e)
		{
			if(maxOccur_FIXED.Checked)
			{
				max = maxOccurTextBox.Text;
			}
		}
	}
}
