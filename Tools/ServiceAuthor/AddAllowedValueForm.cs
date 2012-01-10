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
	/// Summary description for AddAllowedValueForm.
	/// </summary>
	public class AddAllowedValueForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox allowedValueTextBox;
		private System.Windows.Forms.Button OKButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Label label1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public string AllowedValue 
		{
			set {allowedValueTextBox.Text = value;}
			get {return allowedValueTextBox.Text;}
		}

		public AddAllowedValueForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(AddAllowedValueForm));
			this.allowedValueTextBox = new System.Windows.Forms.TextBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// allowedValueTextBox
			// 
			this.allowedValueTextBox.AccessibleDescription = ((string)(resources.GetObject("allowedValueTextBox.AccessibleDescription")));
			this.allowedValueTextBox.AccessibleName = ((string)(resources.GetObject("allowedValueTextBox.AccessibleName")));
			this.allowedValueTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("allowedValueTextBox.Anchor")));
			this.allowedValueTextBox.AutoSize = ((bool)(resources.GetObject("allowedValueTextBox.AutoSize")));
			this.allowedValueTextBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("allowedValueTextBox.BackgroundImage")));
			this.allowedValueTextBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("allowedValueTextBox.Dock")));
			this.allowedValueTextBox.Enabled = ((bool)(resources.GetObject("allowedValueTextBox.Enabled")));
			this.allowedValueTextBox.Font = ((System.Drawing.Font)(resources.GetObject("allowedValueTextBox.Font")));
			this.allowedValueTextBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("allowedValueTextBox.ImeMode")));
			this.allowedValueTextBox.Location = ((System.Drawing.Point)(resources.GetObject("allowedValueTextBox.Location")));
			this.allowedValueTextBox.MaxLength = ((int)(resources.GetObject("allowedValueTextBox.MaxLength")));
			this.allowedValueTextBox.Multiline = ((bool)(resources.GetObject("allowedValueTextBox.Multiline")));
			this.allowedValueTextBox.Name = "allowedValueTextBox";
			this.allowedValueTextBox.PasswordChar = ((char)(resources.GetObject("allowedValueTextBox.PasswordChar")));
			this.allowedValueTextBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("allowedValueTextBox.RightToLeft")));
			this.allowedValueTextBox.ScrollBars = ((System.Windows.Forms.ScrollBars)(resources.GetObject("allowedValueTextBox.ScrollBars")));
			this.allowedValueTextBox.Size = ((System.Drawing.Size)(resources.GetObject("allowedValueTextBox.Size")));
			this.allowedValueTextBox.TabIndex = ((int)(resources.GetObject("allowedValueTextBox.TabIndex")));
			this.allowedValueTextBox.Text = resources.GetString("allowedValueTextBox.Text");
			this.allowedValueTextBox.TextAlign = ((System.Windows.Forms.HorizontalAlignment)(resources.GetObject("allowedValueTextBox.TextAlign")));
			this.allowedValueTextBox.Visible = ((bool)(resources.GetObject("allowedValueTextBox.Visible")));
			this.allowedValueTextBox.WordWrap = ((bool)(resources.GetObject("allowedValueTextBox.WordWrap")));
			// 
			// OKButton
			// 
			this.OKButton.AccessibleDescription = ((string)(resources.GetObject("OKButton.AccessibleDescription")));
			this.OKButton.AccessibleName = ((string)(resources.GetObject("OKButton.AccessibleName")));
			this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("OKButton.Anchor")));
			this.OKButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("OKButton.BackgroundImage")));
			this.OKButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("OKButton.Dock")));
			this.OKButton.Enabled = ((bool)(resources.GetObject("OKButton.Enabled")));
			this.OKButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("OKButton.FlatStyle")));
			this.OKButton.Font = ((System.Drawing.Font)(resources.GetObject("OKButton.Font")));
			this.OKButton.Image = ((System.Drawing.Image)(resources.GetObject("OKButton.Image")));
			this.OKButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("OKButton.ImageAlign")));
			this.OKButton.ImageIndex = ((int)(resources.GetObject("OKButton.ImageIndex")));
			this.OKButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("OKButton.ImeMode")));
			this.OKButton.Location = ((System.Drawing.Point)(resources.GetObject("OKButton.Location")));
			this.OKButton.Name = "OKButton";
			this.OKButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("OKButton.RightToLeft")));
			this.OKButton.Size = ((System.Drawing.Size)(resources.GetObject("OKButton.Size")));
			this.OKButton.TabIndex = ((int)(resources.GetObject("OKButton.TabIndex")));
			this.OKButton.Text = resources.GetString("OKButton.Text");
			this.OKButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("OKButton.TextAlign")));
			this.OKButton.Visible = ((bool)(resources.GetObject("OKButton.Visible")));
			this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.AccessibleDescription = ((string)(resources.GetObject("cancelButton.AccessibleDescription")));
			this.cancelButton.AccessibleName = ((string)(resources.GetObject("cancelButton.AccessibleName")));
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("cancelButton.Anchor")));
			this.cancelButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("cancelButton.BackgroundImage")));
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("cancelButton.Dock")));
			this.cancelButton.Enabled = ((bool)(resources.GetObject("cancelButton.Enabled")));
			this.cancelButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("cancelButton.FlatStyle")));
			this.cancelButton.Font = ((System.Drawing.Font)(resources.GetObject("cancelButton.Font")));
			this.cancelButton.Image = ((System.Drawing.Image)(resources.GetObject("cancelButton.Image")));
			this.cancelButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.ImageAlign")));
			this.cancelButton.ImageIndex = ((int)(resources.GetObject("cancelButton.ImageIndex")));
			this.cancelButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("cancelButton.ImeMode")));
			this.cancelButton.Location = ((System.Drawing.Point)(resources.GetObject("cancelButton.Location")));
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("cancelButton.RightToLeft")));
			this.cancelButton.Size = ((System.Drawing.Size)(resources.GetObject("cancelButton.Size")));
			this.cancelButton.TabIndex = ((int)(resources.GetObject("cancelButton.TabIndex")));
			this.cancelButton.Text = resources.GetString("cancelButton.Text");
			this.cancelButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.TextAlign")));
			this.cancelButton.Visible = ((bool)(resources.GetObject("cancelButton.Visible")));
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// label1
			// 
			this.label1.AccessibleDescription = ((string)(resources.GetObject("label1.AccessibleDescription")));
			this.label1.AccessibleName = ((string)(resources.GetObject("label1.AccessibleName")));
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("label1.Anchor")));
			this.label1.AutoSize = ((bool)(resources.GetObject("label1.AutoSize")));
			this.label1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("label1.Dock")));
			this.label1.Enabled = ((bool)(resources.GetObject("label1.Enabled")));
			this.label1.Font = ((System.Drawing.Font)(resources.GetObject("label1.Font")));
			this.label1.Image = ((System.Drawing.Image)(resources.GetObject("label1.Image")));
			this.label1.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.ImageAlign")));
			this.label1.ImageIndex = ((int)(resources.GetObject("label1.ImageIndex")));
			this.label1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("label1.ImeMode")));
			this.label1.Location = ((System.Drawing.Point)(resources.GetObject("label1.Location")));
			this.label1.Name = "label1";
			this.label1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("label1.RightToLeft")));
			this.label1.Size = ((System.Drawing.Size)(resources.GetObject("label1.Size")));
			this.label1.TabIndex = ((int)(resources.GetObject("label1.TabIndex")));
			this.label1.Text = resources.GetString("label1.Text");
			this.label1.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.TextAlign")));
			this.label1.Visible = ((bool)(resources.GetObject("label1.Visible")));
			// 
			// AddAllowedValueForm
			// 
			this.AcceptButton = this.OKButton;
			this.AccessibleDescription = ((string)(resources.GetObject("$this.AccessibleDescription")));
			this.AccessibleName = ((string)(resources.GetObject("$this.AccessibleName")));
			this.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("$this.Anchor")));
			this.AutoScaleBaseSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScaleBaseSize")));
			this.AutoScroll = ((bool)(resources.GetObject("$this.AutoScroll")));
			this.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMargin")));
			this.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMinSize")));
			this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
			this.CancelButton = this.cancelButton;
			this.ClientSize = ((System.Drawing.Size)(resources.GetObject("$this.ClientSize")));
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label1,
																		  this.cancelButton,
																		  this.OKButton,
																		  this.allowedValueTextBox});
			this.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("$this.Dock")));
			this.Enabled = ((bool)(resources.GetObject("$this.Enabled")));
			this.Font = ((System.Drawing.Font)(resources.GetObject("$this.Font")));
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("$this.ImeMode")));
			this.Location = ((System.Drawing.Point)(resources.GetObject("$this.Location")));
			this.MaximizeBox = false;
			this.MaximumSize = ((System.Drawing.Size)(resources.GetObject("$this.MaximumSize")));
			this.MinimumSize = ((System.Drawing.Size)(resources.GetObject("$this.MinimumSize")));
			this.Name = "AddAllowedValueForm";
			this.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("$this.RightToLeft")));
			this.StartPosition = ((System.Windows.Forms.FormStartPosition)(resources.GetObject("$this.StartPosition")));
			this.Text = resources.GetString("$this.Text");
			this.Visible = ((bool)(resources.GetObject("$this.Visible")));
			this.ResumeLayout(false);

		}
		#endregion

		private void OKButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.OK;
		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;
		}
	}
}
