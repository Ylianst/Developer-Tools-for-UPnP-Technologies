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
	/// <summary>
	/// Summary description for CdsTestGroup.
	/// </summary>
	public sealed class CdsTestGroup : AdvancedTestGroup
	{
		public CdsTestGroup()
			: base
			(
				"UPnP A/V", 
				"ContentDirectory Test Scenarios",
				"UPnP A/V 1.0 ContentDirectory service test suite. This suite can evaluate a UPnP device's implementation compliance towards a UPnP A/V 1.0 ContentDirectory service."
			)
		{
			this.AddSubTest(new Cds_GetSearchCapabilities());
			this.AddSubTest(new Cds_GetSortCapabilities());
			this.AddSubTest(new Cds_GetSystemUpdateID());
			this.AddSubTest(new Cds_BrowseAll());
			this.AddSubTest(new Cds_BrowseRange());
			this.AddSubTest(new Cds_BrowseFilter());
			this.AddSubTest(new Cds_BrowseSortCriteria());
			this.AddSubTest(new Cds_BrowseFilterRangeSort());
		}

		public override void Start(UPnPDevice device)
		{
			UPnPDevice d = device;
			UPnPService[] services = d.GetServices(CpContentDirectory.SERVICE_NAME);
			if (services == null || services.Length == 0) 
			{
				enabled = false;
				return;
			}


			CdsSubTestArgument arg = new CdsSubTestArgument();
			arg._Device = device;
			arg._TestGroupState = new CdsTestGroupState();
			arg._TestGroup = this;
			this.RunTests(null, arg);
		}
	}
}
