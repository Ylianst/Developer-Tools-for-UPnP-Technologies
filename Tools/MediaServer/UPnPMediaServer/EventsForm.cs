using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPMediaServerController
{
	/// <summary>
	/// Summary description for DirectoryConfigurationForm.
	/// </summary>
	public class EventsForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.ListView eventListView;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public EventsForm()
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(EventsForm));
			this.eventListView = new System.Windows.Forms.ListView();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
			this.SuspendLayout();
			// 
			// eventListView
			// 
			this.eventListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																							this.columnHeader1,
																							this.columnHeader2});
			this.eventListView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.eventListView.Name = "eventListView";
			this.eventListView.Size = new System.Drawing.Size(438, 292);
			this.eventListView.TabIndex = 1;
			this.eventListView.View = System.Windows.Forms.View.Details;
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Time";
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Event";
			this.columnHeader2.Width = 360;
			// 
			// EventsForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(438, 292);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.eventListView});
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "EventsForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Intel\'s UPnP Media Server Events";
			this.ResumeLayout(false);

		}
		#endregion

		private void closeButton_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		public void Event(string message) 
		{
			eventListView.Items.Add(new ListViewItem(new string[] {DateTime.Now.ToShortTimeString(),message}));
		}

	}
}
