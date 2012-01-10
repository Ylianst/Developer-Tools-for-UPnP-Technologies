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
    public class CdsResult_BrowseFilter : CdsResult_BrowseStats
    {
        public CdsResult_BrowseAll BrowseAllResults;
        public ArrayList Filters;
        internal int NumberOfProperties;
    }

    /// <summary>
    /// Summary description for Cds_BrowseFilter.
    /// </summary>
    public sealed class Cds_BrowseFilter : Cds_BrowseTest
    {
        private CdsResult_BrowseFilter _Details;
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

            if (PRE.MostMetadata == null)
            {
                return;
            }

            if (PRE.MostMetadata.Properties.Count != PRE.MostMetadata.Properties.PropertyNames.Count)
            {
                return;
            }

            IUPnPMedia MM = PRE.MostMetadata;

            if (MM == null)
            {
                return;
            }

            IMediaContainer MC = PRE.MostMetadata.Parent;

            if (MC == null)
            {
                return;
            }

            int numProps = MM.Properties.Count;
            int numChildren = MC.ChildCount;

            // we browse for "res" and each possible res@attribute
            numProps += (2 * MediaResource.GetPossibleAttributes().Count) + 2;

            int expectedBrowses = 0;
            int inc = numProps / 4;
            if (inc == 0)
            {
                inc = 1;
            }
            for (int nProps = 0; nProps < numProps; nProps++)
            {
                for (int iProp = 0; iProp < numProps; iProp += inc)
                {
                    expectedBrowses++;
                }
            }
            expectedBrowses *= 2;
            int maxTime = 90 * expectedBrowses;
            this._ExpectedTestingTime = maxTime;
        }

        public override UPnPTestStates Run(ICollection otherSubTests, CdsSubTestArgument arg)
        {
            CpContentDirectory CDS = this.GetCDS(arg._Device);
            _Details = new CdsResult_BrowseFilter();
            this._TestState = UPnPTestStates.Running;
            arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, "\"" + this.Name + "\" started.");

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

            if (PRE.MostMetadata == null)
            {
                throw new TestException(this.PRE_BROWSEALL.Name + " failed to find a media object with the most metadata. " + this._Name + " requires this value.", PRE);
            }

            if (PRE.MostMetadata.Properties.Count != PRE.MostMetadata.Properties.PropertyNames.Count)
            {
                throw new TestException(this.Name + " has conflicting reports for the number of metadata properties. " + PRE.MostMetadata.Properties.Count + "/" + PRE.MostMetadata.Properties.PropertyNames.Count, PRE.MostMetadata.Properties);
            }

            IUPnPMedia MM = PRE.MostMetadata;

            if (MM == null)
            {
                string skippedMsg = "\"" + this.Name + "\" skipped because the tested content hierarchy does not have a media object with at least one resource and has an ID!=\"0\"";
                arg.TestGroup.AddEvent(LogImportance.Critical, this.Name, skippedMsg);
                arg.TestGroup.AddResult(skippedMsg);
                return UPnPTestStates.Ready;
            }

            IMediaContainer MC = PRE.MostMetadata.Parent;

            if (MC == null)
            {
                throw new TestException(this.Name + " has MostMetadata.Parent == null", PRE.MostMetadata);
            }

            int numProps = MM.Properties.Count;
            int numChildren = MC.ChildCount;

            // we browse for "res" and each possible res@attribute
            numProps += (2 * MediaResource.GetPossibleAttributes().Count) + 2;

            _Details.NumberOfProperties = numProps;
            _Details.ExpectedTotalBrowseRequests = 0;
            int inc = numProps / 4;
            if (inc == 0)
            {
                inc = 1;
            }
            for (int nProps = 0; nProps < numProps; nProps++)
            {
                for (int iProp = 0; iProp < numProps; iProp += inc)
                {
                    _Details.ExpectedTotalBrowseRequests++;
                }
            }
            _Details.ExpectedTotalBrowseRequests *= 2;
            int maxTime = 90 * _Details.ExpectedTotalBrowseRequests;
            this._ExpectedTestingTime = maxTime;

            // browse metadata with various filter settings
            // browsedirectchildren with various filter settings

            UPnPTestStates state = this._TestState;

            CdsBrowseSearchResults test1 = TestFiltersBrowseMetadata(MM, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA, this, arg, CDS, _Details);

            if (test1.WorstError > state)
            {
                state = test1.WorstError;
            }

            CdsBrowseSearchResults test2 = TestFiltersBrowseMetadata(MM, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN, this, arg, CDS, _Details);

            if (test2.WorstError > state)
            {
                state = test2.WorstError;
            }

            // finish up logging
            this._TestState = state;

            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("\"{0}\" completed", this.Name);

            if (this._TestState <= UPnPTestStates.Running)
            {
                throw new TestException("\"" + this.Name + "\" must have a pass/warn/fail result.", this._TestState);
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
                    throw new TestException("TotalBrowseRequests=" + _Details.TotalBrowseRequests.ToString() + " ExpectedTotal=" + _Details.ExpectedTotalBrowseRequests.ToString(), _Details);
                }
            }

            arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, sb.ToString());

            return this._TestState;
        }

        /// <summary>
        /// Tests filters for metadata.
        /// </summary>
        /// <param name="testThis"></param>
        /// <param name="test"></param>
        /// <param name="arg"></param>
        /// <param name="cds"></param>
        /// <param name="stats"></param>
        /// <returns></returns>
        public static CdsBrowseSearchResults TestFiltersBrowseMetadata(IUPnPMedia testThis, CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag browseFlag, Cds_BrowseFilter test, CdsSubTestArgument arg, CpContentDirectory cds, CdsResult_BrowseFilter stats)
        {
            CdsBrowseSearchResults r = new CdsBrowseSearchResults();

            try
            {
                BrowseInput input = new BrowseInput();
                input.BrowseFlag = browseFlag;
                if (input.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA)
                {
                    input.ObjectID = testThis.ID;
                }
                else
                {
                    input.ObjectID = testThis.ParentID;
                }
                input.SortCriteria = "";

                input.StartingIndex = 0;
                input.RequestedCount = 0;

                //assume the test will pass, but worsen the result when we find errors
                r.SetError(UPnPTestStates.Pass);

                ArrayList propNames = new ArrayList((ICollection)testThis.Properties.PropertyNames);

                // add property names related to resources
                propNames.Add(T[_DIDL.Res]);
                foreach (string str in MediaResource.GetPossibleAttributes())
                {
                    propNames.Add(T[_DIDL.Res] + "@" + str);
                    propNames.Add("@" + str);
                }
                propNames.Add(T[_DIDL.Desc]);

                if (propNames.Count != stats.NumberOfProperties)
                {
                    throw new TestException("Number of calculated metadata properties (" + propNames.Count + ") doesn't match number of actual metadata properties (" + propNames.Count + ") for testing.", stats);
                }

                test._Details.Filters = propNames;

                int inc = propNames.Count / 4;
                if (inc == 0)
                {
                    inc = 1;
                }

                for (int numProps = 0; numProps < propNames.Count; numProps++)
                {
                    for (int iProp = 0; iProp < propNames.Count; iProp += inc)
                    {
                        IList filterSettings = GetFilterSettings(propNames, numProps, iProp, iProp);

                        input.Filter = GetCSVString(filterSettings);

                        CdsBrowseSearchResults br;

                        arg.ActiveTests.UpdateTimeAndProgress(90 * stats.TotalBrowseRequests);

                        br = Cds_BrowseAll.Browse(input, test, arg, cds, stats);

                        if (br.WorstError >= UPnPTestStates.Failed)
                        {
                            StringBuilder teMsg = new StringBuilder();

                            teMsg.AppendFormat("\"{0}\" is terminating early because {1} returned with an error or had problems with the DIDL-Lite.", test.Name, input.PrintBrowseParams());
                            if (br.Result != "")
                            {
                                teMsg.AppendFormat(" Returned DIDL=\"{0}\".", br.Result);
                            }
                            throw new TerminateEarly(teMsg.ToString());
                        }
                        //							if (br.InvokeError != null)
                        //							{
                        //								throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " +input.PrintBrowseParams()+ " returned with an error" + br.InvokeError.Message, br.InvokeError);
                        //							}

                        if (input.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA)
                        {
                            if (br.MediaObjects.Count != 1)
                            {
                                throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " did not return with exactly one media object in its DIDL-Lite response. DIDL-Lite => " + br.Result);
                            }

                            IUPnPMedia mo = (IUPnPMedia)br.MediaObjects[0];

                            CheckReturnedMetadata(mo, testThis, input, filterSettings, br, test);
                            arg.TestGroup.AddEvent(LogImportance.Remark, test.Name, "\"" + test.Name + "\" did a " + input.PrintBrowseParams() + " and encountered no errors in the results.");
                        }
                        else
                        {
                            IList childList = testThis.Parent.CompleteList;

                            foreach (IUPnPMedia gotChild in br.MediaObjects)
                            {

                                IUPnPMedia testAgainstChild = null;
                                foreach (IUPnPMedia child in childList)
                                {
                                    if (child.ID == gotChild.ID)
                                    {
                                        testAgainstChild = child;
                                        break;
                                    }
                                }
                                if (testAgainstChild == null)
                                {
                                    throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned media object with ID=\"" + gotChild.ID + "\", which was not a child object of containerID=\"" + input.ObjectID + "\" during a prerequisite test.");
                                }
                                CheckReturnedMetadata(gotChild, testAgainstChild, input, filterSettings, br, test);
                                arg.TestGroup.AddEvent(LogImportance.Remark, test.Name, "\"" + test.Name + "\" did a " + input.PrintBrowseParams() + " and encountered no errors in the results.");
                            }
                        }
                    }
                }

            }
            catch (TerminateEarly te)
            {
                string reason = "\"" + test.Name + "\" terminating early. Reason => " + te.Message;
                arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, reason);

                r.SetError(UPnPTestStates.Failed);
                return r;
            }

            if (r.WorstError > UPnPTestStates.Warn)
            {
                throw new TestException("\"" + test.Name + "\" should not reach this code if the result is worse than " + UPnPTestStates.Warn.ToString() + ".", null);
            }

            return r;
        }

        private static void CheckReturnedMetadata(IUPnPMedia mo, IUPnPMedia testThis, BrowseInput input, IList filterSettings, CdsBrowseSearchResults br, CdsSubTest test)
        {
            // check non-resource metadata
            string filters = input.Filter;

            foreach (string moPropName in mo.Properties.PropertyNames)
            {
                if (filterSettings.Contains(moPropName) == false)
                {
                    if (moPropName == T[_DC.title])
                    {
                    }
                    else if (moPropName == T[_UPNP.Class])
                    {
                    }
                    else
                    {
                        throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " included a \"" + moPropName + "\" metadata property in its DIDL-Lite response. DIDL-Lite ==> " + br.Result);
                    }
                }
            }

            // check resource metadata
            IList resources = mo.Resources;
            int expectedResCount = 0;
            foreach (IMediaResource res in testThis.Resources)
            {
                foreach (string filter in filterSettings)
                {
                    if ((filter.StartsWith(T[_DIDL.Res])) || (filter.StartsWith("@")))
                    {
                        if (res.ValidAttributes.Contains(filter.Substring(filter.IndexOf("@") + 1)))
                        {
                            expectedResCount++;
                            break;
                        }
                    }
                }
            }

            if (resources.Count < expectedResCount)
            {
                throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned a media object (ID='" + mo.ID + "') with no resources DIDL-Lite when it should have returned at least " + expectedResCount + " " + T[_DIDL.Res] + " objects. DIDL-Lite ==> " + br.Result);
            }
            else
            {
                int iRes = 0;
                foreach (IMediaResource res in mo.Resources)
                {
                    iRes++;
                    if (res.ValidAttributes.Contains(T[_RESATTRIB.protocolInfo]))
                    {
                        foreach (string attrib in res.ValidAttributes)
                        {
                            if (attrib == T[_RESATTRIB.protocolInfo])
                            {
                            }
                            else if ((filterSettings.Contains("@" + attrib)) || (filterSettings.Contains(T[_DIDL.Res] + "@" + attrib)))
                            {
                            }
                            else
                            {
                                throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned resource #" + iRes + " with the \"" + attrib + "\" attribute. DIDL-Lite ==> " + br.Result);
                            }
                        }
                    }
                    else
                    {
                        throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned resource #" + iRes + " without a \"protocolInfo\" attribute. DIDL-Lite ==> " + br.Result);
                    }
                }
            }

            // check desc nodes
            if (filterSettings.Contains(T[_DIDL.Desc]))
            {
                if (mo.DescNodes.Count != testThis.DescNodes.Count)
                {
                    throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned " + mo.DescNodes.Count + " " + T[_DIDL.Desc] + " nodes when it expected to find " + testThis.DescNodes.Count + " such nodes, as found in a prerequisite test.");
                }
            }
            else
            {
                if (mo.DescNodes.Count > 0)
                {
                    throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned " + mo.DescNodes.Count + " " + T[_DIDL.Desc] + " nodes when it expected to find none.");
                }
            }

            int found = 0;
            foreach (string f in filterSettings)
            {
                foreach (string p in mo.Properties.PropertyNames)
                {
                    if (p == f)
                    {
                        found++;
                    }
                }
            }
            bool allStandardFound = false;
            if (found == testThis.Properties.Count)
            {
                allStandardFound = true;
            }

            if (allStandardFound)
            {
                if (mo.Properties.Count != testThis.Properties.Count)
                {
                    throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " + input.PrintBrowseParams() + " returned " + mo.Properties.Count + " metadata non-resource & non-vendor-specific metadata properties when it expected to find " + testThis.Properties.Count + " such metadata properties, as found in a prerequisite test.");
                }
            }
        }

        private static IList GetFilterSettings(IList propertyNames, int numProps, int iProp, int sProp)
        {
            ArrayList filterSettings = new ArrayList();
            filterSettings.Add(propertyNames[iProp]);

            IList resourceProperties = MediaResource.GetPossibleAttributes();
            string didl_res = T[_DIDL.Res] + "@";
            string _res = "@";
            ArrayList addedResProperties = new ArrayList();

            for (int i = 0; i < numProps; i++)
            {
                if (sProp >= propertyNames.Count)
                {
                    sProp = sProp - propertyNames.Count;
                }

                if (sProp != iProp)
                {
                    bool addOK = true;
                    string propName = (string)propertyNames[sProp];

                    if (propName.StartsWith(didl_res))
                    {
                        string resAttrib = propName.Remove(0, didl_res.Length);
                        if (resourceProperties.Contains(resAttrib))
                        {
                            if (addedResProperties.Contains(resAttrib))
                            {
                                addOK = false;
                            }
                            else
                            {
                                addedResProperties.Add(resAttrib);
                            }
                        }
                    }
                    else if (propName.StartsWith(_res))
                    {
                        string resAttrib = propName.Remove(0, _res.Length);
                        if (resourceProperties.Contains(resAttrib))
                        {
                            if (addedResProperties.Contains(resAttrib))
                            {
                                addOK = false;
                            }
                            else
                            {
                                addedResProperties.Add(resAttrib);
                            }
                        }
                    }

                    if (addOK)
                    {
                        filterSettings.Add(propertyNames[sProp]);
                    }
                }

                sProp++;
            }

            return filterSettings;
        }


        /// <summary>
        /// Returns a list of metadata property names that should not appear, given the filtering settings.
        /// </summary>
        /// <param name="c"></param>
        /// <param name="test"></param>
        /// <param name="arg"></param>
        /// <param name="cds"></param>
        /// <param name="stats"></param>
        /// <returns>a list of violating property names</returns>
        public static IList TestFiltering(IUPnPMedia checkThis, ArrayList filterSettings)
        {
            ArrayList v = new ArrayList();

            foreach (string propertyName in checkThis.Properties.PropertyNames)
            {
                // if the property in the media object is not in the list, then it is a violation
                if (filterSettings.Contains(propertyName) == false)
                {
                    v.Add(propertyName);
                }
            }

            return v;
        }

        protected override void SetTestInfo()
        {
            this._Name = "Browse Filter";
            this._Description = "Finds the media object with the most metadata and does a BrowseMetadata on it, and a BrowseDirectChildren on its parent. Both calls use various feature filter settings. Requires a media object (item or container) that is not the root container and also has at least one resource element.";
            this._ExpectedTestingTime = 90;

            this._Prerequisites.Add(this.PRE_BROWSEALL);
        }

        private Cds_BrowseAll PRE_BROWSEALL = new Cds_BrowseAll();

        private static Tags T = Tags.GetInstance();
    }
}
