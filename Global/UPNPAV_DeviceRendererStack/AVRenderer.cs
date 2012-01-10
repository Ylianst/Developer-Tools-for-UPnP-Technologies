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
using System.Threading;
using System.Collections;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace OpenSource.UPnP.AV.RENDERER.Device
{
	/// <summary>
	/// Summary description for AVRenderer.
	/// </summary>
	public class AVRenderer : OpenSource.UPnP.IUPnPDevice
	{
		public delegate int PrepareForConnectionHandler(AVRenderer sender, ProtocolInfoString Info);
		public PrepareForConnectionHandler ManualPrepareForConnection = null;

		public class PrepareForConnectionFailedException : Exception
		{
			public PrepareForConnectionFailedException(String msg):base(msg)
			{
			}
		}
		
		public delegate void ConnectionHandler(AVRenderer sender, AVConnection c);
		public event ConnectionHandler OnNewConnection;
		public event ConnectionHandler OnClosedConnection;

		protected UPnPDevice device;
		public DvAVTransport AVT;
		public DvConnectionManager Manager;
		public DvRenderingControl Control;

		protected int ConnectionMax = 0;
		protected int CurrentConnections = 0;
		protected ProtocolInfoString[] InfoStrings = null;
		protected object ConnectionLock = new object();
		protected Hashtable ID_Table = Hashtable.Synchronized(new Hashtable());

		public AVRenderer(int MaxConnections, ProtocolInfoString[] Info) : this(MaxConnections,Info,null)
		{
		}
		public AVRenderer(int MaxConnections, ProtocolInfoString[] Info, ConnectionHandler h)
		{
			ConnectionMax = MaxConnections;
			OnNewConnection += h;

			InfoStrings = Info;
			device = UPnPDevice.CreateEmbeddedDevice(1,Guid.NewGuid().ToString());
			device.FriendlyName = "AVRenderer Device (" + Dns.GetHostName() +")";

			AVT = new DvAVTransport();
			AVT.GetUPnPService().Major = 1;
			AVT.GetUPnPService().Minor = 0;

			Manager = new DvConnectionManager();
			Manager.GetUPnPService().Major = 1;
			Manager.GetUPnPService().Minor = 0;

			Control = new DvRenderingControl();
			Control.GetUPnPService().Major = 1;
			Control.GetUPnPService().Minor = 0;

			Manager.External_PrepareForConnection = new DvConnectionManager.Delegate_PrepareForConnection(PrepareForConnectionSink);
			Manager.External_ConnectionComplete = new DvConnectionManager.Delegate_ConnectionComplete(ConnectionCompleteSink);
			Manager.External_GetCurrentConnectionIDs = new DvConnectionManager.Delegate_GetCurrentConnectionIDs(GetCurrentConnectionIDsSink);
			Manager.External_GetProtocolInfo = new DvConnectionManager.Delegate_GetProtocolInfo(GetProtocolInfoSink);
			Manager.External_GetCurrentConnectionInfo = new DvConnectionManager.Delegate_GetCurrentConnectionInfo(GetCurrentConnectionInfoSink);

			Manager.Evented_CurrentConnectionIDs = "";
			//Manager.Evented_PhysicalConnections = "";

			string Sink = "";
			foreach(ProtocolInfoString s in InfoStrings)
			{
				if(Sink=="")
				{
					Sink = s.ToString();
				}
				else
				{
					Sink = Sink + "," + s.ToString();
				}
			}

			Manager.Evented_SinkProtocolInfo = Sink;
			Manager.Evented_SourceProtocolInfo = "";

			AVT.Accumulator_LastChange = new InstanceID_LastChangeAccumulator("AVT");
			AVT.ModerationDuration_LastChange = 0.5;
			AVT.Evented_LastChange = "&lt;Event xmlns = &quot;urn:schemas-upnp-org:metadata-1-0/AVT/&quot;/&gt;";

			AVT.External_GetMediaInfo = new DvAVTransport.Delegate_GetMediaInfo(GetMediaInfoSink);
			AVT.External_GetPositionInfo = new DvAVTransport.Delegate_GetPositionInfo(GetPositionInfoSink);
			AVT.External_GetTransportInfo = new DvAVTransport.Delegate_GetTransportInfo(GetTransportInfoSink);
			AVT.External_GetTransportSettings = new DvAVTransport.Delegate_GetTransportSettings(GetTransportSettingsSink);
			AVT.External_GetDeviceCapabilities = new DvAVTransport.Delegate_GetDeviceCapabilities(GetDeviceCapabilitiesSink);
			AVT.External_GetCurrentTransportActions = new DvAVTransport.Delegate_GetCurrentTransportActions(GetCurrentTransportActionsSink);
			
			AVT.External_Play = new DvAVTransport.Delegate_Play(PlaySink);
			AVT.External_Stop = new DvAVTransport.Delegate_Stop(StopSink);
			AVT.External_Pause = new DvAVTransport.Delegate_Pause(PauseSink);
			AVT.External_Record = new DvAVTransport.Delegate_Record(RecordSink);
			AVT.External_Previous = new DvAVTransport.Delegate_Previous(PreviousSink);
			AVT.External_Next = new DvAVTransport.Delegate_Next(NextSink);
			AVT.External_Seek = new DvAVTransport.Delegate_Seek(SeekSink);
			AVT.External_SetAVTransportURI = new DvAVTransport.Delegate_SetAVTransportURI(SetAVTransportURISink);
			AVT.External_SetNextAVTransportURI = new DvAVTransport.Delegate_SetNextAVTransportURI(SetNextAVTransportURISink);
			AVT.External_SetPlayMode = new DvAVTransport.Delegate_SetPlayMode(SetPlayModeSink);
			AVT.External_SetRecordQualityMode = new DvAVTransport.Delegate_SetRecordQualityMode(SetRecordQualityModeSink);
			AVT.External_Record = new DvAVTransport.Delegate_Record(RecordSink);

			Control.Evented_LastChange = "&lt;Event xmlns = &quot;urn:schemas-upnp-org:metadata-1-0/RCS/&quot;/&gt;";

			Control.Accumulator_LastChange = new InstanceID_LastChangeAccumulator("RCS");
			Control.ModerationDuration_LastChange = 1;

			Control.External_GetMute = new DvRenderingControl.Delegate_GetMute(GetMuteSink);
			Control.External_SetMute = new DvRenderingControl.Delegate_SetMute(SetMuteSink);
			Control.External_GetVolume = new DvRenderingControl.Delegate_GetVolume(GetVolumeSink);
			Control.External_SetVolume = new DvRenderingControl.Delegate_SetVolume(SetVolumeSink);
			Control.External_GetBlueVideoBlackLevel  = new DvRenderingControl.Delegate_GetBlueVideoBlackLevel(GetBlueVideoBlackSink);
			Control.External_GetBlueVideoGain = new DvRenderingControl.Delegate_GetBlueVideoGain(GetBlueVideoGainSink);
			Control.External_SetBlueVideoBlackLevel = new DvRenderingControl.Delegate_SetBlueVideoBlackLevel(SetBlueVideoBlackSink);
			Control.External_SetBlueVideoGain = new DvRenderingControl.Delegate_SetBlueVideoGain(SetBlueVideoGainSink);
			Control.External_GetGreenVideoBlackLevel  = new DvRenderingControl.Delegate_GetGreenVideoBlackLevel(GetGreenVideoBlackSink);
			Control.External_GetGreenVideoGain = new DvRenderingControl.Delegate_GetGreenVideoGain(GetGreenVideoGainSink);
			Control.External_SetGreenVideoBlackLevel = new DvRenderingControl.Delegate_SetGreenVideoBlackLevel(SetGreenVideoBlackSink);
			Control.External_SetGreenVideoGain = new DvRenderingControl.Delegate_SetGreenVideoGain(SetGreenVideoGainSink);
			Control.External_GetRedVideoBlackLevel  = new DvRenderingControl.Delegate_GetRedVideoBlackLevel(GetRedVideoBlackSink);
			Control.External_GetRedVideoGain = new DvRenderingControl.Delegate_GetRedVideoGain(GetRedVideoGainSink);
			Control.External_SetRedVideoBlackLevel = new DvRenderingControl.Delegate_SetRedVideoBlackLevel(SetRedVideoBlackSink);
			Control.External_SetRedVideoGain = new DvRenderingControl.Delegate_SetRedVideoGain(SetRedVideoGainSink);
			Control.External_GetBrightness = new DvRenderingControl.Delegate_GetBrightness(GetBrightnessSink);
			Control.External_SetBrightness = new DvRenderingControl.Delegate_SetBrightness(SetBrightnessSink);
			Control.External_GetContrast = new DvRenderingControl.Delegate_GetContrast(GetContrastSink);
			Control.External_SetContrast = new DvRenderingControl.Delegate_SetContrast(SetContrastSink);
			Control.External_GetSharpness = new DvRenderingControl.Delegate_GetSharpness(GetSharpnessSink);
			Control.External_SetSharpness = new DvRenderingControl.Delegate_SetSharpness(SetSharpnessSink);


			Control.External_ListPresets = new DvRenderingControl.Delegate_ListPresets(ListPresetsSink);
			Control.External_SelectPreset = new DvRenderingControl.Delegate_SelectPreset(SelectPresetSink);

			device.Manufacturer = "OpenSource";
			device.ManufacturerURL = "";
			device.PresentationURL = "/";
			device.HasPresentation = false;
			device.ModelName = "Renderer";
			device.ModelDescription = "AV Media Renderer Device";
			device.ModelURL = new Uri("http://www.sourceforge.org");
			device.StandardDeviceType = "MediaRenderer";
			
			device.AddService(Manager);
			device.AddService(Control);
			device.AddService(AVT);

			if(ConnectionMax == 0)
			{
				Manager.Evented_CurrentConnectionIDs = "0";
				CurrentConnections = 1;
				AVConnection c = new AVConnection(this, "", "/", -1, DvConnectionManager.Enum_A_ARG_TYPE_Direction.INPUT, 0, 0, 0);
				c.Parent = this;
				c._WhoCreatedMe = new IPEndPoint(IPAddress.Parse("127.0.0.1"),0);
				ID_Table[(UInt32)0] = c;
				if(h!=null) h(this,c);
			}
		}

		public UPnPDevice GetUPnPDevice()
		{
			return(device);
		}

		internal void ConnectionClosed(AVConnection sender)
		{
			lock(ConnectionLock)
			{
				--CurrentConnections;
				if(CurrentConnections<0) CurrentConnections = 0;
				ID_Table.Remove((UInt32)sender.Connection_ID);
			}
			IDictionaryEnumerator en = ID_Table.GetEnumerator();
			string EventString = "";
			while(en.MoveNext())
			{
				if(EventString == "")
				{
					EventString = en.Key.ToString();
				}
				else
				{
					EventString = EventString + "," + en.Key.ToString();
				}
			}
			Manager.Evented_CurrentConnectionIDs = EventString;
		}

		protected void ConnectionCompleteSink(Int32 ConnectionID)
		{
			if(ID_Table.ContainsKey((UInt32)ConnectionID)==false)
			{
				throw(new UPnPCustomException(802,ConnectionID.ToString() + " is not a valid ID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[(UInt32)ConnectionID];
				ID_Table.Remove((UInt32)ConnectionID);

				if(OnClosedConnection!=null) OnClosedConnection(this,c);
				c.dispose();
			}
		}

		public int GetAndSetNewConnectionID()
		{
			int ConnectionID = 0;
			lock(ConnectionLock)
			{
				++ CurrentConnections;
				Random r = new Random();
				do
				{
					ConnectionID = r.Next(4096,65535);
				}while(ID_Table.ContainsKey(ConnectionID)==true);
				ID_Table[(UInt32)ConnectionID] = "";
			}
			return(ConnectionID);
		}
											 
		protected void PrepareForConnectionSink(System.String RemoteProtocolInfo, System.String PeerConnectionManager, int PeerConnectionID, DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, out System.Int32 ConnectionID, out System.Int32 AVTransportID, out System.Int32 RcsID)
		{
			bool OK = false;
			ConnectionID = 0;
			string EventString = "";

			foreach(ProtocolInfoString s in InfoStrings)
			{
				if(s.Equals(new ProtocolInfoString(RemoteProtocolInfo)))
				{
					OK = true;
					break;
				}
			}

			if(OK==false)
			{
				// We don't support this protocol Info;
				throw(new UPnPCustomException(801,RemoteProtocolInfo + " NOT supported"));
			}

			OK = false;
			if(ManualPrepareForConnection!=null)
			{
				try
				{
					ConnectionID = ManualPrepareForConnection(this,new ProtocolInfoString(RemoteProtocolInfo));
					OK = true;
					ID_Table[(UInt32)ConnectionID] = "";
				}
				catch(PrepareForConnectionFailedException pfcfe)
				{
					throw(new UPnPCustomException(800,pfcfe.Message));
				}
			}

			lock(ConnectionLock)
			{
				if(OK==false)
				{
					if(CurrentConnections<ConnectionMax)
					{ 
						++ CurrentConnections;
						Random r = new Random();
						do
						{
							ConnectionID = r.Next(4096,65535);
						}while(ID_Table.ContainsKey(ConnectionID)==true);
						ID_Table[(UInt32)ConnectionID] = "";
						OK = true;
					}
					else
					{
						OK = false;
					}
				}
				if(OK==true)
				{
					// BuildEventString
					IDictionaryEnumerator KEYS = ID_Table.GetEnumerator();
					while(KEYS.MoveNext())
					{
						if(EventString=="")
						{
							EventString = KEYS.Key.ToString();
						}
						else
						{
							EventString = EventString + "," + KEYS.Key.ToString();
						}
					}
				}
			}

			if(OK==false) throw(new UPnPCustomException(800,"No more resources"));

			AVTransportID = (int)ConnectionID;
			RcsID = (int)ConnectionID;

			AVConnection c = new AVConnection(this, RemoteProtocolInfo, PeerConnectionManager, PeerConnectionID, Direction, ConnectionID, AVTransportID, RcsID);
			c._WhoCreatedMe = AVT.GetCaller();
			
			ID_Table[(UInt32)ConnectionID] = c;

			Manager.Evented_CurrentConnectionIDs = EventString;

			if(OnNewConnection!=null) OnNewConnection(this,c);
		}
		protected void GetProtocolInfoSink(out System.String Source, out System.String Sink)
		{
			Source = "";
			Sink = "";
			foreach(ProtocolInfoString s in InfoStrings)
			{
				if(Sink=="")
				{
					Sink = s.ToString();
				}
				else
				{
					Sink = Sink + "," + s.ToString();
				}
			}
		}
		protected void GetCurrentConnectionIDsSink(out string ID)
		{
			StringBuilder SB = new StringBuilder();
			object[] IDs;

			lock(ConnectionLock)
			{
				int len = ID_Table.Keys.Count;
				IDs = new object[len];
				ID_Table.Keys.CopyTo(IDs,0);
			}

			foreach(UInt32 i in IDs)
			{
				if(SB.Length==0)
				{
					SB.Append(i.ToString());
				}
				else
				{
					SB.Append("," + i.ToString());
				}
			}

			ID = SB.ToString();
		}
		protected void GetCurrentConnectionInfoSink(System.Int32 ConnectionID, out System.Int32 RcsID, out System.Int32 AVTransportID, out System.String ProtocolInfo, out System.String PeerConnectionManager, out System.Int32 PeerConnectionID, out DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, out DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus Status)
		{
			if(ID_Table.ContainsKey((UInt32)ConnectionID)==false)
			{
				throw(new UPnPCustomException(802,ConnectionID.ToString() + " is not a valid ID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[(UInt32)ConnectionID];
				RcsID = c.RenderingControl_ID;
				AVTransportID = c.AVTransport_ID;
				ProtocolInfo = c.InfoString.ToString();
				PeerConnectionManager = c.PeerConnectionManager;
				PeerConnectionID = c.PeerConnectionManagerID;
				Direction = c._Direction;
				Status = c._Status;
			}
		}


		protected void GetMediaInfoSink(System.UInt32 InstanceID, out System.UInt32 NrTracks, out System.String MediaDuration, out System.String CurrentURI, out System.String CurrentURIMetaData, out System.String NextURI, out System.String NextURIMetaData, out DvAVTransport.Enum_PlaybackStorageMedium PlayMedium, out DvAVTransport.Enum_RecordStorageMedium RecordMedium, out DvAVTransport.Enum_RecordMediumWriteStatus WriteStatus)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				NrTracks = c._NumberOfTracks;
				MediaDuration = string.Format("{0:00}",c._Duration.Hours) + ":" + string.Format("{0:00}",c._Duration.Minutes) + ":" + string.Format("{0:00}",c._Duration.Seconds);
				if(c._CurrentURI==null)
				{
					CurrentURI = "";
				}
				else
				{
					CurrentURI = HTTPMessage.UnEscapeString(c._CurrentURI.AbsoluteUri);
				}
				if(c._NextURI==null)
				{
					NextURI = "";
				}
				else
				{
					NextURI = HTTPMessage.UnEscapeString(c._NextURI.AbsoluteUri);
				}

				CurrentURIMetaData = c._CurrentURIMetaData;
				NextURIMetaData = c._NextURIMetaData;
				PlayMedium = c._PlaybackMedium;
				RecordMedium = c._RecordMedium;
				WriteStatus = c._WriteStatus;
			}
		}
		protected void GetPositionInfoSink(System.UInt32 InstanceID, out System.UInt32 Track, out System.String TrackDuration, out System.String TrackEmbeddedMetaData, out System.String TrackURI, out System.String RelTime, out System.String AbsTime, out System.Int32 RelCount, out System.Int32 AbsCount)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];

				Track = c._Track;
				TrackDuration = string.Format("{0:00}",c._TrackDuration.Hours) + ":" + string.Format("{0:00}",c._TrackDuration.Minutes) + ":" + string.Format("{0:00}",c._TrackDuration.Seconds);
				TrackEmbeddedMetaData = c._TrackEmbeddedMetaData;
				if(c._TrackURI==null)
				{
					TrackURI = "";
				}
				else
				{
					TrackURI = c._TrackURI.AbsoluteUri;
				}
				if(c._RelativeTime.TotalSeconds<0)
				{
					RelTime = "NOT_IMPLEMENTED";
				}
				else
				{
					RelTime = string.Format("{0:00}",c._RelativeTime.Hours) + ":" + string.Format("{0:00}",c._RelativeTime.Minutes) + ":" + string.Format("{0:00}",c._RelativeTime.Seconds);
				}
				if(c._AbsoluteTime.TotalSeconds<0)
				{
					AbsTime = "NOT_IMPLEMENTED";
				}
				else
				{
					AbsTime = string.Format("{0:00}",c._AbsoluteTime.Hours) + ":" + string.Format("{0:00}",c._AbsoluteTime.Minutes) + ":" + string.Format("{0:00}",c._AbsoluteTime.Seconds);
				}
				RelCount = c._RelativeCounter;
				AbsCount = c._AbsoluteCounter;
			}
		}
		protected void GetTransportInfoSink(System.UInt32 InstanceID, out DvAVTransport.Enum_TransportState CurrentTransportState, out DvAVTransport.Enum_TransportStatus CurrentTransportStatus, out DvAVTransport.Enum_TransportPlaySpeed CurrentSpeed)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentTransportState = c._CurrentTransportState;
				CurrentSpeed = c._CurrentTransportSpeed;
				CurrentTransportStatus = c._CurrentStatus;
			}
		}
		protected void GetTransportSettingsSink(System.UInt32 InstanceID, out DvAVTransport.Enum_CurrentPlayMode PlayMode, out DvAVTransport.Enum_CurrentRecordQualityMode RecQualityMode)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				PlayMode = c._CurrentPlayMode;
				RecQualityMode = c._CurrentRecMode;
			}
		}
		protected void GetDeviceCapabilitiesSink(System.UInt32 InstanceID, out System.String PlayMedia, out System.String RecMedia, out System.String RecQualityModes)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				PlayMedia = "";
				RecMedia = "";
				RecQualityModes = "";

				foreach(string m in DvAVTransport.Values_PlaybackStorageMedium)
				{
					if(PlayMedia=="")
					{
						PlayMedia = m;
					}
					else
					{
						PlayMedia = PlayMedia + "," + m;
					}
				}
				
				foreach(string m in DvAVTransport.Values_RecordStorageMedium)
				{
					if(RecMedia=="")
					{
						RecMedia = m;
					}
					else
					{
						RecMedia = RecMedia + "," + m;
					}
				}

				foreach(string m in DvAVTransport.Values_CurrentRecordQualityMode)
				{
					if(RecQualityModes=="")
					{
						RecQualityModes = m;
					}
					else
					{
						RecQualityModes = RecQualityModes + "," + m;
					}
				}
			}
		}
		protected void GetCurrentTransportActionsSink(System.UInt32 InstanceID, out System.String Actions)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				Actions = c._Actions;
			}
		}
		protected void PlaySink(System.UInt32 InstanceID, DvAVTransport.Enum_TransportPlaySpeed Speed)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Play(Speed);
			}
		}
		protected void StopSink(System.UInt32 InstanceID)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Stop();
			}
		}
		protected void PauseSink(System.UInt32 InstanceID)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Pause();
			}
		}
		protected void RecordSink(System.UInt32 InstanceID)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Record();
			}
		}
		protected void PreviousSink(System.UInt32 InstanceID)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Previous();
			}
		}
		protected void NextSink(System.UInt32 InstanceID)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.Next();
			}
		}
		protected void SeekSink(System.UInt32 InstanceID, DvAVTransport.Enum_A_ARG_TYPE_SeekMode Unit, System.String Target)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];

				if ((c.NumberOfTracks <= 1) && (Unit == DvAVTransport.Enum_A_ARG_TYPE_SeekMode.TRACK_NR))
				{
					throw new UPnPCustomException(710, "Invalid seek mode. Cannot seek on a zero or one-item playlist or media.");
				}
				c.Seek(Unit,Target);
			}
		}
		protected void SetAVTransportURISink(System.UInt32 InstanceID, System.String CurrentURI, System.String CurrentURIMetaData)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				if((CurrentURI =="")||(CurrentURI==null))
				{
					c.SetAVTransportURI(null,CurrentURIMetaData);
				}
				else
				{
					c.SetAVTransportURI(new Uri(CurrentURI),CurrentURIMetaData);
				}
			}
		}
		protected void SetNextAVTransportURISink(System.UInt32 InstanceID, System.String NextURI, System.String NextURIMetaData)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				if((NextURI =="")||(NextURI==null))
				{
					c.SetNextAVTransportURI(null, NextURIMetaData);
				}
				else
				{
					c.SetNextAVTransportURI(new Uri(NextURI),NextURIMetaData);
				}
			}
		}
		protected void SetPlayModeSink(System.UInt32 InstanceID, DvAVTransport.Enum_CurrentPlayMode NewPlayMode)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.SetPlayMode(NewPlayMode);
			}
		}
		protected void SetRecordQualityModeSink(System.UInt32 InstanceID, DvAVTransport.Enum_CurrentRecordQualityMode NewRecordQualityMode)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c.SetRecordQualityMode(NewRecordQualityMode);
			}
		}

		protected void GetVolumeSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, out System.UInt16 CurrentVolume)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentVolume = c.GetVolume(Channel);
			}
		}
		protected void SetVolumeSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.UInt16 DesiredVolume)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetVolume(Channel,DesiredVolume);
			}
		}

		protected void GetMuteSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, out System.Boolean CurrentMute)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentMute = c.GetMute(Channel);
			}
		}
		protected void SetMuteSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_Channel Channel, System.Boolean DesiredMute)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetMute(Channel,DesiredMute);
			}
		}
		protected void GetBlueVideoBlackSink(System.UInt32 InstanceID, out System.UInt16 CurrentBlueVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentBlueVideoBlackLevel = c.BlueVideoBlackLevel;
			}
		}
		protected void GetBlueVideoGainSink(System.UInt32 InstanceID, out System.UInt16 CurrentBlueVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentBlueVideoGain = c.BlueVideoGain;
			}
		}
		protected void SetBlueVideoBlackSink(System.UInt32 InstanceID, System.UInt16 DesiredBlueVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetBlueVideoBlack(DesiredBlueVideoBlackLevel);
			}
		}
		protected void SetBlueVideoGainSink(System.UInt32 InstanceID, System.UInt16 DesiredBlueVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetBlueVideoGain(DesiredBlueVideoGain);
			}
		}
		
		protected void GetGreenVideoBlackSink(System.UInt32 InstanceID, out System.UInt16 CurrentGreenVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentGreenVideoBlackLevel = c.GreenVideoBlackLevel;
			}
		}
		protected void GetGreenVideoGainSink(System.UInt32 InstanceID, out System.UInt16 CurrentGreenVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentGreenVideoGain = c.GreenVideoGain;
			}
		}
		protected void SetGreenVideoBlackSink(System.UInt32 InstanceID, System.UInt16 DesiredGreenVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetGreenVideoBlack(DesiredGreenVideoBlackLevel);
			}
		}
		protected void SetGreenVideoGainSink(System.UInt32 InstanceID, System.UInt16 DesiredGreenVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetGreenVideoGain(DesiredGreenVideoGain);
			}
		}
		protected void GetRedVideoBlackSink(System.UInt32 InstanceID, out System.UInt16 CurrentRedVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentRedVideoBlackLevel = c.RedVideoBlackLevel;
			}
		}
		protected void GetRedVideoGainSink(System.UInt32 InstanceID, out System.UInt16 CurrentRedVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentRedVideoGain = c.RedVideoGain;
			}
		}
		protected void SetRedVideoBlackSink(System.UInt32 InstanceID, System.UInt16 DesiredRedVideoBlackLevel)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetRedVideoBlack(DesiredRedVideoBlackLevel);
			}
		}
		protected void SetRedVideoGainSink(System.UInt32 InstanceID, System.UInt16 DesiredRedVideoGain)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetRedVideoGain(DesiredRedVideoGain);
			}
		}
		protected void ListPresetsSink(System.UInt32 InstanceID, out System.String CurrentPresetNameList)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentPresetNameList = "";
				foreach(string preset in c.Presets)
				{
					if(CurrentPresetNameList=="")
					{
						CurrentPresetNameList = preset;
					}
					else
					{
						CurrentPresetNameList = CurrentPresetNameList + "," + preset;
					}
				}
			}
		}
		protected void SelectPresetSink(System.UInt32 InstanceID, DvRenderingControl.Enum_A_ARG_TYPE_PresetName PresetName)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				bool OK = false;
				foreach(string preset in c.Presets)
				{
					if(preset==DvRenderingControl.Enum_A_ARG_TYPE_PresetName_ToString(PresetName))
					{
						OK = true;
						break;
					}
				}
				if(OK==true)
				{
					c.CurrentPreset = DvRenderingControl.Enum_A_ARG_TYPE_PresetName_ToString(PresetName);

					c._SetBlueVideoBlack(100);
					c._SetBlueVideoGain(100);
					c._SetBrightness(100);
					c._SetContrast(100);
					c._SetGreenVideoBlack(100);
					c._SetGreenVideoGain(100);
					c._SetRedVideoBlack(100);
					c._SetRedVideoGain(100);
					c._SetSharpness(100);

					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.B, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.CF, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LFC, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LFE, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LS, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RFC, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RS, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SD, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SL, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SR, false);
					c._SetMute(DvRenderingControl.Enum_A_ARG_TYPE_Channel.T, false);

					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.B, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.CF, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LF, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LFC, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LFE, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.LS, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.MASTER, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RF, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RFC, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.RS, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SD, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SL, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.SR, 100);
					c._SetVolume(DvRenderingControl.Enum_A_ARG_TYPE_Channel.T, 100);
				}
				else
				{
					throw(new UPnPCustomException(701,DvRenderingControl.Enum_A_ARG_TYPE_PresetName_ToString(PresetName) + " is not a supported preset"));
				}
			}
		}
		protected void GetContrastSink(System.UInt32 InstanceID, out System.UInt16 CurrentContrast)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentContrast = c.Contrast;
			}
		}
		protected void SetContrastSink(System.UInt32 InstanceID, System.UInt16 DesiredContrast)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetContrast(DesiredContrast);
			}
		}
		protected void GetBrightnessSink(System.UInt32 InstanceID, out System.UInt16 CurrentBrightness)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentBrightness = c.Brightness;
			}
		}
		protected void SetBrightnessSink(System.UInt32 InstanceID, System.UInt16 DesiredBrightness)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];	
				c._SetBrightness(DesiredBrightness);
			}
		}
		protected void GetSharpnessSink(System.UInt32 InstanceID, out System.UInt16 CurrentSharpness)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				CurrentSharpness = c.Sharpness;
			}
		}
		protected void SetSharpnessSink(System.UInt32 InstanceID, System.UInt16 DesiredSharpness)
		{
			if(ID_Table.ContainsKey(InstanceID)==false)
			{
				throw(new UPnPCustomException(802,InstanceID.ToString() + " is not a valid InstanceID"));
			}
			else
			{
				AVConnection c = (AVConnection)ID_Table[InstanceID];
				c._SetSharpness(DesiredSharpness);
			}
		}
		
	}
}
