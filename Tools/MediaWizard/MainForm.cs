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
using System.IO;
using System.Xml;
using System.Net;
using System.Data;
using System.Drawing;
using System.Threading;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnPWizard
{
    /// <summary>
    /// Summary description for MainForm.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form
    {

        private Queue SessionQ = new Queue();


        private bool VolumeChange = false;
        private bool SeekChange = false;

        private System.Threading.Mutex PlayLock = new System.Threading.Mutex();

        private AVRendererDiscovery AVDisco;
        private ArrayList RendererList = new ArrayList();
        private Hashtable ConnectionTable = new Hashtable();
        private Hashtable MenuTable = new Hashtable();
        private Hashtable FileTable = new Hashtable();

        private Hashtable PlayModeTable = new Hashtable();

        private string URL = "";
        private int port = 0;


        private byte[] TheASX;

        private TimeSpan SeekingTime = new TimeSpan(0, 0, 10);

        private AVRenderer CurrentRenderer = null;
        private AVConnection CurrentConnection = null;


        private System.Windows.Forms.Label Status;
        private System.Windows.Forms.Label CurrentPosition;
        private System.Windows.Forms.Button PlayButton;
        private System.Windows.Forms.ImageList playButtonImageList;
        private System.Windows.Forms.ImageList stopButtonImageList;
        private System.Windows.Forms.ImageList pauseImageList;
        private System.Windows.Forms.ImageList nextTrackImageList;
        private System.Windows.Forms.ImageList prevTrackImageList;
        private System.Windows.Forms.Button prevTrackButton;
        private System.Windows.Forms.Button nextTrackButton;
        private System.Windows.Forms.ImageList extenderButtonImageList;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.Button seekFwrdButton;
        private System.Windows.Forms.Button seekBackButton;
        private System.Windows.Forms.ImageList seekBackImageList;
        private System.Windows.Forms.ImageList seekFwrdImageList;
        private System.Windows.Forms.ContextMenu mainPopupMenu;
        private System.Windows.Forms.ContextMenu seekContextMenu;
        private System.Windows.Forms.MenuItem menuItem7;
        private System.Windows.Forms.MenuItem menuItem13;
        private System.Windows.Forms.MenuItem menuItem14;
        private System.Windows.Forms.MenuItem menuItem15;
        private System.Windows.Forms.MenuItem menuItem16;
        private new System.Windows.Forms.Label Location;
        private System.Windows.Forms.Button StopButton;
        private System.Windows.Forms.Button PauseButton;
        private System.Windows.Forms.PictureBox greenVolumeBar;
        private System.Windows.Forms.ContextMenu RendererContextMenu;
        private System.Windows.Forms.MenuItem openFilesMenuItem;
        private System.Windows.Forms.MenuItem menuItem17;
        private System.Windows.Forms.OpenFileDialog openFilesDialog;
        private System.Windows.Forms.Label TrackInfo;
        private System.Windows.Forms.MenuItem CloseConnectionItem;
        private System.Windows.Forms.Label PlayModeLabel;
        private System.Windows.Forms.ContextMenu PlayModeContextMenu;
        private System.Windows.Forms.PictureBox blueProgressPictureBox;
        private System.Windows.Forms.PictureBox greenProgressPictureBox;
        private System.Windows.Forms.PictureBox blueVolumeBar;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem menuItem18;
        private System.Windows.Forms.MenuItem menuItem19;
        private System.Windows.Forms.MenuItem exitMenuItem;
        private System.Windows.Forms.MenuItem menuItem24;
        private System.Windows.Forms.MenuItem menuItem26;
        private System.Windows.Forms.MenuItem menuItem27;
        private System.Windows.Forms.MenuItem menuItem28;
        private System.Windows.Forms.MenuItem menuItem29;
        private System.Windows.Forms.MenuItem menuItem31;
        private System.Windows.Forms.MenuItem onTopMenuItem;
        private System.Windows.Forms.MenuItem menuItem35;
        private System.Windows.Forms.MenuItem openFileMenuItem;
        private System.Windows.Forms.MenuItem debugMenuItem;
        private System.Windows.Forms.MenuItem menuItem21;
        private System.Windows.Forms.MenuItem playModeMenuItem;
        private System.Windows.Forms.MenuItem menuItem22;
        private System.Windows.Forms.MenuItem menuItem23;
        private System.Windows.Forms.MenuItem menuItem30;
        private System.Windows.Forms.MenuItem controlMenuItem;
        private System.Windows.Forms.MenuItem menuItem25;
        private System.Windows.Forms.MenuItem fileMenuItem;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.MenuItem menuItem6;
        private System.Windows.Forms.MenuItem menuItem8;
        private System.Windows.Forms.MenuItem menuItem9;
        private System.Windows.Forms.Label Author;
        private System.Windows.Forms.Label Title;
        private System.Windows.Forms.MenuItem CloseConnectionMenuItem;
        private System.Windows.Forms.MenuItem OpenFilesExistingMenuItem1;
        private System.Windows.Forms.MenuItem openFilesExistingMenuItem2;
        private System.Windows.Forms.Label TransportStatusLabel;
        private System.Windows.Forms.MenuItem extendedM3UMenuItem;
        private System.Windows.Forms.MenuItem menuItem10;
        private System.ComponentModel.IContainer components;

        MiniWebServer ajax = null;

        public MainForm(string[] args)
        {
            //
            // Required for Windows Form Designer support
            //
            MediaResource.AUTOMAPFILE = "file://";
            InitializeComponent();

            //this.Left = Screen.PrimaryScreen.WorkingArea.Width - this.Width + Screen.PrimaryScreen.WorkingArea.Left;
            //this.Top = Screen.PrimaryScreen.WorkingArea.Height - this.Height + Screen.PrimaryScreen.WorkingArea.Top;
            //this.SetClientSizeCore(this.BackgroundImage.Width-1,this.BackgroundImage.Height-1);

            blueProgressPictureBox.Bounds = greenProgressPictureBox.Bounds;
            blueVolumeBar.Bounds = greenVolumeBar.Bounds;

            Bitmap b = new Bitmap(this.BackgroundImage, this.ClientSize);
            Graphics g = Graphics.FromImage(b);
            g.DrawImage(this.BackgroundImage, new Rectangle(0, 0, this.ClientSize.Width, this.ClientSize.Height), 0, 0, this.BackgroundImage.Width, this.BackgroundImage.Height, GraphicsUnit.Pixel);
            this.BackgroundImage = b;

            this.TopMost = onTopMenuItem.Checked;

            foreach (string a in args)
            {
                if (a == "/NOPIPELINE")
                {
                    HTTPRequest.PIPELINE = false;
                }
                else
                    if (a == "/DEBUG")
                    {
                        OpenSource.Utilities.InstanceTracker.Enabled = true;
                        OpenSource.Utilities.InstanceTracker.Display();
                        OpenSource.Utilities.EventLogger.Enabled = true;
                        OpenSource.Utilities.EventLogger.ShowAll = true;
                    }
                    else
                        if (a == "/NOCHUNK")
                        {
                            HTTPSession.CHUNK_ENABLED = false;
                        }
                if (a.StartsWith("/AJAX:"))
                {
                    IPEndPoint ep = new IPEndPoint(IPAddress.Any, int.Parse(a.Substring(a.IndexOf(":") + 1)));
                    ajax = new MiniWebServer(ep);
                    ajax.OnSession += new OpenSource.UPnP.MiniWebServer.NewSessionHandler(ajax_OnSession);
                }
            }
        }

        private void MainForm_Load(object sender, System.EventArgs e)
        {
            AVDisco = new AVRendererDiscovery(new AVRendererDiscovery.DiscoveryHandler(DiscoSink));
            AVDisco.OnRendererRemoved += new AVRendererDiscovery.DiscoveryHandler(RemoveSink);
        }

        protected void ParseDirectory(DirectoryInfo d, Hashtable h)
        {
            FileInfo[] f = d.GetFiles();
            foreach (FileInfo i in f)
            {
                ParseFile(i, h);
            }

            DirectoryInfo[] di = d.GetDirectories();
            foreach (DirectoryInfo ddi in di)
            {
                ParseDirectory(ddi, h);
            }
        }
        protected void ParseFile(FileInfo f, Hashtable h)
        {
            h[f.GetHashCode().ToString() + "/" + f.Name] = f;
        }

        protected MediaResource[] BuildResources(string Interface, IDictionaryEnumerator en)
        {
            ArrayList a = new ArrayList();
            while (en.MoveNext())
            {
                FileInfo f = (FileInfo)en.Value;
                MediaResource mr;
                //PlugFest Temp Fix
                if (port != 80)
                {
                    ResourceBuilder.ResourceAttributes resInfo = new ResourceBuilder.ResourceAttributes();
                    resInfo.contentUri = "file://" + f.FullName;
                    resInfo.protocolInfo = ProtocolInfoString.CreateHttpGetProtocolInfoString(f);

                    mr = ResourceBuilder.CreateResource(resInfo);
                    OpenSource.UPnP.AV.Extensions.MetaData.Finder.PopulateMetaData(mr, f);
                }
                else
                {
                    ResourceBuilder.ResourceAttributes resInfo = new ResourceBuilder.ResourceAttributes();
                    resInfo.contentUri = "file://" + f.FullName;
                    resInfo.protocolInfo = ProtocolInfoString.CreateHttpGetProtocolInfoString(f);

                    mr = ResourceBuilder.CreateResource(resInfo);
                    OpenSource.UPnP.AV.Extensions.MetaData.Finder.PopulateMetaData(mr, f);
                }
                a.Add(mr);
            }
            return ((MediaResource[])a.ToArray(typeof(MediaResource)));
        }

        protected void BuildASX()
        {
            IDictionaryEnumerator en = FileTable.GetEnumerator();

            MemoryStream ms = new MemoryStream();
            XmlTextWriter XDoc = new XmlTextWriter(ms, System.Text.Encoding.UTF8);
            XDoc.Indentation = 5;
            XDoc.Formatting = System.Xml.Formatting.Indented;

            XDoc.WriteStartDocument();
            XDoc.WriteStartElement("ASX");
            XDoc.WriteAttributeString("Version", "3.0");
            XDoc.WriteElementString("TITLE", "Autogenerated Playlist");
            XDoc.WriteElementString("AUTHOR", "OpenSource");

            while (en.MoveNext())
            {
                FileInfo f = (FileInfo)en.Value;

                XDoc.WriteStartElement("ENTRY");
                XDoc.WriteElementString("TITLE", "");
                XDoc.WriteElementString("AUTHOR", "");
                XDoc.WriteStartElement("Ref");
                XDoc.WriteAttributeString("href", URL + (string)en.Key);
                XDoc.WriteEndElement();
                XDoc.WriteEndElement();
            }
            XDoc.WriteEndElement();
            XDoc.WriteEndDocument();
            XDoc.Flush();

            TheASX = new byte[ms.Length - 3];
            ms.Seek(3, SeekOrigin.Begin);
            ms.Read(TheASX, 0, TheASX.Length);
            XDoc.Close();
        }

        protected void RemoveSink(AVRendererDiscovery sender, AVRenderer r)
        {
            RendererList.Remove(r);
            if (CurrentRenderer.UniqueDeviceName == r.UniqueDeviceName)
            {
                CurrentRenderer = null;
                CurrentConnection = null;
                if (RendererList.Count > 0) CurrentRenderer = (AVRenderer)RendererList[0];
                ShowRenderer();
            }
        }
        protected void EventFailSink(AVRenderer sender)
        {
            AVDisco.ForceDisposeRenderer(sender);
        }
        protected void DiscoSink(AVRendererDiscovery sender, AVRenderer r)
        {
            r.OnEventRenewalFailure += new AVRenderer.EventRenewalFailureHandler(EventFailSink);
            lock (this)
            {
                r.OnCreateConnection += new AVRenderer.ConnectionHandler(NewConnectionsSink);
                r.OnRecycledConnection += new AVRenderer.ConnectionHandler(RecycledConnectionsSink);
                r.OnRemovedConnection += new AVRenderer.ConnectionHandler(RemovedConnectionSink);
                r.OnCreateConnectionFailed += new AVRenderer.FailedConnectionHandler(FailedConnectionSink);

                RendererList.Add(r);

                if (CurrentRenderer == null)
                {
                    CurrentRenderer = r;
                    if (CurrentRenderer.Connections.Count != 0)
                    {
                        CurrentConnection = (AVConnection)CurrentRenderer.Connections[0];

                        foreach (AVConnection c in CurrentRenderer.Connections)
                        {
                            c.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateSink);
                            c.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangeSink);
                            c.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangeSink);
                            c.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
                            c.OnNumberOfTracksChanged += new AVConnection.NumberOfTracksChangedHandler(NumberOfTracksChangedSink);
                            c.OnTrackURIChanged += new AVConnection.TrackURIChangedHandler(TrackURIChangedSink);
                            c.OnCurrentPlayModeChanged += new AVConnection.CurrentPlayModeChangedHandler(PlayModeChangedSink);
                        }

                    }
                    else
                    {
                        CurrentConnection = null;
                    }
                    ShowRenderer();
                }
            }
        }

        protected void FailedConnectionSink(AVRenderer sender, AVRenderer.CreateFailedReason reason, object Handle)
        {
            MessageBox.Show("Could not fulfill request: " + reason.ToString());
        }
        protected void RemovedConnectionSink(AVRenderer sender, AVConnection c, object Handle)
        {
            if (CurrentRenderer.UniqueDeviceName == sender.UniqueDeviceName)
            {
                Location.Text = sender.FriendlyName;
                //mediaInfoLabel.Text = "Renderer Connections: " + CurrentRenderer.Connections.Count.ToString();
            }

            if (CurrentConnection != null)
            {
                if (CurrentConnection.Identifier == c.Identifier)
                {
                    if (CurrentRenderer.Connections.Count > 0)
                    {
                        CurrentConnection = (AVConnection)CurrentRenderer.Connections[0];
                    }
                    else
                    {
                        CurrentConnection = null;
                    }
                    ShowRenderer();
                }
            }
        }
        protected void RecycledConnectionsSink(AVRenderer sender, AVConnection c, object Handle)
        {
            if (CurrentRenderer.UniqueDeviceName == sender.UniqueDeviceName)
            {
                if (CurrentConnection == null) CurrentConnection = c;
                ShowRenderer();
            }

            c.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateSink);
            c.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangeSink);
            c.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangeSink);
            c.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
            c.OnNumberOfTracksChanged += new AVConnection.NumberOfTracksChangedHandler(NumberOfTracksChangedSink);
            c.OnTrackURIChanged += new AVConnection.TrackURIChangedHandler(TrackURIChangedSink);
            c.OnCurrentPlayModeChanged += new AVConnection.CurrentPlayModeChangedHandler(PlayModeChangedSink);
            c.OnMute += new AVConnection.MuteStateChangedHandler(MuteSink);

            if (Handle != null) c.Play();
        }
        protected void NewConnectionsSink(AVRenderer sender, AVConnection c, object Handle)
        {
            if (CurrentRenderer.UniqueDeviceName == sender.UniqueDeviceName)
            {
                if (CurrentConnection == null) CurrentConnection = c;
                ShowRenderer();
            }

            c.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateSink);
            c.OnVolume += new AVConnection.VolumeChangedHandler(VolumeChangeSink);
            c.OnPositionChanged += new AVConnection.PositionChangedHandler(PositionChangeSink);
            c.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
            c.OnNumberOfTracksChanged += new AVConnection.NumberOfTracksChangedHandler(NumberOfTracksChangedSink);
            c.OnTrackURIChanged += new AVConnection.TrackURIChangedHandler(TrackURIChangedSink);
            c.OnCurrentPlayModeChanged += new AVConnection.CurrentPlayModeChangedHandler(PlayModeChangedSink);
            c.OnMute += new AVConnection.MuteStateChangedHandler(MuteSink);

            if (Handle != null) c.Play();
        }

        private void UpdateMuteState()
        {
            if (CurrentRenderer != null)
            {
                if (CurrentConnection != null)
                {
                    if (CurrentConnection.IsMute)
                    {
                        SetVolumeLevel("Master", 0);
                    }
                    else
                    {
                        SetVolumeLevel("Master", CurrentConnection.MasterVolume);
                    }

                }
            }
        }
        protected void MuteSink(AVConnection sender, bool NewMuteStatus)
        {
            UpdateMuteState();
        }

        protected void NumberOfTracksChangedSink(AVConnection sender, UInt32 NumOfTracks)
        {
            if (CurrentConnection != null)
            {
                if (CurrentConnection.Identifier == sender.Identifier)
                {
                    TrackInfo.Text = sender.CurrentTrack.ToString() + "/" + sender.NumberOfTracks.ToString();
                }
            }
        }
        protected void TrackChangedSink(AVConnection sender, UInt32 TrackN)
        {
            if (CurrentConnection != null)
            {
                if (CurrentConnection.Identifier == sender.Identifier)
                {
                    TrackInfo.Text = sender.CurrentTrack.ToString() + "/" + sender.NumberOfTracks.ToString();
                }
            }
        }
        protected void TrackURIChangedSink(AVConnection sender)
        {
            if (CurrentConnection != null)
            {
                if (CurrentConnection.Identifier == sender.Identifier)
                {
                    //MetaData.Text = HTTPMessage.UnEscapeString(sender.TrackURI);
                    bool OK = false;
                    if (sender.Container != null)
                    {
                        foreach (MediaItem Item in sender.Container.Items)
                        {
                            foreach (MediaResource R in Item.MergedResources)
                                if (R.ContentUri == sender.TrackURI)
                                {
                                    OK = true;
                                    Author.Text = Item.Creator;
                                    Title.Text = Item.Title;
                                    break;
                                }
                        }
                    }
                    if (OK == false)
                    {
                        if (sender.TrackURI != "")
                        {
                            try
                            {
                                Uri temp = new Uri(sender.TrackURI);
                                string temp2 = HTTPMessage.UnEscapeString(temp.LocalPath);
                                temp2 = temp2.Substring(temp2.LastIndexOf("/"));
                                temp2 = temp2.Substring(0, temp2.LastIndexOf("."));
                                temp2 = temp2.Substring(1);
                                Author.Text = "";
                                Title.Text = temp2;
                            }
                            catch (Exception e)
                            {
                                OpenSource.Utilities.EventLogger.Log(e);
                            }
                        }
                        else
                        {
                            Title.Text = "";
                            Author.Text = "";
                        }
                    }
                }
            }
        }
        public delegate void VoidDelegate();
        protected void ShowRenderer()
        {
            this.BeginInvoke(new VoidDelegate(ShowRendererEx));
        }
        protected void ShowRendererEx()
        {
            PlayModeLabel.Cursor = Cursors.Default;
            if (CurrentRenderer == null)
            {
                CloseConnectionMenuItem.Enabled = false;
                Location.Text = "Scanning for AV Renderers";
                //mediaInfoLabel.Text = "";
                TrackInfo.Text = "";
            }
            else
            {
                Location.Text = CurrentRenderer.FriendlyName;
                //mediaInfoLabel.Text = "Renderer Connections: " + CurrentRenderer.Connections.Count.ToString();
                if (CurrentConnection != null)
                {
                    if (CurrentConnection.TransportStatus != "OK")
                    {
                        TransportStatusLabel.Text = CurrentConnection.TransportStatus;
                    }
                    else
                    {
                        TransportStatusLabel.Text = "";
                    }
                    this.OpenFilesExistingMenuItem1.Enabled = true;
                    this.openFilesExistingMenuItem2.Enabled = true;
                    if (CurrentRenderer.HasConnectionHandling)
                    {
                        Location.Text = CurrentRenderer.FriendlyName + " #" + CurrentConnection.ConnectionID;
                    }
                    else
                    {
                        Location.Text = CurrentRenderer.FriendlyName;
                    }
                    PlayModeLabel.Cursor = Cursors.Hand;

                    PlayButton.Enabled = true;
                    PauseButton.Enabled = true;
                    StopButton.Enabled = true;
                    CloseConnectionItem.Enabled = true;
                    AVConnection c = CurrentConnection;
                    PlayStateSink(c, c.CurrentState);
                    if (c.SupportsCurrentPosition)
                    {
                        PositionChangeSink(c, c.CurrentPosition);
                        this.TrackChangedSink(c, c.CurrentTrack);
                    }
                    else
                    {
                        CurrentPosition.Text = "";
                    }
                    SetVolumeLevel((int)c.MasterVolume);
                    foreach (string chan in c.SupportedChannels)
                    {
                        ushort vl = c.GetVolume(chan);
                        SetVolumeLevel(chan, vl);
                    }
                    UpdateMuteState();
                    TrackInfo.Text = CurrentConnection.CurrentTrack.ToString() + "/" + CurrentConnection.NumberOfTracks.ToString();
                    TrackURIChangedSink(CurrentConnection);

                    switch (CurrentConnection.CurrentPlayMode)
                    {
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.NORMAL:
                            PlayModeLabel.Text = "Normal Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.INTRO:
                            PlayModeLabel.Text = "Introduction Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.SHUFFLE:
                            PlayModeLabel.Text = "Shuffle Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.RANDOM:
                            PlayModeLabel.Text = "Ramdom Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ONE:
                            PlayModeLabel.Text = "Repeat One Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ALL:
                            PlayModeLabel.Text = "Repeat All Play Mode";
                            break;
                        default:
                            PlayModeLabel.Text = "Play Mode: " + CurrentConnection.CurrentPlayMode.ToString();
                            break;
                    }
                }
                else
                {
                    this.OpenFilesExistingMenuItem1.Enabled = false;
                    this.openFilesExistingMenuItem2.Enabled = false;
                    Status.Text = "";
                    TrackInfo.Text = "";
                    PauseButton.Enabled = false;
                    StopButton.Enabled = false;
                    PlayButton.Enabled = false;
                    CloseConnectionItem.Enabled = false;
                    this.Author.Text = "";
                    this.Title.Text = "";
                    PlayModeLabel.Text = "Normal Play Mode";
                    TransportStatusLabel.Text = "";
                }
            }
        }

        protected void SetVolumeLevel(string channel, UInt16 level)
        {
            PictureBox TallBox = null;
            PictureBox ShortBox = null;

            switch (channel)
            {
                case "Master":
                    TallBox = greenVolumeBar;
                    ShortBox = blueVolumeBar;
                    break;
                case "LF":
                    //TallBox = LeftGreenVolumeBar;
                    //ShortBox = LeftBlueVolumeBar;
                    return;
                case "RF":
                    //TallBox = RightGreenVolumeBar;
                    //ShortBox = RightBlueVolumeBar;
                    return;
                default:
                    return;
            }

            double p = (double)1 - (double)level / (double)100;
            int V = (int)((p * (double)TallBox.Height));
            ShortBox.Height = V;

        }
        protected void SetVolumeLevel(int level)
        {
            double p = (double)1 - (double)level / (double)100;
            int V = (int)((p * (double)greenVolumeBar.Height));
            blueVolumeBar.Height = V;
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.Status = new System.Windows.Forms.Label();
            this.mainPopupMenu = new System.Windows.Forms.ContextMenu();
            this.openFilesMenuItem = new System.Windows.Forms.MenuItem();
            this.openFilesExistingMenuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem17 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.CloseConnectionItem = new System.Windows.Forms.MenuItem();
            this.CurrentPosition = new System.Windows.Forms.Label();
            this.playButtonImageList = new System.Windows.Forms.ImageList(this.components);
            this.PlayButton = new System.Windows.Forms.Button();
            this.stopButtonImageList = new System.Windows.Forms.ImageList(this.components);
            this.StopButton = new System.Windows.Forms.Button();
            this.pauseImageList = new System.Windows.Forms.ImageList(this.components);
            this.PauseButton = new System.Windows.Forms.Button();
            this.nextTrackImageList = new System.Windows.Forms.ImageList(this.components);
            this.prevTrackImageList = new System.Windows.Forms.ImageList(this.components);
            this.prevTrackButton = new System.Windows.Forms.Button();
            this.nextTrackButton = new System.Windows.Forms.Button();
            this.Location = new System.Windows.Forms.Label();
            this.RendererContextMenu = new System.Windows.Forms.ContextMenu();
            this.blueProgressPictureBox = new System.Windows.Forms.PictureBox();
            this.extenderButtonImageList = new System.Windows.Forms.ImageList(this.components);
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.seekFwrdButton = new System.Windows.Forms.Button();
            this.seekContextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.menuItem13 = new System.Windows.Forms.MenuItem();
            this.menuItem14 = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.seekFwrdImageList = new System.Windows.Forms.ImageList(this.components);
            this.seekBackButton = new System.Windows.Forms.Button();
            this.seekBackImageList = new System.Windows.Forms.ImageList(this.components);
            this.greenVolumeBar = new System.Windows.Forms.PictureBox();
            this.PlayModeLabel = new System.Windows.Forms.Label();
            this.PlayModeContextMenu = new System.Windows.Forms.ContextMenu();
            this.menuItem25 = new System.Windows.Forms.MenuItem();
            this.greenProgressPictureBox = new System.Windows.Forms.PictureBox();
            this.blueVolumeBar = new System.Windows.Forms.PictureBox();
            this.TrackInfo = new System.Windows.Forms.Label();
            this.openFilesDialog = new System.Windows.Forms.OpenFileDialog();
            this.Author = new System.Windows.Forms.Label();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.fileMenuItem = new System.Windows.Forms.MenuItem();
            this.openFileMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.OpenFilesExistingMenuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItem24 = new System.Windows.Forms.MenuItem();
            this.onTopMenuItem = new System.Windows.Forms.MenuItem();
            this.extendedM3UMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem35 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem18 = new System.Windows.Forms.MenuItem();
            this.menuItem21 = new System.Windows.Forms.MenuItem();
            this.controlMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem26 = new System.Windows.Forms.MenuItem();
            this.menuItem27 = new System.Windows.Forms.MenuItem();
            this.menuItem28 = new System.Windows.Forms.MenuItem();
            this.menuItem30 = new System.Windows.Forms.MenuItem();
            this.menuItem22 = new System.Windows.Forms.MenuItem();
            this.menuItem23 = new System.Windows.Forms.MenuItem();
            this.menuItem29 = new System.Windows.Forms.MenuItem();
            this.playModeMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem31 = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.CloseConnectionMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem19 = new System.Windows.Forms.MenuItem();
            this.debugMenuItem = new System.Windows.Forms.MenuItem();
            this.Title = new System.Windows.Forms.Label();
            this.TransportStatusLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.blueProgressPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenVolumeBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenProgressPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueVolumeBar)).BeginInit();
            this.SuspendLayout();
            // 
            // Status
            // 
            this.Status.BackColor = System.Drawing.Color.Transparent;
            this.Status.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Status.ForeColor = System.Drawing.Color.PaleGreen;
            this.Status.Location = new System.Drawing.Point(271, 94);
            this.Status.Name = "Status";
            this.Status.Size = new System.Drawing.Size(49, 16);
            this.Status.TabIndex = 6;
            this.Status.Text = "Stopped";
            this.Status.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.toolTip.SetToolTip(this.Status, "Player State");
            // 
            // mainPopupMenu
            // 
            this.mainPopupMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.openFilesMenuItem,
            this.openFilesExistingMenuItem2,
            this.menuItem17,
            this.menuItem2,
            this.menuItem3,
            this.menuItem4,
            this.menuItem1,
            this.menuItem6,
            this.menuItem8,
            this.menuItem5,
            this.CloseConnectionItem});
            this.mainPopupMenu.Popup += new System.EventHandler(this.mainPopupMenu_Popup);
            // 
            // openFilesMenuItem
            // 
            this.openFilesMenuItem.DefaultItem = true;
            this.openFilesMenuItem.Index = 0;
            this.openFilesMenuItem.Text = "Open Files...";
            this.openFilesMenuItem.Click += new System.EventHandler(this.openFilesMenuItem_Click);
            // 
            // openFilesExistingMenuItem2
            // 
            this.openFilesExistingMenuItem2.Index = 1;
            this.openFilesExistingMenuItem2.Text = "Open Files [Existing]";
            this.openFilesExistingMenuItem2.Click += new System.EventHandler(this.openFilesExistingMenuItem2_Click);
            // 
            // menuItem17
            // 
            this.menuItem17.Index = 2;
            this.menuItem17.Text = "-";
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 3;
            this.menuItem2.Text = "&Play";
            this.menuItem2.Click += new System.EventHandler(this.PlayButton_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 4;
            this.menuItem3.Text = "&Stop";
            this.menuItem3.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 5;
            this.menuItem4.Text = "P&ause";
            this.menuItem4.Click += new System.EventHandler(this.PauseButton_Click);
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 6;
            this.menuItem1.Text = "-";
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 7;
            this.menuItem6.Text = "&Next Track";
            this.menuItem6.Click += new System.EventHandler(this.nextTrackButton_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 8;
            this.menuItem8.Text = "&Previous Track";
            this.menuItem8.Click += new System.EventHandler(this.prevTrackButton_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 9;
            this.menuItem5.Text = "-";
            // 
            // CloseConnectionItem
            // 
            this.CloseConnectionItem.Index = 10;
            this.CloseConnectionItem.Text = "&Close Connection";
            this.CloseConnectionItem.Click += new System.EventHandler(this.CloseConnectionItem_Click);
            // 
            // CurrentPosition
            // 
            this.CurrentPosition.BackColor = System.Drawing.Color.Transparent;
            this.CurrentPosition.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.CurrentPosition.ForeColor = System.Drawing.Color.PaleGreen;
            this.CurrentPosition.Location = new System.Drawing.Point(264, 110);
            this.CurrentPosition.Name = "CurrentPosition";
            this.CurrentPosition.Size = new System.Drawing.Size(115, 16);
            this.CurrentPosition.TabIndex = 12;
            this.CurrentPosition.Text = "00:00:00 / 00:00:00";
            this.CurrentPosition.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.toolTip.SetToolTip(this.CurrentPosition, "Position Indicator");
            // 
            // playButtonImageList
            // 
            this.playButtonImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("playButtonImageList.ImageStream")));
            this.playButtonImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.playButtonImageList.Images.SetKeyName(0, "");
            this.playButtonImageList.Images.SetKeyName(1, "");
            this.playButtonImageList.Images.SetKeyName(2, "");
            this.playButtonImageList.Images.SetKeyName(3, "");
            // 
            // PlayButton
            // 
            this.PlayButton.BackColor = System.Drawing.Color.Transparent;
            this.PlayButton.CausesValidation = false;
            this.PlayButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.PlayButton.ForeColor = System.Drawing.Color.Transparent;
            this.PlayButton.ImageIndex = 1;
            this.PlayButton.ImageList = this.playButtonImageList;
            this.PlayButton.Location = new System.Drawing.Point(13, 99);
            this.PlayButton.Name = "PlayButton";
            this.PlayButton.Size = new System.Drawing.Size(33, 33);
            this.PlayButton.TabIndex = 1;
            this.PlayButton.Tag = "";
            this.toolTip.SetToolTip(this.PlayButton, "Play");
            this.PlayButton.UseVisualStyleBackColor = false;
            this.PlayButton.Click += new System.EventHandler(this.PlayButton_Click);
            this.PlayButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.PlayButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // stopButtonImageList
            // 
            this.stopButtonImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("stopButtonImageList.ImageStream")));
            this.stopButtonImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.stopButtonImageList.Images.SetKeyName(0, "");
            this.stopButtonImageList.Images.SetKeyName(1, "");
            this.stopButtonImageList.Images.SetKeyName(2, "");
            this.stopButtonImageList.Images.SetKeyName(3, "");
            // 
            // StopButton
            // 
            this.StopButton.BackColor = System.Drawing.Color.Transparent;
            this.StopButton.CausesValidation = false;
            this.StopButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.StopButton.ForeColor = System.Drawing.Color.Transparent;
            this.StopButton.ImageIndex = 2;
            this.StopButton.ImageList = this.stopButtonImageList;
            this.StopButton.Location = new System.Drawing.Point(47, 99);
            this.StopButton.Name = "StopButton";
            this.StopButton.Size = new System.Drawing.Size(33, 33);
            this.StopButton.TabIndex = 2;
            this.toolTip.SetToolTip(this.StopButton, "Stop");
            this.StopButton.UseVisualStyleBackColor = false;
            this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
            this.StopButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.StopButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // pauseImageList
            // 
            this.pauseImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("pauseImageList.ImageStream")));
            this.pauseImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.pauseImageList.Images.SetKeyName(0, "");
            this.pauseImageList.Images.SetKeyName(1, "");
            this.pauseImageList.Images.SetKeyName(2, "");
            this.pauseImageList.Images.SetKeyName(3, "");
            // 
            // PauseButton
            // 
            this.PauseButton.BackColor = System.Drawing.Color.Transparent;
            this.PauseButton.CausesValidation = false;
            this.PauseButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.PauseButton.ForeColor = System.Drawing.Color.Transparent;
            this.PauseButton.ImageIndex = 1;
            this.PauseButton.ImageList = this.pauseImageList;
            this.PauseButton.Location = new System.Drawing.Point(81, 99);
            this.PauseButton.Name = "PauseButton";
            this.PauseButton.Size = new System.Drawing.Size(33, 33);
            this.PauseButton.TabIndex = 3;
            this.toolTip.SetToolTip(this.PauseButton, "Pause");
            this.PauseButton.UseVisualStyleBackColor = false;
            this.PauseButton.Click += new System.EventHandler(this.PauseButton_Click);
            this.PauseButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.PauseButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // nextTrackImageList
            // 
            this.nextTrackImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("nextTrackImageList.ImageStream")));
            this.nextTrackImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.nextTrackImageList.Images.SetKeyName(0, "");
            this.nextTrackImageList.Images.SetKeyName(1, "");
            this.nextTrackImageList.Images.SetKeyName(2, "");
            this.nextTrackImageList.Images.SetKeyName(3, "");
            // 
            // prevTrackImageList
            // 
            this.prevTrackImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("prevTrackImageList.ImageStream")));
            this.prevTrackImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.prevTrackImageList.Images.SetKeyName(0, "");
            this.prevTrackImageList.Images.SetKeyName(1, "");
            this.prevTrackImageList.Images.SetKeyName(2, "");
            this.prevTrackImageList.Images.SetKeyName(3, "");
            // 
            // prevTrackButton
            // 
            this.prevTrackButton.BackColor = System.Drawing.Color.Transparent;
            this.prevTrackButton.CausesValidation = false;
            this.prevTrackButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.prevTrackButton.ForeColor = System.Drawing.Color.Transparent;
            this.prevTrackButton.ImageIndex = 1;
            this.prevTrackButton.ImageList = this.prevTrackImageList;
            this.prevTrackButton.Location = new System.Drawing.Point(119, 99);
            this.prevTrackButton.Name = "prevTrackButton";
            this.prevTrackButton.Size = new System.Drawing.Size(33, 33);
            this.prevTrackButton.TabIndex = 4;
            this.toolTip.SetToolTip(this.prevTrackButton, "Previous Track");
            this.prevTrackButton.UseVisualStyleBackColor = false;
            this.prevTrackButton.Click += new System.EventHandler(this.prevTrackButton_Click);
            this.prevTrackButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.prevTrackButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // nextTrackButton
            // 
            this.nextTrackButton.BackColor = System.Drawing.Color.Transparent;
            this.nextTrackButton.CausesValidation = false;
            this.nextTrackButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.nextTrackButton.ForeColor = System.Drawing.Color.Transparent;
            this.nextTrackButton.ImageIndex = 1;
            this.nextTrackButton.ImageList = this.nextTrackImageList;
            this.nextTrackButton.Location = new System.Drawing.Point(221, 99);
            this.nextTrackButton.Name = "nextTrackButton";
            this.nextTrackButton.Size = new System.Drawing.Size(33, 33);
            this.nextTrackButton.TabIndex = 7;
            this.toolTip.SetToolTip(this.nextTrackButton, "Next Track");
            this.nextTrackButton.UseVisualStyleBackColor = false;
            this.nextTrackButton.Click += new System.EventHandler(this.nextTrackButton_Click);
            this.nextTrackButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.nextTrackButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // Location
            // 
            this.Location.BackColor = System.Drawing.Color.Transparent;
            this.Location.ContextMenu = this.RendererContextMenu;
            this.Location.Cursor = System.Windows.Forms.Cursors.Default;
            this.Location.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Location.ForeColor = System.Drawing.Color.PaleGreen;
            this.Location.Location = new System.Drawing.Point(22, 28);
            this.Location.Name = "Location";
            this.Location.Size = new System.Drawing.Size(344, 16);
            this.Location.TabIndex = 23;
            this.Location.Text = "Scanning for AV Renderers";
            this.Location.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnClick);
            // 
            // blueProgressPictureBox
            // 
            this.blueProgressPictureBox.BackColor = System.Drawing.Color.Transparent;
            this.blueProgressPictureBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.blueProgressPictureBox.Location = new System.Drawing.Point(136, 157);
            this.blueProgressPictureBox.Name = "blueProgressPictureBox";
            this.blueProgressPictureBox.Size = new System.Drawing.Size(240, 18);
            this.blueProgressPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.blueProgressPictureBox.TabIndex = 26;
            this.blueProgressPictureBox.TabStop = false;
            this.toolTip.SetToolTip(this.blueProgressPictureBox, "Position Control");
            this.blueProgressPictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnMoveProgress);
            this.blueProgressPictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnPressProgress);
            this.blueProgressPictureBox.MouseUp += new System.Windows.Forms.MouseEventHandler(this.OnReleaseProgress);
            // 
            // extenderButtonImageList
            // 
            this.extenderButtonImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("extenderButtonImageList.ImageStream")));
            this.extenderButtonImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.extenderButtonImageList.Images.SetKeyName(0, "");
            this.extenderButtonImageList.Images.SetKeyName(1, "");
            this.extenderButtonImageList.Images.SetKeyName(2, "");
            this.extenderButtonImageList.Images.SetKeyName(3, "");
            // 
            // seekFwrdButton
            // 
            this.seekFwrdButton.BackColor = System.Drawing.Color.Transparent;
            this.seekFwrdButton.CausesValidation = false;
            this.seekFwrdButton.ContextMenu = this.seekContextMenu;
            this.seekFwrdButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seekFwrdButton.ForeColor = System.Drawing.Color.Transparent;
            this.seekFwrdButton.ImageIndex = 1;
            this.seekFwrdButton.ImageList = this.seekFwrdImageList;
            this.seekFwrdButton.Location = new System.Drawing.Point(187, 99);
            this.seekFwrdButton.Name = "seekFwrdButton";
            this.seekFwrdButton.Size = new System.Drawing.Size(33, 33);
            this.seekFwrdButton.TabIndex = 6;
            this.toolTip.SetToolTip(this.seekFwrdButton, "Seek Forward");
            this.seekFwrdButton.UseVisualStyleBackColor = false;
            this.seekFwrdButton.Click += new System.EventHandler(this.seekFwrdButton_Click);
            this.seekFwrdButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.seekFwrdButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // seekContextMenu
            // 
            this.seekContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem7,
            this.menuItem13,
            this.menuItem14,
            this.menuItem15,
            this.menuItem16});
            // 
            // menuItem7
            // 
            this.menuItem7.Checked = true;
            this.menuItem7.Index = 0;
            this.menuItem7.Text = "Seek 10 seconds";
            this.menuItem7.Click += new System.EventHandler(this.menuItem7_Click);
            // 
            // menuItem13
            // 
            this.menuItem13.Index = 1;
            this.menuItem13.Text = "Seek 30 seconds";
            this.menuItem13.Click += new System.EventHandler(this.menuItem13_Click);
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 2;
            this.menuItem14.Text = "Seek 2 minutes";
            this.menuItem14.Click += new System.EventHandler(this.menuItem14_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 3;
            this.menuItem15.Text = "Seek 10 minutes";
            this.menuItem15.Click += new System.EventHandler(this.menuItem15_Click);
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 4;
            this.menuItem16.Text = "Seek 30 minutes";
            this.menuItem16.Click += new System.EventHandler(this.menuItem16_Click);
            // 
            // seekFwrdImageList
            // 
            this.seekFwrdImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("seekFwrdImageList.ImageStream")));
            this.seekFwrdImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.seekFwrdImageList.Images.SetKeyName(0, "");
            this.seekFwrdImageList.Images.SetKeyName(1, "");
            this.seekFwrdImageList.Images.SetKeyName(2, "");
            this.seekFwrdImageList.Images.SetKeyName(3, "");
            // 
            // seekBackButton
            // 
            this.seekBackButton.BackColor = System.Drawing.Color.Transparent;
            this.seekBackButton.CausesValidation = false;
            this.seekBackButton.ContextMenu = this.seekContextMenu;
            this.seekBackButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seekBackButton.ForeColor = System.Drawing.Color.Transparent;
            this.seekBackButton.ImageIndex = 1;
            this.seekBackButton.ImageList = this.seekBackImageList;
            this.seekBackButton.Location = new System.Drawing.Point(153, 99);
            this.seekBackButton.Name = "seekBackButton";
            this.seekBackButton.Size = new System.Drawing.Size(33, 33);
            this.seekBackButton.TabIndex = 5;
            this.toolTip.SetToolTip(this.seekBackButton, "Seek Back");
            this.seekBackButton.UseVisualStyleBackColor = false;
            this.seekBackButton.Click += new System.EventHandler(this.seekBackButton_Click);
            this.seekBackButton.MouseDown += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            this.seekBackButton.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SwitchPlayImage);
            // 
            // seekBackImageList
            // 
            this.seekBackImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("seekBackImageList.ImageStream")));
            this.seekBackImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.seekBackImageList.Images.SetKeyName(0, "");
            this.seekBackImageList.Images.SetKeyName(1, "");
            this.seekBackImageList.Images.SetKeyName(2, "");
            this.seekBackImageList.Images.SetKeyName(3, "");
            // 
            // greenVolumeBar
            // 
            this.greenVolumeBar.BackColor = System.Drawing.Color.Transparent;
            this.greenVolumeBar.Cursor = System.Windows.Forms.Cursors.Hand;
            this.greenVolumeBar.Image = ((System.Drawing.Image)(resources.GetObject("greenVolumeBar.Image")));
            this.greenVolumeBar.Location = new System.Drawing.Point(391, 17);
            this.greenVolumeBar.Name = "greenVolumeBar";
            this.greenVolumeBar.Size = new System.Drawing.Size(17, 158);
            this.greenVolumeBar.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.greenVolumeBar.TabIndex = 28;
            this.greenVolumeBar.TabStop = false;
            this.toolTip.SetToolTip(this.greenVolumeBar, "Volume Control");
            this.greenVolumeBar.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnMoveVolume);
            this.greenVolumeBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnPressVolume);
            this.greenVolumeBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.OnReleaseVolume);
            // 
            // PlayModeLabel
            // 
            this.PlayModeLabel.BackColor = System.Drawing.Color.Transparent;
            this.PlayModeLabel.ContextMenu = this.PlayModeContextMenu;
            this.PlayModeLabel.Cursor = System.Windows.Forms.Cursors.Default;
            this.PlayModeLabel.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PlayModeLabel.ForeColor = System.Drawing.Color.SkyBlue;
            this.PlayModeLabel.Location = new System.Drawing.Point(40, 136);
            this.PlayModeLabel.Name = "PlayModeLabel";
            this.PlayModeLabel.Size = new System.Drawing.Size(296, 15);
            this.PlayModeLabel.TabIndex = 36;
            this.PlayModeLabel.Text = "Normal Play Mode";
            this.PlayModeLabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            this.toolTip.SetToolTip(this.PlayModeLabel, "Play Mode");
            this.PlayModeLabel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnPopulatePlayMode);
            // 
            // PlayModeContextMenu
            // 
            this.PlayModeContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem25});
            this.PlayModeContextMenu.Popup += new System.EventHandler(this.playModeMenuItem_Popup);
            // 
            // menuItem25
            // 
            this.menuItem25.Enabled = false;
            this.menuItem25.Index = 0;
            this.menuItem25.Text = "Place Holder";
            // 
            // greenProgressPictureBox
            // 
            this.greenProgressPictureBox.BackColor = System.Drawing.Color.Transparent;
            this.greenProgressPictureBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.greenProgressPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("greenProgressPictureBox.Image")));
            this.greenProgressPictureBox.Location = new System.Drawing.Point(14, 157);
            this.greenProgressPictureBox.Name = "greenProgressPictureBox";
            this.greenProgressPictureBox.Size = new System.Drawing.Size(362, 18);
            this.greenProgressPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.greenProgressPictureBox.TabIndex = 37;
            this.greenProgressPictureBox.TabStop = false;
            this.toolTip.SetToolTip(this.greenProgressPictureBox, "Position Control");
            this.greenProgressPictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnMoveProgress);
            this.greenProgressPictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnPressProgress);
            this.greenProgressPictureBox.MouseUp += new System.Windows.Forms.MouseEventHandler(this.OnReleaseProgress);
            // 
            // blueVolumeBar
            // 
            this.blueVolumeBar.BackColor = System.Drawing.Color.Transparent;
            this.blueVolumeBar.Cursor = System.Windows.Forms.Cursors.Hand;
            this.blueVolumeBar.Location = new System.Drawing.Point(391, 17);
            this.blueVolumeBar.Name = "blueVolumeBar";
            this.blueVolumeBar.Size = new System.Drawing.Size(17, 71);
            this.blueVolumeBar.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.blueVolumeBar.TabIndex = 38;
            this.blueVolumeBar.TabStop = false;
            this.toolTip.SetToolTip(this.blueVolumeBar, "Volume Control");
            this.blueVolumeBar.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnMoveVolume);
            this.blueVolumeBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnPressVolume);
            this.blueVolumeBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.OnReleaseVolume);
            // 
            // TrackInfo
            // 
            this.TrackInfo.BackColor = System.Drawing.Color.Transparent;
            this.TrackInfo.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TrackInfo.ForeColor = System.Drawing.Color.PaleGreen;
            this.TrackInfo.Location = new System.Drawing.Point(328, 94);
            this.TrackInfo.Name = "TrackInfo";
            this.TrackInfo.Size = new System.Drawing.Size(40, 16);
            this.TrackInfo.TabIndex = 29;
            this.TrackInfo.Text = "0/0";
            this.TrackInfo.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip.SetToolTip(this.TrackInfo, "Track Indicator");
            // 
            // openFilesDialog
            // 
            this.openFilesDialog.Filter = "Audio Files (*.mp3,*.wma)|*.mp3;*.wma|Video Files (*.avi,*.asf,*.wmv)|*.avi;*.asf" +
                ";*.wmv";
            this.openFilesDialog.Multiselect = true;
            this.openFilesDialog.Title = "Open Media Files";
            // 
            // Author
            // 
            this.Author.BackColor = System.Drawing.Color.Transparent;
            this.Author.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Author.ForeColor = System.Drawing.Color.PaleGreen;
            this.Author.Location = new System.Drawing.Point(22, 60);
            this.Author.Name = "Author";
            this.Author.Size = new System.Drawing.Size(234, 16);
            this.Author.TabIndex = 35;
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.fileMenuItem,
            this.menuItem18,
            this.controlMenuItem,
            this.menuItem19});
            // 
            // fileMenuItem
            // 
            this.fileMenuItem.Index = 0;
            this.fileMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.openFileMenuItem,
            this.menuItem10,
            this.OpenFilesExistingMenuItem1,
            this.menuItem24,
            this.onTopMenuItem,
            this.extendedM3UMenuItem,
            this.menuItem35,
            this.exitMenuItem});
            this.fileMenuItem.Text = "&File";
            this.fileMenuItem.Popup += new System.EventHandler(this.fileMenuItem_Popup);
            // 
            // openFileMenuItem
            // 
            this.openFileMenuItem.Index = 0;
            this.openFileMenuItem.Text = "&Open Files...";
            this.openFileMenuItem.Click += new System.EventHandler(this.openFilesMenuItem_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 1;
            this.menuItem10.Text = "&Browse Media...";
            this.menuItem10.Click += new System.EventHandler(this.menuItem10_Click);
            // 
            // OpenFilesExistingMenuItem1
            // 
            this.OpenFilesExistingMenuItem1.Enabled = false;
            this.OpenFilesExistingMenuItem1.Index = 2;
            this.OpenFilesExistingMenuItem1.Text = "Open Files in Instance...";
            this.OpenFilesExistingMenuItem1.Visible = false;
            this.OpenFilesExistingMenuItem1.Click += new System.EventHandler(this.OpenFilesExistingMenuItem1_Click);
            // 
            // menuItem24
            // 
            this.menuItem24.Index = 3;
            this.menuItem24.Text = "-";
            // 
            // onTopMenuItem
            // 
            this.onTopMenuItem.Index = 4;
            this.onTopMenuItem.Text = "&Stay On Top";
            this.onTopMenuItem.Click += new System.EventHandler(this.onTopMenuItem_Click);
            // 
            // extendedM3UMenuItem
            // 
            this.extendedM3UMenuItem.Checked = true;
            this.extendedM3UMenuItem.Index = 5;
            this.extendedM3UMenuItem.Text = "&Extended M3U Playlist";
            this.extendedM3UMenuItem.Click += new System.EventHandler(this.extendedM3UMenuItem_Click);
            // 
            // menuItem35
            // 
            this.menuItem35.Index = 6;
            this.menuItem35.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 7;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem18
            // 
            this.menuItem18.Index = 1;
            this.menuItem18.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem21});
            this.menuItem18.Text = "&Device";
            this.menuItem18.Popup += new System.EventHandler(this.menuItem18_Popup);
            // 
            // menuItem21
            // 
            this.menuItem21.Enabled = false;
            this.menuItem21.Index = 0;
            this.menuItem21.Text = "Place Holder";
            // 
            // controlMenuItem
            // 
            this.controlMenuItem.Index = 2;
            this.controlMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem26,
            this.menuItem27,
            this.menuItem28,
            this.menuItem30,
            this.menuItem22,
            this.menuItem23,
            this.menuItem29,
            this.playModeMenuItem,
            this.menuItem9,
            this.CloseConnectionMenuItem});
            this.controlMenuItem.Text = "&Control";
            this.controlMenuItem.Popup += new System.EventHandler(this.controlMenuItem_Popup);
            // 
            // menuItem26
            // 
            this.menuItem26.Index = 0;
            this.menuItem26.Text = "&Play";
            this.menuItem26.Click += new System.EventHandler(this.PlayButton_Click);
            // 
            // menuItem27
            // 
            this.menuItem27.Index = 1;
            this.menuItem27.Text = "&Stop";
            this.menuItem27.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // menuItem28
            // 
            this.menuItem28.Index = 2;
            this.menuItem28.Text = "P&ause";
            this.menuItem28.Click += new System.EventHandler(this.PauseButton_Click);
            // 
            // menuItem30
            // 
            this.menuItem30.Index = 3;
            this.menuItem30.Text = "-";
            // 
            // menuItem22
            // 
            this.menuItem22.Index = 4;
            this.menuItem22.Text = "&Next Track";
            this.menuItem22.Click += new System.EventHandler(this.nextTrackButton_Click);
            // 
            // menuItem23
            // 
            this.menuItem23.Index = 5;
            this.menuItem23.Text = "P&revious Track";
            this.menuItem23.Click += new System.EventHandler(this.prevTrackButton_Click);
            // 
            // menuItem29
            // 
            this.menuItem29.Index = 6;
            this.menuItem29.Text = "-";
            // 
            // playModeMenuItem
            // 
            this.playModeMenuItem.Index = 7;
            this.playModeMenuItem.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem31});
            this.playModeMenuItem.Text = "Play &Mode";
            this.playModeMenuItem.Popup += new System.EventHandler(this.playModeMenuItem_Popup);
            // 
            // menuItem31
            // 
            this.menuItem31.Enabled = false;
            this.menuItem31.Index = 0;
            this.menuItem31.Text = "Place Holder";
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 8;
            this.menuItem9.Text = "-";
            // 
            // CloseConnectionMenuItem
            // 
            this.CloseConnectionMenuItem.Index = 9;
            this.CloseConnectionMenuItem.Text = "&Close Connection";
            this.CloseConnectionMenuItem.Click += new System.EventHandler(this.CloseConnectionItem_Click);
            // 
            // menuItem19
            // 
            this.menuItem19.Index = 3;
            this.menuItem19.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.debugMenuItem});
            this.menuItem19.Text = "&Help";
            // 
            // debugMenuItem
            // 
            this.debugMenuItem.Index = 0;
            this.debugMenuItem.Text = "&Show Debug Information";
            this.debugMenuItem.Click += new System.EventHandler(this.debugMenuItem_Click);
            // 
            // Title
            // 
            this.Title.BackColor = System.Drawing.Color.Transparent;
            this.Title.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Title.ForeColor = System.Drawing.Color.PaleGreen;
            this.Title.Location = new System.Drawing.Point(22, 44);
            this.Title.Name = "Title";
            this.Title.Size = new System.Drawing.Size(344, 16);
            this.Title.TabIndex = 39;
            // 
            // TransportStatusLabel
            // 
            this.TransportStatusLabel.BackColor = System.Drawing.Color.Transparent;
            this.TransportStatusLabel.ForeColor = System.Drawing.Color.Red;
            this.TransportStatusLabel.Location = new System.Drawing.Point(272, 72);
            this.TransportStatusLabel.Name = "TransportStatusLabel";
            this.TransportStatusLabel.Size = new System.Drawing.Size(100, 16);
            this.TransportStatusLabel.TabIndex = 40;
            this.TransportStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // MainForm
            // 
            this.AllowDrop = true;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
            this.ClientSize = new System.Drawing.Size(424, 192);
            this.ContextMenu = this.mainPopupMenu;
            this.Controls.Add(this.TransportStatusLabel);
            this.Controls.Add(this.Location);
            this.Controls.Add(this.Title);
            this.Controls.Add(this.PlayModeLabel);
            this.Controls.Add(this.Author);
            this.Controls.Add(this.TrackInfo);
            this.Controls.Add(this.seekFwrdButton);
            this.Controls.Add(this.seekBackButton);
            this.Controls.Add(this.nextTrackButton);
            this.Controls.Add(this.prevTrackButton);
            this.Controls.Add(this.PauseButton);
            this.Controls.Add(this.StopButton);
            this.Controls.Add(this.PlayButton);
            this.Controls.Add(this.CurrentPosition);
            this.Controls.Add(this.Status);
            this.Controls.Add(this.blueProgressPictureBox);
            this.Controls.Add(this.blueVolumeBar);
            this.Controls.Add(this.greenVolumeBar);
            this.Controls.Add(this.greenProgressPictureBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "AV Media Wizard";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.MainForm_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.MainForm_DragEnter);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.MainForm_KeyUp);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.MainForm_KeyDown);
            ((System.ComponentModel.ISupportInitialize)(this.blueProgressPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenVolumeBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenProgressPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueVolumeBar)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

        private void SwitchPlayImage(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            Button button = (Button)sender;
            if (button.ImageIndex == 1 || button.ImageIndex == 2)
            {
                button.ImageIndex = 3;
            }
            else
            {
                if (button.Text != " ")
                {
                    button.ImageIndex = 1;
                }
                else
                {
                    button.ImageIndex = 2;
                }
            }
        }

        private void SetPlayList(IDictionaryEnumerator en, bool reuse)
        {
            CurrentRenderer.OnCreateConnection += new AVRenderer.ConnectionHandler(CreateConnectionSink);
            CurrentRenderer.OnRecycledConnection += new AVRenderer.ConnectionHandler(CreateConnectionSink);

            MediaResource[] MR = BuildResources(CurrentRenderer.Interface.ToString(), en);

            MediaBuilder.container container = new MediaBuilder.container("Autogenerated Playlist");
            container.ID = Guid.NewGuid().ToString();
            container.IdIsValid = true;
            IMediaContainer mc = MediaBuilder.CreateContainer(container);
            foreach (IMediaResource R in MR)
            {
                mc.AddObject(R.Owner, true);
            }
            if (!reuse)
            {
                lock (CurrentRenderer)
                {
                    CurrentRenderer.CreateConnection(MR, true);
                }
            }
            else
            {
                lock (CurrentConnection)
                {
                    CurrentConnection.CreateConnection(MR, true);
                }
            }
        }

        private void PlayButton_Click(object sender, System.EventArgs e)
        {
            AVRenderer r = CurrentRenderer;

            if (CurrentConnection != null) CurrentConnection.Play();
        }
        private void CreateConnectionSink(AVRenderer sender, AVConnection c, object Handle)
        {
            sender.OnCreateConnection -= new AVRenderer.ConnectionHandler(CreateConnectionSink);
            sender.OnRecycledConnection -= new AVRenderer.ConnectionHandler(CreateConnectionSink);

            CurrentConnection = c;
            ShowRenderer();
        }

        private void PlayModeChangedSink(AVConnection sender, AVConnection.PlayMode NewMode)
        {
            if (CurrentConnection == null) return;
            if (sender.Identifier == CurrentConnection.Identifier)
            {
                switch (NewMode)
                {
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.NORMAL:
                        PlayModeLabel.Text = "Normal Play Mode";
                        break;
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.INTRO:
                        PlayModeLabel.Text = "Introduction Play Mode";
                        break;
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.SHUFFLE:
                        PlayModeLabel.Text = "Shuffle Play Mode";
                        break;
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.RANDOM:
                        PlayModeLabel.Text = "Ramdom Play Mode";
                        break;
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ONE:
                        PlayModeLabel.Text = "Repeat One Play Mode";
                        break;
                    case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ALL:
                        PlayModeLabel.Text = "Repeat All Play Mode";
                        break;
                    default:
                        PlayModeLabel.Text = "Play Mode: " + CurrentConnection.CurrentPlayMode.ToString();
                        break;
                }
            }
        }
        private void VolumeChangeSink(AVConnection sender, UInt16 VolumeValue)
        {
            if (CurrentConnection == null) return;

            if (sender.Identifier == CurrentConnection.Identifier)
            {
                if (CurrentConnection.IsMute) return;
                //Volume.Value = (int)VolumeValue;
                //SetVolumeLevel((int)VolumeValue);

                foreach (string channel in sender.SupportedChannels)
                {
                    UInt16 val = 0;
                    val = sender.GetVolume(channel);
                    SetVolumeLevel(channel, val);
                }
            }
        }

        private void PositionChangeSink(AVConnection sender, TimeSpan TS)
        {
            if (CurrentConnection == null) return;
            if (SeekChange == true || VolumeChange == true) return;

            if (sender.Identifier == CurrentConnection.Identifier)
            {
                string tf = string.Format("{0:00}", TS.Hours) + ":" + string.Format("{0:00}", TS.Minutes) + ":" + string.Format("{0:00}", TS.Seconds);
                string d = string.Format("{0:00}", sender.Duration.Hours) + ":" + string.Format("{0:00}", sender.Duration.Minutes) + ":" + string.Format("{0:00}", sender.Duration.Seconds);
                CurrentPosition.Text = tf + " / " + d;

                long total = sender.Duration.Ticks;
                long current = TS.Ticks;
                double p = (double)current / (double)total;

                Rectangle positionBounds = greenProgressPictureBox.Bounds;
                int nlen = (int)(((double)positionBounds.Width) * p);
                blueProgressPictureBox.Bounds = new Rectangle(positionBounds.Left + nlen, positionBounds.Top, positionBounds.Width - nlen, positionBounds.Height);
            }
        }

        private void PauseButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;

            CurrentConnection.Pause();
        }

        private void StopButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;

            CurrentConnection.Stop();

        }


        private void PlayStateSink(AVConnection sender, AVConnection.PlayState state)
        {
            if (CurrentConnection == null) return;
            if (CurrentConnection.Identifier != sender.Identifier) return;

            PlayButton.ImageIndex = 1;
            StopButton.ImageIndex = 1;
            PauseButton.ImageIndex = 1;
            PlayButton.Text = "";
            StopButton.Text = "";
            PauseButton.Text = "";

            if (CurrentConnection == null) return;
            if (CurrentConnection.Identifier == sender.Identifier)
            {
                switch (state)
                {
                    case AVConnection.PlayState.PLAYING:
                        Status.Text = "Playing";
                        PlayButton.ImageIndex = 2;
                        PlayButton.Text = " ";
                        break;
                    case AVConnection.PlayState.PAUSED:
                        Status.Text = "Paused";
                        PauseButton.ImageIndex = 2;
                        PauseButton.Text = " ";
                        break;
                    case AVConnection.PlayState.STOPPED:
                        Status.Text = "Stopped";
                        StopButton.ImageIndex = 2;
                        StopButton.Text = " ";
                        break;
                    case AVConnection.PlayState.TRANSITIONING:
                        Status.Text = "Transit";
                        break;
                    default:
                        Status.Text = "Unknown";
                        break;
                }
            }
            lock (SessionQ)
            {
                while (SessionQ.Count > 0)
                {
                    HTTPSession s = (HTTPSession)SessionQ.Dequeue();
                    HTTPMessage r = new HTTPMessage("1.1");
                    r.StatusCode = 200;
                    r.StatusData = "OK";
                    r.StringBuffer = ScrapeUI();
                    s.Send(r);
                }
            }
        }

        private void OnSelect(object sender, EventArgs e)
        {
            object j = MenuTable[sender];

            if (j.GetType().FullName == "OpenSource.UPnP.AV.RENDERER.CP.AVConnection")
            {
                CurrentConnection = (AVConnection)j;
                CurrentRenderer = CurrentConnection.Parent;
            }
            else
            {
                CurrentRenderer = (AVRenderer)j;
                if (CurrentRenderer.Connections.Count != 0)
                {
                    CurrentConnection = (AVConnection)CurrentRenderer.Connections[0];
                }
                else
                {
                    CurrentConnection = null;
                }
            }
            ShowRenderer();
        }

        private void CloseSink(HTTPSession sender)
        {
        }
        private void DoneSink(HTTPSession s, Stream ss)
        {
            ss.Close();
            if ((bool)s.StateObject == true)
            {
                s.Close();
            }
        }

        private void NextButton_Click(object sender, System.EventArgs e)
        {

        }

        private void menuItem11_Click(object sender, System.EventArgs e)
        {

        }

        private void notifyIcon_DoubleClick(object sender, System.EventArgs e)
        {
            this.Visible = !this.Visible;
        }

        private void menuItem8_Click(object sender, System.EventArgs e)
        {
            this.Visible = true;
        }

        private void menuItem9_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void OnClick(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button != MouseButtons.Right) return;
            MenuItem i;

            MenuTable.Clear();
            RendererContextMenu.MenuItems.Clear();

            AVRenderer[] R = (AVRenderer[])RendererList.ToArray(typeof(AVRenderer));
            foreach (AVRenderer r in R)
            {
                i = new MenuItem(r.FriendlyName, new EventHandler(OnSelect));
                MenuTable[i] = r;

                if (CurrentRenderer != null)
                {
                    if (CurrentRenderer.UniqueDeviceName == r.UniqueDeviceName)
                    {
                        i.Checked = true;
                    }
                }
                if (r.Connections.Count > 1)
                {
                    foreach (AVConnection c in r.Connections)
                    {
                        MenuItem SubItem = new MenuItem("Connection: " + c.ConnectionID.ToString(), new EventHandler(OnSelect));
                        MenuTable[SubItem] = c;
                        if (CurrentConnection != null)
                        {
                            if (c.Identifier == CurrentConnection.Identifier) SubItem.Checked = true;
                        }
                        i.MenuItems.Add(SubItem);
                    }
                }
                RendererContextMenu.MenuItems.Add(i);
            }
        }

        private void nextTrackButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;
            CurrentConnection.NextTrack();
        }

        private void prevTrackButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;
            CurrentConnection.PreviousTrack();
        }

        private void OnMoveVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // TODO: Check if master volume channel is present!
            if (CurrentConnection == null) return;

            int total = greenVolumeBar.Height;
            int Y = e.Y;
            if (Y < 0) Y = 0;
            if (Y > total) Y = total;

            double p = (double)Y / (double)total;
            int V = (int)(p * (double)total);

            if (VolumeChange)
            {
                blueVolumeBar.Height = V;

                int V2 = greenVolumeBar.Height - blueVolumeBar.Height;
                double p2 = (double)V2 / (double)greenVolumeBar.Height;
                int N2 = (int)(p2 * 100);
                if (N2 != 0)
                {
                    CurrentPosition.Text = "Volume " + N2 + "%";
                }
                else
                {
                    CurrentPosition.Text = "MUTE";
                }
            }
        }

        private void OnPressVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // TODO: Check if master volume channel is present!
            if (CurrentConnection == null) return;
            if (e.Button != MouseButtons.Left) return;
            VolumeChange = true;
            OnMoveVolume(sender, e);
        }

        private void OnReleaseVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // TODO: Check if master volume channel is present!
            VolumeChange = false;
            if (CurrentConnection == null) return;
            if (e.Button != MouseButtons.Left) return;

            int V = greenVolumeBar.Height - blueVolumeBar.Height;
            double p = (double)V / (double)greenVolumeBar.Height;
            int N = (int)(p * 100);

            if (N == 0)
            {
                CurrentConnection.Mute(true);
                return;
            }

            if (CurrentConnection.IsMute)
            {
                CurrentConnection.Mute(false);
            }
            CurrentConnection.MasterVolume = (UInt16)N;

            string tf = string.Format("{0:00}", CurrentConnection.CurrentPosition.Hours) + ":" + string.Format("{0:00}", CurrentConnection.CurrentPosition.Minutes) + ":" + string.Format("{0:00}", CurrentConnection.CurrentPosition.Seconds);
            string d = string.Format("{0:00}", CurrentConnection.Duration.Hours) + ":" + string.Format("{0:00}", CurrentConnection.Duration.Minutes) + ":" + string.Format("{0:00}", CurrentConnection.Duration.Seconds);
            CurrentPosition.Text = tf + " / " + d;
        }

        private void openFilesMenuItem_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openFilesDialog.ShowDialog(this);
            Hashtable temp = new Hashtable();
            FileInfo f;

            if (r == DialogResult.OK)
            {
                foreach (string filename in openFilesDialog.FileNames)
                {
                    f = new FileInfo(filename);
                    ParseFile(f, temp);
                    ParseFile(f, FileTable);
                    //FileInfo fi = new FileInfo(filename);
                    //FileTable.Add(fi.GetHashCode().ToString() + fi.Extension,fi);
                }
                SetPlayList(temp.GetEnumerator(), false);
            }
        }

        private void OnPressProgress(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (CurrentConnection == null || CurrentConnection.SupportsSeek == false) return;
            if (e.Button != MouseButtons.Left) return;

            SeekChange = true;
            OnMoveProgress(sender, e);
        }

        private void OnMoveProgress(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (CurrentConnection == null || CurrentConnection.SupportsSeek == false) return;

            int total = greenProgressPictureBox.Width;
            int X = e.X;
            if (sender == blueProgressPictureBox) X += blueProgressPictureBox.Left - greenProgressPictureBox.Left;
            if (X < 0) X = 0;
            if (X > total) X = total;

            double p = (double)X / (double)total;
            int V = (int)(p * (double)total);

            if (SeekChange)
            {
                Rectangle positionBounds = greenProgressPictureBox.Bounds;
                blueProgressPictureBox.Bounds = new Rectangle(positionBounds.Left + V, positionBounds.Top, positionBounds.Width - V, positionBounds.Height);

                TimeSpan target = new TimeSpan((long)((double)CurrentConnection.Duration.Ticks * p));
                string tf = string.Format("{0:00}", target.Hours) + ":" + string.Format("{0:00}", target.Minutes) + ":" + string.Format("{0:00}", target.Seconds);
                string d = string.Format("{0:00}", CurrentConnection.Duration.Hours) + ":" + string.Format("{0:00}", CurrentConnection.Duration.Minutes) + ":" + string.Format("{0:00}", CurrentConnection.Duration.Seconds);
                CurrentPosition.Text = tf + " / " + d;
            }
        }

        private void OnReleaseProgress(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (CurrentConnection == null || CurrentConnection.SupportsSeek == false) return;
            if (e.Button != MouseButtons.Left) return;


            int total = greenProgressPictureBox.Width;
            int X = e.X;
            if (sender == blueProgressPictureBox) X += blueProgressPictureBox.Left - greenProgressPictureBox.Left;
            if (X < 0) X = 0;
            if (X > total) X = total;

            double p = (double)X / (double)total;

            double SeekPos = (p * (double)CurrentConnection.Duration.TotalSeconds);
            TimeSpan ts = TimeSpan.FromSeconds(SeekPos);
            SeekChange = false;

            if (CurrentConnection != null)
            {
                if (CurrentConnection.SupportsSeek) CurrentConnection.SeekPosition(ts);
            }
        }

        private void CloseConnectionItem_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection != null)
            {
                CurrentConnection.Close();
            }
        }





        private void ReleaseLeftVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            /*
            LeftVolumeChange = false;

            if(CurrentConnection==null) return;
			

            int V = LeftGreenVolumeBar.Height - LeftBlueVolumeBar.Height;

            double p = (double)V / (double)LeftGreenVolumeBar.Height;
            int N = (int)(p*100);

            CurrentConnection.SetVolume("LF",(UInt16)N);
            */
        }

        private void PressRightVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            /*
            RightVolumeChange = true;
            OnMoveRightVolume(sender,e);
            */
        }

        private void OnMoveRightVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            /*
            int total = RightGreenVolumeBar.Height;
            int Y = e.Y;
            if(Y<0) Y=0;
            if(Y>total) Y=total;

            double p = (double)Y/(double)total;
            int V = (int)(p*(double)total);

            if(RightVolumeChange) RightBlueVolumeBar.Height = V;
            */
        }

        private void ReleaseRightVolume(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            /*
            RightVolumeChange = false;

            if(CurrentConnection==null) return;
			

            int V = RightGreenVolumeBar.Height - RightBlueVolumeBar.Height;

            double p = (double)V / (double)RightGreenVolumeBar.Height;
            int N = (int)(p*100);

            CurrentConnection.SetVolume("RF",(UInt16)N);
            */
        }

        private void button3_Click(object sender, System.EventArgs e)
        {
            GC.Collect();
        }

        private void OnPopulatePlayMode(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            PlayModeContextMenu.MenuItems.Clear();
            playModeMenuItem.MenuItems.Clear();
            PlayModeTable.Clear();

            if (CurrentConnection != null)
            {
                AVConnection.PlayMode[] pm = CurrentConnection.GetSupportedPlayModes();
                foreach (AVConnection.PlayMode P in pm)
                {
                    string playModeText;
                    switch (P)
                    {
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.NORMAL:
                            playModeText = "Normal Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.INTRO:
                            playModeText = "Introduction Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.SHUFFLE:
                            playModeText = "Shuffle Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.RANDOM:
                            playModeText = "Ramdom Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ONE:
                            playModeText = "Repeat One Play Mode";
                            break;
                        case OpenSource.UPnP.AV.RENDERER.CP.AVConnection.PlayMode.REPEAT_ALL:
                            playModeText = "Repeat All Play Mode";
                            break;
                        default:
                            playModeText = P.ToString();
                            break;
                    }

                    MenuItem mi = new MenuItem(playModeText, new EventHandler(OnClickPlayMode));
                    if (CurrentConnection.CurrentPlayMode == P) mi.Checked = true;
                    PlayModeTable[mi] = P;
                    PlayModeContextMenu.MenuItems.Add(mi);

                    mi = new MenuItem(playModeText, new EventHandler(OnClickPlayMode));
                    if (CurrentConnection.CurrentPlayMode == P) mi.Checked = true;
                    PlayModeTable[mi] = P;
                    playModeMenuItem.MenuItems.Add(mi);
                }
            }
            else
            {
                MenuItem mi = new MenuItem("No Selected Connection");
                mi.Enabled = false;
                PlayModeContextMenu.MenuItems.Add(mi);

                mi = new MenuItem("No Selected Connection");
                mi.Enabled = false;
                playModeMenuItem.MenuItems.Add(mi);
            }
        }

        private void OnClickPlayMode(object sender, EventArgs e)
        {
            AVConnection.PlayMode PM = (AVConnection.PlayMode)PlayModeTable[sender];
            CurrentConnection.SetPlayMode(PM);
        }

        private void exitMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void debugMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void onTopMenuItem_Click(object sender, System.EventArgs e)
        {
            onTopMenuItem.Checked = !onTopMenuItem.Checked;
            this.TopMost = onTopMenuItem.Checked;
        }

        private void menuItem18_Popup(object sender, System.EventArgs e)
        {
            menuItem18.MenuItems.Clear();
            MenuItem i;
            MenuTable.Clear();
            RendererContextMenu.MenuItems.Clear();

            AVRenderer[] R = (AVRenderer[])RendererList.ToArray(typeof(AVRenderer));

            if (R.Length == 0)
            {
                i = new MenuItem("No Network Devices Found");
                i.Enabled = false;
                menuItem18.MenuItems.Add(i);
            }
            else
            {
                foreach (AVRenderer r in R)
                {
                    i = new MenuItem(r.FriendlyName, new EventHandler(OnSelect));
                    MenuTable[i] = r;

                    if (CurrentRenderer != null)
                    {
                        if (CurrentRenderer.UniqueDeviceName == r.UniqueDeviceName)
                        {
                            i.Checked = true;
                        }
                    }

                    if (r.Connections.Count > 1)
                    {
                        foreach (AVConnection c in r.Connections)
                        {
                            MenuItem SubItem = new MenuItem("Connection: " + c.ConnectionID.ToString(), new EventHandler(OnSelect));
                            MenuTable[SubItem] = c;
                            if (CurrentConnection != null)
                            {
                                if (c.Identifier == CurrentConnection.Identifier) SubItem.Checked = true;
                            }
                            i.MenuItems.Add(SubItem);
                        }
                    }

                    menuItem18.MenuItems.Add(i);
                }
            }
        }

        private void MainForm_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {

        }

        private void MainForm_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyCode == Keys.F12)
            {
                if (this.Menu == null)
                {
                    this.Menu = mainMenu;
                    this.FormBorderStyle = FormBorderStyle.FixedToolWindow;
                    this.SetClientSizeCore(this.BackgroundImage.Width - 1, this.BackgroundImage.Height - 1);
                }
                else
                {
                    this.Menu = null;
                    this.FormBorderStyle = FormBorderStyle.None;
                    this.SetClientSizeCore(this.BackgroundImage.Width - 1, this.BackgroundImage.Height - 1);
                }
                e.Handled = true;
            }
        }

        private void playModeMenuItem_Popup(object sender, System.EventArgs e)
        {
            OnPopulatePlayMode(this, null);
        }

        private void controlMenuItem_Popup(object sender, System.EventArgs e)
        {
            menuItem26.Enabled = (CurrentConnection != null);
            menuItem27.Enabled = (CurrentConnection != null);
            menuItem28.Enabled = (CurrentConnection != null && CurrentConnection.SupportsPause);
            menuItem22.Enabled = (CurrentConnection != null);
            menuItem23.Enabled = (CurrentConnection != null);
            CloseConnectionMenuItem.Enabled = CurrentRenderer != null && CurrentRenderer.HasConnectionHandling && (CurrentConnection != null);
            playModeMenuItem.Enabled = (CurrentConnection != null);
        }

        private void MainForm_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
        {
            if (CurrentRenderer == null) return;

            if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
            {
                string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);

                // TODO: Check Compatibility Here
                if (filenames.Length != 0)
                {
                    e.Effect = DragDropEffects.Link;
                }
            }
        }

        private void MainForm_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
        {
            if (CurrentRenderer == null) return;

            if (e.Data.GetDataPresent(DataFormats.FileDrop) == true)
            {
                string[] filenames = (string[])e.Data.GetData(DataFormats.FileDrop);

                Hashtable temp = new Hashtable();
                FileInfo f;
                foreach (string filename in filenames)
                {
                    if (Directory.Exists(filename))
                    {
                        DirectoryInfo di = new DirectoryInfo(filename);
                        foreach (FileInfo ff in di.GetFiles())
                        {
                            ParseFile(ff, temp);
                            ParseFile(ff, FileTable);
                        }
                    }
                    else
                    {
                        f = new FileInfo(filename);
                        ParseFile(f, temp);
                        ParseFile(f, FileTable);
                        //FileInfo fi = new FileInfo(filename);
                        //FileTable.Add(fi.GetHashCode().ToString() + fi.Extension,fi);
                    }
                }
                SetPlayList(temp.GetEnumerator(), false);
            }
        }

        private void seekFwrdButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;
            CurrentConnection.SeekPosition(CurrentConnection.CurrentPosition.Add(SeekingTime));
        }

        private void seekBackButton_Click(object sender, System.EventArgs e)
        {
            if (CurrentConnection == null) return;
            CurrentConnection.SeekPosition(CurrentConnection.CurrentPosition.Subtract(SeekingTime));
        }

        private void menuItem7_Click(object sender, System.EventArgs e)
        {
            foreach (MenuItem m in seekContextMenu.MenuItems) m.Checked = false;
            menuItem7.Checked = true;
            SeekingTime = new TimeSpan(0, 0, 10);
        }

        private void menuItem13_Click(object sender, System.EventArgs e)
        {
            foreach (MenuItem m in seekContextMenu.MenuItems) m.Checked = false;
            menuItem13.Checked = true;
            SeekingTime = new TimeSpan(0, 0, 30);
        }

        private void menuItem14_Click(object sender, System.EventArgs e)
        {
            foreach (MenuItem m in seekContextMenu.MenuItems) m.Checked = false;
            menuItem14.Checked = true;
            SeekingTime = new TimeSpan(0, 2, 0);
        }

        private void menuItem15_Click(object sender, System.EventArgs e)
        {
            foreach (MenuItem m in seekContextMenu.MenuItems) m.Checked = false;
            menuItem15.Checked = true;
            SeekingTime = new TimeSpan(0, 10, 0);
        }

        private void menuItem16_Click(object sender, System.EventArgs e)
        {
            foreach (MenuItem m in seekContextMenu.MenuItems) m.Checked = false;
            menuItem16.Checked = true;
            SeekingTime = new TimeSpan(0, 30, 0);
        }

        private void fileMenuItem_Popup(object sender, System.EventArgs e)
        {
            openFileMenuItem.Enabled = (CurrentRenderer != null);
        }

        private void mainPopupMenu_Popup(object sender, System.EventArgs e)
        {
            openFilesMenuItem.Enabled = (CurrentRenderer != null);
            menuItem2.Enabled = (CurrentConnection != null);
            menuItem3.Enabled = (CurrentConnection != null);
            menuItem4.Enabled = (CurrentConnection != null && CurrentConnection.SupportsPause);
            menuItem6.Enabled = (CurrentConnection != null);
            menuItem8.Enabled = (CurrentConnection != null);
            CloseConnectionItem.Enabled = CurrentRenderer != null && CurrentRenderer.HasConnectionHandling && (CurrentConnection != null);
        }

        private void OpenFilesExistingMenuItem1_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openFilesDialog.ShowDialog(this);
            Hashtable temp = new Hashtable();
            FileInfo f;

            if (r == DialogResult.OK)
            {
                foreach (string filename in openFilesDialog.FileNames)
                {
                    f = new FileInfo(filename);
                    ParseFile(f, temp);
                    ParseFile(f, FileTable);
                }
                SetPlayList(temp.GetEnumerator(), true);
            }
        }

        private void openFilesExistingMenuItem2_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openFilesDialog.ShowDialog(this);
            Hashtable temp = new Hashtable();
            FileInfo f;

            if (r == DialogResult.OK)
            {
                foreach (string filename in openFilesDialog.FileNames)
                {
                    f = new FileInfo(filename);
                    ParseFile(f, temp);
                    ParseFile(f, FileTable);
                }
                SetPlayList(temp.GetEnumerator(), true);
            }
        }

        private void extendedM3UMenuItem_Click(object sender, System.EventArgs e)
        {
            extendedM3UMenuItem.Checked = !extendedM3UMenuItem.Checked;
            OpenSource.UPnP.AV.RENDERER.CP.AVPlayList.EnableExtendedM3U = extendedM3UMenuItem.Checked;
        }

        private ContainerDiscovery m_RootContainers = ContainerDiscovery.GetInstance();

        private void menuItem10_Click(object sender, System.EventArgs e)
        {
            CdsBrowserForm browser = new CdsBrowserForm();
            browser.ShowDialog();
            browser.Dispose();
        }

        private void ajax_OnSession(MiniWebServer sender, HTTPSession session)
        {
            session.OnReceive += new OpenSource.UPnP.HTTPSession.ReceiveHandler(ajax_OnReceive);
        }

        private string ScrapeUI()
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();

            sb.Append("<CurrentRenderer>");
            if (this.CurrentConnection != null)
            {
                sb.Append("<Friendly>");
                sb.Append(CurrentConnection.FriendlyName);
                sb.Append("</Friendly>");
                sb.Append("<PlayState>");
                sb.Append(CurrentConnection.CurrentState.ToString());
                sb.Append("</PlayState>");
            }
            sb.Append("</CurrentRenderer>");
            return (sb.ToString());
        }
        private void ajax_OnReceive(HTTPSession sender, HTTPMessage msg)
        {
            string query = "";
            HTTPMessage r;

            if (msg.Directive.ToUpper() == "GET" && msg.DirectiveObj.StartsWith("/Query?"))
            {
                query = msg.DirectiveObj.Substring(msg.DirectiveObj.IndexOf("?") + 1);
                if (query == "-1")
                {
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 200;
                    r.StatusData = "OK";
                    r.StringBuffer = ScrapeUI();
                    sender.Send(r);
                }
                if (query == "-2")
                {
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 205;
                    r.StatusData = "OK";
                    sender.Send(r);
                    if (CurrentConnection != null)
                    {
                        CurrentConnection.Play();
                    }
                }
                if (query == "-3")
                {
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 205;
                    r.StatusData = "OK";
                    sender.Send(r);
                    if (CurrentConnection != null)
                    {
                        CurrentConnection.Stop();
                    }
                }
                if (query == "-4")
                {
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 205;
                    r.StatusData = "OK";
                    sender.Send(r);
                    if (CurrentConnection != null)
                    {
                        CurrentConnection.Pause();
                    }
                }
                if (query == "-5")
                {
                    //
                    // Event
                    //
                    lock (SessionQ)
                    {
                        SessionQ.Enqueue(sender);
                    }
                }
                if (query == "-6" || query == "-7")
                {
                    AVRenderer[] Rs = (AVRenderer[])RendererList.ToArray(typeof(AVRenderer));
                    int i = 0;

                    foreach (AVRenderer cr in Rs)
                    {
                        if (cr.UniqueDeviceName == CurrentRenderer.UniqueDeviceName)
                        {
                            break;
                        }
                        ++i;
                    }
                    if (query == "-6")
                    {
                        if (i != 0)
                        {
                            CurrentRenderer = Rs[i - 1];
                            CurrentConnection = (AVConnection)CurrentRenderer.Connections[0];
                            ShowRenderer();
                        }
                    }
                    else
                    {
                        if (i != Rs.Length - 1)
                        {
                            CurrentRenderer = Rs[i + 1];
                            CurrentConnection = (AVConnection)CurrentRenderer.Connections[0];
                            ShowRenderer();
                        }
                    }
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 200;
                    r.StatusData = "OK";
                    r.StringBuffer = ScrapeUI();
                    sender.Send(r);
                }

                if (query.StartsWith("http://"))
                {
                    r = new HTTPMessage("1.1");
                    r.StatusCode = 205;
                    r.StatusData = "OK";
                    sender.Send(r);
                    if (CurrentRenderer != null)
                    {
                        ResourceBuilder.ResourceAttributes resInfo = new ResourceBuilder.ResourceAttributes();
                        resInfo.contentUri = query;
                        resInfo.protocolInfo = new ProtocolInfoString("http-get:*:*:*");
                        CurrentRenderer.OnCreateConnection += new AVRenderer.ConnectionHandler(CreateConnectionSink);
                        CurrentRenderer.OnRecycledConnection += new AVRenderer.ConnectionHandler(CreateConnectionSink);
                        CurrentRenderer.CreateConnection(new MediaResource[1] { ResourceBuilder.CreateResource(resInfo) }, true);
                    }
                }
            }
        }
    }
}
