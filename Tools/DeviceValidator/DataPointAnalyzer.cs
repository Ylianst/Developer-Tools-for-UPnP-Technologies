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

namespace UPnPValidator
{
	/// <summary>
	/// Summary description for DataPointAnalyzer.
	/// </summary>
	public class DataPointAnalyzer
	{
		private ArrayList TimeSpanList = new ArrayList();

		public DataPointAnalyzer()
		{
		}

		public TimeSpan Average
		{
			get
			{
				lock(TimeSpanList)
				{
					long ts = 0;
					foreach(TimeSpan t in TimeSpanList)
					{
						ts += t.Ticks;
					}
					ts = ts / (long)TimeSpanList.Count;
					return(new TimeSpan(ts));
				}
			}
		}
		public TimeSpan Variance
		{
			get
			{
				lock(TimeSpanList)
				{
					TimeSpan AV = Average;
					long SUM = 0;
					foreach(TimeSpan ts in TimeSpanList)
					{
						long temp = ts.Ticks-AV.Ticks;
						temp = temp * temp;
						SUM += temp;
					}
					return(new TimeSpan(SUM/(long)TimeSpanList.Count));
				}
			}
		}
		public TimeSpan StandardDeviation
		{
			get
			{
				lock(TimeSpanList)
				{
					return(new TimeSpan((long)System.Math.Sqrt((double)Variance.Ticks)));
				}
			}
		}

		public void AddDataPoint(TimeSpan dp)
		{
			lock(TimeSpanList)
			{
				TimeSpanList.Add(dp);
			}
		}
	}
}
