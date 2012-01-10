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
	/// Summary description for TraceForm.
	/// </summary>
	public class TraceForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.ListBox ListViewBox;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public TraceForm(string[] items)
		{
			InitializeComponent();
			foreach(string x in items)
			{
				ListViewBox.Items.Add(x);
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
			this.ListViewBox = new System.Windows.Forms.ListBox();
			this.SuspendLayout();
			// 
			// ListViewBox
			// 
			this.ListViewBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ListViewBox.Name = "ListViewBox";
			this.ListViewBox.Size = new System.Drawing.Size(408, 316);
			this.ListViewBox.TabIndex = 0;
			// 
			// TraceForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(408, 318);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.ListViewBox});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "TraceForm";
			this.Text = "TraceForm";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
