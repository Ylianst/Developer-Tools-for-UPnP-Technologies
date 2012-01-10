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

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for PortNumber.
	/// </summary>
	public class PortNumberForm : System.Windows.Forms.Form
	{
		public int Port = 0;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TextBox PortTextBox;
		private System.Windows.Forms.Button OkayButton;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.TextBox publicIP;
		private System.Windows.Forms.CheckBox usePublicIP;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public PortNumberForm(int Port)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			this.Port = Port;
			PortTextBox.Text = Port.ToString();
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.PortTextBox = new System.Windows.Forms.TextBox();
			this.OkayButton = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.publicIP = new System.Windows.Forms.TextBox();
			this.usePublicIP = new System.Windows.Forms.CheckBox();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.PortTextBox);
			this.groupBox1.Location = new System.Drawing.Point(8, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(200, 72);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Choose port number";
			// 
			// PortTextBox
			// 
			this.PortTextBox.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.PortTextBox.Location = new System.Drawing.Point(56, 24);
			this.PortTextBox.MaxLength = 5;
			this.PortTextBox.Name = "PortTextBox";
			this.PortTextBox.Size = new System.Drawing.Size(72, 32);
			this.PortTextBox.TabIndex = 0;
			this.PortTextBox.Text = "";
			// 
			// OkayButton
			// 
			this.OkayButton.Location = new System.Drawing.Point(132, 176);
			this.OkayButton.Name = "OkayButton";
			this.OkayButton.TabIndex = 3;
			this.OkayButton.Text = "Okay";
			this.OkayButton.Click += new System.EventHandler(this.OkayButton_Click_1);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.usePublicIP);
			this.groupBox2.Controls.Add(this.publicIP);
			this.groupBox2.Location = new System.Drawing.Point(8, 80);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(200, 88);
			this.groupBox2.TabIndex = 4;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Public IP Address";
			// 
			// publicIP
			// 
			this.publicIP.Location = new System.Drawing.Point(8, 24);
			this.publicIP.Name = "publicIP";
			this.publicIP.Size = new System.Drawing.Size(176, 20);
			this.publicIP.TabIndex = 0;
			this.publicIP.Text = "";
			// 
			// usePublicIP
			// 
			this.usePublicIP.Location = new System.Drawing.Point(8, 56);
			this.usePublicIP.Name = "usePublicIP";
			this.usePublicIP.Size = new System.Drawing.Size(72, 24);
			this.usePublicIP.TabIndex = 1;
			this.usePublicIP.Text = "Override";
			// 
			// PortNumberForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(218, 208);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.OkayButton);
			this.Controls.Add(this.groupBox1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "PortNumberForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "UPnP Relay";
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion


		private void OkayButton_Click_1(object sender, System.EventArgs e)
		{
			try
			{
				Port = int.Parse(PortTextBox.Text);
				this.DialogResult = DialogResult.OK;
			}
			catch(Exception)
			{
				MessageBox.Show("Please enter a valid integer");
			}
		}
		public string PublicIPAddress
		{
			get
			{
				if(usePublicIP.Checked)
				{
					return(publicIP.Text);
				}
				else
				{
					return("");
				}
			}
		}
	}
}
