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
	/// Summary description for ContainerProperty.
	/// </summary>
	public class ContainerProperty : System.Windows.Forms.Form
	{
		public UPnPComplexType.RestrictionExtension re;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.RadioButton RestrictionRadioButton;
		private System.Windows.Forms.RadioButton ExtensionRadioButton;
		private System.Windows.Forms.ComboBox BaseComboBox;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.TextBox PatternTextBox;
		private System.Windows.Forms.Button OKButton;
		private System.Windows.Forms.Button CancelButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ContainerProperty(UPnPComplexType[] ComplexTypes, UPnPComplexType.GenericContainer gc)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			BaseComboBox.SelectedIndex=0;

			foreach(UPnPComplexType ct in ComplexTypes)
			{
				BaseComboBox.Items.Add(ct);
			}

			if(gc.GetType()==typeof(UPnPComplexType.ComplexContent))
			{
				UPnPComplexType.ComplexContent cc = (UPnPComplexType.ComplexContent)gc;
				if(cc.RestExt != null && cc.RestExt.GetType()==typeof(UPnPComplexType.Restriction))
				{
					UPnPComplexType.Restriction r = (UPnPComplexType.Restriction)((UPnPComplexType.ComplexContent)gc).RestExt;
					PatternTextBox.Text = r.PATTERN;
				}
			}
			else if(gc.GetType()==typeof(UPnPComplexType.SimpleContent))
			{
				UPnPComplexType.SimpleContent cc = (UPnPComplexType.SimpleContent)gc;
				if(cc.RestExt != null && cc.RestExt.GetType()==typeof(UPnPComplexType.Restriction))
				{
					UPnPComplexType.Restriction r = (UPnPComplexType.Restriction)((UPnPComplexType.ComplexContent)gc).RestExt;
					PatternTextBox.Text = r.PATTERN;
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
			this.RestrictionRadioButton = new System.Windows.Forms.RadioButton();
			this.ExtensionRadioButton = new System.Windows.Forms.RadioButton();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.BaseComboBox = new System.Windows.Forms.ComboBox();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.PatternTextBox = new System.Windows.Forms.TextBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.CancelButton = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// RestrictionRadioButton
			// 
			this.RestrictionRadioButton.Checked = true;
			this.RestrictionRadioButton.Location = new System.Drawing.Point(8, 120);
			this.RestrictionRadioButton.Name = "RestrictionRadioButton";
			this.RestrictionRadioButton.Size = new System.Drawing.Size(80, 24);
			this.RestrictionRadioButton.TabIndex = 0;
			this.RestrictionRadioButton.TabStop = true;
			this.RestrictionRadioButton.Text = "Restriction";
			this.RestrictionRadioButton.CheckedChanged += new System.EventHandler(this.OnRadioButtonChanged);
			// 
			// ExtensionRadioButton
			// 
			this.ExtensionRadioButton.Location = new System.Drawing.Point(8, 144);
			this.ExtensionRadioButton.Name = "ExtensionRadioButton";
			this.ExtensionRadioButton.Size = new System.Drawing.Size(72, 24);
			this.ExtensionRadioButton.TabIndex = 1;
			this.ExtensionRadioButton.Text = "Extension";
			this.ExtensionRadioButton.CheckedChanged += new System.EventHandler(this.OnRadioButtonChanged);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.BaseComboBox);
			this.groupBox1.Location = new System.Drawing.Point(8, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(344, 48);
			this.groupBox1.TabIndex = 2;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Base";
			// 
			// BaseComboBox
			// 
			this.BaseComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.BaseComboBox.Items.AddRange(new object[] {
															  "any",
															  "string",
															  "normalizedString",
															  "token",
															  "byte",
															  "unsignedByte",
															  "base64Binary",
															  "base64Binary",
															  "hexBinary",
															  "integer",
															  "positiveInteger",
															  "negativeInteger",
															  "nonNegativeInteger",
															  "nonPositiveInteger",
															  "int",
															  "unsignedInt",
															  "long",
															  "unsignedLong",
															  "short",
															  "unsignedShort",
															  "decimal",
															  "float",
															  "double",
															  "boolean",
															  "time",
															  "dateTime",
															  "duration",
															  "date",
															  "gMonth",
															  "gYear",
															  "gYearMonth",
															  "gDay",
															  "gMonthDay",
															  "Name",
															  "QName",
															  "NCName",
															  "anyURI"});
			this.BaseComboBox.Location = new System.Drawing.Point(8, 16);
			this.BaseComboBox.MaxDropDownItems = 16;
			this.BaseComboBox.Name = "BaseComboBox";
			this.BaseComboBox.Size = new System.Drawing.Size(328, 21);
			this.BaseComboBox.TabIndex = 0;
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.PatternTextBox);
			this.groupBox2.Location = new System.Drawing.Point(8, 64);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(344, 48);
			this.groupBox2.TabIndex = 3;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Pattern";
			// 
			// PatternTextBox
			// 
			this.PatternTextBox.Location = new System.Drawing.Point(8, 16);
			this.PatternTextBox.Name = "PatternTextBox";
			this.PatternTextBox.Size = new System.Drawing.Size(328, 20);
			this.PatternTextBox.TabIndex = 0;
			this.PatternTextBox.Text = "";
			// 
			// OKButton
			// 
			this.OKButton.Location = new System.Drawing.Point(280, 136);
			this.OKButton.Name = "OKButton";
			this.OKButton.TabIndex = 4;
			this.OKButton.Text = "OK";
			this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
			// 
			// CancelButton
			// 
			this.CancelButton.Location = new System.Drawing.Point(192, 136);
			this.CancelButton.Name = "CancelButton";
			this.CancelButton.TabIndex = 5;
			this.CancelButton.Text = "Cancel";
			// 
			// ContainerProperty
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(368, 174);
			this.Controls.Add(this.CancelButton);
			this.Controls.Add(this.OKButton);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.ExtensionRadioButton);
			this.Controls.Add(this.RestrictionRadioButton);
			this.Name = "ContainerProperty";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "ContainerProperty";
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void OnRadioButtonChanged(object sender, System.EventArgs e)
		{
			if(ExtensionRadioButton.Checked)
			{
				PatternTextBox.Enabled = false;
			}
			else
			{
				PatternTextBox.Enabled = true;
			}
		}

		private void OKButton_Click(object sender, System.EventArgs e)
		{
			if(RestrictionRadioButton.Checked)
			{
				UPnPComplexType.Restriction r = new UPnPComplexType.Restriction();
				r.PATTERN = PatternTextBox.Text;
				r.baseType = BaseComboBox.SelectedItem.ToString();
				if(BaseComboBox.SelectedItem.GetType()==typeof(UPnPComplexType))
				{
					r.baseTypeNS = ((UPnPComplexType)BaseComboBox.SelectedItem).Name_NAMESPACE;
				}
				re = r;
			}
			else
			{
				UPnPComplexType.Extension ex = new UPnPComplexType.Extension();
				ex.baseType = BaseComboBox.SelectedItem.ToString();
				if(BaseComboBox.SelectedItem.GetType()==typeof(UPnPComplexType))
				{
					ex.baseTypeNS = ((UPnPComplexType)BaseComboBox.SelectedItem).Name_NAMESPACE;
				}
				re = ex;
			}
			DialogResult = DialogResult.OK;
		}

	}
}
