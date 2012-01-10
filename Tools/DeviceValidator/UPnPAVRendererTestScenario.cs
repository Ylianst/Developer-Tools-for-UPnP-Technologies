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
using System.Text;
using System.Threading;
using System.Collections;
using System.Runtime.Serialization;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.RENDERER.CP;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnPValidator
{
	/// <summary>
	/// Summary description for UPnPAVRendererTestScenario.
	/// </summary>
	public class UPnPAVRendererTestScenario : UPnPValidator.BasicTests.BasicTestGroup
	{
		// Specific Test Variables
		int CurrentTime = 0;
		AVConnection TestConnection = null;

		UPnPDevice TestDevice = null;
		CpConnectionManager CM = null;
		CpAVTransport AVT = null;
		ManualResetEvent Ev = new ManualResetEvent(false);
		ManualResetEvent Ev2 = new ManualResetEvent(false);

		//Hashtable TestList = new Hashtable();
		ManualResetEvent Ev_MetaDataChanged = new ManualResetEvent(false);
		ManualResetEvent Ev_PlayModeChanged = new ManualResetEvent(false);
		ManualResetEvent Ev_NumberOfTracksChanged = new ManualResetEvent(false);
		ManualResetEvent Ev_PlayStateChanged = new ManualResetEvent(false);
		ManualResetEvent Ev_TrackChanged = new ManualResetEvent(false);
		ManualResetEvent Ev_TrackURIChanged = new ManualResetEvent(false);

		//[NonSerialized()] private System.Timers.Timer Countdown;
		[NonSerialized()] private ManualResetEvent MRE;

		public UPnPAVRendererTestScenario()
		{
			Category = "UPnP A/V";
			GroupName = "Renderer Test Scenarios";
			Description ="UPnP/AV 1.0 Renderer test suite. This test suite can only run on a UPnP/AV 1.0 compatible renderer device. Performs various tests that vary depending on the device's capabilities.";

			AddTest("Connection Handling","");
			AddTest("Event Formatting","");
			AddTest("StateVariable Values","");
//			AddTest("HTTP Scenario","");

			Reset();
		}

		public override void Reset() 
		{
			base.Reset();
			if (MRE == null) MRE = new ManualResetEvent(false);
		}

		public override void Start(UPnPDevice device)
		{
			UPnPTestStates Master = UPnPTestStates.Pass;
			TestDevice = device;
			UPnPDevice d = device;

			UPnPService[] services = d.GetServices(CpConnectionManager.SERVICE_NAME);

			if (services == null || services.Length == 0) 
			{
				enabled = false;
				return;
			}

			CM = new CpConnectionManager(services[0]);
	
			string SOURCE="";
			string SINK="";
			DText parser = new DText();
			
			StartCountDown(0,90);
			try
			{
				CM.Sync_GetProtocolInfo(out SOURCE,out SINK);
			}
			catch(UPnPInvokeException)
			{
				Results.Add("Connection Handler Test was aborted because GetProtocolInfo FAILED");
				SetState("Connection Handling",UPnPTestStates.Failed);
				Master = UPnPTestStates.Failed;
			}
			AbortCountDown();
			
			parser.ATTRMARK = ",";
			parser.MULTMARK = ":";

			bool OK = true;

			if(ConnectionManagerEventsTest()==false)
			{
				Results.Add("Connection Handler Test was aborted because of invalid/missing events");
				SetState("Connection Handling",UPnPTestStates.Failed);
				Master = UPnPTestStates.Failed;
			}
			
			if(SINK!="")
			{
				parser[0] = SINK;
				TotalTime = parser.DCOUNT()*120;
				CurrentTime = 0;
				for(int i=1;i<=parser.DCOUNT();++i)
				{
					if(parser.DCOUNT(i)!=4)
					{
						// Invalid Format
						OK = false;	
						AddEvent(LogImportance.Critical,"Connection Handling","   Protocol Info String [" + parser[i] + "] is not in a valid format");
					}
				}
				if(OK)
				{
					AddEvent(LogImportance.Remark,"Connection Handling","   Protocol Info Strings are in the correct format");
				}
				else
				{
					Results.Add("Connection Handler Test was aborted because of invalid Protocol Info Strings");
					SetState("Connection Handling",UPnPTestStates.Failed);
					Master = UPnPTestStates.Failed;
				}

				if(CM.HasAction_PrepareForConnection)
				{
					for(int i=1;i<=parser.DCOUNT();++i)
					{
						if(PrepareForConnectionTest_SINK(parser[i])==false)
						{
							OK = false;
						}
					}
				}
			}
	
			if(OK)
			{
				Results.Add("Connection Handler Test PASSED");
				SetState("Connection Handling",UPnPTestStates.Pass);
			}
			else
			{
				Results.Add("Connection Handler Test FAILED");
				SetState("Connection Handling",UPnPTestStates.Failed);
				Master = UPnPTestStates.Failed;
			}

			OK = true;
			UPnPService[] _AVT = d.GetServices(CpAVTransport.SERVICE_NAME);
			if(_AVT.Length!=0)
			{
				// AVT Tests
				AVT = new CpAVTransport(_AVT[0]);
				if(!Test_AVTransport_LastChange()) OK = false;
			}
			if(OK)
			{
				Results.Add("Event Formatting PASSED");
				SetState("Event Formatting",UPnPTestStates.Pass);
			}
			else
			{
				Results.Add("Event Formatting FAILED");
				SetState("Event Formatting",UPnPTestStates.Failed);
				Master = UPnPTestStates.Failed;
			}

			OK = true;
			_AVT = d.GetServices(CpAVTransport.SERVICE_NAME);
			if(_AVT.Length!=0)
			{
				// AVT Tests
				AVT = new CpAVTransport(_AVT[0]);
				if(!Test_AVTransport_StateVariables()) OK = false;
			}
			if (OK)
			{
				Results.Add("StateVariable Values NOT TESTED (Not implemented)");
				SetState("StateVariable Values",UPnPTestStates.Pass);
			}
			else
			{
				Results.Add("StateVariable Values FAILED");
				SetState("StateVariable Values",UPnPTestStates.Failed);
				Master = UPnPTestStates.Failed;
			}

//			this.HTTP_ScenarioTest();

			state = Master;
		}

		private bool Test_AVTransport_StateVariables()
		{
			bool RetVal = true;

			if(AVT.HasStateVariable_A_ARG_TYPE_InstanceID)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("A_ARG_TYPE_InstanceID").GetNetType()==typeof(uint))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: A_ARG_TYPE_InstanceID [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: A_ARG_TYPE_InstanceID [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: A_ARG_TYPE_InstanceID [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_A_ARG_TYPE_SeekMode)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("A_ARG_TYPE_SeekMode").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: A_ARG_TYPE_SeekMode [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: A_ARG_TYPE_SeekMode [Incorrect Type]");
					RetVal = false;
				}
			}

			if(AVT.HasStateVariable_A_ARG_TYPE_SeekTarget)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("A_ARG_TYPE_SeekTarget").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: A_ARG_TYPE_SeekTarget [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: A_ARG_TYPE_SeekTarget [Incorrect Type]");
					RetVal = false;
				}
			}

			if(AVT.HasStateVariable_AbsoluteCounterPosition)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("AbsoluteCounterPosition").GetNetType()==typeof(int))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: AbsoluteCounterPosition [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AbsoluteCounterPosition [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AbsoluteCounterPosition [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_AbsoluteTimePosition)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("AbsoluteTimePosition").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: AbsoluteTimePosition [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AbsoluteTimePosition [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AbsoluteTimePosition [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_AVTransportURI)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("AVTransportURI").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: AVTransportURI [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AVTransportURI [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AVTransportURI [MISSING]");
				RetVal = false;
			}	

			if(AVT.HasStateVariable_AVTransportURIMetaData)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("AVTransportURIMetaData").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: AVTransportURIMetaData [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AVTransportURIMetaData [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: AVTransportURIMetaData [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_CurrentMediaDuration)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentMediaDuration").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentMediaDuration [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentMediaDuration [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentMediaDuration [MISSING]");
				RetVal = false;
			}	

			if(AVT.HasStateVariable_CurrentPlayMode)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentPlayMode").GetNetType()==typeof(string))
				{
					bool PlayModeOK = false;
					foreach(string PM in AVT.Values_CurrentPlayMode)
					{
						if(PM=="NORMAL")
						{
							PlayModeOK = true;
							break;
						}
					}
					if(PlayModeOK)
					{
						AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentPlayMode [OK]");
					}
					else
					{
						AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentPlayMode [MISSING 'NORMAL' mode]");
						RetVal = false;
					}
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentPlayMode [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentPlayMode [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_CurrentTrack)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentTrack").GetNetType()==typeof(uint))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentTrack [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrack [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrack [MISSING]");
				RetVal = false;
			}	

			if(AVT.HasStateVariable_CurrentTrackDuration)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentTrackDuration").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentTrackDuration [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackDuration [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackDuration [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_CurrentTrackMetaData)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentTrackMetaData").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentTrackMetaData [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackMetaData [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackMetaData [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_CurrentTrackURI)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("CurrentTrackURI").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: CurrentTrackURI [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackURI [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: CurrentTrackURI [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_LastChange)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("LastChange").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: LastChange [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: LastChange [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: LastChange [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_NumberOfTracks)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("NumberOfTracks").GetNetType()==typeof(uint))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: NumberOfTracks [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: NumberOfTracks [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: NumberOfTracks [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_RelativeCounterPosition)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("RelativeCounterPosition").GetNetType()==typeof(int))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: RelativeCounterPosition [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: RelativeCounterPosition [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: RelativeCounterPosition [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_RelativeTimePosition)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("RelativeTimePosition").GetNetType()==typeof(string))
				{
					AddEvent(LogImportance.Remark,"StateVariable Values","AVT: RelativeTimePosition [OK]");
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: RelativeTimePosition [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: RelativeTimePosition [MISSING]");
				RetVal = false;
			}

			if(AVT.HasStateVariable_TransportState)
			{
				if(AVT.GetUPnPService().GetStateVariableObject("TransportState").GetNetType()==typeof(string))
				{
					bool TSOK = false;
					int NumTSOK = 0;
					foreach(string TS in  AVT.Values_TransportState)
					{
						if(TS=="STOPPED")
						{
							++NumTSOK;
							TSOK = true;
							break;
						}
					}
					if(!TSOK)
					{
						AddEvent(LogImportance.Critical,"StateVariable Values","AVT: TransportState ['STOPPED' missing]");
						RetVal = false;
					}

					TSOK = false;
					foreach(string TS in  AVT.Values_TransportState)
					{
						if(TS=="PAUSED_PLAYBACK")
						{
							++NumTSOK;
							TSOK = true;
							break;
						}
					}
					if(!TSOK)
					{
						AddEvent(LogImportance.Critical,"StateVariable Values","AVT: TransportState ['PAUSED_PLAYBACK' missing]");
						RetVal = false;
					}

					TSOK = false;
					foreach(string TS in  AVT.Values_TransportState)
					{
						if(TS=="PLAYING")
						{
							++NumTSOK;
							TSOK = true;
							break;
						}
					}
					if(!TSOK)
					{
						AddEvent(LogImportance.Critical,"StateVariable Values","AVT: TransportState ['PLAYING' missing]");
						RetVal = false;
					}

					if(NumTSOK==3)
					{
						AddEvent(LogImportance.Remark,"StateVariable Values","AVT: TransportState [OK]");
					}
				}
				else
				{
					AddEvent(LogImportance.Critical,"StateVariable Values","AVT: TransportState [Incorrect Type]");
					RetVal = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"StateVariable Values","AVT: TransportState [MISSING]");
				RetVal = false;
			}


			return(RetVal);
		}

		private bool Test_AVTransport_LastChange()
		{
			bool RetVal = true;
			AVT.OnStateVariable_LastChange += new CpAVTransport.StateVariableModifiedHandler_LastChange(AVT_LastChangeSink);
			Ev.Reset();

			AVT._subscribe(150);
			Ev.WaitOne(30000,false);

			string LC = AVT.LastChange;
			if(LC.Substring(0,1)!="<") LC = UPnPStringFormatter.UnEscapeString(LC);

			StringReader SR = new StringReader(LC);
			XmlTextReader XMLDoc = new XmlTextReader(SR);

			try
			{
				XMLDoc.Read();
				XMLDoc.MoveToContent();

				if(XMLDoc.NamespaceURI!="urn:schemas-upnp-org:metadata-1-0/AVT/")
				{
					//Invalid Namespace
					RetVal = false;
					AddEvent(LogImportance.Critical,"Event Formatting","LastChange event was not in the proper namespace");
				}
				else
				{
					// OK
					AddEvent(LogImportance.Remark,"Event Formatting","LastChange event appears OK");
				}
			}
			catch(System.Xml.XmlException)
			{
				// Not a well-formed XML
				RetVal = false;
				AddEvent(LogImportance.Critical,"Event Formatting","LastChange event was not a well formed XML");
			}

			AVT.GetUPnPService().UnSubscribe(null);
			return(RetVal);
		}
		private void AVT_LastChangeSink(CpAVTransport sender, string Val)
		{
			Ev.Set();
		}

		private bool ConnectionManagerEventsTest()
		{
			string ConnectionIDs = "";

			StartCountDown(30,90);
			CM.Sync_GetCurrentConnectionIDs(out ConnectionIDs);
			AbortCountDown();
			CM.OnStateVariable_CurrentConnectionIDs += new CpConnectionManager.StateVariableModifiedHandler_CurrentConnectionIDs(CurrentConnectionIDSink);
			CM._subscribe(250);


			StartCountDown(60,90);
			bool OK = true;
			if(Ev.WaitOne(30000,false))
			{
				if(CM.CurrentConnectionIDs!=ConnectionIDs)
				{
					AddEvent(LogImportance.Critical,"Connection Handling","   Inconsistent ConnectionID");
					AddEvent(LogImportance.Remark,"Connection Handling","      GetCurrentConnectionIDs[" + ConnectionIDs + "] Event[" + CM.CurrentConnectionIDs + "]");
					OK = false;
				}
			}
			else
			{
				AddEvent(LogImportance.Critical,"Connection Handling","   Did not receive events for CurrentConnectionIDs");
				OK = false;
			}

			if(OK)
			{
				AddEvent(LogImportance.Remark,"Connection Handling","   Correctly received CurrentConnectionIDs events");
			}
			CM.OnStateVariable_CurrentConnectionIDs -= new CpConnectionManager.StateVariableModifiedHandler_CurrentConnectionIDs(CurrentConnectionIDSink);
			AbortCountDown();

			return(OK);
		}
		private void CurrentConnectionIDSink(CpConnectionManager sender, string NewVal)
		{
			Ev.Set();
		}
		private bool PrepareForConnectionTest_SINK(string ProtocolInfoString)
		{
			DText parser = new DText();
			bool Found = false;
			parser.ATTRMARK = ",";
			bool RetVal = true;
			int ConnID=0,AVTID=0,RcsID=0;
			Ev.Reset();
			CM.OnStateVariable_CurrentConnectionIDs += new CpConnectionManager.StateVariableModifiedHandler_CurrentConnectionIDs(CurrentConnectionIDs_P4C);
			
			StartCountDown(CurrentTime,TotalTime);
			try
			{
				CM.Sync_PrepareForConnection(ProtocolInfoString,"/",-1,CpConnectionManager.Enum_A_ARG_TYPE_Direction.INPUT,out ConnID,out AVTID, out RcsID);
			}
			catch(UPnPInvokeException)
			{
				AddEvent(LogImportance.Critical,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection could not be created");
				RetVal = false;
			}
			AbortCountDown();
			CurrentTime += 30;
			
			if(RetVal)
			{
				StartCountDown(CurrentTime,TotalTime);
				if(Ev.WaitOne(30000,false))
				{
					AbortCountDown();
					CurrentTime += 30;

					// Check if evented correctly
					parser[0] = CM.CurrentConnectionIDs;
					Found = false;
					for(int i=1;i<=parser.DCOUNT();++i)
					{
						if(int.Parse(parser[i])==ConnID)
						{
							Found = true;
							break;
						}
					}
					if(Found)
					{
						// Event OK
						Ev.Reset();

						StartCountDown(CurrentTime,TotalTime);
						try
						{
							CM.Sync_ConnectionComplete(ConnID);
						}
						catch(UPnPInvokeException)
						{
							AddEvent(LogImportance.Critical,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection could not be closed");
							RetVal = false;
						}
						AbortCountDown();
						CurrentTime += 30;

						if(RetVal)
						{
							StartCountDown(CurrentTime,TotalTime);
							if(Ev.WaitOne(30000,false))
							{
								AddEvent(LogImportance.Remark,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection Handling OK");
							}
							else
							{
								AddEvent(LogImportance.Critical,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection Created/Evented, but CLOSE did not produce events");
								RetVal = false;
							}
							AbortCountDown();
							CurrentTime += 30;
						}

					}
					else
					{
						CurrentTime += 30;
						// Event not found, FAIL
						AddEvent(LogImportance.Critical,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection Created, but event failed");
						RetVal = false;
					}
				}
				else
				{
					CurrentTime += 30;
					// Failed to send event
					AddEvent(LogImportance.Critical,"Connection Handling","   Sink Protocol Info [" + ProtocolInfoString + "] Connection Created, but event missing");
					RetVal = false;
				}
			}
			CM.OnStateVariable_CurrentConnectionIDs -= new CpConnectionManager.StateVariableModifiedHandler_CurrentConnectionIDs(CurrentConnectionIDs_P4C);
			return(RetVal);
		}
		private void CurrentConnectionIDs_P4C(CpConnectionManager sender, string NewVal)
		{
			Ev.Set();
		}
		protected MediaResource[] BuildResources(string Interface, IDictionaryEnumerator en)
		{
			ArrayList a = new ArrayList();
			while(en.MoveNext())
			{
				FileInfo f = (FileInfo)en.Value;
				MediaResource mr;

				ResourceBuilder.ResourceAttributes resInfo = new ResourceBuilder.ResourceAttributes();
				resInfo.contentUri = "file://" + f.FullName;
				resInfo.protocolInfo = ProtocolInfoString.CreateHttpGetProtocolInfoString(f);

				mr = ResourceBuilder.CreateResource(resInfo);
				OpenSource.UPnP.AV.Extensions.MetaData.Finder.PopulateMetaData(mr,f);

				a.Add(mr);
			}
			return((MediaResource[])a.ToArray(typeof(MediaResource)));
		}
		private void CreateConnectionSink(AVRenderer sender, AVConnection c, object Tag)
		{
			TestConnection = c;
			Ev2.Set();
		}
		private void RecycledConnectionSink(AVRenderer sender, AVConnection c, object Tag)
		{
			TestConnection = c;
			Ev2.Set();
		}
		private void InitSink(AVRenderer sender)
		{
			Ev2.Set();
		}
		/*
		private bool HTTP_ScenarioTest()
		{
			MediaResource.AUTOMAPFILE = "file://";
			Ev2.Reset();
			AVRenderer renderer = new AVRenderer(TestDevice);
			renderer.OnInitialized += new AVRenderer.OnInitializedHandler(InitSink);
			if(!renderer.IsInit)
			{
				Ev2.WaitOne(15000,false);
			}

			renderer.OnCreateConnection += new AVRenderer.ConnectionHandler(CreateConnectionSink);
			renderer.OnRecycledConnection += new AVRenderer.ConnectionHandler(RecycledConnectionSink);
			Ev2.Reset();

			TestList = new Hashtable();
			TestList[1] = new FileInfo("C:\\1.mp3");
			TestList[2] = new FileInfo("C:\\2.mp3");
			TestList[3] = new FileInfo("C:\\3.mp3");

			MediaResource[] MR = BuildResources(TestDevice.InterfaceToHost.ToString(),TestList.GetEnumerator());
			renderer.CreateConnection(MR);

			Ev2.WaitOne(30000,false);
			if(TestConnection==null)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Could not create connection");
				return(false);
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Connection Successful");
			}

			Ev2.Reset();
			TestConnection.OnCurrentMetaDataChanged += new AVConnection.CurrentMetaDataChangedHandler(MetaDataSink);
			TestConnection.OnCurrentPlayModeChanged += new AVConnection.CurrentPlayModeChangedHandler(PlayModeSink);
			TestConnection.OnNumberOfTracksChanged += new AVConnection.NumberOfTracksChangedHandler(NumberOfTracksSink);
			TestConnection.OnPlayStateChanged += new AVConnection.PlayStateChangedHandler(PlayStateSink);
			TestConnection.OnTrackChanged += new AVConnection.CurrentTrackChangedHandler(TrackChangedSink);
			TestConnection.OnTrackURIChanged += new AVConnection.TrackURIChangedHandler(TrackURIChangedSink);

			Ev_MetaDataChanged.Reset();
			Ev_PlayModeChanged.Reset();
			Ev_NumberOfTracksChanged.Reset();
			Ev_PlayStateChanged.Reset();
			Ev_TrackChanged.Reset();
			Ev_TrackURIChanged.Reset();

			TestConnection.Play();

			Ev2.WaitOne(20000,false);
			if(TestConnection.CurrentState!=AVConnection.PlayState.PLAYING)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Renderer transition to Play State - FAILED");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Renderer transition to Play State - OK");
			}

			Ev_NumberOfTracksChanged.WaitOne(3000,false);
			if(TestConnection.NumberOfTracks!=3)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Correct number of Tracks - FAILED");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Correct number of Tracks - OK");
			}
			Ev_TrackChanged.WaitOne(3000,false);
			if(TestConnection.CurrentTrack!=1)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Correct Track Number - FAILED");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Correct Track Number - OK");
			}
			
			Ev_MetaDataChanged.WaitOne(3000,false);
			if(TestConnection.CurrentItem==null)
			{
				AddEvent(LogImportance.High,"HTTP Scenario","MetaData Handling - WARNING");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","MetaData Handling - OK");
			}

			Ev_PlayStateChanged.Reset();
			TestConnection.Pause();
			Ev_PlayStateChanged.WaitOne(10000,false);
			if(TestConnection.CurrentState!=AVConnection.PlayState.PAUSED)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Play --> Pause - FAILED");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Play --> Pause - OK");
			}

			Ev_PlayStateChanged.Reset();
			TestConnection.Play();
			Ev_PlayStateChanged.WaitOne(10000,false);
			if(TestConnection.CurrentState!=AVConnection.PlayState.PLAYING)
			{
				AddEvent(LogImportance.Critical,"HTTP Scenario","Pause --> Play - FAILED");
			}
			else
			{
				AddEvent(LogImportance.Remark,"HTTP Scenario","Pause --> Play - OK");
			}

			Ev2.Reset();
			

			return(true);
		}
		*/
		private void MetaDataSink(AVConnection c)
		{
			Ev_MetaDataChanged.Set();
		}
		private void PlayModeSink(AVConnection sender, AVConnection.PlayMode NewMode)
		{
			Ev_PlayModeChanged.Set();
		}
		private void NumberOfTracksSink(AVConnection sender, UInt32 NewNumberOfTracks)
		{
			Ev_NumberOfTracksChanged.Set();
		}
		private void PlayStateSink(AVConnection sender, AVConnection.PlayState NewState)
		{
			Ev_PlayStateChanged.Set();
		}
		private void TrackChangedSink(AVConnection sender, UInt32 NewTrackNumber)
		{
			Ev_TrackChanged.Set();
		}
		private void TrackURIChangedSink(AVConnection sender)
		{
			Ev_TrackURIChanged.Set();
		}
	}
}
