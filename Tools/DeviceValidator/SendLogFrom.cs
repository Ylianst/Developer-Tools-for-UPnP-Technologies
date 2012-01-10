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

namespace UPnPValidator
{
	/// <summary>
	/// Summary description for SendLogFrom.
	/// </summary>
	public class SendLogFrom : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label CompanyLabel;
		private System.Windows.Forms.TextBox CompanyNameTextBox;
		private System.Windows.Forms.TextBox EmailAddressTextBox;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private new System.Windows.Forms.Button CancelButton;
		private System.Windows.Forms.Button SendButton;
		private System.Windows.Forms.TextBox DeviceDescriptionTextBox;
		private System.Windows.Forms.TextBox SMTPServerTextBox;
		private System.Windows.Forms.Label label3;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public SendLogFrom()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		public new string CompanyName
		{
			get {return CompanyNameTextBox.Text;}
		}

		public string EmailAddress
		{
			get {return EmailAddressTextBox.Text;}
		}

		public string DeviceDescription
		{
			get {return DeviceDescriptionTextBox.Text;}
		}
		public string SMTPServer
		{
			get {return SMTPServerTextBox.Text;}
			set {SMTPServerTextBox.Text = value;}
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(SendLogFrom));
			this.CompanyLabel = new System.Windows.Forms.Label();
			this.CompanyNameTextBox = new System.Windows.Forms.TextBox();
			this.EmailAddressTextBox = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.DeviceDescriptionTextBox = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.CancelButton = new System.Windows.Forms.Button();
			this.SendButton = new System.Windows.Forms.Button();
			this.SMTPServerTextBox = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// CompanyLabel
			// 
			this.CompanyLabel.Location = new System.Drawing.Point(16, 18);
			this.CompanyLabel.Name = "CompanyLabel";
			this.CompanyLabel.Size = new System.Drawing.Size(120, 16);
			this.CompanyLabel.TabIndex = 0;
			this.CompanyLabel.Text = "Company";
			// 
			// CompanyNameTextBox
			// 
			this.CompanyNameTextBox.Location = new System.Drawing.Point(152, 16);
			this.CompanyNameTextBox.Name = "CompanyNameTextBox";
			this.CompanyNameTextBox.Size = new System.Drawing.Size(192, 20);
			this.CompanyNameTextBox.TabIndex = 1;
			this.CompanyNameTextBox.Text = "";
			// 
			// EmailAddressTextBox
			// 
			this.EmailAddressTextBox.Location = new System.Drawing.Point(152, 48);
			this.EmailAddressTextBox.Name = "EmailAddressTextBox";
			this.EmailAddressTextBox.Size = new System.Drawing.Size(192, 20);
			this.EmailAddressTextBox.TabIndex = 3;
			this.EmailAddressTextBox.Text = "";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(16, 48);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(112, 16);
			this.label1.TabIndex = 2;
			this.label1.Text = "Email Address";
			// 
			// DeviceDescriptionTextBox
			// 
			this.DeviceDescriptionTextBox.Location = new System.Drawing.Point(152, 112);
			this.DeviceDescriptionTextBox.Multiline = true;
			this.DeviceDescriptionTextBox.Name = "DeviceDescriptionTextBox";
			this.DeviceDescriptionTextBox.Size = new System.Drawing.Size(192, 64);
			this.DeviceDescriptionTextBox.TabIndex = 5;
			this.DeviceDescriptionTextBox.Text = "";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(16, 112);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(120, 16);
			this.label2.TabIndex = 4;
			this.label2.Text = "Device Description";
			// 
			// CancelButton
			// 
			this.CancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.CancelButton.Location = new System.Drawing.Point(152, 192);
			this.CancelButton.Name = "CancelButton";
			this.CancelButton.Size = new System.Drawing.Size(96, 23);
			this.CancelButton.TabIndex = 6;
			this.CancelButton.Text = "Cancel";
			// 
			// SendButton
			// 
			this.SendButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.SendButton.Location = new System.Drawing.Point(248, 192);
			this.SendButton.Name = "SendButton";
			this.SendButton.Size = new System.Drawing.Size(96, 23);
			this.SendButton.TabIndex = 7;
			this.SendButton.Text = "Send";
			// 
			// SMTPServerTextBox
			// 
			this.SMTPServerTextBox.Location = new System.Drawing.Point(152, 80);
			this.SMTPServerTextBox.Name = "SMTPServerTextBox";
			this.SMTPServerTextBox.Size = new System.Drawing.Size(192, 20);
			this.SMTPServerTextBox.TabIndex = 9;
			this.SMTPServerTextBox.Text = "";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(16, 80);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(112, 16);
			this.label3.TabIndex = 8;
			this.label3.Text = "SMTP Server";
			// 
			// SendLogFrom
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(376, 230);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.SMTPServerTextBox,
																		  this.label3,
																		  this.SendButton,
																		  this.CancelButton,
																		  this.DeviceDescriptionTextBox,
																		  this.label2,
																		  this.EmailAddressTextBox,
																		  this.label1,
																		  this.CompanyNameTextBox,
																		  this.CompanyLabel});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "SendLogFrom";
			this.Text = "SendLogFrom";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
