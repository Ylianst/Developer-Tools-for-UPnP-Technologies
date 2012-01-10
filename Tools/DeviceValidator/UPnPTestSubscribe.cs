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
using System.Threading;
using System.Net.Sockets;
using System.Collections;
using System.Runtime.Serialization;
using OpenSource.UPnP;

namespace UPnPValidator.BasicTests
{
	/// <summary>
	/// Tests service event subscription, un-subscription and subscription renewal.
	/// </summary>
	[Serializable()] 
	public class UPnPTestSubscribe : BasicTestGroup
	{
		// Specific Test Variables
		private Hashtable EventTable = new Hashtable();
		private int NumEvents = 0;
		[NonSerialized()] private ManualResetEvent MRE;
		[NonSerialized()] private string SID;

		public UPnPTestSubscribe() 
		{
			Category = "Eventing";
			GroupName = "Subscribe";
			Description = "Subscription test suite. Tests service event subscription, un-subscription, subscription renewal and check for correct results, correct error codes and proper eventing and formatting of event messages.";

			Reset();
		}

		public override void Reset() 
		{
			base.Reset();
			if (MRE == null) MRE = new ManualResetEvent(false);
		}

		public override void Start(UPnPDevice device) 
		{
			if(!Enabled) return;

			UPnPDevice dv = device;
			while(dv.ParentDevice!=null)
			{
				dv = dv.ParentDevice;
			}

			state = UPnPTestStates.Running;
			UPnPService[] _S = device.GetServices("urn:");

			foreach(UPnPService s in _S)
			{
				bool ok = false;
				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					if(v.SendEvent)
					{
						ok = true;
						break;
					}
				}
				if(ok)
				{

					UPnPDebugObject d = new UPnPDebugObject(s);
					Uri EventUri = new Uri((string)d.GetField("__eventurl"));
				
					IPEndPoint dest = new IPEndPoint(IPAddress.Parse(EventUri.Host),EventUri.Port);
					HTTPMessage R = new HTTPMessage();
					R.Directive = "SUBSCRIBE";
					R.DirectiveObj = HTTPMessage.UnEscapeString(EventUri.PathAndQuery);
					R.AddTag("Host",dest.ToString());
					R.AddTag("Callback","<http://" + dv.InterfaceToHost.ToString()+ ":" + NetworkInfo.GetFreePort(10000,50000,dv.InterfaceToHost).ToString() + ">");
					//R.AddTag("Callback","<http://127.0.0.1:55555>");
					R.AddTag("NT","upnp:event");
					R.AddTag("Timeout","Second-15");

					System.Console.WriteLine(R.GetTag("Callback"));

					MRE.Reset();
					SID = "";
					StartCountDown(30);
					HTTPRequest rq = new HTTPRequest();
					rq.OnResponse += new HTTPRequest.RequestHandler(SubscribeSink);
				
					AddHTTPMessage(R);

					rq.PipelineRequest(dest,R,s);
					MRE.WaitOne(30000,false);
					AbortCountDown();

					if (SID=="")
					{
						AddEvent(LogImportance.Critical,"Subscribe","SUBSCRIBE: " + s.ServiceURN + " << FAILED >>");
						AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

						result = "Subscription test failed."; // TODO
						state = UPnPTestStates.Failed;
						return;
					}
					else
					{
						AddEvent(LogImportance.Remark,"Subscribe","SUBSCRIBE: " + s.ServiceURN + " << OK >>");

						// Renew Test
						R = new HTTPMessage();
						R.Directive = "SUBSCRIBE";
						R.DirectiveObj = HTTPMessage.UnEscapeString(EventUri.PathAndQuery);
						R.AddTag("Host",dest.ToString());
						R.AddTag("SID",SID);
						R.AddTag("Timeout","Second-15");
						StartCountDown(30);
						SID = "";
						MRE.Reset();

						AddHTTPMessage(R);

						rq = new HTTPRequest();
						rq.OnResponse += new HTTPRequest.RequestHandler(SubscribeSink);
						rq.PipelineRequest(dest,R,s);

						MRE.WaitOne(30000,false);
						AbortCountDown();

						if (SID=="")
						{
							AddEvent(LogImportance.Critical,"Subscribe","SUBSCRIBE (Renew): " + s.ServiceURN + " << FAILED >>");
							AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

							result = "Subscription test failed."; // TODO
							state = UPnPTestStates.Failed;
							return;
						}
						else
						{
							AddEvent(LogImportance.Remark,"Subscribe","SUBSCRIBE (Renew): " + s.ServiceURN + " << OK >>");

							// Cancel
							R = new HTTPMessage();
							R.Directive = "UNSUBSCRIBE";
							R.DirectiveObj = HTTPMessage.UnEscapeString(EventUri.PathAndQuery);
							R.AddTag("Host",dest.ToString());
							R.AddTag("SID",SID);

							StartCountDown(30);
							SID = "";
							MRE.Reset();
							rq = new HTTPRequest();
							rq.OnResponse += new HTTPRequest.RequestHandler(CancelSink);

							AddHTTPMessage(R);

							rq.PipelineRequest(dest,R,s);

							MRE.WaitOne(30000,false);
							AbortCountDown();

							if (SID=="")
							{
								AddEvent(LogImportance.Critical,"Subscribe","UNSUBSCRIBE: " + s.ServiceURN + " << FAILED >>");
								AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

								result = "Subscription test failed.";
								state = UPnPTestStates.Failed;
								return;
							}
							else
							{
								AddEvent(LogImportance.Remark,"Subscribe","UNSUBSCRIBE: " + s.ServiceURN + " << OK >>");
							}

							/* Test for duplicate SID
							 * as well as initial events */
					
							EventTable.Clear();
							NumEvents = 0;
							foreach(UPnPStateVariable V in s.GetStateVariables())
							{
								if(V.SendEvent)
								{
									++ NumEvents;
									EventTable[V.Name] = false;
									V.OnModified -= new UPnPStateVariable.ModifiedHandler(StateVarModifiedSink);
									V.OnModified += new UPnPStateVariable.ModifiedHandler(StateVarModifiedSink);
								}
							}
							if(EventTable.Count>0)
							{
								MRE.Reset();
								s.OnSubscribe -= new UPnPService.UPnPEventSubscribeHandler(OnSubscribeSink);
								s.OnSubscribe += new UPnPService.UPnPEventSubscribeHandler(OnSubscribeSink);
								s.UnSubscribe(null);
								foreach(UPnPStateVariable V in s.GetStateVariables())
								{
									V.Clear();
								}
								s.Subscribe(120,null);
								MRE.WaitOne(30000,false);
								if(SID=="")
								{
									// Subscribe Failed
									AddEvent(LogImportance.Critical,"Subscribe","SUBSCRIBE(2): " + s.ServiceURN + " << FAILED >>");
									AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

									result = "Subscription test failed.";
									state = UPnPTestStates.Failed;
									return;
								}
								else
								{
									if(SID==null)
									{
										// Duplicate SID
										// Subscribe Failed
										AddEvent(LogImportance.Critical,"Subscribe","SUBSCRIBE(2): " + s.ServiceURN + " << FAILED, duplicate SID >>");
										AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

										result = "Subscription test failed.";
										state = UPnPTestStates.Failed;
										return;
									}
									else
									{
										// Check Hashtable
										IDictionaryEnumerator de = EventTable.GetEnumerator();
										bool OK = true;
										while(de.MoveNext())
										{
											if((bool)de.Value==false)
											{
												// No Event Received
												OK = false;
												AddEvent(LogImportance.Critical,"Subscribe","   StateVariable: " + (string)de.Key + " >> Event NOT received");
											}
											else
											{
												// OK
												AddEvent(LogImportance.Remark,"Subscribe","   StateVariable: " + (string)de.Key + " >> Event OK");
											}
										}
										if(OK==false)
										{
											AddEvent(LogImportance.Critical,"Subscribe","SUBSCRIBE(2): " + s.ServiceURN + " << FAILED, Did not receive all events >>");
											AddEvent(LogImportance.Remark,"Subscribe","Aborting tests");

											result = "Subscription test failed.";
											state = UPnPTestStates.Failed;
											return;
										}
									}
								}
							}
						}
					
					}
				}
			}

			result = "Subscribe Tests OK";
			state = UPnPTestStates.Pass;
		}
		private void StateVarModifiedSink(UPnPStateVariable sender, object newval)
		{
			EventTable[sender.Name] = true;
			if(Interlocked.Decrement(ref NumEvents)==0)
			{
				MRE.Set();
			}
		}
		private void OnSubscribeSink(UPnPService sender, bool OK)
		{
			if(OK==false)
			{
				SID = "";
				MRE.Set();
			}
			else
			{
				if((string)(new UPnPDebugObject(sender)).GetField("CurrentSID")==SID)
				{
					// Duplicate
					SID = null;
					MRE.Set();
				}
				else
				{
					SID = (string)(new UPnPDebugObject(sender)).GetField("CurrentSID");
				}
			}
		}

		private void SubscribeSink(HTTPRequest sender, HTTPMessage MSG, object Tag)
		{
			if (MSG!=null)
			{
				AddPacket(MSG);

				if (MSG.StatusCode==200)
				{
					SID = MSG.GetTag("SID");
				}
			}
			MRE.Set();
		}

		private void CancelSink(HTTPRequest sender, HTTPMessage MSG, object Tag)
		{
			if (MSG!=null)
			{
				AddPacket(MSG);

				if (MSG.StatusCode==200)
				{
					SID = "CANCELED";
				}
			}
			MRE.Set();
		}

		private void AddHTTPMessage(HTTPMessage msg)
		{
			AddPacket(msg);
		}

		private void StartCountDown(int seconds)
		{
			Countdown.AutoReset = true;
			this.TimeLeft = seconds;
			Countdown.Start();
		}

		/*
		private void CountdownSink(object sender, System.Timers.ElapsedEventArgs e)
		{
			--TimeLeft;
			if (TimeLeft == 0) Countdown.Stop();
		}
		

		private void AbortCountDown()
		{
			Countdown.Stop();
		}
	    */
	}


}
