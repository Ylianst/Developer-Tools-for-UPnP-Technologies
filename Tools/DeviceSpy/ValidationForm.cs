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

namespace UPnpSpy
{
	/// <summary>
	/// Summary description for ValidationForm.
	/// </summary>
	public class ValidationForm : System.Windows.Forms.Form
	{
		private UPnPService service;
		private System.Windows.Forms.CheckedListBox ServiceBox;
		private System.Windows.Forms.ListBox FailedBox;
		private System.Windows.Forms.TextBox DescBox;
		private System.Windows.Forms.Button GoButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ValidationForm(UPnPService S)
		{
			service = S;
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			foreach(UPnPAction A in S.Actions)
			{
				ServiceBox.Items.Add(A,CheckState.Unchecked);
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
			this.ServiceBox = new System.Windows.Forms.CheckedListBox();
			this.FailedBox = new System.Windows.Forms.ListBox();
			this.DescBox = new System.Windows.Forms.TextBox();
			this.GoButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// ServiceBox
			// 
			this.ServiceBox.Location = new System.Drawing.Point(8, 16);
			this.ServiceBox.Name = "ServiceBox";
			this.ServiceBox.Size = new System.Drawing.Size(232, 274);
			this.ServiceBox.Sorted = true;
			this.ServiceBox.TabIndex = 0;
			// 
			// FailedBox
			// 
			this.FailedBox.Location = new System.Drawing.Point(280, 16);
			this.FailedBox.Name = "FailedBox";
			this.FailedBox.Size = new System.Drawing.Size(224, 95);
			this.FailedBox.TabIndex = 1;
			// 
			// DescBox
			// 
			this.DescBox.Location = new System.Drawing.Point(248, 120);
			this.DescBox.Multiline = true;
			this.DescBox.Name = "DescBox";
			this.DescBox.ReadOnly = true;
			this.DescBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.DescBox.Size = new System.Drawing.Size(256, 168);
			this.DescBox.TabIndex = 2;
			this.DescBox.Text = "";
			// 
			// GoButton
			// 
			this.GoButton.Location = new System.Drawing.Point(248, 16);
			this.GoButton.Name = "GoButton";
			this.GoButton.Size = new System.Drawing.Size(24, 96);
			this.GoButton.TabIndex = 3;
			this.GoButton.Text = "Go";
			this.GoButton.Click += new System.EventHandler(this.GoButton_Click);
			// 
			// ValidationForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(512, 294);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.GoButton,
																		  this.DescBox,
																		  this.FailedBox,
																		  this.ServiceBox});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "ValidationForm";
			this.Text = "ValidationForm";
			this.ResumeLayout(false);

		}
		#endregion

		private void GoButton_Click(object sender, System.EventArgs e)
		{
			foreach(UPnPAction A in ServiceBox.CheckedItems)
			{
				TestAction(A);
			}
		}
		private void TestAction(UPnPAction A)
		{
			ArrayList ArgList = new ArrayList();
			Hashtable RelTable = new Hashtable();
			UPnPArgument G;
			foreach(UPnPArgument arg in A.Arguments)
			{
				if(arg.IsReturnValue==false)
				{
					G = new UPnPArgument(arg.Name,UPnPService.CreateObjectInstance(arg.RelatedStateVar.GetNetType(),null));
					RelTable[G] = arg.RelatedStateVar;
					ArgList.Add(G);
				}
			}


		}
	}
}
