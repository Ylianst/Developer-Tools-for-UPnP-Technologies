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
using System.IO;
using System.Xml;
using System.Text;
using System.Threading;
using System.Reflection;
using System.Collections;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Builds <see cref="DvMediaResource"/> objects.
	/// </summary>
	public class DvResourceBuilder
	{
		/// <summary>
		/// Creates a new resource, given a contentUri and a protocolInfo.
		/// </summary>
		/// <param name="contentUri"></param>
		/// <param name="protocolInfo"></param>
		/// <returns></returns>
		public static DvMediaResource CreateResource (string contentUri, string protocolInfo)
		{
			ResourceBuilder.ResourceAttributes attribs = new ResourceBuilder.ResourceAttributes();
			attribs.contentUri = contentUri;
			attribs.protocolInfo = new ProtocolInfoString(protocolInfo);

			return DvResourceBuilder.CreateResource(attribs, false);
		}

		/// <summary>
		/// Creates a new resource, given the attributes for the resource and an indication
		/// of whether item resource should be importable.
		/// </summary>
		/// <param name="attribs"></param>
		/// <param name="allowImport"></param>
		/// <returns></returns>
		public static DvMediaResource CreateResource(ResourceBuilder.ResourceAttributes attribs, bool allowImport)
		{
			DvMediaResource newRes = new DvMediaResource();
			ResourceBuilder.SetCommonAttributes(newRes, attribs);

			newRes.CheckLocalFileExists();
			newRes.AllowImport = allowImport;

			return newRes;
		}
	}
}
