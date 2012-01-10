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

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for RendererAudioControlForm.
	/// </summary>
	public class RendererAudioControlForm : System.Windows.Forms.Form
	{
		private AVConnection connection = null;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TrackBar flTrackBar;
		private System.Windows.Forms.TrackBar frTrackBar;
		private System.Windows.Forms.TrackBar rlTrackBar;
		private System.Windows.Forms.TrackBar rrTrackBar;
		private System.Windows.Forms.TrackBar masterTrackBar;
		private System.Windows.Forms.CheckBox checkBox2;
		private System.Windows.Forms.CheckBox checkBox3;
		private System.Windows.Forms.CheckBox checkBox4;
		private System.Windows.Forms.CheckBox checkBox5;
		private System.Windows.Forms.CheckBox masterMuteCheckBox;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public AVConnection Connection 
		{
			set 
			{
				if (connection != null) 
				{
					connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
					connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					masterTrackBar.Value = 0;
					masterTrackBar.Enabled = false;
					masterMuteCheckBox.Checked = false;
					masterMuteCheckBox.Enabled = false;
					this.Text = "Audio Controls";
				}
				connection = value;
				if (connection != null) 
				{
					connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
					connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					masterTrackBar.Value = connection.MasterVolume;
					masterTrackBar.Enabled = true;
					masterMuteCheckBox.Checked = connection.IsMute;
					masterMuteCheckBox.Enabled = true;
					this.Text = "Audio Controls - (" + connection.ConnectionID.ToString() + ") " + connection.Parent.FriendlyName;
				}
			}
		}

		public RendererAudioControlForm()
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(RendererAudioControlForm));
			this.flTrackBar = new System.Windows.Forms.TrackBar();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.frTrackBar = new System.Windows.Forms.TrackBar();
			this.rlTrackBar = new System.Windows.Forms.TrackBar();
			this.rrTrackBar = new System.Windows.Forms.TrackBar();
			this.label1 = new System.Windows.Forms.Label();
			this.masterTrackBar = new System.Windows.Forms.TrackBar();
			this.masterMuteCheckBox = new System.Windows.Forms.CheckBox();
			this.checkBox2 = new System.Windows.Forms.CheckBox();
			this.checkBox3 = new System.Windows.Forms.CheckBox();
			this.checkBox4 = new System.Windows.Forms.CheckBox();
			this.checkBox5 = new System.Windows.Forms.CheckBox();
			((System.ComponentModel.ISupportInitialize)(this.flTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.frTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.rlTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.rrTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.masterTrackBar)).BeginInit();
			this.SuspendLayout();
			// 
			// flTrackBar
			// 
			this.flTrackBar.Location = new System.Drawing.Point(80, 24);
			this.flTrackBar.Maximum = 100;
			this.flTrackBar.Name = "flTrackBar";
			this.flTrackBar.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.flTrackBar.Size = new System.Drawing.Size(45, 104);
			this.flTrackBar.TabIndex = 1;
			this.flTrackBar.TickFrequency = 10;
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(64, 8);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(64, 16);
			this.label2.TabIndex = 3;
			this.label2.Text = "Front Left";
			this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(128, 8);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(64, 16);
			this.label3.TabIndex = 4;
			this.label3.Text = "Front Right";
			this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(192, 8);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(64, 16);
			this.label4.TabIndex = 5;
			this.label4.Text = "Rear Left";
			this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(256, 8);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(64, 16);
			this.label5.TabIndex = 6;
			this.label5.Text = "Rear Right";
			this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// frTrackBar
			// 
			this.frTrackBar.Location = new System.Drawing.Point(144, 24);
			this.frTrackBar.Maximum = 100;
			this.frTrackBar.Name = "frTrackBar";
			this.frTrackBar.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.frTrackBar.Size = new System.Drawing.Size(45, 104);
			this.frTrackBar.TabIndex = 7;
			this.frTrackBar.TickFrequency = 10;
			// 
			// rlTrackBar
			// 
			this.rlTrackBar.Location = new System.Drawing.Point(208, 24);
			this.rlTrackBar.Maximum = 100;
			this.rlTrackBar.Name = "rlTrackBar";
			this.rlTrackBar.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.rlTrackBar.Size = new System.Drawing.Size(45, 104);
			this.rlTrackBar.TabIndex = 8;
			this.rlTrackBar.TickFrequency = 10;
			// 
			// rrTrackBar
			// 
			this.rrTrackBar.Location = new System.Drawing.Point(272, 24);
			this.rrTrackBar.Maximum = 100;
			this.rrTrackBar.Name = "rrTrackBar";
			this.rrTrackBar.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.rrTrackBar.Size = new System.Drawing.Size(45, 104);
			this.rrTrackBar.TabIndex = 9;
			this.rrTrackBar.TickFrequency = 10;
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(0, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(64, 16);
			this.label1.TabIndex = 2;
			this.label1.Text = "Master";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// masterTrackBar
			// 
			this.masterTrackBar.Location = new System.Drawing.Point(16, 24);
			this.masterTrackBar.Maximum = 100;
			this.masterTrackBar.Name = "masterTrackBar";
			this.masterTrackBar.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.masterTrackBar.Size = new System.Drawing.Size(45, 104);
			this.masterTrackBar.TabIndex = 0;
			this.masterTrackBar.TickFrequency = 10;
			this.masterTrackBar.Scroll += new System.EventHandler(this.masterTrackBar_Scroll);
			// 
			// masterMuteCheckBox
			// 
			this.masterMuteCheckBox.Location = new System.Drawing.Point(8, 128);
			this.masterMuteCheckBox.Name = "masterMuteCheckBox";
			this.masterMuteCheckBox.Size = new System.Drawing.Size(48, 24);
			this.masterMuteCheckBox.TabIndex = 10;
			this.masterMuteCheckBox.Text = "Mute";
			this.masterMuteCheckBox.CheckedChanged += new System.EventHandler(this.masterMuteCheckBox_CheckedChanged);
			// 
			// checkBox2
			// 
			this.checkBox2.Location = new System.Drawing.Point(72, 128);
			this.checkBox2.Name = "checkBox2";
			this.checkBox2.Size = new System.Drawing.Size(48, 24);
			this.checkBox2.TabIndex = 11;
			this.checkBox2.Text = "Mute";
			// 
			// checkBox3
			// 
			this.checkBox3.Location = new System.Drawing.Point(136, 128);
			this.checkBox3.Name = "checkBox3";
			this.checkBox3.Size = new System.Drawing.Size(48, 24);
			this.checkBox3.TabIndex = 12;
			this.checkBox3.Text = "Mute";
			// 
			// checkBox4
			// 
			this.checkBox4.Location = new System.Drawing.Point(200, 128);
			this.checkBox4.Name = "checkBox4";
			this.checkBox4.Size = new System.Drawing.Size(48, 24);
			this.checkBox4.TabIndex = 13;
			this.checkBox4.Text = "Mute";
			// 
			// checkBox5
			// 
			this.checkBox5.Location = new System.Drawing.Point(264, 128);
			this.checkBox5.Name = "checkBox5";
			this.checkBox5.Size = new System.Drawing.Size(48, 24);
			this.checkBox5.TabIndex = 14;
			this.checkBox5.Text = "Mute";
			// 
			// RendererAudioControlForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(326, 156);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.checkBox5,
																		  this.checkBox4,
																		  this.checkBox3,
																		  this.checkBox2,
																		  this.masterMuteCheckBox,
																		  this.rrTrackBar,
																		  this.rlTrackBar,
																		  this.frTrackBar,
																		  this.label5,
																		  this.label4,
																		  this.label3,
																		  this.label2,
																		  this.label1,
																		  this.flTrackBar,
																		  this.masterTrackBar});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "RendererAudioControlForm";
			this.Text = "Audio Control";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.RendererAudioControlForm_Closing);
			((System.ComponentModel.ISupportInitialize)(this.flTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.frTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.rlTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.rrTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.masterTrackBar)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		private void VolumeChangedHandlerSink(AVConnection sender, UInt16 volume)
		{
			masterTrackBar.Value = (int)connection.MasterVolume;
		}

		private void MuteStateChangedHandlerSink(AVConnection sender, bool NewMuteStatus)
		{
			masterMuteCheckBox.Checked = NewMuteStatus;
		}

		private void RendererAudioControlForm_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
		}

		private void masterTrackBar_Scroll(object sender, System.EventArgs e)
		{
			connection.MasterVolume = (ushort)masterTrackBar.Value;
		}

		private void masterMuteCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			connection.Mute(masterMuteCheckBox.Checked);
		}

	}
}
