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
    /// Summary description for AdvancedTestGroup.
    /// </summary>
    public abstract class AdvancedTestGroup : BasicTests.BasicTestGroup
    {
        /// <summary>
        /// An AdvancedTestGroup is made up of a group of <see cref="ISubTest"/> objects that may require other
        /// <see cref="ISubTest"/> objects to run as prerequisite tests.
        /// </summary>
        /// <param name="category"></param>
        /// <param name="groupName"></param>
        /// <param name="description"></param>
        public AdvancedTestGroup(string category, string groupName, string description)
        {
            this.Category = category;
            this.GroupName = groupName;
            this.Description = description;
        }

        /// <summary>
        /// Returns a shallow copy list of <see cref="ISubTest"/> objects
        /// that make up the test group.
        /// </summary>
        public virtual IList SubTests
        {
            get
            {
                ArrayList result = new ArrayList((ICollection)AllTests.Values);
                return result;
            }
        }

        /// <summary>
        /// Calls the <see cref="BasicTestGroup.SetState"/> method for each
        /// subtest in this group's <see cref="AdvancedTestgroup.SubTests"/>
        /// property.
        /// </summary>
        public virtual void UpdateTestStates()
        {
            foreach (ISubTest sub in this.SubTests)
            {
                this.SetState(sub.Name, sub.TestState);
            }
        }

        /// <summary>
        /// This test builds a <see cref="TestQueue"/> and runs
        /// all of the subtests identified by their name.
        /// </summary>
        /// <param name="runThese">names of tests to run; null indicates all tests of the testgroup</param>
        /// <param name="arg">argument to send to the <see cref="ISubTest"/> objects</param>
        public void RunTests(string[] runThese, ISubTestArgument arg)
        {
            TestQueue tq = null;

            if (runThese == null)
            {
                tq = new TestQueue(this.AllTests.Values, arg);
            }
            else
            {
                ArrayList run = new ArrayList();
                foreach (string name in runThese)
                {
                    bool added = false;
                    foreach (ISubTest sub in this.AllTests.Values)
                    {
                        if (name == sub.Name)
                        {
                            run.Add(sub);
                            added = true;
                            break;
                        }
                    }

                    if (added == false)
                    {
                        throw new ApplicationException("RunTests(): Specified the name of a test that does not exist in the test group.");
                    }
                }

                tq = new TestQueue(run, arg);
            }

            // run all of the tests
            arg.ActiveTests = tq;
            tq.RunQueue();
        }

        /// <summary>
        /// Derived classes call this method to add subtests associated
        /// with this test group. The implementation will add the
        /// <see cref="ISubTest"/> objects to the <see cref="AdvancedTestGroup.AllTests"/>
        /// field.
        /// </summary>
        /// <param name="subTest"></param>
        protected void AddSubTest(ISubTest subTest)
        {
            foreach (ISubTest sub in AllTests.Values)
            {
                bool found = false;
                if (sub.Name == subTest.Name)
                {
                    found = true;
                }

                if (found)
                {
                    throw new ApplicationException("Cannot have two subtests with the same name.");
                }
            }

            AllTests.Add(subTest, subTest);

            this.AddTest(subTest.Name, subTest.Description);
        }


        /// <summary>
        /// Derived classes use this to store the subtests for this test group.
        /// </summary>
        protected SortedList AllTests = new SortedList(new SubTestOrderer());

        //private int m_TotalTime = 0;
    }

    public struct SubTestOrderer : IComparer
    {
        public int Compare(object sub1, object sub2)
        {
            SubTest s1 = (SubTest)sub1;
            SubTest s2 = (SubTest)sub2;
            int cmp = s1.CompareTo(s2);

            if (cmp == 0)
            {
                return -1;
            }

            return cmp;
        }
    }
}
