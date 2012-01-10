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
using System.Xml;
using System.Collections;
using OpenSource.UPnP;

namespace UPnPValidator.BasicTests
{
	/// <summary>
	/// Implements a view only test group.  This is used in the Validator
	/// when looking at results.  This class should be thought of as a dummy
	/// class so that the existing validator framework can be used to view 
	/// the previous results, however no testing should be possible with 
	/// this class.
	/// </summary>
	public class UPnPTestViewOnly : BasicTestGroup
	{
		public UPnPTestViewOnly(string category, XmlElement description)
		{
			_Category = category;

			grabTopLevelInfo(description);
			foreach (XmlElement t in description.ChildNodes)
			{
				if(t.Name.Equals("Test"))
				{
					grabTestInfo(t);
					foreach (XmlElement l in t.ChildNodes)
					{
						this.grabLogEntry(t.GetAttribute("name"), l);
					}
				}
				else
				{
					this.grabHttpPacket(t);
				}
			}
		}

		private void grabTopLevelInfo(XmlElement elt)
		{
			string name = elt.GetAttribute("name");
			string state = elt.GetAttribute("state");
			string description = elt.GetAttribute("description");

			_GroupName = name;
			_state = Util.StringToUPnPTestStates(state);
			_Description = description;
		}

		private void grabTestInfo(XmlElement testElement)
		{
			string name = testElement.GetAttribute("name");
			string state = testElement.GetAttribute("state");
			string result = testElement.GetAttribute("result");
			string description = testElement.GetAttribute("description");

			Results.Add(result);
			TestList.Add(new string[] {name, description});
			
			UPnPTestStates[] oldStates = states;
			states = new UPnPTestStates[TestList.Count];

			states[oldStates.Length] = Util.StringToUPnPTestStates(state);

			for(int i=0;i<oldStates.Length;++i)
			{
				states[i] = oldStates[i];
			}
		}

		private void grabHttpPacket(XmlElement packetElement)
		{
			string packetString = packetElement.InnerText;
			System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
			System.Text.Encoder utf8Enc = utf8.GetEncoder();
			char [] packetChars = packetString.ToCharArray();
			int len = packetChars.Length;
			byte [] packetBytes = new byte[len];
			utf8Enc.GetBytes(packetChars, 0, len, packetBytes, 0, true);

			HTTPMessage m = HTTPMessage.ParseByteArray(packetBytes);

			packets.Add(m);
		}

		private void grabLogEntry(string testname, XmlElement logElement)
		{
			string importance = logElement.GetAttribute("importance");
			string message = logElement.InnerText;

			LogStruct log = new LogStruct();

			log.TestName = testname;
			log.importance = Util.StringToLogImportance(importance);
			log.LogEntry = message;

			LogList.Add(log);			
		}


		public override void Start(UPnPDevice device)
		{
			throw new Exception("This method should never be called");
		}
	}
}
