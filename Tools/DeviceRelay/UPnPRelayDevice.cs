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
using System.Threading;
using System.Collections;
using OpenSource.UPnP;

namespace UPnPRelay
{
    /// <summary>
    /// Summary description for UPnPRelayDevice.
    /// </summary>
    public class UPnPRelayDevice
    {
        public delegate void InvokeResponseHandler(int ErrorCode, string ErrorString, byte[] Args, int Handle);
        public InvokeResponseHandler ILCB;

        private long ActionCounter = 0;
        private long EventCounter = 0;

        private int InitCounter = 0;

        public delegate void ActionCounterHandler(UPnPRelayDevice sender, long ActionCounter);
        public delegate void EventCounterHandler(UPnPRelayDevice sender, long EventCounter);
        public event ActionCounterHandler OnAction;
        public event EventCounterHandler OnEvent;
        public string DV = "";
        public Gatekeeper Creator = null;

        private UPnPDevice ProxyDevice;
        public readonly CpGateKeeper CP;
        private Hashtable UDNTable = new Hashtable();
        private Hashtable ReverseUDNTable = new Hashtable();
        private Hashtable PendingActionTable = new Hashtable();

        ~UPnPRelayDevice()
        {
            OpenSource.Utilities.InstanceTracker.Remove(this);
            if (ProxyDevice != null)
            {
                ProxyDevice.StopDevice();
                ProxyDevice = null;
            }
        }
        public UPnPRelayDevice(UPnPDevice device, CpGateKeeper _CP)
        {
            OpenSource.Utilities.InstanceTracker.Add(this);
            ILCB = new InvokeResponseHandler(InvokeResponseSink);
            CP = _CP;
            ProxyDevice = UPnPDevice.CreateRootDevice(750, double.Parse(device.Version), "");
            ProxyDevice.UniqueDeviceName = Guid.NewGuid().ToString();

            ProxyDevice.HasPresentation = false;
            ProxyDevice.FriendlyName = "*" + device.FriendlyName;
            ProxyDevice.Manufacturer = device.Manufacturer;
            ProxyDevice.ManufacturerURL = device.ManufacturerURL;
            ProxyDevice.ModelName = device.ModelName;
            ProxyDevice.DeviceURN = device.DeviceURN;


            foreach (UPnPService S in device.Services)
            {
                UPnPService S2 = (UPnPService)S.Clone();
                foreach (UPnPAction A in S2.Actions)
                {
                    A.ParentService = S2;
                    A.SpecialCase += new UPnPAction.SpecialInvokeCase(InvokeSink);
                }

                UPnPDebugObject dbg = new UPnPDebugObject(S2);

                dbg.SetField("SCPDURL", "_" + S2.ServiceID + "_scpd.xml");
                dbg.SetProperty("ControlURL", "_" + S2.ServiceID + "_control");
                dbg.SetProperty("EventURL", "_" + S2.ServiceID + "_event");
                ProxyDevice.AddService(S2);
            }

            UDNTable[device.UniqueDeviceName] = ProxyDevice;
            ReverseUDNTable[ProxyDevice.UniqueDeviceName] = device.UniqueDeviceName;
            foreach (UPnPDevice _ed in device.EmbeddedDevices)
            {
                ProcessDevice(_ed);
            }
        }
        public void InitStateTableThenStart()
        {
            InitCounter = CountServices(ProxyDevice);
            InitStateTable(ProxyDevice);
        }
        private int CountServices(UPnPDevice d)
        {
            int RetVal = 0;
            RetVal += d.Services.Length;
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                RetVal += CountServices(ed);
            }
            return (RetVal);
        }
        private void InitStateTable(UPnPDevice d)
        {
            foreach (UPnPService s in d.Services)
            {
                InitStateTable(d, s);
            }
            foreach (UPnPDevice ed in d.EmbeddedDevices)
            {
                InitStateTable(ed);
            }
        }
        private void InitStateTable(UPnPDevice d, UPnPService s)
        {
            string _udn = (string)this.ReverseUDNTable[d.UniqueDeviceName];
            CP.GetStateTable(_udn, s.ServiceID, s, new CpGateKeeper.Delegate_OnResult_GetStateTable(GetStateTableSink));
        }
        private void GetStateTableSink(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.Byte[] Variables, UPnPInvokeException e, object _Tag)
        {
            UPnPService s = (UPnPService)_Tag;
            UPnPArgument[] Vars = Gatekeeper.ParseArguments(Variables);
            foreach (UPnPArgument var in Vars)
            {
                (new UPnPDebugObject(s)).SetProperty("ValidationMode", false);
                s.SetStateVariable(var.Name, var.DataValue);
            }
            if (Interlocked.Decrement(ref InitCounter) == 0)
            {
                ProxyDevice.StartDevice();
            }
            if (Vars.Length > 0)
            {
                EventCounter += Vars.Length;
                if (this.OnEvent != null) OnEvent(this, EventCounter);
            }
        }

        private void ProcessDevice(UPnPDevice device)
        {
            UPnPDevice d = UPnPDevice.CreateEmbeddedDevice(double.Parse(device.Version), Guid.NewGuid().ToString());
            d.FriendlyName = "*" + device.FriendlyName;
            d.Manufacturer = device.Manufacturer;
            d.ManufacturerURL = device.ManufacturerURL;
            d.ModelDescription = device.ModelDescription;
            d.ModelURL = device.ModelURL;
            d.DeviceURN = device.DeviceURN;
            UDNTable[device.UniqueDeviceName] = d;
            ReverseUDNTable[d.UniqueDeviceName] = device.UniqueDeviceName;

            foreach (UPnPService S in device.Services)
            {
                UPnPService S2 = (UPnPService)S.Clone();
                foreach (UPnPAction A in S2.Actions)
                {
                    A.ParentService = S2;
                    A.SpecialCase += new UPnPAction.SpecialInvokeCase(InvokeSink);
                }

                UPnPDebugObject dbg = new UPnPDebugObject(S2);

                dbg.SetField("SCPDURL", "_" + S2.ServiceID + "_scpd.xml");
                dbg.SetProperty("ControlURL", "_" + S2.ServiceID + "_control");
                dbg.SetProperty("EventURL", "_" + S2.ServiceID + "_event");
                d.AddService(S2);
            }
            ((UPnPDevice)UDNTable[device.ParentDevice.UniqueDeviceName]).AddDevice(d);

            foreach (UPnPDevice _ed in device.EmbeddedDevices)
            {
                ProcessDevice(_ed);
            }

        }

        public void StartDevice()
        {
            ProxyDevice.StartDevice();
        }
        public void StopDevice()
        {
            ProxyDevice.StopDevice();
        }
        public string FriendlyName
        {
            get
            {
                return (ProxyDevice.FriendlyName);
            }
        }
        public Uri Origin
        {
            get
            {
                UPnPDevice d = CP.GetUPnPService().ParentDevice;
                while (d.ParentDevice != null)
                    d = d.ParentDevice;
                return (d.BaseURL);
            }
        }
        public string UDN
        {
            get
            {
                return ((string)ReverseUDNTable[ProxyDevice.UniqueDeviceName]);
            }
        }
        public bool ContainsUDN(string UDN)
        {
            return (UDNTable.ContainsKey(UDN));
        }

        public void FireEvent(string UDN, string ID, string VarName, string VarValue)
        {
            ++EventCounter;
            UPnPDevice d = (UPnPDevice)UDNTable[UDN];
            UPnPService s = d.GetService(ID);

            (new UPnPDebugObject(s)).SetProperty("ValidationMode", false);
            s.SetStateVariable(VarName, VarValue);

            if (this.OnEvent != null) OnEvent(this, EventCounter);
        }

        private void InvokeResponseSink(int ErrorCode, string ErrorString, byte[] Args, int Handle)
        {
            try
            {
                UPnPAction A = (UPnPAction)this.PendingActionTable[Handle];
                PendingActionTable.Remove(Handle);
                if (ErrorCode == 0)
                {
                    UPnPArgument[] OutArgs = Gatekeeper.ParseArguments(Args);
                    object RetObj = null;
                    if (A.HasReturnValue)
                    {
                        UPnPArgument RA = A.GetRetArg();
                        foreach (UPnPArgument OA in OutArgs)
                        {
                            if (OA.Name == RA.Name)
                            {
                                RetObj = OA.DataValue;
                                break;
                            }
                        }
                    }
                    A.ParentService.DelayedInvokeResponse(0, RetObj, OutArgs, null);
                }
                else
                {
                    A.ParentService.DelayedInvokeResponse(0, null, null, new UPnPCustomException(ErrorCode, ErrorString));
                }
            }
            catch (Exception) { }
        }
        private void InvokeSink(UPnPAction sender, UPnPArgument[] _Args, out object RetVal, out UPnPArgument[] _OutArgs)
        {
            string DeviceUDN = (string)ReverseUDNTable[sender.ParentService.ParentDevice.UniqueDeviceName];
            string ServiceID = sender.ParentService.ServiceID;

            byte[] InArgs = Gatekeeper.BuildArguments(_Args);
            //byte[] OutArgs;

            ++ActionCounter;
            if (this.OnAction != null) OnAction(this, ActionCounter);

            int Handle = Creator.GetNewHandle();
            Creator.InvokeLaterTable[Handle] = ILCB;
            PendingActionTable[Handle] = sender;
            CP.InvokeAsync(DV, DeviceUDN, ServiceID, sender.Name, InArgs, Handle);

            UPnPArgument[] Args;
            sender.ParentService.DelayInvokeRespose(0, out Args);
            throw (new DelayedResponseException());

            /*
            CP.Sync_Invoke(DeviceUDN,ServiceID,sender.Name,
                InArgs, out OutArgs);

            UPnPArgument[] Outputs = Gatekeeper.ParseArguments(OutArgs);
            ArrayList alist = new ArrayList();
            RetVal = null;


            foreach(UPnPArgument A in Outputs)
            {
                if(A.IsReturnValue)
                {
                    RetVal = A.DataValue;
                }
                else
                {
                    alist.Add(A);
                }
            }
			
            _OutArgs = (UPnPArgument[])alist.ToArray(typeof(UPnPArgument));
            */
        }
    }
}
