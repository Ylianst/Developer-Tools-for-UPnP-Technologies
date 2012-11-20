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
	/// Summary description for Container.
	/// </summary>
	public class Container : System.Windows.Forms.UserControl
	{
		private DesignComplexType parent;
		private UPnPComplexType[] ComplexTypeList;

		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.Panel itemPanel;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.RadioButton radioButton4;
		private System.Windows.Forms.RadioButton SimpleContent;
		private System.Windows.Forms.RadioButton ComplexContent;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.RadioButton None;
		private System.Windows.Forms.RadioButton Choice;
		private System.Windows.Forms.RadioButton Sequence;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.TextBox MinOccurs;
		private System.Windows.Forms.RadioButton MaxOccursRadio1;
		private System.Windows.Forms.RadioButton MaxOccursRadio2;
		private System.Windows.Forms.TextBox MaxOccursTextBox;
		private System.Windows.Forms.Button AddGroupingButton;
		private System.Windows.Forms.Button AddFieldButton;
		private System.Windows.Forms.Button DownButton_OutterPanel;
		private System.Windows.Forms.Button UpButton_OutterPanel;
		private System.Windows.Forms.Button DownButton_InnerPanel;
		private System.Windows.Forms.Button UpButton_InnerPanel;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Container(DesignComplexType p, OpenSource.UPnP.UPnPComplexType[] typeList)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
			ComplexTypeList = typeList;
			parent = p;
			// TODO: Add any initialization after the InitializeComponent call
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Container));
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.AddGroupingButton = new System.Windows.Forms.Button();
			this.AddFieldButton = new System.Windows.Forms.Button();
			this.itemPanel = new System.Windows.Forms.Panel();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.radioButton4 = new System.Windows.Forms.RadioButton();
			this.SimpleContent = new System.Windows.Forms.RadioButton();
			this.ComplexContent = new System.Windows.Forms.RadioButton();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.None = new System.Windows.Forms.RadioButton();
			this.Choice = new System.Windows.Forms.RadioButton();
			this.Sequence = new System.Windows.Forms.RadioButton();
			this.DownButton_OutterPanel = new System.Windows.Forms.Button();
			this.UpButton_OutterPanel = new System.Windows.Forms.Button();
			this.DownButton_InnerPanel = new System.Windows.Forms.Button();
			this.UpButton_InnerPanel = new System.Windows.Forms.Button();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.MinOccurs = new System.Windows.Forms.TextBox();
			this.MaxOccursRadio1 = new System.Windows.Forms.RadioButton();
			this.MaxOccursRadio2 = new System.Windows.Forms.RadioButton();
			this.MaxOccursTextBox = new System.Windows.Forms.TextBox();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox4.SuspendLayout();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Location = new System.Drawing.Point(24, 0);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(640, 264);
			this.tabControl1.TabIndex = 0;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.DownButton_InnerPanel);
			this.tabPage1.Controls.Add(this.UpButton_InnerPanel);
			this.tabPage1.Controls.Add(this.AddGroupingButton);
			this.tabPage1.Controls.Add(this.AddFieldButton);
			this.tabPage1.Controls.Add(this.itemPanel);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Size = new System.Drawing.Size(632, 238);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "Content";
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.Add(this.groupBox2);
			this.tabPage2.Controls.Add(this.groupBox1);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Size = new System.Drawing.Size(632, 238);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Configuration";
			// 
			// AddGroupingButton
			// 
			this.AddGroupingButton.Location = new System.Drawing.Point(8, 40);
			this.AddGroupingButton.Name = "AddGroupingButton";
			this.AddGroupingButton.Size = new System.Drawing.Size(88, 23);
			this.AddGroupingButton.TabIndex = 20;
			this.AddGroupingButton.Text = "Add Grouping";
			// 
			// AddFieldButton
			// 
			this.AddFieldButton.Location = new System.Drawing.Point(8, 16);
			this.AddFieldButton.Name = "AddFieldButton";
			this.AddFieldButton.Size = new System.Drawing.Size(88, 23);
			this.AddFieldButton.TabIndex = 19;
			this.AddFieldButton.Text = "Add Field";
			this.AddFieldButton.Click += new System.EventHandler(this.AddFieldButton_Click);
			// 
			// itemPanel
			// 
			this.itemPanel.AutoScroll = true;
			this.itemPanel.BackColor = System.Drawing.SystemColors.ControlLight;
			this.itemPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.itemPanel.Location = new System.Drawing.Point(104, 0);
			this.itemPanel.Name = "itemPanel";
			this.itemPanel.Size = new System.Drawing.Size(520, 232);
			this.itemPanel.TabIndex = 16;
			// 
			// groupBox2
			// 
			this.groupBox2.BackColor = System.Drawing.SystemColors.ControlLight;
			this.groupBox2.Controls.Add(this.radioButton4);
			this.groupBox2.Controls.Add(this.SimpleContent);
			this.groupBox2.Controls.Add(this.ComplexContent);
			this.groupBox2.Location = new System.Drawing.Point(16, 112);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(424, 72);
			this.groupBox2.TabIndex = 22;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Content";
			// 
			// radioButton4
			// 
			this.radioButton4.BackColor = System.Drawing.SystemColors.ControlLight;
			this.radioButton4.Checked = true;
			this.radioButton4.Location = new System.Drawing.Point(8, 48);
			this.radioButton4.Name = "radioButton4";
			this.radioButton4.Size = new System.Drawing.Size(80, 16);
			this.radioButton4.TabIndex = 2;
			this.radioButton4.TabStop = true;
			this.radioButton4.Text = "Unspec";
			// 
			// SimpleContent
			// 
			this.SimpleContent.BackColor = System.Drawing.SystemColors.ControlLight;
			this.SimpleContent.Location = new System.Drawing.Point(8, 32);
			this.SimpleContent.Name = "SimpleContent";
			this.SimpleContent.Size = new System.Drawing.Size(80, 16);
			this.SimpleContent.TabIndex = 1;
			this.SimpleContent.Text = "Simple";
			// 
			// ComplexContent
			// 
			this.ComplexContent.BackColor = System.Drawing.SystemColors.ControlLight;
			this.ComplexContent.Location = new System.Drawing.Point(8, 16);
			this.ComplexContent.Name = "ComplexContent";
			this.ComplexContent.Size = new System.Drawing.Size(80, 16);
			this.ComplexContent.TabIndex = 0;
			this.ComplexContent.Text = "Complex";
			// 
			// groupBox1
			// 
			this.groupBox1.BackColor = System.Drawing.SystemColors.ControlLight;
			this.groupBox1.Controls.Add(this.groupBox4);
			this.groupBox1.Controls.Add(this.groupBox3);
			this.groupBox1.Controls.Add(this.None);
			this.groupBox1.Controls.Add(this.Choice);
			this.groupBox1.Controls.Add(this.Sequence);
			this.groupBox1.Location = new System.Drawing.Point(16, 24);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(424, 80);
			this.groupBox1.TabIndex = 21;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Type";
			// 
			// None
			// 
			this.None.BackColor = System.Drawing.SystemColors.ControlLight;
			this.None.Checked = true;
			this.None.Location = new System.Drawing.Point(8, 53);
			this.None.Name = "None";
			this.None.Size = new System.Drawing.Size(80, 24);
			this.None.TabIndex = 2;
			this.None.TabStop = true;
			this.None.Text = "None";
			// 
			// Choice
			// 
			this.Choice.BackColor = System.Drawing.SystemColors.ControlLight;
			this.Choice.Location = new System.Drawing.Point(8, 35);
			this.Choice.Name = "Choice";
			this.Choice.Size = new System.Drawing.Size(80, 24);
			this.Choice.TabIndex = 1;
			this.Choice.Text = "Choice";
			// 
			// Sequence
			// 
			this.Sequence.BackColor = System.Drawing.SystemColors.ControlLight;
			this.Sequence.Location = new System.Drawing.Point(8, 16);
			this.Sequence.Name = "Sequence";
			this.Sequence.Size = new System.Drawing.Size(80, 24);
			this.Sequence.TabIndex = 0;
			this.Sequence.Text = "Sequence";
			// 
			// DownButton_OutterPanel
			// 
			this.DownButton_OutterPanel.Image = ((System.Drawing.Image)(resources.GetObject("DownButton_OutterPanel.Image")));
			this.DownButton_OutterPanel.Location = new System.Drawing.Point(0, 240);
			this.DownButton_OutterPanel.Name = "DownButton_OutterPanel";
			this.DownButton_OutterPanel.Size = new System.Drawing.Size(24, 22);
			this.DownButton_OutterPanel.TabIndex = 18;
			this.DownButton_OutterPanel.Text = "button2";
			// 
			// UpButton_OutterPanel
			// 
			this.UpButton_OutterPanel.Image = ((System.Drawing.Image)(resources.GetObject("UpButton_OutterPanel.Image")));
			this.UpButton_OutterPanel.Location = new System.Drawing.Point(0, 24);
			this.UpButton_OutterPanel.Name = "UpButton_OutterPanel";
			this.UpButton_OutterPanel.Size = new System.Drawing.Size(24, 22);
			this.UpButton_OutterPanel.TabIndex = 17;
			this.UpButton_OutterPanel.Text = "button1";
			// 
			// DownButton_InnerPanel
			// 
			this.DownButton_InnerPanel.Image = ((System.Drawing.Image)(resources.GetObject("DownButton_InnerPanel.Image")));
			this.DownButton_InnerPanel.Location = new System.Drawing.Point(40, 136);
			this.DownButton_InnerPanel.Name = "DownButton_InnerPanel";
			this.DownButton_InnerPanel.Size = new System.Drawing.Size(24, 22);
			this.DownButton_InnerPanel.TabIndex = 22;
			this.DownButton_InnerPanel.Text = "button2";
			// 
			// UpButton_InnerPanel
			// 
			this.UpButton_InnerPanel.Image = ((System.Drawing.Image)(resources.GetObject("UpButton_InnerPanel.Image")));
			this.UpButton_InnerPanel.Location = new System.Drawing.Point(40, 112);
			this.UpButton_InnerPanel.Name = "UpButton_InnerPanel";
			this.UpButton_InnerPanel.Size = new System.Drawing.Size(24, 22);
			this.UpButton_InnerPanel.TabIndex = 21;
			this.UpButton_InnerPanel.Text = "button1";
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.MinOccurs);
			this.groupBox3.Location = new System.Drawing.Point(112, 16);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(80, 56);
			this.groupBox3.TabIndex = 3;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "MinOccurs";
			// 
			// groupBox4
			// 
			this.groupBox4.Controls.Add(this.MaxOccursTextBox);
			this.groupBox4.Controls.Add(this.MaxOccursRadio2);
			this.groupBox4.Controls.Add(this.MaxOccursRadio1);
			this.groupBox4.Location = new System.Drawing.Point(200, 16);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(208, 56);
			this.groupBox4.TabIndex = 4;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "MaxOccurs";
			// 
			// MinOccurs
			// 
			this.MinOccurs.Location = new System.Drawing.Point(24, 24);
			this.MinOccurs.Name = "MinOccurs";
			this.MinOccurs.Size = new System.Drawing.Size(32, 20);
			this.MinOccurs.TabIndex = 0;
			this.MinOccurs.Text = "1";
			// 
			// MaxOccursRadio1
			// 
			this.MaxOccursRadio1.Checked = true;
			this.MaxOccursRadio1.Location = new System.Drawing.Point(8, 24);
			this.MaxOccursRadio1.Name = "MaxOccursRadio1";
			this.MaxOccursRadio1.Size = new System.Drawing.Size(16, 24);
			this.MaxOccursRadio1.TabIndex = 0;
			this.MaxOccursRadio1.TabStop = true;
			// 
			// MaxOccursRadio2
			// 
			this.MaxOccursRadio2.Location = new System.Drawing.Point(104, 24);
			this.MaxOccursRadio2.Name = "MaxOccursRadio2";
			this.MaxOccursRadio2.Size = new System.Drawing.Size(96, 24);
			this.MaxOccursRadio2.TabIndex = 1;
			this.MaxOccursRadio2.Text = "UNBOUNDED";
			// 
			// MaxOccursTextBox
			// 
			this.MaxOccursTextBox.Location = new System.Drawing.Point(32, 24);
			this.MaxOccursTextBox.Name = "MaxOccursTextBox";
			this.MaxOccursTextBox.Size = new System.Drawing.Size(32, 20);
			this.MaxOccursTextBox.TabIndex = 2;
			this.MaxOccursTextBox.Text = "1";
			// 
			// Container
			// 
			this.BackColor = System.Drawing.SystemColors.ControlLight;
			this.Controls.Add(this.DownButton_OutterPanel);
			this.Controls.Add(this.UpButton_OutterPanel);
			this.Controls.Add(this.tabControl1);
			this.Name = "Container";
			this.Size = new System.Drawing.Size(672, 272);
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.groupBox1.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.groupBox4.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void AddFieldButton_Click(object sender, System.EventArgs e)
		{
			ComplexItem ci = new ComplexItem(this, ComplexTypeList);
			ci.Dock = DockStyle.Top;
			itemPanel.Controls.Add(ci);
		}
		public void moveUp(Control obj)
		{
			int pos = itemPanel.Controls.GetChildIndex(obj,false);
			if(pos>=0)
			{
				itemPanel.Controls.SetChildIndex(obj,pos+1);
			}
		}
		public void moveDown(Control obj)
		{
			int pos = itemPanel.Controls.GetChildIndex(obj,false);
			if(pos>0)
			{
				itemPanel.Controls.SetChildIndex(obj,pos-1);
			}
		}
	}
}
