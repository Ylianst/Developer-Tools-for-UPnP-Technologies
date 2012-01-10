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

namespace OpenSource.UPnP.AV.MediaServer.CP
{

	/// <summary>
	/// Thrown when attempting to make a request on a remote
	/// media server when the related object cannot obtain
	/// a reference to the remote media server. This happens
	/// most often when the <see cref="CpMediaContainer"/>.GetServer()
	/// method returns null.
	/// </summary>
	public class Error_CannotGetServer : ApplicationException
	{
		public Error_CannotGetServer (ICpMedia mediaObj)
			: base ("Cannot get MediaServer object")
		{
			this.RemovedObject = mediaObj;
		}

		public ICpMedia RemovedObject;
	}

	/// <summary>
	/// Thrown when attempting to get the parent of a media object.
	/// </summary>
	public class Error_CannotGetParent : ApplicationException
	{
		public Error_CannotGetParent (ICpMedia mediaObj)
			: base ("Cannot get CpMediaContainer parent")
		{
			this.ObjectMissingParent = mediaObj;
		}

		public ICpMedia ObjectMissingParent;
	}

	/// <summary>
	/// Thrown when attempting to request a media server to do 
	/// somethign with a media object that isn't part of that server.
	/// </summary>
	public class Error_MediaNotOnServer : ApplicationException
	{
		public Error_MediaNotOnServer (ICpMedia mediaObj, CpMediaServer server)
			: base ("Media object not on server")
		{
			this.Media = mediaObj;
			this.Server = server;
		}

		public ICpMedia Media;
		public CpMediaServer Server;
	}

	/// <summary>
	/// Thrown when attempting to request a media server to do 
	/// somethign with a resource object that isn't part of that server.
	/// </summary>
	public class Error_ResourceNotOnServer : ApplicationException
	{
		public Error_ResourceNotOnServer (ICpResource resource, CpMediaServer server)
			: base ("Resource object not on server")
		{
			this.Resource = resource;
			this.Server = server;
		}

		public ICpResource Resource;
		public CpMediaServer Server;
	}

	/// <summary>
	/// Thrown when attempting to request an action on a media server
	/// that would result in the creation of a child item under an item object
	/// or would result in the creation of a child pointing to a container.
	/// </summary>
	public class Error_CannotRequestCreate : ApplicationException
	{
		public Error_CannotRequestCreate (ICpMedia parent, ICpMedia child)
			: base ("Cannot create object")
		{
			Parent = parent;
			Child = child;
		}

		/// <summary>
		/// Represents proposed child item or underlying referenced item.
		/// </summary>
		public ICpMedia Child;
		/// <summary>
		/// Represents the container where the child item was supposed to go.
		/// </summary>
		public ICpMedia Parent;
	}
}
