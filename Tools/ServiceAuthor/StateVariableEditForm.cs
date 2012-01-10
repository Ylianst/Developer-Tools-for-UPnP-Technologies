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
	/// Summary description for StateVariableEditForm.
	/// </summary>
	public class StateVariableEditForm : System.Windows.Forms.Form
	{
		private UPnPService service;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Button addAllowedButton;
		private System.Windows.Forms.Button removeAllowedButton;
		private System.Windows.Forms.TextBox nameTextBox;
		private System.Windows.Forms.ComboBox typeComboBox;
		private System.Windows.Forms.ListBox allowedValuesListBox;
		private System.Windows.Forms.TextBox minTextBox;
		private System.Windows.Forms.TextBox maxTextBox;
		private System.Windows.Forms.TextBox stepTextBox;
		private System.Windows.Forms.CheckBox allowedValuesCheckBox;
		private System.Windows.Forms.CheckBox minCheckBox;
		private System.Windows.Forms.CheckBox maxCheckBox;
		private System.Windows.Forms.CheckBox stepCheckBox;
		private System.Windows.Forms.CheckBox eventedCheckBox;
		private System.Windows.Forms.TextBox defaultValueTextBox;
		private System.Windows.Forms.CheckBox defaultValueCheckBox;
		private System.Windows.Forms.Button advButton;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.CheckBox MulticastEvent;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public StateVariableEditForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			typeComboBox.SelectedIndex = 0;

			this.SetClientSizeCore(this.ClientRectangle.Width,pictureBox1.Top);
			advButton.Text = "More >>";
		}
		public StateVariableEditForm(UPnPService ParentService):this()
		{
			service = ParentService;
			foreach(UPnPComplexType CT in service.GetComplexTypeList())
			{
				typeComboBox.Items.Add(CT);
			}
		}
		

		public UPnPStateVariable StateVariable 
		{
			get {return GetState();}
			set {SetState(value);}
		}

		private UPnPStateVariable GetState() 
		{
			UPnPStateVariable state = null;
			switch (typeComboBox.SelectedIndex)
			{
				case 0: // Boolean
					state = new UPnPStateVariable(nameTextBox.Text,typeof(bool),eventedCheckBox.Checked);
					break;
				case 1: // Byte Array
					state = new UPnPStateVariable(nameTextBox.Text,typeof(byte[]),eventedCheckBox.Checked);
					break;
				case 2: // Integer 8
					state = new UPnPStateVariable(nameTextBox.Text,typeof(sbyte),eventedCheckBox.Checked);
					break;
				case 3: // Integer 16
					state = new UPnPStateVariable(nameTextBox.Text,typeof(short),eventedCheckBox.Checked);
					break;
				case 4: // Integer 32
					state = new UPnPStateVariable(nameTextBox.Text,typeof(int),eventedCheckBox.Checked);
					break;
				case 5: // String
					state = new UPnPStateVariable(nameTextBox.Text,typeof(string),eventedCheckBox.Checked);
					break;
				case 6: // Unsigned Integer 8
					state = new UPnPStateVariable(nameTextBox.Text,typeof(byte),eventedCheckBox.Checked);
					break;
				case 7: // Unsigned Integer 16
					state = new UPnPStateVariable(nameTextBox.Text,typeof(ushort),eventedCheckBox.Checked);
					break;
				case 8: // Unsigned Integer 32
					state = new UPnPStateVariable(nameTextBox.Text,typeof(uint),eventedCheckBox.Checked);
					break;
				case 9: // Uri
					state = new UPnPStateVariable(nameTextBox.Text,typeof(System.Uri),eventedCheckBox.Checked);
					break;
				case 10: // DateTime
					state = new UPnPStateVariable(nameTextBox.Text,typeof(DateTime),eventedCheckBox.Checked);
					break;
				default:
					state = new UPnPStateVariable(nameTextBox.Text,(UPnPComplexType)typeComboBox.SelectedItem);
					break;
			}
			state.MulticastEvent = MulticastEvent.Checked;
			if (allowedValuesCheckBox.Checked == true && allowedValuesListBox.Items.Count > 0) 
			{
				string[] allowed = new string[allowedValuesListBox.Items.Count];
				int i = 0;
				foreach (string s in allowedValuesListBox.Items) 
				{
					allowed[i] = s;
					i++;
				}
				state.AllowedStringValues = allowed;
			}
			if (defaultValueCheckBox.Checked == true && defaultValueTextBox.Text.Length > 0) 
			{
				state.DefaultValue = defaultValueTextBox.Text;
			}
			if (minCheckBox.Checked == true)
			{
				// TODO: Read only...
				state.Minimum = long.Parse(minTextBox.Text);
			}
			if (maxCheckBox.Checked == true)
			{
				state.Maximum = long.Parse(maxTextBox.Text);
			}
			if (stepCheckBox.Checked == true)
			{
				state.Step = long.Parse(stepTextBox.Text);
			}
			return state;
		}

		private void SetState(UPnPStateVariable state) 
		{
			if (state == null) return;
			nameTextBox.Text = state.Name;
			eventedCheckBox.Checked = state.SendEvent;
			MulticastEvent.Checked = state.MulticastEvent;
			this.service = state.OwningService;

			if (state.DefaultValue != null) 
			{
				defaultValueCheckBox.Checked = true;
				defaultValueTextBox.Text = state.DefaultValue.ToString();
			}

			switch (state.ValueType) 
			{
				case "boolean":
					typeComboBox.SelectedIndex = 0;
					break;
				case "bin.base64":
					typeComboBox.SelectedIndex = 1;
					break;
				case "i1":
					typeComboBox.SelectedIndex = 2;
					break;
				case "i2":
					typeComboBox.SelectedIndex = 3;
					break;
				case "i4":
					typeComboBox.SelectedIndex = 4;
					break;
				case "string":
					typeComboBox.SelectedIndex = 5;
					break;
				case "ui1":
					typeComboBox.SelectedIndex = 6;
					break;
				case "ui2":
					typeComboBox.SelectedIndex = 7;
					break;
				case "ui4":
					typeComboBox.SelectedIndex = 8;
					break;
				case "uri":
					typeComboBox.SelectedIndex = 9;
					break;
				case "dateTime":
					typeComboBox.SelectedIndex = 10;
					break;
			}
			if (state.AllowedStringValues != null) 
			{
				allowedValuesCheckBox.Checked = true;
				allowedValuesListBox.BackColor = SystemColors.Window;
				foreach (string s in state.AllowedStringValues) 
				{
					allowedValuesListBox.Items.Add(s);
				}
			}
			if (state.Minimum != null) 
			{
				minCheckBox.Checked = true;
				minTextBox.Enabled = true;
				minTextBox.Text = state.Minimum.ToString();
			}
			if (state.Maximum != null) 
			{
				maxCheckBox.Checked = true;
				maxTextBox.Enabled = true;
				maxTextBox.Text = state.Maximum.ToString();
			}
			if (state.Step != null) 
			{
				stepCheckBox.Checked = true;
				stepTextBox.Enabled = true;
				stepTextBox.Text = state.Step.ToString();
			}
			if(state.ComplexType!=null)
			{
				int i=10;
				foreach(UPnPComplexType CT in state.OwningService.GetComplexTypeList())
				{
					typeComboBox.Items.Add(CT);
					if(state.ComplexType==CT){typeComboBox.SelectedIndex = i;}
					++i;
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

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StateVariableEditForm));
            this.label1 = new System.Windows.Forms.Label();
            this.nameTextBox = new System.Windows.Forms.TextBox();
            this.typeComboBox = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.allowedValuesCheckBox = new System.Windows.Forms.CheckBox();
            this.minCheckBox = new System.Windows.Forms.CheckBox();
            this.maxCheckBox = new System.Windows.Forms.CheckBox();
            this.stepCheckBox = new System.Windows.Forms.CheckBox();
            this.minTextBox = new System.Windows.Forms.TextBox();
            this.maxTextBox = new System.Windows.Forms.TextBox();
            this.stepTextBox = new System.Windows.Forms.TextBox();
            this.allowedValuesListBox = new System.Windows.Forms.ListBox();
            this.addAllowedButton = new System.Windows.Forms.Button();
            this.removeAllowedButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.eventedCheckBox = new System.Windows.Forms.CheckBox();
            this.defaultValueTextBox = new System.Windows.Forms.TextBox();
            this.defaultValueCheckBox = new System.Windows.Forms.CheckBox();
            this.advButton = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.MulticastEvent = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // nameTextBox
            // 
            resources.ApplyResources(this.nameTextBox, "nameTextBox");
            this.nameTextBox.Name = "nameTextBox";
            // 
            // typeComboBox
            // 
            this.typeComboBox.DisplayMember = "Boolean";
            this.typeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            resources.ApplyResources(this.typeComboBox, "typeComboBox");
            this.typeComboBox.Items.AddRange(new object[] {
            resources.GetString("typeComboBox.Items"),
            resources.GetString("typeComboBox.Items1"),
            resources.GetString("typeComboBox.Items2"),
            resources.GetString("typeComboBox.Items3"),
            resources.GetString("typeComboBox.Items4"),
            resources.GetString("typeComboBox.Items5"),
            resources.GetString("typeComboBox.Items6"),
            resources.GetString("typeComboBox.Items7"),
            resources.GetString("typeComboBox.Items8"),
            resources.GetString("typeComboBox.Items9"),
            resources.GetString("typeComboBox.Items10")});
            this.typeComboBox.Name = "typeComboBox";
            this.typeComboBox.ValueMember = "Boolean";
            this.typeComboBox.SelectedIndexChanged += new System.EventHandler(this.typeComboBox_SelectedIndexChanged);
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // allowedValuesCheckBox
            // 
            resources.ApplyResources(this.allowedValuesCheckBox, "allowedValuesCheckBox");
            this.allowedValuesCheckBox.Name = "allowedValuesCheckBox";
            this.allowedValuesCheckBox.CheckedChanged += new System.EventHandler(this.allowedValuesCheckBox_CheckedChanged);
            // 
            // minCheckBox
            // 
            resources.ApplyResources(this.minCheckBox, "minCheckBox");
            this.minCheckBox.Name = "minCheckBox";
            this.minCheckBox.CheckedChanged += new System.EventHandler(this.minCheckBox_CheckedChanged);
            // 
            // maxCheckBox
            // 
            resources.ApplyResources(this.maxCheckBox, "maxCheckBox");
            this.maxCheckBox.Name = "maxCheckBox";
            this.maxCheckBox.CheckedChanged += new System.EventHandler(this.maxCheckBox_CheckedChanged);
            // 
            // stepCheckBox
            // 
            resources.ApplyResources(this.stepCheckBox, "stepCheckBox");
            this.stepCheckBox.Name = "stepCheckBox";
            this.stepCheckBox.CheckedChanged += new System.EventHandler(this.stepCheckBox_CheckedChanged);
            // 
            // minTextBox
            // 
            resources.ApplyResources(this.minTextBox, "minTextBox");
            this.minTextBox.Name = "minTextBox";
            // 
            // maxTextBox
            // 
            resources.ApplyResources(this.maxTextBox, "maxTextBox");
            this.maxTextBox.Name = "maxTextBox";
            // 
            // stepTextBox
            // 
            resources.ApplyResources(this.stepTextBox, "stepTextBox");
            this.stepTextBox.Name = "stepTextBox";
            // 
            // allowedValuesListBox
            // 
            this.allowedValuesListBox.BackColor = System.Drawing.SystemColors.Control;
            resources.ApplyResources(this.allowedValuesListBox, "allowedValuesListBox");
            this.allowedValuesListBox.Name = "allowedValuesListBox";
            // 
            // addAllowedButton
            // 
            resources.ApplyResources(this.addAllowedButton, "addAllowedButton");
            this.addAllowedButton.Name = "addAllowedButton";
            this.addAllowedButton.Click += new System.EventHandler(this.addAllowedButton_Click);
            // 
            // removeAllowedButton
            // 
            resources.ApplyResources(this.removeAllowedButton, "removeAllowedButton");
            this.removeAllowedButton.Name = "removeAllowedButton";
            this.removeAllowedButton.Click += new System.EventHandler(this.removeAllowedButton_Click);
            // 
            // okButton
            // 
            resources.ApplyResources(this.okButton, "okButton");
            this.okButton.Name = "okButton";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            resources.ApplyResources(this.cancelButton, "cancelButton");
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // eventedCheckBox
            // 
            resources.ApplyResources(this.eventedCheckBox, "eventedCheckBox");
            this.eventedCheckBox.Name = "eventedCheckBox";
            // 
            // defaultValueTextBox
            // 
            resources.ApplyResources(this.defaultValueTextBox, "defaultValueTextBox");
            this.defaultValueTextBox.Name = "defaultValueTextBox";
            // 
            // defaultValueCheckBox
            // 
            resources.ApplyResources(this.defaultValueCheckBox, "defaultValueCheckBox");
            this.defaultValueCheckBox.Name = "defaultValueCheckBox";
            this.defaultValueCheckBox.CheckedChanged += new System.EventHandler(this.defaultValueCheckBox_CheckedChanged);
            // 
            // advButton
            // 
            resources.ApplyResources(this.advButton, "advButton");
            this.advButton.Name = "advButton";
            this.advButton.Click += new System.EventHandler(this.advButton_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.Color.DimGray;
            resources.ApplyResources(this.pictureBox1, "pictureBox1");
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.TabStop = false;
            // 
            // pictureBox2
            // 
            this.pictureBox2.BackColor = System.Drawing.Color.DimGray;
            resources.ApplyResources(this.pictureBox2, "pictureBox2");
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.TabStop = false;
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.Name = "label3";
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // MulticastEvent
            // 
            resources.ApplyResources(this.MulticastEvent, "MulticastEvent");
            this.MulticastEvent.Name = "MulticastEvent";
            // 
            // StateVariableEditForm
            // 
            this.AcceptButton = this.okButton;
            resources.ApplyResources(this, "$this");
            this.CancelButton = this.cancelButton;
            this.Controls.Add(this.MulticastEvent);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.advButton);
            this.Controls.Add(this.defaultValueCheckBox);
            this.Controls.Add(this.defaultValueTextBox);
            this.Controls.Add(this.eventedCheckBox);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.typeComboBox);
            this.Controls.Add(this.nameTextBox);
            this.Controls.Add(this.removeAllowedButton);
            this.Controls.Add(this.addAllowedButton);
            this.Controls.Add(this.allowedValuesListBox);
            this.Controls.Add(this.stepTextBox);
            this.Controls.Add(this.maxTextBox);
            this.Controls.Add(this.minTextBox);
            this.Controls.Add(this.stepCheckBox);
            this.Controls.Add(this.maxCheckBox);
            this.Controls.Add(this.minCheckBox);
            this.Controls.Add(this.allowedValuesCheckBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MaximizeBox = false;
            this.Name = "StateVariableEditForm";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		private void allowedValuesCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			addAllowedButton.Enabled = allowedValuesCheckBox.Checked;
			removeAllowedButton.Enabled = allowedValuesCheckBox.Checked;
			allowedValuesListBox.Enabled = allowedValuesCheckBox.Checked;
			if (allowedValuesCheckBox.Checked == true) 
			{
				allowedValuesListBox.BackColor = SystemColors.Window;
			} 
			else 
			{
				allowedValuesListBox.BackColor = SystemColors.Control;
			}
		}

		private void minCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			minTextBox.Enabled = minCheckBox.Checked;
		}

		private void maxCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			maxTextBox.Enabled = maxCheckBox.Checked;
		}

		private void stepCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			stepTextBox.Enabled = stepCheckBox.Checked;
		}

		private void defaultValueCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			defaultValueTextBox.Enabled = defaultValueCheckBox.Checked;
		}

		private void okButton_Click(object sender, System.EventArgs e)
		{
			if (nameTextBox.Text.Length == 0) 
			{
				MessageBox.Show(this,"State variables must have a name","State variable",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
			} 
			else
			{
				this.DialogResult = DialogResult.OK;
			}
		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;		
		}

		private void addAllowedButton_Click(object sender, System.EventArgs e)
		{
			if(allowedValuesCheckBox.Visible==true)
			{
				AddAllowedValueForm form = new AddAllowedValueForm();
				DialogResult r = form.ShowDialog(this);
				if (r == DialogResult.OK && form.AllowedValue.Length > 0) 
				{
					allowedValuesListBox.Items.Add(form.AllowedValue);
				}
			}
		}

		private void removeAllowedButton_Click(object sender, System.EventArgs e)
		{
			if (allowedValuesListBox.SelectedIndex >= 0) 
			{
				allowedValuesListBox.Items.RemoveAt(allowedValuesListBox.SelectedIndex);
			}
		}

		private void advButton_Click(object sender, System.EventArgs e)
		{
			if (this.ClientRectangle.Height == pictureBox1.Top)
			{
				this.SetClientSizeCore(this.ClientRectangle.Width,pictureBox2.Top);
				advButton.Text = "Less <<";
			} 
			else 
			{
				this.SetClientSizeCore(this.ClientRectangle.Width,pictureBox1.Top);
				advButton.Text = "More >>";
			}
		}

		private void typeComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if(((ComboBox)sender).SelectedIndex>9)
			{
				addAllowedButton.Enabled = false;
				removeAllowedButton.Enabled = false;
				allowedValuesListBox.Enabled = false;
				allowedValuesListBox.BackColor = SystemColors.Window;
				allowedValuesCheckBox.Visible = false;

				defaultValueCheckBox.Enabled = false;
				defaultValueTextBox.Enabled = false;
				minCheckBox.Enabled = false;
				minTextBox.Enabled = false;
				maxCheckBox.Enabled = false;
				maxTextBox.Enabled = false;
				stepCheckBox.Enabled = false;
				stepTextBox.Enabled = false;
			}
			else
			{
				allowedValuesCheckBox.Visible = true;
				defaultValueCheckBox.Enabled = true;
				minCheckBox.Enabled = true;
				maxCheckBox.Enabled = true;
				stepCheckBox.Enabled = true;
				if(defaultValueCheckBox.Checked) {defaultValueTextBox.Enabled=true;}
				if(minCheckBox.Checked) {minTextBox.Enabled=true;}
				if(maxCheckBox.Checked) {maxTextBox.Enabled = true;}
				if(stepCheckBox.Checked) {stepTextBox.Enabled = true;}

				addAllowedButton.Enabled = allowedValuesCheckBox.Checked;
				removeAllowedButton.Enabled = allowedValuesCheckBox.Checked;
				allowedValuesListBox.Enabled = allowedValuesCheckBox.Checked;
				if (allowedValuesCheckBox.Checked == true) 
				{
					allowedValuesListBox.BackColor = SystemColors.Window;
				} 
				else 
				{
					allowedValuesListBox.BackColor = SystemColors.Control;
				}
			}
		}

	}
}
