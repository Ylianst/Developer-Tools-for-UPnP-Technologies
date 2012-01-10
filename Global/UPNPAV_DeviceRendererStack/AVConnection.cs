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
using System.Net;
using System.Text;
using System.Collections;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace OpenSource.UPnP.AV.RENDERER.Device
{
	/// <summary>
	/// Summary description for AVConnection.
	/// </summary>
	public class AVConnection
	{
		public object Tag = null;
		private bool disposed = false;
		public string[] Presets = new string[2]{"FactoryDefaults","InstallationDefaults"};
		public string _CurrentPreset = "FactoryDefaults";
		public string CurrentPreset
		{
			get
			{
				return(_CurrentPreset);
			}
			set
			{
				_CurrentPreset = value;
				if(OnCurrentPreset!=null) OnCurrentPreset(this);
			}
		}

		protected internal AVRenderer Parent = null;
		
		protected Hashtable MuteChannelTable = Hashtable.Synchronized(new Hashtable());
		protected Hashtable VolumeChannelTable = Hashtable.Synchronized(new Hashtable());

		internal DvAVTransport.Enum_TransportStatus _CurrentStatus = DvAVTransport.Enum_TransportStatus.OK;
		internal Int32 Connection_ID;
		internal int AVTransport_ID;
		internal int RenderingControl_ID;
		internal ProtocolInfoString _InfoString;
		internal string _PeerManager;
		internal int _PeerManagerID;
		internal DvConnectionManager.Enum_A_ARG_TYPE_Direction _Direction;
		internal DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus _Status = DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus.UNKNOWN;
		internal IPEndPoint _WhoCreatedMe = null;

		internal UInt32 _NumberOfTracks = 0;
		internal TimeSpan _Duration = TimeSpan.FromSeconds(0);
		internal Uri _CurrentURI = null;
		internal Uri _NextURI = null;
		internal string _CurrentURIMetaData = "";
		internal string _NextURIMetaData = "";
		internal DvAVTransport.Enum_PlaybackStorageMedium _PlaybackMedium = DvAVTransport.Enum_PlaybackStorageMedium.UNKNOWN;
		internal DvAVTransport.Enum_RecordStorageMedium _RecordMedium = DvAVTransport.Enum_RecordStorageMedium.UNKNOWN;
		internal DvAVTransport.Enum_RecordMediumWriteStatus _WriteStatus = DvAVTransport.Enum_RecordMediumWriteStatus.UNKNOWN;

		internal UInt32 _Track = 0;
		internal TimeSpan _TrackDuration = TimeSpan.FromSeconds(0);
		internal string _TrackEmbeddedMetaData = "";
		internal Uri _TrackURI = null;
		internal TimeSpan _RelativeTime = TimeSpan.FromSeconds(-1);
		internal TimeSpan _AbsoluteTime = TimeSpan.FromSeconds(-1);
		internal int _RelativeCounter = int.MaxValue;
		internal int _AbsoluteCounter = int.MaxValue;

		internal DvAVTransport.Enum_TransportState _CurrentTransportState = DvAVTransport.Enum_TransportState.NO_MEDIA_PRESENT;
		internal DvAVTransport.Enum_TransportPlaySpeed _CurrentTransportSpeed = DvAVTransport.Enum_TransportPlaySpeed._1;
		internal DvAVTransport.Enum_CurrentPlayMode _CurrentPlayMode = DvAVTransport.Enum_CurrentPlayMode.NORMAL;
		internal DvAVTransport.Enum_CurrentRecordQualityMode _CurrentRecMode = DvAVTransport.Enum_CurrentRecordQualityMode._0_BASIC;

		//internal string _PlayMedia = "";
		//internal string _RecMedia = "";
		//internal string _RecQualityModes = "0:BASIC";
		internal string _Actions = "";

		internal UInt16 _BlueVideoBlack = 0;
		internal UInt16 _BlueVideoGain = 0;
		internal UInt16 _GreenVideoBlack = 0;
		internal UInt16 _GreenVideoGain = 0;
		internal UInt16 _RedVideoBlack = 0;
		internal UInt16 _RedVideoGain = 0;
		internal UInt16 _Contrast = 0;
		internal UInt16 _Brightness = 0;
		internal UInt16 _Sharpness = 0;

		public delegate void VariableChangedHandler(AVConnection sender);
		public event VariableChangedHandler OnCurrentURIChanged;
		public event VariableChangedHandler OnNextURIChanged;
		public event VariableChangedHandler OnCurrentPlayModeChanged;
		public event VariableChangedHandler OnCurrentRecordQualityModeChanged;
		public event VariableChangedHandler OnBlueVideoBlackLevelChanged;
		public event VariableChangedHandler OnBlueVideoGainChanged;
		public event VariableChangedHandler OnRedVideoBlackLevelChanged;
		public event VariableChangedHandler OnRedVideoGainChanged;
		public event VariableChangedHandler OnGreenVideoBlackLevelChanged;
		public event VariableChangedHandler OnGreenVideoGainChanged;
		public event VariableChangedHandler OnCurrentPreset;
		public event VariableChangedHandler OnContrastChanged;
		public event VariableChangedHandler OnSharpnessChanged;
		public event VariableChangedHandler OnBrightnessChanged;

		public delegate void PlayHandler(AVConnection sender, DvAVTransport.Enum_TransportPlaySpeed Speed);
		public event PlayHandler OnPlay;
		public delegate void StopPauseRecordHandler(AVConnection sender);
		public event StopPauseRecordHandler OnStop;
		public event StopPauseRecordHandler OnPause;
		public event StopPauseRecordHandler OnRecord;
		public delegate void PreviousNextHandler(AVConnection sender);
		public event PreviousNextHandler OnPrevious;
		public event PreviousNextHandler OnNext;
		public delegate void SeekHandler(AVConnection sender, DvAVTransport.Enum_A_ARG_TYPE_SeekMode SeekMode, string Target);
		public event SeekHandler OnSeek;

		public delegate void ControlChangedHandler(AVConnection sender, UInt16 NewValue);
		public delegate void VolumeChangedHandler(AVConnection sender, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, UInt16 NewValue);
		public delegate void MuteChangedHandler(AVConnection sender, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, bool NewValue);

		public event MuteChangedHandler OnMuteChanged;
		public event VolumeChangedHandler OnVolumeChanged;

		~AVConnection()
		{
			if(disposed) return;

			dispose();
		}
		public AVConnection(AVRenderer _parent, System.String RemoteProtocolInfo, System.String PeerConnectionManager, int PeerConnectionID, DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, System.Int32 ConnectionID, System.Int32 AVTransportID, System.Int32 RcsID)
		{
			Parent = _parent;
			foreach(string V in DvRenderingControl.Values_A_ARG_TYPE_Channel)
			{
				MuteChannelTable[V] = false;
				VolumeChannelTable[V] = (UInt16)0;
			}

			_InfoString = new ProtocolInfoString(RemoteProtocolInfo);
			_PeerManager = PeerConnectionManager;
			_PeerManagerID = PeerConnectionID;
			Connection_ID = ConnectionID;
			AVTransport_ID = AVTransportID;
			RenderingControl_ID = RcsID;
			_Direction = Direction;
		}

		public IPEndPoint WhoCreatedMe
		{
			get
			{
				return(_WhoCreatedMe);
			}
		}

		public void SetAllowedTransportActions(bool Play, bool Stop, bool Pause, bool Next, bool Previous)
		{
			string ta = "";
			ArrayList a = new ArrayList();

			if(Play) a.Add("Play");
			if(Stop) a.Add("Stop");
			if(Pause) a.Add("Pause");
			if(Next) a.Add("Next");
			if(Previous) a.Add("Previous");

			for(int i=0;i<a.Count;++i)
			{
				if(ta=="")
				{
					ta = (string)a[i];
				}
				else
				{
					ta += "," + (string)a[i];
				}
				
			}
			this._Actions = ta;
		}

		public bool GetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel)
		{
			return((bool)MuteChannelTable[DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel)]);
		}
		public void SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, bool DesiredMute)
		{
			MuteChannelTable[DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel)] = DesiredMute;
			
			RC_LastChange("Mute",DesiredMute.ToString().ToLower(),"channel",DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel));

//			if(Channel==DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER)
//			{
//				RC_LastChange("Mute",DesiredMute.ToString().ToLower());
//			}
		}
		public UInt16 GetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel)
		{
			string arg = DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel);
			return((UInt16)VolumeChannelTable[arg]);
		}
		public void SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, UInt16 DesiredVolume)
		{
			string arg = DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel);
			VolumeChannelTable[arg] = DesiredVolume;
			RC_LastChange("Volume",DesiredVolume.ToString(),"channel", DvRenderingControl.Enum_A_ARG_TYPE_Channel_ToString(Channel));
/*
			if(Channel==RenderingControl.Enum_A_ARG_TYPE_Channel.MASTER)
			{
				RC_LastChange("Volume",DesiredVolume.ToString());
			}*/
		}

		public Int32 ID
		{
			get
			{
				return(this.Connection_ID);
			}
		}
		public UInt16 Contrast
		{
			get
			{
				return(_Contrast);
			}
			set
			{
				_Contrast = value;
				RC_LastChange("Contrast",value.ToString());
			}
		}
		
		public UInt16 Brightness
		{
			get
			{
				return(_Brightness);
			}
			set
			{
				_Brightness = value;
				RC_LastChange("Brightness",value.ToString());
			}
		}


		public UInt16 Sharpness
		{
			get
			{
				return(_Sharpness);
			}
			set
			{
				_Sharpness = value;
				RC_LastChange("Sharpness",value.ToString());
			}
		}

		public UInt16  BlueVideoBlackLevel
		{
			get
			{
				return(this._BlueVideoBlack);
			}
			set
			{
				_BlueVideoBlack = value;
				RC_LastChange("BlueVideoBlackLevel",value.ToString());
			}
		}
		public UInt16  BlueVideoGain
		{
			get
			{
				return(this._BlueVideoGain);
			}
			set
			{
				_BlueVideoGain = value;
				RC_LastChange("BlueVideoGain",value.ToString());
			}
		}
		public UInt16  GreenVideoBlackLevel
		{
			get
			{
				return(this._GreenVideoBlack);
			}
			set
			{
				_GreenVideoBlack = value;
				RC_LastChange("GreenVideoBlackLevel",value.ToString());
			}
		}
		public UInt16  GreenVideoGain
		{
			get
			{
				return(this._GreenVideoGain);
			}
			set
			{
				_GreenVideoGain = value;
				RC_LastChange("GreenVideoGain",value.ToString());
			}
		}
		public UInt16  RedVideoBlackLevel
		{
			get
			{
				return(this._RedVideoBlack);
			}
			set
			{
				_RedVideoBlack = value;
				RC_LastChange("RedVideoBlackLevel",value.ToString());
			}
		}
		public UInt16  RedVideoGain
		{
			get
			{
				return(this._RedVideoGain);
			}
			set
			{
				_RedVideoGain = value;
				RC_LastChange("RedVideoGain",value.ToString());
			}
		}

		public UInt32 NumberOfTracks
		{
			get
			{
				return(this._NumberOfTracks);
			}
			set
			{
				_NumberOfTracks = value;
				AVT_LastChange("NumberOfTracks",value.ToString());
			}
		}
		public UInt32 CurrentTrack
		{
			get
			{
				return(this._Track);
			}
			set
			{
				_Track = value;
				AVT_LastChange("CurrentTrack",value.ToString());
			}
		}

		protected void AVT_LastChange(string VarName,string VarValue)
		{
			StringBuilder s = new StringBuilder();

			s.Append("<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\">\r\n");
			s.Append("   <InstanceID val=\"" + AVTransport_ID.ToString() + "\">\r\n");
			s.Append("        <" + VarName + " val=\"" + UPnPStringFormatter.EscapeString(VarValue) + "\"/>\r\n");
			s.Append("   </InstanceID>\r\n");
			s.Append("</Event>");

			string ss = UPnPStringFormatter.EscapeString(s.ToString());

			Parent.AVT.Evented_LastChange = UPnPStringFormatter.EscapeString(s.ToString());
		}
		protected void RC_LastChange(string VarName, string VarValue)
		{
			RC_LastChange(VarName, VarValue, null, null);
		}
		protected void RC_LastChange(string VarName,string VarValue, string attr, string attrval)
		{
			StringBuilder s = new StringBuilder();

			s.Append("<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/RCS/\">\r\n");
			s.Append("   <InstanceID val=\"" + AVTransport_ID.ToString() + "\">\r\n");
			s.Append("        <" + VarName + " ");
			if(attr!=null)
			{
				s.Append(attr + "=\"" + attrval + "\" ");
			}
			s.Append("val=\"" + VarValue + "\"/>\r\n");
			s.Append("   </InstanceID>\r\n");
			s.Append("</Event>");

			Parent.Control.Evented_LastChange = UPnPStringFormatter.EscapeString(s.ToString());
		}

		public void dispose()
		{
			disposed = true;

			Parent.ConnectionClosed(this);
		}

		public ProtocolInfoString InfoString
		{
			get
			{
				return(this._InfoString);
			}
		}
		public string PeerConnectionManager
		{
			get
			{
				return(this._PeerManager);
			}
		}

		public int PeerConnectionManagerID
		{
			get
			{
				return(this._PeerManagerID);
			}
		}

		public DvAVTransport.Enum_TransportState CurrentTransportState
		{
			get
			{
				return(this._CurrentTransportState);
			}
			set
			{
				_CurrentTransportState = value;
				AVT_LastChange("TransportState",DvAVTransport.Enum_TransportState_ToString(value));
			}
		}
		public int CurrentRelativeCounter
		{
			get
			{
				return(this._RelativeCounter);
			}
			set
			{
				_RelativeCounter = value;
				AVT_LastChange("RelativeCounterPosition",value.ToString());
			}
		}
		public int CurrentAbsoluteCounter
		{
			get
			{
				return(this._AbsoluteCounter);
			}
			set
			{
				_AbsoluteCounter = value;
				AVT_LastChange("AbsoluteCounterPosition",value.ToString());
			}
		}
		public TimeSpan TrackDuration
		{
			get
			{
				return(this._TrackDuration);
			}
			set
			{
                if (_TrackDuration.Ticks == value.Ticks) return;
				_TrackDuration = value;
				if (this._TrackDuration.TotalSeconds > 0)
				{
					AVT_LastChange("CurrentTrackDuration",string.Format("{0:00}",value.Hours) + ":" + string.Format("{0:00}",value.Minutes) + ":" + string.Format("{0:00}",value.Seconds));
				}
				else
				{
					AVT_LastChange("CurrentTrackDuration","NOT_IMPLEMENTED");
				}
			}
		}
		public TimeSpan CurrentAbsoluteTimePosition
		{
			get
			{
				return(this._AbsoluteTime);
			}
			set
			{
				_AbsoluteTime = value;
				AVT_LastChange("AbsoluteTimePosition",string.Format("{0:00}",value.Hours) + ":" + string.Format("{0:00}",value.Minutes) + ":" + string.Format("{0:00}",value.Seconds));

			}
		}
		public TimeSpan CurrentRelativeTimePosition
		{
			get
			{
				return(this._RelativeTime);	
			}
			set
			{
				_RelativeTime = value;
				//AVT_LastChange("RelativeTimePosition",string.Format("{0:00}",value.Hours) + ":" + string.Format("{0:00}",value.Minutes) + ":" + string.Format("{0:00}",value.Seconds));
			}
		}

		public string TrackMetaData
		{
			get
			{
				return(this._TrackEmbeddedMetaData);
			}
			set
			{
				_TrackEmbeddedMetaData = value;
				AVT_LastChange("CurrentTrackEmbeddedMetaData",value);
			}
		}
		public Uri TrackURI
		{
			get
			{
				return(this._TrackURI);
			}
			set
			{
				_TrackURI = value;
				if(value==null)
				{
					AVT_LastChange("CurrentTrackURI","");
				}
				else
				{
					AVT_LastChange("CurrentTrackURI",value.AbsoluteUri);
				}
			}
		}
		public Uri CurrentURI
		{
			get
			{
				return(_CurrentURI);
			}
			set
			{
				_CurrentURI = value;
				if(value==null)
				{
					AVT_LastChange("AVTransportURI","");
				}
				else
				{
					AVT_LastChange("AVTransportURI",HTTPMessage.UnEscapeString(value.AbsoluteUri));
				}
				if(this.OnCurrentURIChanged!=null) OnCurrentURIChanged(this);
			}
		}

		private TimeSpan m_MediaDuration = new TimeSpan(0);
		public TimeSpan MediaDuration
		{
			get
			{
				return this.m_MediaDuration;
			}
			set
			{
                if (this.m_MediaDuration.Ticks == value.Ticks) return;
                this.m_MediaDuration = value;
				if (this.m_MediaDuration.Ticks == 0)
				{
					AVT_LastChange("CurrentMediaDuration", "NOT_IMPLEMENTED");
				}
				else
				{
					AVT_LastChange("CurrentMediaDuration", string.Format("{0:00}", this.m_MediaDuration.Hours) + ":" + string.Format("{0:00}", this.m_MediaDuration.Minutes) + ":" + string.Format("{0:00}", this.m_MediaDuration.Seconds));
				}
			}
		}

		public string CurrentUriMetaData
		{
			get
			{
				return(this._CurrentURIMetaData);
			}
			set
			{
				_CurrentURIMetaData = value;
				AVT_LastChange("AVTransportURIMetaData",UPnPStringFormatter.EscapeString(value));
			}
		}
		public Uri NextURI
		{
			get
			{
				return(_NextURI);
			}
			set
			{
				_NextURI = value;
				if(value==null)
				{
					AVT_LastChange("NextAVTransportURI","");
				}
				else
				{
					AVT_LastChange("NextAVTransportURI",HTTPMessage.UnEscapeString(value.AbsoluteUri));
				}
				if(this.OnNextURIChanged!=null) OnNextURIChanged(this);
			}
		}
		public DvAVTransport.Enum_CurrentPlayMode CurrentPlayMode
		{
			get
			{
				return(this._CurrentPlayMode);
			}
			set
			{
				_CurrentPlayMode = value;
				AVT_LastChange("CurrentPlayMode",DvAVTransport.Enum_CurrentPlayMode_ToString(value));
				if(this.OnCurrentPlayModeChanged!=null) OnCurrentPlayModeChanged(this);
			}
		}
		public DvAVTransport.Enum_CurrentRecordQualityMode CurrentRecordQualityMode
		{
			get
			{
				return(this._CurrentRecMode);
			}
			set
			{
				_CurrentRecMode = value;
				AVT_LastChange("CurrentRecordQualityMode",DvAVTransport.Enum_CurrentRecordQualityMode_ToString(value));
				if(this.OnCurrentRecordQualityModeChanged!=null) this.OnCurrentRecordQualityModeChanged(this);
			}
		}



		internal void Play(DvAVTransport.Enum_TransportPlaySpeed speed)
		{
			_CurrentTransportSpeed = speed;
			if(OnPlay!=null) OnPlay(this,speed);
		}
		internal void Stop()
		{
			if(OnStop!=null) OnStop(this);
		}
		internal void Pause()
		{
			if(OnPause!=null) OnPause(this);
		}
		internal void Record()
		{
			if(OnRecord!=null) OnRecord(this);
		}
		internal void Previous()
		{
			if(OnPrevious!=null) OnPrevious(this);
		}
		internal void Next()
		{
			if(OnNext!=null) OnNext(this);
		}
		internal void Seek(DvAVTransport.Enum_A_ARG_TYPE_SeekMode Unit, System.String Target)
		{
			if(OnSeek!=null) OnSeek(this,Unit,Target);
		}
		internal void SetAVTransportURI(Uri TheURI, string MetaData)
		{
			CurrentURI = TheURI;
			//this.CurrentUriMetaData = MetaData;
			if(OnCurrentURIChanged!=null) OnCurrentURIChanged(this);
		}
		internal void SetNextAVTransportURI(Uri TheURI, string MetaData)
		{
			NextURI = TheURI;
			this._NextURIMetaData = MetaData;
			if(OnNextURIChanged!=null) OnNextURIChanged(this);
		}
		internal void SetPlayMode(DvAVTransport.Enum_CurrentPlayMode NewMode)
		{
			CurrentPlayMode = NewMode;
			if(OnCurrentPlayModeChanged!=null) OnCurrentPlayModeChanged(this);
		}
		internal void SetRecordQualityMode(DvAVTransport.Enum_CurrentRecordQualityMode NewMode)
		{
			this.CurrentRecordQualityMode = NewMode;
			if(this.OnCurrentRecordQualityModeChanged!=null) this.OnCurrentRecordQualityModeChanged(this);
		}
		internal void _SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, bool DesiredMute)
		{
			SetMute(Channel,DesiredMute);
			if(OnMuteChanged!=null) OnMuteChanged(this,Channel,DesiredMute);
		}
		internal void _SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, UInt16 DesiredVolume)
		{
			SetVolume(Channel,DesiredVolume);
			if(OnVolumeChanged!=null) OnVolumeChanged(this,Channel,DesiredVolume);
		}
		internal void _SetBlueVideoBlack(UInt16 DesiredBlueVideoBlack)
		{
			this.BlueVideoBlackLevel = DesiredBlueVideoBlack;
			if(OnBlueVideoBlackLevelChanged!=null) OnBlueVideoBlackLevelChanged(this);
		}
		internal void _SetBlueVideoGain(UInt16 DesiredBlueVideoGain)
		{
			this.BlueVideoGain = DesiredBlueVideoGain;
			if(OnBlueVideoGainChanged!=null) OnBlueVideoGainChanged(this);
		}
		internal void _SetGreenVideoBlack(UInt16 DesiredGreenVideoBlack)
		{
			this.GreenVideoBlackLevel = DesiredGreenVideoBlack;
			if(OnGreenVideoBlackLevelChanged!=null) OnGreenVideoBlackLevelChanged(this);
		}
		internal void _SetGreenVideoGain(UInt16 DesiredGreenVideoGain)
		{
			this.GreenVideoGain = DesiredGreenVideoGain;
			if(OnGreenVideoGainChanged!=null) OnGreenVideoGainChanged(this);
		}
		internal void _SetRedVideoBlack(UInt16 DesiredRedVideoBlack)
		{
			this.RedVideoBlackLevel = DesiredRedVideoBlack;
			if(OnRedVideoBlackLevelChanged!=null) OnRedVideoBlackLevelChanged(this);
		}
		internal void _SetRedVideoGain(UInt16 DesiredRedVideoGain)
		{
			this.RedVideoGain = DesiredRedVideoGain;
			if(OnRedVideoGainChanged!=null) OnRedVideoGainChanged(this);
		}
		internal void _SetSharpness(UInt16 DesiredSharpness)
		{
			Sharpness = DesiredSharpness;
			if(OnSharpnessChanged!=null) OnSharpnessChanged(this);
		}
		internal void _SetBrightness(UInt16 DesiredBrightness)
		{
			Brightness = DesiredBrightness;
			if(OnBrightnessChanged!=null) OnBrightnessChanged(this);
		}
		internal void _SetContrast(UInt16 DesiredContrast)
		{
			Contrast = DesiredContrast;
			if(OnContrastChanged!=null) OnContrastChanged(this);
		}
	}
}
