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
using System.Text;
using OpenSource.UPnP;

namespace UPnPRelay
{
	/// <summary>
	/// Summary description for ProxyDeviceFactory.
	/// </summary>
	public class ProxyDeviceFactory
	{
		public delegate void OnDeviceHandler(ProxyDeviceFactory sender, UPnPRelayDevice D);
		public event OnDeviceHandler OnDevice;

		private int Counter = 0;
		private CpGateKeeper HOME;
		private UPnPDevice _D;

		~ProxyDeviceFactory()
		{
			OpenSource.Utilities.InstanceTracker.Remove(this);
		}
		public ProxyDeviceFactory(CpGateKeeper home, UPnPDevice D, OnDeviceHandler Callback)
		{
			OpenSource.Utilities.InstanceTracker.Add(this);
			HOME = home;
			OnDevice += Callback;
			_D = D;

			foreach(UPnPDevice ed in D.EmbeddedDevices)
			{
				ProcessEmbeddedDevice(ed,false);
			}
			ProcessServices(D,false);

			foreach(UPnPDevice ed in D.EmbeddedDevices)
			{
				ProcessEmbeddedDevice(ed,true);
			}
			ProcessServices(D,true);
		}

		private void ProcessEmbeddedDevice(UPnPDevice D, bool OK)
		{
			foreach(UPnPDevice ed in D.EmbeddedDevices)
			{
				ProcessEmbeddedDevice(ed, OK);
			}
			ProcessServices(D, OK);
		}
		private void ProcessServices(UPnPDevice D, bool OK)
		{
			foreach(UPnPService S in D.Services)
			{
				if(OK==false)
				{
					++Counter;
				}
				else
				{
					HOME.GetDocument(D.UniqueDeviceName,S.ServiceID,S,new CpGateKeeper.Delegate_OnResult_GetDocument(GetDocumentSink));
				}
			}
		}
		private void GetDocumentSink(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.Byte[] Document, UPnPInvokeException e, object _Tag)
		{
			if(e!=null) return;
			UTF8Encoding U = new UTF8Encoding();

			string XML = U.GetString(Document);
			UPnPService S = (UPnPService)_Tag;

			(new UPnPDebugObject(S)).InvokeNonStaticMethod("ParseSCPD",new object[1]{XML});
			--Counter;
			if(Counter==0)
			{
				if(OnDevice!=null) OnDevice(this,new UPnPRelayDevice(_D,HOME));
			}
		}

	}
}
