using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPMediaServerController
{
	/// <summary>
	/// Summary description for SocketDataWindow.
	/// </summary>
	public class SocketDataWindow : System.Windows.Forms.Form
	{
		public System.Windows.Forms.TextBox txtbox_SocketData;
		private System.Windows.Forms.ContextMenu contextMenu1;
		private System.Windows.Forms.MenuItem menuItem2;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public SocketDataWindow()
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
			this.txtbox_SocketData = new System.Windows.Forms.TextBox();
			this.contextMenu1 = new System.Windows.Forms.ContextMenu();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.SuspendLayout();
			// 
			// txtbox_SocketData
			// 
			this.txtbox_SocketData.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.txtbox_SocketData.ContextMenu = this.contextMenu1;
			this.txtbox_SocketData.Location = new System.Drawing.Point(8, 8);
			this.txtbox_SocketData.MaxLength = 1048576;
			this.txtbox_SocketData.Multiline = true;
			this.txtbox_SocketData.Name = "txtbox_SocketData";
			this.txtbox_SocketData.ReadOnly = true;
			this.txtbox_SocketData.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtbox_SocketData.Size = new System.Drawing.Size(464, 368);
			this.txtbox_SocketData.TabIndex = 0;
			this.txtbox_SocketData.Text = "";
			// 
			// contextMenu1
			// 
			this.contextMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						 this.menuItem2});
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 0;
			this.menuItem2.Text = "Clear";
			this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
			// 
			// SocketDataWindow
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(480, 382);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.txtbox_SocketData});
			this.Name = "SocketDataWindow";
			this.Text = "SocketDataWindow";
			this.ResumeLayout(false);

		}
		#endregion

		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			if (this.OnClear != null)
			{
				this.OnClear(this);
			}
		}

		public delegate void OnClearHandler (SocketDataWindow sender);
		public event OnClearHandler OnClear;
	}
}
