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

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Summary description for DvMediaBuilder.
	/// </summary>
	public class DvMediaBuilder
	{
		/// <summary>
		/// Given a DIDL-Lite document in string form, this method
		/// creates a set of subtrees that represent the document.
		/// </summary>
		/// <param name="DidlLiteXml"></param>
		/// <returns>
		/// arraylist of 
		/// <see cref="DvMediaItem"/> or
		/// <see cref="DvMediaContainer"/> objects.
		/// </returns>
		/// <exception cref="OpenSource.UPnP.AV.CdsMetadata.Error_BadMetadata">
		/// Thrown when the DIDL-Lite is not well formed or not compliant
		/// with ContentDirectory specifications.
		/// </exception>
		public static ArrayList BuildMediaBranches(string DidlLiteXml)
		{
			ArrayList newBranches = MediaBuilder.BuildMediaBranches(DidlLiteXml, typeof(DvMediaItem), typeof(DvMediaContainer));

			//recurse the branches and ensure all have tracking on
			foreach (IDvMedia dvm in newBranches)
			{
				EnableMetadataTracking(dvm);
			}

			return newBranches;
		}

		private static void EnableMetadataTracking(IDvMedia dvm)
		{
			DvMediaContainer dvc = dvm as DvMediaContainer;
			DvMediaItem dvi = dvm as DvMediaItem;

			if (dvc != null)
			{
				dvc.TrackMetadataChanges = true;
				foreach (IDvMedia child in dvc.CompleteList)
				{
					EnableMetadataTracking(child);
				}
			}
			else if (dvi != null)
			{
				dvi.TrackMetadataChanges = true;
			}
		}

		/// <summary>
		/// Creates a 
		/// <see cref="DvMediaItem"/>
		/// object, given a metadata instantiation
		/// block.
		/// </summary>
		/// <param name="info">
		/// The metadata to use when instantiating the media.
		/// </param>
		/// <returns>a new media item</returns>
		public static DvMediaItem CreateItem (MediaBuilder.item info)
		{
			DvMediaItem newObj = new DvMediaItem();
			MediaBuilder.SetObjectProperties(newObj, info);
			newObj.TrackMetadataChanges = true;
			return newObj;
		}

		/// <summary>
		/// Creates a 
		/// <see cref="DvMediaContainer"/>
		/// object, given a metadata instantiation
		/// block.
		/// </summary>
		/// <param name="info">
		/// The metadata to use when instantiating the media.
		/// </param>
		/// <returns>a new media container</returns>
		public static DvMediaContainer CreateContainer (MediaBuilder.container info)
		{
			DvMediaContainer newObj = new DvMediaContainer();
			MediaBuilder.SetObjectProperties(newObj, info);
			newObj.TrackMetadataChanges = true;
			return newObj;
		}

		

		/// <summary>
		/// Used by 
		/// <see cref="MediaServerDevice"/>
		/// to instantiate a root container. 
		/// </summary>
		/// <param name="info">metadata information for the container</param>
		/// <returns>the new root container</returns>
		internal static DvRootContainer CreateRoot(MediaBuilder.container info)
		{
			info.ID = "0";
			info.IdIsValid = true;
			DvRootContainer root = new DvRootContainer();
			MediaBuilder.SetObjectProperties(root, info);
			root.TrackMetadataChanges = true;
			return root;
		}
	}
}
