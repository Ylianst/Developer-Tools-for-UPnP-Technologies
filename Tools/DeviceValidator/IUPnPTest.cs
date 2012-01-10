using System;
using System.Collections;
using Intel.UPNP;

namespace UPnPValidator
{
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

	public interface IUPnPTestGroup
	{
		event EventHandler		OnStateChanged;
		event EventHandler		OnPacketTraceChanged;
		event EventHandler		OnProgressChanged;
		event LogEventHandler	OnEventLog;

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
