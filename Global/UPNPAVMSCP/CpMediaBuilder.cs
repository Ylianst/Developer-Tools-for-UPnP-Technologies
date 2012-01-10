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
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// Summary description for CpMediaBuilder.
	/// </summary>
	public class CpMediaBuilder
	{
		/// <summary>
		/// Given a DIDL-Lite document in string form, this method
		/// creates a set of subtrees that represent the document.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		/// <returns>
		/// arraylist of 
		/// <see cref="CpMediaItem"/> or
		/// <see cref="CpMediaContainer"/> objects.
		/// </returns>
		/// <exception cref="OpenSource.UPnP.AV.CdsMetadata.Error_BadMetadata">
		/// Thrown when the DIDL-Lite is not well formed or not compliant
		/// with ContentDirectory specifications.
		/// </exception>
		public static ArrayList BuildMediaBranches(string DidlLiteXml)
		{
			return MediaBuilder.BuildMediaBranches(DidlLiteXml, typeof(CpMediaItem), typeof(CpMediaContainer));
		}

		/// <summary>
		/// Creates a 
		/// <see cref="CpMediaItem"/>
		/// object, given a metadata instantiation
		/// block.
		/// </summary>
		/// <param name="info">
		/// The metadata to use when instantiating the media.
		/// </param>
		/// <returns>a new media item</returns>
		public static CpMediaItem CreateItem (MediaBuilder.item info)
		{
			CpMediaItem newObj = new CpMediaItem();
			MediaBuilder.SetObjectProperties(newObj, info);
			return newObj;
		}

		/// <summary>
		/// Creates a 
		/// <see cref="CpMediaContainer"/>
		/// object, given a metadata instantiation
		/// block.
		/// </summary>
		/// <param name="info">
		/// The metadata to use when instantiating the media.
		/// </param>
		/// <returns>a new media container</returns>
		public static CpMediaContainer CreateContainer (MediaBuilder.container info)
		{
			CpMediaContainer newObj = new CpMediaContainer();
			MediaBuilder.SetObjectProperties(newObj, info);
			return newObj;
		}

		

		/// <summary>
		/// Used by 
		/// <see cref="CpMediaBuilder"/>
		/// to instantiate a root container. 
		/// </summary>
		/// <param name="info">metadata information for the container</param>
		/// <returns>the new root container</returns>
		internal static CpRootContainer CreateRoot(MediaBuilder.container info)
		{
			info.ID = "0";
			info.IdIsValid = true;
			CpRootContainer root = new CpRootContainer();
			MediaBuilder.SetObjectProperties(root, info);
			return root;
		}
	}
}
