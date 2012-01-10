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
using System.Data;
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
	/// Summary description for RendererControl.
	/// </summary>
	public class RendererControl : System.Windows.Forms.UserControl
	{
		private bool AdjustingVolume = false;
		private bool AdjustingLeftVolume = false;
		private bool AdjustingRightVolume = false;

		private AVConnection connection = null;
		private System.Windows.Forms.PictureBox pausePictureBox2;
		private System.Windows.Forms.PictureBox stopPictureBox2;
		private System.Windows.Forms.PictureBox playPictureBox2;
		private System.Windows.Forms.PictureBox pausePictureBox1;
		private System.Windows.Forms.PictureBox stopPictureBox1;
		private System.Windows.Forms.PictureBox playPictureBox1;
		private System.Windows.Forms.PictureBox pictureBox3;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.PictureBox mutePictureBox;
		private System.Windows.Forms.PictureBox mutedPictureBox;
		private System.Windows.Forms.Button muteButton;
		private System.Windows.Forms.Button pauseButton;
		private System.Windows.Forms.Button stopButton;
		private System.Windows.Forms.Button playButton;
		private System.Windows.Forms.TrackBar volumeTrackBar;
		private System.Windows.Forms.PictureBox pictureBox6;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.ProgressBar mediaProgressBar;
		private System.Windows.Forms.Button prevTrackButton;
		private System.Windows.Forms.Button nextTrackButton;
		private System.Windows.Forms.Button recordButton;
		private System.Windows.Forms.PictureBox recordPictureBox1;
		private System.Windows.Forms.PictureBox recordPictureBox2;
		private System.Windows.Forms.PictureBox pictureBox5;
		private System.Windows.Forms.PictureBox pictureBox7;
		private System.Windows.Forms.TrackBar leftChannelTrackBar;
		private System.Windows.Forms.TrackBar rightChannelTrackBar;
		private System.Windows.Forms.ToolTip toolTip;
		private System.ComponentModel.IContainer components;

		public RendererControl()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
		}

		public AVConnection Connection  
		{
			get 
			{
				return connection;
			}
			set 
			{
				if (connection != null) 
				{
					connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
					connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
					connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
					connection = null;
				}
				connection = value;

				if (connection != null) 
				{
					if(connection.SupportsRecord==false)
					{
						recordButton.Enabled = false;
					}
					else
					{
						recordButton.Enabled = true;
					}
					if(connection.SupportsPause==false) 
					{
						pauseButton.Enabled = false;
					}
					else
					{
						pauseButton.Enabled = true;
					}

					if (connection.SupportsCurrentPosition == true) 
					{
						mediaProgressBar.Maximum = (int)connection.Duration.TotalSeconds;
						if (connection.CurrentPosition.TotalSeconds <= connection.Duration.TotalSeconds) 
						{
							mediaProgressBar.Value = (int)connection.CurrentPosition.TotalSeconds;
						}
						mediaProgressBar.Enabled = true;
					} 
					else
					{
						mediaProgressBar.Maximum = 100;
						mediaProgressBar.Value = 100;
						mediaProgressBar.Enabled = false;
					}
					volumeTrackBar.Value = (int)connection.MasterVolume;
					foreach(string Channels in connection.SupportedChannels)
					{
						if(Channels=="Master") volumeTrackBar.Value = (int)connection.MasterVolume;
						if(Channels=="LF") leftChannelTrackBar.Value = (int)connection.GetVolume(Channels);
						if(Channels=="RF")
						{
							rightChannelTrackBar.Value = (int)connection.GetVolume(Channels);
						}
					}

					connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
					connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
					connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);

					if (connection.IsMute == true)
					{
						muteButton.Image = mutedPictureBox.Image;
					} 
					else 
					{
						muteButton.Image = mutePictureBox.Image;
					}

					PlayStateChangedHandlerSink(connection,connection.CurrentState);
				}
			}
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if (connection != null) 
			{
				connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection = null;
			}

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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(RendererControl));
			this.panel1 = new System.Windows.Forms.Panel();
			this.mediaProgressBar = new System.Windows.Forms.ProgressBar();
			this.pictureBox5 = new System.Windows.Forms.PictureBox();
			this.pictureBox7 = new System.Windows.Forms.PictureBox();
			this.leftChannelTrackBar = new System.Windows.Forms.TrackBar();
			this.rightChannelTrackBar = new System.Windows.Forms.TrackBar();
			this.recordPictureBox2 = new System.Windows.Forms.PictureBox();
			this.recordPictureBox1 = new System.Windows.Forms.PictureBox();
			this.pauseButton = new System.Windows.Forms.Button();
			this.stopButton = new System.Windows.Forms.Button();
			this.recordButton = new System.Windows.Forms.Button();
			this.nextTrackButton = new System.Windows.Forms.Button();
			this.prevTrackButton = new System.Windows.Forms.Button();
			this.pictureBox6 = new System.Windows.Forms.PictureBox();
			this.pausePictureBox2 = new System.Windows.Forms.PictureBox();
			this.stopPictureBox2 = new System.Windows.Forms.PictureBox();
			this.playPictureBox2 = new System.Windows.Forms.PictureBox();
			this.pausePictureBox1 = new System.Windows.Forms.PictureBox();
			this.stopPictureBox1 = new System.Windows.Forms.PictureBox();
			this.playPictureBox1 = new System.Windows.Forms.PictureBox();
			this.pictureBox3 = new System.Windows.Forms.PictureBox();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.mutePictureBox = new System.Windows.Forms.PictureBox();
			this.mutedPictureBox = new System.Windows.Forms.PictureBox();
			this.muteButton = new System.Windows.Forms.Button();
			this.playButton = new System.Windows.Forms.Button();
			this.volumeTrackBar = new System.Windows.Forms.TrackBar();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.panel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.leftChannelTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.rightChannelTrackBar)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).BeginInit();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.mediaProgressBar,
																				 this.pictureBox5,
																				 this.pictureBox7,
																				 this.leftChannelTrackBar,
																				 this.rightChannelTrackBar,
																				 this.recordPictureBox2,
																				 this.recordPictureBox1,
																				 this.pauseButton,
																				 this.stopButton,
																				 this.recordButton,
																				 this.nextTrackButton,
																				 this.prevTrackButton,
																				 this.pictureBox6,
																				 this.pausePictureBox2,
																				 this.stopPictureBox2,
																				 this.playPictureBox2,
																				 this.pausePictureBox1,
																				 this.stopPictureBox1,
																				 this.playPictureBox1,
																				 this.pictureBox3,
																				 this.pictureBox2,
																				 this.mutePictureBox,
																				 this.mutedPictureBox,
																				 this.muteButton,
																				 this.playButton,
																				 this.volumeTrackBar});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(576, 56);
			this.panel1.TabIndex = 54;
			// 
			// mediaProgressBar
			// 
			this.mediaProgressBar.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.mediaProgressBar.Cursor = System.Windows.Forms.Cursors.Hand;
			this.mediaProgressBar.Location = new System.Drawing.Point(8, 40);
			this.mediaProgressBar.Name = "mediaProgressBar";
			this.mediaProgressBar.Size = new System.Drawing.Size(560, 12);
			this.mediaProgressBar.TabIndex = 54;
			this.mediaProgressBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseUp);
			this.mediaProgressBar.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseMove);
			this.mediaProgressBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseDown);
			// 
			// pictureBox5
			// 
			this.pictureBox5.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox5.Image")));
			this.pictureBox5.Location = new System.Drawing.Point(429, 9);
			this.pictureBox5.Name = "pictureBox5";
			this.pictureBox5.Size = new System.Drawing.Size(11, 17);
			this.pictureBox5.TabIndex = 64;
			this.pictureBox5.TabStop = false;
			// 
			// pictureBox7
			// 
			this.pictureBox7.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox7.Image")));
			this.pictureBox7.Location = new System.Drawing.Point(493, 9);
			this.pictureBox7.Name = "pictureBox7";
			this.pictureBox7.Size = new System.Drawing.Size(18, 15);
			this.pictureBox7.TabIndex = 63;
			this.pictureBox7.TabStop = false;
			// 
			// leftChannelTrackBar
			// 
			this.leftChannelTrackBar.Location = new System.Drawing.Point(429, 3);
			this.leftChannelTrackBar.Maximum = 100;
			this.leftChannelTrackBar.Name = "leftChannelTrackBar";
			this.leftChannelTrackBar.Size = new System.Drawing.Size(56, 45);
			this.leftChannelTrackBar.TabIndex = 62;
			this.leftChannelTrackBar.TickFrequency = 20;
			this.toolTip.SetToolTip(this.leftChannelTrackBar, "Left Audio Channel Volume");
			this.leftChannelTrackBar.Value = 100;
			this.leftChannelTrackBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.LeftVolumeMouseUp);
			this.leftChannelTrackBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.LeftVolumeMouseDown);
			this.leftChannelTrackBar.Scroll += new System.EventHandler(this.leftChannelVolume_Scrolled);
			// 
			// rightChannelTrackBar
			// 
			this.rightChannelTrackBar.Location = new System.Drawing.Point(501, 3);
			this.rightChannelTrackBar.Maximum = 100;
			this.rightChannelTrackBar.Name = "rightChannelTrackBar";
			this.rightChannelTrackBar.Size = new System.Drawing.Size(56, 45);
			this.rightChannelTrackBar.TabIndex = 65;
			this.rightChannelTrackBar.TickFrequency = 20;
			this.toolTip.SetToolTip(this.rightChannelTrackBar, "Right Audio Channel Volume");
			this.rightChannelTrackBar.Value = 100;
			this.rightChannelTrackBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.RightVolumeMouseUp);
			this.rightChannelTrackBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.RightVolumeMouseDown);
			this.rightChannelTrackBar.Scroll += new System.EventHandler(this.rightChannelVolume_Scrolled);
			// 
			// recordPictureBox2
			// 
			this.recordPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordPictureBox2.Image")));
			this.recordPictureBox2.Location = new System.Drawing.Point(80, 80);
			this.recordPictureBox2.Name = "recordPictureBox2";
			this.recordPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.recordPictureBox2.TabIndex = 59;
			this.recordPictureBox2.TabStop = false;
			this.recordPictureBox2.Visible = false;
			// 
			// recordPictureBox1
			// 
			this.recordPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordPictureBox1.Image")));
			this.recordPictureBox1.Location = new System.Drawing.Point(80, 56);
			this.recordPictureBox1.Name = "recordPictureBox1";
			this.recordPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.recordPictureBox1.TabIndex = 58;
			this.recordPictureBox1.TabStop = false;
			this.recordPictureBox1.Visible = false;
			// 
			// pauseButton
			// 
			this.pauseButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("pauseButton.Image")));
			this.pauseButton.Location = new System.Drawing.Point(139, 2);
			this.pauseButton.Name = "pauseButton";
			this.pauseButton.Size = new System.Drawing.Size(32, 32);
			this.pauseButton.TabIndex = 34;
			this.toolTip.SetToolTip(this.pauseButton, "Pause");
			this.pauseButton.Click += new System.EventHandler(this.pauseButton_Click);
			// 
			// stopButton
			// 
			this.stopButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopButton.Image")));
			this.stopButton.Location = new System.Drawing.Point(107, 2);
			this.stopButton.Name = "stopButton";
			this.stopButton.Size = new System.Drawing.Size(32, 32);
			this.stopButton.TabIndex = 33;
			this.toolTip.SetToolTip(this.stopButton, "Stop");
			this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
			// 
			// recordButton
			// 
			this.recordButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordButton.Image")));
			this.recordButton.Location = new System.Drawing.Point(75, 2);
			this.recordButton.Name = "recordButton";
			this.recordButton.Size = new System.Drawing.Size(32, 32);
			this.recordButton.TabIndex = 57;
			this.toolTip.SetToolTip(this.recordButton, "Record");
			this.recordButton.Click += new System.EventHandler(this.recordButton_Click);
			// 
			// nextTrackButton
			// 
			this.nextTrackButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("nextTrackButton.Image")));
			this.nextTrackButton.Location = new System.Drawing.Point(210, 2);
			this.nextTrackButton.Name = "nextTrackButton";
			this.nextTrackButton.Size = new System.Drawing.Size(32, 32);
			this.nextTrackButton.TabIndex = 56;
			this.toolTip.SetToolTip(this.nextTrackButton, "Next Track");
			this.nextTrackButton.Click += new System.EventHandler(this.nextTrackButton_Click);
			// 
			// prevTrackButton
			// 
			this.prevTrackButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("prevTrackButton.Image")));
			this.prevTrackButton.Location = new System.Drawing.Point(178, 2);
			this.prevTrackButton.Name = "prevTrackButton";
			this.prevTrackButton.Size = new System.Drawing.Size(32, 32);
			this.prevTrackButton.TabIndex = 55;
			this.toolTip.SetToolTip(this.prevTrackButton, "Previous Track");
			this.prevTrackButton.Click += new System.EventHandler(this.prevTrackButton_Click);
			// 
			// pictureBox6
			// 
			this.pictureBox6.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox6.Image")));
			this.pictureBox6.Location = new System.Drawing.Point(3, 2);
			this.pictureBox6.Name = "pictureBox6";
			this.pictureBox6.Size = new System.Drawing.Size(32, 32);
			this.pictureBox6.TabIndex = 53;
			this.pictureBox6.TabStop = false;
			// 
			// pausePictureBox2
			// 
			this.pausePictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pausePictureBox2.Image")));
			this.pausePictureBox2.Location = new System.Drawing.Point(144, 80);
			this.pausePictureBox2.Name = "pausePictureBox2";
			this.pausePictureBox2.Size = new System.Drawing.Size(24, 24);
			this.pausePictureBox2.TabIndex = 49;
			this.pausePictureBox2.TabStop = false;
			this.pausePictureBox2.Visible = false;
			// 
			// stopPictureBox2
			// 
			this.stopPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopPictureBox2.Image")));
			this.stopPictureBox2.Location = new System.Drawing.Point(112, 80);
			this.stopPictureBox2.Name = "stopPictureBox2";
			this.stopPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.stopPictureBox2.TabIndex = 48;
			this.stopPictureBox2.TabStop = false;
			this.stopPictureBox2.Visible = false;
			// 
			// playPictureBox2
			// 
			this.playPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("playPictureBox2.Image")));
			this.playPictureBox2.Location = new System.Drawing.Point(48, 80);
			this.playPictureBox2.Name = "playPictureBox2";
			this.playPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.playPictureBox2.TabIndex = 47;
			this.playPictureBox2.TabStop = false;
			this.playPictureBox2.Visible = false;
			// 
			// pausePictureBox1
			// 
			this.pausePictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("pausePictureBox1.Image")));
			this.pausePictureBox1.Location = new System.Drawing.Point(144, 56);
			this.pausePictureBox1.Name = "pausePictureBox1";
			this.pausePictureBox1.Size = new System.Drawing.Size(24, 24);
			this.pausePictureBox1.TabIndex = 46;
			this.pausePictureBox1.TabStop = false;
			this.pausePictureBox1.Visible = false;
			// 
			// stopPictureBox1
			// 
			this.stopPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopPictureBox1.Image")));
			this.stopPictureBox1.Location = new System.Drawing.Point(112, 56);
			this.stopPictureBox1.Name = "stopPictureBox1";
			this.stopPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.stopPictureBox1.TabIndex = 45;
			this.stopPictureBox1.TabStop = false;
			this.stopPictureBox1.Visible = false;
			// 
			// playPictureBox1
			// 
			this.playPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("playPictureBox1.Image")));
			this.playPictureBox1.Location = new System.Drawing.Point(48, 56);
			this.playPictureBox1.Name = "playPictureBox1";
			this.playPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.playPictureBox1.TabIndex = 44;
			this.playPictureBox1.TabStop = false;
			this.playPictureBox1.Visible = false;
			// 
			// pictureBox3
			// 
			this.pictureBox3.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox3.Image")));
			this.pictureBox3.Location = new System.Drawing.Point(290, 4);
			this.pictureBox3.Name = "pictureBox3";
			this.pictureBox3.Size = new System.Drawing.Size(19, 24);
			this.pictureBox3.TabIndex = 40;
			this.pictureBox3.TabStop = false;
			// 
			// pictureBox2
			// 
			this.pictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox2.Image")));
			this.pictureBox2.Location = new System.Drawing.Point(392, 4);
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.Size = new System.Drawing.Size(24, 24);
			this.pictureBox2.TabIndex = 39;
			this.pictureBox2.TabStop = false;
			// 
			// mutePictureBox
			// 
			this.mutePictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("mutePictureBox.Image")));
			this.mutePictureBox.Location = new System.Drawing.Point(256, 56);
			this.mutePictureBox.Name = "mutePictureBox";
			this.mutePictureBox.Size = new System.Drawing.Size(24, 24);
			this.mutePictureBox.TabIndex = 38;
			this.mutePictureBox.TabStop = false;
			this.mutePictureBox.Visible = false;
			// 
			// mutedPictureBox
			// 
			this.mutedPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("mutedPictureBox.Image")));
			this.mutedPictureBox.Location = new System.Drawing.Point(232, 56);
			this.mutedPictureBox.Name = "mutedPictureBox";
			this.mutedPictureBox.Size = new System.Drawing.Size(24, 24);
			this.mutedPictureBox.TabIndex = 37;
			this.mutedPictureBox.TabStop = false;
			this.mutedPictureBox.Visible = false;
			// 
			// muteButton
			// 
			this.muteButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("muteButton.Image")));
			this.muteButton.Location = new System.Drawing.Point(250, 2);
			this.muteButton.Name = "muteButton";
			this.muteButton.Size = new System.Drawing.Size(32, 32);
			this.muteButton.TabIndex = 36;
			this.toolTip.SetToolTip(this.muteButton, "Mute");
			this.muteButton.Click += new System.EventHandler(this.muteButton_Click);
			// 
			// playButton
			// 
			this.playButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("playButton.Image")));
			this.playButton.Location = new System.Drawing.Point(43, 2);
			this.playButton.Name = "playButton";
			this.playButton.Size = new System.Drawing.Size(32, 32);
			this.playButton.TabIndex = 32;
			this.toolTip.SetToolTip(this.playButton, "Play");
			this.playButton.Click += new System.EventHandler(this.playButton_Click);
			// 
			// volumeTrackBar
			// 
			this.volumeTrackBar.Location = new System.Drawing.Point(298, 4);
			this.volumeTrackBar.Maximum = 100;
			this.volumeTrackBar.Name = "volumeTrackBar";
			this.volumeTrackBar.Size = new System.Drawing.Size(102, 45);
			this.volumeTrackBar.TabIndex = 35;
			this.volumeTrackBar.TickFrequency = 10;
			this.toolTip.SetToolTip(this.volumeTrackBar, "Master Audio Volume");
			this.volumeTrackBar.Value = 50;
			this.volumeTrackBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.MasterVolumeMouseUp);
			this.volumeTrackBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.MasterVolumeMouseDown);
			this.volumeTrackBar.Scroll += new System.EventHandler(this.volumeTrackBar_Scroll);
			// 
			// RendererControl
			// 
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel1});
			this.Name = "RendererControl";
			this.Size = new System.Drawing.Size(576, 56);
			this.panel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.leftChannelTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.rightChannelTrackBar)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		private void PlayStateChangedHandlerSink(AVConnection sender, AVConnection.PlayState NewState)
		{
			playButton.Image = playPictureBox2.Image;
			recordButton.Image = recordPictureBox2.Image;
			stopButton.Image = stopPictureBox2.Image;
			pauseButton.Image = pausePictureBox2.Image;

			switch (NewState) 
			{
				case AVConnection.PlayState.PLAYING:
					playButton.Image = playPictureBox1.Image;
					break;
				case AVConnection.PlayState.RECORDING:
					recordButton.Image = recordPictureBox1.Image;
					break;
				case AVConnection.PlayState.SEEKING:
				case AVConnection.PlayState.STOPPED:
					stopButton.Image = stopPictureBox1.Image;					
					break;
				case AVConnection.PlayState.PAUSED:
					pauseButton.Image = pausePictureBox1.Image;					
					break;
			}
		}

		private void MuteStateChangedHandlerSink(AVConnection sender, bool NewMuteStatus)
		{
			if (connection.IsMute == true) 
			{
				muteButton.Image = mutedPictureBox.Image;
			} 
			else 
			{
				muteButton.Image = mutePictureBox.Image;
			}
		}

		private void VolumeChangedHandlerSink(AVConnection sender, UInt16 volume)
		{
			if(sender.Identifier!=connection.Identifier) return;
			foreach(string Channels in sender.SupportedChannels)
			{
				if(AdjustingVolume==false)
				{
					if(Channels=="Master") volumeTrackBar.Value = (int)connection.MasterVolume;
				}
				if(AdjustingLeftVolume==false)
				{
					if(Channels=="LF") leftChannelTrackBar.Value = (int)connection.GetVolume(Channels);
				}
				if(AdjustingRightVolume==false)
				{
					if(Channels=="RF") rightChannelTrackBar.Value = (int)connection.GetVolume(Channels);
				}
			}
			//volumeTrackBar.Value = (int)connection.MasterVolume;
		}

		private void PositionChangedHandlerSink(AVConnection sender, TimeSpan position)
		{
			if (mediaProgressBar.Tag == null && sender.Duration.TotalSeconds != 0) 
			{
				if(position.TotalSeconds<sender.Duration.TotalSeconds)
				{
					mediaProgressBar.Maximum = (int)sender.Duration.TotalSeconds;
					mediaProgressBar.Value = (int)position.TotalSeconds;
				}
			}
		}

		private void playButton_Click(object sender, System.EventArgs e)
		{
			if (connection != null) {connection.Play();}
		}

		private void recordButton_Click(object sender, System.EventArgs e)
		{
			if (connection != null) {connection.Record();}
		}

		private void stopButton_Click(object sender, System.EventArgs e)
		{
			if (connection != null) {connection.Stop();}
		}

		private void pauseButton_Click(object sender, System.EventArgs e)
		{
			if (connection != null) {connection.Pause();}
		}

		private void muteButton_Click(object sender, System.EventArgs e)
		{
			if (muteButton.Image == mutedPictureBox.Image)
			{
				connection.Mute(false);
			}
			else
			{
				connection.Mute(true);
			}
		}

		private void volumeTrackBar_Scroll(object sender, System.EventArgs e)
		{
			connection.MasterVolume = (ushort)volumeTrackBar.Value;
		}

		private void prevTrackButton_Click(object sender, System.EventArgs e)
		{
			connection.PreviousTrack();
		}

		private void nextTrackButton_Click(object sender, System.EventArgs e)
		{
			connection.NextTrack();
		}

		private void mediaProgressBar_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			mediaProgressBar.Tag = "Seek Lock";
			double seekTargetRatio = ((double)e.X) / ((double)mediaProgressBar.Width);
			if (seekTargetRatio > 1) seekTargetRatio = 1;
			if (seekTargetRatio < 0) seekTargetRatio = 0;
			mediaProgressBar.Value = (int)(((double)mediaProgressBar.Maximum) * seekTargetRatio);
		}

		private void mediaProgressBar_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			double seekTargetRatio = (double)e.X / (double)mediaProgressBar.Width;
			if (seekTargetRatio > 1) seekTargetRatio = 1;
			if (seekTargetRatio < 0) seekTargetRatio = 0;
			if (connection.Duration.Ticks != 0) 
			{
				TimeSpan target = new TimeSpan((long)((double)connection.Duration.Ticks * seekTargetRatio));
				connection.SeekPosition(target);
				mediaProgressBar.Maximum = (int)connection.Duration.TotalSeconds;
				if (mediaProgressBar.Maximum != 0) mediaProgressBar.Value = (int)connection.CurrentPosition.TotalSeconds;
			}
			mediaProgressBar.Tag = null;
		}

		private void mediaProgressBar_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right) 
			{
				double seekTargetRatio = (double)e.X / (double)mediaProgressBar.Width;
				if (seekTargetRatio > 1) seekTargetRatio = 1;
				if (seekTargetRatio < 0) seekTargetRatio = 0;
				mediaProgressBar.Value = (int)((double)mediaProgressBar.Maximum * seekTargetRatio);
			}
		}

		private void leftChannelVolume_Scrolled(object sender, System.EventArgs e)
		{
			try
			{
				connection.SetVolume("LF",(ushort)this.leftChannelTrackBar.Value);
			}
			catch(Exception)
			{
			}
		}

		private void rightChannelVolume_Scrolled(object sender, System.EventArgs e)
		{
			try
			{
				connection.SetVolume("RF",(ushort)this.rightChannelTrackBar.Value);
			}
			catch(Exception)
			{
			}
		}

		private void MasterVolumeMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingVolume = true;
		}

		private void MasterVolumeMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingVolume = false;
		}

		private void LeftVolumeMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingLeftVolume = true;
		}

		private void LeftVolumeMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingLeftVolume = false;
		}

		private void RightVolumeMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingRightVolume = true;
		}

		private void RightVolumeMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			AdjustingRightVolume = false;
		}

	}
}
