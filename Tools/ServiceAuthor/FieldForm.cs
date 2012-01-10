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

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for FieldForm.
	/// </summary>
	public class FieldForm : System.Windows.Forms.Form
	{
		public UPnPComplexType.ContentData NewContentItem;

		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.TextBox NameTextBox;
		private System.Windows.Forms.TextBox textBox3;
		private System.Windows.Forms.ComboBox TypeComboBox;
		private System.Windows.Forms.Button OKButton;
		private UPnPComplexType[] ComplexTypeList;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.TextBox MinOccursTextBox;
		private System.Windows.Forms.TextBox MaxOccursTextBox;
		private System.Windows.Forms.RadioButton MaxOccursRadio1;
		private System.Windows.Forms.RadioButton MaxOccursRadio2;
		private System.Windows.Forms.Button CancelButton;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.RadioButton ElementRadioButton;
		private System.Windows.Forms.RadioButton AttributeRadioButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public FieldForm(UPnPComplexType[] ComplexTypes, UPnPComplexType.ContentData cd)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			ComplexTypeList = ComplexTypes;
		
			foreach(UPnPComplexType ct in ComplexTypes)
			{
				TypeComboBox.Items.Add(ct);
			}

			if(cd==null)
			{
				TypeComboBox.SelectedIndex=11;
			}
			else
			{
				if(cd.GetType() == typeof(UPnPComplexType.Element))
				{
					ElementRadioButton.Checked = true;
				}
				else
				{
					AttributeRadioButton.Checked = true;
				}
				this.NameTextBox.Text = cd.Name;
				this.MinOccursTextBox.Text = cd.MinOccurs;
				if(cd.MaxOccurs == "UNBOUNDED")
				{
					this.MaxOccursRadio2.Checked = true;
				}
				else
				{
					this.MaxOccursRadio2.Checked = false;
					this.MaxOccursTextBox.Text = cd.MaxOccurs;
				}
				bool ok=false;
				foreach(UPnPComplexType ct in ComplexTypes)
				{
					if(ct.Name_LOCAL == cd.Type && ct.Name_NAMESPACE==cd.TypeNS)
					{
						ok=true;
						TypeComboBox.SelectedItem = ct;
						break;
					}
				}
				if(!ok)
				{
					TypeComboBox.SelectedItem = cd.Type;
				}

			}


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
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.NameTextBox = new System.Windows.Forms.TextBox();
			this.textBox3 = new System.Windows.Forms.TextBox();
			this.TypeComboBox = new System.Windows.Forms.ComboBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.MinOccursTextBox = new System.Windows.Forms.TextBox();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.MaxOccursRadio2 = new System.Windows.Forms.RadioButton();
			this.MaxOccursRadio1 = new System.Windows.Forms.RadioButton();
			this.MaxOccursTextBox = new System.Windows.Forms.TextBox();
			this.CancelButton = new System.Windows.Forms.Button();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.AttributeRadioButton = new System.Windows.Forms.RadioButton();
			this.ElementRadioButton = new System.Windows.Forms.RadioButton();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.SuspendLayout();
			// 
			// textBox1
			// 
			this.textBox1.Location = new System.Drawing.Point(16, 16);
			this.textBox1.Name = "textBox1";
			this.textBox1.ReadOnly = true;
			this.textBox1.Size = new System.Drawing.Size(72, 20);
			this.textBox1.TabIndex = 0;
			this.textBox1.TabStop = false;
			this.textBox1.Text = "Name";
			// 
			// NameTextBox
			// 
			this.NameTextBox.Location = new System.Drawing.Point(88, 16);
			this.NameTextBox.Name = "NameTextBox";
			this.NameTextBox.Size = new System.Drawing.Size(312, 20);
			this.NameTextBox.TabIndex = 1;
			this.NameTextBox.Text = "";
			this.NameTextBox.TextChanged += new System.EventHandler(this.OnTextChanged);
			// 
			// textBox3
			// 
			this.textBox3.Location = new System.Drawing.Point(16, 40);
			this.textBox3.Name = "textBox3";
			this.textBox3.ReadOnly = true;
			this.textBox3.Size = new System.Drawing.Size(72, 20);
			this.textBox3.TabIndex = 2;
			this.textBox3.TabStop = false;
			this.textBox3.Text = "Type";
			// 
			// TypeComboBox
			// 
			this.TypeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.TypeComboBox.Items.AddRange(new object[] {
															  "UI1",
															  "UI2",
															  "UI4",
															  "I1",
															  "I2",
															  "I4",
															  "int",
															  "R4",
															  "R8",
															  "float",
															  "char",
															  "string",
															  "date",
															  "dateTime",
															  "dateTime.tz",
															  "time",
															  "time.tz",
															  "boolean",
															  "bin.base64",
															  "bin.hex",
															  "uri",
															  "uuid"});
			this.TypeComboBox.Location = new System.Drawing.Point(88, 40);
			this.TypeComboBox.Name = "TypeComboBox";
			this.TypeComboBox.Size = new System.Drawing.Size(312, 21);
			this.TypeComboBox.TabIndex = 3;
			// 
			// OKButton
			// 
			this.OKButton.Enabled = false;
			this.OKButton.Location = new System.Drawing.Point(328, 152);
			this.OKButton.Name = "OKButton";
			this.OKButton.TabIndex = 4;
			this.OKButton.Text = "OK";
			this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.MinOccursTextBox);
			this.groupBox1.Location = new System.Drawing.Point(8, 80);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(88, 64);
			this.groupBox1.TabIndex = 5;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "MinOccurs";
			// 
			// MinOccursTextBox
			// 
			this.MinOccursTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.MinOccursTextBox.Location = new System.Drawing.Point(16, 24);
			this.MinOccursTextBox.MaxLength = 5;
			this.MinOccursTextBox.Name = "MinOccursTextBox";
			this.MinOccursTextBox.Size = new System.Drawing.Size(48, 22);
			this.MinOccursTextBox.TabIndex = 0;
			this.MinOccursTextBox.Text = "1";
			this.MinOccursTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.MaxOccursRadio2);
			this.groupBox2.Controls.Add(this.MaxOccursRadio1);
			this.groupBox2.Controls.Add(this.MaxOccursTextBox);
			this.groupBox2.Location = new System.Drawing.Point(104, 80);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(208, 64);
			this.groupBox2.TabIndex = 6;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "MaxOccurs";
			// 
			// MaxOccursRadio2
			// 
			this.MaxOccursRadio2.Location = new System.Drawing.Point(80, 24);
			this.MaxOccursRadio2.Name = "MaxOccursRadio2";
			this.MaxOccursRadio2.Size = new System.Drawing.Size(96, 24);
			this.MaxOccursRadio2.TabIndex = 3;
			this.MaxOccursRadio2.Text = "UNBOUNDED";
			// 
			// MaxOccursRadio1
			// 
			this.MaxOccursRadio1.Checked = true;
			this.MaxOccursRadio1.Location = new System.Drawing.Point(8, 24);
			this.MaxOccursRadio1.Name = "MaxOccursRadio1";
			this.MaxOccursRadio1.Size = new System.Drawing.Size(16, 24);
			this.MaxOccursRadio1.TabIndex = 2;
			this.MaxOccursRadio1.TabStop = true;
			// 
			// MaxOccursTextBox
			// 
			this.MaxOccursTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.MaxOccursTextBox.Location = new System.Drawing.Point(24, 24);
			this.MaxOccursTextBox.Name = "MaxOccursTextBox";
			this.MaxOccursTextBox.Size = new System.Drawing.Size(48, 22);
			this.MaxOccursTextBox.TabIndex = 1;
			this.MaxOccursTextBox.Text = "1";
			this.MaxOccursTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// CancelButton
			// 
			this.CancelButton.Enabled = false;
			this.CancelButton.Location = new System.Drawing.Point(240, 152);
			this.CancelButton.Name = "CancelButton";
			this.CancelButton.TabIndex = 7;
			this.CancelButton.Text = "Cancel";
			this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.AttributeRadioButton);
			this.groupBox3.Controls.Add(this.ElementRadioButton);
			this.groupBox3.Location = new System.Drawing.Point(320, 80);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(80, 64);
			this.groupBox3.TabIndex = 8;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Property";
			// 
			// AttributeRadioButton
			// 
			this.AttributeRadioButton.Location = new System.Drawing.Point(8, 40);
			this.AttributeRadioButton.Name = "AttributeRadioButton";
			this.AttributeRadioButton.Size = new System.Drawing.Size(64, 16);
			this.AttributeRadioButton.TabIndex = 1;
			this.AttributeRadioButton.Text = "Attribute";
			// 
			// ElementRadioButton
			// 
			this.ElementRadioButton.Checked = true;
			this.ElementRadioButton.Location = new System.Drawing.Point(8, 16);
			this.ElementRadioButton.Name = "ElementRadioButton";
			this.ElementRadioButton.Size = new System.Drawing.Size(64, 16);
			this.ElementRadioButton.TabIndex = 0;
			this.ElementRadioButton.TabStop = true;
			this.ElementRadioButton.Text = "Element";
			// 
			// FieldForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(416, 190);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.CancelButton);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.OKButton);
			this.Controls.Add(this.TypeComboBox);
			this.Controls.Add(this.textBox3);
			this.Controls.Add(this.NameTextBox);
			this.Controls.Add(this.textBox1);
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "FieldForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Field";
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion


		private void OnTextChanged(object sender, System.EventArgs e)
		{
			if(this.NameTextBox.Text=="")
			{
				OKButton.Enabled=false;
			}
			else
			{
				OKButton.Enabled = true;
			}
		}

		private void OKButton_Click(object sender, System.EventArgs e)
		{
			if(ElementRadioButton.Checked)
			{
				NewContentItem = new UPnPComplexType.Element();
			}
			else
			{
				NewContentItem = new UPnPComplexType.Attribute();
			}
			
			NewContentItem.Name = this.NameTextBox.Text;
			if(TypeComboBox.SelectedItem.GetType()==typeof(UPnPComplexType))
			{
				// Complex Type
				NewContentItem.Type = ((UPnPComplexType)TypeComboBox.SelectedItem).Name_LOCAL;
				NewContentItem.TypeNS = ((UPnPComplexType)TypeComboBox.SelectedItem).Name_NAMESPACE;
				NewContentItem.MinOccurs = MinOccursTextBox.Text;
				NewContentItem.MaxOccurs = MaxOccursRadio1.Checked?MaxOccursTextBox.Text:"UNBOUNDED";
			}
			else
			{
				// Simple Type
				NewContentItem.Type = TypeComboBox.SelectedItem.ToString();
				NewContentItem.TypeNS = "";
				NewContentItem.MinOccurs = MinOccursTextBox.Text;
				NewContentItem.MaxOccurs = MaxOccursRadio1.Checked?MaxOccursTextBox.Text:"UNBOUNDED";
			}
			this.DialogResult = DialogResult.OK;
		}

		private void CancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;
		}
	}
}
