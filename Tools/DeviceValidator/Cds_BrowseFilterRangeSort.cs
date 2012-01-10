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
using System.Collections;
using System.Text.RegularExpressions;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnPValidator
{
	public class CdsResult_BrowseFilterRangeSort: CdsResult_BrowseStats
	{
		public CdsResult_BrowseFilter	FilterResults;
		public CdsResult_BrowseRange	RangeResults;
		public CdsResult_BrowseSortCriteria		SortResults;
	}

	/// <summary>
	/// Summary description for Cds_BrowseFilterRangeSort.
	/// </summary>
	public class Cds_BrowseFilterRangeSort : Cds_BrowseTest
	{
		private CdsResult_BrowseFilterRangeSort _Details;
		public override object Details { get { return _Details; } }
		public override CdsResult_BrowseStats BrowseStats { get { return _Details; } }
		public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
		{
			// get the results from the prerequisite tests
			CdsResult_BrowseFilter	FILTER_RESULTS = null;
			CdsResult_BrowseRange	RANGE_RESULTS = null;
			CdsResult_BrowseSortCriteria SORT_RESULTS = null;

			foreach (ISubTest preTest in otherSubTests)
			{
				if (preTest.Name == this.PRE_FILTER.Name)
				{
					FILTER_RESULTS = preTest.Details as CdsResult_BrowseFilter;
				}
				else if (preTest.Name == this.PRE_RANGE.Name)
				{
					RANGE_RESULTS = preTest.Details as CdsResult_BrowseRange;
				}
				else if (preTest.Name == this.PRE_SORT.Name)
				{
					SORT_RESULTS = preTest.Details as CdsResult_BrowseSortCriteria;
				}
			}

			if (FILTER_RESULTS == null)
			{
				return;
			}

			if (RANGE_RESULTS == null)
			{
				return;
			}

			if (SORT_RESULTS == null)
			{
				return;
			}

			CdsResult_BrowseAll BROWSE_ALL = FILTER_RESULTS.BrowseAllResults;
			if (BROWSE_ALL.LargestContainer == null)
			{
				return;
			}
			if (BROWSE_ALL.MostMetadata == null)
			{
				return;
			}
			if (SORT_RESULTS.SortFields == null)
			{
				return;
			}

			int max = Math.Max(FILTER_RESULTS.Filters.Count, BROWSE_ALL.LargestContainer.ChildCount);
			max = Math.Max(max, SORT_RESULTS.SortFields.Count);
			this._ExpectedTestingTime = max * 900;
		}

		public override UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg)
		{
			CpContentDirectory CDS = this.GetCDS(arg._Device);
			_Details = new CdsResult_BrowseFilterRangeSort();
			this._TestState = UPnPTestStates.Running;
			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name + "\" started.");


			// get the results from the prerequisite tests
			CdsResult_BrowseFilter	FILTER_RESULTS = null;
			CdsResult_BrowseRange	RANGE_RESULTS = null;
			CdsResult_BrowseSortCriteria SORT_RESULTS = null;

			try
			{
				foreach (ISubTest preTest in otherSubTests)
				{
					if (preTest.Name == this.PRE_FILTER.Name)
					{
						FILTER_RESULTS = preTest.Details as CdsResult_BrowseFilter;
					}
					else if (preTest.Name == this.PRE_RANGE.Name)
					{
						RANGE_RESULTS = preTest.Details as CdsResult_BrowseRange;
					}
					else if (preTest.Name == this.PRE_SORT.Name)
					{
						SORT_RESULTS = preTest.Details as CdsResult_BrowseSortCriteria;
					}
				}

				if (FILTER_RESULTS == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_FILTER.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}

				if (RANGE_RESULTS == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_RANGE.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}

				if (SORT_RESULTS == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_SORT.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}

			}
			catch (Exception e)
			{
				throw new TestException(this._Name + " requires that the \"" + this.PRE_FILTER.Name + "\" and \"" + this.PRE_RANGE+ "\" and \"" + this.PRE_SORT + "\" tests be run before. An error occurred when attempting to obtain the results of those prerequisites.", otherSubTests, e);
			}

			UPnPTestStates state = this._TestState;
			CdsResult_BrowseAll BROWSE_ALL = FILTER_RESULTS.BrowseAllResults;
			if (BROWSE_ALL.LargestContainer == null)
			{
				throw new TestException(new Cds_BrowseAll().Name + " failed to find the container with the most child objects. " +this._Name+ " requires this value.", BROWSE_ALL);
			}
			if (BROWSE_ALL.MostMetadata == null)
			{
				throw new TestException(new Cds_BrowseAll().Name + " failed to find the media object with the most metadata. " +this._Name+ " requires this value.", BROWSE_ALL);
			}
			if (SORT_RESULTS.SortFields == null)
			{
				throw new TestException(new Cds_BrowseAll().Name + " failed to find the sortable fields. " +this._Name+ " requires this value.", SORT_RESULTS);
			}

			int max = Math.Max(FILTER_RESULTS.Filters.Count, BROWSE_ALL.LargestContainer.ChildCount);
			max = Math.Max(max, SORT_RESULTS.SortFields.Count);

			this._Details.ExpectedTotalBrowseRequests = max+1;
			this._Details.TotalBrowseRequests = 0;
			this._ExpectedTestingTime = this._Details.ExpectedTotalBrowseRequests * 900;
			arg.ActiveTests.UpdateTimeAndProgress(this._Details.TotalBrowseRequests*900);

			state = UPnPTestStates.Pass;
			for (int i=0; i <= max; i++)
			{
				ArrayList filterList = new ArrayList();
				for (int j=0; j < i; j++)
				{
					if (j < FILTER_RESULTS.Filters.Count)
					{
						filterList.Add(FILTER_RESULTS.Filters[j]);
					}
					else
					{
						break;
					}
				}
				string filterSettings = Cds_BrowseTest.GetCSVString(filterList);


				uint range = (uint) i;
				if (range > BROWSE_ALL.LargestContainer.ChildCount)
				{
					range = (uint) BROWSE_ALL.LargestContainer.ChildCount;
				}


				ArrayList sortList = new ArrayList();
				for (int j=0; j < i; j++)
				{
					if (j < SORT_RESULTS.SortFields.Count)
					{
						sortList.Add(SORT_RESULTS.SortFields[j]);
					}
					else
					{
						break;
					}
				}

				BrowseInput input = new BrowseInput();
				input.ObjectID = BROWSE_ALL.LargestContainer.ID;
				input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN;
				input.Filter = filterSettings;
				input.RequestedCount = range;
				input.SortCriteria = Cds_BrowseSortCriteria.GetSortCriteriaString(sortList, i);
				input.StartingIndex = 0;

				this._ExpectedTestingTime = (max) * 900;
				arg.ActiveTests.UpdateTimeAndProgress(this._Details.TotalBrowseRequests*900);
				CdsBrowseSearchResults br = Cds_BrowseTest.Browse(input, this, arg, CDS, _Details);

				MediaContainer original = (MediaContainer) BROWSE_ALL.LargestContainer;
				uint ignored;
				IList expectedResults;
				
				expectedResults = original.BrowseSorted(0, input.RequestedCount, new MediaSorter(true, input.SortCriteria), out ignored);

				try
				{
					if (br.WorstError <= UPnPTestStates.Warn)
					{
						if (br.MediaObjects.Count != expectedResults.Count)
						{
							throw new TerminateEarly(input.PrintBrowseParams()+ " returned DIDL-Lite that declared " +br.MediaObjects.Count+ " media objects but test expected " + expectedResults.Count.ToString() + " media objects as found in a prerequisite test.");
						}

						bool warnOrder = false;
						for (int ri=0; ri < br.MediaObjects.Count; ri++)
						{
							IUPnPMedia resultObj = (IUPnPMedia) br.MediaObjects[ri];
							IUPnPMedia originalObj = (IUPnPMedia) expectedResults[ri];

							if (resultObj.ID != originalObj.ID)
							{
								warnOrder = true;
							}

							foreach (string propName in resultObj.Properties.PropertyNames)
							{
								if (filterList.Contains(propName) == false)
								{
									if (
										(propName != T[_DC.title]) && 
										(propName != T[_UPNP.Class])
										)
									{
										StringBuilder msg = new StringBuilder();
										msg.AppendFormat("\"" + this.Name + "\" is terminating early because {0} returned DIDL-Lite with \"{1}\" metadata when it should not have done so.", input.PrintBrowseParams(), propName);
										throw new TerminateEarly(msg.ToString());
									}
								}
							}
						}

						int expectedCount = i;
						if ((i == 0) || (i > BROWSE_ALL.LargestContainer.ChildCount))
						{
							expectedCount = BROWSE_ALL.LargestContainer.ChildCount;
						}

						if (br.MediaObjects.Count != expectedCount)
						{
							StringBuilder msg = new StringBuilder();
							msg.AppendFormat("\"{0}\" did a {1} and the DIDL-Lite result only has {2} media objects when {3} media objects were expected.", this.Name, input.PrintBrowseParams(), br.MediaObjects.Count, expectedCount);
							msg.AppendFormat(".\r\nDIDL-Lite ==> {0}", br.Result);
							throw new TerminateEarly(msg.ToString());
						}

						if (warnOrder)
						{
							/*
							ArrayList missingResults = new ArrayList();

							foreach (IUPnPMedia em in expectedResults)
							{
								bool found = false;
								foreach (IUPnPMedia fm in br.MediaObjects)
								{
									if (em.ID == fm.ID)
									{
										found = true;
										break;
									}
								}

								if (found == false)
								{
									missingResults.Add(em);
								}
							}

							if (missingResults.Count > 0)
							{
								state = UPnPTestStates.Failed;
								StringBuilder msg = new StringBuilder();
								msg.AppendFormat("\"{0}\" did a {1} and the result is missing media objects.", this.Name, input.PrintBrowseParams());
								msg.Append("\r\nExpected order of IDs: ");
								int z = 0;
								foreach (IUPnPMedia em in expectedResults)
								{
									if (z > 0)
									{
										msg.Append(",");
									}
									msg.AppendFormat("\"{0}\"", em.ID);
									z++;
								}
								msg.Append("\r\nDIDL-Lite result's order of IDs: ");
								z = 0;
								foreach (IUPnPMedia em in br.MediaObjects)
								{
									if (z > 0)
									{
										msg.Append(",");
									}
									msg.AppendFormat("\"{0}\"", em.ID);
									z++;
								}
								msg.AppendFormat(".\r\nDIDL-Lite ==> {0}", br.Result);
								throw new TerminateEarly(msg.ToString());
							}
							else
							*/
							{
								StringBuilder msg = new StringBuilder();
								msg.AppendFormat("WARNING: \"{0}\" did a {1} and got items in a different order. Target CDS either has an error in its sorting logic, or sorting logic intentionally deviates from test.", this.Name, input.PrintBrowseParams());
								msg.Append("\r\nExpected order of IDs: ");
								int z = 0;
								foreach (IUPnPMedia em in expectedResults)
								{
									if (z > 0)
									{
										msg.Append(",");
									}
									msg.AppendFormat("\"{0}\"", em.ID);
									z++;
								}
								msg.Append("\r\nDIDL-Lite result's order of IDs: ");
								z = 0;
								foreach (IUPnPMedia em in br.MediaObjects)
								{
									if (z > 0)
									{
										msg.Append(",");
									}
									msg.AppendFormat("\"{0}\"", em.ID);
									z++;
								}
								msg.AppendFormat(".\r\nDIDL-Lite ==> {0}", br.Result);
								arg._TestGroup.AddEvent(LogImportance.Medium, this.Name, msg.ToString());
								state = UPnPTestStates.Warn;
							}
						}
					}
					else
					{
						throw new TerminateEarly("\"" + this.Name + "\" is terminating early because " +input.PrintBrowseParams()+ " returned with an error or had problems with the DIDL-Lite.");
					}
				}
				catch (TerminateEarly te)
				{
					arg._TestGroup.AddEvent(LogImportance.Critical, this.Name, "\""+this.Name+"\" terminating early. Reason ==> " + te.Message);
					state = UPnPTestStates.Failed;
					break;
				}
			}


			// finish up logging
			this._TestState = state;

			StringBuilder sb = new StringBuilder();
			sb.AppendFormat("\"{0}\" completed", this.Name);

			if (this._TestState <= UPnPTestStates.Running)
			{
				throw new TestException("\"" +this.Name+ "\" must have a pass/warn/fail result.", this._TestState);
			}

			switch (this._TestState)
			{
				case UPnPTestStates.Pass:
					sb.Append(" successfully.");
					break;
				case UPnPTestStates.Warn:
					sb.Append(" with warnings.");
					break;
				case UPnPTestStates.Failed:
					sb.Append(" with a failed result.");
					break;
			}

			arg._TestGroup.AddResult(sb.ToString());
            
			if (this._TestState <= UPnPTestStates.Warn)
			{
				if (_Details.TotalBrowseRequests != _Details.ExpectedTotalBrowseRequests)
				{
					throw new TestException("TotalBrowseRequests="+_Details.TotalBrowseRequests.ToString()+" ExpectedTotal="+_Details.ExpectedTotalBrowseRequests.ToString(), _Details);
				}
			}

			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, this.Name + " finished.");

			return this._TestState;
		}

		protected override void SetTestInfo()
		{
			this._Name = "Browse Filter, Range, & Sort";
			this._Description = "Finds the container with the most children calls BrowseDirectChildren on it with various Filter, Range, and SortCriteria settings.";
			this._ExpectedTestingTime = 900 * 3 * 3;

			this._Prerequisites.Add(this.PRE_FILTER);
			this._Prerequisites.Add(this.PRE_RANGE);
			this._Prerequisites.Add(this.PRE_SORT);
		}

		private Cds_BrowseFilter		PRE_FILTER = new Cds_BrowseFilter();
		private Cds_BrowseRange			PRE_RANGE = new Cds_BrowseRange();
		private Cds_BrowseSortCriteria	PRE_SORT = new Cds_BrowseSortCriteria();

		private static Tags T = Tags.GetInstance();

	}
}
