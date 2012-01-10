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
	/// Summary description for ToXmlDataDv.
	/// </summary>
	public class ToXmlDataDv : ToXmlData
	{
		/// <summary>
		/// If this field is non-null, then it means the
		/// <see cref="DvMediaContainer"/> and <see cref="DvMediaItem"/>
		/// class is responsible for printing an additional resource
		/// for each baseUri (string) found in the <see cref="ArrayList"/>.
		/// </summary>
		public ArrayList BaseUris = null;

		/// <summary>
		/// Default constructor.
		/// </summary>
		public ToXmlDataDv(){}

		/// <summary>
		/// Instantiates object with custom recurse and desired properties options.
		/// </summary>
		/// <param name="isRecursive"></param>
		/// <param name="desiredProperties"></param>
		public ToXmlDataDv(bool isRecursive, ArrayList desiredProperties)
			: base (isRecursive, desiredProperties)
		{
		}

		public ToXmlDataDv(bool isRecursive, ArrayList desiredProperties, bool includeElementDeclaration)
			: base (isRecursive, desiredProperties, includeElementDeclaration)
		{
		}	
	}
}
