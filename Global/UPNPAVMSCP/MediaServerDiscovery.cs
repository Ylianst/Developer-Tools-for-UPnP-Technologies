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
using System.Text;
using OpenSource.UPnP;
using System.Threading;
using OpenSource.UPnP.AV;
using System.Collections;
using OpenSource.Utilities;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.CP
{
	/// <summary>
	/// This class abstracts the gory details of finding MediaServer devices.
	/// The class employs a concept of a "good" server and a non-good server.
	/// Good servers the ones that properly handle the subscription request
	/// for both services and then properly event. All servers are considered
	/// non-good until they do this. Programmers may choose to use only the
	/// good servers, as it may indicate a more reliable implementation.
	/// The servers are configured to virtualize only their containers.
	/// 
	/// <para>
	/// It should be noted that this class has been hard-coded to 
	/// instantiate the root container with settings that will
	/// always assume that all items and container are not to
	/// be persisted, thus requiring the use of a <see cref="CdsSpider"/>
	/// object to ensure that child objects are persisted as desired.
	/// The basic reason for this design decision is that 
	/// MediaServerDiscovery should/must allow for static scenarios,
	/// where multiple plug-in applications share the same global address
	/// space, thus allowing a single instance of a media object to
	/// be shared. Thus <see cref="MediaServerDiscovery"/>, 
	/// <see cref="ContainerDiscovery"/>, and all media objects become
	/// shared resources across multiple applications with each
	/// application using the <see cref="CdsSpider"/> object to
	/// cause persistence of the items that are of interest
	/// to individual applications.
	/// </para>
	/// </summary>
	public sealed class MediaServerDiscovery 
	{
		/// <summary>
		/// Unhooks all events
		/// </summary>
		public void Dispose()
		{
			TheInstance.OnCpServerAdded -= new _MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnGoodServerAdded);
			TheInstance.OnCpServerRemoved -= new _MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnGoodServerRemoved);
			TheInstance.OnServerSeen -= new _MediaServerDiscovery.Delegate_OnServerDeviceChange(this.Sink_OnServerAdded);
			TheInstance.OnServerGone -= new _MediaServerDiscovery.Delegate_OnServerDeviceChange(this.Sink_OnServerRemoved);

			this.OnGoodServerAdded = null;
			this.OnGoodServerRemoved = null;
			this.OnServerGone = null;
			this.OnServerSeen = null;
		}

		/// <summary>
		/// This delegate is used when a MediaServer shows up or disappears.
		/// Determination of whether the server is good or non-good isn't known.
		/// </summary>
		public delegate void Delegate_OnServerDeviceChange(MediaServerDiscovery sender, UPnPDevice device);
		/// <summary>
		/// This delegate is used when a good MediaServer shows up or disappears.
		/// </summary>
		public delegate void Delegate_OnGoodServersChange(MediaServerDiscovery sender, CpMediaServer server);

		/// <summary>
		/// Fired when a server shows up.
		/// </summary>
		public event Delegate_OnServerDeviceChange OnServerSeen;
		/// <summary>
		/// Fired when a server disappears.
		/// </summary>
		public event Delegate_OnServerDeviceChange OnServerGone;

		/// <summary>
		/// Fired when a good server shows up.
		/// </summary>
		public event Delegate_OnGoodServersChange OnGoodServerAdded;
		/// <summary>
		/// Fired when a good server disappears.
		/// </summary>
		public event Delegate_OnGoodServersChange OnGoodServerRemoved;

		/// <summary>
		/// Returns a shallow copy array of non-good servers.
		/// </summary>
		public CpMediaServer[] NonGoodServers
		{
			get
			{
				return TheInstance.NonGoodServers;
			}
		}

		/// <summary>
		/// Returns a shallow copy array of good servers.
		/// </summary>
		public CpMediaServer[] GoodServers
		{
			get
			{
				return TheInstance.GoodServers;
			}
		}

		/// <summary>
		/// Constructs a MediaServerDiscovery object.
		/// </summary>
		/// <param name="onServerAddedCallback">null, if not interested in the OnServerSeen event; otherwise adds the callback to the multicasted event</param>
		/// <param name="onServerRemovedCallback">null, if not interested in the OnServerGone event; otherwise adds the callback to the multicasted event</param>
		/// <param name="onGoodServerAddedCallback">null, if not interested in the OnGoodServerAdded event; strongly recommended that this is not null; otherwise adds the callback to the multicasted event</param>
		/// <param name="onGoodServerRemovedCallback">null, if not interested in the OnGoodServerRemoved event; otherwise adds the callback to the multicasted event</param>
		public MediaServerDiscovery
			(
			Delegate_OnServerDeviceChange onServerAddedCallback,
			Delegate_OnServerDeviceChange onServerRemovedCallback,
			Delegate_OnGoodServersChange onGoodServerAddedCallback,
			Delegate_OnGoodServersChange onGoodServerRemovedCallback
			)
		{
			if (onServerAddedCallback != null)
			{
				this.OnServerSeen += onServerAddedCallback;
			}

			if (onServerRemovedCallback != null)
			{
				this.OnServerGone += onServerRemovedCallback;
			}

			if (onGoodServerAddedCallback != null)
			{
				this.OnGoodServerAdded += onGoodServerAddedCallback;
			}
		
			if (onGoodServerRemovedCallback != null)
			{
				this.OnGoodServerRemoved += onGoodServerRemovedCallback;
			}

			CpMediaServer[] servers = null;
			CpMediaServer[] goodServers = null;

			TheLock.WaitOne();
			if (TheInstance != null)
			{
				servers = TheInstance.GoodServers;
				goodServers = TheInstance.GoodServers;
			}
			else
			{
				TheInstance = new _MediaServerDiscovery(true);
			}
			TheLock.ReleaseMutex();

			TheInstance.OnCpServerAdded += new _MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnGoodServerAdded);
			TheInstance.OnCpServerRemoved += new _MediaServerDiscovery.Delegate_OnGoodServersChange(this.Sink_OnGoodServerRemoved);
			TheInstance.OnServerSeen += new _MediaServerDiscovery.Delegate_OnServerDeviceChange(this.Sink_OnServerAdded);
			TheInstance.OnServerGone += new _MediaServerDiscovery.Delegate_OnServerDeviceChange(this.Sink_OnServerRemoved);

			if (servers != null)
			{
				if (this.OnServerSeen != null)
				{
					foreach (CpMediaServer server in servers)
					{
						if (servers.Length > 0)
						{
							this.OnServerSeen(this, server.ConnectionManager.GetUPnPService().ParentDevice);
						}
					}	
				}
			}


			if (goodServers != null)
			{
				if (this.OnGoodServerAdded != null)
				{
					foreach (CpMediaServer server in goodServers)
					{
						if (servers.Length > 0)
						{
							this.OnGoodServerAdded(this, server);
						}
					}	
				}
			}
		}

		/// <summary>
		/// Method executes when a server is discovered.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="device"></param>
		private void Sink_OnServerAdded (_MediaServerDiscovery sender, UPnPDevice device)
		{
			if(this.OnServerSeen != null)
			{
				this.OnServerSeen(this, device);
			}
		}

		/// <summary>
		/// Method executes when a server says BEY BYE.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="device"></param>
		private void Sink_OnServerRemoved (_MediaServerDiscovery sender, UPnPDevice device)
		{
			if(this.OnServerGone != null)
			{
				this.OnServerGone(this, device);
			}
		}
		
		/// <summary>
		/// Method executes when a server has been promoted to be a "good" server.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="server"></param>
		private void Sink_OnGoodServerAdded (_MediaServerDiscovery sender, CpMediaServer server)
		{
			if(this.OnGoodServerAdded != null)
			{
				this.OnGoodServerAdded(this, server);
			}
		}

		/// <summary>
		/// Method executes when a "good" server has done bye bye.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="server"></param>
		private void Sink_OnGoodServerRemoved (_MediaServerDiscovery sender, CpMediaServer server)
		{
			if(this.OnGoodServerRemoved != null)
			{
				this.OnGoodServerRemoved(this, server);
			}		
		}

		/// <summary>
		/// The one and only instance.
		/// </summary>
		private static _MediaServerDiscovery TheInstance;

		private static Mutex TheLock = new Mutex();
	}

	/// <summary>
	/// Handles the core logic for finding MediaServerDiscovery in
	/// such a way that we minimize impact by having a static list.
	/// </summary>
	internal sealed class _MediaServerDiscovery 
	{
		/// <summary>
		/// This delegate is used when a MediaServer shows up or disappears.
		/// Determination of whether the server is good or non-good isn't known.
		/// </summary>
		public delegate void Delegate_OnServerDeviceChange(_MediaServerDiscovery sender, UPnPDevice device);
		/// <summary>
		/// This delegate is used when a good MediaServer shows up or disappears.
		/// </summary>
		public delegate void Delegate_OnGoodServersChange(_MediaServerDiscovery sender, CpMediaServer server);

		/// <summary>
		/// Fired when a server shows up.
		/// </summary>
		public event Delegate_OnServerDeviceChange OnServerSeen;
		/// <summary>
		/// Fired when a server disappears.
		/// </summary>
		public event Delegate_OnServerDeviceChange OnServerGone;

		/// <summary>
		/// Fired when a good server shows up.
		/// </summary>
		public event Delegate_OnGoodServersChange OnCpServerAdded;
		/// <summary>
		/// Fired when a good server disappears.
		/// </summary>
		public event Delegate_OnGoodServersChange OnCpServerRemoved;

		/// <summary>
		/// Returns an array of non-good servers.
		/// </summary>
		public CpMediaServer[] NonGoodServers
		{
			get
			{
				CpMediaServer[] results = null;
				lock (LockHashes)
				{
					ICollection servers = UdnToInitStatus.Values;
					results = new CpMediaServer[servers.Count];
					int i=0;
					foreach (InitStatus status in servers)
					{
						results[i] = status.Server;
						i++;
					}
				}

				return results;
			}
		}

		/// <summary>
		/// Returns an array of good servers.
		/// </summary>
		public CpMediaServer[] GoodServers
		{
			get
			{
				CpMediaServer[] results = null;
				lock (LockHashes)
				{
					ICollection servers = UdnToServer.Values;
					
					results = new CpMediaServer[servers.Count];
					int i=0;
					foreach (CpMediaServer server in servers)
					{
						results[i] = server;
						i++;
					}
				}

				return results;
			}
		}


		/// <summary>
		/// This is the internal constructor, that basically makes it so
		/// only one object is responsible for keeping track of 
		/// media server objects.
		/// </summary>
		/// <param name="readOnlyDesiredContainerState">
		/// If true, then the public programmer cannot set the desired
		/// state of containers, automatically causing each container
		/// to virtualize only their immediate child containers.
		/// </param>
		internal _MediaServerDiscovery
			(
			bool readOnlyDesiredContainerState
			)
		{
			m_Scp = new UPnPSmartControlPoint(
				new UPnPSmartControlPoint.DeviceHandler(this.Temporary_AddServer),
				null,
				new string[2] { CpContentDirectory.SERVICE_NAME, CpConnectionManager.SERVICE_NAME } );
			
			m_Scp.OnRemovedDevice += new UPnPSmartControlPoint.DeviceHandler(this.RemoveServer);
			m_readOnlyDesiredState = readOnlyDesiredContainerState;
		}

		/// <summary>
		/// Indicates if the containers have a read-only desired state.
		/// If true, then all of the CpMediaContainer objects will
		/// only virtualize their immediate child containers.
		/// </summary>
		private bool m_readOnlyDesiredState;

		/// <summary>
		/// Memory cleanup.
		/// </summary>
		public void Dispose()
		{
			m_Scp.OnRemovedDevice -= new UPnPSmartControlPoint.DeviceHandler(this.RemoveServer);
			this.OnCpServerAdded = null;
			this.OnCpServerRemoved = null;
			this.OnServerGone = null;
			this.OnServerSeen = null;
			m_Scp = null;
		}

		/// <summary>
		/// When a MediaServer device is found, we add it to a temp list.
		/// Then we attempt to subscribe to its services.
		/// </summary>
		/// <param name="sender">the smart cp that found the device</param>
		/// <param name="device">the mediaserver device</param>
		private void Temporary_AddServer(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			if (this.OnServerSeen != null)
			{
				this.OnServerSeen(this, device);
			}

			UPnPService sCD = device.GetServices(CpContentDirectory.SERVICE_NAME)[0];
			UPnPService sCM = device.GetServices(CpConnectionManager.SERVICE_NAME)[0];

			CpMediaServer newServer;
			try
			{
				newServer = new CpMediaServer(device);
			}
			catch (UPnPCustomException)
			{
				newServer = null;
			}
			
			if (newServer != null)
			{
				InitStatus status = new InitStatus();
				status.SubcribeCD = false;
				status.SubcribeCM = false;
				status.EventedCD = false;
				status.EventedCM = false;
				status.Server = newServer;

				lock (LockHashes)
				{
					UdnToInitStatus[device.UniqueDeviceName] = status;
				}

				newServer.ConnectionManager.OnSubscribe += new CpConnectionManager.SubscribeHandler(this.Sink_OnCmServiceSubscribe);
				newServer.ContentDirectory.OnSubscribe += new CpContentDirectory.SubscribeHandler(this.Sink_OnCdServiceSubscribe);
				
				newServer.ConnectionManager.OnStateVariable_SourceProtocolInfo += new CpConnectionManager.StateVariableModifiedHandler_SourceProtocolInfo(this.Sink_OnCmEvented);
				newServer.ContentDirectory.OnStateVariable_SystemUpdateID += new CpContentDirectory.StateVariableModifiedHandler_SystemUpdateID(this.Sink_OnCdEvented);
				
				newServer.ConnectionManager._subscribe(600);
				newServer.ContentDirectory._subscribe(600);
			}
		}

		/// <summary>
		/// Executes when the ContentDirectory service returns on the subscribe status.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="success"></param>
		private void Sink_OnCdServiceSubscribe(CpContentDirectory sender, bool success)
		{
			sender.OnSubscribe -= new CpContentDirectory.SubscribeHandler(this.Sink_OnCdServiceSubscribe);
			string udn = sender.GetUPnPService().ParentDevice.UniqueDeviceName;

			lock (LockHashes)
			{
				InitStatus status = (InitStatus) UdnToInitStatus[udn];
				if (status != null)
				{
					status.ZeroMeansDone--;
					status.SubcribeCD = success;
					this.ProcessInitStatusChange(udn);
				}
			}
		}

		/// <summary>
		/// Executes when the ConnectionManager service returns on the subscribe status.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="success"></param>
		private void Sink_OnCmServiceSubscribe(CpConnectionManager sender, bool success)
		{
			sender.OnSubscribe -= new CpConnectionManager.SubscribeHandler(this.Sink_OnCmServiceSubscribe);
			string udn = sender.GetUPnPService().ParentDevice.UniqueDeviceName;

			lock (LockHashes)
			{
				InitStatus status = (InitStatus) UdnToInitStatus[udn];
				if (status != null)
				{
					status.ZeroMeansDone--;
					status.SubcribeCM = success;
					this.ProcessInitStatusChange(udn);
				}
			}
		}

		/// <summary>
		/// Executes when the ContentDirectory events for the first time.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="newVal"></param>
		private void Sink_OnCdEvented (CpContentDirectory sender, UInt32 newVal)
		{
			sender.OnStateVariable_SystemUpdateID -= new CpContentDirectory.StateVariableModifiedHandler_SystemUpdateID(this.Sink_OnCdEvented);
			string udn = sender.GetUPnPService().ParentDevice.UniqueDeviceName;

			lock (LockHashes)
			{
				InitStatus status = (InitStatus) UdnToInitStatus[udn];
				if (status != null)
				{
					status.ZeroMeansDone--;
					status.EventedCD = true;
					this.ProcessInitStatusChange(udn);
				}
			}
		}

		/// <summary>
		/// Executes when the ConnectionManager events for the first time.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="newVal"></param>
		private void Sink_OnCmEvented (CpConnectionManager sender, string newVal)
		{
			sender.OnStateVariable_SourceProtocolInfo -= new CpConnectionManager.StateVariableModifiedHandler_SourceProtocolInfo(this.Sink_OnCmEvented);
			string udn = sender.GetUPnPService().ParentDevice.UniqueDeviceName;

			lock (LockHashes)
			{
				InitStatus status = (InitStatus) UdnToInitStatus[udn];
				if (status != null)
				{
					status.ZeroMeansDone--;
					status.EventedCM = true;
					this.ProcessInitStatusChange(udn);
				}
			}
		}
		
		/// <summary>
		/// Whenever Sink_Onxxx method executes, it calls this method to
		/// determine whether a server has been upgraded from non-good
		/// to good status.
		/// </summary>
		/// <param name="udn"></param>
		private void ProcessInitStatusChange (string udn)
		{
			CpMediaServer addedThis = null;

			InitStatus status = null;
			lock (LockHashes)
			{
				status = (InitStatus) UdnToInitStatus[udn];

				if (status != null)
				{
					if (status.ZeroMeansDone == 0)
					{
						if (
							(status.EventedCD) &&
							(status.EventedCM) &&
							(status.SubcribeCD) &&
							(status.SubcribeCM)
							)
						{
							// We were evented for both services
							// and we subscribed successfully,
							// so we're good to go.
							UdnToInitStatus.Remove(udn);
							UdnToServer[udn] = status.Server;
							addedThis = status.Server;
						}
						else
						{
							// we didn't subscribe successfully
							// or we never got evented after
							// we subscribed... but this
							// code should never execute because
							// we will have never decremented
							// ZeroMeansDone==0
						}
					}

					if (addedThis == null)
					{
						if (BadServersAreGoodServersToo)
						{
							// but since we're configured to be
							// nice to crappy servers that 
							// don't event properly... we'll
							// promote the server to full status
							UdnToInitStatus.Remove(udn);
							UdnToServer[udn] = status.Server;
							addedThis = status.Server;
						}
					}
				}
			}

			if (addedThis != null)
			{
				if (this.OnCpServerAdded != null)
				{
					this.OnCpServerAdded(this, addedThis);
				}
			}
		}

		/// <summary>
		/// Method executes when smart control point notices that 
		/// a upnp  media server has left the network.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="device"></param>
		private void RemoveServer(UPnPSmartControlPoint sender, UPnPDevice device)
		{
			string udn = device.UniqueDeviceName;

			CpMediaServer removeThis = null;

			lock (LockHashes)
			{
				if (UdnToInitStatus.Contains(udn))
				{
					InitStatus status = (InitStatus) UdnToInitStatus[udn];
				}

				if (UdnToServer.Contains(udn))
				{
					removeThis = (CpMediaServer) UdnToServer[udn];
					UdnToServer.Remove(udn);
				}
			}

			if (this.OnServerGone != null)
			{
				this.OnServerGone(this, device);
			}

			if (removeThis != null)
			{
				if (this.OnCpServerRemoved != null)
				{
					this.OnCpServerRemoved(this, removeThis);
				}
			}

			System.GC.Collect();
		}

		/// <summary>
		/// Object is used a lock to force consistency in the
		/// hashtables.
		/// </summary>
		private object LockHashes = new object();

		/// <summary>
		/// Contains the init status for a server, keyed by the device's udn.
		/// </summary>
		private Hashtable UdnToInitStatus = new Hashtable();

		/// <summary>
		/// Hashtable of UDN to 
		/// <see cref="CpMediaServer"/>
		/// objects that are actively mirroring content hierarchies.
		/// </summary>
		private Hashtable UdnToServer = new Hashtable();
		
		/// <summary>
		/// UPNP smart control point that tells me when
		/// mediaservers show up.
		/// </summary>
		private UPnPSmartControlPoint m_Scp;

		/// <summary>
		/// If this is set to true, then all discovered
		/// bad servers will be treated as good servers.
		/// This value should not be switched on-off 
		/// regularly as it is not yet thread-safe.
		/// </summary>
		public const bool BadServersAreGoodServersToo = true;

		/// <summary>
		/// The subscribe status for a media server.
		/// </summary>
		private class InitStatus
		{
			public bool SubcribeCM;
			public bool SubcribeCD;

			public bool EventedCM;
			public bool EventedCD;

			public int ZeroMeansDone = 4;

			public CpMediaServer Server;
		}
	}
}
