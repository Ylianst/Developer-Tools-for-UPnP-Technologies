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
	/// Summary description for QueryBox.
	/// </summary>
	public class QueryBox : System.Windows.Forms.Form
	{
		public string ServiceURN = "";

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TextBox URNTextBox;
		private System.Windows.Forms.Button OKButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public QueryBox()
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(QueryBox));
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.URNTextBox = new System.Windows.Forms.TextBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.URNTextBox});
			this.groupBox1.Location = new System.Drawing.Point(8, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(496, 56);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Service URN";
			// 
			// URNTextBox
			// 
			this.URNTextBox.Location = new System.Drawing.Point(16, 24);
			this.URNTextBox.Name = "URNTextBox";
			this.URNTextBox.Size = new System.Drawing.Size(472, 20);
			this.URNTextBox.TabIndex = 0;
			this.URNTextBox.Text = "urn:schemas-upnp-org:service:";
			// 
			// OKButton
			// 
			this.OKButton.Location = new System.Drawing.Point(424, 72);
			this.OKButton.Name = "OKButton";
			this.OKButton.Size = new System.Drawing.Size(80, 24);
			this.OKButton.TabIndex = 1;
			this.OKButton.Text = "OK";
			this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
			// 
			// QueryBox
			// 
			this.AcceptButton = this.OKButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(514, 104);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.OKButton,
																		  this.groupBox1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "QueryBox";
			this.Text = "QueryBox";
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void OKButton_Click(object sender, System.EventArgs e)
		{
			ServiceURN = URNTextBox.Text;
			this.DialogResult = DialogResult.OK;
		}
	}
}
