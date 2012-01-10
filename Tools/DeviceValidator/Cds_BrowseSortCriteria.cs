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
	public class CdsResult_BrowseSortCriteria: CdsResult_BrowseStats
	{
		public CdsResult_BrowseAll BrowseAllResults;
		public CdsResult_GetSortCapabilities SortCapsResults;
		public ArrayList SortFields;
	}

	/// <summary>
	/// Summary description for Cds_BrowseSortCriteria.
	/// </summary>
	public class Cds_BrowseSortCriteria: Cds_BrowseTest
	{
		private CdsResult_BrowseSortCriteria _Details;
		public override object Details { get { return _Details; } }
		public override CdsResult_BrowseStats BrowseStats { get { return _Details; } }

		public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
		{
			// get the results from the prerequisite tests
			CdsResult_BrowseAll BROWSE_RESULTS = null;
			CdsResult_GetSortCapabilities SORTCAPS = null;

			foreach (ISubTest preTest in otherSubTests)
			{
				if (preTest.Name == this.PRE_BROWSEALL.Name)
				{
					BROWSE_RESULTS = preTest.Details as CdsResult_BrowseAll;
				}
				else if (preTest.Name == this.PRE_SORTCAPS.Name)
				{
					SORTCAPS = preTest.Details as CdsResult_GetSortCapabilities;
				}
			}

			if (BROWSE_RESULTS == null)
			{
				return;
			}

			if (SORTCAPS == null)
			{
				return;
			}

			if (BROWSE_RESULTS.LargestContainer == null)
			{
				return;
			}

			MediaContainer MC = BROWSE_RESULTS.LargestContainer as MediaContainer;
			if (MC == null)
			{
				return;
			}
			
			ArrayList sortFields = new ArrayList();
			if (SORTCAPS.SortCapabilities == "")
			{
			}
			else if (SORTCAPS.SortCapabilities == "*")
			{
				sortFields = (ArrayList) BROWSE_RESULTS.PropertyNames.Clone();
			}
			else
			{
				sortFields.AddRange ( GetSortFields(SORTCAPS.SortCapabilities) );
			}

			int fieldCount = sortFields.Count;
			IList childList = BROWSE_RESULTS.LargestContainer.CompleteList;
			uint inc = (uint) (childList.Count / 3);
			int firstInc = (fieldCount / 3);
			if (firstInc == 0)
			{
				firstInc = 1;
			}
			int totalBrowses = 0;
			for (int numFields = 0; numFields < fieldCount; numFields++)
			{
				for (int first = 0; first < fieldCount; first+=firstInc)
				{
					//for (uint i=0; i < childList.Count; i+=inc)
					{
						totalBrowses++;
					}
				}
			}
			//add one for an unsorted browse
			totalBrowses++;
			//multiply by 2 because we have 2 rounds to check for consistency in ordered results
			totalBrowses *= 2;
			//calculate expected time
			this._ExpectedTestingTime = totalBrowses * 900;
		}

		private struct Round2
		{
			public BrowseInput Input;
			public CdsBrowseSearchResults PreviousResult;
		}

		public override UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg)
		{
			CpContentDirectory CDS = this.GetCDS(arg._Device);
			_Details = new CdsResult_BrowseSortCriteria();
			this._TestState = UPnPTestStates.Running;
			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name + "\" started.");

			// get the results from the prerequisite tests
			CdsResult_BrowseAll BROWSE_RESULTS = null;
			CdsResult_GetSortCapabilities SORTCAPS = null;
			try
			{
				foreach (ISubTest preTest in otherSubTests)
				{
					if (preTest.Name == this.PRE_BROWSEALL.Name)
					{
						BROWSE_RESULTS = preTest.Details as CdsResult_BrowseAll;
					}
					else if (preTest.Name == this.PRE_SORTCAPS.Name)
					{
						SORTCAPS = preTest.Details as CdsResult_GetSortCapabilities;
					}
				}

				if (BROWSE_RESULTS == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_BROWSEALL.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}

				if (SORTCAPS == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_SORTCAPS.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}
			}
			catch (Exception e)
			{
				throw new TestException(this._Name + " requires that the \"" + this.PRE_BROWSEALL.Name + "\" and \"" + this.PRE_SORTCAPS+ "\" tests be run before. An error occurred when attempting to obtain the results of those prerequisites.", otherSubTests, e);
			}
			_Details.BrowseAllResults = BROWSE_RESULTS;
			_Details.SortCapsResults = SORTCAPS;

			UPnPTestStates state = this._TestState;

			if (BROWSE_RESULTS.LargestContainer == null)
			{
				throw new TestException(this.PRE_BROWSEALL.Name + " failed to find the container with the most child objects. " +this._Name+ " requires this value.", BROWSE_RESULTS);
			}

			MediaContainer MC = BROWSE_RESULTS.LargestContainer as MediaContainer;
			if (MC == null)
			{
				throw new TestException(this.PRE_BROWSEALL.Name + " has the largest container as type \"" +BROWSE_RESULTS.LargestContainer.GetType().ToString() +"\" when \"" +this.Name+ "\" requires \"" +typeof(MediaContainer).ToString()+ "\".", BROWSE_RESULTS);
			}
			

			ArrayList sortFields = new ArrayList();
			if (SORTCAPS.SortCapabilities == "")
			{
				//arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name+"\" has no sorting capabilities.");
			}
			else if (SORTCAPS.SortCapabilities == "*")
			{
				sortFields = (ArrayList) BROWSE_RESULTS.PropertyNames.Clone();
			}
			else
			{
				sortFields.AddRange ( GetSortFields(SORTCAPS.SortCapabilities) );
			}

			_Details.ExpectedTotalBrowseRequests = 0;
			_Details.SortFields = sortFields;
			int fieldCount = sortFields.Count;
			IList childList = BROWSE_RESULTS.LargestContainer.CompleteList;
			_Details.ExpectedTotalBrowseRequests = 0;//fieldCount * fieldCount * fieldCount;
			uint inc = (uint) (childList.Count / 3);
			int firstInc = (fieldCount / 3);
			if (firstInc == 0)
			{
				firstInc = 1;
			}
			for (int numFields = 0; numFields < fieldCount; numFields++)
			{
				for (int first = 0; first < fieldCount; first+=firstInc)
				{
					//for (uint i=0; i < childList.Count; i+=inc)
					{
						_Details.ExpectedTotalBrowseRequests++;
					}
				}
			}
			// add 1 for an unsorted browse
			_Details.ExpectedTotalBrowseRequests++;
			//multiply by 2 because we have 2 rounds to check for consistency in ordered results
			_Details.ExpectedTotalBrowseRequests *= 2;

			//calculate time
			this._ExpectedTestingTime = _Details.ExpectedTotalBrowseRequests * 900;
			arg.ActiveTests.UpdateTimeAndProgress(0);

			if (state <= UPnPTestStates.Running)
			{
				state = UPnPTestStates.Pass;
				try
				{
					ArrayList round2 = new ArrayList();

					//perform the standard unsorted browse
					BrowseInput input = new BrowseInput();
					input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN;
					input.StartingIndex = 0;
					input.ObjectID = MC.ID;
					input.RequestedCount = 0;
					input.Filter = "*";
					input.SortCriteria = "";

					CdsBrowseSearchResults br = Browse(input, this, arg, CDS, _Details);
					Round2 r2 = new Round2();
					r2.Input = (BrowseInput) input.Clone();
					r2.PreviousResult = br;
					round2.Add(r2);

					for (int numFields = 0; numFields < fieldCount; numFields++)
					{
						for (int first = 0; first < fieldCount; first+=firstInc)
						{

							ArrayList sortSettings = GetSortSettings(sortFields, first, first);
							input.SortCriteria = GetSortCriteriaString(sortSettings, numFields+first);
							arg.ActiveTests.UpdateTimeAndProgress(_Details.TotalBrowseRequests * 900);

							uint ignored;

							//use this sorter for to determine the expected order of the media objects
							IMediaSorter sorter = new MediaSorter(true, input.SortCriteria);
							IList expectedSorted = MC.BrowseSorted(0, 0, sorter, out ignored);

							br = Browse(input, this, arg, CDS, _Details);
							arg.ActiveTests.UpdateTimeAndProgress(_Details.TotalBrowseRequests * 900);
							
							this.CompareResultsAgainstExpected(br, expectedSorted, ref state, arg, input, false);

							r2 = new Round2();
							r2.Input = (BrowseInput) input.Clone();
							r2.PreviousResult = br;
							round2.Add(r2);
						}
					}

					//do round2 - check for consistency in results
					foreach (Round2 r in round2)
					{
						br = Browse(r.Input, this, arg, CDS, _Details);
						arg.ActiveTests.UpdateTimeAndProgress(_Details.TotalBrowseRequests * 900);
						this.CompareResultsAgainstExpected(br, r.PreviousResult.MediaObjects, ref state, arg, r.Input, true);
					}
				}
				catch (TerminateEarly te)
				{
					string reason = "\"" +this.Name+ "\" terminating early. Reason => " + te.Message;
					arg._TestGroup.AddEvent(LogImportance.Critical, this.Name, reason);

					state = UPnPTestStates.Failed;
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

			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, sb.ToString());

			return this._TestState;
		}


		public void CompareResultsAgainstExpected(CdsBrowseSearchResults br, IList expectedResults, ref UPnPTestStates state, CdsSubTestArgument arg, BrowseInput input, bool strictOrder)
		{
			if (br.WorstError >= UPnPTestStates.Failed)
			{
				throw new TerminateEarly("\"" + this.Name + "\" is terminating early because " +input.PrintBrowseParams()+ " returned with an error or had problems with the DIDL-Lite.");
			}
			else
			{
				if (br.MediaObjects.Count != expectedResults.Count)
				{
					throw new TerminateEarly("\""+this.Name+"\" did a " +input.PrintBrowseParams()+ " and it should have returned "+expectedResults.Count+ " media objects. DIDL-Lite contained " +br.MediaObjects.Count+ " media objects. DIDL-Lite => " + br.Result);
				}

				bool warnResults = false;
				for (int i=0; i < br.MediaObjects.Count; i++)
				{

					IUPnPMedia gotThis = (IUPnPMedia) br.MediaObjects[i];
					IUPnPMedia expectedMedia = (IUPnPMedia) expectedResults[i];

					if (gotThis.ID == expectedMedia.ID)
					{
						//arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name+"\" did a " +input.PrintBrowseParams()+ " and encountered no errors in the results.");
					}
					else
					{
						bool failed = false;
						if ((input.SortCriteria == null) || (input.SortCriteria == ""))
						{
							failed = true;
						}
						else
						{
							// Use this sorter to test for value-equality in situations where the expected order didn't match.
							// We need to do this because two media objects may be value-equivalent according to a sorting
							// algorithm, in which case there's no way to really distinguish what order they should be in.
							IMediaSorter sorter2 = new MediaSorter(false, input.SortCriteria);

							int cmp = sorter2.Compare(gotThis, expectedMedia);
							if (cmp != 0)
							{
								arg.TestGroup.AddEvent(LogImportance.Medium, this.Name, "\""+this.Name+"\" found media object ID=\""+gotThis.ID+"\" when it expected to find \""+expectedMedia.ID+"\" and they are not equal in their sorted order.");
								warnResults = true;
							}
							else
							{
								if (strictOrder == false)
								{
									arg.TestGroup.AddEvent(LogImportance.Low, this.Name, "\""+this.Name+"\" found media object ID=\""+gotThis.ID+"\" when it expected to find \""+expectedMedia.ID+"\" but since they are effectively value-equivalent, the ordering is OK.");
								}
								else
								{
									failed = true;
								}
							}
						}

						if (failed)
						{
							StringBuilder msg = new StringBuilder();
							msg.AppendFormat("\"{0}\" did a {1} and the order of object ID's in the result conflicts with previous browse requests.");
							msg.AppendFormat("\r\n\r\nReceived objects in order by ID: ");
							int z = 0;
							foreach (IUPnPMedia em in br.MediaObjects)
							{
								if (z > 0)
								{
									msg.Append(",");
								}
								msg.AppendFormat("\"{0}\"", em.ID);
								z++;
							}
							msg.Append("\r\n\r\nThe expected order by ID is: ");
							z = 0;
							foreach (IUPnPMedia em in expectedResults)
							{
								if (z > 0)
								{
									msg.Append(",");
								}
								msg.AppendFormat("\"{0}\"", em.ID);
								z++;
							}
							msg.AppendFormat(".\r\n\r\nDIDL-Lite ==> {0}", br.Result);
							throw new TerminateEarly(msg.ToString());
						}
					}
				}

				if (warnResults == false)
				{
					arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name+"\" did a " +input.PrintBrowseParams()+ " and encountered no errors or warnings in the results.");
				}
				else
				{
					StringBuilder msg = new StringBuilder();
					msg.AppendFormat("WARNING: \"{0}\" did a {1} and \r\nreceived results in the following order by ID: ", this.Name, input.PrintBrowseParams());
					int z = 0;
					foreach (IUPnPMedia em in br.MediaObjects)
					{
						if (z > 0)
						{
							msg.Append(",");
						}
						msg.AppendFormat("\"{0}\"", em.ID);
						z++;
					}
					msg.Append("\r\n\r\nThe expected order by ID is: ");
					z = 0;
					foreach (IUPnPMedia em in expectedResults)
					{
						if (z > 0)
						{
							msg.Append(",");
						}
						msg.AppendFormat("\"{0}\"", em.ID);
						z++;
					}
					msg.AppendFormat(".\r\n\r\nDIDL-Lite ==> {0}", br.Result);
					// warn
					state = UPnPTestStates.Warn;
					arg._TestGroup.AddEvent(LogImportance.Medium, this.Name, msg.ToString());
				}
			}
		}

		public static bool DoZeroOneBitCountsMatch (int bitLength, int zeroOneBits)
		{
			int one = 0;
			int zero = 0;
			for (int i=0; i < bitLength; i++)
			{
				int mask = 1 << i;
				bool isOne = ((zeroOneBits & mask) != 0);

				if (isOne)
				{
					one++;
				}
				else
				{
					zero++;
				}
			}

			return (one == zero);
		}

		public static string GetSortCriteriaString (IList fields, int ascendingDescendingBits)
		{
			StringBuilder sb = new StringBuilder();
			int sbi = 0;
			foreach (string val in fields)
			{
				if (sbi > 0)
				{
					sb.Append(",");
				}

				int mask = 1 << sbi;
				bool isAscending = ((ascendingDescendingBits & mask) != 0);

				if (isAscending)
				{
					sb.Append("+");
				}
				else
				{
					sb.Append("-");
				}

				sb.Append(val);
				sbi++;
			}
			return sb.ToString();
		}

		private ArrayList GetSortSettings(IList fields, int first, int from)
		{
			ArrayList sortSettings = new ArrayList();
			sortSettings.Add( fields[first] );
			for (int i=0; i < fields.Count; i++)
			{
				if (from >= fields.Count)
				{
					from = from - fields.Count;
				}

				bool added = false;
				string str = (string)fields[from];
				int posAmp = str.IndexOf("@");
				if (posAmp >= 0)
				{
					str = str.Remove(0, posAmp);
				}
				foreach (string sortField in sortSettings)
				{
					if (sortField.IndexOf(str) >= 0)
					{
						added = true;
						break;
					}
				}

				if (added == false)
				{
					sortSettings.Add( fields[from] );
				}
				from++;
			}

			return sortSettings;
		}

		/// <summary>
		/// Parses the sort capabilities
		/// </summary>
		/// <param name="sortCaps"></param>
		/// <returns></returns>
		private static string[] GetSortFields(string sortCaps)
		{
			return sortCaps.Split(',');
		}

		protected override void SetTestInfo()
		{
			this._Name = "Browse SortCriteria";
			this._Description = "Finds the container with the most children calls BrowseDirectChildren on it with various SortCriteria strings.";
			this._ExpectedTestingTime = 900;

			this._Prerequisites.Add(this.PRE_BROWSEALL);
			this._Prerequisites.Add(this.PRE_SORTCAPS);
		}

		private Cds_BrowseAll PRE_BROWSEALL = new Cds_BrowseAll();
		private Cds_GetSortCapabilities PRE_SORTCAPS = new Cds_GetSortCapabilities();

		private static Tags T = Tags.GetInstance();

	}
}
