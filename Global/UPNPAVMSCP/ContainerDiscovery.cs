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

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// ContainerDiscovery will find content on UPnP MediaServers that are
	/// considered "good" - eg, servers that properly implement
	/// SystemUpdateId state variable and event it as well as
	/// implement SinkProtocolInfo.
	/// </summary>
	public class ContainerDiscovery
	{		
		/// <summary>
		/// For those wishing to use the all
		/// discovered root containers as children of
		/// another aggregating root container, 
		/// the AllRoots field suffices to that end.
		/// </summary>
		public readonly CpRootCollectionContainer AllRoots = new CpRootCollectionContainer();

		/// <summary>
		/// Returns a list of root containers found on the upnp network.
		/// </summary>
		public IList RootContainers
		{
			get
			{
				CpMediaServer[] servers = this.m_ServerFinder.GoodServers;
				CpRootContainer[] roots = new CpRootContainer[servers.Length];
				int i=0;
				foreach (CpMediaServer server in servers)
				{
					roots[i] = server.Root;
					i++;
				}

				return roots;
			}
		}

		/// <summary>
		/// Memory cleanup
		/// </summary>
		public void Dispose()
		{
			this.m_ServerFinder = null;
			this.m_ServerFinder.OnGoodServerAdded -= new MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnServerAdded);
			this.m_ServerFinder.OnGoodServerRemoved -= new MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnServerRemoved);
		}


		/// <summary>
		/// Executed when a server shows up.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="server"></param>
		private void Sink_OnServerAdded (MediaServerDiscovery sender, CpMediaServer server)
		{
			server.Root.Update();
			this.AllRoots.AddRootContainer(server.Root);
			this.AllRoots.NotifyRootsOfChange();
		}

		/// <summary>
		/// Executed when a server disappears.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="server"></param>
		private void Sink_OnServerRemoved (MediaServerDiscovery sender, CpMediaServer server)
		{
			this.AllRoots.RemoveRootContainer(server.Root);
			this.AllRoots.NotifyRootsOfChange();
		}

		/// <summary>
		/// Returns a static instance of the ContainerDiscovery object.
		/// </summary>
		/// <returns></returns>
		public static ContainerDiscovery GetInstance()
		{
			lock (LockInit)
			{
				if (TheContainerFinder == null)
				{
					TheContainerFinder = new ContainerDiscovery();
				}
			}
			return TheContainerFinder;
		}

		/// <summary>
		/// Creates the static instance.
		/// </summary>
		private ContainerDiscovery()
		{
			this.m_ServerFinder = new MediaServerDiscovery
				(
				null,
				null,
				new MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnServerAdded),
				new MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnServerRemoved)
				);
		}

		/// <summary>
		/// Keeps a reference to the object that finds media servers.
		/// </summary>
		private MediaServerDiscovery m_ServerFinder = null;

		private static object LockInit = new object();
		private static ContainerDiscovery TheContainerFinder = null;
	}
}
