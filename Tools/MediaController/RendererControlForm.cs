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
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.MediaServer.CP;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnpMediaController
{
	/// <summary>
	/// Summary description for RendererControlForm.
	/// </summary>
	public class RendererControlForm : System.Windows.Forms.Form
	{
		private RendererAudioControlForm rendererAudioControlForm;
		private MainForm parent;
		private AVRenderer renderer;
		private AVConnection connection;
		private long PendingConnection = -1;

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.PictureBox pictureBox6;
		private System.Windows.Forms.Panel panel2;
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
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem menuItem6;
		private System.Windows.Forms.Label rendererNameLabel;
		private System.Windows.Forms.Label rendererInformationLabel;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.MainMenu rendererMenu;
		private System.Windows.Forms.MenuItem stayOnTopMenuItem;
		private System.Windows.Forms.ToolTip toolTip;
		private System.Windows.Forms.Button instanceButton;
		private System.Windows.Forms.Panel metaDataPanel;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label contentUriLabel;
		private System.Windows.Forms.MenuItem playMenuItem;
		private System.Windows.Forms.MenuItem stopMenuItem;
		private System.Windows.Forms.MenuItem pauseMenuItem;
		private System.Windows.Forms.MenuItem muteMenuItem;
		private System.Windows.Forms.ProgressBar mediaProgressBar;
		private System.Windows.Forms.Button prevTrackButton;
		private System.Windows.Forms.Button nextTrackButton;
		private System.Windows.Forms.MenuItem nextTrackMenuItem;
		private System.Windows.Forms.MenuItem prevTrackMenuItem;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label positionLabel;
		private System.Windows.Forms.MenuItem audioControlsMenuItem;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.MenuItem closeInstanceMenuItem;
		private System.Windows.Forms.MenuItem recordMenuItem;
		private System.Windows.Forms.PictureBox recordPictureBox2;
		private System.Windows.Forms.PictureBox recordPictureBox1;
		private System.Windows.Forms.Button recordButton;
		private System.Windows.Forms.MenuItem menuItem7;
		private System.ComponentModel.IContainer components;

		public RendererControlForm(MainForm parent, AVRenderer renderer, AVConnection connection)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			this.parent = parent;
			this.renderer = renderer;
			this.connection = connection;

			renderer.OnCreateConnection += new AVRenderer.ConnectionHandler(RendererCreateConnectionSink);
			renderer.OnRecycledConnection += new AVRenderer.ConnectionHandler(RendererRecycledConnectionSink);
			renderer.OnRemovedConnection +=  new AVRenderer.ConnectionHandler(RendererRemovedConnectionSink);

			if (connection == null && renderer.Connections.Count > 0) 
			{
				connection = (AVConnection)renderer.Connections[0];
				this.connection = connection;
			}

			if (connection != null)
			{
				connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
				connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(CurrentTrackChangedHandlerSink);
				connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				volumeTrackBar.Value = (int)connection.MasterVolume;
				PositionChangedHandlerSink(connection, connection.CurrentPosition);

				muteMenuItem.Checked = connection.IsMute;
				if (connection.IsMute == true) 
				{
					muteButton.Image = mutedPictureBox.Image;
				} 
				else 
				{
					muteButton.Image = mutePictureBox.Image;
				}
			}

			this.Text = "Renderer - " + renderer.FriendlyName;
			rendererNameLabel.Text = renderer.FriendlyName;

			if (connection != null) PlayStateChangedHandlerSink(connection,connection.CurrentState);
			UpdateUserInterface();
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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(RendererControlForm));
			this.panel1 = new System.Windows.Forms.Panel();
			this.instanceButton = new System.Windows.Forms.Button();
			this.rendererInformationLabel = new System.Windows.Forms.Label();
			this.rendererNameLabel = new System.Windows.Forms.Label();
			this.pictureBox6 = new System.Windows.Forms.PictureBox();
			this.panel2 = new System.Windows.Forms.Panel();
			this.recordButton = new System.Windows.Forms.Button();
			this.recordPictureBox2 = new System.Windows.Forms.PictureBox();
			this.recordPictureBox1 = new System.Windows.Forms.PictureBox();
			this.nextTrackButton = new System.Windows.Forms.Button();
			this.prevTrackButton = new System.Windows.Forms.Button();
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
			this.pauseButton = new System.Windows.Forms.Button();
			this.stopButton = new System.Windows.Forms.Button();
			this.playButton = new System.Windows.Forms.Button();
			this.volumeTrackBar = new System.Windows.Forms.TrackBar();
			this.metaDataPanel = new System.Windows.Forms.Panel();
			this.positionLabel = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.contentUriLabel = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.rendererMenu = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.stayOnTopMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem7 = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuItem10 = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.playMenuItem = new System.Windows.Forms.MenuItem();
			this.recordMenuItem = new System.Windows.Forms.MenuItem();
			this.stopMenuItem = new System.Windows.Forms.MenuItem();
			this.pauseMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem6 = new System.Windows.Forms.MenuItem();
			this.nextTrackMenuItem = new System.Windows.Forms.MenuItem();
			this.prevTrackMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.audioControlsMenuItem = new System.Windows.Forms.MenuItem();
			this.muteMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.closeInstanceMenuItem = new System.Windows.Forms.MenuItem();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.mediaProgressBar = new System.Windows.Forms.ProgressBar();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).BeginInit();
			this.metaDataPanel.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.instanceButton,
																				 this.rendererInformationLabel,
																				 this.rendererNameLabel,
																				 this.pictureBox6});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(358, 48);
			this.panel1.TabIndex = 52;
			// 
			// instanceButton
			// 
			this.instanceButton.Anchor = (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right);
			this.instanceButton.Enabled = false;
			this.instanceButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.instanceButton.Location = new System.Drawing.Point(316, 7);
			this.instanceButton.Name = "instanceButton";
			this.instanceButton.Size = new System.Drawing.Size(32, 32);
			this.instanceButton.TabIndex = 8;
			this.instanceButton.Text = "1";
			this.toolTip.SetToolTip(this.instanceButton, "Instance Tuggle Button");
			this.instanceButton.Click += new System.EventHandler(this.instanceButton_Click);
			// 
			// rendererInformationLabel
			// 
			this.rendererInformationLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.rendererInformationLabel.Location = new System.Drawing.Point(40, 24);
			this.rendererInformationLabel.Name = "rendererInformationLabel";
			this.rendererInformationLabel.Size = new System.Drawing.Size(272, 16);
			this.rendererInformationLabel.TabIndex = 54;
			this.rendererInformationLabel.Text = "Renderer Information";
			// 
			// rendererNameLabel
			// 
			this.rendererNameLabel.Location = new System.Drawing.Point(40, 8);
			this.rendererNameLabel.Name = "rendererNameLabel";
			this.rendererNameLabel.Size = new System.Drawing.Size(272, 16);
			this.rendererNameLabel.TabIndex = 53;
			this.rendererNameLabel.Text = "Renderer Name";
			// 
			// pictureBox6
			// 
			this.pictureBox6.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox6.Image")));
			this.pictureBox6.Location = new System.Drawing.Point(4, 6);
			this.pictureBox6.Name = "pictureBox6";
			this.pictureBox6.Size = new System.Drawing.Size(32, 32);
			this.pictureBox6.TabIndex = 52;
			this.pictureBox6.TabStop = false;
			// 
			// panel2
			// 
			this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.recordButton,
																				 this.recordPictureBox2,
																				 this.recordPictureBox1,
																				 this.nextTrackButton,
																				 this.prevTrackButton,
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
																				 this.pauseButton,
																				 this.stopButton,
																				 this.playButton,
																				 this.volumeTrackBar});
			this.panel2.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel2.Location = new System.Drawing.Point(0, 48);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(358, 48);
			this.panel2.TabIndex = 53;
			// 
			// recordButton
			// 
			this.recordButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordButton.Image")));
			this.recordButton.Location = new System.Drawing.Point(40, 8);
			this.recordButton.Name = "recordButton";
			this.recordButton.Size = new System.Drawing.Size(32, 32);
			this.recordButton.TabIndex = 62;
			this.recordButton.Click += new System.EventHandler(this.recordButton_Click);
			// 
			// recordPictureBox2
			// 
			this.recordPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordPictureBox2.Image")));
			this.recordPictureBox2.Location = new System.Drawing.Point(40, 72);
			this.recordPictureBox2.Name = "recordPictureBox2";
			this.recordPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.recordPictureBox2.TabIndex = 61;
			this.recordPictureBox2.TabStop = false;
			this.recordPictureBox2.Visible = false;
			// 
			// recordPictureBox1
			// 
			this.recordPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("recordPictureBox1.Image")));
			this.recordPictureBox1.Location = new System.Drawing.Point(40, 48);
			this.recordPictureBox1.Name = "recordPictureBox1";
			this.recordPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.recordPictureBox1.TabIndex = 60;
			this.recordPictureBox1.TabStop = false;
			this.recordPictureBox1.Visible = false;
			// 
			// nextTrackButton
			// 
			this.nextTrackButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("nextTrackButton.Image")));
			this.nextTrackButton.Location = new System.Drawing.Point(169, 8);
			this.nextTrackButton.Name = "nextTrackButton";
			this.nextTrackButton.Size = new System.Drawing.Size(32, 32);
			this.nextTrackButton.TabIndex = 5;
			this.nextTrackButton.Click += new System.EventHandler(this.nextTrackButton_Click);
			// 
			// prevTrackButton
			// 
			this.prevTrackButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("prevTrackButton.Image")));
			this.prevTrackButton.Location = new System.Drawing.Point(137, 8);
			this.prevTrackButton.Name = "prevTrackButton";
			this.prevTrackButton.Size = new System.Drawing.Size(32, 32);
			this.prevTrackButton.TabIndex = 4;
			this.prevTrackButton.Click += new System.EventHandler(this.prevTrackButton_Click);
			// 
			// pausePictureBox2
			// 
			this.pausePictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pausePictureBox2.Image")));
			this.pausePictureBox2.Location = new System.Drawing.Point(104, 72);
			this.pausePictureBox2.Name = "pausePictureBox2";
			this.pausePictureBox2.Size = new System.Drawing.Size(24, 24);
			this.pausePictureBox2.TabIndex = 49;
			this.pausePictureBox2.TabStop = false;
			this.pausePictureBox2.Visible = false;
			// 
			// stopPictureBox2
			// 
			this.stopPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopPictureBox2.Image")));
			this.stopPictureBox2.Location = new System.Drawing.Point(72, 72);
			this.stopPictureBox2.Name = "stopPictureBox2";
			this.stopPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.stopPictureBox2.TabIndex = 48;
			this.stopPictureBox2.TabStop = false;
			this.stopPictureBox2.Visible = false;
			// 
			// playPictureBox2
			// 
			this.playPictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("playPictureBox2.Image")));
			this.playPictureBox2.Location = new System.Drawing.Point(8, 72);
			this.playPictureBox2.Name = "playPictureBox2";
			this.playPictureBox2.Size = new System.Drawing.Size(24, 24);
			this.playPictureBox2.TabIndex = 47;
			this.playPictureBox2.TabStop = false;
			this.playPictureBox2.Visible = false;
			// 
			// pausePictureBox1
			// 
			this.pausePictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("pausePictureBox1.Image")));
			this.pausePictureBox1.Location = new System.Drawing.Point(104, 48);
			this.pausePictureBox1.Name = "pausePictureBox1";
			this.pausePictureBox1.Size = new System.Drawing.Size(24, 24);
			this.pausePictureBox1.TabIndex = 46;
			this.pausePictureBox1.TabStop = false;
			this.pausePictureBox1.Visible = false;
			// 
			// stopPictureBox1
			// 
			this.stopPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopPictureBox1.Image")));
			this.stopPictureBox1.Location = new System.Drawing.Point(72, 48);
			this.stopPictureBox1.Name = "stopPictureBox1";
			this.stopPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.stopPictureBox1.TabIndex = 45;
			this.stopPictureBox1.TabStop = false;
			this.stopPictureBox1.Visible = false;
			// 
			// playPictureBox1
			// 
			this.playPictureBox1.Image = ((System.Drawing.Bitmap)(resources.GetObject("playPictureBox1.Image")));
			this.playPictureBox1.Location = new System.Drawing.Point(8, 48);
			this.playPictureBox1.Name = "playPictureBox1";
			this.playPictureBox1.Size = new System.Drawing.Size(24, 24);
			this.playPictureBox1.TabIndex = 44;
			this.playPictureBox1.TabStop = false;
			this.playPictureBox1.Visible = false;
			// 
			// pictureBox3
			// 
			this.pictureBox3.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox3.Image")));
			this.pictureBox3.Location = new System.Drawing.Point(237, 10);
			this.pictureBox3.Name = "pictureBox3";
			this.pictureBox3.Size = new System.Drawing.Size(19, 24);
			this.pictureBox3.TabIndex = 40;
			this.pictureBox3.TabStop = false;
			// 
			// pictureBox2
			// 
			this.pictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox2.Image")));
			this.pictureBox2.Location = new System.Drawing.Point(329, 10);
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.Size = new System.Drawing.Size(24, 24);
			this.pictureBox2.TabIndex = 39;
			this.pictureBox2.TabStop = false;
			// 
			// mutePictureBox
			// 
			this.mutePictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("mutePictureBox.Image")));
			this.mutePictureBox.Location = new System.Drawing.Point(291, 53);
			this.mutePictureBox.Name = "mutePictureBox";
			this.mutePictureBox.Size = new System.Drawing.Size(24, 24);
			this.mutePictureBox.TabIndex = 38;
			this.mutePictureBox.TabStop = false;
			this.mutePictureBox.Visible = false;
			// 
			// mutedPictureBox
			// 
			this.mutedPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("mutedPictureBox.Image")));
			this.mutedPictureBox.Location = new System.Drawing.Point(259, 53);
			this.mutedPictureBox.Name = "mutedPictureBox";
			this.mutedPictureBox.Size = new System.Drawing.Size(24, 24);
			this.mutedPictureBox.TabIndex = 37;
			this.mutedPictureBox.TabStop = false;
			this.mutedPictureBox.Visible = false;
			// 
			// muteButton
			// 
			this.muteButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("muteButton.Image")));
			this.muteButton.Location = new System.Drawing.Point(205, 8);
			this.muteButton.Name = "muteButton";
			this.muteButton.Size = new System.Drawing.Size(32, 32);
			this.muteButton.TabIndex = 6;
			this.muteButton.Click += new System.EventHandler(this.muteButton_Click);
			// 
			// pauseButton
			// 
			this.pauseButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("pauseButton.Image")));
			this.pauseButton.Location = new System.Drawing.Point(103, 8);
			this.pauseButton.Name = "pauseButton";
			this.pauseButton.Size = new System.Drawing.Size(32, 32);
			this.pauseButton.TabIndex = 3;
			this.pauseButton.Click += new System.EventHandler(this.pauseButton_Click);
			// 
			// stopButton
			// 
			this.stopButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("stopButton.Image")));
			this.stopButton.Location = new System.Drawing.Point(71, 8);
			this.stopButton.Name = "stopButton";
			this.stopButton.Size = new System.Drawing.Size(32, 32);
			this.stopButton.TabIndex = 2;
			this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
			// 
			// playButton
			// 
			this.playButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("playButton.Image")));
			this.playButton.Location = new System.Drawing.Point(8, 8);
			this.playButton.Name = "playButton";
			this.playButton.Size = new System.Drawing.Size(32, 32);
			this.playButton.TabIndex = 1;
			this.playButton.Click += new System.EventHandler(this.playButton_Click);
			// 
			// volumeTrackBar
			// 
			this.volumeTrackBar.Location = new System.Drawing.Point(249, 10);
			this.volumeTrackBar.Maximum = 100;
			this.volumeTrackBar.Name = "volumeTrackBar";
			this.volumeTrackBar.Size = new System.Drawing.Size(88, 45);
			this.volumeTrackBar.TabIndex = 7;
			this.volumeTrackBar.TickFrequency = 10;
			this.volumeTrackBar.Value = 50;
			this.volumeTrackBar.Scroll += new System.EventHandler(this.volumeTrackBar_Scroll);
			// 
			// metaDataPanel
			// 
			this.metaDataPanel.AllowDrop = true;
			this.metaDataPanel.BackColor = System.Drawing.Color.Black;
			this.metaDataPanel.Controls.AddRange(new System.Windows.Forms.Control[] {
																						this.positionLabel,
																						this.label2,
																						this.contentUriLabel,
																						this.label1});
			this.metaDataPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.metaDataPanel.Location = new System.Drawing.Point(0, 96);
			this.metaDataPanel.Name = "metaDataPanel";
			this.metaDataPanel.Size = new System.Drawing.Size(358, 51);
			this.metaDataPanel.TabIndex = 54;
			this.metaDataPanel.DragEnter += new System.Windows.Forms.DragEventHandler(this.metaDataPanel_DragEnter);
			this.metaDataPanel.DragDrop += new System.Windows.Forms.DragEventHandler(this.metaDataPanel_DragDrop);
			// 
			// positionLabel
			// 
			this.positionLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.positionLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.positionLabel.ForeColor = System.Drawing.Color.Gold;
			this.positionLabel.Location = new System.Drawing.Point(64, 24);
			this.positionLabel.Name = "positionLabel";
			this.positionLabel.Size = new System.Drawing.Size(288, 16);
			this.positionLabel.TabIndex = 5;
			this.positionLabel.Text = "(unknown)";
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.White;
			this.label2.Location = new System.Drawing.Point(8, 24);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(56, 16);
			this.label2.TabIndex = 4;
			this.label2.Text = "Position:";
			// 
			// contentUriLabel
			// 
			this.contentUriLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.contentUriLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.contentUriLabel.ForeColor = System.Drawing.Color.Gold;
			this.contentUriLabel.Location = new System.Drawing.Point(64, 8);
			this.contentUriLabel.Name = "contentUriLabel";
			this.contentUriLabel.Size = new System.Drawing.Size(288, 16);
			this.contentUriLabel.TabIndex = 3;
			this.contentUriLabel.Text = "(none)";
			// 
			// label1
			// 
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.White;
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(56, 16);
			this.label1.TabIndex = 0;
			this.label1.Text = "Media:";
			// 
			// rendererMenu
			// 
			this.rendererMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																						 this.menuItem1,
																						 this.menuItem2});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.stayOnTopMenuItem,
																					  this.menuItem7,
																					  this.menuItem4,
																					  this.menuItem10});
			this.menuItem1.Text = "&File";
			// 
			// stayOnTopMenuItem
			// 
			this.stayOnTopMenuItem.Checked = true;
			this.stayOnTopMenuItem.Index = 0;
			this.stayOnTopMenuItem.Text = "&Stay on Top";
			this.stayOnTopMenuItem.Click += new System.EventHandler(this.stayOnTopMenuItem_Click);
			// 
			// menuItem7
			// 
			this.menuItem7.Index = 1;
			this.menuItem7.Text = "Show Packet Capture";
			this.menuItem7.Click += new System.EventHandler(this.menuItem7_Click);
			// 
			// menuItem4
			// 
			this.menuItem4.Index = 2;
			this.menuItem4.Text = "-";
			// 
			// menuItem10
			// 
			this.menuItem10.Index = 3;
			this.menuItem10.Text = "&Close";
			this.menuItem10.Click += new System.EventHandler(this.menuItem10_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.playMenuItem,
																					  this.recordMenuItem,
																					  this.stopMenuItem,
																					  this.pauseMenuItem,
																					  this.menuItem6,
																					  this.nextTrackMenuItem,
																					  this.prevTrackMenuItem,
																					  this.menuItem5,
																					  this.audioControlsMenuItem,
																					  this.muteMenuItem,
																					  this.menuItem3,
																					  this.closeInstanceMenuItem});
			this.menuItem2.Text = "&Control";
			this.menuItem2.Popup += new System.EventHandler(this.menuItem2_Popup);
			// 
			// playMenuItem
			// 
			this.playMenuItem.Index = 0;
			this.playMenuItem.Text = "&Play";
			this.playMenuItem.Click += new System.EventHandler(this.playButton_Click);
			// 
			// recordMenuItem
			// 
			this.recordMenuItem.Index = 1;
			this.recordMenuItem.Text = "Record";
			// 
			// stopMenuItem
			// 
			this.stopMenuItem.Checked = true;
			this.stopMenuItem.Index = 2;
			this.stopMenuItem.Text = "&Stop";
			this.stopMenuItem.Click += new System.EventHandler(this.stopButton_Click);
			// 
			// pauseMenuItem
			// 
			this.pauseMenuItem.Index = 3;
			this.pauseMenuItem.Text = "P&ause";
			this.pauseMenuItem.Click += new System.EventHandler(this.pauseButton_Click);
			// 
			// menuItem6
			// 
			this.menuItem6.Index = 4;
			this.menuItem6.Text = "-";
			// 
			// nextTrackMenuItem
			// 
			this.nextTrackMenuItem.Index = 5;
			this.nextTrackMenuItem.Text = "&Next Track";
			this.nextTrackMenuItem.Click += new System.EventHandler(this.nextTrackButton_Click);
			// 
			// prevTrackMenuItem
			// 
			this.prevTrackMenuItem.Index = 6;
			this.prevTrackMenuItem.Text = "P&revious Track";
			this.prevTrackMenuItem.Click += new System.EventHandler(this.prevTrackButton_Click);
			// 
			// menuItem5
			// 
			this.menuItem5.Index = 7;
			this.menuItem5.Text = "-";
			// 
			// audioControlsMenuItem
			// 
			this.audioControlsMenuItem.Index = 8;
			this.audioControlsMenuItem.Text = "Audio Controls...";
			this.audioControlsMenuItem.Click += new System.EventHandler(this.audioControlsMenuItem_Click);
			// 
			// muteMenuItem
			// 
			this.muteMenuItem.Index = 9;
			this.muteMenuItem.Text = "&Mute";
			this.muteMenuItem.Click += new System.EventHandler(this.muteButton_Click);
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 10;
			this.menuItem3.Text = "-";
			// 
			// closeInstanceMenuItem
			// 
			this.closeInstanceMenuItem.Index = 11;
			this.closeInstanceMenuItem.Text = "&Close Instance";
			this.closeInstanceMenuItem.Click += new System.EventHandler(this.closeInstanceMenuItem_Click);
			// 
			// mediaProgressBar
			// 
			this.mediaProgressBar.Cursor = System.Windows.Forms.Cursors.Hand;
			this.mediaProgressBar.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.mediaProgressBar.Location = new System.Drawing.Point(0, 147);
			this.mediaProgressBar.Name = "mediaProgressBar";
			this.mediaProgressBar.Size = new System.Drawing.Size(358, 12);
			this.mediaProgressBar.TabIndex = 56;
			this.mediaProgressBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseUp);
			this.mediaProgressBar.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseMove);
			this.mediaProgressBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mediaProgressBar_MouseDown);
			// 
			// RendererControlForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(358, 159);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.metaDataPanel,
																		  this.mediaProgressBar,
																		  this.panel2,
																		  this.panel1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Menu = this.rendererMenu;
			this.Name = "RendererControlForm";
			this.Text = "Renderer";
			this.TopMost = true;
			this.DragDrop += new System.Windows.Forms.DragEventHandler(this.metaDataPanel_DragDrop);
			this.Closed += new System.EventHandler(this.RendererControlForm_Closed);
			this.DragEnter += new System.Windows.Forms.DragEventHandler(this.metaDataPanel_DragEnter);
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).EndInit();
			this.metaDataPanel.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void stayOnTopMenuItem_Click(object sender, System.EventArgs e)
		{
			stayOnTopMenuItem.Checked = !stayOnTopMenuItem.Checked;
			this.TopMost = stayOnTopMenuItem.Checked;
		}

		protected void RendererRecycledConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			if (renderer != sender) MessageBox.Show(this,"Incorrect renderer event");
			if (r != connection && (connection == null || (long)Handle == PendingConnection))
			{
				if (connection != null)
				{
					connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
					connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
					connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
					connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
					connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				}
				connection = r;
				connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
				connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
			}
			UpdateUserInterface();
		}

		protected void RendererRemovedConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			UpdateUserInterface();
		}

		protected void RendererCreateConnectionSink(AVRenderer sender, AVConnection r, object Handle)
		{
			if (renderer != sender) MessageBox.Show(this,"Incorrect renderer event");
			if (connection == null || (long)Handle == PendingConnection) 
			{
				if (connection != null) 
				{
					connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
					connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
					connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
					connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
					connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				}
				connection = r;
				connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
				connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				UpdateUserInterface();
			}
		}

		private void OnMediaResourceChangedHandlerSink(AVConnection sender, IMediaResource target)
		{
			UpdateUserInterface();		
		}

		private void PlayStateChangedHandlerSink(AVConnection sender, AVConnection.PlayState NewState)
		{
			if (sender != connection) return;

			playButton.Image = playPictureBox2.Image;
			recordButton.Image = recordPictureBox2.Image;
			stopButton.Image = stopPictureBox2.Image;
			pauseButton.Image = pausePictureBox2.Image;

			switch (NewState) 
			{
				case AVConnection.PlayState.PLAYING:
					playButton.Image = playPictureBox1.Image;
					playMenuItem.Checked = true;
					recordMenuItem.Checked = false;
					stopMenuItem.Checked = false;
					pauseMenuItem.Checked = false;
					break;
				case AVConnection.PlayState.RECORDING:
					recordButton.Image = recordPictureBox1.Image;				
					playMenuItem.Checked = false;
					recordMenuItem.Checked = true;
					stopMenuItem.Checked = false;
					pauseMenuItem.Checked = false;
					break;
				case AVConnection.PlayState.SEEKING:
					stopButton.Image = stopPictureBox1.Image;					
					playMenuItem.Checked = false;
					recordMenuItem.Checked = false;
					stopMenuItem.Checked = true;
					pauseMenuItem.Checked = false;
					break;
				case AVConnection.PlayState.STOPPED:
					stopButton.Image = stopPictureBox1.Image;					
					playMenuItem.Checked = false;
					recordMenuItem.Checked = false;
					stopMenuItem.Checked = true;
					pauseMenuItem.Checked = false;
					break;
				case AVConnection.PlayState.PAUSED:
					pauseButton.Image = pausePictureBox1.Image;					
					playMenuItem.Checked = false;
					recordMenuItem.Checked = false;
					stopMenuItem.Checked = false;
					pauseMenuItem.Checked = true;
					break;
			}

		}

		private void MuteStateChangedHandlerSink(AVConnection sender, bool NewMuteStatus)
		{
			if (sender != connection) return;

			muteMenuItem.Checked = connection.IsMute;
			if (connection.IsMute == true) 
			{
				muteButton.Image = mutedPictureBox.Image;
			} 
			else 
			{
				muteButton.Image = mutePictureBox.Image;
			}
		}

		private void VolumeChangedHandlerSink(AVConnection sender, UInt16 Volume)
		{
			if (connection != sender) return;

			volumeTrackBar.Value = (int)connection.MasterVolume;
		}

		private void CurrentTrackChangedHandlerSink(AVConnection sender, uint track) 
		{
			if (sender != connection) return;
			if (mediaProgressBar.Tag != null) return;
			UpdateUserInterface();
		}

		private void PositionChangedHandlerSink(AVConnection sender, TimeSpan position)
		{
			if (sender != connection) return;
			if (mediaProgressBar.Tag != null) return;
			UpdateUserInterface();
		}

		private void RemovedConnectionHandlerSink(AVConnection sender)
		{
			if (connection == sender) 
			{
				// Unplug the connection
				connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
				connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				connection = null;

				if (renderer.Connections.Count > 0) 
				{
					connection = (AVConnection)renderer.Connections[0];
					connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
					connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
					connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
					connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
					connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
					connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				}
			}
			UpdateUserInterface();
		}

		private void UpdateUserInterface() 
		{
			if (renderer.Connections.Count == 0) 
			{
				rendererInformationLabel.Text = "No connections";
			} 
			else if (renderer.Connections.Count == 1) 
			{
				rendererInformationLabel.Text = "1 connection";
			} 
			else
			{
				if (connection == null)
				{
					rendererInformationLabel.Text = renderer.Connections.Count.ToString() + " connections, none selected";
				} 
				else 
				{
					rendererInformationLabel.Text = renderer.Connections.Count.ToString() + " connections, instance #" + connection.ConnectionID + " selected";
				}
			}

			instanceButton.Enabled = (renderer.Connections.Count > 1);
			if (connection != null)
			{
				instanceButton.Text = (renderer.Connections.IndexOf(connection)+1).ToString();
				if (connection.MediaResource != null) 
				{
					contentUriLabel.Text = connection.MediaResource.ContentUri;
				} 
				else 
				{
					contentUriLabel.Text = "(none)";
				}

				if (mediaProgressBar.Tag == null) 
				{
					if (connection.SupportsCurrentPosition == true) 
					{
						mediaProgressBar.Maximum = (int)connection.Duration.TotalSeconds;
						if((int)connection.CurrentPosition.TotalSeconds <= mediaProgressBar.Maximum)
						{
							if (mediaProgressBar.Maximum != 0) mediaProgressBar.Value = (int)connection.CurrentPosition.TotalSeconds;

							string tf = string.Format("{0:00}",connection.CurrentPosition.Hours) + ":" + string.Format("{0:00}",connection.CurrentPosition.Minutes) + ":" + string.Format("{0:00}",connection.CurrentPosition.Seconds);
							string d = string.Format("{0:00}",connection.Duration.Hours) + ":" + string.Format("{0:00}",connection.Duration.Minutes) + ":" + string.Format("{0:00}",connection.Duration.Seconds);
							positionLabel.Text = "Track " + connection.CurrentTrack.ToString() + " of " + connection.NumberOfTracks.ToString() + ", " + tf + " / " + d;
						} 
					}
					else
					{
						string d = string.Format("{0:00}",connection.Duration.Hours) + ":" + string.Format("{0:00}",connection.Duration.Minutes) + ":" + string.Format("{0:00}",connection.Duration.Seconds);
						positionLabel.Text = "Track " + connection.CurrentTrack.ToString() + " of " + connection.NumberOfTracks.ToString() + ", Duration " + d;
					}
				}

				muteMenuItem.Checked = connection.IsMute;
				if (connection.IsMute == true) 
				{
					muteButton.Image = mutedPictureBox.Image;
				} 
				else 
				{
					muteButton.Image = mutePictureBox.Image;
				}
			} 
			else 
			{
				instanceButton.Text = "-";
				contentUriLabel.Text = "(none)";
				positionLabel.Text = "00:00:00";
			}

			if(connection!=null)
			{
				if(connection.SupportsRecord)
				{
					recordButton.Enabled = true;
				}
				else
				{
					recordButton.Enabled = false;
				}
				if(connection.SupportsPause)
				{
					pauseButton.Enabled = true;
				}
				else
				{
					pauseButton.Enabled = false;
				}

				// Reset Event Values
				this.PlayStateChangedHandlerSink(connection,connection.CurrentState);
				foreach(string channel in connection.SupportedChannels)
				{
					if(channel=="Master") volumeTrackBar.Value = (int) connection.MasterVolume;
				}
			}
		}

		private void metaDataPanel_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(typeof(MediaItem[])) == true)
			{
				MediaItem[] items = (MediaItem[])e.Data.GetData(typeof(MediaItem[]));
				if (items.Length > 0) SetupConnection(items);
			}

			if (e.Data.GetDataPresent(typeof(string)) == true)
			{
				string stritems = (string)e.Data.GetData(typeof(string));

				ArrayList items = MediaBuilder.BuildMediaBranches(stritems,typeof(MediaItem),typeof(MediaContainer));
				if (items.Count > 0) SetupConnection((MediaItem[])items.ToArray(typeof(MediaItem)));
			}
		}

		public void SetupConnection(MediaItem[] items)
		{
			if (items == null || items.Length == 0) return;
			PendingConnection = DateTime.Now.Ticks;
			renderer.CreateConnection(items, PendingConnection);
		}

		public void SetupConnection(ICpContainer container)
		{
			if (container == null) return;
			PendingConnection = DateTime.Now.Ticks;
			renderer.CreateConnection(container, PendingConnection);
		}

		private void metaDataPanel_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if(e.Data.GetDataPresent(typeof(CpMediaItem[])))
			{
				CpMediaItem[] items = (CpMediaItem[])e.Data.GetData(typeof(CpMediaItem[]));
				if (items != null && items.Length > 0) 
				{
					// TODO: Check to see if one of the resources is compatible with the renderer
					e.Effect = DragDropEffects.Copy;
				}
			}

			if (e.Data.GetDataPresent(typeof(string)) == true)
			{
				string items = (string)e.Data.GetData(typeof(string));

				if (items != null && items.StartsWith("<DIDL-Lite "))
				{
					e.Effect = DragDropEffects.Copy;
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
			if (connection == null) return;
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
			if (connection == null) return;
			connection.MasterVolume = (ushort)volumeTrackBar.Value;
		}

		private void RendererControlForm_Closed(object sender, System.EventArgs e)
		{
			if (connection != null) 
			{
				connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
				connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
				connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
				connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
				connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
				connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);
				connection = null;
			}

			parent.RendererControlFormClose(this,renderer);
		}

		private void menuItem10_Click(object sender, System.EventArgs e)
		{
			Close();
		}

		private void prevTrackButton_Click(object sender, System.EventArgs e)
		{
			connection.PreviousTrack();
		}

		private void nextTrackButton_Click(object sender, System.EventArgs e)
		{
			connection.NextTrack();		
		}

		private void audioControlsMenuItem_Click(object sender, System.EventArgs e)
		{
			rendererAudioControlForm = new RendererAudioControlForm();
			rendererAudioControlForm.Connection = connection;
			rendererAudioControlForm.ShowDialog(this);
			rendererAudioControlForm.Dispose();
			rendererAudioControlForm = null;
		}

		private void instanceButton_Click(object sender, System.EventArgs e)
		{
			int i = renderer.Connections.IndexOf(connection)+1;
			if (i >= renderer.Connections.Count) i = 0;
			AVConnection nextconnection = (AVConnection)renderer.Connections[i];

			if (nextconnection == null || nextconnection == connection) return;

			// Unplug the connection
			connection.OnMediaResourceChanged -= new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
			connection.OnMute -= new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
			connection.OnPlayStateChanged -= new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
			connection.OnPositionChanged -= new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
			connection.OnRemoved -= new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
			connection.OnVolume -= new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);

			// Setup the new one
			connection = nextconnection;
			connection.OnMediaResourceChanged += new AVConnection.MediaResourceChangedHandler(OnMediaResourceChangedHandlerSink);
			connection.OnMute += new AVConnection.MuteStateChangedHandler(MuteStateChangedHandlerSink);
			connection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateChangedHandlerSink);
			connection.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangedHandlerSink);
			connection.OnRemoved += new AVConnection.RendererHandler(RemovedConnectionHandlerSink);
			connection.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangedHandlerSink);

			UpdateUserInterface();
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
			UpdateUserInterface();
		}

		private void mediaProgressBar_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right) 
			{
				double seekTargetRatio = (double)e.X / (double)mediaProgressBar.Width;
				if (seekTargetRatio > 1) seekTargetRatio = 1;
				if (seekTargetRatio < 0) seekTargetRatio = 0;
				mediaProgressBar.Value = (int)((double)mediaProgressBar.Maximum * seekTargetRatio);

				TimeSpan target = new TimeSpan((long)((double)connection.Duration.Ticks * seekTargetRatio));
				string tf = string.Format("{0:00}",target.Hours) + ":" + string.Format("{0:00}",target.Minutes) + ":" + string.Format("{0:00}",target.Seconds);
				string d = string.Format("{0:00}",connection.Duration.Hours) + ":" + string.Format("{0:00}",connection.Duration.Minutes) + ":" + string.Format("{0:00}",connection.Duration.Seconds);
				positionLabel.Text = "Track " + connection.CurrentTrack.ToString() + " of " + connection.NumberOfTracks.ToString() + ", " + tf + " / " + d;
			}
		}

		private void closeInstanceMenuItem_Click(object sender, System.EventArgs e)
		{
			if (connection != null) 
			{
				connection.Close();
			}
		}

		private void menuItem2_Popup(object sender, System.EventArgs e)
		{
			closeInstanceMenuItem.Enabled = (connection != null && connection.IsCloseSupported == true);
		}

		private void menuItem7_Click(object sender, System.EventArgs e)
		{
			/*
			if (rendererDebugForm == null) rendererDebugForm = new RendererDebugForm(renderer);
			rendererDebugForm.Show();
			rendererDebugForm.Activate();
			*/
		}

	}
}
