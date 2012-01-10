using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for CertToolFiles.
	/// </summary>
	public class CertToolFiles : System.Windows.Forms.Form
	{
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Button okButton;
		public System.Windows.Forms.TextBox devicePath;
		public System.Windows.Forms.TextBox servicePath;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CertToolFiles()
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.okButton = new System.Windows.Forms.Button();
			this.devicePath = new System.Windows.Forms.TextBox();
			this.servicePath = new System.Windows.Forms.TextBox();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.devicePath);
			this.groupBox1.Location = new System.Drawing.Point(8, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(416, 64);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Path for Device files";
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.servicePath);
			this.groupBox2.Location = new System.Drawing.Point(8, 80);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(416, 48);
			this.groupBox2.TabIndex = 1;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Path for Service files";
			// 
			// okButton
			// 
			this.okButton.Location = new System.Drawing.Point(344, 144);
			this.okButton.Name = "okButton";
			this.okButton.TabIndex = 2;
			this.okButton.Text = "Okay";
			this.okButton.Click += new System.EventHandler(this.okButton_Click);
			// 
			// devicePath
			// 
			this.devicePath.Location = new System.Drawing.Point(8, 24);
			this.devicePath.Name = "devicePath";
			this.devicePath.Size = new System.Drawing.Size(368, 20);
			this.devicePath.TabIndex = 0;
			this.devicePath.Text = "";
			// 
			// servicePath
			// 
			this.servicePath.Location = new System.Drawing.Point(8, 16);
			this.servicePath.Name = "servicePath";
			this.servicePath.Size = new System.Drawing.Size(368, 20);
			this.servicePath.TabIndex = 0;
			this.servicePath.Text = "";
			// 
			// CertToolFiles
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(432, 182);
			this.Controls.Add(this.okButton);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Name = "CertToolFiles";
			this.Text = "CertToolFiles";
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void okButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.OK;
		}
	}
}
