using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Net;
using System.Diagnostics;
using System.ServiceProcess;
using System.Reflection;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.Runtime.Remoting.Channels.Http;
using UPnPMediaServerCore;

namespace UPnPMediaServerService
{
	public class MediaServerService : System.ServiceProcess.ServiceBase
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		public MediaServerCore mediaServer;
		public System.Runtime.Remoting.Channels.IChannel channel;

		public MediaServerService()
		{
			// This call is required by the Windows.Forms Component Designer.
			InitializeComponent();
		}

		// The main entry point for the process
		static void Main()
		{
			System.ServiceProcess.ServiceBase[] ServicesToRun;
	
			// More than one user Service may run within the same process. To add
			// another service to this process, change the following line to
			// create a second service object. For example,
			//
			//   ServicesToRun = New System.ServiceProcess.ServiceBase[] {new Service1(), new MySecondUserService()};
			//
			ServicesToRun = new System.ServiceProcess.ServiceBase[] { new MediaServerService() };

			System.ServiceProcess.ServiceBase.Run(ServicesToRun);
		}

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			// 
			// MediaServerService
			// 
			this.ServiceName = "Intel\'s UPnP Media Server";
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		/// <summary>
		/// Set things in motion so your service can do its work.
		/// </summary>
		protected override void OnStart(string[] args)
		{
			if (mediaServer != null) this.OnStop();
			mediaServer = new MediaServerCore("Intel's Media Server (" + System.Windows.Forms.SystemInformation.ComputerName + ")");

			System.Collections.Specialized.ListDictionary channelProperties = new System.Collections.Specialized.ListDictionary();
			channelProperties.Add("port", 12329);

			HttpChannel channel = new HttpChannel(channelProperties,
				new SoapClientFormatterSinkProvider(),
				new SoapServerFormatterSinkProvider());

			//channel = new TcpChannel(12329);
			ChannelServices.RegisterChannel(channel);

			RemotingConfiguration.ApplicationName = "IntelUPnPMediaServer";
			RemotingConfiguration.RegisterWellKnownServiceType(typeof(UPnPMediaServer), 
				"UPnPMediaServer.soap",
				WellKnownObjectMode.Singleton);
		}
 
		/// <summary>
		/// Stop this service.
		/// </summary>
		protected override void OnStop()
		{
			ChannelServices.UnregisterChannel(channel);
			channel = null;
			mediaServer.Dispose();
			mediaServer = null;
		}

	}

}
