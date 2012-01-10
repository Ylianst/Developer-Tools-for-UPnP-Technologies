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

namespace UPnPMediaServerController
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class ConfigForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox txtbox_SearchCaps;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtbox_SortCaps;
		private System.Windows.Forms.Button btn_OK;
		private System.Windows.Forms.Button btn_Cancel;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ConfigForm(string searchCaps, string sortCaps)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			this.txtbox_SearchCaps.Text = searchCaps;
			this.txtbox_SortCaps.Text = sortCaps;
		}

		public string SearchCapabilities
		{
			get
			{
				return this.txtbox_SearchCaps.Text;
			}
		}

		public string SortCapabilities
		{
			get
			{
				return this.txtbox_SortCaps.Text;
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
			this.txtbox_SearchCaps = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.txtbox_SortCaps = new System.Windows.Forms.TextBox();
			this.btn_OK = new System.Windows.Forms.Button();
			this.btn_Cancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// txtbox_SearchCaps
			// 
			this.txtbox_SearchCaps.Location = new System.Drawing.Point(120, 16);
			this.txtbox_SearchCaps.Multiline = true;
			this.txtbox_SearchCaps.Name = "txtbox_SearchCaps";
			this.txtbox_SearchCaps.Size = new System.Drawing.Size(384, 88);
			this.txtbox_SearchCaps.TabIndex = 0;
			this.txtbox_SearchCaps.Text = "";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 16);
			this.label1.Name = "label1";
			this.label1.TabIndex = 1;
			this.label1.Text = "SearchCapabilities";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(8, 104);
			this.label2.Name = "label2";
			this.label2.TabIndex = 3;
			this.label2.Text = "SortCapabilities";
			// 
			// txtbox_SortCaps
			// 
			this.txtbox_SortCaps.Location = new System.Drawing.Point(120, 104);
			this.txtbox_SortCaps.Multiline = true;
			this.txtbox_SortCaps.Name = "txtbox_SortCaps";
			this.txtbox_SortCaps.Size = new System.Drawing.Size(384, 88);
			this.txtbox_SortCaps.TabIndex = 2;
			this.txtbox_SortCaps.Text = "";
			// 
			// btn_OK
			// 
			this.btn_OK.Location = new System.Drawing.Point(432, 200);
			this.btn_OK.Name = "btn_OK";
			this.btn_OK.TabIndex = 4;
			this.btn_OK.Text = "OK";
			// 
			// btn_Cancel
			// 
			this.btn_Cancel.Cursor = System.Windows.Forms.Cursors.Default;
			this.btn_Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btn_Cancel.Location = new System.Drawing.Point(352, 200);
			this.btn_Cancel.Name = "btn_Cancel";
			this.btn_Cancel.TabIndex = 5;
			this.btn_Cancel.Text = "Cancel";
			// 
			// ConfigForm
			// 
			this.AcceptButton = this.btn_OK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.btn_Cancel;
			this.ClientSize = new System.Drawing.Size(512, 230);
			this.Controls.Add(this.btn_Cancel);
			this.Controls.Add(this.btn_OK);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.txtbox_SortCaps);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.txtbox_SearchCaps);
			this.Name = "ConfigForm";
			this.Text = "MediaServer Configuration";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
