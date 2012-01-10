using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPMediaServer
{
	/// <summary>
	/// Summary description for DebugForm.
	/// </summary>
	public class DebugForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox txtDbg;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public DebugForm()
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
			this.txtDbg = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// txtDbg
			// 
			this.txtDbg.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.txtDbg.Location = new System.Drawing.Point(8, 8);
			this.txtDbg.Multiline = true;
			this.txtDbg.Name = "txtDbg";
			this.txtDbg.Size = new System.Drawing.Size(552, 472);
			this.txtDbg.TabIndex = 0;
			this.txtDbg.Text = "";
			this.txtDbg.Visible = false;
			// 
			// DebugForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(568, 486);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.txtDbg});
			this.Name = "DebugForm";
			this.Text = "DebugForm";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
