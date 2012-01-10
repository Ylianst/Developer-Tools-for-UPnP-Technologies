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

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for DesignComplexType.
	/// </summary>
	public class DesignComplexType : System.Windows.Forms.Form
	{
		private OpenSource.UPnP.UPnPComplexType[] complexTypeList;
		public OpenSource.UPnP.UPnPComplexType ComplexType = null;

		public string FieldName;
		public string FieldType;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Button addFieldButton;
		private System.Windows.Forms.Panel itemPanel;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TextBox customNamespaceTextBox;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.TextBox complexTypeNameTextBox;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public DesignComplexType(OpenSource.UPnP.UPnPComplexType[] complexTypes)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			complexTypeList = complexTypes;
				
			//
			// TODO: Add any constructor code after InitializeComponent call
			//
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
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.addFieldButton = new System.Windows.Forms.Button();
            this.itemPanel = new System.Windows.Forms.Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.customNamespaceTextBox = new System.Windows.Forms.TextBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.complexTypeNameTextBox = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(8, 392);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(88, 23);
            this.okButton.TabIndex = 2;
            this.okButton.Text = "OK";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Location = new System.Drawing.Point(8, 360);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(88, 23);
            this.cancelButton.TabIndex = 3;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // addFieldButton
            // 
            this.addFieldButton.Location = new System.Drawing.Point(8, 328);
            this.addFieldButton.Name = "addFieldButton";
            this.addFieldButton.Size = new System.Drawing.Size(88, 23);
            this.addFieldButton.TabIndex = 5;
            this.addFieldButton.Text = "Add Grouping";
            this.addFieldButton.Click += new System.EventHandler(this.addFieldButton_Click);
            // 
            // itemPanel
            // 
            this.itemPanel.AutoScroll = true;
            this.itemPanel.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.itemPanel.Location = new System.Drawing.Point(8, 8);
            this.itemPanel.Name = "itemPanel";
            this.itemPanel.Size = new System.Drawing.Size(688, 304);
            this.itemPanel.TabIndex = 6;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.customNamespaceTextBox);
            this.groupBox1.Location = new System.Drawing.Point(104, 320);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(592, 48);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Namespace";
            // 
            // customNamespaceTextBox
            // 
            this.customNamespaceTextBox.Location = new System.Drawing.Point(8, 16);
            this.customNamespaceTextBox.Name = "customNamespaceTextBox";
            this.customNamespaceTextBox.Size = new System.Drawing.Size(576, 20);
            this.customNamespaceTextBox.TabIndex = 0;
            this.customNamespaceTextBox.Text = "http://www.vendor.org/Schemas";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.complexTypeNameTextBox);
            this.groupBox2.Location = new System.Drawing.Point(104, 369);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(592, 48);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Complex Type Name";
            // 
            // complexTypeNameTextBox
            // 
            this.complexTypeNameTextBox.Location = new System.Drawing.Point(8, 16);
            this.complexTypeNameTextBox.Name = "complexTypeNameTextBox";
            this.complexTypeNameTextBox.Size = new System.Drawing.Size(576, 20);
            this.complexTypeNameTextBox.TabIndex = 0;
            // 
            // DesignComplexType
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(704, 430);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.itemPanel);
            this.Controls.Add(this.addFieldButton);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "DesignComplexType";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Design Complex Type";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

		}
		#endregion

		private void fieldNameTextBox_TextChanged(object sender, System.EventArgs e)
		{

		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;
		}

		private void okButton_Click(object sender, System.EventArgs e)
		{
			if (itemPanel.Controls.Count > 0)
			{
				ComplexType = new OpenSource.UPnP.UPnPComplexType(complexTypeNameTextBox.Text, customNamespaceTextBox.Text);

                // TODO: The following lines make no sense, this feature does not work.
				foreach(Container C in itemPanel.Controls)
				{
					//ComplexType.AddField(C.GetFieldInfo());
                    OpenSource.UPnP.UPnPComplexType.GenericContainer gc = new OpenSource.UPnP.UPnPComplexType.GenericContainer();
                    ComplexType.AddContainer(gc);
				}
			}
			this.DialogResult = DialogResult.OK;
		}

		private void addFieldButton_Click(object sender, System.EventArgs e)
		{
//			ComplexItem ci = new ComplexItem(this,complexTypeList);
//
//			ci.Dock = DockStyle.Top;
//			itemPanel.Controls.Add(ci);
			Container c = new Container(this, complexTypeList);
			c.Dock = DockStyle.Top;
			itemPanel.Controls.Add(c);
		}

		public void moveUp(Control obj)
		{
			int pos = itemPanel.Controls.GetChildIndex(obj,false);
			if(pos >= 0)
			{
				itemPanel.Controls.SetChildIndex(obj, pos + 1);
			}
		}
		public void moveDown(Control obj)
		{
			int pos = itemPanel.Controls.GetChildIndex(obj,false);
			if(pos > 0)
			{
				itemPanel.Controls.SetChildIndex(obj, pos - 1);
			}
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
			TestForm t = new TestForm();
			t.ShowDialog();
		}
	}
}
