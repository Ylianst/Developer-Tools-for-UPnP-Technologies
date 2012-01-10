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
    /// Transparent ClientSide UPnP Service
    /// </summary>
    public class CpGateKeeper
    {
       private Hashtable UnspecifiedTable = Hashtable.Synchronized(new Hashtable());
       internal UPnPService _S;

       public UPnPService GetUPnPService()
       {
            return(_S);
       }
       public static string SERVICE_NAME = "urn:schemas-upnp-org:service:UPnPRelay:";
       public double VERSION
       {
           get
           {
               return(double.Parse(_S.Version));
           }
       }

       public delegate void SubscribeHandler(CpGateKeeper sender, bool Success);
       public event SubscribeHandler OnSubscribe;
       public delegate void Delegate_OnResult_Invoke(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Byte[] OutArgs, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_Invoke OnResult_Invoke;
       protected ArrayList WeakList_Invoke = new ArrayList();
       public void AddWeakEvent_Result_Invoke(Delegate_OnResult_Invoke d)
       {
           WeakList_Invoke.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_Invoke(Delegate_OnResult_Invoke d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_Invoke.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_Invoke)W.Target == d)
                  {
                           WeakList_Invoke.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_FireEvent(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.String StateVariable, System.String Value, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_FireEvent OnResult_FireEvent;
       protected ArrayList WeakList_FireEvent = new ArrayList();
       public void AddWeakEvent_Result_FireEvent(Delegate_OnResult_FireEvent d)
       {
           WeakList_FireEvent.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_FireEvent(Delegate_OnResult_FireEvent d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_FireEvent.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_FireEvent)W.Target == d)
                  {
                           WeakList_FireEvent.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_AddDevice(CpGateKeeper sender, System.String Sender, System.String DeviceUDN, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_AddDevice OnResult_AddDevice;
       protected ArrayList WeakList_AddDevice = new ArrayList();
       public void AddWeakEvent_Result_AddDevice(Delegate_OnResult_AddDevice d)
       {
           WeakList_AddDevice.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_AddDevice(Delegate_OnResult_AddDevice d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_AddDevice.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_AddDevice)W.Target == d)
                  {
                           WeakList_AddDevice.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_InvokeAsync(CpGateKeeper sender, System.String Caller, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Int32 Handle, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_InvokeAsync OnResult_InvokeAsync;
       protected ArrayList WeakList_InvokeAsync = new ArrayList();
       public void AddWeakEvent_Result_InvokeAsync(Delegate_OnResult_InvokeAsync d)
       {
           WeakList_InvokeAsync.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_InvokeAsync(Delegate_OnResult_InvokeAsync d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_InvokeAsync.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_InvokeAsync)W.Target == d)
                  {
                           WeakList_InvokeAsync.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_GetDocument(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.Byte[] Document, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_GetDocument OnResult_GetDocument;
       protected ArrayList WeakList_GetDocument = new ArrayList();
       public void AddWeakEvent_Result_GetDocument(Delegate_OnResult_GetDocument d)
       {
           WeakList_GetDocument.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_GetDocument(Delegate_OnResult_GetDocument d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_GetDocument.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_GetDocument)W.Target == d)
                  {
                           WeakList_GetDocument.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_InvokeAsyncResponse(CpGateKeeper sender, System.Int32 Handle, System.Byte[] OutArgs, System.Int32 ErrorCode, System.String ErrorString, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_InvokeAsyncResponse OnResult_InvokeAsyncResponse;
       protected ArrayList WeakList_InvokeAsyncResponse = new ArrayList();
       public void AddWeakEvent_Result_InvokeAsyncResponse(Delegate_OnResult_InvokeAsyncResponse d)
       {
           WeakList_InvokeAsyncResponse.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_InvokeAsyncResponse(Delegate_OnResult_InvokeAsyncResponse d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_InvokeAsyncResponse.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_InvokeAsyncResponse)W.Target == d)
                  {
                           WeakList_InvokeAsyncResponse.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_Register(CpGateKeeper sender, System.Uri Proxy, System.Boolean Reverse, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_Register OnResult_Register;
       protected ArrayList WeakList_Register = new ArrayList();
       public void AddWeakEvent_Result_Register(Delegate_OnResult_Register d)
       {
           WeakList_Register.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_Register(Delegate_OnResult_Register d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_Register.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_Register)W.Target == d)
                  {
                           WeakList_Register.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_GetStateTable(CpGateKeeper sender, System.String DeviceUDN, System.String ServiceID, System.Byte[] Variables, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_GetStateTable OnResult_GetStateTable;
       protected ArrayList WeakList_GetStateTable = new ArrayList();
       public void AddWeakEvent_Result_GetStateTable(Delegate_OnResult_GetStateTable d)
       {
           WeakList_GetStateTable.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_GetStateTable(Delegate_OnResult_GetStateTable d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_GetStateTable.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_GetStateTable)W.Target == d)
                  {
                           WeakList_GetStateTable.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_RemoveDevice(CpGateKeeper sender, System.String DeviceUDN, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_RemoveDevice OnResult_RemoveDevice;
       protected ArrayList WeakList_RemoveDevice = new ArrayList();
       public void AddWeakEvent_Result_RemoveDevice(Delegate_OnResult_RemoveDevice d)
       {
           WeakList_RemoveDevice.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_RemoveDevice(Delegate_OnResult_RemoveDevice d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_RemoveDevice.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_RemoveDevice)W.Target == d)
                  {
                           WeakList_RemoveDevice.Remove(W);
							  break;
                  }
               }
           }
       }
       public delegate void Delegate_OnResult_UnRegister(CpGateKeeper sender, System.Uri Proxy, UPnPInvokeException e, object _Tag);
       public event Delegate_OnResult_UnRegister OnResult_UnRegister;
       protected ArrayList WeakList_UnRegister = new ArrayList();
       public void AddWeakEvent_Result_UnRegister(Delegate_OnResult_UnRegister d)
       {
           WeakList_UnRegister.Add(new WeakReference(d));
       }
       public void RemoveWeakEvent_Result_UnRegister(Delegate_OnResult_UnRegister d)
       {
           WeakReference[] WR = (WeakReference[])WeakList_UnRegister.ToArray(typeof(WeakReference));
           foreach(WeakReference W in WR)
           {
               if(W.IsAlive)
               {
                  if((Delegate_OnResult_UnRegister)W.Target == d)
                  {
                           WeakList_UnRegister.Remove(W);
							  break;
                  }
               }
           }
       }

        public CpGateKeeper(UPnPService s)
        {
            _S = s;
            _S.OnSubscribe += new UPnPService.UPnPEventSubscribeHandler(_subscribe_sink);
        }
        public void Dispose()
        {
            _S.OnSubscribe -= new UPnPService.UPnPEventSubscribeHandler(_subscribe_sink);
            OnSubscribe = null;
            OnResult_Invoke = null;
            OnResult_FireEvent = null;
            OnResult_AddDevice = null;
            OnResult_InvokeAsync = null;
            OnResult_GetDocument = null;
            OnResult_InvokeAsyncResponse = null;
            OnResult_Register = null;
            OnResult_GetStateTable = null;
            OnResult_RemoveDevice = null;
            OnResult_UnRegister = null;
        }
        public void _subscribe(int Timeout)
        {
            _S.Subscribe(Timeout, null);
        }
        protected void _subscribe_sink(UPnPService sender, bool OK)
        {
            if(OnSubscribe!=null)
            {
                OnSubscribe(this, OK);
            }
        }
        public void SetUnspecifiedValue(string EnumType, string val)
        {
            string hash = Thread.CurrentThread.GetHashCode().ToString() + ":" + EnumType;
            UnspecifiedTable[hash] = val;
        }
        public string GetUnspecifiedValue(string EnumType)
        {
            string hash = Thread.CurrentThread.GetHashCode().ToString() + ":" + EnumType;
            if(UnspecifiedTable.ContainsKey(hash)==false)
            {
               return("");
            }
            string RetVal = (string)UnspecifiedTable[hash];
            return(RetVal);
        }
        public System.Boolean Reverse
        {
            get
            {
               return((System.Boolean)_S.GetStateVariable("Reverse"));
            }
        }
        public System.String ErrorString
        {
            get
            {
               return((System.String)_S.GetStateVariable("ErrorString"));
            }
        }
        public System.String StateVariableName
        {
            get
            {
               return((System.String)_S.GetStateVariable("StateVariableName"));
            }
        }
        public System.String ServiceID
        {
            get
            {
               return((System.String)_S.GetStateVariable("ServiceID"));
            }
        }
        public System.Byte[] Document
        {
            get
            {
               return((System.Byte[])_S.GetStateVariable("Document"));
            }
        }
        public System.Byte[] Args
        {
            get
            {
               return((System.Byte[])_S.GetStateVariable("Args"));
            }
        }
        public System.Int32 Handle
        {
            get
            {
               return((System.Int32)_S.GetStateVariable("Handle"));
            }
        }
        public System.Uri ProxyUri
        {
            get
            {
               return((System.Uri)_S.GetStateVariable("ProxyUri"));
            }
        }
        public System.String ActionName
        {
            get
            {
               return((System.String)_S.GetStateVariable("ActionName"));
            }
        }
        public System.String StateVariableValue
        {
            get
            {
               return((System.String)_S.GetStateVariable("StateVariableValue"));
            }
        }
        public System.String DeviceUDN
        {
            get
            {
               return((System.String)_S.GetStateVariable("DeviceUDN"));
            }
        }
        public System.Int32 ErrorCode
        {
            get
            {
               return((System.Int32)_S.GetStateVariable("ErrorCode"));
            }
        }
        public bool HasStateVariable_Reverse
        {
            get
            {
               if(_S.GetStateVariableObject("Reverse")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_ErrorString
        {
            get
            {
               if(_S.GetStateVariableObject("ErrorString")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_StateVariableName
        {
            get
            {
               if(_S.GetStateVariableObject("StateVariableName")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_ServiceID
        {
            get
            {
               if(_S.GetStateVariableObject("ServiceID")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_Document
        {
            get
            {
               if(_S.GetStateVariableObject("Document")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_Args
        {
            get
            {
               if(_S.GetStateVariableObject("Args")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_Handle
        {
            get
            {
               if(_S.GetStateVariableObject("Handle")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_ProxyUri
        {
            get
            {
               if(_S.GetStateVariableObject("ProxyUri")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_ActionName
        {
            get
            {
               if(_S.GetStateVariableObject("ActionName")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_StateVariableValue
        {
            get
            {
               if(_S.GetStateVariableObject("StateVariableValue")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_DeviceUDN
        {
            get
            {
               if(_S.GetStateVariableObject("DeviceUDN")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasStateVariable_ErrorCode
        {
            get
            {
               if(_S.GetStateVariableObject("ErrorCode")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_Invoke
        {
            get
            {
               if(_S.GetAction("Invoke")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_FireEvent
        {
            get
            {
               if(_S.GetAction("FireEvent")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_AddDevice
        {
            get
            {
               if(_S.GetAction("AddDevice")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_InvokeAsync
        {
            get
            {
               if(_S.GetAction("InvokeAsync")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_GetDocument
        {
            get
            {
               if(_S.GetAction("GetDocument")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_InvokeAsyncResponse
        {
            get
            {
               if(_S.GetAction("InvokeAsyncResponse")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_Register
        {
            get
            {
               if(_S.GetAction("Register")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_GetStateTable
        {
            get
            {
               if(_S.GetAction("GetStateTable")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_RemoveDevice
        {
            get
            {
               if(_S.GetAction("RemoveDevice")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public bool HasAction_UnRegister
        {
            get
            {
               if(_S.GetAction("UnRegister")==null)
               {
                   return(false);
               }
               else
               {
                   return(true);
               }
            }
        }
        public void Sync_Invoke(System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, out System.Byte[] OutArgs)
        {
           UPnPArgument[] args = new UPnPArgument[5];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Action", Action);
           args[3] = new UPnPArgument("InArgs", InArgs);
           args[4] = new UPnPArgument("OutArgs", "");
            _S.InvokeSync("Invoke", args);
            DeviceUDN = (System.String) args[0].DataValue;
            ServiceID = (System.String) args[1].DataValue;
            Action = (System.String) args[2].DataValue;
            InArgs = (System.Byte[]) args[3].DataValue;
            OutArgs = (System.Byte[]) args[4].DataValue;
            return;
        }
        public void Invoke(System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs)
        {
            Invoke(DeviceUDN, ServiceID, Action, InArgs, null, null);
        }
        public void Invoke(System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, object _Tag, Delegate_OnResult_Invoke _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[5];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Action", Action);
           args[3] = new UPnPArgument("InArgs", InArgs);
           args[4] = new UPnPArgument("OutArgs", "");
           _S.InvokeAsync("Invoke", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_Invoke), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_Invoke));
        }
        private void Sink_Invoke(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_Invoke)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_Invoke != null)
                {
                   OnResult_Invoke(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_Invoke.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_Invoke)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_Invoke.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_Invoke(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_Invoke)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
            }
            else
            {
                if(OnResult_Invoke != null)
                {
                     OnResult_Invoke(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_Invoke.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_Invoke)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.Byte[] )Args[3].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_Invoke.Remove(wr);
                    }
                }
            }
        }
        public void Sync_FireEvent(System.String DeviceUDN, System.String ServiceID, System.String StateVariable, System.String Value)
        {
           UPnPArgument[] args = new UPnPArgument[4];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("StateVariable", StateVariable);
           args[3] = new UPnPArgument("Value", Value);
            _S.InvokeSync("FireEvent", args);
            DeviceUDN = (System.String) args[0].DataValue;
            ServiceID = (System.String) args[1].DataValue;
            StateVariable = (System.String) args[2].DataValue;
            Value = (System.String) args[3].DataValue;
            return;
        }
        public void FireEvent(System.String DeviceUDN, System.String ServiceID, System.String StateVariable, System.String Value)
        {
            FireEvent(DeviceUDN, ServiceID, StateVariable, Value, null, null);
        }
        public void FireEvent(System.String DeviceUDN, System.String ServiceID, System.String StateVariable, System.String Value, object _Tag, Delegate_OnResult_FireEvent _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[4];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("StateVariable", StateVariable);
           args[3] = new UPnPArgument("Value", Value);
           _S.InvokeAsync("FireEvent", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_FireEvent), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_FireEvent));
        }
        private void Sink_FireEvent(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_FireEvent)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_FireEvent != null)
                {
                   OnResult_FireEvent(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_FireEvent.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_FireEvent)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_FireEvent.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_FireEvent(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_FireEvent)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_FireEvent != null)
                {
                     OnResult_FireEvent(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_FireEvent.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_FireEvent)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_FireEvent.Remove(wr);
                    }
                }
            }
        }
        public void Sync_AddDevice(System.String Sender, System.String DeviceUDN)
        {
           UPnPArgument[] args = new UPnPArgument[2];
           args[0] = new UPnPArgument("Sender", Sender);
           args[1] = new UPnPArgument("DeviceUDN", DeviceUDN);
            _S.InvokeSync("AddDevice", args);
            Sender = (System.String) args[0].DataValue;
            DeviceUDN = (System.String) args[1].DataValue;
            return;
        }
        public void AddDevice(System.String Sender, System.String DeviceUDN)
        {
            AddDevice(Sender, DeviceUDN, null, null);
        }
        public void AddDevice(System.String Sender, System.String DeviceUDN, object _Tag, Delegate_OnResult_AddDevice _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[2];
           args[0] = new UPnPArgument("Sender", Sender);
           args[1] = new UPnPArgument("DeviceUDN", DeviceUDN);
           _S.InvokeAsync("AddDevice", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_AddDevice), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_AddDevice));
        }
        private void Sink_AddDevice(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_AddDevice)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_AddDevice != null)
                {
                   OnResult_AddDevice(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_AddDevice.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_AddDevice)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_AddDevice.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_AddDevice(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_AddDevice)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_AddDevice != null)
                {
                     OnResult_AddDevice(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_AddDevice.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_AddDevice)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_AddDevice.Remove(wr);
                    }
                }
            }
        }
        public void Sync_InvokeAsync(System.String Caller, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Int32 Handle)
        {
           UPnPArgument[] args = new UPnPArgument[6];
           args[0] = new UPnPArgument("Caller", Caller);
           args[1] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[2] = new UPnPArgument("ServiceID", ServiceID);
           args[3] = new UPnPArgument("Action", Action);
           args[4] = new UPnPArgument("InArgs", InArgs);
           args[5] = new UPnPArgument("Handle", Handle);
            _S.InvokeSync("InvokeAsync", args);
            Caller = (System.String) args[0].DataValue;
            DeviceUDN = (System.String) args[1].DataValue;
            ServiceID = (System.String) args[2].DataValue;
            Action = (System.String) args[3].DataValue;
            InArgs = (System.Byte[]) args[4].DataValue;
            Handle = (System.Int32) args[5].DataValue;
            return;
        }
        public void InvokeAsync(System.String Caller, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Int32 Handle)
        {
            InvokeAsync(Caller, DeviceUDN, ServiceID, Action, InArgs, Handle, null, null);
        }
        public void InvokeAsync(System.String Caller, System.String DeviceUDN, System.String ServiceID, System.String Action, System.Byte[] InArgs, System.Int32 Handle, object _Tag, Delegate_OnResult_InvokeAsync _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[6];
           args[0] = new UPnPArgument("Caller", Caller);
           args[1] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[2] = new UPnPArgument("ServiceID", ServiceID);
           args[3] = new UPnPArgument("Action", Action);
           args[4] = new UPnPArgument("InArgs", InArgs);
           args[5] = new UPnPArgument("Handle", Handle);
           _S.InvokeAsync("InvokeAsync", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_InvokeAsync), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_InvokeAsync));
        }
        private void Sink_InvokeAsync(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_InvokeAsync)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_InvokeAsync != null)
                {
                   OnResult_InvokeAsync(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_InvokeAsync.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_InvokeAsync)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_InvokeAsync.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_InvokeAsync(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_InvokeAsync)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_InvokeAsync != null)
                {
                     OnResult_InvokeAsync(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_InvokeAsync.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_InvokeAsync)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.String )Args[2].DataValue, (System.String )Args[3].DataValue, (System.Byte[] )Args[4].DataValue, (System.Int32 )Args[5].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_InvokeAsync.Remove(wr);
                    }
                }
            }
        }
        public void Sync_GetDocument(System.String DeviceUDN, System.String ServiceID, out System.Byte[] Document)
        {
           UPnPArgument[] args = new UPnPArgument[3];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Document", "");
            _S.InvokeSync("GetDocument", args);
            DeviceUDN = (System.String) args[0].DataValue;
            ServiceID = (System.String) args[1].DataValue;
            Document = (System.Byte[]) args[2].DataValue;
            return;
        }
        public void GetDocument(System.String DeviceUDN, System.String ServiceID)
        {
            GetDocument(DeviceUDN, ServiceID, null, null);
        }
        public void GetDocument(System.String DeviceUDN, System.String ServiceID, object _Tag, Delegate_OnResult_GetDocument _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[3];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Document", "");
           _S.InvokeAsync("GetDocument", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_GetDocument), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_GetDocument));
        }
        private void Sink_GetDocument(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_GetDocument)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_GetDocument != null)
                {
                   OnResult_GetDocument(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_GetDocument.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_GetDocument)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_GetDocument.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_GetDocument(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_GetDocument)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
            }
            else
            {
                if(OnResult_GetDocument != null)
                {
                     OnResult_GetDocument(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_GetDocument.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_GetDocument)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_GetDocument.Remove(wr);
                    }
                }
            }
        }
        public void Sync_InvokeAsyncResponse(System.Int32 Handle, System.Byte[] OutArgs, System.Int32 ErrorCode, System.String ErrorString)
        {
           UPnPArgument[] args = new UPnPArgument[4];
           args[0] = new UPnPArgument("Handle", Handle);
           args[1] = new UPnPArgument("OutArgs", OutArgs);
           args[2] = new UPnPArgument("ErrorCode", ErrorCode);
           args[3] = new UPnPArgument("ErrorString", ErrorString);
            _S.InvokeSync("InvokeAsyncResponse", args);
            Handle = (System.Int32) args[0].DataValue;
            OutArgs = (System.Byte[]) args[1].DataValue;
            ErrorCode = (System.Int32) args[2].DataValue;
            ErrorString = (System.String) args[3].DataValue;
            return;
        }
        public void InvokeAsyncResponse(System.Int32 Handle, System.Byte[] OutArgs, System.Int32 ErrorCode, System.String ErrorString)
        {
            InvokeAsyncResponse(Handle, OutArgs, ErrorCode, ErrorString, null, null);
        }
        public void InvokeAsyncResponse(System.Int32 Handle, System.Byte[] OutArgs, System.Int32 ErrorCode, System.String ErrorString, object _Tag, Delegate_OnResult_InvokeAsyncResponse _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[4];
           args[0] = new UPnPArgument("Handle", Handle);
           args[1] = new UPnPArgument("OutArgs", OutArgs);
           args[2] = new UPnPArgument("ErrorCode", ErrorCode);
           args[3] = new UPnPArgument("ErrorString", ErrorString);
           _S.InvokeAsync("InvokeAsyncResponse", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_InvokeAsyncResponse), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_InvokeAsyncResponse));
        }
        private void Sink_InvokeAsyncResponse(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_InvokeAsyncResponse)StateInfo[1])(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_InvokeAsyncResponse != null)
                {
                   OnResult_InvokeAsyncResponse(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_InvokeAsyncResponse.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_InvokeAsyncResponse)wr.Target)(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_InvokeAsyncResponse.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_InvokeAsyncResponse(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_InvokeAsyncResponse)StateInfo[1])(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_InvokeAsyncResponse != null)
                {
                     OnResult_InvokeAsyncResponse(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_InvokeAsyncResponse.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_InvokeAsyncResponse)wr.Target)(this, (System.Int32 )Args[0].DataValue, (System.Byte[] )Args[1].DataValue, (System.Int32 )Args[2].DataValue, (System.String )Args[3].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_InvokeAsyncResponse.Remove(wr);
                    }
                }
            }
        }
        public void Sync_Register(System.Uri Proxy, System.Boolean Reverse)
        {
           UPnPArgument[] args = new UPnPArgument[2];
           args[0] = new UPnPArgument("Proxy", Proxy);
           args[1] = new UPnPArgument("Reverse", Reverse);
            _S.InvokeSync("Register", args);
            Proxy = (System.Uri) args[0].DataValue;
            Reverse = (System.Boolean) args[1].DataValue;
            return;
        }
        public void Register(System.Uri Proxy, System.Boolean Reverse)
        {
            Register(Proxy, Reverse, null, null);
        }
        public void Register(System.Uri Proxy, System.Boolean Reverse, object _Tag, Delegate_OnResult_Register _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[2];
           args[0] = new UPnPArgument("Proxy", Proxy);
           args[1] = new UPnPArgument("Reverse", Reverse);
           _S.InvokeAsync("Register", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_Register), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_Register));
        }
        private void Sink_Register(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_Register)StateInfo[1])(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_Register != null)
                {
                   OnResult_Register(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_Register.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_Register)wr.Target)(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_Register.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_Register(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_Register)StateInfo[1])(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_Register != null)
                {
                     OnResult_Register(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_Register.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_Register)wr.Target)(this, (System.Uri )Args[0].DataValue, (System.Boolean )Args[1].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_Register.Remove(wr);
                    }
                }
            }
        }
        public void Sync_GetStateTable(System.String DeviceUDN, System.String ServiceID, out System.Byte[] Variables)
        {
           UPnPArgument[] args = new UPnPArgument[3];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Variables", "");
            _S.InvokeSync("GetStateTable", args);
            DeviceUDN = (System.String) args[0].DataValue;
            ServiceID = (System.String) args[1].DataValue;
            Variables = (System.Byte[]) args[2].DataValue;
            return;
        }
        public void GetStateTable(System.String DeviceUDN, System.String ServiceID)
        {
            GetStateTable(DeviceUDN, ServiceID, null, null);
        }
        public void GetStateTable(System.String DeviceUDN, System.String ServiceID, object _Tag, Delegate_OnResult_GetStateTable _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[3];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           args[1] = new UPnPArgument("ServiceID", ServiceID);
           args[2] = new UPnPArgument("Variables", "");
           _S.InvokeAsync("GetStateTable", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_GetStateTable), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_GetStateTable));
        }
        private void Sink_GetStateTable(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_GetStateTable)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_GetStateTable != null)
                {
                   OnResult_GetStateTable(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_GetStateTable.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_GetStateTable)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[] )Args[2].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_GetStateTable.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_GetStateTable(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_GetStateTable)StateInfo[1])(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
            }
            else
            {
                if(OnResult_GetStateTable != null)
                {
                     OnResult_GetStateTable(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_GetStateTable.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_GetStateTable)wr.Target)(this, (System.String )Args[0].DataValue, (System.String )Args[1].DataValue, (System.Byte[])UPnPService.CreateObjectInstance(typeof(System.Byte[]),null), e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_GetStateTable.Remove(wr);
                    }
                }
            }
        }
        public void Sync_RemoveDevice(System.String DeviceUDN)
        {
           UPnPArgument[] args = new UPnPArgument[1];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
            _S.InvokeSync("RemoveDevice", args);
            DeviceUDN = (System.String) args[0].DataValue;
            return;
        }
        public void RemoveDevice(System.String DeviceUDN)
        {
            RemoveDevice(DeviceUDN, null, null);
        }
        public void RemoveDevice(System.String DeviceUDN, object _Tag, Delegate_OnResult_RemoveDevice _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[1];
           args[0] = new UPnPArgument("DeviceUDN", DeviceUDN);
           _S.InvokeAsync("RemoveDevice", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_RemoveDevice), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_RemoveDevice));
        }
        private void Sink_RemoveDevice(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_RemoveDevice)StateInfo[1])(this, (System.String )Args[0].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_RemoveDevice != null)
                {
                   OnResult_RemoveDevice(this, (System.String )Args[0].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_RemoveDevice.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_RemoveDevice)wr.Target)(this, (System.String )Args[0].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_RemoveDevice.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_RemoveDevice(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_RemoveDevice)StateInfo[1])(this, (System.String )Args[0].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_RemoveDevice != null)
                {
                     OnResult_RemoveDevice(this, (System.String )Args[0].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_RemoveDevice.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_RemoveDevice)wr.Target)(this, (System.String )Args[0].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_RemoveDevice.Remove(wr);
                    }
                }
            }
        }
        public void Sync_UnRegister(System.Uri Proxy)
        {
           UPnPArgument[] args = new UPnPArgument[1];
           args[0] = new UPnPArgument("Proxy", Proxy);
            _S.InvokeSync("UnRegister", args);
            Proxy = (System.Uri) args[0].DataValue;
            return;
        }
        public void UnRegister(System.Uri Proxy)
        {
            UnRegister(Proxy, null, null);
        }
        public void UnRegister(System.Uri Proxy, object _Tag, Delegate_OnResult_UnRegister _Callback)
        {
           UPnPArgument[] args = new UPnPArgument[1];
           args[0] = new UPnPArgument("Proxy", Proxy);
           _S.InvokeAsync("UnRegister", args, new object[2]{_Tag,_Callback},new UPnPService.UPnPServiceInvokeHandler(Sink_UnRegister), new UPnPService.UPnPServiceInvokeErrorHandler(Error_Sink_UnRegister));
        }
        private void Sink_UnRegister(UPnPService sender, string MethodName, UPnPArgument[] Args, object RetVal, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_UnRegister)StateInfo[1])(this, (System.Uri )Args[0].DataValue, null, StateInfo[0]);
            }
            else
            {
                if(OnResult_UnRegister != null)
                {
                   OnResult_UnRegister(this, (System.Uri )Args[0].DataValue, null, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_UnRegister.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_UnRegister)wr.Target)(this, (System.Uri )Args[0].DataValue, null, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_UnRegister.Remove(wr);
                    }
                }
            }
        }
        private void Error_Sink_UnRegister(UPnPService sender, string MethodName, UPnPArgument[] Args, UPnPInvokeException e, object _Tag)
        {
            object[] StateInfo = (object[])_Tag;
            if(StateInfo[1]!=null)
            {
                ((Delegate_OnResult_UnRegister)StateInfo[1])(this, (System.Uri )Args[0].DataValue, e, StateInfo[0]);
            }
            else
            {
                if(OnResult_UnRegister != null)
                {
                     OnResult_UnRegister(this, (System.Uri )Args[0].DataValue, e, StateInfo[0]);
                }
                WeakReference[] w = (WeakReference[])WeakList_UnRegister.ToArray(typeof(WeakReference));
                foreach(WeakReference wr in w)
                {
                    if(wr.IsAlive==true)
                    {
                       ((Delegate_OnResult_UnRegister)wr.Target)(this, (System.Uri )Args[0].DataValue, e, StateInfo[0]);
                    }
                    else
                    {
                        WeakList_UnRegister.Remove(wr);
                    }
                }
            }
        }
    }
}