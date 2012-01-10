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
using System.Collections;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// Declares a method that can be used to print the resources of an <see cref="IDvMedia"/>
	/// object, so that each resource is printed once for each
	/// network interface.
	/// </summary>
	public class InnerXmlWriterDv
	{
		public static void WriteInnerXmlResources(IUPnPMedia mo, InnerXmlWriter.DelegateShouldPrintResources shouldPrintResources, ToXmlFormatter formatter, ToXmlData data, XmlTextWriter xmlWriter)
		{
			IDvMedia dvm = (IDvMedia) mo;
			ToXmlDataDv txdv = (ToXmlDataDv) data;

			if (shouldPrintResources(txdv.DesiredProperties))
			{
				if (txdv.BaseUris == null) { txdv.BaseUris = new ArrayList(); }
				if (txdv.BaseUris.Count == 0) { txdv.BaseUris.Add(txdv.BaseUri); }

				ToXmlFormatter resFormatter = formatter;
				resFormatter.StartElement = null;
				resFormatter.EndElement = null;
				resFormatter.WriteInnerXml = null;
				resFormatter.WriteValue = null;

				// Code is unfinished - intended to allow a media object to
				// print duplicate resource elements so that each resource
				// is printed once for every available network interface.

				foreach (string baseUri in txdv.BaseUris)
				{
					txdv.BaseUri = baseUri;
					foreach (IMediaResource res in dvm.MergedResources)
					{
						// Set up the resource formatter to use the
						// default StartElement, EndElement, WriteInnerXml, and WriteValue
						// implementations. This stuff has no effect
						// if the WriteResource field has been assigned.
						res.ToXml(resFormatter, txdv, xmlWriter);
					}
				}
			}
		}
	}
}
