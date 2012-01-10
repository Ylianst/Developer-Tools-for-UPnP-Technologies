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
using System.Xml;
using System.Text;
using System.Collections;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnPValidator
{

	public class CdsResult_BrowseStats : CdsTestResult
	{
		public int ExpectedTotalBrowseRequests = 0;
		public int TotalBrowseRequests = 0;
		public int TotalContainers = 1;
		public int TotalItems = 0;
	}

	public class CdsResult_BrowseAll : CdsResult_BrowseStats
	{
		public IMediaContainer Root;
		public ArrayList AllObjects = new ArrayList();

		public IMediaContainer LargestContainer;
		public IUPnPMedia MostMetadata;
		public ArrayList PropertyNames = new ArrayList();
	}

	/// <summary>
	/// Declares a bunch of inner classes useful for browse tests.
	/// </summary>
	public abstract class Cds_BrowseTest : CdsSubTest
	{
		public class BrowseInput : InputParams
		{
			public System.String ObjectID;
			public CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag;
			public System.String Filter;
			public System.UInt32 StartingIndex;
			public System.UInt32 RequestedCount;
			public System.String SortCriteria;
			public string PrintBrowseParams()
			{
				BrowseInput bi = this;
				StringBuilder sb = new StringBuilder();

				sb.AppendFormat("Browse(\"{0}\", {1}, \"{2}\", {3}, {4}, \"{5}\")", bi.ObjectID, bi.BrowseFlag.ToString(), bi.Filter, bi.StartingIndex, bi.RequestedCount, bi.SortCriteria);

				return sb.ToString();
			}
		}


		public abstract CdsResult_BrowseStats BrowseStats { get; }


		public static string GetCSVString(IList values)
		{
			StringBuilder sb = new StringBuilder();
			int sbi = 0;
			ArrayList seenAlready = new ArrayList();
			foreach (string val in values)
			{
				if (sbi > 0)
				{
					sb.Append(",");
				}

				sb.Append(val);
				sbi++;
			}
			return sb.ToString();
		}


		/// <summary>
		/// Performs a Browse invocation.
		/// </summary>
		/// <param name="input"></param>
		/// <param name="test"></param>
		/// <param name="arg"></param>
		/// <param name="cds"></param>
		/// <param name="stats"></param>
		/// <returns></returns>
		public static CdsBrowseSearchResults Browse(BrowseInput input, CdsSubTest test, CdsSubTestArgument arg, CpContentDirectory cds, CdsResult_BrowseStats stats)
		{

			CdsBrowseSearchResults results = new CdsBrowseSearchResults();
			results.SetError(UPnPTestStates.Pass);
			results.ResultErrors = new ArrayList();
			
			arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, "\""+test.Name+"\" about to do " + input.PrintBrowseParams()+ ".");
			try
			{
				cds.Sync_Browse(input.ObjectID, input.BrowseFlag, input.Filter, input.StartingIndex, input.RequestedCount, input.SortCriteria, out results.Result, out results.NumberReturned, out results.TotalMatches, out results.UpdateID);
			}
			catch (UPnPInvokeException error)
			{
				results.InvokeError = error;
			}

			if (results.InvokeError == null)
			{
				arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, "\""+test.Name+"\" completed " + input.PrintBrowseParams()+ " with no errors returned by the device.");
			}
			else
			{
				arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, "\""+test.Name+"\" completed " + input.PrintBrowseParams()+ " with the device returning an error.");
			}

			stats.TotalBrowseRequests++;
			ArrayList branches = null;
			if (results.InvokeError == null)
			{
				try
				{
					if (results.Result != null)
					{
						if (results.Result != "")
						{
							bool schemaOK = CheckDidlLiteSchema(results.Result);

							if (schemaOK)
							{
								results.MediaObjects = branches = MediaBuilder.BuildMediaBranches(results.Result, typeof (MediaItem), typeof(MediaContainer), true);

								if (branches.Count != results.NumberReturned)
								{
									results.ResultErrors.Add (new CdsException(input.PrintBrowseParams() + " has the \"Result\" argument indicating the presence of " +branches.Count+ " media objects but the request returned NumberReturned=" +results.NumberReturned+"."));
								}

								if (input.BrowseFlag == CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA)
								{
									if (branches.Count != 1)
									{
										results.ResultErrors.Add (new CdsException(input.PrintBrowseParams() + " has the \"Result\" argument indicating the presence of " +branches.Count+ " media objects but the request should have only returned 1 media object."));
									}
								}

								foreach (IUPnPMedia mobj in branches)
								{
									IMediaContainer imc = mobj as IMediaContainer;

									if (imc != null)
									{
										if (imc.CompleteList.Count > 0)
										{
											StringBuilder offendingList = new StringBuilder();
											int offenses = 0;
											foreach (IUPnPMedia offending in imc.CompleteList)
											{
												if (offenses > 0)
												{
													offendingList.Append(",");
												}
												offendingList.AppendFormat("\"{0}\"", offending.ID);
												offenses++;
											}
											results.ResultErrors.Add (new CdsException(input.PrintBrowseParams() + " has the \"Result\" argument with a declared container (ID=\""+imc.ID+"\") element that also includes its immediate children. Illegally declared media objects in the response are: "+offendingList.ToString()));
										}
									}
								}
							}
						}
					}
				}
				catch (Exception error2)
				{
					results.ResultErrors.Add (error2);
					if (results.MediaObjects == null)
					{
						results.MediaObjects = new ArrayList();
					}
				}
			}

			// log any errors
			if ((results.InvokeError != null) || (results.ResultErrors.Count > 0))
			{
				LogErrors(arg._TestGroup, test, input, "Browse", results.InvokeError, results.ResultErrors);
				results.SetError(UPnPTestStates.Failed);
			}

			return results;
		}

		public static bool CheckDidlLiteSchema (string DidlLiteXml)
		{
			// TODO: Add schema validation
			// TODO: Add check for blank title.
			// TODO: Add check for upnp:class <==> <container/item> declarator
			return true;
		}
	}

	/// <summary>
	/// Summary description for Cds_BrowseAll.
	/// </summary>
	public sealed class Cds_BrowseAll : Cds_BrowseTest
	{
		private CdsResult_BrowseAll _Details;
		public override object Details { get { return _Details; } }
		public override CdsResult_BrowseStats BrowseStats { get { return _Details; } }

		public override void CalculateExpectedTestingTime(ICollection otherSubTests, ISubTestArgument arg)
		{
		}

		public override UPnPTestStates Run (ICollection otherSubTests, CdsSubTestArgument arg)
		{
			// init basic stuff
			CpContentDirectory CDS = this.GetCDS(arg._Device);
			_Details = new CdsResult_BrowseAll();

			// set up a queue of containers to browse, starting with root container
			Queue C = new Queue();
			_Details.Root = new MediaContainer();
			_Details.Root.ID = "0";
			_Details.AllObjects.Add(_Details.Root);
			_Details.TotalContainers = 1;
			_Details.TotalItems = 0;
			C.Enqueue(_Details.Root);

			// if we have containers to browse, do so
			this._TestState = UPnPTestStates.Running;
			UPnPTestStates testResult = UPnPTestStates.Ready;
			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, "\""+this.Name + "\" started.");
			while (C.Count > 0)
			{
				IMediaContainer c = (IMediaContainer) C.Dequeue();

				this._ExpectedTestingTime = _Details.ExpectedTotalBrowseRequests * 30;
				arg.ActiveTests.UpdateTimeAndProgress( _Details.TotalBrowseRequests * 30);

				//
				// get the container's metadata
				//

				IUPnPMedia metadata;
				CdsBrowseSearchResults results = GetContainerMetadataAndValidate(c.ID, CDS, this, arg, _Details, out metadata);

				testResult = results.WorstError;

				if (testResult > UPnPTestStates.Warn)
				{
					arg._TestGroup.AddEvent(LogImportance.Critical, this.Name, this.Name + " terminating because container metadata could not be obtained or the metadata was not CDS-compliant.");
					testResult = UPnPTestStates.Failed;
					this._TestState = testResult;
					return this._TestState;
				}

				if (metadata != null)
				{
					try
					{
						c.UpdateObject(metadata);
						c.Tag = results.UpdateID;
					}
					catch (Exception e)
					{
						UpdateObjectError uoe = new UpdateObjectError();
						uoe.UpdateThis = c;
						uoe.Metadata = metadata;
						throw new TestException("Critical error updating metadata of a container using UpdateObject()", uoe, e);
					}
				}
				else
				{
					string reason = "\"" +this.Name + "\" terminating because container metadata could not be cast into object form.";
					arg._TestGroup.AddEvent(LogImportance.Critical, this.Name, reason);
					arg._TestGroup.AddResult("\""+this.Name + "\" test failed. " + reason);
					testResult = UPnPTestStates.Failed;
					this._TestState = testResult;
					return this._TestState;
				}

				//
				// Now get the container's children
				//
				ArrayList children = new ArrayList();
				try
				{
					children = GetContainerChildrenAndValidate(c, CDS, this, arg, _Details, C);

					if ((_Details.LargestContainer == null) || (children.Count > _Details.LargestContainer.ChildCount))
					{
						_Details.LargestContainer = c;
					}
				}
				catch (TerminateEarly te)
				{
					string reason = "\"" +this.Name + "\" terminating early. Reason => " + te.Message;
					arg._TestGroup.AddEvent(LogImportance.Critical, this.Name, reason);
					arg._TestGroup.AddResult("\""+this.Name + "\" test failed. " + reason);
					testResult = UPnPTestStates.Failed;
					this._TestState = testResult;
					return this._TestState;
				}
			}
			
			if (testResult >= UPnPTestStates.Failed)
			{
				throw new TestException("Execution should not reach this code if testResult is WARN or worse.", testResult);
			}

			if (testResult == UPnPTestStates.Ready)
			{
				throw new TestException("We should not return Ready state.", testResult);
			}
			
			StringBuilder sb = new StringBuilder();
			sb.Append("\""+this._Name + "\" test finished");

			if (testResult == UPnPTestStates.Warn)
			{
				sb.Append(" with warnings");
			}

			sb.AppendFormat(" and found {0}/{1}/{2} TotalObjects/TotalContainers/TotalItems.", _Details.AllObjects.Count, _Details.TotalContainers, _Details.TotalItems);

			arg.TestGroup.AddResult(sb.ToString());
			arg._TestGroup.AddEvent(LogImportance.Remark, this.Name, this.Name + " completed.");

			this._TestState = testResult;

			if (this._TestState <= UPnPTestStates.Warn)
			{
				if (_Details.TotalBrowseRequests != _Details.ExpectedTotalBrowseRequests)
				{
					throw new TestException("TotalBrowseRequests="+_Details.TotalBrowseRequests.ToString()+" ExpectedTotal="+_Details.ExpectedTotalBrowseRequests.ToString(), _Details);
				}
			}
			return this._TestState;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="parent"></param>
		/// <param name="cds"></param>
		/// <param name="test"></param>
		/// <param name="arg"></param>
		/// <param name="details"></param>
		/// <param name="C"></param>
		/// <returns></returns>
		/// <exception cref="TerminateEarly">
		/// </exception>
		public static ArrayList GetContainerChildrenAndValidate(IMediaContainer parent, CpContentDirectory cds, CdsSubTest test, CdsSubTestArgument arg, CdsResult_BrowseAll details, Queue C)
		{
			uint totalExpected = uint.MaxValue;
			uint currentChild = 0;

			ArrayList children = new ArrayList();

			while (currentChild < totalExpected)
			{
				BrowseInput input = new BrowseInput();
				input.ObjectID = parent.ID;
				input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN;
				input.Filter = "*";
				input.StartingIndex = currentChild;
				input.RequestedCount = 1;
				input.SortCriteria = "";

				string containerID = parent.ID;

				if (currentChild == 0)
				{
					test.SetExpectedTestingTime ((++details.ExpectedTotalBrowseRequests) * 30);
					arg.ActiveTests.UpdateTimeAndProgress( details.TotalBrowseRequests * 30);
				}

				CdsBrowseSearchResults results = Browse(input, test, arg, cds, details);

				test.SetExpectedTestingTime ((details.ExpectedTotalBrowseRequests) * 30);
				arg.ActiveTests.UpdateTimeAndProgress( (details.TotalBrowseRequests) * 30);

				if (results.WorstError >= UPnPTestStates.Failed)
				{
					throw new TerminateEarly("\"" + test.Name + "\" is terminating early because " +input.PrintBrowseParams()+ " returned with an error or had problems with the DIDL-Lite.");
				}
				else
				{
					if (results.NumberReturned != 1)
					{
						if (currentChild != 0)
						{
							results.SetError(UPnPTestStates.Failed);
							arg._TestGroup.AddEvent(LogImportance.Low, test.Name, "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned NumberReturned=" +results.NumberReturned+ " when it should logically be 1.");
						}
					}

					if (results.TotalMatches != totalExpected)
					{
						if (currentChild != 0)
						{
							results.SetError(UPnPTestStates.Failed);
							arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned TotalMatches=" +results.TotalMatches+ " when it should logically be " +totalExpected+ " as reported in an earlier Browse request. This portion of the test requires that a MediaServer device not be in a state where its content hierarchy will change.");
						}
						else
						{
							totalExpected = results.TotalMatches;
							if (totalExpected > 0)
							{
								details.ExpectedTotalBrowseRequests += ((int)results.TotalMatches * 2) - 1;
								test.SetExpectedTestingTime ((details.ExpectedTotalBrowseRequests) * 30);
								arg.ActiveTests.UpdateTimeAndProgress( details.TotalBrowseRequests * 30);
							}
						}
					}

					if (results.MediaObjects != null)
					{
						if (results.MediaObjects.Count == 1)
						{
							IUPnPMedia child = results.MediaObjects[0] as IUPnPMedia;

							if (child == null)
							{
								throw new TestException("\"" + test.Name + "\"" + " has a TEST LOGIC ERROR. Browse returned without errors but the child object's metadata is not stored in an IUPnPMedia object. The offending type is " + results.MediaObjects[0].GetType().ToString(), results.MediaObjects[0]);
							}

							// ensure no duplicates in object ID
							foreach (IUPnPMedia previousChild in details.AllObjects)
							{
								if (previousChild.ID == child.ID)
								{
									string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned an object with ID=\"" +child.ID+ "\" which conflicts with a previously seen media object in ParentContainerID=\"" +previousChild.ParentID+ "\".";
									arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
									throw new TerminateEarly(msg);
								}
							}

							// ensure updateID is the same between BrowseDirectChildren and earlier BrowseMetadata.

							try
							{
								uint previousUpdateID = (uint) parent.Tag;
								if (results.UpdateID != previousUpdateID)
								{
									string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned an UpdateID=" +results.UpdateID+ " whilst an UpdateID=" +previousUpdateID+ " was obtained in a previous call for ContainerID=\"" +parent.ID+ "\" with BrowseMetadata.";
									arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
									throw new TerminateEarly(msg);
								}
							}
							catch (TerminateEarly te)
							{
								throw te;
							}
							catch (Exception e)
							{
								throw new TestException(test.Name + " has a TEST LOGIC ERROR. Error comparing UpdateID values", parent, e);
							}

							// add the child to lists: C, parent's child list, and Allobjects
							try
							{
								parent.AddObject(child, false);
							}
							catch (Exception e)
							{
								AddObjectError aoe = new AddObjectError();
								aoe.Parent = parent;
								aoe.Child = child;
								throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but the child object could not be added to its parent.", aoe, e);
							}

							details.AllObjects.Add(child);
							children.Add(child);
							if (child.IsContainer)
							{
								C.Enqueue(child);
								details.TotalContainers++;
							}
							else
							{
								details.TotalItems++;
							}

							//
							// Do a BrowseMetadata and check to see if the XML values are the same.
							//

							CdsBrowseSearchResults compareResults = CheckMetadata(child, cds, test, arg, details);

							if (compareResults.InvokeError != null)
							{
								arg._TestGroup.AddEvent(LogImportance.High, test.Name, test.Name + ": Browse(BrowseDirectChildren,StartingIndex="+currentChild+",RequestedCount=0) on ContainerID=["+containerID+"] succeeded with warnings because a BrowseMetadata request was rejected by the CDS.");
							}
							else if (compareResults.ResultErrors.Count > 0)
							{
								string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " failed because a BrowseMetadata request succeeded but the DIDL-Lite could not be represented in object form. Invalid DIDL-Lite is most likely the cause.";
								arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
								throw new TerminateEarly(msg);
							}
							else if (compareResults.WorstError >= UPnPTestStates.Failed)
							{
								string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " failed because one or more child object's failed a comparison of results between BrowseDirectChildren and BrowseMetadata or encountered some other critical error in that process.";
								arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
								throw new TerminateEarly(msg);
							}
							else
							{
								//string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " succeeded.";
								//arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, msg);
							}

							// 
							// Track the metadata properties found so far
							// as we may use them in Browse SortCriteria
							//

							// standard top-level attributes 
							Tags T = Tags.GetInstance();
							AddTo(details.PropertyNames, "@"+T[_ATTRIB.id]);
							AddTo(details.PropertyNames, "@"+T[_ATTRIB.parentID]);
							AddTo(details.PropertyNames, "@"+T[_ATTRIB.restricted]);
							if (child.IsContainer)
							{
								if (child.IsSearchable)
								{
									AddTo(details.PropertyNames, "@"+T[_ATTRIB.searchable]);
									AddTo(details.PropertyNames, T[_DIDL.Container]+"@"+T[_ATTRIB.searchable]);
								}
								
								AddTo(details.PropertyNames, T[_DIDL.Container]+"@"+T[_ATTRIB.id]);
								AddTo(details.PropertyNames, T[_DIDL.Container]+"@"+T[_ATTRIB.parentID]);
								AddTo(details.PropertyNames, T[_DIDL.Container]+"@"+T[_ATTRIB.restricted]);
							}
							else if (child.IsItem)
							{
								if (child.IsReference)
								{
									AddTo(details.PropertyNames, "@"+T[_ATTRIB.refID]);
									AddTo(details.PropertyNames, T[_DIDL.Item]+"@"+T[_ATTRIB.refID]);
								}
								
								AddTo(details.PropertyNames, T[_DIDL.Item]+"@"+T[_ATTRIB.id]);
								AddTo(details.PropertyNames, T[_DIDL.Item]+"@"+T[_ATTRIB.parentID]);
								AddTo(details.PropertyNames, T[_DIDL.Item]+"@"+T[_ATTRIB.restricted]);
							}

							// standard metadata 
							IMediaProperties properties = child.MergedProperties;
							IList propertyNames = properties.PropertyNames;
							foreach (string propertyName in propertyNames)
							{
								if (details.PropertyNames.Contains(propertyName) == false)
								{
									details.PropertyNames.Add(propertyName);

									// add attributes if they are not added
									IList propertyValues = properties[propertyName];
									foreach (ICdsElement val in propertyValues)
									{
										ICollection attributes = val.ValidAttributes;
										foreach (string attribName in attributes)
										{
											StringBuilder sbpn = new StringBuilder();
											sbpn.AppendFormat("{0}@{1}", propertyName, attribName);
											string fullAttribName = sbpn.ToString();
											AddTo(details.PropertyNames, fullAttribName);
										}
									}
								}

								// resources 
								IList resources = child.MergedResources;
								foreach (IMediaResource res in resources)
								{
									ICollection attributes = res.ValidAttributes;
									foreach (string attribName in attributes)
									{
										string name1 = "res@"+attribName;
										string name2 = "@"+attribName;

										AddTo(details.PropertyNames, name1);
										AddTo(details.PropertyNames, name2);
									}
								}

								if (resources.Count > 0)
								{
									AddTo(details.PropertyNames, T[_DIDL.Res]);
								}
							}
						}
						else
						{
							if (results.TotalMatches > 0)
							{
								results.SetError(UPnPTestStates.Failed);
								string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " did not yield exactly one CDS-compliant media object in its result. Instantiated a total of " +results.MediaObjects.Count+ " media objects.";
								arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
								throw new TerminateEarly(msg);
							}
						}
					}
					else
					{
						throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but no media objects were instantiated.", null);
					}
				}

				currentChild++;
			}

			return children;
		}

		private static void AddTo(ArrayList al, string val)
		{
			if (al.Contains(val) == false)
			{
				al.Add(val);
			}
		}

		public static CdsBrowseSearchResults CheckMetadata(IUPnPMedia checkAgainstThis, CpContentDirectory cds, CdsSubTest test, CdsSubTestArgument arg, CdsResult_BrowseAll details)
		{
			//
			// Save a reference to the media object with the most filterable properties
			int numProperties = 0;
			if (details.MostMetadata != null)
			{
				numProperties = details.MostMetadata.Properties.Count;

				if (details.MostMetadata.DescNodes.Count > 0)
				{
					numProperties ++;
				}
			}

			int checkValue = 0;
			
			if (checkAgainstThis.ID != "0")
			{
				if (checkAgainstThis.Resources.Length > 0)
				{
					checkValue = checkAgainstThis.Properties.Count;
					if (checkAgainstThis.DescNodes.Count > 0)
					{
						checkValue++;
					}
				}
			}

			if (checkValue > numProperties)
			{
				details.MostMetadata = checkAgainstThis;
			}

			// do a browsemetadata on the media object and determine if the metadata matche.

			BrowseInput input = new BrowseInput();
			input.ObjectID = checkAgainstThis.ID;
			input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA;
			input.Filter = "*";
			input.StartingIndex = 0;
			input.RequestedCount = 0;
			input.SortCriteria = "";
			IUPnPMedia metadata = null;

			CdsBrowseSearchResults results = Browse(input, test, arg, cds, details);

			test.SetExpectedTestingTime ((details.ExpectedTotalBrowseRequests) * 30);
			arg.ActiveTests.UpdateTimeAndProgress( (details.TotalBrowseRequests) * 30);

			if (results.WorstError <= UPnPTestStates.Warn)
			{
				if (results.NumberReturned != 1)
				{
					results.SetError(UPnPTestStates.Warn);
					string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned NumberReturned=" +results.NumberReturned+ " when it should logically be 1. This output parameter is not really useful for BrowseMetadata so this logic error does not prevent towards certification, but it should be fixed.";
					arg._TestGroup.AddEvent(LogImportance.Low, test.Name, msg);
				}

				if (results.TotalMatches != 1)
				{
					results.SetError(UPnPTestStates.Warn);
					string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned TotalMatches=" +results.TotalMatches+ " when it should logically be 1. This output parameter is not really useful BrowseMetadata so this logic error does not prevent towards certification, but it should be fixed.";
					arg._TestGroup.AddEvent(LogImportance.Low, test.Name, msg);
				}

				if (results.MediaObjects != null)
				{
					if (results.MediaObjects.Count == 1)
					{
						metadata = results.MediaObjects[0] as IUPnPMedia;

						if (metadata == null)
						{
							throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but the object's metadata is not stored in an IUPnPMedia object. The offending type is " + results.MediaObjects[0].GetType().ToString(), results.MediaObjects[0]);
						}

						if (metadata.ID != input.ObjectID)
						{
							results.SetError(UPnPTestStates.Failed);
							string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object with ID=\"" +metadata.ID+ "\" when it should be \"" +input.ObjectID+"\" as indicated by the input parameter.";
							arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
						}

						if (metadata.ParentID != checkAgainstThis.ParentID)
						{
							results.SetError(UPnPTestStates.Failed);
							string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object with parentID=\"" +metadata.ParentID+ "\" when it should be \"" +checkAgainstThis.ParentID+ "\".";
							arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
						}

						string original;
						string received;
						
						try
						{
							original = checkAgainstThis.ToDidl();
							received = metadata.ToDidl();
						}
						catch 
						{
							results.SetError(UPnPTestStates.Failed);
							string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " encountered errors with the DIDL-Lite.";
							arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
							return results;
						}

						if (string.Compare(original, received) == 0)
						{
							//string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " successfully returned a DIDL-Lite media object that succesfully matches with previously seen metadata.";
							//arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, msg);
						}
						else
						{
							System.Xml.XmlDocument doc1 = new XmlDocument();
							XmlDocument doc2 = new XmlDocument();
							doc1.LoadXml(original);
							doc2.LoadXml(received);

							bool isMatch = true;
							foreach (XmlElement el1 in doc1.GetElementsByTagName("*"))
							{
								bool foundElement = false;
								foreach (XmlElement el2 in doc2.GetElementsByTagName("*"))
								{
									if (
										(el1.Name != "item") &&
										(el1.Name != "container") &&
										(el1.OuterXml == el2.OuterXml)
										)
									{
										foundElement = true;
										break;
									}
									else if (
										(
										(el1.Name == "DIDL-Lite") ||
										(el1.Name == "item") ||
										(el1.Name == "container")
										) &&
										(el1.Name == el2.Name)
										)
									{
										foundElement = true;
										break;
									}
								}

								if (foundElement == false)
								{
									isMatch = false;
									break;
								}
							}

							if (isMatch==false)
							{
								results.SetError(UPnPTestStates.Failed);
								string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object that failed to match with previously seen metadata.";
								arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
								arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, test.Name + ": Original=\""+original+"\" Received=\""+received+"\"");
							}
						}
					}
					else
					{
						results.SetError(UPnPTestStates.Failed);
						string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " did not yield exactly one CDS-compliant media object in its result. Instantiated a total of " +results.MediaObjects.Count+ " media objects.";
						arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
					}
				}
				else
				{
					throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but no media objects were instantiated.", null);
				}
			}

			return results;
		}


		public static CdsBrowseSearchResults GetContainerMetadataAndValidate(string containerID, CpContentDirectory cds, CdsSubTest test, CdsSubTestArgument arg, CdsResult_BrowseAll details, out IUPnPMedia metadata)
		{
			BrowseInput input = new BrowseInput();
			input.ObjectID = containerID;
			input.BrowseFlag = CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEMETADATA;
			input.Filter = "*";
			input.StartingIndex = 0;
			input.RequestedCount = 0;
			input.SortCriteria = "";
			metadata = null;

			test.SetExpectedTestingTime ((++details.ExpectedTotalBrowseRequests) * 30);
			arg.ActiveTests.UpdateTimeAndProgress( details.TotalBrowseRequests * 30);

			CdsBrowseSearchResults results = Browse(input, test, arg, cds, details);

			test.SetExpectedTestingTime ((details.ExpectedTotalBrowseRequests) * 30);
			arg.ActiveTests.UpdateTimeAndProgress( (details.TotalBrowseRequests) * 30);

			if (results.InvokeError == null)
			{
				if (results.NumberReturned != 1)
				{
					//results.SetError(UPnPTestStates.Warn);
					string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned NumberReturned=" +results.NumberReturned+ " when it should logically be 1. This output parameter is not really useful for BrowseMetadata so this logic error does not prevent towards certification, but it should be fixed.";
					//arg._TestGroup.AddEvent(LogImportance.Low, test.Name, msg);
					results.ResultErrors.Add( new Exception(msg) );
				}

				if (results.TotalMatches != 1)
				{
					//results.SetError(UPnPTestStates.Warn);
					string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned TotalMatches=" +results.TotalMatches+ " when it should logically be 1. This output parameter is not really useful BrowseMetadata so this logic error does not prevent towards certification, but it should be fixed.";
					//arg._TestGroup.AddEvent(LogImportance.Low, test.Name, msg);
					results.ResultErrors.Add( new Exception(msg) );
				}

				if (results.MediaObjects != null)
				{
					if (results.MediaObjects.Count == 1)
					{
						metadata = results.MediaObjects[0] as IUPnPMedia;

						if (metadata == null)
						{
							throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but the container's metadata is not stored in an IUPnPMedia object. The offending type is " + results.MediaObjects[0].GetType().ToString(), results.MediaObjects[0]);
						}

						IMediaContainer imc = metadata as IMediaContainer;

						//
						// check metadata 
						//

						if (imc == null)
						{
							//results.SetError(UPnPTestStates.Failed);
							string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object but it was not declared with a \"container\" element.";
							//arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
							results.ResultErrors.Add( new Exception(msg) );
						}
						else
						{
						}

						if (metadata.ID != containerID)
						{
							//results.SetError(UPnPTestStates.Failed);
							string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object with ID=\"" +metadata.ID+ "\" when it should be \"" +containerID+"\" as indicated by the input parameter.";
							//arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
							results.ResultErrors.Add( new Exception(msg) );
						}

						if (containerID == "0")
						{
							if (metadata.ParentID != "-1")
							{
								//results.SetError(UPnPTestStates.Failed);
								string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " returned a DIDL-Lite media object with parentID=\"" +metadata.ID+ "\" when it must be \"-1\" because the container is the root container.";
								//arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
								results.ResultErrors.Add( new Exception(msg) );
							}

							// no need to check parentID values for other containers because
							// they get checked when getting the children for this container.
						}

						if (results.WorstError < UPnPTestStates.Failed)
						{
							//string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " succeeded.";
							//arg._TestGroup.AddEvent(LogImportance.Remark, test.Name, msg);
						}
					}
					else
					{
						//results.SetError(UPnPTestStates.Failed);
						//string msg = "\"" + test.Name + "\": " + input.PrintBrowseParams() + " did not yield exactly one CDS-compliant media object in its result. Instantiated a total of " +results.MediaObjects.Count+ " media objects.";
						//arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg);
					}
				}
				else
				{
					//throw new TestException(test.Name + " has a TEST LOGIC ERROR. Browse returned without errors but no media objects were instantiated.", null);
				}
			}

			if ((results.InvokeError != null) || (results.ResultErrors.Count > 0))
			{
				StringBuilder msg = new StringBuilder();
				results.SetError(UPnPTestStates.Failed);
				
				msg.AppendFormat("\"{0}\": {1} did not yield exactly one CDS-compliant media object in its result. Instantiated a total of {2} media objects.", test.Name, input.PrintBrowseParams(), results.MediaObjects.Count);
				msg.AppendFormat("\r\nAdditional Information:");
				if (results.InvokeError != null)
				{
					msg.AppendFormat("\r\n{0}", results.InvokeError.Message);
				}
				
				foreach (Exception e in results.ResultErrors)
				{
					msg.AppendFormat("\r\n{0}", e.Message);
				}

				arg._TestGroup.AddEvent(LogImportance.Critical, test.Name, msg.ToString());
			}

			return results;
		}
		

		protected override void SetTestInfo()
		{
			this._Name = "Browse All";
			this._Description = "Browse entire hierarchy using Filter=\"*\", StartingIndex=?, RequestedCount=1, SortCriteria=\"\".";
			this._ExpectedTestingTime = 30;
		}
	}
}
