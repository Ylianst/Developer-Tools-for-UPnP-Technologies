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
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for TransferForm.
	/// </summary>
	public class TransferForm : System.Windows.Forms.Form
	{
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.ProgressBar transferProgressBar;
		private System.Windows.Forms.Label transferProgressLabel;
		private System.Windows.Forms.TextBox destinationTextBox;
		private System.Windows.Forms.TextBox sourceTextBox;
		private System.Windows.Forms.TextBox stateTextBox;
		private IResourceTransfer transferObject;

		public TransferForm(IResourceTransfer transferObject)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			this.transferObject = transferObject;
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(TransferForm));
			this.transferProgressBar = new System.Windows.Forms.ProgressBar();
			this.label1 = new System.Windows.Forms.Label();
			this.transferProgressLabel = new System.Windows.Forms.Label();
			this.okButton = new System.Windows.Forms.Button();
			this.destinationTextBox = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.sourceTextBox = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.cancelButton = new System.Windows.Forms.Button();
			this.label5 = new System.Windows.Forms.Label();
			this.stateTextBox = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// transferProgressBar
			// 
			this.transferProgressBar.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.transferProgressBar.Location = new System.Drawing.Point(8, 112);
			this.transferProgressBar.Name = "transferProgressBar";
			this.transferProgressBar.Size = new System.Drawing.Size(362, 23);
			this.transferProgressBar.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
			this.label1.Location = new System.Drawing.Point(8, 96);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(136, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Transfer Progress...";
			// 
			// transferProgressLabel
			// 
			this.transferProgressLabel.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.transferProgressLabel.Location = new System.Drawing.Point(234, 96);
			this.transferProgressLabel.Name = "transferProgressLabel";
			this.transferProgressLabel.Size = new System.Drawing.Size(136, 16);
			this.transferProgressLabel.TabIndex = 2;
			this.transferProgressLabel.Text = "0%";
			this.transferProgressLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// okButton
			// 
			this.okButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.okButton.Enabled = false;
			this.okButton.Location = new System.Drawing.Point(290, 144);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(80, 23);
			this.okButton.TabIndex = 3;
			this.okButton.Text = "OK";
			// 
			// destinationTextBox
			// 
			this.destinationTextBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.destinationTextBox.Location = new System.Drawing.Point(88, 56);
			this.destinationTextBox.Name = "destinationTextBox";
			this.destinationTextBox.ReadOnly = true;
			this.destinationTextBox.Size = new System.Drawing.Size(282, 20);
			this.destinationTextBox.TabIndex = 4;
			this.destinationTextBox.Text = "";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(8, 61);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(64, 16);
			this.label3.TabIndex = 5;
			this.label3.Text = "Destination";
			// 
			// sourceTextBox
			// 
			this.sourceTextBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.sourceTextBox.Location = new System.Drawing.Point(88, 32);
			this.sourceTextBox.Name = "sourceTextBox";
			this.sourceTextBox.ReadOnly = true;
			this.sourceTextBox.Size = new System.Drawing.Size(282, 20);
			this.sourceTextBox.TabIndex = 6;
			this.sourceTextBox.Text = "";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(8, 37);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(56, 16);
			this.label4.TabIndex = 7;
			this.label4.Text = "Source";
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.cancelButton.Location = new System.Drawing.Point(202, 144);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(80, 23);
			this.cancelButton.TabIndex = 8;
			this.cancelButton.Text = "Cancel";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(8, 13);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(80, 16);
			this.label5.TabIndex = 9;
			this.label5.Text = "Transfer State";
			// 
			// stateTextBox
			// 
			this.stateTextBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.stateTextBox.Location = new System.Drawing.Point(88, 8);
			this.stateTextBox.Name = "stateTextBox";
			this.stateTextBox.ReadOnly = true;
			this.stateTextBox.Size = new System.Drawing.Size(282, 20);
			this.stateTextBox.TabIndex = 10;
			this.stateTextBox.Text = "";
			// 
			// TransferForm
			// 
			this.AcceptButton = this.okButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(378, 176);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.stateTextBox,
																		  this.label5,
																		  this.cancelButton,
																		  this.label4,
																		  this.sourceTextBox,
																		  this.label3,
																		  this.destinationTextBox,
																		  this.okButton,
																		  this.transferProgressLabel,
																		  this.label1,
																		  this.transferProgressBar});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "TransferForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Media Transfer";
			this.Load += new System.EventHandler(this.TransferForm_Load);
			this.ResumeLayout(false);

		}
		#endregion

		private void TransferForm_Load(object sender, System.EventArgs e)
		{
			transferObject.RequestGetTransferProgress(null,new CpMediaDelegates.Delegate_ResultGetTransferProgress(ResultGetTransferProgressSink));
		}

		static int antiblockcount = 0;
		private void ResultGetTransferProgressSink(IResourceTransfer transferObject, CpContentDirectory.Enum_A_ARG_TYPE_TransferStatus transferStatus, System.Int64 transferLength, System.Int64 transferTotal, object Tag, UPnPInvokeException error, Exception castError) 
		{
			label1.Text = (antiblockcount++).ToString();

			if (transferLength < 0) 
			{
				transferProgressLabel.Text = transferTotal + " bytes send";
				transferProgressBar.Value = 0;
				transferProgressBar.Maximum = 100;
			}
			else
			{
				transferProgressBar.Value = (int)transferTotal;
				transferProgressBar.Maximum = (int)transferLength;
				transferProgressLabel.Text = (((double)transferLength / (double)transferTotal) * 100).ToString() + "%";
			}
			stateTextBox.Text = transferStatus.ToString();

			if (transferStatus == CpContentDirectory.Enum_A_ARG_TYPE_TransferStatus.IN_PROGRESS) 
			{
				transferObject.RequestGetTransferProgress(null,new CpMediaDelegates.Delegate_ResultGetTransferProgress(ResultGetTransferProgressSink));
			}
		}

	}
}
