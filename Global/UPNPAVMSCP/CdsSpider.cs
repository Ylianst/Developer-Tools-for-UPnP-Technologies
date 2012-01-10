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
using OpenSource.Utilities;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Summary description for CdsSpider.
	/// </summary>
	public class CdsSpider
	{
		/// <summary>
		/// Used to indicate a set of changes for the spider's matched objects.
		/// </summary>
		public delegate void Delegate_OnMatchesChanged (CdsSpider sender, IList mediaObjects);

		/// <summary>
		/// Event is fired when the current list is cleared or changes 
		/// so it has no matches.
		/// </summary>
		public event Delegate_OnMatchesChanged OnMatchesCleared;
		
		/// <summary>
		/// Event is fired when the current list gains additional items
		/// in its matched list. The listing of media objects
		/// </summary>
		public event Delegate_OnMatchesChanged OnMatchesAdded;
		
		/// <summary>
		/// Event is fired when the current list no longer matches items 
		/// from since the previous list. 
		/// </summary>
		public event Delegate_OnMatchesChanged OnMatchesRemoved;

		/// <summary>
		/// Event is fired when the MonitorThis property is no
		/// longer valid because the container has been removed
		/// from the MediaServer's tree.
		/// </summary>
		public event Delegate_OnMatchesChanged OnContainerGone;

		/// <summary>
		/// Event is fired when the MonitorThis container
		/// notifies the spider that it's done giving results
		/// for now.
		/// </summary>
		public event Delegate_OnMatchesChanged OnUpdateDone;

		/// <summary>
		/// Default constructor that sets up a spider to have zero
		/// matches for any container that it may have an interest in.
		/// </summary>
		public CdsSpider()
		{
			OpenSource.Utilities.InstanceTracker.Add(this);
			System.Threading.Interlocked.Increment(ref SpiderCounter);
		}

		/// <summary>
		/// Finalizer.
		/// </summary>
		~CdsSpider()
		{
			OpenSource.Utilities.InstanceTracker.Remove(this);
			System.Threading.Interlocked.Decrement(ref SpiderCounter);
			try
			{
				if (this.MonitorThis != null)
				{
					this.MonitorThis = null;
				}
			}
			catch
			{
				// exceptions get thrown because a mutex somewhere is null
			}
		}
		private static long SpiderCounter = 0;

		/// <summary>
		/// Allows public programmers to attach some miscellaneous info
		/// to a spider.
		/// </summary>
		public object Tag = null;

		/// <summary>
		/// Sets/Gets the media container that this spider should
		/// consider its home container. If the container disappears,
		/// then the value returns null.
		/// <para>
		/// Programmers should be aware that this setting
		/// this property will clear the current list
		/// of matched objects.
		/// </para>
		/// <para>
		/// If the new value of the property is equal
		/// by reference to the old value, then nothing
		/// happens. If the new value of the property maps
		/// is non-null, but the container is determined
		/// to be orphaned, then we throw an
		/// <see cref="Error_CannotGetServer"/> exception.
		/// </para>
		/// <para>
		/// Programmers must be aware that setting this
		/// property to null will cause the container
		/// object to orphan descendent objects that
		/// do not have another CdsSpider object causing
		/// a descendent to remain in memory. 
		/// </para>
		/// <para>
		/// In these examples assume the existence of containers A, B, C,
		/// where B is child of A*, and C* is child of B*. Also assume
		/// that S1 and S2 are unique spider objects.
		/// </para>
		/// 
		/// <para>
		/// Proper usage example 1:
		/// <list type="bullet">
		/// <description>
		/// S1 monitors A*, causing all ancestors of A* to observe
		/// that A* has a spider. A* will also request the remote media server
		/// to for its children. This will cause S1 to report the
		/// existence of B*.
		/// </description>
		/// <description>
		/// S2 then monitors B*, causing all ancestors of B* to observe
		/// that B* has a spider. B* will also request the remote media server
		/// to for its children. This will cause S2 to report the
		/// existence of C*.
		/// </description>
		/// <description>
		/// S1 now monitors null, which notifies A* that it no longer
		/// is being monitored. This causes A* to desire the removal of B* (and
		/// all other direct children of A*).
		/// However A* observes that one or more child/descendents of B* are
		/// monitored by one or more spiders, so A* will not remove B*
		/// from its list of child objects.
		/// </description>
		/// </list>
		/// </para>
		/// 
		/// <para>
		/// Proper usage example 2: 
		/// <list type="bullet">
		/// <description>
		/// S1 monitors A*, causing all ancestors of A* to observe
		/// that A* has a spider. A* will also request the remote media server
		/// to for its children. This will cause S1 to report the
		/// existence of B*.
		/// </description>
		/// <description>
		/// S1 now monitors B*, causing all ancestors of B* to observe
		/// that B* has a spider. This change also notifies A* that it no longer
		/// is being monitored. This causes A* to desire the removal of B* (and
		/// all other direct children of A*).
		/// However A* observes that one or more child/descendents of B* are
		/// monitored by one or more spiders, so A* will not remove B*
		/// from its list of child objects. B* will then request the remote media server
		/// to for its children. This will cause S1 to report the
		/// existence of C*. 
		/// </description>
		/// </para>
		/// 
		/// <para>
		/// Improper usage example 1: 
		/// <list type="bullet">
		/// <description>
		/// S1 monitors A*, causing all ancestors of A* to observe
		/// that A* has a spider. A* will also request the remote media server
		/// to for its children. This will cause S1 to report the
		/// existence of B*.
		/// </description>
		/// <description>
		/// Programmer obtains a strong reference to B*.
		/// </description>
		/// <description>
		/// S1 now monitors null, which notifies A* that it no longer
		/// is being monitored. This causes A* to desire the removal of B* (and
		/// all other direct children of A*).
		/// A* observes that no child/descendents of B* are
		/// monitored by one or more spiders, so A* will remove B*
		/// from its list of child objects.
		/// </description>
		/// <description>
		/// Using the strong reference of B& obtained earlier,
		/// S2 monitors B*. B* will attempt to find its children
		/// but will fail in its attempt because B* has been orphaned
		/// from the virtualized content hierarchy.
		/// </description>
		/// </para>
		/// </summary>
		public CpMediaContainer MonitorThis
		{
			get
			{
				return this.m_MonitorThis;
			}
			set
			{
				bool requestUpdates = false;
				lock (this.m_Matches.SyncRoot)
				{
					if (this.m_MonitorThis != value)
					{
						this.m_IsDead = false;
						CpMediaContainer c = this.m_MonitorThis;

						if (value == null)
						{
							this.m_MonitorThis = null;
							this.ClearMatches();
						}
						else
						{
							this.m_MonitorThis = value;
							ExpectingResults = true;
							value.SubscribeSpider(this);
							requestUpdates = true;
						}

						if (c != null)
						{
							c.UnsubscribeSpider(this);
							if (requestUpdates)
							{
								if (value != c)
								{
									this.RemoveUnwantedMatches();
								}
							}
						}
					}
				}
								
				if (requestUpdates)
				{
					// tell the container to update itself completely,
					// because what the container has cached in memory may
					// not be the complete result set desired by the spider

					// This execution path also throw an exception if
					// when attempting to get an update for a container
					// that is orphaned.
					this.RequestUpdates();
				}
			}
		}

		internal bool ExpectingResults = false;

		/// <summary>
		/// Gets or sets the comparison expression associated
		/// with this spider.
		/// <para>
		/// Programmers should be aware that this setting
		/// this property will clear the current list
		/// of matched objects.
		/// </para>
		/// </summary>
		public IMediaComparer Comparer
		{
			get
			{
				return this.m_Comparer;
			}
			set
			{
				IMediaComparer oldComparer = this.m_Comparer;
				bool rescan = false;
				lock (this.m_Matches.SyncRoot)
				{
					if (oldComparer != value)
					{
						rescan = true;
						this.m_Comparer = value;
				
						this.RemoveUnwantedMatches();

						// don't rescan everything if the assignment
						// indicates that the results would be a subset
						if (oldComparer != null)
						{
							Type type = oldComparer.GetType();
							Type type2 = this.m_Comparer.GetType();
							if (type == typeof(MatchOnAny))
							{
								if (
									(type2 == typeof(MatchOnNever)) ||
									(type2 == typeof(MatchOnContainers)) ||
									(type2 == typeof(MatchOnItem)) ||
									(type2 == typeof(MatchOnAny))
									)
								{
									rescan = false;
								}
							}
							else if (type == this.m_Comparer.GetType())
							{
								if (
									(type2 == typeof(MatchOnNever)) ||
									(type2 == typeof(MatchOnContainers)) ||
									(type2 == typeof(MatchOnItem)) ||
									(type2 == typeof(MatchOnAny))
									)
								{
									rescan = false;
								}
							}
						}

					}
				}
				if (rescan)
				{
					this.ExpectingResults = true;
					this.RequestUpdates();
				}
				else
				{
					if (this.OnUpdateDone != null)
					{
						this.OnUpdateDone(this, null);
					}
				}
			}
		}

		/// <summary>
		/// Gets or sets the sorting object associated
		/// with this spider.
		/// <para>
		/// Programmers should be aware that this setting
		/// this property will clear the current list
		/// of matched objects.
		/// </para>
		/// </summary>
		public IMediaSorter Sorter
		{
			get
			{
				return this.m_Sorter;
			}
			set
			{
				lock (this.m_Matches)
				{
					this.m_Sorter = value;
					this.Resort();
				}
				this.RequestUpdates();
			}
		}

		/// <summary>
		/// Returns a listing of 
		/// <see cref="ICpMedia"/> objects
		/// that match the spider's filtering settings.
		/// The order of the listing will also conform
		/// to any sorting rules applied to the spider.
		/// </summary>
		public IList MatchedObjects
		{
			get
			{
				IList results = this.GetResults(true, true);
				return results;
			}
		}

		/// <summary>
		/// Returns a listing of 
		/// <see cref="CpMediaItem"/> objects
		/// that match the spider's filtering settings.
		/// The order of the listing will also conform
		/// to any sorting rules applied to the spider.
		/// </summary>
		public IList MatchedItems
		{
			get
			{
				IList results = this.GetResults(true, false);
				return results;
			}
		}


		/// <summary>
		/// Returns a listing of 
		/// <see cref="CpMediaContainer"/> objects
		/// that match the spider's filtering settings.
		/// The order of the listing will also conform
		/// to any sorting rules applied to the spider.
		/// </summary>
		public IList MatchedContainers
		{
			get
			{
				IList results = this.GetResults(false, true);
				return results;
			}
		}

		/// <summary>
		/// Returns true if the media object is of interest to the spider.
		/// </summary>
		/// <param name="mediaObj"></param>
		/// <returns></returns>
		internal bool IsMatch (IUPnPMedia mediaObj)
		{
			if (this.m_Comparer != null)
			{
				return this.m_Comparer.IsMatch(mediaObj);
			}

			return false;
		}

		internal bool IsMatch(IUPnPMedia mediaObj, bool returnFalseIfMatchedAlready)
		{
			if (returnFalseIfMatchedAlready)
			{
				if (this.m_Matches.Contains(mediaObj))
				{
					return false;
				}
			}

			return this.IsMatch(mediaObj);
		}
		

		/// <summary>
		/// <see cref="CpMediaContainer.NotifySpidersRemove"/> calls this
		/// method to indicate that child objects have been removed
		/// from the container.
		/// </summary>
		/// <param name="thisChanged">the container that witnessed the change</param>
		/// <param name="removeThese">an ArrayList of STRONG REFERENCES to <see cref="ICpMedia"/> objects</param>
		internal void NotifySinkRemove (CpMediaContainer thisChanged, ArrayList removeThese)
		{
			ArrayList removedObjects = new ArrayList(removeThese.Count);
			lock (this.m_Matches.SyncRoot)
			{
				foreach (ICpMedia obj in this.m_Matches)
				{
					if (removeThese.Contains(obj))
					{
						removedObjects.Add(obj);
					}
				}

				foreach (ICpMedia obj in removedObjects)
				{
					this.m_Matches.Remove(obj);
				}
			}

			if (removedObjects.Count > 0)
			{
				if (this.OnMatchesRemoved != null)
				{
					this.OnMatchesRemoved (this, removedObjects);
				}
			}
		}

		/// <summary>
		/// <see cref="CpMediaContainer.NotifySpidersOfGoneContainer"/> calls this
		/// method to indicate that the container the spider happens
		/// to be monitoring no longer exists.
		/// </summary>
		/// <param name="thisChanged"></param>
		internal void NotifySinkContainerGone (CpMediaContainer thisChanged)
		{
			lock (this.m_Matches.SyncRoot)
			{
				if (thisChanged == this.MonitorThis)
				{
					this.m_IsDead = true;
					if (this.OnContainerGone != null)
					{
						this.OnContainerGone(this, null);
					}

					if (this.MonitorThis == thisChanged)
					{
						this.MonitorThis = null;
					}
				}
			}
		}

		/// <summary>
		/// <see cref="CpMediaContainer"/> call this method to
		/// indicate that it's done giving notifications
		/// for new/removed/updated items for a while.
		/// </summary>
		/// <param name="thisChanged"></param>
		internal void NotifySinkUpdateDone (CpMediaContainer thisChanged)
		{
			if (thisChanged == this.m_MonitorThis)
			{
				this.m_BrowsingError = null;
				if (this.OnUpdateDone != null)
				{
					this.OnUpdateDone(this, null);
				}
			}
		}

		public Exception m_BrowsingError = null;
		public Exception BrowsingError { get { return this.m_BrowsingError; } }

		internal void NotifySinkBrowsingError (CpMediaContainer thisChanged, Exception e)
		{
			if (thisChanged == this.m_MonitorThis)
			{
				this.m_BrowsingError = e;
				if (this.OnUpdateDone != null)
				{
					this.OnUpdateDone(this, null);
				}
			}		
		}

		/// <summary>
		/// If the spider's container has been removed, then
		/// the spider is dead. The value becomes true 
		/// when the MonitorThis property is assigned,
		/// even if the assignment is a null container.
		/// </summary>
		public bool IsDead
		{
			get
			{
				return this.m_IsDead;
			}
		}

		private bool m_IsDead = false;

		/// <summary>
		/// <see cref="CpMediaContainer.NotifySpidersAdd"/> calls this
		/// method to indicate that a container's contents have changed. 
		/// Upon exiting this method, the
		/// CdsSpider acknowledges that the container has changed
		/// and that the container has no obligation to keep its
		/// references to child objects any longer. 
		/// </summary>
		/// <param name="thisChanged">the container that has changed</param>
		/// <param name="addThese">
		/// A reliable listing of new matches for the container's current state.
		/// Each element will be a strong reference to an <see cref="ICpMedia"/> object.
		/// Assume that the container has already filtered the results so that
		/// addThese only contains elements that match the comparer for this spider.
		/// </param>
		internal void NotifySinkAdd (CpMediaContainer thisChanged, ArrayList addThese)
		{
			bool eventAdded = false;
			lock (this.m_Matches.SyncRoot)
			{
				if (thisChanged == this.MonitorThis)
				{
					ExpectingResults = false;
					if (addThese.Count > 0)
					{
						if ((this.m_Matches.Count == 0))
						{
							this.m_Matches = new ArrayList(addThese.Count);
						}

						if (this.m_Sorter != null)
						{
							// perform a sorted insert for each item
							_SortedList sortThese = new _SortedList(this.m_Sorter, true);
							foreach (ICpMedia obj in addThese)
							{
								sortThese.Set(this.m_Matches, obj, false);
							}
						}
						else
						{
							this.m_Matches.AddRange((ICollection) addThese);
						}

						eventAdded = true;
					}
				}
			}

			if (eventAdded)
			{
				if (this.OnMatchesAdded != null)
				{
					this.OnMatchesAdded (this, addThese);
				}
			}
		}


		/// <summary>
		/// Call this method if a spider is no longer interested in an object
		/// that was earlier marked as being of interest to this spider.
		/// </summary>
		/// <param name="obj"></param>
		private void DecrementMatch(ICpMedia obj)
		{
			CpMediaContainer cpc = obj as CpMediaContainer;
			CpMediaItem cpi = obj as CpMediaItem;
			if (cpc != null)
			{
				cpc.DecrementSpiderMatches();
			}
			else if (cpi != null)
			{
				cpi.DecrementSpiderMatches();
			}
		}

		private void Resort()
		{
			if (this.m_Sorter != null)
			{
				lock (this.m_Matches.SyncRoot)
				{
					ArrayList unsorted = this.m_Matches;
					this.m_Matches = new ArrayList(unsorted.Count);

					_SortedList sortThese = new _SortedList(this.m_Sorter, true);
					foreach (ICpMedia obj in unsorted)
					{
						sortThese.Set(this.m_Matches, obj, false);
						//this.SortedInsert(obj);
					}
				}

				if (this.OnMatchesCleared != null)
				{
					this.OnMatchesCleared(this, (IList) EmptyList.Clone());
				}

				if (this.OnMatchesAdded != null)
				{
					this.OnMatchesAdded(this, (IList) this.m_Matches.Clone());
				}
			}
		}

		private static readonly ArrayList EmptyList = new ArrayList(0);

		/// <summary>
		/// Clears the current set of matched media objects.
		/// </summary>
		private void ClearMatches()
		{
			IList oldMatches = null;

			// save a copy of the match list and then clear it
			lock (this.m_Matches.SyncRoot)
			{
				if (this.m_Matches.Count > 0)
				{
					oldMatches = this.m_Matches;
					this.m_Matches = (ArrayList) EmptyList.Clone();
				}
			}

			// properly clean up the old match list
			if (oldMatches != null)
			{
				if (oldMatches.Count > 0)
				{
					if (this.OnMatchesCleared != null)
					{
						this.OnMatchesCleared (this, this.m_Matches);
					}

					// ensure that each previously matched item
					// is infomred that it is no longer of interest
					// to a spider
					foreach (ICpMedia oldMatch in oldMatches)
					{
						this.DecrementMatch(oldMatch);
					}
				}
			}
		}	
		
	
		/// <summary>
		/// Returns the desired subset of matches by their general type.
		/// </summary>
		/// <param name="includeItems"></param>
		/// <param name="includeContainers"></param>
		/// <returns></returns>
		private IList GetResults(bool includeItems, bool includeContainers)
		{
			//this.m_LockMatches.AcquireWriterLock(-1);
			ArrayList results = new ArrayList();
			lock (this.m_Matches.SyncRoot)
			{
				if (this.m_Matches != null)
				{
					results = new ArrayList(this.m_Matches.Count);
					ArrayList removeThese = new ArrayList();
					foreach (ICpMedia obj in this.m_Matches)
					{
						if (
							((obj.IsItem) && (includeItems)) ||
							((obj.IsContainer) && (includeContainers))
							)
						{
							results.Add(obj);
						}
					}
				}
				//this.m_LockMatches.ReleaseWriterLock();
			}
			return results;
		}

		/// <summary>
		/// Removes the unwanted matches from the current match list.
		/// </summary>
		private void RemoveUnwantedMatches()
		{
			//this.m_LockMatches.AcquireWriterLock(-1);
			ArrayList removeThese = new ArrayList();
			ArrayList removedObjects = new ArrayList();
			lock (this.m_Matches.SyncRoot)
			{
				int i = 0;
				foreach (ICpMedia obj in this.m_Matches)
				{
					if (
						(this.m_Comparer.IsMatch(obj) == false) ||
						(obj.Parent != this.m_MonitorThis) ||
						(this.m_MonitorThis == null)
						)
					{
						removeThese.Add(i);
						removedObjects.Add(obj);
						this.DecrementMatch(obj);
					}

					i++;
				}
				if (removeThese.Count > 0)
				{
					i = 0;
					foreach (int index in removeThese)
					{
						this.m_Matches.RemoveAt(index - i);
						i++;
					}
				}
			}
			//this.m_LockMatches.ReleaseWriterLock();

			if (this.OnMatchesRemoved != null)
			{
				if (removedObjects.Count > 0)
				{
					this.OnMatchesRemoved(this, removedObjects);
				}
			}
		}

		/// <summary>
		/// Tells the container to refresh this spider's listing.
		/// </summary>
		private void RequestUpdates()
		{
			if (this.m_MonitorThis != null)
			{
				this.m_MonitorThis.ForceUpdate(true);
			}
		}

		/// <summary>
		/// Indicates the starting index (of a zero-based listing of the 
		/// comlete sorted result set) that the spider should save.
		/// </summary>
		//private int m_Start = 0;

		/// <summary>
		/// Indicates the maximum number of items the spider should be interested in.
		/// </summary>
		//private int m_MaxCount = int.MaxValue;
		
		/// <summary>
		/// This expression is what determines if a media object
		/// is of interest to the spider.
		/// </summary>
		private IMediaComparer m_Comparer = null;

		/// <summary>
		/// This is the object that will handle the sorting of results.
		/// </summary>
		private IMediaSorter m_Sorter = null;

		/// <summary>
		/// Keeps a sorted list of items in an arraylist. We'll
		/// use a SortedList object to do the initial sorting,
		/// but we dump the sorted results into an ArrayList
		/// so as to not hold strong references to the 
		/// media objects.
		/// </summary>
		private ArrayList m_Matches = EmptyList;

		/// <summary>
		/// Keep a reference of the container that this
		/// spider happens to be monitoring.
		/// </summary>
		private CpMediaContainer m_MonitorThis = null;

		private static readonly MatchOnNever ClearAllMatches = new MatchOnNever();
	}
}
