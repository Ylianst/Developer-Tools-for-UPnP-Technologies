using System;
using System.IO;
using System.Data;
using System.Drawing;
using System.Threading;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using IntelMediaServerServiceLib;

namespace IntelMediaServerConfig
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class MainForm : System.Windows.Forms.Form
	{
		private IntelMediaServerConfigClass MediaServer;
		private DirectoryInfo DirInfoMyMusic;
		private DirectoryInfo DirInfoMyPictures;
		private DirectoryInfo DirInfoMyVideos;
		private DirectoryInfo DirInfoMyDocuments;

		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.Button closeButton;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.LinkLabel linkLabel1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.LinkLabel linkLabel2;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.ImageList folderImageList;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.PictureBox pictureBox3;
		private System.Windows.Forms.PictureBox pictureBox4;
		private System.Windows.Forms.Label label11;
		private System.Windows.Forms.Label label12;
		private System.Windows.Forms.Label label13;
		private System.Windows.Forms.TabPage mediaSharingTabPage;
		private System.Windows.Forms.TabPage accessControlTabPage;
		private System.Windows.Forms.TabPage aboutTabPage;
		private System.Windows.Forms.TabPage mediaTransferTabPage;
		private System.Windows.Forms.CheckBox shareMyVideosCheckBox;
		private System.Windows.Forms.CheckBox shareMyPicturesCheckBox;
		private System.Windows.Forms.CheckBox shareMyMusicCheckBox;
		private System.Windows.Forms.Button removeSharedFolderButton;
		private System.Windows.Forms.Button addSharedFolderButton;
		private System.Windows.Forms.ListView sharedFoldersListView;
		private System.Windows.Forms.RadioButton allowEveryoneRadioButton;
		private System.Windows.Forms.RadioButton allowLocalNetworkRadioButton;
		private System.Windows.Forms.RadioButton allowDeviceListRadioButton;
		private System.Windows.Forms.ListView allowedDevicesListView;
		private System.Windows.Forms.Button removeAllowedDeviceButton;
		private System.Windows.Forms.Button addAllowedDeviceButton;
		private System.Windows.Forms.Label allowedDeviceListLabel;
		private System.Windows.Forms.Panel mediaTransfersPanel;
		private System.Windows.Forms.Label mediaTransfersLabel;
		private System.Windows.Forms.Label numberOfContentRequestsLabel;
		private System.Windows.Forms.Label numberOfContentTransfersLabel;
		private System.Windows.Forms.Label numberOfDeniedRequestsLabel;
		private System.Windows.Forms.PictureBox upnpForumPictureBox;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.MenuItem openFolderMenuItem;
		private System.Windows.Forms.MenuItem menuItem2;
		private System.Windows.Forms.MenuItem removeFolderMenuItem;
		private System.Windows.Forms.ContextMenu sharedFoldersContextMenu;
		private System.ComponentModel.IContainer components;

		public MainForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			MediaServer = new IntelMediaServerConfigClass();
			MediaServer.ServerLogEvent += new _IIntelMediaServerConfigEvents_ServerLogEventEventHandler(MediaServerEventSink);
			//eventLogTextBox.Text = MediaServer.GetVersionInfo() + "\r\n";

			DirInfoMyMusic = new DirectoryInfo(System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal) + "\\My Music");
			DirInfoMyPictures = new DirectoryInfo(System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal) + "\\My Pictures");
			DirInfoMyVideos = new DirectoryInfo(System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal) + "\\My Videos");
			DirInfoMyDocuments = new DirectoryInfo(System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal));

			MediaTransferControl x;
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
			x = new MediaTransferControl();
			x.Dock = DockStyle.Top;
			mediaTransfersPanel.Controls.Add(x);
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );

			MediaServer = null;
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MainForm));
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.mediaSharingTabPage = new System.Windows.Forms.TabPage();
			this.removeSharedFolderButton = new System.Windows.Forms.Button();
			this.addSharedFolderButton = new System.Windows.Forms.Button();
			this.label6 = new System.Windows.Forms.Label();
			this.shareMyVideosCheckBox = new System.Windows.Forms.CheckBox();
			this.shareMyPicturesCheckBox = new System.Windows.Forms.CheckBox();
			this.shareMyMusicCheckBox = new System.Windows.Forms.CheckBox();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.sharedFoldersListView = new System.Windows.Forms.ListView();
			this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
			this.folderImageList = new System.Windows.Forms.ImageList(this.components);
			this.label5 = new System.Windows.Forms.Label();
			this.accessControlTabPage = new System.Windows.Forms.TabPage();
			this.removeAllowedDeviceButton = new System.Windows.Forms.Button();
			this.addAllowedDeviceButton = new System.Windows.Forms.Button();
			this.allowedDeviceListLabel = new System.Windows.Forms.Label();
			this.allowedDevicesListView = new System.Windows.Forms.ListView();
			this.allowDeviceListRadioButton = new System.Windows.Forms.RadioButton();
			this.allowLocalNetworkRadioButton = new System.Windows.Forms.RadioButton();
			this.allowEveryoneRadioButton = new System.Windows.Forms.RadioButton();
			this.pictureBox3 = new System.Windows.Forms.PictureBox();
			this.label7 = new System.Windows.Forms.Label();
			this.mediaTransferTabPage = new System.Windows.Forms.TabPage();
			this.numberOfDeniedRequestsLabel = new System.Windows.Forms.Label();
			this.numberOfContentTransfersLabel = new System.Windows.Forms.Label();
			this.numberOfContentRequestsLabel = new System.Windows.Forms.Label();
			this.mediaTransfersLabel = new System.Windows.Forms.Label();
			this.label13 = new System.Windows.Forms.Label();
			this.label12 = new System.Windows.Forms.Label();
			this.label11 = new System.Windows.Forms.Label();
			this.mediaTransfersPanel = new System.Windows.Forms.Panel();
			this.pictureBox4 = new System.Windows.Forms.PictureBox();
			this.label8 = new System.Windows.Forms.Label();
			this.aboutTabPage = new System.Windows.Forms.TabPage();
			this.label4 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.linkLabel2 = new System.Windows.Forms.LinkLabel();
			this.label2 = new System.Windows.Forms.Label();
			this.linkLabel1 = new System.Windows.Forms.LinkLabel();
			this.label1 = new System.Windows.Forms.Label();
			this.upnpForumPictureBox = new System.Windows.Forms.PictureBox();
			this.closeButton = new System.Windows.Forms.Button();
			this.sharedFoldersContextMenu = new System.Windows.Forms.ContextMenu();
			this.openFolderMenuItem = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.removeFolderMenuItem = new System.Windows.Forms.MenuItem();
			this.tabControl1.SuspendLayout();
			this.mediaSharingTabPage.SuspendLayout();
			this.accessControlTabPage.SuspendLayout();
			this.mediaTransferTabPage.SuspendLayout();
			this.aboutTabPage.SuspendLayout();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.tabControl1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					  this.mediaSharingTabPage,
																					  this.accessControlTabPage,
																					  this.mediaTransferTabPage,
																					  this.aboutTabPage});
			this.tabControl1.Location = new System.Drawing.Point(80, 72);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(360, 344);
			this.tabControl1.TabIndex = 1;
			// 
			// mediaSharingTabPage
			// 
			this.mediaSharingTabPage.Controls.AddRange(new System.Windows.Forms.Control[] {
																							  this.removeSharedFolderButton,
																							  this.addSharedFolderButton,
																							  this.label6,
																							  this.shareMyVideosCheckBox,
																							  this.shareMyPicturesCheckBox,
																							  this.shareMyMusicCheckBox,
																							  this.pictureBox2,
																							  this.sharedFoldersListView,
																							  this.label5});
			this.mediaSharingTabPage.Location = new System.Drawing.Point(4, 22);
			this.mediaSharingTabPage.Name = "mediaSharingTabPage";
			this.mediaSharingTabPage.Size = new System.Drawing.Size(352, 318);
			this.mediaSharingTabPage.TabIndex = 1;
			this.mediaSharingTabPage.Text = "Media Sharing";
			// 
			// removeSharedFolderButton
			// 
			this.removeSharedFolderButton.Enabled = false;
			this.removeSharedFolderButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("removeSharedFolderButton.Image")));
			this.removeSharedFolderButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.removeSharedFolderButton.Location = new System.Drawing.Point(8, 272);
			this.removeSharedFolderButton.Name = "removeSharedFolderButton";
			this.removeSharedFolderButton.Size = new System.Drawing.Size(168, 32);
			this.removeSharedFolderButton.TabIndex = 8;
			this.removeSharedFolderButton.Text = "Remove Folder";
			this.removeSharedFolderButton.Click += new System.EventHandler(this.removeSharedFolderButton_Click);
			// 
			// addSharedFolderButton
			// 
			this.addSharedFolderButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("addSharedFolderButton.Image")));
			this.addSharedFolderButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.addSharedFolderButton.Location = new System.Drawing.Point(176, 272);
			this.addSharedFolderButton.Name = "addSharedFolderButton";
			this.addSharedFolderButton.Size = new System.Drawing.Size(168, 32);
			this.addSharedFolderButton.TabIndex = 7;
			this.addSharedFolderButton.Text = "Add Folder";
			this.addSharedFolderButton.Click += new System.EventHandler(this.addSharedFolderButton_Click);
			// 
			// label6
			// 
			this.label6.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label6.Location = new System.Drawing.Point(8, 120);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(336, 16);
			this.label6.TabIndex = 6;
			this.label6.Text = "Additional Shared Folders";
			// 
			// shareMyVideosCheckBox
			// 
			this.shareMyVideosCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.shareMyVideosCheckBox.Location = new System.Drawing.Point(56, 88);
			this.shareMyVideosCheckBox.Name = "shareMyVideosCheckBox";
			this.shareMyVideosCheckBox.Size = new System.Drawing.Size(288, 16);
			this.shareMyVideosCheckBox.TabIndex = 5;
			this.shareMyVideosCheckBox.Text = "Share My Videos";
			this.shareMyVideosCheckBox.CheckedChanged += new System.EventHandler(this.shareMyVideosCheckBox_CheckedChanged);
			// 
			// shareMyPicturesCheckBox
			// 
			this.shareMyPicturesCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.shareMyPicturesCheckBox.Location = new System.Drawing.Point(56, 68);
			this.shareMyPicturesCheckBox.Name = "shareMyPicturesCheckBox";
			this.shareMyPicturesCheckBox.Size = new System.Drawing.Size(288, 16);
			this.shareMyPicturesCheckBox.TabIndex = 4;
			this.shareMyPicturesCheckBox.Text = "Share My Pictures";
			this.shareMyPicturesCheckBox.CheckedChanged += new System.EventHandler(this.shareMyPicturesCheckBox_CheckedChanged);
			// 
			// shareMyMusicCheckBox
			// 
			this.shareMyMusicCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.shareMyMusicCheckBox.Location = new System.Drawing.Point(56, 48);
			this.shareMyMusicCheckBox.Name = "shareMyMusicCheckBox";
			this.shareMyMusicCheckBox.Size = new System.Drawing.Size(288, 16);
			this.shareMyMusicCheckBox.TabIndex = 3;
			this.shareMyMusicCheckBox.Text = "Share My Music";
			this.shareMyMusicCheckBox.CheckedChanged += new System.EventHandler(this.shareMyMusicCheckBox_CheckedChanged);
			// 
			// pictureBox2
			// 
			this.pictureBox2.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox2.Image")));
			this.pictureBox2.Location = new System.Drawing.Point(8, 44);
			this.pictureBox2.Name = "pictureBox2";
			this.pictureBox2.Size = new System.Drawing.Size(40, 40);
			this.pictureBox2.TabIndex = 2;
			this.pictureBox2.TabStop = false;
			// 
			// sharedFoldersListView
			// 
			this.sharedFoldersListView.AllowDrop = true;
			this.sharedFoldersListView.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.sharedFoldersListView.AutoArrange = false;
			this.sharedFoldersListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																									this.columnHeader1});
			this.sharedFoldersListView.FullRowSelect = true;
			this.sharedFoldersListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
			this.sharedFoldersListView.HideSelection = false;
			this.sharedFoldersListView.LabelWrap = false;
			this.sharedFoldersListView.Location = new System.Drawing.Point(8, 136);
			this.sharedFoldersListView.MultiSelect = false;
			this.sharedFoldersListView.Name = "sharedFoldersListView";
			this.sharedFoldersListView.Size = new System.Drawing.Size(336, 136);
			this.sharedFoldersListView.SmallImageList = this.folderImageList;
			this.sharedFoldersListView.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.sharedFoldersListView.TabIndex = 1;
			this.sharedFoldersListView.View = System.Windows.Forms.View.Details;
			this.sharedFoldersListView.DragDrop += new System.Windows.Forms.DragEventHandler(this.sharedFoldersListView_DragDrop);
			this.sharedFoldersListView.DragEnter += new System.Windows.Forms.DragEventHandler(this.sharedFoldersListView_DragEnter);
			this.sharedFoldersListView.SelectedIndexChanged += new System.EventHandler(this.sharedFoldersListView_SelectedIndexChanged);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Width = 314;
			// 
			// folderImageList
			// 
			this.folderImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.folderImageList.ImageSize = new System.Drawing.Size(16, 16);
			this.folderImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("folderImageList.ImageStream")));
			this.folderImageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// label5
			// 
			this.label5.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label5.Location = new System.Drawing.Point(8, 8);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(336, 32);
			this.label5.TabIndex = 0;
			this.label5.Text = "Use this tab to configure the media content folders that are available to any med" +
				"ia device connected on the network.";
			// 
			// accessControlTabPage
			// 
			this.accessControlTabPage.Controls.AddRange(new System.Windows.Forms.Control[] {
																							   this.removeAllowedDeviceButton,
																							   this.addAllowedDeviceButton,
																							   this.allowedDeviceListLabel,
																							   this.allowedDevicesListView,
																							   this.allowDeviceListRadioButton,
																							   this.allowLocalNetworkRadioButton,
																							   this.allowEveryoneRadioButton,
																							   this.pictureBox3,
																							   this.label7});
			this.accessControlTabPage.Location = new System.Drawing.Point(4, 22);
			this.accessControlTabPage.Name = "accessControlTabPage";
			this.accessControlTabPage.Size = new System.Drawing.Size(352, 318);
			this.accessControlTabPage.TabIndex = 2;
			this.accessControlTabPage.Text = "Access Control";
			// 
			// removeAllowedDeviceButton
			// 
			this.removeAllowedDeviceButton.Enabled = false;
			this.removeAllowedDeviceButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("removeAllowedDeviceButton.Image")));
			this.removeAllowedDeviceButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.removeAllowedDeviceButton.Location = new System.Drawing.Point(8, 272);
			this.removeAllowedDeviceButton.Name = "removeAllowedDeviceButton";
			this.removeAllowedDeviceButton.Size = new System.Drawing.Size(168, 32);
			this.removeAllowedDeviceButton.TabIndex = 12;
			this.removeAllowedDeviceButton.Text = "Remove Device";
			// 
			// addAllowedDeviceButton
			// 
			this.addAllowedDeviceButton.Enabled = false;
			this.addAllowedDeviceButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("addAllowedDeviceButton.Image")));
			this.addAllowedDeviceButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.addAllowedDeviceButton.Location = new System.Drawing.Point(176, 272);
			this.addAllowedDeviceButton.Name = "addAllowedDeviceButton";
			this.addAllowedDeviceButton.Size = new System.Drawing.Size(168, 32);
			this.addAllowedDeviceButton.TabIndex = 11;
			this.addAllowedDeviceButton.Text = "Add Device";
			// 
			// allowedDeviceListLabel
			// 
			this.allowedDeviceListLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.allowedDeviceListLabel.Location = new System.Drawing.Point(8, 120);
			this.allowedDeviceListLabel.Name = "allowedDeviceListLabel";
			this.allowedDeviceListLabel.Size = new System.Drawing.Size(336, 16);
			this.allowedDeviceListLabel.TabIndex = 10;
			this.allowedDeviceListLabel.Text = "Allowed Media Device List";
			// 
			// allowedDevicesListView
			// 
			this.allowedDevicesListView.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.allowedDevicesListView.Enabled = false;
			this.allowedDevicesListView.Location = new System.Drawing.Point(8, 136);
			this.allowedDevicesListView.Name = "allowedDevicesListView";
			this.allowedDevicesListView.Size = new System.Drawing.Size(336, 136);
			this.allowedDevicesListView.SmallImageList = this.folderImageList;
			this.allowedDevicesListView.TabIndex = 9;
			this.allowedDevicesListView.View = System.Windows.Forms.View.List;
			this.allowedDevicesListView.DoubleClick += new System.EventHandler(this.openFolderMenuItem_Click);
			// 
			// allowDeviceListRadioButton
			// 
			this.allowDeviceListRadioButton.Location = new System.Drawing.Point(56, 88);
			this.allowDeviceListRadioButton.Name = "allowDeviceListRadioButton";
			this.allowDeviceListRadioButton.Size = new System.Drawing.Size(288, 16);
			this.allowDeviceListRadioButton.TabIndex = 6;
			this.allowDeviceListRadioButton.Text = "Allow only media devices in the allowed list";
			this.allowDeviceListRadioButton.CheckedChanged += new System.EventHandler(this.allowEveryoneRadioButton_CheckedChanged);
			// 
			// allowLocalNetworkRadioButton
			// 
			this.allowLocalNetworkRadioButton.Location = new System.Drawing.Point(56, 68);
			this.allowLocalNetworkRadioButton.Name = "allowLocalNetworkRadioButton";
			this.allowLocalNetworkRadioButton.Size = new System.Drawing.Size(288, 16);
			this.allowLocalNetworkRadioButton.TabIndex = 5;
			this.allowLocalNetworkRadioButton.Text = "Allow only media devices on my local network";
			this.allowLocalNetworkRadioButton.CheckedChanged += new System.EventHandler(this.allowEveryoneRadioButton_CheckedChanged);
			// 
			// allowEveryoneRadioButton
			// 
			this.allowEveryoneRadioButton.Checked = true;
			this.allowEveryoneRadioButton.Location = new System.Drawing.Point(56, 48);
			this.allowEveryoneRadioButton.Name = "allowEveryoneRadioButton";
			this.allowEveryoneRadioButton.Size = new System.Drawing.Size(288, 16);
			this.allowEveryoneRadioButton.TabIndex = 4;
			this.allowEveryoneRadioButton.TabStop = true;
			this.allowEveryoneRadioButton.Text = "Allow everyone to see and use the media";
			this.allowEveryoneRadioButton.CheckedChanged += new System.EventHandler(this.allowEveryoneRadioButton_CheckedChanged);
			// 
			// pictureBox3
			// 
			this.pictureBox3.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox3.Image")));
			this.pictureBox3.Location = new System.Drawing.Point(8, 48);
			this.pictureBox3.Name = "pictureBox3";
			this.pictureBox3.Size = new System.Drawing.Size(40, 56);
			this.pictureBox3.TabIndex = 3;
			this.pictureBox3.TabStop = false;
			// 
			// label7
			// 
			this.label7.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label7.Location = new System.Drawing.Point(8, 8);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(336, 32);
			this.label7.TabIndex = 1;
			this.label7.Text = "Use this tab to configure which network device will have access to the shared med" +
				"ia folders.";
			// 
			// mediaTransferTabPage
			// 
			this.mediaTransferTabPage.Controls.AddRange(new System.Windows.Forms.Control[] {
																							   this.numberOfDeniedRequestsLabel,
																							   this.numberOfContentTransfersLabel,
																							   this.numberOfContentRequestsLabel,
																							   this.mediaTransfersLabel,
																							   this.label13,
																							   this.label12,
																							   this.label11,
																							   this.mediaTransfersPanel,
																							   this.pictureBox4,
																							   this.label8});
			this.mediaTransferTabPage.Location = new System.Drawing.Point(4, 22);
			this.mediaTransferTabPage.Name = "mediaTransferTabPage";
			this.mediaTransferTabPage.Size = new System.Drawing.Size(352, 318);
			this.mediaTransferTabPage.TabIndex = 4;
			this.mediaTransferTabPage.Text = "Media Transfers";
			// 
			// numberOfDeniedRequestsLabel
			// 
			this.numberOfDeniedRequestsLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.numberOfDeniedRequestsLabel.Location = new System.Drawing.Point(280, 88);
			this.numberOfDeniedRequestsLabel.Name = "numberOfDeniedRequestsLabel";
			this.numberOfDeniedRequestsLabel.Size = new System.Drawing.Size(56, 16);
			this.numberOfDeniedRequestsLabel.TabIndex = 12;
			this.numberOfDeniedRequestsLabel.Text = "0";
			this.numberOfDeniedRequestsLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// numberOfContentTransfersLabel
			// 
			this.numberOfContentTransfersLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.numberOfContentTransfersLabel.Location = new System.Drawing.Point(280, 68);
			this.numberOfContentTransfersLabel.Name = "numberOfContentTransfersLabel";
			this.numberOfContentTransfersLabel.Size = new System.Drawing.Size(56, 16);
			this.numberOfContentTransfersLabel.TabIndex = 11;
			this.numberOfContentTransfersLabel.Text = "0";
			this.numberOfContentTransfersLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// numberOfContentRequestsLabel
			// 
			this.numberOfContentRequestsLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.numberOfContentRequestsLabel.Location = new System.Drawing.Point(280, 48);
			this.numberOfContentRequestsLabel.Name = "numberOfContentRequestsLabel";
			this.numberOfContentRequestsLabel.Size = new System.Drawing.Size(56, 16);
			this.numberOfContentRequestsLabel.TabIndex = 10;
			this.numberOfContentRequestsLabel.Text = "0";
			this.numberOfContentRequestsLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// mediaTransfersLabel
			// 
			this.mediaTransfersLabel.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.mediaTransfersLabel.Location = new System.Drawing.Point(8, 120);
			this.mediaTransfersLabel.Name = "mediaTransfersLabel";
			this.mediaTransfersLabel.Size = new System.Drawing.Size(336, 16);
			this.mediaTransfersLabel.TabIndex = 9;
			this.mediaTransfersLabel.Text = "No media transfers currently in progress";
			// 
			// label13
			// 
			this.label13.Location = new System.Drawing.Point(72, 88);
			this.label13.Name = "label13";
			this.label13.Size = new System.Drawing.Size(208, 16);
			this.label13.TabIndex = 8;
			this.label13.Text = "Number of Denied Requests";
			// 
			// label12
			// 
			this.label12.Location = new System.Drawing.Point(72, 68);
			this.label12.Name = "label12";
			this.label12.Size = new System.Drawing.Size(208, 16);
			this.label12.TabIndex = 7;
			this.label12.Text = "Number of Content Transfer Requests";
			// 
			// label11
			// 
			this.label11.Location = new System.Drawing.Point(72, 48);
			this.label11.Name = "label11";
			this.label11.Size = new System.Drawing.Size(208, 16);
			this.label11.TabIndex = 6;
			this.label11.Text = "Number of Content List Requests";
			// 
			// mediaTransfersPanel
			// 
			this.mediaTransfersPanel.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.mediaTransfersPanel.AutoScroll = true;
			this.mediaTransfersPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.mediaTransfersPanel.Location = new System.Drawing.Point(8, 136);
			this.mediaTransfersPanel.Name = "mediaTransfersPanel";
			this.mediaTransfersPanel.Size = new System.Drawing.Size(336, 168);
			this.mediaTransfersPanel.TabIndex = 5;
			// 
			// pictureBox4
			// 
			this.pictureBox4.Image = ((System.Drawing.Bitmap)(resources.GetObject("pictureBox4.Image")));
			this.pictureBox4.Location = new System.Drawing.Point(8, 48);
			this.pictureBox4.Name = "pictureBox4";
			this.pictureBox4.Size = new System.Drawing.Size(56, 56);
			this.pictureBox4.TabIndex = 4;
			this.pictureBox4.TabStop = false;
			// 
			// label8
			// 
			this.label8.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label8.Location = new System.Drawing.Point(8, 8);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(336, 32);
			this.label8.TabIndex = 2;
			this.label8.Text = "This tab lists all the current media transfers occuring from this computer to med" +
				"ia devices connection on the network.";
			// 
			// aboutTabPage
			// 
			this.aboutTabPage.Controls.AddRange(new System.Windows.Forms.Control[] {
																					   this.label4,
																					   this.label3,
																					   this.linkLabel2,
																					   this.label2,
																					   this.linkLabel1,
																					   this.label1,
																					   this.upnpForumPictureBox});
			this.aboutTabPage.Location = new System.Drawing.Point(4, 22);
			this.aboutTabPage.Name = "aboutTabPage";
			this.aboutTabPage.Size = new System.Drawing.Size(352, 318);
			this.aboutTabPage.TabIndex = 3;
			this.aboutTabPage.Text = "About";
			// 
			// label4
			// 
			this.label4.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label4.Location = new System.Drawing.Point(8, 48);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(328, 40);
			this.label4.TabIndex = 10;
			this.label4.Text = "UPnP AV 1.0 Compatible Content Directory Service\nUsing the Intel Authoring Tools " +
				"for UPnP Technologies";
			// 
			// label3
			// 
			this.label3.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label3.Location = new System.Drawing.Point(8, 136);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(328, 16);
			this.label3.TabIndex = 9;
			this.label3.Text = "Intel® UPnP* Technology";
			// 
			// linkLabel2
			// 
			this.linkLabel2.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.linkLabel2.Location = new System.Drawing.Point(16, 152);
			this.linkLabel2.Name = "linkLabel2";
			this.linkLabel2.Size = new System.Drawing.Size(320, 19);
			this.linkLabel2.TabIndex = 8;
			this.linkLabel2.TabStop = true;
			this.linkLabel2.Text = "http://www.intel.com/technology/upnp/";
			this.linkLabel2.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabel2_LinkClicked);
			// 
			// label2
			// 
			this.label2.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label2.Location = new System.Drawing.Point(8, 96);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(328, 16);
			this.label2.TabIndex = 7;
			this.label2.Text = "Intel® Developer Network for Digital Home";
			// 
			// linkLabel1
			// 
			this.linkLabel1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.linkLabel1.Location = new System.Drawing.Point(16, 112);
			this.linkLabel1.Name = "linkLabel1";
			this.linkLabel1.Size = new System.Drawing.Size(320, 19);
			this.linkLabel1.TabIndex = 6;
			this.linkLabel1.TabStop = true;
			this.linkLabel1.Text = "http://www.intel.com/technology/dhdevnet/";
			this.linkLabel1.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabel1_LinkClicked);
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.label1.Font = new System.Drawing.Font("Arial", 11.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.Location = new System.Drawing.Point(8, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(328, 24);
			this.label1.TabIndex = 5;
			this.label1.Text = "Intel® Digital Home Media Server";
			// 
			// upnpForumPictureBox
			// 
			this.upnpForumPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.upnpForumPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("upnpForumPictureBox.Image")));
			this.upnpForumPictureBox.Location = new System.Drawing.Point(184, 208);
			this.upnpForumPictureBox.Name = "upnpForumPictureBox";
			this.upnpForumPictureBox.Size = new System.Drawing.Size(152, 96);
			this.upnpForumPictureBox.TabIndex = 3;
			this.upnpForumPictureBox.TabStop = false;
			// 
			// closeButton
			// 
			this.closeButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
			this.closeButton.BackColor = System.Drawing.SystemColors.Control;
			this.closeButton.Location = new System.Drawing.Point(328, 420);
			this.closeButton.Name = "closeButton";
			this.closeButton.Size = new System.Drawing.Size(112, 24);
			this.closeButton.TabIndex = 3;
			this.closeButton.Text = "Close";
			this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
			// 
			// sharedFoldersContextMenu
			// 
			this.sharedFoldersContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																									 this.openFolderMenuItem,
																									 this.menuItem2,
																									 this.removeFolderMenuItem});
			this.sharedFoldersContextMenu.Popup += new System.EventHandler(this.sharedFoldersContextMenu_Popup);
			// 
			// openFolderMenuItem
			// 
			this.openFolderMenuItem.DefaultItem = true;
			this.openFolderMenuItem.Index = 0;
			this.openFolderMenuItem.Text = "&Open Folder";
			this.openFolderMenuItem.Click += new System.EventHandler(this.openFolderMenuItem_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.Text = "-";
			// 
			// removeFolderMenuItem
			// 
			this.removeFolderMenuItem.Index = 2;
			this.removeFolderMenuItem.Text = "&Remove Shared Folder";
			this.removeFolderMenuItem.Click += new System.EventHandler(this.removeSharedFolderButton_Click);
			// 
			// MainForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackgroundImage = ((System.Drawing.Bitmap)(resources.GetObject("$this.BackgroundImage")));
			this.ClientSize = new System.Drawing.Size(442, 448);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.closeButton,
																		  this.tabControl1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.HelpButton = true;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "MainForm";
			this.Text = "Intel Digital Home Media Server";
			this.Load += new System.EventHandler(this.MainForm_Load);
			this.tabControl1.ResumeLayout(false);
			this.mediaSharingTabPage.ResumeLayout(false);
			this.accessControlTabPage.ResumeLayout(false);
			this.mediaTransferTabPage.ResumeLayout(false);
			this.aboutTabPage.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new MainForm());
		}

		private void MediaServerEventSink(string msg) 
		{
			//eventLogTextBox.Text += (msg + "\r\n");
		}

		private void linkLabel1_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start("http://www.intel.com/technology/dhdevnet/");
		}

		private void linkLabel2_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start("http://www.intel.com/technology/upnp/");
		}

		private void closeButton_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void refreshSharedFolders()
		{
			shareMyMusicCheckBox.Enabled = false;
			shareMyPicturesCheckBox.Enabled = false;
			shareMyVideosCheckBox.Enabled = false;

			bool shareMyMusicCheck = false;
			bool shareMyPicturesCheck = false;
			bool shareMyVideosCheck = false;

			string folders = MediaServer.GetSharedFolders();
			sharedFoldersListView.BeginUpdate();
			sharedFoldersListView.Items.Clear();
			if (folders != null)
			{
				int i = folders.IndexOf(";");
				while (i > 0 && folders.Length > 1) 
				{
					string dir = folders.Substring(0,i);
					string dirname = dir;

					if (dir.Equals(DirInfoMyMusic.FullName)) shareMyMusicCheck = true;
					else if (dir.Equals(DirInfoMyPictures.FullName)) shareMyPicturesCheck = true;
					else if (dir.Equals(DirInfoMyVideos.FullName)) shareMyVideosCheck = true;
					else
					{
							if (dir.StartsWith(DirInfoMyDocuments.FullName) == true)
							{
								dirname = DirInfoMyDocuments.Name + dir.Substring(DirInfoMyDocuments.FullName.Length);
							}
							ListViewItem item = new ListViewItem(dirname,0);
							item.Tag = dir;
							sharedFoldersListView.Items.Add(item);
					}

					folders = folders.Substring(i+1);
					i = folders.IndexOf(";");
				}
			}
			sharedFoldersListView.EndUpdate();

			shareMyMusicCheckBox.Checked = shareMyMusicCheck;
			shareMyPicturesCheckBox.Checked = shareMyPicturesCheck;
			shareMyVideosCheckBox.Checked = shareMyVideosCheck;

			shareMyMusicCheckBox.Enabled = DirInfoMyMusic.Exists;
			shareMyPicturesCheckBox.Enabled = DirInfoMyPictures.Exists;
			shareMyVideosCheckBox.Enabled = DirInfoMyVideos.Exists;
		}

		private void addSharedFolderButton_Click(object sender, System.EventArgs e)
		{
			Shell32.Shell shell = new Shell32.ShellClass();
			Shell32.Folder folder = shell.BrowseForFolder(this.Handle.ToInt32(),"Select a directory to share. That directory and all sub-folders will also be made available on the network.",1,null);
			string directoryString = null;

			if (folder != null)
			{
				if (folder.ParentFolder != null)
				{
					for (int i = 0 ; i < folder.ParentFolder.Items().Count ; i++)
					{
						if (folder.ParentFolder.Items().Item(i).IsFolder)
						{
							if (((Shell32.Folder)(folder.ParentFolder.Items().Item(i).GetFolder)).Title == folder.Title)
							{
								directoryString = folder.ParentFolder.Items().Item(i).Path;
								break;
							}
						}
					}
				}

				if (directoryString == null || directoryString.StartsWith("::") == true)
				{
					MessageBox.Show(this,"Invalid folder","Output Folder",MessageBoxButtons.OK,MessageBoxIcon.Warning);
					directoryString = null;
				}
			}

			if (directoryString != null)
			{
				MediaServer.AddSharedFolder(directoryString);
				refreshSharedFolders();
			}
		}

		private void MainForm_Load(object sender, System.EventArgs e)
		{
			Thread.Sleep(200);
			refreshSharedFolders();
		}

		private void sharedFoldersListView_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if (sharedFoldersListView.SelectedItems.Count != 0) 
			{
				sharedFoldersListView.ContextMenu = sharedFoldersContextMenu;
				removeSharedFolderButton.Enabled = true;
			} 
			else 
			{
				sharedFoldersListView.ContextMenu = null;
				removeSharedFolderButton.Enabled = false;
			}
		}

		private void removeSharedFolderButton_Click(object sender, System.EventArgs e)
		{
			if (sharedFoldersListView.SelectedItems.Count == 0) return;
			MediaServer.RemoveSharedFolder((string)(sharedFoldersListView.SelectedItems[0].Tag));
			refreshSharedFolders();
			sharedFoldersListView_SelectedIndexChanged(this,null);
		}

		private void openFolderMenuItem_Click(object sender, System.EventArgs e)
		{
			if (sharedFoldersListView.SelectedItems.Count == 0) return;
			DirectoryInfo dir = new DirectoryInfo((string)(sharedFoldersListView.SelectedItems[0].Tag));
			if (dir.Exists == true) System.Diagnostics.Process.Start(dir.FullName);
		}

		private void sharedFoldersContextMenu_Popup(object sender, System.EventArgs e)
		{
			if (sharedFoldersListView.SelectedItems.Count == 0) return;
			DirectoryInfo dir = new DirectoryInfo((string)(sharedFoldersListView.SelectedItems[0].Tag));
			openFolderMenuItem.Enabled = dir.Exists;
		}

		private void allowEveryoneRadioButton_CheckedChanged(object sender, System.EventArgs e)
		{
			allowedDevicesListView.Enabled = (allowDeviceListRadioButton.Checked == true);
			addAllowedDeviceButton.Enabled = (allowDeviceListRadioButton.Checked == true);
			if (allowDeviceListRadioButton.Checked == true) 
			{
				removeAllowedDeviceButton.Enabled = (allowedDevicesListView.SelectedItems.Count != 0);
			}
			else 
			{
				removeAllowedDeviceButton.Enabled = false;
			}
		}

		private void sharedFoldersListView_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
			{
				string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);
				if (filenames.Length != 1) return;
				DirectoryInfo dir = new DirectoryInfo(filenames[0]);
				if (dir.Exists == false) return;
				e.Effect = DragDropEffects.Copy;
			}
		}

		private void sharedFoldersListView_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
			{
				string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);
				if (filenames.Length != 1) return;
				DirectoryInfo dir = new DirectoryInfo(filenames[0]);
				if (dir.Exists == false) return;
				MediaServer.AddSharedFolder(dir.FullName);
				refreshSharedFolders();
			}
		}

		private void shareFolderhandling(bool action,string Name,DirectoryInfo dir)
		{
			shareMyMusicCheckBox.Enabled = false;
			shareMyPicturesCheckBox.Enabled = false;
			shareMyVideosCheckBox.Enabled = false;

			if (action == true)
			{
				if (MessageBox.Show(this,"Share \""+Name+"\" folder and all sub-folders?","Intel Media Server",MessageBoxButtons.YesNo,MessageBoxIcon.Question) == DialogResult.Yes)
				{
					MediaServer.AddSharedFolder(dir.FullName);
				}
				refreshSharedFolders();
			}
			else
			{
				if (MessageBox.Show(this,"Stop sharing \""+Name+"\" folder and all sub-folders?","Intel Media Server",MessageBoxButtons.YesNo,MessageBoxIcon.Question) == DialogResult.Yes)
				{
					MediaServer.RemoveSharedFolder(dir.FullName);
				}
				refreshSharedFolders();
			}
		}

		private void shareMyMusicCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			if (shareMyMusicCheckBox.Enabled == false) return;
			shareFolderhandling(shareMyMusicCheckBox.Checked,"My Music",DirInfoMyMusic);
		}

		private void shareMyPicturesCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			if (shareMyPicturesCheckBox.Enabled == false) return;
			shareFolderhandling(shareMyPicturesCheckBox.Checked,"My Pictures",DirInfoMyPictures);
		}

		private void shareMyVideosCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			if (shareMyVideosCheckBox.Enabled == false) return;
			shareFolderhandling(shareMyVideosCheckBox.Checked,"My Videos",DirInfoMyVideos);
		}

	}
}
