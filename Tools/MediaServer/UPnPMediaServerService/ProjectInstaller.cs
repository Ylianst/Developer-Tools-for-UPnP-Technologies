using System;
using System.Collections;
using System.ComponentModel;
using System.Configuration.Install;

namespace UPnPMediaServerService
{
	/// <summary>
	/// Summary description for ProjectInstaller.
	/// </summary>
	[RunInstaller(true)]
	public class ProjectInstaller : System.Configuration.Install.Installer
	{
		private System.ServiceProcess.ServiceProcessInstaller serviceProcessInstaller;
		private System.ServiceProcess.ServiceInstaller serviceInstaller;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ProjectInstaller()
		{
			// This call is required by the Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitComponent call
		}

		// Add the service description
		public override void Install(IDictionary mySavedState)
		{
			base.Install(mySavedState);    // This must be called first
			Microsoft.Win32.RegistryKey ServDescription = null;
			try 
			{
				ServDescription = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"System\\CurrentControlSet\\Services\\Intel's UPnP Media Server",true);
				ServDescription.SetValue("Description", "Share files as network discoverable media");
				ServDescription.Close();
			}
			catch(Exception ex) 
			{
				Console.WriteLine(ex);
			}
		}

		#region Component Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.serviceProcessInstaller = new System.ServiceProcess.ServiceProcessInstaller();
			this.serviceInstaller = new System.ServiceProcess.ServiceInstaller();
			// 
			// serviceProcessInstaller
			// 
			this.serviceProcessInstaller.Account = System.ServiceProcess.ServiceAccount.LocalSystem;
			this.serviceProcessInstaller.Password = null;
			this.serviceProcessInstaller.Username = null;
			// 
			// serviceInstaller
			// 
			this.serviceInstaller.DisplayName = "Intel\'s UPnP Media Server";
			this.serviceInstaller.ServiceName = "Intel\'s UPnP Media Server";
			this.serviceInstaller.StartType = System.ServiceProcess.ServiceStartMode.Automatic;
			// 
			// ProjectInstaller
			// 
			this.Installers.AddRange(new System.Configuration.Install.Installer[] {
																					  this.serviceProcessInstaller,
																					  this.serviceInstaller});

		}
		#endregion
	}
}
