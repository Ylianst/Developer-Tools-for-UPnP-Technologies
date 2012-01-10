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
	public sealed class TestQueue
	{
		public TestQueue(ICollection subTests, ISubTestArgument arg)
		{
			foreach (ISubTest sub in subTests)
			{
				AddTest(sub);
			}

			Arg = arg;
		}

		public void RunQueue()
		{
			Arg.TestGroup.state = UPnPTestStates.Running;

			UPnPService[] services = Arg.Device.Services;
			UPnPServiceWatcher[] watchers = new UPnPServiceWatcher[services.Length];
			for (int i=0; i < services.Length; i++)
			{
				//watchers[i] = new UPnPServiceWatcher(services[i], null, new UPnPServiceWatcher.SniffPacketHandler(this.SniffPacketSink));
			}

			foreach (ISubTest sub in Q.Values)
			{
				Arg.TestGroup.SetState(sub.Name, UPnPTestStates.Running);

				bool cont = true;
				foreach (SubTest pre1 in sub.Prerequisites)
				{
					pre1.CalculateExpectedTestingTime(Q.Values, Arg);

					ISubTest pre = null;
					foreach (ISubTest pre2 in Q.Values)
					{
						if (pre2.Name == pre1.Name)
						{
							pre = pre2;
						}
					}

					if (
						(!
						(pre.TestState == UPnPTestStates.Pass) ||
						(pre.TestState == UPnPTestStates.Warn)
						)
						)
					{
						cont = false;
					}
				}

				this.UpdateTimeAndProgress(0);
				if (cont)
				{
					UPnPTestStates result = sub.Run(Q.Values, Arg);
					Arg.TestGroup.SetState(sub.Name, result);

					if (sub.TestState != result)
					{
						throw new ApplicationException("Test state does not match the set value.");
					}
				}
				this.UpdateTimeAndProgress(0);
			}

			UPnPTestStates MasterResult = UPnPTestStates.Pass;
			foreach (ISubTest done in Q.Values)
			{
				if (done.TestState > MasterResult)
				{
					MasterResult = done.TestState;
					break;
				}
			}

			for (int i=0; i < services.Length; i++)
			{
				//watchers[i].OnSniffPacket -= new UPnPServiceWatcher.SniffPacketHandler(this.SniffPacketSink);
			}


			Arg.TestGroup.state = MasterResult;
		}

		/// <summary>
		/// Logs packets.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="MSG"></param>
		private void SniffPacketSink(UPnPServiceWatcher sender, HTTPMessage MSG)
		{
			this.Arg.TestGroup.AddPacket(MSG);
		}

		/// <summary>
		/// <see cref="ISubTest"/> objects can call this method
		/// to start a countdown timer and progress bar info. The
		/// method should be called regularly after the completion
		/// of a subtest and also when a subtest wants to update its
		/// progress with finer granularity.
		/// 
		/// <para>
		/// Method updates the TotalTime property using the <see cref="ISubTest.TestingTime"/>
		/// values of all tests in the group.
		/// </para>
		/// 
		/// <para>
		/// Progress is determined by the sum of <see cref="ISubTest.TestingTime"/> 
		/// values of completed subtests and the "secondOffsetForCurrentSubTest"
		/// value.
		/// </para>
		/// </summary>
		/// <param name="secondOffsetForCurrentSubTest">
		/// This value should be zero unless an actively executing subtest wants to report
		/// additional progress within itself, in which case the value should reflect
		/// the total number of elapsed seconds (in terms of total test time). 
		/// </param>
		public void UpdateTimeAndProgress(int secondOffsetForCurrentSubTest)
		{
			int elapsed = 0;
			m_TotalTime = 0;

			foreach (ISubTest test in this.Q.Values)
			{
				if (test.Enabled)
				{
					m_TotalTime += test.ExpectedTestingTime;
				}

				// calculate elapsed time
				if (
					(
					(test.TestState == UPnPTestStates.Pass) ||
					(test.TestState == UPnPTestStates.Warn)
					)
					&&
					test.Enabled
					)
				{
					elapsed += test.ExpectedTestingTime;
				}
			}

			elapsed += secondOffsetForCurrentSubTest;

			Arg.TestGroup.AbortCountDown();
			Arg.TestGroup.StartCountDown(elapsed, m_TotalTime);
		}

		/// <summary>
		/// Total expected time until completion of the test.
		/// This value may grow during execution of subtests.
		/// </summary>
		private int m_TotalTime = 0;
		
		/// <summary>
		/// Total time expected for this entire test group. This value
		/// may change as the test progresses.
		/// </summary>
		public int TotalTime { get { return this.m_TotalTime; } }

		private void AddTest (ISubTest sub)
		{
			bool found = false;
			foreach (ISubTest st in Q.Values)
			{
				if (st.Name == sub.Name)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				foreach (ISubTest pre in sub.Prerequisites)
				{
					AddTest(pre);
				}

				Q.Add(sub, sub);
			}
		}

		private SortedList Q = new SortedList(new SubTestOrderer());
		private ISubTestArgument Arg;
	}
}
