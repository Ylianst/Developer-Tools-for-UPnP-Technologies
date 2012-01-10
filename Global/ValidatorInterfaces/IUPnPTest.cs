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

namespace UPnPValidator
{

	/// <summary>
	/// A utility class for converting back and forth between the 
	/// UPnPValidator enumerated values and strings.
	/// </summary>
	public class Util
	{
		public static string LogImportanceToString(LogImportance l)
		{
			switch (l) 
			{
				case LogImportance.Critical:
					return "Error";
				case LogImportance.High:
				case LogImportance.Low:
				case LogImportance.Medium:
					return "Warning";
				case LogImportance.Remark:
					return "Info";
			}
			return "";
		}

		public static LogImportance StringToLogImportance(string importanceString)
		{
			switch (importanceString)
			{
				case "Error":
					return LogImportance.Critical;
				case "Warning":
					return LogImportance.High;
				case "Info":
					return LogImportance.Remark;
			}
			throw new Exception("No such LogImportance: " + importanceString);
		}

		public static string UPnPTestStatesToString(UPnPTestStates s)
		{
			switch(s)
			{
				case UPnPTestStates.Failed:
					return "Failed";
				case UPnPTestStates.Pass:
					return "Pass";
				case UPnPTestStates.Ready:
					return "Ready";
				case UPnPTestStates.Running:
					return "Running";
				case UPnPTestStates.Warn:
					return "Warn";
			}
			return "";
		}

		public static UPnPTestStates StringToUPnPTestStates(string stateString)
		{
			switch (stateString)
			{
				case "Failed":
					return UPnPTestStates.Failed;
				case "Pass":
					return UPnPTestStates.Pass;
				case "Ready":
					return UPnPTestStates.Ready;
				case "Running":
					return UPnPTestStates.Running;
				case "Warn":
					return UPnPTestStates.Warn;
			}
			throw new Exception("No such UPnPTestState: " + stateString);
		}
	}


	public enum UPnPTestStates 
	{
		Ready = 0,
		Running,
		Pass,
		Warn,
		Failed,
	}

	public enum LogImportance
	{
		Low,
		Medium,
		High,
		Critical,
		Remark
	}

	public struct LogStruct
	{
		public LogImportance importance;
		public string TestName;
		public string LogEntry;
	}

	public delegate void LogEventHandler(IUPnPTestGroup sender, LogStruct log);
	public delegate void PacketTraceHandler(IUPnPTestGroup sender, HTTPMessage packet);

	public interface IUPnPTestGroup
	{
		event EventHandler			OnStateChanged;
		event PacketTraceHandler	OnPacketTraceChanged;
		event EventHandler			OnProgressChanged;
		event LogEventHandler		OnEventLog;
		
		UPnPTestStates	 GroupState { get; }
		string			 Category { get; }
		string			 GroupName { get; }
		string			 Description { get; }
		IList			 Log { get; }
		byte			 Progress { get; }
		IList			 PacketTrace { get; }
		object			 Tag { get; set; }
		bool			 Enabled { get; set; }

		UPnPTestStates[] TestStates { get; }
		string[]		 TestNames { get; }
		string[]		 TestDescription { get; }
		string[]		 Result { get; }

		void Reset();
		void Start(UPnPDevice device);
		void Cancel();
	}
}
