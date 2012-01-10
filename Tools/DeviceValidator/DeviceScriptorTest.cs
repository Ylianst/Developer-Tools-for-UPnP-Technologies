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
using System.Reflection;
using OpenSource.UPnP;
using OpenSource.DeviceScriptor;

namespace UPnPValidator.BasicTests
{
	/// <summary>
	/// Summary description for DeviceScriptorTest.
	/// </summary>
	[Serializable()] 
	public class DeviceScriptorTest : BasicTestGroup
	{
		[NonSerialized()]private DeviceScriptorInterface  ScriptorInterface = null;
		
		public DeviceScriptorTest(string ScriptProjectFileName)
		{
			ScriptorInterface = LoadDeviceScriptorPlugin();
			if (ScriptorInterface == null)
			{
				throw new Exception("Failed to load the Device Scriptor Plug-in. Please make sure the DeviceScriptor.exe file is in the same directory");
			}
			FileInfo Info = new FileInfo(ScriptProjectFileName);
			char [] Separator = {'.'};
			string [] NameExt = Info.Name.Split(Separator);
//			for(int i=0; i<NameExt.GetLength(0);i++)
//				if (i !=  (NameExt.GetLength(0) -1))	//ignore the last one since it is the extension
//					GroupName += NameExt[i];
			if ((NameExt != null)&&(NameExt.GetLength(0)>0))
				GroupName = NameExt[0];
			try 
			{
				ScriptorInterface.LoadScriptProject(ScriptProjectFileName);
			}
			catch (Exception ex)
			{
				throw new Exception("Failed to load the Script Project: " + ex.Message);
			}
			Category = "Script Projects";
			Description = ScriptorInterface.ScriptProjectDescription;
			ScriptorInterface.OnProgressChanged += new ProgressEventHandler(OnProgressChangedSink);
			ScriptorInterface.OnStateChanged += new StateEventHandler(OnStateChangedSink);
			ScriptorInterface.OnPacketTraceChanged += new PacketEventHandler(OnPacketTraceChangedSink);
			ScriptorInterface.OnEventLog += new ScriptLogEventHandler(OnEventLogSink);
			AddTestFromScriptProject();
			Reset();
		}

		private void AddTestFromScriptProject()
		{
			string [] TestName;
			string [] TestDescription;
			ScriptorInterface.GetStateNamesAndDescriptions(out TestName, out TestDescription);
			if (TestName != null)
			{
				for (int i=0; i<TestName.GetLength(0); i++)
					AddTest(TestName[i], TestDescription[i]);
			}
		}

		private void OnPacketTraceChangedSink(DeviceScriptorInterface sender, PacketLogStruct PacketLog)
		{
			AddPacket(PacketLog.packet);
		}

		private void OnEventLogSink(DeviceScriptorInterface sender, LogInfo log)
		{
			AddEvent(GetEquivalentLogImportance(log.importance), log.TestName, log.LogEntry);
		}

		private void OnProgressChangedSink(DeviceScriptorInterface sender, byte progess) 
		{
			SetProgress(progess);
		}
		
		private void OnStateChangedSink(DeviceScriptorInterface sender, StateEventStruct StateInfo)
		{
			UPnPTestStates ReportedState = GetEquivalentUPnPTestState(StateInfo.States);
			for (int i=0; i<TestList.Count; i++)
			{
				if (StateInfo.ScriptName == ((string [])TestList[i])[0])
				{
					if (ReportedState > states[i])
					{
						states[i] = ReportedState;
						SetState(StateInfo.ScriptName, ReportedState);
					}
				}
			}
		}

		private UPnPTestStates GetEquivalentUPnPTestState(ScriptStates state)
		{
			switch(state)
			{
				case ScriptStates.Normal:
					return UPnPTestStates.Ready;
				case ScriptStates.Skipped:
					return UPnPTestStates.Failed;
				case ScriptStates.Pass:
					return UPnPTestStates.Pass;
				case ScriptStates.Warning:
					return UPnPTestStates.Warn;
				case ScriptStates.Failure:
					return UPnPTestStates.Failed;
				default:
					return UPnPTestStates.Ready;
			}
		}

		private LogImportance GetEquivalentLogImportance(LogLevels level)
		{
			switch(level)
			{
				case LogLevels.Information:
					return LogImportance.Remark;
				case LogLevels.Success:
					return LogImportance.Remark;
				case LogLevels.Warning:
					return LogImportance.Medium;
				case LogLevels.Failure:
					return LogImportance.High;
			}
			return LogImportance.Remark;
		}

		public override void Start(UPnPDevice device)
		{
			if (ScriptorInterface != null)
			{
				Reset();
				ScriptorInterface.SetUPnPDevice(device);
				ScriptorInterface.Execute();
				for (int i=0; i<states.Length; i++)
				{
					if (state < states[i])
						state = states[i];
				}
			}
		}

		private DeviceScriptorInterface LoadDeviceScriptorPlugin()
		{
			// Find all files
					
			FileInfo[] files = new DirectoryInfo(System.Windows.Forms.Application.StartupPath).GetFiles();
			foreach (FileInfo file in files) 
			{
				// analyze only dll and exe files
				if (file.Extension.ToUpper() == ".DLL" || file.Extension.ToUpper() == ".EXE") 
				{
					// If a file points to an plugin that implement the interface
					Type ClassImplementedInterface = null;
					Assembly assembly = null;
					assembly = GetPluginInfo(file, typeof(DeviceScriptorInterface), ref ClassImplementedInterface);
					if ((assembly != null) && (ClassImplementedInterface != null))
					{
						DeviceScriptorInterface plugin=null;
						try
						{
							plugin = assembly.CreateInstance(ClassImplementedInterface.FullName) as DeviceScriptorInterface;
						}
						catch(Exception)
						{
							plugin = null;
						}
						if (plugin != null)
						return plugin;
					}
				}
			}
			return null;
		}

		private Assembly GetPluginInfo(FileInfo file, Type QueriedInterface, ref Type ClassImplementedThatInterface)
		{
			// Load the assembly,
			// scan the types,
			// scan the interfaces of each type,
			// and if we find ITestGroup, then load it.

			if (file.Exists == false) return null;
			Assembly assembly = Assembly.LoadFrom(file.FullName);
			ClassImplementedThatInterface = null;

			Type[] types = assembly.GetTypes();
			foreach (Type t in types)
			{
				Type[] interfaces = t.GetInterfaces();
				foreach (Type i in interfaces) 
				{
					if (i.FullName == QueriedInterface.FullName) 
					{
						ClassImplementedThatInterface = t;
						return assembly;
					}
				}
			}
			return null;
		}
	}
}
