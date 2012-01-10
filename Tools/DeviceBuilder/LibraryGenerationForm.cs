using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using Intel.UPNP;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for CodeGenerationForm.
	/// </summary>
	public class LibraryGenerationForm : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button outputDirectoryButton;
		private System.Windows.Forms.Label label14;
		private System.Windows.Forms.TextBox genOutputTextBox;
		private System.Windows.Forms.Button generateButton;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TextBox outputPathTextBox;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox platformComboBox;
		private System.Windows.Forms.ComboBox languageComboBox;
		private System.Windows.Forms.ComboBox newLineComboBox;
		private System.Windows.Forms.ComboBox callConventionComboBox;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.ComboBox indentComboBox;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox classNameTextBox;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.TextBox libPrefixTextBox;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.CheckedListBox modulesCheckedListBox;
		private System.Windows.Forms.TextBox licenseTextBox;
		private System.Windows.Forms.Label label8;

		public LibraryGenerationForm()
		{
			InitializeComponent();

			platformComboBox.SelectedIndex = 0;
			languageComboBox.SelectedIndex = 0;
			newLineComboBox.SelectedIndex = 0;
			indentComboBox.SelectedIndex = 0;
			callConventionComboBox.SelectedIndex = 0;
		}

		public Hashtable Settings 
		{
			get
			{
				Hashtable t = new Hashtable();
				t["outputpath"] = outputPathTextBox.Text;
				t["platform"] = platformComboBox.SelectedIndex;
				t["language"] = languageComboBox.SelectedIndex;
				t["newline"] = newLineComboBox.SelectedIndex;
				t["callconvention"] = callConventionComboBox.SelectedIndex;
				t["indent"] = indentComboBox.SelectedIndex;			
				t["classname"] = classNameTextBox.Text;
				return t;
			}
			set
			{
				if (value.Contains("outputpath")) outputPathTextBox.Text = (string)value["outputpath"];
				if (value.Contains("platform")) platformComboBox.SelectedIndex = (int)value["platform"];
				if (value.Contains("language")) languageComboBox.SelectedIndex = (int)value["language"];
				if (value.Contains("newline")) newLineComboBox.SelectedIndex = (int)value["newline"];
				if (value.Contains("callconvention")) callConventionComboBox.SelectedIndex = (int)value["callconvention"];
				if (value.Contains("indent")) indentComboBox.SelectedIndex = (int)value["indent"];
				if (value.Contains("classname")) classNameTextBox.Text = (string)value["classname"];
				classNameTextBox.Enabled = (languageComboBox.SelectedIndex == 1);
			}
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(LibraryGenerationForm));
			this.outputDirectoryButton = new System.Windows.Forms.Button();
			this.label14 = new System.Windows.Forms.Label();
			this.genOutputTextBox = new System.Windows.Forms.TextBox();
			this.generateButton = new System.Windows.Forms.Button();
			this.label10 = new System.Windows.Forms.Label();
			this.outputPathTextBox = new System.Windows.Forms.TextBox();
			this.platformComboBox = new System.Windows.Forms.ComboBox();
			this.label1 = new System.Windows.Forms.Label();
			this.languageComboBox = new System.Windows.Forms.ComboBox();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.newLineComboBox = new System.Windows.Forms.ComboBox();
			this.label4 = new System.Windows.Forms.Label();
			this.callConventionComboBox = new System.Windows.Forms.ComboBox();
			this.label6 = new System.Windows.Forms.Label();
			this.indentComboBox = new System.Windows.Forms.ComboBox();
			this.label7 = new System.Windows.Forms.Label();
			this.classNameTextBox = new System.Windows.Forms.TextBox();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.libPrefixTextBox = new System.Windows.Forms.TextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.modulesCheckedListBox = new System.Windows.Forms.CheckedListBox();
			this.label5 = new System.Windows.Forms.Label();
			this.licenseTextBox = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// outputDirectoryButton
			// 
			this.outputDirectoryButton.Image = ((System.Drawing.Bitmap)(resources.GetObject("outputDirectoryButton.Image")));
			this.outputDirectoryButton.Location = new System.Drawing.Point(456, 65);
			this.outputDirectoryButton.Name = "outputDirectoryButton";
			this.outputDirectoryButton.Size = new System.Drawing.Size(29, 25);
			this.outputDirectoryButton.TabIndex = 16;
			this.outputDirectoryButton.Click += new System.EventHandler(this.outputDirectoryButton_Click);
			// 
			// label14
			// 
			this.label14.Location = new System.Drawing.Point(9, 355);
			this.label14.Name = "label14";
			this.label14.Size = new System.Drawing.Size(178, 19);
			this.label14.TabIndex = 15;
			this.label14.Text = "Code Generation Output";
			// 
			// genOutputTextBox
			// 
			this.genOutputTextBox.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.genOutputTextBox.Location = new System.Drawing.Point(9, 374);
			this.genOutputTextBox.Multiline = true;
			this.genOutputTextBox.Name = "genOutputTextBox";
			this.genOutputTextBox.ReadOnly = true;
			this.genOutputTextBox.Size = new System.Drawing.Size(477, 79);
			this.genOutputTextBox.TabIndex = 12;
			this.genOutputTextBox.Text = "";
			// 
			// generateButton
			// 
			this.generateButton.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right);
			this.generateButton.Location = new System.Drawing.Point(9, 465);
			this.generateButton.Name = "generateButton";
			this.generateButton.Size = new System.Drawing.Size(477, 37);
			this.generateButton.TabIndex = 11;
			this.generateButton.Text = "Generate Selected Modules";
			this.generateButton.Click += new System.EventHandler(this.generateButton_Click);
			// 
			// label10
			// 
			this.label10.Location = new System.Drawing.Point(9, 65);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(75, 19);
			this.label10.TabIndex = 10;
			this.label10.Text = "Output Path";
			// 
			// outputPathTextBox
			// 
			this.outputPathTextBox.Location = new System.Drawing.Point(131, 65);
			this.outputPathTextBox.Name = "outputPathTextBox";
			this.outputPathTextBox.Size = new System.Drawing.Size(318, 20);
			this.outputPathTextBox.TabIndex = 9;
			this.outputPathTextBox.Text = "C:\\Temp2";
			this.toolTip1.SetToolTip(this.outputPathTextBox, "The output path of the generated files. Generated files will overwrite existing f" +
				"iles without prompting.");
			// 
			// platformComboBox
			// 
			this.platformComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.platformComboBox.Items.AddRange(new object[] {
																  "Standard POSIX (Standard Sockets)",
																  "Windows, Embedded Windows (WinSock1)",
																  "Windows, Embedded Windows (WinSock2)",
																  "PocketPC 2002 (WinSock1)"});
			this.platformComboBox.Location = new System.Drawing.Point(131, 9);
			this.platformComboBox.Name = "platformComboBox";
			this.platformComboBox.Size = new System.Drawing.Size(355, 21);
			this.platformComboBox.TabIndex = 17;
			this.toolTip1.SetToolTip(this.platformComboBox, "Target platform for generated code.");
			this.platformComboBox.SelectedIndexChanged += new System.EventHandler(this.platformComboBox_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(9, 9);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(103, 19);
			this.label1.TabIndex = 18;
			this.label1.Text = "Target Platform";
			// 
			// languageComboBox
			// 
			this.languageComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.languageComboBox.Items.AddRange(new object[] {
																  "C  (Flat Interface)"});
			this.languageComboBox.Location = new System.Drawing.Point(131, 37);
			this.languageComboBox.Name = "languageComboBox";
			this.languageComboBox.Size = new System.Drawing.Size(355, 21);
			this.languageComboBox.TabIndex = 19;
			this.toolTip1.SetToolTip(this.languageComboBox, "Target language. Both C and C++ will generate flat interfaces, but C++ is sometim" +
				"es favored in some projects & environements.");
			this.languageComboBox.SelectedIndexChanged += new System.EventHandler(this.languageComboBox_SelectedIndexChanged);
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(9, 37);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(112, 19);
			this.label2.TabIndex = 20;
			this.label2.Text = "Target Language";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(9, 94);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(112, 18);
			this.label3.TabIndex = 21;
			this.label3.Text = "New Line Format";
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
			this.newLineComboBox.TabIndex = 22;
			this.toolTip1.SetToolTip(this.newLineComboBox, "The type of newline used to generate the code. On UNIX style platforms, using CR+" +
				"LF may lead to a compilable stack that will not function correctly.");
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(9, 121);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(122, 19);
			this.label4.TabIndex = 23;
			this.label4.Text = "Calling Convention";
			// 
			// callConventionComboBox
			// 
			this.callConventionComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.callConventionComboBox.Items.AddRange(new object[] {
																		"None - Compiler default calling convention",
																		"_stdcall - Standard C and C++ calling convention",
																		"_fastcall - Register passing calling convention"});
			this.callConventionComboBox.Location = new System.Drawing.Point(131, 121);
			this.callConventionComboBox.Name = "callConventionComboBox";
			this.callConventionComboBox.Size = new System.Drawing.Size(355, 21);
			this.callConventionComboBox.TabIndex = 24;
			this.toolTip1.SetToolTip(this.callConventionComboBox, "This setting can be used to force all generated methods to be implemented with a " +
				"given calling convention. _fastcall may lead to smaller and faster code.");
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(9, 178);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(122, 18);
			this.label6.TabIndex = 27;
			this.label6.Text = "Code Indention";
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
			this.indentComboBox.TabIndex = 28;
			this.toolTip1.SetToolTip(this.indentComboBox, "Code indentation setting.");
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(9, 205);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(122, 19);
			this.label7.TabIndex = 29;
			this.label7.Text = "C++ Class Name";
			// 
			// classNameTextBox
			// 
			this.classNameTextBox.Enabled = false;
			this.classNameTextBox.Location = new System.Drawing.Point(131, 205);
			this.classNameTextBox.Name = "classNameTextBox";
			this.classNameTextBox.Size = new System.Drawing.Size(355, 20);
			this.classNameTextBox.TabIndex = 30;
			this.classNameTextBox.Text = "MicroStack";
			this.toolTip1.SetToolTip(this.classNameTextBox, "When generating C++ code, the class name used to wrap the generated device stack." +
				"");
			// 
			// libPrefixTextBox
			// 
			this.libPrefixTextBox.Location = new System.Drawing.Point(131, 150);
			this.libPrefixTextBox.Name = "libPrefixTextBox";
			this.libPrefixTextBox.Size = new System.Drawing.Size(355, 20);
			this.libPrefixTextBox.TabIndex = 34;
			this.libPrefixTextBox.Text = "ILib";
			this.toolTip1.SetToolTip(this.libPrefixTextBox, "This string prefixes every generated method of common libraries (Parsers, HTTP, S" +
				"SDP...)");
			// 
			// label8
			// 
			this.label8.Location = new System.Drawing.Point(9, 150);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(122, 19);
			this.label8.TabIndex = 33;
			this.label8.Text = "Library Code Prefix";
			// 
			// modulesCheckedListBox
			// 
			this.modulesCheckedListBox.IntegralHeight = false;
			this.modulesCheckedListBox.Items.AddRange(new object[] {
																	   "Parsing module (ILibParsers.c/.h)",
																	   "HTTP client module (ILibHTTPClient.c/.h)",
																	   "SSDP client module (ILibSSDPClient.c/.h)",
																	   "Async Socket Module (ILibAsyncSocket.c/.h)",
																	   "Async Server Socket Module (ILibAsyncServerSocket.c/.h)",
																	   "HTTP/1.1 Compliant WebClient (ILibWebClient.c/.h)",
																	   "HTTP/1.1 Compliant WebServer (ILibWebServer.c/.h)"});
			this.modulesCheckedListBox.Location = new System.Drawing.Point(9, 253);
			this.modulesCheckedListBox.Name = "modulesCheckedListBox";
			this.modulesCheckedListBox.Size = new System.Drawing.Size(468, 93);
			this.modulesCheckedListBox.TabIndex = 35;
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(9, 234);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(178, 19);
			this.label5.TabIndex = 36;
			this.label5.Text = "Generated Modules";
			// 
			// licenseTextBox
			// 
			this.licenseTextBox.Location = new System.Drawing.Point(18, 383);
			this.licenseTextBox.Multiline = true;
			this.licenseTextBox.Name = "licenseTextBox";
			this.licenseTextBox.Size = new System.Drawing.Size(459, 66);
			this.licenseTextBox.TabIndex = 43;
			this.licenseTextBox.Text = @"/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002, 2003 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code (""Material"") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: <FILE>
 * $Revision: <REVISION>
 * $Author:   <AUTHOR>
 * $Date:     <DATE>
 *
 */";
			this.licenseTextBox.Visible = false;
			// 
			// LibraryGenerationForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(495, 513);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.licenseTextBox,
																		  this.label5,
																		  this.modulesCheckedListBox,
																		  this.libPrefixTextBox,
																		  this.label8,
																		  this.classNameTextBox,
																		  this.label7,
																		  this.indentComboBox,
																		  this.label6,
																		  this.callConventionComboBox,
																		  this.label4,
																		  this.newLineComboBox,
																		  this.label3,
																		  this.languageComboBox,
																		  this.platformComboBox,
																		  this.outputDirectoryButton,
																		  this.label14,
																		  this.genOutputTextBox,
																		  this.generateButton,
																		  this.label10,
																		  this.outputPathTextBox,
																		  this.label1,
																		  this.label2});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "LibraryGenerationForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Module Generation";
			this.ResumeLayout(false);

		}
		#endregion

		private void generateButton_Click(object sender, System.EventArgs e)
		{
			DirectoryInfo outputDir = new DirectoryInfo(outputPathTextBox.Text);
			if (outputDir.Exists == false) 
			{
				MessageBox.Show(this,"Output Path is invalid","Code Generator");
				return;
			}

			// POSIX, Windows, PocketPC Code Generation
			LibraryGenerator libgen = new LibraryGenerator();
			genOutputTextBox.Clear();

			switch (platformComboBox.SelectedIndex) 
			{
				case 0:
					libgen.Platform = LibraryGenerator.PLATFORMS.POSIX;
					libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
					break;
				case 1:
					libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
					libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
					libgen.WinSock = 1;
					break;
				case 2:
					libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
					libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
					libgen.WinSock = 2;
					break;
				case 3:
					libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
					libgen.SubTarget = LibraryGenerator.SUBTARGETS.PPC2003;
					libgen.WinSock = 1;
					break;
			}

			switch (languageComboBox.SelectedIndex) 
			{
				case 0:
					libgen.Language = LibraryGenerator.LANGUAGES.C;
					break;
				case 1:
					libgen.Language = LibraryGenerator.LANGUAGES.CPP;
					break;
			}

			switch (newLineComboBox.SelectedIndex) 
			{
				case 0:
					libgen.CodeNewLine = "\r\n";
					break;
				case 1:
					libgen.CodeNewLine = "\n";
					break;
			}

			switch (callConventionComboBox.SelectedIndex) 
			{
				case 0:
					libgen.CallingConvention = ""; break;
				case 1:
					libgen.CallingConvention = "_stdcall "; break;
				case 2:
					libgen.CallingConvention = "_fastcall "; break;
			}

			switch (indentComboBox.SelectedIndex) 
			{
				case 0:
					libgen.CodeTab = "\t"; break;
				case 1:
					libgen.CodeTab = " "; break;
				case 2:
					libgen.CodeTab = "  "; break;
				case 3:
					libgen.CodeTab = "   "; break;
				case 4:
					libgen.CodeTab = "    "; break;
				case 5:
					libgen.CodeTab = "     "; break;
				case 6:
					libgen.CodeTab = "      "; break;
			}
			
			libgen.CallPrefix = libPrefixTextBox.Text;
			libgen.CallLibPrefix = libPrefixTextBox.Text;
			libgen.VersionString = "Intel DeviceBuilder Build#" + Application.ProductVersion;
			libgen.ClassName = classNameTextBox.Text;

			// Setup License
			string license = licenseTextBox.Text;
			license = license.Replace("<AUTHOR>","Intel Corporation, Intel Device Builder");
			license = license.Replace("<REVISION>","#" + Application.ProductVersion);
			license = license.Replace("<DATE>",DateTime.Now.ToLongDateString());
			libgen.License = license;

			string lib;

			if(modulesCheckedListBox.GetItemCheckState(3)==CheckState.Checked)
			{
				SourceCodeRepository.Generate_AsyncSocket(libPrefixTextBox.Text,outputDir);
			}
			if(modulesCheckedListBox.GetItemCheckState(4)==CheckState.Checked)
			{
				SourceCodeRepository.Generate_AsyncServerSocket(libPrefixTextBox.Text,outputDir);
			}
			if(modulesCheckedListBox.GetItemCheckState(5)==CheckState.Checked)
			{
				SourceCodeRepository.Generate_WebClient(libPrefixTextBox.Text,outputDir,false);
			}
			if(modulesCheckedListBox.GetItemCheckState(6)==CheckState.Checked)
			{
				SourceCodeRepository.Generate_WebServer(libPrefixTextBox.Text,outputDir,false);
			}

			libgen.OnLogOutput += new LibraryGenerator.LogOutputHandler(Log);
			try 
			{
				if (modulesCheckedListBox.GetItemCheckState(0) == CheckState.Checked) 
				{
					SourceCodeRepository.Generate_Parsers(libPrefixTextBox.Text,outputDir);
				}
				if (modulesCheckedListBox.GetItemCheckState(1) == CheckState.Checked) libgen.Build_UPnPHTTPClient(outputDir);
				if (modulesCheckedListBox.GetItemCheckState(2) == CheckState.Checked)
				{
					SourceCodeRepository.Generate_SSDPClient(libPrefixTextBox.Text,outputDir,true);
				}
			} 
			catch
			{
				MessageBox.Show(this,"Error Generating Code","Code Generator");
			}

			libgen.OnLogOutput -= new LibraryGenerator.LogOutputHandler(Log);
		}

		private void Log(object sender, string msg) 
		{
			genOutputTextBox.Text += msg + "\r\n";
		}

		private void outputDirectoryButton_Click(object sender, System.EventArgs e)
		{
			Shell32.Shell shell = new Shell32.ShellClass();
			Shell32.Folder folder = shell.BrowseForFolder(this.Handle.ToInt32(),"Select a directory to share. That directory and all sub-folders will also be made available on the network.",1,null);
			string directoryString = null;
			if (folder != null)
			{
				if (folder.ParentFolder != null)
				{
					for (int i = 0 ; i < folder.ParentFolder.Items().Count ; i++)
					{
						if (folder.ParentFolder.Items().Item(i).IsFolder)
						{
							if (((Shell32.Folder)(folder.ParentFolder.Items().Item(i).GetFolder)).Title == folder.Title)
							{
								directoryString = folder.ParentFolder.Items().Item(i).Path;
								break;
							}
						}
					}
				}

				if (directoryString == null || directoryString.StartsWith("::") == true)
				{
					MessageBox.Show(this,"Invalid folder","Output Folder",MessageBoxButtons.OK,MessageBoxIcon.Warning);
				}
				else
				{
					outputPathTextBox.Text = directoryString;
				}
			}
		}

		private void languageComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			classNameTextBox.Enabled = (languageComboBox.SelectedIndex == 1);
		}

		private void platformComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if (platformComboBox.SelectedIndex == 4) 
			{
				languageComboBox.Enabled = false;
				newLineComboBox.Enabled = false;
				callConventionComboBox.Enabled = false;
				libPrefixTextBox.Enabled = false;
				indentComboBox.Enabled = false;
				classNameTextBox.Enabled = false;
			} 
			else 
			{
				languageComboBox.Enabled = true;
				newLineComboBox.Enabled = true;
				callConventionComboBox.Enabled = true;
				libPrefixTextBox.Enabled = true;
				indentComboBox.Enabled = true;
				classNameTextBox.Enabled = (languageComboBox.SelectedIndex == 1);
			}
		}

	}
}
