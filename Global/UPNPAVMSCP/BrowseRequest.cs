using System;
using System.Collections;

namespace Intel.UPNP.AV.MediaServer.CP
{
	/// <summary>
	/// Keeps the values associated with a browse request.
	/// </summary>
	public struct BrowseRequest
	{
		public CpContentDirectory.Enum_A_ARG_TYPE_BrowseFlag BrowseFlag;
		
		public uint StartIndex;
		public uint RequestCount;
		
		public string Filter;
		public string SortCriteria;

		public uint UpdateID;

		public object Tag;
	}
}
