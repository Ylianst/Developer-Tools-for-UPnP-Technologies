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
using System.Data;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using System.CodeDom.Compiler;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.Device;

namespace UPnPRenderer
{
    /// <summary>
    /// Summary description for MainForm.
    /// </summary>
    public class RendererForm : System.Windows.Forms.Form
    {
        private bool REPEAT_ALL = false;

        private delegate void VoidDelegate();

        //private string M3U = "";
        private Hashtable FetchTable = Hashtable.Synchronized(new Hashtable());
        private AVConnection _connection;
        private delegate void GetMediaInfoFormDelegate(out UInt32 Tracks, out double Duration);
        private delegate void GetPositionInfoFormDelegate(out int RCount, out double CurrentPosition);
        private Size windowSize;

        private Queue ShuffleQueue = new Queue();
        protected UInt16 LeftChannel = 100;
        protected UInt16 RightChannel = 100;
        private string LastPosition = "";
        protected ArrayList ImageVideoForms = new ArrayList();

        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem3;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem viewControlMenuItem;
        private System.Windows.Forms.MenuItem viewStatusMenuItem;
        private System.Windows.Forms.MenuItem exitMenuItem;
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.Panel controlPanel;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.MenuItem viewConnectionMenuItem;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label contentUriLabel;
        private System.Windows.Forms.Label nextContentUriLabel;
        private System.Windows.Forms.Button setContentUriButton;
        private System.Windows.Forms.Button setNextContentUriButton;
        private System.Windows.Forms.Button playButton;
        private System.Windows.Forms.TrackBar volumeTrackBar;
        private System.Windows.Forms.Button stopButton;
        private System.Windows.Forms.Button pauseButton;
        private System.Windows.Forms.Button muteButton;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.MenuItem menuItem2;
        private System.Windows.Forms.MenuItem menuItem5;
        private System.Windows.Forms.MenuItem menuItem6;
        private System.Windows.Forms.PictureBox mutedPictureBox;
        private System.Windows.Forms.PictureBox mutePictureBox;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.PictureBox pictureBox3;
        private System.Windows.Forms.MenuItem viewMediaPlayerCtrlsMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.MenuItem openFileMenuItem;
        private System.Windows.Forms.PictureBox pictureBox4;
        private System.Windows.Forms.PictureBox pictureBox5;
        private System.Windows.Forms.Panel progressPanel;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.MenuItem viewProgressBarMenuItem;
        private System.Windows.Forms.PictureBox PauseGrey;
        private System.Windows.Forms.PictureBox StopGrey;
        private System.Windows.Forms.PictureBox PlayGrey;
        private System.Windows.Forms.PictureBox PauseGreen;
        private System.Windows.Forms.PictureBox StopGreen;
        private System.Windows.Forms.PictureBox PlayGreen;
        private System.Windows.Forms.Button RecordButton;
        private System.Windows.Forms.PictureBox RecordGrey;
        private System.Windows.Forms.PictureBox RecordRed;
        private System.Windows.Forms.TrackBar leftChannelTrackBar;
        private System.Windows.Forms.TrackBar rightChannelTrackBar;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label PresetLabel;
        private System.Windows.Forms.Label PlayModeLabel;
        private System.Windows.Forms.Label RecordQualityModeLabel;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Panel connectionPanel;
        private System.Windows.Forms.Button VideoButton;
        private System.Windows.Forms.Timer positionTimer;
        private System.Windows.Forms.MenuItem debugMenuItem;
        private System.Windows.Forms.MenuItem menuItem9;
        private System.Windows.Forms.MenuItem helpMenuItem;
        private AxWMPLib.AxWindowsMediaPlayer player;
        private System.ComponentModel.IContainer components;

        public void set_Playing()
        {
            playButton.Image = PlayGreen.Image;
            stopButton.Image = StopGrey.Image;
            pauseButton.Image = PauseGrey.Image;
            RecordButton.Image = RecordGrey.Image;
        }
        public void set_Pause()
        {
            playButton.Image = PlayGrey.Image;
            stopButton.Image = StopGrey.Image;
            pauseButton.Image = PauseGreen.Image;
            RecordButton.Image = RecordGrey.Image;
        }
        public void set_Stop()
        {
            playButton.Image = PlayGrey.Image;
            stopButton.Image = StopGreen.Image;
            pauseButton.Image = PauseGrey.Image;
            RecordButton.Image = RecordGrey.Image;
        }

        public RendererForm(AVConnection connection)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            _connection = connection;
            _connection.OnPlay += new AVConnection.PlayHandler(PlaySink);
            _connection.OnPause += new AVConnection.StopPauseRecordHandler(PauseSink);
            _connection.OnStop += new AVConnection.StopPauseRecordHandler(StopSink);
            _connection.OnNext += new AVConnection.PreviousNextHandler(NextSink);
            _connection.OnPrevious += new AVConnection.PreviousNextHandler(PreviousSink);
            _connection.OnMuteChanged += new AVConnection.MuteChangedHandler(MuteSink);
            _connection.OnVolumeChanged += new AVConnection.VolumeChangedHandler(VolumeSink);
            _connection.OnCurrentURIChanged += new AVConnection.VariableChangedHandler(UriChangeSink);
            _connection.OnNextURIChanged += new AVConnection.VariableChangedHandler(NextUriChangeSink);
            _connection.OnSeek += new AVConnection.SeekHandler(SeekSink);
            _connection.OnRecord += new AVConnection.StopPauseRecordHandler(RecordSink);
            _connection.OnCurrentPreset += new AVConnection.VariableChangedHandler(PresetSink);
            _connection.OnCurrentRecordQualityModeChanged += new AVConnection.VariableChangedHandler(RecModeSink);
            _connection.OnCurrentPlayModeChanged += new AVConnection.VariableChangedHandler(PlayModeSink);

            this.Text = "Media Renderer [" + connection.ID.ToString() + "] <" + connection.WhoCreatedMe.ToString() + ">";
            PresetLabel.Text = connection.CurrentPreset;
            this.RecordQualityModeLabel.Text = DvAVTransport.Enum_CurrentRecordQualityMode_ToString(_connection.CurrentRecordQualityMode);
            this.PlayModeLabel.Text = DvAVTransport.Enum_CurrentPlayMode_ToString(_connection.CurrentPlayMode);

            muteButton.Image = mutePictureBox.Image;
            contentUriLabel.Text = player.URL;
            toolTip.SetToolTip(contentUriLabel, player.URL);

            SetVolume(50, DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER);
            SetVolume(100, DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF);
            SetVolume(100, DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF);
            connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, 50);
            connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF, 100);
            connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF, 100);

            VolumeSink(connection, DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, (UInt16)50);
        }

        private void NextShuffleTrack()
        {
            if (InvokeRequired) { Invoke(new VoidDelegate(NextShuffleTrack)); return; }

            if (ShuffleQueue.Count == 0)
            {
                player.Ctlcontrols.stop();
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
                return;
            }
            else
            {
                int n = (int)ShuffleQueue.Dequeue();
                if (n > 0)
                {
                    for (int i = 0; i < n; ++i)
                    {
                        player.Ctlcontrols.next();
                    }
                }
                if (n < 0)
                {
                    for (int i = 0; i < (System.Math.Abs(n)); ++i)
                    {
                        player.Ctlcontrols.previous();
                    }
                }
            }
            _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.PLAYING;
        }

        protected void PreProcessPlayList(HTTPRequest sender, HTTPMessage M, object Tag)
        {
            //			if(M!=null)
            //			{
            //				if(M.StatusCode==200)
            //				{
            //					M3U = M.StringBuffer;	
            //				}
            //			}
            //			sender.Dispose();
            //			FetchTable.Remove(sender);
        }

        protected void UriChangeSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(UriChangeSink), sender); return; }

            if (sender.ID == _connection.ID)
            {
                if (sender.CurrentURI != null)
                {
                    //					if(sender.InfoString.MimeType.ToLower()=="audio/mpegurl" ||
                    //						sender.CurrentURI.PathAndQuery.ToLower().EndsWith(".m3u"))
                    //					{
                    //						HTTPRequest R = new HTTPRequest();
                    //						R.OnResponse += new HTTPRequest.RequestHandler(PreProcessPlayList);
                    //						FetchTable[R] = R;
                    //						R.PipelineRequest(sender.CurrentURI,sender);
                    //					}
                    contentUriLabel.Text = sender.CurrentURI.AbsoluteUri;
                }
                else
                {
                    contentUriLabel.Text = "";
                }

                if (player.playState == WMPLib.WMPPlayState.wmppsPlaying)
                {
                    StopSink(sender);
                    PlaySink(sender, DvAVTransport.Enum_TransportPlaySpeed._1);
                }
            }
        }
        protected void NextUriChangeSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(NextUriChangeSink), sender); return; }

            if (sender.ID == _connection.ID)
            {
                if (sender.NextURI != null)
                {
                    this.nextContentUriLabel.Text = sender.NextURI.AbsoluteUri;
                }
                else
                {
                    this.nextContentUriLabel.Text = "";
                }
            }
        }
        protected void PresetSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(PresetSink), sender); return; }

            if (sender.ID == _connection.ID) PresetLabel.Text = sender.CurrentPreset;
        }
        protected void PlayModeSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(PlayModeSink), sender); return; }

            if (sender.ID == _connection.ID)
            {
                this.PlayModeLabel.Text = DvAVTransport.Enum_CurrentPlayMode_ToString(sender.CurrentPlayMode);

                //player.PreviewMode = sender.CurrentPlayMode == DvAVTransport.Enum_CurrentPlayMode.INTRO;
                REPEAT_ALL = sender.CurrentPlayMode == DvAVTransport.Enum_CurrentPlayMode.REPEAT_ALL;
            }
        }
        protected void RecModeSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(RecModeSink), sender); return; }

            if (sender.ID == _connection.ID)
            {
                this.RecordQualityModeLabel.Text = DvAVTransport.Enum_CurrentRecordQualityMode_ToString(sender.CurrentRecordQualityMode);
            }
        }

        protected void SeekSink(AVConnection sender, DvAVTransport.Enum_A_ARG_TYPE_SeekMode SeekMode, string Target)
        {
            if (InvokeRequired) { Invoke(new AVConnection.SeekHandler(SeekSink), sender, SeekMode, Target); return; }
            
            DText p = new DText();
            p.ATTRMARK = ":";
            p[0] = Target;

            if ((SeekMode == DvAVTransport.Enum_A_ARG_TYPE_SeekMode.REL_TIME) && (this.m_SeekPositionEnabled))
            {
                TimeSpan ts = new TimeSpan(int.Parse(p[1]), int.Parse(p[2]), int.Parse(p[3]));
                player.Ctlcontrols.currentPosition = ts.TotalSeconds;
            }
        }

        protected void PauseSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(PauseSink), sender); return; }

            if (sender.CurrentTransportState == DvAVTransport.Enum_TransportState.RECORDING)
            {
                set_Pause();
                sender.CurrentTransportState = DvAVTransport.Enum_TransportState.PAUSED_RECORDING;
            }
            else
            {
                player.Ctlcontrols.pause();
            }
        }

        protected void StopSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(StopSink), sender); return; }

            if ((sender.CurrentTransportState == DvAVTransport.Enum_TransportState.RECORDING) ||
                (sender.CurrentTransportState == DvAVTransport.Enum_TransportState.PAUSED_RECORDING))
            {
                set_Stop();
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
            }
            else
            {
                player.Ctlcontrols.stop();
            }
        }

        protected void VolumeSink(AVConnection sender, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.UInt16 DesiredVolume)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VolumeChangedHandler(VolumeSink), sender, Channel, DesiredVolume); return; }

            SetVolume((int)DesiredVolume, Channel);

            switch (Channel)
            {
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF:
                    LeftChannel = DesiredVolume;
                    break;
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF:
                    RightChannel = DesiredVolume;
                    break;
            }
            int balance = ((int)System.Math.Pow(((double)LeftChannel - (double)RightChannel) / (double)10, 4));
            if (LeftChannel > RightChannel) balance = balance * (-1);

            if (Channel != DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER)
            {
                player.settings.balance = balance;
                DesiredVolume = (UInt16)((float)System.Math.Max(LeftChannel, RightChannel) * ((float)sender.GetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER) / (float)100));
            }
            else
            {
                DesiredVolume = (UInt16)((float)System.Math.Max(LeftChannel, RightChannel) * ((float)sender.GetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER) / (float)100));
            }

            //int nv = -1 * (int)(System.Math.Pow(((100 - ((((double)DesiredVolume) / 2.2) + 20)) / 10),4));
            player.settings.volume = DesiredVolume;
        }

        protected void MuteSink(AVConnection sender, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, bool NewMute)
        {
            if (InvokeRequired) { Invoke(new AVConnection.MuteChangedHandler(MuteSink), sender, Channel, NewMute); return; }

            if (Channel == DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER)
            {
                player.settings.mute = (bool)NewMute;
                SetMuteButton((bool)NewMute);
            }
        }

        protected void PlaySink(AVConnection sender, DvAVTransport.Enum_TransportPlaySpeed Speed)
        {
            if (InvokeRequired) { Invoke(new AVConnection.PlayHandler(PlaySink), sender, Speed); return; }

            if (sender.CurrentURI == null)
            {
                player.Ctlcontrols.stop();
                sender.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
                return;
            }

            if (sender.CurrentTransportState == DvAVTransport.Enum_TransportState.PAUSED_PLAYBACK)
            {
                player.Ctlcontrols.play();
            }
            else
            {
                if (sender.CurrentTransportState != DvAVTransport.Enum_TransportState.PLAYING)
                {
                    if (sender.CurrentURI.LocalPath.EndsWith(".m3u", true, System.Threading.Thread.CurrentThread.CurrentUICulture))
                    {
                        lock (this.m_LockRequest)
                        {
                            sender.CurrentTransportState = DvAVTransport.Enum_TransportState.TRANSITIONING;
                            string mediaUri = sender.CurrentURI.AbsoluteUri.Trim();//.ToLower();
                            if (this.m_PlainM3U.ContainsKey(mediaUri))
                            {
                                player.openPlayer((string)this.m_PlainM3U[mediaUri]);

                                StreamReader sr = System.IO.File.OpenText((string)this.m_PlainM3U[mediaUri]);
                                this.m_M3U = sr.ReadToEnd();

                                sr = System.IO.File.OpenText((string)this.m_Metadata[mediaUri]);
                                this.m_METADATA = sr.ReadToEnd();
                                DetermineMediaDuration(this.m_METADATA);
                            }
                            else
                            {
                                HTTPRequest R = new HTTPRequest();
                                R.OnResponse += new HTTPRequest.RequestHandler(Sink_AcquireAndSetPlaylist);
                                this.PlaylistRequests[R] = sender.CurrentURI.AbsoluteUri.Trim();//.ToLower();
                                this.m_LastRequest = R;
                                try
                                {
                                    R.PipelineRequest(new Uri(mediaUri), sender);
                                }
                                catch
                                {
                                    this.m_LastRequest = null;
                                    this.PlaylistRequests.Remove(R);
                                }
                            }
                        }
                    }
                    else
                    {
                        lock (this.m_LockRequest)
                        {
                            sender.CurrentTransportState = DvAVTransport.Enum_TransportState.TRANSITIONING;
                            player.URL = sender.CurrentURI.ToString();
                            player.Ctlcontrols.play();
                        }
                    }

                }
            }
        }

        protected void DetermineMediaDuration(string m3uMetadata)
        {
            int duration = -1;

            if ((m3uMetadata == null) || (m3uMetadata.Trim() == ""))
            {
                this._connection.MediaDuration = new TimeSpan(0);
            }
            else
            {
                duration = 0;

                DText p = new DText();
                p.ATTRMARK = "\n";
                p.MULTMARK = ",";

                p[0] = this.m_METADATA;

                for (int i = 1; i < p.DCOUNT(); i++)
                {
                    string line = p[i].Trim();
                    string dur = p[i, 1];

                    try
                    {
                        int d = int.Parse(dur);
                        if (d > 0)
                        {
                            duration += d;
                        }
                    }
                    catch { }
                }

                this._connection.MediaDuration = new TimeSpan(0, 0, 0, duration, 0);
            }
        }

        protected void DetermineTrackMetadataAndDuration(int i, bool updateMetadata, bool updateDuration)
        {
            DText p;

            p = new DText();
            p.ATTRMARK = "\n";
            p.MULTMARK = ",";
            p.SUBVMARK = "-";
            p[0] = this.m_METADATA;

            string line = p[i].Trim();
            string seconds = p[i, 1].Trim();
            string comment = p[i, 2].Trim();
            string creator = p[i, 2, 1].Trim();
            string title = p[i, 2, 2].Trim();

            int duration = -1;

            if ((creator != "") && (title != ""))
            {
                // keep as is
            }
            else if ((creator == "") || (title == ""))
            {
                if (comment != "")
                {
                    // set comment as title, creator as unknown
                    title = comment;
                    creator = "Unknown Artist";

                    if (seconds != "")
                    {
                        try
                        {
                            int colonPos = seconds.IndexOf(":");
                            duration = int.Parse(seconds.Substring(colonPos + 1).Trim());
                        }
                        catch
                        {
                            duration = -1;
                        }
                    }
                    else
                    {
                        duration = -1;
                    }
                }
                else
                {
                    title = "Unknown Title";
                    creator = "Unknown Artist";
                    duration = -1;
                }
            }

            p = new DText();
            p.ATTRMARK = "\n";
            p[0] = this.m_M3U;
            string uri = p[i].Trim();//.ToLower();

            if (
                (uri.IndexOf(".wmv") > 0) ||
                (uri.IndexOf(".wma") > 0) ||
                (uri.IndexOf(".asf") > 0)
                )
            {
                this.m_SeekPositionEnabled = false;
            }
            else
            {
                this.m_SeekPositionEnabled = true;
            }

            if (updateMetadata)
            {
                string mclass;
                if (
                    (uri.IndexOf(".mpeg") > 0) ||
                    (uri.IndexOf(".wmv") > 0) ||
                    (uri.IndexOf(".avi") > 0) ||
                    (uri.IndexOf(".rm") > 0)
                    )
                {
                    mclass = "object.item.videoItem";
                }
                else if (
                    (uri.IndexOf(".mp3") > 0) ||
                    (uri.IndexOf(".wma") > 0) ||
                    (uri.IndexOf(".wav") > 0) ||
                    (uri.IndexOf(".ra") > 0)
                    )
                {
                    mclass = "object.item.audioItem";
                }
                else
                {
                    mclass = "object.item";
                }

                StringBuilder sb = new StringBuilder(1024);
                sb.AppendFormat("<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"><item id=\"X\" parentID=\"Y\" restricted=\"1\"><dc:title>{0}</dc:title><dc:creator>{1}</dc:creator><upnp:class>{2}</upnp:class></item></DIDL-Lite>", title, creator, mclass);

                try
                {
                    _connection.TrackMetaData = sb.ToString();
                }
                catch (Exception e)
                {
                    OpenSource.Utilities.EventLogger.Log(e, "Error applying metadata.");
                }
            }

            if (updateDuration)
            {
                if (duration < 0)
                {
                    _connection.TrackDuration = new TimeSpan(0);
                }
                else
                {
                    _connection.TrackDuration = new TimeSpan(0, 0, 0, duration, 0);
                }

                int mpdur = (int)player.currentMedia.duration;
                if (mpdur > duration)
                {
                    _connection.TrackDuration = new TimeSpan(0, 0, 0, mpdur, 0);
                }
            }
        }

        protected bool m_SeekPositionEnabled = true;
        protected string m_M3U = "";
        protected string m_METADATA = "";
        protected Hashtable m_PlainM3U = new Hashtable();
        protected Hashtable m_Metadata = new Hashtable();
        protected TempFileCollection m_TFC = new TempFileCollection();
        protected object m_LockRequest = new object();
        protected HTTPRequest m_LastRequest = null;
        protected Hashtable PlaylistRequests = new Hashtable();
        protected void Sink_AcquireAndSetPlaylist(HTTPRequest sender, HTTPMessage M, object Tag)
        {
            AVConnection avc = (AVConnection)Tag;

            lock (this.m_LockRequest)
            {
                if (this.m_LastRequest == sender)
                {
                    this.m_LastRequest = null;

                    if (M != null)
                    {
                        if (M.StatusCode == 200)
                        {
                            // The URI had a response.
                            // If the URI is for an M3U, then strip out the lines that
                            // begin with # and save to a local file so that mediaplayer
                            // uses the local file because it can't handle extended M3U.
                            string uri = ((string)(this.PlaylistRequests[sender]));

                            if (
                                (M.ContentType.IndexOf("audio/mpegurl") > 0) ||
                                (uri.EndsWith(".m3u"))
                                )
                            {
                                DText dt = new DText();
                                dt.ATTRMARK = "\n";
                                dt[0] = M.StringBuffer;

                                StringBuilder plainM3U = new StringBuilder(M.StringBuffer.Length);
                                StringBuilder metadata = new StringBuilder(M.StringBuffer.Length);

                                bool expectingTrackUri = false;

                                string comment = "";
                                string trackUri = "";
                                for (int i = 1; i <= dt.DCOUNT(); i++)
                                {
                                    string line = dt[i].Trim();
                                    //string lower = line.ToLower();

                                    if ((expectingTrackUri) && (line.StartsWith("#")))
                                    {
                                        // expecting URI and got another comment
                                        // TASK: overwrite comment
                                        comment = line;
                                    }
                                    else if ((expectingTrackUri == false) && (line.StartsWith("#")))
                                    {
                                        if (i == 1)
                                        {
                                            expectingTrackUri = false;
                                        }
                                        else
                                        {
                                            // expecting comment and got another comment
                                            // TASK: save comment
                                            comment = line;
                                            expectingTrackUri = true;
                                        }
                                    }
                                    else if ((expectingTrackUri) && (line.StartsWith("#") == false) && (line != ""))
                                    {
                                        // expecting URI and got a URI
                                        // TASK: write comment and URI to stringbuffers

                                        trackUri = line;
                                        metadata.AppendFormat("{0}\n", comment);
                                        plainM3U.AppendFormat("{0}\n", trackUri);

                                        comment = null;
                                        trackUri = null;
                                        expectingTrackUri = false;
                                    }
                                    else if ((expectingTrackUri == false) && (line.StartsWith("#") == false) && (line != ""))
                                    {
                                        // expecting comment and got a URI
                                        // TASK: write empty comment and URI to stringbuffers
                                        trackUri = line;
                                        int slashPos = line.LastIndexOf("/");
                                        int queryPos = line.LastIndexOf("?");

                                        if (slashPos < queryPos)
                                        {
                                            int len = queryPos - slashPos - 1;
                                            comment = line.Substring(slashPos + 1, len);
                                        }
                                        else
                                        {
                                            comment = line.Substring(slashPos + 1);
                                        }

                                        metadata.AppendFormat("#EXTINF:-1,{0}\n", comment);
                                        plainM3U.AppendFormat("{0}\n", trackUri);

                                        comment = null;
                                        trackUri = null;
                                        expectingTrackUri = false;
                                    }
                                }

                                string guid = Guid.NewGuid().ToString();

                                string tfn = guid + ".m3u";
                                this.m_TFC.AddFile(tfn, false);
                                StreamWriter sw = System.IO.File.CreateText(tfn);
                                sw.Write(plainM3U);
                                sw.Flush();
                                sw.Close();
                                this.m_M3U = plainM3U.ToString();
                                this.m_PlainM3U[uri] = tfn;

                                string mfn = guid + ".md";
                                this.m_TFC.AddFile(mfn, false);
                                sw = System.IO.File.CreateText(mfn);
                                sw.Write(metadata);
                                sw.Flush();
                                sw.Close();
                                this.m_METADATA = metadata.ToString();
                                this.m_Metadata[uri] = mfn;

                                //player.FileName = tfn;
                                player.openPlayer(tfn);
                                DetermineMediaDuration(this.m_METADATA);
                            }
                            else
                            {
                                //player.Open(avc.CurrentURI.AbsoluteUri);
                                player.openPlayer(avc.CurrentURI.AbsoluteUri);
                                this.m_M3U = avc.CurrentURI.AbsoluteUri;
                                this.m_METADATA = "";
                                DetermineMediaDuration(this.m_METADATA);
                            }
                        }
                    }
                }
            }

            this.PlaylistRequests.Remove(sender);
            sender.Dispose();
        }

        protected void RecordSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.VariableChangedHandler(RecordSink), sender); return; }

            if (RecordButton.Image != RecordRed.Image)
            {
                playButton.Image = PlayGrey.Image;
                stopButton.Image = StopGrey.Image;
                pauseButton.Image = PauseGrey.Image;

                // NotRecording
                RecordButton.Image = RecordRed.Image;
                sender.CurrentTransportState = DvAVTransport.Enum_TransportState.RECORDING;
            }
        }

        protected void NextSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.PreviousNextHandler(NextSink), sender); return; }

            try
            {
                player.Ctlcontrols.next();
            }
            catch (Exception) { }
        }

        protected void PreviousSink(AVConnection sender)
        {
            if (InvokeRequired) { Invoke(new AVConnection.PreviousNextHandler(PreviousSink), sender); return; }

            try
            {
                player.Ctlcontrols.previous();
            }
            catch (Exception)
            { }
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

            if (this.m_TFC != null)
            {
                this.m_TFC.Delete();
                this.m_TFC = null;
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RendererForm));
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.mainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.openFileMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.exitMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.viewControlMenuItem = new System.Windows.Forms.MenuItem();
            this.viewConnectionMenuItem = new System.Windows.Forms.MenuItem();
            this.viewStatusMenuItem = new System.Windows.Forms.MenuItem();
            this.viewProgressBarMenuItem = new System.Windows.Forms.MenuItem();
            this.viewMediaPlayerCtrlsMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.helpMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.debugMenuItem = new System.Windows.Forms.MenuItem();
            this.controlPanel = new System.Windows.Forms.Panel();
            this.RecordRed = new System.Windows.Forms.PictureBox();
            this.RecordGrey = new System.Windows.Forms.PictureBox();
            this.RecordButton = new System.Windows.Forms.Button();
            this.PauseGrey = new System.Windows.Forms.PictureBox();
            this.StopGrey = new System.Windows.Forms.PictureBox();
            this.PlayGrey = new System.Windows.Forms.PictureBox();
            this.PauseGreen = new System.Windows.Forms.PictureBox();
            this.StopGreen = new System.Windows.Forms.PictureBox();
            this.PlayGreen = new System.Windows.Forms.PictureBox();
            this.pictureBox5 = new System.Windows.Forms.PictureBox();
            this.pictureBox4 = new System.Windows.Forms.PictureBox();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.volumeTrackBar = new System.Windows.Forms.TrackBar();
            this.mutePictureBox = new System.Windows.Forms.PictureBox();
            this.mutedPictureBox = new System.Windows.Forms.PictureBox();
            this.muteButton = new System.Windows.Forms.Button();
            this.pauseButton = new System.Windows.Forms.Button();
            this.stopButton = new System.Windows.Forms.Button();
            this.playButton = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.leftChannelTrackBar = new System.Windows.Forms.TrackBar();
            this.rightChannelTrackBar = new System.Windows.Forms.TrackBar();
            this.connectionPanel = new System.Windows.Forms.Panel();
            this.VideoButton = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.RecordQualityModeLabel = new System.Windows.Forms.Label();
            this.PlayModeLabel = new System.Windows.Forms.Label();
            this.PresetLabel = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.nextContentUriLabel = new System.Windows.Forms.Label();
            this.setNextContentUriButton = new System.Windows.Forms.Button();
            this.setContentUriButton = new System.Windows.Forms.Button();
            this.contentUriLabel = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.progressPanel = new System.Windows.Forms.Panel();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.positionTimer = new System.Windows.Forms.Timer(this.components);
            this.player = new AxWMPLib.AxWindowsMediaPlayer();
            this.controlPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.RecordRed)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.RecordGrey)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PauseGrey)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.StopGrey)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PlayGrey)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PauseGreen)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.StopGreen)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.PlayGreen)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox5)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.mutePictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.mutedPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.leftChannelTrackBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.rightChannelTrackBar)).BeginInit();
            this.connectionPanel.SuspendLayout();
            this.progressPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.player)).BeginInit();
            this.SuspendLayout();
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 423);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(488, 16);
            this.statusBar.TabIndex = 1;
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem3,
            this.menuItem4});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.openFileMenuItem,
            this.menuItem2,
            this.menuItem5,
            this.menuItem6,
            this.exitMenuItem});
            this.menuItem1.Text = "&File";
            // 
            // openFileMenuItem
            // 
            this.openFileMenuItem.Index = 0;
            this.openFileMenuItem.Text = "Open File...";
            this.openFileMenuItem.Click += new System.EventHandler(this.openFileMenuItem_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 1;
            this.menuItem2.Text = "Set &Content URI...";
            this.menuItem2.Click += new System.EventHandler(this.setContentUriButton_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 2;
            this.menuItem5.Text = "Set &Next Content URI...";
            this.menuItem5.Click += new System.EventHandler(this.setNextContentUriButton_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 3;
            this.menuItem6.Text = "-";
            // 
            // exitMenuItem
            // 
            this.exitMenuItem.Index = 4;
            this.exitMenuItem.Text = "E&xit";
            this.exitMenuItem.Click += new System.EventHandler(this.exitMenuItem_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 1;
            this.menuItem3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.viewControlMenuItem,
            this.viewConnectionMenuItem,
            this.viewStatusMenuItem,
            this.viewProgressBarMenuItem,
            this.viewMediaPlayerCtrlsMenuItem});
            this.menuItem3.Text = "&View";
            // 
            // viewControlMenuItem
            // 
            this.viewControlMenuItem.Checked = true;
            this.viewControlMenuItem.Index = 0;
            this.viewControlMenuItem.Text = "&Controls";
            this.viewControlMenuItem.Click += new System.EventHandler(this.viewControlMenuItem_Click);
            // 
            // viewConnectionMenuItem
            // 
            this.viewConnectionMenuItem.Index = 1;
            this.viewConnectionMenuItem.Text = "C&onnection";
            this.viewConnectionMenuItem.Click += new System.EventHandler(this.viewConnectionMenuItem_Click);
            // 
            // viewStatusMenuItem
            // 
            this.viewStatusMenuItem.Checked = true;
            this.viewStatusMenuItem.Index = 2;
            this.viewStatusMenuItem.Text = "&Status Bar";
            this.viewStatusMenuItem.Click += new System.EventHandler(this.viewStatusMenuItem_Click);
            // 
            // viewProgressBarMenuItem
            // 
            this.viewProgressBarMenuItem.Checked = true;
            this.viewProgressBarMenuItem.Index = 3;
            this.viewProgressBarMenuItem.Text = "Progress Bar";
            this.viewProgressBarMenuItem.Click += new System.EventHandler(this.viewProgressBarMenuItem_Click);
            // 
            // viewMediaPlayerCtrlsMenuItem
            // 
            this.viewMediaPlayerCtrlsMenuItem.Index = 4;
            this.viewMediaPlayerCtrlsMenuItem.Text = "Media Controls";
            this.viewMediaPlayerCtrlsMenuItem.Click += new System.EventHandler(this.viewMediaPlayerCtrlsMenuItem_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuItem,
            this.menuItem9,
            this.debugMenuItem});
            this.menuItem4.Text = "&Help";
            // 
            // helpMenuItem
            // 
            this.helpMenuItem.Index = 0;
            this.helpMenuItem.Shortcut = System.Windows.Forms.Shortcut.F1;
            this.helpMenuItem.Text = "&Help Topics";
            this.helpMenuItem.Click += new System.EventHandler(this.helpMenuItem_Click);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 1;
            this.menuItem9.Text = "-";
            // 
            // debugMenuItem
            // 
            this.debugMenuItem.Index = 2;
            this.debugMenuItem.Text = "&Show Debug Information";
            this.debugMenuItem.Click += new System.EventHandler(this.debugMenuItem_Click);
            // 
            // controlPanel
            // 
            this.controlPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.controlPanel.Controls.Add(this.RecordRed);
            this.controlPanel.Controls.Add(this.RecordGrey);
            this.controlPanel.Controls.Add(this.RecordButton);
            this.controlPanel.Controls.Add(this.PauseGrey);
            this.controlPanel.Controls.Add(this.StopGrey);
            this.controlPanel.Controls.Add(this.PlayGrey);
            this.controlPanel.Controls.Add(this.PauseGreen);
            this.controlPanel.Controls.Add(this.StopGreen);
            this.controlPanel.Controls.Add(this.PlayGreen);
            this.controlPanel.Controls.Add(this.pictureBox5);
            this.controlPanel.Controls.Add(this.pictureBox4);
            this.controlPanel.Controls.Add(this.pictureBox3);
            this.controlPanel.Controls.Add(this.pictureBox2);
            this.controlPanel.Controls.Add(this.volumeTrackBar);
            this.controlPanel.Controls.Add(this.mutePictureBox);
            this.controlPanel.Controls.Add(this.mutedPictureBox);
            this.controlPanel.Controls.Add(this.muteButton);
            this.controlPanel.Controls.Add(this.pauseButton);
            this.controlPanel.Controls.Add(this.stopButton);
            this.controlPanel.Controls.Add(this.playButton);
            this.controlPanel.Controls.Add(this.pictureBox1);
            this.controlPanel.Controls.Add(this.leftChannelTrackBar);
            this.controlPanel.Controls.Add(this.rightChannelTrackBar);
            this.controlPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.controlPanel.Location = new System.Drawing.Point(0, 0);
            this.controlPanel.Name = "controlPanel";
            this.controlPanel.Size = new System.Drawing.Size(488, 40);
            this.controlPanel.TabIndex = 4;
            // 
            // RecordRed
            // 
            this.RecordRed.Image = ((System.Drawing.Image)(resources.GetObject("RecordRed.Image")));
            this.RecordRed.Location = new System.Drawing.Point(152, 48);
            this.RecordRed.Name = "RecordRed";
            this.RecordRed.Size = new System.Drawing.Size(32, 24);
            this.RecordRed.TabIndex = 27;
            this.RecordRed.TabStop = false;
            // 
            // RecordGrey
            // 
            this.RecordGrey.BackColor = System.Drawing.Color.Transparent;
            this.RecordGrey.Image = ((System.Drawing.Image)(resources.GetObject("RecordGrey.Image")));
            this.RecordGrey.Location = new System.Drawing.Point(152, 72);
            this.RecordGrey.Name = "RecordGrey";
            this.RecordGrey.Size = new System.Drawing.Size(24, 24);
            this.RecordGrey.TabIndex = 26;
            this.RecordGrey.TabStop = false;
            // 
            // RecordButton
            // 
            this.RecordButton.Image = ((System.Drawing.Image)(resources.GetObject("RecordButton.Image")));
            this.RecordButton.Location = new System.Drawing.Point(64, 3);
            this.RecordButton.Name = "RecordButton";
            this.RecordButton.Size = new System.Drawing.Size(32, 32);
            this.RecordButton.TabIndex = 25;
            this.RecordButton.Click += new System.EventHandler(this.RecordButton_Click);
            // 
            // PauseGrey
            // 
            this.PauseGrey.Image = ((System.Drawing.Image)(resources.GetObject("PauseGrey.Image")));
            this.PauseGrey.Location = new System.Drawing.Point(120, 72);
            this.PauseGrey.Name = "PauseGrey";
            this.PauseGrey.Size = new System.Drawing.Size(24, 24);
            this.PauseGrey.TabIndex = 24;
            this.PauseGrey.TabStop = false;
            this.PauseGrey.Visible = false;
            // 
            // StopGrey
            // 
            this.StopGrey.Image = ((System.Drawing.Image)(resources.GetObject("StopGrey.Image")));
            this.StopGrey.Location = new System.Drawing.Point(88, 72);
            this.StopGrey.Name = "StopGrey";
            this.StopGrey.Size = new System.Drawing.Size(24, 24);
            this.StopGrey.TabIndex = 23;
            this.StopGrey.TabStop = false;
            this.StopGrey.Visible = false;
            // 
            // PlayGrey
            // 
            this.PlayGrey.Image = ((System.Drawing.Image)(resources.GetObject("PlayGrey.Image")));
            this.PlayGrey.Location = new System.Drawing.Point(56, 72);
            this.PlayGrey.Name = "PlayGrey";
            this.PlayGrey.Size = new System.Drawing.Size(24, 24);
            this.PlayGrey.TabIndex = 22;
            this.PlayGrey.TabStop = false;
            this.PlayGrey.Visible = false;
            // 
            // PauseGreen
            // 
            this.PauseGreen.Image = ((System.Drawing.Image)(resources.GetObject("PauseGreen.Image")));
            this.PauseGreen.Location = new System.Drawing.Point(120, 48);
            this.PauseGreen.Name = "PauseGreen";
            this.PauseGreen.Size = new System.Drawing.Size(24, 24);
            this.PauseGreen.TabIndex = 21;
            this.PauseGreen.TabStop = false;
            this.PauseGreen.Visible = false;
            // 
            // StopGreen
            // 
            this.StopGreen.Image = ((System.Drawing.Image)(resources.GetObject("StopGreen.Image")));
            this.StopGreen.Location = new System.Drawing.Point(88, 48);
            this.StopGreen.Name = "StopGreen";
            this.StopGreen.Size = new System.Drawing.Size(24, 24);
            this.StopGreen.TabIndex = 20;
            this.StopGreen.TabStop = false;
            this.StopGreen.Visible = false;
            // 
            // PlayGreen
            // 
            this.PlayGreen.Image = ((System.Drawing.Image)(resources.GetObject("PlayGreen.Image")));
            this.PlayGreen.Location = new System.Drawing.Point(56, 48);
            this.PlayGreen.Name = "PlayGreen";
            this.PlayGreen.Size = new System.Drawing.Size(24, 24);
            this.PlayGreen.TabIndex = 19;
            this.PlayGreen.TabStop = false;
            this.PlayGreen.Visible = false;
            // 
            // pictureBox5
            // 
            this.pictureBox5.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox5.Image")));
            this.pictureBox5.Location = new System.Drawing.Point(338, 8);
            this.pictureBox5.Name = "pictureBox5";
            this.pictureBox5.Size = new System.Drawing.Size(11, 17);
            this.pictureBox5.TabIndex = 12;
            this.pictureBox5.TabStop = false;
            // 
            // pictureBox4
            // 
            this.pictureBox4.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox4.Image")));
            this.pictureBox4.Location = new System.Drawing.Point(408, 9);
            this.pictureBox4.Name = "pictureBox4";
            this.pictureBox4.Size = new System.Drawing.Size(18, 15);
            this.pictureBox4.TabIndex = 11;
            this.pictureBox4.TabStop = false;
            // 
            // pictureBox3
            // 
            this.pictureBox3.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox3.Image")));
            this.pictureBox3.Location = new System.Drawing.Point(204, 6);
            this.pictureBox3.Name = "pictureBox3";
            this.pictureBox3.Size = new System.Drawing.Size(19, 24);
            this.pictureBox3.TabIndex = 9;
            this.pictureBox3.TabStop = false;
            // 
            // pictureBox2
            // 
            this.pictureBox2.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox2.Image")));
            this.pictureBox2.Location = new System.Drawing.Point(304, 6);
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.Size = new System.Drawing.Size(24, 24);
            this.pictureBox2.TabIndex = 8;
            this.pictureBox2.TabStop = false;
            // 
            // volumeTrackBar
            // 
            this.volumeTrackBar.Location = new System.Drawing.Point(216, 0);
            this.volumeTrackBar.Maximum = 100;
            this.volumeTrackBar.Name = "volumeTrackBar";
            this.volumeTrackBar.Size = new System.Drawing.Size(96, 45);
            this.volumeTrackBar.TabIndex = 4;
            this.volumeTrackBar.TickFrequency = 10;
            this.toolTip.SetToolTip(this.volumeTrackBar, "Volume");
            this.volumeTrackBar.Value = 50;
            this.volumeTrackBar.Scroll += new System.EventHandler(this.volumeTrackBar_Scroll);
            // 
            // mutePictureBox
            // 
            this.mutePictureBox.Image = ((System.Drawing.Image)(resources.GetObject("mutePictureBox.Image")));
            this.mutePictureBox.Location = new System.Drawing.Point(216, 72);
            this.mutePictureBox.Name = "mutePictureBox";
            this.mutePictureBox.Size = new System.Drawing.Size(24, 24);
            this.mutePictureBox.TabIndex = 7;
            this.mutePictureBox.TabStop = false;
            this.mutePictureBox.Visible = false;
            // 
            // mutedPictureBox
            // 
            this.mutedPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("mutedPictureBox.Image")));
            this.mutedPictureBox.Location = new System.Drawing.Point(192, 72);
            this.mutedPictureBox.Name = "mutedPictureBox";
            this.mutedPictureBox.Size = new System.Drawing.Size(24, 24);
            this.mutedPictureBox.TabIndex = 6;
            this.mutedPictureBox.TabStop = false;
            this.mutedPictureBox.Visible = false;
            // 
            // muteButton
            // 
            this.muteButton.Image = ((System.Drawing.Image)(resources.GetObject("muteButton.Image")));
            this.muteButton.Location = new System.Drawing.Point(168, 3);
            this.muteButton.Name = "muteButton";
            this.muteButton.Size = new System.Drawing.Size(32, 32);
            this.muteButton.TabIndex = 5;
            this.toolTip.SetToolTip(this.muteButton, "Mute");
            this.muteButton.Click += new System.EventHandler(this.muteButton_Click);
            // 
            // pauseButton
            // 
            this.pauseButton.Image = ((System.Drawing.Image)(resources.GetObject("pauseButton.Image")));
            this.pauseButton.Location = new System.Drawing.Point(128, 3);
            this.pauseButton.Name = "pauseButton";
            this.pauseButton.Size = new System.Drawing.Size(32, 32);
            this.pauseButton.TabIndex = 3;
            this.toolTip.SetToolTip(this.pauseButton, "Pause");
            this.pauseButton.Click += new System.EventHandler(this.pauseButton_Click);
            // 
            // stopButton
            // 
            this.stopButton.Image = ((System.Drawing.Image)(resources.GetObject("stopButton.Image")));
            this.stopButton.Location = new System.Drawing.Point(96, 3);
            this.stopButton.Name = "stopButton";
            this.stopButton.Size = new System.Drawing.Size(32, 32);
            this.stopButton.TabIndex = 2;
            this.toolTip.SetToolTip(this.stopButton, "Stop");
            this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // playButton
            // 
            this.playButton.Image = ((System.Drawing.Image)(resources.GetObject("playButton.Image")));
            this.playButton.Location = new System.Drawing.Point(32, 3);
            this.playButton.Name = "playButton";
            this.playButton.Size = new System.Drawing.Size(32, 32);
            this.playButton.TabIndex = 1;
            this.toolTip.SetToolTip(this.playButton, "Play");
            this.playButton.Click += new System.EventHandler(this.playButton_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(2, 3);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(32, 32);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // leftChannelTrackBar
            // 
            this.leftChannelTrackBar.Location = new System.Drawing.Point(340, 0);
            this.leftChannelTrackBar.Maximum = 100;
            this.leftChannelTrackBar.Name = "leftChannelTrackBar";
            this.leftChannelTrackBar.Size = new System.Drawing.Size(56, 45);
            this.leftChannelTrackBar.TabIndex = 10;
            this.leftChannelTrackBar.TickFrequency = 20;
            this.toolTip.SetToolTip(this.leftChannelTrackBar, "Left Audio Channel Volume");
            this.leftChannelTrackBar.Value = 100;
            this.leftChannelTrackBar.Scroll += new System.EventHandler(this.LeftVolumeChanged);
            // 
            // rightChannelTrackBar
            // 
            this.rightChannelTrackBar.Location = new System.Drawing.Point(416, 0);
            this.rightChannelTrackBar.Maximum = 100;
            this.rightChannelTrackBar.Name = "rightChannelTrackBar";
            this.rightChannelTrackBar.Size = new System.Drawing.Size(56, 45);
            this.rightChannelTrackBar.TabIndex = 28;
            this.rightChannelTrackBar.TickFrequency = 20;
            this.toolTip.SetToolTip(this.rightChannelTrackBar, "Right Audio Channel Volume");
            this.rightChannelTrackBar.Value = 100;
            this.rightChannelTrackBar.Scroll += new System.EventHandler(this.RightVolumeChanged);
            // 
            // connectionPanel
            // 
            this.connectionPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.connectionPanel.Controls.Add(this.VideoButton);
            this.connectionPanel.Controls.Add(this.label5);
            this.connectionPanel.Controls.Add(this.label4);
            this.connectionPanel.Controls.Add(this.RecordQualityModeLabel);
            this.connectionPanel.Controls.Add(this.PlayModeLabel);
            this.connectionPanel.Controls.Add(this.PresetLabel);
            this.connectionPanel.Controls.Add(this.label3);
            this.connectionPanel.Controls.Add(this.nextContentUriLabel);
            this.connectionPanel.Controls.Add(this.setNextContentUriButton);
            this.connectionPanel.Controls.Add(this.setContentUriButton);
            this.connectionPanel.Controls.Add(this.contentUriLabel);
            this.connectionPanel.Controls.Add(this.label2);
            this.connectionPanel.Controls.Add(this.label1);
            this.connectionPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.connectionPanel.Location = new System.Drawing.Point(0, 40);
            this.connectionPanel.Name = "connectionPanel";
            this.connectionPanel.Size = new System.Drawing.Size(488, 128);
            this.connectionPanel.TabIndex = 5;
            this.connectionPanel.Visible = false;
            // 
            // VideoButton
            // 
            this.VideoButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.VideoButton.Location = new System.Drawing.Point(448, 80);
            this.VideoButton.Name = "VideoButton";
            this.VideoButton.Size = new System.Drawing.Size(32, 32);
            this.VideoButton.TabIndex = 13;
            this.VideoButton.Text = ">>";
            this.VideoButton.Click += new System.EventHandler(this.VideoButton_Click);
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 104);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(80, 16);
            this.label5.TabIndex = 12;
            this.label5.Text = "RecQuality";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(8, 80);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(72, 16);
            this.label4.TabIndex = 11;
            this.label4.Text = "Play Mode";
            // 
            // RecordQualityModeLabel
            // 
            this.RecordQualityModeLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.RecordQualityModeLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.RecordQualityModeLabel.Location = new System.Drawing.Point(104, 104);
            this.RecordQualityModeLabel.Name = "RecordQualityModeLabel";
            this.RecordQualityModeLabel.Size = new System.Drawing.Size(336, 16);
            this.RecordQualityModeLabel.TabIndex = 10;
            // 
            // PlayModeLabel
            // 
            this.PlayModeLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.PlayModeLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.PlayModeLabel.Location = new System.Drawing.Point(104, 80);
            this.PlayModeLabel.Name = "PlayModeLabel";
            this.PlayModeLabel.Size = new System.Drawing.Size(336, 16);
            this.PlayModeLabel.TabIndex = 9;
            // 
            // PresetLabel
            // 
            this.PresetLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.PresetLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.PresetLabel.Location = new System.Drawing.Point(104, 56);
            this.PresetLabel.Name = "PresetLabel";
            this.PresetLabel.Size = new System.Drawing.Size(336, 17);
            this.PresetLabel.TabIndex = 8;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 58);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(80, 16);
            this.label3.TabIndex = 7;
            this.label3.Text = "Current Preset";
            // 
            // nextContentUriLabel
            // 
            this.nextContentUriLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.nextContentUriLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.nextContentUriLabel.Location = new System.Drawing.Point(104, 31);
            this.nextContentUriLabel.Name = "nextContentUriLabel";
            this.nextContentUriLabel.Size = new System.Drawing.Size(336, 16);
            this.nextContentUriLabel.TabIndex = 5;
            // 
            // setNextContentUriButton
            // 
            this.setNextContentUriButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.setNextContentUriButton.Location = new System.Drawing.Point(447, 29);
            this.setNextContentUriButton.Name = "setNextContentUriButton";
            this.setNextContentUriButton.Size = new System.Drawing.Size(32, 18);
            this.setNextContentUriButton.TabIndex = 4;
            this.setNextContentUriButton.Text = "Set";
            this.setNextContentUriButton.Click += new System.EventHandler(this.setNextContentUriButton_Click);
            // 
            // setContentUriButton
            // 
            this.setContentUriButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.setContentUriButton.Location = new System.Drawing.Point(447, 7);
            this.setContentUriButton.Name = "setContentUriButton";
            this.setContentUriButton.Size = new System.Drawing.Size(32, 18);
            this.setContentUriButton.TabIndex = 3;
            this.setContentUriButton.Text = "Set";
            this.setContentUriButton.Click += new System.EventHandler(this.setContentUriButton_Click);
            // 
            // contentUriLabel
            // 
            this.contentUriLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.contentUriLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.contentUriLabel.Location = new System.Drawing.Point(104, 8);
            this.contentUriLabel.Name = "contentUriLabel";
            this.contentUriLabel.Size = new System.Drawing.Size(336, 16);
            this.contentUriLabel.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(8, 32);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(96, 16);
            this.label2.TabIndex = 1;
            this.label2.Text = "Next Content URI:";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(88, 16);
            this.label1.TabIndex = 0;
            this.label1.Text = "Content URI:";
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "Media Files (*.mp3, *.avi, *.wmv, *.wma, *.wav)|*.mp3;*.avi;*.wmv;*.wma;*.wav";
            this.openFileDialog.Title = "Open Media File";
            // 
            // progressPanel
            // 
            this.progressPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.progressPanel.Controls.Add(this.progressBar);
            this.progressPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.progressPanel.Location = new System.Drawing.Point(0, 409);
            this.progressPanel.Name = "progressPanel";
            this.progressPanel.Size = new System.Drawing.Size(488, 14);
            this.progressPanel.TabIndex = 6;
            // 
            // progressBar
            // 
            this.progressBar.Dock = System.Windows.Forms.DockStyle.Fill;
            this.progressBar.Location = new System.Drawing.Point(0, 0);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(484, 10);
            this.progressBar.TabIndex = 0;
            // 
            // positionTimer
            // 
            this.positionTimer.Enabled = true;
            this.positionTimer.Interval = 1000;
            this.positionTimer.Tick += new System.EventHandler(this.positionTimer_Tick);
            // 
            // player
            // 
            this.player.Dock = System.Windows.Forms.DockStyle.Fill;
            this.player.Enabled = true;
            this.player.Location = new System.Drawing.Point(0, 168);
            this.player.Name = "player";
            this.player.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("player.OcxState")));
            this.player.Size = new System.Drawing.Size(488, 241);
            this.player.TabIndex = 7;
            this.player.DoubleClickEvent += new AxWMPLib._WMPOCXEvents_DoubleClickEventHandler(this.player_DoubleClickEvent);
            this.player.MouseDownEvent += new AxWMPLib._WMPOCXEvents_MouseDownEventHandler(this.player_MouseDownEvent);
            this.player.OpenStateChange += new AxWMPLib._WMPOCXEvents_OpenStateChangeEventHandler(this.player_OpenStateChange);
            this.player.PlayStateChange += new AxWMPLib._WMPOCXEvents_PlayStateChangeEventHandler(this.player_PlayStateChange);
            // 
            // RendererForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(488, 439);
            this.Controls.Add(this.player);
            this.Controls.Add(this.progressPanel);
            this.Controls.Add(this.connectionPanel);
            this.Controls.Add(this.controlPanel);
            this.Controls.Add(this.statusBar);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "RendererForm";
            this.Text = "AV Media Renderer";
            this.Closed += new System.EventHandler(this.OnFormClose);
            this.Load += new System.EventHandler(this.RendererForm_Load);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.RendererForm_HelpRequested);
            this.controlPanel.ResumeLayout(false);
            this.controlPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.RecordRed)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.RecordGrey)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PauseGrey)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.StopGrey)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PlayGrey)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PauseGreen)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.StopGreen)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.PlayGreen)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox5)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.volumeTrackBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.mutePictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.mutedPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.leftChannelTrackBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.rightChannelTrackBar)).EndInit();
            this.connectionPanel.ResumeLayout(false);
            this.progressPanel.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.player)).EndInit();
            this.ResumeLayout(false);

        }
        #endregion

        private void exitMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void viewControlMenuItem_Click(object sender, System.EventArgs e)
        {
            viewControlMenuItem.Checked = !viewControlMenuItem.Checked;
            controlPanel.Visible = viewControlMenuItem.Checked;
        }

        private void viewStatusMenuItem_Click(object sender, System.EventArgs e)
        {
            viewStatusMenuItem.Checked = !viewStatusMenuItem.Checked;
            statusBar.Visible = viewStatusMenuItem.Checked;
        }

        private void viewConnectionMenuItem_Click(object sender, System.EventArgs e)
        {
            viewConnectionMenuItem.Checked = !viewConnectionMenuItem.Checked;
            connectionPanel.Visible = viewConnectionMenuItem.Checked;
        }

        private void setContentUriButton_Click(object sender, System.EventArgs e)
        {
            ContentUriForm form = new ContentUriForm();
            form.Title = "Set Content URI";
            form.Question = "Content URI";
            //if (Transport.AVTransportURI != null) form.URI = Transport.AVTransportURI.ToString();
            DialogResult r = form.ShowDialog(this);
            if (r == DialogResult.OK)
            {
                try
                {
                    this._connection.CurrentURI = new Uri(form.URI);
                    //SetAVTransportURISink(0,form.URI);
                }
                catch { }
            }
        }

        private void setNextContentUriButton_Click(object sender, System.EventArgs e)
        {
            ContentUriForm form = new ContentUriForm();
            form.Title = "Set Next Content URI";
            form.Question = "Next Content URI";
            //if (Transport.NextAVTransportURI != null) form.URI = Transport.NextAVTransportURI.ToString();
            DialogResult r = form.ShowDialog(this);
            if (r == DialogResult.OK)
            {
                try
                {
                    //SetNextAVTransportURISink(0,form.URI);
                }
                catch { }
            }
        }

        private void muteButton_Click(object sender, System.EventArgs e)
        {
            _connection.SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, (muteButton.Image == mutePictureBox.Image));
            this.MuteSink(this._connection, DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, (muteButton.Image == mutePictureBox.Image));
        }

        private void playButton_Click(object sender, System.EventArgs e)
        {
            PlaySink(this._connection, DvAVTransport.Enum_TransportPlaySpeed._1);
        }

        private void stopButton_Click(object sender, System.EventArgs e)
        {
            StopSink(this._connection);
        }

        private void pauseButton_Click(object sender, System.EventArgs e)
        {
            PauseSink(this._connection);
        }

        private void volumeTrackBar_Scroll(object sender, System.EventArgs e)
        {
            _connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, (UInt16)volumeTrackBar.Value);
            this.VolumeSink(_connection, DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, (UInt16)volumeTrackBar.Value);
        }

        public void SetVolume(int volume, DvRenderingControl.Enum_A_ARG_TYPE_Channel channel)
        {
            switch (channel)
            {
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER:
                    volumeTrackBar.Value = volume;
                    break;
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF:
                    this.leftChannelTrackBar.Value = volume;
                    break;
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF:
                    this.rightChannelTrackBar.Value = volume;
                    break;
            }
        }

        public void SetMuteButton(bool mute)
        {
            if (mute == true)
            {
                muteButton.Image = mutedPictureBox.Image;
            }
            else
            {
                muteButton.Image = mutePictureBox.Image;
            }
        }

        public void SetContentUri(string Current, string Next)
        {
            contentUriLabel.Text = Current;
            toolTip.SetToolTip(contentUriLabel, Current);
            nextContentUriLabel.Text = Next;
            toolTip.SetToolTip(nextContentUriLabel, Next);
        }

        private void viewMediaPlayerCtrlsMenuItem_Click(object sender, System.EventArgs e)
        {
            viewMediaPlayerCtrlsMenuItem.Checked = !viewMediaPlayerCtrlsMenuItem.Checked;

            if (viewMediaPlayerCtrlsMenuItem.Checked)
            {
                player.uiMode = "full";
            }
            else
            {
                player.uiMode = "none";
            }
        }

        private void viewMediaPlayerDisplayMenuItem_Click(object sender, System.EventArgs e)
        {
            //viewMediaPlayerDisplayMenuItem.Checked = !viewMediaPlayerDisplayMenuItem.Checked;
            //player.ShowDisplay = viewMediaPlayerDisplayMenuItem.Checked;
        }

        private void openFileMenuItem_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openFileDialog.ShowDialog(this);
            if (r == DialogResult.OK)
            {
                try
                {
                    FileInfo fi = new FileInfo(openFileDialog.FileName);
                    this._connection.CurrentURI = new Uri(fi.FullName);
                    //this.SetAVTransportURISink(0,openFileDialog.FileName);
                }
                catch { }
            }
        }


        private void positionTimer_Tick(object sender, System.EventArgs e)
        {
            if ((player.Ctlcontrols.currentPosition > 0) && (player.Ctlcontrols.currentPosition <= player.currentMedia.duration))
            {
                progressBar.Maximum = (int)player.currentMedia.duration;
                progressBar.Value = (int)player.Ctlcontrols.currentPosition;

                // Build Position in ProperFormat of
                //  "00:00:00"

                TimeSpan TS = new TimeSpan(0, 0, (int)player.Ctlcontrols.currentPosition);
                string tf = string.Format("{0:00}", TS.Hours) + ":" + string.Format("{0:00}", TS.Minutes) + ":" + string.Format("{0:00}", TS.Seconds);

                if (LastPosition != tf)
                {
                    LastPosition = tf;
                    _connection.CurrentRelativeTimePosition = TS;
                }

                _connection.TrackDuration = new TimeSpan(0, 0, 0, (int)(player.currentMedia.duration), 0);
            }
        }


        private void viewProgressBarMenuItem_Click(object sender, System.EventArgs e)
        {
            viewProgressBarMenuItem.Checked = !viewProgressBarMenuItem.Checked;
            progressBar.Visible = viewProgressBarMenuItem.Checked;
        }

        private void DetermineTrackUri(int i)
        {
            DText p = new DText();
            p.ATTRMARK = "\n";
            p[0] = this.m_M3U;

            string uri = p[i].Trim();
            _connection.TrackURI = new Uri(uri);
        }

        private void OnNewStream(object sender, System.EventArgs e)
        {
            /*
			int noft = 0;
			int ct = 0;
            if (player.settings.playCount.Ctlcontrols.GetCurrentEntry() == -1)
			{
				_connection.CurrentTrack = 1;
				ct = 1;
			}
			else
			{
				_connection.CurrentTrack = (UInt32)player.GetCurrentEntry();
				ct = (Int32)player.GetCurrentEntry();
			}

            if (player.currentPlaylist.count != 0)
			{
				noft = (System.Int32)player.currentPlaylist.count;
				_connection.NumberOfTracks = (System.UInt32)player.currentPlaylist.count;
			}
			else
			{
				noft = 1;
				_connection.NumberOfTracks = (UInt32)1;
			}
		
			// Update TrackUri
			if (noft == 1)
			{
				_connection.TrackURI = _connection.CurrentURI;
			}
			else
			{
				this.DetermineTrackUri(ct);
			}

			this.DetermineTrackMetadataAndDuration(ct, true, true);
            */
        }

        private void OnFormClose(object sender, System.EventArgs e)
        {
            foreach (ImageVideoForm ivf in ImageVideoForms)
            {
                ivf.Dispose();
            }
            _connection.dispose();
        }

        private void RecordButton_Click(object sender, System.EventArgs e)
        {
            RecordSink(this._connection);
        }

        private void LeftVolumeChanged(object sender, System.EventArgs e)
        {
            _connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF, (UInt16)leftChannelTrackBar.Value);
            this.VolumeSink(_connection, DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF, (UInt16)leftChannelTrackBar.Value);
        }

        private void RightVolumeChanged(object sender, System.EventArgs e)
        {
            _connection.SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF, (UInt16)rightChannelTrackBar.Value);
            this.VolumeSink(_connection, DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF, (UInt16)rightChannelTrackBar.Value);
        }

        private void VideoButton_Click(object sender, System.EventArgs e)
        {
            ImageVideoForm ivf = new ImageVideoForm(this, _connection);
            ImageVideoForms.Add(ivf);
            ivf.Show();
        }
        internal void RemoveMe(ImageVideoForm ivf)
        {
            ImageVideoForms.Remove(ivf);
        }

        private void helpMenuItem_Click(object sender, System.EventArgs e)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "AV MediaRenderer");
        }

        private void RendererForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
        {
            Help.ShowHelp(this, "ToolsHelp.chm", HelpNavigator.KeywordIndex, "AV MediaRenderer");
        }

        private void debugMenuItem_Click(object sender, System.EventArgs e)
        {
            OpenSource.Utilities.InstanceTracker.Display();
        }

        private void player_DoubleClickEvent(object sender, AxWMPLib._WMPOCXEvents_DoubleClickEvent e)
        {
            if (this.FormBorderStyle == FormBorderStyle.SizableToolWindow)
            {
                this.Size = windowSize;
                this.FormBorderStyle = FormBorderStyle.Sizable;
                this.Menu = mainMenu;
                controlPanel.Visible = viewControlMenuItem.Checked;
                connectionPanel.Visible = viewConnectionMenuItem.Checked;
                statusBar.Visible = viewStatusMenuItem.Checked;
            }
            else
            {
                windowSize = this.Size;
                Size size = player.Size;
                controlPanel.Visible = false;
                connectionPanel.Visible = false;
                statusBar.Visible = false;
                this.Menu = null;
                this.FormBorderStyle = FormBorderStyle.SizableToolWindow;
                this.Size = size;
            }
        }

        private void player_PlayStateChange(object sender, AxWMPLib._WMPOCXEvents_PlayStateChangeEvent e)
        {
            WMPLib.WMPPlayState state = player.playState;
            if (state == WMPLib.WMPPlayState.wmppsMediaEnded)
            {
                statusBar.Text = "Transport state now closed";
                this.set_Stop();
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
            }
            if (state == WMPLib.WMPPlayState.wmppsPaused)
            {
                this.set_Pause();
                statusBar.Text = "Transport state now paused";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.PAUSED_PLAYBACK;
            }
            if (state == WMPLib.WMPPlayState.wmppsPlaying)
            {
                this.set_Playing();
                statusBar.Text = "Transport state now playing";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.PLAYING;

            }
            if (state == WMPLib.WMPPlayState.wmppsScanForward)
            {
                statusBar.Text = "Transport state now seeking";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.TRANSITIONING;
            }
            if (state == WMPLib.WMPPlayState.wmppsScanReverse)
            {
                statusBar.Text = "Transport state now seeking";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.TRANSITIONING;
            }
            if (state == WMPLib.WMPPlayState.wmppsStopped)
            {
                this.set_Stop();
                if ((int)player.currentMedia.duration == (int)player.Ctlcontrols.currentPosition)
                {
                    // End of Stream
                    /*
                    if(Transport.NextAVTransportURI!=null)
                    {
                        Transport.AVTransportURI = Transport.NextAVTransportURI;
                        Transport.NextAVTransportURI = null;
                        if(Transport.AVTransportURI!=null)
                        {
                            Transport.TransportState = DvAVTransport.Enum_TransportState.STOPPED;
                            Transport.Play(0,AVTransport.Enum_TransportPlaySpeed._1);
                            return;
                        }
                    }
                    */

                }

                statusBar.Text = "Transport state now stopped";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
            }
            if (state == WMPLib.WMPPlayState.wmppsWaiting)
            {
                statusBar.Text = "Transport state now seeking";
                _connection.CurrentTransportState = DvAVTransport.Enum_TransportState.TRANSITIONING;
            }

            // Trigger a UPnP event
            //				string statestr = "NO_MEDIA_PRESENT";
            /*
                switch (Transport.TransportState) 
                {
                    case DvAVTransport.Enum_TransportState.PLAYING:
                        statestr = "PLAYING";
                        set_Playing();
                        break;
                    case DvAVTransport.Enum_TransportState.PAUSED_PLAYBACK:
                        statestr = "PAUSED_PLAYBACK";
                        set_Pause();
                        break;
                    case DvAVTransport.Enum_TransportState.PAUSED_RECORDING:
                        statestr = "PAUSED_RECORDING";
                        break;
                    case DvAVTransport.Enum_TransportState.STOPPED:
                        statestr = "STOPPED";
                        set_Stop();
                        break;
                    case DvAVTransport.Enum_TransportState.SEEKING:
                        statestr = "SEEKING";
                        break;
                    case DvAVTransport.Enum_TransportState.RECORDING:
                        statestr = "RECORDING";
                        break;
                }

                System.IO.StringWriter SW = new System.IO.StringWriter();
                XmlTextWriter XDoc = new XmlTextWriter(SW);				
                XDoc.WriteStartDocument();
                XDoc.WriteStartElement("InstanceId");
                XDoc.WriteAttributeString("val","0");
                XDoc.WriteStartElement("TransportState");
                XDoc.WriteAttributeString("val",statestr);
                XDoc.WriteEndElement();
                XDoc.WriteEndElement();
                XDoc.WriteEndDocument();
                XDoc.Flush();
                Transport.Evented_LastChange = SW.ToString();
                XDoc.Close();*/
        }

        private void player_MouseDownEvent(object sender, AxWMPLib._WMPOCXEvents_MouseDownEvent e)
        {
            // Nop
        }

        private void player_OpenStateChange(object sender, AxWMPLib._WMPOCXEvents_OpenStateChangeEvent e)
        {
            WMPLib.WMPPlayState x = (WMPLib.WMPPlayState)e.newState;
            if (x == WMPLib.WMPPlayState.wmppsStopped) this._connection.CurrentTransportState = DvAVTransport.Enum_TransportState.STOPPED;
        }

        private void player_EndOfStream(object sender, AxWMPLib._WMPOCXEvents_EndOfStreamEvent e)
        {

        }

        private void RendererForm_Load(object sender, EventArgs e)
        {
            player.uiMode = "none";
        }

        /*
        private void NextSink(System.UInt32 InstanceID)
        {
            object[] args = new object[1];
            args[0] = InstanceID;
            Invoke(new DvAVTransport.Delegate_Next(NextSinkEx),args);
        }
        private void NextSinkEx(System.UInt32 InstanceID)
        {
            player.Next();
        }
        private void PreviousSink(System.UInt32 InstanceID)
        {
            object[] args = new object[1];
            args[0] = InstanceID;
            Invoke(new DvAVTransport.Delegate_Previous(PreviousSinkEx),args);
        }
        private void PreviousSinkEx(System.UInt32 InstanceID)
        {
            player.Previous();
        }
        private void SetNextAVTransportURISink(System.UInt32 InstanceID, System.String NextURI)
        {
            Transport.NextAVTransportURI = NextURI;

            string curi = "";
            if(Transport.AVTransportURI!=null)
            {
                curi = Transport.AVTransportURI;
            }
            SetContentUri(curi,Transport.NextAVTransportURI);
        }
        private void GetMediaInfoSink(System.UInt32 InstanceID, out System.UInt32 NrTracks, out System.String MediaDuration, out System.String CurrentURI, out System.String NextURI, out DvAVTransport.Enum_PlaybackStorageMedium PlayMedium, out DvAVTransport.Enum_RecordStorageMedium RecordMedium, out DvAVTransport.Enum_RecordMediumWriteStatus  WriteStatus)
        {
            object[] args = new Object[2];
		
            args[0] = (UInt32)0;
            args[1] = (double)0;

            Invoke(new GetMediaInfoFormDelegate(GetMediaInfoSinkEx),args);
            NrTracks = (UInt32)args[0];
            MediaDuration = ((double)args[1]).ToString();
            CurrentURI = Transport.AVTransportURI;
            NextURI = Transport.NextAVTransportURI;
            PlayMedium = DvAVTransport.Enum_PlaybackStorageMedium.UNKNOWN;
            RecordMedium = DvAVTransport.Enum_RecordStorageMedium.UNKNOWN;
            WriteStatus = DvAVTransport.Enum_RecordMediumWriteStatus.NOT_WRITABLE;
        }
        private void GetMediaInfoSinkEx(out UInt32 NrTracks, out double MediaDuration)
        {
            if(player.EntryCount!=0)
            {
                NrTracks = (System.UInt32)player.EntryCount;
            }
            else
            {
                NrTracks = 1;
            }

            MediaDuration = player.Duration;
        }
        private void SetAVTransportURISink(System.UInt32 InstanceID, System.String CurrentURI)
        {
            Transport.AVTransportURI = CurrentURI;
            string nuri = "";
            if(Transport.NextAVTransportURI!=null)
            {
                nuri = Transport.NextAVTransportURI;
            }
            SetContentUri(CurrentURI,nuri);
            //_player.FileName = CurrentURI.ToString();
			
            // *** BUG! This line should work but fails....
            // mainform.SetContentUri(AVTransportURI.ToString(),NextAVTransportURI.ToString());

            System.IO.StringWriter SW = new System.IO.StringWriter();
            XmlTextWriter XDoc = new XmlTextWriter(SW);
				
            XDoc.WriteStartDocument();
            XDoc.WriteStartElement("InstanceId");
            XDoc.WriteAttributeString("val","0");
            XDoc.WriteStartElement("AVTransportURI");
            XDoc.WriteAttributeString("val",CurrentURI);
            XDoc.WriteEndElement();
            XDoc.WriteEndElement();
            XDoc.WriteEndDocument();
            XDoc.Flush();
            Transport.Evented_LastChange = SW.ToString();
            XDoc.Close();

        }
        private void PauseSink(System.UInt32 InstanceID)
        {
            object[] args = new object[1];
            args[0] = InstanceID;
            Invoke(new DvAVTransport.Delegate_Pause(PauseSinkEx),args);
        }
        private void PauseSinkEx(System.UInt32 InstanceID)
        {
            try
            {
                player.Pause();
            }
            catch{}
        }
        private void GetPositionInfoSink(System.UInt32 InstanceID, out System.UInt32 Track, out System.String TrackDuration, out System.String TrackEmbeddedMetaData, out System.String TrackURI, out System.String RelTime, out System.String AbsTime, out System.Int32 RelCount, out System.Int32 AbsCount)
        {
            object[] args = new object[2];
            args[0] = (int)0;
            args[1] = (double)0;

            Invoke(new GetPositionInfoFormDelegate(GetPositionInfoSinkEx),args);

            if((int)args[0]!=0)
            {
                Track = Transport.CurrentTrack;
                RelCount = (int)args[0];
            }
            else
            {
                RelCount = 1;
                Track = 1;
            }
            TrackDuration = Transport.CurrentTrackDuration;
            TrackEmbeddedMetaData = Transport.CurrentTrackEmbeddedMetaData;
            TrackURI = Transport.CurrentTrackURI;
            RelTime = ((double)args[1]).ToString();
            if(RelTime=="-1") RelTime = "0";
            AbsTime = "NOT_IMPLEMENTED";
			
            AbsCount = int.MaxValue;
        }
        private void GetPositionInfoSinkEx(out int RCount, out double CurrentPosition)
        {
            RCount = player.EntryCount;
            CurrentPosition = player.CurrentPosition;
        }


        private void GetTransportSettingsSink(System.UInt32 InstanceID, out DvAVTransport.Enum_CurrentPlayMode PlayMode, out DvAVTransport.Enum_CurrentRecordQualityMode RecQualityMode)
        {
            PlayMode = Transport.CurrentPlayMode;
            RecQualityMode = DvAVTransport.Enum_CurrentRecordQualityMode.NOT_IMPLEMENTED;
        }

        private void GetTransportInfoSink(System.UInt32 InstanceID, out DvAVTransport.Enum_TransportState CurrentTransportState, out DvAVTransport.Enum_TransportPlaySpeed CurrentSpeed)
        {
            CurrentTransportState = Transport.TransportState;
            CurrentSpeed = Transport.TransportPlaySpeed;
        }
		
        private void GetDeviceCapabilitiesSink(System.UInt32 InstanceID, out System.String PlayMedia, out System.String RecMedia, out System.String RecQualityModes)
        {
            RecMedia = "";
            RecQualityModes = "";
            PlayMedia = "UNKNOWN";
        }

        private void GetVolumeSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, out System.UInt16 CurrentVolume)
        {
            UInt16 cv = (UInt16)(player.Volume + 10000);
			
            if(cv>=9500)
            {
                int v = 10000 - cv;
                double p = (double)v/500;
                v = (System.UInt16)(p*50);
                v = 100- v;
                CurrentVolume = (UInt16)v;
            }
            else
            {
                if(cv<7000)
                {
                    cv = 7000;
                }
                int v = cv-7000;
                double p = (double)v/2500;
                v = (UInt16)(p*50);
                CurrentVolume = (UInt16)v;
            }
        }
        private void GetMuteSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, out System.Boolean CurrentMute)
        {
            CurrentMute = player.Mute;
        }
        private void SetVolumeSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.UInt16 DesiredVolume)
        {
            object[] args = new object[3];
            args[0] = InstanceID;
            args[1] = Channel;
            args[2] = DesiredVolume;
            Invoke(new DvRenderingControl.Delegate_SetVolume(SetVolumeSinkEx),args);
        }
        private void SetVolumeSinkEx(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.UInt16 DesiredVolume)
        {
            switch(Channel)
            {
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF:
                    LeftChannel = DesiredVolume;
                    break;
                case DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF:
                    RightChannel = DesiredVolume;
                    break;
            }
            int balance = ((int)System.Math.Pow(((double)LeftChannel-(double)RightChannel)/(double)10,4));
            if(LeftChannel>RightChannel) balance = balance * (-1);

            if(Channel!=RenderingControl.Enum_A_ARG_TYPE_Channel.MASTER)
            {
                player.Balance = balance;
                DesiredVolume = (UInt16)((float)System.Math.Max(LeftChannel,RightChannel)*((float)Control.Volume/(float)100));
            }
            else
            {
                SetVolume(DesiredVolume);
            }
			
            int nv = -1 * (int)(System.Math.Pow(((100 - ((((double)DesiredVolume) / 2.2) + 20)) / 10),4));
            player.Volume = nv;
			
            System.IO.StringWriter SW = new System.IO.StringWriter();
            XmlTextWriter XDoc = new XmlTextWriter(SW);

            XDoc.WriteStartDocument();
            XDoc.WriteStartElement("InstanceId");
            XDoc.WriteAttributeString("val","0");
            XDoc.WriteStartElement("Volume");
            XDoc.WriteAttributeString("val",DesiredVolume.ToString());
            XDoc.WriteEndElement();
            XDoc.WriteEndElement();
            XDoc.WriteEndDocument();
            XDoc.Flush();
            Control.Evented_LastChange = SW.ToString();
            XDoc.Close();
        }
        private void SetMuteSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.Boolean DesiredMute)
        {
            object[] args = new object[3];
            args[0] = InstanceID;
            args[1] = Channel;
            args[2] = DesiredMute;
            Invoke(new DvRenderingControl.Delegate_SetMute(SetMuteSinkEx),args);
        }
        private void SetMuteSinkEx(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.Boolean DesiredMute)
        {
            player.Mute = DesiredMute;
            Control.Mute = DesiredMute;
            SetMuteButton(DesiredMute);

            System.IO.StringWriter SW = new System.IO.StringWriter();
			
            XmlTextWriter XDoc = new XmlTextWriter(SW);				
            XDoc.WriteStartDocument();
            XDoc.WriteStartElement("InstanceId");
            XDoc.WriteAttributeString("val","0");
            XDoc.WriteStartElement("Mute");
            XDoc.WriteAttributeString("val",DesiredMute.ToString());
            XDoc.WriteEndElement();
            XDoc.WriteEndElement();
            XDoc.WriteEndDocument();
            XDoc.Flush();
            Control.Evented_LastChange = SW.ToString();
            XDoc.Close();
        }

        protected void SeekSink(System.UInt32 InstanceID, DvAVTransport.Enum_A_ARG_TYPE_SeekMode Unit, System.String Target)
        {
            object[] args = new object[3];
            args[0] = InstanceID;
            args[1] = Unit;
            args[2] = Target;

            Invoke(new DvAVTransport.Delegate_Seek(SeekSinkEx),args);
        }
        protected void SeekSinkEx(System.UInt32 InstanceID, DvAVTransport.Enum_A_ARG_TYPE_SeekMode Unit, System.String Target)
        {
            DText p = new DText();
            p.ATTRMARK = ":";
            p[0] = Target;

            TimeSpan ts = new TimeSpan(int.Parse(p[1]),int.Parse(p[2]),int.Parse(p[3]));
            player.CurrentPosition = ts.TotalSeconds;
        }
        */

    }
}
