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

namespace UPnPValidator
{
	/// <summary>
	/// Provides state object and other information specific to a subtest when
	/// calling the <see cref="ISubTest.Run"/> method.
	/// </summary>
	public interface ISubTestArgument
	{

		/// <summary>
		/// Device under test.
		/// </summary>
		UPnPDevice Device { get; }

		/// <summary>
		/// Implementation-specific information about the state
		/// of a current test group. Often used a means to
		/// provide references to detailed results of
		/// subtests, so that subtests can obtain the
		/// results 
		/// </summary>
		object TestGroupState { get; }

		/// <summary>
		/// Provides information to the test group, which gives access to 
		/// CountDown methods.
		/// </summary>
		BasicTests.BasicTestGroup TestGroup { get; }

		/// <summary>
		/// Listing of tests that the argument will be used with.
		/// </summary>
		TestQueue ActiveTests { get; set; }
	}

	public interface ISubTest
	{
		/// <summary>
		/// If true, then the subtest is enabled.
		/// </summary>
		bool Enabled { get; set; }

		/// <summary>
		/// Unique name for the test within a test group.
		/// </summary>
		string Name { get; }
		
		/// <summary>
		/// A description of the subtest.
		/// </summary>
		string Description { get; }

		/// <summary>
		/// Tells the test to recalculate its expected run-time,
		/// useful for calculating expected time when the results
		/// of a prerequisite affect the running time of a test.
		/// </summary>
		/// <param name="otherSubTests"></param>
		/// <param name="arg"></param>
		void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg);

		/// <summary>
		/// The expected time in seconds that the subtest should take.
		/// This value may change.
		/// </summary>
		int ExpectedTestingTime { get; }

		/// <summary>
		/// Test-defined object for describing the results of the tests 
		/// in terms of object variables.
		/// </summary>
		object Details { get; }
		
		/// <summary>
		/// A list of prerequisite subtests.
		/// </summary>
		SubTest[] Prerequisites { get; }

		/// <summary>
		/// Current test state of the subtest.
		/// </summary>
		UPnPTestStates TestState { get; }

		/// <summary>
		/// Run the subtest.
		/// </summary>
		/// <param name="otherSubTests">
		/// Collection of <see cref="ISubTest"/> objects that have run or will run
		/// as part of a sequence of subtests along with this test. Provided so that
		/// a subtest can have information about what has run before and after it.
		/// </param>
		/// <param name="arg">
		/// This <see cref="ISubTestArgument"/> object can be used to pass state information
		/// from one subtest to another.
		/// </param>
		/// <returns>indicates the result of the test</returns>
		UPnPTestStates Run(ICollection otherSubTests, ISubTestArgument arg);
	}

	public abstract class SubTest : ISubTest, IComparable
	{
		protected string _Name;
		public string Name { get { return _Name; } }
		
		protected string _Description;
		public string Description { get { return _Description; } }

		protected int _ExpectedTestingTime = 0;
		public int ExpectedTestingTime { get { return _ExpectedTestingTime; } }
		internal void SetExpectedTestingTime (int expectedTime) { this._ExpectedTestingTime = expectedTime; }

		public abstract object Details { get; }
		public abstract void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg);

		protected ArrayList _Prerequisites = new ArrayList();
		public SubTest[] Prerequisites { get { return (SubTest[]) _Prerequisites.ToArray(typeof(SubTest)); } }

		protected UPnPTestStates _TestState = UPnPTestStates.Ready;
		public UPnPTestStates TestState { get { return _TestState; } }

		public bool _Enabled = true;
		public bool Enabled { get { return _Enabled; } set { _Enabled = value; } }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="otherSubTests">
		/// Collection of <see cref="ISubTest"/> objects that have run or will run
		/// as part of a sequence of subtests along with this test. Provided so that
		/// a subtest can have information about what has run before and after it.
		/// </param>
		/// <param name="arg">
		/// This <see cref="ISubTestArgument"/> object can be used to pass state information
		/// from one subtest to another.
		/// </param>
		/// <returns>indicates the result of the test</returns>
		public abstract UPnPTestStates Run (ICollection otherSubTests, ISubTestArgument arg);

		/// <summary>
		/// Allows SubTest objects to be sorted according to their prerequisites.
		/// </summary>
		/// <param name="subTest">other <see cref="SubTest"/> object to compare against</param>
		/// <returns>0=if no dependencies; 1=if this test has dependency on other test; -1=if this test is dependency of other test</returns>
		public int CompareTo(object subTest)
		{
			SubTest other = (SubTest) subTest;

			int result = 0;
			foreach (SubTest prereq in this._Prerequisites)
			{
				if (other.Name == prereq.Name)
				{
					// other subtest is a prerequisite of this test,
					// so this test has a greater value
					result = 1;
				}
			}

			foreach (SubTest otherPrereq in other._Prerequisites)
			{
				if (this.Name == otherPrereq.Name)
				{
					if (result == 0)
					{
						// this test is a prereq of the other test
						// so this test has a lower value
						result = -1;
					}
					else
					{
						throw new ApplicationException(this.Name + " and " + other.Name + " are prerequisites of each other.");
					}
				}
			}

			return result;
		}

		/// <summary>
		/// Base class for storing input parameters to a upnp action.
		/// Derived classes should declare public fields if they 
		/// desire those fields to get printed in the LogInvokeError method.
		/// The order of the fields should match the order of the input parameters.
		/// </summary>
		public abstract class InputParams : ICloneable
		{
			public virtual object Clone()
			{
				return this.MemberwiseClone();
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="testGroup"></param>
		/// <param name="test"></param>
		/// <param name="input"></param>
		/// <param name="methodName"></param>
		/// <param name="invokeError"></param>
		/// <param name="otherErrors"></param>
		public static void LogErrors(AdvancedTestGroup testGroup, CdsSubTest test, InputParams input, string methodName, UPnPInvokeException invokeError, IList otherErrors)
		{
			if (invokeError != null)
			{
				StringBuilder msg = new StringBuilder();
				msg.Append("\r\n");
				msg.AppendFormat("[{0}]({1}) <Invoke Error>. ", testGroup.GroupName, test.Name, LogImportance.Critical.ToString());
				FieldInfo[] fi = input.GetType().GetFields();

				msg.AppendFormat("Method={0}\r\n \tInput=(", methodName);
				for (int i=0; i < fi.Length; i++)
				{
					msg.Append("\r\n\t\t");
					object val = fi[i].GetValue(input);
					string valString = "";
					if (val != null)
					{
						valString = val.ToString();
					}
					if (i > 0)
					{
						msg.Append(", ");
					}
					msg.AppendFormat("[{0}={1}]", fi[i].Name, valString);
				}
				msg.Append("\r\n\t).");
				msg.AppendFormat("\r\n\tInvokeErrorMessage=<{0}>.", PrintStackTraceRecursively(invokeError, "\t\t"));
				foreach (Exception e in otherErrors)
				{
					if (e != null)
					{
						msg.AppendFormat("\r\n\tAdditionalErrorInfo=<{0}>.", PrintStackTraceRecursively(e, "\t\t"));
					}
				}
				testGroup.AddEvent(LogImportance.Critical, test.Name, msg.ToString());
			}
		}

		public static string PrintStackTraceRecursively(Exception e, string tabs)
		{
			string msg = "\r\n"+ e.Message.Replace("\r\n", "\r\n"+tabs);
			
			string msg2 = "";
			
			if (e.InnerException != null)
			{
				msg2 = PrintStackTraceRecursively(e.InnerException, tabs+"\t");
			}

			return msg + msg2;
		}
	}
}
