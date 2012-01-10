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
using System.Text;
using System.Drawing;
using System.Threading;
using System.Collections;
using System.Drawing.Imaging;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.DV;

namespace UPnPMediaServerCore
{
	/// <summary>
	/// Summary description for MediaServerCore.
	/// </summary>
	public sealed class MediaServerCore
	{ 
		internal static MediaServerCore serverCore;
		public static int CacheTime = 1800;
		public static string CustomUDN = "";
		public static bool INMPR = true;

		private OpenSource.UPnP.UPnPDeviceWatcher m_DeviceWatcher;
		private MediaServerDevice mediaServer;
		private DvMediaContainer rootContainer;
		private int totalDirectoryCount = 0;
		private int totalFileCount = 0;
		private Hashtable watcherTable = new Hashtable();
		private Hashtable permissionsTable = new Hashtable();

		public delegate void MediaServerCoreDebugHandler(MediaServerCore sender, string message);
		public event MediaServerCoreDebugHandler OnDebugMessage;
		
		public delegate void MediaServerCoreEventHandler(MediaServerCore sender);
		public event MediaServerCoreEventHandler OnStatsChanged;
		public event MediaServerCoreEventHandler OnHttpTransfersChanged;
		public event MediaServerCoreEventHandler OnDirectoriesChanged;

		public delegate void SocketDataHandler(MediaServerCore sender, string socketData);
		public event SocketDataHandler OnSocketData;

		[Serializable()]
		public struct TransferStruct
		{
			public bool Incoming;
			public IPEndPoint Source;
			public IPEndPoint Destination;
			public string ResourceName;
			public long ResourceLength;
			public long ResourcePosition;
		}

		[Serializable()]
		public class SharedDirectoryInfo
		{
			public string directory;
			public bool restricted;
			public bool readOnly;
		}

		[Serializable()]
		private class InnerMediaDirectory 
		{
			public DirectoryInfo directory;
			public string directoryname;
			[NonSerialized()] public FileSystemWatcher watcher;
			public bool restricted;
			public bool readOnly;
		}

		public IList HttpTransfers
		{
			get {return mediaServer.HttpTransfers;}
		}

		public MediaServerDevice.Statistics Statistics
		{
			get {return mediaServer.Stats;}
		}

		public void ResetCoreRoot()
		{
			MediaBuilder.SetNextID(0);
			rootContainer = mediaServer.Root;

			MediaBuilder.container containerImagesInfo = new MediaBuilder.container("All Image Items");
			containerImagesInfo.IsRestricted = true;
			
			MediaBuilder.container containerAudioItemsInfo = new MediaBuilder.container("All Audio Items");
			containerAudioItemsInfo.IsRestricted = true;

			MediaBuilder.container containerVideoItemsInfo = new MediaBuilder.container("All Video Items");
			containerVideoItemsInfo.IsRestricted = true;

			MediaBuilder.container containerPlaylistsInfo = new MediaBuilder.container("All Playlists");
			containerPlaylistsInfo.IsRestricted = true;

			m_ImageItems = (DvMediaContainer) DvMediaBuilder.CreateContainer(containerImagesInfo);

			m_AudioItems = (DvMediaContainer) DvMediaBuilder.CreateContainer(containerAudioItemsInfo);
			
			m_VideoItems = (DvMediaContainer) DvMediaBuilder.CreateContainer(containerVideoItemsInfo);

			m_Playlists = (DvMediaContainer) DvMediaBuilder.CreateContainer(containerPlaylistsInfo);

			rootContainer.AddObject(m_ImageItems, true);
			rootContainer.AddObject(m_AudioItems, true);
			rootContainer.AddObject(m_VideoItems, true);
			rootContainer.AddObject(m_Playlists, true);
		}

		public string SearchCapabilities
		{
			get
			{
				return this.mediaServer.SearchCapabilities;
			}
			set
			{
				this.mediaServer.SearchCapabilities = value;
			}
		}

		public string SortCapabilities
		{
			get
			{
				return this.mediaServer.SortCapabilities;
			}
			set
			{
				this.mediaServer.SortCapabilities = value;
			}
		}

		public MediaServerCore(string friendlyName)
		{
			if (serverCore != null) throw new Exception("Only a single MediaServerCode instance is allowed");
			serverCore = this;

            OpenSource.Utilities.EventLogger.SetLog("Media Server", "MediaServerCode", System.Windows.Forms.Application.ProductVersion.ToString());

			DeviceInfo info = new DeviceInfo();
			info.AllowRemoteContentManagement = true;
			info.FriendlyName = friendlyName;
			info.Manufacturer = "OpenSource";
			info.ManufacturerURL = "";
			info.ModelName = "Media Server";
			info.ModelDescription = "Provides content through UPnP ContentDirectory service";
			info.ModelURL = "";
			info.ModelNumber = "0.765";
			info.LocalRootDirectory = "";
			Tags T = Tags.GetInstance();
			info.SearchCapabilities = "dc:title,dc:creator,upnp:class,upnp:album,res@protocolInfo,res@size,res@bitrate";
			info.SortCapabilities = "dc:title,dc:creator,upnp:class,upnp:album";
			info.EnableSearch = true;
			info.CacheTime = MediaServerCore.CacheTime;
			info.CustomUDN = MediaServerCore.CustomUDN;
			info.INMPR03 = MediaServerCore.INMPR;

			// encode in UTF16 instead of UTF8
			MediaObject.ENCODE_UTF8 = false;

			mediaServer = new MediaServerDevice(info, null, true, "http-get:*:*:*", "");
			mediaServer.OnStatsChanged += new MediaServerDevice.Delegate_MediaServerHandler(StatsChangedChangedSink);
			mediaServer.OnHttpTransfersChanged += new MediaServerDevice.Delegate_MediaServerHandler(HttpTransfersChangedSink);
			mediaServer.OnFileNotMapped = new MediaServerDevice.Delegate_FileNotMappedHandler(this.Handle_OnRequestUnmappedFile);
			mediaServer.OnRequestAddBranch = new MediaServerDevice.Delegate_AddBranch(this.Handle_OnRequestAddBranch);
			mediaServer.OnRequestRemoveBranch = new MediaServerDevice.Delegate_RemoveBranch(this.Handle_OnRequestRemoveBranch);
			mediaServer.OnRequestChangeMetadata = new MediaServerDevice.Delegate_ChangeMetadata(this.Handle_OnRequestChangeMetadata);
			mediaServer.OnRequestSaveBinary = new MediaServerDevice.Delegate_ModifyBinary (this.Handle_OnRequestSaveBinary);
			mediaServer.OnRequestDeleteBinary = new MediaServerDevice.Delegate_ModifyBinary(this.Handle_OnRequestDeleteBinary);
	
			this.ResetCoreRoot();

			this.m_DeviceWatcher = new UPnPDeviceWatcher(mediaServer._Device);
			this.m_DeviceWatcher.OnSniff += new OpenSource.UPnP.UPnPDeviceWatcher.SniffHandler(this.Sink_DeviceWatcherSniff);

			this.mediaServer.Start();
			m_Paused = false;
		}
		private bool m_Paused;
		private DvMediaContainer m_ImageItems;
		private DvMediaContainer m_AudioItems;
		private DvMediaContainer m_VideoItems;
		private DvMediaContainer m_Playlists;

        public void Stop()
        {
            if (this.mediaServer != null)
            {
                this.mediaServer.Stop();
            }
        }

		public void ChangePauseState()
		{
			if (this.IsPaused)
			{
				this.m_Paused = false;
				this.mediaServer.Start();
			}
			else
			{
				this.m_Paused = true;
				this.mediaServer.Stop();
			}
		}

		public bool IsPaused
		{
			get { return this.m_Paused; }
		}

		public void Dispose()
		{
			rootContainer = null;
			mediaServer.Dispose();
			mediaServer = null;
			serverCore = null;
			this.m_DeviceWatcher = null;
			this.m_DeviceWatcher.OnSniff -= new OpenSource.UPnP.UPnPDeviceWatcher.SniffHandler(this.Sink_DeviceWatcherSniff);
		}

		public void StatsChangedChangedSink(MediaServerDevice sender) 
		{
			if (OnStatsChanged != null) OnStatsChanged(this);
		}

		public void HttpTransfersChangedSink(MediaServerDevice sender) 
		{
			if (OnHttpTransfersChanged != null) OnHttpTransfersChanged(this);
		}

		public int TotalDirectoryCount
		{
			get {return totalDirectoryCount;}
		}

		public int TotalFileCount
		{
			get {return totalFileCount;}
		}

		public IList Directories
		{
			get
			{
				ArrayList directories = new ArrayList();
				foreach (DvMediaContainer container in rootContainer.Containers) 
				{
					InnerMediaDirectory imd = (InnerMediaDirectory)(container.Tag);
					if (imd != null) 
					{
						SharedDirectoryInfo sdi = new SharedDirectoryInfo();
						sdi.directory = imd.directoryname;
						sdi.readOnly = imd.readOnly;
						sdi.restricted = imd.restricted;
						directories.Add(sdi);
					}
				}
				return directories;
			} 
		}

		public bool AddDirectory(DirectoryInfo directory) 
		{
			bool retVal = false;

			this.m_LockRoot.WaitOne();
			Exception error = null;
			try
			{
				retVal = AddDirectoryEx(rootContainer,directory);
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();

			if (error != null)
			{
				throw new ApplicationException("AddDirectory() Error", error);
			}

			return retVal;
		}

		/// <summary>
		/// Updates the metadata of container, container's children, and
		/// resources associated with both.
		/// 
		/// <para>
		/// We currently use a file system watcher to manage the metadata
		/// for most things. The only thing we don't do is calcuate the
		/// used storage space of a folder and other storage stats.
		/// </para>
		/// </summary>
		/// <param name="container"></param>
		private void Sink_UpdateContainerMetadata(DvMediaContainer container)
		{
			// Determine the used space by calculating the sum of the storage used by all child objects' resources.
			/*
			long storageUsed = 0;
			if (container.Class.FullClassName.StartsWith("object.container.storage"))
			{
				foreach (IUPnPMedia child in container.CompleteList)
				{
					foreach (IMediaResource res in child.Resources)
					{
						if (res.HasSize)
						{
							// silly UPnP-AV/CDS authors forgot to make size into
							// a signed value, so i'll just fudge the value here
							// so that I remain schema-compliant in every way...
							// AV Forum should eventually change this though.
							long v;
							if (res.Size.m_Value > long.MaxValue)
							{
								v = long.MaxValue;
							}
							else
							{
								v = (long) res.Size.m_Value;
							}
							storageUsed += v;
						}
					}
				}

				container.SetPropertyValue_Long(Tags.GetInstance()[CommonPropertyNames.storageUsed], storageUsed);
			}
			*/
		}

		private void Sink_OnChildRemoved (IDvContainer parent, ICollection removedThese)
		{
			foreach (IUPnPMedia removed in removedThese)
			{
				DvMediaContainer mc = removed as DvMediaContainer;
				if (mc != null)
				{
					mc.OnChildrenRemoved -= new DvDelegates.Delegate_OnChildrenRemove(this.Sink_OnChildRemoved);;
					mc.Callback_UpdateMetadata = null;
				}
			}
		}

		private bool AddDirectoryEx(DvMediaContainer container, DirectoryInfo directory) 
		{
			if (directory.Exists == false) return false;

			MediaBuilder.storageFolder storageInfo = new MediaBuilder.storageFolder(directory.Name);
			storageInfo.Searchable = true;
			storageInfo.IsRestricted = false;
			DvMediaContainer newContainer = DvMediaBuilder.CreateContainer(storageInfo);
			newContainer.OnChildrenRemoved += new DvDelegates.Delegate_OnChildrenRemove(this.Sink_OnChildRemoved);
			newContainer.Callback_UpdateMetadata = new DvMediaContainer.Delegate_UpdateMetadata(this.Sink_UpdateContainerMetadata);
			
			InnerMediaDirectory mediadir = new InnerMediaDirectory();
			mediadir.directory = directory;
			mediadir.directoryname = directory.FullName;
			mediadir.watcher = new FileSystemWatcher(directory.FullName);
			mediadir.watcher.Changed += new FileSystemEventHandler(OnDirectoryChangedSink);
			mediadir.watcher.Created += new FileSystemEventHandler(OnDirectoryCreatedSink);
			mediadir.watcher.Deleted += new FileSystemEventHandler(OnDirectoryDeletedSink);
			mediadir.watcher.Renamed += new RenamedEventHandler(OnFileSystemRenameSink);
			mediadir.restricted = true;
			mediadir.readOnly = true;
			
			watcherTable.Add(mediadir.watcher,newContainer);
			mediadir.watcher.EnableRaisingEvents = true;

			newContainer.Tag = mediadir;

			FileInfo[] files = directory.GetFiles();
			ArrayList addedFiles = new ArrayList(files.Length);
			foreach (FileInfo file in files)
			{
				IDvMedia newObj = CreateObjFromFile(file, new ArrayList());
				if (newObj != null)
				{
					addedFiles.Add(newObj);
					totalFileCount++;
				}
			}

			newContainer.AddObjects(addedFiles, true);

			// Add the new container to an existing container.
			container.AddObject(newContainer, true);

			foreach (IDvMedia item in addedFiles)
			{
				if (item.Class.IsA(MediaBuilder.StandardMediaClasses.AudioItem))
				{
					this.m_AudioItems.AddReference((DvMediaItem)item);
				}
				else if (item.Class.IsA(MediaBuilder.StandardMediaClasses.ImageItem))
				{
					this.m_ImageItems.AddReference((DvMediaItem)item);
				}
				else if (item.Class.IsA(MediaBuilder.StandardMediaClasses.VideoItem))
				{
					this.m_VideoItems.AddReference((DvMediaItem)item);
				}
				else if (item.Class.IsA(MediaBuilder.StandardMediaClasses.PlaylistContainer))
				{
					// References to playlists are not allowed, so we have to build
					// completely new instances of the container and its resources.
					// However the new container can have references to the items.
					DvMediaContainer originalC = (DvMediaContainer) item;
					
					MediaBuilder.playlistContainer plcInfo = new MediaBuilder.playlistContainer(originalC.Title);
					DvMediaContainer newC = DvMediaBuilder.CreateContainer(plcInfo);

					foreach (DvMediaResource res in originalC.Resources)
					{
						ResourceBuilder.AllResourceAttributes resAttribs = new ResourceBuilder.AllResourceAttributes();
						resAttribs.contentUri = res.ContentUri;
						foreach (string attribute in res.ValidAttributes)
						{
							object obj = res[attribute];
							_RESATTRIB attribName = (_RESATTRIB) Enum.Parse(typeof(_RESATTRIB), attribute, true);

							switch (attribName)
							{
								case _RESATTRIB.bitrate:
									resAttribs.bitrate = (_UInt) obj;
									break;
								case _RESATTRIB.bitsPerSample:
									resAttribs.bitsPerSample = (_UInt) obj;
									break;
								case _RESATTRIB.colorDepth:
									resAttribs.colorDepth = (_UInt) obj;
									break;
								case _RESATTRIB.duration:
									resAttribs.duration = (_TimeSpan) obj;
									break;
								case _RESATTRIB.importUri:
									//do not allow import
									break;
								case _RESATTRIB.nrAudioChannels:
									resAttribs.nrAudioChannels = (_UInt) obj;
									break;
								case _RESATTRIB.protection:
									resAttribs.protection = (string) obj;
									break;
								case _RESATTRIB.protocolInfo:
									resAttribs.protocolInfo = new ProtocolInfoString(((ProtocolInfoString)obj).ToString());
									break;
								case _RESATTRIB.resolution:
									resAttribs.resolution = (ImageDimensions) obj;
									break;
								case _RESATTRIB.sampleFrequency:
									resAttribs.sampleFrequency = (_UInt) obj;
									break;
								case _RESATTRIB.size:
									resAttribs.size = (_ULong) obj;
									break;
							}
						}
						DvMediaResource newCR = DvResourceBuilder.CreateResource(resAttribs, false);
						newCR.AllowImport = res.AllowImport;
						newCR.CheckAutomapFileExists = res.CheckAutomapFileExists;
						newCR.HideContentUri = res.HideContentUri;
						newCR.MakeStreamAtHttpGetTime = res.MakeStreamAtHttpGetTime;
						newCR.Tag = res.Tag;

						newC.AddResource(newCR);
					}

					// The child container should only have items.
					// If the child-container has child containers, then
					// then this is bad because those child playlists
					// should have been converted to a flat list
					// in the recursive building of the hierarchy
					foreach (DvMediaItem childItem in originalC.CompleteList)
					{
						newC.AddReference(childItem);
					}

					this.m_Playlists.AddObject(newC, true);
				}
			}

			DirectoryInfo[] directories = directory.GetDirectories();
			foreach (DirectoryInfo dir in directories)
			{
				AddDirectoryEx(newContainer,dir);
			}

			totalDirectoryCount++;
			if (OnStatsChanged != null) OnStatsChanged(this);
			return true;
		}

		public bool RemoveDirectory(DirectoryInfo directory) 
		{
			DvMediaContainer selectedContainer = null;
			foreach (DvMediaContainer container in rootContainer.Containers) 
			{
				InnerMediaDirectory mediadir = (InnerMediaDirectory)container.Tag;
				if (mediadir != null && directory.FullName == mediadir.directory.FullName)
				{
					selectedContainer = container;
					break;
				}
			}
			if (selectedContainer == null) return false;

			RemoveContainerEx(selectedContainer);
			rootContainer.RemoveBranch(selectedContainer);
			if (OnStatsChanged != null) OnStatsChanged(this);
			return true;
		}

		private void RemoveContainerEx(DvMediaContainer container) 
		{
			InnerMediaDirectory mediadir = (InnerMediaDirectory)container.Tag;
			mediadir.watcher.EnableRaisingEvents = false;
			watcherTable.Remove(mediadir.watcher);
			mediadir.watcher.Changed -= new FileSystemEventHandler(OnDirectoryChangedSink);
			mediadir.watcher.Created -= new FileSystemEventHandler(OnDirectoryCreatedSink);
			mediadir.watcher.Deleted -= new FileSystemEventHandler(OnDirectoryDeletedSink);
			mediadir.watcher.Renamed -= new RenamedEventHandler(OnFileSystemRenameSink);
			mediadir.watcher.Dispose();
			mediadir.watcher = null;
			mediadir.directory = null;

			RemoveInnerContainers(container);

			totalDirectoryCount--;
			totalFileCount -= container.Items.Count;
			if (OnStatsChanged != null) OnStatsChanged(this);
		}

		private void RemoveInnerContainers(DvMediaContainer container) 
		{
			foreach (DvMediaContainer c in container.Containers) 
			{
				InnerMediaDirectory mediadir = c.Tag as InnerMediaDirectory;
				if (mediadir != null)
				{
					mediadir.watcher.EnableRaisingEvents = false;
					watcherTable.Remove(mediadir.watcher);
					mediadir.watcher.Changed -= new FileSystemEventHandler(OnDirectoryChangedSink);
					mediadir.watcher.Created -= new FileSystemEventHandler(OnDirectoryCreatedSink);
					mediadir.watcher.Deleted -= new FileSystemEventHandler(OnDirectoryDeletedSink);
					mediadir.watcher.Renamed -= new RenamedEventHandler(OnFileSystemRenameSink);
					mediadir.watcher.Dispose();
					mediadir.watcher = null;
					mediadir.directory = null;

					RemoveInnerContainers(c);
					totalFileCount -= c.Items.Count;
					container.RemoveBranch(c);
					totalDirectoryCount--;
					if (OnStatsChanged != null) OnStatsChanged(this);
				}
				else if (c.Tag.GetType() == typeof(FileInfo))
				{
					// this is a playlist container, so decrement
					// the file count
					totalFileCount--;
				}
			}
		}

		public bool UpdatePermissions(DirectoryInfo directory, bool restricted, bool readOnly)
		{
			DvMediaContainer selectedContainer = null;
			foreach (DvMediaContainer container in rootContainer.Containers) 
			{
				InnerMediaDirectory mediadir = (InnerMediaDirectory)container.Tag;
				if (mediadir != null && directory.FullName == mediadir.directory.FullName)
				{
					selectedContainer = container;
					break;
				}
			}
			if (selectedContainer == null) return false;

			InnerMediaDirectory mediadir2 = (InnerMediaDirectory)selectedContainer.Tag;
			mediadir2.restricted = restricted;
			mediadir2.readOnly = readOnly;

			UpdatePermissionsEx(selectedContainer, restricted, readOnly);

			if (OnStatsChanged != null) OnStatsChanged(this);
			return true;
		}

		public void UpdatePermissionsEx(DvMediaContainer container, bool restricted, bool readOnly)
		{
//			if (readOnly) 
//			{
//				//container.WriteStatus = OpenSource.UPnP.AV.CdsMetadata.EnumWriteStatus.NOT_WRITABLE;
//			} 
//			else 
//			{
//				//container.WriteStatus = OpenSource.UPnP.AV.CdsMetadata.EnumWriteStatus.WRITABLE;
//			}
			container.IsRestricted = restricted;

			foreach (DvMediaItem i in container.Items) 
			{
//				if (readOnly)
//				{
//					//i.WriteStatus = OpenSource.UPnP.AV.CdsMetadata.EnumWriteStatus.NOT_WRITABLE;
//				} 
//				else 
//				{
//					//i.WriteStatus = OpenSource.UPnP.AV.CdsMetadata.EnumWriteStatus.WRITABLE;
//				}
				i.IsRestricted = restricted;
				
				foreach (DvMediaResource res in i.Resources)
				{
					if ((container.IsRestricted == false) && (i.IsRestricted == false))
					{
						res.AllowImport = true;
					}
					else
					{
						res.AllowImport = false;
					}
				}
			}
			
			foreach (DvMediaContainer c in container.Containers) 
			{
				UpdatePermissionsEx(c,restricted,readOnly);
			}
		}

		private void Handle_OnRequestUnmappedFile (MediaServerDevice sender, MediaServerDevice.FileNotMapped getPacket)
		{
			string path_query = getPacket.RequestedResource.ContentUri.Substring(DvMediaResource.AUTOMAPFILE.Length);

			DText path_queryParser = new DText();

			if (getPacket.RequestedResource.Owner.Class.IsA(MediaBuilder.StandardMediaClasses.ImageItem))
			{
				path_queryParser.ATTRMARK = "?";
				path_queryParser.MULTMARK = "=";
				path_queryParser.SUBVMARK = ",";
				path_queryParser[0] = path_query;

				string path = path_queryParser[1];
				string query = path_queryParser[2];
				string format = path_queryParser[2,1];
				string resolution = path_queryParser[2,2];

				string resX = path_queryParser[2,2,1];
				string resY = path_queryParser[2,2,2];

				int width = int.Parse(resX);
				int height = int.Parse(resY);
				int max = Math.Max(width, height);

				Image image = Image.FromFile(path);
				Image thumbnail = image.GetThumbnailImage(width, height, null, IntPtr.Zero);

				getPacket.RedirectedStream = new MemoryStream();
				thumbnail.Save(getPacket.RedirectedStream, ImageFormat.Jpeg);
			}
			else if (
				(getPacket.RequestedResource.Owner.Class.IsA(MediaBuilder.StandardMediaClasses.PlaylistContainer)) ||
				(getPacket.RequestedResource.Owner.Class.IsA(MediaBuilder.StandardMediaClasses.PlaylistItem))
				)
			{
				path_queryParser.ATTRMARK = "?";
				path_queryParser.MULTMARK = "=";
				path_queryParser[0] = path_query;

				string path = path_queryParser[1];
				string query = path_queryParser[2];
				string format = path_queryParser[2,1];
				string baseUrl = getPacket.LocalInterface;

				FileInfo fi = (FileInfo) ((DvMediaResource) getPacket.RequestedResource).Tag;

				//TODO: build m3u
				MemoryStream m3u = new MemoryStream((int)fi.Length * 5);	
				StreamWriter sw = new StreamWriter(m3u, System.Text.Encoding.UTF8);
				getPacket.RedirectedStream = m3u;

				sw.Write("\n");
				sw.Flush();
				m3u.Position = 0;
				
				DvMediaContainer dvc = (DvMediaContainer) getPacket.RequestedResource.Owner;

				sw.WriteLine("#EXTM3U");
				foreach (DvMediaItem dvi in dvc.CompleteList)
				{
					StringBuilder extInfo = new StringBuilder(dvi.Title.Length + dvi.Creator.Length + 15);
					StringBuilder uri = new StringBuilder(1024);

					//digital home looks for a single '-' to delimit creator/title
					extInfo.AppendFormat("#EXTINF:-1,{0} - {1}", dvi.Creator.Replace("-", "_"), dvi.Title.Replace("-", "_"));
					sw.WriteLine(extInfo.ToString());

					IList resources = dvi.MergedResources;
					DvMediaResource res = resources[0] as DvMediaResource;
					if (res != null)
					{
						if (res.ContentUri.StartsWith(MediaResource.AUTOMAPFILE))
						{
							uri.AppendFormat("http://{0}/{1}{2}", baseUrl, mediaServer.VirtualDirName, res.RelativeContentUri);
						}
						else
						{
							uri.Append(res.ContentUri);
						}

						sw.WriteLine(uri);
					}
					else
					{
						StringBuilder msg = new StringBuilder();
						if (resources.Count == 0)
						{
							msg.AppendFormat("MediaServerCore.Handle_OnRequestUnmappedFile() encountered a media object ID=\"{0}\" Title=\"{1}\" with zero resources.", dvi.ID, dvi.Title);
						}
						else
						{
							msg.AppendFormat("MediaServerCore.Handle_OnRequestUnmappedFile() encountered a media object ID=\"{0}\" Title=\"{1}\" with resource that is not a DvMediaResource.", dvi.ID, dvi.Title);
						}
						OpenSource.Utilities.EventLogger.Log(msg.ToString());
					}
				}

				sw.Flush();
			}
		}
		private static DText DT = new DText();

		private void Handle_OnRequestSaveBinary(MediaServerDevice sender, IDvResource res)
		{
			Exception error = null;
			this.m_LockRoot.WaitOne();
			try
			{// Throw exceptions if files are to not be deleted.
				// 
				if (res.AllowImport == false)
				{
					throw new Error_AccessDenied("The resource cannot be overwritten or created.");
				}
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();
			if (error != null)
			{
				throw new ApplicationException("Handle_OnRequestSaveBinary()", error);
			}

		}
			
		private void Handle_OnRequestDeleteBinary(MediaServerDevice sender, IDvResource res)
		{
			Exception error = null;
			this.m_LockRoot.WaitOne();
			try
			{

				// Throw exceptions if the file should not be deleted.
				// 
				if (res.AllowImport == false)
				{
					//throw new Error_AccessDenied("The resource cannot be deleted.");
				}

				/// The UPNP layer will not delete the actual file from the local file system.
				/// I should note that if multiple resources are bound to the same file, then
				/// the logic in this method prevents the file from being deleted.
				/// 
				if (res.ContentUri.StartsWith(DvMediaResource.AUTOMAPFILE))
				{
					string filename = res.ContentUri.Substring(DvMediaResource.AUTOMAPFILE.Length);
					File.Delete(filename);
				}
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();
			if (error != null)
			{
				throw new ApplicationException("Handle_OnRequestDeleteBinary()", error);
			}

		}

		private void Handle_OnRequestAddBranch(MediaServerDevice sender, DvMediaContainer parentContainer, ref IDvMedia[] addTheseBranches)
		{
			Exception error = null;
			this.m_LockRoot.WaitOne();

			try
			{
				if (parentContainer == this.mediaServer.Root)
				{
					throw new Error_RestrictedObject("Cannot create objects directly in the root container.");
				}
				else if (parentContainer.IsRestricted)
				{
					throw new Error_RestrictedObject("Cannot create objects in a restricted container.");
				}

				InnerMediaDirectory mediadir = (InnerMediaDirectory) parentContainer.Tag;
				bool allowNewLocalResources = true;
				if (mediadir != null)
				{
					allowNewLocalResources = Directory.Exists(mediadir.directory.FullName);
				}
				foreach (IDvMedia branch in addTheseBranches)
				{
					// Throw an exception if ANYTHING is bad. Add is always
					// atomic per request.
					// 
					ValidateBranch(parentContainer, branch, allowNewLocalResources);
				}

				foreach (IDvMedia branch in addTheseBranches)
				{
					// No exceptions were thrown so go ahead and add new
					// files and directories to the local file system
					// for the new tree.
					//
					if (branch.IsReference == true)
					{
						parentContainer.AddBranch(branch);
					}
					else
					{
						ModifyLocalFileSystem(parentContainer, branch);
					}
				}
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();

			if (error != null)
			{
				throw new ApplicationException("Handle_OnRequestAddBranch()", error);
			}
		}

		private void ModifyLocalFileSystem(DvMediaContainer branchFrom, IDvMedia branch)
		{
			DvMediaContainer parent = (DvMediaContainer) branchFrom;

			IList resources = branch.Resources;

			if (branch.IsContainer)
			{
				DvMediaContainer container = (DvMediaContainer) branch;
				if (container.Class.ToString().StartsWith("object.container.storage"))
				{
					// This container is a storage container of some sort
					// so it should map to local directory.
					// 
					string newDirPath = "";
					if (container.Tag != null)
					{
						newDirPath = container.Tag.ToString();
					}

					// Make a directory representing this container
					// and set the container's tag to point to the
					// media dir info.
					// 
					DirectoryInfo newDirInfo = Directory.CreateDirectory(newDirPath);
					InnerMediaDirectory mediadir = new InnerMediaDirectory();
					mediadir.directory = newDirInfo;
					mediadir.directoryname = newDirInfo.FullName;
					container.Tag = mediadir;
				}
				else if (container.Class.ToString().StartsWith("object.container"))
				{
					// This is a container, but it doesn't nececessarily map
					// to a local directory. The container subtree may have
					// items and references, so persist them virtually 
					// in the hierarchy but not on disk.
					// 
					container.Tag = null;
				}

				foreach (IDvMedia child in container.CompleteList)
				{
					ModifyLocalFileSystem(container, child);
				}
			}

		}


		private void CreateResources(ICollection resources)
		{
			foreach (DvMediaResource res in resources)
			{
				string uri = res.ContentUri;
				if (uri.StartsWith(DvMediaResource.AUTOMAPFILE))
				{
					string filePath = res.ContentUri.Substring(DvMediaResource.AUTOMAPFILE.Length);
				
					if (File.Exists(filePath))
					{
						// The file exists, so simply keep a reference in the 
						// content hierarchy.... although one would hope it
						// gets persisted in some way, perhaps with a local
						// file shortcut.
						// 
						res.AllowImport = false;
						res.HideContentUri = false;
					}
					else
					{
						// The file doesn't exist, which means it's a file
						// intended as an upload destination. 
						// 
						// Create a placeholder for now.
						// 
						res.AllowImport = true;
						res.HideContentUri = true;
					}
				}
			}
		}

		private void ValidateBranch(DvMediaContainer branchFrom, IDvMedia branch, bool allowNewLocalResources)
		{
			DvMediaContainer parent = branchFrom;
			string basePath = "";
			if (allowNewLocalResources)
			{
				/// The parent container's TAG field will have one of two things:
				///		1) string representation of the local path that it maps to..
				///		2) InnerMediaDirectory structure describing the local path that it maps to.
				///		3) Empty string value or null if no mapping to a local file.
				///		
				if (parent.Tag == null)
				{
				}
				else if (parent.Tag.GetType() == new InnerMediaDirectory().GetType())
				{
					InnerMediaDirectory imd = (InnerMediaDirectory) parent.Tag;
					basePath = imd.directory.FullName + "\\";
				}
				else if (parent.Tag.GetType() == typeof(string))
				{
					basePath = parent.Tag + "\\";
				}

				/// The new branch must be a storage container in order to
				/// map to a local file of sort. May consider making
				/// the class more restricted to object.container.storageFolder.
				/// 
				if (branch.IsContainer)
				{
					DvMediaContainer container = (DvMediaContainer) branch;
					
					if (container.Class.ToString().StartsWith("object.container.storage"))
					//if (true)
					{
						container.Tag = basePath + container.Title;
					}
					else
					{
						container.Tag = null;
					}
				}
			}

			if (branch.IsContainer)
			{
				DvMediaContainer container = (DvMediaContainer) branch;
				foreach (IDvMedia child in container.CompleteList)
				{
					ValidateBranch(container, child, allowNewLocalResources);
				}
			}
			else if (branch.IsItem)
			{
			}
			else
			{
				throw new Exception("Error: Could not validate branch. Branch must be a container, reference, or item.");
			}

			if (branch.IsReference == false)
			{
				/// Each of these resources should be actual children of the item.
				/// They should not be resources obtained through a reference.
				/// This impelementation does not allow references to declare
				/// their own resources.
				/// 
				IList resources = branch.Resources;
				if (resources != null)
				{
					if (resources.Count > 0)
					{
						foreach (DvMediaResource res in resources)
						{
							if (res.ContentUri.StartsWith(DvMediaResource.AUTOMAPFILE))
							{
								if (allowNewLocalResources)
								{
									string filePath = res.ContentUri.Substring(DvMediaResource.AUTOMAPFILE.Length);

									if (File.Exists(filePath))
									{
										if (Directory.Exists(filePath))
										{
											throw new UPnPCustomException(810, "The specified local file-uri is a directory. (" +res.ContentUri+")");
										}

										FileInfo fi = new FileInfo(filePath);

										string protocol = "http-get";
										string network = "*";
										string mime;
										string classType;
										MimeTypes.ExtensionToMimeType(fi.Extension, out mime, out classType);
										string info = "*";

										StringBuilder sb = new StringBuilder(100);
										sb.AppendFormat("{0}:{1}:{2}:{3}", protocol, network, mime, info);

										ProtocolInfoString protInfo = new ProtocolInfoString(sb.ToString());
										res.SetProtocolInfo(protInfo);
									}
									else
									{
										throw new UPnPCustomException(811, "The specified local file-uri does not exist. (" +res.ContentUri+")");
									}
								}
								else
								{
									throw new Error_BadMetadata("Cannot create local http-get resources that are descendents from the specified container.");
								}
							}
							else if ((res.ContentUri == "") && (basePath != ""))
							{
								// Get a unique filename where we can save the binary when it gets imported
								// or sent.
								string filePath = res.GenerateLocalFilePath(basePath);

								/// Set the content uri to map to the determined local path...
								/// although it's understood that the file does not exist yet.
								/// 
								res.SetContentUri(filePath);
							}
							else if (res.ContentUri != "")
							{
								string importUri = res.ImportUri;
								//int x = 3;
							}
							else
							{
								throw new Error_RestrictedObject("The container specified does not allow creation of storage containers or resources.");
							}
						}
					}
				}
			}
		}

		private void Handle_OnRequestRemoveBranch (MediaServerDevice sender, DvMediaContainer parentContainer, IDvMedia removeThisBranch)
		{
			Exception error = null;
			this.m_LockRoot.WaitOne();
			try
			{
				parentContainer.RemoveBranch(removeThisBranch);
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();
			if (error != null)
			{
				throw new ApplicationException("Handle_OnRequestRemoveBranch()", error);
			}

		}

		private void Handle_OnRequestChangeMetadata (MediaServerDevice sender, IDvMedia oldObject, IDvMedia newObject)
		{
			Exception error = null;
			this.m_LockRoot.WaitOne();

			try
			{

				/// TODO: Add permissions logic
				/// 
				/// The following metadata fields must be the same between the old and new objects.
				///		ID,
				///		refID
				///		Parent
				///		IsRestricted
				///		
				if (oldObject.IsRestricted)
				{
					//Allow everything for now
					//throw new Error_RestrictedObject("Cannot modify a restricted object.");
				}

				if (oldObject.ID != newObject.ID)
				{
					throw new Error_ReadOnlyTag("Cannot modify ID");
				}
			
				if (oldObject.IsContainer != newObject.IsContainer)
				{
					throw new Error_BadMetadata("Cannot change containers into items.");
				}
			
				if (oldObject.IsItem != newObject.IsItem)
				{
					throw new Error_BadMetadata("Cannot change items into containers.");
				}

				if (oldObject.IsRestricted != newObject.IsRestricted)
				{
					throw new Error_ReadOnlyTag("Cannot change the \"restricted\" attribute.");
				}

				if ((oldObject.IsReference) || (newObject.IsReference))
				{
					if (oldObject.IsReference == newObject.IsReference)
					{
						DvMediaItem oldItem = (DvMediaItem) oldObject;
						DvMediaItem newItem = (DvMediaItem) newObject;

						string refid1 = oldItem.RefID;
						string refid2 = newItem.RefID;

						if (string.Compare(refid1, refid2) != 0)
						{
							throw new Error_ReadOnlyTag("Cannot change the \"refID\" attribute.");
						}
					}
					else
					{
						throw new Error_BadMetadata("Cannot change a reference item into a non-reference.");
					}
				}
			}
			catch (Exception e)
			{
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();
			if (error != null)
			{
				throw new ApplicationException("Handle_OnRequestChangeMetadata()", error);
			}

			// TODO: Actually check validity for metadata of media objects and their resources.

		}

		private void OnDirectoryChangedSink(object sender, FileSystemEventArgs e)
		{
			//Debug("OnDirectoryChangedSink: " + e.ChangeType.ToString());
		}

		private void OnDirectoryCreatedSink(object sender, FileSystemEventArgs e)
		{
			FileSystemWatcher watcher = (FileSystemWatcher)sender;
			DvMediaContainer container = (DvMediaContainer)watcherTable[watcher];
			if (container == null) 
			{
				if (OnDebugMessage != null) OnDebugMessage(this,"WATCH EVENT FOR UNKNOWN CONTAINER");
			}

			if (File.Exists(e.FullPath) == true) 
			{
				container.AddBranch(CreateObjFromFile(new FileInfo(e.FullPath), new ArrayList()));
				totalFileCount++;
				Debug("File System Create: "+e.Name);
				if (OnStatsChanged != null) OnStatsChanged(this);
			} 
			else if (Directory.Exists(e.FullPath) == true)
			{
				AddDirectoryEx(container, new DirectoryInfo(e.FullPath));
				Debug("File System Dir Create: "+e.Name);
			}
		}

		private void OnDirectoryDeletedSink(object sender, FileSystemEventArgs e)
		{
			FileSystemWatcher watcher = (FileSystemWatcher)sender;
			DvMediaContainer container = (DvMediaContainer)watcherTable[watcher];
			if (container == null) 
			{
				if (OnDebugMessage != null) OnDebugMessage(this,"WATCH EVENT FOR UNKNOWN CONTAINER");
			}

			// If this is a file, this block of code will work
			DvMediaItem changeditem = null;
			foreach (DvMediaItem item in container.Items) 
			{
				foreach (DvMediaResource res in item.Resources)
				{
					if (res.ContentUri.ToLower() == e.FullPath.ToLower()) 
					{
						changeditem = item;
						break;
					}
				}
			}

			if (changeditem != null) 
			{
				container.RemoveBranch(changeditem);
				totalFileCount--;
				if (OnStatsChanged != null) OnStatsChanged(this);
				Debug("File System Delete: "+e.Name);
				return;
			}

			DvMediaContainer changedcontainer = null;
			foreach (DvMediaContainer c in container.Containers)
			{
				InnerMediaDirectory mediadir = (InnerMediaDirectory)c.Tag;
				if (mediadir.directory.FullName.ToLower() == e.FullPath.ToLower()) 
				{
					changedcontainer = c;
					break;
				}
			}

			if (changedcontainer != null) 
			{
				RemoveContainerEx(changedcontainer);
				container.RemoveBranch(changedcontainer);
				Debug("File System Dir Delete: "+e.Name);
				return;
			}

			Debug("FAILED File System Delete: "+e.Name);
		}

		private void OnFileSystemRenameSink(object sender,RenamedEventArgs e)
		{
			FileSystemWatcher watcher = (FileSystemWatcher)sender;
			DvMediaContainer container = (DvMediaContainer)watcherTable[watcher];
			if (container == null) 
			{
				if (OnDebugMessage != null) OnDebugMessage(this,"WATCH EVENT FOR UNKNOWN CONTAINER");
			}

			if (File.Exists(e.FullPath) == true) 
			{
				DvMediaItem changeditem = null;
				foreach (DvMediaItem item in container.Items) 
				{
					foreach (DvMediaResource res in item.Resources)
					{
						Uri uri = new Uri(res.ContentUri);
						if (uri.LocalPath.ToLower() == e.OldFullPath.ToLower()) 
						{
							changeditem = item;
							break;
						}
					}
				}

				if (changeditem != null) 
				{
					container.RemoveBranch(changeditem);
					container.AddBranch(CreateObjFromFile(new FileInfo(e.FullPath), new ArrayList()));
					Debug("File System Rename: "+e.OldName+"->"+e.Name);
				}
				else
				{
					Debug("FAILED File System Rename: "+e.OldName+"->"+e.Name);
				}
			} 
			else if (Directory.Exists(e.FullPath) == true)
			{
				DvMediaContainer changedcontainer = null;
				foreach (DvMediaContainer c in container.Containers)
				{
					InnerMediaDirectory mediadir = (InnerMediaDirectory)c.Tag;
					if (mediadir.directory.FullName.ToLower() == e.OldFullPath.ToLower()) 
					{
						changedcontainer = c;
						break;
					}
				}

				if (changedcontainer != null) 
				{
					RemoveContainerEx(changedcontainer);
					container.RemoveBranch(changedcontainer);
					AddDirectoryEx(container, new DirectoryInfo(e.FullPath));
					Debug("File System Dir Rename: "+e.OldName+"->"+e.Name);
				}
				else
				{
					Debug("FAILED File System Dir Rename: "+e.OldName+"->"+e.Name);
				}
			}

		}

		private void Debug(string msg) 
		{
			if (OnDebugMessage != null) OnDebugMessage(this,msg);
		}

		private ArrayList m_MimeTypes = new ArrayList();

		private IDvMedia CreateObjFromFile(FileInfo file, ArrayList childPlaylists) 
		{
			IDvMedia retVal = null;

			string ext = file.Extension.ToUpper();
			string mime = null, mclass = null;

			switch (ext)
			{
				case ".ASF":
				case ".AVI":
				case ".WMV":
				case ".MPEG":
				case ".MPEG2":
				case ".MPG":
					retVal = CreateItemFromGenericVideoFile(file);
					break;

				case ".WAV":
				case ".WMA":
				case ".MP3":
					DvMediaItem item = CreateItemFromMp3WmaFile(file);
					if (item == null)
					{
						item = this.CreateAudioItemFromFormatedNameFile(file);
					}
					retVal = item;
					break;

				case ".M3U":
					retVal = this.CreateM3uPlaylistContainer(file, childPlaylists);
					break;

				case ".ASX":
					break;

				case ".GIF":
				case ".JPG":
				case ".BMP":
				case ".TIF":
				case ".PNG":
					retVal = CreateItemFromImageFile(file);
					break;
				
				case ".CDSLNK":
					retVal = CreateItemFromCdsLink(file);
					break;

				default:
					retVal = CreateItemFromGenericFile(file);
					break;
			}

			if (retVal != null)
			{
				MimeTypes.ExtensionToMimeType(ext, out mime, out mclass);
			}

			if (mime != null)
			{
				if (this.m_MimeTypes.Contains(mime) == false)
				{
					this.m_MimeTypes.Add(mime);
					ProtocolInfoString[] ps = new ProtocolInfoString[this.m_MimeTypes.Count];
					for (int i=0; i < this.m_MimeTypes.Count; i++)
					{
						ps[i] = new ProtocolInfoString("http-get:*:"+this.m_MimeTypes[i].ToString()+":*");
					}
					this.mediaServer.SourceProtocolInfoSet = ps;
				}
			}

			return retVal;
		}

		private DvMediaContainer CreateM3uPlaylistContainer(FileInfo file, ArrayList childPlaylists)
		{
			// prevent an infinite loop of playlists containing each other
			if (childPlaylists.Contains(file.Name) == false)
			{
				childPlaylists.Add(file.Name);
			}
			else
			{
				return null;
			}

			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);
			
			string title = Path.GetFileNameWithoutExtension(file.Name);
			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();

			MediaBuilder.playlistContainer info = new MediaBuilder.playlistContainer(title);
			DvMediaContainer newPC = DvMediaBuilder.CreateContainer(info);
			newPC.Tag = file;

			// parse the M3U playlist

			StreamReader sr = File.OpenText(file.FullName);
			while (sr.Peek() > -1)
			{
				string itemPath = sr.ReadLine();

				if (Directory.Exists(itemPath) == false)
				{
					if (File.Exists(itemPath) == true)
					{
						FileInfo fi = new FileInfo(itemPath);
						IDvMedia playlistItem = this.CreateObjFromFile(fi, childPlaylists);

						playlistItem.WriteStatus = EnumWriteStatus.NOT_WRITABLE;
						playlistItem.IsRestricted = true;

						if (playlistItem != null)
						{
							if (playlistItem.IsItem)
							{
								// This is a child item.
								DvMediaItem childItem = (DvMediaItem) playlistItem;
								newPC.AddObject(childItem, true);
							}
							else
							{
								// This is a child playlist, but we want to
								// flatten a hierarchy of playlists, so save
								// the references to the children,
								// remove them from the original container,
								// and add them to the current container.

								DvMediaContainer childPlaylist = (DvMediaContainer) playlistItem;
								IList children = childPlaylist.CompleteList;
								childPlaylist.RemoveObjects(children);
								newPC.AddObjects(children, true);
							}
						}
					}
				}
			}
			sr.Close();

			//TODO: add playlist resource
			// add a resource that will represent the playlist as an m3u
			DvMediaResource newRes = this.BuildM3uResource(file, protInfo);
			newPC.AddResource(newRes);

			return newPC;
		}

		private DvMediaResource BuildM3uResource (FileInfo file, string protInfo)
		{
			ResourceBuilder.AllResourceAttributes resInfo = new ResourceBuilder.AllResourceAttributes();
			resInfo.contentUri = MediaResource.AUTOMAPFILE + file.FullName + "?format=m3u";
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			DvMediaResource newRes = DvResourceBuilder.CreateResource(resInfo, true);
			newRes.AllowImport = false;
			newRes.MakeStreamAtHttpGetTime = true;
			newRes.Tag = file;

			return newRes;
		}

		private DvMediaItem CreateItemFromGenericFile(FileInfo file) 
		{
			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string title = Path.GetFileNameWithoutExtension(file.Name);
			string creator = file.Directory.Name;

			MediaBuilder.item info = new MediaBuilder.item(title);
			info.creator = creator;
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			ResourceBuilder.VideoItem resInfo = new ResourceBuilder.VideoItem();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			return newMedia;
		}

		private DvMediaItem CreateItemFromGenericAudioFile(FileInfo file) 
		{
			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string title = Path.GetFileNameWithoutExtension(file.Name);
			string creator = file.Directory.Name;

			MediaBuilder.audioItem info = new MediaBuilder.audioItem(title);
			info.creator = creator;
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			//DvMediaResource res = DvResourceBuilder.CreateResource_HttpGet(file,false);
			ResourceBuilder.VideoItem resInfo = new ResourceBuilder.VideoItem();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;

			newMedia.AddResource(res);

			return newMedia;
		}

		private DvMediaItem CreateItemFromGenericVideoFile(FileInfo file) 
		{
			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string title = Path.GetFileNameWithoutExtension(file.Name);
			string creator = file.Directory.Name;
			
			string genre = null;
			string album = null;
			long duration = 0;
			int bitrate = 0;
			long fileSize = file.Length;

			/*
			if ((file.Extension.ToUpper() == ".WMV") || (file.Extension.ToUpper() == ".ASF"))
			{
				MetadataParser.CMediaMetadataClass mp = new MetadataParser.CMediaMetadataClass();
				mp.ParseMetadata_WindowsMediaPlayerFriendly(file.FullName, out title, out creator, out album, out genre, out duration, out bitrate, out fileSize);
			}
			*/

			if ((title == null) || (title == ""))
			{
				title = Path.GetFileNameWithoutExtension(file.Name);
				creator = file.Directory.Name;
			}

			MediaBuilder.videoItem info = new MediaBuilder.videoItem(title);
			info.creator = creator;
			if (genre != null)
			{
				info.genre = new string[1];
				info.genre[0] = genre;
			}
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			//DvMediaResource res = DvResourceBuilder.CreateResource_HttpGet(file,false);
			ResourceBuilder.AllResourceAttributes resInfo = new ResourceBuilder.AllResourceAttributes();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)fileSize);
			if (bitrate > 0)
			{
				resInfo.bitrate = new _UInt((uint)bitrate/8);
			}
			if (duration > 0)
			{
				resInfo.duration = new _TimeSpan(new TimeSpan(duration));
			}
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;

			newMedia.AddResource(res);

			return newMedia;
		}

		// For files with filenames that have the format: "creator - title"
		private DvMediaItem CreateItemFromFormatedNameFile(FileInfo file) 
		{
			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string ct = Path.GetFileNameWithoutExtension(file.Name);

			DText DT = new DText();
			DT.ATTRMARK = "-";
			string title;
			string creator;

			DT[0] = ct;
			if (DT.DCOUNT() == 1)
			{
				creator = "";
				title = DT[1].Trim();
			}
			else
			{
				creator = DT[1].Trim();
				title = DT[2].Trim();
			}

			MediaBuilder.item info = new MediaBuilder.item(title);
			info.creator = creator;
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			//DvMediaResource res = DvResourceBuilder.CreateResource_HttpGet(file,false);
			ResourceBuilder.VideoItem resInfo = new ResourceBuilder.VideoItem();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			return newMedia;
		}

		// For files with filenames that have the format: "creator - title"
		private DvMediaItem CreateAudioItemFromFormatedNameFile(FileInfo file) 
		{
			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string ct = Path.GetFileNameWithoutExtension(file.Name);

			DText DT = new DText();
			DT.ATTRMARK = "-";
			DT[0] = ct;

			string title;
			string creator;
			if (DT.DCOUNT() == 1)
			{
				creator = "";
				title = DT[1].Trim();
			}
			else
			{
				creator = DT[1].Trim();
				title = DT[2].Trim();
			}

			MediaBuilder.audioItem info = new MediaBuilder.audioItem(title);
			info.creator = creator;
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			//DvMediaResource res = DvResourceBuilder.CreateResource_HttpGet(file,false);
			ResourceBuilder.VideoItem resInfo = new ResourceBuilder.VideoItem();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			return newMedia;
		}

		//MetadataParser.CMediaMetadataClass TheMetadataParser = null;
		private DvMediaItem CreateItemFromMp3WmaFile(FileInfo file)
		{
            return null;
            /*
			if (file.Exists == false) return null;

			string title = null, creator = null, album = null, genre = null;
			int bitrate = -1;
			long duration = -1;
			long fileSize = -1;
			
			try
			{
				if (TheMetadataParser==null)
				{
					TheMetadataParser = new MetadataParser.CMediaMetadataClass();
				}
				TheMetadataParser.ParseMetadata_WindowsMediaPlayerFriendly(file.FullName, out title, out creator, out album, out genre, out duration, out bitrate, out fileSize);
			}
			catch (Exception e)
			{
				title = creator = album = genre = null;
				fileSize = duration = bitrate = -1;
			}

			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			if ((title==null)||(title.Length == 0)) title = Path.GetFileNameWithoutExtension(file.Name);
			if (creator==null) creator = "-Unknown-";

			MediaBuilder.musicTrack info = new MediaBuilder.musicTrack(title);
			info.creator = creator;
			if ((album != null) && (album.Length > 0))
			{
				info.album = new string[1];
				info.album[0] = album;
			}
			if ((genre != null) && (genre.Length > 0))
			{
				info.genre = new string[1];
				info.genre[0] = genre;
			}

			info.date = new _DateTime(file.CreationTime);
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			ResourceBuilder.MusicTrack resInfo = new ResourceBuilder.MusicTrack();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);

			if (fileSize >= 0)
			{
				resInfo.size = new _ULong((ulong)fileSize);
			}

			if (bitrate >= 0)
			{
				resInfo.bitrate = new _UInt((uint)bitrate/8);
			}

			if (duration >= 0)
			{
				resInfo.duration = new _TimeSpan(new TimeSpan(duration));
			}

			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			return newMedia;
			*/
			/*
			Stream fileData;
			byte[] buffer = new byte[128];
			char[] CharBuffer = new char[128];
			string StrBuffer = "";

			fileData = new FileStream(file.FullName,FileMode.Open,FileAccess.Read,FileShare.ReadWrite);
			if (fileData.Length <= 128) 
			{
				fileData.Close();
				return null;
			}

			fileData.Seek(-128,SeekOrigin.End);
			fileData.Read(buffer,0,128);
			fileData.Close();

			for (int id=0;id<128;++id)
			{
				CharBuffer[id] = Convert.ToChar(buffer[id]);
				if (CharBuffer[id]==0) {CharBuffer[id] = ' ';}
			}
			StrBuffer = new String(CharBuffer,0,128);

			if(StrBuffer.Substring(0,3) != "TAG") return null;

			// This has a valid ID3 Tag
			string SongTitle = StrBuffer.Substring(3,30).Trim();
			string AlbumName = StrBuffer.Substring(63,30).Trim();
			string ArtistName = StrBuffer.Substring(33,30).Trim();

			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			if (SongTitle.Length == 0) SongTitle = Path.GetFileNameWithoutExtension(file.Name);
			if (ArtistName.Length == 0) ArtistName = file.Directory.Name;

			MediaBuilder.musicTrack info = new MediaBuilder.musicTrack(SongTitle);
			info.creator = ArtistName;
			//info.artist = new String[1];
			
			//indicate band members
			//info.artist[0] = new PersonWithRole(ArtistName

			//TODO - proper format of date
			//info.date = file.CreationTime.ToLongDateString();

			info.date = new _DateTime(file.CreationTime);
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			ResourceBuilder.MusicTrack resInfo = new ResourceBuilder.MusicTrack();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);

			//resInfo.bitrate = new _UInt(0);
			//resInfo.duration = new _TimeSpan(new TimeSpan(1,2,3,4,0));
			// TODO - Add more data...

			//DvMediaResource res = ResourceBuilder.CreateDvMusicTrack(DvMediaResource.AUTOMAPFILE + file.FullName,protInfo,false,resInfo);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			return newMedia;
			*/
		}

		private DvMediaItem CreateItemFromImageFile(FileInfo file)
		{
			if (file.Exists == false) return null;

			Image image = Image.FromFile(file.FullName);
			if (image == null) return null;

			string mime, mediaClass;
			MimeTypes.ExtensionToMimeType(file.Extension, out mime, out mediaClass);

			string protInfo = new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", mime).ToString();
			string PictureTitle = PictureTitle = Path.GetFileNameWithoutExtension(file.Name);
			string Creator = file.Directory.Name;

			MediaBuilder.photo info = new MediaBuilder.photo(PictureTitle);
			info.creator = Creator;
			//info.date = file.CreationTime.ToLongDateString();
			info.date = new _DateTime(file.CreationTime);
			DvMediaItem newMedia = DvMediaBuilder.CreateItem(info);

			ResourceBuilder.ImageItem resInfo = new ResourceBuilder.ImageItem();
			resInfo.contentUri = DvMediaResource.AUTOMAPFILE + file.FullName;
			resInfo.protocolInfo = new ProtocolInfoString(protInfo);
			resInfo.size = new _ULong((ulong)file.Length);
			resInfo.colorDepth = new _UInt(GetColorDepth(image.PixelFormat));
			resInfo.resolution = new ImageDimensions(image.Width, image.Height);//image.Width + "x" + image.Height;

			//DvMediaResource res = ResourceBuilder.CreateDvImageItem(DvMediaResource.AUTOMAPFILE + file.FullName,protInfo,false,resInfo);
			DvMediaResource res = DvResourceBuilder.CreateResource(resInfo, true);
			res.Tag = file;
			newMedia.AddResource(res);

			// do a jpeg transcoding resource if appropriate
			string mime2, mclass2;
			MimeTypes.ExtensionToMimeType(".jpg", out mime2, out mclass2);
			if (mime !=  mime2)
			{
				string format = "image/jpeg";

				StringBuilder jpgResUri = new StringBuilder(200);
				jpgResUri.AppendFormat("{0}?{3}={1},{2}", resInfo.contentUri, image.Width, image.Height, format);
				resInfo.contentUri = jpgResUri.ToString();
				resInfo.protocolInfo = new ProtocolInfoString(new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", format).ToString());
				DvMediaResource jpgRes = DvResourceBuilder.CreateResource(resInfo, true);
				jpgRes.MakeStreamAtHttpGetTime = true;
				jpgRes.OverrideFileExtenstion = ".jpg";
				newMedia.AddResource(jpgRes);
			}

			// do a jpeg thumbnail resource

			float factor = GetZoomFactor(80, image);

			if (factor > 0.01)
			{
				int thumResX = (int) (image.Width * factor);
				int thumResY = (int) (image.Height * factor);

				string format = "image/jpeg";

				StringBuilder thumResUri = new StringBuilder(200);
				thumResUri.AppendFormat("{0}?{3}={1},{2}", resInfo.contentUri, thumResX, thumResY, format);
				resInfo.contentUri = thumResUri.ToString();
				resInfo.protocolInfo = new ProtocolInfoString(new System.Text.StringBuilder().AppendFormat("http-get:*:{0}:*", format).ToString());
				resInfo.resolution = new ImageDimensions(thumResX, thumResY);
				DvMediaResource thumbnail = DvResourceBuilder.CreateResource(resInfo, true);
				thumbnail.MakeStreamAtHttpGetTime = true;
				thumbnail.OverrideFileExtenstion = ".jpg";
				newMedia.AddResource(thumbnail);
			}

			return newMedia;
		}

		private DvMediaItem CreateItemFromCdsLink(FileInfo file)
		{
			if (file.Exists == false) return null;

			StreamReader fileData = File.OpenText(file.FullName);
			string filexml = fileData.ReadToEnd();
			fileData.Close();

			ArrayList items = MediaBuilder.BuildMediaBranches(filexml,typeof(DvMediaItem),typeof(DvMediaContainer));
			if (items.Count != 1) return null;
			if (items[0].GetType() != typeof(DvMediaItem)) return null;
			return (DvMediaItem)items[0];
		}

		private float GetZoomFactor(int maxResXorY, Image image)
		{
			float factorX = (float) 80 / (float) image.Width;
			float factorY = (float) 80 / (float) image.Height;
			float factor = Math.Min(factorX, factorY);
			return factor;
		}

		private uint GetColorDepth(PixelFormat pixelFormat) 
		{
			switch (pixelFormat) 
			{
				case PixelFormat.Format16bppArgb1555:
				case PixelFormat.Format16bppGrayScale:
				case PixelFormat.Format16bppRgb555:
				case PixelFormat.Format16bppRgb565:
					 return 16;
				case PixelFormat.Format1bppIndexed:
					 return 1;
				case PixelFormat.Format24bppRgb:
					 return 24;
				case PixelFormat.Format32bppArgb:
				case PixelFormat.Format32bppPArgb:
				case PixelFormat.Format32bppRgb:
					 return 32;
				case PixelFormat.Format48bppRgb:
					 return 48;
				case PixelFormat.Format4bppIndexed:
					 return 12;
				case PixelFormat.Format64bppArgb:
				case PixelFormat.Format64bppPArgb:
					 return 64;
				case PixelFormat.Format8bppIndexed:
					return 8;
				default:
					return 0;
			}
		}

		private void Sink_DeviceWatcherSniff(byte[] raw, int offset, int length)
		{
			string str = UTF8.GetString(raw, offset, length);

			if (this.OnSocketData != null)
			{
				this.OnSocketData(this, str);
			}
		}
	
		System.Text.UTF8Encoding UTF8 = new UTF8Encoding();

		/// <summary>
		/// Locks the content hierarchy at the root container.
		/// </summary>
		private Mutex m_LockRoot = new Mutex();

		/// <summary>
		/// Helper function for serializing/deserializing content hierarchies.
		/// Assumes that the root is locked.
		/// </summary>
		private void ClearContentHierarchy()
		{
			// remove all children at root
			IList children = this.rootContainer.CompleteList;
			this.rootContainer.RemoveObjects(children);
			this.totalDirectoryCount = 0;
			this.totalFileCount = 0;

			// reset unique id
			MediaBuilder.SetNextID(0);
		}

		/// <summary>
		/// This recursively method serializes the content hierarchy.
		/// </summary>
		/// <param name="fstream"></param>
		/// <param name="container"></param>
		private void SerializeContainer(BinaryFormatter formatter, FileStream fstream, DvMediaContainer container, Hashtable refItems)
		{
			IList children = container.CompleteList;

			// Indicate that a container is being serialized, then
			// serialize the metadata for this container, 
			// and serialize the number of children,
			// then recursively serialize 
			formatter.Serialize(fstream, container);
			formatter.Serialize(fstream, children.Count);
			foreach (IUPnPMedia child in children)
			{
				DvMediaContainer dvc = child as DvMediaContainer;
				DvMediaItem dvi = child as DvMediaItem;

				if (dvc != null)
				{
					this.SerializeContainer(formatter, fstream, dvc, refItems);
				}
				else if (dvi != null)
				{
					// This is an item, so serialize the media item.
					// If the item is a refItem, provide a mapping
					// from item ID to refID.
					formatter.Serialize(fstream, dvi);
					
					if (dvi.IsReference)
					{
						if ((dvi.RefID != null) && (dvi.RefID != ""))
						{
							refItems[dvi.ID] = dvi.RefID;
						}
					}
				}
				else
				{
					throw new ApplicationException("The MediaServer has a IUPnPMedia item with ID=\"" + child.ID +"\" that is neither a DvMediaItem nor a DvMediaContainer.");
				}
			}
		}

		/// <summary>
		/// This method serializes the entire content hierarchy into a single file.
		/// </summary>
		public void SerializeTree(BinaryFormatter formatter, FileStream fstream)
		{
			this.m_LockRoot.WaitOne();
			Exception error = null;
			try
			{
				// Recursively serialize the content hierarchy,
				// then serialize the hashtable mapping of reference items, 
				// then serialize the current id counter,
				Hashtable refItems = new Hashtable();
				this.SerializeContainer(formatter, fstream, this.mediaServer.Root, refItems);
				formatter.Serialize(fstream, refItems);

				string lastID = MediaBuilder.GetMostRecentUniqueId();
				formatter.Serialize(fstream, lastID);
			}
			catch (Exception e)
			{
				error = e;
			}

			if (error != null)
			{
				throw new Exception("SerializeTree() error", error);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="container"></param>
		/// <param name="formatter"></param>
		/// <param name="fstream"></param>
		private void DeserializeContainer(DvMediaContainer container, BinaryFormatter formatter, FileStream fstream)
		{
			// get number of children in container
			int count = (int) formatter.Deserialize (fstream);

			ArrayList children = new ArrayList(count);
			object obj;
			DvMediaItem dvi;
			DvMediaContainer dvc;
			for (int i=0; i<count; i++)
			{
				try
				{
					obj = formatter.Deserialize(fstream);
				}
				catch (Exception e)
				{
					throw new SerializationException("Error deserializing a child of containerID=\"" +container.ID+ "\" Title=\"" +container.Title+ "\".", e);
				}

				dvi = obj as DvMediaItem;
				dvc = obj as DvMediaContainer;

				if (dvc != null)
				{
					this.DeserializeContainer(dvc, formatter, fstream);
					children.Add(dvc);
				}
				else if (dvi != null)
				{
					children.Add(dvi);
				}
				else
				{
					throw new ApplicationException("The MediaServer deserialized an object that is neither a DvMediaItem nor a DvMediaContainer.");
				}

			}
			container.AddObjects(children, true);
		}

		/// <summary>
		/// Deserializes a content hierarchy (including the creation of reference items),
		/// and then resolves differences with the file system afterwards.
		/// </summary>
		/// <param name="fileName"></param>
		public void DeserializeTree(BinaryFormatter formatter, FileStream fstream)
		{
			this.m_LockRoot.WaitOne();
			Exception error = null;
			try
			{
				// Clear current content tree
				this.ClearContentHierarchy();

				// recursively deserialize the container hierarchy
				// then deserizlize last ID string
				DvMediaContainer root = (DvMediaContainer) formatter.Deserialize(fstream);
				this.mediaServer.Root.UpdateObject(root);
				this.DeserializeContainer(this.mediaServer.Root, formatter, fstream);

				// deserialize hashtable of refitems, and attach reference items
				// to their underlying items
				Hashtable mapping = (Hashtable) formatter.Deserialize(fstream);
				Hashtable cache = new Hashtable();
				foreach (string referringId in mapping.Keys)
				{
					string underlyingId = (string) mapping[referringId];

					DvMediaItem underlying = this.mediaServer.Root.GetDescendent(underlyingId, cache) as DvMediaItem;
					DvMediaItem referring = this.mediaServer.Root.GetDescendent(referringId, cache) as DvMediaItem;

					if ((underlying != null) && (referring != null))
					{
						DvMediaItem.AttachRefItem(underlying, referring);
					}
					else
					{
						throw new NullReferenceException("At least one DvMediaItem is null.");
					}
				}

				string lastID = (string) formatter.Deserialize(fstream);
				MediaBuilder.PrimeNextId(lastID);

				// The tree has been deserialized... now remove the containers/items from the
				// tree that are no longer on disk and also add containers/items
				// that are new since the original time of serialization

				this.AdjustContainer(this.mediaServer.Root);
			}
			catch (Exception e)
			{
				this.ClearContentHierarchy();
				error = e;
			}
			this.m_LockRoot.ReleaseMutex();

			if (error != null)
			{
				throw new Exception("DeserializeTree() error", error);
			}
		}

		private void AdjustContainer(DvMediaContainer c)
		{
			InnerMediaDirectory imd = c.Tag as InnerMediaDirectory;
			bool dirExists = false;
			
			// Remove non-root containers if the associated directory no longer exists.
			if (c.IsRootContainer == false)
			{
				if (imd != null)
				{
					imd.directory = new DirectoryInfo(imd.directoryname);
					if (imd.directory.Exists == false)
					{
						c.Parent.RemoveObject(c);
					}
					else
					{
						dirExists = true;
					}
				}
			}

			// c.CompleteList returns a shallow copy
			foreach (IDvMedia dv in c.CompleteList)
			{
				if (dv.IsContainer)
				{
					this.AdjustContainer((DvMediaContainer) dv);
				}
				else if (dv.IsReference)
				{
					// ignore references
				}
				else
				{
					// this is an item; dv.Resources returns a shallow copy
					foreach (IDvResource res in dv.Resources)
					{
						if (res.ContentUri.StartsWith(MediaResource.AUTOMAPFILE))
						{
							string localPath = res.ContentUri.Remove(0, MediaResource.AUTOMAPFILE.Length);
							int quesMark = localPath.LastIndexOf('?');
							if (quesMark >= 0)
							{
								localPath = localPath.Substring(0, localPath.LastIndexOf('?'));
							}

							if (File.Exists(localPath) == false)
							{
								dv.RemoveResource(res);
							}
						}
					}

					// if the item has no resources, remove it
					if (dv.Resources.Length == 0)
					{
						dv.Parent.RemoveObject(dv);
					}
				}
			}


			if (dirExists)
			{
				this.totalDirectoryCount++;

				// Scan the directory for containers and files that don't exist
				// in the current tree
				DirectoryInfo[] subdirs = imd.directory.GetDirectories();
				FileInfo[] files = imd.directory.GetFiles();
				IList containers = c.Containers;
				IList items = c.Items;

				foreach (DirectoryInfo subdir in subdirs)
				{
					bool found = false;
					foreach (IDvContainer dvc in containers)
					{
						InnerMediaDirectory imd2 = dvc.Tag as InnerMediaDirectory;

						if (imd2 != null)
						{
							// this container has a mapping to a local directory,
							// so determine if the container matches the 
							// target name
							if (string.Compare(subdir.FullName, imd2.directoryname, true) == 0)
							{
								found = true;
								break;
							}
						}
					}

					if (found == false)
					{
						// the subdirectory wasn't found as a container in the tree,
						// so go ahead and add it
						this.AddDirectoryEx(c, subdir);
					}
				}

				foreach (FileInfo file in files)
				{
					bool found = false;
					foreach (IDvItem dvi in items)
					{
						if (dvi.IsReference == false)
						{
							bool foundRes = false;
							foreach (IDvResource res in dvi.Resources)
							{
								string localPath = res.ContentUri.Remove(0, MediaResource.AUTOMAPFILE.Length);
								int quesMark = localPath.LastIndexOf('?');
								if (quesMark >= 0)
								{
									localPath = localPath.Substring(0, localPath.LastIndexOf('?'));
								}
							
								if (string.Compare(localPath, file.FullName, true) == 0)
								{
									foundRes = true;
								}
								break;
							}

							if (foundRes)
							{
								found = true;
								break;
							}
						}					
					}

					foreach (IDvContainer dvc in containers)
					{
						if (dvc.Class.IsA(MediaBuilder.StandardMediaClasses.PlaylistContainer))
						{
							FileInfo plfi = dvc.Tag as FileInfo;
							if (plfi != null)
							{
								if (string.Compare(plfi.FullName, file.FullName, true) == 0)
								{
									found = true;
								}
							}
						}
					}

					if (found == false)
					{
						// the file wasn't found as a resource in the 
						// content hierarchy, so go ahead and add it
						IDvMedia newItem = this.CreateObjFromFile(file, new ArrayList());
						c.AddObject(newItem, true);
					}

					// we either already had a media item for the file, or we just added one
					this.totalFileCount++;
				}


				// Also initialize inner media directory objects with a file
				// system watcher if the directory still exists.
				// We do this at the end because we don't want to subscribe
				// to changes until after we've added files.
				imd.watcher = new FileSystemWatcher(imd.directory.FullName);
				imd.watcher.Changed += new FileSystemEventHandler(OnDirectoryChangedSink);
				imd.watcher.Created += new FileSystemEventHandler(OnDirectoryCreatedSink);
				imd.watcher.Deleted += new FileSystemEventHandler(OnDirectoryDeletedSink);
				imd.watcher.Renamed += new RenamedEventHandler(OnFileSystemRenameSink);
			}
		}
	}

	/*
	public class IUPnPMediaServerEvents
	{
		void MediaServerStatsChanged() {}
		void MediaHttpTransferssChanged() {}
		void MediaServerDebugMessage(string message) {}
	}
	*/

	public class UPnPMediaServer : MarshalByRefObject
	{
		public delegate void NotifyEvent();
		public delegate void DebugNotifyEvent(string message);

		public event NotifyEvent MediaServerStatsChanged;
		public event NotifyEvent MediaHttpTransfersChanged;
		public event DebugNotifyEvent MediaServerDebugMessage;

		public int mediaServerStatsUpdateId = 1;
		public int mediaHttpTransfersUpdateId = 1;
		public int mediaSharedDirectoryUpdateId = 1;

		public void DeserializeTree(BinaryFormatter formatter, FileStream fstream)
		{
			if (MediaServerCore.serverCore == null) return;

			MediaServerCore.serverCore.DeserializeTree(formatter, fstream);
			this.mediaSharedDirectoryUpdateId++;
		}
		public void SerializeTree(BinaryFormatter formatter, FileStream fstream)
		{
			if (MediaServerCore.serverCore == null) return;
			MediaServerCore.serverCore.SerializeTree(formatter, fstream);
		}

		public void ResetTree()
		{
			if (MediaServerCore.serverCore == null) return;
			MediaServerCore.serverCore.ResetCoreRoot();
		}

		public UPnPMediaServer()
		{
			if (MediaServerCore.serverCore == null) return;
			MediaServerCore.serverCore.OnStatsChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaServerStatsChangedSink);
			MediaServerCore.serverCore.OnHttpTransfersChanged += new MediaServerCore.MediaServerCoreEventHandler(MediaHttpTransferssChangedSink);
			MediaServerCore.serverCore.OnDebugMessage += new MediaServerCore.MediaServerCoreDebugHandler(MediaServerCoreDebugSink);
		}

		~UPnPMediaServer()
		{
			if (MediaServerCore.serverCore == null) return;
			MediaServerCore.serverCore.OnStatsChanged -= new MediaServerCore.MediaServerCoreEventHandler(MediaServerStatsChangedSink);
			MediaServerCore.serverCore.OnHttpTransfersChanged -= new MediaServerCore.MediaServerCoreEventHandler(MediaHttpTransferssChangedSink);
			MediaServerCore.serverCore.OnDebugMessage -= new MediaServerCore.MediaServerCoreDebugHandler(MediaServerCoreDebugSink);
		}

		public Exception AddDirectory(DirectoryInfo directory, bool restricted, bool allowWrite)
		{
			try 
			{
				if (MediaServerCore.serverCore == null) return new NullReferenceException("No MediaServer object exists for the application.");
				bool r = MediaServerCore.serverCore.AddDirectory(directory);
				mediaSharedDirectoryUpdateId++;
				if (mediaSharedDirectoryUpdateId < 0) mediaSharedDirectoryUpdateId = 1;
				return null;
			}
			catch (Exception ex)
			{
				OpenSource.Utilities.EventLogger.Log(ex);
				return ex;
			}
		}

		public bool RemoveDirectory(DirectoryInfo directory) 
		{
			try
			{
				if (MediaServerCore.serverCore == null) return false;
				bool r = MediaServerCore.serverCore.RemoveDirectory(directory);
				mediaSharedDirectoryUpdateId++;
				if (mediaSharedDirectoryUpdateId < 0) mediaSharedDirectoryUpdateId = 1;
				return r;
			}
			catch (Exception ex)
			{
				OpenSource.Utilities.EventLogger.Log(ex);
				return false;
			}
		}

		public bool UpdatePermissions(DirectoryInfo directory, bool restricted, bool allowWrite) 
		{
			try
			{
				if (MediaServerCore.serverCore == null) return false;
				bool r = MediaServerCore.serverCore.UpdatePermissions(directory, restricted, allowWrite);
				mediaSharedDirectoryUpdateId++;
				if (mediaSharedDirectoryUpdateId < 0) mediaSharedDirectoryUpdateId = 1;
				return r;
			}
			catch (Exception ex)
			{
				OpenSource.Utilities.EventLogger.Log(ex);
				return false;
			}
		}

		public int MediaServerStatsUpdateId 
		{
			get {return mediaServerStatsUpdateId;}
		}

		public int MediaHttpTransfersUpdateId 
		{
			get {return mediaHttpTransfersUpdateId;}
		}

		public int MediaSharedDirectoryUpdateId 
		{
			get {return mediaSharedDirectoryUpdateId;}
		}

		public int TotalDirectoryCount
		{
			get {
				if (MediaServerCore.serverCore == null) return 0;
				return MediaServerCore.serverCore.TotalDirectoryCount;
			}
		}

		public int TotalFileCount
		{
			get {
				if (MediaServerCore.serverCore == null) return 0;
				return MediaServerCore.serverCore.TotalFileCount;
			}
		}

		public IList HttpTransfers
		{
			get {
				try 
				{
					if (MediaServerCore.serverCore == null)
					{
						return null;
					}

					ArrayList result = new ArrayList();
					foreach (MediaServerDevice.HttpTransfer transfer in MediaServerCore.serverCore.HttpTransfers) 
					{
						MediaServerCore.TransferStruct t = new MediaServerCore.TransferStruct();
						t.Incoming = transfer.Incoming;
						t.Source = transfer.Source;
						t.Destination = transfer.Destination;
						t.ResourceName = transfer.Resource.ContentUri;
						t.ResourceLength = transfer.TransferSize;
						t.ResourcePosition = transfer.Position;
						result.Add(t);
					}
					return result;
				}
				catch (Exception ex) 
				{
					OpenSource.Utilities.EventLogger.Log(ex);
					return null;
				}
			}
		}

		public string[] GetSharedDirectoryNames()
		{
			try 
			{
				if (MediaServerCore.serverCore == null) return null;
				IList dirs = MediaServerCore.serverCore.Directories;
				string[] result = new string[dirs.Count];
				int i =0;
				foreach (MediaServerCore.SharedDirectoryInfo sdi in dirs)
				{
					result[i] = (string)sdi.directory.Clone();
					i++;
				}
				return result;
			}
			catch (Exception ex)
			{
				OpenSource.Utilities.EventLogger.Log(ex);
				return null;
			}
		}

		public IList GetSharedDirectories()
		{
			try 
			{
				if (MediaServerCore.serverCore == null) return null;
				return MediaServerCore.serverCore.Directories;
			}
			catch (Exception ex) 
			{
				OpenSource.Utilities.EventLogger.Log(ex);
				return null;
			}
		}

		private void MediaServerStatsChangedSink(MediaServerCore sender)
		{
			mediaServerStatsUpdateId++;
			if (mediaServerStatsUpdateId < 0) mediaServerStatsUpdateId = 1;
			if (MediaServerStatsChanged != null) MediaServerStatsChanged();
		}

		private void MediaHttpTransferssChangedSink(MediaServerCore sender)
		{
			mediaHttpTransfersUpdateId++;
			if (mediaHttpTransfersUpdateId < 0) mediaHttpTransfersUpdateId = 1;
			if (MediaHttpTransfersChanged != null) MediaHttpTransfersChanged();
		}

		private void MediaServerCoreDebugSink(MediaServerCore sender, string msg) 
		{
			if (MediaServerDebugMessage != null) MediaServerDebugMessage(msg);
		}
	}

}
