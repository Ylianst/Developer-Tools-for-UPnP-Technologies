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
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
	public delegate UPnPDevice[] GetDevicesHandler();


	/// <summary>
	/// Summary description for CodeGenerationForm.
	/// </summary>
	public class CodeGenerationForm : System.Windows.Forms.Form
	{
		private GetDevicesHandler GetDevices;
		private ServiceGenerator.StackConfiguration StackConfiguration;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TextBox libPrefixTextBox;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.TextBox classNameTextBox;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.ComboBox indentComboBox;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.ComboBox callConventionComboBox;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox newLineComboBox;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox platformComboBox;
		private System.Windows.Forms.Button outputDirectoryButton;
		private System.Windows.Forms.Button generateButton;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TextBox outputPathTextBox;
		private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox genOutputTextBox;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.TextBox licenseTextBox;
        private System.Windows.Forms.TextBox projectNameTextBox;
        private System.Windows.Forms.CheckBox UPnP1dot1Enabled;
		private System.Windows.Forms.TabPage tabPage6;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.CheckBox externCallbackCheckBox;
		private System.Windows.Forms.CheckBox errorEncodingCheckBox;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.RadioButton DoNotGenerateRadioButton;
		private System.Windows.Forms.RadioButton AnyTagRadioButton;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.CheckBox SampleApplication;
		private System.Windows.Forms.CheckBox defaultIPAddressMonitorCheckBox;
		private System.Windows.Forms.CheckBox IncludeThreadPool;
		private System.Windows.Forms.CheckBox InitThreadPool;
		private System.Windows.Forms.TextBox NumThreadPoolThreads;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.CheckBox dynamicObjectModelCheckBox;
		private System.Windows.Forms.CheckBox CPlusPlusWrapperCheckBox;
		private System.Windows.Forms.GroupBox groupBox5;
		private System.Windows.Forms.CheckBox http11CheckBox;
		private System.Windows.Forms.TextBox MaxHTTPHeaderSizeTextBox;
        private System.Windows.Forms.TextBox MaxHTTPPacketSizeTextBox;
		private System.Windows.Forms.Label label12;
		private System.Windows.Forms.TextBox InitialHTTPBufferSizeTextBox;
		private System.Windows.Forms.Label label13;
		private System.Windows.Forms.GroupBox groupBox6;
		private System.Windows.Forms.CheckBox CertToolCheckBox;
        private Label label16;
        private FolderBrowserDialog folderBrowserDialog;
		private System.Windows.Forms.RadioButton CleanSchemaRadioButton;

        private static object LogErrorFileLock = new object();
        private static void LogErrorToFile(string message)
        {
            lock (LogErrorFileLock)
            {
                File.AppendAllText(System.Windows.Forms.Application.StartupPath + "\\Exceptions.txt", message, Encoding.UTF8);
            }
        }

		public CodeGenerationForm(GetDevicesHandler devices, Hashtable stackSettings)
		{
			InitializeComponent();
            this.Settings = stackSettings;
			this.GetDevices = devices;
		}

		public Hashtable Settings 
		{
			get
			{
				return(StackConfiguration.ToHashtable());
			}
			set
			{
				StackConfiguration = ServiceGenerator.StackConfiguration.FromHashTable(value);
				UpdateConfigurationUI();
			}
		}

		public void UpdateConfigurationUI()
		{
			this.IncludeThreadPool.Checked = StackConfiguration.GenerateThreadPoolLibrary;
			this.InitThreadPool.Checked = StackConfiguration.InitThreadPoolInSampleApp;
			if (StackConfiguration.ThreadPoolThreads_InSampleApp==0)
			{
				this.NumThreadPoolThreads.Text = "3";
			}
			else
			{
				this.NumThreadPoolThreads.Text = StackConfiguration.ThreadPoolThreads_InSampleApp.ToString();
			}
			this.UPnP1dot1Enabled.Checked = StackConfiguration.UPNP_1dot1;
			this.outputPathTextBox.Text = StackConfiguration.outputpath;
			this.platformComboBox.SelectedIndex = (int)StackConfiguration.TargetPlatform;
            if (StackConfiguration.projectname != null && StackConfiguration.projectname.Length > 0) this.projectNameTextBox.Text = StackConfiguration.projectname;
			this.newLineComboBox.SelectedIndex = (int)StackConfiguration.newline;
			this.callConventionComboBox.SelectedIndex = (int)StackConfiguration.callconvention;
			this.libPrefixTextBox.Text = StackConfiguration.prefixlib;
			this.indentComboBox.SelectedIndex = (int)StackConfiguration.indent;
			if (StackConfiguration.classname != null && StackConfiguration.classname.Length > 0) this.classNameTextBox.Text = StackConfiguration.classname;
			this.errorEncodingCheckBox.Checked = StackConfiguration.ExplicitErrorEncoding;
			this.externCallbackCheckBox.Checked = StackConfiguration.EXTERN_Callbacks;
			this.defaultIPAddressMonitorCheckBox.Checked = StackConfiguration.DefaultIPAddressMonitor;
			this.http11CheckBox.Checked = StackConfiguration.HTTP_1dot1;
			this.SampleApplication.Checked = !StackConfiguration.SupressSampleProject;
			this.dynamicObjectModelCheckBox.Checked = StackConfiguration.DynamicObjectModel;
			this.CPlusPlusWrapperCheckBox.Checked = StackConfiguration.CPlusPlusWrapper;

			if (StackConfiguration.CPlusPlusWrapper)
			{
				this.externCallbackCheckBox.Checked = false;
				this.externCallbackCheckBox.Enabled = false;
			}

			this.CleanSchemaRadioButton.Checked = false;
			this.DoNotGenerateRadioButton.Checked = false;
			this.AnyTagRadioButton.Checked = false;
			switch(StackConfiguration.SchemaGeneration)
			{
				case ServiceGenerator.XSDSchemaGeneration.NONE:
					this.DoNotGenerateRadioButton.Checked = true;
					break;
				case ServiceGenerator.XSDSchemaGeneration.WITH_ANY_TAGS:
					this.AnyTagRadioButton.Checked = true;
					break;
				case ServiceGenerator.XSDSchemaGeneration.WITHOUT_ANY_TAGS:
					this.CleanSchemaRadioButton.Checked = true;
					break;
			}

			this.MaxHTTPHeaderSizeTextBox.Text = StackConfiguration.MaxHTTPHeaderSize.ToString();
			this.MaxHTTPPacketSizeTextBox.Text = StackConfiguration.MaxHTTPPacketSize.ToString();
			this.InitialHTTPBufferSizeTextBox.Text = StackConfiguration.InitialHTTPBufferSize.ToString();
			this.CertToolCheckBox.Checked = StackConfiguration.GenerateCertToolFiles;
		}


		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if ( disposing )
			{
				if (components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CodeGenerationForm));
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.libPrefixTextBox = new System.Windows.Forms.TextBox();
            this.classNameTextBox = new System.Windows.Forms.TextBox();
            this.indentComboBox = new System.Windows.Forms.ComboBox();
            this.callConventionComboBox = new System.Windows.Forms.ComboBox();
            this.newLineComboBox = new System.Windows.Forms.ComboBox();
            this.platformComboBox = new System.Windows.Forms.ComboBox();
            this.outputPathTextBox = new System.Windows.Forms.TextBox();
            this.projectNameTextBox = new System.Windows.Forms.TextBox();
            this.UPnP1dot1Enabled = new System.Windows.Forms.CheckBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.outputDirectoryButton = new System.Windows.Forms.Button();
            this.generateButton = new System.Windows.Forms.Button();
            this.label10 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.label16 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.InitialHTTPBufferSizeTextBox = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.MaxHTTPPacketSizeTextBox = new System.Windows.Forms.TextBox();
            this.MaxHTTPHeaderSizeTextBox = new System.Windows.Forms.TextBox();
            this.http11CheckBox = new System.Windows.Forms.CheckBox();
            this.CPlusPlusWrapperCheckBox = new System.Windows.Forms.CheckBox();
            this.IncludeThreadPool = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label5 = new System.Windows.Forms.Label();
            this.NumThreadPoolThreads = new System.Windows.Forms.TextBox();
            this.InitThreadPool = new System.Windows.Forms.CheckBox();
            this.defaultIPAddressMonitorCheckBox = new System.Windows.Forms.CheckBox();
            this.SampleApplication = new System.Windows.Forms.CheckBox();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.CertToolCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.DoNotGenerateRadioButton = new System.Windows.Forms.RadioButton();
            this.AnyTagRadioButton = new System.Windows.Forms.RadioButton();
            this.CleanSchemaRadioButton = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.dynamicObjectModelCheckBox = new System.Windows.Forms.CheckBox();
            this.externCallbackCheckBox = new System.Windows.Forms.CheckBox();
            this.errorEncodingCheckBox = new System.Windows.Forms.CheckBox();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.licenseTextBox = new System.Windows.Forms.TextBox();
            this.label14 = new System.Windows.Forms.Label();
            this.genOutputTextBox = new System.Windows.Forms.TextBox();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.SuspendLayout();
            // 
            // libPrefixTextBox
            // 
            this.libPrefixTextBox.Location = new System.Drawing.Point(131, 150);
            this.libPrefixTextBox.Name = "libPrefixTextBox";
            this.libPrefixTextBox.Size = new System.Drawing.Size(355, 20);
            this.libPrefixTextBox.TabIndex = 56;
            this.libPrefixTextBox.Text = "ILib";
            this.toolTip1.SetToolTip(this.libPrefixTextBox, "This string prefixes every generated method of common libraries (Parsers, HTTP, S" +
                    "SDP...)");
            // 
            // classNameTextBox
            // 
            this.classNameTextBox.Enabled = false;
            this.classNameTextBox.Location = new System.Drawing.Point(131, 205);
            this.classNameTextBox.Name = "classNameTextBox";
            this.classNameTextBox.Size = new System.Drawing.Size(355, 20);
            this.classNameTextBox.TabIndex = 54;
            this.classNameTextBox.Text = "OpenSource.DeviceBuilder";
            this.toolTip1.SetToolTip(this.classNameTextBox, "When generating .net code, the namespace used to wrap the generated device stack." +
                    "");
            // 
            // indentComboBox
            // 
            this.indentComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.indentComboBox.Items.AddRange(new object[] {
            "1 Tab",
            "1 Space",
            "2 Spaces",
            "3 Spaces",
            "4 Spaces",
            "5 Spaces",
            "6 Spaces"});
            this.indentComboBox.Location = new System.Drawing.Point(131, 178);
            this.indentComboBox.Name = "indentComboBox";
            this.indentComboBox.Size = new System.Drawing.Size(355, 21);
            this.indentComboBox.TabIndex = 52;
            this.toolTip1.SetToolTip(this.indentComboBox, "Code indentation setting.");
            // 
            // callConventionComboBox
            // 
            this.callConventionComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.callConventionComboBox.Items.AddRange(new object[] {
            "None - Compiler default calling convention",
            "_stdcall - Standard C and C++ calling convention",
            "_fastcall - Register passing calling convention"});
            this.callConventionComboBox.Location = new System.Drawing.Point(131, 122);
            this.callConventionComboBox.Name = "callConventionComboBox";
            this.callConventionComboBox.Size = new System.Drawing.Size(355, 21);
            this.callConventionComboBox.TabIndex = 48;
            this.toolTip1.SetToolTip(this.callConventionComboBox, "This setting can be used to force all generated methods to be implemented with a " +
                    "given calling convention. _fastcall may lead to smaller and faster code.");
            // 
            // newLineComboBox
            // 
            this.newLineComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.newLineComboBox.Items.AddRange(new object[] {
            "CR+LF, Windows style code",
            "LF, UNIX style code"});
            this.newLineComboBox.Location = new System.Drawing.Point(131, 94);
            this.newLineComboBox.Name = "newLineComboBox";
            this.newLineComboBox.Size = new System.Drawing.Size(355, 21);
            this.newLineComboBox.TabIndex = 46;
            this.toolTip1.SetToolTip(this.newLineComboBox, "The type of newline used to generate the code. On UNIX style platforms, using CR+" +
                    "LF may lead to a compilable stack that will not function correctly.");
            // 
            // platformComboBox
            // 
            this.platformComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.platformComboBox.Items.AddRange(new object[] {
            "C Stack - POSIX, Linux, BSD, OSX, Android",
            "C Stack - Windows WinSock1 (Not Supported)",
            "C Stack - Windows WinSock2",
            "C Stack - PocketPC (Not Supported)",
            "C Stack - Symbian OS v9.1 (Not Supported)",
            ".NET Framework Stack (C#)",
            "Java Stack - Android"});
            this.platformComboBox.Location = new System.Drawing.Point(131, 9);
            this.platformComboBox.Name = "platformComboBox";
            this.platformComboBox.Size = new System.Drawing.Size(355, 21);
            this.platformComboBox.TabIndex = 41;
            this.toolTip1.SetToolTip(this.platformComboBox, "Target platform for generated code.");
            this.platformComboBox.SelectedIndexChanged += new System.EventHandler(this.platformComboBox_SelectedIndexChanged);
            // 
            // outputPathTextBox
            // 
            this.outputPathTextBox.Location = new System.Drawing.Point(131, 65);
            this.outputPathTextBox.Name = "outputPathTextBox";
            this.outputPathTextBox.Size = new System.Drawing.Size(322, 20);
            this.outputPathTextBox.TabIndex = 35;
            this.outputPathTextBox.Text = "C:\\Temp";
            this.toolTip1.SetToolTip(this.outputPathTextBox, "The output path of the generated files. Generated files will overwrite existing f" +
                    "iles without prompting.");
            // 
            // projectNameTextBox
            // 
            this.projectNameTextBox.Location = new System.Drawing.Point(131, 37);
            this.projectNameTextBox.Name = "projectNameTextBox";
            this.projectNameTextBox.Size = new System.Drawing.Size(355, 20);
            this.projectNameTextBox.TabIndex = 57;
            this.projectNameTextBox.Text = "SampleApplication";
            this.toolTip1.SetToolTip(this.projectNameTextBox, "This string prefixes every generated action. It can be used like a namespace in C" +
                    ".");
            // 
            // UPnP1dot1Enabled
            // 
            this.UPnP1dot1Enabled.Location = new System.Drawing.Point(9, 20);
            this.UPnP1dot1Enabled.Name = "UPnP1dot1Enabled";
            this.UPnP1dot1Enabled.Size = new System.Drawing.Size(448, 20);
            this.UPnP1dot1Enabled.TabIndex = 32;
            this.UPnP1dot1Enabled.Text = "UPnP/1.1 (Beta) Support (UPnP/1.0 Support if unchecked)";
            this.toolTip1.SetToolTip(this.UPnP1dot1Enabled, "This box will add UPnP/1.1 support to the total code size of the generated stack." +
                    "");
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage6);
            this.tabControl1.Controls.Add(this.tabPage4);
            this.tabControl1.Location = new System.Drawing.Point(9, 9);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(503, 332);
            this.tabControl1.TabIndex = 35;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.projectNameTextBox);
            this.tabPage1.Controls.Add(this.libPrefixTextBox);
            this.tabPage1.Controls.Add(this.label8);
            this.tabPage1.Controls.Add(this.classNameTextBox);
            this.tabPage1.Controls.Add(this.label7);
            this.tabPage1.Controls.Add(this.indentComboBox);
            this.tabPage1.Controls.Add(this.label6);
            this.tabPage1.Controls.Add(this.callConventionComboBox);
            this.tabPage1.Controls.Add(this.label4);
            this.tabPage1.Controls.Add(this.newLineComboBox);
            this.tabPage1.Controls.Add(this.label3);
            this.tabPage1.Controls.Add(this.platformComboBox);
            this.tabPage1.Controls.Add(this.outputDirectoryButton);
            this.tabPage1.Controls.Add(this.generateButton);
            this.tabPage1.Controls.Add(this.label10);
            this.tabPage1.Controls.Add(this.outputPathTextBox);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Controls.Add(this.label2);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(495, 306);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "General";
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(6, 150);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(120, 20);
            this.label8.TabIndex = 55;
            this.label8.Text = "Library Code Prefix";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(6, 206);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(120, 19);
            this.label7.TabIndex = 53;
            this.label7.Text = "Namespace";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(6, 178);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(120, 20);
            this.label6.TabIndex = 51;
            this.label6.Text = "Code Indention";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(6, 122);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(120, 21);
            this.label4.TabIndex = 47;
            this.label4.Text = "Calling Convention";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(6, 94);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(120, 21);
            this.label3.TabIndex = 45;
            this.label3.Text = "New Line Format";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // outputDirectoryButton
            // 
            this.outputDirectoryButton.Image = ((System.Drawing.Image)(resources.GetObject("outputDirectoryButton.Image")));
            this.outputDirectoryButton.Location = new System.Drawing.Point(456, 65);
            this.outputDirectoryButton.Name = "outputDirectoryButton";
            this.outputDirectoryButton.Size = new System.Drawing.Size(29, 25);
            this.outputDirectoryButton.TabIndex = 40;
            this.outputDirectoryButton.Click += new System.EventHandler(this.outputDirectoryButton_Click);
            // 
            // generateButton
            // 
            this.generateButton.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.generateButton.Location = new System.Drawing.Point(0, 268);
            this.generateButton.Name = "generateButton";
            this.generateButton.Size = new System.Drawing.Size(495, 38);
            this.generateButton.TabIndex = 37;
            this.generateButton.Text = " Generate the Stack";
            this.generateButton.Click += new System.EventHandler(this.generateButton_Click);
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(6, 65);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(120, 20);
            this.label10.TabIndex = 36;
            this.label10.Text = "Output Path";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(6, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(120, 21);
            this.label1.TabIndex = 42;
            this.label1.Text = "Target Platform";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(6, 37);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(120, 20);
            this.label2.TabIndex = 44;
            this.label2.Text = "Project Name";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.groupBox2);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(495, 306);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Features";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.UPnP1dot1Enabled);
            this.groupBox2.Controls.Add(this.groupBox5);
            this.groupBox2.Controls.Add(this.CPlusPlusWrapperCheckBox);
            this.groupBox2.Controls.Add(this.IncludeThreadPool);
            this.groupBox2.Controls.Add(this.groupBox4);
            this.groupBox2.Location = new System.Drawing.Point(8, 8);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(480, 288);
            this.groupBox2.TabIndex = 22;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Feature Setup";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.label16);
            this.groupBox5.Controls.Add(this.label13);
            this.groupBox5.Controls.Add(this.InitialHTTPBufferSizeTextBox);
            this.groupBox5.Controls.Add(this.label12);
            this.groupBox5.Controls.Add(this.MaxHTTPPacketSizeTextBox);
            this.groupBox5.Controls.Add(this.MaxHTTPHeaderSizeTextBox);
            this.groupBox5.Controls.Add(this.http11CheckBox);
            this.groupBox5.Location = new System.Drawing.Point(8, 42);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(464, 112);
            this.groupBox5.TabIndex = 36;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Network Configuration (C Stacks Only)";
            // 
            // label16
            // 
            this.label16.Location = new System.Drawing.Point(19, 41);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(189, 16);
            this.label16.TabIndex = 38;
            this.label16.Text = "Max HTTP Header Size (0=No Limit)";
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(19, 86);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(189, 16);
            this.label13.TabIndex = 37;
            this.label13.Text = "Initial HTTP Buffer Size";
            // 
            // InitialHTTPBufferSizeTextBox
            // 
            this.InitialHTTPBufferSizeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.InitialHTTPBufferSizeTextBox.Location = new System.Drawing.Point(356, 83);
            this.InitialHTTPBufferSizeTextBox.Name = "InitialHTTPBufferSizeTextBox";
            this.InitialHTTPBufferSizeTextBox.Size = new System.Drawing.Size(100, 20);
            this.InitialHTTPBufferSizeTextBox.TabIndex = 36;
            this.InitialHTTPBufferSizeTextBox.Text = "65535";
            this.InitialHTTPBufferSizeTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.InitialHTTPBufferSizeTextBox.TextChanged += new System.EventHandler(this.OnNumericTextBoxChanged);
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(19, 64);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(189, 16);
            this.label12.TabIndex = 35;
            this.label12.Text = "Max HTTP Packet Size (0=No Limit)";
            // 
            // MaxHTTPPacketSizeTextBox
            // 
            this.MaxHTTPPacketSizeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.MaxHTTPPacketSizeTextBox.Location = new System.Drawing.Point(356, 61);
            this.MaxHTTPPacketSizeTextBox.Name = "MaxHTTPPacketSizeTextBox";
            this.MaxHTTPPacketSizeTextBox.Size = new System.Drawing.Size(100, 20);
            this.MaxHTTPPacketSizeTextBox.TabIndex = 33;
            this.MaxHTTPPacketSizeTextBox.Text = "0";
            this.MaxHTTPPacketSizeTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.MaxHTTPPacketSizeTextBox.TextChanged += new System.EventHandler(this.OnNumericTextBoxChanged);
            // 
            // MaxHTTPHeaderSizeTextBox
            // 
            this.MaxHTTPHeaderSizeTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.MaxHTTPHeaderSizeTextBox.Location = new System.Drawing.Point(356, 38);
            this.MaxHTTPHeaderSizeTextBox.Name = "MaxHTTPHeaderSizeTextBox";
            this.MaxHTTPHeaderSizeTextBox.Size = new System.Drawing.Size(100, 20);
            this.MaxHTTPHeaderSizeTextBox.TabIndex = 32;
            this.MaxHTTPHeaderSizeTextBox.Text = "0";
            this.MaxHTTPHeaderSizeTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.MaxHTTPHeaderSizeTextBox.TextChanged += new System.EventHandler(this.OnNumericTextBoxChanged);
            // 
            // http11CheckBox
            // 
            this.http11CheckBox.Location = new System.Drawing.Point(16, 16);
            this.http11CheckBox.Name = "http11CheckBox";
            this.http11CheckBox.Size = new System.Drawing.Size(288, 21);
            this.http11CheckBox.TabIndex = 31;
            this.http11CheckBox.Text = "HTTP/1.1 Support (HTTP/1.0 Support if unchecked)";
            // 
            // CPlusPlusWrapperCheckBox
            // 
            this.CPlusPlusWrapperCheckBox.Location = new System.Drawing.Point(9, 176);
            this.CPlusPlusWrapperCheckBox.Name = "CPlusPlusWrapperCheckBox";
            this.CPlusPlusWrapperCheckBox.Size = new System.Drawing.Size(296, 24);
            this.CPlusPlusWrapperCheckBox.TabIndex = 35;
            this.CPlusPlusWrapperCheckBox.Text = "Generate C++ Object Abstraction for C Stack";
            this.CPlusPlusWrapperCheckBox.CheckedChanged += new System.EventHandler(this.CPlusPlusWrapperCheckBox_CheckedChanged);
            // 
            // IncludeThreadPool
            // 
            this.IncludeThreadPool.Enabled = false;
            this.IncludeThreadPool.Location = new System.Drawing.Point(9, 160);
            this.IncludeThreadPool.Name = "IncludeThreadPool";
            this.IncludeThreadPool.Size = new System.Drawing.Size(456, 16);
            this.IncludeThreadPool.TabIndex = 34;
            this.IncludeThreadPool.Text = "Include support for threadpooling, using ILibThreadPool. (Only valid for C Stacks" +
                ")";
            this.IncludeThreadPool.CheckStateChanged += new System.EventHandler(this.OnThreadPoolCheckStateChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.label5);
            this.groupBox4.Controls.Add(this.NumThreadPoolThreads);
            this.groupBox4.Controls.Add(this.InitThreadPool);
            this.groupBox4.Controls.Add(this.defaultIPAddressMonitorCheckBox);
            this.groupBox4.Controls.Add(this.SampleApplication);
            this.groupBox4.Location = new System.Drawing.Point(8, 200);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(464, 80);
            this.groupBox4.TabIndex = 33;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Sample Application";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(144, 56);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(304, 16);
            this.label5.TabIndex = 36;
            this.label5.Text = "threads into a sample thread pool in the sample application";
            // 
            // NumThreadPoolThreads
            // 
            this.NumThreadPoolThreads.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NumThreadPoolThreads.Location = new System.Drawing.Point(104, 52);
            this.NumThreadPoolThreads.MaxLength = 3;
            this.NumThreadPoolThreads.Name = "NumThreadPoolThreads";
            this.NumThreadPoolThreads.Size = new System.Drawing.Size(32, 20);
            this.NumThreadPoolThreads.TabIndex = 35;
            this.NumThreadPoolThreads.Text = "3";
            this.NumThreadPoolThreads.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // InitThreadPool
            // 
            this.InitThreadPool.Enabled = false;
            this.InitThreadPool.Location = new System.Drawing.Point(40, 51);
            this.InitThreadPool.Name = "InitThreadPool";
            this.InitThreadPool.Size = new System.Drawing.Size(64, 24);
            this.InitThreadPool.TabIndex = 34;
            this.InitThreadPool.Text = "Initialize";
            // 
            // defaultIPAddressMonitorCheckBox
            // 
            this.defaultIPAddressMonitorCheckBox.Checked = true;
            this.defaultIPAddressMonitorCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.defaultIPAddressMonitorCheckBox.Location = new System.Drawing.Point(40, 35);
            this.defaultIPAddressMonitorCheckBox.Name = "defaultIPAddressMonitorCheckBox";
            this.defaultIPAddressMonitorCheckBox.Size = new System.Drawing.Size(384, 18);
            this.defaultIPAddressMonitorCheckBox.TabIndex = 32;
            this.defaultIPAddressMonitorCheckBox.Text = "Add default network interface monitoring code to sample application";
            // 
            // SampleApplication
            // 
            this.SampleApplication.Checked = true;
            this.SampleApplication.CheckState = System.Windows.Forms.CheckState.Checked;
            this.SampleApplication.Location = new System.Drawing.Point(16, 16);
            this.SampleApplication.Name = "SampleApplication";
            this.SampleApplication.Size = new System.Drawing.Size(176, 20);
            this.SampleApplication.TabIndex = 33;
            this.SampleApplication.Text = "Generate Sample Application";
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.groupBox6);
            this.tabPage6.Controls.Add(this.groupBox3);
            this.tabPage6.Controls.Add(this.groupBox1);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Size = new System.Drawing.Size(495, 306);
            this.tabPage6.TabIndex = 5;
            this.tabPage6.Text = "Advanced";
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.CertToolCheckBox);
            this.groupBox6.Location = new System.Drawing.Point(8, 188);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(480, 48);
            this.groupBox6.TabIndex = 25;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Certification Test Tool";
            // 
            // CertToolCheckBox
            // 
            this.CertToolCheckBox.Location = new System.Drawing.Point(16, 16);
            this.CertToolCheckBox.Name = "CertToolCheckBox";
            this.CertToolCheckBox.Size = new System.Drawing.Size(152, 24);
            this.CertToolCheckBox.TabIndex = 0;
            this.CertToolCheckBox.Text = "Generate cert  tool files";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.DoNotGenerateRadioButton);
            this.groupBox3.Controls.Add(this.AnyTagRadioButton);
            this.groupBox3.Controls.Add(this.CleanSchemaRadioButton);
            this.groupBox3.Location = new System.Drawing.Point(8, 96);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(480, 89);
            this.groupBox3.TabIndex = 24;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Control && Eventing Schema Generation";
            // 
            // DoNotGenerateRadioButton
            // 
            this.DoNotGenerateRadioButton.Location = new System.Drawing.Point(16, 60);
            this.DoNotGenerateRadioButton.Name = "DoNotGenerateRadioButton";
            this.DoNotGenerateRadioButton.Size = new System.Drawing.Size(448, 24);
            this.DoNotGenerateRadioButton.TabIndex = 2;
            this.DoNotGenerateRadioButton.Text = "Do not generate XSD files";
            // 
            // AnyTagRadioButton
            // 
            this.AnyTagRadioButton.Checked = true;
            this.AnyTagRadioButton.Location = new System.Drawing.Point(16, 39);
            this.AnyTagRadioButton.Name = "AnyTagRadioButton";
            this.AnyTagRadioButton.Size = new System.Drawing.Size(448, 24);
            this.AnyTagRadioButton.TabIndex = 1;
            this.AnyTagRadioButton.TabStop = true;
            this.AnyTagRadioButton.Text = "With \"Any\" tags (Recommanded)";
            // 
            // CleanSchemaRadioButton
            // 
            this.CleanSchemaRadioButton.Location = new System.Drawing.Point(16, 18);
            this.CleanSchemaRadioButton.Name = "CleanSchemaRadioButton";
            this.CleanSchemaRadioButton.Size = new System.Drawing.Size(448, 24);
            this.CleanSchemaRadioButton.TabIndex = 0;
            this.CleanSchemaRadioButton.Text = "Clean - Generate without any tags (For testing only)";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.dynamicObjectModelCheckBox);
            this.groupBox1.Controls.Add(this.externCallbackCheckBox);
            this.groupBox1.Controls.Add(this.errorEncodingCheckBox);
            this.groupBox1.Location = new System.Drawing.Point(8, 8);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(480, 88);
            this.groupBox1.TabIndex = 22;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Code Optimizations";
            // 
            // dynamicObjectModelCheckBox
            // 
            this.dynamicObjectModelCheckBox.Location = new System.Drawing.Point(18, 59);
            this.dynamicObjectModelCheckBox.Name = "dynamicObjectModelCheckBox";
            this.dynamicObjectModelCheckBox.Size = new System.Drawing.Size(206, 24);
            this.dynamicObjectModelCheckBox.TabIndex = 24;
            this.dynamicObjectModelCheckBox.Text = "Allow dynamic device configuration";
            // 
            // externCallbackCheckBox
            // 
            this.externCallbackCheckBox.Location = new System.Drawing.Point(18, 40);
            this.externCallbackCheckBox.Name = "externCallbackCheckBox";
            this.externCallbackCheckBox.Size = new System.Drawing.Size(450, 19);
            this.externCallbackCheckBox.TabIndex = 23;
            this.externCallbackCheckBox.Text = "Use hard coded \"extern\" callbacks";
            // 
            // errorEncodingCheckBox
            // 
            this.errorEncodingCheckBox.Location = new System.Drawing.Point(18, 18);
            this.errorEncodingCheckBox.Name = "errorEncodingCheckBox";
            this.errorEncodingCheckBox.Size = new System.Drawing.Size(450, 18);
            this.errorEncodingCheckBox.TabIndex = 21;
            this.errorEncodingCheckBox.Text = "Use short SOAP error messages";
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.licenseTextBox);
            this.tabPage4.Controls.Add(this.label14);
            this.tabPage4.Controls.Add(this.genOutputTextBox);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Size = new System.Drawing.Size(495, 306);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Generator Log";
            // 
            // licenseTextBox
            // 
            this.licenseTextBox.Location = new System.Drawing.Point(18, 37);
            this.licenseTextBox.Multiline = true;
            this.licenseTextBox.Name = "licenseTextBox";
            this.licenseTextBox.Size = new System.Drawing.Size(459, 253);
            this.licenseTextBox.TabIndex = 42;
            this.licenseTextBox.Text = "/*\r\n * \r\n * $Workfile: <FILE>\r\n * $Revision: <REVISION>\r\n * $Author:   <AUTHOR>\r\n" +
                " * $Date:     <DATE>\r\n *\r\n */";
            this.licenseTextBox.Visible = false;
            // 
            // label14
            // 
            this.label14.Location = new System.Drawing.Point(9, 9);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(178, 19);
            this.label14.TabIndex = 41;
            this.label14.Text = "Code Generation Output";
            // 
            // genOutputTextBox
            // 
            this.genOutputTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.genOutputTextBox.Location = new System.Drawing.Point(9, 28);
            this.genOutputTextBox.Multiline = true;
            this.genOutputTextBox.Name = "genOutputTextBox";
            this.genOutputTextBox.ReadOnly = true;
            this.genOutputTextBox.Size = new System.Drawing.Size(475, 265);
            this.genOutputTextBox.TabIndex = 40;
            // 
            // CodeGenerationForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(518, 350);
            this.Controls.Add(this.tabControl1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "CodeGenerationForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Code Generation";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.OnClosing);
            this.Load += new System.EventHandler(this.CodeGenerationForm_Load);
            this.HelpRequested += new System.Windows.Forms.HelpEventHandler(this.CodeGenerationForm_HelpRequested);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.tabPage6.ResumeLayout(false);
            this.groupBox6.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.tabPage4.PerformLayout();
            this.ResumeLayout(false);

		}
		#endregion

		private void UpdateSettings()
		{
			StackConfiguration.callconvention = (ServiceGenerator.CALLINGCONVENTION)(int)this.callConventionComboBox.SelectedIndex;
			StackConfiguration.classname = this.classNameTextBox.Text;
			StackConfiguration.DefaultIPAddressMonitor = this.defaultIPAddressMonitorCheckBox.Checked;
			StackConfiguration.ExplicitErrorEncoding = this.errorEncodingCheckBox.Checked;
			StackConfiguration.EXTERN_Callbacks = this.externCallbackCheckBox.Checked;
			StackConfiguration.HTTP_1dot1 = this.http11CheckBox.Checked;
			StackConfiguration.indent = (ServiceGenerator.INDENTATION)(int)this.indentComboBox.SelectedIndex;
			StackConfiguration.newline = (ServiceGenerator.NEWLINETYPE)(int)this.newLineComboBox.SelectedIndex;
			StackConfiguration.outputpath = this.outputPathTextBox.Text;
			StackConfiguration.prefixlib = this.libPrefixTextBox.Text;
			StackConfiguration.projectname = this.projectNameTextBox.Text;
			StackConfiguration.SupressSampleProject = !this.SampleApplication.Checked;
			StackConfiguration.TargetPlatform = (ServiceGenerator.PLATFORMS)(int)this.platformComboBox.SelectedIndex;
			StackConfiguration.UPNP_1dot1 = this.UPnP1dot1Enabled.Checked;
			StackConfiguration.GenerateThreadPoolLibrary = this.IncludeThreadPool.Checked && this.IncludeThreadPool.Enabled;
			StackConfiguration.InitThreadPoolInSampleApp = this.InitThreadPool.Checked && this.InitThreadPool.Enabled;
			StackConfiguration.DynamicObjectModel = this.dynamicObjectModelCheckBox.Checked;
			StackConfiguration.CPlusPlusWrapper = this.CPlusPlusWrapperCheckBox.Checked;
			StackConfiguration.MaxHTTPPacketSize = int.Parse(this.MaxHTTPPacketSizeTextBox.Text);
			StackConfiguration.MaxHTTPHeaderSize = int.Parse(this.MaxHTTPHeaderSizeTextBox.Text);
			StackConfiguration.InitialHTTPBufferSize = int.Parse(this.InitialHTTPBufferSizeTextBox.Text);
			StackConfiguration.GenerateCertToolFiles = this.CertToolCheckBox.Checked;

			if (StackConfiguration.InitialHTTPBufferSize==0)
			{
				MessageBox.Show("Initial HTTP Buffer size cannot be 0. Setting to default value of 65535");
				StackConfiguration.InitialHTTPBufferSize = 65535;
			}

			try
			{
				int x = int.Parse(NumThreadPoolThreads.Text);
				if (x<=0)
				{
					StackConfiguration.ThreadPoolThreads_InSampleApp = 3;
					StackConfiguration.InitThreadPoolInSampleApp = false;
				}
				else
				{
					StackConfiguration.ThreadPoolThreads_InSampleApp = x;
				}
			}
			catch
			{
				StackConfiguration.ThreadPoolThreads_InSampleApp = 3;
				StackConfiguration.InitThreadPoolInSampleApp = false;
			}

			if (this.DoNotGenerateRadioButton.Checked)
			{
				StackConfiguration.SchemaGeneration = ServiceGenerator.XSDSchemaGeneration.NONE;
			}
			else if (this.CleanSchemaRadioButton.Checked)
			{
				StackConfiguration.SchemaGeneration = ServiceGenerator.XSDSchemaGeneration.WITHOUT_ANY_TAGS;
			}
			else
			{
				StackConfiguration.SchemaGeneration = ServiceGenerator.XSDSchemaGeneration.WITH_ANY_TAGS;
			}
		}
		public void generateButton_Click(object sender, System.EventArgs e)
		{
			string license;
			UPnPDevice[] devices = this.GetDevices();

			UpdateSettings();

            DirectoryInfo outputDir = null;
            try
            {
                outputDir = new DirectoryInfo(outputPathTextBox.Text);
            }
            catch (Exception) { }
            if (outputDir == null || outputDir.Exists == false) 
			{
				MessageBox.Show(this, "Output Path is invalid", "Code Generator", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
            if (StackConfiguration.TargetPlatform == ServiceGenerator.PLATFORMS.ANDROID)
            {
                //
                // Validate for Java/Android Projects that the appopriate fields are filled in
                //
                if (projectNameTextBox.Text == "")
                {
                    MessageBox.Show("Project name must be filled out!", "Code Generator", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                if (classNameTextBox.Text.Split(new String[] { "." }, StringSplitOptions.None).Length < 2)
                {
                    MessageBox.Show("Namespace (Package) is too short. Must be at least two identifiers seperated by a period. (sample.application)", "Code Generator", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
            }


			string buttonText = generateButton.Text;
			generateButton.Text = "Generating Stack...";
			generateButton.Enabled = false;


			CodeGenerator gen = null;

			if (StackConfiguration.SchemaGeneration != ServiceGenerator.XSDSchemaGeneration.NONE)
			{
				foreach(UPnPDevice device in devices)
				{
					// Build the Control Schemas
					GenerateControlSchemas(device,outputDir,StackConfiguration.SchemaGeneration==ServiceGenerator.XSDSchemaGeneration.WITHOUT_ANY_TAGS);

					// Build the Event Schemas
					GenerateEventSchemas(device,outputDir,StackConfiguration.SchemaGeneration==ServiceGenerator.XSDSchemaGeneration.WITHOUT_ANY_TAGS);
				}
			}

			if (StackConfiguration.GenerateCertToolFiles)
			{
				foreach(UPnPDevice device in devices)
				{
					ServiceGenerator.Configuration DeviceConf = (ServiceGenerator.Configuration)device.User;
					if (DeviceConf.ConfigType==ServiceGenerator.ConfigurationType.DEVICE)
					{
						BuildCertToolFiles(device,outputDir);
					}
				}
			}

			switch(StackConfiguration.TargetPlatform)
			{
                case ServiceGenerator.PLATFORMS.ANDROID:
                        #region Java/Android Device Stack
                   
						gen = new JavaAndroidGenerator(StackConfiguration);
						genOutputTextBox.Clear();

						gen.OnLogOutput += new DotNetGenerator.LogOutputHandler(Log);
						try 
						{
							gen.Generate(devices,outputDir);
						} 
						catch (Exception ex)
						{
							MessageBox.Show(this, "Error Generating Code. Exception logged to \"Exceptions.txt\".", "Code Generator");
                            LogErrorToFile(string.Format("{0} - {1}\r\n", DateTime.Now.ToString(), ex.ToString()));
                            OpenSource.UPnP.AutoUpdate.ReportCrash(Application.ProductName, ex.ToString());
						}
						gen.OnLogOutput -= new DotNetGenerator.LogOutputHandler(Log);
						#endregion
                    break;
				case ServiceGenerator.PLATFORMS.DOTNET:
					#region .NET Stack (Device & Control Point)
						#region .NET Device Stack
						gen = new DotNetGenerator(classNameTextBox.Text,StackConfiguration);
						genOutputTextBox.Clear();

						gen.OnLogOutput += new DotNetGenerator.LogOutputHandler(Log);
						try 
						{
							gen.Generate(devices,outputDir);
						} 
						catch
						{
							MessageBox.Show(this,"Error Generating Code","Code Generator");
						}
						gen.OnLogOutput -= new DotNetGenerator.LogOutputHandler(Log);
						#endregion
						#region .NET CP Stack
						gen = new CPDotNetGenerator(classNameTextBox.Text,StackConfiguration);
						genOutputTextBox.Clear();

						gen.OnLogOutput += new CPDotNetGenerator.LogOutputHandler(Log);
						try 
						{
							gen.Generate(devices,outputDir);
						} 
						catch
						{
							MessageBox.Show(this,"Error Generating Code","Code Generator");
						}
						gen.OnLogOutput -= new CPDotNetGenerator.LogOutputHandler(Log);
						#endregion
					#endregion
					break;
				default:
					// POSIX, Windows, PocketPC Code Generation
					#region POSIX, Windows, PocketPC (Device & Control Point), Symbian OS
				
					#region Generator Setup
					gen = new EmbeddedCGenerator(StackConfiguration);

					// Setup License
					license = licenseTextBox.Text;
					license = license.Replace("<AUTHOR>","Device Builder");
					license = license.Replace("<REVISION>","#" + Application.ProductVersion);
					license = license.Replace("<DATE>",DateTime.Now.ToLongDateString());
					gen.License = license;
					
					gen.OnLogOutput += new EmbeddedCGenerator.LogOutputHandler(Log);
					#endregion

					gen.Generate(devices,outputDir);

					gen.OnLogOutput -= new EmbeddedCGenerator.LogOutputHandler(Log);
					#endregion
					break;
			}
		
			generateButton.Enabled = true;
			generateButton.Text = buttonText;
		}

		private void BuildCertToolFiles(UPnPDevice device, DirectoryInfo outputDir)
		{
			StringBuilder sb = new StringBuilder();
			ServiceGenerator.Configuration DeviceConf = (ServiceGenerator.Configuration)device.User;

			sb.Append("<device>");
			if (device.Services.Length>0)
			{
				sb.Append("<serviceList>");
				foreach(UPnPService s in device.Services)
				{
					sb.Append("<service>");
					sb.Append("<serviceType>"+s.ServiceURN+"</serviceType>");
					sb.Append("<serviceId>"+s.ServiceID+"</serviceId>");
					sb.Append("</service>");
					BuildCertToolFiles_Service(s,outputDir);
				}
				sb.Append("</serviceList>");
			}
			sb.Append("</device>");
			

			StreamWriter W;
			DText d = new DText();
			d.ATTRMARK = ":";
			d[0] = device.DeviceURN;

			W = File.CreateText(outputDir.FullName + "\\"+d[4]+device.Version.ToString()+".xml");
			W.Write(sb.ToString());
			W.Close();

		}
		private void BuildCertToolFiles_Service(UPnPService s, DirectoryInfo outputDir)
		{
			StringBuilder sb = new StringBuilder();
			sb.Append("<scpd>");

			sb.Append("<serviceStateTable>");
			foreach(UPnPStateVariable v in s.GetStateVariables())
			{
				sb.Append("<stateVariable>");
				sb.Append("<name>"+v.Name+"</name>");
				sb.Append("<sendEventsAttribute>");
				if (v.SendEvent)
				{
					sb.Append("yes");
				}
				else
				{
					sb.Append("no");
				}
				sb.Append("</sendEventsAttribute>");
				sb.Append("<dataType>"+v.ValueType+"</dataType>");
				if (v.AllowedStringValues!=null && v.AllowedStringValues.Length>0)
				{
					sb.Append("<allowedValueList>");
					foreach(string av in v.AllowedStringValues)
					{
						sb.Append("<allowedValue>"+av+"</allowedValue>");
					}
					sb.Append("</allowedValueList>");
				}
				sb.Append("</stateVariable>");
			}
			sb.Append("</serviceStateTable>");

			if (s.Actions.Count>0)
			{
				sb.Append("<actionList>");
				foreach(UPnPAction a in s.Actions)
				{
					sb.Append("<action>");
					sb.Append("<name>"+a.Name+"</name>");
					if (a.ArgumentList.Length>0)
					{
						sb.Append("<argumentList>");
						foreach(UPnPArgument ag in a.ArgumentList)
						{
							sb.Append("<argument>");
							sb.Append("<name>"+ag.Name+"</name>");
							sb.Append("<direction>"+ag.Direction+"</direction>");
							sb.Append("<relatedStateVariable>"+ag.RelatedStateVar.Name+"</relatedStateVariable>");
							sb.Append("</argument>");
						}																								
						sb.Append("</argumentList>");
					}
					sb.Append("</action>");
				}
				sb.Append("</actionList>");
			}

			sb.Append("</scpd>");

			StreamWriter W;
			DText d = new DText();
			d.ATTRMARK = ":";
			d[0] = s.ServiceURN;

			W = File.CreateText(outputDir.FullName + "\\" + d[4] + s.Version.ToString() + ".xml");
			W.Write(sb.ToString());
			W.Close();
		}

		private void Log(object sender, string msg) 
		{
			genOutputTextBox.Text += msg + "\r\n";
		}

		private void outputDirectoryButton_Click(object sender, System.EventArgs e)
		{
            DirectoryInfo di = null;
            try
            {
                di = new DirectoryInfo(outputPathTextBox.Text);
                if (di.Exists) folderBrowserDialog.SelectedPath = di.FullName;
            }
            catch (Exception) { }
            if (folderBrowserDialog.ShowDialog(this) == DialogResult.OK) outputPathTextBox.Text = folderBrowserDialog.SelectedPath;
		}

		private void platformComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			this.IncludeThreadPool.Enabled = false;
			this.InitThreadPool.Enabled = false;
			this.NumThreadPoolThreads.Enabled = false;
            label7.Text = "Namespace";

			switch (platformComboBox.SelectedIndex) 
			{
				case (int)ServiceGenerator.PLATFORMS.DOTNET: // .NET Framework Code Generation
					projectNameTextBox.Enabled = false;
					newLineComboBox.Enabled = false;
					callConventionComboBox.Enabled = false;
					libPrefixTextBox.Enabled = false;
					indentComboBox.Enabled = false;
					classNameTextBox.Enabled = true;
					defaultIPAddressMonitorCheckBox.Enabled = false;

					errorEncodingCheckBox.Enabled = false;
					externCallbackCheckBox.Enabled = false;
					http11CheckBox.Enabled = false;
					break;
                case (int)ServiceGenerator.PLATFORMS.ANDROID: // Java Android Code Generation
                    projectNameTextBox.Enabled = true;
					newLineComboBox.Enabled = false;
					callConventionComboBox.Enabled = false;
					libPrefixTextBox.Enabled = false;
					indentComboBox.Enabled = false;
					classNameTextBox.Enabled = true;
					defaultIPAddressMonitorCheckBox.Enabled = false;

					errorEncodingCheckBox.Enabled = false;
					externCallbackCheckBox.Enabled = false;
					http11CheckBox.Enabled = false;
                    label7.Text = "Package";
					break;
				default: // Microstack Generation
					this.IncludeThreadPool.Enabled = true;
					this.InitThreadPool.Enabled = IncludeThreadPool.Checked;
					this.NumThreadPoolThreads.Enabled = IncludeThreadPool.Checked;

					projectNameTextBox.Enabled = false;
					newLineComboBox.Enabled = true;
					callConventionComboBox.Enabled = true;
					libPrefixTextBox.Enabled = true;
					indentComboBox.Enabled = true;
					classNameTextBox.Enabled = false;
					defaultIPAddressMonitorCheckBox.Enabled = true;

					errorEncodingCheckBox.Enabled = true;
					externCallbackCheckBox.Enabled = true;
					http11CheckBox.Enabled = true;
					break;
			}
		}

		private void CodeGenerationForm_HelpRequested(object sender, System.Windows.Forms.HelpEventArgs hlpevent)
		{
			Help.ShowHelp(this,"AuthoringToolsHelp.chm",HelpNavigator.KeywordIndex,"Device Generation Form");
		}
		private void GenerateEventSchemas(UPnPDevice d, System.IO.DirectoryInfo dirInfo, bool cleanSchema)
		{
			System.IO.MemoryStream ms = new MemoryStream();
			System.Xml.XmlTextWriter X = new System.Xml.XmlTextWriter(ms,System.Text.Encoding.UTF8);
			X.Formatting = System.Xml.Formatting.Indented;


			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				GenerateEventSchemas(ed,dirInfo,cleanSchema);
			}
			foreach(UPnPService s in d.Services)
			{
				Hashtable h = new Hashtable();
				int j=1;

				foreach(string sn in s.GetSchemaNamespaces())
				{
					h[sn] = "CT"+j.ToString();
					++j;
				}
				X.WriteStartDocument();
				X.WriteStartElement("xsd","schema","http://www.w3.org/2001/XMLSchema");
				X.WriteAttributeString("targetNamespace","urn:schemas-upnp-org:event-1-0");
				X.WriteAttributeString("xmlns","upnp",null,"http://www.upnp.org/Schema/DataTypes");
				X.WriteAttributeString("xmlns","urn:schemas-upnp-org:event-1-0");

				foreach(UPnPStateVariable v in s.GetStateVariables())
				{
					if (v.SendEvent)
					{
						X.WriteStartElement("xsd","element",null); // Element1
						X.WriteAttributeString("name","propertyset");
						X.WriteAttributeString("type","propertysetType");
						
						if (!cleanSchema)
						{
							X.WriteComment("Note: Some schema validation tools may consider the following xsd:any element to be ambiguous in its placement");
							X.WriteStartElement("xsd","any",null);
							X.WriteAttributeString("namespace","##other");
							X.WriteAttributeString("minOccurs","0");
							X.WriteAttributeString("maxOccurs","unbounded");
							X.WriteAttributeString("processContents","lax");
							X.WriteEndElement(); //ANY
						}

						X.WriteStartElement("xsd","complexType",null);
						X.WriteAttributeString("name","propertysetType");

						X.WriteStartElement("xsd","sequence",null);
						X.WriteStartElement("xsd","element",null); // Element2
						X.WriteAttributeString("name","property");
						X.WriteAttributeString("maxOccurs","unbounded");
						X.WriteStartElement("xsd","complexType",null);
						X.WriteStartElement("xsd","sequence",null);


						X.WriteStartElement("xsd","element",null); // Element3
						X.WriteAttributeString("name",v.Name);
						if (v.ComplexType==null)
						{
							// Simple Type
							X.WriteStartElement("xsd","complexType",null);
							X.WriteStartElement("xsd","simpleContent",null);
							X.WriteStartElement("xsd","extension",null);
							X.WriteAttributeString("base","upnp:"+v.ValueType);
							if (!cleanSchema)
							{
								X.WriteStartElement("xsd","anyAttribute",null);
								X.WriteAttributeString("namespace","##other");
								X.WriteAttributeString("processContents","lax");
								X.WriteEndElement(); // anyAttribute
							}
							X.WriteEndElement(); // extension
							X.WriteEndElement(); // simpleContent
							X.WriteEndElement(); // complexType
						}
						else
						{
							// Complex Type
							X.WriteAttributeString("type",h[v.ComplexType.Name_NAMESPACE].ToString()+":"+v.ComplexType.Name_LOCAL);
						}
						X.WriteEndElement(); // Element3
						if (!cleanSchema)
						{
							X.WriteStartElement("xsd","any",null);
							X.WriteAttributeString("namespace","##other");
							X.WriteAttributeString("minOccurs","0");
							X.WriteAttributeString("maxOccurs","unbounded");
							X.WriteAttributeString("processContents","lax");
							X.WriteEndElement(); // any
						}
						X.WriteEndElement(); // sequence
						if (!cleanSchema)
						{
							X.WriteStartElement("xsd","anyAttribute",null);
							X.WriteAttributeString("namespace","##other");
							X.WriteAttributeString("processContents","lax");
							X.WriteEndElement(); // anyAttribute
						}
						X.WriteEndElement(); // complexType
						X.WriteEndElement(); // Element2
						if (!cleanSchema)
						{
							X.WriteStartElement("xsd","any",null);
							X.WriteAttributeString("namespace","##other");
							X.WriteAttributeString("minOccurs","0");
							X.WriteAttributeString("maxOccurs","unbounded");
							X.WriteAttributeString("processContents","lax");
							X.WriteEndElement(); // any
						}
						X.WriteEndElement(); // sequence
						if (!cleanSchema)
						{
							X.WriteStartElement("xsd","anyAttribute",null);
							X.WriteAttributeString("namespace","##other");
							X.WriteAttributeString("processContents","lax");
							X.WriteEndElement(); // anyAttribute
						}
						X.WriteEndElement(); // complexType;
						X.WriteEndElement(); // Element1
					}
				}

				X.WriteEndElement(); // schema
				X.WriteEndDocument();

				StreamWriter writer3;

				DText PP = new DText();
				PP.ATTRMARK = ":";
				PP[0] = s.ServiceURN;
				writer3 = File.CreateText(dirInfo.FullName + "\\"+PP[PP.DCOUNT()-1]+"_Events.xsd");
				
				System.Text.UTF8Encoding U = new System.Text.UTF8Encoding();
				X.Flush();
				ms.Flush();
				writer3.Write(U.GetString(ms.ToArray(),2,ms.ToArray().Length-2));
				writer3.Close();
				ms = new MemoryStream();
				X = new System.Xml.XmlTextWriter(ms,System.Text.Encoding.UTF8);
				X.Formatting = System.Xml.Formatting.Indented;
			}
		}
		private void GenerateControlSchemas(UPnPDevice d, System.IO.DirectoryInfo dirInfo, bool cleanSchema)
		{
			System.IO.MemoryStream ms = new MemoryStream();
			System.Xml.XmlTextWriter X = new System.Xml.XmlTextWriter(ms,System.Text.Encoding.UTF8);
			X.Formatting = System.Xml.Formatting.Indented;


			foreach(UPnPDevice ed in d.EmbeddedDevices)
			{
				GenerateControlSchemas(ed,dirInfo,cleanSchema);
			}
			foreach(UPnPService s in d.Services)
			{
				Hashtable h = new Hashtable();
				int j=1;

				foreach(string sn in s.GetSchemaNamespaces())
				{
					h[sn] = "CT"+j.ToString();
					++j;
				}
				X.WriteStartDocument();
				X.WriteStartElement("xsd","schema","http://www.w3.org/2001/XMLSchema");
				X.WriteAttributeString("targetNamespace",s.ServiceURN);
				X.WriteAttributeString("xmlns",s.ServiceURN);
				X.WriteAttributeString("xmlns","upnp",null,"http://www.upnp.org/Schema/DataTypes");
				IDictionaryEnumerator NE = h.GetEnumerator();
				while(NE.MoveNext())
				{
					X.WriteAttributeString("xmlns",NE.Value.ToString(),null,NE.Key.ToString());
				}

				foreach(UPnPAction a in s.Actions)
				{
					X.WriteStartElement("xsd","element",null);
					X.WriteAttributeString("name",a.Name);
					X.WriteAttributeString("type",a.Name+"Type");
					X.WriteEndElement();
					X.WriteStartElement("xsd","element",null);
					X.WriteAttributeString("name",a.Name+"Response");
					X.WriteAttributeString("type",a.Name+"ResponseType");
					X.WriteEndElement();			

					if (!cleanSchema)
					{
						X.WriteComment("Note: Some schema validation tools may consider the following xsd:any element ambiguous in this placement");
						X.WriteStartElement("xsd","any",null);
						X.WriteAttributeString("namespace","##other");
						X.WriteAttributeString("minOccurs","0");
						X.WriteAttributeString("maxOccurs","unbounded");
						X.WriteAttributeString("processContents","lax");
						X.WriteEndElement(); // ANY
					}

					X.WriteStartElement("xsd","complexType",null);
					X.WriteAttributeString("name",a.Name+"Type");
					

					X.WriteStartElement("xsd","sequence",null);


					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="in")
						{
							X.WriteStartElement("xsd","element",null);
							X.WriteAttributeString("name",arg.Name);
							if (arg.RelatedStateVar.ComplexType==null)
							{
								// Simple Types
								X.WriteStartElement("xsd","complexType",null);
								X.WriteStartElement("xsd","simpleContent",null);
								X.WriteStartElement("xsd","extension",null);
								X.WriteAttributeString("base","upnp:"+arg.RelatedStateVar.ValueType);
								
								if (!cleanSchema)
								{
									X.WriteStartElement("xsd","anyAttribute",null);
									X.WriteAttributeString("namespace","##other");
									X.WriteAttributeString("processContents","lax");
									X.WriteEndElement(); // anyAttribute
								}
								
								X.WriteEndElement(); // Extension
								X.WriteEndElement(); // simpleConent
								X.WriteEndElement(); // complexType
							}
							else
							{
								// Complex Types
								X.WriteAttributeString("type",h[arg.RelatedStateVar.ComplexType.Name_NAMESPACE].ToString()+":"+arg.RelatedStateVar.ComplexType.Name_LOCAL);
							}
							X.WriteEndElement(); // element
						}
					}

					if (!cleanSchema)
					{
						X.WriteStartElement("xsd","any",null);
						X.WriteAttributeString("namespace","##other");
						X.WriteAttributeString("minOccurs","0");
						X.WriteAttributeString("maxOccurs","unbounded");
						X.WriteAttributeString("processContents","lax");
						X.WriteEndElement(); // any
					}


					X.WriteEndElement(); // sequence

					if (!cleanSchema)
					{
						X.WriteStartElement("xsd","anyAttribute",null);
						X.WriteAttributeString("namespace","##other");
						X.WriteAttributeString("processContents","lax");
						X.WriteEndElement(); // anyAttribute
					}
					X.WriteEndElement(); // complexType


					// ActionResponse
					X.WriteStartElement("xsd","complexType",null);
					X.WriteAttributeString("name",a.Name+"ResponseType");
					X.WriteStartElement("xsd","sequence",null);

					foreach(UPnPArgument arg in a.Arguments)
					{
						if (arg.Direction=="out" || arg.IsReturnValue)
						{
							X.WriteStartElement("xsd","element",null);
							X.WriteAttributeString("name",arg.Name);
							if (arg.RelatedStateVar.ComplexType==null)
							{
								// Simple
								X.WriteStartElement("xsd","complexType",null);
								X.WriteStartElement("xsd","simpleContent",null);
								X.WriteStartElement("xsd","extension",null);
								X.WriteAttributeString("base","upnp:"+arg.RelatedStateVar.ValueType);
								if (!cleanSchema)
								{
									X.WriteStartElement("xsd","anyAttribute",null);
									X.WriteAttributeString("namespace","##other");
									X.WriteAttributeString("processContents","lax");
									X.WriteEndElement(); // anyAttribute
								}
								X.WriteEndElement(); // extension
								X.WriteEndElement(); // simpleContent
								X.WriteEndElement(); // complexType
							}
							else
							{
								// Complex
								X.WriteAttributeString("type",h[arg.RelatedStateVar.ComplexType.Name_NAMESPACE].ToString()+":"+arg.RelatedStateVar.ComplexType.Name_LOCAL);
							}
							X.WriteEndElement(); // Element
						}
					}
					// After all arguments
					if (!cleanSchema)
					{
						X.WriteStartElement("xsd","any",null);
						X.WriteAttributeString("namespace","##other");
						X.WriteAttributeString("minOccurs","0");
						X.WriteAttributeString("maxOccurs","unbounded");
						X.WriteAttributeString("processContents","lax");
						X.WriteEndElement(); // any
					}
					X.WriteEndElement(); // sequence
					if (!cleanSchema)
					{
						X.WriteStartElement("xsd","anyAttribute",null);
						X.WriteAttributeString("namespace","##other");
						X.WriteAttributeString("processContents","lax");
						X.WriteEndElement(); // anyAttribute
					}
					X.WriteEndElement(); // complexType
				}

				X.WriteEndElement(); //schema
				X.WriteEndDocument();

				StreamWriter writer3;

				DText PP = new DText();
				PP.ATTRMARK = ":";
				PP[0] = s.ServiceURN;
				writer3 = File.CreateText(dirInfo.FullName + "\\"+PP[PP.DCOUNT()-1]+".xsd");
				
				System.Text.UTF8Encoding U = new System.Text.UTF8Encoding();
				X.Flush();
				ms.Flush();
				writer3.Write(U.GetString(ms.ToArray(),2,ms.ToArray().Length-2));
				writer3.Close();
				ms = new MemoryStream();
				X = new System.Xml.XmlTextWriter(ms,System.Text.Encoding.UTF8);
				X.Formatting = System.Xml.Formatting.Indented;
			}
		}

		private void CodeGenerationForm_Load(object sender, System.EventArgs e)
		{
		
		}

		private void OnClosing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			UpdateSettings();
		}

		private void OnThreadPoolCheckStateChanged(object sender, System.EventArgs e)
		{
			this.InitThreadPool.Enabled = this.IncludeThreadPool.Checked;
			this.NumThreadPoolThreads.Enabled = this.IncludeThreadPool.Checked;
		}

		private void OnNumericTextBoxChanged(object sender, System.EventArgs e)
		{
			if (((TextBox)sender).Text.Length>0)
			{
				int idx = ((TextBox)sender).SelectionStart;
				if (idx<0){idx=((TextBox)sender).Text.Length;}
				try
				{
					int.Parse(((TextBox)sender).Text);
				}
				catch
				{
					((TextBox)sender).Text=((TextBox)sender).Text.Remove(idx-1,1);
				}
				((TextBox)sender).SelectionStart = idx;
			}
			else
			{
				((TextBox)sender).Text = "0";
			}
		}

		private void CPlusPlusWrapperCheckBox_CheckedChanged(object sender, System.EventArgs e)
		{
			if (this.CPlusPlusWrapperCheckBox.Checked)
			{
				this.externCallbackCheckBox.Checked = false;
				this.externCallbackCheckBox.Enabled = false;
			}
			else
			{
				this.externCallbackCheckBox.Enabled = true;
			}
		}
	}
}
