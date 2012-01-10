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
	public class CdsResult_GetSearchCapabilities : CdsTestResult
	{
		public string SearchCapabilities;
	}
	
	/// <summary>
	/// Summary description for .
	/// </summary>
	public sealed class Cds_GetSearchCapabilities : CdsSubTest
	{
		private CdsResult_GetSearchCapabilities _Details;
		public override object Details { get { return _Details; } }
		
		public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
		{
		}

		public override UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg)
		{
			this._TestState = UPnPTestStates.Running;

			CpContentDirectory CDS = this.GetCDS(arg._Device);
			_Details = new CdsResult_GetSearchCapabilities();

			try
			{
				if (CDS != null)
				{
					DateTime start = System.DateTime.Now;
					CDS.Sync_GetSearchCapabilities(out _Details.SearchCapabilities);
					this.LogResponseTime(start, _Details, arg);
				}
			}
			catch (UPnPInvokeException ie)
			{
				arg.TestGroup.AddResult(this._Name + " test failed because of an invocation error: " + ie.Message);
				this._TestState = UPnPTestStates.Failed;
				return this._TestState;
			}

			/* REMOVED BECAUSE EMPTY STRING IS LEGIT
			if ((_Details.SearchCapabilities == null) || (_Details.SearchCapabilities == ""))
			{
				arg.TestGroup.AddResult(this._Name + " test failed because result was empty or null string.");
				return UPnPTestStates.Failed;
			}
			*/

			arg._TestGroupState.SearchCapabilities = _Details.SearchCapabilities;
			arg.TestGroup.AddResult(this._Name + " test passed. Returned=\"" + _Details.SearchCapabilities + "\".");
			this._TestState = UPnPTestStates.Pass;
			return this._TestState;
		}

		protected override void SetTestInfo()
		{
			this._Name = "GetSearchCapabilities";
			this._Description = "Obtains searching capabilities for the CDS. Required action.";
			this._ExpectedTestingTime = 30;
		}
	}
}
