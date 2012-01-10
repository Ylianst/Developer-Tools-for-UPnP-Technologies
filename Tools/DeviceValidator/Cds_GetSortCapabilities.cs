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

    public class CdsResult_GetSortCapabilities : CdsTestResult
    {
        public string SortCapabilities;
    }
    /// <summary>
    /// Summary description for Cds_GetSortCapabilities.
    /// </summary>
    public class Cds_GetSortCapabilities : CdsSubTest
    {
        private CdsResult_GetSortCapabilities _Details;
        public override object Details { get { return _Details; } }

        public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
        {
        }
        public override UPnPTestStates Run(ICollection otherSubTests, CdsSubTestArgument arg)
        {
            this._TestState = UPnPTestStates.Running;

            CpContentDirectory CDS = this.GetCDS(arg._Device);
            _Details = new CdsResult_GetSortCapabilities();
            _Details.SortCapabilities = null;

            try
            {
                if (CDS != null)
                {
                    DateTime start = System.DateTime.Now;
                    CDS.Sync_GetSortCapabilities(out _Details.SortCapabilities);
                    this.LogResponseTime(start, _Details, arg);
                }
            }
            catch (UPnPInvokeException ie)
            {
                arg.TestGroup.AddResult(this._Name + " test failed because of an invocation error: " + ie.Message);
                this._TestState = UPnPTestStates.Failed;
                return this._TestState;
            }

            /* NKIDD - REMOVED BECAUSE EMPTY STRING IS LEGIT
            if ((_Details.SortCapabilities == null) || (_Details.SortCapabilities == ""))
            {
                arg.TestGroup.AddResult(this._Name + " test failed because result was empty or null string.");
                this._TestState = UPnPTestStates.Failed;
                return this._TestState;
            }
            */

            arg._TestGroupState.SortCapabilities = _Details.SortCapabilities;
            arg.TestGroup.AddResult(this._Name + " test passed. Returned=\"" + _Details.SortCapabilities + "\".");
            this._TestState = UPnPTestStates.Pass;
            return this._TestState;
        }

        protected override void SetTestInfo()
        {
            this._Name = "GetSortCapabilities";
            this._Description = "Obtains sorting capabilities for the CDS. Required action.";
            this._ExpectedTestingTime = 30;
        }
    }
}
