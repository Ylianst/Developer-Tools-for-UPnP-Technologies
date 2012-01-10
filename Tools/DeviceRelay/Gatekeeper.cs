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
using System.Threading;
using System.Collections;
using OpenSource.UPnP;

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for Gatekeeper.
	/// </summary>
	public class Gatekeeper
	{
		internal Hashtable InvokeLaterTable = new Hashtable();
		int h = 0;
		public string PublicIP = "";

		public int GetNewHandle()
		{
			return(Interlocked.Increment(ref h));
		}

		private UPnPService.UPnPServiceInvokeHandler A_ICB = null;
		private UPnPService.UPnPServiceInvokeErrorHandler A_IECB = null;
		public delegate void OnUPnPRelayDeviceHandler(Gatekeeper sender, UPnPRelayDevice d);
		public event OnUPnPRelayDeviceHandler OnUPnPRelayDevice;
		public event OnUPnPRelayDeviceHandler OnUPnPRelayDeviceRemoved;

		public delegate void OnEventHandler(UPnPDevice sender, string Name);
		public event OnEventHandler OnEvent;

		public delegate void OnActionHandler(UPnPDevice DeviceSender, UPnPService ServiceSender, string ActionName);
		public event OnActionHandler OnAction;

		private int Port = 0;

		// BRIAN - TODO - I changed this to public, should be private
		public Hashtable ProxyTable = new Hashtable();

		private Hashtable FactoryTable = Hashtable.Synchronized(new Hashtable());
		private Hashtable ProxyFactoryTable = Hashtable.Synchronized(new Hashtable());
		private ArrayList ProcessLaterList = new ArrayList();

		private DvGateKeeper DV = null;
		private UPnPDevice Root = null;
		private ArrayList ShareList = new ArrayList();
		private Hashtable RegisteredTable = new Hashtable();

		~Gatekeeper()
		{
			IDictionaryEnumerator en = RegisteredTable.GetEnumerator();
			while(en.MoveNext())
			{
				foreach(UPnPDevice d in ShareList)
				{
					((CpGateKeeper)en.Value).RemoveDevice(d.UniqueDeviceName);
				}
			}
		}

		public void Dispose()
		{
			IDictionaryEnumerator en = RegisteredTable.GetEnumerator();
			while(en.MoveNext())
			{
				foreach(UPnPDevice d in ShareList)
				{
					((CpGateKeeper)en.Value).RemoveDevice(d.UniqueDeviceName);
				}
			}

			lock(ProxyTable)
			{
				en = ProxyTable.GetEnumerator();
				while(en.MoveNext())
				{
					((UPnPRelayDevice)en.Value).StopDevice();
				}
				ProxyTable.Clear();
			}
			Thread.Sleep(1000);
		}

		public Gatekeeper(int PortNumber)
		{
			A_ICB = new UPnPService.UPnPServiceInvokeHandler(A_InvokeSink);
			A_IECB = new UPnPService.UPnPServiceInvokeErrorHandler(A_InvokeErrorSink);

			Port = PortNumber;
			Root = UPnPDevice.CreateRootDevice(1000,1,"");
			Root.FriendlyName = "UPnPShare";

			Root.StandardDeviceType = "UPnPGateKeeper";
			DV = new DvGateKeeper();
			Root.AddService(DV);

			DV.External_Register = new DvGateKeeper.Delegate_Register(RegisterSink);
			DV.External_UnRegister = new DvGateKeeper.Delegate_UnRegister(UnRegisterSink);
			DV.External_GetDocument = new DvGateKeeper.Delegate_GetDocument(GetDocumentSink);
			DV.External_AddDevice = new DvGateKeeper.Delegate_AddDevice(AddDeviceSink);
			DV.External_RemoveDevice = new DvGateKeeper.Delegate_RemoveDevice(RemovedDeviceSink);
			DV.External_FireEvent = new DvGateKeeper.Delegate_FireEvent(FireEventSink);
			DV.External_GetStateTable = new DvGateKeeper.Delegate_GetStateTable(GetStateTableSink);

			DV.External_Invoke = new DvGateKeeper.Delegate_Invoke(InvokeSink);
			DV.External_InvokeAsync = new DvGateKeeper.Delegate_InvokeAsync(InvokeAsyncSink);
			DV.External_InvokeAsyncResponse = new DvGateKeeper.Delegate_InvokeAsyncResponse(InvokeAsyncResponseSink);

			(new UPnPDebugObject(Root)).SetField("NoSSDP",true);
			
			Root.StartDevice(PortNumber);	
		}
		
		public void AddDevice(UPnPDevice d)
		{
			ShareList.Add(d);
			d.OnRemoved += new UPnPDevice.OnRemovedHandler(RemovedSink);
			SubscribeAllServices(d);

			lock(RegisteredTable)
			{
				IDictionaryEnumerator de = RegisteredTable.GetEnumerator();
				while(de.MoveNext())
				{
					((CpGateKeeper)de.Value).AddDevice(Root.UniqueDeviceName,d.UniqueDeviceName);
				}
			}
		}
		public void RemoveDevice(UPnPDevice d)
		{
			d.OnRemoved -= new UPnPDevice.OnRemovedHandler(RemovedSink);
			UNSubscribeAllServices(d);
			ShareList.Remove(d);

			lock(RegisteredTable)
			{
				IDictionaryEnumerator de = RegisteredTable.GetEnumerator();
				while(de.MoveNext())
				{
					((CpGateKeeper)de.Value).RemoveDevice(d.UniqueDeviceName);
				}
			}
		}
		public void SwitchToPort(int PortNumber)
		{
			Port = PortNumber;
			Root.StopDevice();
			Root.StartDevice(PortNumber);
		}
		private void InvokeAsyncSink(System.String Caller, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Int32 Handle)
		{
			foreach(UPnPDevice d in ShareList)
			{
				UPnPDevice _D = d.GetDevice(DeviceUDN);
				if(_D!=null)
				{
					UPnPService s = _D.GetService(ServiceID);
					if(s!=null)
					{
						if(this.OnAction!=null) OnAction(d,s,Action);
						InvokeAsync(s,Action,InArgs, Caller, Handle);
						return;
					}
				}
			}
			throw(new UPnPCustomException(800,"No Match"));
		}
		private void InvokeAsyncResponseSink(System.Int32 Handle, System.Byte[] OutArgs, int ErrorCode, string ErrorString)
		{
			if(InvokeLaterTable.ContainsKey(Handle)==false) return;

			UPnPRelayDevice.InvokeResponseHandler CB = (UPnPRelayDevice.InvokeResponseHandler)InvokeLaterTable[Handle];
			InvokeLaterTable.Remove(Handle);
			CB(ErrorCode,ErrorString,OutArgs,Handle);
		}
		private void GetStateTableSink(System.String DeviceUDN, System.String ServiceID, out System.Byte[] Variables)
		{
			foreach(UPnPDevice d in ShareList)
			{
				UPnPDevice _D = d.GetDevice(DeviceUDN);
				if(_D!=null)
				{
					UPnPService s = _D.GetService(ServiceID);
					if(s!=null)
					{
						ArrayList t = new ArrayList();
						foreach(UPnPStateVariable V in s.GetStateVariables())
						{
							if(V.SendEvent)
							{
								t.Add(new UPnPArgument(V.Name,UPnPService.SerializeObjectInstance(V.Value)));
							}
						}
						Variables = Gatekeeper.BuildArguments((UPnPArgument[])t.ToArray(typeof(UPnPArgument)));
						return;
					}
				}
			}
			throw(new UPnPCustomException(800,"No Match"));
		}
		private void RemovedSink(UPnPDevice d)
		{
			lock(RegisteredTable)
			{
				IDictionaryEnumerator en = RegisteredTable.GetEnumerator();
				while(en.MoveNext())
				{
					CpGateKeeper cp = (CpGateKeeper)en.Value;
					cp.RemoveDevice(d.UniqueDeviceName);
				}
			}
		}
		private void RemovedDeviceSink(System.String DeviceUDN)
		{
			UPnPRelayDevice rd = null;
			lock(ProxyTable)
			{
				if(ProxyTable.ContainsKey(DeviceUDN))
				{
					rd = (UPnPRelayDevice)ProxyTable[DeviceUDN];
					rd.StopDevice();
					ProxyTable.Remove(DeviceUDN);			
				}
			}
			if(rd!=null && this.OnUPnPRelayDeviceRemoved!=null)
				OnUPnPRelayDeviceRemoved(this,rd);
		}
		private void SubscribeAllServices(UPnPDevice d)
		{
			foreach(UPnPDevice _D in d.EmbeddedDevices)
			{
				SubscribeAllServices(_D);
			}
			foreach(UPnPService S in d.Services)
			{
				foreach(UPnPStateVariable V in S.GetStateVariables())
				{
					if(V.SendEvent)
					{
						V.OnModified += new UPnPStateVariable.ModifiedHandler(EventSink);
					}
				}
				S.Subscribe(250,null);
			}
		}
		private void UNSubscribeAllServices(UPnPDevice d)
		{
			foreach(UPnPDevice _D in d.EmbeddedDevices)
			{
				UNSubscribeAllServices(_D);
			}
			foreach(UPnPService S in d.Services)
			{
				foreach(UPnPStateVariable V in S.GetStateVariables())
				{
					if(V.SendEvent)
					{
						V.OnModified  -= new UPnPStateVariable.ModifiedHandler(EventSink);
					}
				}
			}
		}
		private void FireEventSink(System.String DeviceUDN, System.String ServiceID, System.String StateVariable, System.String Value)
		{
			lock(ProxyTable)
			{
				IDictionaryEnumerator en = ProxyTable.GetEnumerator();
				while(en.MoveNext())
				{
					UPnPRelayDevice rd = (UPnPRelayDevice)en.Value;
					if(rd.ContainsUDN(DeviceUDN))
					{
						rd.FireEvent(DeviceUDN,ServiceID,StateVariable,Value);
						return;
					}
				}
			}
		}
		private void EventSink(UPnPStateVariable sender, object val)
		{
			lock(RegisteredTable)
			{
				IDictionaryEnumerator en = RegisteredTable.GetEnumerator();
				while(en.MoveNext())
				{
					CpGateKeeper cp = (CpGateKeeper)en.Value;

					cp.FireEvent(sender.OwningService.ParentDevice.UniqueDeviceName,
						sender.OwningService.ServiceID,
						sender.Name,
						UPnPService.SerializeObjectInstance(val));

				}
			}

			if(this.OnEvent!=null)
			{
				UPnPDevice _d = sender.OwningService.ParentDevice;
				while(_d.ParentDevice!=null)
					_d = _d.ParentDevice;
					
				OnEvent(_d,sender.Name);
			}
		}
		private void AddDeviceSink(string SenderUDN, System.String DeviceUDN)
		{
			lock(RegisteredTable)
			{
				if(RegisteredTable.ContainsKey(SenderUDN)==true)
				{
					CpGateKeeper HOME = (CpGateKeeper)RegisteredTable[SenderUDN];
					HOME.GetDocument(DeviceUDN,"",null,new CpGateKeeper.Delegate_OnResult_GetDocument(CPGetDocumentSink));
				}
				else
				{
					ProcessLaterList.Add(new object[2]{SenderUDN,DeviceUDN});
				}
			}
		}
		private void CPGetDocumentSink(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.Byte[] Document, UPnPInvokeException e, object _Tag)
		{
			if(e!=null) return;
			UTF8Encoding U = new UTF8Encoding();

			string XML = U.GetString(Document);
			Uri SourceUri = new Uri("http://127.0.0.1");
			IPAddress Intfce = null;

			UPnPDevice d = (UPnPDevice)(new UPnPDebugObject(typeof(OpenSource.UPnP.UPnPDevice))).InvokeStaticMethod("Parse",new object[3]
				{XML,SourceUri,Intfce});

			ProxyDeviceFactory df = new ProxyDeviceFactory(sender,d,new ProxyDeviceFactory.OnDeviceHandler(NewDeviceSink));
			ProxyFactoryTable[df] = df;
		}
		private void NewDeviceSink(ProxyDeviceFactory sender, UPnPRelayDevice d)
		{
			ProxyFactoryTable.Remove(sender);
			lock(ProxyTable)
			{
				ProxyTable[d.UDN] = d;
				d.DV = Root.UniqueDeviceName;
				d.Creator = this;
				d.InitStateTableThenStart();
			}

			if(this.OnUPnPRelayDevice!=null) OnUPnPRelayDevice(this,d);
		}

		private void ProxySink_NoReverse(UPnPDeviceFactory sender, UPnPDevice d, Uri LocationUri)
		{
			FactoryTable.Remove(sender);
			sender.Shutdown();
			CpGateKeeper CP = new CpGateKeeper(d.GetServices(CpGateKeeper.SERVICE_NAME)[0]);
			lock(RegisteredTable)
			{
				RegisteredTable[CP.GetUPnPService().ParentDevice.UniqueDeviceName] = CP;
				object[] PL = (object[])ProcessLaterList.ToArray(typeof(object));
				foreach(object PL2 in PL)
				{
					object[] PL3 = (object[])PL2;
					string SenderUDN = (string)PL3[0];
					string DeviceUDN = (string)PL3[1];

					if(RegisteredTable.ContainsKey(SenderUDN))
					{
						CpGateKeeper HOME = (CpGateKeeper)RegisteredTable[SenderUDN];
						HOME.GetDocument(DeviceUDN,"",null,new CpGateKeeper.Delegate_OnResult_GetDocument(CPGetDocumentSink));		
						ProcessLaterList.Remove(PL2);
					}
				}
			}
			foreach(UPnPDevice t in ShareList)
			{
				CP.AddDevice(Root.UniqueDeviceName,t.UniqueDeviceName);
			}
		}
		private void ProxySink(UPnPDeviceFactory sender, UPnPDevice d, Uri LocationUri)
		{
			FactoryTable.Remove(sender);
			sender.Shutdown();
			CpGateKeeper CP = new CpGateKeeper(d.GetServices(CpGateKeeper.SERVICE_NAME)[0]);
			string useThisIP = d.InterfaceToHost.ToString();
		
			if(this.PublicIP!="") {useThisIP = PublicIP;}
		

			lock(RegisteredTable)
			{
				RegisteredTable[CP.GetUPnPService().ParentDevice.UniqueDeviceName] = CP;
				object[] PL = (object[])ProcessLaterList.ToArray(typeof(object));
				foreach(object PL2 in PL)
				{
					object[] PL3 = (object[])PL2;
					string SenderUDN = (string)PL3[0];
					string DeviceUDN = (string)PL3[1];

					if(RegisteredTable.ContainsKey(SenderUDN))
					{
						CpGateKeeper HOME = (CpGateKeeper)RegisteredTable[SenderUDN];
						HOME.GetDocument(DeviceUDN,"",null,new CpGateKeeper.Delegate_OnResult_GetDocument(CPGetDocumentSink));		
						ProcessLaterList.Remove(PL2);
					}
				}
			}
			foreach(UPnPDevice t in ShareList)
			{
				CP.AddDevice(Root.UniqueDeviceName,t.UniqueDeviceName);
			}

			CP.Register(new Uri("http://" + useThisIP + ":" + Port.ToString()),false);
		}

		private void RegisterSink(System.Uri Proxy, bool Reverse)
		{
			UPnPDeviceFactory df;
			if(Reverse)
			{
				df = new UPnPDeviceFactory(Proxy,1000,new UPnPDeviceFactory.UPnPDeviceHandler(ProxySink), null, null, null);
			}
			else
			{
                df = new UPnPDeviceFactory(Proxy, 1000, new UPnPDeviceFactory.UPnPDeviceHandler(ProxySink_NoReverse), null, null, null);
			}
			FactoryTable[df] = df;
		}
		private void UnRegisterSink(System.Uri Proxy)
		{
		}
		private void GetDocumentSink(System.String DeviceUDN, System.String ServiceID, out System.Byte[] Document)
		{
			if(ServiceID!="")
			{
				foreach(UPnPDevice d in ShareList)
				{
					UPnPDevice _D = d.GetDevice(DeviceUDN);
					if(_D!=null)
					{
						UPnPService s = _D.GetService(ServiceID);
						if(s!=null)
						{
							Document = s.GetSCPDXml();
							return;
						}
					}
				}
			}
			else
			{
				foreach(UPnPDevice d in ShareList)
				{
					if(d.UniqueDeviceName==DeviceUDN)
					{
						Document = d.GetRootDeviceXML(DV.GetReceiver());
						return;
					}
				}
			}
			throw(new UPnPCustomException(800,"No Match"));
		}
		public void InvokeSink(System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, out System.Byte[] OutArgs)
		{
			OutArgs = null;
			foreach(UPnPDevice d in ShareList)
			{
				UPnPDevice _D = d.GetDevice(DeviceUDN);
				if(_D!=null)
				{
					UPnPService s = _D.GetService(ServiceID);
					if(s!=null)
					{
						if(this.OnAction!=null) OnAction(d,s,Action);
						Invoke(s,Action,InArgs,out OutArgs);
						return;
					}
				}
			}
			throw(new UPnPCustomException(800,"No Match"));
		}
		public void InvokeAsync(UPnPService S, string Action, byte[] InArgs, string Caller, int Handle)
		{
			UPnPAction A = S.GetAction(Action);
			ArrayList AList = new ArrayList();

			foreach(UPnPArgument arg in A.ArgumentList)
			{
				if(arg.IsReturnValue==false && arg.Direction=="out")
				{
					UPnPArgument _arg = (UPnPArgument)arg.Clone();
					_arg.DataValue = UPnPService.CreateObjectInstance(arg.RelatedStateVar.GetNetType(),null);
					AList.Add(_arg);
				}
			}

			UPnPArgument[] Temp = ParseArguments(InArgs);
			foreach(UPnPArgument _arg in Temp)
			{
				_arg.DataType = A.GetArg(_arg.Name).RelatedStateVar.ValueType;
				_arg.Direction = "in";
				AList.Add(_arg);
			}
			
			UPnPArgument[] Arguments = (UPnPArgument[])AList.ToArray(typeof(UPnPArgument));
			
			(new UPnPDebugObject(S)).SetProperty("ValidationMode",false);
			S.InvokeAsync(Action,Arguments,new Object[2]{Caller,Handle},A_ICB,A_IECB);
		}
		private void A_InvokeSink(UPnPService sender, String MethodName, UPnPArgument[] Args, Object ReturnValue, object Tag)
		{
			object[] state = (object[])Tag;
			string Caller = (string)state[0];
			int Handle = (int)state[1];


			ArrayList RetList = new ArrayList();
			UPnPAction A = sender.GetAction(MethodName);
			if(A.HasReturnValue==true)
			{
				UPnPArgument RA = (UPnPArgument)A.GetRetArg().Clone();
				RA.DataValue = ReturnValue;
				RetList.Add(RA);
			}

			foreach(UPnPArgument OA in Args)
			{
				if(OA.Direction=="out")
				{
					RetList.Add(OA);
				}
			}

			byte[] OutArgs = BuildArguments((UPnPArgument[])RetList.ToArray(typeof(UPnPArgument)));
			CpGateKeeper C = (CpGateKeeper)RegisteredTable[Caller];
			C.InvokeAsyncResponse(Handle,OutArgs,0,"");
		}
		private void A_InvokeErrorSink(UPnPService sender, String MethodName, UPnPArgument[] Args, UPnPInvokeException e, object Tag)
		{
			object[] state = (object[])Tag;
			string Caller = (string)state[0];
			int Handle = (int)state[1];

			CpGateKeeper C = (CpGateKeeper)RegisteredTable[Caller];
			if(e.UPNP!=null)
			{
				C.InvokeAsyncResponse(Handle,new byte[0],e.UPNP.ErrorCode,e.UPNP.ErrorDescription);
			}
			else
			{
				C.InvokeAsyncResponse(Handle,new byte[0],500,e.Message);
			}
		}

		public void Invoke(UPnPService S, string Action, byte[] InArgs, out byte[] OutArgs)
		{
			UPnPAction A = S.GetAction(Action);
			ArrayList AList = new ArrayList();

			foreach(UPnPArgument arg in A.ArgumentList)
			{
				if(arg.IsReturnValue==false && arg.Direction=="out")
				{
					UPnPArgument _arg = (UPnPArgument)arg.Clone();
					_arg.DataValue = UPnPService.CreateObjectInstance(arg.RelatedStateVar.GetNetType(),null);
					AList.Add(_arg);
				}
			}

			UPnPArgument[] Temp = ParseArguments(InArgs);
			foreach(UPnPArgument _arg in Temp)
			{
				_arg.DataType = A.GetArg(_arg.Name).RelatedStateVar.ValueType;
				_arg.Direction = "in";
				AList.Add(_arg);
			}
			
			UPnPArgument[] Arguments = (UPnPArgument[])AList.ToArray(typeof(UPnPArgument));
			
			(new UPnPDebugObject(S)).SetProperty("ValidationMode",false);
			object RetVal = null;

			try
			{
				RetVal = S.InvokeSync(Action,Arguments);
			}
			catch(UPnPInvokeException ie)
			{
				if(ie.UPNP!=null)
				{
					throw(ie.UPNP);
				}
				else
				{
					throw(ie);
				}
			}
			
			ArrayList RetList = new ArrayList();
			
			if(A.HasReturnValue==true)
			{
				UPnPArgument RA = (UPnPArgument)A.GetRetArg().Clone();
				RA.DataValue = RetVal;
				RetList.Add(RA);
			}

			foreach(UPnPArgument OA in Arguments)
			{
				if(OA.Direction=="out")
				{
					RetList.Add(OA);
				}
			}

			OutArgs = BuildArguments((UPnPArgument[])RetList.ToArray(typeof(UPnPArgument)));
		}

		public static UPnPArgument[] ParseArguments(byte[] Arguments)
		{
			ArrayList RetList = new ArrayList();
			int LofA = 0;
			int i=0;

			if(Arguments.Length==0) return(new UPnPArgument[0]);

			do
			{
				LofA = BitConverter.ToInt32(Arguments,i);
				RetList.Add(ParseArgument(Arguments,i,LofA));
				i+=LofA;
			}while(i<Arguments.Length);

			return((UPnPArgument[])RetList.ToArray(typeof(UPnPArgument)));

		}
		public static UPnPArgument ParseArgument(byte[] buffer, int offset, int count)
		{
			UTF8Encoding U = new UTF8Encoding();
			int LON = BitConverter.ToInt32(buffer,offset + 4);
			string name = U.GetString(buffer,offset + 8,LON);
			int LOV = BitConverter.ToInt32(buffer,offset + 8 + LON);

			string StringValue = U.GetString(buffer,offset + 12+LON,LOV);
			UPnPArgument RetVal = new UPnPArgument(name,StringValue);
			return(RetVal);
		}
		public static byte[] BuildArguments(UPnPArgument[] Args)
		{
			MemoryStream ms = new MemoryStream();
			UTF8Encoding U = new UTF8Encoding();

			byte[] Name;
			byte[] Val;
			byte[] LofA;
			byte[] LofN;
			byte[] LofV;

			foreach(UPnPArgument A in Args)
			{
				Name = U.GetBytes(A.Name);
				Val = U.GetBytes(UPnPService.SerializeObjectInstance(A.DataValue));
				
				LofV = BitConverter.GetBytes(Val.Length);
				LofN = BitConverter.GetBytes(Name.Length);
				LofA = BitConverter.GetBytes(Val.Length+
									4+
									Name.Length+
									4+
									4);
				ms.Write(LofA,0,LofA.Length);
				ms.Write(LofN,0,LofN.Length);
				ms.Write(Name,0,Name.Length);
				ms.Write(LofV,0,LofV.Length);
				ms.Write(Val,0,Val.Length);
			}
			ms.Flush();
			return(ms.ToArray());
		}
	}
}
