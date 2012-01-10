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
using System.Threading;
using System.Collections;
using OpenSource.Utilities;

namespace OpenSource.UPnP
{
	/// <summary>
	/// Safe timer, that doesn't spawn new threads, or resources that you own
	/// </summary>
	public sealed class SafeTimer
	{
		private bool WaitFlag;
		private bool StartFlag;
		private int timeout;
		/// <summary>
		/// These group of members are used to notify the modules above that an event has fired
		/// without holding any strong references
		/// </summary>
		private WeakEvent ElapsedWeakEvent = new WeakEvent();
		public delegate void TimeElapsedHandler();
		public event TimeElapsedHandler OnElapsed
		{
			add {ElapsedWeakEvent.Register(value);}
			remove {ElapsedWeakEvent.UnRegister(value);}
		}

		/// <summary>
		/// Time Period
		/// </summary>
		public int Interval = 0;
		public bool AutoReset = false;

		private RegisteredWaitHandle handle;
		private ManualResetEvent mre = new ManualResetEvent(false);
		private object RegLock = new object();
		private WaitOrTimerCallback WOTcb;

		public SafeTimer()
		{
			WaitFlag = false;
			timeout = 0;
			WOTcb = new WaitOrTimerCallback(HandleTimer);
			OpenSource.Utilities.InstanceTracker.Add(this);
		}
		public SafeTimer(int Milliseconds, bool Auto):this()
		{
            if (Milliseconds <= 0 || Milliseconds >= Int32.MaxValue) Milliseconds = 1000; // This should never happen
			Interval = Milliseconds;
			AutoReset = Auto;
			OpenSource.Utilities.InstanceTracker.Add(this);
		}
		public void Start()
		{
			lock(RegLock)
			{
				if (WaitFlag == false)
				{
					mre.Reset();
					if (handle != null) handle.Unregister(null);
					handle = ThreadPool.RegisterWaitForSingleObject(mre, WOTcb, null, Interval, true);
				}
				else
				{
					StartFlag = true;
					if (Interval < timeout) timeout = Interval;
				}
			}
		}
		public void dispose()
		{
			if (handle!=null)
			{
				handle.Unregister(null);
			}
		}
		public void Stop()
		{
			bool IsOK;
			lock(RegLock)
			{
				if (handle!=null)
				{
					IsOK = handle.Unregister(null);
				}
				handle = null;
			}
		}
		private void HandleTimer(Object State, bool TimedOut)
		{
			if (TimedOut == false)
			{
				return;
			}
			
			lock(RegLock)
			{
				if (handle != null)
				{
					handle.Unregister(null);
					handle = null;
				}
				WaitFlag = true;
				StartFlag = false;
				timeout = Interval;
			}
			this.ElapsedWeakEvent.Fire();
			
			if (AutoReset == true)
			{
				lock(RegLock)
				{
					mre.Reset();
					handle = ThreadPool.RegisterWaitForSingleObject(mre, WOTcb, null, Interval, true);
				}
			}
			else
			{
				lock(RegLock)
				{
					if (WaitFlag == true && StartFlag == true)
					{
						Interval = timeout;
						mre.Reset();
						if (handle != null) handle.Unregister(null);
						handle = ThreadPool.RegisterWaitForSingleObject(mre, WOTcb, null, Interval, true);
					}
					WaitFlag = false;
					StartFlag = false;
				}
			}
		}
	}
}
