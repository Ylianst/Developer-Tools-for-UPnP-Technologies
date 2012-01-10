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
using System.Net;
using System.Text;
using System.Data;
using System.Drawing;
using System.Threading;
using System.Reflection;
using System.Net.Sockets;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using System.Runtime.Serialization;
using OpenSource.UPnP;

namespace UPnPValidator.BasicTests
{
	/// <summary>
	/// Tests service event subscription, un-subscription and subscription renewal.
	/// </summary>
	[Serializable()] 
	public class UPnPTestInvokeValid : BasicTestGroup
	{
		// Generic Test Variables
		bool WARN_MASTER,WARN_VALID,WARN_ALLOWED,WARN_RANGE;
		bool FAIL_MASTER,FAIL_VALID,FAIL_ALLOWED,FAIL_RANGE;

		// Specific Test Variables
		[NonSerialized()] private ManualResetEvent MRE;

		public UPnPTestInvokeValid() 
		{
			Category = "Control";
			GroupName = "Invocation Test Suite";
			Description = "Action invocation test suite. Includes invoking each action of each service on the target device with valid and invalid values, checking for returned error codes, response time and many other factors.";
			AddTest("Valid Values","");
			AddTest("Allowed Values","");
			AddTest("Range","");
			Reset();
		}

		public override void Reset() 
		{
			base.Reset();
			if (MRE == null) MRE = new ManualResetEvent(false);
		}

		public override void Start(UPnPDevice device) 
		{
			Reset();

			UPnPService[] _S = device.GetServices("urn:");

			TotalTime = 0;
			int CurrentTime = 0;
			foreach(UPnPService s in _S)
			{
				TotalTime += s.Actions.Count*30;
			}
			foreach(UPnPService s in _S)
			{
				UPnPServiceWatcher W = new UPnPServiceWatcher(s,null,new UPnPServiceWatcher.SniffPacketHandler(SniffPacketSink));

				foreach(UPnPAction A in s.Actions)
				{
					StartCountDown(CurrentTime,TotalTime);
					BasicControlTest_ACTION(s,A);
					AbortCountDown();
					CurrentTime += 30;
				}
			}
			
			if(WARN_VALID && !FAIL_VALID)
			{
				Results.Add("Invocations: Valid Values  -  Had undesirable behavior");
				SetState("Valid Values", UPnPTestStates.Warn);
			}
			else
			{
				if(FAIL_VALID)
				{
					Results.Add("Invocations: Valid Values  -  Failed");
					SetState("Valid Values",UPnPTestStates.Failed);
				}
				else
				{
					Results.Add("Invocations: Valid Values  -  Passed");
					SetState("Valid Values",UPnPTestStates.Pass);
				}
			}

			CurrentTime = 0;
			foreach(UPnPService s in _S)
			{
				UPnPServiceWatcher W = new UPnPServiceWatcher(s,null,new UPnPServiceWatcher.SniffPacketHandler(SniffPacketSink));

				foreach(UPnPAction A in s.Actions)
				{
					StartCountDown(CurrentTime,TotalTime);
					BasicControlTest_ACTION_BadAllowedValues(s,A);
					AbortCountDown();
					CurrentTime += 30;
				}
			}
			if(WARN_ALLOWED && !FAIL_ALLOWED)
			{
				Results.Add("Invocations: Allowed Values  -  Had undesirable behavior");
				SetState("Allowed Values",UPnPTestStates.Warn);
			}
			else
			{
				if(FAIL_ALLOWED)
				{
					Results.Add("Invocations: Allowed Values  -  Failed");
					SetState("Allowed Values",UPnPTestStates.Failed);
				}
				else
				{
					Results.Add("Invocations: Allowed Values  -  Passed");
					SetState("Allowed Values",UPnPTestStates.Pass);
				}
			}

			CurrentTime = 0;
			foreach(UPnPService s in _S)
			{
				UPnPServiceWatcher W = new UPnPServiceWatcher(s,null,new UPnPServiceWatcher.SniffPacketHandler(SniffPacketSink));

				foreach(UPnPAction A in s.Actions)
				{
					StartCountDown(CurrentTime,TotalTime);
					BasicControlTest_ACTION_BadRange(s,A);
					AbortCountDown();
					CurrentTime += 30;
				}
			}
			if(WARN_RANGE && !FAIL_RANGE)
			{
				Results.Add("Invocations: Range  -  Had undesirable behavior");
				SetState("Range",UPnPTestStates.Warn);
			}
			else
			{
				if(FAIL_RANGE)
				{
					Results.Add("Invocations: Range  -  Failed");
					SetState("Range",UPnPTestStates.Failed);
				}
				else
				{
					Results.Add("Invocations: Range  -  Passed");
					SetState("Range",UPnPTestStates.Pass);
				}
			}

			WARN_MASTER = WARN_VALID||WARN_ALLOWED||WARN_RANGE;
			FAIL_MASTER = FAIL_VALID||FAIL_ALLOWED||FAIL_RANGE;

			if(WARN_MASTER && !FAIL_MASTER)
			{
				state = UPnPTestStates.Warn;
			}
			else
			{
				if(FAIL_MASTER)
				{
					state = UPnPTestStates.Failed;
				}
				else
				{
					state = UPnPTestStates.Pass;
				}
			}

			SetProgress(100);			
		}

		private void SniffPacketSink(UPnPServiceWatcher sender, HTTPMessage MSG)
		{
			AddPacket(MSG);
		}

		private bool BasicControlTest_ACTION(UPnPService s, UPnPAction A)
		{
			ArrayList ArgList = BasicControlTest_BuildActionArgs(A);
			try
			{
				s.InvokeSync(A.Name,(UPnPArgument[])ArgList.ToArray(typeof(UPnPArgument)));
				AddEvent(LogImportance.Remark,"Valid Values","Invoke: " + A.Name + " << OK >>");
			}
			catch(UPnPInvokeException ex)
			{
				if(ex.UPNP.ErrorCode==401 ||
					ex.UPNP.ErrorCode==402)
				{
					// Oh really?
					WARN_VALID = true;
					AddEvent(LogImportance.High,"Valid Values",
						"   Invoke: " + A.Name);
					AddEvent(LogImportance.High,"Valid Values",
						"      Cannot return Error Code " + ex.UPNP.ErrorCode.ToString());

					foreach(UPnPArgument _arg in ex.Arguments)
					{
						AddEvent(LogImportance.High,"Valid Values",
							"         " + _arg.Name + ": " + UPnPService.SerializeObjectInstance(_arg.DataValue));
					}
				}
				else
				{
					// Fine, I suppose
					AddEvent(LogImportance.Remark,"Valid Values",
						"Invoke: " + A.Name + " << " + ex.UPNP.ErrorCode + " " + ex.UPNP.ErrorDescription + "  OK >>");
				}
			}
			return true;
		}

		private bool BasicControlTest_ACTION_BadRange(UPnPService s, UPnPAction A)
		{
			(new UPnPDebugObject(s)).SetProperty("ValidationMode",false);

			ArrayList ArgList = null;
			foreach(UPnPArgument arg in A.ArgumentList)
			{
				if(arg.IsReturnValue==false && arg.Direction=="in")
				{
					if(arg.RelatedStateVar.Maximum!=null || 
						arg.RelatedStateVar.Minimum!=null)
					{
						ArgList = BasicControlTest_BuildActionArgs(A);
						for(int i=0;i<ArgList.Count;++i)
						{
							if(((UPnPArgument)ArgList[i]).Name==arg.Name)
							{
								if(arg.RelatedStateVar.Minimum!=null)
								{
									FieldInfo mi = arg.RelatedStateVar.GetNetType().GetField("MinValue");
									if(mi.GetValue(null).ToString()!=arg.RelatedStateVar.Minimum.ToString())
									{
										((UPnPArgument)ArgList[i]).DataValue = mi.GetValue(null);
										
										AddEvent(LogImportance.Remark,"Range","Invoke <<" + arg.Name + " value too small>> " + A.Name);
										try
										{
											s.InvokeSync(A.Name,(UPnPArgument[])ArgList.ToArray(typeof(UPnPArgument)));
											WARN_RANGE = true;
											AddEvent(LogImportance.High,"Range","   Device failed to validate argument");
										}
										catch(UPnPInvokeException ex)
										{
											if(ex.UPNP!=null)
											{
												if(ex.UPNP.ErrorCode<501)
												{
													AddEvent(LogImportance.Remark,"Range","   Device SUCCESSFULLY validated argument");
				
												}
												else
												{
													WARN_RANGE = true;
													AddEvent(LogImportance.High,"Range","   Device returned code: " + ex.UPNP.ErrorCode.ToString() + " but failed to validate argument");
												}
											}
											else
											{
												FAIL_RANGE = true;
												AddEvent(LogImportance.Critical,"Range","   Device returned non SOAP-Encoded Error");
											}
										}
									}
								}
								if(arg.RelatedStateVar.Maximum!=null)
								{
									FieldInfo mi = arg.RelatedStateVar.GetNetType().GetField("MaxValue");
									if(mi.GetValue(null).ToString()!=arg.RelatedStateVar.Maximum.ToString())
									{
										((UPnPArgument)ArgList[i]).DataValue = mi.GetValue(null);

										AddEvent(LogImportance.Remark,"Range","Invoke <<" + arg.Name + " value too big>> " + A.Name);
										try
										{
											s.InvokeSync(A.Name,(UPnPArgument[])ArgList.ToArray(typeof(UPnPArgument)));
											WARN_RANGE = true;
											AddEvent(LogImportance.High,"Range","   Device failed to validate argument");
										}
										catch(UPnPInvokeException ex)
										{
											if(ex.UPNP!=null)
											{
												if(ex.UPNP.ErrorCode<501)
												{
													AddEvent(LogImportance.Remark,"Range","   Device SUCCESSFULLY validated argument");
				
												}
												else
												{
													WARN_RANGE = true;
													AddEvent(LogImportance.High,"Range","   Device returned code: " + ex.UPNP.ErrorCode.ToString() + " but failed to validate argument");
												}
											}
											else
											{
												FAIL_RANGE = true;
												AddEvent(LogImportance.Critical,"Range","   Device returned non SOAP-Encoded Error");
											}
										}
									}
								}

							}
						}
					}
				}
			}

			(new UPnPDebugObject(s)).SetProperty("ValidationMode",true);
			return true;
		}

		private ArrayList BasicControlTest_BuildActionArgs(UPnPAction A)
		{
			ArrayList ArgList = new ArrayList();
			foreach(UPnPArgument arg in A.ArgumentList)
			{
				if(arg.IsReturnValue==false)
				{
					UPnPArgument NArg;
//					if(arg.RelatedStateVar.GetNetType().FullName=="System.String")
//					{
//						NArg = new UPnPArgument(arg.Name,"Sample String");
//					}
//					else
//					{
						NArg = new UPnPArgument(arg.Name,UPnPService.CreateObjectInstance(arg.RelatedStateVar.GetNetType(),null));
//					}
					if(arg.RelatedStateVar.AllowedStringValues!=null) NArg.DataValue = arg.RelatedStateVar.AllowedStringValues[0];
					if(arg.RelatedStateVar.Minimum!=null) NArg.DataValue = arg.RelatedStateVar.Minimum;
					if(arg.RelatedStateVar.Maximum!=null) NArg.DataValue = arg.RelatedStateVar.Maximum;
					ArgList.Add(NArg);
				}
			}
			return(ArgList);
		}

		private bool BasicControlTest_ACTION_BadAllowedValues(UPnPService s, UPnPAction A)
		{
			(new UPnPDebugObject(s)).SetProperty("ValidationMode",false);
			ArrayList ArgList = null;
			foreach(UPnPArgument arg in A.ArgumentList)
			{
				if(arg.IsReturnValue==false && arg.Direction=="in")
				{
					if(arg.RelatedStateVar.AllowedStringValues!=null)
					{
						ArgList = BasicControlTest_BuildActionArgs(A);
						for(int i=0;i<ArgList.Count;++i)
						{
							if(((UPnPArgument)ArgList[i]).Name==arg.Name)
							{
								string val = "!@#$";
								bool OK = true;
								do
								{
									OK = true;
									for(int j=0;j<arg.RelatedStateVar.AllowedStringValues.Length;++j)
									{
										if(val==arg.RelatedStateVar.AllowedStringValues[j])
										{
											OK = false;
											break;
										}
									}
									if(OK==false) val += val;
								}while (OK==false);
								((UPnPArgument)ArgList[i]).DataValue = val;
								
								AddEvent(LogImportance.Remark,"Allowed Values","Invoke <<Bad " + arg.Name + ">> " + A.Name);
								try
								{
									s.InvokeSync(A.Name,(UPnPArgument[])ArgList.ToArray(typeof(UPnPArgument)));
									WARN_ALLOWED = true;
									AddEvent(LogImportance.High,"Allowed Values","   Device failed to validate argument");
								}
								catch(UPnPInvokeException ex)
								{
									if(ex.UPNP!=null)
									{
										if(ex.UPNP.ErrorCode<501)
										{
											AddEvent(LogImportance.Remark,"Allowed Values","   Device SUCCESSFULLY validated argument");
										}
										else
										{
											WARN_ALLOWED = true;
											AddEvent(LogImportance.Remark,"Allowed Values","   Device returned code: " + ex.UPNP.ErrorCode.ToString() + " but failed to validate argument");
										}
									}
									else
									{
										FAIL_ALLOWED = true;
										AddEvent(LogImportance.Critical,"Allowed Values","   Device returned non SOAP-Encoded Error");
									}
								}
							}
						}
					}
				}
			}
			(new UPnPDebugObject(s)).SetProperty("ValidationMode",true);
			return true;
		}

		private void AddHTTPMessage(HTTPMessage msg)
		{
			AddPacket(msg);
		}

	}


}
