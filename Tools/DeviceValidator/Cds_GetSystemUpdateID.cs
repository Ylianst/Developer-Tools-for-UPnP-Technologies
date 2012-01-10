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
using OpenSource.UPnP.AV;

namespace UPnPValidator
{
    public class CdsResult_GetSystemUpdateID : CdsTestResult
    {
        public string SystemUpdateID;
    }

    /// <summary>
    /// Summary description for Cds_GetSystemUpdateID.
    /// </summary>
    public class Cds_GetSystemUpdateID : CdsSubTest
    {
        private CdsResult_GetSystemUpdateID _Details;
        public override object Details { get { return _Details; } }

        public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
        {
        }
        public override UPnPTestStates Run(ICollection otherSubTests, CdsSubTestArgument arg)
        {
            this._TestState = UPnPTestStates.Running;

            CpContentDirectory CDS = this.GetCDS(arg._Device);
            _Details = new CdsResult_GetSystemUpdateID();

            uint updateID = 0;
            try
            {
                if (CDS != null)
                {
                    DateTime start = System.DateTime.Now;
                    CDS.Sync_GetSystemUpdateID(out updateID);
                    this.LogResponseTime(start, _Details, arg);
                }
            }
            catch (UPnPInvokeException ie)
            {
                arg.TestGroup.AddResult(this._Name + " test failed because of an invocation error: " + ie.Message);
                return UPnPTestStates.Failed;
            }

            arg._TestGroupState.SystemUpdateID = updateID;

            arg.TestGroup.AddResult(this._Name + " test passed. Returned=\"" + updateID.ToString() + "\".");
            this._TestState = UPnPTestStates.Pass;
            return this._TestState;
        }

        /// <summary>
        /// Updates the <see cref="CdsSubTestArgument.SystemUpdateID"/> value with the specified ID.
        /// Checks the current value of the state variable and returns zero if the 
        /// specified ID matches the value of the state variable.
        /// </summary>
        /// <param name="updateID"></param>
        /// <param name="arg"></param>
        /// <returns>0=equal; 1=updateID is greater than device's state variable; -1=updateID is less than device's state variable</returns>
        public static int UpdateSystemUpdateID(uint updateID, CdsSubTestArgument arg, CdsSubTest sub)
        {
            arg._TestGroupState.SystemUpdateID = updateID;

            CpContentDirectory CDS = sub.GetCDS(arg._Device);

            uint updateID2 = CDS.SystemUpdateID;

            if (updateID > updateID2)
            {
                return 1;
            }
            else if (updateID < updateID2)
            {
                return -1;
            }

            return 0;
        }

        protected override void SetTestInfo()
        {
            this._Name = "GetSystemUpdateID";
            this._Description = "Obtains UpdateID for the entire content hierarchy system. Required action.";
            this._ExpectedTestingTime = 30;
        }
    }
}
