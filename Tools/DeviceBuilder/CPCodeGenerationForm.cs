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
	public class CPCodeGenerationForm : System.Windows.Forms.Form
	{
		private System.ComponentModel.IContainer components;

		private UPnPDevice device;
		private Hashtable serviceNames;
		private Hashtable stackSettings;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TextBox libPrefixTextBox;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.TextBox classNameTextBox;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.ComboBox indentComboBox;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox prefixTextBox;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.ComboBox callConventionComboBox;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox newLineComboBox;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox languageComboBox;
		private System.Windows.Forms.ComboBox platformComboBox;
		private System.Windows.Forms.Button outputDirectoryButton;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TextBox outputPathTextBox;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button generateButton;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TextBox genOutputTextBox;
		private System.Windows.Forms.Label label14;
		private System.Windows.Forms.TextBox licenseTextBox;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Data.DataSet dataSet1;
		private System.Data.DataTable dataTable1;
		private System.Data.DataColumn dataColumn1;
		private System.Data.DataColumn dataColumn2;
		private System.Windows.Forms.CheckBox SampleApplication;
		private System.Windows.Forms.CheckBox HTTP;
		private System.Windows.Forms.CheckBox IPAddressMonitor;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.DataGrid dataGrid1;
		private System.Windows.Forms.Label label17;
		private System.Windows.Forms.CheckBox UPnP1dot1Enabled;
		private ArrayList FragResponseActions;
		private ArrayList EscapeActions;

		public CPCodeGenerationForm(UPnPDevice device, Hashtable serviceNames, Hashtable stackSettings, ArrayList FragResponseActions, ArrayList EscapeActions)
		{
			InitializeComponent();

			platformComboBox.SelectedIndex = 0;
			languageComboBox.SelectedIndex = 0;
			newLineComboBox.SelectedIndex = 0;
			indentComboBox.SelectedIndex = 0;
			callConventionComboBox.SelectedIndex = 0;
			this.device = device;
			this.serviceNames = serviceNames;
			this.stackSettings = stackSettings;
			this.FragResponseActions = FragResponseActions;
			this.EscapeActions = EscapeActions;
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
				t["prefix"] = prefixTextBox.Text;				
				t["prefixlib"] = libPrefixTextBox.Text;				
				t["indent"] = indentComboBox.SelectedIndex;			
				t["classname"] = classNameTextBox.Text;
				t["IPAddressMonitor"] = IPAddressMonitor.Checked;
				t["HTTP11Support"] = this.HTTP.Checked;
				t["SupressSample"] = !SampleApplication.Checked;

				Hashtable cf = new Hashtable();
				foreach(System.Data.DataRow r in dataSet1.Tables[0].Rows)
				{
					cf.Add((string)r.ItemArray[0],(string)r.ItemArray[1]);
				}
				t["CustomFields"] = cf;

				return t;
			}
			set
			{
				if (value.Contains("outputpath")) outputPathTextBox.Text = (string)value["outputpath"];
				if (value.Contains("platform")) platformComboBox.SelectedIndex = (int)value["platform"];
				if (value.Contains("language")) languageComboBox.SelectedIndex = (int)value["language"];
				if (value.Contains("newline")) newLineComboBox.SelectedIndex = (int)value["newline"];
				if (value.Contains("callconvention")) callConventionComboBox.SelectedIndex = (int)value["callconvention"];
				if (value.Contains("prefix")) prefixTextBox.Text = (string)value["prefix"];
				if (value.Contains("prefixlib")) libPrefixTextBox.Text = (string)value["prefixlib"];
				if (value.Contains("indent")) indentComboBox.SelectedIndex = (int)value["indent"];
				if (value.Contains("classname")) classNameTextBox.Text = (string)value["classname"];
				if (value.Contains("IPAddressMonitor")) IPAddressMonitor.Checked = (bool)value["IPAddressMonitor"];
				if (value.Contains("HTTP11Support")) HTTP.Checked = (bool)value["HTTP11Support"];				
				if (value.Contains("SupressSample")) {SampleApplication.Checked = !(bool)value["SupressSample"];}	
				
				if (value.Contains("CustomFields"))
				{
					dataSet1.Tables[0].Clear();
					IDictionaryEnumerator e = ((Hashtable)value["CustomFields"]).GetEnumerator();
					while(e.MoveNext())
					{
						dataSet1.Tables[0].Rows.Add(new object[2]{e.Key,e.Value});
					}
				}

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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(CPCodeGenerationForm));
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.libPrefixTextBox = new System.Windows.Forms.TextBox();
			this.classNameTextBox = new System.Windows.Forms.TextBox();
			this.indentComboBox = new System.Windows.Forms.ComboBox();
			this.prefixTextBox = new System.Windows.Forms.TextBox();
			this.callConventionComboBox = new System.Windows.Forms.ComboBox();
			this.newLineComboBox = new System.Windows.Forms.ComboBox();
			this.languageComboBox = new System.Windows.Forms.ComboBox();
			this.platformComboBox = new System.Windows.Forms.ComboBox();
			this.outputPathTextBox = new System.Windows.Forms.TextBox();
			this.UPnP1dot1Enabled = new System.Windows.Forms.CheckBox();
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.generateButton = new System.Windows.Forms.Button();
			this.label8 = new System.Windows.Forms.Label();
			this.label7 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.outputDirectoryButton = new System.Windows.Forms.Button();
			this.label10 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.tabPage3 = new System.Windows.Forms.TabPage();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.label17 = new System.Windows.Forms.Label();
			this.dataGrid1 = new System.Windows.Forms.DataGrid();
			this.dataSet1 = new System.Data.DataSet();
			this.dataTable1 = new System.Data.DataTable();
			this.dataColumn1 = new System.Data.DataColumn();
			this.dataColumn2 = new System.Data.DataColumn();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.SampleApplication = new System.Windows.Forms.CheckBox();
			this.HTTP = new System.Windows.Forms.CheckBox();
			this.IPAddressMonitor = new System.Windows.Forms.CheckBox();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.licenseTextBox = new System.Windows.Forms.TextBox();
			this.label14 = new System.Windows.Forms.Label();
			this.genOutputTextBox = new System.Windows.Forms.TextBox();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage3.SuspendLayout();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.dataGrid1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.dataSet1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.dataTable1)).BeginInit();
			this.groupBox1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.SuspendLayout();
			// 
			// libPrefixTextBox
			// 
			this.libPrefixTextBox.Location = new System.Drawing.Point(131, 178);
			this.libPrefixTextBox.Name = "libPrefixTextBox";
			this.libPrefixTextBox.Size = new System.Drawing.Size(355, 20);
			this.libPrefixTextBox.TabIndex = 51;
			this.libPrefixTextBox.Text = "ILib";
			this.toolTip1.SetToolTip(this.libPrefixTextBox, "This string prefixes every generated method of common libraries (Parsers, HTTP, S" +
				"SDP...)");
			// 
			// classNameTextBox
			// 
			this.classNameTextBox.Enabled = false;
			this.classNameTextBox.Location = new System.Drawing.Point(131, 234);
			this.classNameTextBox.Name = "classNameTextBox";
			this.classNameTextBox.Size = new System.Drawing.Size(355, 20);
			this.classNameTextBox.TabIndex = 49;
			this.classNameTextBox.Text = "Intel.DeviceBuilder";
			this.toolTip1.SetToolTip(this.classNameTextBox, "When generating .net code, the namespace used to wrap the generated control point" +
				" stack.");
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
			this.indentComboBox.Location = new System.Drawing.Point(131, 205);
			this.indentComboBox.Name = "indentComboBox";
			this.indentComboBox.Size = new System.Drawing.Size(355, 21);
			this.indentComboBox.TabIndex = 47;
			this.toolTip1.SetToolTip(this.indentComboBox, "Code indentation setting.");
			// 
			// prefixTextBox
			// 
			this.prefixTextBox.Location = new System.Drawing.Point(131, 150);
			this.prefixTextBox.Name = "prefixTextBox";
			this.prefixTextBox.Size = new System.Drawing.Size(355, 20);
			this.prefixTextBox.TabIndex = 45;
			this.prefixTextBox.Text = "UPnP";
			this.toolTip1.SetToolTip(this.prefixTextBox, "This string prefixes every generated action. It can be used like a namespace in C" +
				".");
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
			this.callConventionComboBox.TabIndex = 43;
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
			this.newLineComboBox.TabIndex = 41;
			this.toolTip1.SetToolTip(this.newLineComboBox, "The type of newline used to generate the code. On UNIX style platforms, using CR+" +
				"LF may lead to a compilable stack that will not function correctly.");
			// 
			// languageComboBox
			// 
			this.languageComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.languageComboBox.Items.AddRange(new object[] {
																  "C  (Flat Interface)"});
			this.languageComboBox.Location = new System.Drawing.Point(131, 37);
			this.languageComboBox.Name = "languageComboBox";
			this.languageComboBox.Size = new System.Drawing.Size(355, 21);
			this.languageComboBox.TabIndex = 38;
			this.toolTip1.SetToolTip(this.languageComboBox, "Target language. Both C and C++ will generate flat interfaces, but C++ is sometim" +
				"es favored in some projects & environements.");
			this.languageComboBox.Click += new System.EventHandler(this.languageComboBox_SelectedIndexChanged);
			// 
			// platformComboBox
			// 
			this.platformComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.platformComboBox.Items.AddRange(new object[] {
																  "Intel C Stack - POSIX, Linux (Standard Sockets)",
																  "Intel C Stack - Windows 95,98,NT,XP (WinSock1)",
																  "Intel C Stack - Windows 98,NT,XP (WinSock2)",
																  "Intel C Stack - PocketPC 2002/2003 (WinSock1)",
																  "Intel .NET Framework Stack (C#)"});
			this.platformComboBox.Location = new System.Drawing.Point(131, 9);
			this.platformComboBox.Name = "platformComboBox";
			this.platformComboBox.Size = new System.Drawing.Size(355, 21);
			this.platformComboBox.TabIndex = 36;
			this.toolTip1.SetToolTip(this.platformComboBox, "Target platform for generated code.");
			this.platformComboBox.SelectedIndexChanged += new System.EventHandler(this.platformComboBox_SelectedIndexChanged);
			// 
			// outputPathTextBox
			// 
			this.outputPathTextBox.Location = new System.Drawing.Point(131, 65);
			this.outputPathTextBox.Name = "outputPathTextBox";
			this.outputPathTextBox.Size = new System.Drawing.Size(315, 20);
			this.outputPathTextBox.TabIndex = 33;
			this.outputPathTextBox.Text = "C:\\Temp2";
			this.toolTip1.SetToolTip(this.outputPathTextBox, "The output path of the generated files. Generated files will overwrite existing f" +
				"iles without prompting.");
			// 
			// UPnP1dot1Enabled
			// 
			this.UPnP1dot1Enabled.Location = new System.Drawing.Point(8, 24);
			this.UPnP1dot1Enabled.Name = "UPnP1dot1Enabled";
			this.UPnP1dot1Enabled.Size = new System.Drawing.Size(448, 16);
			this.UPnP1dot1Enabled.TabIndex = 33;
			this.UPnP1dot1Enabled.Text = "UPnP/1.1 Support (UPnP/1.0 Support if unchecked)";
			this.toolTip1.SetToolTip(this.UPnP1dot1Enabled, "This box will add UPnP/1.1 support to the total code size of the generated stack." +
				"");
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage3);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Location = new System.Drawing.Point(9, 9);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(505, 343);
			this.tabControl1.TabIndex = 33;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.generateButton);
			this.tabPage1.Controls.Add(this.libPrefixTextBox);
			this.tabPage1.Controls.Add(this.label8);
			this.tabPage1.Controls.Add(this.classNameTextBox);
			this.tabPage1.Controls.Add(this.label7);
			this.tabPage1.Controls.Add(this.indentComboBox);
			this.tabPage1.Controls.Add(this.label6);
			this.tabPage1.Controls.Add(this.prefixTextBox);
			this.tabPage1.Controls.Add(this.label5);
			this.tabPage1.Controls.Add(this.callConventionComboBox);
			this.tabPage1.Controls.Add(this.label4);
			this.tabPage1.Controls.Add(this.newLineComboBox);
			this.tabPage1.Controls.Add(this.label3);
			this.tabPage1.Controls.Add(this.languageComboBox);
			this.tabPage1.Controls.Add(this.platformComboBox);
			this.tabPage1.Controls.Add(this.outputDirectoryButton);
			this.tabPage1.Controls.Add(this.label10);
			this.tabPage1.Controls.Add(this.outputPathTextBox);
			this.tabPage1.Controls.Add(this.label1);
			this.tabPage1.Controls.Add(this.label2);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Size = new System.Drawing.Size(497, 317);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "General";
			// 
			// generateButton
			// 
			this.generateButton.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.generateButton.Location = new System.Drawing.Point(0, 279);
			this.generateButton.Name = "generateButton";
			this.generateButton.Size = new System.Drawing.Size(497, 38);
			this.generateButton.TabIndex = 52;
			this.generateButton.Text = "Click Here To Generate a Control Point Stack";
			this.generateButton.Click += new System.EventHandler(this.generateButton_Click);
			// 
			// label8
			// 
			this.label8.Location = new System.Drawing.Point(9, 182);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(122, 14);
			this.label8.TabIndex = 50;
			this.label8.Text = "Library Code Prefix";
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(9, 238);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(122, 14);
			this.label7.TabIndex = 48;
			this.label7.Text = "Namespace";
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(9, 209);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(122, 19);
			this.label6.TabIndex = 46;
			this.label6.Text = "Code Indention";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(9, 154);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(122, 18);
			this.label5.TabIndex = 44;
			this.label5.Text = "Code Prefix";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(9, 125);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(122, 15);
			this.label4.TabIndex = 42;
			this.label4.Text = "Calling Convention";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(9, 98);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(112, 18);
			this.label3.TabIndex = 40;
			this.label3.Text = "New Line Format";
			// 
			// outputDirectoryButton
			// 
			this.outputDirectoryButton.Image = ((System.Drawing.Image)(resources.GetObject("outputDirectoryButton.Image")));
			this.outputDirectoryButton.Location = new System.Drawing.Point(455, 65);
			this.outputDirectoryButton.Name = "outputDirectoryButton";
			this.outputDirectoryButton.Size = new System.Drawing.Size(29, 25);
			this.outputDirectoryButton.TabIndex = 35;
			this.outputDirectoryButton.Click += new System.EventHandler(this.outputDirectoryButton_Click);
			// 
			// label10
			// 
			this.label10.Location = new System.Drawing.Point(9, 69);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(75, 15);
			this.label10.TabIndex = 34;
			this.label10.Text = "Output Path";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(9, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(103, 15);
			this.label1.TabIndex = 37;
			this.label1.Text = "Target Platform";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(9, 41);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(112, 19);
			this.label2.TabIndex = 39;
			this.label2.Text = "Target Language";
			// 
			// tabPage3
			// 
			this.tabPage3.Controls.Add(this.groupBox2);
			this.tabPage3.Controls.Add(this.groupBox1);
			this.tabPage3.Location = new System.Drawing.Point(4, 22);
			this.tabPage3.Name = "tabPage3";
			this.tabPage3.Size = new System.Drawing.Size(497, 317);
			this.tabPage3.TabIndex = 2;
			this.tabPage3.Text = "Advanced";
			this.tabPage3.Click += new System.EventHandler(this.tabPage3_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.label17);
			this.groupBox2.Controls.Add(this.dataGrid1);
			this.groupBox2.Location = new System.Drawing.Point(8, 112);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(480, 200);
			this.groupBox2.TabIndex = 5;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Custom Tags";
			// 
			// label17
			// 
			this.label17.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.label17.Location = new System.Drawing.Point(8, 16);
			this.label17.Name = "label17";
			this.label17.Size = new System.Drawing.Size(464, 40);
			this.label17.TabIndex = 6;
			this.label17.Text = @"This tab is used to add custom XML tag support to a control point stack. For example: Adding ""X_DLNADOC"" to the FieldName and ""urn:schemas-dlna.org:device-1-0"" to the FieldNameSpace will add support for detecting if such a tag exists in discovered devices.";
			// 
			// dataGrid1
			// 
			this.dataGrid1.CaptionVisible = false;
			this.dataGrid1.DataMember = "Table1";
			this.dataGrid1.DataSource = this.dataSet1;
			this.dataGrid1.HeaderForeColor = System.Drawing.SystemColors.ControlText;
			this.dataGrid1.Location = new System.Drawing.Point(8, 64);
			this.dataGrid1.Name = "dataGrid1";
			this.dataGrid1.PreferredColumnWidth = 210;
			this.dataGrid1.Size = new System.Drawing.Size(464, 128);
			this.dataGrid1.TabIndex = 5;
			// 
			// dataSet1
			// 
			this.dataSet1.DataSetName = "NewDataSet";
			this.dataSet1.Locale = new System.Globalization.CultureInfo("en-US");
			this.dataSet1.Tables.AddRange(new System.Data.DataTable[] {
																		  this.dataTable1});
			// 
			// dataTable1
			// 
			this.dataTable1.Columns.AddRange(new System.Data.DataColumn[] {
																			  this.dataColumn1,
																			  this.dataColumn2});
			this.dataTable1.TableName = "Table1";
			// 
			// dataColumn1
			// 
			this.dataColumn1.AllowDBNull = false;
			this.dataColumn1.ColumnName = "FieldName";
			this.dataColumn1.DefaultValue = "";
			// 
			// dataColumn2
			// 
			this.dataColumn2.AllowDBNull = false;
			this.dataColumn2.ColumnName = "FieldNameSpace";
			this.dataColumn2.DefaultValue = "";
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.UPnP1dot1Enabled);
			this.groupBox1.Controls.Add(this.SampleApplication);
			this.groupBox1.Controls.Add(this.HTTP);
			this.groupBox1.Controls.Add(this.IPAddressMonitor);
			this.groupBox1.Location = new System.Drawing.Point(8, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(480, 96);
			this.groupBox1.TabIndex = 3;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Control Point Setup";
			// 
			// SampleApplication
			// 
			this.SampleApplication.Checked = true;
			this.SampleApplication.CheckState = System.Windows.Forms.CheckState.Checked;
			this.SampleApplication.Location = new System.Drawing.Point(8, 72);
			this.SampleApplication.Name = "SampleApplication";
			this.SampleApplication.Size = new System.Drawing.Size(464, 16);
			this.SampleApplication.TabIndex = 5;
			this.SampleApplication.Text = "Generate Sample Application";
			// 
			// HTTP
			// 
			this.HTTP.Location = new System.Drawing.Point(8, 40);
			this.HTTP.Name = "HTTP";
			this.HTTP.Size = new System.Drawing.Size(464, 16);
			this.HTTP.TabIndex = 4;
			this.HTTP.Text = "HTTP/1.1 Support (HTTP/1.0 Support if unchecked)";
			// 
			// IPAddressMonitor
			// 
			this.IPAddressMonitor.Location = new System.Drawing.Point(8, 56);
			this.IPAddressMonitor.Name = "IPAddressMonitor";
			this.IPAddressMonitor.Size = new System.Drawing.Size(464, 16);
			this.IPAddressMonitor.TabIndex = 3;
			this.IPAddressMonitor.Text = "Add default network interface monitoring code to sample application";
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.Add(this.licenseTextBox);
			this.tabPage2.Controls.Add(this.label14);
			this.tabPage2.Controls.Add(this.genOutputTextBox);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Size = new System.Drawing.Size(497, 317);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Generator Log";
			// 
			// licenseTextBox
			// 
			this.licenseTextBox.Location = new System.Drawing.Point(18, 37);
			this.licenseTextBox.Multiline = true;
			this.licenseTextBox.Name = "licenseTextBox";
			this.licenseTextBox.Size = new System.Drawing.Size(459, 253);
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
			// label14
			// 
			this.label14.Location = new System.Drawing.Point(9, 9);
			this.label14.Name = "label14";
			this.label14.Size = new System.Drawing.Size(178, 19);
			this.label14.TabIndex = 16;
			this.label14.Text = "Code Generation Output";
			// 
			// genOutputTextBox
			// 
			this.genOutputTextBox.Location = new System.Drawing.Point(9, 28);
			this.genOutputTextBox.Multiline = true;
			this.genOutputTextBox.Name = "genOutputTextBox";
			this.genOutputTextBox.ReadOnly = true;
			this.genOutputTextBox.Size = new System.Drawing.Size(477, 271);
			this.genOutputTextBox.TabIndex = 13;
			this.genOutputTextBox.Text = "";
			// 
			// CPCodeGenerationForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(520, 356);
			this.Controls.Add(this.tabControl1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.Name = "CPCodeGenerationForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Control Point Stack Code Generation";
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage3.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.dataGrid1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.dataSet1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.dataTable1)).EndInit();
			this.groupBox1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		public void generateButton_Click(object sender, System.EventArgs e)
		{
			DirectoryInfo outputDir = new DirectoryInfo(outputPathTextBox.Text);
			if (outputDir.Exists == false) 
			{
				MessageBox.Show(this,"Output Path is invalid","Code Generator");
				return;
			}

			string buttonText = generateButton.Text;
			generateButton.Text = "Generating Stack...";
			generateButton.Enabled = false;

			if(UPnP1dot1Enabled.Checked==true)
			{
				device.ArchitectureVersion = "1.1";
			}
			else
			{
				device.ArchitectureVersion = "1.0";
			}

			if(platformComboBox.SelectedIndex==4)
			{
				// .net
				CPDotNetGenerator gen = new CPDotNetGenerator(classNameTextBox.Text);
				genOutputTextBox.Clear();

				gen.VersionString = "Intel StackBuilder Build#" + Application.ProductVersion;
				gen.StartupPath = Application.StartupPath;
				gen.OnLogOutput += new CPDotNetGenerator.LogOutputHandler(Log);
				try 
				{
					gen.Generate(device,outputDir,serviceNames);
				} 
				catch
				{
					MessageBox.Show(this,"Error Generating Code","Code Generator");
				}
				gen.OnLogOutput -= new CPDotNetGenerator.LogOutputHandler(Log);
			}
			else
			{
				CPEmbeddedCGenerator gen = new CPEmbeddedCGenerator();
				LibraryGenerator libgen = new LibraryGenerator();

				genOutputTextBox.Clear();

				gen.EnableDefaultIPAddressMonitor = IPAddressMonitor.Checked;

				switch (platformComboBox.SelectedIndex) 
				{
					case 0:
						gen.Platform = CPEmbeddedCGenerator.PLATFORMS.POSIX;
						gen.SubTarget = CPEmbeddedCGenerator.SUBTARGETS.NONE;
						libgen.Platform = LibraryGenerator.PLATFORMS.POSIX;
						libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
						break;
					case 1:
						gen.Platform = CPEmbeddedCGenerator.PLATFORMS.WINDOWS;
						gen.SubTarget = CPEmbeddedCGenerator.SUBTARGETS.NONE;
						gen.WinSock = 1;
						libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
						libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
						libgen.WinSock = 1;
						break;
					case 2:
						gen.Platform = CPEmbeddedCGenerator.PLATFORMS.WINDOWS;
						gen.SubTarget = CPEmbeddedCGenerator.SUBTARGETS.NONE;
						gen.WinSock = 2;
						libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
						libgen.SubTarget = LibraryGenerator.SUBTARGETS.NONE;
						libgen.WinSock = 2;
						break;
					case 3:
						gen.Platform = CPEmbeddedCGenerator.PLATFORMS.WINDOWS;
						gen.SubTarget = CPEmbeddedCGenerator.SUBTARGETS.PPC2003;
						gen.WinSock = 1;
						libgen.Platform = LibraryGenerator.PLATFORMS.WINDOWS;
						libgen.SubTarget = LibraryGenerator.SUBTARGETS.PPC2003;
						libgen.WinSock = 1;
						break;
					case 4:
						gen.Platform = CPEmbeddedCGenerator.PLATFORMS.POSIX;
						gen.SubTarget = CPEmbeddedCGenerator.SUBTARGETS.NUCLEUS;
						gen.WinSock = 1;
						libgen.Platform = LibraryGenerator.PLATFORMS.POSIX;
						libgen.SubTarget = LibraryGenerator.SUBTARGETS.NUCLEUS;
						libgen.WinSock = 1;
						break;
				}

				switch (languageComboBox.SelectedIndex) 
				{
					case 0:
						gen.Language = CPEmbeddedCGenerator.LANGUAGES.C;
						libgen.Language = LibraryGenerator.LANGUAGES.C;
						break;
					case 1:
						gen.Language = CPEmbeddedCGenerator.LANGUAGES.CPP;
						libgen.Language = LibraryGenerator.LANGUAGES.CPP;
						break;
				}

				switch (newLineComboBox.SelectedIndex) 
				{
					case 0:
						gen.CodeNewLine = "\r\n"; break;
					case 1:
						gen.CodeNewLine = "\n"; break;
				}

				switch (callConventionComboBox.SelectedIndex) 
				{
					case 0:
						gen.CallingConvention = ""; break;
					case 1:
						gen.CallingConvention = "_stdcall "; break;
					case 2:
						gen.CallingConvention = "_fastcall "; break;
				}

				switch (indentComboBox.SelectedIndex) 
				{
					case 0:
						gen.CodeTab = "\t"; break;
					case 1:
						gen.CodeTab = " "; break;
					case 2:
						gen.CodeTab = "  "; break;
					case 3:
						gen.CodeTab = "   "; break;
					case 4:
						gen.CodeTab = "    "; break;
					case 5:
						gen.CodeTab = "     "; break;
					case 6:
						gen.CodeTab = "      "; break;
				}

				gen.CallPrefix = prefixTextBox.Text;
				gen.CallLibPrefix = libPrefixTextBox.Text;
				gen.Settings = Settings;
				gen.FragResponseActions = FragResponseActions;
				gen.EscapeActions = EscapeActions;
				gen.VersionString = "Intel DeviceBuilder Build#" + Application.ProductVersion;
				gen.ClassName = classNameTextBox.Text;
				gen.UseVersion = Application.ProductVersion.Substring(0,Application.ProductVersion.LastIndexOf("."));
				gen.BasicHTTP = !(HTTP.Checked);

				libgen.CodeNewLine = gen.CodeNewLine;
				libgen.CallingConvention = gen.CallingConvention;
				libgen.CodeTab = gen.CodeTab;
				libgen.CodeNewLine = gen.CodeNewLine;
				libgen.CallPrefix = libPrefixTextBox.Text;
				libgen.VersionString = gen.VersionString;
				libgen.ClassName = gen.ClassName;

				foreach(System.Data.DataRow r in dataSet1.Tables[0].Rows)
				{
					gen.CustomTagList.Add(new object[2]{(string)r.ItemArray[0],(string)r.ItemArray[1]});
				}


				// Setup License
				string license = licenseTextBox.Text;
				license = license.Replace("<AUTHOR>","Intel Corporation, Intel Device Builder");
				license = license.Replace("<REVISION>","#" + Application.ProductVersion);
				license = license.Replace("<DATE>",DateTime.Now.ToLongDateString());
				gen.License = license;
				libgen.License = license;

				gen.OnLogOutput += new CPEmbeddedCGenerator.LogOutputHandler(Log);
				libgen.OnLogOutput += new LibraryGenerator.LogOutputHandler(Log);

				try
				{
					SourceCodeRepository.Generate_UPnPControlPointStructs(libPrefixTextBox.Text,outputDir);
					SourceCodeRepository.Generate_Parsers(this.libPrefixTextBox.Text,outputDir);
					SourceCodeRepository.Generate_SSDPClient(this.libPrefixTextBox.Text,outputDir,UPnP1dot1Enabled.Checked);

					SourceCodeRepository.Generate_AsyncSocket(this.libPrefixTextBox.Text,outputDir);
					SourceCodeRepository.Generate_AsyncServerSocket(this.libPrefixTextBox.Text,outputDir);
					SourceCodeRepository.Generate_WebClient(this.libPrefixTextBox.Text,outputDir,!HTTP.Checked);
					SourceCodeRepository.Generate_WebServer(this.libPrefixTextBox.Text,outputDir,!HTTP.Checked);
					
					gen.Generate(device,outputDir,serviceNames,!(bool)gen.Settings["SupressSample"]);
				} 
				catch(Exception ddd)
				{
					MessageBox.Show(this,"Error Generating Code","Code Generator");
				}

				libgen.OnLogOutput -= new LibraryGenerator.LogOutputHandler(Log);
				gen.OnLogOutput -= new CPEmbeddedCGenerator.LogOutputHandler(Log);
			}

			generateButton.Enabled = true;
			generateButton.Text = buttonText;
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

		private void tabPage3_Click(object sender, System.EventArgs e)
		{
		
		}

		private void platformComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if(platformComboBox.SelectedIndex==4)
			{
				// .net
				this.languageComboBox.Enabled = false;
				this.newLineComboBox.Enabled = false;
				this.callConventionComboBox.Enabled = false;
				this.prefixTextBox.Enabled = false;
				this.libPrefixTextBox.Enabled = false;
				this.indentComboBox.Enabled = false;
				this.classNameTextBox.Enabled = true;
			}
			else
			{
				this.languageComboBox.Enabled = true;
				this.newLineComboBox.Enabled = true;
				this.callConventionComboBox.Enabled = true;
				this.prefixTextBox.Enabled = true;
				this.libPrefixTextBox.Enabled = true;
				this.indentComboBox.Enabled = true;
				this.classNameTextBox.Enabled = false;
			}
		}
	}
}
