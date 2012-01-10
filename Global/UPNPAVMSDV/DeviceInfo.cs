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

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// This class helps to minimize the number of parameters
	/// sent when instantiating a 
	/// <see cref="MediaServerDevice"/>
	/// object.
	/// </summary>
	public class DeviceInfo
	{
		/// <summary>
		/// Comma-separated value list of fields that can be used for search.
		/// </summary>
		public string SearchCapabilities = "";
		/// <summary>
		/// Comma-separated value list of fields that can be sorted.
		/// </summary>
		public string SortCapabilities = "";
		/// <summary>
		/// Indicates if the MediaServer should advertise
		/// search related methods.
		/// </summary>
		public bool EnableSearch = false;
		/// <summary>
		/// Indicates if the MediaServer should advertise
		/// the methods for managing a content hierarchy.
		/// </summary>
		public bool AllowRemoteContentManagement = false;
		/// <summary>
		/// Indicates if the MediaServer should advertise the
		/// PrepareForConnection action. Forr HTTP-GET
		/// content, this value should be false.
		/// </summary>
		public bool EnablePrepareForConnection = false;

		/// <summary>
		/// Indicates if the MediaServer should advertise the
		/// ConnectionComplete action. Forr HTTP-GET
		/// content, this value should be false.
		/// </summary>
		public bool EnableConnectionComplete = false;

		/// <summary>
		/// Indicates if the device claims to be INMPR'03 compliant
		/// </summary>
		public bool INMPR03 = true;

		/// <summary>
		/// Friendly name of the device.
		/// </summary>
		public string FriendlyName;
		/// <summary>
		/// Manufacturer of the upnp device.
		/// </summary>
		public string Manufacturer;
		/// <summary>
		/// URL for the manufacturer.
		/// </summary>
		public string ManufacturerURL;
		/// <summary>
		/// Manufacturer specified model name.
		/// </summary>
		public string ModelName;
		/// <summary>
		/// Short description of the model.
		/// </summary>
		public string ModelDescription;
		/// <summary>
		/// URL where information about model can be retrieved.
		/// </summary>
		public string ModelURL; 
		/// <summary>
		/// Manufacturer spefied model number.
		/// </summary>
		public string ModelNumber;
		/// <summary>
		/// Local root directory for virtual serving.
		/// Can usually be left as an empty string
		/// or a root directory.
		/// </summary>
		public string LocalRootDirectory;
		public string CustomUDN = "";
		public int CacheTime = 1800;
	}
}
