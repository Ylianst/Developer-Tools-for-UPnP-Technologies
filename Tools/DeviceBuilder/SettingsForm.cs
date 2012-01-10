using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for SettingsForm.
	/// </summary>
	public class SettingsForm : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.CheckBox errorEncodingCheckBox;
		private System.Windows.Forms.TextBox maxHttpHeaderTextBox;
		private System.Windows.Forms.TextBox maxSoapBodyTextBox;
		private System.Windows.Forms.TextBox ssdpCycleTextBox;

		public Hashtable Settings
		{
			get 
			{
				Hashtable t = new Hashtable();
				t["ExplicitErrorEncoding"] = errorEncodingCheckBox.Checked;
				t["MaxHttpHeaderTextBox"] = int.Parse(maxHttpHeaderTextBox.Text);
				t["MaxSoapBodySize"] = int.Parse(maxSoapBodyTextBox.Text);
				t["SsdpCycleTime"] = int.Parse(ssdpCycleTextBox.Text);
				return t;
			}
			set 
			{
				if (value.Contains("ExplicitErrorEncoding")) {errorEncodingCheckBox.Checked = (bool)value["ExplicitErrorEncoding"];}
				if (value.Contains("MaxHttpHeaderTextBox")) {maxHttpHeaderTextBox.Text = ((int)value["MaxHttpHeaderTextBox"]).ToString();}
				if (value.Contains("MaxSoapBodySize")) {maxSoapBodyTextBox.Text = ((int)value["MaxSoapBodySize"]).ToString();}
				if (value.Contains("SsdpCycleTime")) {ssdpCycleTextBox.Text = ((int)value["SsdpCycleTime"]).ToString();}
			}
		}

		public SettingsForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(SettingsForm));
			this.cancelButton = new System.Windows.Forms.Button();
			this.okButton = new System.Windows.Forms.Button();
			this.errorEncodingCheckBox = new System.Windows.Forms.CheckBox();
			this.maxSoapBodyTextBox = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.ssdpCycleTextBox = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.maxHttpHeaderTextBox = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(192, 112);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(96, 24);
			this.cancelButton.TabIndex = 4;
			this.cancelButton.Text = "Cancel";
			// 
			// okButton
			// 
			this.okButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.okButton.Location = new System.Drawing.Point(288, 112);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(96, 24);
			this.okButton.TabIndex = 3;
			this.okButton.Text = "OK";
			// 
			// errorEncodingCheckBox
			// 
			this.errorEncodingCheckBox.Location = new System.Drawing.Point(8, 8);
			this.errorEncodingCheckBox.Name = "errorEncodingCheckBox";
			this.errorEncodingCheckBox.Size = new System.Drawing.Size(272, 16);
			this.errorEncodingCheckBox.TabIndex = 5;
			this.errorEncodingCheckBox.Text = "Encode more explicit error response messages";
			// 
			// maxSoapBodyTextBox
			// 
			this.maxSoapBodyTextBox.Location = new System.Drawing.Point(8, 56);
			this.maxSoapBodyTextBox.Name = "maxSoapBodyTextBox";
			this.maxSoapBodyTextBox.Size = new System.Drawing.Size(56, 20);
			this.maxSoapBodyTextBox.TabIndex = 6;
			this.maxSoapBodyTextBox.Text = "8000";
			this.maxSoapBodyTextBox.TextChanged += new System.EventHandler(this.maxSoapBodyTextBox_TextChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(72, 59);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(312, 16);
			this.label1.TabIndex = 7;
			this.label1.Text = "Maximum inbound SOAP action HTTP body size (in bytes)";
			// 
			// ssdpCycleTextBox
			// 
			this.ssdpCycleTextBox.Location = new System.Drawing.Point(8, 80);
			this.ssdpCycleTextBox.Name = "ssdpCycleTextBox";
			this.ssdpCycleTextBox.Size = new System.Drawing.Size(56, 20);
			this.ssdpCycleTextBox.TabIndex = 8;
			this.ssdpCycleTextBox.Text = "1800";
			this.ssdpCycleTextBox.TextChanged += new System.EventHandler(this.ssdpCycleTextBox_TextChanged);
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(72, 83);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(312, 16);
			this.label2.TabIndex = 9;
			this.label2.Text = "Device SSDP notify cycle time (in seconds)";
			// 
			// maxHttpHeaderTextBox
			// 
			this.maxHttpHeaderTextBox.Location = new System.Drawing.Point(8, 32);
			this.maxHttpHeaderTextBox.Name = "maxHttpHeaderTextBox";
			this.maxHttpHeaderTextBox.Size = new System.Drawing.Size(56, 20);
			this.maxHttpHeaderTextBox.TabIndex = 10;
			this.maxHttpHeaderTextBox.Text = "4000";
			this.maxHttpHeaderTextBox.TextChanged += new System.EventHandler(this.maxHttpHeaderTextBox_TextChanged);
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(72, 35);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(312, 16);
			this.label3.TabIndex = 11;
			this.label3.Text = "Maximum inbound HTTP header size (in bytes)";
			// 
			// SettingsForm
			// 
			this.AcceptButton = this.okButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.cancelButton;
			this.ClientSize = new System.Drawing.Size(392, 142);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label3,
																		  this.maxHttpHeaderTextBox,
																		  this.label2,
																		  this.ssdpCycleTextBox,
																		  this.label1,
																		  this.maxSoapBodyTextBox,
																		  this.errorEncodingCheckBox,
																		  this.cancelButton,
																		  this.okButton});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "SettingsForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Device Generation Settings";
			this.ResumeLayout(false);

		}
		#endregion

		private void maxHttpHeaderTextBox_TextChanged(object sender, System.EventArgs e)
		{
			try 
			{
				maxHttpHeaderTextBox.Text = int.Parse(maxHttpHeaderTextBox.Text).ToString();
			} 
			catch
			{
				maxHttpHeaderTextBox.Text = "0";
			}
		}

		private void maxSoapBodyTextBox_TextChanged(object sender, System.EventArgs e)
		{
			try 
			{
				maxSoapBodyTextBox.Text = int.Parse(maxSoapBodyTextBox.Text).ToString();
			} 
			catch
			{
				maxHttpHeaderTextBox.Text = "0";
			}
		}

		private void ssdpCycleTextBox_TextChanged(object sender, System.EventArgs e)
		{
			try 
			{
				ssdpCycleTextBox.Text = int.Parse(ssdpCycleTextBox.Text).ToString();
			} 
			catch
			{
				maxHttpHeaderTextBox.Text = "0";
			}
		}

	}
}
