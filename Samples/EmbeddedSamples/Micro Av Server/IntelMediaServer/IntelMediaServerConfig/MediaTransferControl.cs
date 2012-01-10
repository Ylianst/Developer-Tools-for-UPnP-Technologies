using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace IntelMediaServerConfig
{
	/// <summary>
	/// Summary description for MediaTransferControl.
	/// </summary>
	public class MediaTransferControl : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.PictureBox transferPictureBox;
		private System.Windows.Forms.Button backButton;
		private System.Windows.Forms.ProgressBar transferProgressBar;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public MediaTransferControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitForm call

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

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MediaTransferControl));
			this.backButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.transferPictureBox = new System.Windows.Forms.PictureBox();
			this.transferProgressBar = new System.Windows.Forms.ProgressBar();
			this.SuspendLayout();
			// 
			// backButton
			// 
			this.backButton.Dock = System.Windows.Forms.DockStyle.Fill;
			this.backButton.Name = "backButton";
			this.backButton.Size = new System.Drawing.Size(336, 38);
			this.backButton.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label1.Location = new System.Drawing.Point(38, 5);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(291, 16);
			this.label1.TabIndex = 4;
			// 
			// transferPictureBox
			// 
			this.transferPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("transferPictureBox.Image")));
			this.transferPictureBox.Location = new System.Drawing.Point(8, 8);
			this.transferPictureBox.Name = "transferPictureBox";
			this.transferPictureBox.Size = new System.Drawing.Size(24, 24);
			this.transferPictureBox.TabIndex = 3;
			this.transferPictureBox.TabStop = false;
			// 
			// transferProgressBar
			// 
			this.transferProgressBar.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.transferProgressBar.Location = new System.Drawing.Point(40, 23);
			this.transferProgressBar.Name = "transferProgressBar";
			this.transferProgressBar.Size = new System.Drawing.Size(288, 10);
			this.transferProgressBar.TabIndex = 5;
			// 
			// MediaTransferControl
			// 
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.transferProgressBar,
																		  this.label1,
																		  this.transferPictureBox,
																		  this.backButton});
			this.Name = "MediaTransferControl";
			this.Size = new System.Drawing.Size(336, 38);
			this.ResumeLayout(false);

		}
		#endregion
	}
}
