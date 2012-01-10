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
using System.Reflection;
using System.Collections;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnPValidator
{
	public class CdsTestResult
	{
		/// <summary>
		/// Device's response time in milliseconds for a subtest.
		/// </summary>
		public long ResponseTime;

		/// <summary>
		/// Average response time in milliseconds, where the subtest
		/// defines what a unit is.
		/// </summary>
		public long AverageUnitResponseTime;

		/// <summary>
		/// A string, such as "media object" or "invocation",
		/// to be used with <see cref="CdsTestResult.AverageUnitResponseTime"/>.
		/// </summary>
		public string UnitDescription;
	}


	/// <summary>
	/// Class is used to pass state information between
	/// CDS related subtests.
	/// </summary>
	public class CdsTestGroupState
	{
		public string SearchCapabilities;
		public string SortCapabilities;
		public uint SystemUpdateID;
	}

	public class CdsSubTestArgument : ISubTestArgument
	{
		public UPnPDevice _Device;
		public UPnPDevice Device { get { return _Device; } }
		
		public CdsTestGroup _TestGroup;
		public BasicTests.BasicTestGroup TestGroup { get { return _TestGroup; } }

		public CdsTestGroupState _TestGroupState;
		public object TestGroupState { get { return _TestGroup; } }

		private TestQueue _TestQueue;
		public TestQueue ActiveTests { get { return this._TestQueue; } set { _TestQueue = value; } }
	}

	public abstract class CdsSubTest : SubTest
	{
		public CdsSubTest()
		{
			this.SetTestInfo();
		}

		public override UPnPTestStates Run (ICollection otherSubTests, ISubTestArgument arg)
		{
			return this.Run(otherSubTests, (CdsSubTestArgument) arg);
		}

		protected abstract void SetTestInfo();
		public abstract UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg);

		public CpContentDirectory GetCDS(UPnPDevice device)
		{
			UPnPDevice d = device;

			UPnPService[] services = d.GetServices(CpContentDirectory.SERVICE_NAME);

			if (services == null || services.Length == 0) 
			{
				return null;
			}

			return new CpContentDirectory(services[0]);
		}

		public override abstract void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg);

		protected void LogResponseTime (DateTime started, CdsTestResult result, CdsSubTestArgument arg)
		{
			DateTime end = DateTime.Now;

			long ticks = end.Ticks - started.Ticks;
			long millisec = ticks / 10000;

			result.ResponseTime = millisec;
			arg._TestGroup.AddEvent(LogImportance.Remark, this._Name, this._Name + " subtest completion time: " + millisec.ToString() + "ms.");
		}
		protected void LogAverageUnitResponseTime (long averageTime, CdsTestResult result, CdsSubTestArgument arg)
		{
			result.AverageUnitResponseTime = averageTime;
			arg._TestGroup.AddEvent(LogImportance.Remark, this._Name, this._Name + " average unit response time: " + averageTime.ToString() + "ms" + result.UnitDescription);
		}


		/// <summary>
		/// Used to log errors when doing Cds related tests.
		/// </summary>
		public class CdsException : Exception
		{
			public CdsException(string message) : base(message){}
			public CdsException(string message, Exception innerException) : base (message, innerException) {}
		}

		/// <summary>
		/// Used to log errors in the test framework for CDS related tests.
		/// </summary>
		public class TestException : Exception
		{
			public TestException(string message, object obj) : base("TEST ERROR: " + message){ Obj = obj; }
			public TestException(string message, object obj, Exception innerException) : base ("TEST ERROR: " + message, innerException) { Obj = obj;}
			public readonly object Obj;
		}




		/// <summary>
		/// Stores the results for a Browse or Search invoke.
		/// </summary>
		public struct CdsBrowseSearchResults
		{
			/// <summary>
			/// The XML/Didl-Lite result.
			/// </summary>
			public string Result;
			/// <summary>
			/// Number of media objects in the result.
			/// </summary>
			public uint NumberReturned;
			/// <summary>
			/// Number of media objects that matched the query.
			/// </summary>
			public uint TotalMatches;
			/// <summary>
			/// The object ID of the browsed container, or 
			/// ignored if browsing content metadata.
			/// </summary>
			public uint UpdateID;
			/// <summary>
			/// A list of media objects instantiated from the Result XML.
			/// </summary>
			public IList MediaObjects;
			/// <summary>
			/// A reported error from the media server. Null indicates no error.
			/// </summary>
			public UPnPInvokeException InvokeError;
			/// <summary>
			/// Any additional errors encountered when casting the result XML
			/// into IUPnPMedia objects.
			/// </summary>
			public ArrayList ResultErrors;

			/// <summary>
			/// Indicates the worst severity level encountered.
			/// </summary>
			public UPnPTestStates WorstError;

			public void SetError(UPnPTestStates severity)
			{
				if ((int)severity > (int)this.WorstError)
				{
					this.WorstError = severity;
				}
			}
		}

		public struct UpdateObjectError
		{
			public IUPnPMedia UpdateThis;
			public IUPnPMedia Metadata;
		}
		public struct AddObjectError
		{
			public IMediaContainer Parent;
			public IUPnPMedia Child;
		}

		public class TerminateEarly : Exception
		{
			public TerminateEarly(string msg) : base(msg) {}
			public TerminateEarly(string msg, Exception innerException) : base (msg, innerException) {}
		}



	}
}
