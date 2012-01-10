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
using System.Collections;
using OpenSource.UPnP;

namespace UPnPValidator.BasicTests
{
    /// <summary>
    /// Summary description for BasicTestGroup.
    /// </summary>
    [Serializable()]
    public abstract class BasicTestGroup : IUPnPTestGroup
    {
        // Interface implementation
        public event EventHandler OnStateChanged;
        public event PacketTraceHandler OnPacketTraceChanged;
        public event EventHandler OnProgressChanged;
        public event LogEventHandler OnEventLog;

        public UPnPTestStates GroupState { get { return state; } }
        public UPnPTestStates[] TestStates { get { return states; } }
        protected string _Category = "NOT_SET";
        public string Category
        {
            get
            {
                return (_Category);
            }
            set
            {
                _Category = value;
            }
        }
        protected string _GroupName = "NOT_SET";
        public string GroupName
        {
            get
            {
                return (_GroupName);
            }
            set
            {
                _GroupName = value;
            }
        }

        public string[] TestNames
        {
            get
            {
                ArrayList temp = new ArrayList();
                foreach (object[] obj in TestList)
                {
                    temp.Add(obj[0]);
                }
                return ((string[])temp.ToArray(typeof(string)));
            }
        }

        public string[] TestDescription
        {
            get
            {
                ArrayList temp = new ArrayList();
                foreach (object[] obj in TestList)
                {
                    temp.Add(obj[1]);
                }
                return ((string[])temp.ToArray(typeof(string)));
            }
        }

        public string[] Result
        {
            get
            {
                return ((string[])Results.ToArray(typeof(string)));
            }
        }
        protected string _Description = "NOT_SET";
        public string Description
        {
            get
            {
                return (_Description);
            }
            set
            {
                _Description = value;
            }
        }
        public IList Log { get { return (IList)LogList.Clone(); } }
        public byte Progress { get { return progress; } }
        public IList PacketTrace { get { return packets; } }
        public object Tag { get { return tag; } set { tag = value; } }
        public bool Enabled { get { return enabled; } set { enabled = value; } }

        // Generic Test Variables
        protected ArrayList TestList = new ArrayList();
        protected ArrayList LogList = new ArrayList();
        protected ArrayList Results = new ArrayList();
        protected UPnPTestStates _state = UPnPTestStates.Ready;
        public UPnPTestStates state
        {
            get
            {
                return (_state);
            }
            set
            {
                _state = value;
                if (this.OnStateChanged != null) OnStateChanged(this, null);
            }
        }
        protected UPnPTestStates[] states = new UPnPTestStates[0];
        protected ArrayList packets = new ArrayList();
        protected string log = "";
        protected string result = "";
        protected bool enabled = true;
        [NonSerialized()]
        protected byte progress = 0;
        [NonSerialized()]
        protected object tag;
        [NonSerialized()]
        protected System.Timers.Timer Countdown;
        protected DateTime StartTime;
        protected int TimeLeft = 0;
        protected int TotalTime = 0;

        public void AddResult(string result)
        {
            Results.Add(result);
        }

        public void AddPacket(HTTPMessage packet)
        {
            packets.Add(packet);
            if (this.OnPacketTraceChanged != null) OnPacketTraceChanged(this, packet);
        }

        public void AddEvent(LogImportance importance, string TestName, string entry)
        {
            LogStruct ls = new LogStruct();
            ls.importance = importance;
            ls.LogEntry = entry;
            ls.TestName = TestName;
            LogList.Add(ls);
            if (this.OnEventLog != null) OnEventLog((IUPnPTestGroup)this, ls);
        }

        public void SetState(string TestName, UPnPTestStates NewState)
        {
            if (TestName == GroupName)
            {
                state = NewState;
                return;
            }
            int i = 0;
            foreach (string s in this.TestNames)
            {
                if (s == TestName)
                {
                    states[i] = NewState;
                    if (this.OnStateChanged != null) OnStateChanged(this, null);
                    break;
                }
                ++i;
            }
        }

        protected void AddTest(string TestName, string TestDescription)
        {
            TestList.Add(new string[2] { TestName, TestDescription });
            states = new UPnPTestStates[TestList.Count];
            for (int i = 0; i < states.Length; ++i)
            {
                states[i] = UPnPTestStates.Ready;
            }
        }

        protected void AddMessage(int Indents, string msg)
        {
            string x = "";
            for (int i = 0; i < Indents; ++i) x += "     ";
            log += x + msg + "\r\n";
            //if (OnLogChanged != null) OnLogChanged(this,null);
        }

        public virtual void Reset()
        {
            if (Countdown == null)
            {
                Countdown = new System.Timers.Timer(1000);
                Countdown.Elapsed += new System.Timers.ElapsedEventHandler(this.CountdownSink);
            }
            state = UPnPTestStates.Ready;
            for (int i = 0; i < states.Length; ++i)
            {
                states[i] = UPnPTestStates.Ready;
            }
            progress = 0;
            packets.Clear();
            enabled = true;
            log = "";
            result = "";
        }

        public void SetProgress(byte p)
        {
            progress = p;
            if (this.OnProgressChanged != null) OnProgressChanged(this, null);
        }

        public void StartCountDown(int seconds, int total)
        {
            TotalTime = total;
            Countdown.AutoReset = true;
            this.TimeLeft = total - seconds;

            double p = (double)TimeLeft / (double)TotalTime;
            p = (double)1 - p;
            p = p * (double)100;
            progress = (byte)p;

            if (OnProgressChanged != null) OnProgressChanged(this, null);
            Countdown.Start();
        }

        protected void CountdownSink(object sender, System.Timers.ElapsedEventArgs e)
        {
            --TimeLeft;

            double p = (double)TimeLeft / (double)TotalTime;
            p = (double)1 - p;
            p = p * (double)100;
            progress = (byte)p;
            if (OnProgressChanged != null) OnProgressChanged(this, null);

            if (TimeLeft == 0) Countdown.Stop();
        }

        public void AbortCountDown()
        {
            Countdown.Stop();
        }

        public virtual void Cancel()
        {
        }
        public abstract void Start(UPnPDevice device);
        public BasicTestGroup()
        {
            Reset();
        }
    }
}
