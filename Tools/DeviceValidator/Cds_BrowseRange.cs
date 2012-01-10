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
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnPValidator
{
	public class CdsResult_BrowseRange : CdsResult_BrowseStats
	{
		public CdsResult_BrowseAll BrowseAllResults;
	}

	/// <summary>
	/// Summary description for Cds_BrowseRange.
	/// </summary>
	public sealed class Cds_BrowseRange : Cds_BrowseTest
	{
		private CdsResult_BrowseRange _Details;
		public override object Details { get { return _Details; } }
		public override CdsResult_BrowseStats BrowseStats { get { return _Details; } }

		public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
		{
			// get the results from the BrowseAll test
			CdsResult_BrowseAll PRE = null;
			foreach (ISubTest preTest in otherSubTests)
			{
				if (preTest.Name == this.PRE_BROWSEALL.Name)
				{
					if (preTest.TestState >= UPnPTestStates.Pass)
					{
						PRE = preTest.Details as CdsResult_BrowseAll;
						break;
					}
				}
			}

			if (PRE == null)
			{
				return;
			}

			if (PRE.LargestContainer == null)
			{
				return;
			}

			if (PRE.Root == null)
			{
				return;
			}

			int totalBrowses = CalculateExpectedBrowseRequests(PRE.LargestContainer) + CalculateExpectedBrowseRequests(PRE.Root);

			int maxTime = 300 * totalBrowses;
			this._ExpectedTestingTime = maxTime;
		}

		public override UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg)
		{
			CpContentDirectory CDS = this.GetCDS(arg._Device);
			_Details = new CdsResult_BrowseRange();
			this._TestState = UPnPTestStates.Running;

			// get the results from the BrowseAll test
			CdsResult_BrowseAll PRE = null;
			try
			{
				foreach (ISubTest preTest in otherSubTests)
				{
					if (preTest.Name == this.PRE_BROWSEALL.Name)
					{
						PRE = preTest.Details as CdsResult_BrowseAll;
						break;
					}
				}

				if (PRE == null)
				{
					throw new TestException(this._Name + " requires that the \"" + this.PRE_BROWSEALL.Name + "\" test be run as a prerequisite. The results from that test cannot be obtained.", otherSubTests);
				}
			}
			catch (Exception e)
			{
				throw new TestException(this._Name + " requires that the \"" + this.PRE_BROWSEALL.Name + "\" test be run before. An error occurred when attempting to obtain the results of a prerequisite.", otherSubTests, e);
			}
			_Details.BrowseAllResults = PRE;

			if (PRE.LargestContainer == null)
			{
				throw new TestException(this.PRE_BROWSEALL.Name + " failed to find the container with the most child objects. " +this._Name+ " requires this value.", PRE);
			}

			if (PRE.Root == null)
			{
				throw new TestException(this.PRE_BROWSEALL.Name + " failed to find the root container. " +this._Name+ " requires this value.", PRE);
			}

			//calculate expected test time
			//int maxC = PRE.LargestContainer.ChildCount + 1;
			//int rootC = PRE.Root.ChildCount + 1;
			//_Details.ExpectedTotalBrowseRequests = (maxC * maxC) + (rootC * rootC);
			_Details.ExpectedTotalBrowseRequests = CalculateExpectedBrowseRequests(PRE.LargestContainer) + CalculateExpectedBrowseRequests(PRE.Root);
			int maxTime = 300 * _Details.ExpectedTotalBrowseRequests;
			this._ExpectedTestingTime = maxTime;
			arg.ActiveTests.UpdateTimeAndProgress(0);


			// test the root container
			arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\"" + this.Name + "\" started testing root container.");
			CdsBrowseSearchResults rootResults = TestContainerRanges(PRE.Root, this, arg, CDS, _Details);
			arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\"" + this.Name + "\" finished testing root container.");

			UPnPTestStates state = this._TestState;

			if (state < rootResults.WorstError)
			{
				state = rootResults.WorstError;
			}

			//test largest container
			arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\"" + this.Name + "\" started testing containerID=\"" + PRE.LargestContainer.ID + "\".");
			CdsBrowseSearchResults cResults = TestContainerRanges(PRE.LargestContainer, this, arg, CDS, _Details);
			arg.TestGroup.AddEvent(LogImportance.Remark, this.Name, "\"" + this.Name + "\" finished testing containerID=\"" + PRE.LargestContainer.ID + "\".");

			if (state < cResults.WorstError)
			{
				state = cResults.WorstError;
			}


			// finish up logging
			this._TestState = state;

			if (this._TestState >= UPnPTestStates.Warn)
			{
				arg._TestGroup.AddEvent(LogImportance.High, this.Name, "\"" + this.Name + "\" expects all Browse requests to succeed. A CDS should not return errors caused by [(StartingIndex + RequestedCount) >= container.ChildCount] because control points cannot assume a particular range. For leniency, the test will pass with warnings for CDS implementations that return an error when (StartingIndex >= container.ChildCount).");
			}

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

			return this._TestState;
		}

		private static int LIMIT = 3;
		public static int CalculateExpectedBrowseRequests(IMediaContainer c)
		{
			int count = 0;
			for (uint start = 0; start < c.ChildCount+1; start++)
			{
				for (uint requested = 0; requested < c.ChildCount+1; requested++)
				{
					uint start_requested = start+requested;
					bool doBrowse = false;
					int tenthCount = c.ChildCount / 10;
					int quarterCount = c.ChildCount / 4;
					if (quarterCount == 0)
					{
						quarterCount = 1;
					}
					if (tenthCount == 0)
					{
						tenthCount = 1;
					}

					if (start < LIMIT)
					{
						if (
							(requested == 0) ||
							(start_requested > c.ChildCount - LIMIT)
							)
						{
							doBrowse = true;
						}

					}
					else if (start >= c.ChildCount - LIMIT)
					{
						if (start_requested < c.ChildCount+LIMIT)
						{
							doBrowse = true;
						}
					}
					else if ((start % quarterCount == 0) && (requested % tenthCount == 0))
					{
						doBrowse = true;
					}

					if (doBrowse)
					{
						count++;
					}
				}
			}

			return count;
		}

		/// <summary>
		/// Tests the container with BrowseDirectChildren and different ranges.
		/// </summary>
		/// <param name="c"></param>
		/// <param name="test"></param>
		/// <param name="arg"></param>
		/// <param name="cds"></param>
		/// <param name="stats"></param>
		/// <returns></returns>
		public static CdsBrowseSearchResults TestContainerRanges(IMediaContainer c, CdsSubTest test, CdsSubTestArgument arg, CpContentDirectory cds, CdsResult_BrowseStats stats)
		{
			CdsBrowseSearchResults r = new CdsBrowseSearchResults();

			try
			{
				BrowseInput input = new BrowseInput();
				input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN;
				input.Filter = "*";
				input.ObjectID = c.ID;
				input.SortCriteria = "";

				input.StartingIndex = 0;
				input.RequestedCount = 0;

				if (c.CompleteList.Count != c.ChildCount)
				{
					throw new TestException("\""+test.Name+"\" has c.CompleteList.Count=" +c.CompleteList.Count+ " but c.ChildCount=" +c.ChildCount+ ".", c);
				}

				//assume the test will pass, but worsen the result when we find errors
				r.SetError(UPnPTestStates.Pass);

				// test starting index = 0 to c.ChildCount+1
				// test requested count = 0 to c.ChildCoun+1
				for (uint start = 0; start < c.ChildCount+1; start++)
				{
					for (uint requested = 0; requested < c.ChildCount+1; requested++)
					{
						uint start_requested = start+requested;
						bool doBrowse = false;
						int tenthCount = c.ChildCount / 10;
						int quarterCount = c.ChildCount / 4;
						if (quarterCount == 0)
						{
							quarterCount = 1;
						}
						if (tenthCount == 0)
						{
							tenthCount = 1;
						}

						if (start < LIMIT)
						{
							if (
								(requested == 0) ||
								(start_requested > c.ChildCount - LIMIT)
								)
							{
								doBrowse = true;
							}

						}
						else if (start >= c.ChildCount - LIMIT)
						{
							if (start_requested < c.ChildCount+LIMIT)
							{
								doBrowse = true;
							}
						}
						else if ((start % quarterCount == 0) && (requested % tenthCount == 0))
						{
							doBrowse = true;
						}


						if (doBrowse == false)
						{
							//stats.TotalBrowseRequests++;
							//arg.ActiveTests.UpdateTimeAndProgress(stats.TotalBrowseRequests * 300);
						}
						else
						{
							arg.ActiveTests.UpdateTimeAndProgress(stats.TotalBrowseRequests * 300);

							input.StartingIndex = start;
							input.RequestedCount = requested;
							CdsBrowseSearchResults br;
						
							br = Cds_BrowseAll.Browse(input, test, arg, cds, stats);

							if (br.WorstError >= UPnPTestStates.Failed)
							{
								if (
									(input.StartingIndex < c.ChildCount) 
									)
								{
									throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " +input.PrintBrowseParams()+ " returned with an error or had problems with the DIDL-Lite.");
								}
								else
								{
									arg._TestGroup.AddEvent(LogImportance.High, test.Name, "\"" + test.Name + "\": Warning: " +input.PrintBrowseParams()+ " returned an error.");
									r.SetError(UPnPTestStates.Warn);
								}
							}

							// check return values

							if (br.NumberReturned != br.MediaObjects.Count)
							{
								throw new TerminateEarly("\""+test.Name+"\" did a "+ input.PrintBrowseParams() + " and the number of media objects instantiated from the DIDL-Lite (instantiated=" +br.MediaObjects.Count+ ") does not match NumberReturned=" +br.NumberReturned+".");
							}

							long expectedReturned;
						
							if (input.StartingIndex == 0)
							{
								if (input.RequestedCount == 0)
								{
									expectedReturned = c.ChildCount;
								}
								else if (input.RequestedCount > c.ChildCount)
								{
									expectedReturned = c.ChildCount;
								}
								else if (input.RequestedCount <= c.ChildCount)
								{
									expectedReturned = input.RequestedCount;
								}
								else
								{
									throw new TestException("\""+test.Name+"\" should not reach here.", null);
								}
							}
							else
							{
								if (input.RequestedCount == 0)
								{
									expectedReturned = c.ChildCount - input.StartingIndex;
								}
								else
								{
									expectedReturned = c.ChildCount - input.StartingIndex;

									if (expectedReturned > input.RequestedCount)
									{
										expectedReturned = input.RequestedCount;
									}
								}
							}
						
							if ((expectedReturned < 0) || (expectedReturned > c.ChildCount))
							{
								throw new TestException("\""+test.Name+"\" did a " + input.PrintBrowseParams() + " and the expected number of media objects is invalid=" + expectedReturned + ".", br);
							}

							if (br.NumberReturned != expectedReturned)
							{
								throw new TerminateEarly("\""+test.Name+"\" did a "+ input.PrintBrowseParams() + " and NumberReturned=" +br.NumberReturned+ " but test expects " +expectedReturned+ " child objects according to results from a prerequisite test.");
							}

							if (br.TotalMatches != c.ChildCount)
							{
								throw new TerminateEarly("\""+test.Name+"\" did a "+ input.PrintBrowseParams() + " and TotalMatches=" +br.TotalMatches+ " but test found " +c.ChildCount+ " child objects in a prerequisite test.");
							}

							uint cUpdateID = 0;
						
							try
							{
								cUpdateID = (uint) c.Tag;
							}
							catch (Exception ce)
							{
								throw new TestException("\""+test.Name+"\" could not cast c.Tag into a uint value", null, ce);
							}

							if (br.UpdateID != cUpdateID)
							{
								throw new TerminateEarly("\""+test.Name+"\" did a "+ input.PrintBrowseParams() + " and UpdateID=" +br.UpdateID+ " but test expected=" +cUpdateID+ " as found in a prerequisite test.");
							}

							arg.TestGroup.AddEvent(LogImportance.Remark, test.Name, "\""+test.Name+"\" did a " +input.PrintBrowseParams()+ " and encountered no errors in the results.");
							arg.ActiveTests.UpdateTimeAndProgress(stats.TotalBrowseRequests * 300);
						}
					}
				}
			}
			catch (TerminateEarly te)
			{
				arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, test.Name + " is terminating early because of the following error: " + te.Message);
				r.SetError(UPnPTestStates.Failed);
				return r;
			}

			if (r.WorstError > UPnPTestStates.Warn)
			{
				throw new TestException("\"" + test.Name + "\" should not reach this code if the result is worse than " + UPnPTestStates.Warn.ToString() + ".", null);
			}

			return r;
		}

		protected override void SetTestInfo()
		{
			this._Name = "Browse Range";
			this._Description = "Finds the container with the most child objects in the \"" + PRE_BROWSEALL.Name + "\" test and tests BrowseDirectChildren with various ranges. Also applies the same tests to the root container of the CDS.";
			this._ExpectedTestingTime = 30;

			this._Prerequisites.Add(this.PRE_BROWSEALL);
		}

		private Cds_BrowseAll PRE_BROWSEALL = new Cds_BrowseAll();
	}
}
