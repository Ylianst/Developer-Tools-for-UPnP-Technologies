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
using System.Net;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace UPnPSniffer
{
    /// <summary>
    /// Summary description for GetForm.
    /// </summary>
    public class HttpRequestor : System.Windows.Forms.Form
    {

        private Queue RequestQ = new Queue();

        private HTTPSession s;
        private HTTPSessionWatcher SW;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TextBox SniffText;
        private System.Windows.Forms.TextBox GetText;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.ComboBox MethodBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button RequestButton;
        private System.Windows.Forms.TextBox addressTextBox;
        private System.Windows.Forms.MainMenu mainMenu1;
        private System.Windows.Forms.ContextMenu networkContextMenu;
        private System.Windows.Forms.MenuItem networkSeparatorMenuItem;
        private System.Windows.Forms.MenuItem networkCopyMenuItem;
        private System.Windows.Forms.Panel customRequestPanel;
        private System.Windows.Forms.ComboBox versionComboBox;
        private System.Windows.Forms.TextBox fieldsTextBox;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox directiveTextBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox directiveObjTextBox;
        private System.Windows.Forms.TextBox bodyTextBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ContextMenu requestContextMenu;
        private System.Windows.Forms.MenuItem RequestQueueMenuItem;

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public HttpRequestor()
        {
            InitializeComponent();
            MethodBox.SelectedIndex = 0;
            versionComboBox.SelectedIndex = 1;
        }

        private void Request(object msg, IPEndPoint dest)
        {
            if (dest == null) return;
            s = new HTTPSession(new IPEndPoint(IPAddress.Any, 0), dest,
                new HTTPSession.SessionHandler(CreateSink),
                new HTTPSession.SessionHandler(FailSink),
                msg);
        }

        public string Address
        {
            get { return addressTextBox.Text; }
        }

        private HTTPMessage Request(Uri GetUri, bool isGetRequest, bool isHttp1_1)
        {
            directiveObjTextBox.Text = GetUri.PathAndQuery;

            IPAddress Addr = null;
            try
            {
                if (GetUri.HostNameType == UriHostNameType.Dns)
                {
                    Addr = Dns.GetHostEntry(GetUri.Host).AddressList[0];
                }
                else
                {
                    Addr = IPAddress.Parse(GetUri.Host);
                }
            }
            catch (Exception)
            {
                return null;
            }
            if (Addr == null) return null;

            IPEndPoint ep = new IPEndPoint(Addr, GetUri.Port);

            // NKIDD - ADDED support for requesting GET/HEAD 1.0/1.1

            HTTPMessage req = null;
            if (isHttp1_1)
            {
                req = new HTTPMessage();
            }
            else
            {
                req = new HTTPMessage("1.0");
            }

            if (isGetRequest)
            {
                req.Directive = "GET";
            }
            else
            {
                req.Directive = "HEAD";
            }

            req.DirectiveObj = HTTPMessage.UnEscapeString(GetUri.PathAndQuery);
            req.AddTag("Host", ep.ToString());

            this.Text = "Device Sniffer - " + GetUri.AbsoluteUri;

            SniffText.Text = "";
            GetText.Text = "";

            return (req);
        }

        private void FailSink(HTTPSession FS)
        {
            SniffText.Text = "Could not connect to resouce.";
            GetText.Text = "Could not connect to resouce.";
        }

        private void CloseSink(HTTPSession CS)
        {

        }
        private void CreateSink(HTTPSession CS)
        {
            CS.OnClosed += new HTTPSession.SessionHandler(CloseSink);

            SW = new HTTPSessionWatcher(CS);
            SW.OnSniff += new HTTPSessionWatcher.SniffHandler(SniffSink);
            CS.OnReceive += new HTTPSession.ReceiveHandler(ReceiveSink);
            if (CS.StateObject.GetType() == typeof(HTTPMessage))
            {
                CS.Send((HTTPMessage)CS.StateObject);
            }
            else
            {
                Queue Q = (Queue)CS.StateObject;
                HTTPMessage M;
                M = (HTTPMessage)Q.Dequeue();
                while (M != null && Q.Count > 0)
                {
                    CS.Send(M);
                    M = (HTTPMessage)Q.Dequeue();
                }
            }
        }

        private void SniffSink(byte[] raw, int offset, int length)
        {
            UTF8Encoding U = new UTF8Encoding();
            SniffText.Text = SniffText.Text + U.GetString(raw, offset, length);
        }

        private void ReceiveSink(HTTPSession sender, HTTPMessage msg)
        {
            GetText.Text = msg.StringPacket;
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        private void RequestButton_Click(object sender, System.EventArgs e)
        {
            if (RequestButton.Text == "Request")
            {
                if (RequestQ.Count == 0)
                {
                    Request(Request(), GetIPEndPoint());
                }
                else
                {
                    Request(RequestQ, GetIPEndPoint());
                    RequestQ = new Queue();
                }
            }
            else
            {
                RequestQ.Enqueue(Request());
            }
        }

        public void Request(string address)
        {
            addressTextBox.Text = address;
            try
            {
                directiveObjTextBox.Text = new Uri(address).PathAndQuery;
            }
            catch { }
            Request();
        }

        private IPEndPoint GetIPEndPoint()
        {
            IPAddress Addr;
            Uri uri;
            try
            {
                uri = new Uri(addressTextBox.Text);

                if (uri.HostNameType == UriHostNameType.Dns)
                {
                    Addr = Dns.GetHostEntry(uri.Host).AddressList[0];
                }
                else
                {
                    Addr = IPAddress.Parse(uri.Host);
                }
            }
            catch
            {
                SniffText.Text = "Invalid Request URI";
                GetText.Text = "Invalid Request URI";
                return null;
            }
            return new IPEndPoint(Addr, uri.Port);
        }

        private HTTPMessage Request()
        {
            HTTPMessage RetVal = null;
            if (this.Visible == false)
            {
                Show();
            }
            else
            {
                Activate();
            }

            Uri uri;
            try
            {
                uri = new Uri(addressTextBox.Text);
            }
            catch
            {
                SniffText.Text = "Invalid Request URI";
                GetText.Text = "Invalid Request URI";
                return (null);
            }

            SniffText.Text = "";
            GetText.Text = "";

            if (MethodBox.SelectedIndex == 0)
            {
                // GET 1.0
                RetVal = Request(uri, true, false);
            }
            else if (MethodBox.SelectedIndex == 1)
            {
                // GET 1.1
                RetVal = Request(uri, true, true);
            }
            else if (MethodBox.SelectedIndex == 2)
            {
                // HEAD 1.0
                RetVal = Request(uri, false, false);
            }
            else if (MethodBox.SelectedIndex == 3)
            {
                // HEAD 1.1
                RetVal = Request(uri, false, true);
            }
            else if (MethodBox.SelectedIndex == 4)
            {
                HTTPMessage msg = new HTTPMessage();
                msg.Directive = directiveTextBox.Text;
                msg.DirectiveObj = directiveObjTextBox.Text;

                if (versionComboBox.SelectedIndex == 0)
                {
                    msg.Version = "0.9";
                }
                if (versionComboBox.SelectedIndex == 1)
                {
                    msg.Version = "1.0";
                }
                if (versionComboBox.SelectedIndex == 2)
                {
                    msg.Version = "1.1";
                }

                try
                {
                    bool addreturn = false;
                    if (fieldsTextBox.Text.EndsWith("\r\n") == false)
                    {
                        fieldsTextBox.Text += "\r\n";
                        addreturn = true;
                    }
                    int pos = 0;
                    while (pos != -1)
                    {
                        int s1 = fieldsTextBox.Text.IndexOf(":", pos);
                        int s2 = fieldsTextBox.Text.IndexOf("\r\n", pos);
                        if (s1 == -1 || s2 == -1) break;
                        msg.AppendTag(fieldsTextBox.Text.Substring(pos, s1 - pos), fieldsTextBox.Text.Substring(s1 + 1, s2 - s1 - 1));
                        pos = s2 + 2;
                    }
                    if (addreturn == true)
                    {
                        fieldsTextBox.Text = fieldsTextBox.Text.Substring(0, fieldsTextBox.Text.Length - 2);
                    }
                }
                catch { }
                msg.StringBuffer = bodyTextBox.Text;

                RetVal = msg;
            }

            if (RetVal == null)
            {
                SniffText.Text = "Invalid Request URI";
                GetText.Text = "Invalid Request URI";
            }

            return (RetVal);
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(HttpRequestor));
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.SniffText = new System.Windows.Forms.TextBox();
            this.networkContextMenu = new System.Windows.Forms.ContextMenu();
            this.networkCopyMenuItem = new System.Windows.Forms.MenuItem();
            this.networkSeparatorMenuItem = new System.Windows.Forms.MenuItem();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.GetText = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.RequestButton = new System.Windows.Forms.Button();
            this.requestContextMenu = new System.Windows.Forms.ContextMenu();
            this.RequestQueueMenuItem = new System.Windows.Forms.MenuItem();
            this.addressTextBox = new System.Windows.Forms.TextBox();
            this.MethodBox = new System.Windows.Forms.ComboBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.mainMenu1 = new System.Windows.Forms.MainMenu();
            this.customRequestPanel = new System.Windows.Forms.Panel();
            this.label6 = new System.Windows.Forms.Label();
            this.bodyTextBox = new System.Windows.Forms.TextBox();
            this.versionComboBox = new System.Windows.Forms.ComboBox();
            this.fieldsTextBox = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.directiveObjTextBox = new System.Windows.Forms.TextBox();
            this.directiveTextBox = new System.Windows.Forms.TextBox();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.customRequestPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Alignment = System.Windows.Forms.TabAlignment.Bottom;
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 248);
            this.tabControl1.Multiline = true;
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(488, 166);
            this.tabControl1.TabIndex = 2;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.SniffText);
            this.tabPage1.Location = new System.Drawing.Point(4, 4);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(480, 140);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Network Data";
            // 
            // SniffText
            // 
            this.SniffText.ContextMenu = this.networkContextMenu;
            this.SniffText.Dock = System.Windows.Forms.DockStyle.Fill;
            this.SniffText.Location = new System.Drawing.Point(0, 0);
            this.SniffText.Multiline = true;
            this.SniffText.Name = "SniffText";
            this.SniffText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.SniffText.Size = new System.Drawing.Size(480, 140);
            this.SniffText.TabIndex = 1;
            this.SniffText.Text = "";
            // 
            // networkContextMenu
            // 
            this.networkContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							   this.networkCopyMenuItem,
																							   this.networkSeparatorMenuItem});
            this.networkContextMenu.Popup += new System.EventHandler(this.networkContextMenu_Popup);
            // 
            // networkCopyMenuItem
            // 
            this.networkCopyMenuItem.Index = 0;
            this.networkCopyMenuItem.Text = "&Copy to Clipboard";
            this.networkCopyMenuItem.Click += new System.EventHandler(this.networkCopyMenuItem_Click);
            // 
            // networkSeparatorMenuItem
            // 
            this.networkSeparatorMenuItem.Index = 1;
            this.networkSeparatorMenuItem.Text = "-";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.GetText);
            this.tabPage2.Location = new System.Drawing.Point(4, 4);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Size = new System.Drawing.Size(480, 140);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Parsed Data";
            // 
            // GetText
            // 
            this.GetText.Dock = System.Windows.Forms.DockStyle.Fill;
            this.GetText.Location = new System.Drawing.Point(0, 0);
            this.GetText.Multiline = true;
            this.GetText.Name = "GetText";
            this.GetText.ReadOnly = true;
            this.GetText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.GetText.Size = new System.Drawing.Size(480, 140);
            this.GetText.TabIndex = 2;
            this.GetText.Text = "";
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add(this.label2);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Controls.Add(this.RequestButton);
            this.panel1.Controls.Add(this.addressTextBox);
            this.panel1.Controls.Add(this.MethodBox);
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(488, 53);
            this.panel1.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(48, 30);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(64, 16);
            this.label2.TabIndex = 26;
            this.label2.Text = "Address";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(48, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 16);
            this.label1.TabIndex = 25;
            this.label1.Text = "Request Type";
            // 
            // RequestButton
            // 
            this.RequestButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.RequestButton.ContextMenu = this.requestContextMenu;
            this.RequestButton.Location = new System.Drawing.Point(412, 2);
            this.RequestButton.Name = "RequestButton";
            this.RequestButton.Size = new System.Drawing.Size(64, 20);
            this.RequestButton.TabIndex = 24;
            this.RequestButton.Text = "Request";
            this.RequestButton.Click += new System.EventHandler(this.RequestButton_Click);
            // 
            // requestContextMenu
            // 
            this.requestContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							   this.RequestQueueMenuItem});
            // 
            // RequestQueueMenuItem
            // 
            this.RequestQueueMenuItem.Index = 0;
            this.RequestQueueMenuItem.Text = "Toggle Mode";
            this.RequestQueueMenuItem.Click += new System.EventHandler(this.RequestQueueMenuItem_Click);
            // 
            // addressTextBox
            // 
            this.addressTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.addressTextBox.Location = new System.Drawing.Point(128, 26);
            this.addressTextBox.Name = "addressTextBox";
            this.addressTextBox.Size = new System.Drawing.Size(348, 20);
            this.addressTextBox.TabIndex = 23;
            this.addressTextBox.Text = "http://";
            this.addressTextBox.TextChanged += new System.EventHandler(this.addressTextBox_TextChanged);
            // 
            // MethodBox
            // 
            this.MethodBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.MethodBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.MethodBox.Items.AddRange(new object[] {
														   "HTTP/1.0 GET",
														   "HTTP/1.1 GET",
														   "HTTP/1.0 HEAD",
														   "HTTP/1.1 HEAD",
														   "CUSTOM HTTP"});
            this.MethodBox.Location = new System.Drawing.Point(128, 2);
            this.MethodBox.Name = "MethodBox";
            this.MethodBox.Size = new System.Drawing.Size(276, 21);
            this.MethodBox.TabIndex = 22;
            this.MethodBox.SelectedIndexChanged += new System.EventHandler(this.MethodBox_SelectedIndexChanged);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(9, 13);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(32, 32);
            this.pictureBox1.TabIndex = 21;
            this.pictureBox1.TabStop = false;
            // 
            // customRequestPanel
            // 
            this.customRequestPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.customRequestPanel.Controls.Add(this.label6);
            this.customRequestPanel.Controls.Add(this.bodyTextBox);
            this.customRequestPanel.Controls.Add(this.versionComboBox);
            this.customRequestPanel.Controls.Add(this.fieldsTextBox);
            this.customRequestPanel.Controls.Add(this.label8);
            this.customRequestPanel.Controls.Add(this.label5);
            this.customRequestPanel.Controls.Add(this.label4);
            this.customRequestPanel.Controls.Add(this.label3);
            this.customRequestPanel.Controls.Add(this.directiveObjTextBox);
            this.customRequestPanel.Controls.Add(this.directiveTextBox);
            this.customRequestPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.customRequestPanel.Location = new System.Drawing.Point(0, 53);
            this.customRequestPanel.Name = "customRequestPanel";
            this.customRequestPanel.Size = new System.Drawing.Size(488, 195);
            this.customRequestPanel.TabIndex = 5;
            this.customRequestPanel.Visible = false;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(8, 114);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(100, 14);
            this.label6.TabIndex = 21;
            this.label6.Text = "Body";
            // 
            // bodyTextBox
            // 
            this.bodyTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.bodyTextBox.Location = new System.Drawing.Point(8, 128);
            this.bodyTextBox.Multiline = true;
            this.bodyTextBox.Name = "bodyTextBox";
            this.bodyTextBox.Size = new System.Drawing.Size(468, 56);
            this.bodyTextBox.TabIndex = 20;
            this.bodyTextBox.Text = "";
            // 
            // versionComboBox
            // 
            this.versionComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.versionComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.versionComboBox.Items.AddRange(new object[] {
																 "HTTP/0.9",
																 "HTTP/1.0",
																 "HTTP/1.1"});
            this.versionComboBox.Location = new System.Drawing.Point(384, 24);
            this.versionComboBox.Name = "versionComboBox";
            this.versionComboBox.Size = new System.Drawing.Size(96, 21);
            this.versionComboBox.TabIndex = 19;
            // 
            // fieldsTextBox
            // 
            this.fieldsTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.fieldsTextBox.Location = new System.Drawing.Point(8, 64);
            this.fieldsTextBox.Multiline = true;
            this.fieldsTextBox.Name = "fieldsTextBox";
            this.fieldsTextBox.Size = new System.Drawing.Size(468, 48);
            this.fieldsTextBox.TabIndex = 18;
            this.fieldsTextBox.Text = "";
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(8, 48);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(96, 16);
            this.label8.TabIndex = 17;
            this.label8.Text = "Fields";
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.Location = new System.Drawing.Point(380, 8);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(88, 16);
            this.label5.TabIndex = 11;
            this.label5.Text = "Version";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(80, 8);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(88, 16);
            this.label4.TabIndex = 10;
            this.label4.Text = "Directive Object";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 8);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(56, 16);
            this.label3.TabIndex = 9;
            this.label3.Text = "Directive";
            // 
            // directiveObjTextBox
            // 
            this.directiveObjTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right)));
            this.directiveObjTextBox.Location = new System.Drawing.Point(80, 24);
            this.directiveObjTextBox.Name = "directiveObjTextBox";
            this.directiveObjTextBox.ReadOnly = true;
            this.directiveObjTextBox.Size = new System.Drawing.Size(292, 20);
            this.directiveObjTextBox.TabIndex = 7;
            this.directiveObjTextBox.Text = "/";
            this.directiveObjTextBox.DoubleClick += new System.EventHandler(this.OnDblClick_DirectiveObject);
            // 
            // directiveTextBox
            // 
            this.directiveTextBox.Location = new System.Drawing.Point(8, 24);
            this.directiveTextBox.Name = "directiveTextBox";
            this.directiveTextBox.Size = new System.Drawing.Size(64, 20);
            this.directiveTextBox.TabIndex = 6;
            this.directiveTextBox.Text = "GET";
            // 
            // HttpRequestor
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(488, 414);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.customRequestPanel);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu1;
            this.Name = "HttpRequestor";
            this.Text = "Device Sniffer - HTTP Requestor";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.HttpRequestor_Closing);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.customRequestPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private void HttpRequestor_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            e.Cancel = true;
            this.Hide();
        }

        private void WebPageOpenMenuSink(object sender, System.EventArgs arg)
        {
            MenuItem mi = (MenuItem)sender;
            Request(mi.Text.Substring(5));
        }

        private void networkContextMenu_Popup(object sender, System.EventArgs e)
        {
            while (networkContextMenu.MenuItems.Count > 2)
            {
                networkContextMenu.MenuItems.RemoveAt(2);
            }
            networkSeparatorMenuItem.Visible = false;

            int pos = 0;
            pos = SniffText.Text.IndexOf("http://", pos);
            while (pos != -1)
            {
                int posfin1 = SniffText.Text.IndexOf("\r\n", pos);
                int posfin2 = SniffText.Text.IndexOf("<", pos);
                int posfin = -1;
                if (posfin1 == -1) posfin = posfin2; else if (posfin2 == -1) posfin = posfin1; else posfin = Math.Min(posfin1, posfin2);
                MenuItem mi = new MenuItem("Open " + SniffText.Text.Substring(pos, posfin - pos), new EventHandler(WebPageOpenMenuSink));
                networkContextMenu.MenuItems.Add(mi);
                networkSeparatorMenuItem.Visible = true;
                pos = SniffText.Text.IndexOf("http://", posfin);
            }
        }

        private void networkCopyMenuItem_Click(object sender, System.EventArgs e)
        {
            Clipboard.SetDataObject(SniffText.Text);
        }

        private void MethodBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            customRequestPanel.Visible = (MethodBox.SelectedIndex == 4);
        }

        private void addressTextBox_TextChanged(object sender, System.EventArgs e)
        {
            try
            {
                directiveObjTextBox.Text = new Uri(addressTextBox.Text).PathAndQuery;
            }
            catch
            {
                directiveObjTextBox.Text = "/";
            }
        }

        private void RequestQueueMenuItem_Click(object sender, System.EventArgs e)
        {
            RequestButton.Text = RequestButton.Text == "Request" ? "Queue" : "Request";
        }

        private void OnDblClick_DirectiveObject(object sender, System.EventArgs e)
        {
            this.directiveObjTextBox.ReadOnly = !this.directiveObjTextBox.ReadOnly;
        }
    }
}
