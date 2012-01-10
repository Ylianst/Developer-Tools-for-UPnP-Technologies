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
using OpenSource.UPnP;

namespace OpenSource.DeviceScriptor
{
	public enum LogLevels 
	{
		Information = 0,
		Success = 1,
		Warning = 2,
		Failure = 3,
		Skipped = 4
	}

	public enum ScriptStates
	{
		Normal = 0,
		Skipped = 1,
		Pass = 2,
		Warning = 3,
		Failure = 4
	}

	public struct LogInfo
	{
		public LogLevels	importance;
		public string		TestName;
		public string		LogEntry;
	}

	public struct PacketLogStruct
	{
		public HTTPMessage packet;
	}

	public struct StateEventStruct
	{
		public ScriptStates States;
		public string		ScriptName;
	}

	public delegate void ScriptLogEventHandler(DeviceScriptorInterface sender, LogInfo log);
	public delegate void PacketEventHandler(DeviceScriptorInterface sender, PacketLogStruct log);
	public delegate void StateEventHandler(DeviceScriptorInterface sender, StateEventStruct StateInfo);
	public delegate void ProgressEventHandler(DeviceScriptorInterface sender, byte progess);

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	public interface DeviceScriptorInterface
	{
		event StateEventHandler			OnStateChanged;
		event ProgressEventHandler		OnProgressChanged;
		event PacketEventHandler		OnPacketTraceChanged;
		event ScriptLogEventHandler		OnEventLog;

		void LoadScriptProject(string FileName);
		void Execute();
		void SetUPnPDevice(UPnPDevice SelectedDevice);
		void GetStateNamesAndDescriptions(out string [] TestNames, 
											out string [] TestDescriptions);
		string ScriptProjectDescription{get;}
	}
}
