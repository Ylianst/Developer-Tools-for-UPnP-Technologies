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
using System.Net;
using System.Xml;
using System.Text;
using OpenSource.UPnP;
using System.Threading;
using System.Net.Sockets;
using System.Collections;
using OpenSource.UPnP.AV;
using OpenSource.Utilities;
using OpenSource.UPnP.AV.CdsMetadata;

namespace OpenSource.UPnP.AV.MediaServer.DV
{
	/// <summary>
	/// <para>
	/// This class is responsible for taking a content hierarchy of 
	/// <see cref="IDvMedia"/>
	/// objects and advertising them on the UPNP network through
	/// a ContentDirectory service. Public programmers should build 
	/// a content hierarchy and use the 
	/// <see cref="DvRootContainer"/>
	/// object as the root container for all content. This root container
	/// is owned by the MediaServerDevice class and is not reassignable.
	/// </para>
	/// 
	/// <para>
	/// This class will automatically map local file resources with a ContentUri beginning with 
	/// <see cref="OpenSource.UPnP.AV.CdsMetadata.MediaResource.AUTOMAPFILE"/>
	/// to an http URL matching the interface on which a UPNP request came on. This prevents
	/// issues with advertising content with host-names that are DNS-resolvable or advertising
	/// content with IP addresses that are not routable from the requestor.
	/// </para>
	/// 
	/// <para>
	/// This class exposes a number of events that also assist in reporting activities 
	/// such as the transfering or uploading of files
	/// (<see cref="MediaServerDevice.OnHttpTransfersChanged"/>),
	/// the number of times each UPNP action has been executed on the MediaServer, 
	/// (<see cref="MediaServerDevice.OnStatsChanged"/>),
	/// as well as some delegates for when a UPNP control point has requested the MediaServerDevice 
	/// object to modify the content hierarchy in some way
	/// (<see cref="MediaServerDevice.OnRequestChangeMetadata"/>,
	/// (<see cref="MediaServerDevice.OnRequestAddBranch"/>,
	/// (<see cref="MediaServerDevice.OnRequestRemoveBranch"/>,
	/// (<see cref="MediaServerDevice.OnRequestDeleteBinary"/>, and
	/// (<see cref="MediaServerDevice.OnRequestSaveBinary"/>).
	/// These delegates are intentionally not events, as the MediaServerDevice depends on
	/// a timely response from a single owner and expects a 
	/// <see cref="OpenSource.UPnP.UPnPCustomException"/>
	/// to be thrown if the request is denied.
	/// </para>
	/// </summary>
	public class MediaServerDevice : IUPnPDevice
	{
		/// <summary>
		/// Call this to prevent memory leaks.
		/// </summary>
		public void Dispose()
		{
			this.m_Root.OnContainerChanged -= new DvRootContainer.Delegate_OnContainerChanged(this.Sink_ContainerChanged);
			this.m_LFT.OnExpired -= new LifeTimeMonitor.LifeTimeHandler(this.Sink_OnExpired);
		}

		/// <summary>
		/// This delegate is used when the server attempts to map a resource that should map to
		/// a local file, but cannot find the local file.
		/// If application logic determines that it has a stream it can send in response to the HTTP-GET
		/// request for non-mappable a resource using the <see cref="MediaResource.AUTOMAPFILE"/> convention,
		/// then it simply needs to set the <see cref="FileNotMapped.RedirectedStream"/>
		/// property on fileNotMapped to a non-null stream object. Otherwise, leaving it blank will
		/// return a 404 file not found message.
		/// </summary>
		public delegate void Delegate_FileNotMappedHandler (MediaServerDevice sender, FileNotMapped fileNotMapped);
		
		/// <summary>
		/// Fired when a file is requested for HTTP-GET, but the actual file was not
		/// found on the local system, so event the owner of the object, they may
		/// want to do something like provide a real-time stream that isn't stored.
		/// Methods subscribing to this event should handle the event quickly because
		/// the thread will have to do work upon returning.
		/// </summary>
		public Delegate_FileNotMappedHandler OnFileNotMapped;
		/// <summary>
		/// Delegate used to event generic MediaServerDevice events.
		/// </summary>
		public delegate void Delegate_MediaServerHandler (MediaServerDevice sender);
		/// <summary>
		/// Fired when a UPNP action is called, thus changing the statistics property for this MediaServerDevice.
		/// Changes are reflected in the <see cref="MediaServerDevice.Stats"/>property.
		/// </summary>
		public event Delegate_MediaServerHandler OnStatsChanged;
		/// <summary>
		/// Fired when a file starts to transfer, either for upload or download purposes. 
		/// Changes are reflected in the <see cref="MediaServerDevice.HttpTransfers"/>property.
		/// Methods subscribing to this event should handle the event quickly because
		/// the thread will have to do work upon returning.
		/// </summary>
		public event Delegate_MediaServerHandler OnHttpTransfersChanged;

		/// <summary>
		/// Delegate is used to event a requested change in metadata. 
		/// </summary>
		public delegate void Delegate_ChangeMetadata (MediaServerDevice sender, IDvMedia oldObject, IDvMedia newObject);
		/// <summary>
		/// <para>
		/// This event is fired when a control-point invokes the ContentDirectory.UpdateObject action.
		/// Public programmers will be able to compare two 
		/// <see cref="IDvMedia"/>
		/// objects to determine if the changes should occur. If the subscribe to this event
		/// rejects the change, the should throw a 
		/// <see cref="OpenSource.UPnP.UPnPCustomException"/> or throw one of the derived
		/// exceptions included with this library. Some suggested exceptions (although definitely not the definitive list) include the following.
		/// <list type="bullet">
		/// <item><description><see cref="Error_RequiredTag"/></description></item>
		/// <item><description><see cref="Error_ReadOnlyTag"/></description></item>
		/// <item><description><see cref="Error_RestrictedObject"/></description></item>
		/// <item><description><see cref="Error_BadMetadata"/></description></item>
		/// <item><description><see cref="Error_AccessDenied"/></description></item>
		/// </list>
		/// </para>
		/// 
		/// <para>
		/// Public programmers are strongly warned to NOT add the object with the proposed metadata
		/// values to the content hierarchy. This is an absolute NO-NO! The state of the 
		/// media object with the proposed metadata is consistent. It may own references to resources
		/// that are owned by the original/target object.
		/// </para>
		/// 
		/// <para>
		/// Public programmers comparing the resources of the objects can compare
		/// the resources by reference. Resources attached to the original/target object
		/// that are also attached to the object with the proposed metadata indicate
		/// that the resource will remain attached to the original. Resources
		/// missing from the object with the new metadata (but present in the
		/// original object) indicate that some resources have been proposed for removal.
		/// Resources present in the object with new metadata but not present in the
		/// original indicate a proposition to add resources.
		/// </para>
		/// </summary>
		public Delegate_ChangeMetadata OnRequestChangeMetadata;

		/// <summary>
		/// Delegate used a branch or leaf in the content hierarchy is requested for removal.
		/// </summary>
		public delegate void Delegate_RemoveBranch (MediaServerDevice sender, DvMediaContainer parentContainer, IDvMedia removeThisBranch);
		/// <summary>
		/// Delegate used when one or more branches in the content hierarchy are requested for addition.
		/// </summary>
		public delegate void Delegate_AddBranch (MediaServerDevice sender, DvMediaContainer parentContainer, ref IDvMedia[] addTheseBranches);

		/// <summary>
		/// <para>
		/// This delegate is executed when one or more branches in the content hierarchy are requested for removal
		/// through a control point's invocation of the ContentDirectory.DestroyObject action.
		/// Public programmers will get a single
		/// <see cref="IDvMedia"/> object as well as the 
		/// <see cref="DvMediaContainer"/> object to remove the branch from.
		/// Public programmers should throw a 
		/// <see cref="OpenSource.UPnP.UPnPCustomException"/>
		/// or throw one the standardized exceptions to reject the request. Generally, the following
		/// exceptions will suffice, but public programmers may find a need to define their own.
		/// <list type="bullet">
		/// <item><description><see cref="Error_RestrictedObject"/></description></item>
		/// <item><description><see cref="Error_AccessDenied"/></description></item>
		/// </list>
		/// </para>
		/// <para>
		/// If no exceptions are thrown by the method consuming this event, then the MediaServerDevice
		/// object will simply respond with a result indicating the item was destroyed. 
		/// The method consuming this event IS RESPONSIBLE FOR HANDLING TARGET OBJECT REMOVAL. This
		/// also includes the implementation of whether or not resources are physically deleted
		/// from the disk (or simply the resources
		/// </para>
		/// <para>
		/// This asymmetrical content-management policy for insertion and removal branches is intentional.
		/// In the case of insertion, the owner code must accept or reject the entire proposal
		/// to add new branches because the UPNP response sent on the wire must assume an atomic
		/// nature to the request. In the case of removal, the UPNP request only deals with one
		/// object so the request is either accepted or rejected. However objects may have resources 
		/// that may need to be deleted from the local file system. 
		/// (The behavior to delete actual binaries not a required. Rather it is a design decision for whoever builds a component that uses 
		/// MediaServerDevice object). For this reason, MediaServerDevice expects the owner of 
		/// MediaServerDevice to properly remove the object from the parent container if 
		/// approving the request.
		/// </para>
		/// <para>
		/// Public programmers should take note that the target parent container may also be the
		/// <see cref="DvRootContainer"/> object owned by the
		/// <see cref="MediaServerDevice"/> object.
		/// </para>
		/// </summary>
		public Delegate_RemoveBranch OnRequestRemoveBranch;
		
		/// <summary>
		/// <para>
		/// This delegate is executed when one or more branches in the content hierarchy are requested for addition
		/// through the ContentDirectory.CreateObject action. Although the ContentDirectory.CreateObject action
		/// only supports the ability to create a single object in a single call, this implementation of ContentDirectory
		/// will properly support the extended ability to create multiple objects at once. In such a scenario
		/// the control-point application will need to implement logic to understand the multiple-item
		/// response as well as the comma-separated value list of multiple object IDs. If the control-point
		/// opts to create a single object, the response will conform to the standard ContentDirectory
		/// conventions for ContentDirectory.CreateObject.
		/// </para>
		/// 
		/// <para>
		/// A method wired to this event will get an array of 
		/// <see cref="IDvMedia"/> objects as well as the intended
		/// <see cref="DvMediaContainer"/> object to act as the parent container.
		/// Public programmers should throw a 
		/// <see cref="OpenSource.UPnP.UPnPCustomException"/>
		/// or throw one the standardized exceptions to reject the request. Generally, the following
		/// exceptions will suffice, but public programmers may find a need to define their own.
		/// <list type="bullet">
		/// <item><description><see cref="Error_RestrictedObject"/></description></item>
		/// <item><description><see cref="Error_BadMetadata"/></description></item>
		/// <item><description><see cref="Error_AccessDenied"/></description></item>
		/// </list>
		/// </para>
		/// <para>
		/// If no exceptions are thrown by the method consuming this event, then the MediaServerDevice
		/// object will automatically add every proposed branch to the list. The method consuming
		/// this event SHOULD NOT ADD THE BRANCHES. The method consuming this event must accept
		/// or reject all proposed branches, as the AddBranches operation is atomic.
		/// </para>
		/// <para>
		/// Public programmers should take note that the target parent container may also be the
		/// <see cref="DvRootContainer"/> object owned by the
		/// <see cref="MediaServerDevice"/> object.
		/// </para>
		/// </summary>
		public Delegate_AddBranch OnRequestAddBranch;

		/// <summary>
		/// TODO: Allow a feature to redirect a stream, in the case of saving binaries.
		/// This delegate is used when a control point requests the removal/creation/overwrite of a physical binary.
		/// </summary>
		public delegate void Delegate_ModifyBinary (MediaServerDevice sender, IDvResource resource);
		
		/// <summary>
		/// <para>
		/// This delegate is called when a control-point invokes the ContentDirectory.DeleteResource action.
		/// The method wired to this delegate will get the resource that is requested for deleting.
		/// Whether or not the resource is physically deleted is decided by the implementer using
		/// the MediaServerDevice object.
		/// </para>
		/// <para>
		/// Public programmers should throw a 
		/// <see cref="OpenSource.UPnP.UPnPCustomException"/>
		/// or throw one the standardized exceptions to reject the request. Generally, the following
		/// exceptions will suffice, but public programmers may find a need to define their own.
		/// <list type="bullet">
		/// <item><description><see cref="Error_RestrictedObject"/></description></item>
		/// <item><description><see cref="Error_BadMetadata"/></description></item>
		/// <item><description><see cref="Error_AccessDenied"/></description></item>
		/// </list>
		/// </para>
		/// </summary>
		public Delegate_ModifyBinary OnRequestDeleteBinary;
		
		/// <summary>
		/// <para>
		/// This delegate is called when a control-point uses HTTP-POST to post a binary or uses
		/// the ContentDirectory.ImportResource action, in an attempt to save a binary to 
		/// an actual resource. Methods wired to this delegate can choose to set the
		/// stream redirection field to a stream
		/// object if the posted/imported binary if the public programmer wants do
		/// something with the file (or to prevent the existing file from being deleted). This
		/// is also useful in when an advertised resource is not HTTP-GET based, but the
		/// programmer wants the resource's binary to have an importUri. 
		/// Keep in mind that if the stream is redirected to something other than the local file
		/// that has been automapped (<see cref="MediaResource.AUTOMAPFILE"/>)
		/// from <see cref="IDvResource.ContentUri"/>, the posted binary
		/// content will not be applied.
		/// </para>
		/// </summary>
		public Delegate_ModifyBinary OnRequestSaveBinary;

		/// <summary>
		/// Returns the UPNP device for this object. Use at your own risk.
		/// </summary>
		public UPnPDevice _Device { get { return this.Device; } }

		/// <summary>
		/// For non-HTTP-get PrepareForConnection requests, this delegate is used.
		/// </summary>
		public delegate void Delegate_PrepareForConnection(System.String RemoteProtocolInfo, System.String PeerConnectionManager, DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, out System.Int32 AVTransportID, out System.Int32 RcsID, out DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus status);

		/// <summary>
		/// If the owner handles its own HTTP connections or any non-HTTP
		/// connections, then this field must be non-null.
		/// 
		/// If a call to PrepareForConnection should not succeed, the executed delegate
		/// should throw a UPnPCustomException() object that has been instantiated
		/// with the correct error code and message.
		/// </summary>
		public Delegate_PrepareForConnection OnCallPrepareForConnection;

		/// <summary>
		/// Contains a listing of ProtocolInfoString objects
		/// that are valid with this device as a source.
		/// The owner may set these appropriately, but
		/// the default is simply http-get:*:*:*.
		/// </summary>
		public ProtocolInfoString[] SourceProtocolInfoSet
		{
			get
			{
				ProtocolInfoString[] protSet = null;
				this.GetProtocolInfoSet(true);
				return protSet;
			}
			set
			{
				ProtocolInfoString[] protSet = value;
				this.UpdateProtocolInfoSet(true, protSet);
			}
		}

		/// <summary>
		/// Contains a listing of ProtocolInfoString objects
		/// that are valid with this device as a sink. The
		/// owner may set these appropriately, but
		/// the default is empty.
		/// 
		/// Although this is a MediaServerDevice, it has the
		/// capacity to allow AVTransport and RenderingControl
		/// services to enable it to be a MediaRenderer as well.
		/// </summary>
		public ProtocolInfoString[] SinkProtocolInfoSet
		{
			get
			{
				ProtocolInfoString[] protSet = null;
				this.GetProtocolInfoSet(false);
				return protSet;
			}
			set
			{
				ProtocolInfoString[] protSet = value;
				this.UpdateProtocolInfoSet(false, protSet);
			}
		}

		/// <summary>
		/// Provides a list of all active connections.
		/// </summary>
		public Connection[] Connections
		{
			get
			{
				this.m_LockConnections.AcquireReaderLock(-1);
				Connection[] conns = new Connection [ this.m_Connections.Count ];
				int i=0;
				foreach (UInt32 id in this.m_Connections.Keys)
				{
					conns[i] = (Connection) this.m_Connections[id];
					i++;
				}
				this.m_LockConnections.ReleaseReaderLock();

				return conns;
			}
		}
		
		/// <summary>
		/// Returns the underlying UPNP device object.
		/// </summary>
		/// <returns></returns>
		public UPnPDevice GetUPnPDevice() { return this.Device; }

		/// <summary>
		/// MediaServers can implement the AVTransport service.
		/// This server doesn't implement that yet.
		/// </summary>
//		public IList AVTransports
//		{
//			get
//			{
//				this.m_LockTransports.AcquireReaderLock(-1);
//
//				IList transports = this.m_AVTransports.ToArray(this.m_AVTransports[0].GetType());
//
//				this.m_LockTransports.ReleaseReaderLock();
//
//				return transports;
//			}
//		}

		/// <summary>
		/// Returns the root container for the media server. 
		/// Programmers can add their containers and item and build
		/// a content hierarchy for the server using this property
		/// as the root container.
		/// </summary>
		public DvMediaContainer Root { get { return this.m_Root; } }

		/// <summary>
		/// Returns a list of 
		/// <see cref="MediaServerDevice.HttpTransfer"/>
		/// .
		/// </summary>
		public IList HttpTransfers 
		{ 
			get 
			{
				this.m_LockHttpTransfers.AcquireReaderLock(-1);
				ArrayList results = new ArrayList(this.m_HttpTransfers.Count);
				results.AddRange(this.m_HttpTransfers.Values);
				this.m_LockHttpTransfers.ReleaseReaderLock();
				return results;
			} 
		}

		/// <summary>
		/// Returns the statistics of all calls made on the object from the UPNP interface.
		/// </summary>
		public Statistics Stats
		{
			get
			{
				return this.m_Stats;
			}
		}

		private LifeTimeMonitor.LifeTimeHandler LTMDelegate = null;
		/// <summary>
		/// Constructor instantiates a properly behaving UPnP-AV
		/// MediaServer device that has a root container.
		/// </summary>
		/// <param name="info">general information about the MediaServer like manufacturer info</param>
		/// <param name="isRootDevice">true if the MediaServer has no parent UPnP device</param>
		/// <param name="enableHttpContentServing">
		/// True, if the MediaServer should support the <see cref="MediaResource.AUTOMAPFILE"/>
		/// convention.
		/// </param>
		/// <param name="initialSourceProtocolInfoSet">
		/// Comma separated value list of protocolInfo strings for this MediaServer as a source.
		/// "HTTP-GET:*:*:*" is also allowed, indicating that any HTTP-GET resource is supported on the server.
		/// Generally, this value should be true unless the application logic for the
		/// MediaServer will always create resource objects with fully pathed URIs that are
		/// accessible from the UPNP network.
		/// </param>
		/// <param name="initialSinkProtocolInfoSet">
		/// Comma separated value list of protocolInfo strings for this MediaServer as a sink.
		/// This should generally be blank although it may be possible to eventually 
		/// migrate this implementation to behave both as a MediaRenderer and a MediaServer,
		/// although there may be subtleties that make such a device impossible.
		/// </param>
		public MediaServerDevice
			(
			DeviceInfo info,
			UPnPDevice parent,
			bool enableHttpContentServing,
			string initialSourceProtocolInfoSet,
			string initialSinkProtocolInfoSet
			)
		{
			// enable HTTP webserving of HTTP-GET resource/content?
			this.EnableHttp = enableHttpContentServing;

			// Wire up the delegate and weak event used when an HTTP transfer should be
			// removed from the server's list. 
			// When an HTTP transfer completes it's progress info still needs to be 
			// available for GetTransferProgress action for at least 30 seconds. 
			// LifeTimeMonitor will delay the removal for such a time, and then
			// execute the delegate.
			this.LTMDelegate = new LifeTimeMonitor.LifeTimeHandler(this.Sink_OnExpired);
			this.m_LFT.OnExpired += LTMDelegate;

			// Create the UPnP device object - no servics are attached yet
			if (parent == null)
			{
				this.Device = UPnPDevice.CreateRootDevice(info.CacheTime, 1.0, info.LocalRootDirectory);
				if(info.CustomUDN!="")
				{
					this.Device.UniqueDeviceName = info.CustomUDN;
				}
			}
			else
			{
				Guid udn = System.Guid.NewGuid();
				this.Device = UPnPDevice.CreateEmbeddedDevice(1.0, udn.ToString());
				parent.AddDevice(this.Device);
			}

			// transfer basic info about the device, like serial #, manufacturer, etc.
			this.Device.HasPresentation = false;
			this.Device.StandardDeviceType = "MediaServer";

			this.Device.FriendlyName		= info.FriendlyName;
			this.Device.Manufacturer		= info.Manufacturer;
			this.Device.ManufacturerURL	= info.ManufacturerURL;
			this.Device.ModelName			= info.ModelName;
			this.Device.ModelDescription	= info.ModelDescription;
			if (info.ModelURL != null)
			{
				try
				{
					this.Device.ModelURL			= new Uri(info.ModelURL);
				}
				catch
				{
					this.Device.ModelURL = null;
				}
			}
			this.Device.ModelNumber		= info.ModelNumber;

			if (info.INMPR03)
			{
				this.Device.AddCustomFieldInDescription("INMPR03", "1.0", "");
			}

			this.ConnectionManager = new DvConnectionManager();
			this.ContentDirectory = new DvContentDirectory();

			// Set periodic behavior for the moderated state variables.
			// Only state variables that do not overwrite a pending
			// value need an accumulator.
			// 
			this.ContentDirectory.ModerationDuration_SystemUpdateID = 2;
			this.ContentDirectory.ModerationDuration_ContainerUpdateIDs = 2;
			this.ContentDirectory.Accumulator_ContainerUpdateIDs = new Accumulator_ContainerUpdateIDs();

			// Determine whether the application logic actually
			// wants control points to have access to content management
			// related methods. If not, then remove those actions.
			if (info.AllowRemoteContentManagement==false)
			{
				this.ContentDirectory.RemoveAction_CreateObject();
				this.ContentDirectory.RemoveAction_CreateReference();
				this.ContentDirectory.RemoveAction_DeleteResource();
				this.ContentDirectory.RemoveAction_DestroyObject();
				this.ContentDirectory.RemoveAction_ImportResource();
				this.ContentDirectory.RemoveAction_UpdateObject();
				this.ContentDirectory.RemoveAction_ExportResource();
				this.ContentDirectory.RemoveAction_GetTransferProgress();
				this.ContentDirectory.RemoveAction_StopTransferResource();
			}

			if (info.EnablePrepareForConnection==false)
			{
				this.ConnectionManager.RemoveAction_PrepareForConnection();
			}

			if (info.EnableConnectionComplete==false)
			{
				this.ConnectionManager.RemoveAction_ConnectionComplete();
			}

			if (
				(info.EnablePrepareForConnection==false) &&  
				(info.EnableConnectionComplete==false)
				)
			{
				ProtocolInfoString protInfo = new ProtocolInfoString("http-get:*:*:*");
				DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus status = DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus.UNKNOWN;

				Connection newConnection = new Connection(GetConnectionID(), -1, -1, -1, protInfo, "/", OpenSource.UPnP.AV.DvConnectionManager.Enum_A_ARG_TYPE_Direction.OUTPUT, OpenSource.UPnP.AV.DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus.UNKNOWN);
				this.AddConnection(newConnection);
			}

			if (info.EnableSearch == false)
			{
				this.ContentDirectory.RemoveAction_Search();
			}

			this.m_SearchCapabilities = info.SearchCapabilities;
			this.m_SortCapabilities = info.SortCapabilities;
			
			//AVTransport and RendererControl not used...yet.
			//this.m_AVTransports = new ArrayList();
			//this.m_RenderingControls = new ArrayList();

			// Set values for each state variable
			// 
			if (this.ConnectionManager.Evented_CurrentConnectionIDs == null)
			{
				this.ConnectionManager.Evented_CurrentConnectionIDs = "";
			}
			
			// this state variable does not exist until UPNP-AV 1.1
			//this.ConnectionManager.Evented_PhysicalConnections = "";

			// have the device advertise the protocolInfo strings for 
			// this media server
			this.UpdateProtocolInfoSet(false, initialSinkProtocolInfoSet);
			this.UpdateProtocolInfoSet(true, initialSourceProtocolInfoSet);

			// initialize state varaibles
			this.ContentDirectory.Evented_ContainerUpdateIDs = "";
			this.ContentDirectory.Evented_SystemUpdateID = 0;
			this.ContentDirectory.Evented_TransferIDs = "";

			// Wire up the ContentDirectory and ConnectionManager actions to actual methods
			// in this class that will actually do the work.
			this.ConnectionManager.External_ConnectionComplete			= new DvConnectionManager.Delegate_ConnectionComplete(this.SinkCm_ConnectionComplete);
			this.ConnectionManager.External_GetCurrentConnectionIDs		= new DvConnectionManager.Delegate_GetCurrentConnectionIDs(this.SinkCm_GetCurrentConnectionIDs);
			this.ConnectionManager.External_GetCurrentConnectionInfo	= new DvConnectionManager.Delegate_GetCurrentConnectionInfo(this.SinkCm_GetCurrentConnectionInfo);
			this.ConnectionManager.External_GetProtocolInfo				= new DvConnectionManager.Delegate_GetProtocolInfo(this.SinkCm_GetProtocolInfo);
			this.ConnectionManager.External_PrepareForConnection		= new DvConnectionManager.Delegate_PrepareForConnection(this.SinkCm_PrepareForConnection);

			this.ContentDirectory.External_Browse					= new DvContentDirectory.Delegate_Browse(this.SinkCd_Browse);
			this.ContentDirectory.External_CreateObject				= new DvContentDirectory.Delegate_CreateObject(this.SinkCd_CreateObject);
			this.ContentDirectory.External_CreateReference			= new DvContentDirectory.Delegate_CreateReference(this.SinkCd_CreateReference);
			this.ContentDirectory.External_DeleteResource			= new DvContentDirectory.Delegate_DeleteResource(this.SinkCd_DeleteResource);
			this.ContentDirectory.External_DestroyObject			= new DvContentDirectory.Delegate_DestroyObject(this.SinkCd_DestroyObject);
			this.ContentDirectory.External_ExportResource			= new DvContentDirectory.Delegate_ExportResource(this.SinkCd_ExportResource);
			this.ContentDirectory.External_GetSearchCapabilities	= new DvContentDirectory.Delegate_GetSearchCapabilities(this.SinkCd_GetSearchCapabilities);
			this.ContentDirectory.External_GetSortCapabilities		= new DvContentDirectory.Delegate_GetSortCapabilities(this.SinkCd_GetSortCapabilities);
			this.ContentDirectory.External_GetSystemUpdateID		= new DvContentDirectory.Delegate_GetSystemUpdateID(this.SinkCd_GetSystemUpdateID);
			this.ContentDirectory.External_GetTransferProgress		= new DvContentDirectory.Delegate_GetTransferProgress(this.SinkCd_GetTransferProgress);
			this.ContentDirectory.External_ImportResource			= new DvContentDirectory.Delegate_ImportResource(this.SinkCd_ImportResource);
			this.ContentDirectory.External_Search					= new DvContentDirectory.Delegate_Search(this.SinkCd_Search);
			this.ContentDirectory.External_StopTransferResource		= new DvContentDirectory.Delegate_StopTransferResource(this.SinkCd_StopTransferResource);
			this.ContentDirectory.External_UpdateObject				= new DvContentDirectory.Delegate_UpdateObject(this.SinkCd_UpdateObject);

			// add the services to the device - voila, it's now a useful device
			this.Device.AddService(this.ConnectionManager);
			this.Device.AddService(this.ContentDirectory);

			// set up a virtual directory for local webserving - we'll always
			// have this here, even if HTTP webserving is not enabled because
			// it really doesn't hurt to have the virtual directory.
			Interlocked.Increment(ref VirtualDirCounter);
			m_VirtualDirName = "MediaServerContent_" + VirtualDirCounter.ToString();
			this.Device.AddVirtualDirectory(m_VirtualDirName, new UPnPDevice.VirtualDirectoryHandler(this.WebServer_OnHeaderReceiveSink), new UPnPDevice.VirtualDirectoryHandler(this.WebServer_OnPacketReceiveSink));

			// create the root container for this media server's content hierarchy.
			// Prevent control points from modifying this root container or
			// creating new objects in the root.
			// Also need to wire up internally visible events so that the MediaServer
			// will properly event changes in the content hierarchy.
			MediaBuilder.container rootInfo = new MediaBuilder.container("Root");
			rootInfo.Searchable = true;
			rootInfo.IsRestricted = true;
			this.m_Root = (DvRootContainer) DvMediaBuilder.CreateRoot(rootInfo);
			this.m_Root.OnContainerChanged += new DvRootContainer.Delegate_OnContainerChanged(this.Sink_ContainerChanged);

			// At this point the device is ready to go... simply call the Start()
			// method to have the device advertise itself.
		}

		/// <summary>
		/// Allows the server to start up or restart.
		/// </summary>
		public void Start()
		{
			this.Device.StartDevice();
		}

		/// <summary>
		/// Allows the server to start up or restart with a specific IP port number.
		/// </summary>
		/// <param name="portNumber"></param>
		public void Start(int portNumber)
		{
			this.Device.StartDevice(portNumber);
		}

		/// <summary>
		/// Pauses the device/ eg, stops advertising itself
		/// and does not allow anybody to use it from the
		/// UPNP network. However the content hierarchy is 
		/// still preserved.
		/// </summary>
		public void Stop()
		{
			this.Device.StopDevice();
		}


		/// <summary>
		/// Method executes when the root container indicates that a descendent container has changed.
		/// Method's purpose is to cause the ContainerUpdateIDs state variable to event
		/// the change.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="thisChanged"></param>
		private void Sink_ContainerChanged(DvRootContainer sender, DvMediaContainer thisChanged)
		{
			this.Lock_SystemUpdateID.WaitOne();
			this.ContentDirectory.Evented_SystemUpdateID = this.ContentDirectory.Evented_SystemUpdateID + 1;
			this.Lock_SystemUpdateID.ReleaseMutex();
			
			this.Lock_ContainerUpdateIDs.WaitOne();
			StringBuilder sb = new StringBuilder(20);
			sb.AppendFormat("{0}{1}{2}", thisChanged.ID, Accumulator_ContainerUpdateIDs.Delimitor, thisChanged.UpdateID);
			this.ContentDirectory.Evented_ContainerUpdateIDs = sb.ToString();
			this.Lock_ContainerUpdateIDs.ReleaseMutex();
		}

		/// <summary>
		/// Method executes when a control point invokes the ConnectionManager.ConnectionComplete action.
		/// Purpose is simply to decrement the number of active connections.
		/// </summary>
		/// <param name="ConnectionID"></param>
		private void SinkCm_ConnectionComplete(System.Int32 ConnectionID)
		{
			if (ConnectionID > Int32.MaxValue)
			{
				throw new Error_InvalidConnection("("+ConnectionID+")");
			}
			
			int id = (Int32) ConnectionID;
			this.m_LockConnections.AcquireWriterLock(-1);
			if (this.m_Connections.ContainsKey(id))
			{
				Connection theConn = (Connection) this.m_Connections[id];
				this.RemoveConnection(theConn);
			}
			this.m_Stats.ExportResource++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ConnectionManager.GetCurrrentConnectionIDs action.
		/// Purpose is simply to return the current value of the CurrentConnectionIDs state variable.
		/// </summary>
		/// <param name="ConnectionIDs"></param>
		private void SinkCm_GetCurrentConnectionIDs(out System.String ConnectionIDs)
		{
			ConnectionIDs = this.ConnectionManager.Evented_CurrentConnectionIDs;
			this.m_Stats.GetCurrentConnectionIDs++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ConnectionManager.GetCurrentConnectionInfo action.
		/// The method will attempt to find the Renderercontrol, AVTransport, peer connection, and other 
		/// information about a specified connection. If it fails, we report that the
		/// specified connection doesn't exist.
		/// </summary>
		/// <param name="ConnectionID">Find info about a connection with this ID.</param>
		/// <param name="RcsID">Return a RendererControl instance ID for the connection.</param>
		/// <param name="AVTransportID">Return a AVTransport instance ID for the connection.</param>
		/// <param name="ProtocolInfo">Return the protocolInfo string used at connection creation time.</param>
		/// <param name="PeerConnectionManager">Return the ConnectionManager UDN/service ID for the connection.</param>
		/// <param name="PeerConnectionID">Return the Connection ID used by the peer ConnectionManager for this connection.</param>
		/// <param name="Direction">Return the input/output value for this server's role. Should be output.</param>
		/// <param name="Status">Returns connection status information.</param>
		private void SinkCm_GetCurrentConnectionInfo(System.Int32 ConnectionID, out System.Int32 RcsID, out System.Int32 AVTransportID, out System.String ProtocolInfo, out System.String PeerConnectionManager, out System.Int32 PeerConnectionID, out DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, out DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus Status)
		{
			if (m_Connections.ContainsKey(ConnectionID))
			{
				Connection c = (Connection) m_Connections[ConnectionID];
				
				RcsID = c.RcsId;
				AVTransportID = c.AVTransportId;
				ProtocolInfo = c.ProtocolInfo.ToString();
				PeerConnectionManager = c.PeerConnectionManager;
				PeerConnectionID = c.PeerConnectionId;
				Direction = c.Direction;
				Status = c.Status;
			}
			else
			{
				throw new Error_InvalidConnection("("+ConnectionID+")");
			}
			this.m_Stats.GetCurrentConnectionInfo++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ConnectionManager.GetProtocolInfo action.
		/// Method returns the current values of the SourceProtocolInfo and SinkProtocolInfo state variables.
		/// </summary>
		/// <param name="Source"></param>
		/// <param name="Sink"></param>
		private void SinkCm_GetProtocolInfo(out System.String Source, out System.String Sink)
		{
			Source = this.ConnectionManager.Evented_SourceProtocolInfo;
			Sink = this.ConnectionManager.Evented_SinkProtocolInfo;
			this.m_Stats.GetProtocolInfo++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ConnectionManager.PrepareForConnection action.
		/// The method will attempt to validate the connection request, reporting an error if
		/// the connection type is not supported. If the connection type is supported, but
		/// it is not based on HTTP-GET we execute the <see cref="MediaServerDevice.OnCallPrepareForConnection"/>
		/// delegate and ask upper-layer software to accept or reject the request.
		/// If the upper-layer is to reject, then it must throw a UPnPCustomException() that provides
		/// a reason for the failure.
		/// </summary>
		/// <param name="RemoteProtocolInfo">The protocolInfo string that identifies transport/network/mime-type/info for the proposed connection.</param>
		/// <param name="PeerConnectionManager">The UDN/Service-ID of the ConnectionManager service at the other end of the connection.</param>
		/// <param name="PeerConnectionID">The connection ID returned by the PrepareForConnection invocation on the other endpoint. Can be empty if unknown.</param>
		/// <param name="Direction">Should generally by OUTPUT.</param>
		/// <param name="ConnectionID">Returns ConnectionID used by this mediaserver to represent the new connection.</param>
		/// <param name="AVTransportID">
		/// Returns AVTransport instance ID used by this mediaserver to represent the new connection.
		/// Value is -1 for HTTP-GET content on a server.
		/// </param>
		/// <param name="RcsID">
		/// Returns RendererControl instance ID used by this mediaserver to represent the new connection.
		/// Value is -1 for HTTP-GET content on a server.
		/// </param>
		private void SinkCm_PrepareForConnection(System.String RemoteProtocolInfo, System.String PeerConnectionManager, System.Int32 PeerConnectionID, DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction, out System.Int32 ConnectionID, out System.Int32 AVTransportID, out System.Int32 RcsID)
		{
			bool ownerProvidesIdInfo = true;
			ProtocolInfoString protInfo = new ProtocolInfoString(RemoteProtocolInfo);

			// Throw an exception if the protocol and direction are not supported by this device.
			// 
			ValidateConnectionRequest(protInfo, Direction);

			// If the instance is configured to serve local HTTP content,
			// then we intercept any PrepareForConnection() invocation
			// that involves HTTP-GET / OUTPUT and create a new ConnectionID,
			// and provide -1 for AVT and RCS instance IDs.
			// 
			// In all other cases, we fire an event asking the owner to
			// provide this information.
			// 
			if ((this.EnableHttp) && (String.Compare(protInfo.Protocol, "http-get", true) == 0) && (Direction == DvConnectionManager.Enum_A_ARG_TYPE_Direction.OUTPUT))
			{
				ownerProvidesIdInfo = false;
			}

			// Obtain a valid connection ID regardless of whether the owner will provide
			// an AVTransportID and/or a RcsID.
			// 
			ConnectionID = GetConnectionID();
			DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus status = DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus.UNKNOWN;

			if (ownerProvidesIdInfo)
			{
				if (this.OnCallPrepareForConnection != null)
				{
					// Application logic needs to provide a instance IDs for AVTransport and Rendering Control services, as well as
					// an initial value for the status.
					this.OnCallPrepareForConnection(RemoteProtocolInfo, PeerConnectionManager, Direction, out AVTransportID, out RcsID, out status);
				}
				else
				{
					throw new Error_InvalidServerConfiguration("PrepareForConnection() cannot be supported until the vendor configures the server correctly.");
				}
			}
			else
			{
				AVTransportID = -1;
				RcsID = -1;
			}

			Connection newConnection = new Connection((int)ConnectionID, PeerConnectionID, RcsID, AVTransportID, protInfo, PeerConnectionManager, Direction, status);
			this.AddConnection(newConnection);
			this.m_Stats.PrepareForConnection++;
			this.FireStatsChange();
		}


		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.Browse action.
		/// Depending on the input parameters, the method either returns the metadata
		/// for the specified object, or the method returns the metadata of all child objects
		/// if direct children metadata was requested and the specified object is actually
		/// a container.
		/// </summary>
		/// <param name="objectID">Browse the metadata or the children of this object</param>
		/// <param name="browseFlag">Indicate whether metadata or child object metadata is desired</param>
		/// <param name="filter">Comma separated value list of metadata properties indicates desired metadata properties to include in response, use * for all.</param>
		/// <param name="startingIndex">Given the entire possible response set, return a subset beginning with this index in the result set.</param>
		/// <param name="requestedCount">Given the entire possible response set, return a subset totalling no more than this many.</param>
		/// <param name="sortCriteria">Specify a comma-separated value list of metadata properties, with a + or - char before each property name to indicate ascending/descending order.</param>
		/// <param name="Result">DIDL-Lite response for desired result set.</param>
		/// <param name="numberReturned">Number of media objects returned in the response. 0 if browsing object metadata.</param>
		/// <param name="totalMatches">Total number of media objects in entire result set. 0 if browsing object metadata.</param>
		/// <param name="updateID">The UpdateID of the object - ignore if an object.item entry. Applies only to object.container entries.</param>
		private void SinkCd_Browse(System.String objectID, DvContentDirectory.Enum_A_ARG_TYPE_BrowseFlag browseFlag, System.String filter, System.UInt32 startingIndex, System.UInt32 requestedCount, System.String sortCriteria, out System.String Result, out System.UInt32 numberReturned, out System.UInt32 totalMatches, out System.UInt32 updateID)
		{
			try
			{
				numberReturned = 0;
				Result = "";
				totalMatches = 0;

				if (requestedCount == 0)
				{
					requestedCount = Convert.ToUInt32(int.MaxValue);
				}

				// Get the item identified by the ID, or throw an exception
				// if the item doesn't exist.
				// 
				IDvMedia entry = this.GetCdsEntry(objectID);

				// Issue a browse on the entry if it's a container and we're browing children.
				// Return the results in entries. Apply sorting if appropriate.
				// 
				IList entries;
				if ((entry.IsContainer) && (browseFlag == DvContentDirectory.Enum_A_ARG_TYPE_BrowseFlag.BROWSEDIRECTCHILDREN))
				{
					DvMediaContainer container = (DvMediaContainer) entry;
					if (sortCriteria.Trim() == "")
					{
						entries = container.Browse(startingIndex, requestedCount, out totalMatches);
					}
					else
					{
						MediaSorter sorter = new MediaSorter(true, sortCriteria);
						entries = container.BrowseSorted(startingIndex, requestedCount, sorter, out totalMatches);
					}

					numberReturned = Convert.ToUInt32(entries.Count);
					updateID = container.UpdateID;
				}
				else
				{
					// We're browsing an item or a container's metadata, so simply set the entry to be
					// the only return value.
					// 
					entries = new ArrayList();
					entries.Add(entry);
					totalMatches = 1;
					numberReturned = 1;
					IDvMedia dvMedia = (IDvMedia) entry;
					DvMediaContainer container = dvMedia as DvMediaContainer;

					if (container == null)
					{
						container = (DvMediaContainer) dvMedia.Parent;
					}

					updateID = container.UpdateID;
				}
				
				// Get the XML response for this result set.
				// Be sure to grab the list of base URLs.
				ArrayList properties = GetFilters(filter);
				string[] baseUrls = GetBaseUrlsByInterfaces();
				Result = BuildXmlRepresentation(baseUrls, properties, entries);
			}
			catch (Exception e)
			{
				Exception ne = new Exception("MediaServerDevice.SinkCd_Browse()", e);
				throw ne;
			}
			this.m_Stats.Browse++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Static variable is here for legacy implementation issues.
		/// </summary>
		private static bool ENCODE_UTF8 = MediaObject.ENCODE_UTF8;
		/// <summary>
		/// Static variable is here for legacy implementation issues.
		/// </summary>
		private static int XML_BUFFER_SIZE = MediaObject.XML_BUFFER_SIZE;

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.CreateObject action.
		/// The invoker is supposed to provide most of the valid DIDL-Lite XML for the new object.
		/// This method will construct a new media object, assign a unique object ID, 
		/// assign appropriate importUri and contentUri values to empty resource attributes
		/// and then output the new object ID as well as the current DIDL-Lite representaiton
		/// of the object.
		/// <para>
		/// The method supports a vendor specific override, where multiple objects can be
		/// created in a single call, by means of using the same syntax as the normal, except
		/// container elements can have child item/container elements, representing
		/// an entire subtree. Can also have multiple item or container elements under DIDL-Lite
		/// tag to indicate multiple subtrees.
		/// </para>
		/// <para>
		/// The method supports a vendor specific override, where resource elements in the
		/// DIDL-Lite can specify the <see cref="MediaResource.AUTOMAPFILE"/> convention
		/// to automatically add locally mapped files. This does assume the control poitn
		/// has intimate knowledge about the file system used by this media server.
		/// </para>
		/// </summary>
		/// <param name="containerID">Create new object in this container.</param>
		/// <param name="Elements">Partial DIDL-Lite document to create the actual object(s).</param>
		/// <param name="objectID">Object ID for created media object.</param>
		/// <param name="Result">DIDL-Lite response for the created object(s).</param>
		private void SinkCd_CreateObject(System.String containerID, System.String Elements, out System.String objectID, out System.String Result)
		{
			try
			{
				DvMediaContainer parent = this.GetContainer(containerID);

				if (parent == null)
				{
					throw new Error_NoSuchObject("The container \""+containerID+"\" does not exist.");
				}

				// builds new subtrees under containerID using the XML in Elements.
				ArrayList newBranches = DvMediaBuilder.BuildMediaBranches(Elements);

				// At this point, newBranches contains a list of IDvMedia objects
				// that are proposed for addition to the content hierarchy.

				if (this.OnRequestAddBranch != null)
				{
					IDvMedia[] addTheseBranches = new IDvMedia[newBranches.Count];
					for (int x=0; x < newBranches.Count; x++)
					{
						addTheseBranches[x] = (IDvMedia) newBranches[x];
					}
					newBranches = null;
					this.OnRequestAddBranch (this, parent, ref addTheseBranches);

					// Reflect the new branches in the tree.
					// 
					foreach (IDvMedia branch in addTheseBranches)
					{
						// Use AddObject() instead of AddBranch()
						// because the branch already has a unique ID
						// generated from MediaBuilder.GetUniqueId().

						//parent.AddBranch(branch);
						parent.AddObject(branch, false);
					}

					// Initialize structures used to process output for this method.
					// 
					StringBuilder newIds = new StringBuilder(9 * addTheseBranches.Length);
					StringBuilder sbXml = null;
					StringWriter sw = null;
					MemoryStream ms = null;
					XmlTextWriter xmlWriter = null;

					// Write the xml response for the operation
					// 
					if (ENCODE_UTF8)
					{
						ms = new MemoryStream(XML_BUFFER_SIZE);
						xmlWriter = new XmlTextWriter(ms, System.Text.Encoding.UTF8);
					}
					else
					{
						sbXml = new StringBuilder(XML_BUFFER_SIZE);
						sw = new StringWriter(sbXml);
						xmlWriter = new XmlTextWriter(sw);
					}

					xmlWriter.Formatting = System.Xml.Formatting.Indented;
					xmlWriter.Namespaces = true;

					// recurse new subtrees and write DIDL-Lite stuff
					MediaObject.WriteResponseHeader(xmlWriter);
					RecurseNewBranches(addTheseBranches, newIds, xmlWriter);
					MediaObject.WriteResponseFooter(xmlWriter);
					xmlWriter.Flush();

					objectID = newIds.ToString();
				
					// properly recast string for UTF8 or UTF-16
					if (ENCODE_UTF8)
					{
						int startPos = 3;
						int len = (int) ms.ToArray().Length - startPos;
						UTF8Encoding utf8e = new UTF8Encoding(false, true);
						Result = utf8e.GetString(ms.ToArray(), startPos, len);
					}
					else
					{
						Result = sbXml.ToString();
					}

					xmlWriter.Close();
				}
				else
				{
					throw new Error_InvalidServerConfiguration("CreateObject() cannot be supported until the vendor configures the server correctly.");
				}
			}
			catch (Exception e)
			{
				Exception ne = new Exception("MediaServerDevice.CreateObject()", e);
				throw ne;
			}

			this.m_Stats.CreateObject++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.CreateReference action.
		/// Method is supposed to create a child item that points to an existing
		/// media item that is somewhere else in the content hierarchy.
		/// The method should throw exceptions if related media objects are not
		/// found and should also prevent control points from creating
		/// references to other containers.
		/// </summary>
		/// <param name="containerID">Create a child item in this container.</param>
		/// <param name="objectID">The new child item should refer to this item.</param>
		/// <param name="NewID">Return the object ID of the new child item.</param>
		private void SinkCd_CreateReference(System.String containerID, System.String objectID, out System.String NewID)
		{
			// ensure that we can find the container where new child should be.
			DvMediaContainer parent = this.GetContainer(containerID);

			if (parent == null)
			{
				throw new Error_NoSuchContainer("("+containerID+")");
			}

			// ensure we can find item that is being referenced
			DvMediaItem refItem = this.GetItem(objectID);

			if (refItem == null)
			{
				throw new Error_NoSuchObject("("+objectID+")");
			}

			// create a new object that points to the referenced item
			refItem.LockReferenceList();
			IDvItem newItem = refItem.CreateReference();
			refItem.UnlockReferenceList();

			// request application logic for approval in adding this new item.
			// Application logic should throw an exception, preferably a UPnPCustomException,
			// to indicate a rejection
			IDvMedia[] branch = new IDvMedia[1];
			branch[0] = newItem;
			if (this.OnRequestAddBranch != null)
			{
				this.OnRequestAddBranch(this, parent, ref branch);
			}
			else
			{
				throw new Error_InvalidServerConfiguration("CreateReference() cannot be supported until the vendor configures the server correctly.");
			}

			// set the value for new ID
			NewID = newItem.ID;

			this.m_Stats.CreateReference++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.DeleteResource action.
		/// The caller provides the URI of the desired resource for deleting. 
		/// The method will attempt to find a <see cref="IDvResource"/> object that
		/// would map to the specified URI resource adn then delete the actual 
		/// binary file that is mapped to that resource if a match was found.
		/// If the delete resource operation failed for whatever reason, we throw
		/// an exception (preferably of UPnPCustomException) to report an error message.
		/// </summary>
		/// <param name="ResourceURI"></param>
		private void SinkCd_DeleteResource(System.Uri ResourceURI)
		{
			// Given the URI, parse out the objectID and a resourceID that are
			// embedded in the URI - this work provided the URI is actually
			// from this this server.
			string objectID, resourceID;
			this.GetObjectResourceIDS(ResourceURI, out objectID, out resourceID);

			if ((objectID == "") || (resourceID == ""))
			{
				throw new Error_NoSuchResource(ResourceURI.ToString());
			}

			// get the resource object from the media object
			IDvResource res = this.GetResource(objectID, resourceID);

			if (res == null)
			{
				throw new Error_NoSuchResource(ResourceURI.ToString());
			}
			else
			{
				//TODO: Might actually want to throw an exception here if the IP address
				//of the requested resource doesn't match an IP address of the server.
				//This may be more trouble than its worth though,a s it means
				//we have to figure out all of the IP addresses that apply to this
				//object and do a match, etc. What happens if the IP address
				//of the server changes spontaneously? Ignore the problem for now.
			}

			// request application logic for permission to delete; application
			// logic should throw an exception to indicate rejection
			if (this.OnRequestDeleteBinary != null)
			{
				this.OnRequestDeleteBinary (this, res);
			}
			else
			{
				throw new Error_InvalidServerConfiguration("DeleteResource() cannot be supported until the vendor configures the server correctly.");
			}

			// Although local files may have been deleted from the file system, we
			// still must practice due diligence and remove the resource from the owner.
			// In our implementation of MediaServer, we require that every resource can 
			// only have one parent, although a binary file may be exposed in ContentDirectory
			// through multiple resource objects.
			if (res.Owner.IsContainer)
			{
				DvMediaContainer owner = (DvMediaContainer) res.Owner;
				owner.RemoveResource(res);
			}
			else
			{
				// Items and reference items are handled the same way, even
				// in the case of an item that is a reference to another item.
				// In this implementation, the underlying item's resources are not modified since
				// we enforce a single-owner rule on resources.
				DvMediaItem owner = (DvMediaItem) res.Owner;
				owner.RemoveResource(res);
			}

			this.m_Stats.DeleteResource++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.DestroyObject action.
		/// The method will find a media object that matches the specified object ID.
		/// Then it will request upper-layer software components for permission to delete the
		/// object. Upper layer software components can reject the request by throwing
		/// an exception (preferably a UPnPCustomException) to indicate that the request
		/// was rejected. If the request was not rejected then the object (and the entire
		/// subtree it represented) is removed. No resources are destroyed in this process.
		/// <para>
		/// The <see cref="MediaServerDevice.OnRequestRemoveBranch"/> field must be
		/// set for this method to properly execute.
		/// </para>
		/// </summary>
		/// <param name="objectID"></param>
		private void SinkCd_DestroyObject(System.String objectID)
		{
			try
			{
				// find the object and throw errors if we can't find the object
				IDvMedia entry = this.GetCdsEntry(objectID);
				DvMediaContainer parent = (DvMediaContainer) entry.Parent;

				if (entry.ID == "0")
				{
					throw new Error_RestrictedObject("Cannot destroy container 0");
				}

				if (entry == null)
				{
					throw new Error_NoSuchObject(objectID);
				}

				// check if object is allowed to be destroyed
				if (entry.IsRestricted)
				{
					throw new Error_RestrictedObject("Cannot destroy object "+objectID);
				}

				// Request application logic to approve destroy request.
				// Application logic should throw exception to report a rejection.
				if (this.OnRequestRemoveBranch != null)
				{
					this.OnRequestRemoveBranch(this, parent, entry);
				}
				else
				{
					throw new Error_InvalidServerConfiguration("DestroyObject() cannot be supported until the vendor configures the server correctly.");
				}

				//Request approved, remove the object and its subtree and references appropriately.

				this.m_Cache.Remove(objectID);
				parent.RemoveObject(entry);
			
				// Force garbage collection, so that all descendents in m_Cache
				// have invalid weak references automatically.
				// 
				GC.Collect();
			}
			catch (Exception e)
			{
				Exception ne = new Exception("MediaServer.SinkCd_DestroyObject()", e);
				throw ne;
			}

			this.m_Stats.DestroyObject++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.ExportResource action.
		/// This method effectively causes the MediaServer to send a local binary to 
		/// an URI that expects an HTTP-POST method of transfer. The method will report
		/// error messages if the specified local URI does not map to a URI that is 
		/// automapped using <see cref="MediaResource.AUTOMAPFILE"/>.
		/// </summary>
		/// <param name="SourceURI"></param>
		/// <param name="DestinationURI"></param>
		/// <param name="TransferID"></param>
		private void SinkCd_ExportResource(System.Uri SourceURI, System.Uri DestinationURI, out System.UInt32 TransferID)
		{
			TransferID = 0;
			Uri dest = DestinationURI;
			string resourceID, objectID;

			// find media and resource IDs given a URI that should map to something
			// served on this server
			this.GetObjectResourceIDS(SourceURI, out objectID, out resourceID);

			if ((objectID == "") || (resourceID == ""))
			{
				throw new Error_NoSuchResource(SourceURI.ToString());
			}

			IDvResource res = this.GetResource(objectID, resourceID);

			if (res != null)
			{			
				//TODO: Might actually want to throw an exception here if the IP address
				//of the requested resource doesn't match an IP address of the server.
				//This may be more trouble than its worth though,a s it means
				//we have to figure out all of the IP addresses that apply to this
				//object and do a match, etc. What happens if the IP address
				//of the server changes spontaneously? Ignore the problem for now.

				//
				// create a socket that will connect to remote host
				// 

				Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

				IPEndPoint remoteIPE = null;
				if (dest.HostNameType == System.UriHostNameType.Dns)
				{
					remoteIPE = new IPEndPoint(Dns.GetHostByName(dest.Host).AddressList[0], dest.Port);
				}
				else
				{
					remoteIPE = new IPEndPoint(IPAddress.Parse(dest.Host), dest.Port);
				}

				if (remoteIPE != null)
				{
					// check if the file actually exists
					string fileName = res.ContentUri.Substring(MediaResource.AUTOMAPFILE.Length);
					if (Directory.Exists(fileName))
					{
						throw new Error_NoSuchResource("The binary could not be found on the system.");
					}

					FileNotMapped mapping = new FileNotMapped();
					StringBuilder li = new StringBuilder();
					li.AppendFormat("{0}:{1}", SourceURI.Host, SourceURI.Port);
					mapping.LocalInterface = li.ToString();
					mapping.RequestedResource = res;
					mapping.RedirectedStream = null;
					if (File.Exists(fileName))
					{
						// the file exists, so go ahead and send it
						mapping.RedirectedStream = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read);
					}
					else
					{
						try
						{
							if (this.OnFileNotMapped != null)
							{
								this.OnFileNotMapped(this, mapping);
							}
						}
						catch (Exception ofnme)
						{
							mapping.RedirectedStream = null;
						}
					}

					if (mapping.RedirectedStream == null)
					{
						throw new Error_NoSuchResource("The binary could not be found on the system.");
					}
					else
					{
						try
						{
							s.Connect(remoteIPE);
						}
						catch 
						{
							throw new UPnPCustomException(800, "Could not connect to the remote address of " +remoteIPE.ToString()+":"+remoteIPE.Port.ToString());
						}

						// Create a session that will post the file

						HTTPSession session = null;
						SessionData sd;

						this.SetupSessionForTransfer(session);
						sd = (SessionData) session.StateObject;
						sd.HttpVer1_1 = true;
						
						HttpTransfer transferInfo = new HttpTransfer(false, true, session, res, mapping.RedirectedStream, mapping.RedirectedStream.Length);
						this.AddTransfer(session, transferInfo);

						session.PostStreamObject(mapping.RedirectedStream, dest.PathAndQuery, res.ProtocolInfo.MimeType);

						TransferID = transferInfo.m_TransferId;
					}
				}
				else
				{
					throw new UPnPCustomException(800, "Could not connect to the socket.");
				}
			}
			else
			{
				throw new Error_NoSuchResource("");
			}

			this.m_Stats.ExportResource++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.GetSearchCapabilities action.
		/// Method simply returns * to indicate that any metadata field can be searched on.
		/// TODO: This method should actually return the list of metadata fields that can be searched on
		/// TODO: because we don't actually support searching on vendor specific metadata...yet.
		/// </summary>
		/// <param name="SearchCaps">
		/// Return comma separated value list of metadata property 
		/// names that can be used for searching.
		/// </param>
		private void SinkCd_GetSearchCapabilities(out System.String SearchCaps)
		{
			SearchCaps = m_SearchCapabilities;
			this.m_Stats.GetSearchCapabilities++;
			this.FireStatsChange();
		}
		private string m_SearchCapabilities;

		public string SearchCapabilities
		{
			get
			{
				return this.m_SearchCapabilities;
			}
			set
			{
				this.m_SearchCapabilities = value;
			}
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.GetSortCapabilities action.
		/// Method simply returns * to indicate that any metadata field can be sorted on.
		/// TODO: This method should actually return the list of metadata fields that can be sorted on
		/// TODO: because we don't actually support sorting on vendor specific metadata...yet.
		/// </summary>
		/// <param name="SortCaps">
		/// Return comma separated value list of metadata property 
		/// names that can be used for sorting.
		/// </param>
		private void SinkCd_GetSortCapabilities(out System.String SortCaps)
		{
			SortCaps = m_SortCapabilities;
			this.m_Stats.GetSortCapabilities++;
			this.FireStatsChange();
		}
		private string m_SortCapabilities;

		public string SortCapabilities
		{
			get
			{
				return this.m_SortCapabilities;
			}
			set
			{
				this.m_SortCapabilities = value;
			}
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.GetSystemUpdateID action.
		/// Returns the current value of the SystemUpdateID state variable.
		/// </summary>
		/// <param name="id">Return uint value to indicate the state of the content hierarchy.</param>
		private void SinkCd_GetSystemUpdateID(out System.UInt32 id)
		{
			id = this.ContentDirectory.Evented_SystemUpdateID;
			this.m_Stats.GetSystemUpdateID++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.GetTransferProgress action.
		/// This method is to return information about an ExportResource or ImportResource transfer.
		/// The method searches its list of pending HttpTransfers with the specified transfer ID.
		/// If it's found then the information is provided. Otherwise, it reports an error.
		/// </summary>
		/// <param name="TransferID">desired transfer id</param>
		/// <param name="TransferStatus">status of the transfer</param>
		/// <param name="TransferLength">total bytes transfered</param>
		/// <param name="TransferTotal">expected byte length of transfer</param>
		private void SinkCd_GetTransferProgress(System.UInt32 TransferID, out DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus TransferStatus, out System.String TransferLength, out System.String TransferTotal)
		{
			if (this.m_HttpTransfers.ContainsKey(TransferID))
			{
				HttpTransfer transfer = (HttpTransfer) this.m_HttpTransfers[TransferID];
				TransferLength = transfer.Position.ToString();
				TransferTotal = transfer.TransferSize.ToString();
				TransferStatus = transfer.TransferStatus;
			}
			else
			{
				throw new Error_NoSuchFileTransfer("("+TransferID.ToString()+")");
			}

			this.m_Stats.GetTransferProgress++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.ImportResource action.
		/// The method first checks to see if the local URI actually maps to an automapped file
		/// and that the remote URI is an HTTP resource.
		/// The method then asks upper software layers to accept or reject the request to
		/// import a resource from a remote URI to a local URI. Upper software layers reject the request by throwing an exception,
		/// preferably a UPnPCustomException. If the request was approved by upper layers,
		/// then we do a bunch of stuff that results in the file getting transferred over HTTP.
		/// </summary>
		/// <param name="SourceURI">the URI where the binary should be obtained</param>
		/// <param name="DestinationURI">the URI (that can map to a local file) where the binary should be stored</param>
		/// <param name="TransferID">Returns ID for the file transfer.</param>
		private void SinkCd_ImportResource(System.Uri SourceURI, System.Uri DestinationURI, out System.UInt32 TransferID)
		{
			string objectID, resourceID;

			// Parse the media and resource IDs from the destination uri.
			this.GetObjectResourceIDS(DestinationURI, out objectID, out resourceID);
			
			if ((objectID == "") || (resourceID == ""))
			{
				throw new Error_NoSuchResource(DestinationURI.ToString());
			}
			else
			{
				// TODO: Might consider throwing ane xception in the
				// rare case that stupid ass control point says to
				// download to a different media server.
			}

			// ensure that we're doing http
			if (SourceURI.Scheme.ToLower().StartsWith("http") == false)
			{
				throw new Error_NonHttpImport(DestinationURI.ToString());
			}

			// get the resource object associated with the destination uri
			IDvResource res = this.GetResource(objectID, resourceID);

			// request application logic to approve the binary transfer;
			// Application logic should throw an exception to rject the request.
			if (this.OnRequestSaveBinary != null)
			{
				this.OnRequestSaveBinary(this, res);
			}
			else
			{
				throw new Error_InvalidServerConfiguration("ImportResource() cannot be supported until the vendor configures the server correctly.");
			}

			//
			// Grab the file through http-get
			//
 
			IPAddress addr = null;
			IPHostEntry ihe = null;
			IPEndPoint dest = null;

			// Attempt to get a routeable IP address for the request

			try
			{
				if(SourceURI.HostNameType == UriHostNameType.Dns)
				{
					ihe = Dns.GetHostByName(SourceURI.Host);
					addr = new IPAddress(ihe.AddressList[0].Address);
				}
				else
				{
					addr = IPAddress.Parse(SourceURI.Host);
				}
			}
			catch
			{
				throw new Error_ConnectionProblem("Could parse or resolve the SourceURI IP address represented by" +SourceURI.ToString());
			}

			dest = new IPEndPoint(addr, SourceURI.Port);

			
			// Open a socket and connect to the remote IP address and port

			System.Net.Sockets.Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			try
			{
				s.Connect(dest);
			}
			catch
			{
				throw new Error_ConnectionProblem("Could not connect to the remote URI " + DestinationURI.ToString());
			}

			// Do a check to make sure we're not dumping to a directory.

			string filename = res.ContentUri.Substring(MediaResource.AUTOMAPFILE.Length);
			if (Directory.Exists(filename))
			{
				throw new Error_ImportError("System error. Resource has been mapped incorrectly. Cannot overwrite a directory with a binary.");
			}
			
			// Create an HTTP session for this socket.
			// Set things up so that the HTTP-GET will automatically dump
			// the body of the message into a binary file that has
			// been automatically mapped.

			HTTPSession session = new HTTPSession(s, null, null);
			this.SetupSessionForTransfer(session);
			session.OnHeader += new HTTPSession.ReceiveHeaderHandler(this.GetRequest_OnHeaderReceiveSink);
			try
			{
				session.UserStream = new FileStream(filename, FileMode.Create, FileAccess.Write, FileShare.None);
			}
			catch
			{
				throw new Error_ImportError("System busy. Could not open file from local system for writing.");
			}

			if (session.UserStream == null)
			{
				throw new Error_ImportError("System error. Cannot write to a null stream.");
			}

			SessionData sd = (SessionData) session.StateObject;
			sd.HttpVer1_1 = false;

			// Create the HTTP message that will request the binary.

			HTTPMessage msg = new HTTPMessage();
			msg.Directive = "GET";
			msg.DirectiveObj = HTTPMessage.UnEscapeString(SourceURI.PathAndQuery);
			msg.AddTag("HOST", dest.ToString());
			msg.Version = "1.0";

			// Create an HttpTransfer object that will represent the progress
			// of this file transfer and add it to the media server's current
			// transfers list.

			long expectedLength = 0;
			HttpTransfer transferInfo = new HttpTransfer(true, true, session, res, session.UserStream, expectedLength);
			this.AddTransfer(session, transferInfo);
			TransferID = transferInfo.m_TransferId;
				
			// Go make the request for the file.
			session.Send(msg);
			
			this.m_Stats.ImportResource++;
			this.FireStatsChange();
		}

		/// <summary>
		/// This method executes when we receive the headers of a response 
		/// to an HTTP-GET message that we sent when requesting a binary
		/// that was to be received from an ImportUri action.
		/// </summary>
		/// <param name="WebSession">the <see cref="HTTPSession"/> object representing the communication</param>
		/// <param name="msg">The response message from the remote endpoint.</param>
		/// <param name="stream">ignored</param>
		private void GetRequest_OnHeaderReceiveSink (HTTPSession WebSession, HTTPMessage msg, System.IO.Stream stream)
		{
			long expectedLength = this.ExtractContentLength(msg);

			//should only have one item
			SessionData sd = (SessionData) WebSession.StateObject;
			if (sd.Transfers.Count != 1)
			{
				throw new Error_TransferProblem(0, null);
			}

			HttpTransfer transferInfo = (HttpTransfer) sd.Transfers.Peek();
			transferInfo.m_TransferSize = expectedLength;

			WebSession.OnHeader -= new HTTPSession.ReceiveHeaderHandler(this.GetRequest_OnHeaderReceiveSink);
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.Search action.
		/// The method will recursively search all descendent objects and determine if
		/// the objects match the search criteria. All objects that match the search
		/// criteria will be included in a flat DIDL-Lite listing of media objects
		/// in the response. Sort criteria and filter criteria are just applied in
		/// the same manner as a browse request.
		/// </summary>
		/// <param name="containerID">Container to search from.</param>
		/// <param name="searchCriteria">Valid CDS search criteria string.</param>
		/// <param name="filter">
		/// Comma separated value list of metadata property names to include in the response.
		/// Return * for all metadata.
		/// </param>
		/// <param name="startingIndex">Given the entire possible response set, return a subset beginning with this index in the result set.</param>
		/// <param name="requestedCount">Given the entire possible response set, return a subset totalling no more than this many.</param>
		/// <param name="sortCriteria">Specify a comma-separated value list of metadata properties, with a + or - char before each property name to indicate ascending/descending order.</param>
		/// <param name="Result">DIDL-Lite response for desired result set.</param>
		/// <param name="numberReturned">Number of media objects returned in the response. 0 if browsing object metadata.</param>
		/// <param name="totalMatches">Total number of media objects in entire result set. 0 if browsing object metadata.</param>
		/// <param name="updateID">The UpdateID of the object - ignore if an object.item entry. Applies only to object.container entries.</param>
		private void SinkCd_Search(System.String containerID, System.String searchCriteria, System.String filter, System.UInt32 startingIndex, System.UInt32 requestedCount, System.String sortCriteria, out System.String Result, out System.UInt32 numberReturned, out System.UInt32 totalMatches, out System.UInt32 updateID)
		{
			try
			{
				numberReturned = 0;
				Result = "";
				totalMatches = 0;
				updateID = 0;

				// Get the container with the ID.
				// 

				IDvMedia entry = this.GetCdsEntry(containerID);
				if (entry.IsContainer == false)
				{
					throw new Error_NoSuchContainer("("+containerID+")");
				}
				DvMediaContainer container = (DvMediaContainer) entry;


				// Issue a search from the container.
				// Search requires a MediaComparer to determine whether an entry matches
				// against the searchCriteria.
				// Sorting is optional, but it requires that we traverse the entire subtree
				// in order to reply properly.
				// 
				IList entries;
				if (sortCriteria.Trim() == "")
				{
					MediaComparer postfix = new MediaComparer(searchCriteria);
					entries = container.Search(postfix, startingIndex, requestedCount, out totalMatches);
				}
				else
				{
					MediaSorter sorter = new MediaSorter(true, sortCriteria);
					MediaComparer postfix = new MediaComparer(searchCriteria);
					entries = container.SearchSorted(postfix, sorter, startingIndex, requestedCount, out totalMatches);
				}

				for (int rem=0; rem < startingIndex; rem++)
				{
					entries.RemoveAt(0);
				}

				numberReturned = Convert.ToUInt32(entries.Count);
				updateID = container.UpdateID;

				// Get the XML response for this result set.
				// Be sure to grab the list of base URLs.
				ArrayList properties = GetFilters(filter);
				string[] baseUrls = GetBaseUrlsByInterfaces();
				Result = BuildXmlRepresentation(baseUrls, properties, entries);

			}
			catch (Exception e)
			{
				Exception ne = new Exception("MediaServer.SinkCd_Search()", e);
				throw ne;
			}
			this.m_Stats.Search++;
			this.FireStatsChange();
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.StopTransferResource action.
		/// The method will effectively cancel an uncompleted ImportResource or ExportResource
		/// request that has not completed the transfer of the binary.
		/// </summary>
		/// <param name="TransferID">the transfer to stop</param>
		private void SinkCd_StopTransferResource(System.UInt32 TransferID)
		{
			HttpTransfer t;
			if (this.m_HttpTransfers.ContainsKey(TransferID))
			{
				t = (HttpTransfer) this.m_HttpTransfers[TransferID];
			}
			else
			{
				throw new Error_NoSuchFileTransfer("("+TransferID.ToString()+")");
			}

			bool found = false;
			if (t != null)
			{
				if (t.ImportExportTransfer)
				{
					found = true;
					t.Close(true);
				}
			}
		
			this.m_Stats.StopTransferResource++;

			if (found == false)
			{
				throw new Error_NoSuchFileTransfer("("+TransferID.ToString()+")");
			}
		}

		/// <summary>
		/// Method executes when a control point invokes the ContentDirectory.UpdateObject action.
		/// The caller must specify the object to be modified, the old XML values to replace,
		/// and the new XML values to use.
		/// </summary>
		/// <param name="objectID">update media object with this ID</param>
		/// <param name="currentTagValue">
		/// Comma separated value list of current XML elements in the object.
		/// Empty value or double comma indicates that a new XML element is to be created.
		/// CDS spec defines that the text of the XML elements must match
		/// exactly with the text provided by the server. This means
		/// XML attributes have to be in the correct order within the XML element.
		/// XML elements need not be in any particular order though.. just the
		/// text between the commas separating things need to be that way.
		/// The order of XML elements correspond with the order of replacement
		/// XML elements.
		/// </param>
		/// <param name="newTagValue">
		/// Comma separated value list of new/replace XML elements in the object.
		/// Empty value or double comma indicates that an existing XML element is to be deleted.
		/// The order of XML elements correspond with the order of current
		/// XML elements.
		/// </param>
		private void SinkCd_UpdateObject(System.String objectID, System.String currentTagValue, System.String newTagValue)
		{
			try
			{
				// Ensure that the specified object actually exists
				IDvMedia obj = (IDvMedia) this.GetCdsEntry(objectID);

				if (obj == null)
				{
					throw new Error_NoSuchObject("("+objectID+")");
				}
				else if (obj.IsReference)
				{
					throw new UPnPCustomException(830, "This server will not allow UpdateObject() on a reference.");
				}

				// Set up text parsers.
				DText currentTagParser = new DText();
				DText newTagParser = new DText();
					
				currentTagParser.ATTRMARK = ",";
				newTagParser.ATTRMARK = ",";

				currentTagParser[0] = currentTagValue;
				newTagParser[0] = newTagValue;

				// ensure that the number of old xml values is equal 
				// to the number of new values

				// BUG - Need to fix a case where the XML values include commas.

				int nFrags = currentTagParser.DCOUNT();
				int nFrags2 = newTagParser.DCOUNT();

				if (nFrags != nFrags2)
				{
					throw new Error_ParameterMismatch("The number of tag/value pairs is not the same between currentTagValue and newTagValue.");
				}

				// determine the base URL using the IP address and port
				// where the request came in from and then
				// build an DIDL-Lite representation of the
				// object that needs updating

				//TODO: get all interfaces
				string baseUrl = this.GetBaseUrlByInterface();
				ArrayList properties = new ArrayList();
				ArrayList entries = new ArrayList();
				entries.Add(obj);

				string oldXml;
				try
				{
					oldXml = BuildXmlRepresentation(baseUrl, properties, entries);
				}
				catch (Exception e)
				{
					throw (e);
				}

				//  At this point oldXml represents the object we're trying to
				//  update, in an xml string format. Now go and replace (or add) all
				//  of the text values appropriately.
				//  

				string newXml = oldXml;
				for (int i=1; i <= nFrags; i++)
				{
					// grab the old and new value XML fragments

					string oldFrag = currentTagParser[i].Trim();
					string newFrag = newTagParser[i].Trim();
				
					if (
						((oldFrag.StartsWith("<") == false) || (oldFrag.EndsWith(">") == false)) &&
						(oldFrag != "")
						)
					{
						throw new Error_InvalidCurrentTagValue("Invalid args. (" + oldFrag + ") is not an xml element.");
					}

					int pos = newXml.IndexOf(oldFrag);
					if ((oldFrag != "") && (pos < 0))
					{
						throw new Error_InvalidCurrentTagValue("Cannot find xml element (" +oldFrag+ ").");
					}

					if (oldFrag == "")
					{
						// The old value XML fragment indicates that a completely
						// new XML element needs to be added.
						if (obj.IsContainer)
						{
							StringBuilder newVal = new StringBuilder(newFrag.Length + 10);
							newVal.AppendFormat("{0}</container>");
							newXml = newXml.Replace("</container>", newVal.ToString());
						}
						else
						{
							StringBuilder newVal = new StringBuilder(newFrag.Length + 10);
							newVal.AppendFormat("{0}</item>", newFrag);
							newXml = newXml.Replace("</item>", newVal.ToString());
						}
					}
					else
					{
						// The new value XML fragment indicates that a current
						// XML fragment must be deleted.
						if (newFrag == "")
						{
							newXml = newXml.Replace(oldFrag, "");
						}
						else
						{
							newXml = newXml.Replace(oldFrag, newFrag);
						}
					}
				}

				// At this point newXml represents what the proposed object should
				// look like with new changes. We'll continue by casting the
				// string into an xmldocument and instantiating a new IDvMedia
				// instance, making sure to keep the ID specified within the newXml.
				// 

				// Cast the string into an XmlDocument so that we can instantiate
				// a media object using an XmlElement object.

				XmlDocument xmldoc = new XmlDocument();
				xmldoc.LoadXml(newXml);

				XmlNodeList didlheader = xmldoc.GetElementsByTagName(T[_DIDL.DIDL_Lite]);
				XmlNode didlRoot = didlheader[0];
				XmlElement xmlElement = (XmlElement) didlRoot.ChildNodes[0];

				IDvMedia newObj;
				if (obj.IsContainer)
				{
					newObj = new DvMediaContainer(xmlElement);
				}
				else
				{
					newObj = new DvMediaItem(xmlElement);
				}

				// Iterate through the resources of the new object
				// and ensure that resources that carried over
				// from the original object's metadata are
				// properly mapped in the new object's list.
				// This way, upper software layers cannot effectively
				// tell the difference if a comparison-by-value is done.

				foreach (IMediaResource newRes in newObj.Resources)
				{
					string uri = newRes.ContentUri;
					int pos = uri.IndexOf(this.m_VirtualDirName);

					if (pos > 0)
					{
						// ensure that contentUri and importUri values don't violate automapping rule
						string subUri = uri.Substring(pos + this.m_VirtualDirName.Length);
						DText DirectiveParser = new DText();
						DirectiveParser.ATTRMARK = "/";

						DirectiveParser[0] = subUri;
						string resourceID = DirectiveParser[2];
						string objID = DirectiveParser[3];
						System.Diagnostics.Debug.Assert(objID == objectID);
						IDvResource res = this.GetResource(objectID, resourceID);

						newRes[T[_RESATTRIB.importUri]] = null;
						newRes.ContentUri = res.ContentUri;
					}
				}

				// Request upper software components to accept/reject
				// request to change metadata. The delegate should
				// throw an exception (preferably UPnPCustomExeption)
				// to indicate a rejection.
				if (this.OnRequestChangeMetadata != null)
				{
					this.OnRequestChangeMetadata (this, obj, newObj);
				}

				// If no rejection, then use the metadata of the new object
				// and apply it to the existing object.
				obj.UpdateObject(newObj);
			}
			catch (Exception e)
			{
				Exception ne = new Exception("MediaServer.SinkCd_UpdateObject()", e);
				throw ne;
			}

			this.m_Stats.UpdateObject++;
			this.FireStatsChange();
		}

		/// <summary>
		/// This method changes the source or sink protocolInfo sets 
		/// for the media server.
		/// </summary>
		/// <param name="sourceProtocolInfo">true, if changing source protocolInfo set</param>
		/// <param name="protocolInfoSet">new protocolInfo strings, separated by commas</param>
		private void UpdateProtocolInfoSet (bool sourceProtocolInfo, string protocolInfoSet)
		{
			DText parser = new DText();
			parser.ATTRMARK = ",";
			parser[0] = protocolInfoSet;

			int cnt = parser.DCOUNT();
			ArrayList prots = new ArrayList();

			for (int i=1; i <= cnt; i++)
			{
				string val = parser[i].Trim();

				if (val != "")
				{
					ProtocolInfoString protInfo = new ProtocolInfoString("*:*:*:*");
					bool error = false;
					try
					{
						protInfo = new ProtocolInfoString(val);
					}
					catch
					{
						error = true;
					}

					if (error == false)
					{
						prots.Add(protInfo);
					}
				}
			}

			ProtocolInfoString[] protArray = null;

			if (prots.Count > 0)
			{
				protArray = (ProtocolInfoString[]) prots.ToArray(prots[0].GetType());
			}

			this.UpdateProtocolInfoSet(sourceProtocolInfo, protArray);
		}

		/// <summary>
		/// Method used by UpdateProtocolInfoSet(bool, string) to
		/// actually change the protocolInfo strings.
		/// This method will also cause the server to event the changes
		/// in the state variable.
		/// </summary>
		/// <param name="sourceProtocolInfo">true if the source protocolInfo set is to be changed</param>
		/// <param name="array">new list of protocolInfo strings for the set</param>
		private void UpdateProtocolInfoSet (bool sourceProtocolInfo, ProtocolInfoString[] array)
		{
			ArrayList arrayList;
			ReaderWriterLock ReaderWriterLock;
			if (sourceProtocolInfo)
			{
				arrayList = this.m_SourceProtocolInfoSet;
				ReaderWriterLock = this.m_LockSourceProtocolInfo;
			}
			else
			{
				arrayList = this.m_SinkProtocolInfoSet;
				ReaderWriterLock = this.m_LockSinkProtocolInfo;
			}

			// Updating the list of protocol info strings.
			// 
			ReaderWriterLock.AcquireWriterLock(-1);

			StringBuilder sb = new StringBuilder();
			if (array != null)
			{
				arrayList.Clear();
				arrayList.AddRange(array);

				for (int i=0; i < arrayList.Count; i++)
				{
					if (i > 0) sb.Append(",");
					sb.Append(array[i].ToString());
				}
			}

			if (sourceProtocolInfo)
			{
				this.ConnectionManager.Evented_SourceProtocolInfo = sb.ToString();
			}
			else
			{
				this.ConnectionManager.Evented_SinkProtocolInfo = sb.ToString();
			}

			ReaderWriterLock.ReleaseWriterLock();
		}

		/// <summary>
		/// Returns the current values of a protocolInfo set.
		/// </summary>
		/// <param name="sourceProtocolInfo">true, if the source protocolInfo set is desired</param>
		/// <returns>array of protocolInfo strings</returns>
		private ProtocolInfoString[] GetProtocolInfoSet (bool sourceProtocolInfo)
		{
			ArrayList arrayList;
			ReaderWriterLock ReaderWriterLock;
			if (sourceProtocolInfo)
			{
				arrayList = this.m_SourceProtocolInfoSet;
				ReaderWriterLock = this.m_LockSourceProtocolInfo;
			}
			else
			{
				arrayList = this.m_SinkProtocolInfoSet;
				ReaderWriterLock = this.m_LockSinkProtocolInfo;
			}
			
			// Obtaining a list of protocol info strings.
			// 
			ReaderWriterLock.AcquireReaderLock(-1);

			ProtocolInfoString[] array = new ProtocolInfoString[ arrayList.Count ];
			for (int i=0; i < arrayList.Count; i++)
			{
				array[i] = (ProtocolInfoString) arrayList[i];
			}
				
			ReaderWriterLock.ReleaseReaderLock();

			return array;
		}


		/// <summary>
		/// Method ensures that the protocolInfo for a PrepareForConnection request
		/// matches up correctly with a supported protocol on the device.
		/// The check is a trivial check and nothing more.
		/// <see cref="MediaServerDevice.SinkCm_PrepareForConnection"/>
		/// will do additional checks to ensure that the entire request is correct.
		/// </summary>
		/// <param name="protInfo">protocolInfo string proposed in the request</param>
		/// <param name="dir">direction of the stream, relative to the server</param>
		private void ValidateConnectionRequest(ProtocolInfoString protInfo, DvConnectionManager.Enum_A_ARG_TYPE_Direction dir)
		{
			bool incompatibleProtocol = true;

			// Grab the correct protocolInfo set, depending on whether
			// the stream is supposed to be a source or a sink.
			ArrayList al;
			if (dir == DvConnectionManager.Enum_A_ARG_TYPE_Direction.INPUT)
			{
				al = this.m_SinkProtocolInfoSet;
			}
			else if (dir == DvConnectionManager.Enum_A_ARG_TYPE_Direction.OUTPUT)
			{
				al = this.m_SourceProtocolInfoSet;
			}
			else
			{
				throw new Error_InvalidDirection("");
			}
		
			// Compare the requested protocol info string against the
			// list of available ones.
			// 
			foreach (ProtocolInfoString prot in al)
			{
				if (protInfo.Matches(prot))
				{
					incompatibleProtocol = false;
					break;
				}
			}

			if (incompatibleProtocol)
			{
				throw new Error_IncompatibleProtocolInfo("("+protInfo.ToString()+")");
			}
		}

		/// <summary>
		/// Creates a unique connection ID for a new connection.
		/// Method will attempt to loop to a previously used
		/// (but currently inactive) ID if reached max int.
		/// </summary>
		/// <returns></returns>
		private int GetConnectionID()
		{
			bool keeplooking = true;
			bool looped = false;
			
			int startedwith = NextConnId;
			while (keeplooking)
			{
				
				if (NextConnId < this.MaxConnections)
				{
					NextConnId++;
				}
				else
				{
					NextConnId = StartConnId;
				}
				
				if (m_Connections.ContainsKey(NextConnId) == false)
				{
					keeplooking = false;
				}
				else if (NextConnId == startedwith)
				{
					looped = true;
					keeplooking = false;
				}
			}
			
			if (looped)
			{
				throw new Error_MaximumConnectionsExceeded("");
			}
			
			return NextConnId;
		}


		/// <summary>
		/// Adds a new Connection object to the server's list of active connections.
		/// </summary>
		/// <param name="newConnection"></param>
		private void AddConnection(Connection newConnection)
		{
			this.m_LockConnections.AcquireWriterLock(-1);
			m_Connections.Add(newConnection.ConnectionId, newConnection);
			this.UpdateConnections();			
			this.m_LockConnections.ReleaseWriterLock();
		}

		/// <summary>
		/// Removes a Connection object from the server's list of active connections.
		/// </summary>
		/// <param name="theConnection"></param>
		private void RemoveConnection(Connection theConnection)
		{
			this.m_LockConnections.AcquireWriterLock(-1);
			m_Connections.Remove(theConnection.ConnectionId);
			this.UpdateConnections();			
			this.m_LockConnections.ReleaseWriterLock();
		}

		/// <summary>
		/// Updates the value of the CurrentConnectionIDs state variable using
		/// the list of active Connection objects for the server.
		/// </summary>
		private void UpdateConnections()
		{
			StringBuilder sb = new StringBuilder(this.m_Connections.Count * 50);
			int i=0;
			foreach (int connId in this.m_Connections.Keys)
			{
				if (i > 0)
				{
					sb.AppendFormat(", {0}", connId);
				}
				else
				{
					sb.AppendFormat("{0}", connId);
				}
				i++;
			}
			this.ConnectionManager.Evented_CurrentConnectionIDs = sb.ToString();
		}

		
		/// <summary>
		/// Build a base url using the IP address of the interface that received the
		/// browse request. This is required since we assume multi-nic machines.
		/// This base url should apply only to resources that are served by
		/// the webserver.
		/// </summary>
		/// <returns></returns>
		private string GetBaseUrlByInterface()
		{
			IPEndPoint ipe = this.ConnectionManager.GetUPnPService().GetReceiver();
			StringBuilder sb = new StringBuilder(35);
			sb.AppendFormat("http://{0}:{1}/{2}", ipe.Address.ToString(), ipe.Port.ToString(), this.m_VirtualDirName);

			return sb.ToString();
		}

		/// <summary>
		/// Grabs a list of base URLs.
		/// </summary>
		/// <returns></returns>
		private string[] GetBaseUrlsByInterfaces()
		{
			string baseUrl = this.GetBaseUrlByInterface();
			IPEndPoint[] localInterfaces = this.Device.LocalIPEndPoints;
			string[] baseUrls = new string[localInterfaces.Length];

			int switchIndex = -1;
			for (int i=0; i < localInterfaces.Length; i++)
			{
				StringBuilder sbli = new StringBuilder(localInterfaces[i].ToString().Length*2);
				sbli.AppendFormat("http://{0}/{1}", localInterfaces[i].ToString(),  this.m_VirtualDirName);
				baseUrls[i] = sbli.ToString();

				if (string.Compare(sbli.ToString(), baseUrl, true) == 0)
				{
					switchIndex = i;
				}
			}
			if (switchIndex < 0)
			{
			}
			else
			{
				if (switchIndex > 0)
				{
					string swstr = (string) baseUrls[switchIndex];
					baseUrls[switchIndex] = baseUrls[0];
					baseUrls[0] = swstr;
				}
			}

			return baseUrls;
		}

		/// <summary>
		/// Takes a comma separated value list of tag names
		/// and returns them individually in an arraylist.
		/// Method is primarily used to get an ArrayList used with the 
		/// "desiredProperties" argument of
		/// <see cref="IUPnPMedia.ToXml"/> and <see cref="IUPnPMedia.ToAlternateXml()"/>
		/// methods.
		/// </summary>
		/// <param name="filters">Comma separated value list of metadata property names.</param>
		/// <returns>
		/// ArrayList where each item is a metadata property name in the input.
		/// The return value is null to indicate an empty string was used as input.
		/// The intent is slightly different from an empty ArrayList, which indicates
		/// the control point specified * for all metadata properties.
		/// </returns>
		private ArrayList GetFilters(string filters)
		{
			ArrayList properties = new ArrayList();
			filters = filters.Trim();
			if (filters == "")
			{
				return null;
			}
			if ((filters == ",") || (filters.IndexOf('*') >= 0))
			{
			}
			else
			{
				DText filterParser = new DText();
				filterParser.ATTRMARK = ",";
				filterParser[0] = filters;
				int size = filterParser.DCOUNT();

				//indicates if the "res" element related attribute was added to the filter list
				bool addRes = false;

				//indicates if the "res" element was added to the filter list 
				bool addedRes = false;

				//Iterate through the comma seaparted value list

				for (int i=1; i <= size; i++)
				{
					// Generally speaking, we're only obligated to support the @attribute
					// syntax for top level DIDL-Lite elements. This includes
					// "item", "container", "res", and "desc" elements. Some XML attributes
					// of those tags are always required, so we may not even bother checking
					// for those attributes because ToXml() and ToAlternateXml()
					// will always print those values.

					string prop = filterParser[i].Trim();

					if (prop == "res")
					{
						// res explicitly added
						addedRes = true;
					}

					// If the property begins with an ampersand,
					// then we check all possible CDS-normative
					// top level element names for a matching XML attribute.
					// Most attributes are related to resource elements,
					// so make note when a resource related attribute
					// was added because we may need to explicitly add
					// the "res" element in the filter list to properly
					// indicate that the "res" elements in DIDL-Lite
					// response are to be returned.

					if (prop.StartsWith("@"))
					{
						prop = prop.Substring(1);

						if (string.Compare(prop, T[_ATTRIB.parentID], true) == 0)
						{
							properties.Add(Tags.PropertyAttributes.item_parentID);
							properties.Add(Tags.PropertyAttributes.container_parentID);
						}
						else if (string.Compare(prop, T[_ATTRIB.childCount], true) == 0)
						{
							properties.Add(Tags.PropertyAttributes.container_childCount);
						}
						
						else if (string.Compare(prop, T[_ATTRIB.bitrate], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_bitrate);
						}
						else if (string.Compare(prop, T[_ATTRIB.bitsPerSample], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_bitsPerSample);
						}
						else if (string.Compare(prop, T[_ATTRIB.colorDepth], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_colorDepth);
						}
						else if (string.Compare(prop, T[_ATTRIB.duration], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_duration);
						}
						else if (string.Compare(prop, T[_ATTRIB.importUri], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_importUri);
						}
						else if (string.Compare(prop, T[_ATTRIB.nrAudioChannels], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_nrAudioChannels);
						}
						else if (string.Compare(prop, T[_ATTRIB.protocolInfo], true) == 0)
						{
							addRes = true;
						}
						else if (string.Compare(prop, T[_ATTRIB.protection], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_protection);
						}
						else if (string.Compare(prop, T[_ATTRIB.resolution], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_resolution);
						}
						else if (string.Compare(prop, T[_ATTRIB.sampleFrequency], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_sampleFrequency);
						}
						else if (string.Compare(prop, T[_ATTRIB.size], true) == 0)
						{
							addRes = true;
							properties.Add(Tags.PropertyAttributes.res_size);
						}

						else if (string.Compare(prop, T[_ATTRIB.name], true) == 0)
						{
							// handle optional "name" attribute for various elements
							properties.Add(Tags.PropertyAttributes.upnp_class);
							properties.Add(Tags.PropertyAttributes.upnp_className);
							properties.Add(Tags.PropertyAttributes.upnp_searchClass);
							properties.Add(Tags.PropertyAttributes.upnp_searchClassName);
						}
					}
					else
					{
						properties.Add(prop);
					}
				}

				if ((addRes) && (!addedRes))
				{
					// only explicitly add "res" element if the other elements
					// were not added
					properties.Add("res");
				}
			}
			
			return properties;
		}


		/// <summary>
		/// Returns a CDS entry in the form of an IDvMedia object.
		/// Throws a UPnPCustomException() if the entry doesnt' exist.
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		private IDvMedia GetCdsEntry(string id)
		{
			IDvMedia entry = this._GetEntry(id);

			if (entry == null)
			{
				throw new Error_NoSuchObject("("+id+")");
			}

			return entry;
		}

		/// <summary>
		/// Easy method for throwing the <see cref="Error_BadMetadata"/> object.
		/// </summary>
		/// <exception cref="Error_BadMetadata">
		/// Always thrown.
		/// </exception>
		private void BadMetadata()
		{
			throw new Error_BadMetadata("");
		}

		/// <summary>
		/// Method starts with the root container and attempts to find the
		/// a descendent media object with a particular object ID.
		/// The <see cref="MediaServerDevice"/> object employs a caching
		/// hashtable so that objects found earlier can be easily found
		/// if they haven't been removed from the content heiarchy.
		/// </summary>
		/// <param name="id">desired object with this ID</param>
		/// <returns>media object with the ID; null if not found</returns>
		private IUPnPMedia GetDescendent(string id)
		{
			IUPnPMedia obj;

			if (id == "0")
			{
				return this.m_Root;
			}
			else
			{
				obj = null;
				WeakReference wr = (WeakReference) this.m_Cache[id];

				if (wr != null)
				{
					if (wr.IsAlive)
					{
						obj = (IUPnPMedia) wr.Target;
					}
				}

				if (obj == null)
				{
					obj = this.m_Root.GetDescendent(id, this.m_Cache);
				}
			}

			System.GC.Collect();
			
			return obj;
		}

		/// <summary>
		/// Attempts to find a descendent media item with the specified ID.
		/// </summary>
		/// <param name="id">desired media item</param>
		/// <returns>desired item, or null if media object with that ID does not exist or if it is a container</returns>
		private DvMediaItem GetItem(string id)
		{
			IDvMedia entry = (IDvMedia) this.GetDescendent(id);

			if (entry != null)
			{
				if (entry.IsItem)
				{
					return (DvMediaItem) entry;
				}
			}

			return null;
		}

		/// <summary>
		/// Attempts to find a descendent media container with the specified ID.
		/// </summary>
		/// <param name="id">desired media container</param>
		/// <returns>desired container, or null if media object with that ID does not exist or if it is an item</returns>
		private DvMediaContainer GetContainer (string id)
		{
			IDvMedia entry = (IDvMedia) this.GetDescendent(id);

			if (entry != null)
			{
				if (entry.IsContainer)
				{
					return (DvMediaContainer) entry;
				}
			}

			return null;
		}

		/// <summary>
		/// Simple wrapper for GetDescendent(string).
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		private IDvMedia _GetEntry (string id)
		{
			IDvMedia entry = (IDvMedia) this.GetDescendent(id);
			
			return entry;
		}

		/// <summary>
		/// Helper function for SinkCd_CreateObject(). Kicks off the process for
		/// obtaining the result xml and the string/CSV of new IDs for created items and containers.
		/// </summary>
		/// <param name="newBranches">list of media item and container objects that represent subtrees that are being created</param>
		/// <param name="newIds">comma separated value list of object IDs found in all of newBranches and its descendents</param>
		/// <param name="resultXml">DIDL-Lite XML for newBranches and its descendents</param>
		private void RecurseNewBranches (IList newBranches, StringBuilder newIds, XmlTextWriter resultXml)
		{
			//TODO: get all interfaces
			string baseUri = this.GetBaseUrlByInterface();
			foreach (IUPnPMedia mo in newBranches)
			{
				//obtains the ids found in this subtree
				ObtainBranchIDs(mo, newIds);

				//prints this branch into xml
				ToXmlDataDv _d = new ToXmlDataDv();
				//TODO: Add multiple uris
				_d.BaseUri = baseUri;
				_d.DesiredProperties = new ArrayList(0);
				_d.IsRecursive = true;
				//mo.ToXml(MediaObject.ToXmlFormatter_Default, _d, resultXml);
				mo.ToXml(ToXmlFormatter.DefaultFormatter, _d, resultXml);
			}
		}

		/// <summary>
		/// Helper function for SinkCd_CreateObject(). 
		/// Obtains IDs of all objects in a subtree.
		/// </summary>
		/// <param name="branch">the media object branch</param>
		/// <param name="newIds">listing of all object IDs in branch and its descendents</param>
		private void ObtainBranchIDs(IUPnPMedia branch, StringBuilder newIds)
		{
			if (newIds.Length == 0)
			{
				newIds.Append(branch.ID);
			}
			else
			{
				newIds.AppendFormat(", {0}", branch.ID);
			}

			IMediaContainer c = branch as IMediaContainer;
			if (c != null)
			{
				foreach (IUPnPMedia child in c.CompleteList)
				{
					this.ObtainBranchIDs(child, newIds);
				}
			}
		}


		/// <summary>
		/// Given a Uri that points to a virtual directory for the server, it will parse out 
		/// out the object ID and the resource ID for the requested URI.
		/// </summary>
		/// <param name="theUri">a Uri that maps to the media server</param>
		/// <param name="objectID">use this to find out what object to obtain</param>
		/// <param name="resourceID">use this to find the resource in the object that the uri maps to</param>
		private void GetObjectResourceIDS(Uri theUri, out string objectID, out string resourceID)
		{
			objectID = "";
			resourceID = "";

			try
			{
				string uri = theUri.ToString();
				string vdirName = "/"+this.m_VirtualDirName+"/";
				int vdirPos = uri.IndexOf(vdirName);
				string path = uri.Substring(vdirPos + vdirName.Length );
				DText pathParser = new DText();
				pathParser.ATTRMARK = "/";
				pathParser[0] = path;

				resourceID = pathParser[1];
				objectID = pathParser[2];
			}
			catch{}
		}

		/// <summary>
		/// Given a on object ID and resource ID (which is an internal way
		/// of tracking resource objects), return the correct resource object.
		/// </summary>
		/// <param name="objectID"></param>
		/// <param name="resourceID"></param>
		/// <returns></returns>
		/// <exception cref="ApplicationException">
		/// Thrown when a media object that is not a DvMediaItem or DvMediaContainer
		/// is found.
		/// </exception>
		private IDvResource GetResource(string objectID, string resourceID)
		{
			// find the descendent media object
			IDvMedia obj = this._GetEntry(objectID);
			IDvResource res = null;

			// find the appropriate resource given the resource ID
			if (obj != null)
			{
				if (obj.GetType() == new DvMediaContainer().GetType())
				{
					DvMediaContainer dvmc = (DvMediaContainer) obj;
					res = dvmc.GetResource(resourceID);
				}
				else if (obj.GetType() == new DvMediaItem().GetType())
				{
					DvMediaItem dvmi = (DvMediaItem) obj;
					res = dvmi.GetResource(resourceID);
				}
				else
				{
					throw new ApplicationException("Found non-DvMediaxxx in content hierarchy.");
				}
			}

			return res;
		}

		/// <summary>
		/// This class is used to track the responses to HTTP-GET requests.
		/// The class is primarily used to properly enable the handling of
		/// pipelined HTTP-GET requests.
		/// <para>
		/// The class essentially works by queuing an <see cref="HttpTransfer"/>
		/// object to the Transfers property any time an HTTP-GET request
		/// is made for the session.
		/// </para>
		/// <para>
		/// When a transfer/response actually completes, then an object
		/// is dequeued from the <see cref="SessionData.Transfers"/> property. 
		/// </para>
		/// <para>
		/// The actual serialization of the responses is handled by the underlying
		/// UPnP stack. This class simply gives this library the means to track
		/// the progress of the responses.
		/// </para>
		/// </summary>
		private class SessionData
		{
			/// <summary>
			/// A queue of HTTP transfer objects
			/// </summary>
			public Queue Transfers = new Queue();
			/// <summary>
			/// Indication of whether the request was HTTP 1.1,
			/// which allows pipelining of HTTP requests.
			/// </summary>
			public bool HttpVer1_1;
			/// <summary>
			/// The number of HTTP-GET requests that have been requested
			/// for this session so far.
			/// </summary>
			public int Requested = 0;
			/// <summary>
			/// The number of responses that have been completed.
			/// </summary>
			public int Completed = 0;
		}

		/// <summary>
		/// This method executes whenever the MediaServer receives the headers for an HTTP message.
		/// (This primarily executes to handle HTTP-GET requests for content.)
		/// The method effectively ensures that the HTTP session's events are wired up
		/// and also sets the SessionData (stored in the <see cref="HTTPSession.StateObject"/>)
		/// is set to either handle HTTP 1.1 pipelining or not.
		/// </summary>
		/// <param name="sender">the UPnPDevice that received the HTTP message</param>
		/// <param name="msg">the HTTP message, with only the headers guaranteed to be there</param>
		/// <param name="WebSession">the HTTP session object that encapsulates the traffic</param>
		/// <param name="VirtualDir">the virtual directory on which the request was received</param>
		private void WebServer_OnHeaderReceiveSink (UPnPDevice sender, HTTPMessage msg, HTTPSession WebSession, string VirtualDir)
		{
			// Setup session for any and all possible transfers.
			// Essentially, we ensure that the HTTP session has a 
			// SessionData object associated with it.
			// Furthermore we ensure that the WebSession's OnClose
			// and OnStreamDone events are wired to actual methods.
			this.SetupSessionForTransfer(WebSession);

			SessionData sd = (SessionData) WebSession.StateObject;

			// update the HTTP 1.1 version flag, which indicates
			// whether we should close the sessio when a stream
			// finishes. Clients should not mix 1.1 calls
			// with other calls because it would be really stupid
			// to asked for non-chunked stuff amidst chunked-encoded 
			// stuff.
			if ((msg.Version == "1.0") || (msg.Version == "0.0"))
			{
				sd.HttpVer1_1 = false;
			}
			else
			{
				sd.HttpVer1_1 = true;
			}
		}

		/// <summary>
		/// Examines an <see cref="HTTPMessage"/> object and
		/// attempts to find the content length of the message body.
		/// </summary>
		/// <param name="msg"></param>
		/// <returns></returns>
		private long ExtractContentLength(HTTPMessage msg)
		{
			long expectedLength = 0;
			string contentLengthString = msg.GetTag("CONTENT-LENGTH");
			try
			{
				expectedLength = long.Parse(contentLengthString);
			}
			catch{}

			return expectedLength;
		}
		
		/// <summary>
		/// This method executes after <see cref="MediaServerDevice.WebServer_OnHeaderReceiveSink"/>
		/// and is responsible for actually routing the HTTP-GET, HTTP-HEAD, or HTTP-POST
		/// message request to the appropriate handler.
		/// </summary>
		/// <param name="sender"><see cref="UPnPDevice"/> that received the HTTP message</param>
		/// <param name="msg">the HTTP message</param>
		/// <param name="WebSession">the HTTP session that carried the message</param>
		/// <param name="VirtualDir">the virtual directory that pathed the message directive object</param>
		private void WebServer_OnPacketReceiveSink (UPnPDevice sender, HTTPMessage msg, HTTPSession WebSession, string VirtualDir)
		{
			if (String.Compare(msg.Directive, "POST", true) == 0)
			{
				this.HandlePostedFileToServer(msg, WebSession);
			}
			else if (
				(String.Compare(msg.Directive, "GET", true) == 0) ||
				(String.Compare(msg.Directive, "HEAD", true) == 0)
				)
			{
				this.HandleGetOrHeadRequest(msg, WebSession);
			}
		}

		/// <summary>
		/// Parses 'rangeStr' for HTTP range sets, and adds the sets into
		/// 'rangeSets'... should an overlapping range set be provided
		/// or if an otherwise invalid range is requested, then we clear
		/// the 'rangeSets'... behavior is taken from http://www.freesoft.org/CIE/RFC/2068/178.htm.
		///
		/// <para>
		/// If the server ignores a byte-range-spec because it is invalid, 
		/// the server should treat the request as if the invalid Range header 
		/// field did not exist. 
		/// (Normally, this means return a 200 response containing the full entity). 
		/// The reason is that the only time a client will make such an invalid 
		/// request is when the entity is smaller than the entity retrieved by a prior request.
		/// [source: http://www.freesoft.org/CIE/RFC/2068/178.htm]
		/// </para>
		/// </summary>
		/// <param name="rangeSets">this ArrayList has range sets added to it</param>
		/// <param name="rangeStr">
		/// This is the HTTP header with the desired ranges.
		/// Text is assumed to be all lower case and trimmed.
		/// </param>
		/// <param name="contentLength">
		/// The entire length of the content, from byte 0.
		/// </param>
		private ArrayList AddRangeSets(ArrayList rangeSets, string rangeStr, long contentLength)
		{
			bool errorEncountered = true;

			errorEncountered = false;
			DText dt = new DText();
			dt.ATTRMARK = "=";
			dt.MULTMARK = ",";
			dt.SUBVMARK = "-";
			dt[0] = rangeStr;

			int numSets = dt.DCOUNT(2);

			for (int i=1; i <= numSets; i++)
			{
				string sOffset = dt[2,i,1].Trim();
				string sEnd = dt[2,i,2].Trim();
				long offset=-1, length=-1, end=-1;

				if ((sOffset == "") && (sEnd == ""))
				{
					// royally screwed up request
					errorEncountered = true;
					break;
				}
				else if ((sOffset == "") && (sEnd != ""))
				{
					// retrieve the last set of bytes identified by sEnd
					try
					{
						offset = 0;
						end = long.Parse(sEnd);
						length = end + 1;
					}
					catch
					{
						errorEncountered = true;
						break;
					}
				}
				else if ((sOffset != "") && (sEnd == ""))
				{
					// retrieve all bytes starting from sOffset
					try
					{
						offset = long.Parse(sOffset);
						end = contentLength - 1;
						length = contentLength - offset;
					}
					catch
					{
						errorEncountered = true;
						break;
					}
				}
				else
				{
					// retrieve bytes from sOffset through sEnd, 
					// inclusive so be sure to add 1 to difference
					try
					{
						offset = long.Parse(sOffset);
						end = long.Parse(sEnd);

						if (offset <= end)
						{
							length = end - offset + 1;
						}
						else
						{
							errorEncountered = true;
						}
					}
					catch
					{
						errorEncountered = true;
						break;
					}
				}

				if (errorEncountered == false)
				{
					System.Diagnostics.Debug.Assert(offset >= 0);
					System.Diagnostics.Debug.Assert(length >= 0);
					System.Diagnostics.Debug.Assert(end >= 0);

					HTTPSession.Range newRange = new HTTPSession.Range(offset, length);
					rangeSets.Add(newRange);
				}
			}

			if (errorEncountered)
			{
				// error parsing value, this is invalid so clear and return
				rangeSets.Clear();
			}

			return rangeSets;
		}


		/// <summary>
		/// This method is called by <see cref="MediaServerDevice.WebServer_OnPacketReceiveSink"/>
		/// and handles the HTTP-GET or HTTP-GET message requests and provides
		/// the appropriate response. In either case, the handler is intended
		/// to handle requests for resource URIs that have been mapped to local
		/// files using the <see cref="MediaResource.AUTOMAPFILE"/> convention.
		/// <para>
		/// If a resource cannot be mapped to a local file, 
		/// then the method will ask upper-layer application logic 
		/// for a stream object to send in response to the request.
		/// This is used primarily in scenarios where the application intentionally
		/// specified a contentUri value for a resource that uses the 
		/// <see cref="MediaResource.AUTOMAPFILE"/> convention but doesn't
		/// actually map to a local file. This gives the upper application layer
		/// to provide transcoded stream objects or some specialized file stream
		/// that is not stored on the local file system.
		/// </para>
		/// <para>
		/// Upper application layers can always opt to leave the stream object
		/// blank, effectively indicating that the HTTP-GET or head request 
		/// cannot be handled because such a file does not exist.
		/// </para>
		/// </summary>
		/// <param name="msg"></param>
		/// <param name="session"></param>
		private void HandleGetOrHeadRequest (HTTPMessage msg, HTTPSession session)
		{
			// Format of DirectiveObj will be
			// "/[res.m_ResourceID]/[item.ID]/[item.Title].[ext]"
			// We want the 
			bool is404 = true;
			Exception problem = null;

			string resourceID = null;
			string objectID = null;

			try
			{
				DText DirectiveParser = new DText();
				DirectiveParser.ATTRMARK = "/";

				DirectiveParser[0] = msg.DirectiveObj;
				resourceID = DirectiveParser[2];
				objectID = DirectiveParser[3];
				IDvResource res = this.GetResource(objectID, resourceID);

				if (res == null)
				{
					throw new Error_GetRequestError(msg.DirectiveObj, null);
				}
				else
				{
					// attempt to figure otu the local file path and the mime type
					string f = MediaResource.AUTOMAPFILE;
					string fileName = res.ContentUri.Substring(f.Length);
					string type = res.ProtocolInfo.MimeType;
							
					if ((type == null) || (type == "") || (type == "*"))
					{
						//content-type not known, programmer
						//that built content-hierarchy didn't provide one

						throw new Error_GetRequestError(msg.DirectiveObj, res);
					}
					else
					{
						// must be a get or head request

						// check if the file actually exists
						if (Directory.Exists(fileName))
						{
							throw new Error_GetRequestError(msg.DirectiveObj, res);
						}

						FileNotMapped mapping = new FileNotMapped();
						mapping.RequestedResource = res;
						mapping.LocalInterface = session.Source.ToString();
						mapping.RedirectedStream = null;
						if (File.Exists(fileName))
						{
							// the file exists, so go ahead and send it
							mapping.RedirectedStream = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read);
						}
						else
						{
							try
							{
								// the file doesn't exist but the owner of this
								// server specified some kind of locally mapped file
								// so perhaps they may want to route a stream object themselves
								if (this.OnFileNotMapped != null)
								{
									this.OnFileNotMapped (this, mapping);
								}
							}
							catch (Exception ofnm)
							{
								mapping.RedirectedStream = null;
							}
						}

						// if the RedirectedStream is blank, then it means
						// no stream can be sent in response to the request

						if (mapping.RedirectedStream != null)
						{
							lock (session)
							{
								// get the intended length, if known
								long expectedLength = -1;
								if (mapping.OverrideRedirectedStreamLength)
								{
									expectedLength = mapping.ExpectedStreamLength;
								}
								else
								{
									expectedLength = mapping.RedirectedStream.Length;
								}

								if (String.Compare(msg.Directive, "HEAD", true) == 0)
								{
									// must be a head request - reply with 200/OK, content type, content length
									HTTPMessage head = new HTTPMessage();
									head.StatusCode = 200;
									head.StatusData = "OK";
									head.ContentType = type;
									if (expectedLength >= 0)
									{
										// if we can calculate the length,
										// then we provide a content-length and
										// also indicate that range requests can be 
										// handled.

										head.OverrideContentLength = true;

										string rangeStr = msg.GetTag("RANGE");
										if ((rangeStr == null) || (rangeStr == ""))
										{
											head.AddTag("CONTENT-LENGTH", expectedLength.ToString());
											head.AddTag("ACCEPT-RANGES", "bytes");
										}
										else
										{
											ArrayList rangeSets = new ArrayList();
											head.StatusCode = 206;
											AddRangeSets(rangeSets, rangeStr.Trim().ToLower(), expectedLength);
											if (rangeSets.Count == 1)
											{
												head.AddTag("Content-Range", "bytes " + ((HTTPSession.Range)(rangeSets[0])).Position.ToString() + "-" + ((int)(((HTTPSession.Range)(rangeSets[0])).Position+((HTTPSession.Range)(rangeSets[0])).Length-1)).ToString() + "/" + expectedLength.ToString());
												head.AddTag("Content-Length", ((HTTPSession.Range)(rangeSets[0])).Length.ToString());
											}
										}
									}
									else
									{
										// can't calculate length => can't do range
										head.AddTag("ACCEPT-RANGES", "none");
									}
									session.Send(head);
									is404 = false;
								}
								else
								{
									ArrayList rangeSets = new ArrayList();
									string rangeStr = msg.GetTag("RANGE");

									// Only allow range requests for content where we have the
									// entire length and also only for requests that have
									// also provided an allowed range.
									if ((rangeStr == null) || (rangeStr != ""))
									{
										if (expectedLength >= 0)
										{
											// validate the requested ranges; if invalid range
											// found, send the entire document...
											AddRangeSets(rangeSets, rangeStr.Trim().ToLower(), expectedLength);
										}
									}

									// must be a get request
									// create an outgoing transfer that is not visible to UPNP
									// GetTransferProgress method, and add the transfer
									HttpTransfer transferInfo = new HttpTransfer(false, false, session, res, mapping.RedirectedStream, expectedLength);
									this.AddTransfer(session, transferInfo);

									if (rangeSets.Count > 0)
									{
										session.SendStreamObject(mapping.RedirectedStream, (HTTPSession.Range[])rangeSets.ToArray(typeof(HTTPSession.Range)), type);
									}
									else
									{
										//start from the beginning
										mapping.RedirectedStream.Seek(0, SeekOrigin.Begin);
										if (expectedLength >= 0)
										{
											session.SendStreamObject(mapping.RedirectedStream, expectedLength, type);
										}
										else
										{
											session.SendStreamObject(mapping.RedirectedStream, type);
										}
									}
									is404 = false;
								}
							}
						}
					}
				}
			}
			catch (Exception error)
			{
				problem = error;
			}

			if (is404)
			{
				StringBuilder sb = new StringBuilder();

				sb.Append("File not found.");
				sb.AppendFormat("\r\n\tRequested: \"{0}\"", msg.DirectiveObj);

				if (objectID != null)
				{
					sb.AppendFormat("\r\n\tObjectID=\"{0}\"", objectID);
				}

				if (resourceID != null)
				{
					sb.AppendFormat("\r\n\tResourceID=\"{0}\"", resourceID);
				}

				Error_GetRequestError getHeadError = problem as Error_GetRequestError;

				if (getHeadError != null)
				{
					sb.Append("\r\n");

					IUPnPMedia mobj = this._GetEntry(objectID);

					if (mobj == null)
					{
						sb.AppendFormat("\r\n\tCould not find object with ID=\"{0}\"", objectID);
					}
					else
					{
						sb.AppendFormat("\r\n\tFound object with ID=\"{0}\"", objectID);
						sb.Append("\r\n---Metadata---\r\n");
						sb.Append(mobj.ToDidl());
					}

					sb.Append("\r\n");

					if (getHeadError.Resource == null)
					{
						sb.Append("\r\n\tResource is null.");
					}
					else
					{
						sb.Append("\r\n\tResource is not null.");

						string uri = getHeadError.Resource.ContentUri;
						if (uri== null)
						{
							sb.Append("\r\n\t\tContentUri of resource is null.");
						}
						else if (uri == "")
						{
							sb.Append("\r\n\t\tContentUri of resource is empty.");
						}
						else
						{
							sb.AppendFormat("\r\n\t\tContentUri of resource is \"{0}\"", uri);
						}
					}
				}

				if (problem != null)
				{
					sb.Append("\r\n");

					Exception e = problem;
					sb.Append("\r\n!!! Exception information !!!");

					while (e != null)
					{
						sb.AppendFormat("\r\nMessage=\"{0}\".\r\nStackTrace=\"{1}\"", e.Message, e.StackTrace);

						e = e.InnerException;
						if (e != null)
						{
							sb.Append("\r\n---InnerException---");
						}
					}
				}

				// file has not been found so return a valid HTTP 404 error message
				HTTPMessage error = new HTTPMessage();
				error.StatusCode = 404;
				error.StatusData = "File not found";
				error.StringBuffer = sb.ToString();
				session.Send(error);
			}
		}

		/// <summary>
		/// Handles the code for when a client attempts to post a binary
		/// to the server. The method attempts to find a resource object
		/// that maps to the specified uri and will then ask upper application
		/// logic for permission to save the binary. Application logic
		/// can reject the request by throwing an exception.
		/// <para>
		/// As an aside, piplelining is really easy for POST messages
		/// because the bodies of POST messages will contain the actual
		/// binary that's getting transfered to the server. Standard pipelining
		/// rules dictate that the next POST message must follow an already 
		/// complete HTTP message. Regardless of whether the request was
		/// GET, POST, or HEAD - setting the WebSession's UserSTream property
		/// should always work.
		/// </para>
		/// </summary>
		/// <param name="msg"></param>
		/// <param name="WebSession"></param>
		private void HandlePostedFileToServer (HTTPMessage msg, HTTPSession WebSession)
		{
			// Format of DirectiveObj will be
			// "/[res.m_ResourceID]/[res.m_Owner.ID]/"
			// 
			DText DirectiveParser = new DText();
			DirectiveParser.ATTRMARK = "/";

			DirectiveParser[0] = msg.DirectiveObj;
			string resourceID = DirectiveParser[2];
			string objectID = DirectiveParser[3];
			
			IDvResource res = this.GetResource(objectID, resourceID);
			WebSession.UserStream = null;

			if (res != null)
			{
				// only receive files that are allow overwriting
				if (res.AllowImport)
				{
					if (this.OnRequestSaveBinary != null)
					{
						// If application has not requested notifications for
						// when something post data, tnen automatically approve
						// the POST. 
					}
					else
					{
						// Allow the application to approve or reject the POST.
						// If the application logic throws an exception,
						// then the WebSession.UserStream field remains null.
						// The message of the exception is used in the HTTP
						// error response to the POST-sender.
						this.OnRequestSaveBinary(this, res);
					}

					// Set the session to write to the following local file.
					// 
					string path = res.ContentUri.Substring(MediaResource.AUTOMAPFILE.Length);

					// attempt to figure out the intended file extension by
					// examining the sender's post request for a content-type.
					// if so, append the extension as appropriately.
					string mime = msg.ContentType;
					string ext = MimeTypes.MimeToExtension(mime);

					if (path.EndsWith(ext) == false)
					{
						path += ext;
					}

					// attempt to figure ot the intended file's length by
					// examining the sender's post request
					long expectedLength = 0;
					try
					{
						expectedLength = this.ExtractContentLength(msg);
					}
					catch
					{
					}

					// Create a stream for the file incoming data,
					// wire up session to dump the incoming data to the stream,
					// create a transfer info block indicating information about 
					// the transfer, and formally add the transfer.
					FileStream fs = new FileStream(path, FileMode.Create, FileAccess.Write, FileShare.None, 4*1024);
					WebSession.UserStream = fs;
					HttpTransfer transferInfo = new HttpTransfer(true, true, WebSession, res, WebSession.UserStream, expectedLength);
					this.AddTransfer(WebSession, transferInfo);
				}
			}
		}


		/// <summary>
		/// If session's state object is null, then create a SessionData
		/// instance as its state object and subscribe to the 
		/// session's OnClosed and OnStreamDone events.
		/// </summary>
		/// <param name="session"></param>
		private void SetupSessionForTransfer (HTTPSession session)
		{
			if (session.StateObject == null)
			{
				session.StateObject = new SessionData();
				session.OnClosed += new HTTPSession.SessionHandler(this.WebSession_OnSessionClosed);
				session.OnStreamDone += new HTTPSession.StreamDoneHandler(this.WebSession_OnStreamDone);
			}
		}

		/// <summary>
		/// Lock the set of existing transfers for the entire server.
		/// Add the new transfer to the session's state object, for future reference.
		/// Obtain a unique transfer ID
		/// Add the new transfer to the set of existing transfers for the entire server.
		/// Unlock the set of existing transfers.
		/// </summary>
		/// <param name="session"></param>
		/// <param name="transferInfo"></param>
		private void AddTransfer(HTTPSession session, HttpTransfer transferInfo)
		{
			this.m_LockHttpTransfers.AcquireWriterLock(-1);

			SessionData sd = (SessionData) session.StateObject;
			sd.Transfers.Enqueue(transferInfo);
			sd.Requested++;

			// obtain unique transfer id
			UInt32 id = (uint) session.GetHashCode();
			while (this.m_HttpTransfers.ContainsKey(id))
			{
				id++;
			}

			this.m_HttpTransfers.Add(id, transferInfo);
			transferInfo.m_TransferId = id;

			this.m_LockHttpTransfers.ReleaseWriterLock();
			this.FireHttpTransfersChange();
		}

		/// <summary>
		/// Marks an HttpTransfer object for removal in 30 seconds.
		/// Also tells an associated resource whether it can
		/// now safely access the automapedd file.
		/// </summary>
		/// <param name="TheSession"></param>
		/// <param name="stream"></param>
		private void MarkTransferForRemoval (HTTPSession TheSession, Stream stream)
		{
			this.m_LockHttpTransfers.AcquireWriterLock(-1);

			SessionData sd = (SessionData) TheSession.StateObject;
			HttpTransfer done = (HttpTransfer) sd.Transfers.Dequeue();
			sd.Completed++;

			UInt32 id = done.TransferID;
			if (this.m_HttpTransfers.ContainsKey(id))
			{
				HttpTransfer transferInfo = (HttpTransfer) this.m_HttpTransfers[id];

				if (transferInfo != done)
				{
					throw new ApplicationException("Bad Evil. The transfers must match.");
				}
				if (transferInfo.Stream != stream)
				{
					throw new ApplicationException("Bad Evil. The streams need to match too.");
				}

				//actually close the stream associated with the transfer
				//so that the file is immediately available for consumption
				transferInfo.Close(false);

				if (transferInfo.Incoming)
				{
					// By calling this method, we should effectively inform
					// a resource that its associated automapped
					// binary file is safe to use
					transferInfo.Resource.CheckLocalFileExists();
				}
			}

			this.m_LockHttpTransfers.ReleaseWriterLock();

			if (done != null)
			{
				this.m_LFT.Add(done, 40);
			}
			else
			{
				throw new ApplicationException("Bad evil. We should always have an HttpTransfer object to remove.");
			}
		}

		/// <summary>
		/// Method actually removes an <see cref="HttpTransfer"/>
		/// object from the mediaserver's list of active transfers.
		/// The method is executed after the LifeTimeMonitor
		/// notes that the prerequisite 30 seconds has passed since
		/// the actual transfer was completed.
		/// Method is called by <see cref="MediaServerDevice.Sink_OnExpired"/>.
		/// </summary>
		/// <param name="transferInfo"></param>
		private void RemoveTransfer (HttpTransfer transferInfo)
		{
			UInt32 id = transferInfo.m_TransferId;
			this.m_LockHttpTransfers.AcquireWriterLock(-1);
			
			bool error = false;
			if (this.m_HttpTransfers.ContainsKey(id))
			{
				HttpTransfer transferInfo2 = (HttpTransfer) this.m_HttpTransfers[id];
				if (transferInfo2 == transferInfo)
				{
					this.m_HttpTransfers.Remove(id);
				}
				else
				{
					error = true;
				}
			}
			else
			{
				error = true;
			}
			this.m_LockHttpTransfers.ReleaseWriterLock();

			if (error)
			{
				throw new Error_TransferProblem(id, transferInfo);
			}

			this.FireHttpTransfersChange();
		}


		/// <summary>
		/// Method executes when the UPnP stack reports that a binary HTTP transfer
		/// has been completed. This applies to both incoming and outgoing 
		/// HTTP based transfers. This does not necessarily indicate that the
		/// HTTP session is over - just that a distinct message (in the form
		/// of a stream object) has completed. 
		/// <para>
		/// This method is supposed to be called before the 
		/// <see cref="MediaServerDevice.WebSession_SessionClosed"/> 
		/// method executes in scenarios where the session actually gets closed.
		/// </para>
		/// <para>
		/// Method is primarily responsible for marking HTTP transfers on a session
		/// into a 30 second timer queue that will cause the eventual removal of 
		/// <see cref="HttpTransfer"/> objects from the queue.
		/// </para>
		/// </summary>
		/// <param name="TheSession"></param>
		/// <param name="stream"></param>
		private void WebSession_OnStreamDone(HTTPSession TheSession, Stream stream)
		{
			lock(TheSession)
			{
				SessionData sd = (SessionData) TheSession.StateObject;

				if (sd.Transfers.Count > 0)
				{
					this.MarkTransferForRemoval(TheSession, stream);
				}
				else
				{
					throw new ApplicationException("bad evil. Can't mark a stream for removal if there's nothing to remove.");
				}

				// Always close the socket if the request was not HTTP 1.1 based
				if (sd.HttpVer1_1 == false)
				{
					//uncomment: TheSession.Close();
				}
			}
		}

		/// <summary>
		/// This method simply tells the session to unwire all of the callbacks
		/// and allows the <see cref="HTTPSession"/> object to properly get
		/// disposed because the callback events are strong references to
		/// this class.
		/// <para>
		/// This method is supposed to be called after the 
		/// <see cref="MediaServerDevice.WebSession_OnStreamDone"/> 
		/// method executes in scenarios where the session actually gets closed.
		/// </para>
		/// </summary>
		/// <param name="TheSession"></param>
		private void WebSession_OnSessionClosed(HTTPSession TheSession)
		{
			TheSession.CancelAllEvents();
		}

		/// <summary>
		/// This method executes when the LifeTimeMonitor has indicated
		/// that the 30 second prerequisite time has passed, so that
		/// <see cref="HttpTransfer"/> objects can be removed from the
		/// MediaServer device's list of active transfers.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="obj"><see cref="HttpTransfer"/> object that can be removed from the server's list</param>
		private void Sink_OnExpired(LifeTimeMonitor sender, object obj)
		{
			if (obj.GetType() == typeof(HttpTransfer))
			{
				this.RemoveTransfer ((HttpTransfer) obj);
			}
			else
			{
				throw new Error_TransferProblem(0, null);
			}
		}

		/// <summary>
		/// This method is used extensively to simply indicate that
		/// a UPnP action was invoked.
		/// </summary>
		private void FireStatsChange()
		{
			if (this.OnStatsChanged != null)
			{
				this.OnStatsChanged(this);
			}
		}

		/// <summary>
		/// This method is responsible for updating the
		/// ContentDirectory's TransferIDs state variable.
		/// <para>
		/// The method works by iterating through the
		/// server's list of transfer objects and 
		/// aggregating the transfers that were initiated through
		/// ImportResource and ExportResource actions
		/// into a string of comma-separated transfer IDs,
		/// and then applying the changes to the state variable.
		/// </para>
		/// </summary>
		private void FireHttpTransfersChange()
		{
			// Lock the listing of transfers for thread-safety.
			this.m_LockHttpTransfers.AcquireReaderLock(-1);

			int i=0;
			ICollection transfers = this.m_HttpTransfers.Values;
			StringBuilder sb = new StringBuilder();

			// Iterate through the transfer objects.
			foreach (HttpTransfer transfer in transfers)
			{
				// If a transfer was initiated through ImportResource or ExportResource, 
				//	then add it to a string.

				if (transfer.ImportExportTransfer)
				{
					UInt32 tid = (UInt32) transfer.TransferID;
					if (i > 0)
					{
						sb.AppendFormat(",{0}", tid.ToString());
					}
					else
					{
						sb.AppendFormat("{0}", tid.ToString());
					}
					i++;
				}
			}

			// Unlock the listing of transfers.
			string transferIDs = this.ContentDirectory.Evented_TransferIDs;
			string newTransfers = sb.ToString();
			this.m_LockHttpTransfers.ReleaseReaderLock();

			// Notify upper layer application logic that the list of 
			// active transfers has changed, if it actually has changed.

			if (this.OnHttpTransfersChanged != null)
			{
				if (string.Compare(transferIDs, newTransfers) != 0)
				{
					// Apply the CSV string to the state variable value
					// which will effectively report all transfers
					// initiated by ImportReosurce and ExportResource.
					this.ContentDirectory.Evented_TransferIDs = newTransfers;
				}

				// Since we want to always report transfers, even if they are
				// not initiated through ImportResource and ExportResource
				this.OnHttpTransfersChanged(this);
			}
		}

		/// <summary>
		/// This method kicks off the processing for writing the DIDL-Lite
		/// responses to Search and Browse actions.
		/// </summary>
		/// <param name="baseUrl">
		/// The http scheme, IP address, port number, and virtual directory that make
		/// up the baseUrl for all http URIs.
		/// </param>
		/// <param name="properties">
		/// ArrayList where each item is a metadata property name, provided 
		/// by the UPNP control point.
		/// </param>
		/// <param name="entries">
		/// The flat listing of media objects that need to be included in the response.
		/// </param>
		/// <returns></returns>
		private static string BuildXmlRepresentation(string baseUrl, ArrayList properties, ICollection entries)
		{
			ToXmlDataDv _d = new ToXmlDataDv();
			_d.BaseUri = baseUrl;
			_d.DesiredProperties = properties;
			_d.IsRecursive = false;

			//string didl = MediaBuilder.BuildDidl(MediaObject.ToXmlFormatter_Default, _d, entries);
			string didl = MediaBuilder.BuildDidl(ToXmlFormatter.DefaultFormatter, _d, entries);
			return didl;
		}

		/// <summary>
		/// This method kicks off the processing for writing the DIDL-Lite
		/// responses to Search and Browse actions.
		/// </summary>
		/// <param name="baseUrls">
		/// A listing of local base URIs to use.
		/// The http scheme, IP address, port number, and virtual directory that make
		/// up the baseUrl for all http URIs.
		/// </param>
		/// <param name="properties">
		/// ArrayList where each item is a metadata property name, provided 
		/// by the UPNP control point.
		/// </param>
		/// <param name="entries">
		/// The flat listing of media objects that need to be included in the response.
		/// </param>
		/// <returns></returns>
		private static string BuildXmlRepresentation(string[] baseUrls, ArrayList properties, ICollection entries)
		{
			ToXmlDataDv _d = new ToXmlDataDv();
			_d.BaseUris = new ArrayList((ICollection) baseUrls);
			_d.DesiredProperties = properties;
			_d.IsRecursive = false;

			//string didl = MediaBuilder.BuildDidl(MediaObject.ToXmlFormatter_Default, _d, entries);
			string didl = MediaBuilder.BuildDidl(ToXmlFormatter.DefaultFormatter, _d, entries);
			return didl;
		}
		
		/// <summary>
		/// This method kicks off the processing for writing the DIDL-Lite
		/// responses to Search and Browse actions.
		/// </summary>
		/// <param name="baseUrls">
		/// A list, where each element is a string with the 
		/// http scheme, IP address, port number, and virtual directory that make
		/// up the baseUrl for all http URIs.
		/// </param>
		/// <param name="properties">
		/// ArrayList where each item is a metadata property name, provided 
		/// by the UPNP control point.
		/// </param>
		/// <param name="entries">
		/// The flat listing of media objects that need to be included in the response.
		/// </param>
		/// <exception cref="InvalidCastException">
		/// Thrown if 'baseUrls' contains a non-string element.
		/// </exception>
		/// <exception cref="ArgumentException">
		/// Thrown if 'baseUrls' is empty.
		/// </exception>
		/// <returns></returns>
		private static string BuildXmlRepresentation(ArrayList baseUrls, ArrayList properties, ICollection entries)
		{
			ToXmlDataDv _d = new ToXmlDataDv();
			
			if (baseUrls.Count == 0)
			{
				throw new ArgumentException("MediaServerDevice.BuildXmlRepresentation() requires that 'baseUrls' be non-empty.");
			}

			// causes invalid cast exception if elements are bad
			foreach (string str in baseUrls);

			_d.BaseUri = null;
			_d.BaseUris = baseUrls;
			_d.DesiredProperties = properties;
			_d.IsRecursive = false;

			//string didl = MediaBuilder.BuildDidl(MediaObject.ToXmlFormatter_Default, _d, entries);
			string didl = MediaBuilder.BuildDidl(ToXmlFormatter.DefaultFormatter, _d, entries);
			return didl;
		}

		/// <summary>
		/// This method creates a transfer ID that can be
		/// applied to an <see cref="HttpTransfer"/> object.
		/// The <see cref="HTTPSession"/> object provides
		/// the base value for the transfer id.
		/// </summary>
		/// <param name="session"></param>
		/// <returns></returns>
		private uint CreateTransferId(HTTPSession session)
		{
			UInt32 id = (uint) session.GetHashCode();
			this.m_LockHttpTransfers.AcquireWriterLock(-1);
			while (this.m_HttpTransfers.ContainsKey(id))
			{
				id++;
			}
			this.m_LockHttpTransfers.ReleaseWriterLock();
			return id;
		}

		/// <summary>
		/// true, if the object has the internal HTTP server 
		/// set to serve content
		/// </summary>
		private bool EnableHttp;

		/// <summary>
		/// Always start with connection ID -1 when creating the first connection object.
		/// </summary>
		private const int StartConnId = -1;

		/// <summary>
		/// Next available connection ID.
		/// </summary>
		private int NextConnId = StartConnId;

		/// <summary>
		/// Maximum number of connections that are possible...
		/// int.MaxValue by default.
		/// </summary>
		public int MaxConnections = int.MaxValue;

		/// <summary>
		/// The name for the virtual directory for HTTP content served from the device.
		/// </summary>
		public string VirtualDirName { get { return this.m_VirtualDirName; } }
		/// <summary>
		/// The name for the virtual directory for HTTP content served from the device.
		/// This string is defined by the device framework.
		/// </summary>
		private string m_VirtualDirName;

		/// <summary>
		/// Locking object for m_HttpTransfers for thread-safety.
		/// </summary>
		private ReaderWriterLock m_LockHttpTransfers = new ReaderWriterLock();
		/// <summary>
		/// Stores the list of active HTTP-based file transfers,
		/// represented as <see cref="HttpTransfer"/> objects
		/// and keyed by transfer ID.
		/// </summary>
		private Hashtable m_HttpTransfers = new Hashtable();

		/// <summary>
		/// Locking object for m_Connections for thread-safety.
		/// </summary>
		private ReaderWriterLock m_LockConnections = new ReaderWriterLock();
		/// <summary>
		/// Stores the list of active connections created through
		/// PrepareForConnection, represented as <see cref="MediaServerDevice.Connection"/>
		/// objects and keyed by connection ID.
		/// </summary>
		private Hashtable m_Connections = new Hashtable();

		/// <summary>
		/// Locking object for m_SourceProtocolInfoSet for thread-safety.
		/// </summary>
		private ReaderWriterLock m_LockSourceProtocolInfo = new ReaderWriterLock();
		/// <summary>
		/// Stores the list of protocolInfo strings of the media server,
		/// represented as <see cref="ProtocolInfoString"/> objects.
		/// </summary>
		private ArrayList m_SourceProtocolInfoSet = new ArrayList();

		/// <summary>
		/// Locking object for m_SinkProtocolInfoSet for thread-safety.
		/// </summary>
		private ReaderWriterLock m_LockSinkProtocolInfo = new ReaderWriterLock();
		/// <summary>
		/// Stores the list of protocolInfo strings of the media server,
		/// represented as <see cref="ProtocolInfoString"/> objects.
		/// </summary>
		private ArrayList m_SinkProtocolInfoSet = new ArrayList();

		/// <summary>
		/// The root container for the content hierarchy associated with
		/// this media server device.
		/// </summary>
		private DvRootContainer m_Root;

		/// <summary>
		/// The <see cref="UPnPDevice"/> object that provides the
		/// interface into the UPnP network and handles the basic
		/// behavior and infrastructure for a UPnP device.
		/// </summary>
		private UPnPDevice Device;

		/// <summary>
		/// The <see cref="IUPnPService"/> object that is added to the
		/// <see cref="MediaServerDevice.Device"/> property to represent
		/// the presence of the Connectionmanager service.
		/// </summary>
		private DvConnectionManager ConnectionManager;
		/// <summary>
		/// The <see cref="IUPnPService"/> object that is added to the
		/// <see cref="MediaServerDevice.Device"/> property to represent
		/// the presence of the ContentDirectory service.
		/// </summary>
		private DvContentDirectory ContentDirectory;

		// NOT USED YET.
		//private ReaderWriterLock m_LockTransports = new ReaderWriterLock();
		//private ArrayList m_AVTransports;
		//private ArrayList m_RenderingControls;

		/// <summary>
		/// Locking object to prevent threading errors when updating the SystemUpdateID
		/// state variable.
		/// </summary>
		private Mutex Lock_SystemUpdateID = new Mutex();
		/// <summary>
		/// Locking object to prevent threading errors when updating the ContainerUpdateIDs
		/// state variable.
		/// </summary>
		private Mutex Lock_ContainerUpdateIDs = new Mutex();

		/// <summary>
		/// Used to obtain string representations of various ContentDirectory
		/// normative XML element names and attribute names.
		/// </summary>
		private static Tags T = Tags.GetInstance();

		/// <summary>
		/// A cache for easily finding a descendent object in the 
		/// media server's content hierarchy.
		/// </summary>
		private Hashtable m_Cache = new Hashtable(2500);

		/// <summary>
		/// Structure stores all of the statistics on
		/// the number of times each action has been invoked.
		/// </summary>
		private Statistics m_Stats = new Statistics();

		/// <summary>
		/// Used in providing a unique virtual directory name for the media server.
		/// </summary>
		private static long VirtualDirCounter=-1;

		/// <summary>
		/// Thsi object provides the delayed-removal behavior of
		/// <see cref="HttpTransfer"/> objects from the active
		/// transfers list, 30 seconds after the transfer
		/// actually completes.
		/// </summary>
		private LifeTimeMonitor m_LFT = new LifeTimeMonitor();

		/// <summary>
		/// Indicates the number of times each action on the
		/// server has been invoked from the UPNP interface.
		/// Each field name represents an available action on the server.
		/// </summary>
		public struct Statistics
		{
			public int Browse;
			public int ExportResource;
			public int StopTransferResource;
			public int DestroyObject;
			public int UpdateObject;
			public int GetSystemUpdateID;
			public int GetTransferProgress;
			public int CreateObject;
			public int ImportResource;
			public int CreateReference;
			public int DeleteResource;
			public int ConnectionComplete;
			public int GetCurrentConnectionInfo;
			public int GetCurrentConnectionIDs;
			public int GetProtocolInfo;
			public int PrepareForConnection;
			public int GetSearchCapabilities;
			public int GetSortCapabilities;
			public int Search;
		}

		/// <summary>
		/// Represents the results for a PrepareForConnection invocation on the server.
		/// </summary>
		public struct Connection
		{
			/// <summary>
			/// The ConnectionID of the connection.
			/// </summary>
			public int ConnectionId;
			
			/// <summary>
			/// The MediaServer's RendererControl service's instance ID that is mapped to this 
			/// connection...should always be -1 unless this server actually
			/// supports a renderer control service.
			/// </summary>
			public int RcsId;
			
			/// <summary>
			/// The MediaServer's AVTransport service's instance ID that is mapped to this 
			/// connection...should always be -1 unless this server actually
			/// supports an AVTransport service.
			/// </summary>
			public int AVTransportId;

			/// <summary>
			/// The protocolInfo specified at the time PrepareForConnection was called.
			/// </summary>
			public ProtocolInfoString ProtocolInfo;

			/// <summary>
			/// the peer connection manager in UDN/Service-ID form provided by the control point.
			/// </summary>
			public string PeerConnectionManager;

			/// <summary>
			/// The remote endpoint's connection ID that maps to this connection.
			/// </summary>
			public int PeerConnectionId;
			
			/// <summary>
			/// Indication if the device is acting as input or output - effectively always
			/// output on the server side.
			/// </summary>
			public DvConnectionManager.Enum_A_ARG_TYPE_Direction Direction;
			
			/// <summary>
			/// Indication on the status of the connection - not really applicable
			/// for HTTP-GET.
			/// </summary>
			public DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus Status;
			
			/// <summary>
			/// Constructor - requires all of the information before the connection is created.
			/// </summary>
			/// <param name="id"></param>
			/// <param name="peerId"></param>
			/// <param name="rcs"></param>
			/// <param name="avt"></param>
			/// <param name="prot"></param>
			/// <param name="peer"></param>
			/// <param name="dir"></param>
			/// <param name="status"></param>
			/// <exception cref="ApplicationException">
			/// Thrown if the proposed connection ID is less than zero.
			/// </exception>
			public Connection (int id, int peerId, int rcs, int avt, ProtocolInfoString prot, string peer, DvConnectionManager.Enum_A_ARG_TYPE_Direction dir, DvConnectionManager.Enum_A_ARG_TYPE_ConnectionStatus status)
			{
				if (id < 0)
				{
					throw new ApplicationException("ConnectionId cannot be negative.");
				}

				ConnectionId = id;
				PeerConnectionId = peerId;
				RcsId = rcs;
				AVTransportId = avt;
				ProtocolInfo = prot;
				PeerConnectionManager = peer;
				Direction = dir;
				Status = status;
			}
		}

		/// <summary>
		/// If a local file cannot be found given a resource that should
		/// theoretically map to it, this class is used in
		/// delegates to redirect streams.
		/// </summary>
		public class FileNotMapped
		{
			/// <summary>
			/// The local interface that should handle the request.
			/// </summary>
			public string LocalInterface;

			/// <summary>
			/// The resource associated with the request.
			/// </summary>
			public IMediaResource RequestedResource;
			/// <summary>
			/// The stream to where the data should be saved to or sent from.
			/// </summary>
			public Stream RedirectedStream;

			/// <summary>
			/// Default value is false and it indicates to <see cref="MediaServerDevice"/>
			/// that it should calculate the content-length of an HTTP request to
			/// equal that of the stream's length. 
			/// 
			/// If the RedirectedStream field is non-null, and the
			/// redirected stream's length field does not have the
			/// complete length (perhaps because the stream does
			/// not have the complete contents for calculating the entire length) 
			/// this field should be set to true. 
			/// </summary>
			public bool OverrideRedirectedStreamLength = false;

			/// <summary>
			/// Default value is -1. If OverrideRedirectedStreamLength is true, then
			/// this value should reflect the total length of the stream.
			/// </summary>
			public long ExpectedStreamLength = -1;
		}

		/// <summary>
		/// Represents an http file being transferred to/from the media server.
		/// </summary>
		public class HttpTransfer
		{
			/// <summary>
			/// Indicates if the transfer is incoming.
			/// </summary>
			public readonly bool Incoming;

			/// <summary>
			/// Indicates a transfer ID - used primarily for UPNP-CDS needs.
			/// </summary>
			public UInt32 TransferID
			{
				get
				{
					return m_TransferId;
				}
			}

			/// <summary>
			/// Provides the last known position of the transfer.
			/// The transfer is not polled, so the current position
			/// is reported on the stream, or the last known
			/// position when the stream was closed.
			/// </summary>
			public long Position
			{
				get
				{
					try
					{
						if (ClosedOrDone)
						{
							return this.lastKnownPos;
						}
					}
					catch
					{
						//int x = 3;
					}

					return this.Stream.Position;
				}
			}

			/// <summary>
			/// This closes the stream, and sets the last known position
			/// so the position can still be obtained after the stream is closed.
			/// </summary>
			/// <param name="deleteFile"></param>
			public void Close(bool deleteFile)
			{
				this.ClosedOrDone = true;

				this.lastKnownPos = this.Stream.Position;
				this.Session.CloseStreamObject(this.Stream);

				if (this.Stream.GetType().ToString() == "System.IO.FileStream")
				{
					if (deleteFile)
					{
						FileStream fs = (FileStream) this.Stream;
						File.Delete(fs.Name);
					}
				}
			}


			/// <summary>
			/// Indicates the resource that the http-file is associated with.
			/// </summary>
			public readonly IDvResource Resource;

			/// <summary>
			/// The expected length of the transfer.
			/// </summary>
			public long TransferSize
			{
				get
				{
					return m_TransferSize;
				}
			}
			internal long m_TransferSize;

			/// <summary>
			/// The stream object associated with the transfer.
			/// </summary>
			internal readonly System.IO.Stream Stream;
			/// <summary>
			/// True, if the stream has been closed or is finished.
			/// </summary>
			private bool ClosedOrDone;
			/// <summary>
			/// Tracks the last known position of the file 
			/// - used only when the stream finishes or is closed.
			/// Otherwise, the stream's position is used.
			/// </summary>
			private long lastKnownPos = 0;
			/// <summary>
			/// True indicates the transfer was initiated through
			/// ImportResource or ExportResource.
			/// </summary>
			internal bool ImportExportTransfer;
			/// <summary>
			/// True indicates an unrecoverable error has occurred
			/// during the transmission.
			/// </summary>
			internal bool CriticalError;
			/// <summary>
			/// The HTTP session that's transmitting the stream forus.
			/// </summary>
			private HTTPSession Session;

			/// <summary>
			/// Returns true if the provided HTTP session object
			/// matches the transfer's session object.
			/// </summary>
			/// <param name="session"></param>
			/// <returns></returns>
			internal bool IsSessionMatch(HTTPSession session)
			{
				return (session == Session) ;
			}
			/// <summary>
			/// The source IP address
			/// </summary>
			public readonly IPEndPoint Source;
			/// <summary>
			/// The destination IP address.
			/// </summary>
			public readonly IPEndPoint Destination;

			/// <summary>
			/// The transfer ID of this object; used for keying.
			/// </summary>
			internal UInt32 m_TransferId;

			/// <summary>
			/// Indicates the general status of the transfer.
			/// </summary>
			public DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus TransferStatus
			{
				get
				{
					if (CriticalError)
					{
						return DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus.ERROR;
					}
					else if ((this.TransferSize == this.Position) && (this.TransferSize != 0))
					{
						return DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus.COMPLETED;
					}
					else if (ClosedOrDone)
					{
						return DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus.STOPPED;
					}
					else
					{
						return DvContentDirectory.Enum_A_ARG_TYPE_TransferStatus.IN_PROGRESS;
					}
				}
			}

			/// <summary>
			/// Constructor - only created by <see cref="MediaServerDevice"/>
			/// when an HTTP-based file transfer occurs for either input or output from
			/// the server.
			/// </summary>
			/// <param name="incoming">required - indicates input or output direction</param>
			/// <param name="importExportTransfer">
			/// required - indicates if transfer was initiated from ImportResource or 
			/// ExportResource action request
			/// </param>
			/// <param name="session">
			/// required - the <see cref="HTTPSession"/> object that's
			/// carrying the stream.</param>
			/// <param name="res">
			/// required - the related <see cref="IDvResource"/> object
			/// that is related to the file transfer
			/// </param>
			/// <param name="stream">
			/// The stream object where the binary data is being stored or obtained from.
			/// </param>
			/// <param name="expectedLength">
			/// The length of the binary data, if known before hand.
			/// </param>
			internal HttpTransfer(bool incoming, bool importExportTransfer, HTTPSession session, IDvResource res, System.IO.Stream stream, long expectedLength)
			{
				this.Incoming = incoming;
				this.ImportExportTransfer = importExportTransfer;
				this.Session = session;

				// explicitly keep the source and remote
				// because if the session gets closed
				// they become null

				this.Source = session.Source;
				this.Destination = session.Remote;
				this.Resource = res;
				this.Stream = stream;
				this.m_TransferSize = expectedLength;
				this.ClosedOrDone = false;
				this.CriticalError = (session == null);
			}
		}
	}
}
